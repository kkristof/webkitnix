#include "GestureRecognizer.h"
#include <cmath>
#include <cstdio>


GestureRecognizer::GestureRecognizer(GestureRecognizerClient *client)
    : m_client(client)
    , m_state(&GestureRecognizer::noGesture)
    , m_timestamp(0)
    , m_doubleTapTimerId(0)
{
}

void GestureRecognizer::reset()
{
    m_state = &GestureRecognizer::noGesture;
}

static const int PanDistanceThreshold = 10;
static const int MaxDoubleTapInterval = 500;

bool exceedsPanThreshold(const Nix::TouchPoint& first, const Nix::TouchPoint& last)
{
    return std::abs(first.globalX - last.globalX) > PanDistanceThreshold || std::abs(first.globalY - last.globalY);
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

void GestureRecognizer::noGesture(const Nix::TouchEvent& event)
{
    Nix::TouchPoint touch = event.touchPoints[0];
    switch (event.type) {
    case Nix::InputEvent::TouchStart:
        m_state = &GestureRecognizer::singleTapPressed;
        m_firstTouchPoint = touch;
        m_previousTouchPoint = touch;
        break;
    case Nix::InputEvent::TouchMove:
    case Nix::InputEvent::TouchEnd:
        fail("received TouchMove or TouchEnd when in NoGesture state.");
        break;
    }
}

void GestureRecognizer::singleTapPressed(const Nix::TouchEvent& event)
{
    Nix::TouchPoint touch = event.touchPoints[0];
    switch (event.type) {
    case Nix::InputEvent::TouchMove:
        // FIXME Both calls to exceedsPanThreshold were using m_previousTouchPoint.
        if (exceedsPanThreshold(touch, m_firstTouchPoint)) {
            updatePanningData(event.timestamp, touch);
        }
        break;
    case Nix::InputEvent::TouchEnd:
        m_state = &GestureRecognizer::waitForDoubleTap;
        m_timestamp = event.timestamp;
        m_doubleTapTimerId = g_timeout_add(MaxDoubleTapInterval, doubleTapTimer, this);
        break;
    case Nix::InputEvent::TouchStart:
        fail("received TouchStart when in SingleTapPressed state.");
        break;
    }
}
void GestureRecognizer::waitForDoubleTap(const Nix::TouchEvent& event)
{
    g_source_remove(m_doubleTapTimerId);
    switch (event.type) {
    case Nix::InputEvent::TouchStart:
        m_state = &GestureRecognizer::doubleTapPressed;
        break;
    case Nix::InputEvent::TouchMove:
    case Nix::InputEvent::TouchEnd:
        fail("received TouchMove or TouchEnd when in WaitForDoubleTap state.");
        break;
    }
}
void GestureRecognizer::doubleTapPressed(const Nix::TouchEvent& event)
{
    Nix::TouchPoint touch = event.touchPoints[0];
    switch (event.type) {
    case Nix::InputEvent::TouchMove:
        if (exceedsPanThreshold(touch, m_firstTouchPoint)) {
            updatePanningData(event.timestamp, touch);
        }
        break;
    case Nix::InputEvent::TouchEnd:
        m_state = &GestureRecognizer::noGesture;
        m_client->handleDoubleTap(event.timestamp, touch);
        break;
    case Nix::InputEvent::TouchStart:
        fail("received TouchStart when in DoubleTapPressed state.");
        break;
    }
}

void GestureRecognizer::updatePanningData(double timestamp, const Nix::TouchPoint& current)
{
    double dx = (current.globalX - m_previousTouchPoint.globalX) / m_client->scale();
    double dy = (current.globalY - m_previousTouchPoint.globalY) / m_client->scale();

    m_state = &GestureRecognizer::panningInProgress;
    m_client->handlePanning(timestamp, dx, dy);
}

void GestureRecognizer::panningInProgress(const Nix::TouchEvent& event)
{
    Nix::TouchPoint touch = event.touchPoints[0];
    switch (event.type) {
    case Nix::InputEvent::TouchMove:
        updatePanningData(event.timestamp, touch);
        break;
    case Nix::InputEvent::TouchEnd:
        m_state = &GestureRecognizer::noGesture;
        m_client->handlePanningFinished(event.timestamp);
        break;
    case Nix::InputEvent::TouchStart:
        fail("received TouchStart when in PanningInProgress state.");
        break;
    }
}

void GestureRecognizer::handleTouchEvent(const Nix::TouchEvent& event)
{
    (this->*m_state)(event);

    Nix::TouchPoint touch = event.touchPoints[0];
    m_previousTouchPoint = touch;
}

gboolean doubleTapTimer(gpointer data)
{
    reinterpret_cast<GestureRecognizer*>(data)->doubleTapTimerTriggered();
    return FALSE;
}

