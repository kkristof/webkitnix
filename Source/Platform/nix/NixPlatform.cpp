#include "NixPlatform.h"

namespace Nix {

namespace Platform {

// The ownership is from the application.
static GamepadClient* s_gamepadClient = 0;

GamepadDevice* getGamepad(int index)
{
    if (!s_gamepadClient)
        return 0;
    return s_gamepadClient->getGamepad(index);
}

void setGamepadClient(GamepadClient* client)
{
	// We need to be able to set a client = 0 for now.
    s_gamepadClient = client;
}

} //namespace Platform

} //namespace Nix
