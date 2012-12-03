#ifndef WebView_h
#define WebView_h

#include "NixEvents.h"
#include <WebKit2/WKContext.h>
#include <WebKit2/WKPage.h>
#include <WebKit2/WKPageGroup.h>

// TODO: Remove this include together with the C++ API.
#include <cairo.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NIXViewClient;

#ifdef __cplusplus
}
#endif

namespace Nix {

class WK_EXPORT WebViewClient {
public:
    WebViewClient(NIXViewClient* viewClient = 0);
    virtual ~WebViewClient();

    virtual void viewNeedsDisplay(WKRect area);
    virtual void webProcessCrashed(WKStringRef url);
    virtual void webProcessRelaunched() { }
    virtual void doneWithTouchEvent(const TouchEvent&, bool wasEventHandled);
    virtual void doneWithGestureEvent(const GestureEvent&, bool wasEventHandled);
    virtual void pageDidRequestScroll(WKPoint position);
    virtual void didChangeContentsSize(WKSize size);
    virtual void didFindZoomableArea(WKPoint target, WKRect area);
    virtual void updateTextInputState(bool isContentEditable, WKRect cursorRect, WKRect editorRect);
    virtual void compositeCustomLayerToCurrentGLContext(uint32_t id, WKRect rect, const float* matrix, float opacity);

private:
    NIXViewClient* m_viewClient;
};

class WK_EXPORT WebView {
public:
    static WebView* create(WKContextRef, WKPageGroupRef, WebViewClient*);
    virtual ~WebView();

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

    virtual void setScale(float) = 0;
    virtual float scale() const = 0;

    virtual WKSize visibleContentsSize() const = 0;

    virtual void setOpacity(float) = 0;
    virtual float opacity() const = 0;

    virtual void paintToCurrentGLContext() = 0;

    virtual void findZoomableAreaForPoint(const WKPoint& point, int horizontalRadius, int verticalRadius) = 0;

    virtual WKPageRef pageRef() = 0;

    virtual void sendEvent(const Nix::InputEvent&) = 0;

    virtual uint32_t addCustomLayer(WKStringRef elementID) = 0;
    virtual void removeCustomLayer(uint32_t id) = 0;

    virtual void suspendActiveDOMObjectsAndAnimations() = 0;
    virtual void resumeActiveDOMObjectsAndAnimations() = 0;
    virtual bool isSuspended() = 0;
};

} // namespace Nix

#ifdef __cplusplus
extern "C" {
#endif

typedef Nix::WebView NIXView;

// NIXViewClient -----------------------------------------------------------
typedef void (*NIXViewCallback)(const void* clientInfo);
typedef void (*NIXViewViewNeedsDisplayCallback)(WKRect area, const void* clientInfo);
typedef void (*NIXViewWebProcessCrashedCallback)(WKStringRef url, const void* clientInfo);
typedef void (*NIXViewDoneWithTouchEventCallback)(const Nix::TouchEvent* touchEvent, bool wasEventHandled, const void* clientInfo);
typedef void (*NIXViewDoneWithGestureEventCallback)(const Nix::GestureEvent* GestureEvent, bool wasEventHandled, const void* clientInfo);
typedef void (*NIXViewPageDidRequestScrollCallback)(WKPoint position, const void* clientInfo);
typedef void (*NIXViewPageDidChangeContentsSizeCallback)(WKSize size, const void* clientInfo);
typedef void (*NIXViewPageDidFindZoomableAreaCallback)(WKPoint target, WKRect area, const void* clientInfo);
typedef void (*NIXViewPageUpdateTextInputStateCallback)(bool isContentEditable, WKRect cursorRect, WKRect editorRect, const void* clientInfo);
typedef void (*NIXViewCompositeCustomLayerToCurrentGLContext)(uint32_t id, WKRect rect, const float* matrix, float opacity, const void* clientInfo);

struct NIXViewClient {
    int                                              version;
    const void*                                      clientInfo;

    // Version 0
    NIXViewViewNeedsDisplayCallback                  viewNeedsDisplay;
    NIXViewWebProcessCrashedCallback                 webProcessCrashed;
    NIXViewCallback                                  webProcessRelaunched;
    NIXViewDoneWithTouchEventCallback                doneWithTouchEvent;
    NIXViewDoneWithGestureEventCallback              doneWithGestureEvent;
    NIXViewPageDidRequestScrollCallback              pageDidRequestScroll;
    NIXViewPageDidChangeContentsSizeCallback         didChangeContentsSize;
    NIXViewPageDidFindZoomableAreaCallback           didFindZoomableArea;
    NIXViewPageUpdateTextInputStateCallback          updateTextInputState;
    NIXViewCompositeCustomLayerToCurrentGLContext    compositeCustomLayerToCurrentGLContext;
};
typedef struct NIXViewClient NIXViewClient;

enum { kNIXViewCurrentVersion = 0 };

// NIXView -----------------------------------------------------------------
WK_EXPORT NIXView* NIXViewCreate(WKContextRef context, WKPageGroupRef pageGroup, NIXViewClient* viewClient);
WK_EXPORT void NIXViewRelease(NIXView* view);

WK_EXPORT void NIXViewInitialize(NIXView* view);

WK_EXPORT WKSize NIXViewSize(NIXView* view);
WK_EXPORT void NIXViewSetSize(NIXView* view, WKSize size);

WK_EXPORT WKPoint NIXViewScrollPosition(NIXView* view);
WK_EXPORT void NIXViewSetScrollPosition(NIXView* view, WKPoint position);

typedef struct {
    double xx; double yx;
    double xy; double yy;
    double x0; double y0;
} NIXMatrix;

WK_EXPORT void NIXViewSetUserViewportTransformation(NIXView* view, const NIXMatrix* userViewportTransformation);
WK_EXPORT WKPoint NIXViewUserViewportToContents(NIXView* view, WKPoint point);

WK_EXPORT bool NIXViewIsFocused(NIXView* view);
WK_EXPORT void NIXViewSetFocused(NIXView* view, bool focused);

WK_EXPORT bool NIXViewIsVisible(NIXView* view);
WK_EXPORT void NIXViewSetVisible(NIXView* view, bool visible);

WK_EXPORT bool NIXViewIsActive(NIXView* view);
WK_EXPORT void NIXViewSetActive(NIXView* view, bool active);

WK_EXPORT bool NIXViewTransparentBackground(NIXView* view);
WK_EXPORT void NIXViewSetTransparentBackground(NIXView* view, bool transparent);

WK_EXPORT bool NIXViewDrawBackground(NIXView* view);
WK_EXPORT void NIXViewSetDrawBackground(NIXView* view, bool drawBackground);

WK_EXPORT float NIXViewScale(NIXView* view);
WK_EXPORT void NIXViewSetScale(NIXView* view, float scale);

WK_EXPORT void NIXViewSetOpacity(NIXView* view, float opacity);
WK_EXPORT float NIXViewOpacity(NIXView* view);

WK_EXPORT WKSize NIXViewVisibleContentsSize(NIXView* view);

WK_EXPORT void NIXViewPaintToCurrentGLContext(NIXView* view);

WK_EXPORT void NIXViewFindZoomableAreaForPoint(NIXView* view, WKPoint point, int horizontalRadius, int verticalRadius);

WK_EXPORT uint32_t NIXViewAddCustomLayer(NIXView* view, WKStringRef elementID);
WK_EXPORT void NIXViewRemoveCustomLayer(NIXView* view, uint32_t id);

WK_EXPORT WKPageRef NIXViewPageRef(NIXView* view);

WK_EXPORT void NIXViewSendMouseEvent(NIXView* view, Nix::MouseEvent* event);
WK_EXPORT void NIXViewSendWheelEvent(NIXView* view, Nix::WheelEvent* event);
WK_EXPORT void NIXViewSendKeyEvent(NIXView* view, Nix::KeyEvent* event);
WK_EXPORT void NIXViewSendTouchEvent(NIXView* view, Nix::TouchEvent* event);
WK_EXPORT void NIXViewSendGestureEvent(NIXView* view, Nix::GestureEvent* event);

WK_EXPORT void NIXViewSuspendActiveDOMObjectsAndAnimations(NIXView* view);
WK_EXPORT void NIXViewResumeActiveDOMObjectsAndAnimations(NIXView* view);
WK_EXPORT bool NIXViewIsSuspended(NIXView* view);

#ifdef __cplusplus
}
#endif

#endif // WebView_h
