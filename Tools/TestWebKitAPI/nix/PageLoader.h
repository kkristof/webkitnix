#ifndef PageLoader_h
#define PageLoader_h

#include "WebKit2/WKPage.h"
#include "WebView.h"

namespace TestWebKitAPI {
namespace Util {

class ForceRepaintClient {
public:
    ForceRepaintClient();
    static void viewNeedsDisplay(WKRect, const void*);
    void setClearColor(int r, int g, int b, int a);
    void setView(NIXView* view) { m_view = view; }
    NIXViewClient* viewClient() { return &m_viewClient; }
private:
    NIXView* m_view;
    NIXViewClient m_viewClient;
    int m_clearR;
    int m_clearG;
    int m_clearB;
    int m_clearA;
};


class PageLoader
{
public:
    PageLoader(NIXView*);

    void waitForLoadURLAndRepaint(const char* resource);
    void forceRepaint();

private:
    NIXView* m_view;
    bool m_didFinishLoadAndRepaint;
    WKPageLoaderClient m_loaderClient;

    static void didForceRepaint(WKErrorRef, void* context);
    static void didFinishLoadForFrame(WKPageRef page, WKFrameRef, WKTypeRef, const void* context);
};

}
}

#endif
