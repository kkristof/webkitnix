#include "LinuxWindow.h"
#include <Ecore_X.h>

// TODO: We are assuming Xlib is being used as backend for ecore-x, we
// need it because of GLX. We probably should enforce this.
LinuxWindow::LinuxWindow(LinuxWindowClient* client)
    : m_display(reinterpret_cast<Display*>(ecore_x_display_get()))
    , m_visualInfo(0)
    , m_client(client)
    , m_width(800)
    , m_height(600)
{
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
    setAttributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask;

    m_window = XCreateWindow(m_display, m_rootWindow, 0, 0, m_width, m_height, 0, m_visualInfo->depth, InputOutput, m_visualInfo->visual, CWColormap | CWEventMask, &setAttributes);

    ecore_event_handler_add(ECORE_X_EVENT_ANY, ecoreXEventAnyCallback, reinterpret_cast<void*>(this));

    XMapWindow(m_display, m_window);
    XStoreName(m_display, m_window, "MiniBrowser");

    m_glContext = glXCreateContext(m_display, m_visualInfo, 0, GL_TRUE);
    glXMakeCurrent(m_display, m_window, m_glContext);

    glEnable(GL_DEPTH_TEST);
}

LinuxWindow::~LinuxWindow()
{
    glXMakeCurrent(m_display, None, 0);
    glXDestroyContext(m_display, m_glContext);
    XDestroyWindow(m_display, m_window);
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

Eina_Bool LinuxWindow::ecoreXEventAnyCallback(void* data, int eventType, void* event)
{
    XEvent* ev = reinterpret_cast<XEvent*>(event);
    LinuxWindow* window = reinterpret_cast<LinuxWindow*>(data);

    window->handleXEvent(ev);

    return ECORE_CALLBACK_DONE;
}

void LinuxWindow::handleXEvent(XEvent* event)
{
    if (event->type == ConfigureNotify) {
        updateSizeIfNeeded(event->xconfigure.width, event->xconfigure.height);
        return;
    }

    if (!m_client)
        return;

    switch (event->type) {
    case Expose:
        m_client->handleExposeEvent();
        break;
    case KeyPress:
        m_client->handleKeyPressEvent(*reinterpret_cast<XKeyPressedEvent*>(event));
        break;
    case KeyRelease:
        m_client->handleKeyReleaseEvent(*reinterpret_cast<XKeyReleasedEvent*>(event));
        break;
    case ButtonPress:
        m_client->handleButtonPressEvent(*reinterpret_cast<XButtonPressedEvent*>(event));
        break;
    case ButtonRelease:
        m_client->handleButtonReleaseEvent(*reinterpret_cast<XButtonReleasedEvent*>(event));
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
