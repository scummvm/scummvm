# This toggle redirects stdout/err over a USB Gecko adapter in memcard slot b.
# It is safe to keep this enabled, even if no such adapter is present.
DEBUG_WII_USBGECKO = 1

# This toggle enables simple memory statistics. The amount of physical
# available memory will be printed to stderr when it shrinks.
# The buckets of the allocator are not taken into account.
DEBUG_WII_MEMSTATS = 0

# This toggle sets up the GDB stub. Upon a crash or a reset button press
# attach a remote GDB via `make debug` (requires a USB Gecko adapter).
DEBUG_WII_GDB = 0

# Builds a Gamecube version. Cleanup object files before flipping this!
GAMECUBE = 0

# List of game engines to compile in. Comment the line to disable an engine.
ENABLE_SCUMM = STATIC_PLUGIN
ENABLE_SCUMM_7_8 = STATIC_PLUGIN
ENABLE_HE = STATIC_PLUGIN
# ENABLE_AGI = STATIC_PLUGIN
ENABLE_AGOS = STATIC_PLUGIN
ENABLE_CINE = STATIC_PLUGIN
ENABLE_CRUISE = STATIC_PLUGIN
ENABLE_DRASCULA = STATIC_PLUGIN
ENABLE_GOB = STATIC_PLUGIN
ENABLE_GROOVIE = STATIC_PLUGIN
ENABLE_IGOR = STATIC_PLUGIN
ENABLE_KYRA = STATIC_PLUGIN
ENABLE_LURE = STATIC_PLUGIN
ENABLE_M4 = STATIC_PLUGIN
ENABLE_MADE = STATIC_PLUGIN
ENABLE_PARALLACTION = STATIC_PLUGIN
ENABLE_QUEEN = STATIC_PLUGIN
ENABLE_SAGA = STATIC_PLUGIN
ENABLE_IHNM = STATIC_PLUGIN
ENABLE_SAGA2 = STATIC_PLUGIN
ENABLE_SKY = STATIC_PLUGIN
ENABLE_SWORD1 = STATIC_PLUGIN
ENABLE_SWORD2 = STATIC_PLUGIN
ENABLE_TINSEL = STATIC_PLUGIN
ENABLE_TOUCHE = STATIC_PLUGIN
ENABLE_TUCKER = STATIC_PLUGIN

# Scalers are currently not supported by this port.
DISABLE_HQ_SCALERS = 1
DISABLE_SCALERS = 1

# MT32 emulation, included in ScummVM. The Gamecube does not have enough
# memory to use it.
ifeq ($(GAMECUBE),1)
USE_MT32EMU = 0
else
USE_MT32EMU = 1
endif

# Additional features to compile in. zLib and MAD are inluded in libogc, the
# others require installed headers and crosscompiled static libraries.
USE_ZLIB = 1
USE_MAD = 1
USE_TREMOR = 1
USE_FLAC = 1
#USE_MPEG2 = 1

#
# Don't change anything below this line unless you know what you are doing.
#

srcdir = ../../..
VPATH = $(srcdir)
HAVE_GCC3 = 1

DISTPATH = $(srcdir)/dists/wii

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

PREFIX  = $(DEVKITPPC)/bin/powerpc-gekko-
CXX     = $(PREFIX)g++
AS      = $(PREFIX)gcc
LD      = $(PREFIX)gcc
AR      = $(PREFIX)ar cru
RANLIB  = $(PREFIX)ranlib
STRIP   = $(PREFIX)strip -g
GDB     = $(PREFIX)gdb
MKDIR   = mkdir -p
RM      = rm -f
RM_REC  = rm -rf
CP      = cp -f

ifeq ($(GAMECUBE),1)
TARGET  = scummvm-gc
MACHDEP  = -DGEKKO -DGAMECUBE -mogc -mcpu=750 -meabi -mhard-float \
			-ffunction-sections -fdata-sections -fmodulo-sched
LIBDIR   = $(DEVKITPRO)/libogc/lib/cube
LIBS      = -lstdc++ -lfat -logc -lm
else
TARGET  = scummvm-wii
MACHDEP  = -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float \
			-ffunction-sections -fdata-sections -fmodulo-sched
LIBDIR   = $(DEVKITPRO)/libogc/lib/wii
LIBS      = -lstdc++ -ldi -lfat -lwiiuse -lbte -logc -lm
endif

INCDIR   = $(srcdir) . $(srcdir)/engines/ $(DEVKITPRO)/libogc/include

CXXFLAGS = -g -Os -Wall $(MACHDEP) -D__WII__ -Wno-multichar -Wno-long-long \
			-Wno-unknown-pragmas -Wno-reorder -fno-exceptions -fno-rtti

CXXFLAGS += $(addprefix -I,$(INCDIR))
LDFLAGS   = -g $(MACHDEP) -Wl,-Map,$(TARGET).elf.map
LDFLAGS  += $(addprefix -L,$(LIBDIR))

CXXFLAGS += -I$(DEVKITPRO)/3rd/wii/include
LDFLAGS  += -L$(DEVKITPRO)/3rd/wii/lib

ifeq ($(DEBUG_WII_USBGECKO),1)
CXXFLAGS += -DDEBUG_WII_USBGECKO
LIBS     += -ldb
endif

ifeq ($(DEBUG_WII_MEMSTATS),1)
CXXFLAGS += -DDEBUG_WII_MEMSTATS
endif

ifeq ($(DEBUG_WII_GDB),1)
CXXFLAGS += -DDEBUG_WII_GDB
endif

ifeq ($(USE_ZLIB),1)
CXXFLAGS += -DUSE_ZLIB
LIBS     += -lz
endif

ifeq ($(USE_MAD),1)
CXXFLAGS += -DUSE_MAD -I$(DEVKITPRO)/libogc/include/mad
LIBS     += -lmad
endif

ifeq ($(USE_TREMOR),1)
CXXFLAGS += -DUSE_VORBIS -DUSE_TREMOR
LIBS     += -lvorbisidec
endif

ifeq ($(USE_FLAC),1)
CXXFLAGS += -DUSE_FLAC
LIBS     += -lFLAC
endif

ifeq ($(USE_MPEG2),1)
CXXFLAGS += -DUSE_MPEG2
LIBS     += -lmpeg2
endif

ifeq ($(USE_MT32EMU),1)
CXXFLAGS += -DUSE_MT32EMU
endif

OBJS := backends/platform/wii/main.o \
		backends/platform/wii/gx_supp.o \
		backends/platform/wii/osystem.o \
		backends/platform/wii/osystem_gfx.o \
		backends/platform/wii/osystem_sfx.o \
		backends/platform/wii/osystem_events.o

include $(srcdir)/Makefile.common

.PHONY: clean-wii distclean-wii upload dist

all: $(TARGET).dol

$(TARGET).dol: $(TARGET).elf
	$(DEVKITPPC)/bin/elf2dol $< $@

$(TARGET).elf: $(OBJS)
	$(LD) $^ $(LDFLAGS) $(LIBS) -o $@

clean: clean-wii

clean-wii:
	@$(RM) $(TARGET).elf $(TARGET).elf.map $(TARGET).dol

distclean: distclean-wii

distclean-wii:
	@$(RM_REC) dist

upload: $(TARGET).dol
ifeq ($(GAMECUBE),1)
	$(DEVKITPPC)/bin/geckoupload $(TARGET).dol
else
	$(DEVKITPPC)/bin/wiiload $(TARGET).dol
endif

gdb:
	$(GDB) -n $(TARGET).elf

debug:
	$(GDB) -n $(TARGET).elf -x gdb.txt

dist:
	$(MKDIR) dist/scummvm
ifeq ($(GAMECUBE),1)
	$(CP) $(TARGET).dol dist/scummvm/
else
	$(CP) $(TARGET).dol dist/scummvm/boot.dol
	$(CP) $(DISTPATH)/meta.xml dist/scummvm/
	$(CP) $(DISTPATH)/icon.png dist/scummvm/
endif
	$(CP) $(DISTPATH)/READMII dist/scummvm/
	$(CP) $(srcdir)/AUTHORS dist/scummvm/
	$(CP) $(srcdir)/COPYING dist/scummvm/
	$(CP) $(srcdir)/COPYRIGHT dist/scummvm/
	$(CP) $(srcdir)/NEWS dist/scummvm/
	$(CP) $(srcdir)/README dist/scummvm/
	$(CP) $(DIST_FILES_THEMES) dist/scummvm/
ifneq ($(DIST_FILES_ENGINEDATA),)
	$(CP) $(DIST_FILES_ENGINEDATA) dist/scummvm/
endif

