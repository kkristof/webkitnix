#ifndef GLContextNix_h
#define GLContextNix_h

#include "GLContext.h"

namespace WebCore {

class GLContextNix : public GLContext {
    WTF_MAKE_NONCOPYABLE(GLContextNix);
public:
    static PassOwnPtr<GLContextNix> createFromCurrentGLContext();

    // TODO: Not used by Nix. Some of this are GTK-only.
    virtual void swapBuffers() {}
    virtual bool canRenderToDefaultFramebuffer() { return false; }
    virtual IntSize defaultFrameBufferSize() { return IntSize(); }

    // TODO: Used only as a key in HashMaps, see if we can change WebKit code to not
    // rely on this anymore (at least in our platform).
    virtual PlatformGraphicsContext3D platformContext() { return this; }

private:
    GLContextNix() {}
};

} // namespace WebCore

#endif // GLContextNix_h
