# Copyright (C) 2012 INdT - Instituto Nokia de Tecnologia
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#    * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#    * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import unittest
import os
from copy import deepcopy

from webkitpy.common.system.executive_mock import MockExecutive, MockExecutive2
from webkitpy.common.system.outputcapture import OutputCapture
from webkitpy.common.system.systemhost_mock import MockSystemHost
from webkitpy.layout_tests.port import port_testcase
from webkitpy.layout_tests.port.nix import NixPort
from webkitpy.tool.mocktool import MockOptions


class NixPortTest(port_testcase.PortTestCase):
    port_name = 'nix'
    port_maker = NixPort
    search_paths_cases = [
        {'search_paths':['nix'], 'os_name':'linux'}
    ]
    expectation_files_cases = [
        {'search_paths':['nix', 'wk2'], 'os_name':'linux'}
    ]

    def _assert_search_path(self, search_paths, os_name):
        host = MockSystemHost(os_name=os_name)
        port = self.make_port(port_name=self.port_name, host=host, options=MockOptions(webkit_test_runner=True))
        absolute_search_paths = map(port._webkit_baseline_path, search_paths)

        self.assertEquals(port.baseline_search_path(), absolute_search_paths)

    def _assert_expectations_files(self, search_paths, os_name):
        host = MockSystemHost(os_name=os_name)
        port = self.make_port(port_name=self.port_name, host=host, options=MockOptions(webkit_test_runner=True))
        self.assertEquals(port.expectations_files(), search_paths)

    def test_baseline_search_path(self):
        for case in self.search_paths_cases:
            self._assert_search_path(**case)

    def test_expectations_files(self):
        for case in self.expectation_files_cases:
            expectations_case = deepcopy(case)
            expectations_case['search_paths'] = map(lambda path: '/mock-checkout/LayoutTests/platform/%s/TestExpectations' % (path), expectations_case['search_paths'])
            self._assert_expectations_files(**expectations_case)
