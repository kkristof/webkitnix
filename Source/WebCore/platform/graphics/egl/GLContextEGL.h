#ifndef GLContextEGL_h
#define GLContextEGL_h

#include "GLContext.h"
#include "NotImplemented.h"
#include <EGL/egl.h>

namespace WebCore {

class GLContextEGL : public GLContext {
    WTF_MAKE_NONCOPYABLE(GLContextEGL);
public:
    // TODO: Add support for Offscreen buffers.
    static PassOwnPtr<GLContextEGL> createFromCurrentGLContext();

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

    GLContextEGL();

    EGLDisplay m_display;
    EGLSurface m_surface;
    EGLContext m_context;
};

} // namespace WebCore

#endif // GLContextEGL_h
