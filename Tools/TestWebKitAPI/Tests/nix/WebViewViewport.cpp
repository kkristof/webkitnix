#include "config.h"

#include "NIXView.h"
#include "NIXViewAutoPtr.h"
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

    // Checking that without scrolling, we correctly see the white background.
    Util::RGBAPixel firstDotBefore = offscreenBuffer.readPixelAtPoint(0, 0);
    EXPECT_EQ(Util::RGBAPixel::white(), firstDotBefore);
    Util::RGBAPixel secondDotBefore = offscreenBuffer.readPixelAtPoint(200, size.height - 1);
    EXPECT_EQ(Util::RGBAPixel::white(), secondDotBefore);

    // Scale and scroll so that the first dot be at the topleft of the view.
    NIXViewSetScale(view.get(), 0.5);
    NIXViewSetScrollPosition(view.get(), WKPointMake(400, 10000));
    loader.forceRepaint();

    // Now check that the black dots are in the expected places.
    Util::RGBAPixel firstDotAfter = offscreenBuffer.readPixelAtPoint(0, 0);
    EXPECT_EQ(Util::RGBAPixel::black(), firstDotAfter);
    Util::RGBAPixel secondDotAfter = offscreenBuffer.readPixelAtPoint(200, size.height - 1);
    EXPECT_EQ(Util::RGBAPixel::black(), secondDotAfter);
}

}
