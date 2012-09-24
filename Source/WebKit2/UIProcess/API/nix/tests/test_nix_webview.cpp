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

#include "UnitTestUtils/NixUnitTestBase.h"
#include "UnitTestUtils/NixUnitTestEnvironment.h"


using namespace NixUnitTest;

extern NixUnitTestEnvironment* environment;


TEST_F(NixUnitTestBase, test_webview_setsize)
{
    webView()->setSize(125,389);
    ASSERT_EQ(webView()->width(), 125);
    ASSERT_EQ(webView()->height(), 389);
}

TEST_F(NixUnitTestBase, test_webview_setfocused)
{
    webView()->setFocused(false);
    ASSERT_EQ(webView()->isFocused(), false);
    webView()->setFocused(true);
    ASSERT_EQ(webView()->isFocused(), true);
}

TEST_F(NixUnitTestBase, test_webview_setvisible)
{
    webView()->setVisible(false);
    ASSERT_EQ(webView()->isVisible(), false);
    webView()->setVisible(true);
    ASSERT_EQ(webView()->isVisible(), true);
}

TEST_F(NixUnitTestBase, test_webview_setactive)
{
    webView()->setActive(false);
    ASSERT_EQ(webView()->isActive(), false);
    webView()->setActive(true);
    ASSERT_EQ(webView()->isActive(), true);
}
