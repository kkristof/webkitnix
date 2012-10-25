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

#ifndef WebEventFactoryNix_h
#define WebEventFactoryNix_h

#include "WebEvent.h"

#include <NativeWebMouseEvent.h>
#include <NativeWebWheelEvent.h>
#include <NativeWebKeyboardEvent.h>
#if ENABLE(GESTURE_EVENTS)
#include <NativeWebGestureEvent.h>
#endif
#if ENABLE(TOUCH_EVENTS)
#include <NativeWebTouchEvent.h>
#endif

namespace WebKit {

class WebEventFactory {
public:
    static WebMouseEvent createWebMouseEvent(const Nix::MouseEvent&, WebCore::IntPoint* lastCursorPosition);
    static WebWheelEvent createWebWheelEvent(const Nix::WheelEvent&);
    static WebKeyboardEvent createWebKeyboardEvent(const Nix::KeyEvent&);
#if ENABLE(GESTURE_EVENTS)
    static WebGestureEvent createWebGestureEvent(const Nix::GestureEvent&);
#endif
#if ENABLE(TOUCH_EVENTS)
    static WebTouchEvent createWebTouchEvent(const Nix::TouchEvent&);
#endif
};

} // namespace WebKit

#endif // WebEventFactoryNix_h
