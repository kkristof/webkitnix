#include "config.h"
#include "WebViewClient.h"

#include "WebView.h"
#include <cstring>

namespace WebKit {

void WebViewClient::initialize(const NIXViewClient* client)
{
    if (!client) {
        memset(&m_client, 0, sizeof(m_client));
        return;
    }

    // TODO: Once struct versions start to matter, change WebViewClient to inherit from
    // APIClient and set the appropriate traits in APIClientTraits. We'll get the right
    // implementation of initialize() for free.
    ASSERT(client->version == kNIXViewClientCurrentVersion);
    m_client = *client;
}

void WebViewClient::viewNeedsDisplay(WebView* view, WKRect rect)
{
    if (!m_client.viewNeedsDisplay)
        return;
    m_client.viewNeedsDisplay(toAPI(view), rect, m_client.clientInfo);
}

void WebViewClient::webProcessCrashed(WebView* view, WKStringRef url)
{
    if (!m_client.webProcessCrashed)
        return;
    m_client.webProcessCrashed(toAPI(view), url, m_client.clientInfo);
}

void WebViewClient::webProcessRelaunched(WebView* view)
{
    if (!m_client.webProcessRelaunched)
        return;
    m_client.webProcessRelaunched(toAPI(view), m_client.clientInfo);
}

void WebViewClient::doneWithTouchEvent(WebView* view, const NIXTouchEvent& event, bool wasEventHandled)
{
    if (!m_client.doneWithTouchEvent)
        return;
    m_client.doneWithTouchEvent(toAPI(view), &event, wasEventHandled, m_client.clientInfo);
}

void WebViewClient::doneWithGestureEvent(WebView* view, const NIXGestureEvent& event, bool wasEventHandled)
{
    if (!m_client.doneWithGestureEvent)
        return;
    m_client.doneWithGestureEvent(toAPI(view), &event, wasEventHandled, m_client.clientInfo);
}

void WebViewClient::pageDidRequestScroll(WebView* view, WKPoint point)
{
    if (!m_client.pageDidRequestScroll)
        return;
    m_client.pageDidRequestScroll(toAPI(view), point, m_client.clientInfo);
}
void WebViewClient::didChangeContentsSize(WebView* view, WKSize size)
{
    if (!m_client.didChangeContentsSize)
        return;
    m_client.didChangeContentsSize(toAPI(view), size, m_client.clientInfo);
}

void WebViewClient::didFindZoomableArea(WebView* view, WKPoint target, WKRect area)
{
    if (!m_client.didFindZoomableArea)
        return;
    m_client.didFindZoomableArea(toAPI(view), target, area, m_client.clientInfo);
}

void WebViewClient::updateTextInputState(WebView* view, bool isContentEditable, WKRect cursorRect, WKRect editorRect)
{
    if (!m_client.updateTextInputState)
        return;
    m_client.updateTextInputState(toAPI(view), isContentEditable, cursorRect, editorRect, m_client.clientInfo);
}

} // namespace WebKit
