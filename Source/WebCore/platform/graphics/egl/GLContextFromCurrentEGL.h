#ifndef GLContextFromCurrentEGL_h
#define GLContextFromCurrentEGL_h

#include "GLContext.h"
#include "NotImplemented.h"
#include <EGL/egl.h>

namespace WebCore {

class GLContextFromCurrentEGL : public GLContext {
    WTF_MAKE_NONCOPYABLE(GLContextFromCurrentEGL);
public:
    // TODO: Add support for Offscreen buffers.
    static PassOwnPtr<GLContextFromCurrentEGL> createFromCurrentGLContext();

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

    GLContextFromCurrentEGL();

    EGLDisplay m_display;
    EGLSurface m_surface;
    EGLContext m_context;
};

} // namespace WebCore

#endif // GLContextFromCurrentEGL_h
