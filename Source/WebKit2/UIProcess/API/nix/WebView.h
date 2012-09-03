#ifndef WebView_h
#define WebView_h

#include <WebKit2/WKContext.h>
#include <WebKit2/WKPage.h>
#include <WebKit2/WKPageGroup.h>

namespace Nix {

struct WK_EXPORT InputEvent {
    enum Type {
        MouseDown,
        MouseUp,
        MouseMove,
        MouseWheel,
        KeyDown,
        KeyUp,
        TouchStart,
        TouchMove,
        TouchEnd,
        TouchCancel
    };

    enum Modifiers {
        ShiftKey    = 1 << 0,
        ControlKey  = 1 << 1,
        AltKey      = 1 << 2,
        MetaKey     = 1 << 3,
        CapsLockKey = 1 << 4
    };

    InputEvent() { }
    InputEvent(Type type, uint32_t modifiers, double timestamp)
        : type(type)
        , modifiers(modifiers)
        , timestamp(timestamp)
    {

    }

    bool shiftKey() const { return modifiers & ShiftKey; }
    bool controlKey() const { return modifiers & ControlKey; }
    bool altKey() const { return modifiers & AltKey; }
    bool metaKey() const { return modifiers & MetaKey; }
    bool capsLockKey() const { return modifiers & CapsLockKey; }

    Type type;
    int modifiers;
    double timestamp;
};

struct WK_EXPORT MouseEvent : public InputEvent {
    enum Button {
        NoButton = -1,
        LeftButton,
        MiddleButton,
        RightButton
    };

    MouseEvent() { }
    MouseEvent(InputEvent::Type type, Button button, int x, int y, int globalX, int globalY, int clickCount, uint32_t modifiers, double timestamp)
        : InputEvent(type, modifiers, timestamp)
        , button(button)
        , x(x)
        , y(y)
        , globalX(globalX)
        , globalY(globalY)
        , clickCount(clickCount)
    {

    }

    Button button;
    int x;
    int y;
    int globalX;
    int globalY;
    int clickCount;
};

class WebViewClient {
public:
    virtual ~WebViewClient() { }

    virtual void viewNeedsDisplay(int x, int y, int width, int height) = 0;
};

class WK_EXPORT WebView {
public:
    static WebView* create(WKContextRef, WKPageGroupRef, WebViewClient*);
    virtual ~WebView() { }

    virtual void initialize() = 0;

    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual void setSize(int width, int height) = 0;

    virtual bool isFocused() const = 0;
    virtual void setFocused(bool) = 0;

    virtual bool isVisible() const = 0;
    virtual void setVisible(bool) = 0;

    virtual bool isActive() const = 0;
    virtual void setActive(bool) = 0;

    virtual void paintToCurrentGLContext() = 0;

    virtual WKPageRef pageRef() = 0;

    // TODO: Create a proper datastructure to use as argument for events.
    virtual void sendMouseEvent(const Nix::MouseEvent&) = 0;
    virtual void sendKeyEvent(bool, char) = 0;
};

} // namespace Nix

#endif // WebView_h
