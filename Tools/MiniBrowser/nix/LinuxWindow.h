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

#ifndef LinuxWindow_h
#define LinuxWindow_h

#include "XlibEventSource.h"
#include <WebKit2/WKGeometry.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <utility>

void fatalError(const char* message);

class LinuxWindowClient {
public:
    virtual ~LinuxWindowClient() { };
    virtual void handleExposeEvent() = 0;
    virtual void handleKeyPressEvent(const XKeyPressedEvent&) = 0;
    virtual void handleKeyReleaseEvent(const XKeyReleasedEvent&) = 0;
    virtual void handleButtonPressEvent(const XButtonPressedEvent&) = 0;
    virtual void handleButtonReleaseEvent(const XButtonReleasedEvent&) = 0;
    virtual void handlePointerMoveEvent(const XPointerMovedEvent&) = 0;

    virtual void handleSizeChanged(int width, int height) = 0;
    virtual void handleClosed() = 0;
};

class LinuxWindow : public XlibEventSource::Client {
public:
    LinuxWindow(LinuxWindowClient*, int width, int height);
    ~LinuxWindow();

    WKSize size() const;

    void makeCurrent();
    void swapBuffers();

    LinuxWindowClient* client() const { return m_client; }
    Display* display() const { return m_display; }

private:
    struct GLContextData;

    VisualID setupXVisualID();
    void createGLContext();
    void destroyGLContext();

    // XlibEventSource::Client.
    virtual void handleXEvent(const XEvent&);

    Window createXWindow(VisualID visualID);
    void updateSizeIfNeeded(int width, int height);

    LinuxWindowClient* m_client;
    XlibEventSource* m_eventSource;

    Display* m_display;
    Window m_window;

    GLContextData* m_glContextData;

    int m_width;
    int m_height;
};

#endif // LinuxWindow_h
