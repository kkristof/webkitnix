#include "config.h"
#include "GLContextNix.h"

#include "OpenGLShims.h"

namespace WebCore {

PassOwnPtr<GLContextNix> GLContextNix::createFromCurrentGLContext() {
    static bool initialized = false;
    static bool success = true;
    if (!initialized) {
        success = initializeOpenGLShims();
        initialized = true;
    }
    if (!success)
        return nullptr;
    return adoptPtr(new GLContextNix());
}

} // namespace WebCore
