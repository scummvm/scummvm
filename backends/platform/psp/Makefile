# SCUMMVM-PSP MakeFile
# $URL$
# $Id$

VERBOSE_BUILD=1
#Use only this section to modify how the makefile behaves ------------

# Scummvm engine config: choose which engines are enabled
ENABLE_SCUMM = $(ENABLED)
ENABLE_SCUMM_7_8 = $(ENABLED)
ENABLE_HE = $(ENABLED)
ENABLE_AGI = $(ENABLED)
ENABLE_AGOS = $(ENABLED)
ENABLE_AGOS2 = $(ENABLED)
ENABLE_CINE = $(ENABLED)
ENABLE_CRUISE = $(ENABLED)
ENABLE_DRACI = $(ENABLED)
ENABLE_DRASCULA = $(ENABLED)
ENABLE_GOB = $(ENABLED)
ENABLE_GROOVIE = $(ENABLED)
ENABLE_GROOVIE2 = $(ENABLED)
ENABLE_IGOR = $(ENABLED)
ENABLE_IHNM = $(ENABLED)
ENABLE_KYRA = $(ENABLED)
ENABLE_LOL = $(ENABLED)
ENABLE_LURE = $(ENABLED)
ENABLE_M4 = $(ENABLED)
ENABLE_MADE = $(ENABLED)
ENABLE_PARALLACTION = $(ENABLED)
ENABLE_QUEEN = $(ENABLED)
ENABLE_SAGA = $(ENABLED)
ENABLE_SAGA2 = $(ENABLED)
ENABLE_SCI = $(ENABLED)
ENABLE_SCI32 = $(ENABLED)
ENABLE_SKY = $(ENABLED)
ENABLE_SWORD1 = $(ENABLED)
ENABLE_SWORD2 = $(ENABLED)
ENABLE_TEENAGENT = $(ENABLED)
ENABLE_TINSEL = $(ENABLED)
ENABLE_TOUCHE = $(ENABLED)
ENABLE_TUCKER = $(ENABLED)


#Set to 1 to enable, 0 to disable dynamic modules
DYNAMIC_MODULES = 1
#Set to 1 to enable, 0 to disable libmad and libogg
USE_LIBMAD = 1
USE_LIBOGG = 1
# --------------------------------------------------------------------


# General variables
srcdir = ../../..
VPATH = $(srcdir)
TARGET = scummvm-psp
# PSP SDK location variables
PSPSDK := $(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin

#check PSPSDK presence
ifeq ($(PSPSDK),)
$(error $$(PSPSDK) cannot be obtained.)
endif


# Variables for common Scummvm makefile
#CC	 = psp-gcc
CXX      = psp-g++
CXXFLAGS = -O3 -Wall -Wno-multichar -fno-exceptions -fno-rtti
DEFINES  = -D__PSP__ -DNONSTANDARD_PORT -DDISABLE_TEXT_CONSOLE -DDISABLE_COMMAND_LINE -DUSE_ZLIB
LDFLAGS  :=
INCDIR   := $(srcdir) . $(srcdir)/engines/ $(PSPSDK)/include
INCLUDES := $(addprefix -I, $(INCDIR))
#EXECUTABLE = $(TARGET)
DEPDIR = .deps
MODULE_DIRS += ./
MKDIR	= mkdir -p
RM	= rm -f
RM_REC	= rm -rf
AR      = psp-ar cru
RANLIB  = psp-ranlib
AS      = psp-gcc
LD      = psp-gcc
HAVE_GCC3 = true
CXX_UPDATE_DEP_FLAG = -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP

# Variables for dynamic plugin building
PLUGIN_PREFIX =
PLUGIN_SUFFIX = .plg
PLUGIN_EXTRA_DEPS = plugin.syms scummvm-psp.elf
# ,--retain-symbols-file,plugin.syms
PLUGIN_LDFLAGS = -nostartfiles -Wl,-q,--just-symbols=scummvm-psp.org.elf,-Tplugin.ld \
 		-lstdc++ -lc -Wl,-Map,mapfile.txt

#PLUGIN_EXTRA_DEPS = plugin.ld plugin.syms scummvm-psp.elf
#PLUGIN_LDFLAGS = -Wl -Tplugin.ld --just-symbols=scummvm-psp.org.elf --retain-symbols-file plugin.syms

# Remove unneeded scalers
DISABLE_SCALERS = true
DISABLE_HQ_SCALERS = true

# PSP-specific variables
STRIP   = psp-strip
MKSFO	= mksfoex -d MEMSIZE=1
PACK_PBP = pack-pbp
FIXUP	= psp-fixup-imports

# Test for adding different libs
ifeq ($(USE_LIBMAD),1)
DEFINES += -DUSE_MAD
LIBS	+= -lmad
endif
ifeq ($(USE_LIBOGG), 1)
DEFINES += -DUSE_VORBIS -DUSE_TREMOR
LIBS	+= -lvorbisidec
endif

# Test for dynamic plugins
ifeq ($(DYNAMIC_MODULES),1)
ENABLED = DYNAMIC_PLUGIN
DEFINES += -DDYNAMIC_MODULES
PRE_OBJS_FLAGS = -Wl,--whole-archive
POST_OBJS_FLAGS = -Wl,--no-whole-archive
else
ENABLED = STATIC_PLUGIN
endif

# SDL Libs and Flags
SDLFLAGS := $(shell sdl-config --cflags)
SDLLIBS  := $(shell sdl-config --libs)
# PSP LIBS
PSPLIBS = -lpspdisplay -lpspgu -lpspctrl -lpspsdk -lpspnet \
 	-lpspnet_inet -lpsputility -lpspuser -lpsppower 

# Add in PSPSDK includes and libraries.
CXXFLAGS += $(SDLFLAGS)
LIBS     += -lz -lstdc++ -lc $(SDLLIBS) $(PSPLIBS) 

OBJS := powerman.o \
	psp_main.o \
	osys_psp.o \
	kbd_ss_c.o \
	kbd_s_c.o \
	kbd_ls_c.o \
	kbd_l_c.o \
	trace.o \
	psploader.o

# Include common Scummvm makefile
include $(srcdir)/Makefile.common

# Variables for PSP-specific rules
PSP_EBOOT_SFO = param.sfo
PSP_EBOOT_TITLE = ScummVM-PSP
PSP_EBOOT = EBOOT.PBP
PSP_EBOOT_ICON = icon0.png
PSP_EBOOT_ICON1 = NULL
PSP_EBOOT_PIC0 = pic0.png
PSP_EBOOT_PIC1 = pic1.png
PSP_EBOOT_SND0 = NULL
PSP_EBOOT_PSAR = NULL

LDFLAGS += -Wl,-Tmain_prog.ld

all: $(PSP_EBOOT)

$(TARGET).elf: $(OBJS)
	$(LD) $(PRE_OBJS_FLAGS) $(OBJS) $(POST_OBJS_FLAGS) $(LDFLAGS) $(LIBS) -o $@
	cp $(TARGET).elf $(TARGET).org.elf
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
		
