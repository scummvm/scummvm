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

# Set location of ndsdir so that we can easily refer to files in it
ndsdir = backends/platform/ds

# Compiler options for files which should be optimised for speed
OPT_SPEED := -O3 -marm

# Compiler options for files which should be optimised for space
OPT_SIZE := -Os -mthumb

# By default optimize for size
CXXFLAGS += $(OPT_SIZE)

# Files listed below will be optimisied for speed, otherwise they will be optimised for space.
# TODO: Several of these files probably should not be optimized for speed, but for now
# we replicate the *precise* list from the old DS makefile, to ensure full compatibility.
# Eventually, we should tune this list.
$(ndsdir)/blitters.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
$(ndsdir)/dsmain.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
$(ndsdir)/osystem_ds.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
base/main.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
sound/rate.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
sound/softsynth/opl/mame.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/agi/sprite.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/agos/gfx.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/agos/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/cine/gfx.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/cruise/actor.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/cruise/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/draci/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/draci/sprite.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/gob/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/groovie/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/kyra/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/m4/actor.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/m4/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/m4/sprite.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/made/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/actor_path.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/actor_walk.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/actor.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/gfx.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/image.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/isomap.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/saga/sprite.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/sci/engine/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/scumm/actor.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/scumm/gfx.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/scumm/script.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/sword2/sprite.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
engines/teenagent/actor.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)
# TODO: Fingolfin says: optimizing kyra/staticres.o for size would
# save about 30k, so maybe consider that?
#engines/kyra/staticres.o: CXXFLAGS:=$(CXXFLAGS) $(OPT_SPEED)



#############################################################################
#
# ARM9 rules.
#
#############################################################################

all: scummvm.nds

clean: dsclean

dsclean:
	$(RM) scummvm.nds
	$(RM_REC) romfs

.PHONY: dsclean

# TODO: Add a 'dsdist' target ?

%.nds: %.elf romfs
	ndstool -c $@ -9 $< -b $(srcdir)/$(ndsdir)/logo.bmp "$(@F);ScummVM $(VERSION);DS Port" -d romfs

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
