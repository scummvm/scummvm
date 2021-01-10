######################################################################
# Unit/regression tests, based on CxxTest.
# Use the 'test' target to run them.
# Edit TESTS and TESTLIBS to add more tests.
#
######################################################################

TESTS        := $(srcdir)/test/common/*.h $(srcdir)/test/audio/*.h $(srcdir)/test/math/*.h
TEST_LIBS    :=

ifdef POSIX
TEST_LIBS += test/null_osystem.o \
	backends/fs/posix/posix-fs-factory.o \
	backends/fs/posix/posix-fs.o \
	backends/fs/posix/posix-iostream.o \
	backends/fs/abstract-fs.o \
	backends/fs/stdiostream.o \
	backends/modular-backend.o \
	test/stubs.o
endif

ifdef WIN32
TEST_LIBS += test/null_osystem.o \
	backends/fs/posix/posix-fs-factory.o \
	backends/fs/posix/posix-fs.o \
	backends/fs/posix/posix-iostream.o \
	backends/fs/abstract-fs.o \
	backends/fs/stdiostream.o \
	backends/modular-backend.o \
	test/stubs.o
endif

TEST_LIBS +=	audio/libaudio.a math/libmath.a common/libcommon.a

ifeq ($(ENABLE_WINTERMUTE), STATIC_PLUGIN)
	TESTS += $(srcdir)/test/engines/wintermute/*.h
	TEST_LIBS += engines/wintermute/libwintermute.a
endif

ifeq ($(ENABLE_ULTIMA), STATIC_PLUGIN)
	TESTS += $(srcdir)/test/engines/ultima/*/*/*.h
	TEST_LIBS += engines/ultima/libultima.a
endif

#
TEST_FLAGS   := --runner=StdioPrinter --no-std --no-eh --include=$(srcdir)/test/cxxtest_mingw.h
TEST_CFLAGS  := $(CFLAGS) -I$(srcdir)/test/cxxtest
TEST_LDFLAGS := $(LDFLAGS) $(LIBS)
TEST_CXXFLAGS := $(filter-out -Wglobal-constructors,$(CXXFLAGS))

ifdef WIN32
TEST_LDFLAGS := $(filter-out -mwindows,$(TEST_LDFLAGS))
TEST_LIBS += backends/fs/windows/windows-fs-factory.o backends/fs/windows/windows-fs.o
endif

ifdef N64
TEST_LDFLAGS := $(filter-out -mno-crt0,$(TEST_LDFLAGS))
endif

ifdef PSP
TEST_LIBS += backends/platform/psp/memory.o \
	backends/platform/psp/mp3.o \
	backends/platform/psp/trace.o
endif

# Enable this to get an X11 GUI for the error reporter.
#TEST_FLAGS   += --gui=X11Gui
#TEST_LDFLAGS += -L/usr/X11R6/lib -lX11


test: test/runner
	./test/runner
test/runner: test/runner.cpp $(TEST_LIBS) copy-dat
	+$(QUIET_CXX)$(LD) $(TEST_CXXFLAGS) $(CPPFLAGS) $(TEST_CFLAGS) -o $@ test/runner.cpp $(TEST_LIBS) $(TEST_LDFLAGS)
test/runner.cpp: $(TESTS)
	@mkdir -p test
	$(srcdir)/test/cxxtest/cxxtestgen.py $(TEST_FLAGS) -o $@ $+

clean: clean-test
clean-test:
	-$(RM) test/runner.cpp test/runner test/engine-data/encoding.dat
	-rmdir test/engine-data

copy-dat:
	$(MKDIR) test/engine-data
	$(CP) $(srcdir)/dists/engine-data/encoding.dat test/engine-data/encoding.dat

.PHONY: test clean-test copy-dat
