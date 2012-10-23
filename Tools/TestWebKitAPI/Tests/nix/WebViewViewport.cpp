#include "config.h"

#include "WebView.h"
#include "PageLoader.h"
#include "GLUtilities.h"
#include "WebKit2/WKRetainPtr.h"
#include <memory>

namespace TestWebKitAPI {

TEST(WebKitNix, WebViewViewport)
{
    using namespace WebKit;

    /*
     This test opens a page with contents size of 20000x980 pixels, a white
     background, no viewport metatag and two black dots located at
     10000x400 and 10400x800, then it scales and scroll the page to check if
     the black pixels are where they are expected to be.
     */

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());
    Util::ForceRepaintClient client;
    std::auto_ptr<Nix::WebView> webView(Nix::WebView::create(context.get(), 0, &client));
    client.setView(webView.get());
    client.setClearColor(0, 0, 1, 1);
    webView->initialize();
    WKPageSetUseFixedLayout(webView->pageRef(), true);

    Util::PageLoader loader(webView.get());

    const unsigned width = 490; // 980 / 2
    const unsigned height = 200;
    webView->setSize(width, height);

    Util::GLOffscreenBuffer offscreenBuffer(width, height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());
    glViewport(0, 0, width, height);

    loader.waitForLoadURLAndRepaint("../nix/WebViewViewport");
    unsigned char sample[4];

    // All white
    glReadPixels(0, 199, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &sample);
    EXPECT_EQ(0xFF, sample[0]);
    EXPECT_EQ(0xFF, sample[1]);
    EXPECT_EQ(0xFF, sample[2]);
    EXPECT_EQ(0xFF, sample[3]);

    webView->setScale(0.5);
    webView->setScrollPosition(400, 10000);
    loader.forceRepaint();

    // The black dot should be on 0,0
    glReadPixels(0, 199, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &sample);
    EXPECT_EQ(0x00, int(sample[0]));
    EXPECT_EQ(0x00, int(sample[1]));
    EXPECT_EQ(0x00, int(sample[2]));
    EXPECT_EQ(0xFF, int(sample[3]));

    // And another black dot on 200, 200
    // See the HTML file for more info
    glReadPixels(200, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &sample);
    EXPECT_EQ(0x00, int(sample[0]));
    EXPECT_EQ(0x00, int(sample[1]));
    EXPECT_EQ(0x00, int(sample[2]));
    EXPECT_EQ(0xFF, int(sample[3]));

    webView.release();
}

}
