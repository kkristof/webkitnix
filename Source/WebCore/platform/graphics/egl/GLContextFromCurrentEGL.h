/*
 * Copyright (C) 2012-2013 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
    virtual void waitNative() { return; }

private:
    static bool initialize();

    GLContextFromCurrentEGL();

    EGLDisplay m_display;
    EGLSurface m_surface;
    EGLContext m_context;
};

} // namespace WebCore

#endif // GLContextFromCurrentEGL_h
