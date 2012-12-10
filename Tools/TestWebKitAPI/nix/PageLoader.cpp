#include "config.h"
#include "PageLoader.h"
#include "PlatformUtilities.h"
#include "WebView.h"
#include "WebKit2/WKRetainPtr.h"
#include <cstring>

#if USE(OPENGL_ES_2)
#include <GLES2/gl2.h>
#elif USE(OPENGL)
#include <GL/gl.h>
#endif

using namespace TestWebKitAPI::Util;

PageLoader::PageLoader(NIXView view)
    : m_view(view)
    , m_didFinishLoadAndRepaint(false)
{
    std::memset(&m_loaderClient, 0, sizeof(m_loaderClient));

    m_loaderClient.didFinishLoadForFrame = didFinishLoadForFrame;
    m_loaderClient.clientInfo = this;
    WKPageSetPageLoaderClient(NIXViewPageRef(m_view), &m_loaderClient);
}

void PageLoader::didForceRepaint(WKErrorRef, void* context)
{
    reinterpret_cast<PageLoader*>(context)->m_didFinishLoadAndRepaint = true;
}

void PageLoader::didFinishLoadForFrame(WKPageRef page, WKFrameRef, WKTypeRef, const void* context)
{
    WKPageForceRepaint(page, const_cast<void*>(context), &PageLoader::didForceRepaint);
}

void PageLoader::waitForLoadURLAndRepaint(const char* resource)
{
    WKRetainPtr<WKURLRef> urlRef = adoptWK(createURLForResource(resource, "html"));
    WKPageLoadURL(NIXViewPageRef(m_view), urlRef.get());
    Util::run(&m_didFinishLoadAndRepaint);
    m_didFinishLoadAndRepaint = false;
}

void PageLoader::forceRepaint()
{
    WKPageForceRepaint(NIXViewPageRef(m_view), this, &PageLoader::didForceRepaint);
    Util::run(&m_didFinishLoadAndRepaint);
    m_didFinishLoadAndRepaint = false;
}

ForceRepaintClient::ForceRepaintClient()
    : m_clearR(0)
    , m_clearG(0)
    , m_clearB(0)
    , m_clearA(0)
{
    memset(&m_viewClient, 0, sizeof(NIXViewClient));
    m_viewClient.version = kNIXViewCurrentVersion;
    m_viewClient.clientInfo = this;
    m_viewClient.viewNeedsDisplay = viewNeedsDisplay;
}

void ForceRepaintClient::setClearColor(int r, int g, int b, int a)
{
    m_clearR = r;
    m_clearG = g;
    m_clearB = b;
    m_clearA = a;
}

void ForceRepaintClient::viewNeedsDisplay(WKRect, const void* clientInfo)
{
    ForceRepaintClient* client = static_cast<ForceRepaintClient*>(const_cast<void*>(clientInfo));
    assert(client);
    assert(client->m_view);
    glClearColor(client->m_clearR, client->m_clearG, client->m_clearB, client->m_clearA);

    glClear(GL_COLOR_BUFFER_BIT);
    NIXViewPaintToCurrentGLContext(client->m_view);
}
