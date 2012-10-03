#ifndef GLContextFromCurrentGLX_h
#define GLContextFromCurrentGLX_h

#include "GLContext.h"
#include "NotImplemented.h"
#include <GL/glx.h>

namespace WebCore {

// NOTE: This class exists to avoid making changes on the existing
// GLContextGLX until we start upstreaming our changes. The
// functionality here should be integrated into that class.

class GLContextFromCurrentGLX : public GLContext {
    WTF_MAKE_NONCOPYABLE(GLContextFromCurrentGLX);
public:
    static PassOwnPtr<GLContextFromCurrentGLX> createFromCurrentGLContext();

    virtual bool makeContextCurrent();

    // These are not used by Nix.
    virtual void swapBuffers() { notImplemented(); }
    virtual IntSize defaultFrameBufferSize() { notImplemented(); return IntSize(); }

    // TODO: This is not used in GLContext interface, it's used by
    // GLContextGLX only as an implementation detail.
    virtual bool canRenderToDefaultFramebuffer() { return false; }

    // TODO: Used only as a key in HashMaps, see if we can change WebKit code to not
    // rely on this anymore (at least in our platform).
    virtual PlatformGraphicsContext3D platformContext() { return this; }

private:
    static bool initialize();

    GLContextFromCurrentGLX();

    Display* m_display;
    GLXDrawable m_drawable;
    GLXContext m_context;
};

} // namespace WebCore

#endif // GLContextFromCurrentGLX_h
