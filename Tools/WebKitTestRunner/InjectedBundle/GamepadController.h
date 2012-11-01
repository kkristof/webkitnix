#ifndef GamepadController_h
#define GamepadController_h

#include "JSWrappable.h"
#include <JavaScriptCore/JSObjectRef.h>
#include <wtf/Platform.h>
#include <wtf/RefPtr.h>

namespace WTR {

class GamepadController : public JSWrappable {
public:
    static PassRefPtr<GamepadController> create();
    ~GamepadController();

    void makeWindowObject(JSContextRef, JSObjectRef windowObject, JSValueRef* exception);
    virtual JSClassRef wrapperClass();

    // Bound methods
    void connect(int index);
    void disconnect(int index);
    void setId(int index, JSStringRef id);
    void setButtonCount(int index, int button);
    void setButtonData(int index, int button, float buttonData);
    void setAxisCount(int index, int axis);
    void setAxisData(int index, int button, float axisData);

private:
    GamepadController();

};

} // namespace WTR
#endif // GamepadController_h
