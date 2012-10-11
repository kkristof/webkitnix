#ifndef TouchMocker_h
#define TouchMocker_h

#include <NixEvents.h>
#include <WebView.h>
#include <map>

class TouchMocker {
public:
    TouchMocker(Nix::WebView*);
    ~TouchMocker();

    bool handleMousePress(const Nix::MouseEvent&);
    bool handleMouseRelease(const Nix::MouseEvent&);
    bool handleMouseMove(const Nix::MouseEvent&);
    bool handleKeyRelease(const Nix::KeyEvent&);

    void paintTouchPoints();
    bool needsRepaint() const { return m_needsRepaint; }

private:
    void trackTouchPoint(unsigned id, Nix::TouchPoint::TouchState state, int x, int y, int globalX, int globalY);
    void updateTouchPointsState(Nix::TouchPoint::TouchState);
    void releaseTouchPoints(double timestamp);

    void prepareTouchEvent(Nix::TouchPoint::TouchState state, double timestamp);
    void sendCurrentTouchEvent();

    void sendGestureSingleTap(int x, int y, int globalX, int globalY);
    void setNeedsRepaint(bool needsRepaint);
    void loadTouchPointTexture();

    std::map<unsigned, Nix::TouchPoint> m_mapTouchIdToTouchPoint;
    Nix::WebView* m_webView;
    double m_timestamp;
    Nix::InputEvent::Type m_touchType;
    bool m_needsRepaint;
    unsigned m_touchTextureId;
};


#endif
