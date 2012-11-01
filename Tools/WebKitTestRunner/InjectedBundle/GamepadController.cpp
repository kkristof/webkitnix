#include "config.h"
#include "GamepadController.h"

#include "InjectedBundlePage.h"
#include "JSGamepadController.h"
#include <JavaScriptCore/JSObjectRef.h>
#include "StringFunctions.h"

namespace WTR {

PassRefPtr<GamepadController> GamepadController::create()
{
    return adoptRef(new GamepadController);
}

void GamepadController::makeWindowObject(JSContextRef context, JSObjectRef windowObject, JSValueRef* exception)
{
    setProperty(context, windowObject, "gamepadController", this, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, exception);
}

JSClassRef GamepadController::wrapperClass()
{
    return JSGamepadController::gamepadControllerClass();
}

} // namespace WTR

