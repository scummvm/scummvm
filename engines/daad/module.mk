MODULE := engines/daad

MODULE_OBJS := \
	daad.o \
	daad_file.o \
	daad_vid.o \
	metaengine.o

# The vendored ADP interpreter
# Unchanged from its parent repository
DAAD_ADP_SOURCES := \
	ddb ddb_data ddb_draw ddb_dump ddb_inp ddb_pal ddb_paw ddb_play ddb_psg \
	ddb_run ddb_scr ddb_snap ddb_vid ddb_wintext ddb_xmsg \
	dim dim_adf dim_cpc dim_fat \
	dmg dmg_audio dmg_cach dmg_cga dmg_ega dmg_font dmg_imgc dmg_imgp dmg_pcx \
	dmg_rlec dmg_rleo dmg_rleop dmg_univ dmg_zx0 \
	os_arena os_char os_lib os_mem scrfile vid_font

MODULE_OBJS += $(addprefix adp/src-common/, $(addsuffix .o, $(DAAD_ADP_SOURCES)))

MODULE_DIRS += \
	engines/daad

# This module can be built as a plugin
ifeq ($(ENABLE_DAAD), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# ADP build configuration, applied as target specific variables so that none
# of it leaks into the rest of the build.
#
#  _STDCLIB              use stdint/stdbool/stdlib types; without it os_types.h
#                        redefines bool, size_t and NULL and clashes with C++
#  HAS_VIRTUALFILESYSTEM File is a vtable struct, implemented in daad_file.cpp
#  HAS_CLIPBOARD         paste into the input line via OSystem
#  HAS_PCX/HAS_XMSG      external .VGA/.PCX pictures and extended messages
#  HAS_SPECTRUM          .zxs pictures and Spectrum .SCR loading screens
#  HAS_DRAWSTRING        vector graphics condacts (src-common/ddb_draw.cpp)
#  HAS_SNAPSHOTS/HAS_PAWS  databases embedded in snapshots, and PAWS SDBs
#  HAS_PSG               Atari ST / Amiga PSG music
#
# Deliberately NOT defined: _UNIX (it would pull in dirent based code and the
# Salvador ZX0 *compressor*, which the player does not need), HAS_TESTMODE,
# HAS_FULLSCREEN and DEBUG_ALLOCS.
DAAD_ADP_BASE := -I$(srcdir)/engines/daad/adp/include
DAAD_ADP_BASE += -D_STDCLIB
DAAD_ADP_BASE += -DHAS_VIRTUALFILESYSTEM -DHAS_CLIPBOARD -DHAS_XMSG -DHAS_PCX
DAAD_ADP_BASE += -DHAS_SPECTRUM -DHAS_DRAWSTRING -DHAS_SNAPSHOTS -DHAS_PAWS
DAAD_ADP_BASE += -DHAS_PSG -DHAS_HIRES_FONT=1 -DHAS_WINDOWTEXT=1

# The vendored sources get the renaming layer force included.
# Our own shim files include it themselves, after their ScummVM includes 
# (see the comment at the top of adp_prefix.h for why the order matters)
DAAD_ADP_FLAGS := $(DAAD_ADP_BASE) -include $(srcdir)/engines/daad/adp_prefix.h

DAAD_ADP_OBJS := $(addprefix engines/daad/adp/src-common/, $(addsuffix .o, $(DAAD_ADP_SOURCES)))
DAAD_SHIM_OBJS := $(addprefix engines/daad/, daad.o daad_file.o daad_vid.o)

$(DAAD_ADP_OBJS): CXXFLAGS += $(DAAD_ADP_FLAGS)
$(DAAD_SHIM_OBJS): CXXFLAGS += $(DAAD_ADP_BASE)
