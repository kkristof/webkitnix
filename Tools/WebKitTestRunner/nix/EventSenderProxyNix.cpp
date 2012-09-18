/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Zan Dobersek <zandobersek@gmail.com>
 * Copyright (C) 2009 Holger Hans Peter Freyther
 * Copyright (C) 2010 Igalia S.L.
 * Copyright (C) 2011 ProFUSION Embedded Systems
 * Copyright (C) 2012 Samsung Electronics
 * Copyright (C) 2012 INdT - Instituto Nokia de Tecnologia
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "EventSenderProxy.h"

#include "NotImplemented.h"
#include "PlatformWebView.h"
#include "TestController.h"

#include <unistd.h>
#include <wtf/OwnArrayPtr.h>
#include <wtf/PassOwnArrayPtr.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>
#include <API/nix/WebView.h>

namespace WTR {

struct WTREvent {
    Nix::InputEvent* event;
    int delay;
};

static WTREvent eventQueue[1024];
static unsigned endOfQueue;
static bool isReplayingEvents;

EventSenderProxy::EventSenderProxy(TestController* testController)
    : m_testController(testController)
    , m_time(0)
    , m_leftMouseButtonDown(false)
    , m_clickCount(0)
    , m_clickTime(0)
    , m_clickButton(0)
{
    memset(eventQueue, 0, sizeof(eventQueue));
    endOfQueue = 0;
    isReplayingEvents = false;
}

void EventSenderProxy::updateClickCountForButton(int button)
{
    if (m_time - m_clickTime < 1 && m_position == m_clickPosition && button == m_clickButton) {
        ++m_clickCount;
        m_clickTime = m_time;
        return;
    }

    m_clickCount = 1;
    m_clickTime = m_time;
    m_clickPosition = m_position;
    m_clickButton = button;
}

Nix::MouseEvent* EventSenderProxy::createMouseEvent(Nix::InputEvent::Type type, unsigned button, WKEventModifiers wkModifiers)
{
    Nix::MouseEvent* ev = new Nix::MouseEvent;
    ev->type = type;
    ev->modifiers = wkModifiers;
    ev->timestamp = m_time;
    ev->button = (Nix::MouseEvent::Button) button;
    ev->x = m_position.x;
    ev->y = m_position.y;
    ev->globalX = m_position.x;
    ev->globalY = m_position.y;
    ev->clickCount = m_clickCount;
    return ev;
}

void EventSenderProxy::mouseDown(unsigned button, WKEventModifiers wkModifiers)
{
    updateClickCountForButton(button);

    Nix::InputEvent* ev = createMouseEvent(Nix::InputEvent::MouseDown, button, wkModifiers);
    sendOrQueueEvent(ev);
}

void EventSenderProxy::mouseUp(unsigned button, WKEventModifiers wkModifiers)
{
    Nix::InputEvent* ev = createMouseEvent(Nix::InputEvent::MouseUp, button, wkModifiers);
    m_clickPosition = m_position;
    m_clickTime = m_time;

    sendOrQueueEvent(ev);
}

void EventSenderProxy::mouseMoveTo(double x, double y)
{
    m_position.x = x;
    m_position.y = y;

    Nix::InputEvent* ev = createMouseEvent(Nix::InputEvent::MouseMove, 0, 0);
    sendOrQueueEvent(ev);
}

void EventSenderProxy::mouseScrollBy(int horizontal, int vertical)
{
    notImplemented();
}

void EventSenderProxy::leapForward(int milliseconds)
{
    eventQueue[endOfQueue].delay = milliseconds;
    m_time += milliseconds;
}

void EventSenderProxy::keyDown(WKStringRef keyRef, WKEventModifiers wkModifiers, unsigned location)
{
    notImplemented();
}

#if ENABLE(TOUCH_EVENTS)
void EventSenderProxy::addTouchPoint(int x, int y)
{
    notImplemented();
}

void EventSenderProxy::updateTouchPoint(int index, int x, int y)
{
    notImplemented();
}

void EventSenderProxy::setTouchModifier(WKEventModifiers modifier, bool enable)
{
    notImplemented();
}

void EventSenderProxy::touchStart()
{
    notImplemented();
}

void EventSenderProxy::touchMove()
{
    notImplemented();
}

void EventSenderProxy::touchEnd()
{
    notImplemented();
}

void EventSenderProxy::touchCancel()
{
    notImplemented();
}

void EventSenderProxy::clearTouchPoints()
{
    notImplemented();
}

void EventSenderProxy::releaseTouchPoint(int index)
{
    notImplemented();
}

void EventSenderProxy::cancelTouchPoint(int index)
{
    notImplemented();
}

void EventSenderProxy::setTouchPointRadius(int radiusX, int radiusY)
{
    notImplemented();
}

#endif

static void dispatchAndDeleteEvent(TestController* testController, Nix::InputEvent* event)
{
    PlatformWKView webView = testController->mainWebView()->platformView();
    webView->sendEvent(*event);
    delete event;
}

void EventSenderProxy::sendOrQueueEvent(Nix::InputEvent* event)
{
    if (!endOfQueue && !eventQueue[endOfQueue].delay) {
        dispatchAndDeleteEvent(m_testController, event);
        return;
    }
    eventQueue[endOfQueue++].event = event;
    replaySavedEvents();
}

void EventSenderProxy::replaySavedEvents()
{
    if (isReplayingEvents)
        return;

    isReplayingEvents = true;
    int i = 0;

    while (i < endOfQueue) {
        WTREvent& ev = eventQueue[i];
        if (ev.delay)
            usleep(ev.delay * 1000);
        i++;

        dispatchAndDeleteEvent(m_testController, ev.event);
        ev.event = 0;
        ev.delay = 0;
    }

    endOfQueue = 0;
    isReplayingEvents = false;
}

}
