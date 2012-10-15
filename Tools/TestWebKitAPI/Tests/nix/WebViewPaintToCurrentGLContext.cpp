#include "config.h"

#include "GLUtilities.h"
#include "PlatformUtilities.h"
#include <WebKit2/WKContext.h>
#include <WebKit2/WKPage.h>
#include <WebKit2/WKRetainPtr.h>
#include <WebView.h>

namespace TestWebKitAPI {

static bool didFinishLoadAndRepaint = false;
static Nix::WebView* webView = 0;

static void didForceRepaint(WKErrorRef, void*)
{
    didFinishLoadAndRepaint = true;
}

static void didFinishLoadForFrame(WKPageRef page, WKFrameRef, WKTypeRef, const void*)
{
    WKPageForceRepaint(page, 0, didForceRepaint);
}

namespace {

class TestWebViewClient : public Nix::WebViewClient {
public:
    void viewNeedsDisplay(int, int, int, int) {
        // FIXME: We need to paint to consume the frame, I'm not sure
        // if there's other way to force it.
        glClear(GL_COLOR_BUFFER_BIT);
        webView->paintToCurrentGLContext();
    }
    void webProcessCrashed(WKStringRef) {}
    void webProcessRelaunched() {}
};

} // namespace

static void waitForLoadURLAndRepaint(const char* resource)
{
    WKRetainPtr<WKURLRef> urlRef = adoptWK(Util::createURLForResource(resource, "html"));
    WKPageLoadURL(webView->pageRef(), urlRef.get());
    Util::run(&didFinishLoadAndRepaint);
    didFinishLoadAndRepaint = false;
}

TEST(WebKitNix, WebViewPaintToCurrentGLContext)
{
    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());

    TestWebViewClient client;
    webView = Nix::WebView::create(context.get(), 0, &client);
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
    waitForLoadURLAndRepaint("../nix/red-background");
    webView->paintToCurrentGLContext();

    unsigned char redSample[4];
    glReadPixels(0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &redSample);
    EXPECT_EQ(0xFF, redSample[0]);
    EXPECT_EQ(0x00, redSample[1]);
    EXPECT_EQ(0x00, redSample[2]);
    EXPECT_EQ(0xFF, redSample[3]);

    glClear(GL_COLOR_BUFFER_BIT);
    waitForLoadURLAndRepaint("../nix/green-background");
    webView->paintToCurrentGLContext();

    unsigned char greenSample[4];
    glReadPixels(0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &greenSample);
    EXPECT_EQ(0x00, greenSample[0]);
    EXPECT_EQ(0xFF, greenSample[1]);
    EXPECT_EQ(0x00, greenSample[2]);
    EXPECT_EQ(0xFF, greenSample[3]);
}

} // TestWebKitAPI
