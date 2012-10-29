#ifndef WebView_h
#define WebView_h

#include "NixEvents.h"
#include <WebKit2/WKContext.h>
#include <WebKit2/WKPage.h>
#include <WebKit2/WKPageGroup.h>
#include <cairo.h>

namespace Nix {

class WebViewClient {
public:
    virtual ~WebViewClient() { }

    virtual void viewNeedsDisplay(int x, int y, int width, int height);
    virtual void webProcessCrashed(WKStringRef url);
    virtual void webProcessRelaunched();
    virtual void doneWithTouchEvent(const TouchEvent&, bool wasEventHandled);
    virtual void doneWithGestureEvent(const GestureEvent&, bool wasEventHandled);
    virtual void pageDidRequestScroll(int x, int y);
    virtual void didChangeContentsSize(int width, int height);
    virtual void didFindZoomableArea(WKPoint target, WKRect area);
};

class WK_EXPORT WebView {
public:
    static WebView* create(WKContextRef, WKPageGroupRef, WebViewClient*);
    virtual ~WebView() { }

    virtual void initialize() = 0;

    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual void setSize(int width, int height) = 0;

    virtual double scrollX() const = 0;
    virtual double scrollY() const = 0;
    virtual void setScrollPosition(double x, double y) = 0;

    virtual void setUserViewportTransformation(const cairo_matrix_t& userViewportTransformation) = 0;
    virtual void userViewportToContents(int* x, int* y) = 0;

    virtual bool isFocused() const = 0;
    virtual void setFocused(bool) = 0;

    virtual bool isVisible() const = 0;
    virtual void setVisible(bool) = 0;

    virtual bool isActive() const = 0;
    virtual void setActive(bool) = 0;

    virtual void setTransparentBackground(bool) = 0;
    virtual bool transparentBackground() const = 0;

    virtual void setDrawBackground(bool) = 0;
    virtual bool drawBackground() const = 0;

    virtual void setScale(double) = 0;
    virtual double scale() const  = 0;

    virtual int visibleContentWidth() const = 0;
    virtual int visibleContentHeight() const = 0;

    virtual void setOpacity(double) = 0;
    virtual double opacity() const = 0;

    virtual void paintToCurrentGLContext() = 0;

    virtual void findZoomableAreaForPoint(int x, int y, int horizontalRadius, int verticalRadius) = 0;

    virtual WKPageRef pageRef() = 0;

    virtual void sendEvent(const Nix::InputEvent&) = 0;
};

} // namespace Nix

#endif // WebView_h
