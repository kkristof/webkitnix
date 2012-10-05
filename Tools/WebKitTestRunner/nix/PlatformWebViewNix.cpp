/*
 * Copyright (C) 2012 INdT - Instituto Nokia de Tecnologia
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "PlatformWebView.h"

#include "WebKit2/WKAPICast.h"
#include "WebView.h"

class WTRWebViewClient : public Nix::WebViewClient {
public:
    WTRWebViewClient() : Nix::WebViewClient(), m_view(0) {}
    void viewNeedsDisplay(int x, int y, int width, int height) {}
    void webProcessCrashed(WKStringRef) {}
    void webProcessRelaunched() {}
    void pageDidRequestScroll(int x, int y) { if (m_view) m_view->setScrollPosition(x, y); }

    void setView(Nix::WebView* view) { m_view = view; }

private:
    Nix::WebView* m_view;
};

using namespace WebKit;

namespace WTR {

PlatformWebView::PlatformWebView(WKContextRef context, WKPageGroupRef pageGroup)
{
    m_webViewClient = new WTRWebViewClient;
    m_view = Nix::WebView::create(context, pageGroup, m_webViewClient);
    static_cast<WTRWebViewClient*>(m_webViewClient)->setView(m_view);
    m_view->initialize();
    WKPageSetUseFixedLayout(m_view->pageRef(), true);
    m_window = 0;
}

PlatformWebView::~PlatformWebView()
{
    delete m_webViewClient;
}

void PlatformWebView::resizeTo(unsigned width, unsigned height)
{
    m_view->setSize(width, height);
}

WKPageRef PlatformWebView::page()
{
    return m_view->pageRef();
}

void PlatformWebView::focus()
{
    m_view->setFocused(true);
}

WKRect PlatformWebView::windowFrame()
{
    return WKRectMake(0, 0, 0, 0);
}

void PlatformWebView::setWindowFrame(WKRect frame)
{
    resizeTo(frame.size.width, frame.size.height);
}

void PlatformWebView::addChromeInputField()
{
}

void PlatformWebView::removeChromeInputField()
{
}

void PlatformWebView::makeWebViewFirstResponder()
{
}

WKRetainPtr<WKImageRef> PlatformWebView::windowSnapshotImage()
{
    // FIXME: implement to capture pixels in the UI process,
    // which may be necessary to capture things like 3D transforms.
    return 0;
}

} // namespace WTR

