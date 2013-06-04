/*
 * Copyright (C) 2013 University of Szeged
 * Copyright (C) 2013 Kristof Kosztyo
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
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PlatformContexGL2D_h
#define PlatformContexGL2D_h

#include "GL2DDefs.h"
#include "GraphicsTypes.h"
#include "FloatRect.h"
#include "IntPoint.h"
#include "IntRect.h"
#include "IntSize.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

namespace WebCore{

class Color;
class GLPlatformContext;
class GLPlatformSurface;
class NativeImageGL2D;

class PlatformContextGL2D : public RefCounted<PlatformContextGL2D> {
public:
    struct ShaderVariable {
        const char *name;
        GLint location;
    };

    static PassRefPtr<PlatformContextGL2D> create(NativeImageGL2D* targetTexture)
    {
        return adoptRef(new PlatformContextGL2D(targetTexture));
    }

    static PlatformContextGL2D* release(PassRefPtr<PlatformContextGL2D> context) { return context.leakRef(); }
    static void createGLContextIfNeed()
    {
        if (!s_offScreenContext)
            createGLContext();
    }
    static GLPlatformContext* offScreenContext() { return s_offScreenContext; }
    static GLPlatformSurface* offScreenSurface() { return s_offScreenSurface; }
    NativeImageGL2D* targetTexture() const { return m_targetTexture; }

    void save();
    void restore();

    void translate(float x, float y);
    float transform(int i) const { return m_state.m_transform[i]; }

    void clip(const FloatRect&);

    void copyRect(const FloatRect&, PlatformContextGL2D*, const FloatRect&, CompositeOperator, BlendMode);
    void fillRect(const FloatRect&, const Color&);

private:
    PlatformContextGL2D(NativeImageGL2D*);
    static void createGLContext();
    static bool supportsSubImage()
    {
#if USE(OPENGL_ES_2)
        return s_supportsSubImage;
#else
        return true;
#endif
    }

    static bool compileShaderInternal(GLuint*, const GLchar*, const GLchar*, ShaderVariable*);
    static bool compileShader(GLuint* compiledShader, const GLchar* vertexShaderSource, const GLchar* fragmentShaderSource, ShaderVariable* variables)
    {
        if (*compiledShader)
            return true;
        return compileShaderInternal(compiledShader, vertexShaderSource, fragmentShaderSource, variables);
    }

    struct GL2DState {
        float m_transform[6];
        IntRect m_scissorRect;
    };

    NativeImageGL2D* m_targetTexture;
    GL2DState m_state;
    Vector<GL2DState> m_stack;

    static GLPlatformContext* s_offScreenContext;
    static GLPlatformSurface* s_offScreenSurface;
#if USE(OPENGL_ES_2)
    static bool s_supportsSubImage;
#endif
};

} //namespace WebCore

#endif
