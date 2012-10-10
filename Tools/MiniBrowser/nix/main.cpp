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
#include <cstdio>
#include <cstring>
#include <glib.h>
#include <string>
#include <fstream>

#include <wtf/Platform.h>
#include <WebKit2/WKRetainPtr.h>

class MiniBrowser : public Nix::WebViewClient, public LinuxWindowClient {
public:

    enum Mode {
        MobileMode,
        DesktopMode
    };

    MiniBrowser(GMainLoop* mainLoop, Mode mode);
    virtual ~MiniBrowser();

    WKPageRef pageRef() const { return m_webView->pageRef(); }

    // LinuxWindowClient.
    virtual void handleExposeEvent() { updateDisplay(); }
    virtual void handleKeyPressEvent(const XKeyPressedEvent&);
    virtual void handleKeyReleaseEvent(const XKeyReleasedEvent&);
    virtual void handleButtonPressEvent(const XButtonPressedEvent&);
    virtual void handleButtonReleaseEvent(const XButtonReleasedEvent&);
    virtual void handlePointerMoveEvent(const XPointerMovedEvent&);
    virtual void handleSizeChanged(int, int);
    virtual void handleClosed();

    // Nix::WebViewClient.
    virtual void viewNeedsDisplay(int, int, int, int) { updateDisplay(); }
    virtual void webProcessCrashed(WKStringRef url);
    virtual void webProcessRelaunched();
    virtual void pageDidRequestScroll(int x, int y) { m_webView->setScrollPosition(x, y); }

    void setTouchEmulationMode(bool enabled);
    Mode mode() const { return m_mode; }

private:
    void handleWheelEvent(const XButtonPressedEvent&);
    void updateClickCount(const XButtonPressedEvent&);

    void updateDisplay();

    WKRetainPtr<WKContextRef> m_context;
    WKRetainPtr<WKPageGroupRef> m_pageGroup;
    LinuxWindow* m_window;
    Nix::WebView* m_webView;
    GMainLoop* m_mainLoop;
    double m_lastClickTime;
    int m_lastClickX;
    int m_lastClickY;
    Nix::MouseEvent::Button m_lastClickButton;
    unsigned m_clickCount;
    TouchMocker* m_touchMocker;
    Mode m_mode;
};

MiniBrowser::MiniBrowser(GMainLoop* mainLoop, Mode mode)
    : m_context(AdoptWK, WKContextCreate())
    , m_pageGroup(AdoptWK, (WKPageGroupCreateWithIdentifier(WKStringCreateWithUTF8CString(""))))
    , m_window(new LinuxWindow(this))
    , m_webView(0)
    , m_mainLoop(mainLoop)
    , m_lastClickTime(0)
    , m_lastClickX(0)
    , m_lastClickY(0)
    , m_lastClickButton(Nix::MouseEvent::NoButton)
    , m_clickCount(0)
    , m_touchMocker(0)
    , m_mode(mode)
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

    std::pair<int, int> size = m_window->size();
    m_webView->setSize(size.first, size.second);

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
    Nix::KeyEvent ev;
    ev.type = (event.type == KeyPress) ? Nix::InputEvent::KeyDown : Nix::InputEvent::KeyUp;
    ev.modifiers = convertXEventModifiersToNativeModifiers(event.state);
    ev.timestamp = convertXEventTimeToNixTimestamp(event.time);
    ev.shouldUseUpperCase = useUpperCase;
    ev.isKeypad = isKeypadKeysym(symbol);
    ev.key = convertXKeySymToNativeKeycode(symbol);
    return ev;
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
        Nix::KeyEvent ev = convertXKeyEventToNixKeyEvent(event, symbol, shouldUseUpperCase);
        m_webView->sendEvent(ev);
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
    Nix::KeyEvent ev = convertXKeyEventToNixKeyEvent(event, symbol, shouldUseUpperCase);
    if (m_touchMocker && m_touchMocker->handleKeyRelease(ev)) {
        if (m_touchMocker->needsRepaint())
            updateDisplay();
        return;
    }
    m_webView->sendEvent(ev);
}

void MiniBrowser::handleWheelEvent(const XButtonPressedEvent& event)
{
    // Same constant we use inside WebView to calculate the ticks. See also WebCore::Scrollbar::pixelsPerLineStep().
    const float pixelsPerStep = 40.0f;

    Nix::WheelEvent ev;
    ev.type = Nix::InputEvent::Wheel;
    ev.modifiers = convertXEventModifiersToNativeModifiers(event.state);
    ev.timestamp = convertXEventTimeToNixTimestamp(event.time);
    ev.x = event.x;
    ev.y = event.y;
    ev.globalX = event.x_root;
    ev.globalY = event.y_root;
    ev.delta = pixelsPerStep * (event.button == 4 ? 1 : -1);
    ev.orientation = event.state & Mod1Mask ? Nix::WheelEvent::Horizontal : Nix::WheelEvent::Vertical;
    m_webView->sendEvent(ev);
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
    if (!m_webView)
        return;

    if (event.button == 4 || event.button == 5) {
        handleWheelEvent(event);
        return;
    }

    updateClickCount(event);

    Nix::MouseEvent ev;
    ev.type = Nix::InputEvent::MouseDown;
    ev.button = convertXEventButtonToNativeMouseButton(event.button);
    ev.x = event.x;
    ev.y = event.y;
    ev.globalX = event.x_root;
    ev.globalY = event.y_root;
    ev.clickCount = m_clickCount;
    ev.modifiers = convertXEventModifiersToNativeModifiers(event.state);
    ev.timestamp = convertXEventTimeToNixTimestamp(event.time);
    if (m_touchMocker && m_touchMocker->handleMousePress(ev)) {
        if (m_touchMocker->needsRepaint())
            updateDisplay();
        return;
    }
    m_webView->sendEvent(ev);
}

void MiniBrowser::handleButtonReleaseEvent(const XButtonReleasedEvent& event)
{
    if (!m_webView || event.button == 4 || event.button == 5)
        return;

    Nix::MouseEvent ev;
    ev.type = Nix::InputEvent::MouseUp;
    ev.button = convertXEventButtonToNativeMouseButton(event.button);
    ev.x = event.x;
    ev.y = event.y;
    ev.globalX = event.x_root;
    ev.globalY = event.y_root;
    ev.clickCount = 0;
    ev.modifiers = convertXEventModifiersToNativeModifiers(event.state);
    ev.timestamp = convertXEventModifiersToNativeModifiers(event.state);
    if (m_touchMocker && m_touchMocker->handleMouseRelease(ev)) {
        if (m_touchMocker->needsRepaint())
            updateDisplay();
        return;
    }
    m_webView->sendEvent(ev);
}

void MiniBrowser::handlePointerMoveEvent(const XPointerMovedEvent& event)
{
    if (!m_webView)
        return;

    Nix::MouseEvent ev;
    ev.type = Nix::InputEvent::MouseMove;
    ev.button = Nix::MouseEvent::NoButton;
    ev.x = event.x;
    ev.y = event.y;
    ev.globalX = event.x_root;
    ev.globalY = event.y_root;
    ev.clickCount = 0;
    ev.modifiers = convertXEventModifiersToNativeModifiers(event.state);
    ev.timestamp = convertXEventTimeToNixTimestamp(event.time);
    if (m_touchMocker && m_touchMocker->handleMouseMove(ev)) {
        if (m_touchMocker->needsRepaint())
            updateDisplay();
        return;
    }
    m_webView->sendEvent(ev);
}

void MiniBrowser::handleSizeChanged(int width, int height)
{
    if (!m_webView)
        return;
    printf("size changed to %dx%d\n", width, height);
    m_webView->setSize(width, height);
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

    std::pair<int, int> size = m_window->size();
    glViewport(0, 0, size.first, size.second);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_webView->paintToCurrentGLContext();
    if (m_touchMocker)
        m_touchMocker->paintTouchPoints();

    m_window->swapBuffers();
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

int main(int argc, char* argv[])
{
    printf("MiniBrowser: Use Alt + Left and Alt + Right to navigate back and forward.\n");

    std::string url;
    MiniBrowser::Mode browserMode = MiniBrowser::MobileMode;
    bool touchEmulationEnabled = false;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--desktop"))
            browserMode = MiniBrowser::DesktopMode;
        else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--touch-emulation"))
            touchEmulationEnabled = true;
        else
            url = argv[i];
    }

    if (url.empty()) {
        url = "http://www.google.com";
    } else if (url.find("http") != 0 && url.find("file://") != 0) {
        std::ifstream localFile(url.c_str());
        url.insert(0, localFile ? "file://" : "http://");
    }

    GMainLoop* mainLoop = g_main_loop_new(0, false);
    MiniBrowser browser(mainLoop, browserMode);

    if (browser.mode() == MiniBrowser::MobileMode || touchEmulationEnabled) {
        printf("Touch Emulation Mode toggled. Hold Control key to build and emit a multi-touch event: each mouse button should be a different touch point. Release Control Key to clear all tracking pressed touches.\n");
        browser.setTouchEmulationMode(true);
    }

    WKPageLoadURL(browser.pageRef(), WKURLCreateWithUTF8CString(url.c_str()));

    g_main_loop_run(mainLoop);
    g_main_loop_unref(mainLoop);
}
