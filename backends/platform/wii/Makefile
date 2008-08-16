# this enables port specific debug messages and redirects stdout/err over
# usbgecko in memcard slot b
DEBUG_WII = 1

# builds a gamecube version. cleanup object files before flipping this
GAMECUBE = 0

ENABLE_SCUMM = STATIC_PLUGIN
ENABLE_SCUMM_7_8 = STATIC_PLUGIN
ENABLE_HE = STATIC_PLUGIN
# ENABLE_AGI = STATIC_PLUGIN
ENABLE_AGOS = STATIC_PLUGIN
ENABLE_CINE = STATIC_PLUGIN
ENABLE_CRUISE = STATIC_PLUGIN
ENABLE_DRASCULA = STATIC_PLUGIN
ENABLE_GOB = STATIC_PLUGIN
ENABLE_IGOR = STATIC_PLUGIN
ENABLE_KYRA = STATIC_PLUGIN
ENABLE_LURE = STATIC_PLUGIN
ENABLE_M4 = STATIC_PLUGIN
ENABLE_MADE = STATIC_PLUGIN
ENABLE_PARALLACTION = STATIC_PLUGIN
ENABLE_QUEEN = STATIC_PLUGIN
ENABLE_SAGA = STATIC_PLUGIN
ENABLE_SKY = STATIC_PLUGIN
ENABLE_SWORD1 = STATIC_PLUGIN
ENABLE_SWORD2 = STATIC_PLUGIN
ENABLE_TINSEL = STATIC_PLUGIN
ENABLE_TOUCHE = STATIC_PLUGIN

DISABLE_HQ_SCALERS = 1
DISABLE_SCALERS = 1

USE_ZLIB = 1
USE_MAD = 1
USE_TREMOR = 1
USE_FLAC = 1
USE_MPEG2 = 1
ifeq ($(GAMECUBE),1)
USE_MT32EMU = 0
else
USE_MT32EMU = 1
endif

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
OBJCOPY = $(PREFIX)objcopy
MKDIR   = mkdir -p
RM      = rm -f
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
LIBS      = -lstdc++ -lfat -lwiiuse -lbte -logc -lm
endif

INCDIR   = $(srcdir) . $(srcdir)/engines/ $(DEVKITPRO)/libogc/include

CXXFLAGS = -g -Os -Wall $(MACHDEP) -D__WII__ -Wno-multichar -Wno-long-long \
			-Wno-unknown-pragmas -Wno-reorder -fno-exceptions -fno-rtti

CXXFLAGS += $(addprefix -I,$(INCDIR))
LDFLAGS   = -g $(MACHDEP) -Wl,-Map,$(TARGET).elf.map
LDFLAGS  += $(addprefix -L,$(LIBDIR))

CXXFLAGS += -I$(DEVKITPRO)/3rd/wii/include
LDFLAGS  += -L$(DEVKITPRO)/3rd/wii/lib

ifdef DEBUG_WII
CXXFLAGS += -DDEBUG_WII
LIBS     += -ldb
endif

ifdef USE_ZLIB
CXXFLAGS += -DUSE_ZLIB
LIBS     += -lz
endif

ifdef USE_MAD
CXXFLAGS += -DUSE_MAD -I$(DEVKITPRO)/libogc/include/mad
LIBS     += -lmad
endif

ifdef USE_TREMOR
CXXFLAGS += -DUSE_VORBIS -DUSE_TREMOR
LIBS     += -lvorbisidec
endif

ifdef USE_FLAC
CXXFLAGS += -DUSE_FLAC
LIBS     += -lFLAC
endif

ifdef USE_MPEG2
CXXFLAGS += -DUSE_MPEG2
LIBS     += -lmpeg2
endif

ifdef USE_MT32EMU
CXXFLAGS += -DUSE_MT32EMU
endif

OBJS := backends/platform/wii/main.o \
		backends/platform/wii/gecko_console.o \
		backends/platform/wii/gx_supp.o \
		backends/platform/wii/osystem.o \
		backends/platform/wii/osystem_gfx.o \
		backends/platform/wii/osystem_sfx.o \
		backends/platform/wii/osystem_events.o

include $(srcdir)/Makefile.common

.PHONY: clean-wii distclean-wii upload dist

all: $(TARGET).dol

$(TARGET).dol: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).elf: $(OBJS)
	$(LD) $^ $(LDFLAGS) $(LIBS) -o $@

clean: clean-wii

clean-wii:
	@-$(RM) $(TARGET).elf $(TARGET).elf.map $(TARGET).dol

distclean: distclean-wii

distclean-wii:
	@-$(RM) dist

upload:
ifeq ($(GAMECUBE),1)
	$(DEVKITPPC)/bin/geckoupload $(TARGET).dol
else
	$(DEVKITPPC)/bin/wiiload $(TARGET).dol
endif

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
	$(CP) $(DIST_FILES_ENGINEDATA) dist/scummvm/

