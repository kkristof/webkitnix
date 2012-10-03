#include "config.h"

#include "PlatformUtilities.h"
#include <WebKit2/WKContext.h>
#include <WebKit2/WKPage.h>
#include <WebKit2/WKRetainPtr.h>
#include <WebView.h>

namespace TestWebKitAPI {

static bool didWebProcessCrash = false;
static bool didWebProcessRelaunch = false;
static bool didFinishLoad = false;

static void didFinishLoadForFrame(WKPageRef page, WKFrameRef, WKTypeRef, const void*)
{
    didFinishLoad = true;
}

namespace {
class TestWebViewClient : public Nix::WebViewClient {
public:
    void viewNeedsDisplay(int, int, int, int) {}

    void webProcessCrashed(WKStringRef)
    {
        didWebProcessCrash = true;
    }

    void webProcessRelaunched()
    {
        didWebProcessRelaunch = true;
    }
};
} // namespace

TEST(WebKitNix, WebViewWebProcessCrashed)
{
    WKRetainPtr<WKContextRef> context = adoptWK(Util::createContextForInjectedBundleTest("WebViewWebProcessCrashedTest"));

    TestWebViewClient client;
    Nix::WebView* webView = Nix::WebView::create(context.get(), 0, &client);
    webView->initialize();

    WKPageLoaderClient loaderClient;
    memset(&loaderClient, 0, sizeof(loaderClient));

    loaderClient.version = 0;
    loaderClient.didFinishLoadForFrame = didFinishLoadForFrame;
    WKPageSetPageLoaderClient(webView->pageRef(), &loaderClient);

    const unsigned width = 100;
    const unsigned height = 100;
    webView->setSize(width, height);

    WKRetainPtr<WKURLRef> redUrl = adoptWK(Util::createURLForResource("../nix/red-background", "html"));
    WKPageLoadURL(webView->pageRef(), redUrl.get());
    Util::run(&didFinishLoad);
    didFinishLoad = false;

    WKContextPostMessageToInjectedBundle(context.get(), Util::toWK("Crash").get(), 0);
    Util::run(&didWebProcessCrash);

    WKRetainPtr<WKURLRef> greenUrl = adoptWK(Util::createURLForResource("../nix/green-background", "html"));
    WKPageLoadURL(webView->pageRef(), greenUrl.get());
    Util::run(&didFinishLoad);

    ASSERT_TRUE(didWebProcessCrash);
    ASSERT_TRUE(didWebProcessRelaunch);
    ASSERT_TRUE(didFinishLoad);
}

} // TestWebKitAPI
