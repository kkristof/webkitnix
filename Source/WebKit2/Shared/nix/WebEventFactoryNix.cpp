/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "NotImplemented.h"
#include "WebEventFactoryNix.h"
#include "WindowsKeyboardCodes.h"

#include <WebCore/Scrollbar.h>
#include <wtf/text/WTFString.h>

using namespace WebCore;

namespace WebKit {

static WebEvent::Type convertToWebEventType(Nix::InputEvent::Type type)
{
    switch (type) {
    case Nix::InputEvent::MouseDown:
        return WebEvent::MouseDown;
    case Nix::InputEvent::MouseUp:
        return WebEvent::MouseUp;
    case Nix::InputEvent::MouseMove:
        return WebEvent::MouseMove;
    case Nix::InputEvent::Wheel:
        return WebEvent::Wheel;
    case Nix::InputEvent::KeyDown:
        return WebEvent::KeyDown;
    case Nix::InputEvent::KeyUp:
        return WebEvent::KeyUp;
    case Nix::InputEvent::TouchStart:
        return WebEvent::TouchStart;
    case Nix::InputEvent::TouchMove:
        return WebEvent::TouchMove;
    case Nix::InputEvent::TouchEnd:
        return WebEvent::TouchEnd;
    case Nix::InputEvent::TouchCancel:
        return WebEvent::TouchCancel;
    case Nix::InputEvent::GestureSingleTap:
        return WebEvent::GestureSingleTap;
    default:
        notImplemented();
    }
    return WebEvent::MouseMove;
}

static WebEvent::Modifiers convertToWebEventModifiers(unsigned modifiers)
{
    unsigned webModifiers = 0;
    if (modifiers & Nix::InputEvent::ShiftKey)
        webModifiers |= WebEvent::ShiftKey;
    if (modifiers & Nix::InputEvent::ControlKey)
        webModifiers |= WebEvent::ControlKey;
    if (modifiers & Nix::InputEvent::AltKey)
        webModifiers |= WebEvent::AltKey;
    if (modifiers & Nix::InputEvent::MetaKey)
        webModifiers |= WebEvent::MetaKey;
    if (modifiers & Nix::InputEvent::CapsLockKey)
        webModifiers |= WebEvent::CapsLockKey;
    return static_cast<WebEvent::Modifiers>(webModifiers);
}

static WebPlatformTouchPoint::TouchPointState convertToWebTouchState(const Nix::TouchPoint::TouchState& state)
{
    switch (state) {
    case Nix::TouchPoint::TouchReleased:
        return WebPlatformTouchPoint::TouchReleased;
    case Nix::TouchPoint::TouchPressed:
        return WebPlatformTouchPoint::TouchPressed;
    case Nix::TouchPoint::TouchMoved:
        return WebPlatformTouchPoint::TouchMoved;
    case Nix::TouchPoint::TouchStationary:
        return WebPlatformTouchPoint::TouchStationary;
    case Nix::TouchPoint::TouchCancelled:
        return WebPlatformTouchPoint::TouchCancelled;
    default:
        notImplemented();
    }
    return WebPlatformTouchPoint::TouchCancelled;
}

static WebMouseEvent::Button convertToWebMouseEventButton(Nix::MouseEvent::Button button)
{
    switch (button) {
    case Nix::MouseEvent::NoButton:
        return WebMouseEvent::NoButton;
    case Nix::MouseEvent::LeftButton:
        return WebMouseEvent::LeftButton;
    case Nix::MouseEvent::FourthButton:
    case Nix::MouseEvent::MiddleButton:
        return WebMouseEvent::MiddleButton;
    case Nix::MouseEvent::RightButton:
        return WebMouseEvent::RightButton;
    default:
        notImplemented();
    }
    return WebMouseEvent::NoButton;
}

static String keyIdentifierForNixKeyCode(Nix::KeyEvent::Key keyCode)
{
    switch (keyCode) {
    case Nix::KeyEvent::Key_Menu:
    case Nix::KeyEvent::Key_Alt:
        return ASCIILiteral("Alt");
    case Nix::KeyEvent::Key_Clear:
        return ASCIILiteral("Clear");
    case Nix::KeyEvent::Key_Down:
        return ASCIILiteral("Down");
    case Nix::KeyEvent::Key_End:
        return ASCIILiteral("End");
    case Nix::KeyEvent::Key_Return:
    case Nix::KeyEvent::Key_Enter:
        return ASCIILiteral("Enter");
    case Nix::KeyEvent::Key_Execute:
        return ASCIILiteral("Execute");
    case Nix::KeyEvent::Key_F1:
        return ASCIILiteral("F1");
    case Nix::KeyEvent::Key_F2:
        return ASCIILiteral("F2");
    case Nix::KeyEvent::Key_F3:
        return ASCIILiteral("F3");
    case Nix::KeyEvent::Key_F4:
        return ASCIILiteral("F4");
    case Nix::KeyEvent::Key_F5:
        return ASCIILiteral("F5");
    case Nix::KeyEvent::Key_F6:
        return ASCIILiteral("F6");
    case Nix::KeyEvent::Key_F7:
        return ASCIILiteral("F7");
    case Nix::KeyEvent::Key_F8:
        return ASCIILiteral("F8");
    case Nix::KeyEvent::Key_F9:
        return ASCIILiteral("F9");
    case Nix::KeyEvent::Key_F10:
        return ASCIILiteral("F10");
    case Nix::KeyEvent::Key_F11:
        return ASCIILiteral("F11");
    case Nix::KeyEvent::Key_F12:
        return ASCIILiteral("F12");
    case Nix::KeyEvent::Key_F13:
        return ASCIILiteral("F13");
    case Nix::KeyEvent::Key_F14:
        return ASCIILiteral("F14");
    case Nix::KeyEvent::Key_F15:
        return ASCIILiteral("F15");
    case Nix::KeyEvent::Key_F16:
        return ASCIILiteral("F16");
    case Nix::KeyEvent::Key_F17:
        return ASCIILiteral("F17");
    case Nix::KeyEvent::Key_F18:
        return ASCIILiteral("F18");
    case Nix::KeyEvent::Key_F19:
        return ASCIILiteral("F19");
    case Nix::KeyEvent::Key_F20:
        return ASCIILiteral("F20");
    case Nix::KeyEvent::Key_F21:
        return ASCIILiteral("F21");
    case Nix::KeyEvent::Key_F22:
        return ASCIILiteral("F22");
    case Nix::KeyEvent::Key_F23:
        return ASCIILiteral("F23");
    case Nix::KeyEvent::Key_F24:
        return ASCIILiteral("F24");
    case Nix::KeyEvent::Key_Help:
        return ASCIILiteral("Help");
    case Nix::KeyEvent::Key_Home:
        return ASCIILiteral("Home");
    case Nix::KeyEvent::Key_Insert:
        return ASCIILiteral("Insert");
    case Nix::KeyEvent::Key_Left:
        return ASCIILiteral("Left");
    case Nix::KeyEvent::Key_PageDown:
        return ASCIILiteral("PageDown");
    case Nix::KeyEvent::Key_PageUp:
        return ASCIILiteral("PageUp");
    case Nix::KeyEvent::Key_Pause:
        return ASCIILiteral("Pause");
    case Nix::KeyEvent::Key_Print:
        return ASCIILiteral("PrintScreen");
    case Nix::KeyEvent::Key_Right:
        return ASCIILiteral("Right");
    case Nix::KeyEvent::Key_Select:
        return ASCIILiteral("Select");
    case Nix::KeyEvent::Key_Up:
        return ASCIILiteral("Up");
    case Nix::KeyEvent::Key_Delete:
        return ASCIILiteral("U+007F");
    case Nix::KeyEvent::Key_Backspace:
        return ASCIILiteral("U+0008");
    case Nix::KeyEvent::Key_Tab:
        return ASCIILiteral("U+0009");
    case Nix::KeyEvent::Key_Backtab:
        return ASCIILiteral("U+0009");
    default:
        return String::format("U+%04X", toASCIIUpper((int) keyCode));
    }
}

static int windowsKeyCodeForKeyEvent(Nix::KeyEvent::Key keycode, bool isKeypad)
{
    if (isKeypad) {
        switch (keycode) {
        case Nix::KeyEvent::Key_0:
            return VK_NUMPAD0;
        case Nix::KeyEvent::Key_1:
            return VK_NUMPAD1;
        case Nix::KeyEvent::Key_2:
            return VK_NUMPAD2;
        case Nix::KeyEvent::Key_3:
            return VK_NUMPAD3;
        case Nix::KeyEvent::Key_4:
            return VK_NUMPAD4;
        case Nix::KeyEvent::Key_5:
            return VK_NUMPAD5;
        case Nix::KeyEvent::Key_6:
            return VK_NUMPAD6;
        case Nix::KeyEvent::Key_7:
            return VK_NUMPAD7;
        case Nix::KeyEvent::Key_8:
            return VK_NUMPAD8;
        case Nix::KeyEvent::Key_9:
            return VK_NUMPAD9;
        case Nix::KeyEvent::Key_Asterisk:
            return VK_MULTIPLY;
        case Nix::KeyEvent::Key_Plus:
            return VK_ADD;
        case Nix::KeyEvent::Key_Minus:
            return VK_SUBTRACT;
        case Nix::KeyEvent::Key_Period:
            return VK_DECIMAL;
        case Nix::KeyEvent::Key_Slash:
            return VK_DIVIDE;
        case Nix::KeyEvent::Key_PageUp:
            return VK_PRIOR;
        case Nix::KeyEvent::Key_PageDown:
            return VK_NEXT;
        case Nix::KeyEvent::Key_End:
            return VK_END;
        case Nix::KeyEvent::Key_Home:
            return VK_HOME;
        case Nix::KeyEvent::Key_Left:
            return VK_LEFT;
        case Nix::KeyEvent::Key_Up:
            return VK_UP;
        case Nix::KeyEvent::Key_Right:
            return VK_RIGHT;
        case Nix::KeyEvent::Key_Down:
            return VK_DOWN;
        case Nix::KeyEvent::Key_Enter:
        case Nix::KeyEvent::Key_Return:
            return VK_RETURN;
        case Nix::KeyEvent::Key_Insert:
            return VK_INSERT;
        case Nix::KeyEvent::Key_Delete:
            return VK_DELETE;
        default:
            return 0;
        }

    } else {
        switch (keycode) {
        case Nix::KeyEvent::Key_Backspace:
            return VK_BACK;
        case Nix::KeyEvent::Key_Backtab:
        case Nix::KeyEvent::Key_Tab:
            return VK_TAB;
        case Nix::KeyEvent::Key_Clear:
            return VK_CLEAR;
        case Nix::KeyEvent::Key_Enter:
        case Nix::KeyEvent::Key_Return:
            return VK_RETURN;
        case Nix::KeyEvent::Key_Shift:
            return VK_SHIFT;
        case Nix::KeyEvent::Key_Control:
            return VK_CONTROL;
        case Nix::KeyEvent::Key_Menu:
        case Nix::KeyEvent::Key_Alt:
            return VK_MENU;
        case Nix::KeyEvent::Key_F1:
            return VK_F1;
        case Nix::KeyEvent::Key_F2:
            return VK_F2;
        case Nix::KeyEvent::Key_F3:
            return VK_F3;
        case Nix::KeyEvent::Key_F4:
            return VK_F4;
        case Nix::KeyEvent::Key_F5:
            return VK_F5;
        case Nix::KeyEvent::Key_F6:
            return VK_F6;
        case Nix::KeyEvent::Key_F7:
            return VK_F7;
        case Nix::KeyEvent::Key_F8:
            return VK_F8;
        case Nix::KeyEvent::Key_F9:
            return VK_F9;
        case Nix::KeyEvent::Key_F10:
            return VK_F10;
        case Nix::KeyEvent::Key_F11:
            return VK_F11;
        case Nix::KeyEvent::Key_F12:
            return VK_F12;
        case Nix::KeyEvent::Key_F13:
            return VK_F13;
        case Nix::KeyEvent::Key_F14:
            return VK_F14;
        case Nix::KeyEvent::Key_F15:
            return VK_F15;
        case Nix::KeyEvent::Key_F16:
            return VK_F16;
        case Nix::KeyEvent::Key_F17:
            return VK_F17;
        case Nix::KeyEvent::Key_F18:
            return VK_F18;
        case Nix::KeyEvent::Key_F19:
            return VK_F19;
        case Nix::KeyEvent::Key_F20:
            return VK_F20;
        case Nix::KeyEvent::Key_F21:
            return VK_F21;
        case Nix::KeyEvent::Key_F22:
            return VK_F22;
        case Nix::KeyEvent::Key_F23:
            return VK_F23;
        case Nix::KeyEvent::Key_F24:
            return VK_F24;
        case Nix::KeyEvent::Key_Pause:
            return VK_PAUSE;
        case Nix::KeyEvent::Key_CapsLock:
            return VK_CAPITAL;
        case Nix::KeyEvent::Key_Kana_Lock:
        case Nix::KeyEvent::Key_Kana_Shift:
            return VK_KANA;
        case Nix::KeyEvent::Key_Hangul:
            return VK_HANGUL;
        case Nix::KeyEvent::Key_Hangul_Hanja:
            return VK_HANJA;
        case Nix::KeyEvent::Key_Kanji:
            return VK_KANJI;
        case Nix::KeyEvent::Key_Escape:
            return VK_ESCAPE;
        case Nix::KeyEvent::Key_Space:
            return VK_SPACE;
        case Nix::KeyEvent::Key_PageUp:
            return VK_PRIOR;
        case Nix::KeyEvent::Key_PageDown:
            return VK_NEXT;
        case Nix::KeyEvent::Key_End:
            return VK_END;
        case Nix::KeyEvent::Key_Home:
            return VK_HOME;
        case Nix::KeyEvent::Key_Left:
            return VK_LEFT;
        case Nix::KeyEvent::Key_Up:
            return VK_UP;
        case Nix::KeyEvent::Key_Right:
            return VK_RIGHT;
        case Nix::KeyEvent::Key_Down:
            return VK_DOWN;
        case Nix::KeyEvent::Key_Select:
            return VK_SELECT;
        case Nix::KeyEvent::Key_Print:
            return VK_SNAPSHOT;
        case Nix::KeyEvent::Key_Execute:
            return VK_EXECUTE;
        case Nix::KeyEvent::Key_Insert:
            return VK_INSERT;
        case Nix::KeyEvent::Key_Delete:
            return VK_DELETE;
        case Nix::KeyEvent::Key_Help:
            return VK_HELP;
        case Nix::KeyEvent::Key_0:
        case Nix::KeyEvent::Key_ParenLeft:
            return VK_0;
        case Nix::KeyEvent::Key_1:
            return VK_1;
        case Nix::KeyEvent::Key_2:
        case Nix::KeyEvent::Key_At:
            return VK_2;
        case Nix::KeyEvent::Key_3:
        case Nix::KeyEvent::Key_NumberSign:
            return VK_3;
        case Nix::KeyEvent::Key_4:
        case Nix::KeyEvent::Key_Dollar:
            return VK_4;
        case Nix::KeyEvent::Key_5:
        case Nix::KeyEvent::Key_Percent:
            return VK_5;
        case Nix::KeyEvent::Key_6:
        case Nix::KeyEvent::Key_AsciiCircum:
            return VK_6;
        case Nix::KeyEvent::Key_7:
        case Nix::KeyEvent::Key_Ampersand:
            return VK_7;
        case Nix::KeyEvent::Key_8:
        case Nix::KeyEvent::Key_Asterisk:
            return VK_8;
        case Nix::KeyEvent::Key_9:
        case Nix::KeyEvent::Key_ParenRight:
            return VK_9;
        case Nix::KeyEvent::Key_A:
            return VK_A;
        case Nix::KeyEvent::Key_B:
            return VK_B;
        case Nix::KeyEvent::Key_C:
            return VK_C;
        case Nix::KeyEvent::Key_D:
            return VK_D;
        case Nix::KeyEvent::Key_E:
            return VK_E;
        case Nix::KeyEvent::Key_F:
            return VK_F;
        case Nix::KeyEvent::Key_G:
            return VK_G;
        case Nix::KeyEvent::Key_H:
            return VK_H;
        case Nix::KeyEvent::Key_I:
            return VK_I;
        case Nix::KeyEvent::Key_J:
            return VK_J;
        case Nix::KeyEvent::Key_K:
            return VK_K;
        case Nix::KeyEvent::Key_L:
            return VK_L;
        case Nix::KeyEvent::Key_M:
            return VK_M;
        case Nix::KeyEvent::Key_N:
            return VK_N;
        case Nix::KeyEvent::Key_O:
            return VK_O;
        case Nix::KeyEvent::Key_P:
            return VK_P;
        case Nix::KeyEvent::Key_Q:
            return VK_Q;
        case Nix::KeyEvent::Key_R:
            return VK_R;
        case Nix::KeyEvent::Key_S:
            return VK_S;
        case Nix::KeyEvent::Key_T:
            return VK_T;
        case Nix::KeyEvent::Key_U:
            return VK_U;
        case Nix::KeyEvent::Key_V:
            return VK_V;
        case Nix::KeyEvent::Key_W:
            return VK_W;
        case Nix::KeyEvent::Key_X:
            return VK_X;
        case Nix::KeyEvent::Key_Y:
            return VK_Y;
        case Nix::KeyEvent::Key_Z:
            return VK_Z;
        case Nix::KeyEvent::Key_Meta:
            return VK_LWIN;
        case Nix::KeyEvent::Key_NumLock:
            return VK_NUMLOCK;
        case Nix::KeyEvent::Key_ScrollLock:
            return VK_SCROLL;
        case Nix::KeyEvent::Key_Semicolon:
        case Nix::KeyEvent::Key_Colon:
            return VK_OEM_1;
        case Nix::KeyEvent::Key_Plus:
        case Nix::KeyEvent::Key_Equal:
            return VK_OEM_PLUS;
        case Nix::KeyEvent::Key_Comma:
        case Nix::KeyEvent::Key_Less:
            return VK_OEM_COMMA;
        case Nix::KeyEvent::Key_Minus:
        case Nix::KeyEvent::Key_Underscore:
            return VK_OEM_MINUS;
        case Nix::KeyEvent::Key_Period:
        case Nix::KeyEvent::Key_Greater:
            return VK_OEM_PERIOD;
        case Nix::KeyEvent::Key_Slash:
        case Nix::KeyEvent::Key_Question:
            return VK_OEM_2;
        case Nix::KeyEvent::Key_AsciiTilde:
        case Nix::KeyEvent::Key_QuoteLeft:
            return VK_OEM_3;
        case Nix::KeyEvent::Key_BracketLeft:
        case Nix::KeyEvent::Key_BraceLeft:
            return VK_OEM_4;
        case Nix::KeyEvent::Key_Backslash:
        case Nix::KeyEvent::Key_Bar:
            return VK_OEM_5;
        case Nix::KeyEvent::Key_BracketRight:
        case Nix::KeyEvent::Key_BraceRight:
            return VK_OEM_6;
        case Nix::KeyEvent::Key_QuoteDbl:
            return VK_OEM_7;
        default:
            return 0;
        }
    }
}

static String keyTextForNixKeyEvent(const Nix::KeyEvent& event)
{
    int keycode = static_cast<int>(event.key);
    if (isASCIIPrintable(keycode))
        return String::format("%c", event.shouldUseUpperCase ? toASCIIUpper(keycode) : toASCIILower(keycode));

    switch (event.key) {
    case Nix::KeyEvent::Key_Tab:
    case Nix::KeyEvent::Key_Backtab:
        return "\t";
    case Nix::KeyEvent::Key_Enter:
    case Nix::KeyEvent::Key_Return:
        return "\r";
    default:
        break;
    }

    return "";
}

WebMouseEvent WebEventFactory::createWebMouseEvent(const Nix::MouseEvent& event, const cairo_matrix_t& fromItemTransform, WebCore::IntPoint* lastCursorPosition)
{
    WebEvent::Type type = convertToWebEventType(event.type);
    WebMouseEvent::Button button = convertToWebMouseEventButton(event.button);

    double x = event.x;
    double y = event.y;
    cairo_matrix_transform_point(&fromItemTransform, &x, &y);

    float deltaX = x - lastCursorPosition->x();
    float deltaY = y - lastCursorPosition->y();
    int clickCount = event.clickCount;
    WebEvent::Modifiers modifiers = convertToWebEventModifiers(event.modifiers);
    double timestamp = event.timestamp;
    IntPoint globalPosition = IntPoint(event.globalX, event.globalY);
    lastCursorPosition->setX(x);
    lastCursorPosition->setY(y);

    return WebMouseEvent(type, button, WebCore::IntPoint(x, y), globalPosition, deltaX, deltaY, 0.0f, clickCount, modifiers, timestamp);
}

WebWheelEvent WebEventFactory::createWebWheelEvent(const Nix::WheelEvent& event, const cairo_matrix_t& fromItemTransform)
{
    WebEvent::Type type = convertToWebEventType(event.type);

    double x = event.x;
    double y = event.y;
    cairo_matrix_transform_point(&fromItemTransform, &x, &y);

    IntPoint position = IntPoint(x, y);
    IntPoint globalPosition = IntPoint(event.globalX, event.globalY);
    FloatSize delta = event.orientation == Nix::WheelEvent::Vertical ? FloatSize(0, event.delta) : FloatSize(event.delta, 0);
    WebEvent::Modifiers modifiers = convertToWebEventModifiers(event.modifiers);
    double timestamp = event.timestamp;

    const float ticks = event.delta / float(Scrollbar::pixelsPerLineStep());
    FloatSize wheelTicks = Nix::WheelEvent::Vertical ? FloatSize(0, ticks) : FloatSize(ticks, 0);

    return WebWheelEvent(type, position, globalPosition, delta, wheelTicks, WebWheelEvent::ScrollByPixelWheelEvent, modifiers, timestamp);
}

WebKeyboardEvent WebEventFactory::createWebKeyboardEvent(const Nix::KeyEvent& event)
{
    WebEvent::Type type = convertToWebEventType(event.type);
    const WTF::String text = keyTextForNixKeyEvent(event);
    const WTF::String unmodifiedText = text;
    bool isAutoRepeat = false;
    bool isSystemKey = false;
    bool isKeypad = event.isKeypad;
    const WTF::String keyIdentifier = keyIdentifierForNixKeyCode(event.key);
    int windowsVirtualKeyCode = windowsKeyCodeForKeyEvent(event.key, isKeypad);
    int nativeVirtualKeyCode = 0;
    int macCharCode = 0;
    WebEvent::Modifiers modifiers = convertToWebEventModifiers(event.modifiers);
    double timestamp = event.timestamp;

    return WebKeyboardEvent(type, text, unmodifiedText, keyIdentifier, windowsVirtualKeyCode, nativeVirtualKeyCode, macCharCode, isAutoRepeat, isKeypad, isSystemKey, modifiers, timestamp);
}

WebGestureEvent WebEventFactory::createWebGestureEvent(const Nix::GestureEvent& event, const cairo_matrix_t& fromItemTransform)
{
    WebEvent::Type type = convertToWebEventType(event.type);
    double x = event.x;
    double y = event.y;
    cairo_matrix_transform_point(&fromItemTransform, &x, &y);
    IntPoint position = IntPoint(x, y);
    IntPoint globalPosition = IntPoint(event.globalX, event.globalY);
    WebEvent::Modifiers modifiers = convertToWebEventModifiers(event.modifiers);
    double timestamp = event.timestamp;
    IntSize area = IntSize(event.width, event.height);
    FloatPoint delta = FloatPoint(event.deltaX, event.deltaY);

    return WebGestureEvent(type, position, globalPosition, modifiers, timestamp, area, delta);
}

#if ENABLE(TOUCH_EVENTS)
WebTouchEvent WebEventFactory::createWebTouchEvent(const Nix::TouchEvent& event, const cairo_matrix_t& fromItemTransform)
{
    WebEvent::Type type = convertToWebEventType(event.type);
    Vector<WebPlatformTouchPoint> touchPoints;
    WebEvent::Modifiers modifiers = convertToWebEventModifiers(event.modifiers);
    double timestamp = event.timestamp;

    for (size_t i = 0; i < event.touchPoints.size(); ++i) {
        const Nix::TouchPoint& touch = event.touchPoints[i];
        uint32_t id = static_cast<uint32_t>(touch.id);
        WebPlatformTouchPoint::TouchPointState state = convertToWebTouchState(touch.state);
        IntPoint screenPosition = IntPoint(touch.globalX, touch.globalY);

        double x = touch.x;
        double y = touch.y;
        cairo_matrix_transform_point(&fromItemTransform, &x, &y);
        IntPoint position(x, y);

        IntSize radius = IntSize(touch.horizontalRadius, touch.verticalRadius);
        float rotationAngle = touch.rotationAngle;
        float force = touch.pressure;

        WebPlatformTouchPoint webTouchPoint = WebPlatformTouchPoint(id, state, screenPosition, position, radius, rotationAngle, force);
        touchPoints.append(webTouchPoint);
    }

    return WebTouchEvent(type, touchPoints, modifiers, timestamp);
}
#endif
} // namespace WebKit
