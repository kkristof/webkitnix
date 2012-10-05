#ifndef TouchMocker_h
#define TouchMocker_h

#include <NixEvents.h>
#include <WebView.h>
#include <map>

using namespace Nix;

class TouchMocker {
public:
    TouchMocker(WebView*);

    bool handleMousePress(const MouseEvent&);
    bool handleMouseRelease(const MouseEvent&);
    bool handleMouseMove(const MouseEvent&);
    bool handleKeyRelease(const KeyEvent&);

    void paintTouchPoints();
    bool needsRepaint() const { return m_needsRepaint; }

private:
    void trackTouchPoint(unsigned id, TouchPoint::TouchState state, int x, int y, int globalX, int globalY);
    void updateTouchPointsState(TouchPoint::TouchState);
    void releaseTouchPoints(double timestamp);

    void prepareTouchEvent(TouchPoint::TouchState state, double timestamp);
    void sendCurrentTouchEvent();

    void sendGestureSingleTap(int x, int y, int globalX, int globalY);
    void setNeedsRepaint(bool needsRepaint);

    std::map<unsigned, TouchPoint> m_mapTouchIdToTouchPoint;
    WebView* m_webView;
    double m_timestamp;
    InputEvent::Type m_touchType;
    bool m_needsRepaint;
};


#endif
