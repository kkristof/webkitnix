/*
 * Copyright (C) 2012-2013 Nokia Corporation and/or its subsidiary(-ies).
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "GestureRecognizer.h"
#include "LinuxWindow.h"
#include "Options.h"
#include "TouchMocker.h"
#include "XlibEventUtils.h"
#include <GL/gl.h>
#include <WebKit2/WKPreferences.h>
#include <WebKit2/WKPreferencesPrivate.h>
#include <WebKit2/WKString.h>
#include <WebKit2/WKURL.h>
#include <NIXView.h>
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
#include <wtf/UnusedParam.h>
#include <WebKit2/WKRetainPtr.h>

extern "C" {
static gboolean callUpdateDisplay(gpointer);
extern void glUseProgram(GLuint);
}

class MiniBrowser : public LinuxWindowClient, public GestureRecognizerClient {
public:

    enum Mode {
        MobileMode,
        DesktopMode
    };

    MiniBrowser(GMainLoop* mainLoop, Mode mode, int width, int height, int viewportHorizontalDisplacement, int viewportVerticalDisplacement);
    virtual ~MiniBrowser();

    WKPageRef pageRef() const { return NIXViewGetPage(m_view); }

    // LinuxWindowClient.
    virtual void handleExposeEvent() { scheduleUpdateDisplay(); }
    virtual void handleKeyPressEvent(const XKeyPressedEvent&);
    virtual void handleKeyReleaseEvent(const XKeyReleasedEvent&);
    virtual void handleButtonPressEvent(const XButtonPressedEvent&);
    virtual void handleButtonReleaseEvent(const XButtonReleasedEvent&);
    virtual void handlePointerMoveEvent(const XPointerMovedEvent&);
    virtual void handleSizeChanged(int, int);
    virtual void handleClosed();

    // NIXViewClient.
    static void viewNeedsDisplay(NIXView, WKRect area, const void* clientInfo);
    static void webProcessCrashed(NIXView, WKStringRef url, const void* clientInfo);
    static void webProcessRelaunched(NIXView, const void* clientInfo);
    static void pageDidRequestScroll(NIXView, WKPoint position, const void* clientInfo);
    static void didChangeContentsSize(NIXView, WKSize size, const void* clientInfo);
    static void didChangeViewportAttributes(NIXView, float width, float height, float minimumScale, float maximumScale, float initialScale, int userScalable, const void* clientInfo);
    static void didFindZoomableArea(NIXView, WKPoint target, WKRect area, const void* clientInfo);
    static void doneWithTouchEvent(NIXView, const NIXTouchEvent* event, bool wasEventHandled, const void* clientInfo);
    static void doneWithGestureEvent(NIXView, const NIXGestureEvent* event, bool wasEventHandled, const void* clientInfo);
    static void updateTextInputState(NIXView, bool isContentEditable, WKRect cursorRect, WKRect editorRect, const void* clientInfo);

    // WKPageLoadClient.
    static void didStartProgress(WKPageRef, const void* clientInfo);

    // GestureRecognizerClient.
    virtual void handleSingleTap(double timestamp, const NIXTouchPoint&);
    virtual void handleDoubleTap(double timestamp, const NIXTouchPoint&);
    virtual void handlePanning(double timestamp, WKPoint delta);
    virtual void handlePanningFinished(double timestamp);
    virtual void handlePinch(double timestamp, WKPoint delta, double scale, WKPoint contentCenter);
    virtual void handlePinchFinished(double timestamp);

    virtual double scale();

    void setTouchEmulationMode(bool enabled);
    Mode mode() const { return m_mode; }
private:

    enum ScaleBehavior {
        AdjustToBoundaries = 1 << 0,
        LowerMinimumScale = 1 << 1
    };
    void handleWheelEvent(const XButtonPressedEvent&);
    void updateClickCount(const XButtonPressedEvent&);

    void updateDisplay();
    void scheduleUpdateDisplay();
    WKPoint adjustScrollPositionToBoundaries(WKPoint position);
    void adjustScrollPosition();
    void adjustViewportToTextInputArea();
    double scaleToFitContents();

    void scaleAtPoint(const WKPoint& point, double scale, ScaleBehavior scaleBehavior = AdjustToBoundaries);

    NIXView webViewAtX11Position(const WKPoint& poisition);

    WKRetainPtr<WKContextRef> m_context;
    WKRetainPtr<WKPageGroupRef> m_pageGroup;
    LinuxWindow* m_window;
    NIXView m_view;
    WKRect m_viewRect;
    GMainLoop* m_mainLoop;
    double m_lastClickTime;
    int m_lastClickX;
    int m_lastClickY;
    WKEventMouseButton m_lastClickButton;
    unsigned m_clickCount;
    TouchMocker* m_touchMocker;
    Mode m_mode;
    bool m_displayUpdateScheduled;
    WKSize m_contentsSize;
    GestureRecognizer m_gestureRecognizer;
    bool m_postponeTextInputUpdates;
    bool m_shouldFocusEditableArea;
    WKRect m_cursorRect;
    WKRect m_editorRect;
    bool m_shouldRestoreViewportWhenLosingFocus;
    double m_scaleBeforeFocus;
    WKPoint m_scrollPositionBeforeFocus;
    bool m_pageHandlesTouchEvents;

    float m_viewportWidth;
    float m_viewportHeight;
    float m_viewportMinScale;
    float m_viewportMaxScale;
    float m_viewportInitScale;
    bool m_viewportUserScalable;

    friend gboolean callUpdateDisplay(gpointer);
};

MiniBrowser::MiniBrowser(GMainLoop* mainLoop, Mode mode, int width, int height, int viewportHorizontalDisplacement, int viewportVerticalDisplacement)
    : m_context(AdoptWK, WKContextCreate())
    , m_pageGroup(AdoptWK, (WKPageGroupCreateWithIdentifier(WKStringCreateWithUTF8CString("MiniBrowser"))))
    , m_window(new LinuxWindow(this, width, height))
    , m_view(0)
    , m_mainLoop(mainLoop)
    , m_lastClickTime(0)
    , m_lastClickX(0)
    , m_lastClickY(0)
    , m_lastClickButton(kWKEventMouseButtonNoButton)
    , m_clickCount(0)
    , m_touchMocker(0)
    , m_mode(mode)
    , m_displayUpdateScheduled(false)
    , m_gestureRecognizer(GestureRecognizer(this))
    , m_postponeTextInputUpdates(true)
    , m_shouldFocusEditableArea(false)
    , m_shouldRestoreViewportWhenLosingFocus(false)
    , m_pageHandlesTouchEvents(false)
    , m_viewportMinScale(0.25)
    , m_viewportMaxScale(5)
    , m_viewportUserScalable(true)
    , m_viewportInitScale(1)
{
    g_main_loop_ref(m_mainLoop);

    WKPreferencesRef preferences = WKPageGroupGetPreferences(m_pageGroup.get());
    WKPreferencesSetAcceleratedCompositingEnabled(preferences, true);
    WKPreferencesSetFrameFlatteningEnabled(preferences, true);
    WKPreferencesSetDeveloperExtrasEnabled(preferences, true);
    WKPreferencesSetWebGLEnabled(preferences, true);

    m_view = NIXViewCreate(m_context.get(), m_pageGroup.get());

    NIXViewClient viewClient;
    memset(&viewClient, 0, sizeof(NIXViewClient));
    viewClient.version = kNIXViewClientCurrentVersion;
    viewClient.clientInfo = this;
    viewClient.viewNeedsDisplay = MiniBrowser::viewNeedsDisplay;
    viewClient.webProcessCrashed = MiniBrowser::webProcessCrashed;
    viewClient.webProcessRelaunched = MiniBrowser::webProcessRelaunched;
    viewClient.doneWithTouchEvent = MiniBrowser::doneWithTouchEvent;
    viewClient.doneWithGestureEvent = MiniBrowser::doneWithGestureEvent;
    viewClient.pageDidRequestScroll = MiniBrowser::pageDidRequestScroll;
    viewClient.didChangeContentsSize = MiniBrowser::didChangeContentsSize;
    viewClient.didChangeViewportAttributes = MiniBrowser::didChangeViewportAttributes;
    viewClient.didFindZoomableArea = MiniBrowser::didFindZoomableArea;
    viewClient.updateTextInputState = MiniBrowser::updateTextInputState;
    NIXViewSetViewClient(m_view, &viewClient);

    NIXViewInitialize(m_view);

    WKPageLoaderClient loaderClient;
    memset(&loaderClient, 0, sizeof(loaderClient));
    loaderClient.version = kWKPageLoaderClientCurrentVersion;
    loaderClient.clientInfo = this;
    loaderClient.didStartProgress = MiniBrowser::didStartProgress;
    WKPageSetPageLoaderClient(pageRef(), &loaderClient);

    if (m_mode == MobileMode)
        WKPageSetUseFixedLayout(pageRef(), true);

    WKSize size = m_window->size();
    m_viewRect = WKRectMake(viewportHorizontalDisplacement, viewportVerticalDisplacement, size.width - viewportHorizontalDisplacement, size.height - viewportVerticalDisplacement);
    NIXViewSetSize(m_view, m_viewRect.size);

    if (viewportHorizontalDisplacement || viewportVerticalDisplacement) {
        NIXMatrix transform = NIXMatrixMakeTranslation(viewportHorizontalDisplacement, viewportVerticalDisplacement);
        NIXViewSetUserViewportTransformation(m_view, &transform);
    }

    NIXViewSetFocused(m_view, true);
    NIXViewSetVisible(m_view, true);
    NIXViewSetActive(m_view, true);
}

MiniBrowser::~MiniBrowser()
{
    g_main_loop_unref(m_mainLoop);

    NIXViewRelease(m_view);
    delete m_window;
    delete m_touchMocker;
}

void MiniBrowser::setTouchEmulationMode(bool enabled)
{
    if (enabled && !m_touchMocker) {
        m_touchMocker = new TouchMocker(m_view);
    } else if (!enabled && m_touchMocker) {
        delete m_touchMocker;
        m_touchMocker = 0;
    }
}


enum NavigationCommand {
    NoNavigation,
    BackNavigation,
    ForwardNavigation,
    ReloadNavigation
};

static NavigationCommand checkNavigationCommand(const KeySym keySym, const unsigned state)
{
    if (keySym == XK_F5)
        return ReloadNavigation;
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

static NIXKeyEvent convertXKeyEventToNixKeyEvent(const XKeyEvent& event, const KeySym& symbol, bool useUpperCase)
{
    NIXKeyEvent nixEvent;
    nixEvent.type = (event.type == KeyPress) ? kNIXInputEventTypeKeyDown : kNIXInputEventTypeKeyUp;
    nixEvent.modifiers = convertXEventModifiersToNativeModifiers(event.state);
    nixEvent.timestamp = convertXEventTimeToNixTimestamp(event.time);
    nixEvent.shouldUseUpperCase = useUpperCase;
    nixEvent.isKeypad = isKeypadKeysym(symbol);
    nixEvent.key = convertXKeySymToNativeKeycode(symbol);
    return nixEvent;
}

static NIXMouseEvent convertXButtonEventToNixButtonEvent(NIXView view, const XButtonEvent& event, NIXInputEventType type, unsigned clickCount)
{
    NIXMouseEvent nixEvent;
    nixEvent.type = type;
    nixEvent.button = convertXEventButtonToNativeMouseButton(event.button);
    WKPoint contentsPoint = NIXViewUserViewportToContents(view, WKPointMake(event.x, event.y));
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
    if (!m_view)
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
    case ReloadNavigation:
        WKPageReload(pageRef());
        return;
    default:
        NIXKeyEvent nixEvent = convertXKeyEventToNixKeyEvent(event, symbol, shouldUseUpperCase);
        NIXViewSendKeyEvent(m_view, &nixEvent);
    }
}

void MiniBrowser::handleKeyReleaseEvent(const XKeyReleasedEvent& event)
{
    if (!m_view)
        return;

    bool shouldUseUpperCase;
    KeySym symbol = chooseSymbolForXKeyEvent(event, &shouldUseUpperCase);
    if (checkNavigationCommand(symbol, event.state) != NoNavigation)
        return;
    NIXKeyEvent nixEvent = convertXKeyEventToNixKeyEvent(event, symbol, shouldUseUpperCase);
    if (m_touchMocker && m_touchMocker->handleKeyRelease(nixEvent)) {
        scheduleUpdateDisplay();
        return;
    }
    NIXViewSendKeyEvent(m_view, &nixEvent);
}

void MiniBrowser::handleWheelEvent(const XButtonPressedEvent& event)
{
    WKPoint contentsPoint = NIXViewUserViewportToContents(m_view, WKPointMake(event.x, event.y));

    if (m_mode == MobileMode && event.state & ControlMask) {
        double newScale = NIXViewScale(m_view) * (event.button == 4 ? 1.1 : 0.9);
        scaleAtPoint(contentsPoint, newScale);
        return;
    }

    // Same constant we use inside WebView to calculate the ticks. See also WebCore::Scrollbar::pixelsPerLineStep().
    const float pixelsPerStep = 40.0f;

    NIXWheelEvent nixEvent;
    nixEvent.type = kNIXInputEventTypeWheel;
    nixEvent.modifiers = convertXEventModifiersToNativeModifiers(event.state);
    nixEvent.timestamp = convertXEventTimeToNixTimestamp(event.time);
    nixEvent.x = contentsPoint.x;
    nixEvent.y = contentsPoint.y;
    nixEvent.globalX = event.x_root;
    nixEvent.globalY = event.y_root;
    nixEvent.delta = pixelsPerStep * (event.button == 4 ? 1 : -1);
    nixEvent.orientation = event.state & ShiftMask ? kNIXWheelEventOrientationHorizontal : kNIXWheelEventOrientationVertical;
    NIXViewSendWheelEvent(m_view, &nixEvent);
}

static const double doubleClickInterval = 300;
static const double horizontalMarginForViewportAdjustment = 8.0;
static const double scaleFactorForTextInputFocus = 2.0;

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
    NIXView view = webViewAtX11Position(WKPointMake(event.x, event.y));
    if (!view)
        return;

    if (event.button == 4 || event.button == 5) {
        handleWheelEvent(event);
        return;
    }

    updateClickCount(event);

    NIXMouseEvent nixEvent = convertXButtonEventToNixButtonEvent(view, event, kNIXInputEventTypeMouseDown, m_clickCount);
    if (m_touchMocker && m_touchMocker->handleMousePress(nixEvent, WKPointMake(event.x, event.y))) {
        scheduleUpdateDisplay();
        return;
    }
    NIXViewSendMouseEvent(view, &nixEvent);
}

void MiniBrowser::handleButtonReleaseEvent(const XButtonReleasedEvent& event)
{
    if (event.button == 4 || event.button == 5)
        return;

    NIXMouseEvent nixEvent = convertXButtonEventToNixButtonEvent(m_view, event, kNIXInputEventTypeMouseUp, 0);
    if (m_touchMocker && m_touchMocker->handleMouseRelease(nixEvent)) {
        scheduleUpdateDisplay();
        return;
    }

    // The mouse release event was allowed to be sent to the TouchMocker because it
    // may be tracking a button press that happened in a valid position.
    NIXView view = webViewAtX11Position(WKPointMake(event.x, event.y));
    if (!view)
        return;
    NIXViewSendMouseEvent(view, &nixEvent);
}

void MiniBrowser::handlePointerMoveEvent(const XPointerMovedEvent& event)
{
    NIXMouseEvent nixEvent;
    nixEvent.type = kNIXInputEventTypeMouseMove;
    nixEvent.button = kWKEventMouseButtonNoButton;
    WKPoint contentsPoint = NIXViewUserViewportToContents(m_view, WKPointMake(event.x, event.y));
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
    NIXView view = webViewAtX11Position(WKPointMake(event.x, event.y));
    if (!view)
        return;
    NIXViewSendMouseEvent(view, &nixEvent);
}

void MiniBrowser::handleSizeChanged(int width, int height)
{
    if (!m_view)
        return;

    m_viewRect.size.width = width - m_viewRect.origin.x;
    m_viewRect.size.height = height - m_viewRect.origin.y;
    NIXViewSetSize(m_view, m_viewRect.size);

    if (m_mode == MobileMode)
        NIXViewSetScale(m_view, scaleToFitContents());
}

void MiniBrowser::handleClosed()
{
    g_main_loop_quit(m_mainLoop);
}

void MiniBrowser::updateDisplay()
{
    if (!m_view || !m_window)
        return;

    m_window->makeCurrent();

    WKSize size = m_window->size();
    glViewport(0, 0, size.width, size.height);
    glClearColor(0.4, 0.4, 0.4, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    NIXViewPaintToCurrentGLContext(m_view);
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
    WKSize visibleContentsSize = NIXViewVisibleContentsSize(m_view);
    double rightBoundary = m_contentsSize.width - visibleContentsSize.width;
    // Contents height may be shorter than the scaled viewport height.
    double bottomBoundary = m_contentsSize.height < visibleContentsSize.height ? 0 : m_contentsSize.height - visibleContentsSize.height;

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
    return NIXViewSize(m_view).width / m_contentsSize.width;
}

void MiniBrowser::adjustScrollPosition()
{
    WKPoint position = adjustScrollPositionToBoundaries(NIXViewScrollPosition(m_view));
    if (position == NIXViewScrollPosition(m_view))
        return;
    NIXViewSetScrollPosition(m_view, position);
}

void MiniBrowser::viewNeedsDisplay(NIXView, WKRect area, const void* clientInfo)
{
    UNUSED_PARAM(area);
    MiniBrowser* mb = static_cast<MiniBrowser*>(const_cast<void*>(clientInfo));
    mb->scheduleUpdateDisplay();
}

void MiniBrowser::webProcessCrashed(NIXView, WKStringRef url, const void* clientInfo)
{
    UNUSED_PARAM(clientInfo);
    size_t urlStringSize =  WKStringGetMaximumUTF8CStringSize(url);
    char* urlString = new char[urlStringSize];
    WKStringGetUTF8CString(url, urlString, urlStringSize);
    fprintf(stderr, "The web process has crashed on '%s'.\n", urlString);
    delete urlString;
}

void MiniBrowser::webProcessRelaunched(NIXView, const void* clientInfo)
{
    UNUSED_PARAM(clientInfo);
    fprintf(stdout, "The web process has been restarted.\n");
}

void MiniBrowser::pageDidRequestScroll(NIXView, WKPoint position, const void* clientInfo)
{
    MiniBrowser* mb = static_cast<MiniBrowser*>(const_cast<void*>(clientInfo));
    if (!NIXViewIsSuspended(mb->m_view))
        NIXViewSetScrollPosition(mb->m_view, mb->adjustScrollPositionToBoundaries(position));
}

void MiniBrowser::didChangeContentsSize(NIXView, WKSize size, const void* clientInfo)
{
    MiniBrowser* mb = static_cast<MiniBrowser*>(const_cast<void*>(clientInfo));
    mb->m_contentsSize = size;

    if (mb->m_mode == MiniBrowser::MobileMode)
        mb->adjustScrollPosition();
}

void MiniBrowser::didChangeViewportAttributes(NIXView, float width, float height, float minimumScale, float maximumScale, float initialScale, int userScalable, const void* clientInfo)
{
    MiniBrowser* mb = static_cast<MiniBrowser*>(const_cast<void*>(clientInfo));

    mb->m_viewportWidth = width;
    mb->m_viewportHeight = height;
    mb->m_viewportMinScale = minimumScale;
    mb->m_viewportMaxScale = maximumScale;
    mb->m_viewportInitScale = initialScale;
    mb->m_viewportUserScalable = userScalable;

    if (mb->m_viewportInitScale < 0) {
        double scale = mb->scaleToFitContents();
        if (scale < minimumScale)
            scale = minimumScale;
        else if (scale > maximumScale)
            scale = maximumScale;
        mb->m_viewportInitScale = scale;
    }

    if (!mb->m_viewportUserScalable)
        mb->m_viewportMaxScale = mb->m_viewportMinScale = mb->m_viewportInitScale;

    NIXViewSetScale(mb->m_view, mb->m_viewportInitScale);
}

void MiniBrowser::didFindZoomableArea(NIXView, WKPoint target, WKRect area, const void* clientInfo)
{
    MiniBrowser* mb = static_cast<MiniBrowser*>(const_cast<void*>(clientInfo));

    // Zoomable area width is the same as web page width, and this is fully visible.
    if (mb->m_contentsSize.width == area.size.width && mb->m_contentsSize.width == NIXViewVisibleContentsSize(mb->m_view).width)
        return;

    // The zoomed area will look nicer with a horizontal margin.
    double scale = NIXViewSize(mb->m_view).width / (area.size.width + horizontalMarginForViewportAdjustment * 2.0);

    // Trying to zoom to an area with the same scale factor causes a zoom out.
    if (scale == NIXViewScale(mb->m_view))
        scale = mb->scaleToFitContents();
    else {
        // We want the zoomed content area to fit horizontally in the WebView,
        // so let's give the scaleAtPoint method a suitable value.
        target.x = area.origin.x - horizontalMarginForViewportAdjustment;
        NIXViewSetScrollPosition(mb->m_view, WKPointMake(target.x, NIXViewScrollPosition(mb->m_view).y));
    }

    mb->scaleAtPoint(target, scale);
}

void MiniBrowser::doneWithTouchEvent(NIXView, const NIXTouchEvent* event, bool wasEventHandled, const void* clientInfo)
{
    MiniBrowser* mb = static_cast<MiniBrowser*>(const_cast<void*>(clientInfo));
    if (mb->m_pageHandlesTouchEvents)
        return;

    if (wasEventHandled)
        mb->m_pageHandlesTouchEvents = true;
    else
        mb->m_gestureRecognizer.handleTouchEvent(*event);
}

void MiniBrowser::doneWithGestureEvent(NIXView, const NIXGestureEvent* event, bool wasEventHandled, const void* clientInfo)
{
    if (wasEventHandled)
        return;

    MiniBrowser* mb = static_cast<MiniBrowser*>(const_cast<void*>(clientInfo));

    if (event->type == kNIXInputEventTypeGestureSingleTap && mb->m_shouldFocusEditableArea) {
        mb->m_shouldFocusEditableArea = false;
        mb->adjustViewportToTextInputArea();
    }

    mb->m_postponeTextInputUpdates = true;
}

double MiniBrowser::scale()
{
    return NIXViewScale(m_view);
}

void MiniBrowser::handleSingleTap(double timestamp, const NIXTouchPoint& touchPoint)
{
    NIXGestureEvent gestureEvent;
    gestureEvent.type = kNIXInputEventTypeGestureSingleTap;
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

    m_postponeTextInputUpdates = false;
    NIXViewSendGestureEvent(m_view, &gestureEvent);
}

void MiniBrowser::handleDoubleTap(double timestamp, const NIXTouchPoint& touchPoint)
{
    if (!m_viewportUserScalable)
        return;
    WKPoint contentsPoint = WKPointMake(touchPoint.x, touchPoint.y);
    NIXViewFindZoomableAreaForPoint(m_view, contentsPoint, touchPoint.verticalRadius, touchPoint.horizontalRadius);
}

void MiniBrowser::handlePanning(double timestamp, WKPoint delta)
{
    // When the user is panning around the contents we don't force the page scroll position
    // to respect any boundaries other than the physical constraints of the device from where
    // the user input came. This will be adjusted after the user interaction ends.
    NIXViewSuspendActiveDOMObjectsAndAnimations(m_view);
    WKPoint position = NIXViewScrollPosition(m_view);
    position.x -= delta.x;
    position.y -= delta.y;
    NIXViewSetScrollPosition(m_view, position);
}

void MiniBrowser::handlePanningFinished(double timestamp)
{
    adjustScrollPosition();
    NIXViewResumeActiveDOMObjectsAndAnimations(m_view);
}

void MiniBrowser::handlePinch(double timestamp, WKPoint delta, double scale, WKPoint contentCenter)
{
    if (!m_viewportUserScalable)
        return;

    // Scaling: The page should be scaled proportionally to the distance of the pinch.
    // Scrolling: If the center of the pinch initially was position (120,120) in content
    //            coordinates, them during the page must be scrolled to keep the pinch center
    //            at the same coordinates.
    NIXViewSuspendActiveDOMObjectsAndAnimations(m_view);
    WKPoint position = WKPointMake(NIXViewScrollPosition(m_view).x - delta.x, NIXViewScrollPosition(m_view).y - delta.y);

    NIXViewSetScrollPosition(m_view, position);
    scaleAtPoint(contentCenter, scale, LowerMinimumScale);
}

void MiniBrowser::handlePinchFinished(double timestamp)
{
    double scale = NIXViewScale(m_view);
    bool needsScale = false;
    double minimumScale = double(NIXViewSize(m_view).width) / m_contentsSize.width;

    if (scale > m_viewportMaxScale) {
        scale = m_viewportMaxScale;
        needsScale = true;
    } else if (scale < m_viewportMinScale) {
        scale = m_viewportMinScale;
        needsScale = true;
    } else if (scale < minimumScale) {
        scale = minimumScale;
        needsScale = true;
    }

    if (needsScale)
        NIXViewSetScale(m_view, scale);

    adjustScrollPosition();
    NIXViewResumeActiveDOMObjectsAndAnimations(m_view);
}

void MiniBrowser::scaleAtPoint(const WKPoint& point, double scale, ScaleBehavior scaleBehavior)
{
    if (!m_viewportUserScalable)
        return;

    double minimumScale = double(NIXViewSize(m_view).width) / m_contentsSize.width;

    if (scaleBehavior & LowerMinimumScale)
        minimumScale *= 0.5;

    if (scale < minimumScale)
        scale = minimumScale;

    // Calculate new scroll points that will keep the content
    // approximately at the same visual point.
    double scaleRatio = NIXViewScale(m_view) / scale;
    WKPoint position = WKPointMake(point.x - (point.x - NIXViewScrollPosition(m_view).x) * scaleRatio,
                                   point.y - (point.y - NIXViewScrollPosition(m_view).y) * scaleRatio);

    NIXViewSetScale(m_view, scale);
    if (scaleBehavior & AdjustToBoundaries)
        position = adjustScrollPositionToBoundaries(position);
    NIXViewSetScrollPosition(m_view, position);
}

static inline bool areaContainsPoint(const WKRect& area, const WKPoint& point)
{
    return !(point.x < area.origin.x || point.y < area.origin.y || point.x >= (area.origin.x + area.size.width) || point.y >= (area.origin.y + area.size.height));
}

NIXView MiniBrowser::webViewAtX11Position(const WKPoint& position)
{
    if (areaContainsPoint(m_viewRect, position))
        return m_view;
    return 0;
}

void MiniBrowser::adjustViewportToTextInputArea()
{
    m_shouldRestoreViewportWhenLosingFocus = true;
    m_scaleBeforeFocus = NIXViewScale(m_view);
    m_scrollPositionBeforeFocus = NIXViewScrollPosition(m_view);

    NIXViewSetScale(m_view, scaleFactorForTextInputFocus);

    // After scaling to fit editor rect width, we align vertically based on cursor rect.
    WKPoint scrollPosition;
    scrollPosition.x = m_editorRect.origin.x - scaleFactorForTextInputFocus * horizontalMarginForViewportAdjustment;
    double verticalOffset = (NIXViewVisibleContentsSize(m_view).height - m_cursorRect.size.height) / 2.0;
    scrollPosition.y = m_cursorRect.origin.y - verticalOffset;
    scrollPosition = adjustScrollPositionToBoundaries(scrollPosition);
    NIXViewSetScrollPosition(m_view, scrollPosition);
}

static inline bool WKRectIsEqual(const WKRect& a, const WKRect& b)
{
    return a.origin == b.origin && a.size.width == b.size.width && a.size.height == b.size.height;
}

void MiniBrowser::updateTextInputState(NIXView, bool isContentEditable, WKRect cursorRect, WKRect editorRect, const void* clientInfo)
{
    MiniBrowser* mb = static_cast<MiniBrowser*>(const_cast<void*>(clientInfo));

    if (mb->m_postponeTextInputUpdates)
        return;

    if (isContentEditable) {
        // If we're only moving cursor inside the current editor, then we should not focus it again.
        if (WKRectIsEqual(editorRect, mb->m_editorRect))
            return;

        mb->m_shouldFocusEditableArea = true;
        mb->m_cursorRect = cursorRect;
        mb->m_editorRect = editorRect;
    } else {
        if (mb->m_shouldRestoreViewportWhenLosingFocus) {
            mb->m_shouldRestoreViewportWhenLosingFocus = false;
            NIXViewSetScale(mb->m_view, mb->m_scaleBeforeFocus);
            NIXViewSetScrollPosition(mb->m_view, mb->m_scrollPositionBeforeFocus);
        }
        mb->m_cursorRect = WKRectMake(0, 0, 0, 0);
        mb->m_editorRect = WKRectMake(0, 0, 0, 0);
    }
}

void MiniBrowser::didStartProgress(WKPageRef, const void* clientInfo)
{
    MiniBrowser* mb = static_cast<MiniBrowser*>(const_cast<void*>(clientInfo));
    mb->m_pageHandlesTouchEvents = false;
    mb->m_gestureRecognizer.reset();
}

int main(int argc, char* argv[])
{
    Options options;
    if (!options.parse(argc, argv))
        return 1;

    printf("MiniBrowser: Use Alt + Left and Alt + Right to navigate back and forward. Use F5 to reload.\n");

    std::string url = options.url;
    if (url.empty())
        url = "http://www.google.com";
    else if (url.find("http") != 0 && url.find("file://") != 0) {
        std::ifstream localFile(url.c_str());
        url.insert(0, localFile ? "file://" : "http://");
    }

    GMainLoop* mainLoop = g_main_loop_new(0, false);
    MiniBrowser browser(mainLoop, options.desktopModeEnabled ? MiniBrowser::DesktopMode : MiniBrowser::MobileMode, options.width, options.height, options.viewportHorizontalDisplacement, options.viewportVerticalDisplacement);

    if (options.forceTouchEmulationEnabled || !options.desktopModeEnabled) {
        printf("Touch Emulation Mode enabled. Hold Control key to build and emit a multi-touch event: each mouse button should be a different touch point. Release Control Key to clear all tracking pressed touches.\n");
        browser.setTouchEmulationMode(true);
    }

    if (!options.userAgent.empty())
        WKPageSetCustomUserAgent(browser.pageRef(), WKStringCreateWithUTF8CString(options.userAgent.c_str()));

    if (!options.desktopModeEnabled)
        printf("Use Control + mouse wheel to zoom in and out.\n");

    WKPageLoadURL(browser.pageRef(), WKURLCreateWithUTF8CString(url.c_str()));

    g_main_loop_run(mainLoop);
    g_main_loop_unref(mainLoop);
}
