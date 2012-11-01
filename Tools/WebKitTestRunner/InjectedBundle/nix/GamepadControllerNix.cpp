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
