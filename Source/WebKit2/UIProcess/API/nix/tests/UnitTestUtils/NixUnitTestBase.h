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

#ifndef NixUnitTestBase_h
#define NixUnitTestBase_h

#include <glib.h>
#include <gtest/gtest.h>

#include <WebView.h>
#include <WebKit2/WKString.h>
#include <WebKit2/WKRetainPtr.h>

class TestWebViewClient;

namespace NixUnitTest {

class NixUnitTestBase : public ::testing::Test {
public:
    ::Nix::WebView* webView() { return m_webView; }

protected:
    NixUnitTestBase();

    virtual void SetUp();
    virtual void TearDown();

private:
    WKRetainPtr<WKContextRef> m_context;
    WKRetainPtr<WKPageGroupRef> m_pageGroup;
    ::Nix::WebView* m_webView;
    GMainLoop* m_mainLoop;
    TestWebViewClient* m_browser;
};

} // namespace NixUnitTest

#endif // NixUnitTestBase_h
