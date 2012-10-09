#include "config.h"
#include "GLContextFromCurrentEGL.h"

#include "OpenGLShims.h"

namespace WebCore {

PassOwnPtr<GLContextFromCurrentEGL> GLContextFromCurrentEGL::createFromCurrentGLContext()
{
    if (!initialize())
        return nullptr;
    return adoptPtr(new GLContextFromCurrentEGL());
}

GLContextFromCurrentEGL::GLContextFromCurrentEGL()
    : m_display(eglGetCurrentDisplay())
    , m_surface(eglGetCurrentSurface(EGL_DRAW))
    , m_context(eglGetCurrentContext())
{

}

bool GLContextFromCurrentEGL::makeContextCurrent()
{
    GLContext::makeContextCurrent();
    return eglMakeCurrent(m_display, m_surface, m_surface, m_context);
}

bool GLContextFromCurrentEGL::initialize()
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
