#include "config.h"

#include "GLUtilities.h"
#include "PageLoader.h"
#include "PlatformUtilities.h"
#include "WebView.h"
#include "WebKit2/WKContext.h"
#include "WebKit2/WKRetainPtr.h"
#include <GL/gl.h>
#include <memory>

namespace TestWebKitAPI {

static bool s_didFindZoomableArea;
const WKPoint touchPoint = {13, 18};

class TestWebViewClient : public Util::ForceRepaintClient
{
    void didFindZoomableArea(WKPoint target, WKRect area)
    {
        EXPECT_EQ(target.x, touchPoint.x);
        EXPECT_EQ(target.y, touchPoint.y);
        EXPECT_EQ(area.origin.x, 0);
        EXPECT_EQ(area.origin.y, 0);
        EXPECT_EQ(area.size.width, 57);
        EXPECT_EQ(area.size.height, 43);
        s_didFindZoomableArea = true;
    }
};

TEST(WebKitNix, WebViewFindZoomableArea)
{
    // This test opens a webpage that contains a white background, no viewport
    // metatag and a red rectangle (57x43)px at (0,0) position. After that we call
    // findZoomableArea and it should return the rectangle's size + coordinates
    // from touch point (13,18).

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());

    TestWebViewClient client;
    std::auto_ptr<Nix::WebView> webView(Nix::WebView::create(context.get(), 0, &client));

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

    loader.waitForLoadURLAndRepaint("../nix/red-rectangle");

    // Using same touch radius as MockedTouchPoint::MockedTouchPoint()
    webView->findZoomableAreaForPoint(touchPoint.x, touchPoint.y, 20, 20);

    Util::run(&s_didFindZoomableArea);
}

} // TestWebKitAPI
