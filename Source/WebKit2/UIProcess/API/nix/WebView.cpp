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
    void updateVisibleContents();

    void transformPointToViewCoordinates(double& x, double& y);

    void sendMouseEvent(const Nix::MouseEvent&);
    void sendWheelEvent(const Nix::WheelEvent&);
    void sendKeyEvent(const Nix::KeyEvent&);
    void sendTouchEvent(const Nix::TouchEvent& event);

    WebViewClient* m_client;
    WTF::RefPtr<WebPageProxy> m_webPageProxy;
    bool m_focused;
    bool m_visible;
    bool m_active;
    IntSize m_size;
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

    DrawingAreaProxy* drawingArea = m_webPageProxy->drawingArea();
    if (!drawingArea)
        return;

    drawingArea->setVisibleContentsRect(IntRect(m_scrollPosition, IntSize(visibleContentWidth(), visibleContentHeight())), m_scale, FloatPoint());
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
    case Nix::InputEvent::KeyDown:
        return WebEvent::KeyDown;
    case Nix::InputEvent::KeyUp:
        return WebEvent::KeyUp;
    case Nix::InputEvent::TouchStart:
        return WebEvent::TouchStart;
    case Nix::InputEvent::TouchMove:
        return WebEvent::TouchMove;
    case Nix::InputEvent::TouchEnd:
        return WebEvent::TouchEnd;
    case Nix::InputEvent::TouchCancel:
        return WebEvent::TouchCancel;
    default:
        notImplemented();
    }
    return WebEvent::MouseMove;
}

static WebEvent::Modifiers convertToWebEventModifiers(unsigned modifiers)
{
    unsigned webModifiers;
    if (modifiers & InputEvent::ShiftKey)
        webModifiers |= WebEvent::ShiftKey;
    if (modifiers & InputEvent::ControlKey)
        webModifiers |= WebEvent::ControlKey;
    if (modifiers & InputEvent::AltKey)
        webModifiers |= WebEvent::AltKey;
    if (modifiers & InputEvent::MetaKey)
        webModifiers |= WebEvent::MetaKey;
    if (modifiers & InputEvent::CapsLockKey)
        webModifiers |= WebEvent::CapsLockKey;
    return static_cast<WebEvent::Modifiers>(webModifiers);
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
            sendKeyEvent(static_cast<const Nix::KeyEvent&>(event));
            break;
        case InputEvent::TouchStart:
        case InputEvent::TouchMove:
        case InputEvent::TouchEnd:
        case InputEvent::TouchCancel:
            sendTouchEvent(static_cast<const Nix::TouchEvent&>(event));
            break;
    }
}

static WebPlatformTouchPoint::TouchPointState convertToWebTouchState(const Nix::TouchPoint::TouchState& state)
{
    switch (state) {
    case Nix::TouchPoint::TouchReleased:
        return WebPlatformTouchPoint::TouchReleased;
    case Nix::TouchPoint::TouchPressed:
        return WebPlatformTouchPoint::TouchPressed;
    case Nix::TouchPoint::TouchMoved:
        return WebPlatformTouchPoint::TouchMoved;
    case Nix::TouchPoint::TouchStationary:
        return WebPlatformTouchPoint::TouchStationary;
    case Nix::TouchPoint::TouchCancelled:
        return WebPlatformTouchPoint::TouchCancelled;
    default:
        notImplemented();
    }
    return WebPlatformTouchPoint::TouchCancelled;
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
    WebEvent::Modifiers modifiers = convertToWebEventModifiers(event.modifiers);
    double timestamp = event.timestamp;
    IntPoint position = IntPoint(x, y);
    IntPoint globalPosition = IntPoint(event.globalX, event.globalY);
    m_lastCursorPosition = position;

    WebMouseEvent webEvent(type, button, position, globalPosition, deltaX, deltaY, 0.0f, clickCount, modifiers, timestamp);
    m_webPageProxy->handleMouseEvent(NativeWebMouseEvent(webEvent));
}

void WebViewImpl::sendTouchEvent(const Nix::TouchEvent& event)
{
    WebEvent::Type type = convertToWebEventType(event.type);
    Vector<WebPlatformTouchPoint> touchPoints;
    WebEvent::Modifiers modifiers = convertToWebEventModifiers(event.modifiers);
    double timestamp = event.timestamp;

    for (size_t i = 0; i < event.touchPoints.size(); ++i) {
        const Nix::TouchPoint& touch = event.touchPoints[i];
        uint32_t id = static_cast<uint32_t>(touch.id);
        WebPlatformTouchPoint::TouchPointState state = convertToWebTouchState(touch.state);
        IntPoint screenPosition = IntPoint(touch.globalX, touch.globalY);
        IntPoint position = IntPoint(touch.x, touch.y);
        IntSize radius = IntSize(touch.horizontalRadius, touch.verticalRadius);
        float rotationAngle = touch.rotationAngle;
        float force = touch.pressure;

        WebPlatformTouchPoint webTouchPoint = WebPlatformTouchPoint(id, state, screenPosition, position, radius, rotationAngle, force);
        touchPoints.append(webTouchPoint);
    }

    WebTouchEvent webEvent(type, touchPoints, modifiers, timestamp);
    m_webPageProxy->handleTouchEvent(NativeWebTouchEvent(webEvent));
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
//    setScrollPosition(point.x(), point.y());
    // FIXME: It's not clear to me yet whether we should ask for display here or this is
    // at the wrong level and we should simply notify the client about this.
    setViewNeedsDisplay(IntRect(IntPoint(), m_size));
}

void WebViewImpl::transformPointToViewCoordinates(double& x, double& y)
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
    WebEvent::Modifiers modifiers = convertToWebEventModifiers(event.modifiers);
    double timestamp = event.timestamp;

    const float ticks = event.delta / float(Scrollbar::pixelsPerLineStep());
    FloatSize wheelTicks = Nix::WheelEvent::Vertical ? FloatSize(0, ticks) : FloatSize(ticks, 0);

    WebWheelEvent webEvent(type, position, globalPosition, delta, wheelTicks, WebWheelEvent::ScrollByPixelWheelEvent, modifiers, timestamp);
    m_webPageProxy->handleWheelEvent(NativeWebWheelEvent(webEvent));
}

static String keyIdentifierForNixKeyCode(KeyEvent::Key keyCode)
{
    switch (keyCode) {
    case KeyEvent::Key_Menu:
    case KeyEvent::Key_Alt:
        return ASCIILiteral("Alt");
    case KeyEvent::Key_Clear:
        return ASCIILiteral("Clear");
    case KeyEvent::Key_Down:
        return ASCIILiteral("Down");
    case KeyEvent::Key_End:
        return ASCIILiteral("End");
    case KeyEvent::Key_Return:
    case KeyEvent::Key_Enter:
        return ASCIILiteral("Enter");
    case KeyEvent::Key_Execute:
        return ASCIILiteral("Execute");
    case KeyEvent::Key_F1:
        return ASCIILiteral("F1");
    case KeyEvent::Key_F2:
        return ASCIILiteral("F2");
    case KeyEvent::Key_F3:
        return ASCIILiteral("F3");
    case KeyEvent::Key_F4:
        return ASCIILiteral("F4");
    case KeyEvent::Key_F5:
        return ASCIILiteral("F5");
    case KeyEvent::Key_F6:
        return ASCIILiteral("F6");
    case KeyEvent::Key_F7:
        return ASCIILiteral("F7");
    case KeyEvent::Key_F8:
        return ASCIILiteral("F8");
    case KeyEvent::Key_F9:
        return ASCIILiteral("F9");
    case KeyEvent::Key_F10:
        return ASCIILiteral("F10");
    case KeyEvent::Key_F11:
        return ASCIILiteral("F11");
    case KeyEvent::Key_F12:
        return ASCIILiteral("F12");
    case KeyEvent::Key_F13:
        return ASCIILiteral("F13");
    case KeyEvent::Key_F14:
        return ASCIILiteral("F14");
    case KeyEvent::Key_F15:
        return ASCIILiteral("F15");
    case KeyEvent::Key_F16:
        return ASCIILiteral("F16");
    case KeyEvent::Key_F17:
        return ASCIILiteral("F17");
    case KeyEvent::Key_F18:
        return ASCIILiteral("F18");
    case KeyEvent::Key_F19:
        return ASCIILiteral("F19");
    case KeyEvent::Key_F20:
        return ASCIILiteral("F20");
    case KeyEvent::Key_F21:
        return ASCIILiteral("F21");
    case KeyEvent::Key_F22:
        return ASCIILiteral("F22");
    case KeyEvent::Key_F23:
        return ASCIILiteral("F23");
    case KeyEvent::Key_F24:
        return ASCIILiteral("F24");
    case KeyEvent::Key_Help:
        return ASCIILiteral("Help");
    case KeyEvent::Key_Home:
        return ASCIILiteral("Home");
    case KeyEvent::Key_Insert:
        return ASCIILiteral("Insert");
    case KeyEvent::Key_Left:
        return ASCIILiteral("Left");
    case KeyEvent::Key_PageDown:
        return ASCIILiteral("PageDown");
    case KeyEvent::Key_PageUp:
        return ASCIILiteral("PageUp");
    case KeyEvent::Key_Pause:
        return ASCIILiteral("Pause");
    case KeyEvent::Key_Print:
        return ASCIILiteral("PrintScreen");
    case KeyEvent::Key_Right:
        return ASCIILiteral("Right");
    case KeyEvent::Key_Select:
        return ASCIILiteral("Select");
    case KeyEvent::Key_Up:
        return ASCIILiteral("Up");
    case KeyEvent::Key_Delete:
        return ASCIILiteral("U+007F");
    case KeyEvent::Key_Backspace:
        return ASCIILiteral("U+0008");
    case KeyEvent::Key_Tab:
        return ASCIILiteral("U+0009");
    case KeyEvent::Key_Backtab:
        return ASCIILiteral("U+0009");
    default:
        return String::format("U+%04X", toASCIIUpper((int) keyCode));
    }
}

static int windowsKeyCodeForKeyEvent(KeyEvent::Key keycode, bool isKeypad)
{
    if (isKeypad) {
        switch (keycode) {
        case KeyEvent::Key_0:
            return VK_NUMPAD0;
        case KeyEvent::Key_1:
            return VK_NUMPAD1;
        case KeyEvent::Key_2:
            return VK_NUMPAD2;
        case KeyEvent::Key_3:
            return VK_NUMPAD3;
        case KeyEvent::Key_4:
            return VK_NUMPAD4;
        case KeyEvent::Key_5:
            return VK_NUMPAD5;
        case KeyEvent::Key_6:
            return VK_NUMPAD6;
        case KeyEvent::Key_7:
            return VK_NUMPAD7;
        case KeyEvent::Key_8:
            return VK_NUMPAD8;
        case KeyEvent::Key_9:
            return VK_NUMPAD9;
        case KeyEvent::Key_Asterisk:
            return VK_MULTIPLY;
        case KeyEvent::Key_Plus:
            return VK_ADD;
        case KeyEvent::Key_Minus:
            return VK_SUBTRACT;
        case KeyEvent::Key_Period:
            return VK_DECIMAL;
        case KeyEvent::Key_Slash:
            return VK_DIVIDE;
        case KeyEvent::Key_PageUp:
            return VK_PRIOR;
        case KeyEvent::Key_PageDown:
            return VK_NEXT;
        case KeyEvent::Key_End:
            return VK_END;
        case KeyEvent::Key_Home:
            return VK_HOME;
        case KeyEvent::Key_Left:
            return VK_LEFT;
        case KeyEvent::Key_Up:
            return VK_UP;
        case KeyEvent::Key_Right:
            return VK_RIGHT;
        case KeyEvent::Key_Down:
            return VK_DOWN;
        case KeyEvent::Key_Enter:
        case KeyEvent::Key_Return:
            return VK_RETURN;
        case KeyEvent::Key_Insert:
            return VK_INSERT;
        case KeyEvent::Key_Delete:
            return VK_DELETE;
        default:
            return 0;
        }

    } else {
        switch (keycode) {
        case KeyEvent::Key_Backspace:
            return VK_BACK;
        case KeyEvent::Key_Backtab:
        case KeyEvent::Key_Tab:
            return VK_TAB;
        case KeyEvent::Key_Clear:
            return VK_CLEAR;
        case KeyEvent::Key_Enter:
        case KeyEvent::Key_Return:
            return VK_RETURN;
        case KeyEvent::Key_Shift:
            return VK_SHIFT;
        case KeyEvent::Key_Control:
            return VK_CONTROL;
        case KeyEvent::Key_Menu:
        case KeyEvent::Key_Alt:
            return VK_MENU;
        case KeyEvent::Key_F1:
            return VK_F1;
        case KeyEvent::Key_F2:
            return VK_F2;
        case KeyEvent::Key_F3:
            return VK_F3;
        case KeyEvent::Key_F4:
            return VK_F4;
        case KeyEvent::Key_F5:
            return VK_F5;
        case KeyEvent::Key_F6:
            return VK_F6;
        case KeyEvent::Key_F7:
            return VK_F7;
        case KeyEvent::Key_F8:
            return VK_F8;
        case KeyEvent::Key_F9:
            return VK_F9;
        case KeyEvent::Key_F10:
            return VK_F10;
        case KeyEvent::Key_F11:
            return VK_F11;
        case KeyEvent::Key_F12:
            return VK_F12;
        case KeyEvent::Key_F13:
            return VK_F13;
        case KeyEvent::Key_F14:
            return VK_F14;
        case KeyEvent::Key_F15:
            return VK_F15;
        case KeyEvent::Key_F16:
            return VK_F16;
        case KeyEvent::Key_F17:
            return VK_F17;
        case KeyEvent::Key_F18:
            return VK_F18;
        case KeyEvent::Key_F19:
            return VK_F19;
        case KeyEvent::Key_F20:
            return VK_F20;
        case KeyEvent::Key_F21:
            return VK_F21;
        case KeyEvent::Key_F22:
            return VK_F22;
        case KeyEvent::Key_F23:
            return VK_F23;
        case KeyEvent::Key_F24:
            return VK_F24;
        case KeyEvent::Key_Pause:
            return VK_PAUSE;
        case KeyEvent::Key_CapsLock:
            return VK_CAPITAL;
        case KeyEvent::Key_Kana_Lock:
        case KeyEvent::Key_Kana_Shift:
            return VK_KANA;
        case KeyEvent::Key_Hangul:
            return VK_HANGUL;
        case KeyEvent::Key_Hangul_Hanja:
            return VK_HANJA;
        case KeyEvent::Key_Kanji:
            return VK_KANJI;
        case KeyEvent::Key_Escape:
            return VK_ESCAPE;
        case KeyEvent::Key_Space:
            return VK_SPACE;
        case KeyEvent::Key_PageUp:
            return VK_PRIOR;
        case KeyEvent::Key_PageDown:
            return VK_NEXT;
        case KeyEvent::Key_End:
            return VK_END;
        case KeyEvent::Key_Home:
            return VK_HOME;
        case KeyEvent::Key_Left:
            return VK_LEFT;
        case KeyEvent::Key_Up:
            return VK_UP;
        case KeyEvent::Key_Right:
            return VK_RIGHT;
        case KeyEvent::Key_Down:
            return VK_DOWN;
        case KeyEvent::Key_Select:
            return VK_SELECT;
        case KeyEvent::Key_Print:
            return VK_SNAPSHOT;
        case KeyEvent::Key_Execute:
            return VK_EXECUTE;
        case KeyEvent::Key_Insert:
            return VK_INSERT;
        case KeyEvent::Key_Delete:
            return VK_DELETE;
        case KeyEvent::Key_Help:
            return VK_HELP;
        case KeyEvent::Key_0:
        case KeyEvent::Key_ParenLeft:
            return VK_0;
        case KeyEvent::Key_1:
            return VK_1;
        case KeyEvent::Key_2:
        case KeyEvent::Key_At:
            return VK_2;
        case KeyEvent::Key_3:
        case KeyEvent::Key_NumberSign:
            return VK_3;
        case KeyEvent::Key_4:
        case KeyEvent::Key_Dollar:
            return VK_4;
        case KeyEvent::Key_5:
        case KeyEvent::Key_Percent:
            return VK_5;
        case KeyEvent::Key_6:
        case KeyEvent::Key_AsciiCircum:
            return VK_6;
        case KeyEvent::Key_7:
        case KeyEvent::Key_Ampersand:
            return VK_7;
        case KeyEvent::Key_8:
        case KeyEvent::Key_Asterisk:
            return VK_8;
        case KeyEvent::Key_9:
        case KeyEvent::Key_ParenRight:
            return VK_9;
        case KeyEvent::Key_A:
            return VK_A;
        case KeyEvent::Key_B:
            return VK_B;
        case KeyEvent::Key_C:
            return VK_C;
        case KeyEvent::Key_D:
            return VK_D;
        case KeyEvent::Key_E:
            return VK_E;
        case KeyEvent::Key_F:
            return VK_F;
        case KeyEvent::Key_G:
            return VK_G;
        case KeyEvent::Key_H:
            return VK_H;
        case KeyEvent::Key_I:
            return VK_I;
        case KeyEvent::Key_J:
            return VK_J;
        case KeyEvent::Key_K:
            return VK_K;
        case KeyEvent::Key_L:
            return VK_L;
        case KeyEvent::Key_M:
            return VK_M;
        case KeyEvent::Key_N:
            return VK_N;
        case KeyEvent::Key_O:
            return VK_O;
        case KeyEvent::Key_P:
            return VK_P;
        case KeyEvent::Key_Q:
            return VK_Q;
        case KeyEvent::Key_R:
            return VK_R;
        case KeyEvent::Key_S:
            return VK_S;
        case KeyEvent::Key_T:
            return VK_T;
        case KeyEvent::Key_U:
            return VK_U;
        case KeyEvent::Key_V:
            return VK_V;
        case KeyEvent::Key_W:
            return VK_W;
        case KeyEvent::Key_X:
            return VK_X;
        case KeyEvent::Key_Y:
            return VK_Y;
        case KeyEvent::Key_Z:
            return VK_Z;
        case KeyEvent::Key_Meta:
            return VK_LWIN;
        case KeyEvent::Key_NumLock:
            return VK_NUMLOCK;
        case KeyEvent::Key_ScrollLock:
            return VK_SCROLL;
        case KeyEvent::Key_Semicolon:
        case KeyEvent::Key_Colon:
            return VK_OEM_1;
        case KeyEvent::Key_Plus:
        case KeyEvent::Key_Equal:
            return VK_OEM_PLUS;
        case KeyEvent::Key_Comma:
        case KeyEvent::Key_Less:
            return VK_OEM_COMMA;
        case KeyEvent::Key_Minus:
        case KeyEvent::Key_Underscore:
            return VK_OEM_MINUS;
        case KeyEvent::Key_Period:
        case KeyEvent::Key_Greater:
            return VK_OEM_PERIOD;
        case KeyEvent::Key_Slash:
        case KeyEvent::Key_Question:
            return VK_OEM_2;
        case KeyEvent::Key_AsciiTilde:
        case KeyEvent::Key_QuoteLeft:
            return VK_OEM_3;
        case KeyEvent::Key_BracketLeft:
        case KeyEvent::Key_BraceLeft:
            return VK_OEM_4;
        case KeyEvent::Key_Backslash:
        case KeyEvent::Key_Bar:
            return VK_OEM_5;
        case KeyEvent::Key_BracketRight:
        case KeyEvent::Key_BraceRight:
            return VK_OEM_6;
        case KeyEvent::Key_QuoteDbl:
            return VK_OEM_7;
        default:
            return 0;
        }
    }
}

static String keyTextForNixKeyEvent(const KeyEvent& event)
{
    int keycode = static_cast<int>(event.key);
    if (isASCIIPrintable(keycode))
        return String::format("%c", event.shouldUseUpperCase ? toASCIIUpper(keycode) : toASCIILower(keycode));

    switch (event.key) {
    case KeyEvent::Key_Tab:
    case KeyEvent::Key_Backtab:
        return "\t";
    case KeyEvent::Key_Enter:
    case KeyEvent::Key_Return:
        return "\r";
    default:
        break;
    }

    return "";
}

void WebViewImpl::sendKeyEvent(const KeyEvent& event)
{
    WebEvent::Type type = convertToWebEventType(event.type);
    const WTF::String text = keyTextForNixKeyEvent(event);
    const WTF::String unmodifiedText = text;
    bool isAutoRepeat = false;
    bool isSystemKey = false;
    bool isKeypad = event.isKeypad;
    const WTF::String keyIdentifier = keyIdentifierForNixKeyCode(event.key);
    int windowsVirtualKeyCode = windowsKeyCodeForKeyEvent(event.key, isKeypad);
    int nativeVirtualKeyCode = 0;
    int macCharCode = 0;
    WebEvent::Modifiers modifiers = convertToWebEventModifiers(event.modifiers);
    double timestamp = event.timestamp;

    WebKeyboardEvent webEvent(type, text, unmodifiedText, keyIdentifier, windowsVirtualKeyCode, nativeVirtualKeyCode, macCharCode, isAutoRepeat, isKeypad, isSystemKey, modifiers, timestamp);
    m_webPageProxy->handleKeyboardEvent(NativeWebKeyboardEvent(webEvent));
}

} // namespace Nix
