#ifndef NixPlatform_h
#define NixPlatform_h

#include <vector>
#include <string>

namespace Nix {

namespace Platform {

    struct GamepadDevice {
        bool connected;
        std::string id;
        unsigned long long lastTimestamp;

        std::vector<float> axes;
        std::vector<float> buttons;
    };

    class GamepadClient {
    public:
        virtual GamepadDevice* getGamepad(int index) = 0;
    };

    void setGamepadClient(GamepadClient*);
    GamepadDevice* getGamepad(int index);
} //namespace Platform

} //namespace Nix

#endif // NixPlatform_h
