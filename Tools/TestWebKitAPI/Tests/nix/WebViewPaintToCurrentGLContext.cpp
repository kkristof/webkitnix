#include "config.h"

#include "GLUtilities.h"
#include "PageLoader.h"
#include "NIXView.h"
#include "NIXViewAutoPtr.h"
#include "WebKit2/WKContext.h"
#include "WebKit2/WKRetainPtr.h"

namespace TestWebKitAPI {

TEST(WebKitNix, WebViewPaintToCurrentGLContext)
{
    const WKSize size = WKSizeMake(100, 100);
    ToolsNix::GLOffscreenBuffer offscreenBuffer(size.width, size.height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());
    NIXViewAutoPtr view(NIXViewCreate(context.get(), 0));
    Util::ForceRepaintClient client(view.get());
    client.setClearColor(0, 0, 1, 1);

    NIXViewInitialize(view.get());
    NIXViewSetSize(view.get(), size);
    Util::PageLoader loader(view.get());

    glViewport(0, 0, size.width, size.height);
    glClearColor(0, 0, 1, 1);

    glClear(GL_COLOR_BUFFER_BIT);
    ToolsNix::RGBAPixel clearedSample = offscreenBuffer.readPixelAtPoint(0, 0);
    EXPECT_EQ(ToolsNix::RGBAPixel::blue(), clearedSample);

    glClear(GL_COLOR_BUFFER_BIT);
    loader.waitForLoadURLAndRepaint("../nix/red-background");
    ToolsNix::RGBAPixel redSample = offscreenBuffer.readPixelAtPoint(0, 0);
    EXPECT_EQ(ToolsNix::RGBAPixel::red(), redSample);

    glClear(GL_COLOR_BUFFER_BIT);
    loader.waitForLoadURLAndRepaint("../nix/green-background");
    ToolsNix::RGBAPixel greenSample = offscreenBuffer.readPixelAtPoint(0, 0);
    EXPECT_EQ(ToolsNix::RGBAPixel::green(), greenSample);
}

} // TestWebKitAPI
