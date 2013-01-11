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
#include "GLUtilities.h"

#include "WebKit2/WKAPICast.h"
#include "NIXView.h"

#include <WebKit2/WKImageCairo.h>
#include <cairo/cairo.h>
#include <cassert>

extern "C" {
static gboolean callUpdateDisplay(gpointer);
}

using namespace WebKit;
using namespace WTR;

static void pageDidRequestScroll(NIXView view, WKPoint position, const void* clientInfo)
{
    NIXViewSetScrollPosition(view, position);
}

static void viewNeedsDisplay(NIXView view, WKRect area, const void* clientInfo)
{
    PlatformWebView* platview = static_cast<PlatformWebView*>(const_cast<void*>(clientInfo));
    platview->scheduleDisplayUpdate();
}

static gboolean callUpdateDisplay(gpointer data)
{
    PlatformWebView* platview = reinterpret_cast<PlatformWebView*>(data);
    platview->performDisplayUpdate();
    return 0;
}

namespace WTR {

void PlatformWebView::scheduleDisplayUpdate()
{
    if (m_scheduledDisplayUpdateID)
        return;

    m_scheduledDisplayUpdateID = g_timeout_add(0, callUpdateDisplay, this);
}

void PlatformWebView::performDisplayUpdate()
{
    if (!m_view || !m_scheduledDisplayUpdateID)
        return;

    m_scheduledDisplayUpdateID = 0;
    bool didMakeCurrent = m_offscreenBuffer->makeCurrent();
    assert(didMakeCurrent);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    NIXViewPaintToCurrentGLContext(m_view);
}

PlatformWebView::PlatformWebView(WKContextRef context, WKPageGroupRef pageGroup, WKDictionaryRef options)
{
    m_view = NIXViewCreate(context, pageGroup);
    m_window = 0;
    m_scheduledDisplayUpdateID = 0;

    WKSize size = WKSizeMake(800, 600);

    m_offscreenBuffer = new ToolsNix::GLOffscreenBuffer(size.width, size.height);
    m_offscreenBuffer->makeCurrent();
    glViewport(0, 0, size.width, size.height);

    NIXViewClient viewClient;
    memset(&viewClient, 0, sizeof(NIXViewClient));
    viewClient.version = kNIXViewClientCurrentVersion;
    viewClient.clientInfo = this;
    viewClient.pageDidRequestScroll = pageDidRequestScroll;
    viewClient.viewNeedsDisplay = viewNeedsDisplay;
    NIXViewSetViewClient(m_view, &viewClient);

    NIXViewInitialize(m_view);
    NIXViewSetSize(m_view, size);
}

PlatformWebView::~PlatformWebView()
{
    if (m_scheduledDisplayUpdateID)
        g_source_remove(m_scheduledDisplayUpdateID);
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
    int width = NIXViewSize(m_view).width;
    int height = NIXViewSize(m_view).height;
    cairo_format_t format = CAIRO_FORMAT_ARGB32;

    WKImageOptions options;
    int stride = 0;
    stride = cairo_format_stride_for_width(format, width);

    int size = stride * height * 4;
    std::auto_ptr<unsigned char> data(new unsigned char[size]);
    if (!data.get())
        return 0;
    memset(data.get(), 0, size);

    bool didMakeCurrent = m_offscreenBuffer->makeCurrent();
    assert(didMakeCurrent);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Read line by line using the Cairo internal format.
    for (int y = 0; y < height; y++)
        glReadPixels(0, (height - (y + 1)), width, 1, GL_BGRA, GL_UNSIGNED_BYTE, data.get()+(y*stride));

    cairo_surface_t* surface = cairo_image_surface_create_for_data(data.get(), format, width, height, stride);

    if (!surface)
        return 0;

    WKImageRef resultImage =  WKImageCreateFromCairoSurface(surface, options);
    cairo_surface_destroy(surface);
    return adoptWK(resultImage);
}

} // namespace WTR

