#include "config.h"

#include "NIXView.h"
#include "PageLoader.h"
#include "GLUtilities.h"
#include "WebKit2/WKRetainPtr.h"

namespace TestWebKitAPI {

TEST(WebKitNix, WebViewViewport)
{
    // This test opens a page with contents size of 20000x980 pixels, a white
    // background, no viewport metatag and two black dots located at
    // 10000x400 and 10400x800, then it scales and scroll the page to check if
    // the black pixels are where they are expected to be.

    const unsigned contentsWidth = 980;
    const WKSize size = WKSizeMake(contentsWidth / 2, 200);
    Util::GLOffscreenBuffer offscreenBuffer(size.width, size.height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());
    glViewport(0, 0, size.width, size.height);

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());
    NIXViewAutoPtr view(NIXViewCreate(context.get(), 0));
    Util::ForceRepaintClient client(view.get());
    client.setClearColor(0, 0, 1, 1);

    NIXViewInitialize(view.get());
    WKPageSetUseFixedLayout(NIXViewGetPage(view.get()), true);
    NIXViewSetSize(view.get(), size);

    Util::PageLoader loader(view.get());
    loader.waitForLoadURLAndRepaint("../nix/WebViewViewport");
    unsigned char sample[4];

    // All white.
    glReadPixels(0, size.height - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &sample);
    EXPECT_EQ(0xFF, sample[0]);
    EXPECT_EQ(0xFF, sample[1]);
    EXPECT_EQ(0xFF, sample[2]);
    EXPECT_EQ(0xFF, sample[3]);

    NIXViewSetScale(view.get(), 0.5);
    NIXViewSetScrollPosition(view.get(), WKPointMake(400, 10000));
    loader.forceRepaint();

    // The black dot should be on 0,0
    glReadPixels(0, size.height - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &sample);
    EXPECT_EQ(0x00, int(sample[0]));
    EXPECT_EQ(0x00, int(sample[1]));
    EXPECT_EQ(0x00, int(sample[2]));
    EXPECT_EQ(0xFF, int(sample[3]));

    // And another black dot on 200, 200
    glReadPixels(size.height, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &sample);
    EXPECT_EQ(0x00, int(sample[0]));
    EXPECT_EQ(0x00, int(sample[1]));
    EXPECT_EQ(0x00, int(sample[2]));
    EXPECT_EQ(0xFF, int(sample[3]));
}

}
