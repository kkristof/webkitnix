#ifndef WebViewClient_h
#define WebViewClient_h

#include <nix/NIXView.h>

namespace WebKit {

// TODO: Once versions start to matter, change this to inherit from APIClient and set
// the appropriate traits in APIClientTraits.
class WebViewClient {
public:
    WebViewClient()
    {
        initialize(0);
    }

    void initialize(const NIXViewClient* client);

    const NIXViewClient& client() const { return m_client; }

    void viewNeedsDisplay(WKRect area);
    void webProcessCrashed(WKStringRef url);
    void webProcessRelaunched() { }
    void doneWithTouchEvent(const NIXTouchEvent&, bool wasEventHandled);
    void doneWithGestureEvent(const NIXGestureEvent&, bool wasEventHandled);
    void pageDidRequestScroll(WKPoint position);
    void didChangeContentsSize(WKSize size);
    void didFindZoomableArea(WKPoint target, WKRect area);
    void updateTextInputState(bool isContentEditable, WKRect cursorRect, WKRect editorRect);
    void compositeCustomLayerToCurrentGLContext(uint32_t id, WKRect rect, const float* matrix, float opacity);

private:
    NIXViewClient m_client;
};

} // namespace WebKit

#endif // WebViewClient_h
