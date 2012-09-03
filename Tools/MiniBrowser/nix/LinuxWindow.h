#ifndef LinuxWindow_h
#define LinuxWindow_h

#include "XlibEventSource.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <utility>

class LinuxWindowClient {
public:
    virtual ~LinuxWindowClient() { };
    virtual void handleExposeEvent() = 0;
    virtual void handleKeyPressEvent(const XKeyPressedEvent&) = 0;
    virtual void handleKeyReleaseEvent(const XKeyReleasedEvent&) = 0;
    virtual void handleButtonPressEvent(const XButtonPressedEvent&) = 0;
    virtual void handleButtonReleaseEvent(const XButtonReleasedEvent&) = 0;
    virtual void handlePointerMoveEvent(const XPointerMovedEvent&) = 0;

    virtual void handleSizeChanged(int width, int height) = 0;
    virtual void handleClosed() = 0;
};

class LinuxWindow : public XlibEventSource::Client {
public:
    LinuxWindow(LinuxWindowClient*);
    ~LinuxWindow();

    std::pair<int, int> size() const;

    void makeCurrent();
    void swapBuffers();

    LinuxWindowClient* client() const { return m_client; }
    Display* display() const { return m_display; }

private:
    // XlibEventSource::Client.
    virtual void handleXEvent(const XEvent&);

    void updateSizeIfNeeded(int width, int height);

    Display* m_display;
    LinuxWindowClient* m_client;
    Window m_rootWindow;
    XVisualInfo* m_visualInfo;
    Colormap m_colormap;
    Window m_window;
    GLXContext m_glContext;
    XlibEventSource* m_eventSource;

    int m_width;
    int m_height;
};

#endif // LinuxWindow_h
