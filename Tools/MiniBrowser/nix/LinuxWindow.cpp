#include "LinuxWindow.h"

#include <cstdio>
#include <cstdlib>
#include <glib.h>

Atom wmDeleteMessageAtom;

static void error(const char* message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

LinuxWindow::LinuxWindow(LinuxWindowClient* client)
    : m_client(client)
    , m_eventSource(0)
    , m_display(XOpenDisplay(0))
    , m_width(800)
    , m_height(600)
{
    if (!m_display)
        error("Error: couldn't connect to X server\n");

    m_eglDisplay = eglGetDisplay(m_display);
    if (!m_eglDisplay)
        error("Error: eglGetDisplay() failed\n");

    if (!eglInitialize(m_eglDisplay, 0, 0))
        error("Error: eglInitialize() failed\n");

    static const EGLint attributes[] = {
        EGL_DEPTH_SIZE, 24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLConfig config;
    EGLint configCount;
    if (!eglChooseConfig(m_eglDisplay, attributes, &config, 1, &configCount))
        error("Error: couldn't get an EGL visual config\n");

    EGLint visualID;
    if (!eglGetConfigAttrib(m_eglDisplay, config, EGL_NATIVE_VISUAL_ID, &visualID))
        error("Error: couldn't get an EGL visual config\n");

    m_eventSource = new XlibEventSource(m_display, this);
    m_window = createXWindow(visualID);

    eglBindAPI(EGL_OPENGL_API);
    m_context = eglCreateContext(m_eglDisplay, config, EGL_NO_CONTEXT, 0);
    if (!m_context)
        error("Error: eglCreateContext failed\n");

    m_surface = eglCreateWindowSurface(m_eglDisplay, config, m_window, 0);

    makeCurrent();
    glEnable(GL_DEPTH_TEST);
}

LinuxWindow::~LinuxWindow()
{
    delete m_eventSource;

    eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(m_eglDisplay, m_context);
    eglDestroySurface(m_eglDisplay, m_surface);
    eglTerminate(m_eglDisplay);

    XDestroyWindow(m_display, m_window);
    XCloseDisplay(m_display);
}

std::pair<int, int> LinuxWindow::size() const
{
    return std::make_pair(m_width, m_height);
}

void LinuxWindow::makeCurrent()
{
    eglMakeCurrent(m_eglDisplay, m_surface, m_surface, m_context);
}

void LinuxWindow::swapBuffers()
{
    eglSwapBuffers(m_eglDisplay, m_surface);
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

Window LinuxWindow::createXWindow(EGLint visualID)
{
    XVisualInfo visualInfoTemplate;
    int visualInfoCount;
    visualInfoTemplate.visualid = visualID;
    XVisualInfo* visualInfo = XGetVisualInfo(m_display, VisualIDMask, &visualInfoTemplate, &visualInfoCount);
    if (!visualInfo)
        error("Error: couldn't get X visual\n");

    Window rootWindow = DefaultRootWindow(m_display);

    XSetWindowAttributes setAttributes;
    setAttributes.colormap = XCreateColormap(m_display, rootWindow, visualInfo->visual, AllocNone);
    setAttributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask;

    Window window = XCreateWindow(m_display, rootWindow, 0, 0, m_width, m_height, 0, visualInfo->depth, InputOutput, visualInfo->visual, CWColormap | CWEventMask, &setAttributes);
    XFree(visualInfo);

    wmDeleteMessageAtom = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(m_display, window, &wmDeleteMessageAtom, 1);

    XMapWindow(m_display, window);
    XStoreName(m_display, window, "MiniBrowser");

    return window;
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
