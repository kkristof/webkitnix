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
    TestWebViewClient(int delta) {
        cairo_matrix_init_translate(&m_matrix, delta, delta);
    }
    void viewNeedsDisplay(int, int, int, int) {
        // FIXME: We need to paint to consume the frame, I'm not sure
        // if there's other way to force it.
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        webView->paintToCurrentGLContext();
    }
    cairo_matrix_t viewToScreenTransform() {
        return m_matrix;
    }
    void webProcessCrashed(WKStringRef) {}
    void webProcessRelaunched() {}
private:
    cairo_matrix_t m_matrix;
};

} // namespace

static void waitForLoadURLAndRepaint(const char* resource)
{
    WKRetainPtr<WKURLRef> urlRef = adoptWK(Util::createURLForResource(resource, "html"));
    WKPageLoadURL(webView->pageRef(), urlRef.get());
    Util::run(&didFinishLoadAndRepaint);
    didFinishLoadAndRepaint = false;
}

TEST(WebKitNix, WebViewTranslated)
{
    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());

    const int translationDelta = 20;
    TestWebViewClient client(translationDelta);
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
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    waitForLoadURLAndRepaint("../nix/red-background");
    webView->paintToCurrentGLContext();

    // Note that glReadPixels [0, 0] is at the bottom-left of the buffer, so a diagonal
    // line from the top-left, to the bottom-right, will have X values going up and Y
    // values going down. We are testing this diagonal here.

    // Original background.
    unsigned char sample[4 * width * height];
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &sample);
    for (int x = 0, y = height - 1; x < translationDelta; x++, y--) {
        int index = 4 * (y * height + x);
        EXPECT_EQ(0x00, sample[index]) << "Error when checking RED for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0x00, sample[index + 1]) << "Error when checking GREEN for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 2]) << "Error when checking BLUE for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 3]) << "Error when checking ALPHA for pixel (" << x << ", " << y << ")";
    }

    // Red background page.
    for (int x = translationDelta, y = height - translationDelta - 1; x < width; x++, y--) {
        int index = 4 * (y * height + x);
        EXPECT_EQ(0xFF, sample[index]) << "Error when checking RED for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0x00, sample[index + 1]) << "Error when checking GREEN for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0x00, sample[index + 2]) << "Error when checking BLUE for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 3]) << "Error when checking ALPHA for pixel (" << x << ", " << y << ")";
    }
}

} // TestWebKitAPI
