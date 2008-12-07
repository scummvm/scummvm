# $URL$
# $Id$

MODULE := tools

MODULE_DIRS += \
	tools/


#######################################################################
# Tools directory
#######################################################################

TOOLS := \
	tools/delua$(EXEEXT) \
	tools/imc2wav$(EXEEXT) \
	tools/int2flt$(EXEEXT) \
	tools/set2fig$(EXEEXT) \
	tools/unlab$(EXEEXT) \
	tools/vima$(EXEEXT) \
	tools/patch_ex/patch_ex$(EXEEXT)

# below not added as it depends for ppm, bpm library
#	tools/mat2ppm$(EXEEXT)
#	tools/bm2ppm$(EXEEXT)

# Make sure the 'all' / 'clean' targets build/clean the tools, too
#all:
clean: clean-tools

# Main target
tools: $(TOOLS)

clean-tools:
	-$(RM) $(TOOLS)
	-$(RM) tools/patch_ex/*.o
	-$(RM) -r tools/patch_ex/.deps

#
# Build rules for the tools
#

tools/delua$(EXEEXT): $(srcdir)/tools/delua.cpp
	$(MKDIR) tools/$(DEPDIR)
	$(CXX) $(CFLAGS) $(DEFINES) -DHAVE_CONFIG_H -I. -Iengine -Iengine/lua -Wall \
	engine/localize.o engine/registry.o \
	engine/lua/lapi.o engine/lua/lauxlib.o engine/lua/lbuffer.o engine/lua/lbuiltin.o \
	engine/lua/ldo.o engine/lua/lfunc.o engine/lua/lgc.o engine/lua/liolib.o \
	engine/lua/llex.o engine/lua/lmathlib.o engine/lua/lmem.o engine/lua/lobject.o \
	engine/lua/lparser.o engine/lua/lrestore.o engine/lua/lsave.o engine/lua/lstate.o \
	engine/lua/lstring.o engine/lua/lstrlib.o engine/lua/ltable.o engine/lua/ltask.o \
	engine/lua/ltm.o engine/lua/lundump.o engine/lua/lvm.o engine/lua/lzio.o \
	-Lcommon -lcommon $(LIBS) -o $@ $<

tools/mat2ppm$(EXEEXT): $(srcdir)/tools/mat2ppm.cpp
	$(MKDIR) tools/$(DEPDIR)
	$(CXX) $(CFLAGS) -Wall -lppm -o $@ $<

tools/bmtoppm$(EXEEXT): $(srcdir)/tools/bmtoppm.cpp
	$(MKDIR) tools/$(DEPDIR)
	$(CXX) $(CFLAGS) -Wall -lppm -lpbm -o $@ $<

tools/imc2wav$(EXEEXT): $(srcdir)/tools/imc2wav.cpp
	$(MKDIR) tools/$(DEPDIR)
	$(CXX) $(CFLAGS) -Wall -o $@ $<

tools/int2flt$(EXEEXT): $(srcdir)/tools/int2flt.cpp
	$(MKDIR) tools/$(DEPDIR)
	$(CXX) $(CFLAGS) -Wall -o $@ $<

tools/set2fig$(EXEEXT): $(srcdir)/tools/set2fig.cpp
	$(MKDIR) tools/$(DEPDIR)
	$(CXX) $(CFLAGS) -Wall -o $@ $<

tools/unlab$(EXEEXT): $(srcdir)/tools/unlab.cpp
	$(MKDIR) tools/$(DEPDIR)
	$(CXX) $(CFLAGS) -Wall -o $@ $<

tools/vima$(EXEEXT): $(srcdir)/tools/vima.cpp
	$(MKDIR) tools/$(DEPDIR)
	$(CXX) $(CFLAGS) -Wall -o $@ $<

tools/patch_ex/patch_ex$(EXEEXT): $(srcdir)/tools/patch_ex/patch_ex.o $(srcdir)/tools/patch_ex/mszipd.o $(srcdir)/tools/patch_ex/cabd.o
	$(MKDIR) tools/patch_ex/$(DEPDIR)
	$(CXX) $(CFLAGS) tools/patch_ex/mszipd.o tools/patch_ex/cabd.o -Wall -o $@ $<

.PHONY: clean-tools tools
