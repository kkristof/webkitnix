#include "config.h"

#include "PlatformUtilities.h"
#include <WebKit2/WKContext.h>
#include <WebKit2/WKPage.h>
#include <WebKit2/WKRetainPtr.h>
#include <WebView.h>

static bool didFinishLoad = false;

static void didFinishLoadForFrame(WKPageRef page, WKFrameRef, WKTypeRef, const void*)
{
    didFinishLoad = true;
}

namespace {

class EmptyWebViewClient : public Nix::WebViewClient {
public:
    void viewNeedsDisplay(int, int, int, int) { }
    void webProcessCrashed(WKStringRef) {}
    void webProcessRelaunched() {}
};

}

namespace TestWebKitAPI {

TEST(WebKitNix, WebViewPaintToCurrentGLContext)
{
    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());
    WKRetainPtr<WKPageGroupRef> pageGroup = adoptWK(WKPageGroupCreateWithIdentifier(WKStringCreateWithUTF8CString("")));

    EmptyWebViewClient client;
    Nix::WebView* webView = Nix::WebView::create(context.get(), pageGroup.get(), &client);
    webView->initialize();
    WKPageSetUseFixedLayout(webView->pageRef(), true);

    WKPageLoaderClient loaderClient;
    memset(&loaderClient, 0, sizeof(loaderClient));

    loaderClient.version = 0;
    loaderClient.didFinishLoadForFrame = didFinishLoadForFrame;
    WKPageSetPageLoaderClient(webView->pageRef(), &loaderClient);

    WKRetainPtr<WKURLRef> url = adoptWK(Util::createURLForResource("../nix/red-background", "html"));
    WKPageLoadURL(webView->pageRef(), url.get());
    Util::run(&didFinishLoad);

    // TODO: Implement the test!
}

} // TestWebKitAPI


