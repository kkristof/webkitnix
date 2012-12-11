#include "config.h"

#include "GLUtilities.h"
#include "PageLoader.h"
#include "NIXView.h"
#include "WebKit2/WKContext.h"
#include "WebKit2/WKRetainPtr.h"

namespace TestWebKitAPI {

TEST(WebKitNix, WebViewPaintToCurrentGLContext)
{
    const WKSize size = WKSizeMake(100, 100);
    Util::GLOffscreenBuffer offscreenBuffer(size.width, size.height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());
    NIXViewAutoPtr view(NIXViewCreate(context.get(), 0));
    Util::ForceRepaintClient client(view.get());
    client.setClearColor(0, 0, 1, 1);

    NIXViewInitialize(view.get());
    WKPageSetUseFixedLayout(NIXViewGetPage(view.get()), true);
    NIXViewSetSize(view.get(), size);
    Util::PageLoader loader(view.get());

    glViewport(0, 0, size.width, size.height);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    unsigned char clearedSample[4];
    glReadPixels(0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &clearedSample);
    EXPECT_EQ(0x00, clearedSample[0]);
    EXPECT_EQ(0x00, clearedSample[1]);
    EXPECT_EQ(0xFF, clearedSample[2]);
    EXPECT_EQ(0xFF, clearedSample[3]);

    glClear(GL_COLOR_BUFFER_BIT);
    loader.waitForLoadURLAndRepaint("../nix/red-background");

    unsigned char redSample[4];
    glReadPixels(0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &redSample);
    EXPECT_EQ(0xFF, redSample[0]);
    EXPECT_EQ(0x00, redSample[1]);
    EXPECT_EQ(0x00, redSample[2]);
    EXPECT_EQ(0xFF, redSample[3]);

    glClear(GL_COLOR_BUFFER_BIT);
    loader.waitForLoadURLAndRepaint("../nix/green-background");

    unsigned char greenSample[4];
    glReadPixels(0, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &greenSample);
    EXPECT_EQ(0x00, greenSample[0]);
    EXPECT_EQ(0xFF, greenSample[1]);
    EXPECT_EQ(0x00, greenSample[2]);
    EXPECT_EQ(0xFF, greenSample[3]);
}

} // TestWebKitAPI
