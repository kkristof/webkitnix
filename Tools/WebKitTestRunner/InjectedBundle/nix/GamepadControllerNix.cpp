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

#include "config.h"
#include "GamepadController.h"
#include "GamepadClient.h"

#include <wtf/OwnPtr.h>
#include <wtf/PassOwnPtr.h>
#include "StringFunctions.h"

namespace WTR {

static WTF::OwnPtr<GamepadClient> s_gamepadClient;

GamepadController::GamepadController()
{
    if (!s_gamepadClient)
        s_gamepadClient = WTF::adoptPtr(new GamepadClient());

    Nix::Platform::setGamepadClient(s_gamepadClient.get());
}

GamepadController::~GamepadController()
{
}

void GamepadController::connect(int index)
{
    if (index < 0 || index >= MAX_GAMEPAD_DEVICES)
        return;

    s_gamepadClient->m_gamepads[index].connected = true;
}

void GamepadController::disconnect(int index)
{
    if (index < 0 || index >= MAX_GAMEPAD_DEVICES)
        return;

    s_gamepadClient->m_gamepads[index].connected = false;
}

void GamepadController::setId(int index, JSStringRef id)
{
    if (index < 0 || index >= MAX_GAMEPAD_DEVICES)
        return;

    s_gamepadClient->m_gamepads[index].id = toSTD(toWK(id));
}

void GamepadController::setButtonCount(int index, int buttons)
{
    if (index < 0 || index >= MAX_GAMEPAD_DEVICES)
        return;

    if (buttons < 0)
        return;

    std::vector<float> buttonsList(buttons, 0.0);
    s_gamepadClient->m_gamepads[index].buttons = buttonsList;
}

void GamepadController::setButtonData(int index, int button, float buttonData)
{
    if (index < 0 || index >= MAX_GAMEPAD_DEVICES)
        return;

    if (button < 0 || button >= s_gamepadClient->m_gamepads[index].buttons.size())
        return;

    s_gamepadClient->m_gamepads[index].buttons[button] = buttonData;
}

void GamepadController::setAxisCount(int index, int axis)
{
    if (index < 0 || index >= MAX_GAMEPAD_DEVICES)
        return;

    if (axis < 0)
        return;

    std::vector<float> axesList(axis, 0.0);
    s_gamepadClient->m_gamepads[index].axes = axesList;
}

void GamepadController::setAxisData(int index, int axis, float axisData)
{
    if (index < 0 || index >= MAX_GAMEPAD_DEVICES)
        return;

    if (axis < 0 || axis >= s_gamepadClient->m_gamepads[index].axes.size())
        return;

    s_gamepadClient->m_gamepads[index].axes[axis] = axisData;
}

Nix::Platform::GamepadDevice* GamepadClient::getGamepad(int index)
{
    if (index < 0 || index >= MAX_GAMEPAD_DEVICES)
        return 0;

    return &m_gamepads[index];
}

} // namespace WTR
