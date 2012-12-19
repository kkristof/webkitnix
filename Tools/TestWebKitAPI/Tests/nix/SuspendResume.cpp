#include "config.h"

#include "GLUtilities.h"
#include "PageLoader.h"
#include "PlatformUtilities.h"
#include "NIXView.h"
#include "NIXViewAutoPtr.h"
#include "WebKit2/WKContext.h"
#include "WebKit2/WKRetainPtr.h"

namespace TestWebKitAPI {

TEST(WebKitNix, SuspendResumeAPI)
{
    // This test uses a page that changes its title - showing current time.
    // As the suspend method is called the page's changes are paused. The
    // animation continues as we call the resume method. During this process
    // there are checkpoints to ensure that changes are pausing/resuming.


    const WKSize size = WKSizeMake(150, 100);
    ToolsNix::GLOffscreenBuffer offscreenBuffer(size.width, size.height);
    ASSERT_TRUE(offscreenBuffer.makeCurrent());

    WKRetainPtr<WKContextRef> context = adoptWK(WKContextCreate());
    NIXViewAutoPtr view(NIXViewCreate(context.get(), 0));

    Util::ForceRepaintClient client(view.get());
    client.setClearColor(0, 0, 1, 1);

    NIXViewInitialize(view.get());
    NIXViewSetSize(view.get(), size);

    glViewport(0, 0, size.width, size.height);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    Util::PageLoader loader(view.get());
    loader.waitForLoadURLAndRepaint("../nix/SuspendResume");

    size_t bufferSize = WKStringGetMaximumUTF8CStringSize(WKPageCopyTitle(NIXViewGetPage(view.get())));
    char firstSampleBeforeSuspend[bufferSize];
    char secondSampleBeforeSuspend[bufferSize];
    char firstSampleAfterSuspend[bufferSize];
    char secondSampleAfterSuspend[bufferSize];
    char firstSampleAfterResume[bufferSize];

    WKStringGetUTF8CString(WKPageCopyTitle(NIXViewGetPage(view.get())), firstSampleBeforeSuspend, bufferSize);

    // After collecting the first sample we wait 0.1s to collect the next sample.
    // A repaint is needed to get viewport updated accordingly. This proccess is
    // repeated for each collected sample.
    Util::sleep(0.1);
    loader.forceRepaint();
    WKStringGetUTF8CString(WKPageCopyTitle(NIXViewGetPage(view.get())), secondSampleBeforeSuspend, bufferSize);
    // The timmer is ticking - two different samples.
    EXPECT_STRNE(firstSampleBeforeSuspend, secondSampleBeforeSuspend);

    Util::sleep(0.1);
    NIXViewSuspendActiveDOMObjectsAndAnimations(view.get());
    loader.forceRepaint();
    WKStringGetUTF8CString(WKPageCopyTitle(NIXViewGetPage(view.get())), firstSampleAfterSuspend, bufferSize);
    // The timmer is paused - still two different samples.
    EXPECT_STRNE(secondSampleBeforeSuspend, firstSampleAfterSuspend);

    Util::sleep(0.1);
    loader.forceRepaint();
    WKStringGetUTF8CString(WKPageCopyTitle(NIXViewGetPage(view.get())), secondSampleAfterSuspend, bufferSize);
    // The timmer is paused - two samples collected while paused so they are equal.
    EXPECT_STREQ(firstSampleAfterSuspend, secondSampleAfterSuspend);

    NIXViewResumeActiveDOMObjectsAndAnimations(view.get());
    Util::sleep(0.1);
    loader.forceRepaint();
    WKStringGetUTF8CString(WKPageCopyTitle(NIXViewGetPage(view.get())), firstSampleAfterResume, bufferSize);
    // The timmer is ticking again - two different samples.
    EXPECT_STRNE(secondSampleAfterSuspend, firstSampleAfterResume);
}

} // TestWebKitAPI
