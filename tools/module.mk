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
	$(CXX) $(CFLAGS) $(DEFINES) -DHAVE_CONFIG_H -I. -I$(srcdir) -I$(srcdir)/engines/grim -I$(srcdir)/engines/grim/lua -Wall \
	engines/grim/localize.o engines/grim/registry.o \
	engines/grim/lua/lapi.o engines/grim/lua/lauxlib.o engines/grim/lua/lbuffer.o engines/grim/lua/lbuiltin.o \
	engines/grim/lua/ldo.o engines/grim/lua/lfunc.o engines/grim/lua/lgc.o engines/grim/lua/liolib.o \
	engines/grim/lua/llex.o engines/grim/lua/lmathlib.o engines/grim/lua/lmem.o engines/grim/lua/lobject.o \
	engines/grim/lua/lstx.o engines/grim/lua/lrestore.o engines/grim/lua/lsave.o engines/grim/lua/lstate.o \
	engines/grim/lua/lstring.o engines/grim/lua/lstrlib.o engines/grim/lua/ltable.o engines/grim/lua/ltask.o \
	engines/grim/lua/ltm.o engines/grim/lua/lundump.o engines/grim/lua/lvm.o engines/grim/lua/lzio.o \
	-Lcommon -lcommon -o $@ $<

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

tools/patch_ex/patch_ex$(EXEEXT): tools/patch_ex/patch_ex.o tools/patch_ex/mszipd.o tools/patch_ex/cabd.o
	$(MKDIR) tools/patch_ex/$(DEPDIR)
	$(CXX) $(CFLAGS) tools/patch_ex/mszipd.o tools/patch_ex/cabd.o -Wall -o $@ $<

.PHONY: clean-tools tools
