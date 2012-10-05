#include "TouchMocker.h"

#include <GL/gl.h>

// On single touches, every mouse interaction is used as touch event.
// But while you keep Control key pressed, some mouse events may be ignored,
// like mouse moves. Holding that key means you're building a multi-touch event,
// so every time you click with a certain mouse button, it will either press or
// move a given touch point. So to release all touches, just release Control key.

static unsigned touchIdForMouseButton(MouseEvent::Button button)
{
    switch (button) {
    case MouseEvent::LeftButton:
        return 1;
    case MouseEvent::MiddleButton:
        return 2;
    case MouseEvent::RightButton:
        return 3;
    default:
        return 4;
    }
}

static bool isSingleTouch(const InputEvent& event)
{
    return !event.controlKey();
}

TouchMocker::TouchMocker(WebView* webView)
    : m_webView(webView)
{
    m_mapTouchIdToTouchPoint.clear();
}

void TouchMocker::setNeedsRepaint(bool needsRepaint)
{
    m_needsRepaint = needsRepaint;
}

void TouchMocker::paintTouchPoints()
{
    setNeedsRepaint(false);

    if (m_mapTouchIdToTouchPoint.empty())
        return;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_webView->width(), m_webView->height(), 0, -1, 1);

    std::map<unsigned, TouchPoint>::const_iterator it;
    for (it = m_mapTouchIdToTouchPoint.begin(); it != m_mapTouchIdToTouchPoint.end(); ++it) {
        const TouchPoint& touch = it->second;
        double x1 = touch.x - touch.horizontalRadius;
        double y1 = touch.y - touch.verticalRadius;
        double x2 = touch.x + touch.horizontalRadius;
        double y2 = touch.y + touch.verticalRadius;
        glColor3d(1.0, 0.0, 1.0);
        glRectd(x1, y1, x2, y2);
    }
}

bool TouchMocker::handleMousePress(const MouseEvent& event)
{
    unsigned id = touchIdForMouseButton(event.button);
    bool shouldPress = !m_mapTouchIdToTouchPoint.count(id);
    TouchPoint::TouchState state = shouldPress ? TouchPoint::TouchPressed : TouchPoint::TouchMoved;
    trackTouchPoint(id, state, event.x, event.y, event.globalX, event.globalY);
    prepareTouchEvent(state, event.timestamp);
    sendCurrentTouchEvent();
    setNeedsRepaint(true);
    return true;
}

bool TouchMocker::handleMouseRelease(const MouseEvent& event)
{
    if (isSingleTouch(event)) {
        releaseTouchPoints(event.timestamp);
        setNeedsRepaint(true);
    }
    return true;
}

bool TouchMocker::handleMouseMove(const MouseEvent& event)
{
    if (isSingleTouch(event) && m_mapTouchIdToTouchPoint.size() == 1) {
        unsigned touchId = m_mapTouchIdToTouchPoint.begin()->first;
        trackTouchPoint(touchId, TouchPoint::TouchMoved, event.x, event.y, event.globalX, event.globalY);
        prepareTouchEvent(TouchPoint::TouchMoved, event.timestamp);
        sendCurrentTouchEvent();
        setNeedsRepaint(true);
    }
    return true;
}

bool TouchMocker::handleKeyRelease(const KeyEvent& event)
{
    if (event.key == KeyEvent::Key_Control) {
        setNeedsRepaint(!m_mapTouchIdToTouchPoint.empty());
        releaseTouchPoints(event.timestamp);
        return true;
    }
    return false;
}

void TouchMocker::releaseTouchPoints(double timestamp)
{
    if (m_mapTouchIdToTouchPoint.empty())
        return;

    // FIXME: When we have proper gesture recognition, this code should move away.
    TouchPoint touch;
    bool shouldDoSingleTap = false;
    if (m_mapTouchIdToTouchPoint.size() == 1) {
        // Keep track of the single touch we have so far to emit it as SingleTap afterwards.
        touch = m_mapTouchIdToTouchPoint.begin()->second;
        shouldDoSingleTap = true;
    }

    updateTouchPointsState(TouchPoint::TouchReleased);
    prepareTouchEvent(TouchPoint::TouchReleased, timestamp);
    sendCurrentTouchEvent();
    m_mapTouchIdToTouchPoint.clear();

    if (shouldDoSingleTap)
        sendGestureSingleTap(touch.x, touch.y, touch.globalX, touch.globalY);
}

void TouchMocker::updateTouchPointsState(TouchPoint::TouchState state)
{
    std::map<unsigned, TouchPoint>::iterator it;
    for (it = m_mapTouchIdToTouchPoint.begin(); it != m_mapTouchIdToTouchPoint.end(); ++it)
        it->second.state = state;
}

void TouchMocker::trackTouchPoint(unsigned id, TouchPoint::TouchState state, int x, int y, int globalX, int globalY)
{
    // While we update some touch's state the others should be on stationary state.
    updateTouchPointsState(TouchPoint::TouchStationary);

    TouchPoint touch;
    touch.id = id;
    touch.state = state;
    touch.x = x;
    touch.y = y;
    touch.globalX = globalX;
    touch.globalY = globalY;
    // FIXME: Hardcoded values for now, if these are proven to be useful we need to change it.
    touch.verticalRadius = 20;
    touch.horizontalRadius = 20;
    touch.rotationAngle = 0.0;
    touch.pressure = 1.0;

    m_mapTouchIdToTouchPoint[id] = touch;
}

void TouchMocker::prepareTouchEvent(TouchPoint::TouchState state, double timestamp)
{
    switch (state) {
    case TouchPoint::TouchPressed:
        m_touchType = InputEvent::TouchStart;
        break;
    case TouchPoint::TouchMoved:
        m_touchType = InputEvent::TouchMove;
        break;
    case TouchPoint::TouchReleased:
        m_touchType = InputEvent::TouchEnd;
        break;
    }

    m_timestamp = timestamp;
}

void TouchMocker::sendCurrentTouchEvent()
{
    if (m_mapTouchIdToTouchPoint.empty())
        return;

    TouchEvent ev;
    ev.type = m_touchType;
    ev.timestamp = m_timestamp;
    ev.touchPoints.clear();
    std::map<unsigned, TouchPoint>::iterator it;
    for (it = m_mapTouchIdToTouchPoint.begin(); it != m_mapTouchIdToTouchPoint.end(); ++it)
        ev.touchPoints.push_back(it->second);

    m_webView->sendEvent(ev);
}

void TouchMocker::sendGestureSingleTap(int x, int y, int globalX, int globalY)
{
    GestureEvent gestureEvent;
    gestureEvent.type = InputEvent::GestureSingleTap;
    gestureEvent.timestamp = m_timestamp;
    gestureEvent.modifiers = 0;
    gestureEvent.x = x;
    gestureEvent.y = y;
    gestureEvent.globalX = globalX;
    gestureEvent.globalY = globalY;
    // FIXME: Hardcoded values for now, if these are proven to be useful we need to change it.
    gestureEvent.width = 20;
    gestureEvent.height = 20;
    gestureEvent.deltaX = 0.0;
    gestureEvent.deltaY = 0.0;

    m_webView->sendEvent(gestureEvent);
}
