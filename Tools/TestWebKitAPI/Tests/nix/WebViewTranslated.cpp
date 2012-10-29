#include "config.h"

#include "PageLoader.h"
#include "GLUtilities.h"
#include "WebView.h"
#include "WebKit2/WKContext.h"
#include "WebKit2/WKRetainPtr.h"
#include <GL/gl.h>
#include <memory>

namespace TestWebKitAPI {

TEST(WebKitNix, WebViewTranslated)
{
    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());

    const int translationDelta = 20;
    Util::ForceRepaintClient client;
    std::auto_ptr<Nix::WebView> webView(Nix::WebView::create(context.get(), 0, &client));

    cairo_matrix_t transform;
    cairo_matrix_init_translate(&transform, translationDelta, translationDelta);
    webView->setUserViewportTransformation(transform);

    client.setView(webView.get());
    client.setClearColor(0, 0, 1, 1);
    webView->initialize();
    WKPageSetUseFixedLayout(webView->pageRef(), true);

    const unsigned width = 100;
    const unsigned height = 100;
    webView->setSize(width, height);

    Util::GLOffscreenBuffer offscreenBuffer(width, height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());

    glViewport(0, 0, width, height);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    Util::PageLoader loader(webView.get());

    loader.waitForLoadURLAndRepaint("../nix/red-background");

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
