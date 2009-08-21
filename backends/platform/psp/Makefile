# SCUMMVM-PSP MakeFile
# $URL$
# $Id$

ENABLED=STATIC_PLUGIN

#control build
DISABLE_SCALERS = true
DISABLE_HQ_SCALERS = true

ENABLE_SCUMM = $(ENABLED)
ENABLE_SCUMM_7_8 = $(ENABLED)
#ENABLE_HE = $(ENABLED)
ENABLE_AGI = $(ENABLED)
ENABLE_AGOS = $(ENABLED)
#ENABLE_CINE = $(ENABLED)
#ENABLE_CRUISE = $(ENABLED)
ENABLE_DRASCULA = $(ENABLED)
ENABLE_GOB = $(ENABLED)
#ENABLE_IGOR = $(ENABLED)
ENABLE_KYRA = $(ENABLED)
ENABLE_LURE = $(ENABLED)
#ENABLE_M4 = $(ENABLED)
#ENABLE_MADE = $(ENABLED)
#ENABLE_PARALLACTION = $(ENABLED)
#ENABLE_QUEEN = $(ENABLED)
#ENABLE_SAGA = $(ENABLED)
ENABLE_SKY = $(ENABLED)
ENABLE_SWORD1 = $(ENABLED)
ENABLE_SWORD2 = $(ENABLED)
#ENABLE_TINSEL = $(ENABLED)
#ENABLE_TOUCHE = $(ENABLED)


srcdir = ../../..
VPATH = $(srcdir)
HAVE_GCC3 = false

PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
LDFLAGS =

CXX     = psp-g++
AS      = psp-gcc
LD      = psp-gcc
AR      = psp-ar cru
RANLIB  = psp-ranlib
STRIP   = psp-strip
MKDIR	= mkdir -p
RM	= rm -f
RM_REC	= rm -rf
MKSFO	= mksfoex -d MEMSIZE=1
PACK_PBP = pack-pbp
FIXUP	= psp-fixup-imports

#check PSPSDK presence
ifeq ($(PSPSDK),)
$(error $$(PSPSDK) is undefined.  Use "PSPSDK := $$(shell psp-config --pspsdk-path)" in your Makefile)
endif

# Add in PSPSDK includes and libraries.
INCDIR   := $(srcdir) . $(srcdir)/engines/ . $(PSPSDK)/include
LIBDIR   := $(LIBDIR) . $(PSPSDK)/lib

CXXFLAGS = -O3 -Wall -D__PSP__ -DNONSTANDARD_PORT -DDISABLE_TEXT_CONSOLE -DDISABLE_COMMAND_LINE -DUSE_ZLIB -DDISABLE_DOSBOX_OPL -Wno-multichar `$(PSPBIN)/sdl-config --cflags`
CXXFLAGS:= $(addprefix -I,$(INCDIR)) $(CXXFLAGS)
LDFLAGS := $(addprefix -L,$(LIBDIR)) $(LDFLAGS)
LIBS     =

#comment this out if you don't want to use libmad
CXXFLAGS+= -DUSE_MAD
LIBS	+= -lmad

#comment this out if you don't want to use libtremor
CXXFLAGS+= -DUSE_VORBIS -DUSE_TREMOR
LIBS	+= -lvorbisidec

LIBS    += `$(PSPBIN)/sdl-config --libs` -lz -lstdc++ -lc -lpspdisplay -lpspgu -lpspctrl -lpspsdk -lpspnet -lpspnet_inet -lpsputility -lpspuser -lpsppower

CXXFLAGS := $(CXXFLAGS) -fno-exceptions -fno-rtti

TARGET = scummvm-psp
OBJS := powerman.o \
	psp_main.o \
	osys_psp.o \
	kbd_ss_c.o \
	kbd_s_c.o \
	kbd_ls_c.o \
	kbd_l_c.o \
	trace.o

DEPDIR = .deps

include $(srcdir)/Makefile.common

PSP_EBOOT_SFO = param.sfo
PSP_EBOOT_TITLE = ScummVM-PSP
PSP_EBOOT = EBOOT.PBP
PSP_EBOOT_ICON = icon0.png
PSP_EBOOT_ICON1 = NULL
PSP_EBOOT_PIC0 = pic0.png
PSP_EBOOT_PIC1 = pic1.png
PSP_EBOOT_SND0 = NULL
PSP_EBOOT_PSAR = NULL

all: $(PSP_EBOOT)

$(TARGET).elf: $(OBJS)
	$(LD) $^ $(LIBS) -o $@
	$(FIXUP) $@

$(PSP_EBOOT_SFO):
	$(MKSFO) '$(PSP_EBOOT_TITLE)' $@

$(PSP_EBOOT): $(TARGET).elf $(PSP_EBOOT_SFO)
	$(STRIP) $(TARGET).elf
	$(PACK_PBP) $@ $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON) $(PSP_EBOOT_ICON1) \
		$(PSP_EBOOT_PIC0) $(PSP_EBOOT_PIC1) $(PSP_EBOOT_SND0)  \
		$(TARGET).elf $(PSP_EBOOT_PSAR)

kxploit: $(TARGET).elf $(PSP_EBOOT_SFO)
	mkdir -p "$(TARGET)"
	$(STRIP) $(TARGET).elf -o $(TARGET)/$(PSP_EBOOT)
	mkdir -p "$(TARGET)%"
	$(PACK_PBP) "$(TARGET)%/$(PSP_EBOOT)" $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON)  \
		$(PSP_EBOOT_ICON1) $(PSP_EBOOT_PIC0) $(PSP_EBOOT_PIC1)  \
		$(PSP_EBOOT_SND0) NULL $(PSP_EBOOT_PSAR)

SCEkxploit: $(TARGET).elf $(PSP_EBOOT_SFO)
	mkdir -p "__SCE__$(TARGET)"
	$(STRIP) $(TARGET).elf -o __SCE__$(TARGET)/$(PSP_EBOOT)
	mkdir -p "%__SCE__$(TARGET)"
	$(PACK_PBP) "%__SCE__$(TARGET)/$(PSP_EBOOT)" $(PSP_EBOOT_SFO) $(PSP_EBOOT_ICON)  \
		$(PSP_EBOOT_ICON1) $(PSP_EBOOT_PIC0) $(PSP_EBOOT_PIC1)  \
		$(PSP_EBOOT_SND0) NULL $(PSP_EBOOT_PSAR)

