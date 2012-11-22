#ifndef GLUtilities_h
#define GLUtilities_h

#if USE(OPENGL_ES_2)
#include <GLES2/gl2.h>
#elif USE(OPENGL)
#include <GL/gl.h>
#endif

#if USE(EGL)
#include <EGL/egl.h>
#else
#include <X11/Xlib.h>
#include <GL/glx.h>
#endif

namespace TestWebKitAPI {
namespace Util {

class GLOffscreenBuffer {
public:
    GLOffscreenBuffer(unsigned width, unsigned height);
    ~GLOffscreenBuffer();

    bool makeCurrent();
    void dumpToPng(const char* fileName);

private:
    int m_width;
    int m_height;
#if USE(EGL)
    EGLDisplay m_display;
    EGLContext m_context;
    EGLSurface m_surface;
#else
    Display* m_display;
    GLXPbuffer m_pbuffer;
    GLXContext m_context;
#endif
};

} // namespace Util
} // namespace TestWebKitAPI

#endif // GLUtilities_h
