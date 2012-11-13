#include "config.h"
#include "MainLoop.h"
#include <cstdlib>
#include <glib.h>

static GMainLoop* g_mainLoop = 0;

extern "C" {
static void unrefMainLoop()
{
    g_main_loop_unref(g_mainLoop);
}
}

namespace TestWebKitAPI {
namespace Util {

void initializeMainLoop()
{
    assert(!g_mainLoop);
    g_mainLoop = g_main_loop_new(0, false);
    std::atexit(unrefMainLoop);
}

GMainLoop* mainLoop()
{
    assert(g_mainLoop);
    return g_mainLoop;
}

} // namespace Util
} // namespace TestWebKitAPI
