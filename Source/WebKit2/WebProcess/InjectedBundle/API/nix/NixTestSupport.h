#ifndef NixTestSupport_h
#define NixTestSupport_h

#include <WebKit2/WKBase.h>
#include <JavaScriptCore/JavaScript.h>

namespace Nix {
    WK_EXPORT void injectInternalsObject(JSContextRef);
    WK_EXPORT void resetInternalsObject(JSContextRef);
}

#endif
