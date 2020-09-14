# To approximate the DS builds A, B, C, ... run our configure like this
#   configure --host=ds --disable-translation --disable-all-engines OPTIONS
# where OPTIONS is...
# build A:  --enable-scumm
# build B:  --enable-sky --enable-queen
# build C:  --enable-agos
# build D:  --enable-gob --enable-cine --enable-agi
# build E:  --enable-saga --disable-mad
# build F:  --enable-kyra --disable-mad
# build G:  --enable-lure
# build H:  --enable-parallaction
# build I:  --enable-made --disable-mad
# build K:  --enable-cruise --disable-mad
#
# However, this could be automated using a simple script, which generates
# subdirs for each build, and runs configure in those subdirs with the right
# parameters (all builds would still share the same set of source code files,
# thanks to our "out of tree" building support).
#
# This does not currently take care of some things:
# * It does not #define DS_BUILD_A etc. -- most uses of that should be
#   eliminated, though. Only usage should be for selecting the default config
#   file (and for that we should really rather allow overriding the value of
#   DEFAULT_CONFIG_FILE).
#   There are a few game specific hacks which are currently controlled by this,
#   too; we need to investigate those.

all: scummvm.nds

clean: dsclean

dsclean:
	$(RM) scummvm.nds
	$(RM_REC) romfs

.PHONY: dsclean

# TODO: Add a 'dsdist' target ?

%.nds: %.elf romfs
	ndstool -c $@ -9 $< -b $(srcdir)/backends/platform/ds/logo.bmp "$(@F);ScummVM $(VERSION);DS Port" -d romfs

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


# Command to build libmad is:
# ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork'
#
# I actually had to use
# ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork' LDFLAGS='C:/Progra~1/devkitpro/libnds/lib/libnds9.a' --disable-shared --disable-debugging
#
# Fingolfin used
# CXX=arm-eabi-g++ CC=arm-eabi-gcc ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork' --disable-shared --disable-debugging LDFLAGS=$DEVKITPRO/libnds/lib/libnds9.a
