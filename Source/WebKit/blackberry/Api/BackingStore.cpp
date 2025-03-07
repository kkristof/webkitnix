/*
 * Copyright (C) 2009, 2010, 2011, 2012, 2013 Research In Motion Limited. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "BackingStore.h"

#include "BackingStoreClient.h"
#include "BackingStoreTile.h"
#include "BackingStoreVisualizationViewportAccessor.h"
#include "BackingStore_p.h"
#include "FatFingers.h"
#include "Frame.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "InspectorController.h"
#include "InspectorInstrumentation.h"
#include "Page.h"
#include "SurfacePool.h"
#include "WebPage.h"
#include "WebPageClient.h"
#include "WebPageCompositorClient.h"
#include "WebPageCompositor_p.h"
#include "WebPage_p.h"
#include "WebSettings.h"

#include <BlackBerryPlatformExecutableMessage.h>
#include <BlackBerryPlatformGraphics.h>
#include <BlackBerryPlatformIntRectRegion.h>
#include <BlackBerryPlatformLog.h>
#include <BlackBerryPlatformMessage.h>
#include <BlackBerryPlatformMessageClient.h>
#include <BlackBerryPlatformScreen.h>
#include <BlackBerryPlatformSettings.h>
#include <BlackBerryPlatformViewportAccessor.h>
#include <BlackBerryPlatformWindow.h>

#include <wtf/CurrentTime.h>
#include <wtf/MathExtras.h>
#include <wtf/NotFound.h>

#define SUPPRESS_NON_VISIBLE_REGULAR_RENDER_JOBS 0
#define ENABLE_SCROLLBARS 1
#define ENABLE_REPAINTONSCROLL 1
#define DEBUG_BACKINGSTORE 0
#define DEBUG_CHECKERBOARD 0
#define DEBUG_WEBCORE_REQUESTS 0
#define DEBUG_VISUALIZE 0
#define DEBUG_TILEMATRIX 0

using namespace WebCore;
using namespace std;

using BlackBerry::Platform::Graphics::Window;
using BlackBerry::Platform::IntRect;
using BlackBerry::Platform::IntPoint;
using BlackBerry::Platform::IntSize;

namespace BlackBerry {
namespace WebKit {

WebPage* BackingStorePrivate::s_currentBackingStoreOwner = 0;

typedef std::pair<int, int> Divisor;
typedef Vector<Divisor> DivisorList;
// FIXME: Cache this and/or use a smarter algorithm.
static DivisorList divisors(unsigned n)
{
    DivisorList divisors;
    for (unsigned i = 1; i <= n; ++i)
        if (!(n % i))
            divisors.append(std::make_pair(i, n / i));
    return divisors;
}

static bool divisorIsPerfectWidth(Divisor divisor, Platform::IntSize size, int tileWidth)
{
    return size.width() <= divisor.first * tileWidth && abs(size.width() - divisor.first * tileWidth) < tileWidth;
}

static bool divisorIsPerfectHeight(Divisor divisor, Platform::IntSize size, int tileHeight)
{
    return size.height() <= divisor.second * tileHeight && abs(size.height() - divisor.second * tileHeight) < tileHeight;
}

static bool divisorIsPreferredDirection(Divisor divisor, BackingStorePrivate::TileMatrixDirection direction)
{
    if (direction == BackingStorePrivate::Vertical)
        return divisor.second > divisor.first;
    return divisor.first > divisor.second;
}

// Compute best divisor given the ratio determined by size.
static Divisor bestDivisor(Platform::IntSize size, int tileWidth, int tileHeight,
                           int minimumNumberOfTilesWide, int minimumNumberOfTilesHigh,
                           BackingStorePrivate::TileMatrixDirection direction)
{
    // The point of this function is to determine the number of tiles in each
    // dimension. We do this by looking to match the tile matrix width/height
    // ratio as closely as possible with the width/height ratio of the contents.
    // We also look at the direction passed to give preference to one dimension
    // over another. This method could probably be made faster, but it gets the
    // job done.
    SurfacePool* surfacePool = SurfacePool::globalSurfacePool();
    ASSERT(!surfacePool->isEmpty());

    // Store a static list of possible divisors.
    static DivisorList divisorList = divisors(surfacePool->numberOfBackingStoreFrontBuffers());

    // The ratio we're looking to best imitate.
    float ratio = static_cast<float>(size.width()) / static_cast<float>(size.height());

    Divisor bestDivisor;
    for (size_t i = 0; i < divisorList.size(); ++i) {
        Divisor divisor = divisorList[i];

        const bool isPerfectWidth = divisorIsPerfectWidth(divisor, size, tileWidth);
        const bool isPerfectHeight = divisorIsPerfectHeight(divisor, size, tileHeight);
        const bool isValidWidth = divisor.first >= minimumNumberOfTilesWide || isPerfectWidth;
        const bool isValidHeight = divisor.second >= minimumNumberOfTilesHigh || isPerfectHeight;
        if (!isValidWidth || !isValidHeight)
            continue;

        if (isPerfectWidth || isPerfectHeight) {
            bestDivisor = divisor; // Found a perfect fit!
#if DEBUG_TILEMATRIX
            Platform::logAlways(Platform::LogLevelCritical,
                "bestDivisor found perfect size isPerfectWidth=%s isPerfectHeight=%s",
                isPerfectWidth ? "true" : "false",
                isPerfectHeight ? "true" : "false");
#endif
            break;
        }

        // Store basis of comparison.
        if (!bestDivisor.first || !bestDivisor.second) {
            bestDivisor = divisor;
            continue;
        }

        // If the current best divisor agrees with the preferred tile matrix direction,
        // then continue if the current candidate does not.
        if (divisorIsPreferredDirection(bestDivisor, direction) && !divisorIsPreferredDirection(divisor, direction))
            continue;

        // Compare ratios.
        float diff1 = fabs((static_cast<float>(divisor.first) / static_cast<float>(divisor.second)) - ratio);
        float diff2 = fabs((static_cast<float>(bestDivisor.first) / static_cast<float>(bestDivisor.second)) - ratio);
        if (diff1 < diff2)
            bestDivisor = divisor;
    }

    return bestDivisor;
}

Platform::IntRect BackingStoreGeometry::backingStoreRect() const
{
    return Platform::IntRect(backingStoreOffset(), backingStoreSize());
}

Platform::IntSize BackingStoreGeometry::backingStoreSize() const
{
    return Platform::IntSize(numberOfTilesWide() * BackingStorePrivate::tileWidth(), numberOfTilesHigh() * BackingStorePrivate::tileHeight());
}

bool BackingStoreGeometry::isTileCorrespondingToBuffer(TileIndex index, TileBuffer* tileBuffer) const
{
    return tileBuffer
        && scale() == tileBuffer->lastRenderScale()
        && originOfTile(index) == tileBuffer->lastRenderOrigin();
}

BackingStorePrivate::BackingStorePrivate()
    : m_suspendScreenUpdateCounterWebKitThread(0)
    , m_suspendBackingStoreUpdates(0)
    , m_suspendGeometryUpdates(0)
    , m_resumeOperation(BackingStore::None)
    , m_suspendScreenUpdatesWebKitThread(true)
    , m_suspendScreenUpdatesUserInterfaceThread(true)
    , m_suspendRenderJobs(false)
    , m_suspendRegularRenderJobs(false)
    , m_tileMatrixContainsUsefulContent(false)
    , m_tileMatrixNeedsUpdate(false)
    , m_isScrollingOrZooming(false)
    , m_webPage(0)
    , m_client(0)
    , m_renderQueue(adoptPtr(new RenderQueue(this)))
    , m_hasBlitJobs(false)
    , m_webPageBackgroundColor(WebCore::Color::white)
    , m_preferredTileMatrixDimension(Vertical)
{
    m_frontState = reinterpret_cast<unsigned>(new BackingStoreGeometry);
}

BackingStorePrivate::~BackingStorePrivate()
{
    BackingStoreGeometry* front = reinterpret_cast<BackingStoreGeometry*>(m_frontState);
    delete front;
    m_frontState = 0;
}

void BackingStorePrivate::instrumentBeginFrame()
{
#if ENABLE(INSPECTOR)
    WebPagePrivate::core(m_webPage)->inspectorController()->didBeginFrame();
#endif
}

void BackingStorePrivate::instrumentCancelFrame()
{
#if ENABLE(INSPECTOR)
    WebPagePrivate::core(m_webPage)->inspectorController()->didCancelFrame();
#endif
}

bool BackingStorePrivate::isOpenGLCompositing() const
{
    if (Window* window = m_webPage->client()->window())
        return window->windowUsage() == Window::GLES2Usage;

    // If there's no window, OpenGL rendering is currently the only option.
    return true;
}

void BackingStorePrivate::suspendBackingStoreUpdates()
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());

    if (m_suspendBackingStoreUpdates) {
        BBLOG(Platform::LogLevelInfo,
            "Backingstore already suspended, increasing suspend counter.");
    }

    ++m_suspendBackingStoreUpdates;
}

void BackingStorePrivate::suspendGeometryUpdates()
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());

    if (m_suspendGeometryUpdates) {
        BBLOG(Platform::LogLevelInfo,
            "Backingstore geometry already suspended, increasing suspend counter.");
    }

    ++m_suspendGeometryUpdates;
}

void BackingStorePrivate::suspendScreenUpdates()
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());

    if (m_suspendScreenUpdateCounterWebKitThread) {
        BBLOG(Platform::LogLevelInfo,
            "Screen already suspended, increasing suspend counter.");
    }

    // Make sure the user interface thread gets the message before we proceed
    // because blitVisibleContents() can be called from the user interface
    // thread and it must honor this flag.
    ++m_suspendScreenUpdateCounterWebKitThread;
    updateSuspendScreenUpdateState();
}

void BackingStorePrivate::resumeBackingStoreUpdates()
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());

    ASSERT(m_suspendBackingStoreUpdates >= 1);
    if (m_suspendBackingStoreUpdates < 1) {
        Platform::logAlways(Platform::LogLevelCritical,
            "Call mismatch: Backingstore hasn't been suspended, therefore won't resume!");
        return;
    }

    // Set a flag indicating that we're about to resume backingstore updates and
    // the tile matrix should be updated as a consequence by the first render
    // job that happens after this resumption of backingstore updates.
    if (m_suspendBackingStoreUpdates == 1)
        setTileMatrixNeedsUpdate();

    --m_suspendBackingStoreUpdates;

    dispatchRenderJob();
}


void BackingStorePrivate::resumeGeometryUpdates()
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());

    ASSERT(m_suspendGeometryUpdates >= 1);
    if (m_suspendGeometryUpdates < 1) {
        Platform::logAlways(Platform::LogLevelCritical,
            "Call mismatch: Backingstore geometry hasn't been suspended, therefore won't resume!");
        return;
    }

    // Set a flag indicating that we're about to resume geometry updates and
    // the tile matrix should be updated as a consequence by the first render
    // job that happens after this resumption of geometry updates.
    if (m_suspendGeometryUpdates == 1)
        setTileMatrixNeedsUpdate();

    --m_suspendGeometryUpdates;

    dispatchRenderJob();
}

void BackingStorePrivate::resumeScreenUpdates(BackingStore::ResumeUpdateOperation op)
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());
    ASSERT(m_suspendScreenUpdateCounterWebKitThread);

    if (!m_suspendScreenUpdateCounterWebKitThread) {
        Platform::logAlways(Platform::LogLevelCritical,
            "Call mismatch: Screen hasn't been suspended, therefore won't resume!");
        return;
    }

    // Out of all nested resume calls, resume with the maximum-impact operation.
    if (op == BackingStore::RenderAndBlit
        || (m_resumeOperation == BackingStore::None && op == BackingStore::Blit))
        m_resumeOperation = op;

    if (m_suspendScreenUpdateCounterWebKitThread >= 2) { // we're still suspended
        BBLOG(Platform::LogLevelInfo,
            "Screen and backingstore still suspended, decreasing suspend counter.");
        --m_suspendScreenUpdateCounterWebKitThread;
        return;
    }

    op = m_resumeOperation;
    m_resumeOperation = BackingStore::None;

#if USE(ACCELERATED_COMPOSITING)
    if (op != BackingStore::None) {
        if (isOpenGLCompositing() && !isActive()) {
            m_webPage->d->setCompositorDrawsRootLayer(true);
            m_webPage->d->setNeedsOneShotDrawingSynchronization();
            --m_suspendScreenUpdateCounterWebKitThread;
            updateSuspendScreenUpdateState();
            return;
        }

        m_webPage->d->setNeedsOneShotDrawingSynchronization();
    }
#endif

    // Render visible contents if necessary.
    if (op == BackingStore::RenderAndBlit) {
        updateTileMatrixIfNeeded();
        TileIndexList visibleTiles = visibleTileIndexes(frontState());
        TileIndexList renderedTiles = render(visibleTiles);

        if (renderedTiles.size() != visibleTiles.size()) {
            // Add unrendered leftover tiles to the render queue.
            for (unsigned i = 0; i < visibleTiles.size(); ++i) {
                if (!renderedTiles.contains(visibleTiles[i])) {
                    Platform::IntRect tileRect(frontState()->originOfTile(visibleTiles[i]), tileSize());
                    m_renderQueue->addToQueue(RenderQueue::VisibleZoom, tileRect);
                }
            }
        }
    }

    // Make sure the user interface thread gets the message before we proceed
    // because blitVisibleContents() can be called from the user interface
    // thread and it must honor this flag.
    --m_suspendScreenUpdateCounterWebKitThread;
    updateSuspendScreenUpdateState();

    if (op == BackingStore::None)
        return;
#if USE(ACCELERATED_COMPOSITING)
    // It needs layout and render before committing root layer if we set OSDS
    if (m_webPage->d->needsOneShotDrawingSynchronization())
        m_webPage->d->requestLayoutIfNeeded();

    // This will also blit since we set the OSDS flag above.
    m_webPage->d->commitRootLayerIfNeeded();
#else
    // Do some blitting if necessary.
    if (op == BackingStore::Blit || op == BackingStore::RenderAndBlit)
        blitVisibleContents();
#endif
}

void BackingStorePrivate::updateSuspendScreenUpdateState(bool* hasSyncedToUserInterfaceThread)
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());

    bool isBackingStoreUsable = isActive() && m_tileMatrixContainsUsefulContent
        && (m_suspendBackingStoreUpdates || !m_renderQueue->hasCurrentVisibleZoomJob()); // Backingstore is not usable while we're waiting for an ("atomic") zoom job to finish.

    bool shouldSuspend = m_suspendScreenUpdateCounterWebKitThread
        || !m_webPage->isVisible()
        || (!isBackingStoreUsable && !m_webPage->d->compositorDrawsRootLayer());

    if (m_suspendScreenUpdatesWebKitThread == shouldSuspend) {
        if (hasSyncedToUserInterfaceThread)
            *hasSyncedToUserInterfaceThread = false;
        return;
    }

    m_suspendScreenUpdatesWebKitThread = shouldSuspend;

    // FIXME: If we change the backingstore to dispatch geometries, this
    //   assignment should be moved to a dispatched setter function instead.
    m_suspendScreenUpdatesUserInterfaceThread = shouldSuspend;
    BlackBerry::Platform::userInterfaceThreadMessageClient()->syncToCurrentMessage();
    if (hasSyncedToUserInterfaceThread)
        *hasSyncedToUserInterfaceThread = true;
}

void BackingStorePrivate::repaint(const Platform::IntRect& windowRect,
                                  bool contentChanged, bool immediate)
{
     // If immediate is true, then we're being asked to perform synchronously.
     // NOTE: WebCore::ScrollView will call this method with immediate:true and contentChanged:false.
     // This is a special case introduced specifically for the Apple's windows port and can be safely ignored I believe.
     // Now this method will be called from WebPagePrivate::repaint().

    if (contentChanged && !windowRect.isEmpty()) {
        // This windowRect is in document coordinates relative to the viewport,
        // but we need it in pixel contents coordinates.
        const Platform::ViewportAccessor* viewportAccessor = m_webPage->webkitThreadViewportAccessor();
        Platform::IntRect rect = viewportAccessor->roundToPixelFromDocumentContents(viewportAccessor->documentContentsFromViewport(windowRect));
        rect.intersect(viewportAccessor->pixelContentsRect());

        if (rect.isEmpty())
            return;

#if DEBUG_WEBCORE_REQUESTS
        Platform::logAlways(Platform::LogLevelCritical,
            "BackingStorePrivate::repaint rect=%s contentChanged=%s immediate=%s",
            rect.toString().c_str(),
            contentChanged ? "true" : "false",
            immediate ? "true" : "false");
#endif

        if (immediate)
            renderAndBlitImmediately(rect);
        else
            m_renderQueue->addToQueue(RenderQueue::RegularRender, rect);
    }
}

void BackingStorePrivate::slowScroll(const Platform::IntSize& delta, const Platform::IntRect& windowRect, bool immediate)
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());

#if DEBUG_BACKINGSTORE
    // Start the time measurement...
    double time = WTF::currentTime();
#endif

    scrollingStartedHelper(delta);

    // This windowRect is in document coordinates relative to the viewport,
    // but we need it in pixel contents coordinates.
    const Platform::ViewportAccessor* viewportAccessor = m_webPage->webkitThreadViewportAccessor();
    const Platform::IntRect rect = viewportAccessor->roundToPixelFromDocumentContents(viewportAccessor->documentContentsFromViewport(windowRect));

    if (immediate)
        renderAndBlitImmediately(rect);
    else {
        m_renderQueue->addToQueue(RenderQueue::VisibleScroll, rect);
        // We only blit here if the client did not generate the scroll as the
        // client supports blitting asynchronously during scroll operations.
        if (!m_client->isClientGeneratedScroll())
            blitVisibleContents();
    }

#if DEBUG_BACKINGSTORE
    // Stop the time measurement.
    double elapsed = WTF::currentTime() - time;
    Platform::logAlways(Platform::LogLevelCritical, "BackingStorePrivate::slowScroll elapsed=%f", elapsed);
#endif
}

void BackingStorePrivate::scroll(const Platform::IntSize& delta,
                                 const Platform::IntRect& scrollViewRect,
                                 const Platform::IntRect& clipRect)
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());

#if DEBUG_BACKINGSTORE
    // Start the time measurement...
    double time = WTF::currentTime();
#endif

    scrollingStartedHelper(delta);

    // We only blit here if the client did not generate the scroll as the client
    // now supports blitting asynchronously during scroll operations.
    if (!m_client->isClientGeneratedScroll())
        blitVisibleContents();

#if DEBUG_BACKINGSTORE
    // Stop the time measurement.
    double elapsed = WTF::currentTime() - time;
    Platform::logAlways(Platform::LogLevelCritical, "BackingStorePrivate::scroll dx=%d, dy=%d elapsed=%f", delta.width(), delta.height(), elapsed);
#endif
}

void BackingStorePrivate::scrollingStartedHelper(const Platform::IntSize& delta)
{
    // Notify the render queue so that it can shuffle accordingly.
    m_renderQueue->updateSortDirection(delta.width(), delta.height());
    m_renderQueue->visibleContentChanged(visibleContentsRect());

    // Scroll the actual backingstore.
    scrollBackingStore(delta.width(), delta.height());

    // Add any newly visible tiles that have not been previously rendered to the queue
    // and check if the tile was previously rendered by regular render job.
    updateTilesForScrollOrNotRenderedRegion();
}

bool BackingStorePrivate::shouldSuppressNonVisibleRegularRenderJobs() const
{
#if SUPPRESS_NON_VISIBLE_REGULAR_RENDER_JOBS
    return true;
#else
    // Always suppress when loading as this drastically decreases page loading
    // time...
    return m_client->isLoading();
#endif
}

bool BackingStorePrivate::shouldPerformRenderJobs() const
{
    return isActive() && !m_suspendRenderJobs && !m_suspendBackingStoreUpdates && !m_renderQueue->isEmpty(!m_suspendRegularRenderJobs);
}

bool BackingStorePrivate::shouldPerformRegularRenderJobs() const
{
    return shouldPerformRenderJobs() && !m_suspendRegularRenderJobs;
}

static const BlackBerry::Platform::Message::Type RenderJobMessageType = BlackBerry::Platform::Message::generateUniqueMessageType();
class RenderJobMessage : public BlackBerry::Platform::ExecutableMessage {
public:
    RenderJobMessage(BlackBerry::Platform::MessageDelegate* delegate)
        : BlackBerry::Platform::ExecutableMessage(delegate, BlackBerry::Platform::ExecutableMessage::UniqueCoalescing, RenderJobMessageType)
    { }
};

void BackingStorePrivate::dispatchRenderJob()
{
    BlackBerry::Platform::MessageDelegate* messageDelegate = BlackBerry::Platform::createMethodDelegate(&BackingStorePrivate::renderJob, this);
    BlackBerry::Platform::webKitThreadMessageClient()->dispatchMessage(new RenderJobMessage(messageDelegate));
}

void BackingStorePrivate::renderJob()
{
    if (!shouldPerformRenderJobs())
        return;

    instrumentBeginFrame();

#if DEBUG_BACKINGSTORE
    Platform::logAlways(Platform::LogLevelCritical, "BackingStorePrivate::renderJob");
#endif

    m_renderQueue->render(!m_suspendRegularRenderJobs);

    if (shouldPerformRenderJobs())
        dispatchRenderJob();
}

Platform::IntRect BackingStorePrivate::expandedContentsRect() const
{
    return Platform::IntRect(Platform::IntPoint(0, 0), expandedContentsSize());
}

Platform::IntRect BackingStorePrivate::visibleContentsRect() const
{
    const Platform::ViewportAccessor* viewportAccessor = m_webPage->webkitThreadViewportAccessor();
    Platform::IntRect rect = viewportAccessor->pixelViewportRect();
    rect.intersect(viewportAccessor->pixelContentsRect());
    return rect;
}

Platform::IntRect BackingStorePrivate::unclippedVisibleContentsRect() const
{
    const Platform::ViewportAccessor* viewportAccessor = m_webPage->webkitThreadViewportAccessor();
    return viewportAccessor->pixelViewportRect();
}

bool BackingStorePrivate::shouldMoveLeft(const Platform::IntRect& backingStoreRect) const
{
    return canMoveX(backingStoreRect)
            && backingStoreRect.x() > visibleContentsRect().x()
            && backingStoreRect.x() > expandedContentsRect().x();
}

bool BackingStorePrivate::shouldMoveRight(const Platform::IntRect& backingStoreRect) const
{
    return canMoveX(backingStoreRect)
            && backingStoreRect.right() < visibleContentsRect().right()
            && backingStoreRect.right() < expandedContentsRect().right();
}

bool BackingStorePrivate::shouldMoveUp(const Platform::IntRect& backingStoreRect) const
{
    return canMoveY(backingStoreRect)
            && backingStoreRect.y() > visibleContentsRect().y()
            && backingStoreRect.y() > expandedContentsRect().y();
}

bool BackingStorePrivate::shouldMoveDown(const Platform::IntRect& backingStoreRect) const
{
    return canMoveY(backingStoreRect)
            && backingStoreRect.bottom() < visibleContentsRect().bottom()
            && backingStoreRect.bottom() < expandedContentsRect().bottom();
}

bool BackingStorePrivate::canMoveX(const Platform::IntRect& backingStoreRect) const
{
    return backingStoreRect.width() > visibleContentsRect().width();
}

bool BackingStorePrivate::canMoveY(const Platform::IntRect& backingStoreRect) const
{
    return backingStoreRect.height() > visibleContentsRect().height();
}

bool BackingStorePrivate::canMoveLeft(const Platform::IntRect& rect) const
{
    Platform::IntRect backingStoreRect = rect;
    Platform::IntRect visibleContentsRect = this->visibleContentsRect();
    Platform::IntRect contentsRect = this->expandedContentsRect();
    backingStoreRect.move(-tileWidth(), 0);
    return backingStoreRect.right() >= visibleContentsRect.right()
            && backingStoreRect.x() >= contentsRect.x();
}

bool BackingStorePrivate::canMoveRight(const Platform::IntRect& rect) const
{
    Platform::IntRect backingStoreRect = rect;
    Platform::IntRect visibleContentsRect = this->visibleContentsRect();
    Platform::IntRect contentsRect = this->expandedContentsRect();
    backingStoreRect.move(tileWidth(), 0);
    return backingStoreRect.x() <= visibleContentsRect.x()
            && (backingStoreRect.right() <= contentsRect.right()
            || (backingStoreRect.right() - contentsRect.right()) < tileWidth());
}

bool BackingStorePrivate::canMoveUp(const Platform::IntRect& rect) const
{
    Platform::IntRect backingStoreRect = rect;
    Platform::IntRect visibleContentsRect = this->visibleContentsRect();
    Platform::IntRect contentsRect = this->expandedContentsRect();
    backingStoreRect.move(0, -tileHeight());
    return backingStoreRect.bottom() >= visibleContentsRect.bottom()
            && backingStoreRect.y() >= contentsRect.y();
}

bool BackingStorePrivate::canMoveDown(const Platform::IntRect& rect) const
{
    Platform::IntRect backingStoreRect = rect;
    Platform::IntRect visibleContentsRect = this->visibleContentsRect();
    Platform::IntRect contentsRect = this->expandedContentsRect();
    backingStoreRect.move(0, tileHeight());
    return backingStoreRect.y() <= visibleContentsRect.y()
            && (backingStoreRect.bottom() <= contentsRect.bottom()
            || (backingStoreRect.bottom() - contentsRect.bottom()) < tileHeight());
}

Platform::IntRect BackingStorePrivate::backingStoreRectForScroll(int deltaX, int deltaY, const Platform::IntRect& rect) const
{
    // The current rect.
    Platform::IntRect backingStoreRect = rect;

    // This method uses the delta values to describe the backingstore rect
    // given the current scroll direction and the viewport position. However,
    // this method can be called with no deltas whatsoever for instance when
    // the contents size changes or the orientation changes. In this case, we
    // want to use the previous scroll direction to describe the backingstore
    // rect. This will result in less checkerboard.
    if (!deltaX && !deltaY) {
        deltaX = m_previousDelta.width();
        deltaY = m_previousDelta.height();
    }
    m_previousDelta = Platform::IntSize(deltaX, deltaY);

    // Return to origin if need be.
    if (!canMoveX(backingStoreRect) && backingStoreRect.x())
        backingStoreRect.setX(0);

    if (!canMoveY(backingStoreRect) && backingStoreRect.y())
        backingStoreRect.setY(0);

    // Move the rect left.
    while (shouldMoveLeft(backingStoreRect) || (deltaX > 0 && canMoveLeft(backingStoreRect)))
        backingStoreRect.move(-tileWidth(), 0);

    // Move the rect right.
    while (shouldMoveRight(backingStoreRect) || (deltaX < 0 && canMoveRight(backingStoreRect)))
        backingStoreRect.move(tileWidth(), 0);

    // Move the rect up.
    while (shouldMoveUp(backingStoreRect) || (deltaY > 0 && canMoveUp(backingStoreRect)))
        backingStoreRect.move(0, -tileHeight());

    // Move the rect down.
    while (shouldMoveDown(backingStoreRect) || (deltaY < 0 && canMoveDown(backingStoreRect)))
        backingStoreRect.move(0, tileHeight());

    return backingStoreRect;
}

void BackingStorePrivate::setBackingStoreRect(const Platform::IntRect& backingStoreRect, double scale)
{
    if (!m_webPage->isVisible())
        return;

    if (!isActive()) {
        m_webPage->d->setShouldResetTilesWhenShown(true);
        return;
    }

    if (m_suspendBackingStoreUpdates || m_suspendGeometryUpdates)
        return;

    Platform::IntRect oldBackingStoreRect = frontState()->backingStoreRect();
    double currentScale = frontState()->scale();

    if (backingStoreRect == oldBackingStoreRect && scale == currentScale)
        return;

#if DEBUG_TILEMATRIX
    Platform::logAlways(Platform::LogLevelCritical,
        "BackingStorePrivate::setBackingStoreRect changed from %s to %s",
        oldBackingStoreRect.toString().c_str(),
        backingStoreRect.toString().c_str());
#endif

    BackingStoreGeometry* oldGeometry = frontState();
    TileMap oldTileMap = oldGeometry->tileMap();

    TileIndexList indexesToFill = indexesForBackingStoreRect(backingStoreRect);

    ASSERT(static_cast<int>(indexesToFill.size()) == oldTileMap.size());

    m_renderQueue->clear(oldBackingStoreRect, RenderQueue::DontClearRegularRenderJobs);
    m_renderQueue->backingStoreRectChanging(oldBackingStoreRect, backingStoreRect);

    TileMap newTileMap;
    TileMap leftOverTiles;

    // Iterate through our current tile map and add tiles that are rendered with
    // our new backing store rect.
    TileMap::const_iterator tileMapEnd = oldTileMap.end();
    for (TileMap::const_iterator it = oldTileMap.begin(); it != tileMapEnd; ++it) {
        TileIndex oldIndex = it->key;
        TileBuffer* oldTileBuffer = it->value;

        // If the new backing store rect contains this origin, then insert the tile there
        // and mark it as no longer shifted. Note: Platform::IntRect::contains checks for a 1x1 rect
        // below and to the right of the origin so it is correct usage here.
        if (oldTileBuffer && backingStoreRect.contains(oldTileBuffer->lastRenderOrigin())) {
            TileIndex newIndex = indexOfTile(oldTileBuffer->lastRenderOrigin(), backingStoreRect);

            size_t i = indexesToFill.find(newIndex);
            ASSERT(i != WTF::notFound);
            indexesToFill.remove(i);
            newTileMap.add(newIndex, oldTileBuffer);
        } else {
            // Store this tile and index so we can add it to the remaining left over spots...
            leftOverTiles.add(oldIndex, oldTileBuffer);
        }
    }

    ASSERT(static_cast<int>(indexesToFill.size()) == leftOverTiles.size());
    size_t i = 0;
    TileMap::const_iterator leftOverEnd = leftOverTiles.end();
    for (TileMap::const_iterator it = leftOverTiles.begin(); it != leftOverEnd; ++it) {
        TileBuffer* oldTileBuffer = it->value;
        if (i >= indexesToFill.size()) {
            ASSERT_NOT_REACHED();
            break;
        }

        TileIndex newIndex = indexesToFill.at(i);
        newTileMap.add(newIndex, oldTileBuffer);

        ++i;
    }

    // Checks to make sure we haven't lost any tiles.
    ASSERT(oldTileMap.size() == newTileMap.size());

    BackingStoreGeometry* newGeometry = new BackingStoreGeometry;
    newGeometry->setScale(scale);
    newGeometry->setNumberOfTilesWide(backingStoreRect.width() / tileWidth());
    newGeometry->setNumberOfTilesHigh(backingStoreRect.height() / tileHeight());
    newGeometry->setBackingStoreOffset(backingStoreRect.location());
    newGeometry->setTileMap(newTileMap);
    adoptAsFrontState(newGeometry); // swap into UI thread

    // Mark tiles as needing update.
    updateTilesAfterBackingStoreRectChange();
}

void BackingStorePrivate::updateTilesAfterBackingStoreRectChange()
{
    BackingStoreGeometry* geometry = frontState();
    TileMap currentMap = geometry->tileMap();

    TileMap::const_iterator end = currentMap.end();
    for (TileMap::const_iterator it = currentMap.begin(); it != end; ++it) {
        TileIndex index = it->key;
        TileBuffer* tileBuffer = it->value;
        Platform::IntPoint tileOrigin = geometry->originOfTile(index);
        // The rect in transformed contents coordinates.
        Platform::IntRect rect(tileOrigin, tileSize());

        if (geometry->isTileCorrespondingToBuffer(index, tileBuffer)) {
            if (m_renderQueue->regularRenderJobsPreviouslyAttemptedButNotRendered(rect)) {
                // If the render queue previously tried to render this tile, but the
                // tile wasn't visible at the time we can't simply restore the tile
                // since the content is now invalid as far as WebKit is concerned.
                // Instead, we clear that part of the tile if it is visible and then
                // put the tile in the render queue again.

                // Intersect the tile with the not rendered region to get the areas
                // of the tile that we need to clear.
                Platform::IntRectRegion tileNotRenderedRegion = Platform::IntRectRegion::intersectRegions(m_renderQueue->regularRenderJobsNotRenderedRegion(), rect);
                clearAndUpdateTileOfNotRenderedRegion(index, tileBuffer, tileNotRenderedRegion, geometry);
#if DEBUG_BACKINGSTORE
                Platform::logAlways(Platform::LogLevelCritical,
                    "BackingStorePrivate::updateTilesAfterBackingStoreRectChange did clear tile %s",
                    tileNotRenderedRegion.extents().toString().c_str());
#endif
            } else {
                if (!tileBuffer || !tileBuffer->isRendered(tileVisibleContentsRect(index, geometry), geometry->scale())
                    && !isCurrentVisibleJob(index, geometry))
                    updateTile(tileOrigin, false /*immediate*/);
            }
        } else if (rect.intersects(expandedContentsRect()))
            updateTile(tileOrigin, false /*immediate*/);
    }
}

TileIndexList BackingStorePrivate::indexesForBackingStoreRect(const Platform::IntRect& backingStoreRect) const
{
    TileIndexList indexes;
    int numberOfTilesWide = backingStoreRect.width() / tileWidth();
    int numberOfTilesHigh = backingStoreRect.height() / tileHeight();
    for (int y = 0; y < numberOfTilesHigh; ++y) {
        for (int x = 0; x < numberOfTilesWide; ++x) {
            TileIndex index(x, y);
            indexes.append(index);
        }
    }
    return indexes;
}

TileIndex BackingStorePrivate::indexOfTile(const Platform::IntPoint& origin,
                                           const Platform::IntRect& backingStoreRect) const
{
    int offsetX = origin.x() - backingStoreRect.x();
    int offsetY = origin.y() - backingStoreRect.y();
    if (offsetX)
        offsetX = offsetX / tileWidth();
    if (offsetY)
        offsetY = offsetY / tileHeight();
    return TileIndex(offsetX, offsetY);
}

void BackingStorePrivate::clearAndUpdateTileOfNotRenderedRegion(const TileIndex& index, TileBuffer* tileBuffer,
                                                                const Platform::IntRectRegion& tileNotRenderedRegion,
                                                                BackingStoreGeometry* geometry,
                                                                bool update)
{
    if (tileNotRenderedRegion.isEmpty())
        return;

    // Clear the render queue of this region.
    m_renderQueue->clear(tileNotRenderedRegion, RenderQueue::ClearAnyJobs);

    if (update) {
        // Add it again as a regular render job.
        m_renderQueue->addToQueue(RenderQueue::RegularRender, tileNotRenderedRegion);
    }

    if (!tileBuffer)
        return;

    // If the region in question is already marked as not rendered, return early
    if (Platform::IntRectRegion::intersectRegions(tileBuffer->renderedRegion(), tileNotRenderedRegion).isEmpty())
        return;

    // Clear the tile of this region. The back buffer region is invalid anyway, but the front
    // buffer must not be manipulated without synchronization with the compositing thread, or
    // we have a race.
    // Instead of using the customary sequence of copy-back, modify and swap, we send a synchronous
    // message to the compositing thread to avoid the copy-back step and save memory bandwidth.
    // The trade-off is that the WebKit thread might wait a little longer for the compositing thread
    // than it would from a waitForCurrentMessage() call.

    ASSERT(Platform::webKitThreadMessageClient()->isCurrentThread());
    if (!Platform::webKitThreadMessageClient()->isCurrentThread())
        return;

    Platform::userInterfaceThreadMessageClient()->dispatchSyncMessage(
        Platform::createMethodCallMessage(&BackingStorePrivate::clearRenderedRegion,
            this, tileBuffer, tileNotRenderedRegion));
}

void BackingStorePrivate::clearRenderedRegion(TileBuffer* tileBuffer, const Platform::IntRectRegion& region)
{
    ASSERT(Platform::userInterfaceThreadMessageClient()->isCurrentThread());
    if (!Platform::userInterfaceThreadMessageClient()->isCurrentThread())
        return;
    if (!tileBuffer)
        return;

    tileBuffer->clearRenderedRegion(region);
}

bool BackingStorePrivate::isCurrentVisibleJob(const TileIndex& index, BackingStoreGeometry* geometry) const
{
    return m_renderQueue->isCurrentVisibleZoomJob(index)
        || m_renderQueue->isCurrentVisibleScrollJob(index)
        || m_renderQueue->isCurrentVisibleZoomJobCompleted(index)
        || m_renderQueue->isCurrentVisibleScrollJobCompleted(index)
        || m_renderQueue->isCurrentRegularRenderJob(index, geometry);
}

void BackingStorePrivate::scrollBackingStore(int deltaX, int deltaY)
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());

    if (!m_webPage->isVisible())
        return;

    if (!isActive()) {
        m_webPage->d->setShouldResetTilesWhenShown(true);
        return;
    }

    // Calculate our new preferred matrix dimension.
    if (deltaX || deltaY)
        m_preferredTileMatrixDimension = abs(deltaX) > abs(deltaY) ? Horizontal : Vertical;

    // Calculate our preferred matrix geometry.
    Divisor divisor = bestDivisor(expandedContentsSize(),
                                  tileWidth(), tileHeight(),
                                  minimumNumberOfTilesWide(), minimumNumberOfTilesHigh(),
                                  m_preferredTileMatrixDimension);

#if DEBUG_TILEMATRIX
    Platform::logAlways(Platform::LogLevelCritical,
        "BackingStorePrivate::scrollBackingStore divisor %dx%d",
        divisor.first, divisor.second);
#endif

    // Initialize a rect with that new geometry.
    Platform::IntRect backingStoreRect(0, 0, divisor.first * tileWidth(), divisor.second * tileHeight());

    // Scroll that rect so that it fits our contents and viewport and scroll delta.
    backingStoreRect = backingStoreRectForScroll(deltaX, deltaY, backingStoreRect);

    ASSERT(!backingStoreRect.isEmpty());

    setBackingStoreRect(backingStoreRect, m_webPage->d->currentScale());
}

TileIndexList BackingStorePrivate::render(const TileIndexList& tileIndexList)
{
    if (!m_webPage->isVisible())
        return TileIndexList();

    requestLayoutIfNeeded();

    // If no tiles available for us to draw to, someone else has to render the root layer.
    if (!isActive())
        return TileIndexList();

#if DEBUG_BACKINGSTORE
    Platform::logAlways(Platform::LogLevelInfo,
        "BackingStorePrivate::render %d tiles, m_suspendBackingStoreUpdates = %s",
        tileIndexList.size(),
        m_suspendBackingStoreUpdates ? "true" : "false");
#endif

    ASSERT(!m_tileMatrixNeedsUpdate);

    if (tileIndexList.isEmpty())
        return tileIndexList;

    Platform::ViewportAccessor* viewportAccessor = m_webPage->webkitThreadViewportAccessor();

    BackingStoreGeometry* geometry = frontState();
    TileMap oldTileMap = geometry->tileMap();
    double currentScale = geometry->scale();

    BackingStoreGeometry* newGeometry = new BackingStoreGeometry;
    newGeometry->setScale(geometry->scale());
    newGeometry->setNumberOfTilesWide(geometry->numberOfTilesWide());
    newGeometry->setNumberOfTilesHigh(geometry->numberOfTilesHigh());
    newGeometry->setBackingStoreOffset(geometry->backingStoreOffset());
    TileMap newTileMap(oldTileMap); // copy a new, writable version
    TileIndexList renderedTiles;

    for (size_t i = 0; i < tileIndexList.size(); ++i) {
        if (!SurfacePool::globalSurfacePool()->numberOfAvailableBackBuffers()) {
            newGeometry->setTileMap(newTileMap);
            adoptAsFrontState(newGeometry); // this should get us at least one more.

            // newGeometry is now the front state and shouldn't be messed with.
            // Let's create a new one. (The old one will be automatically
            // destroyed by adoptAsFrontState() on being swapped out again.)
            geometry = frontState();
            newGeometry = new BackingStoreGeometry;
            newGeometry->setScale(geometry->scale());
            newGeometry->setNumberOfTilesWide(geometry->numberOfTilesWide());
            newGeometry->setNumberOfTilesHigh(geometry->numberOfTilesHigh());
            newGeometry->setBackingStoreOffset(geometry->backingStoreOffset());
        }

        TileIndex index = tileIndexList[i];
        Platform::IntPoint tileOrigin = newGeometry->originOfTile(index);
        Platform::IntRect dirtyRect(tileOrigin, tileSize());

        // Paint default background if contents rect is empty.
        if (!expandedContentsRect().isEmpty()) {
            // Otherwise we should clip the contents size and render the content.
            dirtyRect.intersect(expandedContentsRect());

            // We probably have extra tiles since the contents size is so small.
            // Save some cycles here...
            if (dirtyRect.isEmpty()) {
#if DEBUG_BACKINGSTORE
                Platform::logAlways(Platform::LogLevelInfo,
                    "BackingStorePrivate::render skipping tile at %s, it's outside the expanded contents rect of %s",
                    newGeometry->originOfTile(index).toString().c_str(),
                    expandedContentsRect().toString().c_str());
#endif
                continue;
            }
        }

        TileBuffer* backBuffer = SurfacePool::globalSurfacePool()->takeBackBuffer();
        ASSERT(backBuffer);

        backBuffer->paintBackground();
        backBuffer->setLastRenderScale(currentScale);
        backBuffer->setLastRenderOrigin(tileOrigin);
        backBuffer->clearRenderedRegion();

        BlackBerry::Platform::Graphics::Buffer* nativeBuffer = backBuffer->nativeBuffer();
        BlackBerry::Platform::Graphics::setBufferOpaqueHint(nativeBuffer, !Color(m_webPage->settings()->backgroundColor()).hasAlpha());

        // TODO: This code is only needed for EGLImage code path, but preferrably BackingStore
        // should not know that, and the synchronization should be in BlackBerry::Platform::Graphics
        // if possible.
        if (isOpenGLCompositing())
            SurfacePool::globalSurfacePool()->waitForBuffer(backBuffer);

        const Platform::FloatPoint documentDirtyRectOrigin = viewportAccessor->toDocumentContents(dirtyRect.location(), currentScale);
        const Platform::IntRect dstRect(dirtyRect.location() - tileOrigin, dirtyRect.size());

        if (!renderContents(nativeBuffer, dstRect, currentScale, documentDirtyRectOrigin, RenderRootLayer))
            continue;

        // Add the newly rendered region to the tile so it can keep track for blits.
        backBuffer->addRenderedRegion(dirtyRect);

        renderedTiles.append(index);
        newTileMap.set(index, backBuffer);
    }

    // Let the render queue know that the tile contents are up to date now.
    m_renderQueue->clear(renderedTiles, frontState(), RenderQueue::DontClearCompletedJobs);

    // If we couldn't render all requested jobs, suspend blitting until we do.
    updateSuspendScreenUpdateState();

    newGeometry->setTileMap(newTileMap);
    adoptAsFrontState(newGeometry);

#if DEBUG_BACKINGSTORE
    Platform::logAlways(Platform::LogLevelInfo,
        "BackingStorePrivate::render done rendering %d tiles.",
        renderedTiles.size());
#endif
    return renderedTiles;
}

void BackingStorePrivate::requestLayoutIfNeeded() const
{
    m_webPage->d->requestLayoutIfNeeded();
}

void BackingStorePrivate::renderAndBlitVisibleContentsImmediately()
{
    renderAndBlitImmediately(visibleContentsRect());
}

void BackingStorePrivate::renderAndBlitImmediately(const Platform::IntRect& rect)
{
    updateTileMatrixIfNeeded();
    m_renderQueue->addToQueue(RenderQueue::VisibleZoom, rect);
    renderJob();
}

void BackingStorePrivate::paintDefaultBackground(const Platform::IntRect& dstRect, Platform::ViewportAccessor* viewportAccessor, bool flush)
{
    Platform::IntRect clippedDstRect = dstRect;

    // Because of rounding it is possible that overScrollRect could be off-by-one larger
    // than the surface size of the window. We prevent this here, by clamping
    // it to ensure that can't happen.
    clippedDstRect.intersect(Platform::IntRect(Platform::IntPoint(0, 0), surfaceSize()));

    if (clippedDstRect.isEmpty())
        return;

    // We have to paint the default background in the case of overzoom and
    // make sure it is invalidated.
    const Platform::IntRect pixelContentsRect = viewportAccessor->pixelContentsRect();
    Platform::IntRectRegion overScrollRegion = Platform::IntRectRegion::subtractRegions(
        clippedDstRect, viewportAccessor->pixelViewportFromContents(pixelContentsRect));

    std::vector<Platform::IntRect> overScrollRects = overScrollRegion.rects();
    for (size_t i = 0; i < overScrollRects.size(); ++i) {
        Platform::IntRect overScrollRect = overScrollRects.at(i);

        if (m_webPage->settings()->isEnableDefaultOverScrollBackground()) {
            fillWindow(BlackBerry::Platform::Graphics::DefaultBackgroundPattern,
                overScrollRect, overScrollRect.location(), 1.0 /*contentsScale*/);
        } else {
            Color color(m_webPage->settings()->overScrollColor());
            clearWindow(overScrollRect, color.red(), color.green(), color.blue(), color.alpha());
        }
    }
}

void BackingStorePrivate::blitVisibleContents(bool force)
{
    if (!BlackBerry::Platform::userInterfaceThreadMessageClient()->isCurrentThread()) {
        BlackBerry::Platform::userInterfaceThreadMessageClient()->dispatchMessage(
            BlackBerry::Platform::createMethodCallMessage(
                &BackingStorePrivate::blitVisibleContents, this, force));
        return;
    }

    if (m_suspendScreenUpdatesUserInterfaceThread) {
        // Avoid client going into busy loop while blit is impossible.
        if (force)
            m_hasBlitJobs = false;
        return;
    }

    if (!force) {
#if USE(ACCELERATED_COMPOSITING)
        // If there's a WebPageCompositorClient, let it schedule the blit.
        if (WebPageCompositorPrivate* compositor = m_webPage->d->compositor()) {
            if (WebPageCompositorClient* client = compositor->client()) {
                client->invalidate(0);
                return;
            }
        }
#endif

        m_hasBlitJobs = true;
        return;
    }

    m_hasBlitJobs = false;

    Platform::ViewportAccessor* viewportAccessor = m_webPage->client()->userInterfaceViewportAccessor();
    if (!viewportAccessor)
        return;
    const Platform::IntRect dstRect = viewportAccessor->destinationSurfaceRect();

    const Platform::IntRect pixelViewportRect = viewportAccessor->pixelViewportRect();
    const Platform::FloatRect documentViewportRect = viewportAccessor->documentFromPixelContents(pixelViewportRect);
    Platform::IntRect pixelSrcRect = pixelViewportRect;
    Platform::FloatRect documentSrcRect = documentViewportRect;

#if DEBUG_VISUALIZE
    // Substitute a srcRect that consists of the whole backingstore geometry
    // instead of the normal viewport so we can visualize the entire
    // backingstore and what it is doing when we scroll and zoom!
    Platform::ViewportAccessor* debugViewportAccessor = new BackingStoreVisualizationViewportAccessor(viewportAccessor, this);
    if (isActive()) {
        viewportAccessor = debugViewportAccessor;
        documentSrcRect = debugViewportAccessor->documentViewportRect();
        pixelSrcRect = debugViewportAccessor->pixelViewportRect();
    }
#endif

#if DEBUG_BACKINGSTORE
    Platform::logAlways(Platform::LogLevelCritical,
        "BackingStorePrivate::blitVisibleContents(): dstRect=%s, documentSrcRect=%s, scale=%f",
        dstRect.toString().c_str(), documentSrcRect.toString().c_str(), viewportAccessor->scale());
#endif

    BlackBerry::Platform::Graphics::Buffer* dstBuffer = buffer();
    ASSERT(dstBuffer);
    if (dstBuffer) {
        // On the GPU, clearing is free and allows for optimizations,
        // so we always want to do this first for the whole surface.
        // Don't call clearWindow() as we don't want to add it to the posted rect.
        BlackBerry::Platform::Graphics::clearBuffer(dstBuffer,
            m_webPageBackgroundColor.red(), m_webPageBackgroundColor.green(),
            m_webPageBackgroundColor.blue(), m_webPageBackgroundColor.alpha());
    } else
        Platform::logAlways(Platform::LogLevelWarn, "Empty window buffer, can't blit contents.");

#if DEBUG_CHECKERBOARD
    bool blitCheckered = false;
#endif

    Vector<TileBuffer*> blittedTiles;

    if (isActive() && !m_webPage->d->compositorDrawsRootLayer()) {
        paintDefaultBackground(dstRect, viewportAccessor, false /*flush*/);

        BackingStoreGeometry* geometry = frontState();
        TileMap currentMap = geometry->tileMap();
        double currentScale = geometry->scale();

        const Platform::IntRect transformedContentsRect = currentScale == viewportAccessor->scale()
            ? viewportAccessor->pixelContentsRect()
            : viewportAccessor->roundFromDocumentContents(viewportAccessor->documentContentsRect(), currentScale);

        // For blitting backingstore tiles, we need the srcRect to be specified
        // in backingstore tile pixel coordinates. If our viewport accessor is
        //  at a different scale, we calculate those coordinates by ourselves.
        const Platform::IntRect transformedSrcRect = currentScale == viewportAccessor->scale()
            ? pixelSrcRect
            : viewportAccessor->roundFromDocumentContents(documentSrcRect, currentScale);

        Platform::IntRect clippedTransformedSrcRect = transformedSrcRect;
        const Platform::IntPoint origin = transformedSrcRect.location();

        // FIXME: This should not explicitly depend on WebCore::.
        TransformationMatrix transformation;
        if (!transformedSrcRect.isEmpty())
            transformation = TransformationMatrix::rectToRect(FloatRect(FloatPoint(0.0, 0.0), WebCore::IntSize(transformedSrcRect.size())), WebCore::IntRect(dstRect));

        // Don't clip to contents if it is empty so we can still paint default background.
        if (!transformedContentsRect.isEmpty()) {
            clippedTransformedSrcRect.intersect(transformedContentsRect);
            if (clippedTransformedSrcRect.isEmpty()) {
                m_webPage->client()->postToSurface(dstRect);
                return;
            }

            Platform::IntRectRegion transformedSrcRegion = clippedTransformedSrcRect;
            Platform::IntRectRegion backingStoreRegion = geometry->backingStoreRect();
            Platform::IntRectRegion checkeredRegion
                = Platform::IntRectRegion::subtractRegions(transformedSrcRegion, backingStoreRegion);

            // Blit checkered to those parts that are not covered by the backingStoreRect.
            std::vector<Platform::IntRect> checkeredRects = checkeredRegion.rects();
            for (size_t i = 0; i < checkeredRects.size(); ++i) {
                Platform::IntRect clippedDstRect = transformation.mapRect(Platform::IntRect(
                    Platform::IntPoint(checkeredRects.at(i).x() - origin.x(), checkeredRects.at(i).y() - origin.y()),
                                       checkeredRects.at(i).size()));
                // To eliminate 1 pixel inflation due to transformation rounding.
                clippedDstRect.intersect(dstRect);
#if DEBUG_CHECKERBOARD
                blitCheckered = true;
#endif

                fillWindow(BlackBerry::Platform::Graphics::CheckerboardPattern,
                    clippedDstRect, checkeredRects.at(i).location(), transformation.a());
            }
        }

        // Get the list of tile rects that makeup the content.
        TileRectList tileRectList = mapFromPixelContentsToTiles(clippedTransformedSrcRect, geometry);
        for (size_t i = 0; i < tileRectList.size(); ++i) {
            TileRect tileRect = tileRectList[i];
            TileIndex index = tileRect.first;
            Platform::IntRect dirtyRect = tileRect.second;

            // Don't clip to contents if it is empty so we can still paint default background.
            if (!transformedContentsRect.isEmpty()) {
                // Otherwise we should clip the contents size and blit.
                dirtyRect.intersect(transformedContentsRect);
            }

            // Save some cycles here...
            if (dirtyRect.isEmpty())
                continue;

            // Now, this dirty rect is in transformed coordinates relative to the
            // transformed contents, but ultimately it needs to be transformed
            // coordinates relative to the viewport.
            dirtyRect.move(-origin.x(), -origin.y());

            TileBuffer* tileBuffer = currentMap.get(index);

            bool isTileCorrespondingToBuffer = geometry->isTileCorrespondingToBuffer(index, tileBuffer);
            bool rendered = tileBuffer && tileBuffer->isRendered(tileRect.second, currentScale);
            bool paintCheckered = !isTileCorrespondingToBuffer || !rendered;

            if (paintCheckered) {
                Platform::IntRect dirtyRectT = transformation.mapRect(dirtyRect);

                if (!transformation.isIdentity()) {
                    // Because of rounding it is possible that dirtyRect could be off-by-one larger
                    // than the surface size of the dst buffer. We prevent this here, by clamping
                    // it to ensure that can't happen.
                    dirtyRectT.intersect(Platform::IntRect(Platform::IntPoint(0, 0), surfaceSize()));
                }
                const Platform::IntPoint contentsOrigin(dirtyRect.x() + origin.x(), dirtyRect.y() + origin.y());
#if DEBUG_CHECKERBOARD
                blitCheckered = true;
#endif
                fillWindow(BlackBerry::Platform::Graphics::CheckerboardPattern,
                    dirtyRectT, contentsOrigin, transformation.a());
            }

            // Blit the visible buffer here if we have visible zoom jobs.
            if (isTileCorrespondingToBuffer) {
                // Intersect the rendered region.
                Platform::IntRectRegion renderedRegion = tileBuffer->renderedRegion();
                std::vector<Platform::IntRect> dirtyRenderedRects = renderedRegion.rects();
                for (size_t j = 0; j < dirtyRenderedRects.size(); ++j) {
                    Platform::IntRect dirtyRenderedRect = intersection(tileRect.second, dirtyRenderedRects.at(j));
                    if (dirtyRenderedRect.isEmpty())
                        continue;
                    // Blit the rendered parts.
                    blitTileRect(tileBuffer, dirtyRenderedRect, origin, transformation, geometry);
                }
                blittedTiles.append(tileBuffer);
            }
        }
    }

    // TODO: This code is only needed for EGLImage code path, but preferrably BackingStore
    // should not know that, and the synchronization should be in BlackBerry::Platform::Graphics
    // if possible.
    if (isOpenGLCompositing())
        SurfacePool::globalSurfacePool()->notifyBuffersComposited(blittedTiles);

#if USE(ACCELERATED_COMPOSITING)
    if (WebPageCompositorPrivate* compositor = m_webPage->d->compositor()) {
        compositor->drawLayers(dstRect, documentSrcRect);
        if (compositor->drawsRootLayer())
            paintDefaultBackground(dstRect, viewportAccessor, false /*flush*/);
    }
#endif

#if DEBUG_VISUALIZE
    if (debugViewportAccessor) {
        Platform::Graphics::Buffer* targetBuffer = buffer();
        Platform::IntRect wkViewport = debugViewportAccessor->roundToPixelFromDocumentContents(Platform::IntRect(m_client->visibleContentsRect()));
        Platform::IntRect uiViewport = debugViewportAccessor->roundToPixelFromDocumentContents(documentViewportRect);
        wkViewport.move(-pixelSrcRect.x(), -pixelSrcRect.y());
        uiViewport.move(-pixelSrcRect.x(), -pixelSrcRect.y());

        // Draw a blue rect for the webkit thread viewport.
        Platform::Graphics::clearBuffer(targetBuffer, Platform::IntRect(wkViewport.x(), wkViewport.y(), wkViewport.width(), 1), 0, 0, 255, 255);
        Platform::Graphics::clearBuffer(targetBuffer, Platform::IntRect(wkViewport.x(), wkViewport.y(), 1, wkViewport.height()), 0, 0, 255, 255);
        Platform::Graphics::clearBuffer(targetBuffer, Platform::IntRect(wkViewport.x(), wkViewport.bottom() - 1, wkViewport.width(), 1), 0, 0, 255, 255);
        Platform::Graphics::clearBuffer(targetBuffer, Platform::IntRect(wkViewport.right() - 1, wkViewport.y(), 1, wkViewport.height()), 0, 0, 255, 255);

        // Draw a red rect for the ui thread viewport.
        Platform::Graphics::clearBuffer(targetBuffer, Platform::IntRect(uiViewport.x(), uiViewport.y(), uiViewport.width(), 1), 255, 0, 0, 255);
        Platform::Graphics::clearBuffer(targetBuffer, Platform::IntRect(uiViewport.x(), uiViewport.y(), 1, uiViewport.height()), 255, 0, 0, 255);
        Platform::Graphics::clearBuffer(targetBuffer, Platform::IntRect(uiViewport.x(), uiViewport.bottom() - 1, uiViewport.width(), 1), 255, 0, 0, 255);
        Platform::Graphics::clearBuffer(targetBuffer, Platform::IntRect(uiViewport.right() - 1, uiViewport.y(), 1, uiViewport.height()), 255, 0, 0, 255);

        delete debugViewportAccessor;
    }
#endif

#if DEBUG_CHECKERBOARD
    static double lastCheckeredTime = 0;

    if (blitCheckered && !lastCheckeredTime) {
        lastCheckeredTime = WTF::currentTime();
        Platform::logAlways(Platform::LogLevelCritical,
            "Blitting checkered pattern at %f", lastCheckeredTime);
    } else if (blitCheckered && lastCheckeredTime) {
        Platform::logAlways(Platform::LogLevelCritical,
            "Blitting checkered pattern at %f", WTF::currentTime());
    } else if (!blitCheckered && lastCheckeredTime) {
        double time = WTF::currentTime();
        Platform::logAlways(Platform::LogLevelCritical,
            "Blitting over checkered pattern at %f took %f", time, time - lastCheckeredTime);
        lastCheckeredTime = 0;
    }
#endif

    m_webPage->client()->postToSurface(dstRect);
}

#if USE(ACCELERATED_COMPOSITING)
void BackingStorePrivate::compositeContents(WebCore::LayerRenderer* layerRenderer, const WebCore::TransformationMatrix& transform, const WebCore::FloatRect& documentContents, bool contentsOpaque)
{
    Platform::ViewportAccessor* viewportAccessor = m_webPage->client()->userInterfaceViewportAccessor();
    if (!viewportAccessor)
        return;

    Platform::IntRect pixelContents = viewportAccessor->roundToPixelFromDocumentContents(documentContents);
    pixelContents.intersect(viewportAccessor->pixelContentsRect());
    if (pixelContents.isEmpty())
        return;

    if (!isActive())
        return;

    if (m_webPage->d->compositorDrawsRootLayer())
        return;

    BackingStoreGeometry* geometry = frontState();
    TileMap currentMap = geometry->tileMap();
    Vector<TileBuffer*> compositedTiles;

    Platform::IntRectRegion pixelContentsRegion = pixelContents;
    Platform::IntRectRegion backingStoreRegion = geometry->backingStoreRect();
    Platform::IntRectRegion clearRegion = Platform::IntRectRegion::subtractRegions(pixelContentsRegion, backingStoreRegion);

    // Clear those parts that are not covered by the backingStoreRect.
    Color clearColor(Color::white);
    std::vector<Platform::IntRect> clearRects = clearRegion.rects();
    for (size_t i = 0; i < clearRects.size(); ++i)
        layerRenderer->drawColor(transform, viewportAccessor->documentFromPixelContents(clearRects.at(i)), clearColor);

    // Get the list of tile rects that makeup the content.
    TileRectList tileRectList = mapFromPixelContentsToTiles(pixelContents, geometry);
    for (size_t i = 0; i < tileRectList.size(); ++i) {
        TileRect tileRect = tileRectList[i];
        TileIndex index = tileRect.first;
        Platform::IntRect dirtyRect = tileRect.second;
        TileBuffer* tileBuffer = currentMap.get(index);

        if (!tileBuffer || !geometry->isTileCorrespondingToBuffer(index, tileBuffer))
            layerRenderer->drawColor(transform, viewportAccessor->documentFromPixelContents(dirtyRect), clearColor);
        else {
            Platform::IntPoint tileOrigin = tileBuffer->lastRenderOrigin();
            Platform::FloatRect tileDocumentContentsRect = viewportAccessor->documentFromPixelContents(tileBuffer->pixelContentsRect());

            layerRenderer->compositeBuffer(transform, tileDocumentContentsRect, tileBuffer->nativeBuffer(), contentsOpaque, 1.0f);
            compositedTiles.append(tileBuffer);

            // Intersect the rendered region and clear unrendered parts.
            Platform::IntRectRegion notRenderedRegion = Platform::IntRectRegion::subtractRegions(dirtyRect, tileBuffer->renderedRegion());
            std::vector<Platform::IntRect> notRenderedRects = notRenderedRegion.rects();
            for (size_t i = 0; i < notRenderedRects.size(); ++i) {
                Platform::IntRect tileSurfaceRect = notRenderedRects.at(i);
                tileSurfaceRect.move(-tileOrigin.x(), -tileOrigin.y());
                layerRenderer->drawColor(transform, viewportAccessor->documentFromPixelContents(tileSurfaceRect), clearColor);
            }
        }
    }

    SurfacePool::globalSurfacePool()->notifyBuffersComposited(compositedTiles);
}
#endif

Platform::IntRect BackingStorePrivate::blitTileRect(TileBuffer* tileBuffer,
    const Platform::IntRect& tilePixelContentsRect,
    const Platform::IntPoint& origin,
    const WebCore::TransformationMatrix& matrix,
    BackingStoreGeometry* geometry)
{
    if (!m_webPage->isVisible() || !isActive() || !tileBuffer)
        return Platform::IntRect();

    Platform::IntRect srcRect = tilePixelContentsRect;
    Platform::IntPoint tileOrigin = tileBuffer->lastRenderOrigin();
    srcRect.move(-tileOrigin.x(), -tileOrigin.y());

    // Now, this dirty rect is in transformed coordinates relative to the
    // transformed contents, but ultimately it needs to be transformed
    // coordinates relative to the viewport.
    Platform::IntRect dstRect = tilePixelContentsRect;
    dstRect.move(-origin.x(), -origin.y());
    dstRect = matrix.mapRect(dstRect);

    if (!matrix.isIdentity()) {
        // Because of rounding it is possible that dstRect could be off-by-one larger
        // than the surface size of the dst buffer. We prevent this here, by clamping
        // it to ensure that can't happen.
        dstRect.intersect(Platform::IntRect(Platform::IntPoint(0, 0), surfaceSize()));
    }

    ASSERT(!dstRect.isEmpty());
    ASSERT(!srcRect.isEmpty());
    if (dstRect.isEmpty() || srcRect.isEmpty())
        return Platform::IntRect();

    blitToWindow(dstRect, tileBuffer->nativeBuffer(), srcRect, BlackBerry::Platform::Graphics::SourceCopy, 255);
    return dstRect;
}

bool BackingStorePrivate::isTileVisible(const TileIndex& index, BackingStoreGeometry* geometry) const
{
    return isTileVisible(geometry->originOfTile(index));
}

bool BackingStorePrivate::isTileVisible(const Platform::IntPoint& origin) const
{
    return Platform::IntRect(origin, tileSize()).intersects(visibleContentsRect());
}

TileIndexList BackingStorePrivate::visibleTileIndexes(BackingStoreGeometry* geometry) const
{
    TileMap tileMap = geometry->tileMap();
    TileIndexList visibleTiles;

    TileMap::const_iterator end = tileMap.end();
    for (TileMap::const_iterator it = tileMap.begin(); it != end; ++it) {
        Platform::IntRect tilePixelContentsRect(geometry->originOfTile(it->key), tileSize());
        if (tilePixelContentsRect.intersects(visibleContentsRect()))
            visibleTiles.append(it->key);
    }
    return visibleTiles;
}

Platform::IntRect BackingStorePrivate::tileVisibleContentsRect(const TileIndex& index, BackingStoreGeometry* geometry) const
{
    if (!isTileVisible(index, geometry))
        return Platform::IntRect();

    return tileContentsRect(index, visibleContentsRect(), geometry);
}

Platform::IntRect BackingStorePrivate::tileContentsRect(const TileIndex& index, const Platform::IntRect& pixelContentsRect, BackingStoreGeometry* state) const
{
    TileRectList tileRectList = mapFromPixelContentsToTiles(pixelContentsRect, state);
    for (size_t i = 0; i < tileRectList.size(); ++i) {
        TileRect tileRect = tileRectList[i];
        if (index == tileRect.first)
            return tileRect.second;
    }
    return Platform::IntRect();
}

void BackingStorePrivate::resetRenderQueue()
{
    m_renderQueue->reset();
}

void BackingStorePrivate::resetTiles()
{
    BackingStoreGeometry* geometry = frontState();

    m_renderQueue->clear(geometry->backingStoreRect(), RenderQueue::ClearAnyJobs);

    BackingStoreGeometry* newGeometry = new BackingStoreGeometry;
    newGeometry->setScale(geometry->scale());
    newGeometry->setNumberOfTilesWide(geometry->numberOfTilesWide());
    newGeometry->setNumberOfTilesHigh(geometry->numberOfTilesHigh());
    newGeometry->setBackingStoreOffset(geometry->backingStoreOffset());

    TileMap currentMap = geometry->tileMap();
    TileMap newTileMap;

    TileMap::const_iterator end = currentMap.end();
    for (TileMap::const_iterator it = currentMap.begin(); it != end; ++it)
        newTileMap.add(it->key, 0); // clear all buffer info from the tile

    newGeometry->setTileMap(newTileMap);
    adoptAsFrontState(newGeometry); // swap into UI thread
}

void BackingStorePrivate::updateTiles(bool updateVisible, bool immediate)
{
    if (!isActive())
        return;

    BackingStoreGeometry* geometry = frontState();
    TileMap currentMap = geometry->tileMap();

    TileMap::const_iterator end = currentMap.end();
    for (TileMap::const_iterator it = currentMap.begin(); it != end; ++it) {
        bool isVisible = isTileVisible(it->key, geometry);
        if (!updateVisible && isVisible)
            continue;
        updateTile(geometry->originOfTile(it->key), immediate);
    }
}

void BackingStorePrivate::updateTilesForScrollOrNotRenderedRegion(bool checkLoading)
{
    // This method looks at all the tiles and if they are visible, but not completely
    // rendered or we are loading, then it updates them. For all tiles, visible and
    // non-visible, if a previous attempt was made to render them during a regular
    // render job, but they were not visible at the time, then update them and if
    // they are currently visible, reset them.

    BackingStoreGeometry* geometry = frontState();
    TileMap currentMap = geometry->tileMap();

    bool isLoading = m_client->loadState() == WebPagePrivate::Committed;
    bool updateNonVisibleTiles = !checkLoading || !isLoading;

    TileMap::const_iterator end = currentMap.end();
    for (TileMap::const_iterator it = currentMap.begin(); it != end; ++it) {
        TileIndex index = it->key;
        TileBuffer* tileBuffer = it->value;
        bool isVisible = isTileVisible(index, geometry);
        Platform::IntPoint tileOrigin = geometry->originOfTile(index);
        // The rect in transformed contents coordinates.
        Platform::IntRect rect(tileOrigin, tileSize());
        if (geometry->isTileCorrespondingToBuffer(index, tileBuffer)
            && m_renderQueue->regularRenderJobsPreviouslyAttemptedButNotRendered(rect)) {
            // If the render queue previously tried to render this tile, but the
            // tile wasn't visible at the time we can't simply restore the tile
            // since the content is now invalid as far as WebKit is concerned.
            // Instead, we clear that part of the tile if it is visible and then
            // put the tile in the render queue again.
            if (isVisible) {
                // Intersect the tile with the not rendered region to get the areas
                // of the tile that we need to clear.
                Platform::IntRectRegion tileNotRenderedRegion = Platform::IntRectRegion::intersectRegions(m_renderQueue->regularRenderJobsNotRenderedRegion(), rect);
                clearAndUpdateTileOfNotRenderedRegion(index, tileBuffer, tileNotRenderedRegion, geometry, false /*update*/);
#if DEBUG_BACKINGSTORE
                Platform::logAlways(Platform::LogLevelCritical,
                    "BackingStorePrivate::updateTilesForScroll did clear tile %s",
                    tileNotRenderedRegion.extents().toString().c_str());
#endif
            }
            updateTile(tileOrigin, false /*immediate*/);
        } else if ((isVisible || updateNonVisibleTiles)
            && (!tileBuffer || !tileBuffer->isRendered(tileVisibleContentsRect(index, geometry), geometry->scale()))
            && !isCurrentVisibleJob(index, geometry))
            updateTile(tileOrigin, false /*immediate*/);
    }
}

void BackingStorePrivate::updateTile(const Platform::IntPoint& origin, bool immediate)
{
    if (!isActive())
        return;

    Platform::IntRect updateRect = Platform::IntRect(origin, tileSize());
    if (immediate)
        renderAndBlitImmediately(updateRect);
    else {
        RenderQueue::JobType jobType = isTileVisible(origin) ? RenderQueue::VisibleScroll : RenderQueue::NonVisibleScroll;
        m_renderQueue->addToQueue(jobType, updateRect);
    }
}

BackingStorePrivate::TileRectList BackingStorePrivate::mapFromPixelContentsToTiles(const Platform::IntRect& rect, BackingStoreGeometry* geometry) const
{
    TileMap tileMap = geometry->tileMap();

    TileRectList tileRectList;
    TileMap::const_iterator end = tileMap.end();
    for (TileMap::const_iterator it = tileMap.begin(); it != end; ++it) {
        TileIndex index = it->key;

        // Need to map the rect to tile coordinates.
        Platform::IntRect r = rect;

        // Do we intersect the current tile or no?
        r.intersect(Platform::IntRect(geometry->originOfTile(index), tileSize()));
        if (r.isEmpty())
            continue;

        // If we do append to list and Voila!
        TileRect tileRect;
        tileRect.first = index;
        tileRect.second = r;
        tileRectList.append(tileRect);
    }
    return tileRectList;
}

void BackingStorePrivate::updateTileMatrixIfNeeded()
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());

    if (!m_tileMatrixNeedsUpdate)
        return;

    m_tileMatrixNeedsUpdate = false;

    // This will update the tile matrix.
    scrollBackingStore(0, 0);
}

void BackingStorePrivate::contentsSizeChanged(const Platform::IntSize&)
{
    setTileMatrixNeedsUpdate();
    updateTileMatrixIfNeeded();
}

void BackingStorePrivate::scrollChanged(const Platform::IntPoint&)
{
    // FIXME: Need to do anything here?
}

void BackingStorePrivate::transformChanged()
{
    if (!m_webPage->isVisible())
        return;

    m_renderQueue->reset();

    if (!isActive()) {
        m_webPage->d->setShouldResetTilesWhenShown(true);
        return;
    }
    resetTiles();
}

void BackingStorePrivate::orientationChanged()
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());
    setTileMatrixNeedsUpdate();
    updateTileMatrixIfNeeded();
}

void BackingStorePrivate::actualVisibleSizeChanged(const Platform::IntSize& size)
{
}

void BackingStorePrivate::createSurfaces()
{
    BackingStoreGeometry* geometry = frontState();
    TileMap initialMap = geometry->tileMap();

    ASSERT(initialMap.isEmpty());

    if (m_webPage->isVisible()) {
        // This method is only to be called as part of setting up a new web page instance and
        // before said instance is made visible so as to ensure a consistent definition of web
        // page visibility. That is, a web page is said to be visible when explicitly made visible.
        ASSERT_NOT_REACHED();
        return;
    }

    // Don't try to blit to screen unless we have a buffer.
    if (!buffer())
        suspendScreenUpdates();

    SurfacePool* surfacePool = SurfacePool::globalSurfacePool();
    surfacePool->initialize(tileSize());

    if (surfacePool->isEmpty()) // Settings specify 0 tiles / no backing store.
        return;

    const Divisor divisor = bestDivisor(expandedContentsSize(), tileWidth(), tileHeight(), minimumNumberOfTilesWide(), minimumNumberOfTilesHigh(), m_preferredTileMatrixDimension);

    int numberOfTilesWide = divisor.first;
    int numberOfTilesHigh = divisor.second;

    TileMap newTileMap;
    for (int y = 0; y < numberOfTilesHigh; ++y) {
        for (int x = 0; x < numberOfTilesWide; ++x) {
            TileIndex index(x, y);
            newTileMap.add(index, 0); // no buffers initially assigned.
        }
    }

    // Set the initial state of the backingstore geometry.
    BackingStoreGeometry* newGeometry = new BackingStoreGeometry;
    newGeometry->setScale(m_webPage->d->currentScale());
    newGeometry->setNumberOfTilesWide(divisor.first);
    newGeometry->setNumberOfTilesHigh(divisor.second);
    newGeometry->setTileMap(newTileMap);
    adoptAsFrontState(newGeometry); // swap into UI thread
}

Platform::IntPoint BackingStoreGeometry::originOfTile(const TileIndex& index) const
{
    return Platform::IntPoint(backingStoreRect().x() + (index.i() * BackingStorePrivate::tileWidth()),
                              backingStoreRect().y() + (index.j() * BackingStorePrivate::tileHeight()));
}

int BackingStorePrivate::minimumNumberOfTilesWide() const
{
    // The minimum number of tiles wide required to fill the viewport + 1 tile extra to allow scrolling.
    return static_cast<int>(ceilf(m_client->transformedViewportSize().width() / static_cast<float>(tileWidth()))) + 1;
}

int BackingStorePrivate::minimumNumberOfTilesHigh() const
{
    // The minimum number of tiles high required to fill the viewport + 1 tile extra to allow scrolling.
    return static_cast<int>(ceilf(m_client->transformedViewportSize().height() / static_cast<float>(tileHeight()))) + 1;
}

Platform::IntSize BackingStorePrivate::expandedContentsSize() const
{
    const Platform::ViewportAccessor* viewportAccessor = m_webPage->webkitThreadViewportAccessor();
    return m_client->transformedViewportSize().expandedTo(viewportAccessor->pixelContentsSize());
}

int BackingStorePrivate::tileWidth()
{
    return tileSize().width();
}

int BackingStorePrivate::tileHeight()
{
    return tileSize().height();
}

Platform::IntSize BackingStorePrivate::tileSize()
{
    static Platform::IntSize tileSize = Platform::Settings::instance()->tileSize();
    return tileSize;
}

bool BackingStorePrivate::renderContents(BlackBerry::Platform::Graphics::Buffer* targetBuffer, const Platform::IntRect& dstRect, double scale, const Platform::FloatPoint& documentRenderOrigin, LayersToRender layersToRender) const
{
#if DEBUG_BACKINGSTORE
    Platform::logAlways(Platform::LogLevelCritical,
        "BackingStorePrivate::renderContents targetBuffer=0x%p dstRect=%s scale=%f documentRenderOrigin=%s",
        targetBuffer, dstRect.toString().c_str(), scale, documentRenderOrigin.toString().c_str());
#endif

    // It is up to callers of this method to perform layout themselves!
    ASSERT(!m_webPage->d->mainFrame()->view()->needsLayout());
    ASSERT(targetBuffer);

    Platform::ViewportAccessor* viewportAccessor = m_webPage->webkitThreadViewportAccessor();
    WebCore::FloatRect renderedFloatRect(documentRenderOrigin, viewportAccessor->toDocumentContents(dstRect.size(), scale));
    WebCore::IntRect contentsRect(WebCore::IntPoint::zero(), m_client->contentsSize());
    Color backgroundColor(m_webPage->settings()->backgroundColor());

    if (contentsRect.isEmpty()
        || backgroundColor.hasAlpha()
        || !WebCore::FloatRect(contentsRect).contains(renderedFloatRect)) {
        // Clear the area if it's not fully covered by (opaque) contents.
        BlackBerry::Platform::Graphics::clearBuffer(targetBuffer, dstRect,
            backgroundColor.red(), backgroundColor.green(),
            backgroundColor.blue(), backgroundColor.alpha());
    }

    if (contentsRect.isEmpty())
        return true;

    Platform::Graphics::Drawable* bufferDrawable = Platform::Graphics::lockBufferDrawable(targetBuffer);
    Platform::Graphics::Buffer* drawingBuffer = 0;

    if (bufferDrawable)
        drawingBuffer = targetBuffer;
    else {
        BBLOG(Platform::LogLevelWarn, "Using temporary buffer to paint contents, look into avoiding this.");

        drawingBuffer = Platform::Graphics::createBuffer(dstRect.size(), Platform::Graphics::BackedWhenNecessary);
        if (!drawingBuffer) {
            Platform::logAlways(Platform::LogLevelWarn, "Could not create temporary buffer, expect bad things to happen.");
            return false;
        }
        bufferDrawable = Platform::Graphics::lockBufferDrawable(drawingBuffer);
        if (!bufferDrawable) {
            Platform::logAlways(Platform::LogLevelWarn, "Could not lock temporary buffer drawable, expect bad things to happen.");
            Platform::Graphics::destroyBuffer(drawingBuffer);
            return false;
        }
    }

    PlatformGraphicsContext* platformGraphicsContext = SurfacePool::globalSurfacePool()->createPlatformGraphicsContext(bufferDrawable);
    ASSERT(platformGraphicsContext);

    {
        GraphicsContext graphicsContext(platformGraphicsContext);

        // Clip the output to the destination pixels.
        graphicsContext.save();
        graphicsContext.clip(dstRect);

        // Translate context according to offset.
        if (targetBuffer == drawingBuffer)
            graphicsContext.translate(-dstRect.x(), -dstRect.y());

        // Add our transformation matrix as the global transform.
        graphicsContext.scale(WebCore::FloatSize(scale, scale));
        graphicsContext.translate(-documentRenderOrigin.x(), -documentRenderOrigin.y());

        // Make sure the rectangle for the rendered rectangle is within the
        // bounds of the page.
        WebCore::IntRect renderedRect = enclosingIntRect(renderedFloatRect);
        renderedRect.intersect(contentsRect);

        // Take care of possible left overflow on RTL page.
        if (int leftOverFlow = m_client->frame()->view()->minimumScrollPosition().x()) {
            renderedRect.move(leftOverFlow, 0);
            graphicsContext.translate(-leftOverFlow, 0);
        }

        // Let WebCore render the page contents into the drawing surface.
        if (layersToRender == RenderRootLayer)
            m_client->frame()->view()->paintContents(&graphicsContext, renderedRect);
        else
            m_client->frame()->view()->paintContentsForSnapshot(&graphicsContext, renderedRect, FrameView::ExcludeSelection, FrameView::DocumentCoordinates);

        graphicsContext.restore();
    }

    SurfacePool::globalSurfacePool()->destroyPlatformGraphicsContext(platformGraphicsContext);
    Platform::Graphics::releaseBufferDrawable(drawingBuffer);

    if (targetBuffer != drawingBuffer) {
        // If we couldn't directly draw to the buffer, copy from the drawing surface.
        const Platform::IntRect srcRect(Platform::IntPoint::zero(), dstRect.size());

        Platform::Graphics::blitToBuffer(targetBuffer, dstRect, drawingBuffer, srcRect);
        Platform::Graphics::destroyBuffer(drawingBuffer);
    }

    return true;
}

#if DEBUG_FAT_FINGERS
static void drawDebugRect(BlackBerry::Platform::Graphics::Buffer* dstBuffer, const Platform::IntRect& dstRect, const Platform::IntRect& srcRect, unsigned char red, unsigned char green, unsigned char blue)
{
    Platform::IntRect drawRect(srcRect);
    drawRect.intersect(dstRect);
    if (!drawRect.isEmpty())
        BlackBerry::Platform::Graphics::clearBuffer(dstBuffer, drawRect, red, green, blue, 128);
}
#endif

void BackingStorePrivate::blitToWindow(const Platform::IntRect& dstRect,
    const Platform::Graphics::Buffer* srcBuffer,
    const Platform::IntRect& srcRect,
    Platform::Graphics::BlendMode blendMode,
    unsigned char globalAlpha)
{
    ASSERT(BlackBerry::Platform::userInterfaceThreadMessageClient()->isCurrentThread());

    BlackBerry::Platform::Graphics::Buffer* dstBuffer = buffer();
    ASSERT(dstBuffer);
    ASSERT(srcBuffer);
    if (!dstBuffer)
        Platform::logAlways(Platform::LogLevelWarn, "Empty window buffer, couldn't blitToWindow");

    BlackBerry::Platform::Graphics::blitToBuffer(dstBuffer, dstRect, srcBuffer, srcRect, blendMode, globalAlpha);

#if DEBUG_FAT_FINGERS
    drawDebugRect(dstBuffer, dstRect, FatFingers::m_debugFatFingerRect, 210, 210, 250);
    drawDebugRect(dstBuffer, dstRect, Platform::IntRect(FatFingers::m_debugFatFingerClickPosition, Platform::IntSize(3, 3)), 0, 0, 0);
    drawDebugRect(dstBuffer, dstRect, Platform::IntRect(FatFingers::m_debugFatFingerAdjustedPosition, Platform::IntSize(5, 5)), 100, 100, 100);
#endif

}

void BackingStorePrivate::fillWindow(Platform::Graphics::FillPattern pattern,
                                     const Platform::IntRect& dstRect,
                                     const Platform::IntPoint& contentsOrigin,
                                     double contentsScale)
{
    ASSERT(BlackBerry::Platform::userInterfaceThreadMessageClient()->isCurrentThread());

    BlackBerry::Platform::Graphics::Buffer* dstBuffer = buffer();
    ASSERT(dstBuffer);
    if (!dstBuffer)
        Platform::logAlways(Platform::LogLevelWarn, "Empty window buffer, couldn't fillWindow");

    if (pattern == BlackBerry::Platform::Graphics::CheckerboardPattern && BlackBerry::Platform::Settings::isPublicBuild()) {
        // For public builds, convey the impression of less checkerboard.
        // For developer builds, keep the checkerboard to get it fixed better.
        BlackBerry::Platform::Graphics::clearBuffer(dstBuffer, dstRect,
            m_webPageBackgroundColor.red(), m_webPageBackgroundColor.green(),
            m_webPageBackgroundColor.blue(), m_webPageBackgroundColor.alpha());
        return;
    }

    BlackBerry::Platform::Graphics::fillBuffer(dstBuffer, pattern, dstRect, contentsOrigin, contentsScale);
}

WebCore::Color BackingStorePrivate::webPageBackgroundColorUserInterfaceThread() const
{
    ASSERT(BlackBerry::Platform::userInterfaceThreadMessageClient()->isCurrentThread());
    return m_webPageBackgroundColor;
}

void BackingStorePrivate::setWebPageBackgroundColor(const WebCore::Color& color)
{
    if (!BlackBerry::Platform::userInterfaceThreadMessageClient()->isCurrentThread()) {
        typedef void (BlackBerry::WebKit::BackingStorePrivate::*FunctionType)(const WebCore::Color&);

        BlackBerry::Platform::userInterfaceThreadMessageClient()->dispatchMessage(
            BlackBerry::Platform::createMethodCallMessage<FunctionType, BackingStorePrivate, WebCore::Color>(
                &BackingStorePrivate::setWebPageBackgroundColor, this, color));
        return;
    }

    m_webPageBackgroundColor = color;
}

void BackingStorePrivate::clearWindow(const Platform::IntRect& rect,
                                      unsigned char red,
                                      unsigned char green,
                                      unsigned char blue,
                                      unsigned char alpha)
{
    if (!BlackBerry::Platform::userInterfaceThreadMessageClient()->isCurrentThread()) {
        typedef void (BlackBerry::WebKit::BackingStorePrivate::*FunctionType)(const Platform::IntRect&,
                                                                           unsigned char,
                                                                           unsigned char,
                                                                           unsigned char,
                                                                           unsigned char);
        BlackBerry::Platform::userInterfaceThreadMessageClient()->dispatchMessage(
            BlackBerry::Platform::createMethodCallMessage<FunctionType,
                                                       BackingStorePrivate,
                                                       Platform::IntRect,
                                                       unsigned char,
                                                       unsigned char,
                                                       unsigned char,
                                                       unsigned char>(
                &BackingStorePrivate::clearWindow, this, rect, red, green, blue, alpha));
        return;
    }

    BlackBerry::Platform::Graphics::Buffer* dstBuffer = buffer();
    ASSERT(dstBuffer);
    if (!dstBuffer)
        Platform::logAlways(Platform::LogLevelWarn, "Empty window buffer, couldn't clearWindow");

    BlackBerry::Platform::Graphics::clearBuffer(dstBuffer, rect, red, green, blue, alpha);
}

bool BackingStorePrivate::isScrollingOrZooming() const
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());
    return m_isScrollingOrZooming;
}

void BackingStorePrivate::setScrollingOrZooming(bool scrollingOrZooming, bool shouldBlit)
{
    ASSERT(BlackBerry::Platform::webKitThreadMessageClient()->isCurrentThread());

    if (m_isScrollingOrZooming == scrollingOrZooming)
        return;

    m_isScrollingOrZooming = scrollingOrZooming;

#if !ENABLE_REPAINTONSCROLL
    m_suspendRenderJobs = scrollingOrZooming; // Suspend the rendering of everything.
#endif

    if (!m_webPage->settings()->shouldRenderAnimationsOnScrollOrZoom())
        m_suspendRegularRenderJobs = scrollingOrZooming; // Suspend the rendering of animations.

    m_webPage->d->m_mainFrame->view()->setConstrainsScrollingToContentEdge(!scrollingOrZooming);

    // Clear this flag since we don't care if the render queue is under pressure
    // or not since we are scrolling and it is more important to not lag than
    // it is to ensure animations achieve better framerates!
    if (scrollingOrZooming)
        m_renderQueue->setCurrentRegularRenderJobBatchUnderPressure(false);
#if ENABLE_SCROLLBARS
    else if (shouldBlit)
        blitVisibleContents();
#endif
    if (!scrollingOrZooming && shouldPerformRegularRenderJobs())
        dispatchRenderJob();
}

BackingStoreGeometry* BackingStorePrivate::frontState() const
{
    return reinterpret_cast<BackingStoreGeometry*>(m_frontState);
}

void BackingStorePrivate::adoptAsFrontState(BackingStoreGeometry* newFrontState)
{
    bool hasValidBuffers = false;

    // Remember the buffers we'll use in the new front state for comparison.
    WTF::Vector<TileBuffer*> newTileBuffers;
    TileMap newTileMap = newFrontState->tileMap();
    TileMap::const_iterator end = newTileMap.end();
    for (TileMap::const_iterator it = newTileMap.begin(); it != end; ++it) {
        if (it->value) {
            hasValidBuffers = true;
            newTileBuffers.append(it->value);
        }
    }

    if (!hasValidBuffers) {
        m_tileMatrixContainsUsefulContent = false;
        updateSuspendScreenUpdateState();
    }

    unsigned newFront = reinterpret_cast<unsigned>(newFrontState);
    BackingStoreGeometry* oldFrontState = frontState();

    // Atomic change.
    _smp_xchg(&m_frontState, newFront);

    bool hasSynced = false;

    if (hasValidBuffers) {
        m_tileMatrixContainsUsefulContent = true;
        updateSuspendScreenUpdateState(&hasSynced);
    }

    if (!hasSynced) {
        // Wait until the user interface thread won't access the old front state anymore.
        BlackBerry::Platform::userInterfaceThreadMessageClient()->syncToCurrentMessage();
    }

    // Reclaim unused old tile buffers as back buffers.
    TileMap oldTileMap = oldFrontState->tileMap();
    end = oldTileMap.end();
    for (TileMap::const_iterator it = oldTileMap.begin(); it != end; ++it) {
        TileBuffer* tileBuffer = it->value;
        if (tileBuffer && !newTileBuffers.contains(tileBuffer))
            SurfacePool::globalSurfacePool()->addBackBuffer(tileBuffer);
    }

    delete oldFrontState;
}

// static
void BackingStorePrivate::setCurrentBackingStoreOwner(WebPage* webPage)
{
    // Let the previously active backingstore release its tile buffers so
    // the new one (e.g. another tab) can use the buffers to render contents.
    if (BackingStorePrivate::s_currentBackingStoreOwner && BackingStorePrivate::s_currentBackingStoreOwner != webPage)
        BackingStorePrivate::s_currentBackingStoreOwner->d->m_backingStore->d->resetTiles();

    BackingStorePrivate::s_currentBackingStoreOwner = webPage;
    if (webPage)
        webPage->backingStore()->d->updateSuspendScreenUpdateState(); // depends on isActive()
}

bool BackingStorePrivate::isActive() const
{
    return BackingStorePrivate::s_currentBackingStoreOwner == m_webPage && SurfacePool::globalSurfacePool()->isActive();
}

void BackingStorePrivate::didRenderContent(const Platform::IntRectRegion& renderedRegion)
{
#if USE(ACCELERATED_COMPOSITING)
    if (m_webPage->d->needsOneShotDrawingSynchronization())
        m_webPage->d->commitRootLayerIfNeeded();
    else
#endif
    {
        if (isScrollingOrZooming())
            return; // don't drag down framerates by double-blitting.

        blitVisibleContents();
    }

    // Don't issue content rendered calls when all we rendered was filler
    // background color before the page is committed.
    if (!m_client->contentsSize().isEmpty()) {
        std::vector<Platform::IntRect> renderedRects = renderedRegion.rects();
        for (size_t i = 0; i < renderedRects.size(); ++i)
            m_webPage->client()->notifyPixelContentRendered(renderedRects[i]);
    }
}

BackingStore::BackingStore(WebPage* webPage, BackingStoreClient* client)
    : d(new BackingStorePrivate)
{
    d->m_webPage = webPage;
    d->m_client = client;
}

BackingStore::~BackingStore()
{
    deleteGuardedObject(d);
    d = 0;
}

void BackingStore::createSurface()
{
    static bool initialized = false;
    if (!initialized) {
        BlackBerry::Platform::Graphics::initialize();
        initialized = true;
    }

    // Triggers creation of surfaces in backingstore.
    d->createSurfaces();

    // Focusing the WebPage triggers a repaint, so while we want it to be
    // focused initially this has to happen after creation of the surface.
    d->m_webPage->setFocused(true);
}

void BackingStore::suspendBackingStoreUpdates()
{
    d->suspendBackingStoreUpdates();
}

void BackingStore::resumeBackingStoreUpdates()
{
    d->resumeBackingStoreUpdates();
}

void BackingStore::suspendGeometryUpdates()
{
    d->suspendGeometryUpdates();
}

void BackingStore::resumeGeometryUpdates()
{
    d->resumeGeometryUpdates();
}

void BackingStore::suspendScreenUpdates()
{
    d->suspendScreenUpdates();
}

void BackingStore::resumeScreenUpdates(ResumeUpdateOperation op)
{
    d->resumeScreenUpdates(op);
}

bool BackingStore::isScrollingOrZooming() const
{
    return d->isScrollingOrZooming();
}

void BackingStore::setScrollingOrZooming(bool scrollingOrZooming)
{
    d->setScrollingOrZooming(scrollingOrZooming);
}

void BackingStore::blitVisibleContents()
{
    d->blitVisibleContents(false /*force*/);
}

void BackingStore::repaint(int x, int y, int width, int height,
                           bool contentChanged, bool immediate)
{
    d->repaint(Platform::IntRect(x, y, width, height), contentChanged, immediate);
}

void BackingStore::acquireBackingStoreMemory()
{
    SurfacePool::globalSurfacePool()->createBuffers();
}

void BackingStore::releaseOwnedBackingStoreMemory()
{
    if (BackingStorePrivate::s_currentBackingStoreOwner == d->m_webPage) {
        // Call resetTiles() (hopefully) after suspendScreenUpdates()
        // so we will not cause checkerboard to be shown before suspending.
        // This causes the tiles in use to be given back to the SurfacePool.
        d->resetTiles();
        SurfacePool::globalSurfacePool()->releaseBuffers();
    }
}

bool BackingStore::hasBlitJobs() const
{
#if USE(ACCELERATED_COMPOSITING)
    // If there's a WebPageCompositorClient, let it schedule the blit.
    WebPageCompositorPrivate* compositor = d->m_webPage->d->compositor();
    if (compositor && compositor->client())
        return false;
#endif

    // Normally, this would be called from the compositing thread,
    // and the flag is set on the compositing thread, so no need for
    // synchronization.
    return d->m_hasBlitJobs;
}

void BackingStore::blitOnIdle()
{
#if USE(ACCELERATED_COMPOSITING)
    // If there's a WebPageCompositorClient, let it schedule the blit.
    WebPageCompositorPrivate* compositor = d->m_webPage->d->compositor();
    if (compositor && compositor->client())
        return;
#endif

    d->blitVisibleContents(true /*force*/);
}

Platform::IntSize BackingStorePrivate::surfaceSize() const
{
    if (Window* window = m_webPage->client()->window())
        return window->surfaceSize();

#if USE(ACCELERATED_COMPOSITING)
    if (WebPageCompositorPrivate* compositor = m_webPage->d->compositor())
        return compositor->context()->surfaceSize();
#endif

    return Platform::IntSize();
}

Platform::Graphics::Buffer* BackingStorePrivate::buffer() const
{
    if (Window* window = m_webPage->client()->window())
        return window->buffer();

#if USE(ACCELERATED_COMPOSITING)
    if (WebPageCompositorPrivate* compositor = m_webPage->d->compositor())
        return compositor->context() ? compositor->context()->buffer() : 0;
#endif

    return 0;
}

bool BackingStore::drawContents(Platform::Graphics::Buffer* buffer, const Platform::IntRect& dstRect, double scale, const Platform::FloatPoint& documentScrollPosition)
{
    if (!buffer || dstRect.isEmpty())
        return false;

    d->requestLayoutIfNeeded();

    return d->renderContents(buffer, dstRect, scale, documentScrollPosition, BackingStorePrivate::RenderAllLayers);
}

}
}
