#include "config.h"

#include "PlatformUtilities.h"
#include <WebKit2/WKContext.h>
#include <WebKit2/WKPage.h>
#include <WebKit2/WKRetainPtr.h>
#include <WebView.h>
#include <memory>

namespace TestWebKitAPI {

static bool didFinishLoad = false;
static bool didUpdateTextInputState = false;
static bool didChangeToContentEditable = false;
static bool isDoneWithSingleTapEvent = false;

static void didFinishLoadForFrame(WKPageRef page, WKFrameRef, WKTypeRef, const void*)
{
    didFinishLoad = true;
}

static void updateTextInputState(bool isContentEditable, WKRect, WKRect, const void*)
{
    didUpdateTextInputState = true;
    didChangeToContentEditable = isContentEditable;
}

static void doneWithGestureEvent(const NIXGestureEvent* event, bool, const void*)
{
    isDoneWithSingleTapEvent = event->type == kNIXInputEventTypeGestureSingleTap;
}

TEST(WebKitNix, WebViewWebProcessCrashed)
{
    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());

    NIXViewClient viewClient;
    memset(&viewClient, 0, sizeof(NIXViewClient));
    viewClient.version = kNIXViewCurrentVersion;
    viewClient.updateTextInputState = updateTextInputState;
    viewClient.doneWithGestureEvent = doneWithGestureEvent;
    std::auto_ptr<NIXView> view(NIXViewCreate(context.get(), 0, &viewClient));
    NIXViewInitialize(view.get());

    WKPageLoaderClient loaderClient;
    memset(&loaderClient, 0, sizeof(loaderClient));

    loaderClient.version = 0;
    loaderClient.didFinishLoadForFrame = didFinishLoadForFrame;
    WKPageSetPageLoaderClient(NIXViewPageRef(view.get()), &loaderClient);

    const WKSize size = WKSizeMake(100, 100);
    NIXViewSetSize(view.get(), size);

    WKRetainPtr<WKURLRef> editableContentUrl = adoptWK(Util::createURLForResource("../nix/single-tap-on-editable-content", "html"));
    WKPageLoadURL(NIXViewPageRef(view.get()), editableContentUrl.get());
    Util::run(&didFinishLoad);

    NIXGestureEvent tapEvent;
    tapEvent.type = kNIXInputEventTypeGestureSingleTap;
    tapEvent.timestamp = 0;
    tapEvent.modifiers = static_cast<NIXInputEventModifiers>(0);
    tapEvent.x = 55;
    tapEvent.y = 55;
    tapEvent.globalX = 55;
    tapEvent.globalY = 55;
    tapEvent.width = 20;
    tapEvent.height = 20;
    tapEvent.deltaX = 0.0;
    tapEvent.deltaY = 0.0;
    NIXViewSendGestureEvent(view.get(), &tapEvent);
    Util::run(&isDoneWithSingleTapEvent);

    ASSERT_TRUE(didFinishLoad);
    ASSERT_TRUE(isDoneWithSingleTapEvent);
    ASSERT_TRUE(didUpdateTextInputState);
    ASSERT_TRUE(didChangeToContentEditable);
}

} // TestWebKitAPI
