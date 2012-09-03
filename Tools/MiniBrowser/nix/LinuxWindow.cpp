#include "LinuxWindow.h"

#include <cstdio>
#include <cstdlib>
#include <glib.h>

Atom wmDeleteMessageAtom;

LinuxWindow::LinuxWindow(LinuxWindowClient* client)
    : m_display(XOpenDisplay(0))
    , m_visualInfo(0)
    , m_client(client)
    , m_width(800)
    , m_height(600)
    , m_eventSource(0)
{
    if (!m_display) {
        printf("Error: couldn't connect to X server\n");
        exit(1);
    }

    m_rootWindow = DefaultRootWindow(m_display);

    GLint attributes[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 24,
        GLX_DOUBLEBUFFER,
        None };
    m_visualInfo = glXChooseVisual(m_display, 0, attributes);
    if (!m_visualInfo) {
        printf("Error: couldn't get a visual\n");
        exit(1);
    }

    m_colormap = XCreateColormap(m_display, m_rootWindow, m_visualInfo->visual, AllocNone);

    XSetWindowAttributes setAttributes;
    setAttributes.colormap = m_colormap;
    setAttributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask;

    m_eventSource = new XlibEventSource(m_display, this);

    m_window = XCreateWindow(m_display, m_rootWindow, 0, 0, m_width, m_height, 0, m_visualInfo->depth, InputOutput, m_visualInfo->visual, CWColormap | CWEventMask, &setAttributes);

    wmDeleteMessageAtom = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(m_display, m_window, &wmDeleteMessageAtom, 1);

    XMapWindow(m_display, m_window);
    XStoreName(m_display, m_window, "MiniBrowser");

    m_glContext = glXCreateContext(m_display, m_visualInfo, 0, GL_TRUE);
    glXMakeCurrent(m_display, m_window, m_glContext);

    glEnable(GL_DEPTH_TEST);
}

LinuxWindow::~LinuxWindow()
{
    delete m_eventSource;
    glXMakeCurrent(m_display, None, 0);
    glXDestroyContext(m_display, m_glContext);
    XDestroyWindow(m_display, m_window);
    XCloseDisplay(m_display);
}

std::pair<int, int> LinuxWindow::size() const
{
    return std::make_pair(m_width, m_height);
}

void LinuxWindow::makeCurrent()
{
    glXMakeCurrent(m_display, m_window, m_glContext);
}

void LinuxWindow::swapBuffers()
{
    glXSwapBuffers(m_display, m_window);
}

void LinuxWindow::handleXEvent(const XEvent& event)
{
    if (event.type == ConfigureNotify) {
        updateSizeIfNeeded(event.xconfigure.width, event.xconfigure.height);
        return;
    }

    if (!m_client)
        return;

    switch (event.type) {
    case Expose:
        m_client->handleExposeEvent();
        break;
    case KeyPress:
        m_client->handleKeyPressEvent(reinterpret_cast<const XKeyPressedEvent&>(event));
        break;
    case KeyRelease:
        m_client->handleKeyReleaseEvent(reinterpret_cast<const XKeyReleasedEvent&>(event));
        break;
    case ButtonPress:
        m_client->handleButtonPressEvent(reinterpret_cast<const XButtonPressedEvent&>(event));
        break;
    case ButtonRelease:
        m_client->handleButtonReleaseEvent(reinterpret_cast<const XButtonReleasedEvent&>(event));
        break;
    case ClientMessage:
        if (event.xclient.data.l[0] == wmDeleteMessageAtom)
            m_client->handleClosed();
        break;
    case MotionNotify:
        m_client->handlePointerMoveEvent(reinterpret_cast<const XPointerMovedEvent&>(event));
        break;
    }
}

void LinuxWindow::updateSizeIfNeeded(int width, int height)
{
    if (width == m_width && height == m_height)
        return;

    m_width = width;
    m_height = height;

    if (!m_client)
        return;

    m_client->handleSizeChanged(width, height);
}
