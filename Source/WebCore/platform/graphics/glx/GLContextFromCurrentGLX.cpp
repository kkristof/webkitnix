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

// NOTE: This class exists to avoid making changes on the existing
// GLContextGLX until we start upstreaming our changes. The
// functionality here should be integrated into that class.

#include "config.h"
#include "GLContextFromCurrentGLX.h"

#include "OpenGLShims.h"

namespace WebCore {

PassOwnPtr<GLContextFromCurrentGLX> GLContextFromCurrentGLX::createFromCurrentGLContext()
{
    if (!initialize())
        return nullptr;
    return adoptPtr(new GLContextFromCurrentGLX());
}

GLContextFromCurrentGLX::GLContextFromCurrentGLX()
    : m_display(glXGetCurrentDisplay())
    , m_drawable(glXGetCurrentDrawable())
    , m_context(glXGetCurrentContext())
{

}

bool GLContextFromCurrentGLX::makeContextCurrent()
{
    GLContext::makeContextCurrent();
    return glXMakeContextCurrent(m_display, m_drawable, m_drawable, m_context);
}

bool GLContextFromCurrentGLX::initialize()
{
    static bool initialized = false;
    static bool success = true;
    if (!initialized) {
        success = initializeOpenGLShims();
        initialized = true;
    }
    return success;
}

} // namespace WebCore
