#ifndef GestureRecognizer_h
#define GestureRecognizer_h

#include <NIXEvents.h>
#include <WebKit2/WKGeometry.h>
#include <glib.h>

extern "C" {
gboolean doubleTapTimer(gpointer);
}

class GestureRecognizerClient {
public:
    virtual void handleSingleTap(double timestamp, const NIXTouchPoint&) { }
    virtual void handleDoubleTap(double timestamp, const NIXTouchPoint&) { }
    virtual void handleTapAndHold(double timestamp, const NIXTouchPoint&) { }

    virtual void handlePanning(double timestamp, WKPoint delta) { }
    virtual void handlePanningFinished(double timestamp) { }

    virtual double scale() { return 1; }

    virtual void handlePinchStarted(double timestamp) { }
    virtual void handlePinch(double timestamp, WKPoint delta, double distanceRatio, WKPoint contentCenter) { }
    virtual void handlePinchFinished(double timestamp) { }
};

class GestureRecognizer {
public:
    GestureRecognizer(GestureRecognizerClient*);

    void handleTouchEvent(const NIXTouchEvent&);
    void reset();


private:
    // State functions.
    void noGesture(const NIXTouchEvent&);
    void singleTapPressed(const NIXTouchEvent&);
    void waitForDoubleTap(const NIXTouchEvent&);
    void doubleTapPressed(const NIXTouchEvent&);
    void panningInProgress(const NIXTouchEvent&);
    void pinchInProgress(const NIXTouchEvent&);

    void updatePanningData(double, const NIXTouchPoint&);
    void setupPinchData(const NIXTouchEvent&);
    void updatePinchData(double, const NIXTouchEvent&);

    typedef void (GestureRecognizer::*StateFunction)(const NIXTouchEvent&);
    StateFunction m_state;

    double m_timestamp;
    NIXTouchPoint m_firstTouchPoint;
    guint m_doubleTapTimerId;
    NIXTouchPoint m_previousTouchPoint;
    GestureRecognizerClient* m_client;

    double m_initialPinchDistance;
    double m_initialPinchScale;
    WKPoint m_previousPinchGlobalCenter;
    WKPoint m_initialPinchContentCenter;

    void fail(const char*);
    void doubleTapTimerTriggered();
    friend gboolean doubleTapTimer(gpointer);
};

#endif
