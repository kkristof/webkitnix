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
#include <wtf/CurrentTime.h>
#include <wtf/OwnArrayPtr.h>
#include <wtf/PassOwnArrayPtr.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>
#include <API/nix/WebView.h>
#include <API/C/WKAPICast.h>

namespace WTR {

// Key event location code defined in DOM Level 3.
enum KeyLocationCode {
    DOMKeyLocationStandard      = 0x00,
    DOMKeyLocationLeft          = 0x01,
    DOMKeyLocationRight         = 0x02,
    DOMKeyLocationNumpad        = 0x03
};

// Unicode values for Apple non-letter keys.
#define KEYCODE_DEL         127
#define KEYCODE_BACKSPACE   8
#define KEYCODE_UPARROW     0xf700
#define KEYCODE_DOWNARROW   0xf701
#define KEYCODE_LEFTARROW   0xf702
#define KEYCODE_RIGHTARROW  0xf703

struct WTREvent {
    Nix::InputEvent* event;
    int delay;
};

static WTREvent eventQueue[1024];
static unsigned endOfQueue;
static bool isReplayingEvents;

static std::vector<Nix::TouchPoint> touchPoints;
static unsigned touchModifiers;
static int touchVerticalRadius;
static int touchHorizontalRadius;

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
    touchPoints.clear();
    touchModifiers = 0;
    touchVerticalRadius = 0;
    touchHorizontalRadius = 0;
}

static const double doubleClickInterval = 300;

static inline double convertToNixTimestamp(double ms)
{
    return ms / 1000.0;
}

void EventSenderProxy::updateClickCountForButton(int button)
{
    if (m_time - m_clickTime <= doubleClickInterval
        && m_position == m_clickPosition
        && button == m_clickButton) {
        ++m_clickCount;
        m_clickTime = m_time;
        return;
    }

    m_clickCount = 1;
    m_clickTime = m_time;
    m_clickPosition = m_position;
    m_clickButton = button;
}

static unsigned convertToNixModifiers(WKEventModifiers modifiersRef)
{
    unsigned modifiers = 0;

    if (modifiersRef & kWKEventModifiersControlKey)
        modifiers |= Nix::InputEvent::ControlKey;
    if (modifiersRef & kWKEventModifiersShiftKey)
        modifiers |= Nix::InputEvent::ShiftKey;
    if (modifiersRef & kWKEventModifiersAltKey)
        modifiers |= Nix::InputEvent::AltKey;
    if (modifiersRef & kWKEventModifiersMetaKey)
        modifiers |= Nix::InputEvent::MetaKey;

    return modifiers;
}

Nix::MouseEvent* EventSenderProxy::createMouseEvent(Nix::InputEvent::Type type, unsigned button, WKEventModifiers wkModifiers)
{
    Nix::MouseEvent* ev = new Nix::MouseEvent;
    ev->type = type;
    ev->modifiers = convertToNixModifiers(wkModifiers);
    ev->timestamp = convertToNixTimestamp(m_time);
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

Nix::KeyEvent* EventSenderProxy::createKeyEvent(Nix::InputEvent::Type type, unsigned code, unsigned nixModifiers, bool shouldUseUpperCase, bool isKeypad)
{
    Nix::KeyEvent* ev = new Nix::KeyEvent();
    ev->type = type;
    ev->key = static_cast<Nix::KeyEvent::Key>(code);
    ev->modifiers = nixModifiers;
    ev->timestamp = convertToNixTimestamp(m_time);
    ev->shouldUseUpperCase = shouldUseUpperCase;
    ev->isKeypad = isKeypad;
    return ev;
}

void EventSenderProxy::keyDown(WKStringRef keyRef, WKEventModifiers wkModifiers, unsigned location)
{
    WTF::String key = WebKit::toWTFString(keyRef);
    unsigned modifiers = convertToNixModifiers(wkModifiers);
    bool isKeypad = (location == DOMKeyLocationNumpad);
    bool shouldUseUpperCase = false;

    unsigned code = 0;
    if (key.length() == 1) {
        code = key[0];
        if (code == '\r')
            code = Nix::KeyEvent::Key_Return;
        else if (code == '\t') {
            code = Nix::KeyEvent::Key_Tab;
            if (modifiers == Nix::InputEvent::ShiftKey)
                code = Nix::KeyEvent::Key_Backtab;
        } else if (code == KEYCODE_DEL || code == KEYCODE_BACKSPACE) {
            code = Nix::KeyEvent::Key_Backspace;
            if (modifiers == Nix::InputEvent::AltKey)
                modifiers = Nix::InputEvent::ControlKey;
        } else if (code == 'o' && modifiers == Nix::InputEvent::ControlKey) {
            // Mimic the emacs ctrl-o binding on Mac by inserting a paragraph
            // separator and then putting the cursor back to its original
            // position. Allows us to pass emacs-ctrl-o.html
            code = '\n';
            modifiers = 0;
            sendOrQueueEvent(createKeyEvent(Nix::InputEvent::KeyDown, code, modifiers));
            sendOrQueueEvent(createKeyEvent(Nix::InputEvent::KeyUp, code, modifiers));
            code = Nix::KeyEvent::Key_Left;
        } else if (code == 'y' && modifiers == Nix::InputEvent::ControlKey)
            code = 'c';
        else if (code == 'k' && modifiers == Nix::InputEvent::ControlKey)
            code = 'x';
        else if (code == 'a' && modifiers == Nix::InputEvent::ControlKey) {
            code = Nix::KeyEvent::Key_Home;
            modifiers = 0;
        } else if (code == KEYCODE_LEFTARROW) {
            code = Nix::KeyEvent::Key_Left;
            if (modifiers & Nix::InputEvent::MetaKey) {
                modifiers -= Nix::InputEvent::MetaKey;
                code = Nix::KeyEvent::Key_Home;
            }
        } else if (code == KEYCODE_RIGHTARROW) {
            code = Nix::KeyEvent::Key_Right;
            if (modifiers & Nix::InputEvent::MetaKey) {
                modifiers -= Nix::InputEvent::MetaKey;
                code = Nix::KeyEvent::Key_End;
            }
        } else if (code == KEYCODE_UPARROW) {
            code = Nix::KeyEvent::Key_Up;
            if (modifiers & Nix::InputEvent::MetaKey) {
                modifiers -= Nix::InputEvent::MetaKey;
                code = Nix::KeyEvent::Key_PageUp;
            }
        } else if (code == KEYCODE_DOWNARROW) {
            code = Nix::KeyEvent::Key_Down;
            if (modifiers & Nix::InputEvent::MetaKey) {
                modifiers -= Nix::InputEvent::MetaKey;
                code = Nix::KeyEvent::Key_PageDown;
            }
        } else {
            shouldUseUpperCase = isASCIIUpper(code);
            code = toASCIIUpper(code);
            if (shouldUseUpperCase)
                modifiers |= Nix::InputEvent::ShiftKey;
        }
    } else {
        if (key.startsWith('F') && key.length() <= 3) {
            key.remove(0, 1);
            int functionKey = key.toInt();
            ASSERT(functionKey >= 1 && functionKey <= 35);
            code = Nix::KeyEvent::Key_F1 + (functionKey - 1);
        } else if (key == "leftArrow")
            code = Nix::KeyEvent::Key_Left;
        else if (key == "rightArrow")
            code = Nix::KeyEvent::Key_Right;
        else if (key == "upArrow")
            code = Nix::KeyEvent::Key_Up;
        else if (key == "downArrow")
            code = Nix::KeyEvent::Key_Down;
        else if (key == "pageUp")
            code = Nix::KeyEvent::Key_PageUp;
        else if (key == "pageDown")
            code = Nix::KeyEvent::Key_PageDown;
        else if (key == "home")
            code = Nix::KeyEvent::Key_Home;
        else if (key == "end")
            code = Nix::KeyEvent::Key_End;
        else if (key == "insert")
            code = Nix::KeyEvent::Key_Insert;
        else if (key == "delete")
            code = Nix::KeyEvent::Key_Delete;
        else if (key == "printScreen")
            code = Nix::KeyEvent::Key_Print;
        else if (key == "menu")
            code = Nix::KeyEvent::Key_Menu;
    }

    sendOrQueueEvent(createKeyEvent(Nix::InputEvent::KeyDown, code, modifiers, shouldUseUpperCase, isKeypad));
    sendOrQueueEvent(createKeyEvent(Nix::InputEvent::KeyUp, code, modifiers, shouldUseUpperCase, isKeypad));
}

#if ENABLE(TOUCH_EVENTS)
void EventSenderProxy::addTouchPoint(int x, int y)
{
    unsigned lowestId = 0;
    for (size_t i = 0; i < touchPoints.size(); i++) {
        if (touchPoints[i].id == lowestId)
            lowestId++;
    }

    Nix::TouchPoint touch;
    touch.id = lowestId;
    touch.state = Nix::TouchPoint::TouchPressed;
    touch.x = x;
    touch.y = y;
    touch.globalX = x;
    touch.globalY = y;
    touch.verticalRadius = touchVerticalRadius;
    touch.horizontalRadius = touchHorizontalRadius;
    touch.rotationAngle = 0.0;
    touch.pressure = 1.0;
    touchPoints.push_back(touch);
}

void EventSenderProxy::updateTouchPoint(int index, int x, int y)
{
    ASSERT(index >= 0 && index < currentTouchPoints.size());

    Nix::TouchPoint& touch = touchPoints[index];
    touch.state = Nix::TouchPoint::TouchMoved;
    touch.x = x;
    touch.y = y;
    touch.globalX = x;
    touch.globalY = y;
    touch.verticalRadius = touchVerticalRadius;
    touch.horizontalRadius = touchHorizontalRadius;
}

void EventSenderProxy::releaseTouchPoint(int index)
{
    ASSERT(index >= 0 && index < currentTouchPoints.size());

    Nix::TouchPoint& touch = touchPoints[index];
    touch.state = Nix::TouchPoint::TouchReleased;
}

void EventSenderProxy::cancelTouchPoint(int index)
{
    ASSERT(index >= 0 && index < currentTouchPoints.size());

    Nix::TouchPoint& touch = touchPoints[index];
    touch.state = Nix::TouchPoint::TouchCancelled;
}

void EventSenderProxy::setTouchModifier(WKEventModifiers wkModifier, bool enable)
{
    unsigned modifier = convertToNixModifiers(wkModifier);
    if (enable)
        touchModifiers |= modifier;
    else
        touchModifiers &= ~modifier;
}

void EventSenderProxy::sendCurrentTouchEvent(Nix::InputEvent::Type touchType)
{
    Nix::TouchEvent* ev = new Nix::TouchEvent();
    ev->modifiers = touchModifiers;
    ev->timestamp = convertToNixTimestamp(m_time);
    ev->type = touchType;
    ev->touchPoints = touchPoints;
    sendOrQueueEvent(ev);

    for (size_t i = 0; i < touchPoints.size(); ++i) {
        Nix::TouchPoint& touchPoint = touchPoints[i];
        if (touchPoint.state == Nix::TouchPoint::TouchReleased) {
            touchPoints.erase(touchPoints.begin() + i);
            --i;
        } else
            touchPoint.state = Nix::TouchPoint::TouchStationary;
    }
}

void EventSenderProxy::touchStart()
{
    sendCurrentTouchEvent(Nix::InputEvent::TouchStart);
}

void EventSenderProxy::touchMove()
{
    sendCurrentTouchEvent(Nix::InputEvent::TouchMove);
}

void EventSenderProxy::touchEnd()
{
    sendCurrentTouchEvent(Nix::InputEvent::TouchEnd);
}

void EventSenderProxy::touchCancel()
{
    sendCurrentTouchEvent(Nix::InputEvent::TouchCancel);
}

void EventSenderProxy::clearTouchPoints()
{
    touchPoints.clear();
}

void EventSenderProxy::setTouchPointRadius(int radiusX, int radiusY)
{
    touchHorizontalRadius = radiusX;
    touchVerticalRadius = radiusY;
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

void EventSenderProxy::continuousMouseScrollBy(int horizontal, int vertical, bool paged)
{
    notImplemented();
}

}
