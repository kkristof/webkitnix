#include "config.h"
#include "InjectedBundleTest.h"
#include <WebKit2/WKRetainPtr.h>

#include <cstdlib>

namespace TestWebKitAPI {

class WebViewWebProcessCrashedTest : public InjectedBundleTest {
public:
    WebViewWebProcessCrashedTest(const std::string& identifier)
        : InjectedBundleTest(identifier)
    {
    }

    virtual void didReceiveMessage(WKBundleRef bundle, WKStringRef messageName, WKTypeRef messageBody)
    {
        if (!WKStringIsEqualToUTF8CString(messageName, "Crash"))
            return;
        abort();
    }
};

static InjectedBundleTest::Register<WebViewWebProcessCrashedTest> registrar("WebViewWebProcessCrashedTest");

} // namespace TestWebKitAPI

