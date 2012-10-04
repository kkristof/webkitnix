#include "config.h"
#include "MainLoop.h"
#include <wtf/gobject/GRefPtr.h>

static GRefPtr<GMainLoop> g_mainLoop = 0;

namespace TestWebKitAPI {
namespace Util {

void initializeMainLoop()
{
    g_mainLoop = adoptGRef(g_main_loop_new(0, false));
}

GMainLoop* mainLoop()
{
    ASSERT(g_mainLoop);
    return g_mainLoop.get();
}

} // namespace Util
} // namespace TestWebKitAPI
