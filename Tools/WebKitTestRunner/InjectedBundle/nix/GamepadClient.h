#ifndef GamepadClient_h
#define GamepadClient_h

#include "NixPlatform.h"

#define MAX_GAMEPAD_DEVICES 4


namespace WTR {

class GamepadClient : public Nix::Platform::GamepadClient {
public:
    GamepadClient() {}

    virtual Nix::Platform::GamepadDevice* getGamepad(int index);

    Nix::Platform::GamepadDevice m_gamepads[MAX_GAMEPAD_DEVICES];
};

} // namespace WTR
#endif // GamepadClient_h
