# -*- Makefile -*-

#
# Tell the DDK how to generate RunTests.cpp from RunTests.tpl and the tests
#

PYTHON=python
CXXTESTGEN=$(PYTHON) $(CXXTESTDIR)/bin/cxxtestgen

TEST_SUITES=$(SUITESDIR)/*.h

RunTests.cpp: RunTests.tpl $(TEST_SUITES)
	$(CXXTESTGEN) -o $@ --template=RunTests.tpl $(TEST_SUITES)
