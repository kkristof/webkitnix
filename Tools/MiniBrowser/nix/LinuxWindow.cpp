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

#include "LinuxWindow.h"

#include <GL/gl.h>
#include <X11/Xutil.h>
#include <cstdio>
#include <cstdlib>
#include <glib.h>

void fatalError(const char* message)
{
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}

Atom wmDeleteMessageAtom;

LinuxWindow::LinuxWindow(LinuxWindowClient* client, int width, int height)
    : m_client(client)
    , m_eventSource(0)
    , m_display(XOpenDisplay(0))
    , m_glContextData(0)
    , m_width(width)
    , m_height(height)
{
    if (!m_display)
        fatalError("couldn't connect to X server\n");

    VisualID visualID = setupXVisualID();
    m_eventSource = new XlibEventSource(m_display, this);
    m_window = createXWindow(visualID);

    createGLContext();
    makeCurrent();
    glEnable(GL_DEPTH_TEST);
}

LinuxWindow::~LinuxWindow()
{
    delete m_eventSource;
    destroyGLContext();
    XDestroyWindow(m_display, m_window);
    XCloseDisplay(m_display);
}

WKSize LinuxWindow::size() const
{
    return WKSizeMake(m_width, m_height);
}

void LinuxWindow::handleXEvent(const XEvent& event)
{
    if (event.type == ConfigureNotify) {
        updateSizeIfNeeded(event.xconfigure.width, event.xconfigure.height);
        return;
    }

    if (!m_client)
        return;

    switch (event.type) {
    case Expose:
        m_client->handleExposeEvent();
        break;
    case KeyPress:
        m_client->handleKeyPressEvent(reinterpret_cast<const XKeyPressedEvent&>(event));
        break;
    case KeyRelease:
        m_client->handleKeyReleaseEvent(reinterpret_cast<const XKeyReleasedEvent&>(event));
        break;
    case ButtonPress:
        m_client->handleButtonPressEvent(reinterpret_cast<const XButtonPressedEvent&>(event));
        break;
    case ButtonRelease:
        m_client->handleButtonReleaseEvent(reinterpret_cast<const XButtonReleasedEvent&>(event));
        break;
    case ClientMessage:
        if (event.xclient.data.l[0] == wmDeleteMessageAtom)
            m_client->handleClosed();
        break;
    case MotionNotify:
        m_client->handlePointerMoveEvent(reinterpret_cast<const XPointerMovedEvent&>(event));
        break;
    }
}

Window LinuxWindow::createXWindow(VisualID visualID)
{
    XVisualInfo visualInfoTemplate;
    int visualInfoCount;
    visualInfoTemplate.visualid = visualID;
    XVisualInfo* visualInfo = XGetVisualInfo(m_display, VisualIDMask, &visualInfoTemplate, &visualInfoCount);
    if (!visualInfo)
        fatalError("couldn't get X visual\n");

    Window rootWindow = DefaultRootWindow(m_display);

    XSetWindowAttributes setAttributes;
    setAttributes.colormap = XCreateColormap(m_display, rootWindow, visualInfo->visual, AllocNone);
    setAttributes.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask;

    Window window = XCreateWindow(m_display, rootWindow, 0, 0, m_width, m_height, 0, visualInfo->depth, InputOutput, visualInfo->visual, CWColormap | CWEventMask, &setAttributes);
    XFree(visualInfo);

    wmDeleteMessageAtom = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(m_display, window, &wmDeleteMessageAtom, 1);

    XMapWindow(m_display, window);
    XStoreName(m_display, window, "MiniBrowser");

    return window;
}

void LinuxWindow::updateSizeIfNeeded(int width, int height)
{
    if (width == m_width && height == m_height)
        return;

    m_width = width;
    m_height = height;

    if (!m_client)
        return;

    m_client->handleSizeChanged(width, height);
}
