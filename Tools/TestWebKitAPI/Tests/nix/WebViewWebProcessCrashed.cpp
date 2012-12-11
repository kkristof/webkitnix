#include "config.h"

#include "PlatformUtilities.h"
#include <WebKit2/WKContext.h>
#include <WebKit2/WKPage.h>
#include <WebKit2/WKRetainPtr.h>
#include <NIXView.h>

namespace TestWebKitAPI {

static bool didWebProcessCrash = false;
static bool didWebProcessRelaunch = false;
static bool didFinishLoad = false;

static void didFinishLoadForFrame(WKPageRef, WKFrameRef, WKTypeRef, const void*)
{
    didFinishLoad = true;
}

static void webProcessCrashed(WKStringRef, const void*)
{
    didWebProcessCrash = true;
}

static void webProcessRelaunched(const void*)
{
    didWebProcessRelaunch = true;
}

TEST(WebKitNix, WebViewWebProcessCrashed)
{
    WKRetainPtr<WKContextRef> context = adoptWK(Util::createContextForInjectedBundleTest("WebViewWebProcessCrashedTest"));

    NIXViewClient viewClient;
    memset(&viewClient, 0, sizeof(NIXViewClient));
    viewClient.version = kNIXViewClientCurrentVersion;
    viewClient.webProcessCrashed = webProcessCrashed;
    viewClient.webProcessRelaunched = webProcessRelaunched;
    NIXViewAutoPtr view(NIXViewCreate(context.get(), 0, &viewClient));
    NIXViewInitialize(view.get());

    WKPageLoaderClient loaderClient;
    memset(&loaderClient, 0, sizeof(loaderClient));

    loaderClient.version = 0;
    loaderClient.didFinishLoadForFrame = didFinishLoadForFrame;
    WKPageSetPageLoaderClient(NIXViewGetPage(view.get()), &loaderClient);

    const WKSize size = WKSizeMake(100, 100);
    NIXViewSetSize(view.get(), size);

    WKRetainPtr<WKURLRef> redUrl = adoptWK(Util::createURLForResource("../nix/red-background", "html"));
    WKPageLoadURL(NIXViewGetPage(view.get()), redUrl.get());
    Util::run(&didFinishLoad);
    didFinishLoad = false;

    WKContextPostMessageToInjectedBundle(context.get(), Util::toWK("Crash").get(), 0);
    Util::run(&didWebProcessCrash);

    WKRetainPtr<WKURLRef> greenUrl = adoptWK(Util::createURLForResource("../nix/green-background", "html"));
    WKPageLoadURL(NIXViewGetPage(view.get()), greenUrl.get());
    Util::run(&didFinishLoad);

    ASSERT_TRUE(didWebProcessCrash);
    ASSERT_TRUE(didWebProcessRelaunch);
    ASSERT_TRUE(didFinishLoad);
}

} // TestWebKitAPI
