/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2012 INdT - Instituto Nokia de Tecnologia
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "WebView.h"

#include "Cursor.h"
#include "DrawingAreaProxyImpl.h"
#include "LayerTreeCoordinatorProxy.h"
#include "LayerTreeRenderer.h"
#include "NativeWebKeyboardEvent.h"
#include "NativeWebMouseEvent.h"
#include "NativeWebWheelEvent.h"
#include "NativeWebGestureEvent.h"
#include "NotImplemented.h"
#include "PageClient.h"
#include "WebContext.h"
#include "WebContextMenuProxy.h"
#include "WebEvent.h"
#include "WebPageGroup.h"
#include "WebPopupMenuProxy.h"
#include "WebPreferences.h"
#include "WindowsKeyboardCodes.h"
#include <WebCore/Scrollbar.h>
#include <wtf/text/WTFString.h>

#include <WebKit2/NixEvents.h>
#include <WebKit2/WebView.h>

using namespace WebCore;
using namespace WebKit;

namespace Nix {

cairo_matrix_t WebViewClient::viewToScreenTransform()
{
    static cairo_matrix_t identityTransform = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
    return identityTransform;
}

void WebViewClient::doneWithTouchEvent(const TouchEvent&, bool)
{

}

void WebViewClient::doneWithGestureEvent(const GestureEvent&, bool)
{

}

class WebViewImpl : public WebView, public PageClient {
public:
    WebViewImpl(WebContext* context, WebPageGroup* pageGroup, WebViewClient* client)
        : m_client(client)
        , m_webPageProxy(context->createWebPage(this, pageGroup))
        , m_focused(true)
        , m_visible(true)
        , m_active(true)
        , m_scale(1.0)
        , m_opacity(1.0)
    {
        m_webPageProxy->pageGroup()->preferences()->setForceCompositingMode(true);
    }

    virtual ~WebViewImpl() { }

    // WebView.
    virtual void initialize();

    virtual int width() const;
    virtual int height() const;
    virtual void setSize(int width, int height);

    virtual int scrollX() const { return m_scrollPosition.x(); }
    virtual int scrollY() const { return m_scrollPosition.y(); }
    virtual void setScrollPosition(int x, int y);

    virtual bool isFocused() const;
    virtual void setFocused(bool);

    virtual bool isVisible() const;
    virtual void setVisible(bool);

    virtual bool isActive() const;
    virtual void setActive(bool);

    virtual void setTransparentBackground(bool);
    virtual bool transparentBackground() const;

    virtual void setDrawBackground(bool);
    virtual bool drawBackground() const;

    virtual void setScale(double);
    virtual double scale() const { return m_scale; }

    virtual int visibleContentWidth() const { return m_size.width() / m_scale; }
    virtual int visibleContentHeight() const { return m_size.height() / m_scale; }


    virtual void setOpacity(double opacity) { m_opacity = opacity < 0.0 ? 0.0 : opacity > 1.0 ? 1.0 : opacity; }
    virtual double opacity() const { return m_opacity; }

    virtual void paintToCurrentGLContext();

    virtual WKPageRef pageRef();

    virtual void sendEvent(const Nix::InputEvent&);

    // PageClient.
    virtual PassOwnPtr<DrawingAreaProxy> createDrawingAreaProxy();
    virtual void setViewNeedsDisplay(const IntRect&);

    virtual bool isViewFocused() { return m_focused; }
    virtual bool isViewVisible() { return m_visible; }
    virtual bool isViewWindowActive() { return m_active; }
    virtual bool isViewInWindow() { return true; } // FIXME
    virtual IntSize viewSize() { return m_size; }
    virtual IntSize contentsSize() { return m_contentsSize; }
    virtual void processDidCrash();
    virtual void didRelaunchProcess() { m_client->webProcessRelaunched(); }

    virtual void pageDidRequestScroll(const IntPoint& point);

    // PageClient not implemented.
    virtual void displayView() { notImplemented(); }
    virtual void scrollView(const IntRect& scrollRect, const IntSize& scrollOffset) { notImplemented(); }

    virtual void pageClosed() { notImplemented(); }

    virtual void toolTipChanged(const String&, const String&) { notImplemented(); }

    virtual void handleDownloadRequest(DownloadProxy*) { notImplemented(); }

    virtual void didChangeContentsSize(const IntSize& size);

    virtual void setCursor(const Cursor&) { notImplemented(); }
    virtual void setCursorHiddenUntilMouseMoves(bool) { notImplemented(); }
    virtual void didChangeViewportProperties(const ViewportAttributes&);

    virtual void registerEditCommand(PassRefPtr<WebEditCommandProxy>, WebPageProxy::UndoOrRedo) { notImplemented(); }
    virtual void clearAllEditCommands() { notImplemented(); }
    virtual bool canUndoRedo(WebPageProxy::UndoOrRedo) { notImplemented(); return false; }
    virtual void executeUndoRedo(WebPageProxy::UndoOrRedo) { notImplemented(); }

    virtual FloatRect convertToDeviceSpace(const FloatRect& rect) { notImplemented(); return rect; }
    virtual FloatRect convertToUserSpace(const FloatRect& rect) { notImplemented(); return rect; }
    virtual IntPoint screenToWindow(const IntPoint& point) { notImplemented(); return point; }
    virtual IntRect windowToScreen(const IntRect& rect) { notImplemented(); return rect; }

    virtual void doneWithKeyEvent(const NativeWebKeyboardEvent&, bool wasEventHandled) { notImplemented(); }
#if ENABLE(GESTURE_EVENTS)
    virtual void doneWithGestureEvent(const NativeWebGestureEvent&, bool wasEventHandled);
#endif
#if ENABLE(TOUCH_EVENTS)
    virtual void doneWithTouchEvent(const NativeWebTouchEvent&, bool wasEventHandled);
#endif

    virtual PassRefPtr<WebPopupMenuProxy> createPopupMenuProxy(WebPageProxy*) { notImplemented(); return PassRefPtr<WebPopupMenuProxy>(); }
    virtual PassRefPtr<WebContextMenuProxy> createContextMenuProxy(WebPageProxy*) { notImplemented(); return PassRefPtr<WebContextMenuProxy>(); }

#if ENABLE(INPUT_TYPE_COLOR)
    virtual PassRefPtr<WebColorChooserProxy> createColorChooserProxy(WebPageProxy*, const Color& initialColor) { notImplemented(); return PassRefPtr<WebColorChooserProxy>(); }
#endif

    virtual void setFindIndicator(PassRefPtr<FindIndicator>, bool fadeOut, bool animate)  { notImplemented(); }

#if USE(ACCELERATED_COMPOSITING)
    virtual void enterAcceleratedCompositingMode(const LayerTreeContext&) { notImplemented(); }
    virtual void exitAcceleratedCompositingMode() { notImplemented(); }
    virtual void updateAcceleratedCompositingMode(const LayerTreeContext&) { notImplemented(); }
#endif

    virtual void didChangeScrollbarsForMainFrame() const { notImplemented(); }

    virtual void didCommitLoadForMainFrame(bool useCustomRepresentation) { notImplemented(); }
    virtual void didFinishLoadingDataForCustomRepresentation(const String& suggestedFilename, const CoreIPC::DataReference&) { notImplemented(); }
    virtual double customRepresentationZoomFactor() { notImplemented(); return 1.0; }
    virtual void setCustomRepresentationZoomFactor(double) { notImplemented(); }

    virtual void flashBackingStoreUpdates(const Vector<IntRect>& updateRects) { notImplemented(); }
    virtual void findStringInCustomRepresentation(const String&, WebKit::FindOptions, unsigned maxMatchCount) { notImplemented(); }
    virtual void countStringMatchesInCustomRepresentation(const String&, WebKit::FindOptions, unsigned maxMatchCount) { notImplemented(); }

private:
    LayerTreeRenderer* layerTreeRenderer();
    void updateVisibleContents();
    void adjustScrollPosition();

    cairo_matrix_t screenToViewMatrix();
    void transformPointToViewCoordinates(double& x, double& y);

    void sendMouseEvent(const Nix::MouseEvent&);
    void sendWheelEvent(const Nix::WheelEvent&);
    void sendKeyEvent(const Nix::KeyEvent&);
    void sendTouchEvent(const Nix::TouchEvent& event);
    void sendGestureEvent(const Nix::GestureEvent& event);

    WebViewClient* m_client;
    WTF::RefPtr<WebPageProxy> m_webPageProxy;
    bool m_focused;
    bool m_visible;
    bool m_active;
    IntSize m_size;
    IntSize m_contentsSize;
    IntPoint m_lastCursorPosition;
    IntPoint m_scrollPosition;
    double m_scale;
    double m_opacity;
};

WebView* WebView::create(WKContextRef contextRef, WKPageGroupRef pageGroupRef, WebViewClient* client)
{
    g_type_init();
    return new WebViewImpl(toImpl(contextRef), toImpl(pageGroupRef), client);
}

void WebViewImpl::initialize()
{
    m_webPageProxy->initializeWebPage();
    layerTreeRenderer()->setActive(true);
}

void WebViewImpl::setTransparentBackground(bool value)
{
    m_webPageProxy->setDrawsTransparentBackground(value);
}

bool WebViewImpl::transparentBackground() const
{
    return m_webPageProxy->drawsTransparentBackground();
}

void WebViewImpl::setDrawBackground(bool value)
{
    m_webPageProxy->setDrawsBackground(value);
}

bool WebViewImpl::drawBackground() const
{
    return m_webPageProxy->drawsBackground();
}

void WebViewImpl::setScale(double scale)
{
    m_scale = scale;
    setSize(m_size.width(), m_size.height());
}

int WebViewImpl::width() const
{
    return m_size.width();
}

int WebViewImpl::height() const
{
    return m_size.height();
}

void WebViewImpl::setSize(int width, int height)
{
    m_size = IntSize(width, height);

    IntSize visibleSize = IntSize(visibleContentWidth(), visibleContentHeight());

    if (m_webPageProxy->useFixedLayout())
        m_webPageProxy->setViewportSize(visibleSize);

    DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();
    if (!drawingArea)
        return;

    drawingArea->setSize(visibleSize, IntSize());
    drawingArea->setVisibleContentsRect(IntRect(m_scrollPosition, visibleSize), m_scale, FloatPoint());
}

void WebViewImpl::setScrollPosition(int x, int y)
{
    if (m_scrollPosition.x() == x && m_scrollPosition.y() == y)
        return;
    m_scrollPosition = IntPoint(x, y);

    adjustScrollPosition();

    DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();
    if (!drawingArea)
        return;

    drawingArea->setVisibleContentsRect(IntRect(m_scrollPosition, IntSize(visibleContentWidth(), visibleContentHeight())), m_scale, FloatPoint());
}

void WebViewImpl::adjustScrollPosition()
{
    int x = m_scrollPosition.x();
    int y = m_scrollPosition.y();

    int rightBoundary = m_contentsSize.width() - visibleContentWidth();
    int bottomBoundary = m_contentsSize.height() - visibleContentHeight();

    if (x < 0)
        x = 0;
    else if (x > rightBoundary)
        x = rightBoundary;
    if (y < 0)
        y = 0;
    else if (y > bottomBoundary)
        y = bottomBoundary;

    if (m_scrollPosition.x() == x && m_scrollPosition.y() == y)
        return;

    m_scrollPosition = IntPoint(x, y);
}

bool WebViewImpl::isFocused() const
{
    return m_focused;
}

void WebViewImpl::setFocused(bool focused)
{
    m_focused = focused;
    m_webPageProxy->viewStateDidChange(WebPageProxy::ViewIsFocused);
}

bool WebViewImpl::isVisible() const
{
    return m_visible;
}

void WebViewImpl::setVisible(bool visible)
{
    m_visible = visible;
    m_webPageProxy->viewStateDidChange(WebPageProxy::ViewIsVisible);
}

bool WebViewImpl::isActive() const
{
    return m_active;
}

void WebViewImpl::setActive(bool active)
{
    m_active = active;
    m_webPageProxy->viewStateDidChange(WebPageProxy::ViewWindowIsActive);
}

PassOwnPtr<DrawingAreaProxy> WebViewImpl::createDrawingAreaProxy()
{
    return DrawingAreaProxyImpl::create(m_webPageProxy.get());
}

void WebViewImpl::setViewNeedsDisplay(const IntRect& rect)
{
    m_client->viewNeedsDisplay(rect.x(), rect.y(), rect.width(), rect.height());
}

WKPageRef WebViewImpl::pageRef()
{
    return toAPI(m_webPageProxy.get());
}

void WebViewImpl::sendEvent(const Nix::InputEvent& event)
{
    using namespace Nix;

    switch (event.type) {
        case InputEvent::MouseDown:
        case InputEvent::MouseUp:
        case InputEvent::MouseMove:
            sendMouseEvent(static_cast<const Nix::MouseEvent&>(event));
            break;
        case InputEvent::Wheel:
            sendWheelEvent(static_cast<const Nix::WheelEvent&>(event));
            break;
        case InputEvent::KeyDown:
        case InputEvent::KeyUp:
            sendKeyEvent(static_cast<const Nix::KeyEvent&>(event));
            break;
        case InputEvent::TouchStart:
        case InputEvent::TouchMove:
        case InputEvent::TouchEnd:
        case InputEvent::TouchCancel:
            sendTouchEvent(static_cast<const Nix::TouchEvent&>(event));
            break;
        case InputEvent::GestureSingleTap:
            sendGestureEvent(static_cast<const Nix::GestureEvent&>(event));
            break;
        default:
            notImplemented();
    }
}

void WebViewImpl::sendMouseEvent(const Nix::MouseEvent& event)
{
    m_webPageProxy->handleMouseEvent(NativeWebMouseEvent(event, screenToViewMatrix(), &m_lastCursorPosition));
}

void WebViewImpl::sendTouchEvent(const Nix::TouchEvent& event)
{
    m_webPageProxy->handleTouchEvent(NativeWebTouchEvent(event, screenToViewMatrix()));
}

// TODO: Create a constructor in TransformationMatrix that takes a cairo_matrix_t.
static TransformationMatrix toTransformationMatrix(const cairo_matrix_t& matrix)
{
    return TransformationMatrix(matrix.xx, matrix.yx, matrix.xy, matrix.yy, matrix.x0, matrix.y0);
}

LayerTreeRenderer* WebViewImpl::layerTreeRenderer()
{
    DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();
    if (!drawingArea)
        return 0;

    LayerTreeCoordinatorProxy* coordinatorProxy = drawingArea->layerTreeCoordinatorProxy();
    if (!coordinatorProxy)
        return 0;

    LayerTreeRenderer* renderer = coordinatorProxy->layerTreeRenderer();
    if (!renderer)
        return 0;

    return renderer;
}

void WebViewImpl::paintToCurrentGLContext()
{
    LayerTreeRenderer* renderer = layerTreeRenderer();
    if (!renderer)
        return;

    renderer->syncRemoteContent();

    cairo_matrix_t viewTransform = m_client->viewToScreenTransform();
    double x = 0;
    double y = 0;
    double width = m_size.width();
    double height = m_size.height();
    cairo_matrix_transform_point(&viewTransform, &x, &y);
    cairo_matrix_transform_distance(&viewTransform, &width, &height);

    cairo_matrix_t scrollTransform;
    cairo_matrix_init_translate(&scrollTransform, -m_scrollPosition.x(), -m_scrollPosition.y());

    cairo_matrix_t scaleTransform;
    cairo_matrix_init_scale(&scaleTransform, m_scale, m_scale);

    cairo_matrix_t transform;
    cairo_matrix_multiply(&transform, &scrollTransform, &scaleTransform);
    cairo_matrix_multiply(&transform, &transform, &viewTransform);

    FloatRect rect(x, y, width, height);
    renderer->paintToCurrentGLContext(toTransformationMatrix(transform), m_opacity, rect);
}

void WebViewImpl::processDidCrash()
{
    m_client->webProcessCrashed(toCopiedAPI(m_webPageProxy->urlAtProcessExit()));
}

void WebViewImpl::pageDidRequestScroll(const IntPoint& point)
{
    m_client->pageDidRequestScroll(point.x(), point.y());
    // FIXME: It's not clear to me yet whether we should ask for display here or this is
    // at the wrong level and we should simply notify the client about this.
    setViewNeedsDisplay(IntRect(IntPoint(), m_size));
}

void WebViewImpl::didChangeContentsSize(const IntSize& size)
{
    m_contentsSize = size;
    adjustScrollPosition();
}

void WebViewImpl::didChangeViewportProperties(const ViewportAttributes& attributes)
{
    if (!m_webPageProxy->useFixedLayout())
        return;

    setScale(m_size.width() / attributes.layoutSize.width());
}

cairo_matrix_t WebViewImpl::screenToViewMatrix()
{
    cairo_matrix_t invertedViewTransform = m_client->viewToScreenTransform();
    cairo_matrix_invert(&invertedViewTransform);

    cairo_matrix_t invertedScrollTransform;
    cairo_matrix_init_translate(&invertedScrollTransform, m_scrollPosition.x(), m_scrollPosition.y());

    cairo_matrix_t invertedScaleTransform;
    cairo_matrix_init_scale(&invertedScaleTransform, 1.0 / m_scale, 1.0 / m_scale);

    cairo_matrix_t transform;
    cairo_matrix_multiply(&transform, &invertedViewTransform, &invertedScaleTransform);
    cairo_matrix_multiply(&transform, &transform, &invertedScrollTransform);

    return transform;
}

void WebViewImpl::transformPointToViewCoordinates(double& x, double& y)
{
    cairo_matrix_t transformMatrix = screenToViewMatrix();
    cairo_matrix_transform_point(&transformMatrix, &x, &y);
}

void WebViewImpl::sendWheelEvent(const Nix::WheelEvent& event)
{
    m_webPageProxy->handleWheelEvent(NativeWebWheelEvent(event, screenToViewMatrix()));
}

void WebViewImpl::sendKeyEvent(const KeyEvent& event)
{
    m_webPageProxy->handleKeyboardEvent(NativeWebKeyboardEvent(event));
}

void WebViewImpl::sendGestureEvent(const GestureEvent& event)
{
    m_webPageProxy->handleGestureEvent(NativeWebGestureEvent(event, screenToViewMatrix()));
}

#if ENABLE(TOUCH_EVENTS)
void WebViewImpl::doneWithTouchEvent(const NativeWebTouchEvent& event, bool wasEventHandled)
{
    m_client->doneWithTouchEvent(event.nativeEvent(), wasEventHandled);
}
#endif

#if ENABLE(GESTURE_EVENTS)
void WebViewImpl::doneWithGestureEvent(const NativeWebGestureEvent& event, bool wasEventHandled)
{
    m_client->doneWithGestureEvent(event.nativeEvent(), wasEventHandled);
}
#endif

} // namespace Nix
