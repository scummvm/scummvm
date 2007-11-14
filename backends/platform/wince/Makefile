# ScummVM Makefile for Windows CE port
# Uses the cegcc toolchain. For build info check out the wiki: http://wiki.scummvm.org
# $URL$
# $Id$

########################################################################
## Do you want a debug build or not?

#WINCE_DEBUG_BUILD = 1
#UNOPTIMIZED_BUILD = 1

########################################################################
## Do you want a build using plugins?

BUILD_PLUGINS = 1

########################################################################
## Disable whichever engines you want here

#DISABLE_SCUMM = 1
#DISABLE_SKY = 1
#DISABLE_QUEEN = 1
#DISABLE_GOB = 1
#DISABLE_LURE = 1
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
DISABLE_DRASCULA = 1
DISABLE_IGOR = 1

########################################################################
## Pick which libraries you want to use here

USE_MAD           = 1
USE_MPEG2         = 1
USE_TREMOR        = 1
#USE_TREMOLO       = 1
USE_FLAC          = 1
USE_ZLIB          = 1

########################################################################
## For remote deployment (copying the built files onto a device
## automatically), we need to know various things, like which
## tools to use. I use pput and pdel from
##     http://www.xs4all.nl/~itsme/projects/xda/tools.html
## and I keep my installation on the storage card, but other people
## may prefer other things.

REMOTE_COPY      = pput
REMOTE_DELETE    = pdel
REMOTE_DIRECTORY = \Storage Card\Program Files\Scummvm

########################################################################
## You're probably not going to want to change these defines...

#DISABLE_HQ_SCALERS = 1
USE_ARM_SOUND_ASM = 1
USE_ARM_SMUSH_ASM = 1
USE_ARM_GFX_ASM   = 1


########################################################################
##    Hopefully you shouldn't need to change anything below here.     ##
########################################################################


srcdir    = ../../..
VPATH     = $(srcdir)

CXX    = arm-wince-mingw32ce-g++
LD     = arm-wince-mingw32ce-g++
AR     = arm-wince-mingw32ce-ar cru
RANLIB = arm-wince-mingw32ce-ranlib
STRIP  = arm-wince-mingw32ce-strip
WINDRES= arm-wince-mingw32ce-windres
MKDIR  = mkdir -p
RM     = rm -f
RM_REC = rm -rf
ECHO   = echo -n
CAT    = cat
AS     = arm-wince-mingw32ce-as

########################################################################
## Set up defines, includes, cflags etc

DEFINES := -D_WIN32_WCE=300 -D__ARM__ -D_ARM_ -DUNICODE -DSCUMM_NEED_ALIGNMENT
DEFINES += -DFPM_DEFAULT

DEFINES += -DNONSTANDARD_PORT
DEFINES += -DWIN32
DEFINES += -Dcdecl= -D__cdecl__= -Wno-multichar

INCLUDES := -I$(srcdir) -I. -I$(srcdir)/engines -Imissing/gcc -Ilibs/include -Ilibs/include/sdl -ICEgui -ICEkeys

CFLAGS :=
ifndef UNOPTIMIZED_BUILD
CFLAGS += -O3 -march=armv4 -mtune=xscale
endif

LDFLAGS := -Wl,-Map,scummvm.exe.map -Wl,--stack,65536
LIBS := -Llibs/lib -lSDL

ifdef WINCE_DEBUG_BUILD
DEFINES += -DDEBUG -DUSE_WINDBG
CFLAGS += -g
LDFLAGS += -debug
endif

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

ifdef USE_ARM_SMUSH_ASM
DEFINES += -DUSE_ARM_SMUSH_ASM
endif

ifdef USE_ARM_GFX_ASM
DEFINES += -DUSE_ARM_GFX_ASM
endif

########################################################################
# Targets follow here

EXECUTABLE = scummvm.exe
CXXFLAGS := $(CFLAGS)
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
OBJS += ARMscaler.o
ifndef BUILD_PLUGINS
OBJS += PocketSCUMM.o
endif

ifdef BUILD_PLUGINS
EXECUTABLE		 = scummvm.dll
PLUGIN_PREFIX		:=
PLUGIN_SUFFIX		:= .dll
PLUGIN_EXTRA_DEPS	 = $(EXECUTABLE)
CXXFLAGS		+= -DDYNAMIC_MODULES
PLUGIN_LDFLAGS		:= -shared -L. -lscummvm -Llibs/lib
PRE_OBJS_FLAGS		:= -Wl,--whole-archive
POST_OBJS_FLAGS		:= -Wl,--export-all-symbols -Wl,--no-whole-archive -Wl,--out-implib,./libscummvm.a -shared
endif

include $(srcdir)/Makefile.common

########################################################################
## Our top level target, and what we deploy depends on whether we are
## building the plugin version or not...
ifdef BUILD_PLUGINS

all: dist-plugins

deploy: deploy-plugins deploy-exe

else

all: dist

deploy: deploy-exe

endif

dist: ARMscaler.o PocketSCUMM.o $(EXECUTABLE)

dist-plugins: $(EXECUTABLE) plugins PocketSCUMM.o plugins-stub

plugins-stub: stub.o
	$(CXX) stub.o PocketSCUMM.o -L. -lscummvm -o scummvm.exe

PocketSCUMM.o: PocketSCUMM.rc
	$(WINDRES) $*.rc $@

ARMscaler.o: ARMscaler.s
	$(AS) $*.s -o $@

clean:
	$(RM) ARMscaler.o PocketSCUMM.o stub.o

deploy-plugins: dist-plugins
	@echo Deploying DLLs
	-$(REMOTE_MKDIR) "$(REMOTE_DIRECTORY)"
	-$(REMOTE_DELETE) "$(REMOTE_DIRECTORY)\*.dll"
	$(REMOTE_COPY) plugins/*.dll "$(REMOTE_DIRECTORY)"
	$(REMOTE_COPY) scummvm.dll "$(REMOTE_DIRECTORY)"

deploy-exe: dist
	@echo Deploying modern theme - errors are normal here!
	-$(REMOTE_MKDIR) "$(REMOTE_DIRECTORY)"
	@-$(REMOTE_COPY) ../../../gui/themes/modern.ini "$(REMOTE_DIRECTORY)"
	@-$(REMOTE_COPY) ../../../gui/themes/modern.zip "$(REMOTE_DIRECTORY)"
	@echo Deploying EXE
	-$(REMOTE_DELETE) "$(REMOTE_DIRECTORY)\scummvm.exe"
	$(REMOTE_COPY) scummvm.exe "$(REMOTE_DIRECTORY)"
