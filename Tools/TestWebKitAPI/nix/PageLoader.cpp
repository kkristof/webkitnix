/*
 * Copyright (C) 2012-2013 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "PageLoader.h"
#include "PlatformUtilities.h"
#include "NIXView.h"
#include "WebKit2/WKRetainPtr.h"
#include <cstring>

#if USE(OPENGL_ES_2)
#include <GLES2/gl2.h>
#elif USE(OPENGL)
#include <GL/gl.h>
#endif

using namespace TestWebKitAPI::Util;

PageLoader::PageLoader(NIXView view)
    : m_view(view)
    , m_didFinishLoadAndRepaint(false)
{
    std::memset(&m_loaderClient, 0, sizeof(m_loaderClient));

    m_loaderClient.didFinishLoadForFrame = didFinishLoadForFrame;
    m_loaderClient.clientInfo = this;
    WKPageSetPageLoaderClient(NIXViewGetPage(m_view), &m_loaderClient);
}

void PageLoader::didForceRepaint(WKErrorRef, void* context)
{
    reinterpret_cast<PageLoader*>(context)->m_didFinishLoadAndRepaint = true;
}

void PageLoader::didFinishLoadForFrame(WKPageRef page, WKFrameRef, WKTypeRef, const void* context)
{
    WKPageForceRepaint(page, const_cast<void*>(context), &PageLoader::didForceRepaint);
}

void PageLoader::waitForLoadURLAndRepaint(const char* resource)
{
    WKRetainPtr<WKURLRef> urlRef = adoptWK(createURLForResource(resource, "html"));
    WKPageLoadURL(NIXViewGetPage(m_view), urlRef.get());
    Util::run(&m_didFinishLoadAndRepaint);
    m_didFinishLoadAndRepaint = false;
}

void PageLoader::forceRepaint()
{
    WKPageForceRepaint(NIXViewGetPage(m_view), this, &PageLoader::didForceRepaint);
    Util::run(&m_didFinishLoadAndRepaint);
    m_didFinishLoadAndRepaint = false;
}

ForceRepaintClient::ForceRepaintClient(NIXView view)
    : m_view(view)
    , m_clearR(0)
    , m_clearG(0)
    , m_clearB(0)
    , m_clearA(0)
{
    NIXViewClient viewClient;
    memset(&viewClient, 0, sizeof(NIXViewClient));
    viewClient.version = kNIXViewClientCurrentVersion;
    viewClient.clientInfo = this;
    viewClient.viewNeedsDisplay = viewNeedsDisplay;
    NIXViewSetViewClient(m_view, &viewClient);
}

void ForceRepaintClient::setClearColor(int r, int g, int b, int a)
{
    m_clearR = r;
    m_clearG = g;
    m_clearB = b;
    m_clearA = a;
}

void ForceRepaintClient::viewNeedsDisplay(NIXView, WKRect, const void* clientInfo)
{
    ForceRepaintClient* client = static_cast<ForceRepaintClient*>(const_cast<void*>(clientInfo));
    assert(client);
    assert(client->m_view);
    glClearColor(client->m_clearR, client->m_clearG, client->m_clearB, client->m_clearA);

    glClear(GL_COLOR_BUFFER_BIT);
    NIXViewPaintToCurrentGLContext(client->m_view);
}
