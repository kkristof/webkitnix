#include "config.h"

#include "PlatformUtilities.h"
#include <WebKit2/WKContext.h>
#include <WebKit2/WKPage.h>
#include <WebKit2/WKRetainPtr.h>
#include <WebView.h>

namespace TestWebKitAPI {

static bool didFinishLoad = false;
static bool didUpdateTextInputState = false;
static bool didChangeToContentEditable = false;
static bool isDoneWithSingleTapEvent = false;

static void didFinishLoadForFrame(WKPageRef page, WKFrameRef, WKTypeRef, const void*)
{
    didFinishLoad = true;
}

namespace {
class TestWebViewClient : public Nix::WebViewClient {
public:
    //void viewNeedsDisplay(WKRect) {}

    void updateTextInputState(bool isContentEditable, WKRect cursorRect, WKRect editorRect)
    {
        didUpdateTextInputState = true;
        if (isContentEditable)
            didChangeToContentEditable = true;
    }

    void doneWithGestureEvent(const Nix::GestureEvent& event, bool wasEventHandled)
    {
        if (event.type != Nix::InputEvent::GestureSingleTap)
            return;

        isDoneWithSingleTapEvent = true;
    }
};
} // namespace

TEST(WebKitNix, WebViewWebProcessCrashed)
{
    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());

    TestWebViewClient client;
    Nix::WebView* webView = Nix::WebView::create(context.get(), 0, &client);
    webView->initialize();

    WKPageLoaderClient loaderClient;
    memset(&loaderClient, 0, sizeof(loaderClient));

    loaderClient.version = 0;
    loaderClient.didFinishLoadForFrame = didFinishLoadForFrame;
    WKPageSetPageLoaderClient(webView->pageRef(), &loaderClient);

    const WKSize size = WKSizeMake(100, 100);
    webView->setSize(size);

    WKRetainPtr<WKURLRef> editableContentUrl = adoptWK(Util::createURLForResource("../nix/single-tap-on-editable-content", "html"));
    WKPageLoadURL(webView->pageRef(), editableContentUrl.get());
    Util::run(&didFinishLoad);

    Nix::GestureEvent tapEvent;
    tapEvent.type = Nix::InputEvent::GestureSingleTap;
    tapEvent.timestamp = 0;
    tapEvent.modifiers = 0;
    tapEvent.x = 55;
    tapEvent.y = 55;
    tapEvent.globalX = 55;
    tapEvent.globalY = 55;
    tapEvent.width = 20;
    tapEvent.height = 20;
    tapEvent.deltaX = 0.0;
    tapEvent.deltaY = 0.0;
    webView->sendEvent(tapEvent);
    Util::run(&isDoneWithSingleTapEvent);

    ASSERT_TRUE(didFinishLoad);
    ASSERT_TRUE(isDoneWithSingleTapEvent);
    ASSERT_TRUE(didUpdateTextInputState);
    ASSERT_TRUE(didChangeToContentEditable);
}

} // TestWebKitAPI
