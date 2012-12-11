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
#include "NIXView.h"

static void pageDidRequestScroll(WKPoint position, const void* clientInfo)
{
    if (!clientInfo)
        return;
    NIXView view = static_cast<NIXView>(const_cast<void*>(clientInfo));
    NIXViewSetScrollPosition(view, position);
}

using namespace WebKit;

namespace WTR {

PlatformWebView::PlatformWebView(WKContextRef context, WKPageGroupRef pageGroup, WKDictionaryRef options)
{
    m_view = NIXViewCreate(context, pageGroup);
    m_window = 0;

    NIXViewClient viewClient;
    memset(&viewClient, 0, sizeof(NIXViewClient));
    viewClient.version = kNIXViewClientCurrentVersion;
    viewClient.pageDidRequestScroll = pageDidRequestScroll;
    viewClient.clientInfo = m_view;
    NIXViewSetViewClient(m_view, &viewClient);

    NIXViewInitialize(m_view);
    NIXViewSetSize(m_view, WKSizeMake(800, 600));
}

PlatformWebView::~PlatformWebView()
{
    NIXViewRelease(m_view);
}

void PlatformWebView::resizeTo(unsigned width, unsigned height)
{
    NIXViewSetSize(m_view, WKSizeMake(width, height));
}

WKPageRef PlatformWebView::page()
{
    return NIXViewGetPage(m_view);
}

void PlatformWebView::focus()
{
    NIXViewSetFocused(m_view, true);
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

