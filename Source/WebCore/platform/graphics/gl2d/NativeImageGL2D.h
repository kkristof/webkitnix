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

#ifndef NativeImageGL2D_h
#define NativeImageGL2D_h

#include "GL2DDefs.h"
#include "IntSize.h"
#include <wtf/Assertions.h>
#include <wtf/FastAllocBase.h>

namespace WebCore {

class NativeImageGL2D {
    WTF_MAKE_FAST_ALLOCATED;
public:

    static NativeImageGL2D* create(int width, int height, const void* buffer = 0, bool alpha = false)
    {
        return new NativeImageGL2D(width, height, buffer, alpha);
    }

    ~NativeImageGL2D();

    const IntSize& size() const { return m_size; }
    int width() const { return m_size.width(); }
    int height() const { return m_size.height(); }
    size_t sizeInBytes() const { return width() * height() * 4; }
    bool hasAlpha() const { return m_alpha; }
    GLuint fbo() const { return m_fbo; }

    void bindMeAsTexture()
    {
        ASSERT(m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
    }

    GLint bindFbo();
    GLint restoreFbo();

protected:
    NativeImageGL2D(int, int, const void* = 0, bool = false);

private:
    IntSize m_size;
    GLuint m_texture;
    GLuint m_fbo;
    GLint m_previousFbo;
    bool m_alpha;
};

}
#endif
