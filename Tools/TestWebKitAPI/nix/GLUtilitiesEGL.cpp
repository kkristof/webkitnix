#include "config.h"

#include "GLUtilities.h"
#include <iostream>

using std::cerr;

namespace TestWebKitAPI {
namespace Util {

GLOffscreenBuffer::GLOffscreenBuffer(unsigned width, unsigned height)
    : m_display(0)
    , m_context(0)
    , m_surface(0)
{
    m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!m_display) {
        cerr << "Error: eglGetDisplay()\n";
        return;
    }

    if (!eglInitialize(m_display, 0, 0)) {
        cerr << "Error: eglInitialize()\n";
        return;
    }

    static const EGLint attributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    EGLConfig config;
    EGLint configCount;
    if (!eglChooseConfig(m_display, attributes, &config, 1, &configCount)) {
        cerr << "Error: eglChooseConfig()\n";
        return;
    }

    if (configCount != 1) {
        cerr << "Error: couldn't get a valid config for EGL\n";
        return;
    }

    if (!eglBindAPI(EGL_OPENGL_API)) {
        cerr << "Error: eglBindAPI()\n";
        return;
    }

    m_context = eglCreateContext(m_display, config, EGL_NO_CONTEXT, 0);
    if (!m_context) {
        cerr << "Error: eglCreateContext()\n";
        return;
    }

    static const EGLint surfaceAttributes[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE
    };
    m_surface = eglCreatePbufferSurface(m_display, config, surfaceAttributes);
    if (!m_surface) {
        cerr << "Error: eglCreatePbufferSurface()\n";
        return;
    }
}

GLOffscreenBuffer::~GLOffscreenBuffer()
{
    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(m_display, m_context);
    eglDestroySurface(m_display, m_surface);
    eglTerminate(m_display);
}

bool GLOffscreenBuffer::wasCorrectlyInitialized()
{
    return m_surface;
}

bool GLOffscreenBuffer::makeCurrent()
{
    eglMakeCurrent(m_display, m_surface, m_surface, m_context);
}

} // namespace Util
} // namespace TestWebKitAPI
