#include "config.h"
#include "GLContextEGL.h"

#include "OpenGLShims.h"

namespace WebCore {

PassOwnPtr<GLContextEGL> GLContextEGL::createFromCurrentGLContext()
{
    if (!initialize())
        return nullptr;
    return adoptPtr(new GLContextEGL());
}

GLContextEGL::GLContextEGL()
    : m_display(eglGetCurrentDisplay())
    , m_surface(eglGetCurrentSurface(EGL_DRAW))
    , m_context(eglGetCurrentContext())
{

}

bool GLContextEGL::makeContextCurrent()
{
    GLContext::makeContextCurrent();
    return eglMakeCurrent(m_display, m_surface, m_surface, m_context);
}

bool GLContextEGL::initialize()
{
    static bool initialized = false;
    static bool success = true;
    if (!initialized) {
        success = initializeOpenGLShims();
        initialized = true;
    }
    return success;
}

} // namespace WebCore
