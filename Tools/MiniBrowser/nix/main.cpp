#include "GestureRecognizer.h"
#include "LinuxWindow.h"
#include "TouchMocker.h"
#include "XlibEventUtils.h"
#include <GL/gl.h>
#include <WebKit2/WKPreferences.h>
#include <WebKit2/WKPreferencesPrivate.h>
#include <WebKit2/WKString.h>
#include <WebKit2/WKURL.h>
#include <WebView.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <glib.h>
#include <string>
#include <fstream>

#include <wtf/Platform.h>
#include <WebKit2/WKRetainPtr.h>

extern "C" {
static gboolean callUpdateDisplay(gpointer);
}

class MiniBrowser : public Nix::WebViewClient, public LinuxWindowClient, public GestureRecognizerClient {
public:

    enum Mode {
        MobileMode,
        DesktopMode
    };

    MiniBrowser(GMainLoop* mainLoop, Mode mode, int width, int height, int viewportHorizontalDisplacement, int viewportVerticalDisplacement);
    virtual ~MiniBrowser();

    WKPageRef pageRef() const { return m_webView->pageRef(); }

    // LinuxWindowClient.
    virtual void handleExposeEvent() { scheduleUpdateDisplay(); }
    virtual void handleKeyPressEvent(const XKeyPressedEvent&);
    virtual void handleKeyReleaseEvent(const XKeyReleasedEvent&);
    virtual void handleButtonPressEvent(const XButtonPressedEvent&);
    virtual void handleButtonReleaseEvent(const XButtonReleasedEvent&);
    virtual void handlePointerMoveEvent(const XPointerMovedEvent&);
    virtual void handleSizeChanged(int, int);
    virtual void handleClosed();

    // Nix::WebViewClient.
    virtual void viewNeedsDisplay(WKRect) { scheduleUpdateDisplay(); }
    virtual void webProcessCrashed(WKStringRef url);
    virtual void webProcessRelaunched();
    virtual void pageDidRequestScroll(WKPoint position);
    virtual void didChangeContentsSize(WKSize size);
    virtual void didFindZoomableArea(WKPoint target, WKRect area);
    virtual void doneWithTouchEvent(const Nix::TouchEvent&, bool wasEventHandled);

    // GestureRecognizerClient.
    virtual void handleSingleTap(double timestamp, const Nix::TouchPoint&);
    virtual void handleDoubleTap(double timestamp, const Nix::TouchPoint&);
    virtual void handlePanning(double timestamp, WKPoint delta);
    virtual void handlePanningFinished(double timestamp);
    virtual void handlePinch(double timestamp, WKPoint delta, double scale, WKPoint contentCenter);
    virtual void handlePinchFinished(double timestamp);

    virtual double scale();

    void setTouchEmulationMode(bool enabled);
    Mode mode() const { return m_mode; }

private:

    enum ScaleBehavior {
        AdjustToBoundaries,
        IgnoreBoundaries
    };
    void handleWheelEvent(const XButtonPressedEvent&);
    void updateClickCount(const XButtonPressedEvent&);

    void updateDisplay();
    void scheduleUpdateDisplay();
    WKPoint adjustScrollPositionToBoundaries(WKPoint position);
    void adjustScrollPosition();
    double scaleToFitContents();

    void scaleAtPoint(const WKPoint& point, double scale, ScaleBehavior shouldAdjustScrollPosition = AdjustToBoundaries);

    Nix::WebView* webViewAtX11Position(const WKPoint& poisition);

    WKRetainPtr<WKContextRef> m_context;
    WKRetainPtr<WKPageGroupRef> m_pageGroup;
    LinuxWindow* m_window;
    Nix::WebView* m_webView;
    WKRect m_webViewRect;
    GMainLoop* m_mainLoop;
    double m_lastClickTime;
    int m_lastClickX;
    int m_lastClickY;
    Nix::MouseEvent::Button m_lastClickButton;
    unsigned m_clickCount;
    TouchMocker* m_touchMocker;
    Mode m_mode;
    bool m_displayUpdateScheduled;
    WKSize m_contentsSize;
    GestureRecognizer m_gestureRecognizer;

    friend gboolean callUpdateDisplay(gpointer);
};

MiniBrowser::MiniBrowser(GMainLoop* mainLoop, Mode mode, int width, int height, int viewportHorizontalDisplacement, int viewportVerticalDisplacement)
    : m_context(AdoptWK, WKContextCreate())
    , m_pageGroup(AdoptWK, (WKPageGroupCreateWithIdentifier(WKStringCreateWithUTF8CString("MiniBrowser"))))
    , m_window(new LinuxWindow(this, width, height))
    , m_webView(0)
    , m_mainLoop(mainLoop)
    , m_lastClickTime(0)
    , m_lastClickX(0)
    , m_lastClickY(0)
    , m_lastClickButton(Nix::MouseEvent::NoButton)
    , m_clickCount(0)
    , m_touchMocker(0)
    , m_mode(mode)
    , m_displayUpdateScheduled(false)
    , m_gestureRecognizer(GestureRecognizer(this))
{
    g_main_loop_ref(m_mainLoop);

    WKPreferencesRef preferences = WKPageGroupGetPreferences(m_pageGroup.get());
    WKPreferencesSetAcceleratedCompositingEnabled(preferences, true);
    WKPreferencesSetFrameFlatteningEnabled(preferences, true);
    WKPreferencesSetDeveloperExtrasEnabled(preferences, true);

    m_webView = Nix::WebView::create(m_context.get(), m_pageGroup.get(), this);
    m_webView->initialize();

    if (m_mode == MobileMode)
        WKPageSetUseFixedLayout(pageRef(), true);

    WKSize size = m_window->size();
    m_webViewRect = WKRectMake(viewportHorizontalDisplacement, viewportVerticalDisplacement, size.width - viewportHorizontalDisplacement, size.height - viewportVerticalDisplacement);
    m_webView->setSize(m_webViewRect.size);

    if (viewportHorizontalDisplacement || viewportVerticalDisplacement) {
        cairo_matrix_t userTransform;
        cairo_matrix_init_translate(&userTransform, viewportHorizontalDisplacement, viewportVerticalDisplacement);
        m_webView->setUserViewportTransformation(userTransform);
    }

    m_webView->setFocused(true);
    m_webView->setVisible(true);
    m_webView->setActive(true);
}

MiniBrowser::~MiniBrowser()
{
    g_main_loop_unref(m_mainLoop);

    delete m_webView;
    delete m_window;
    delete m_touchMocker;
}

void MiniBrowser::setTouchEmulationMode(bool enabled)
{
    if (enabled && !m_touchMocker) {
        m_touchMocker = new TouchMocker(m_webView);
    } else if (!enabled && m_touchMocker) {
        delete m_touchMocker;
        m_touchMocker = 0;
    }
}

enum NavigationCommand {
    NoNavigation,
    BackNavigation,
    ForwardNavigation
};

static NavigationCommand checkNavigationCommand(const KeySym keySym, const unsigned state)
{
    if (!(state & Mod1Mask))
        return NoNavigation;
    if (keySym == XK_Left)
        return BackNavigation;
    if (keySym == XK_Right)
        return ForwardNavigation;
    return NoNavigation;
}

static inline bool isKeypadKeysym(const KeySym symbol)
{
    // Following keypad symbols are specified on Xlib Programming Manual (section: Keyboard Encoding).
    return (symbol >= 0xFF80 && symbol <= 0xFFBD);
}

static KeySym chooseSymbolForXKeyEvent(const XKeyEvent& event, bool* useUpperCase)
{
    KeySym firstSymbol = XLookupKeysym(const_cast<XKeyEvent*>(&event), 0);
    KeySym secondSymbol = XLookupKeysym(const_cast<XKeyEvent*>(&event), 1);
    KeySym lowerCaseSymbol, upperCaseSymbol, chosenSymbol;
    XConvertCase(firstSymbol, &lowerCaseSymbol, &upperCaseSymbol);
    bool numLockModifier = event.state & Mod2Mask;
    bool capsLockModifier = event.state & LockMask;
    bool shiftModifier = event.state & ShiftMask;
    if (numLockModifier && isKeypadKeysym(secondSymbol)) {
        chosenSymbol = shiftModifier ? firstSymbol : secondSymbol;
    } else if (lowerCaseSymbol == upperCaseSymbol) {
        chosenSymbol = shiftModifier ? secondSymbol : firstSymbol;
    } else if (shiftModifier == capsLockModifier)
        chosenSymbol = firstSymbol;
    else
        chosenSymbol = secondSymbol;

    *useUpperCase = (lowerCaseSymbol != upperCaseSymbol && chosenSymbol == upperCaseSymbol);
    XConvertCase(chosenSymbol, &lowerCaseSymbol, &upperCaseSymbol);
    return upperCaseSymbol;
}

static Nix::KeyEvent convertXKeyEventToNixKeyEvent(const XKeyEvent& event, const KeySym& symbol, bool useUpperCase)
{
    Nix::KeyEvent nixEvent;
    nixEvent.type = (event.type == KeyPress) ? Nix::InputEvent::KeyDown : Nix::InputEvent::KeyUp;
    nixEvent.modifiers = convertXEventModifiersToNativeModifiers(event.state);
    nixEvent.timestamp = convertXEventTimeToNixTimestamp(event.time);
    nixEvent.shouldUseUpperCase = useUpperCase;
    nixEvent.isKeypad = isKeypadKeysym(symbol);
    nixEvent.key = convertXKeySymToNativeKeycode(symbol);
    return nixEvent;
}

static Nix::MouseEvent convertXButtonEventToNixButtonEvent(Nix::WebView* webView, const XButtonEvent& event, Nix::InputEvent::Type type, unsigned clickCount)
{
    Nix::MouseEvent nixEvent;
    nixEvent.type = type;
    nixEvent.button = convertXEventButtonToNativeMouseButton(event.button);
    WKPoint contentsPoint = webView->userViewportToContents(WKPointMake(event.x, event.y));
    nixEvent.x = contentsPoint.x;
    nixEvent.y = contentsPoint.y;
    nixEvent.globalX = event.x_root;
    nixEvent.globalY = event.y_root;
    nixEvent.clickCount = clickCount;
    nixEvent.modifiers = convertXEventModifiersToNativeModifiers(event.state);
    nixEvent.timestamp = convertXEventTimeToNixTimestamp(event.time);
    return nixEvent;
}

void MiniBrowser::handleKeyPressEvent(const XKeyPressedEvent& event)
{
    if (!m_webView)
        return;

    bool shouldUseUpperCase;
    KeySym symbol = chooseSymbolForXKeyEvent(event, &shouldUseUpperCase);
    NavigationCommand command = checkNavigationCommand(symbol, event.state);
    switch (command) {
    case BackNavigation:
        WKPageGoBack(pageRef());
        return;
    case ForwardNavigation:
        WKPageGoForward(pageRef());
        return;
    default:
        Nix::KeyEvent nixEvent = convertXKeyEventToNixKeyEvent(event, symbol, shouldUseUpperCase);
        m_webView->sendEvent(nixEvent);
    }
}

void MiniBrowser::handleKeyReleaseEvent(const XKeyReleasedEvent& event)
{
    if (!m_webView)
        return;

    bool shouldUseUpperCase;
    KeySym symbol = chooseSymbolForXKeyEvent(event, &shouldUseUpperCase);
    if (checkNavigationCommand(symbol, event.state) != NoNavigation)
        return;
    Nix::KeyEvent nixEvent = convertXKeyEventToNixKeyEvent(event, symbol, shouldUseUpperCase);
    if (m_touchMocker && m_touchMocker->handleKeyRelease(nixEvent)) {
        scheduleUpdateDisplay();
        return;
    }
    m_webView->sendEvent(nixEvent);
}

void MiniBrowser::handleWheelEvent(const XButtonPressedEvent& event)
{
    WKPoint contentsPoint = m_webView->userViewportToContents(WKPointMake(event.x, event.y));

    if (m_mode == MobileMode && event.state & ShiftMask) {
        double newScale = m_webView->scale() * (event.button == 4 ? 1.1 : 0.9);
        scaleAtPoint(contentsPoint, newScale);
        return;
    }

    // Same constant we use inside WebView to calculate the ticks. See also WebCore::Scrollbar::pixelsPerLineStep().
    const float pixelsPerStep = 40.0f;

    Nix::WheelEvent nixEvent;
    nixEvent.type = Nix::InputEvent::Wheel;
    nixEvent.modifiers = convertXEventModifiersToNativeModifiers(event.state);
    nixEvent.timestamp = convertXEventTimeToNixTimestamp(event.time);
    nixEvent.x = contentsPoint.x;
    nixEvent.y = contentsPoint.y;
    nixEvent.globalX = event.x_root;
    nixEvent.globalY = event.y_root;
    nixEvent.delta = pixelsPerStep * (event.button == 4 ? 1 : -1);
    nixEvent.orientation = event.state & Mod1Mask ? Nix::WheelEvent::Horizontal : Nix::WheelEvent::Vertical;
    m_webView->sendEvent(nixEvent);
}

static const double doubleClickInterval = 300;

void MiniBrowser::updateClickCount(const XButtonPressedEvent& event)
{
    if (m_lastClickX != event.x
        || m_lastClickY != event.y
        || m_lastClickButton != event.button
        || event.time - m_lastClickTime >= doubleClickInterval)
        m_clickCount = 1;
    else
        ++m_clickCount;

    m_lastClickX = event.x;
    m_lastClickY = event.y;
    m_lastClickButton = convertXEventButtonToNativeMouseButton(event.button);
    m_lastClickTime = event.time;
}

void MiniBrowser::handleButtonPressEvent(const XButtonPressedEvent& event)
{
    Nix::WebView* webView = webViewAtX11Position(WKPointMake(event.x, event.y));
    if (!webView)
        return;

    if (event.button == 4 || event.button == 5) {
        handleWheelEvent(event);
        return;
    }

    updateClickCount(event);

    Nix::MouseEvent nixEvent = convertXButtonEventToNixButtonEvent(webView, event, Nix::InputEvent::MouseDown, m_clickCount);
    if (m_touchMocker && m_touchMocker->handleMousePress(nixEvent, WKPointMake(event.x, event.y))) {
        scheduleUpdateDisplay();
        return;
    }
    webView->sendEvent(nixEvent);
}

void MiniBrowser::handleButtonReleaseEvent(const XButtonReleasedEvent& event)
{
    if (event.button == 4 || event.button == 5)
        return;

    Nix::MouseEvent nixEvent = convertXButtonEventToNixButtonEvent(m_webView, event, Nix::InputEvent::MouseUp, 0);
    if (m_touchMocker && m_touchMocker->handleMouseRelease(nixEvent)) {
        scheduleUpdateDisplay();
        return;
    }

    // The mouse release event was allowed to be sent to the TouchMocker because it
    // may be tracking a button press that happened in a valid position.
    Nix::WebView* webView = webViewAtX11Position(WKPointMake(event.x, event.y));
    if (!webView)
        return;
    webView->sendEvent(nixEvent);
}

void MiniBrowser::handlePointerMoveEvent(const XPointerMovedEvent& event)
{
    Nix::MouseEvent nixEvent;
    nixEvent.type = Nix::InputEvent::MouseMove;
    nixEvent.button = Nix::MouseEvent::NoButton;
    WKPoint contentsPoint = m_webView->userViewportToContents(WKPointMake(event.x, event.y));
    nixEvent.x = contentsPoint.x;
    nixEvent.y = contentsPoint.y;
    nixEvent.globalX = event.x_root;
    nixEvent.globalY = event.y_root;
    nixEvent.clickCount = 0;
    nixEvent.modifiers = convertXEventModifiersToNativeModifiers(event.state);
    nixEvent.timestamp = convertXEventTimeToNixTimestamp(event.time);
    if (m_touchMocker && m_touchMocker->handleMouseMove(nixEvent, WKPointMake(event.x, event.y))) {
        scheduleUpdateDisplay();
        return;
    }

    // The mouse move event was allowed to be sent to the TouchMocker because it
    // may be tracking a button press that happened in a valid position.
    Nix::WebView* webView = webViewAtX11Position(WKPointMake(event.x, event.y));
    if (!webView)
        return;
    webView->sendEvent(nixEvent);
}

void MiniBrowser::handleSizeChanged(int width, int height)
{
    if (!m_webView)
        return;

    m_webViewRect.size.width = width - m_webViewRect.origin.x;
    m_webViewRect.size.height = height - m_webViewRect.origin.y;
    m_webView->setSize(m_webViewRect.size);

    if (m_mode == MobileMode)
        m_webView->setScale(scaleToFitContents());
}

void MiniBrowser::handleClosed()
{
    g_main_loop_quit(m_mainLoop);
}

void MiniBrowser::updateDisplay()
{
    if (!m_webView || !m_window)
        return;

    m_window->makeCurrent();

    WKSize size = m_window->size();
    glViewport(0, 0, size.width, size.height);
    glClearColor(0.7, 0.35, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_webView->paintToCurrentGLContext();
    if (m_touchMocker)
        m_touchMocker->paintTouchPoints(size);

    m_window->swapBuffers();
}

static gboolean callUpdateDisplay(gpointer data)
{
    MiniBrowser* browser = reinterpret_cast<MiniBrowser*>(data);

    assert(browser->m_displayUpdateScheduled);
    browser->m_displayUpdateScheduled = false;
    browser->updateDisplay();
    return 0;
}

void MiniBrowser::scheduleUpdateDisplay()
{
    if (m_displayUpdateScheduled)
        return;

    m_displayUpdateScheduled = true;
    g_timeout_add(0, callUpdateDisplay, this);
}

WKPoint MiniBrowser::adjustScrollPositionToBoundaries(WKPoint position)
{
    int rightBoundary = m_contentsSize.width - m_webView->visibleContentWidth();
    // Contents height may be shorter than the scaled viewport height.
    int bottomBoundary = m_contentsSize.height < m_webView->visibleContentHeight() ? 0 : m_contentsSize.height - m_webView->visibleContentHeight();

    if (position.x < 0)
        position.x = 0;
    else if (position.x > rightBoundary)
        position.x = rightBoundary;
    if (position.y < 0)
        position.y = 0;
    else if (position.y > bottomBoundary)
        position.y = bottomBoundary;

    return position;
}

double MiniBrowser::scaleToFitContents()
{
    return m_webView->size().width / m_contentsSize.width;
}

void MiniBrowser::adjustScrollPosition()
{
    WKPoint position = adjustScrollPositionToBoundaries(m_webView->scrollPosition());
    if (position == m_webView->scrollPosition())
        return;
    m_webView->setScrollPosition(position);
}

void MiniBrowser::webProcessCrashed(WKStringRef url)
{
    size_t urlStringSize =  WKStringGetMaximumUTF8CStringSize(url);
    char* urlString = new char[urlStringSize];
    WKStringGetUTF8CString(url, urlString, urlStringSize);
    fprintf(stderr, "The web process has crashed on '%s'.\n", urlString);
    delete urlString;
}

void MiniBrowser::webProcessRelaunched()
{
    fprintf(stdout, "The web process has been restarted.\n");
}

void MiniBrowser::pageDidRequestScroll(WKPoint position)
{
    m_webView->setScrollPosition(adjustScrollPositionToBoundaries(position));
}

void MiniBrowser::didChangeContentsSize(WKSize size)
{
    m_contentsSize = size;

    if (m_mode == MobileMode) {
        m_webView->setScale(scaleToFitContents());
        adjustScrollPosition();
    }
}

void MiniBrowser::didFindZoomableArea(WKPoint target, WKRect area)
{
    // Zoomable area width is the same as web page width, and this is fully visible.
    if (m_contentsSize.width == area.size.width && m_contentsSize.width == m_webView->visibleContentWidth())
        return;

    // The zoomed area will look nicer with a horizontal margin.
    double margin = 8.0;
    double scale = m_webView->size().width / (area.size.width + margin * 2.0);

    // Trying to zoom to an area with the same scale factor causes a zoom out.
    if (scale == m_webView->scale())
        scale = scaleToFitContents();
    else {
        // We want the zoomed content area to fit horizontally in the WebView,
        // so let's give the scaleAtPoint method a suitable value.
        target.x = area.origin.x - margin;
        m_webView->setScrollPosition(WKPointMake(target.x, m_webView->scrollPosition().y));
    }

    scaleAtPoint(target, scale);
}

void MiniBrowser::doneWithTouchEvent(const Nix::TouchEvent& touchEvent, bool wasEventHandled)
{
    if (wasEventHandled)
        return;
    m_gestureRecognizer.handleTouchEvent(touchEvent);
}

double MiniBrowser::scale()
{
    return m_webView->scale();
}

void MiniBrowser::handleSingleTap(double timestamp, const Nix::TouchPoint& touchPoint)
{
    Nix::GestureEvent gestureEvent;
    gestureEvent.type = Nix::InputEvent::GestureSingleTap;
    gestureEvent.timestamp = timestamp;
    gestureEvent.modifiers = 0;
    gestureEvent.x = touchPoint.x;
    gestureEvent.y = touchPoint.y;
    gestureEvent.globalX = touchPoint.globalX;
    gestureEvent.globalY = touchPoint.globalY;
    gestureEvent.width = 20;
    gestureEvent.height = 20;
    gestureEvent.deltaX = 0.0;
    gestureEvent.deltaY = 0.0;

    m_webView->sendEvent(gestureEvent);
}

void MiniBrowser::handleDoubleTap(double timestamp, const Nix::TouchPoint& touchPoint)
{
    WKPoint contentsPoint = WKPointMake(touchPoint.x, touchPoint.y);
    m_webView->findZoomableAreaForPoint(contentsPoint, touchPoint.verticalRadius, touchPoint.horizontalRadius);
}

void MiniBrowser::handlePanning(double timestamp, WKPoint delta)
{
    // When the user is panning around the contents we don't force the page scroll position
    // to respect any boundaries other than the physical constraints of the device from where
    // the user input came. This will be adjusted after the user interaction ends.
    WKPoint position = m_webView->scrollPosition();
    position.x -= delta.x;
    position.y -= delta.y;
    m_webView->setScrollPosition(position);
}

void MiniBrowser::handlePanningFinished(double timestamp)
{
    adjustScrollPosition();
}

void MiniBrowser::handlePinch(double timestamp, WKPoint delta, double scale, WKPoint contentCenter)
{
    // Scaling: The page should be scaled proportionally to the distance of the pinch.
    // Scrolling: If the center of the pinch initially was position (120,120) in content
    //            coordinates, them during the page must be scrolled to keep the pinch center
    //            at the same coordinates.
    WKPoint position = WKPointMake(m_webView->scrollPosition().x - delta.x, m_webView->scrollPosition().y - delta.y);

    m_webView->setScrollPosition(position);
    scaleAtPoint(contentCenter, scale, IgnoreBoundaries);
}

void MiniBrowser::handlePinchFinished(double timestamp)
{
    adjustScrollPosition();
}

void MiniBrowser::scaleAtPoint(const WKPoint& point, double scale, ScaleBehavior shouldAdjustScrollPosition)
{
    double minimumScale = double(m_webView->size().width) / m_contentsSize.width;
    if (scale < minimumScale)
        scale = minimumScale;

    // Calculate new scroll points that will keep the content
    // approximately at the same visual point.
    double scaleRatio = m_webView->scale() / scale;
    WKPoint position = WKPointMake(point.x - (point.x - m_webView->scrollPosition().x) * scaleRatio,
                                   point.y - (point.y - m_webView->scrollPosition().y) * scaleRatio);

    m_webView->setScale(scale);
    if (shouldAdjustScrollPosition == AdjustToBoundaries)
        position = adjustScrollPositionToBoundaries(position);
    m_webView->setScrollPosition(position);
}

static inline bool areaContainsPoint(const WKRect& area, const WKPoint& point)
{
    return !(point.x < area.origin.x || point.y < area.origin.y || point.x >= (area.origin.x + area.size.width) || point.y >= (area.origin.y + area.size.height));
}

Nix::WebView* MiniBrowser::webViewAtX11Position(const WKPoint& position)
{
    if (areaContainsPoint(m_webViewRect, position))
        return m_webView;
    return 0;
}

int main(int argc, char* argv[])
{
    printf("MiniBrowser: Use Alt + Left and Alt + Right to navigate back and forward.\n");

    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    int viewportHorizontalDisplacement = 0;
    int viewportVerticalDisplacement = 0;
    std::string url;
    const char* userAgent = 0;
    MiniBrowser::Mode browserMode = MiniBrowser::MobileMode;
    bool touchEmulationEnabled = false;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--desktop"))
            browserMode = MiniBrowser::DesktopMode;
        else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--touch-emulation"))
            touchEmulationEnabled = true;
        else if (!strcmp(argv[i], "--window-size")) {
            if (i + 1 == argc) {
                fprintf(stderr, "--window-size requires an argument.\n");
                return 1;
            }
            if (sscanf(argv[++i], "%dx%d", &width, &height) != 2) {
                fprintf(stderr, "--window-size format is WIDTHxHEIGHT.\n");
                return 1;
            }
        } else if (!strcmp(argv[i], "--viewport-displacement")) {
            if (i + 1 == argc) {
                fprintf(stderr, "--viewport-displacement requires an argument.\n");
                return 1;
            }
            if (sscanf(argv[++i], "%dx%d", &viewportHorizontalDisplacement, &viewportVerticalDisplacement) != 2) {
                fprintf(stderr, "--viewport-displacement format is HORIZDISPLACEMENTxVERTDISPLACEMENT.\n");
                return 1;
            }
        } else if (!strcmp(argv[i], "--n9")) {
            userAgent = "Mozilla/5.0 (MeeGo; NokiaN9) AppleWebKit/534.13 (KHTML, like Gecko) NokiaBrowser/8.5.0 Mobile Safari/534.13";
        } else if (!strcmp(argv[i], "--ipad")) {
            userAgent = "Mozilla/5.0 (iPad; CPU OS 5_0 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9A334 Safari/7534.48.3";
        } else if (!strcmp(argv[i], "--iphone")) {
            userAgent = "Mozilla/5.0 (iPhone; CPU iPhone OS 5_0 like Mac OS X) AppleWebKit/534.46 (KHTML, like Gecko) Version/5.1 Mobile/9A334 Safari/7534.48.3";
        } else if (!strcmp(argv[i], "--android")) {
            userAgent = "Mozilla/5.0 (Linux; U; Android 4.0.2; en-us; Galaxy Nexus Build/ICL53F) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30";
        } else
            url = argv[i];
    }

    if (url.empty()) {
        url = "http://www.google.com";
    } else if (url.find("http") != 0 && url.find("file://") != 0) {
        std::ifstream localFile(url.c_str());
        url.insert(0, localFile ? "file://" : "http://");
    }

    GMainLoop* mainLoop = g_main_loop_new(0, false);
    MiniBrowser browser(mainLoop, browserMode, width, height, viewportHorizontalDisplacement, viewportVerticalDisplacement);

    if (browser.mode() == MiniBrowser::MobileMode || touchEmulationEnabled) {
        printf("Touch Emulation Mode toggled. Hold Control key to build and emit a multi-touch event: each mouse button should be a different touch point. Release Control Key to clear all tracking pressed touches.\n");
        browser.setTouchEmulationMode(true);
    }

    if (userAgent)
        WKPageSetCustomUserAgent(browser.pageRef(), WKStringCreateWithUTF8CString(userAgent));

    if (browser.mode() == MiniBrowser::MobileMode)
        printf("Use Shift + mouse wheel to zoom in and out.\n");

    WKPageLoadURL(browser.pageRef(), WKURLCreateWithUTF8CString(url.c_str()));

    g_main_loop_run(mainLoop);
    g_main_loop_unref(mainLoop);
}
