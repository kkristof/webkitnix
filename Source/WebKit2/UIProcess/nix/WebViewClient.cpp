#include "config.h"
#include "WebViewClient.h"

#include <cstring>

namespace WebKit {

void WebViewClient::initialize(const NIXViewClient* client)
{
    if (!client) {
        memset(&m_client, 0, sizeof(m_client));
        return;
    }

    ASSERT(client->version == kNIXViewClientCurrentVersion);
    m_client = *client;
}

void WebViewClient::viewNeedsDisplay(WKRect rect)
{
    if (!m_client.viewNeedsDisplay)
        return;
    m_client.viewNeedsDisplay(rect, m_client.clientInfo);
}

void WebViewClient::webProcessCrashed(WKStringRef url)
{
    if (!m_client.webProcessCrashed)
        return;
    m_client.webProcessCrashed(url, m_client.clientInfo);
}

void WebViewClient::webProcessRelaunched()
{
    if (!m_client.webProcessRelaunched)
        return;
    m_client.webProcessRelaunched(m_client.clientInfo);
}

void WebViewClient::doneWithTouchEvent(const NIXTouchEvent& event, bool wasEventHandled)
{
    if (!m_client.doneWithTouchEvent)
        return;
    m_client.doneWithTouchEvent(&event, wasEventHandled, m_client.clientInfo);
}

void WebViewClient::doneWithGestureEvent(const NIXGestureEvent& event, bool wasEventHandled)
{
    if (!m_client.doneWithGestureEvent)
        return;
    m_client.doneWithGestureEvent(&event, wasEventHandled, m_client.clientInfo);
}

void WebViewClient::pageDidRequestScroll(WKPoint point)
{
    if (!m_client.pageDidRequestScroll)
        return;
    m_client.pageDidRequestScroll(point, m_client.clientInfo);
}
void WebViewClient::didChangeContentsSize(WKSize size)
{
    if (!m_client.didChangeContentsSize)
        return;
    m_client.didChangeContentsSize(size, m_client.clientInfo);
}

void WebViewClient::didFindZoomableArea(WKPoint target, WKRect area)
{
    if (!m_client.didFindZoomableArea)
        return;
    m_client.didFindZoomableArea(target, area, m_client.clientInfo);
}

void WebViewClient::updateTextInputState(bool isContentEditable, WKRect cursorRect, WKRect editorRect)
{
    if (!m_client.updateTextInputState)
        return;
    m_client.updateTextInputState(isContentEditable, cursorRect, editorRect, m_client.clientInfo);
}

void WebViewClient::compositeCustomLayerToCurrentGLContext(uint32_t id, WKRect rect, const float* matrix, float opacity)
{
    if (!m_client.compositeCustomLayerToCurrentGLContext)
        return;
    m_client.compositeCustomLayerToCurrentGLContext(id, rect, matrix, opacity, m_client.clientInfo);
}

} // namespace WebKit
