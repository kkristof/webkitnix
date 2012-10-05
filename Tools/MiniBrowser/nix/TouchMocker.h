#ifndef TouchMocker_h
#define TouchMocker_h

#include <NixEvents.h>
#include <WebView.h>
#include <map>

using namespace Nix;

class TouchMocker {
public:
    TouchMocker(WebView*);
    bool handleMousePress(const MouseEvent& event);
    bool handleMouseRelease(const MouseEvent& event);
    bool handleMouseMove(const MouseEvent& event);
    bool handleKeyRelease(const KeyEvent& event);

    void paintTouchPoints() const;

private:
    void trackTouchPoint(unsigned id, TouchPoint::TouchState state, int x, int y, int globalX, int globalY);
    void updateTouchPointsState(TouchPoint::TouchState state);
    void releaseTouchPoints(double timestamp);

    void prepareTouchEvent(TouchPoint::TouchState state, double timestamp);
    void sendCurrentTouchEvent();

    void sendGestureSingleTap(int x, int y, int globalX, int globalY);

    std::map<unsigned, TouchPoint> m_mapTouchIdToTouchPoint;
    WebView* m_webView;
    double m_timestamp;
    InputEvent::Type m_touchType;
};


#endif
