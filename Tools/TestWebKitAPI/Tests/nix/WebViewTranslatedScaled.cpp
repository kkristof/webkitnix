#include "config.h"

#include "PageLoader.h"
#include "GLUtilities.h"
#include "WebView.h"
#include "WebKit2/WKContext.h"
#include "WebKit2/WKRetainPtr.h"
#include <memory>

namespace TestWebKitAPI {

TEST(WebKitNix, WebViewTranslatedScaled)
{
    // This test opens a webpage that contains a white background, no viewport
    // metatag and a red rectangle (20x20)px at (0,0) position. The viewport is
    // then translated to (10,10) position. After that it's applied a scale=2.0
    // At this point we will have a red rectangle of (40x40)px at (10,10).

    const WKSize size = WKSizeMake(100, 100);
    Util::GLOffscreenBuffer offscreenBuffer(size.width, size.height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());

    Util::ForceRepaintClient client;
    std::auto_ptr<NIXView> view(NIXViewCreate(context.get(), 0, client.viewClient()));
    client.setView(view.get());
    client.setClearColor(0, 0, 1, 1);

    cairo_matrix_t cairoTransform;
    const int translationDelta = 10;
    cairo_matrix_init_translate(&cairoTransform, translationDelta, translationDelta);
    NIXMatrix transform;
    transform.xx = cairoTransform.xx;
    transform.yx = cairoTransform.yx;
    transform.xy = cairoTransform.xy;
    transform.yy = cairoTransform.yy;
    transform.x0 = cairoTransform.x0;
    transform.y0 = cairoTransform.y0;
    NIXViewSetUserViewportTransformation(view.get(), &transform);

    NIXViewInitialize(view.get());
    WKPageSetUseFixedLayout(NIXViewPageRef(view.get()), true);
    NIXViewSetSize(view.get(), size);

    glViewport(0, 0, size.width, size.height);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    Util::PageLoader loader(view.get());

    loader.waitForLoadURLAndRepaint("../nix/red-square");

    // Note that glReadPixels [0, 0] is at the bottom-left of the buffer.
    unsigned char sample[4 * int(size.width * size.height)];
    for (double scale = 1.0; scale < 3.0; scale++) {
        NIXViewSetScale(view.get(), scale);
        loader.forceRepaint();
        glReadPixels(0, 0, size.width, size.height, GL_RGBA, GL_UNSIGNED_BYTE, &sample);

        // (Left x Top) must be RED
        int x = translationDelta;
        int y = size.height - translationDelta - 1;
        int index = 4 * (y * size.height + x);
        EXPECT_EQ(0xFF, sample[index]) << "Error when checking RED for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0x00, sample[index + 1]) << "Error when checking GREEN for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0x00, sample[index + 2]) << "Error when checking BLUE for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 3]) << "Error when checking ALPHA for pixel (" << x << ", " << y << ")";

        // (Right x Bottom) must be RED
        x += (20 * scale - 1);
        y -= (20 * scale - 1);
        index = 4 * (y * size.height + x);
        EXPECT_EQ(0xFF, sample[index]) << "Error when checking RED for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0x00, sample[index + 1]) << "Error when checking GREEN for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0x00, sample[index + 2]) << "Error when checking BLUE for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 3]) << "Error when checking ALPHA for pixel (" << x << ", " << y << ")";

        // (Right x Bottom) + (1,1) must be WHITE
        x += 1;
        y -= 1;
        index = 4 * (y * size.height + x);
        EXPECT_EQ(0xFF, sample[index]) << "Error when checking RED for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 1]) << "Error when checking GREEN for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 2]) << "Error when checking BLUE for pixel (" << x << ", " << y << ")";
        EXPECT_EQ(0xFF, sample[index + 3]) << "Error when checking ALPHA for pixel (" << x << ", " << y << ")";
    }

    // FIXME: Leaking memory to avoid bug on WebView destructor or on test
    //        infrastructure destruction that should be fixed ASAP.
    view.release();
}

} // TestWebKitAPI
