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

namespace Nix {

class WebViewImpl : public WebView, public WebKit::PageClient {
public:
    WebViewImpl(WebKit::WebContext* context, WebKit::WebPageGroup* pageGroup, WebViewClient* client)
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
    virtual PassOwnPtr<WebKit::DrawingAreaProxy> createDrawingAreaProxy();
    virtual void setViewNeedsDisplay(const WebCore::IntRect&);

    virtual bool isViewFocused() { return m_focused; }
    virtual bool isViewVisible() { return m_visible; }
    virtual bool isViewWindowActive() { return m_active; }
    virtual bool isViewInWindow() { return true; } // FIXME
    virtual WebCore::IntSize viewSize() { return m_size; }
    virtual void processDidCrash();
    virtual void didRelaunchProcess() { m_client->webProcessRelaunched(); }

    virtual void pageDidRequestScroll(const WebCore::IntPoint& point);

    // PageClient not implemented.
    virtual void displayView() { notImplemented(); }
    virtual void scrollView(const WebCore::IntRect& scrollRect, const WebCore::IntSize& scrollOffset) { notImplemented(); }

    virtual void pageClosed() { notImplemented(); }

    virtual void toolTipChanged(const String&, const String&) { notImplemented(); }

    virtual void handleDownloadRequest(WebKit::DownloadProxy*) { notImplemented(); }

    virtual void didChangeContentsSize(const WebCore::IntSize&) { notImplemented(); }

    virtual void setCursor(const WebCore::Cursor&) { notImplemented(); }
    virtual void setCursorHiddenUntilMouseMoves(bool) { notImplemented(); }
    virtual void didChangeViewportProperties(const WebCore::ViewportAttributes&) { notImplemented(); }

    virtual void registerEditCommand(PassRefPtr<WebKit::WebEditCommandProxy>, WebKit::WebPageProxy::UndoOrRedo) { notImplemented(); }
    virtual void clearAllEditCommands() { notImplemented(); }
    virtual bool canUndoRedo(WebKit::WebPageProxy::UndoOrRedo) { notImplemented(); return false; }
    virtual void executeUndoRedo(WebKit::WebPageProxy::UndoOrRedo) { notImplemented(); }

    virtual WebCore::FloatRect convertToDeviceSpace(const WebCore::FloatRect& rect) { notImplemented(); return rect; }
    virtual WebCore::FloatRect convertToUserSpace(const WebCore::FloatRect& rect) { notImplemented(); return rect; }
    virtual WebCore::IntPoint screenToWindow(const WebCore::IntPoint& point) { notImplemented(); return point; }
    virtual WebCore::IntRect windowToScreen(const WebCore::IntRect& rect) { notImplemented(); return rect; }

    virtual void doneWithKeyEvent(const WebKit::NativeWebKeyboardEvent&, bool wasEventHandled) { notImplemented(); }
#if ENABLE(GESTURE_EVENTS)
    virtual void doneWithGestureEvent(const WebGestureEvent&, bool wasEventHandled) { notImplemented(); }
#endif
#if ENABLE(TOUCH_EVENTS)
    virtual void doneWithTouchEvent(const WebKit::NativeWebTouchEvent&, bool wasEventHandled) { notImplemented(); }
#endif

    virtual PassRefPtr<WebKit::WebPopupMenuProxy> createPopupMenuProxy(WebKit::WebPageProxy*) { notImplemented(); return PassRefPtr<WebKit::WebPopupMenuProxy>(); }
    virtual PassRefPtr<WebKit::WebContextMenuProxy> createContextMenuProxy(WebKit::WebPageProxy*) { notImplemented(); return PassRefPtr<WebKit::WebContextMenuProxy>(); }

#if ENABLE(INPUT_TYPE_COLOR)
    virtual PassRefPtr<WebKit::WebColorChooserProxy> createColorChooserProxy(WebKit::WebPageProxy*, const WebCore::Color& initialColor) { notImplemented(); return PassRefPtr<WebKit::WebColorChooserProxy>(); }
#endif

    virtual void setFindIndicator(PassRefPtr<WebKit::FindIndicator>, bool fadeOut, bool animate)  { notImplemented(); }

#if USE(ACCELERATED_COMPOSITING)
    virtual void enterAcceleratedCompositingMode(const WebKit::LayerTreeContext&) { notImplemented(); }
    virtual void exitAcceleratedCompositingMode() { notImplemented(); }
    virtual void updateAcceleratedCompositingMode(const WebKit::LayerTreeContext&) { notImplemented(); }
#endif

    virtual void didChangeScrollbarsForMainFrame() const { notImplemented(); }

    virtual void didCommitLoadForMainFrame(bool useCustomRepresentation) { notImplemented(); }
    virtual void didFinishLoadingDataForCustomRepresentation(const String& suggestedFilename, const CoreIPC::DataReference&) { notImplemented(); }
    virtual double customRepresentationZoomFactor() { notImplemented(); return 1.0; }
    virtual void setCustomRepresentationZoomFactor(double) { notImplemented(); }

    virtual void flashBackingStoreUpdates(const Vector<WebCore::IntRect>& updateRects) { notImplemented(); }
    virtual void findStringInCustomRepresentation(const String&, WebKit::FindOptions, unsigned maxMatchCount) { notImplemented(); }
    virtual void countStringMatchesInCustomRepresentation(const String&, WebKit::FindOptions, unsigned maxMatchCount) { notImplemented(); }

private:
    void sendMouseEvent(const Nix::MouseEvent&);
    void sendWheelEvent(const Nix::WheelEvent&);

    WebViewClient* m_client;
    WTF::RefPtr<WebKit::WebPageProxy> m_webPageProxy;
    bool m_focused;
    bool m_visible;
    bool m_active;
    WebCore::IntSize m_size;
    WebCore::IntPoint m_lastCursorPosition;
    WebCore::IntPoint m_scrollPosition;
};

WebView* WebView::create(WKContextRef contextRef, WKPageGroupRef pageGroupRef, WebViewClient* client)
{
    g_type_init();
    return new WebViewImpl(WebKit::toImpl(contextRef), WebKit::toImpl(pageGroupRef), client);
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
    m_size = WebCore::IntSize(width, height);
    m_webPageProxy->setViewportSize(m_size);

    WebKit::DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();
    if (!drawingArea)
        return;

    drawingArea->setSize(m_size, WebCore::IntSize());
    const float scale = 1.0;
    drawingArea->setVisibleContentsRect(WebCore::IntRect(m_scrollPosition, m_size), scale, WebCore::FloatPoint());
}

bool WebViewImpl::isFocused() const
{
    return m_focused;
}

void WebViewImpl::setFocused(bool focused)
{
    m_focused = focused;
    m_webPageProxy->viewStateDidChange(WebKit::WebPageProxy::ViewIsFocused);
}

bool WebViewImpl::isVisible() const
{
    return m_visible;
}

void WebViewImpl::setVisible(bool visible)
{
    m_visible = visible;
    m_webPageProxy->viewStateDidChange(WebKit::WebPageProxy::ViewIsVisible);
}

bool WebViewImpl::isActive() const
{
    return m_active;
}

void WebViewImpl::setActive(bool active)
{
    m_active = active;
    m_webPageProxy->viewStateDidChange(WebKit::WebPageProxy::ViewWindowIsActive);
}

PassOwnPtr<WebKit::DrawingAreaProxy> WebViewImpl::createDrawingAreaProxy()
{
    return WebKit::DrawingAreaProxyImpl::create(m_webPageProxy.get());
}

void WebViewImpl::setViewNeedsDisplay(const WebCore::IntRect& rect)
{
    m_client->viewNeedsDisplay(rect.x(), rect.y(), rect.width(), rect.height());
}

WKPageRef WebViewImpl::pageRef()
{
    return toAPI(m_webPageProxy.get());
}

void WebViewImpl::sendKeyEvent(bool down, char key)
{
    WebKit::WebEvent::Type type = down ? WebKit::WebEvent::KeyDown : WebKit::WebEvent::KeyUp;
    const WTF::String text = WTF::String(&key, 1);
    const WTF::String unmodifiedText = WTF::String(&key, 1);
    bool isAutoRepeat = false;
    bool isSystemKey = false; // FIXME: No idea what that is.
    bool isKeypad = false;
    const WTF::String keyIdentifier = WTF::String(&key, 1);
    int windowsVirtualKeyCode = 0;
    int nativeVirtualKeyCode = 0;
    int macCharCode = 0;
    WebKit::WebEvent::Modifiers modifiers = WebKit::WebEvent::Modifiers();
    double timestamp = 0;

    WebKit::WebKeyboardEvent event(type, text, unmodifiedText, keyIdentifier, windowsVirtualKeyCode, nativeVirtualKeyCode, macCharCode, isAutoRepeat, isKeypad, isSystemKey, modifiers, timestamp);
    m_webPageProxy->handleKeyboardEvent(WebKit::NativeWebKeyboardEvent(event));
}

static WebKit::WebEvent::Type convertToWebEventType(Nix::InputEvent::Type type)
{
    switch (type) {
    case Nix::InputEvent::MouseDown:
        return WebKit::WebEvent::MouseDown;
    case Nix::InputEvent::MouseUp:
        return WebKit::WebEvent::MouseUp;
    case Nix::InputEvent::MouseMove:
        return WebKit::WebEvent::MouseMove;
    case Nix::InputEvent::Wheel:
        return WebKit::WebEvent::Wheel;
    default:
        notImplemented();
    }
    return WebKit::WebEvent::MouseMove;
}

static WebKit::WebMouseEvent::Button convertToWebMouseEventButton(Nix::MouseEvent::Button button)
{
    switch (button) {
    case Nix::MouseEvent::NoButton:
        return WebKit::WebMouseEvent::NoButton;
    case Nix::MouseEvent::LeftButton:
        return WebKit::WebMouseEvent::LeftButton;
    case Nix::MouseEvent::FourthButton:
    case Nix::MouseEvent::MiddleButton:
        return WebKit::WebMouseEvent::MiddleButton;
    case Nix::MouseEvent::RightButton:
        return WebKit::WebMouseEvent::RightButton;
    default:
        notImplemented();
    }
    return WebKit::WebMouseEvent::NoButton;
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
    WebKit::WebEvent::Type type = convertToWebEventType(event.type);
    WebKit::WebMouseEvent::Button button = convertToWebMouseEventButton(event.button);
    float deltaX = event.x - m_lastCursorPosition.x();
    float deltaY = event.y - m_lastCursorPosition.y();
    int clickCount = event.clickCount;
    WebKit::WebEvent::Modifiers modifiers = static_cast<WebKit::WebEvent::Modifiers>(event.modifiers);
    double timestamp = event.timestamp;
    WebCore::IntPoint position = WebCore::IntPoint(event.x, event.y);
    WebCore::IntPoint globalPosition = WebCore::IntPoint(event.globalX, event.globalY);
    m_lastCursorPosition = position;

    WebKit::WebMouseEvent webEvent(type, button, position, globalPosition, deltaX, deltaY, 0.0f, clickCount, modifiers, timestamp);
    m_webPageProxy->handleMouseEvent(WebKit::NativeWebMouseEvent(webEvent));
}

void WebViewImpl::paintToCurrentGLContext()
{
    WebKit::DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();
    if (!drawingArea)
        return;

    WebKit::LayerTreeCoordinatorProxy* coordinatorProxy = drawingArea->layerTreeCoordinatorProxy();
    if (!coordinatorProxy)
        return;

    WebKit::LayerTreeRenderer* renderer = coordinatorProxy->layerTreeRenderer();
    if (!renderer)
        return;

    renderer->setActive(true);
    renderer->syncRemoteContent();

    WebCore::TransformationMatrix scrollTransform;
    scrollTransform.translate(-m_scrollPosition.x(), -m_scrollPosition.y());

    WebCore::FloatRect rect(0, 0, m_size.width(), m_size.height());
    renderer->paintToCurrentGLContext(scrollTransform, 1.0, rect);
}

void WebViewImpl::processDidCrash()
{
    m_client->webProcessCrashed(WebKit::toCopiedAPI(m_webPageProxy->urlAtProcessExit()));
}

void WebViewImpl::pageDidRequestScroll(const WebCore::IntPoint& point)
{
    if (m_scrollPosition == point)
        return;
    m_scrollPosition = point;

    WebKit::DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();
    if (!drawingArea)
        return;

    const float scale = 1.0;
    drawingArea->setVisibleContentsRect(WebCore::IntRect(m_scrollPosition, m_size), scale, WebCore::FloatPoint());

    // FIXME: It's not clear to me yet whether we should ask for display here or this is at the wrong level.
    setViewNeedsDisplay(WebCore::IntRect(WebCore::IntPoint(), m_size));
}

void WebViewImpl::sendWheelEvent(const Nix::WheelEvent& event)
{
    WebKit::WebEvent::Type type = convertToWebEventType(event.type);
    WebCore::IntPoint position = WebCore::IntPoint(event.x, event.y);
    WebCore::IntPoint globalPosition = WebCore::IntPoint(event.globalX, event.globalY);
    WebCore::FloatSize delta = event.orientation == Nix::WheelEvent::Vertical ? WebCore::FloatSize(0, event.delta) : WebCore::FloatSize(event.delta, 0);
    WebKit::WebEvent::Modifiers modifiers = static_cast<WebKit::WebEvent::Modifiers>(event.modifiers);
    double timestamp = event.timestamp;

    const float ticks = event.delta / float(WebCore::Scrollbar::pixelsPerLineStep());
    WebCore::FloatSize wheelTicks = Nix::WheelEvent::Vertical ? WebCore::FloatSize(0, ticks) : WebCore::FloatSize(ticks, 0);

    WebKit::WebWheelEvent webEvent(type, position, globalPosition, delta, wheelTicks, WebKit::WebWheelEvent::ScrollByPixelWheelEvent, modifiers, timestamp);
    m_webPageProxy->handleWheelEvent(WebKit::NativeWebWheelEvent(webEvent));
}

} // namespace Nix
