#include "config.h"

#include "NixTestSupport.h"
#include "testing/js/WebCoreTestSupport.h"

void Nix::injectInternalsObject(JSContextRef context)
{
    WebCoreTestSupport::injectInternalsObject(context);
}

void Nix::resetInternalsObject(JSContextRef context)
{
    WebCoreTestSupport::resetInternalsObject(context);
}
