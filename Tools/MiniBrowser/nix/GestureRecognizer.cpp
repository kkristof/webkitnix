#include "GestureRecognizer.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static WKPoint computeCenter(WKPoint a, WKPoint b)
{
    return WKPointMake((a.x + b.x) / 2, (a.y + b.y) / 2);
}

static double computeDistance(WKPoint a, WKPoint b)
{
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

static bool hasTouchPointPressed(const NIXTouchEvent& event)
{
    for (int i = 0; i < event.numTouchPoints; ++i) {
        if (event.touchPoints[i].state == kNIXTouchPointStateTouchPressed)
            return true;
    }
    return false;
}

GestureRecognizer::GestureRecognizer(GestureRecognizerClient *client)
    : m_client(client)
    , m_state(&GestureRecognizer::noGesture)
    , m_timestamp(0)
    , m_doubleTapTimerId(0)
    , m_initialPinchDistance(0)
    , m_initialPinchScale(0)
{
}

void GestureRecognizer::reset()
{
    m_state = &GestureRecognizer::noGesture;
}

static const int PanDistanceThreshold = 10;
static const int MaxDoubleTapInterval = 500;

static bool exceedsPanThreshold(const NIXTouchPoint& first, const NIXTouchPoint& last)
{
    float deltaX = std::abs(first.globalX - last.globalX);
    float deltaY = std::abs(first.globalY - last.globalY);
    return std::max(deltaX, deltaY) > PanDistanceThreshold;
}

void GestureRecognizer::doubleTapTimerTriggered()
{
    if (m_state == &GestureRecognizer::waitForDoubleTap) {
        m_client->handleSingleTap(m_timestamp, m_firstTouchPoint);
        m_state = &GestureRecognizer::noGesture;
    }
}

void GestureRecognizer::fail(const char* reason)
{
    fprintf(stderr, "Gesture recognizer failed: %s\n", reason);
    reset();
}

void GestureRecognizer::noGesture(const NIXTouchEvent& event)
{
    NIXTouchPoint touch = event.touchPoints[0];
    switch (event.type) {
    case kNIXInputEventTypeTouchStart:
        m_state = &GestureRecognizer::singleTapPressed;
        m_firstTouchPoint = touch;
        m_previousTouchPoint = touch;
        break;
    case kNIXInputEventTypeTouchMove:
    case kNIXInputEventTypeTouchEnd:
        fail("received TouchMove or TouchEnd when in NoGesture state.");
        break;
    }
}

void GestureRecognizer::setupPinchData(const NIXTouchEvent& event)
{
    const NIXTouchPoint& first = event.touchPoints[0];
    const NIXTouchPoint& second = event.touchPoints[1];

    m_initialPinchDistance = computeDistance(WKPointMake(first.globalX, first.globalY), WKPointMake(second.globalX, second.globalY));
    m_initialPinchScale = m_client->scale();
    m_initialPinchContentCenter = computeCenter(WKPointMake(first.x, first.y), WKPointMake(second.x, second.y));
    m_previousPinchGlobalCenter = computeCenter(WKPointMake(first.globalX, first.globalY), WKPointMake(second.globalX, second.globalY));
}

void GestureRecognizer::singleTapPressed(const NIXTouchEvent& event)
{
    const NIXTouchPoint& touchPoint = event.touchPoints[0];
    switch (event.type) {
    case kNIXInputEventTypeTouchMove:
        if (exceedsPanThreshold(touchPoint, m_firstTouchPoint)) {
            updatePanningData(event.timestamp, touchPoint);
        }
        break;
    case kNIXInputEventTypeTouchEnd:
        m_state = &GestureRecognizer::waitForDoubleTap;
        m_timestamp = event.timestamp;
        m_doubleTapTimerId = g_timeout_add(MaxDoubleTapInterval, doubleTapTimer, this);
        break;
    case kNIXInputEventTypeTouchStart:
        m_state = &GestureRecognizer::pinchInProgress;
        setupPinchData(event);
        m_client->handlePinchStarted(event.timestamp);
        break;
    }
}
void GestureRecognizer::waitForDoubleTap(const NIXTouchEvent& event)
{
    g_source_remove(m_doubleTapTimerId);
    switch (event.type) {
    case kNIXInputEventTypeTouchStart:
        m_state = &GestureRecognizer::doubleTapPressed;
        break;
    case kNIXInputEventTypeTouchMove:
    case kNIXInputEventTypeTouchEnd:
        fail("received TouchMove or TouchEnd when in WaitForDoubleTap state.");
        break;
    }
}
void GestureRecognizer::doubleTapPressed(const NIXTouchEvent& event)
{
    const NIXTouchPoint& touchPoint = event.touchPoints[0];
    switch (event.type) {
    case kNIXInputEventTypeTouchMove:
        if (exceedsPanThreshold(touchPoint, m_firstTouchPoint))
            updatePanningData(event.timestamp, touchPoint);
        break;
    case kNIXInputEventTypeTouchEnd:
        m_state = &GestureRecognizer::noGesture;
        m_client->handleDoubleTap(event.timestamp, touchPoint);
        break;
    case kNIXInputEventTypeTouchStart:
        m_state = &GestureRecognizer::pinchInProgress;
        setupPinchData(event);
        m_client->handlePinchStarted(event.timestamp);
        break;
    }
}

void GestureRecognizer::updatePanningData(double timestamp, const NIXTouchPoint& current)
{
    WKPoint delta = WKPointMake((current.globalX - m_previousTouchPoint.globalX) / m_client->scale(),
                                (current.globalY - m_previousTouchPoint.globalY) / m_client->scale());

    m_state = &GestureRecognizer::panningInProgress;
    m_client->handlePanning(timestamp, delta);
}

void GestureRecognizer::panningInProgress(const NIXTouchEvent& event)
{
    const NIXTouchPoint& touchPoint = event.touchPoints[0];
    switch (event.type) {
    case kNIXInputEventTypeTouchMove:
        updatePanningData(event.timestamp, touchPoint);
        break;
    case kNIXInputEventTypeTouchEnd:
        m_state = &GestureRecognizer::noGesture;
        m_client->handlePanningFinished(event.timestamp);
        break;
    case kNIXInputEventTypeTouchStart:
        m_state = &GestureRecognizer::pinchInProgress;
        setupPinchData(event);
        m_client->handlePinchStarted(event.timestamp);
        break;
    }
}

void GestureRecognizer::pinchInProgress(const NIXTouchEvent& event)
{
    if (event.numTouchPoints < 2) {
        fail("Received only one touch point while in PinchInProgressState.");
        return;
    }

    switch (event.type) {
    case kNIXInputEventTypeTouchMove:
        updatePinchData(event.timestamp, event);
        break;
    case kNIXInputEventTypeTouchEnd:
        if (hasTouchPointPressed(event))
            m_state = &GestureRecognizer::panningInProgress;
        else {
            m_state == &GestureRecognizer::noGesture;
            reset();
        }
        m_client->handlePinchFinished(event.timestamp);
        break;
    case kNIXInputEventTypeTouchStart:
        // Ignore extra touch points.
        break;
    }
}

void GestureRecognizer::handleTouchEvent(const NIXTouchEvent& event)
{
    (this->*m_state)(event);

    m_previousTouchPoint = event.touchPoints[0];
}

gboolean doubleTapTimer(gpointer data)
{
    reinterpret_cast<GestureRecognizer*>(data)->doubleTapTimerTriggered();
    return FALSE;
}

void GestureRecognizer::updatePinchData(double timestamp, const NIXTouchEvent& event)
{
    assert(m_initialPinchScale > 0);

    const NIXTouchPoint& first = event.touchPoints[0];
    const NIXTouchPoint& second = event.touchPoints[1];

    WKPoint currentCenter = computeCenter(WKPointMake(first.globalX, first.globalY), WKPointMake(second.globalX, second.globalY));
    WKPoint delta = WKPointMake((currentCenter.x - m_previousPinchGlobalCenter.x) / m_client->scale(),
                                (currentCenter.y - m_previousPinchGlobalCenter.y) / m_client->scale());
    double currentDistance = computeDistance(WKPointMake(first.globalX, first.globalY), WKPointMake(second.globalX, second.globalY));
    double newScale = m_initialPinchScale * (currentDistance / m_initialPinchDistance);

    m_client->handlePinch(timestamp, delta, newScale, m_initialPinchContentCenter);

    m_previousPinchGlobalCenter = currentCenter;
}
