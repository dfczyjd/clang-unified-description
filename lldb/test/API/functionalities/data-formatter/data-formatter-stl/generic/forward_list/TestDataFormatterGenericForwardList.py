"""
Test lldb data formatter subsystem.
"""



import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil

USE_LIBSTDCPP = "USE_LIBSTDCPP"
USE_LIBCPP = "USE_LIBCPP"

class TestDataFormatterGenericForwardList(TestBase):

    mydir = TestBase.compute_mydir(__file__)

    def setUp(self):
        TestBase.setUp(self)
        self.line = line_number('main.cpp', '// break here')
        self.namespace = 'std'


    def do_test(self, stdlib_type):
        """Test that std::forward_list is displayed correctly"""
        self.build(dictionary={stdlib_type: "1"})
        lldbutil.run_to_source_breakpoint(self, '// break here',
                lldb.SBFileSpec("main.cpp", False))

        forward_list = self.namespace + '::forward_list'
        self.expect("frame variable empty",
                    substrs=[forward_list,
                             'size=0',
                             '{}'])

        self.expect("frame variable one_elt",
                    substrs=[forward_list,
                             'size=1',
                             '{',
                             '[0] = 47',
                             '}'])

        self.expect("frame variable five_elts",
                    substrs=[forward_list,
                             'size=5',
                             '{',
                             '[0] = 1',
                             '[1] = 22',
                             '[2] = 333',
                             '[3] = 4444',
                             '[4] = 55555',
                             '}'])

        self.expect("settings show target.max-children-count", matching=True,
                    substrs=['target.max-children-count (int) = 256'])

        self.expect("frame variable thousand_elts",matching=False,
                    substrs=[ 
                             '[256]',
                             '[333]',
                             '[444]',
                             '[555]',
                             '[666]',
                             '...'
                             ])
        self.runCmd(
                "settings set target.max-children-count 3",
                check=False)

        self.expect("frame variable thousand_elts",matching=False,
                    substrs=[
                             '[3]',
                             '[4]',
                             '[5]',
                             ])

        self.expect("frame variable thousand_elts",matching=True,
                    substrs=[
                             'size=256',
                             '[0]',
                             '[1]',
                             '[2]',
                             '...'
                             ])

    @add_test_categories(["libstdcxx"])
    def test_libstdcpp(self):
        self.do_test(USE_LIBSTDCPP)

    @add_test_categories(["libc++"])
    def test_libcpp(self):
         self.do_test(USE_LIBCPP)
