// NOTE: This class exists to avoid making changes on the existing
// GLContextGLX until we start upstreaming our changes. The
// functionality here should be integrated into that class.

#include "config.h"
#include "GLContextFromCurrentGLX.h"

#include "OpenGLShims.h"

namespace WebCore {

PassOwnPtr<GLContextFromCurrentGLX> GLContextFromCurrentGLX::createFromCurrentGLContext()
{
    if (!initialize())
        return nullptr;
    return adoptPtr(new GLContextFromCurrentGLX());
}

GLContextFromCurrentGLX::GLContextFromCurrentGLX()
    : m_display(glXGetCurrentDisplay())
    , m_drawable(glXGetCurrentDrawable())
    , m_context(glXGetCurrentContext())
{

}

bool GLContextFromCurrentGLX::makeContextCurrent()
{
    GLContext::makeContextCurrent();
    return glXMakeContextCurrent(m_display, m_drawable, m_drawable, m_context);
}

bool GLContextFromCurrentGLX::initialize()
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
