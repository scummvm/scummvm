######################################################################
# Unit/regression tests, based on CxxTest.
# Use the 'test' target to run them.
# Edit TESTS and TESTLIBS to add more tests.
#
######################################################################

TESTS        := test/common/*.h
TEST_LIBS    := common/libcommon.a

#
TEST_FLAGS   := --runner=StdioPrinter
TEST_CFLAGS  := -Itest/cxxtest
TEST_LDFLAGS := 


# Enable this to get an X11 GUI for the error reporter.
#TEST_FLAGS   += --gui=X11Gui
#TEST_LDFLAGS += -L/usr/X11R6/lib -lX11


test: test/runner
	./test/runner
test/runner: test/runner.cpp $(TEST_LIBS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TEST_LDFLAGS) $(TEST_CFLAGS) -o $@ $+
test/runner.cpp: $(TESTS)
	test/cxxtest/cxxtestgen.py $(TEST_FLAGS) -o $@ $+

.PHONY: test
