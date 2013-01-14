#ifndef WebViewClient_h
#define WebViewClient_h

#include <nix/NIXView.h>

namespace WebKit {

class WebView;

class WebViewClient {
public:
    WebViewClient()
    {
        initialize(0);
    }

    void initialize(const NIXViewClient* client);

    const NIXViewClient& client() const { return m_client; }

    void viewNeedsDisplay(WebView*, WKRect area);
    void webProcessCrashed(WebView*, WKStringRef url);
    void webProcessRelaunched(WebView*);
    void doneWithTouchEvent(WebView*, const NIXTouchEvent&, bool wasEventHandled);
    void doneWithGestureEvent(WebView*, const NIXGestureEvent&, bool wasEventHandled);
    void pageDidRequestScroll(WebView*, WKPoint position);
    void didChangeContentsSize(WebView*, WKSize size);
    void didFindZoomableArea(WebView*, WKPoint target, WKRect area);
    void updateTextInputState(WebView*, bool isContentEditable, WKRect cursorRect, WKRect editorRect);

private:
    NIXViewClient m_client;
};

} // namespace WebKit

#endif // WebViewClient_h
