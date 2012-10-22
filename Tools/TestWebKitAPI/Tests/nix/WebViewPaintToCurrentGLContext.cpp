#include "config.h"

#include "GLUtilities.h"
#include "PageLoader.h"
#include "WebView.h"
#include "WebKit2/WKContext.h"
#include "WebKit2/WKRetainPtr.h"
#include <GL/gl.h>
#include <memory>

namespace TestWebKitAPI {

TEST(WebKitNix, WebViewPaintToCurrentGLContext)
{
    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());
    Util::ForceRepaintClient client;
    std::auto_ptr<Nix::WebView> webView(Nix::WebView::create(context.get(), 0, &client));
    client.setView(webView.get());
    client.setClearColor(0, 0, 1, 1);
    webView->initialize();
    WKPageSetUseFixedLayout(webView->pageRef(), true);

    Util::PageLoader loader(webView.get());

    const unsigned width = 100;
    const unsigned height = 100;
    webView->setSize(width, height);

    Util::GLOffscreenBuffer offscreenBuffer(width, height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());

    glViewport(0, 0, width, height);
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

    // FIXME: Leaking memory to avoid bug on WebView destructor or on test
    //        infrastructure destruction that should be fixed ASAP.
    webView.release();
}

} // TestWebKitAPI
