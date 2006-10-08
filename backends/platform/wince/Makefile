# ScummVM Makefile for Windows CE port
# Uses the VLC toolchain
# $URL:$
# $Id:$

srcdir    = ../../..
VPATH     = $(srcdir)
#HAVE_GCC3 = false

#DISABLE_SCUMM = 1
#DISABLE_SIMON = 1
#DISABLE_SKY = 1
#DISABLE_QUEEN = 1
#DISABLE_GOB = 1
DISABLE_LURE = 1
DISABLE_CINE = 1
DISABLE_SAGA = 1
#DISABLE_KYRA = 1
DISABLE_AGI = 1
#DISABLE_AGOS = 1
#DISABLE_SWORD1 = 1
#DISABLE_SWORD2 = 1


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


DEFINES := -D_WIN32_WCE=300 -DARM -D__ARM__ -D_ARM_ -DUNICODE -DSCUMM_NEED_ALIGNMENT -DFPM_DEFAULT -DUSE_MAD -DNONSTANDARD_PORT -DUSE_TREMOR -DUSE_VORBIS -DUSE_MPEG2 -DWIN32 -DUSE_ZLIB
DEFINES += -D__stdcall= -Dcdecl= -D__cdecl__= -D__cdecl= -Wno-multichar
DEFINES += -O2
DEFINES += -DDEBUG -DUSE_WINDBG -g

INCLUDES := -I$(srcdir) -I. -I$(srcdir)/engines -Ilibs/include -Ilibs/include/sdl -ICEgui -ICEkeys

CFLAGS := 

CXXFLAGS := $(CFLAGS)

LDFLAGS := -Llibs/lib -L/usr/local/wince/cross-tools/lib
LIBS := -lSDL -lzlib -lmad -lmpeg2 -ltremorce --entry WinMainCRTStartup

TARGET = scummvm.exe
OBJS := 
MODULE_DIRS += .

OBJS += CEActionsPocket.o CEDevice.o CEKeysDialog.o CEScaler.o \
        CEActionsSmartphone.o CELauncherDialog.o wince-sdl.o 
OBJS += CEgui/GUIElement.o CEgui/Panel.o CEgui/SDL_ImageResource.o \
        CEgui/ItemAction.o CEgui/PanelItem.o CEgui/Toolbar.o \
        CEgui/ItemSwitch.o CEgui/PanelKeyboard.o CEgui/ToolbarHandler.o 
OBJS += CEkeys/EventsBuffer.o
OBJS += $(srcdir)/gui/Actions.o $(srcdir)/gui/Key.o $(srcdir)/gui/KeysDialog.o 
OBJS += ../sdl/sdl.o ../sdl/graphics.o ../sdl/events.o 
OBJS += missing/missing.o
OBJS += PocketSCUMM.o

include $(srcdir)/Makefile.common

all: dist

dist: $(TARGET)
	bzip2 -fvk $(TARGET)
	$(STRIP) $(TARGET)

PocketSCUMM.o:
	$(WINDRES) $*.rc $@

$(TARGET): $(OBJS)
	$(LD) $^ $(LDFLAGS) $(LIBS) -o $@ -Wl,-Map,scummvm.exe.map -Wl,--stack,65536
