#ifndef TouchMocker_h
#define TouchMocker_h

#include <NixEvents.h>
#include <WebView.h>
#include <map>

struct MockedTouchPoint : public NIXTouchPoint {
    // Means there is a mouse button pressed and the next mouse move
    // will update this touch point.
    bool selected;

    int windowX;
    int windowY;

    MockedTouchPoint();
};

class TouchMocker {
public:
    TouchMocker(NIXView);
    ~TouchMocker();

    bool handleMousePress(const NIXMouseEvent&, const WKPoint& windowPos);
    bool handleMouseRelease(const NIXMouseEvent&);
    bool handleMouseMove(const NIXMouseEvent&, const WKPoint& windowPos);
    bool handleKeyRelease(const NIXKeyEvent&);

    void paintTouchPoints(const WKSize& size);

private:
    void trackTouchPoint(WKEventMouseButton id, NIXTouchPointState state, const NIXMouseEvent& event, const WKPoint& windowPos);
    void updateTouchPointsState(NIXTouchPointState);
    void releaseTouchPoints(double timestamp);

    void sendCurrentTouchEvent(NIXTouchPointState state, double timestamp);

    void loadTouchPointTexture();

    NIXView m_view;
    NIXInputEventType m_touchType;
    typedef std::map<WKEventMouseButton, MockedTouchPoint> TouchMap;
    TouchMap m_touchPoints;

    unsigned m_touchTextureId;
};


#endif
