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
#include "PlatformContextGL2D.h"

#include "Color.h"
#include "GLDefs.h"
#include "GLPlatformContext.h"
#include "GraphicsContext3D.h"
#include "NativeImageGL2D.h"
#include <stdio.h>

namespace WebCore {

static const GLfloat viewportSize = 2048;

GLPlatformContext* PlatformContextGL2D::s_offScreenContext = 0;
GLPlatformSurface* PlatformContextGL2D::s_offScreenSurface = 0;
#if USE(OPENGL_ES_2)
bool PlatformContextGL2D::s_supportsSubImage = false;
#endif

PlatformContextGL2D::PlatformContextGL2D(NativeImageGL2D* targetBuffer)
    : m_targetTexture(targetBuffer)
{
    m_state.m_transform[4] = 0;
    m_state.m_transform[5] = 0;
    createGLContextIfNeed();
    m_state.m_scissorRect.setWidth(targetBuffer->width());
    m_state.m_scissorRect.setHeight(targetBuffer->height());
}

void PlatformContextGL2D::createGLContext()
{
    s_offScreenContext = GLPlatformContext::createContext(GraphicsContext3D::RenderOffscreen).leakPtr();
    s_offScreenSurface = GLPlatformSurface::createOffScreenSurface().leakPtr();
    s_offScreenContext->initialize(s_offScreenSurface);
    glViewport(0, 0, viewportSize, viewportSize);

#if USE(OPENGL_ES_2)
    s_supportsSubImage = GLPlatformContext::supportsGLExtension("GL_EXT_unpack_subimage");
#endif
}

void PlatformContextGL2D::save()
{
    m_stack.append(m_state);
}

void PlatformContextGL2D::restore()
{
    m_state = m_stack.last();
    m_stack.removeLast();
}

void PlatformContextGL2D::translate(float x, float y)
{
    m_state.m_transform[4] += x;
    m_state.m_transform[5] += y;
}

void PlatformContextGL2D::clip(const FloatRect& rect)
{
    FloatRect transformedRect(rect);
    transformedRect.setX(transformedRect.x() + transform(4));
    transformedRect.setY(transformedRect.y() + transform(5));
    m_state.m_scissorRect.intersect(enclosingIntRect(transformedRect));
}

#define GL2D_PROGRAM_STR(...)  #__VA_ARGS__
#define GL2D_PROGRAM(...) GL2D_PROGRAM_STR(__VA_ARGS__)

#define PRECISION "#ifdef GL_ES\nprecision mediump float;\n#endif\n"

bool PlatformContextGL2D::compileShaderInternal(GLuint* compiledShader, const GLchar* vertexShaderSource, const GLchar* fragmentShaderSource, ShaderVariable* variables)
{
    GLuint shaderProgram = 0;
    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;
    GLint intValue;

    ASSERT(!*compiledShader);

    // Shader programs are zero terminated
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &intValue);
    if (intValue != GL_TRUE)
        goto error;

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &intValue);
    if (intValue != GL_TRUE)
        goto error;

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &intValue);
    if (intValue != GL_TRUE)
        goto error;

    // According to the specification, the shaders are kept
    // around until the program object is freed (reference counted).
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);
    while (variables->name) {
        ASSERT((variables->name[0] == 'u' || variables->name[0] == 'a') && variables->name[1] == '_');
        if (variables->name[0] == 'u') {
            variables->location = glGetUniformLocation(shaderProgram, variables->name);
        } else {
            variables->location = glGetAttribLocation(shaderProgram, variables->name);
            glEnableVertexAttribArray(variables->location);
        }
        variables++;
    }
    glUseProgram(0);

    *compiledShader = shaderProgram;
    return true;

error:
    if (vertexShader)
        glDeleteShader(vertexShader);
    if (fragmentShader)
        glDeleteShader(fragmentShader);
    if (shaderProgram)
        glDeleteProgram(shaderProgram);
    return false;
}

static GLubyte g_rectIndicies[] = { 0, 1, 2, 3 };
static GLbyte g_rectPosition[] = { -1, -1, -1, 1, 1, 1, 1, -1 };

static inline GLfloat mapToViewport(GLfloat coord)
{
    return (coord / (viewportSize / 2)) - 1;
}

static const GLchar* textureVertexShader = GL2D_PROGRAM(
    attribute vec2 a_textureCoord;
    attribute vec2 a_position;

    varying vec2 v_textureCoord;

    void main(void)
    {
        v_textureCoord = a_textureCoord;
        gl_Position = vec4(a_position, 0.0, 1.0);
    }
);

static const GLchar* textureFragmentShader = PRECISION GL2D_PROGRAM(
    uniform sampler2D u_texture;

    varying vec2 v_textureCoord;

    void main(void)
    {
        gl_FragColor = texture2D(u_texture, v_textureCoord);
    }
);

static PlatformContextGL2D::ShaderVariable g_textureShaderVariables[] = {
    { "u_texture", 0 },
    { "a_textureCoord", 0 },
    { "a_position", 0 },
    { 0, 0 },
};

static GLuint g_textureShader = 0;

void PlatformContextGL2D::copyRect(const FloatRect& destRect, PlatformContextGL2D* sourceContext, const FloatRect& srcRect, CompositeOperator op, BlendMode blendMode)
{
    GLfloat rectPosition[4 * 2];
    GLfloat texturePosition[4 * 2];

    if (!compileShader(&g_textureShader, textureVertexShader, textureFragmentShader, g_textureShaderVariables))
        return;
    GLint previousFbo = targetTexture()->bindFbo();
    IntRect destination = enclosingIntRect(destRect);

    glUseProgram(g_textureShader);

    glUniform1i(g_textureShaderVariables[0].location, 0);
    glActiveTexture(GL_TEXTURE0);
    sourceContext->targetTexture()->bindTexture();

    rectPosition[0] = mapToViewport(destRect.x() + transform(4));
    rectPosition[1] = mapToViewport(destRect.y() + destRect.height() + transform(5));
    rectPosition[2] = rectPosition[0];
    rectPosition[3] = mapToViewport(destRect.y() + transform(5));
    rectPosition[4] = mapToViewport(destRect.x() + destRect.width() + transform(4));
    rectPosition[5] = rectPosition[1];
    rectPosition[6] = rectPosition[4];
    rectPosition[7] = rectPosition[3];

    GLfloat width = sourceContext->targetTexture()->width();
    GLfloat height = sourceContext->targetTexture()->height();
    texturePosition[0] = srcRect.x() / width;
    texturePosition[1] = (srcRect.y() + srcRect.height()) / height;
    texturePosition[2] = texturePosition[0];
    texturePosition[3] = srcRect.y() / height;
    texturePosition[4] = (srcRect.x() + srcRect.width()) / width;
    texturePosition[5] = texturePosition[1];
    texturePosition[6] = texturePosition[4];
    texturePosition[7] = texturePosition[3];

    glVertexAttribPointer(g_textureShaderVariables[1].location, 2, GL_FLOAT, GL_FALSE, 0, texturePosition);
    glVertexAttribPointer(g_textureShaderVariables[2].location, 2, GL_FLOAT, GL_FALSE, 0, rectPosition);

    glEnable(GL_SCISSOR_TEST);
    glScissor(m_state.m_scissorRect.x(), m_state.m_scissorRect.y(), m_state.m_scissorRect.width(), m_state.m_scissorRect.height());

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, g_rectIndicies);
//    printf("Copy: \n");
//    printf("srcRect: %d %d %d %d | %p\n", enclosingIntRect(srcRect).x(), enclosingIntRect(srcRect).y(), enclosingIntRect(srcRect).width(), enclosingIntRect(srcRect).height(), sourceContext);
//    printf("destRect: %d %d %d %d | %p\n", m_state.m_scissorRect.x(), m_state.m_scissorRect.y(), m_state.m_scissorRect.width(), m_state.m_scissorRect.height(), this);
    glBindFramebuffer(GL_FRAMEBUFFER, previousFbo);
}

const GLchar* simpleVertexShader = GL2D_PROGRAM(
    attribute vec2 a_position;

    void main(void)
    {
        gl_Position = vec4(a_position, 0.0, 1.0);
    }
);

const GLchar* simpleFragmentShader = PRECISION GL2D_PROGRAM(
    uniform vec4 u_color;

    void main(void)
    {
        gl_FragColor = u_color;
    }
);

static PlatformContextGL2D::ShaderVariable g_simpleShaderVariables[] = {
    { "u_color", 0 },
    { "a_position", 0 },
    { 0, 0 },
};

static GLuint g_simpleShader = 0;

void PlatformContextGL2D::fillRect(const FloatRect& rect, const Color& color)
{
    GLfloat rectPosition[4 * 2];
    GLfloat r, g, b, a;

    color.getRGBA(r, g, b, a);
    IntRect targetRect = m_state.m_scissorRect;
    FloatPoint offset(rect.x() + transform(4), rect.y() + transform(5));
    IntRect fillRect = enclosingIntRect(FloatRect(offset, rect.size()));
    targetRect.intersect(fillRect);

//     printf("targetRect: %d %d %d %d | %p\n", targetRect.x(), targetRect.y(), targetRect.width(), targetRect.height(), this);
//     printf("rgba: %02x %02x %02x %02x\n", (int)(r*255),(int)(g*255),(int)(b*255),(int)(a*255));
    glEnable(GL_SCISSOR_TEST);
    glScissor(targetRect.x(), targetRect.y(), targetRect.width(), targetRect.height());
    rectPosition[0] = mapToViewport(targetRect.x());
    rectPosition[1] = mapToViewport(targetRect.y() + targetRect.height());
    rectPosition[2] = rectPosition[0];
    rectPosition[3] = mapToViewport(targetRect.y());
    rectPosition[4] = mapToViewport(targetRect.x() + targetRect.width());
    rectPosition[5] = rectPosition[1];
    rectPosition[6] = rectPosition[4];
    rectPosition[7] = rectPosition[3];

    if (!compileShader(&g_simpleShader, simpleVertexShader, simpleFragmentShader, g_simpleShaderVariables))
        return;

    glUseProgram(g_simpleShader);

    glUniform4f(g_simpleShaderVariables[0].location, r, g, b, a);
    glVertexAttribPointer(g_simpleShaderVariables[1].location, 2, GL_FLOAT, GL_FALSE, 0, rectPosition);

    GLint previousFbo = targetTexture()->bindFbo();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, g_rectIndicies);
}

} // namespace WebCore
