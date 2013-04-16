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

#include "config.h"
#include "NativeImageGL2D.h"
#include "PlatformContextGL2D.h"

#include "stdio.h"

namespace WebCore{

NativeImageGL2D::NativeImageGL2D(int width, int height, const void* buffer,  bool alpha)
    : m_size(width, height)
    , m_alpha(alpha)
    , m_fbo(0)
    , m_previousFbo(0)
    , m_texture(0)
{
    PlatformContextGL2D::createGLContextIfNeed();

    if (!width || !height) {
        m_size = IntSize(0, 0);
        return;
    }

    glGenTextures(1, &m_texture);
    if (!m_texture)
        CRASH();

    glBindTexture(GL_TEXTURE_2D, m_texture);
    // Note: mipmaps are not used.
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
}

NativeImageGL2D::~NativeImageGL2D()
{
    if (m_fbo)
        glDeleteFramebuffers(1, &m_fbo);
    if (m_texture)
        glDeleteTextures(1, &m_texture);
}

GLint NativeImageGL2D::bindFbo()
{
    GLint previousFbo;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFbo);

    if (m_fbo) {
        if (m_fbo != previousFbo)
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        return previousFbo;
    }
    // Create a framebuffer object.
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_previousFbo);
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    ASSERT(m_texture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
    return previousFbo;
}

GLint NativeImageGL2D::restoreFbo()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_previousFbo);
    return m_previousFbo;
}

} // namespace WebCore
