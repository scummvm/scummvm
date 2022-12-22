ifdef DYNAMIC_MODULES
DESCRIPTION ?= Built with dynamic plugin support
else
DESCRIPTION ?= DS Port
endif

NDSTOOL ?= ndstool
GRIT ?= grit

all: scummvm.nds

clean: dsclean

dsclean:
	$(RM) backends/platform/ds/gfx/*.h
	$(RM) backends/platform/ds/gfx/*.s
	$(RM) scummvm.nds
	$(RM) map.txt
	$(RM_REC) romfs
	$(RM_REC) dsdist

dsdist: scummvm.nds $(DIST_FILES_DOCS)
	rm -rf dsdist
	mkdir -p dsdist
	cp scummvm.nds dsdist/
	cp $(DIST_FILES_DOCS) dsdist/

.PHONY: dsclean dsdist

%.nds: %.elf romfs
	$(NDSTOOL) -c $@ -9 $< -b $(srcdir)/backends/platform/ds/logo.bmp "$(@F);ScummVM $(VERSION);$(DESCRIPTION)" -d romfs

romfs: $(DIST_FILES_THEMES) $(DIST_FILES_ENGINEDATA) $(DIST_FILES_NETWORKING) $(DIST_FILES_VKEYBD) $(PLUGINS)
	@rm -rf romfs
	@mkdir -p romfs
	@cp $(DIST_FILES_THEMES) romfs/
ifdef DIST_FILES_ENGINEDATA
	@cp $(DIST_FILES_ENGINEDATA) romfs/
endif
ifdef DIST_FILES_NETWORKING
	@cp $(DIST_FILES_NETWORKING) romfs/
endif
ifdef DIST_FILES_VKEYBD
	@cp $(DIST_FILES_VKEYBD) romfs/
endif
ifeq ($(DYNAMIC_MODULES),1)
	@mkdir -p romfs/plugins
	@for i in $(PLUGINS); do $(STRIP) --strip-debug $$i -o romfs/plugins/`basename $$i`; done
endif


QUIET_GRIT    = @echo '   ' GRIT '   ' $@;

vpath %.grit $(srcdir)
vpath %.png $(srcdir)

%.s %.h	: %.png %.grit
	$(QUIET)$(MKDIR) $(*D)
	$(QUIET_GRIT)$(GRIT) $< -fts -o$*

backends/platform/ds/ds-graphics.o: backends/platform/ds/gfx/banner.o


# Command to build libmad is:
# ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork'
#
# I actually had to use
# ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork' LDFLAGS='C:/Progra~1/devkitpro/libnds/lib/libnds9.a' --disable-shared --disable-debugging
#
# Fingolfin used
# CXX=arm-eabi-g++ CC=arm-eabi-gcc ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork' --disable-shared --disable-debugging LDFLAGS=$DEVKITPRO/libnds/lib/libnds9.a
