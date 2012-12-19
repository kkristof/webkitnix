#include "config.h"

#include "PageLoader.h"
#include "GLUtilities.h"
#include "NIXView.h"
#include "NIXViewAutoPtr.h"
#include "WebKit2/WKContext.h"
#include "WebKit2/WKRetainPtr.h"

namespace TestWebKitAPI {

TEST(WebKitNix, WebViewTranslatedScaled)
{
    // This test opens a webpage that contains a white background, no viewport
    // metatag and a red rectangle (20x20)px at (0,0) position. The viewport is
    // then translated to (10,10) position. After that it's applied a scale=2.0
    // At this point we will have a red rectangle of (40x40)px at (10,10).

    const WKSize size = WKSizeMake(100, 100);
    ToolsNix::GLOffscreenBuffer offscreenBuffer(size.width, size.height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());

    NIXViewAutoPtr view(NIXViewCreate(context.get(), 0));
    Util::ForceRepaintClient client(view.get());
    client.setClearColor(0, 0, 1, 1);

    const int delta = 10;
    NIXMatrix transform = NIXMatrixMakeTranslation(delta, delta);
    NIXViewSetUserViewportTransformation(view.get(), &transform);

    NIXViewInitialize(view.get());
    WKPageSetUseFixedLayout(NIXViewGetPage(view.get()), true);
    NIXViewSetSize(view.get(), size);

    glViewport(0, 0, size.width, size.height);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    Util::PageLoader loader(view.get());

    loader.waitForLoadURLAndRepaint("../nix/red-square");

    for (double scale = 1.0; scale < 3.0; scale++) {
        NIXViewSetScale(view.get(), scale);
        loader.forceRepaint();

        ToolsNix::RGBAPixel outsideTheContent = offscreenBuffer.readPixelAtPoint(delta - 1, delta - 1);
        EXPECT_EQ(ToolsNix::RGBAPixel::blue(), outsideTheContent);

        ToolsNix::RGBAPixel squareTopLeft = offscreenBuffer.readPixelAtPoint(delta, delta);
        EXPECT_EQ(ToolsNix::RGBAPixel::red(), squareTopLeft);

        const int scaledSize = scale * 20;
        ToolsNix::RGBAPixel squareBottomRight = offscreenBuffer.readPixelAtPoint(delta + scaledSize - 1, delta + scaledSize - 1);
        EXPECT_EQ(ToolsNix::RGBAPixel::red(), squareBottomRight);

        ToolsNix::RGBAPixel outsideSquare = offscreenBuffer.readPixelAtPoint(delta + scaledSize, delta + scaledSize);
        EXPECT_EQ(ToolsNix::RGBAPixel::white(), outsideSquare);
    }
}

} // TestWebKitAPI
