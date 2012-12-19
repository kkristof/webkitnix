#include "config.h"

#include "PageLoader.h"
#include "GLUtilities.h"
#include "NIXView.h"
#include "NIXViewAutoPtr.h"
#include "WebKit2/WKContext.h"
#include "WebKit2/WKRetainPtr.h"

namespace TestWebKitAPI {

TEST(WebKitNix, WebViewTranslated)
{
    const WKSize size = WKSizeMake(100, 100);
    ToolsNix::GLOffscreenBuffer offscreenBuffer(size.width, size.height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());

    const int translationDelta = 20;
    NIXViewAutoPtr view(NIXViewCreate(context.get(), 0));
    Util::ForceRepaintClient client(view.get());
    client.setClearColor(0, 0, 1, 1);

    NIXMatrix transform = NIXMatrixMakeTranslation(translationDelta, translationDelta);
    NIXViewSetUserViewportTransformation(view.get(), &transform);

    NIXViewInitialize(view.get());
    WKPageSetUseFixedLayout(NIXViewGetPage(view.get()), true);
    NIXViewSetSize(view.get(), size);

    glViewport(0, 0, size.width, size.height);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    Util::PageLoader loader(view.get());

    loader.waitForLoadURLAndRepaint("../nix/red-background");

    // Note that glReadPixels [0, 0] is at the bottom-left of the buffer, so a diagonal
    // line from the top-left, to the bottom-right, will have X values going up and Y
    // values going down. We are testing this diagonal here.

    // Original background.
    ToolsNix::RGBAPixel sample[int(size.width * size.height)];
    glReadPixels(0, 0, size.width, size.height, GL_RGBA, GL_UNSIGNED_BYTE, &sample);
    for (int x = 0, y = size.height - 1; x < translationDelta; x++, y--) {
        int index = y * size.height + x;
        EXPECT_EQ(ToolsNix::RGBAPixel::blue(), sample[index]) << "Error when checking for pixel (" << x << ", " << y << ")";
    }

    // Red background page.
    for (int x = translationDelta, y = size.height - translationDelta - 1; x < size.width; x++, y--) {
        int index = y * size.height + x;
        EXPECT_EQ(ToolsNix::RGBAPixel::red(), sample[index]) << "Error when checking for pixel (" << x << ", " << y << ")";
    }
}

} // TestWebKitAPI
