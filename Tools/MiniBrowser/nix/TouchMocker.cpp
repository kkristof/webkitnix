#include "TouchMocker.h"

#include <GL/gl.h>
#include <cassert>
#include "touchTexture.h"

using namespace Nix;

// On single touches, every mouse interaction is used as touch event.
// Holding CTRL key means you're building a multi-touch event and each mouse,
// button represents a touch point. All touch points will be released when
// you release the CTRL key.

MockedTouchPoint::MockedTouchPoint()
{
    verticalRadius = 20;
    horizontalRadius = 20;
    rotationAngle = 0.0;
    pressure = 1.0;
    selected = false;
}

static bool isSingleTouch(const InputEvent& event)
{
    return !event.controlKey();
}

TouchMocker::TouchMocker(WebView* webView)
    : m_webView(webView)
{
    loadTouchPointTexture();
}

TouchMocker::~TouchMocker()
{
    glDeleteTextures(1, &m_touchTextureId);
}

void TouchMocker::paintTouchPoints()
{
    static float vertexData[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    static const float texCoords[] = { 0, 0, 1, 0, 0, 1, 1, 1};

    if (m_touchPoints.empty())
        return;

    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_webView->width(), m_webView->height(), 0, -1, 1);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY_EXT);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
    glVertexPointer(3, GL_FLOAT, 0, vertexData);

    glBindTexture(GL_TEXTURE_2D, m_touchTextureId);

    TouchMap::const_iterator it;
    for (it = m_touchPoints.begin(); it != m_touchPoints.end(); ++it) {
        const TouchPoint& touch = it->second;
        double x1 = touch.x - touch.horizontalRadius;
        double y1 = touch.y - touch.verticalRadius;
        double x2 = touch.x + touch.horizontalRadius;
        double y2 = touch.y + touch.verticalRadius;

        vertexData[0] = x1;
        vertexData[1] = y1;
        vertexData[3] = x2;
        vertexData[4] = y1;
        vertexData[6] = x1;
        vertexData[7] = y2;
        vertexData[9] = x2;
        vertexData[10] = y2;
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    glPopMatrix();
}

bool TouchMocker::handleMousePress(const MouseEvent& event)
{
    TouchPoint::TouchState state = m_touchPoints.count(event.button) ? TouchPoint::TouchMoved : TouchPoint::TouchPressed;

    trackTouchPoint(event.button, state, event.x, event.y, event.globalX, event.globalY);
    sendCurrentTouchEvent(state, event.timestamp);
    m_touchPoints[event.button].selected = true;
    return true;
}

bool TouchMocker::handleMouseRelease(const MouseEvent& event)
{
    if (isSingleTouch(event))
        releaseTouchPoints(event.timestamp);
    else
        m_touchPoints[event.button].selected = false;
    return true;
}

bool TouchMocker::handleMouseMove(const MouseEvent& event)
{
    if (m_touchPoints.empty())
        return false;

    TouchMap::const_iterator it = m_touchPoints.begin();
    for (; it != m_touchPoints.end(); ++it) {
        if (it->second.selected) {
            trackTouchPoint(it->first, TouchPoint::TouchMoved, event.x, event.y, event.globalX, event.globalY);
            sendCurrentTouchEvent(TouchPoint::TouchMoved, event.timestamp);
        }
    }

    return true;
}

bool TouchMocker::handleKeyRelease(const KeyEvent& event)
{
    if (event.key == KeyEvent::Key_Control) {
        releaseTouchPoints(event.timestamp);
        return true;
    }
    return false;
}

void TouchMocker::releaseTouchPoints(double timestamp)
{
    if (m_touchPoints.empty())
        return;

    // FIXME: When we have proper gesture recognition, this code should move away.
    TouchPoint touch;
    bool shouldDoSingleTap = false;
    if (m_touchPoints.size() == 1) {
        // Keep track of the single touch we have so far to emit it as SingleTap afterwards.
        touch = m_touchPoints.begin()->second;
        shouldDoSingleTap = true;
    }

    updateTouchPointsState(TouchPoint::TouchReleased);
    sendCurrentTouchEvent(TouchPoint::TouchReleased, timestamp);

    m_touchPoints.clear();

    if (shouldDoSingleTap)
        sendGestureSingleTap(timestamp, touch.x, touch.y, touch.globalX, touch.globalY);
}

void TouchMocker::updateTouchPointsState(TouchPoint::TouchState state)
{
    TouchMap::iterator it = m_touchPoints.begin();
    for (; it != m_touchPoints.end(); ++it)
        it->second.state = state;
}

void TouchMocker::trackTouchPoint(MouseEvent::Button id, TouchPoint::TouchState state, int x, int y, int globalX, int globalY)
{
    // While we update some touch's state the others should be on stationary state.
    updateTouchPointsState(TouchPoint::TouchStationary);

    MockedTouchPoint& touch = m_touchPoints[id];
    touch.id = static_cast<unsigned>(id);
    touch.state = state;
    touch.x = x;
    touch.y = y;
    touch.globalX = globalX;
    touch.globalY = globalY;
}

void TouchMocker::sendCurrentTouchEvent(TouchPoint::TouchState state, double timestamp)
{
    TouchEvent ev;
    ev.timestamp = timestamp;

    switch (state) {
    case TouchPoint::TouchPressed:
        ev.type = InputEvent::TouchStart;
        break;
    case TouchPoint::TouchMoved:
        ev.type = InputEvent::TouchMove;
        break;
    case TouchPoint::TouchReleased:
        ev.type = InputEvent::TouchEnd;
        break;
    }

    TouchMap::iterator it = m_touchPoints.begin();
    for (; it != m_touchPoints.end(); ++it)
        ev.touchPoints.push_back(it->second);

    m_webView->sendEvent(ev);
}

void TouchMocker::sendGestureSingleTap(double timestamp, int x, int y, int globalX, int globalY)
{
    GestureEvent gestureEvent;
    gestureEvent.type = InputEvent::GestureSingleTap;
    gestureEvent.timestamp = timestamp;
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

void TouchMocker::loadTouchPointTexture()
{
    // Texture data size, 64x64 RGBA
    assert(sizeof(textureData) == 64 * 64 * 4);
    glGenTextures(1, &m_touchTextureId);
    glBindTexture(GL_TEXTURE_2D, m_touchTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
}
