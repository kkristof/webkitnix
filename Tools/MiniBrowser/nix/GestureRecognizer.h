#ifndef GestureRecognizer_h
#define GestureRecognizer_h

#include <NixEvents.h>
#include <glib.h>

extern "C" {
gboolean doubleTapTimer(gpointer);
}

class GestureRecognizerClient {
public:
    virtual void handleSingleTap(double timestamp, const Nix::TouchPoint&) { }
    virtual void handleDoubleTap(double timestamp, const Nix::TouchPoint&) { }
    virtual void handleTapAndHold(double timestamp, const Nix::TouchPoint&) { }

    virtual void handlePanning(double timestamp, double dx, double dy) { }
    virtual void handlePanningFinished(double timestamp) { }

    virtual double scale() { return 1; }
};

class GestureRecognizer {
public:
    GestureRecognizer(GestureRecognizerClient*);

    void handleTouchEvent(const Nix::TouchEvent&);
    void reset();


private:
    // State functions.
    void noGesture(const Nix::TouchEvent&);
    void singleTapPressed(const Nix::TouchEvent&);
    void waitForDoubleTap(const Nix::TouchEvent&);
    void doubleTapPressed(const Nix::TouchEvent&);
    void panningInProgress(const Nix::TouchEvent&);

    void updatePanningData(double, const Nix::TouchPoint&);

    typedef void (GestureRecognizer::*StateFunction)(const Nix::TouchEvent&);
    StateFunction m_state;

    double m_timestamp;
    Nix::TouchPoint m_firstTouchPoint;
    guint m_doubleTapTimerId;
    Nix::TouchPoint m_previousTouchPoint;
    GestureRecognizerClient* m_client;

    void fail(const char*);
    void doubleTapTimerTriggered();
    friend gboolean doubleTapTimer(gpointer);
};

#endif
