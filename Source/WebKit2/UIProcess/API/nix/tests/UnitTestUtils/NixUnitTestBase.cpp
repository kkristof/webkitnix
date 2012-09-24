/*
    Copyright (C) 2012 Samsung Electronics
    Copyright (C) 2012 Intel Corporation. All rights reserved.
    Copyright (C) 2012 Instituto Nokia de Tecnologia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <config.h>
#include "NixUnitTestBase.h"
#include "NixUnitTestEnvironment.h"
#include <glib-object.h>

#include <wtf/UnusedParam.h>
#include <WebKit2/WKPreferences.h>
#include <WebKit2/WKPreferencesPrivate.h>

extern NixUnitTest::NixUnitTestEnvironment* environment;

class TestWebViewClient : public Nix::WebViewClient {
public:
    TestWebViewClient(GMainLoop*);
    virtual ~TestWebViewClient();

    // Nix::WebViewClient.
    virtual void viewNeedsDisplay(int, int, int, int) { updateDisplay(); }
    virtual void webProcessCrashed(WKStringRef url);
    virtual void webProcessRelaunched();
    Nix::WebView* m_webView;

private:
    void updateDisplay();

    WKRetainPtr<WKContextRef> m_context;
    WKRetainPtr<WKPageGroupRef> m_pageGroup;
    GMainLoop* m_mainLoop;
};

TestWebViewClient::TestWebViewClient(GMainLoop* mainLoop)
    : m_context(AdoptWK, WKContextCreate())
    , m_pageGroup(AdoptWK, (WKPageGroupCreateWithIdentifier(WKStringCreateWithUTF8CString(""))))
    , m_webView(0)
    , m_mainLoop(mainLoop)
{
    WKPreferencesRef preferences = WKPageGroupGetPreferences(m_pageGroup.get());
    WKPreferencesSetAcceleratedCompositingEnabled(preferences, true);
    WKPreferencesSetFrameFlatteningEnabled(preferences, true);

    m_webView = Nix::WebView::create(m_context.get(), m_pageGroup.get(), this);
    m_webView->initialize();
}

TestWebViewClient::~TestWebViewClient()
{
    delete m_webView;
}

void TestWebViewClient::updateDisplay()
{
}

void TestWebViewClient::webProcessCrashed(WKStringRef url)
{
}

void TestWebViewClient::webProcessRelaunched()
{
}

namespace NixUnitTest {

NixUnitTestBase::NixUnitTestBase()
      : m_webView(0)
      , m_context(AdoptWK, WKContextCreate())
      , m_pageGroup(AdoptWK, (WKPageGroupCreateWithIdentifier(WKStringCreateWithUTF8CString(""))))
{

}

void NixUnitTestBase::SetUp()
{
    m_mainLoop = g_main_loop_new(0, false);
    m_browser = new TestWebViewClient(m_mainLoop);
    m_webView = m_browser->m_webView;
}

void NixUnitTestBase::TearDown()
{
    delete m_browser;
    g_main_loop_unref(m_mainLoop);
}

} // namespace NixUnitTest
