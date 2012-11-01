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

    virtual void viewNeedsDisplay(WKRect area);
    virtual void webProcessCrashed(WKStringRef url);
    virtual void webProcessRelaunched();
    virtual void doneWithTouchEvent(const TouchEvent&, bool wasEventHandled);
    virtual void doneWithGestureEvent(const GestureEvent&, bool wasEventHandled);
    virtual void pageDidRequestScroll(WKPoint position);
    virtual void didChangeContentsSize(WKSize size);
    virtual void didFindZoomableArea(WKPoint target, WKRect area);
    virtual void updateTextInputState(bool isContentEditable, WKRect cursorRect, WKRect editorRect);
};

class WK_EXPORT WebView {
public:
    static WebView* create(WKContextRef, WKPageGroupRef, WebViewClient*);
    virtual ~WebView() { }

    virtual void initialize() = 0;

    virtual WKSize size() const = 0;
    virtual void setSize(const WKSize& size) = 0;

    virtual WKPoint scrollPosition() const = 0;
    virtual void setScrollPosition(const WKPoint& position) = 0;

    virtual void setUserViewportTransformation(const cairo_matrix_t& userViewportTransformation) = 0;
    virtual WKPoint userViewportToContents(WKPoint point) = 0;

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

    virtual void findZoomableAreaForPoint(const WKPoint& point, int horizontalRadius, int verticalRadius) = 0;

    virtual WKPageRef pageRef() = 0;

    virtual void sendEvent(const Nix::InputEvent&) = 0;
};

} // namespace Nix

#endif // WebView_h
