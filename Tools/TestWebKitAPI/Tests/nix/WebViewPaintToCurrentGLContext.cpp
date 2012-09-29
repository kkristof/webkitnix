#include "config.h"

#include "GLUtilities.h"
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

    const unsigned width = 100;
    const unsigned height = 100;
    webView->setSize(width, height);

    Util::GLOffscreenBuffer offscreenBuffer(width, height);
    ASSERT_TRUE(offscreenBuffer.wasCorrectlyInitialized());
    ASSERT_TRUE(offscreenBuffer.makeCurrent());
    glViewport(0, 0, width, height);
    glClearColor(0.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    unsigned char clearedSample[4];
    glReadPixels(0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &clearedSample);
    EXPECT_EQ(0x00, clearedSample[0]);
    EXPECT_EQ(0x00, clearedSample[1]);
    EXPECT_EQ(0xFF, clearedSample[2]);
    EXPECT_EQ(0xFF, clearedSample[3]);

    glClear(GL_COLOR_BUFFER_BIT);
    WKRetainPtr<WKURLRef> redUrl = adoptWK(Util::createURLForResource("../nix/red-background", "html"));
    WKPageLoadURL(webView->pageRef(), redUrl.get());
    // FIXME: We need to use WKPageForceRepaint() here.
    Util::run(&didFinishLoad);
    didFinishLoad = false;

    unsigned char redSample[4];
    glReadPixels(0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &redSample);
    EXPECT_EQ(0xFF, redSample[0]);
    EXPECT_EQ(0x00, redSample[1]);
    EXPECT_EQ(0x00, redSample[2]);
    EXPECT_EQ(0xFF, redSample[3]);
}

} // TestWebKitAPI
