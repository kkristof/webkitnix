#ifndef GLUtilities_h
#define GLUtilities_h

#include <iostream>

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

struct RGBAPixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    bool operator==(const RGBAPixel& other) const { return r == other.r && b == other.b && g == other.g && a == other.a; }

    static RGBAPixel white() { RGBAPixel p = { 255, 255, 255, 255 }; return p; }
    static RGBAPixel black() { RGBAPixel p = { 0, 0, 0, 255 }; return p; }
    static RGBAPixel red() { RGBAPixel p = { 255, 0, 0, 255 }; return p; }
    static RGBAPixel green() { RGBAPixel p = { 0, 255, 0, 255 }; return p; }
    static RGBAPixel blue() { RGBAPixel p = { 0, 0, 255, 255 }; return p; }
};

// Gtest uses this to print RGBAPixels when tests fail.
inline std::ostream& operator<<(std::ostream& os, const RGBAPixel& p) {
    return os << std::hex << std::setfill('0')
              << std::setw(2) << unsigned(p.r)
              << std::setw(2) << unsigned(p.g)
              << std::setw(2) << unsigned(p.b)
              << std::setw(2) << unsigned(p.a);
}

class GLOffscreenBuffer {
public:
    GLOffscreenBuffer(unsigned width, unsigned height);
    ~GLOffscreenBuffer();

    bool makeCurrent();
    void dumpToPng(const char* fileName);
    RGBAPixel readPixelAtPoint(unsigned x, unsigned y);

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
