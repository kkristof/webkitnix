#ifndef PageLoader_h
#define PageLoader_h

#include "WebKit2/WKPage.h"
#include "NIXView.h"

namespace TestWebKitAPI {
namespace Util {

class ForceRepaintClient {
public:
    ForceRepaintClient(NIXView);
    void setClearColor(int r, int g, int b, int a);

private:
    static void viewNeedsDisplay(NIXView, WKRect, const void*);

    NIXView m_view;
    int m_clearR;
    int m_clearG;
    int m_clearB;
    int m_clearA;
};


class PageLoader
{
public:
    PageLoader(NIXView);

    void waitForLoadURLAndRepaint(const char* resource);
    void forceRepaint();

private:
    NIXView m_view;
    bool m_didFinishLoadAndRepaint;
    WKPageLoaderClient m_loaderClient;

    static void didForceRepaint(WKErrorRef, void* context);
    static void didFinishLoadForFrame(WKPageRef page, WKFrameRef, WKTypeRef, const void* context);
};

}
}

#endif
