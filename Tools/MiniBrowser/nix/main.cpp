#include "LinuxWindow.h"
#include <WebKit2/WKPreferences.h>
#include <WebKit2/WKPreferencesPrivate.h>
#include <WebKit2/WKString.h>
#include <WebKit2/WKURL.h>
#include <WebView.h>
#include <cstdio>
#include <glib.h>

#include <wtf/Platform.h>
#include <WebKit2/WKRetainPtr.h>

class MiniBrowser : public Nix::WebViewClient, public LinuxWindowClient {
public:
    MiniBrowser(GMainLoop*);
    virtual ~MiniBrowser();

    WKPageRef pageRef() const { return m_webView->pageRef(); }

    // LinuxWindowClient.
    virtual void handleExposeEvent() { updateDisplay(); }
    virtual void handleKeyPressEvent(const XKeyPressedEvent&);
    virtual void handleKeyReleaseEvent(const XKeyReleasedEvent&);
    virtual void handleButtonPressEvent(const XButtonPressedEvent&);
    virtual void handleButtonReleaseEvent(const XButtonReleasedEvent&);
    virtual void handleSizeChanged(int, int);
    virtual void handleClosed();

    // Nix::WebViewClient.
    virtual void viewNeedsDisplay(int, int, int, int) { updateDisplay(); }

private:
    void updateDisplay();

    WKRetainPtr<WKContextRef> m_context;
    WKRetainPtr<WKPageGroupRef> m_pageGroup;
    LinuxWindow* m_window;
    Nix::WebView* m_webView;
    GMainLoop* m_mainLoop;
};

MiniBrowser::MiniBrowser(GMainLoop* mainLoop)
    : m_context(AdoptWK, WKContextCreate())
    , m_pageGroup(AdoptWK, (WKPageGroupCreateWithIdentifier(WKStringCreateWithUTF8CString(""))))
    , m_window(new LinuxWindow(this))
    , m_webView(0)
    , m_mainLoop(mainLoop)
{
    WKPreferencesRef preferences = WKPageGroupGetPreferences(m_pageGroup.get());
    WKPreferencesSetAcceleratedCompositingEnabled(preferences, true);
    WKPreferencesSetFrameFlatteningEnabled(preferences, true);

    m_webView = Nix::WebView::create(m_context.get(), m_pageGroup.get(), this);
    m_webView->initialize();

    std::pair<int, int> size = m_window->size();
    m_webView->setSize(size.first, size.second);

    m_webView->setFocused(true);
    m_webView->setVisible(true);
    m_webView->setActive(true);
}

MiniBrowser::~MiniBrowser()
{
    delete m_webView;
    delete m_window;
}

void MiniBrowser::handleKeyPressEvent(const XKeyPressedEvent& event)
{
    if (!m_webView)
        return;

    printf("key = %s\n", XKeysymToString(XKeycodeToKeysym(m_window->display(), event.keycode, 0)));
    char key = XKeysymToString(XKeycodeToKeysym(m_window->display(), event.keycode, 0))[0];
    m_webView->sendKeyEvent(true, key);
}

void MiniBrowser::handleKeyReleaseEvent(const XKeyReleasedEvent& event)
{
    if (!m_webView)
        return;

    printf("key = %s\n", XKeysymToString(XKeycodeToKeysym(m_window->display(), event.keycode, 0)));
    char key = XKeysymToString(XKeycodeToKeysym(m_window->display(), event.keycode, 0))[0];
    m_webView->sendKeyEvent(false, key);
}

void MiniBrowser::handleButtonPressEvent(const XButtonPressedEvent& event)
{
    if (!m_webView)
        return;
    m_webView->sendMouseEvent(true, event.x, event.y);
}

void MiniBrowser::handleButtonReleaseEvent(const XButtonReleasedEvent& event)
{
    if (!m_webView)
        return;
    m_webView->sendMouseEvent(false, event.x, event.y);
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

    m_window->swapBuffers();
}

int main(int argc, char* argv[])
{
    GMainLoop* mainLoop = g_main_loop_new(0, false);

    MiniBrowser* browser = new MiniBrowser(mainLoop);

    // TODO: Couldn't make it show anything without fixed layout, maybe a sizing related problem.
    WKPageSetUseFixedLayout(browser->pageRef(), true);
    const char* url = argc > 1 ? argv[1] : "http://www.google.com";
    WKPageLoadURL(browser->pageRef(), WKURLCreateWithUTF8CString(url));

    g_main_loop_run(mainLoop);

    delete browser;
    g_main_loop_unref(mainLoop);
}
