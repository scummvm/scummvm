# ScummVM Makefile for Windows CE port
# Uses the VLC toolchain
# $URL$
# $Id$

########################################################################
## Set this to point to where the tools are installed on your system

wince_gcc_root = /cygdrive/e/wince-gcc-root


########################################################################
## Do you want a debug build or not?

#WINCE_DEBUG_BUILD = 1


########################################################################
## Disable whichever engines you want here

#DISABLE_SCUMM = 1
#DISABLE_SKY = 1
#DISABLE_QUEEN = 1
#DISABLE_GOB = 1
DISABLE_LURE = 1
#DISABLE_CINE = 1
#DISABLE_SAGA = 1
#DISABLE_KYRA = 1
#DISABLE_AGI = 1
#DISABLE_AGOS = 1
#DISABLE_SWORD1 = 1
#DISABLE_SWORD2 = 1
#DISABLE_TOUCHE = 1
#DISABLE_PARALLACTION = 1
DISABLE_CRUISE = 1


########################################################################
## Pick which libraries you want to use here

USE_MAD           = 1
USE_MPEG2         = 1
USE_TREMOR        = 1
#USE_TREMOLO       = 1
USE_FLAC          = 1
USE_ZLIB          = 1


########################################################################
## You're probably not going to want to change these defines...

#DISABLE_HQ_SCALERS = 1
USE_ARM_SOUND_ASM = 1
USE_ARM_SMUSH_ASM = 1


########################################################################
##    Hopefully you shouldn't need to change anything below here.     ##
########################################################################


srcdir    = ../../..
VPATH     = $(srcdir)

CXX    = arm-wince-pe-g++
LD     = arm-wince-pe-g++
AR     = arm-wince-pe-ar cru
RANLIB = arm-wince-pe-ranlib
STRIP  = arm-wince-pe-strip
WINDRES= arm-wince-pe-windres
MKDIR  = mkdir -p
RM     = rm -f
RM_REC = rm -rf
ECHO   = echo -n
CAT    = cat
AS     = arm-wince-pe-as

########################################################################
## Set up defines, includes, cflags etc

DEFINES := -D_WIN32_WCE=300 -DARM -D__ARM__ -D_ARM_ -DUNICODE -DSCUMM_NEED_ALIGNMENT
DEFINES += -DFPM_DEFAULT

DEFINES += -DNONSTANDARD_PORT
DEFINES += -DWIN32 
DEFINES += -D__stdcall= -Dcdecl= -D__cdecl__= -D__cdecl= -Wno-multichar

ifdef WINCE_DEBUG_BUILD
DEFINES += -DDEBUG -DUSE_WINDBG -g
endif

INCLUDES := -I$(srcdir) -I. -I$(srcdir)/engines -Imissing/gcc -Ilibs/include -Ilibs/include/sdl -ICEgui -ICEkeys -I$(wince_gcc_root)/include

CFLAGS := -O3 -march=armv4 -mtune=xscale

CXXFLAGS := $(CFLAGS)

LDFLAGS := -Llibs/lib -L$(wince_gcc_root)/lib
LIBS := -lSDL

ifdef USE_ZLIB
DEFINES += -DUSE_ZLIB
LIBS += -lzlib
endif

ifdef USE_MAD
DEFINES += -DUSE_MAD 
LIBS += -lmad
endif

ifdef USE_MPEG2
DEFINES += -DUSE_MPEG2
LIBS += -lmpeg2
endif

ifdef USE_TREMOR
DEFINES += -DUSE_TREMOR -DUSE_VORBIS
LIBS += -ltremorce
endif

ifdef USE_TREMOLO
DEFINES += -DUSE_TREMOR -DUSE_VORBIS
INCLUDES += -Ilibs/include/tremolo
LIBS += -llibTremolo
endif

ifdef USE_FLAC
DEFINES += -DUSE_FLAC
LIBS += -lFLAC
endif

ifdef USE_ARM_SMUSH
DEFINES += -DUSE_ARM_SMUSH
endif

LIBS += --entry WinMainCRTStartup

########################################################################
# Targets follow here

TARGET = scummvm.exe
OBJS := 
MODULE_DIRS += .

OBJS += CEActionsPocket.o CEDevice.o CEScaler.o \
        CEActionsSmartphone.o CELauncherDialog.o wince-sdl.o 
OBJS += CEgui/GUIElement.o CEgui/Panel.o CEgui/SDL_ImageResource.o \
        CEgui/ItemAction.o CEgui/PanelItem.o CEgui/Toolbar.o \
        CEgui/ItemSwitch.o CEgui/PanelKeyboard.o CEgui/ToolbarHandler.o 
OBJS += CEkeys/EventsBuffer.o
OBJS += $(srcdir)/gui/Actions.o $(srcdir)/gui/Key.o $(srcdir)/gui/KeysDialog.o 
OBJS += ../sdl/sdl.o ../sdl/graphics.o ../sdl/events.o 
OBJS += missing/missing.o
OBJS += PocketSCUMM.o ARMscaler.o

include $(srcdir)/Makefile.common

all: dist

dist: $(TARGET)
	$(STRIP) $(TARGET)

PocketSCUMM.o:
	$(WINDRES) $*.rc $@

ARMscaler.o:
	$(AS) $*.s -o $@

$(TARGET): $(OBJS)
	$(LD) $^ $(LDFLAGS) $(LIBS) -o $@ -Wl,-Map,scummvm.exe.map -Wl,--stack,65536
