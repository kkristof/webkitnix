#ifndef NIXView_h
#define NIXView_h

#include "NixEvents.h"
#include <WebKit2/WKContext.h>
#include <WebKit2/WKPage.h>
#include <WebKit2/WKPageGroup.h>

// TODO: Remove this include together with the C++ API.
#include <cairo.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpaqueNIXView* NIXView;

// NIXViewClient -----------------------------------------------------------
typedef void (*NIXViewCallback)(const void* clientInfo);
typedef void (*NIXViewViewNeedsDisplayCallback)(WKRect area, const void* clientInfo);
typedef void (*NIXViewWebProcessCrashedCallback)(WKStringRef url, const void* clientInfo);
typedef void (*NIXViewDoneWithTouchEventCallback)(const NIXTouchEvent* touchEvent, bool wasEventHandled, const void* clientInfo);
typedef void (*NIXViewDoneWithGestureEventCallback)(const NIXGestureEvent* gestureEvent, bool wasEventHandled, const void* clientInfo);
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

enum { kNIXViewClientCurrentVersion = 0 };

// NIXView -----------------------------------------------------------------
WK_EXPORT NIXView NIXViewCreate(WKContextRef context, WKPageGroupRef pageGroup);
WK_EXPORT void NIXViewRelease(NIXView view);

WK_EXPORT void NIXViewSetViewClient(NIXView view, const NIXViewClient* viewClient);

WK_EXPORT void NIXViewInitialize(NIXView view);

WK_EXPORT WKSize NIXViewSize(NIXView view);
WK_EXPORT void NIXViewSetSize(NIXView view, WKSize size);

WK_EXPORT WKPoint NIXViewScrollPosition(NIXView view);
WK_EXPORT void NIXViewSetScrollPosition(NIXView view, WKPoint position);

typedef struct {
    double xx; double yx;
    double xy; double yy;
    double x0; double y0;
} NIXMatrix;

WK_EXPORT void NIXViewSetUserViewportTransformation(NIXView view, const NIXMatrix* userViewportTransformation);
WK_EXPORT WKPoint NIXViewUserViewportToContents(NIXView view, WKPoint point);

WK_EXPORT bool NIXViewIsFocused(NIXView view);
WK_EXPORT void NIXViewSetFocused(NIXView view, bool focused);

WK_EXPORT bool NIXViewIsVisible(NIXView view);
WK_EXPORT void NIXViewSetVisible(NIXView view, bool visible);

WK_EXPORT bool NIXViewIsActive(NIXView view);
WK_EXPORT void NIXViewSetActive(NIXView view, bool active);

WK_EXPORT bool NIXViewTransparentBackground(NIXView view);
WK_EXPORT void NIXViewSetTransparentBackground(NIXView view, bool transparent);

WK_EXPORT bool NIXViewDrawBackground(NIXView view);
WK_EXPORT void NIXViewSetDrawBackground(NIXView view, bool drawBackground);

WK_EXPORT float NIXViewScale(NIXView view);
WK_EXPORT void NIXViewSetScale(NIXView view, float scale);

WK_EXPORT void NIXViewSetOpacity(NIXView view, float opacity);
WK_EXPORT float NIXViewOpacity(NIXView view);

WK_EXPORT WKSize NIXViewVisibleContentsSize(NIXView view);

WK_EXPORT void NIXViewPaintToCurrentGLContext(NIXView view);

WK_EXPORT void NIXViewFindZoomableAreaForPoint(NIXView view, WKPoint point, int horizontalRadius, int verticalRadius);

WK_EXPORT uint32_t NIXViewAddCustomLayer(NIXView view, WKStringRef elementID);
WK_EXPORT void NIXViewRemoveCustomLayer(NIXView view, uint32_t id);

WK_EXPORT WKPageRef NIXViewGetPage(NIXView view);

WK_EXPORT void NIXViewSendMouseEvent(NIXView view, const NIXMouseEvent* event);
WK_EXPORT void NIXViewSendWheelEvent(NIXView view, const NIXWheelEvent* event);
WK_EXPORT void NIXViewSendKeyEvent(NIXView view, const NIXKeyEvent* event);
WK_EXPORT void NIXViewSendTouchEvent(NIXView view, const NIXTouchEvent* event);
WK_EXPORT void NIXViewSendGestureEvent(NIXView view, const NIXGestureEvent* event);

WK_EXPORT void NIXViewSuspendActiveDOMObjectsAndAnimations(NIXView view);
WK_EXPORT void NIXViewResumeActiveDOMObjectsAndAnimations(NIXView view);
WK_EXPORT bool NIXViewIsSuspended(NIXView view);

#ifdef __cplusplus
}

class NIXViewAutoPtr {
public:
    explicit NIXViewAutoPtr(NIXView view = 0) : m_view(view) { }
    ~NIXViewAutoPtr() { NIXViewRelease(m_view); }

    NIXView get() const { return m_view; }
    bool operator!() const { return !m_view; }

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    typedef NIXView NIXViewAutoPtr::*UnspecifiedBoolType;
    operator UnspecifiedBoolType() const { return m_view ? &NIXViewAutoPtr::m_view : 0; }

    void reset(NIXView view = 0) {
        NIXView old = m_view;
        m_view = view;
        if (old)
            NIXViewRelease(old);
    }

private:
    // Disable copy.
    NIXViewAutoPtr(NIXViewAutoPtr&);
    NIXViewAutoPtr& operator=(const NIXViewAutoPtr&);

    NIXView m_view;
};

#endif // __cplusplus


#endif // NIXView_h
