#ifndef PageLoader_h
#define PageLoader_h

#include "WebKit2/WKPage.h"
#include "WebView.h"

namespace TestWebKitAPI {
namespace Util {

class ForceRepaintClient : public Nix::WebViewClient {
public:
    ForceRepaintClient();
    void viewNeedsDisplay(int, int, int, int);
    void setView(Nix::WebView* webView) { m_webView = webView; }
    void setClearColor(int r, int g, int b, int a);
private:
    Nix::WebView* m_webView;
    int m_clearR;
    int m_clearG;
    int m_clearB;
    int m_clearA;
};


class PageLoader
{
public:
    PageLoader(Nix::WebView*);

    void waitForLoadURLAndRepaint(const char* resource);

private:
    Nix::WebView* m_webView;
    bool m_didFinishLoadAndRepaint;
    WKPageLoaderClient m_loaderClient;

    static void didForceRepaint(WKErrorRef, void* context);
    static void didFinishLoadForFrame(WKPageRef page, WKFrameRef, WKTypeRef, const void* context);
};

}
}

#endif
