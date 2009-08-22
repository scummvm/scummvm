######################################################################
# Unit/regression tests, based on CxxTest.
# Use the 'test' target to run them.
# Edit TESTS and TESTLIBS to add more tests.
#
######################################################################

TESTS        := $(srcdir)/test/common/*.h $(srcdir)/test/sound/*.h
TEST_LIBS    := common/libcommon.a sound/libsound.a

#
TEST_FLAGS   := --runner=StdioPrinter
TEST_CFLAGS  := -I$(srcdir)/test/cxxtest
TEST_LDFLAGS :=


# Enable this to get an X11 GUI for the error reporter.
#TEST_FLAGS   += --gui=X11Gui
#TEST_LDFLAGS += -L/usr/X11R6/lib -lX11


test: test/runner
	./test/runner
test/runner: test/runner.cpp $(TEST_LIBS)
	$(QUIET_LINK)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TEST_LDFLAGS) $(TEST_CFLAGS) -o $@ $+
test/runner.cpp: $(TESTS)
	@mkdir -p test
	$(srcdir)/test/cxxtest/cxxtestgen.py $(TEST_FLAGS) -o $@ $+


clean: clean-test
clean-test:
	-$(RM) test/runner.cpp test/runner

.PHONY: test clean-test
