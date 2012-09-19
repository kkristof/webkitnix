#include "config.h"
#include "WebView.h"

#include "Cursor.h"
#include "DrawingAreaProxyImpl.h"
#include "LayerTreeCoordinatorProxy.h"
#include "LayerTreeRenderer.h"
#include "NativeWebKeyboardEvent.h"
#include "NativeWebMouseEvent.h"
#include "NativeWebWheelEvent.h"
#include "NotImplemented.h"
#include "PageClient.h"
#include "WebContext.h"
#include "WebContextMenuProxy.h"
#include "WebEvent.h"
#include "WebPageGroup.h"
#include "WebPopupMenuProxy.h"
#include "WebPreferences.h"
#include <WebCore/Scrollbar.h>

using namespace WebCore;
using namespace WebKit;

namespace Nix {

cairo_matrix_t WebViewClient::viewToScreenTransform()
{
    static cairo_matrix_t identityTransform = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };
    return identityTransform;
}

class WebViewImpl : public WebView, public PageClient {
public:
    WebViewImpl(WebContext* context, WebPageGroup* pageGroup, WebViewClient* client)
        : m_client(client)
        , m_webPageProxy(context->createWebPage(this, pageGroup))
    {
        m_webPageProxy->pageGroup()->preferences()->setForceCompositingMode(true);
    }

    virtual ~WebViewImpl() { }

    // WebView.
    virtual void initialize();

    virtual int width() const;
    virtual int height() const;
    virtual void setSize(int width, int height);

    virtual bool isFocused() const;
    virtual void setFocused(bool);

    virtual bool isVisible() const;
    virtual void setVisible(bool);

    virtual bool isActive() const;
    virtual void setActive(bool);

    virtual void paintToCurrentGLContext();

    virtual WKPageRef pageRef();

    virtual void sendEvent(const Nix::InputEvent&);
    // FIXME: Move this function to private when we remove it from public interface.
    virtual void sendKeyEvent(bool, char);

    // PageClient.
    virtual PassOwnPtr<DrawingAreaProxy> createDrawingAreaProxy();
    virtual void setViewNeedsDisplay(const IntRect&);

    virtual bool isViewFocused() { return m_focused; }
    virtual bool isViewVisible() { return m_visible; }
    virtual bool isViewWindowActive() { return m_active; }
    virtual bool isViewInWindow() { return true; } // FIXME
    virtual IntSize viewSize() { return m_size; }
    virtual void processDidCrash();
    virtual void didRelaunchProcess() { m_client->webProcessRelaunched(); }

    virtual void pageDidRequestScroll(const IntPoint& point);

    // PageClient not implemented.
    virtual void displayView() { notImplemented(); }
    virtual void scrollView(const IntRect& scrollRect, const IntSize& scrollOffset) { notImplemented(); }

    virtual void pageClosed() { notImplemented(); }

    virtual void toolTipChanged(const String&, const String&) { notImplemented(); }

    virtual void handleDownloadRequest(DownloadProxy*) { notImplemented(); }

    virtual void didChangeContentsSize(const IntSize&) { notImplemented(); }

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

    virtual void doneWithKeyEvent(const NativeWebKeyboardEvent&, bool wasEventHandled) { notImplemented(); }
#if ENABLE(GESTURE_EVENTS)
    virtual void doneWithGestureEvent(const WebGestureEvent&, bool wasEventHandled) { notImplemented(); }
#endif
#if ENABLE(TOUCH_EVENTS)
    virtual void doneWithTouchEvent(const NativeWebTouchEvent&, bool wasEventHandled) { notImplemented(); }
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
    void transformPointToViewCoordinates(double& x, double& y);

    void sendMouseEvent(const Nix::MouseEvent&);
    void sendWheelEvent(const Nix::WheelEvent&);

    WebViewClient* m_client;
    WTF::RefPtr<WebPageProxy> m_webPageProxy;
    bool m_focused;
    bool m_visible;
    bool m_active;
    IntSize m_size;
    IntPoint m_lastCursorPosition;
    IntPoint m_scrollPosition;
};

WebView* WebView::create(WKContextRef contextRef, WKPageGroupRef pageGroupRef, WebViewClient* client)
{
    g_type_init();
    return new WebViewImpl(toImpl(contextRef), toImpl(pageGroupRef), client);
}

void WebViewImpl::initialize()
{
    m_webPageProxy->initializeWebPage();
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
    m_webPageProxy->setViewportSize(m_size);

    DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();
    if (!drawingArea)
        return;

    drawingArea->setSize(m_size, IntSize());
    const float scale = 1.0;
    drawingArea->setVisibleContentsRect(IntRect(m_scrollPosition, m_size), scale, FloatPoint());
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

void WebViewImpl::sendKeyEvent(bool down, char key)
{
    WebEvent::Type type = down ? WebEvent::KeyDown : WebEvent::KeyUp;
    const WTF::String text = WTF::String(&key, 1);
    const WTF::String unmodifiedText = WTF::String(&key, 1);
    bool isAutoRepeat = false;
    bool isSystemKey = false; // FIXME: No idea what that is.
    bool isKeypad = false;
    const WTF::String keyIdentifier = WTF::String(&key, 1);
    int windowsVirtualKeyCode = 0;
    int nativeVirtualKeyCode = 0;
    int macCharCode = 0;
    WebEvent::Modifiers modifiers = WebEvent::Modifiers();
    double timestamp = 0;

    WebKeyboardEvent event(type, text, unmodifiedText, keyIdentifier, windowsVirtualKeyCode, nativeVirtualKeyCode, macCharCode, isAutoRepeat, isKeypad, isSystemKey, modifiers, timestamp);
    m_webPageProxy->handleKeyboardEvent(NativeWebKeyboardEvent(event));
}

static WebEvent::Type convertToWebEventType(Nix::InputEvent::Type type)
{
    switch (type) {
    case Nix::InputEvent::MouseDown:
        return WebEvent::MouseDown;
    case Nix::InputEvent::MouseUp:
        return WebEvent::MouseUp;
    case Nix::InputEvent::MouseMove:
        return WebEvent::MouseMove;
    case Nix::InputEvent::Wheel:
        return WebEvent::Wheel;
    default:
        notImplemented();
    }
    return WebEvent::MouseMove;
}

static WebMouseEvent::Button convertToWebMouseEventButton(Nix::MouseEvent::Button button)
{
    switch (button) {
    case Nix::MouseEvent::NoButton:
        return WebMouseEvent::NoButton;
    case Nix::MouseEvent::LeftButton:
        return WebMouseEvent::LeftButton;
    case Nix::MouseEvent::FourthButton:
    case Nix::MouseEvent::MiddleButton:
        return WebMouseEvent::MiddleButton;
    case Nix::MouseEvent::RightButton:
        return WebMouseEvent::RightButton;
    default:
        notImplemented();
    }
    return WebMouseEvent::NoButton;
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
            notImplemented();
            break;
        case InputEvent::TouchStart:
        case InputEvent::TouchMove:
        case InputEvent::TouchEnd:
        case InputEvent::TouchCancel:
            notImplemented();
            break;
    }
}

void WebViewImpl::sendMouseEvent(const Nix::MouseEvent& event)
{
    WebEvent::Type type = convertToWebEventType(event.type);
    WebMouseEvent::Button button = convertToWebMouseEventButton(event.button);

    double x = event.x;
    double y = event.y;
    transformPointToViewCoordinates(x, y);

    float deltaX = x - m_lastCursorPosition.x();
    float deltaY = y - m_lastCursorPosition.y();
    int clickCount = event.clickCount;
    WebEvent::Modifiers modifiers = static_cast<WebEvent::Modifiers>(event.modifiers);
    double timestamp = event.timestamp;
    IntPoint position = IntPoint(x, y);
    IntPoint globalPosition = IntPoint(event.globalX, event.globalY);
    m_lastCursorPosition = position;

    WebMouseEvent webEvent(type, button, position, globalPosition, deltaX, deltaY, 0.0f, clickCount, modifiers, timestamp);
    m_webPageProxy->handleMouseEvent(NativeWebMouseEvent(webEvent));
}

// TODO: Create a constructor in TransformationMatrix that takes a cairo_matrix_t.
static TransformationMatrix toTransformationMatrix(const cairo_matrix_t& matrix)
{
    return TransformationMatrix(matrix.xx, matrix.yx, matrix.xy, matrix.yy, matrix.x0, matrix.y0);
}

void WebViewImpl::paintToCurrentGLContext()
{
    DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();
    if (!drawingArea)
        return;

    LayerTreeCoordinatorProxy* coordinatorProxy = drawingArea->layerTreeCoordinatorProxy();
    if (!coordinatorProxy)
        return;

    LayerTreeRenderer* renderer = coordinatorProxy->layerTreeRenderer();
    if (!renderer)
        return;

    renderer->setActive(true);
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

    cairo_matrix_t transform;
    cairo_matrix_multiply(&transform, &scrollTransform, &viewTransform);

    FloatRect rect(x, y, width, height);
    renderer->paintToCurrentGLContext(toTransformationMatrix(transform), 1.0, rect);
}

void WebViewImpl::processDidCrash()
{
    m_client->webProcessCrashed(toCopiedAPI(m_webPageProxy->urlAtProcessExit()));
}

void WebViewImpl::pageDidRequestScroll(const IntPoint& point)
{
    if (m_scrollPosition == point)
        return;
    m_scrollPosition = point;

    DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();
    if (!drawingArea)
        return;

    const float scale = 1.0;
    drawingArea->setVisibleContentsRect(IntRect(m_scrollPosition, m_size), scale, FloatPoint());

    // FIXME: It's not clear to me yet whether we should ask for display here or this is at the wrong level.
    setViewNeedsDisplay(IntRect(IntPoint(), m_size));
}

void WebViewImpl::transformPointToViewCoordinates(double& x, double& y)
{
    cairo_matrix_t invertedViewTransform = m_client->viewToScreenTransform();
    cairo_matrix_invert(&invertedViewTransform);

    cairo_matrix_t invertedScrollTransform;
    cairo_matrix_init_translate(&invertedScrollTransform, m_scrollPosition.x(), m_scrollPosition.y());

    cairo_matrix_t transform;
    cairo_matrix_multiply(&transform, &invertedViewTransform, &invertedScrollTransform);

    cairo_matrix_transform_point(&transform, &x, &y);
}

void WebViewImpl::sendWheelEvent(const Nix::WheelEvent& event)
{
    WebEvent::Type type = convertToWebEventType(event.type);

    double x = event.x;
    double y = event.y;
    transformPointToViewCoordinates(x, y);

    IntPoint position = IntPoint(x, y);
    IntPoint globalPosition = IntPoint(event.globalX, event.globalY);
    FloatSize delta = event.orientation == Nix::WheelEvent::Vertical ? FloatSize(0, event.delta) : FloatSize(event.delta, 0);
    WebEvent::Modifiers modifiers = static_cast<WebEvent::Modifiers>(event.modifiers);
    double timestamp = event.timestamp;

    const float ticks = event.delta / float(Scrollbar::pixelsPerLineStep());
    FloatSize wheelTicks = Nix::WheelEvent::Vertical ? FloatSize(0, ticks) : FloatSize(ticks, 0);

    WebWheelEvent webEvent(type, position, globalPosition, delta, wheelTicks, WebWheelEvent::ScrollByPixelWheelEvent, modifiers, timestamp);
    m_webPageProxy->handleWheelEvent(NativeWebWheelEvent(webEvent));
}

} // namespace Nix
