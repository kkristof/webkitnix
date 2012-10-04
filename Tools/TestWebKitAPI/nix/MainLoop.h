#ifndef MainLoop_h
#define MainLoop_h

#include <glib.h>

namespace TestWebKitAPI {
namespace Util {

void initializeMainLoop();
GMainLoop* mainLoop();

}
}

#endif
