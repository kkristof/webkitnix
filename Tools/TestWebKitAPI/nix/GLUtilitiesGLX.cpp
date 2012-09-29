#include "config.h"

#include "GLUtilities.h"
#include <X11/Xlib.h>
#include <iostream>

using std::cerr;

namespace TestWebKitAPI {
namespace Util {

GLOffscreenBuffer::GLOffscreenBuffer(unsigned width, unsigned height)
    : m_display(XOpenDisplay(0))
    , m_pbuffer(0)
    , m_context(0)
{
    if (!m_display) {
        cerr << "Error: XOpenDisplay()\n";
        return;
    }

    static const int configAttributes[] = {
        GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DOUBLEBUFFER, GL_FALSE,
        0
    };
    int configCount;
    GLXFBConfig* config = glXChooseFBConfig(m_display, 0, configAttributes, &configCount);
    if (!configCount) {
        cerr << "Error: glXChooseFBConfig()\n";
        XFree(config);
        XCloseDisplay(m_display);
        return;
    }

    static const int pbufferAttributes[] = {
        GLX_PBUFFER_WIDTH, width,
        GLX_PBUFFER_HEIGHT, height,
        0
    };
    m_pbuffer = glXCreatePbuffer(m_display, config[0], pbufferAttributes);
    if (!m_pbuffer) {
        cerr << "Error: glXCreatePbuffer()\n";
        XFree(config);
        XCloseDisplay(m_display);
        return;
    }

    m_context = glXCreateNewContext(m_display, config[0], GLX_RGBA_TYPE, 0, GL_TRUE);
    XFree(config);
    if (!m_context) {
        cerr << "Error: glXCreateNewContext()\n";
        glXDestroyPbuffer(m_display, m_pbuffer);
        XCloseDisplay(m_display);
        return;
    }
}

GLOffscreenBuffer::~GLOffscreenBuffer()
{
    if (!m_context)
        return;
    glXMakeCurrent(m_display, None, None);
    glXDestroyContext(m_display, m_context);
    glXDestroyPbuffer(m_display, m_pbuffer);
    XCloseDisplay(m_display);
}

bool GLOffscreenBuffer::wasCorrectlyInitialized()
{
    return m_context;
}

bool GLOffscreenBuffer::makeCurrent()
{
    glXMakeCurrent(m_display, m_pbuffer, m_context);
}

} // namespace Util
} // namespace TestWebKitAPI
