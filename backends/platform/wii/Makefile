DEBUG_WII = 1

ENABLE_SCUMM = 1
ENABLE_SCUMM_7_8 = 1
ENABLE_HE = 1
# ENABLE_AGI = 1
ENABLE_AGOS = 1
ENABLE_CINE = 1
ENABLE_CRUISE = 1
ENABLE_DRASCULA = 1
ENABLE_GOB = 1
ENABLE_IGOR = 1
ENABLE_KYRA = 1
ENABLE_LURE = 1
ENABLE_M4 = 1
ENABLE_MADE = 1
ENABLE_PARALLACTION = 1
ENABLE_QUEEN = 1
ENABLE_SAGA = 1
ENABLE_SKY = 1
ENABLE_SWORD1 = 1
ENABLE_SWORD2 = 1
ENABLE_TOUCHE = 1

DISABLE_HQ_SCALERS = 1
DISABLE_SCALERS = 1

USE_ZLIB = 1
USE_MAD = 1
USE_TREMOR = 1
USE_FLAC = 1
USE_MPEG2 = 1
USE_MT32EMU = 1

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

TARGET  = scummvm-wii

MACHDEP  = -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float \
			-ffunction-sections -fdata-sections -fmodulo-sched

INCDIR   = $(srcdir) . $(srcdir)/engines/ $(DEVKITPRO)/libogc/include
LIBDIR   = $(DEVKITPRO)/libogc/lib/wii

CXXFLAGS = -g -Os -Wall $(MACHDEP) -D__WII__ -Wno-multichar -Wno-long-long \
		-Wno-unknown-pragmas -Wno-reorder -fno-exceptions -fno-rtti

CXXFLAGS += $(addprefix -I,$(INCDIR))
LDFLAGS   = -g $(MACHDEP) -Wl,-Map,$(TARGET).elf.map
LDFLAGS  += $(addprefix -L,$(LIBDIR))
LIBS      = -lstdc++ -lfat -lwiiuse -lbte -logc -lm

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
	$(DEVKITPPC)/bin/wiiload $(TARGET).dol

dist:
	$(MKDIR) dist/apps/scummvm
	$(CP) $(TARGET).dol dist/apps/scummvm/boot.dol
	$(CP) $(DISTPATH)/meta.xml dist/apps/scummvm/
	$(CP) $(DISTPATH)/icon.png dist/apps/scummvm/
	$(CP) $(DISTPATH)/READMII dist/apps/scummvm/
	$(CP) $(srcdir)/AUTHORS dist/apps/scummvm/
	$(CP) $(srcdir)/COPYING dist/apps/scummvm/
	$(CP) $(srcdir)/COPYRIGHT dist/apps/scummvm/
	$(CP) $(srcdir)/NEWS dist/apps/scummvm/
	$(CP) $(srcdir)/README dist/apps/scummvm/
	$(CP) $(DIST_FILES_THEMES) dist/apps/scummvm/
	$(CP) $(DIST_FILES_ENGINEDATA) dist/apps/scummvm/

