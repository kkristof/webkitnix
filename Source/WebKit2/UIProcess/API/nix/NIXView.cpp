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
#include "NIXView.h"

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
#include "APIClient.h"
#include <JavaScriptCore/WebKitAvailability.h>
#include <WebCore/Scrollbar.h>
#include <WebCore/TextureMapperGL.h>
#include <WebKit2/NixEvents.h>
#include <WebKit2/WKBundle.h>
#include <wtf/text/WTFString.h>
#include <wtf/MathExtras.h>
#include <cairo.h>

using namespace WebCore;
using namespace WebKit;

namespace Nix {

// TODO: Once versions start to matter, change this to inherit from APIClient and set
// the appropriate traits in APIClientTraits.
class WebViewClient {
public:
    WebViewClient()
    {
        initialize(0);
    }

    void initialize(const NIXViewClient* client)
    {
        if (!client) {
            memset(&m_client, 0, sizeof(m_client));
            return;
        }

        ASSERT(client->version == kNIXViewClientCurrentVersion);
        m_client = *client;
    }

    const NIXViewClient& client() const { return m_client; }

    void viewNeedsDisplay(WKRect area);
    void webProcessCrashed(WKStringRef url);
    void webProcessRelaunched() { }
    void doneWithTouchEvent(const NIXTouchEvent&, bool wasEventHandled);
    void doneWithGestureEvent(const NIXGestureEvent&, bool wasEventHandled);
    void pageDidRequestScroll(WKPoint position);
    void didChangeContentsSize(WKSize size);
    void didFindZoomableArea(WKPoint target, WKRect area);
    void updateTextInputState(bool isContentEditable, WKRect cursorRect, WKRect editorRect);
    void compositeCustomLayerToCurrentGLContext(uint32_t id, WKRect rect, const float* matrix, float opacity);

private:
    NIXViewClient m_client;
};


void WebViewClient::viewNeedsDisplay(WKRect rect)
{
    if (!m_client.viewNeedsDisplay)
        return;
    m_client.viewNeedsDisplay(rect, m_client.clientInfo);
}

void WebViewClient::webProcessCrashed(WKStringRef url)
{
    if (!m_client.webProcessCrashed)
        return;
    m_client.webProcessCrashed(url, m_client.clientInfo);
}

void WebViewClient::doneWithTouchEvent(const NIXTouchEvent& event, bool wasEventHandled)
{
    if (!m_client.doneWithTouchEvent)
        return;
    m_client.doneWithTouchEvent(&event, wasEventHandled, m_client.clientInfo);
}

void WebViewClient::doneWithGestureEvent(const NIXGestureEvent& event, bool wasEventHandled)
{
    if (!m_client.doneWithGestureEvent)
        return;
    m_client.doneWithGestureEvent(&event, wasEventHandled, m_client.clientInfo);
}

void WebViewClient::pageDidRequestScroll(WKPoint point)
{
    if (!m_client.pageDidRequestScroll)
        return;
    m_client.pageDidRequestScroll(point, m_client.clientInfo);
}
void WebViewClient::didChangeContentsSize(WKSize size)
{
    if (!m_client.didChangeContentsSize)
        return;
    m_client.didChangeContentsSize(size, m_client.clientInfo);
}

void WebViewClient::didFindZoomableArea(WKPoint target, WKRect area)
{
    if (!m_client.didFindZoomableArea)
        return;
    m_client.didFindZoomableArea(target, area, m_client.clientInfo);
}

void WebViewClient::updateTextInputState(bool isContentEditable, WKRect cursorRect, WKRect editorRect)
{
    if (!m_client.updateTextInputState)
        return;
    m_client.updateTextInputState(isContentEditable, cursorRect, editorRect, m_client.clientInfo);
}

void WebViewClient::compositeCustomLayerToCurrentGLContext(uint32_t id, WKRect rect, const float* matrix, float opacity)
{
    if (!m_client.compositeCustomLayerToCurrentGLContext)
        return;
    m_client.compositeCustomLayerToCurrentGLContext(id, rect, matrix, opacity, m_client.clientInfo);
}


class WebViewImpl : public PageClient {
public:
    WebViewImpl(WebContext* context, WebPageGroup* pageGroup, const NIXViewClient* viewClient)
        : m_webPageProxy(context->createWebPage(this, pageGroup))
        , m_focused(true)
        , m_visible(true)
        , m_active(true)
        , m_scale(1.f)
        , m_opacity(1.f)
        , m_isSuspended(false)
    {
        m_viewClient.initialize(viewClient);
        m_webPageProxy->pageGroup()->preferences()->setForceCompositingMode(true);
        cairo_matrix_t identityTransform;
        cairo_matrix_init_identity(&identityTransform);
        setUserViewportTransformation(identityTransform);
    }

    virtual ~WebViewImpl() { }

    // WebView.
    void initialize();

    WKSize size() const { return WKSizeMake(m_size.width(), m_size.height()); }
    void setSize(const WKSize& size);

    WKPoint scrollPosition() const { return WKPointMake(m_scrollPosition.x(), m_scrollPosition.y()); }
    void setScrollPosition(const WKPoint& position);

    void setUserViewportTransformation(const cairo_matrix_t& userViewportTransformation) { m_userViewportTransformation = userViewportTransformation; }
    WKPoint userViewportToContents(WKPoint point);

    bool isFocused() const;
    void setFocused(bool);

    bool isVisible() const;
    void setVisible(bool);

    bool isActive() const;
    void setActive(bool);

    void setTransparentBackground(bool);
    bool transparentBackground() const;

    void setDrawBackground(bool);
    bool drawBackground() const;

    void setScale(float);
    float scale() const { return m_scale; }

    WKSize visibleContentsSize() const { return WKSizeMake(m_size.width() / m_scale, m_size.height() / m_scale); }

    void setOpacity(float opacity) { m_opacity = clampTo(opacity, 0.f, 1.f); }
    float opacity() const { return m_opacity; }

    void paintToCurrentGLContext();

    void commitViewportChanges();

    void findZoomableAreaForPoint(const WKPoint& point, int horizontalRadius, int verticalRadius);

    WKPageRef pageRef();

    void sendEvent(const Nix::InputEvent&);

    void sendMouseEvent(const NIXMouseEvent&);
    void sendWheelEvent(const NIXWheelEvent&);
    void sendKeyEvent(const NIXKeyEvent&);
    void sendTouchEvent(const NIXTouchEvent&);
    void sendGestureEvent(const NIXGestureEvent&);

    uint32_t addCustomLayer(WKStringRef elementID);
    void removeCustomLayer(uint32_t);

    // PageClient.
    virtual PassOwnPtr<DrawingAreaProxy> createDrawingAreaProxy();
    virtual void setViewNeedsDisplay(const IntRect&);

    virtual bool isViewFocused() { return m_focused; }
    virtual bool isViewVisible() { return m_visible; }
    virtual bool isViewWindowActive() { return m_active; }
    virtual bool isViewInWindow() { return true; } // FIXME
    virtual IntSize viewSize() { return m_size; }
    virtual void processDidCrash();
    virtual void didRelaunchProcess() { m_viewClient.webProcessRelaunched(); }

    virtual void pageDidRequestScroll(const IntPoint& point);
    virtual void didChangeContentsSize(const IntSize& size);
    virtual void didFindZoomableArea(const IntPoint& target, const IntRect& area);

    virtual void pageTransitionViewportReady();

    // PageClient not implemented.
    virtual void displayView() { notImplemented(); }
    virtual void scrollView(const IntRect&, const IntSize&) { notImplemented(); }

    virtual void pageClosed() { notImplemented(); }

    virtual void toolTipChanged(const String&, const String&) { notImplemented(); }

    virtual void setCursor(const Cursor&) { notImplemented(); }
    virtual void setCursorHiddenUntilMouseMoves(bool) { notImplemented(); }
    virtual void didChangeViewportProperties(const ViewportAttributes&) { notImplemented(); }

    virtual void registerEditCommand(PassRefPtr<WebEditCommandProxy>, WebPageProxy::UndoOrRedo) { notImplemented(); }
    virtual void clearAllEditCommands() { notImplemented(); }
    virtual bool canUndoRedo(WebPageProxy::UndoOrRedo) { notImplemented(); return false; }
    virtual void executeUndoRedo(WebPageProxy::UndoOrRedo) { notImplemented(); }

    virtual FloatRect convertToDeviceSpace(const FloatRect& rect) { notImplemented(); return rect; }
    virtual FloatRect convertToUserSpace(const FloatRect& rect) { notImplemented(); return rect; }
    virtual IntPoint screenToWindow(const IntPoint& point) { notImplemented(); return point; }
    virtual IntRect windowToScreen(const IntRect& rect) { notImplemented(); return rect; }

    virtual void doneWithKeyEvent(const NativeWebKeyboardEvent&, bool) { notImplemented(); }
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

    virtual void setFindIndicator(PassRefPtr<FindIndicator>, bool, bool)  { notImplemented(); }

#if USE(ACCELERATED_COMPOSITING)
    virtual void enterAcceleratedCompositingMode(const LayerTreeContext&) { notImplemented(); }
    virtual void exitAcceleratedCompositingMode() { notImplemented(); }
    virtual void updateAcceleratedCompositingMode(const LayerTreeContext&) { notImplemented(); }
#endif

    virtual void didChangeScrollbarsForMainFrame() const { notImplemented(); }

    virtual void didCommitLoadForMainFrame(bool) { notImplemented(); }
    virtual void didFinishLoadingDataForCustomRepresentation(const String&, const CoreIPC::DataReference&) { notImplemented(); }
    virtual double customRepresentationZoomFactor() { notImplemented(); return 1.0; }
    virtual void setCustomRepresentationZoomFactor(double) { notImplemented(); }

    virtual void flashBackingStoreUpdates(const Vector<IntRect>&) { notImplemented(); }
    virtual void findStringInCustomRepresentation(const String&, WebKit::FindOptions, unsigned) { notImplemented(); }
    virtual void countStringMatchesInCustomRepresentation(const String&, WebKit::FindOptions, unsigned) { notImplemented(); }

    virtual void updateTextInputState();
    virtual void didRenderFrame(const WebCore::IntSize&, const WebCore::IntRect&) { notImplemented(); }

    virtual void suspendActiveDOMObjectsAndAnimations();
    virtual void resumeActiveDOMObjectsAndAnimations();
    virtual bool isSuspended();

private:
    IntPoint roundedViewportPosition() const;
    LayerTreeRenderer* layerTreeRenderer();
    void updateVisibleContents();

    cairo_matrix_t userViewportToContentTransformation() const;
    cairo_matrix_t contentToUserViewportTransformation() const;


    FloatRect visibleRect() const;

    WebViewClient m_viewClient;
    WTF::RefPtr<WebPageProxy> m_webPageProxy;
    bool m_focused;
    bool m_visible;
    bool m_active;
    bool m_isSuspended;
    IntSize m_size;
    IntSize m_contentsSize;
    IntPoint m_lastCursorPosition;
    FloatPoint m_scrollPosition;
    float m_scale;
    float m_opacity;
    cairo_matrix_t m_userViewportTransformation;

    class CustomRenderer : public TextureMapperPlatformLayer {
    public:
        static PassOwnPtr<CustomRenderer> create(WebViewClient* client)
        {
            return adoptPtr(new CustomRenderer(client));
        }

        void setID(uint32_t id) { m_id = id; }
        void invalidate() { m_valid = false; }

    protected:
        virtual void paintToTextureMapper(TextureMapper*, const FloatRect&, const TransformationMatrix& modelViewMatrix = TransformationMatrix(), float opacity = 1.0, BitmapTexture* mask = 0);

    private:
        CustomRenderer(WebViewClient* client)
            : m_client(client)
            , m_valid(true)
        {
        }

        uint32_t m_id;
        bool m_valid;
        WebViewClient* m_client;
    };

    HashMap<uint32_t, OwnPtr<CustomRenderer> > m_customRenderers;
};

void WebViewImpl::CustomRenderer::paintToTextureMapper(TextureMapper* texmap, const FloatRect& rect, const TransformationMatrix& matrix, float opacity, BitmapTexture*)
{
    ASSERT(texmap->accelerationMode() == TextureMapper::OpenGLMode);
    static_cast<TextureMapperGL*>(texmap)->graphicsContext3D()->makeContextCurrent();
    WKRect wkRect = WKRectMake(rect.x(), rect.y(), rect.width(), rect.height());
    const float m4[] = {
        matrix.m11(), matrix.m12(), matrix.m13(), matrix.m14(),
        matrix.m21(), matrix.m22(), matrix.m23(), matrix.m24(),
        matrix.m31(), matrix.m32(), matrix.m33(), matrix.m34(),
        matrix.m41(), matrix.m42(), matrix.m43(), matrix.m44()
    };
    m_client->compositeCustomLayerToCurrentGLContext(m_id, wkRect, m4, opacity);
}

void WebViewImpl::removeCustomLayer(uint32_t id)
{
    HashMap<uint32_t, OwnPtr<CustomRenderer> >::iterator it = m_customRenderers.find(id);
    if (it == m_customRenderers.end())
        return;

    it->value->invalidate();
}


uint32_t WebViewImpl::addCustomLayer(WKStringRef elementID)
{
    DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();
    if (!drawingArea)
        return 0;

    LayerTreeCoordinatorProxy* coordinator = drawingArea->layerTreeCoordinatorProxy();
    String str = toImpl(elementID)->string();

    OwnPtr<CustomRenderer> renderer = CustomRenderer::create(&m_viewClient);

    uint32_t id = coordinator->addCustomPlatformLayer(str, renderer.get());
    renderer->setID(id);
    m_customRenderers.add(id, renderer.release());
    return id;
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

void WebViewImpl::setScale(float scale)
{
    if (m_scale == scale)
        return;

    m_scale = scale;
    if (!m_isSuspended)
        commitViewportChanges();
}

void WebViewImpl::setSize(const WKSize& size)
{
    IntSize newSize(size.width, size.height);

    if (m_size == newSize)
        return;

    m_size = newSize;
    commitViewportChanges();
}


IntPoint WebViewImpl::roundedViewportPosition() const
{
    return IntPoint(round(m_scrollPosition.x() * m_scale), round(m_scrollPosition.y() * m_scale));
}

void WebViewImpl::setScrollPosition(const WKPoint& position)
{
    if (m_scrollPosition.x() == position.x && m_scrollPosition.y() == position.y)
        return;

    FloatPoint trajectoryVector(position.x - m_scrollPosition.x(), position.y - m_scrollPosition.y());
    m_scrollPosition = FloatPoint(position.x, position.y);

    DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();
    if (!drawingArea)
        return;

    if (!m_isSuspended)
        drawingArea->setVisibleContentsRect(visibleRect(), m_scale, trajectoryVector);
}

WKPoint WebViewImpl::userViewportToContents(WKPoint point)
{
    cairo_matrix_t transformMatrix = userViewportToContentTransformation();
    cairo_matrix_transform_point(&transformMatrix, &point.x, &point.y);
    return point;
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
    m_viewClient.viewNeedsDisplay(WKRectMake(rect.x(), rect.y(), rect.width(), rect.height()));
}

WKPageRef WebViewImpl::pageRef()
{
    return toAPI(m_webPageProxy.get());
}

void WebViewImpl::sendEvent(const Nix::InputEvent&)
{
    notImplemented();
}

void WebViewImpl::sendMouseEvent(const NIXMouseEvent& event)
{
    m_webPageProxy->handleMouseEvent(NativeWebMouseEvent(event, &m_lastCursorPosition));
}

void WebViewImpl::sendTouchEvent(const NIXTouchEvent& event)
{
    m_webPageProxy->handleTouchEvent(NativeWebTouchEvent(event));
}

void WebViewImpl::sendWheelEvent(const NIXWheelEvent& event)
{
    m_webPageProxy->handleWheelEvent(NativeWebWheelEvent(event));
}

void WebViewImpl::sendKeyEvent(const NIXKeyEvent& event)
{
    m_webPageProxy->handleKeyboardEvent(NativeWebKeyboardEvent(event));
}

void WebViewImpl::sendGestureEvent(const NIXGestureEvent& event)
{
    m_webPageProxy->handleGestureEvent(NativeWebGestureEvent(event));
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

FloatRect WebViewImpl::visibleRect() const
{
    WKSize vcSize = visibleContentsSize();
    FloatRect viewport(m_scrollPosition, FloatSize(vcSize.width, vcSize.height));
    FloatRect visibleRect(0, 0, m_contentsSize.width(), m_contentsSize.height());
    visibleRect.intersect(viewport);
    return visibleRect;
}

cairo_matrix_t WebViewImpl::contentToUserViewportTransformation() const
{
    cairo_matrix_t transform;
    if (m_webPageProxy->useFixedLayout()) {
        // We switch the order between scale and translate here to make the resulting matrix hold
        // integer values for translation. If we did the natural order, the translation would be
        // the product between scroll position and scale and it could yield a float number for offset,
        // which would make the tiles blurry when painted. In the reverse order, we apply translation first
        // but it is a translation that already takes into account the scale, so the new matrix should be the
        // the same as in the natural order. In this case, though, we are free to do the rounding of the offset
        // and now will never translate into non-integer unit, producing now a slightly different matrix with
        // rounded translation.
        IntPoint roundedScrollPosition = roundedViewportPosition();
        cairo_matrix_init_translate(&transform, -roundedScrollPosition.x(), -roundedScrollPosition.y());
        cairo_matrix_scale(&transform, m_scale, m_scale);
    } else
        cairo_matrix_init_scale(&transform, m_scale, m_scale);
    cairo_matrix_multiply(&transform, &transform, &m_userViewportTransformation);
    return transform;
}

void WebViewImpl::paintToCurrentGLContext()
{
    LayerTreeRenderer* renderer = layerTreeRenderer();
    if (!renderer)
        return;

    double x = 0;
    double y = 0;
    double width = m_size.width();
    double height = m_size.height();
    cairo_matrix_transform_point(&m_userViewportTransformation, &x, &y);
    cairo_matrix_transform_distance(&m_userViewportTransformation, &width, &height);

    FloatRect rect(x, y, width, height);
    renderer->paintToCurrentGLContext(toTransformationMatrix(contentToUserViewportTransformation()), m_opacity, rect);
}

void WebViewImpl::commitViewportChanges()
{
    DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();

    if (!drawingArea)
        return;
    if (m_webPageProxy->useFixedLayout()) {
        m_webPageProxy->setViewportSize(m_size);
        drawingArea->layerTreeCoordinatorProxy()->setContentsSize(m_contentsSize);
    } else {
        drawingArea->setSize(m_size, IntSize());
    }
    drawingArea->setVisibleContentsRect(visibleRect(), m_scale, FloatPoint());
}

void WebViewImpl::findZoomableAreaForPoint(const WKPoint& point, int horizontalRadius, int verticalRadius)
{
    IntPoint contentsPoint(point.x, point.y);
    IntSize touchSize(horizontalRadius * 2, verticalRadius * 2);
    m_webPageProxy->findZoomableAreaForPoint(contentsPoint, touchSize);
}

void WebViewImpl::processDidCrash()
{
    m_viewClient.webProcessCrashed(toCopiedAPI(m_webPageProxy->urlAtProcessExit()));
}

void WebViewImpl::pageDidRequestScroll(const IntPoint& point)
{
    m_viewClient.pageDidRequestScroll(WKPointMake(point.x(), point.y()));
    // FIXME: It's not clear to me yet whether we should ask for display here or this is
    // at the wrong level and we should simply notify the client about this.
    setViewNeedsDisplay(IntRect(IntPoint(), m_size));
}

void WebViewImpl::didChangeContentsSize(const IntSize& size)
{
    m_contentsSize = size;
    m_viewClient.didChangeContentsSize(WKSizeMake(size.width(), size.height()));
    commitViewportChanges();
}

void WebViewImpl::didFindZoomableArea(const IntPoint& target, const IntRect& area)
{
    m_viewClient.didFindZoomableArea(WKPointMake(target.x(), target.y()), WKRectMake(area.x(), area.y(), area.width(), area.height()));
}

void WebViewImpl::pageTransitionViewportReady()
{
    m_webPageProxy->commitPageTransitionViewport();
}

cairo_matrix_t WebViewImpl::userViewportToContentTransformation() const
{
    cairo_matrix_t transform = contentToUserViewportTransformation();
    cairo_matrix_invert(&transform);
    return transform;
}

#if ENABLE(TOUCH_EVENTS)
void WebViewImpl::doneWithTouchEvent(const NativeWebTouchEvent& event, bool wasEventHandled)
{
    m_viewClient.doneWithTouchEvent(*event.nativeEvent(), wasEventHandled);
}
#endif

#if ENABLE(GESTURE_EVENTS)
void WebViewImpl::doneWithGestureEvent(const NativeWebGestureEvent& event, bool wasEventHandled)
{
    m_viewClient.doneWithGestureEvent(*event.nativeEvent(), wasEventHandled);
}
#endif

void WebViewImpl::updateTextInputState()
{
    const EditorState& editor = m_webPageProxy->editorState();
    bool isContentEditable = editor.isContentEditable;
    const IntRect& cursorRect = editor.cursorRect;
    const IntRect& editorRect = editor.editorRect;
    m_viewClient.updateTextInputState(isContentEditable, toAPI(cursorRect), toAPI(editorRect));
}

void WebViewImpl::suspendActiveDOMObjectsAndAnimations()
{
    m_webPageProxy->suspendActiveDOMObjectsAndAnimations();
    m_isSuspended = true;
}

void WebViewImpl::resumeActiveDOMObjectsAndAnimations()
{
    m_webPageProxy->resumeActiveDOMObjectsAndAnimations();
    m_isSuspended = false;
    commitViewportChanges();
}

bool WebViewImpl::isSuspended()
{
    return m_isSuspended;
}

} // namespace Nix

static Nix::WebViewImpl* toImpl(NIXView view)
{
    return reinterpret_cast<Nix::WebViewImpl*>(view);
}

static NIXView toAPI(Nix::WebViewImpl* view)
{
    return reinterpret_cast<NIXView>(view);
}

NIXView NIXViewCreate(WKContextRef context, WKPageGroupRef pageGroup, const NIXViewClient* viewClient)
{
    g_type_init();
    return toAPI(new Nix::WebViewImpl(toImpl(context), toImpl(pageGroup), viewClient));
}

void NIXViewRelease(NIXView view)
{
    delete toImpl(view);
}

void NIXViewInitialize(NIXView view)
{
    toImpl(view)->initialize();
}

WKSize NIXViewSize(NIXView view)
{
    return toImpl(view)->size();
}

void NIXViewSetSize(NIXView view, WKSize size)
{
    toImpl(view)->setSize(size);
}

WKPoint NIXViewScrollPosition(NIXView view)
{
    return toImpl(view)->scrollPosition();
}

void NIXViewSetScrollPosition(NIXView view, WKPoint position)
{
    toImpl(view)->setScrollPosition(position);
}

void NIXViewSetUserViewportTransformation(NIXView view, const NIXMatrix* userViewportTransformation)
{
    cairo_matrix_t transform;
    transform.xx = userViewportTransformation->xx;
    transform.yx = userViewportTransformation->yx;
    transform.xy = userViewportTransformation->xy;
    transform.yy = userViewportTransformation->yy;
    transform.x0 = userViewportTransformation->x0;
    transform.y0 = userViewportTransformation->y0;
    toImpl(view)->setUserViewportTransformation(transform);
}

WKPoint NIXViewUserViewportToContents(NIXView view, WKPoint point)
{
    return toImpl(view)->userViewportToContents(point);
}

bool NIXViewIsFocused(NIXView view)
{
    return toImpl(view)->isFocused();
}

void NIXViewSetFocused(NIXView view, bool focused)
{
    toImpl(view)->setFocused(focused);
}

bool NIXViewIsVisible(NIXView view)
{
    return toImpl(view)->isVisible();
}

void NIXViewSetVisible(NIXView view, bool visible)
{
    toImpl(view)->setVisible(visible);
}

bool NIXViewIsActive(NIXView view)
{
    return toImpl(view)->isActive();
}

void NIXViewSetActive(NIXView view, bool active)
{
    toImpl(view)->setActive(active);
}

bool NIXViewTransparentBackground(NIXView view)
{
    return toImpl(view)->transparentBackground();
}

void NIXViewSetTransparentBackground(NIXView view, bool transparent)
{
    toImpl(view)->setTransparentBackground(transparent);
}

bool NIXViewDrawBackground(NIXView view)
{
    return toImpl(view)->drawBackground();
}

void NIXViewSetDrawBackground(NIXView view, bool drawBackground)
{
    toImpl(view)->setDrawBackground(drawBackground);
}

float NIXViewScale(NIXView view)
{
    return toImpl(view)->scale();
}

void NIXViewSetScale(NIXView view, float scale)
{
    toImpl(view)->setScale(scale);
}

void NIXViewSetOpacity(NIXView view, float opacity)
{
    toImpl(view)->setOpacity(opacity);
}

float NIXViewOpacity(NIXView view)
{
    return toImpl(view)->opacity();
}

WKSize NIXViewVisibleContentsSize(NIXView view)
{
    return toImpl(view)->visibleContentsSize();
}

void NIXViewPaintToCurrentGLContext(NIXView view)
{
    toImpl(view)->paintToCurrentGLContext();
}

void NIXViewFindZoomableAreaForPoint(NIXView view, WKPoint point, int horizontalRadius, int verticalRadius)
{
    toImpl(view)->findZoomableAreaForPoint(point, horizontalRadius, verticalRadius);
}

uint32_t NIXViewAddCustomLayer(NIXView view, WKStringRef elementID)
{
    return toImpl(view)->addCustomLayer(elementID);
}

void NIXViewRemoveCustomLayer(NIXView view, uint32_t id)
{
    toImpl(view)->removeCustomLayer(id);
}

WKPageRef NIXViewGetPage(NIXView view)
{
    return toImpl(view)->pageRef();
}

void NIXViewSendMouseEvent(NIXView view, const NIXMouseEvent* event)
{
    toImpl(view)->sendMouseEvent(*event);
}

void NIXViewSendWheelEvent(NIXView view, const NIXWheelEvent* event)
{
    toImpl(view)->sendWheelEvent(*event);
}

void NIXViewSendKeyEvent(NIXView view, const NIXKeyEvent* event)
{
    toImpl(view)->sendKeyEvent(*event);
}

void NIXViewSendTouchEvent(NIXView view, const NIXTouchEvent* event)
{
    toImpl(view)->sendTouchEvent(*event);
}

void NIXViewSendGestureEvent(NIXView view, const NIXGestureEvent* event)
{
    toImpl(view)->sendGestureEvent(*event);
}

void NIXViewSuspendActiveDOMObjectsAndAnimations(NIXView view)
{
    toImpl(view)->suspendActiveDOMObjectsAndAnimations();
}

void NIXViewResumeActiveDOMObjectsAndAnimations(NIXView view)
{
    toImpl(view)->resumeActiveDOMObjectsAndAnimations();
}

bool NIXViewIsSuspended(NIXView view)
{
    return toImpl(view)->isSuspended();
}

