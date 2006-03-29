######################################################################
# Unit/regression tests, based on CxxTest.
# Use the 'test' target to run them.
# Edit 'TESTS' to add more tests.
######################################################################

CXXTEST := test/cxxtest
TESTS := test/common/*.h
CPPFLAGS += -I$(CXXTEST)
test: runner
	./runner
runner: runner.o common/libcommon.a
	$(CXX) -o $@ $+
runner.cpp: $(TESTS)
	$(CXXTEST)/cxxtestgen.py --error-printer -o  $@ $+

.PHONY: test
