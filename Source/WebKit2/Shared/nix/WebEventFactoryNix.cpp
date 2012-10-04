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
