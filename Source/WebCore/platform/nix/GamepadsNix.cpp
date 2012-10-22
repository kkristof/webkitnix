/*
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
#include "Gamepads.h"

#include "GamepadList.h"
#include "NixPlatform.h"

namespace WebCore {

void sampleGamepads(GamepadList* into)
{
    for (unsigned i = 0; i < into->length(); i++) {
        Nix::Platform::GamepadDevice* gamepadDevice = Nix::Platform::getGamepad(i);
        if (gamepadDevice && gamepadDevice->connected) {
            RefPtr<Gamepad> gamepad = into->item(i);
            if (!gamepad)
                gamepad = Gamepad::create();

            gamepad->index(i);
            gamepad->id(String(gamepadDevice->id.c_str()).simplifyWhiteSpace());
            gamepad->timestamp(gamepadDevice->lastTimestamp);
            gamepad->axes(gamepadDevice->axes.size(), gamepadDevice->axes.data());
            gamepad->buttons(gamepadDevice->buttons.size(), gamepadDevice->buttons.data());

            into->set(i, gamepad);
        } else
            into->set(i, 0);
    }
}

} // namespace WebCore
