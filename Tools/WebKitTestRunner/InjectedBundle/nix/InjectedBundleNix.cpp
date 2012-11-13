#include "config.h"
#include "InjectedBundle.h"

#include <wtf/Assertions.h>
#include <wtf/Threading.h>

namespace WTR {

void InjectedBundle::platformInitialize(WKTypeRef)
{
    WTF::initializeThreading();
    WTFInstallReportBacktraceOnCrashHook();
}

}
