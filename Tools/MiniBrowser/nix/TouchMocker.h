#ifndef TouchMocker_h
#define TouchMocker_h

#include <NixEvents.h>
#include <WebView.h>
#include <map>

struct MockedTouchPoint : public Nix::TouchPoint {
    // Means there is a mouse button pressed and the next mouse move
    // will update this touch point.
    bool selected;

    MockedTouchPoint();
};

class TouchMocker {
public:
    TouchMocker(Nix::WebView*);
    ~TouchMocker();

    bool handleMousePress(const Nix::MouseEvent&);
    bool handleMouseRelease(const Nix::MouseEvent&);
    bool handleMouseMove(const Nix::MouseEvent&);
    bool handleKeyRelease(const Nix::KeyEvent&);

    void paintTouchPoints();

private:
    void trackTouchPoint(Nix::MouseEvent::Button id, Nix::TouchPoint::TouchState state, int x, int y, int globalX, int globalY);
    void updateTouchPointsState(Nix::TouchPoint::TouchState);
    void releaseTouchPoints(double timestamp);

    void sendCurrentTouchEvent(Nix::TouchPoint::TouchState state, double timestamp);

    void sendGestureSingleTap(double timestamp, int x, int y, int globalX, int globalY);
    void loadTouchPointTexture();

    Nix::WebView* m_webView;
    Nix::InputEvent::Type m_touchType;
    typedef std::map<Nix::MouseEvent::Button, MockedTouchPoint> TouchMap;
    TouchMap m_touchPoints;

    unsigned m_touchTextureId;
};


#endif
