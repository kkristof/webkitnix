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

TEST(WebKitNix, SuspendResumeAPI)
{
    // This test uses a page that changes its title - showing current time.
    // As the suspend method is called the page's changes are paused. The
    // animation continues as we call the resume method. During this process
    // there are checkpoints to ensure that changes are pausing/resuming.


    const WKSize size = WKSizeMake(150, 100);
    Util::GLOffscreenBuffer offscreenBuffer(size.width, size.height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());
    Util::ForceRepaintClient client;
    std::auto_ptr<Nix::WebView> webView(Nix::WebView::create(context.get(), 0, &client));

    client.setView(webView.get());
    client.setClearColor(0, 0, 1, 1);
    webView->initialize();
    webView->setSize(size);

    glViewport(0, 0, size.width, size.height);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    Util::PageLoader loader(webView.get());
    loader.waitForLoadURLAndRepaint("../nix/SuspendResume");

    size_t bufferSize = WKStringGetMaximumUTF8CStringSize(WKPageCopyTitle(webView->pageRef()));
    char firstSampleBeforeSuspend[bufferSize];
    char secondSampleBeforeSuspend[bufferSize];
    char firstSampleAfterSuspend[bufferSize];
    char secondSampleAfterSuspend[bufferSize];
    char firstSampleAfterResume[bufferSize];

    WKStringGetUTF8CString(WKPageCopyTitle(webView->pageRef()), firstSampleBeforeSuspend, bufferSize);

    // After collecting the first sample we wait 0.1s to collect the next sample.
    // A repaint is needed to get viewport updated accordingly. This proccess is
    // repeated for each collected sample.
    Util::sleep(0.1);
    loader.forceRepaint();
    WKStringGetUTF8CString(WKPageCopyTitle(webView->pageRef()), secondSampleBeforeSuspend, bufferSize);
    // The timmer is ticking - two different samples.
    EXPECT_STRNE(firstSampleBeforeSuspend, secondSampleBeforeSuspend);

    Util::sleep(0.1);
    webView->suspendActiveDOMObjectsAndAnimations();
    loader.forceRepaint();
    WKStringGetUTF8CString(WKPageCopyTitle(webView->pageRef()), firstSampleAfterSuspend, bufferSize);
    // The timmer is paused - still two different samples.
    EXPECT_STRNE(secondSampleBeforeSuspend, firstSampleAfterSuspend);

    Util::sleep(0.1);
    loader.forceRepaint();
    WKStringGetUTF8CString(WKPageCopyTitle(webView->pageRef()), secondSampleAfterSuspend, bufferSize);
    // The timmer is paused - two samples collected while paused so they are equal.
    EXPECT_STREQ(firstSampleAfterSuspend, secondSampleAfterSuspend);

    webView->resumeActiveDOMObjectsAndAnimations();
    Util::sleep(0.1);
    loader.forceRepaint();
    WKStringGetUTF8CString(WKPageCopyTitle(webView->pageRef()), firstSampleAfterResume, bufferSize);
    // The timmer is ticking again - two different samples.
    EXPECT_STRNE(secondSampleAfterSuspend, firstSampleAfterResume);
}

} // TestWebKitAPI
