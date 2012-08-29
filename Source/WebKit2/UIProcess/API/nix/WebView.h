#ifndef WebView_h
#define WebView_h

#include <WebKit2/WKContext.h>
#include <WebKit2/WKPage.h>
#include <WebKit2/WKPageGroup.h>

namespace Nix {

class WebViewClient {
public:
    virtual ~WebViewClient() { }

    virtual void viewNeedsDisplay(int x, int y, int width, int height) = 0;
};

class WK_EXPORT WebView {
public:
    static WebView* create(WKContextRef, WKPageGroupRef, WebViewClient*);
    virtual ~WebView() { }

    virtual void initialize() = 0;

    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual void setSize(int width, int height) = 0;

    virtual bool isFocused() const = 0;
    virtual void setFocused(bool) = 0;

    virtual bool isVisible() const = 0;
    virtual void setVisible(bool) = 0;

    virtual bool isActive() const = 0;
    virtual void setActive(bool) = 0;

    virtual void paintToCurrentGLContext() = 0;

    virtual WKPageRef pageRef() = 0;

    // TODO: Create a proper datastructure to use as argument for events.
    virtual void sendKeyEvent(bool, char) = 0;
    virtual void sendMouseEvent(bool, int x, int y) = 0;
};

} // namespace Nix

#endif // WebView_h
