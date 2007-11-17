MODULE := tools/skycpt

MODULE_DIRS += \
	tools/skycpt/

TOOLS += \
	$(MODULE)/skycpt$(EXEEXT)

MODULE_OBJS := \
	$(MODULE)/AsciiCptCompile.o \
	$(MODULE)/KmpSearch.o \
	$(MODULE)/TextFile.o \
	$(MODULE)/cptcompiler.o \
	$(MODULE)/cpthelp.o \
	$(MODULE)/idFinder.o

$(MODULE)/skycpt$(EXEEXT): $(MODULE_OBJS)
	$(CXX) $(LDFLAGS) $+ -o $@
