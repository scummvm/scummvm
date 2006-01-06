CC		:= arm-elf-gcc
CXX		:= arm-elf-g++
LD		:= arm-elf-g++
AS		:= arm-elf-as
AR      := arm-elf-ar cru
RANLIB  := arm-elf-ranlib
RM      := rm -f
MKDIR   := mkdir -p
ECHO    := echo -n
CAT     := cat
RM      := rm -f
# recursive version of RM
RM_REC  := $(RM) -r
ZIP     := zip -q
CP      := cp
OBJCOPY	:= arm-elf-objcopy
FXETOOL	:= b2fxec


#######################################################################
# Default compilation parameters. Normally don't edit these           #
#######################################################################

CXXFLAGS:= -g -O
DEFINES :=
LDFLAGS :=
INCLUDES:= -I. -Icommon
LIBS	:=
OBJS	:=

# Turn on useful warnings
CXXFLAGS+= -Wall -pedantic -Wpointer-arith -Wcast-qual -Wcast-align -Wconversion
CXXFLAGS+= -Wshadow -Wuninitialized -Wimplicit -Wundef
CXXFLAGS+= -Wno-long-long -Wno-multichar -Wno-unknown-pragmas -Wno-reorder
CXXFLAGS+= -Wwrite-strings -fcheck-new -Wctor-dtor-privacy -Wnon-virtual-dtor

# GPSDK (SDK, Includes and Startup Files) base dir
GPSDK			=	/usr/compat/gp32/share/sdk

# Outputs
EXEEXT			=	.elf
MAPFILE			=	scummvm.map
BIN			=	scummvm.gxb
FXE			=	scummvm.fxe

# Stripped Build? (Smaller ELF, Minimal debug symbol information).
# You MUST comment this out with a # if you wish to debug your code.
# STRIP_DEBUG		=	-Wl,--strip-debug

# Plugins hack
srcdir			=	./

# GP32 Direct Execute Tool (i.e. pclink -e on Windows, gplink run on Linux)
EXECUTE_TOOL	=	pclink -e
#
#	       		-ffast-math \
#	       		-fshort-double \
#
#
#

				#-Wno-multichar \
				# Remove the below from the above flags for Debug.
CFLAGS		=	-marm -march=armv4t -mtune=arm920 -mapcs \
	       		-finline-functions \
	       		-fshort-enums \
	       		-mstructure-size-boundary=32 \
	       		-mno-thumb-interwork \
				-I$(GPSDK)/include \
				-g \
	       		-O2 \
	       		-fomit-frame-pointer

CPPFLAGS	=	$(CFLAGS)

DEFINES 	= 	-D__GP32__
DEFINES		+=	-DNONSTANDARD_PORT

# Support Vorbis Tremor Patches.
#DEFINES	+=	-DVORBIS_TREMOR

# Support LIBMAD Patches.
#DEFINES	+=	-DGP32_MADMP3

# Support for 8:3 save files names (The GP32 uses FAT12/16 (no vFAT) for the file system).
DEFINES		+=	-DSHORT_SAVENAMES

# Support for the unsigned sound mixer.
DEFINES		+=	-DOUTPUT_UNSIGNED_AUDIO

# Support for the GP32 (fmOPL derived) MIDI engine.
#	-	NOT fully implemented yet.
DEFINES		+=	-DUSE_GP32_FMOPL

LDSPECS		=	-specs=gp32_gpsdk.specs

LDFLAGS 	= 	$(STRIP_DEBUG) \
				-Wl,-Map,$(MAPFILE) \
				$(LDSPECS) -Wl,--no-warn-mismatch

LIBS		+=	-L$(GPSDK)/lib \
				-lgpmem -lgpos -lgpstdio -lgpstdlib -lgpsound -lgpgraphic -lgpfont \
				-lm -lc -lgcc

INCLUDES	+=	-Ibackends/gp32 -I$(GPSDK)/include

MODULES		+=	backends/gp32

#GP32 Debug - Remove from Release builds
#	This builds in the GP32 GDB USB Stub. Don't use it unless you know what your doing.
#	You also need to remove ANY optemisation from the compiler flags.
#DEFINES	+=	-DGP32_GDB
#OBJS	+=	backends/gp32/debug-gdbstub-usb.o

# Standard librarys and optimization modules
OBJS	+=	backends/gp32/startup.o \
			backends/gp32/memcpy.o \
			backends/gp32/gp_asmlib.o \
			backends/gp32/gp_clipped.o \
			backends/gp32/fontdata.o

# Custom GP32 std library
OBJS	+=	backends/gp32/gp32std.o \
			backends/gp32/gp32std_grap.o \
			backends/gp32/gp32std_input.o \
			backends/gp32/gp32std_sound.o \
			#backends/gp32/dmaaudio_asm.o \
			#backends/gp32/dmaaudio.o \

#Main ScummVM for the GP32 Backend
OBJS	+= 	backends/gp32/gp32_main.o \
			backends/gp32/gp32_launcher.o \
			backends/gp32/gp32_osys.o \
			backends/gp32/gp-fs.o

#install: all
					#$(EXECUTE_TOOL) $(BIN)

$(FXE): $(BIN)
					$(FXETOOL) -f -a "The ScummVM Team" -t "ScummVM for the GP32" $< $@
#					$(FXETOOL) -f -a "The ScummVM Team" -t "ScummVM for the GP32" -b scummvm.bmp $< $@

$(BIN): scummvm$(EXEEXT)
					$(OBJCOPY) -O binary $< $@

#######################################################################
# Compile options - you can modify these to tweak ScummVM compilation #
#######################################################################

# Uncomment this to activate the MAD lib for compressed sound files
# DEFINES += -DUSE_MAD
# LIBS    += -lmad

# Uncomment this to activate the Ogg Vorbis lib for compressed sound files
# You MUST use Tremor for Ogg Vorbis support on the GP32 port.
# DEFINES += -DUSE_VORBIS
# LIBS    += -lvorbisfile -lvorbis

# Uncomment this to activate the ALSA lib for midi
# DEFINES += -DUSE_ALSA
# LIBS    += -lasound

# Uncomment this to cause warnings to be treated as errors
# CXXFLAGS+= -Werror

# Uncomment this to use GCC 3.x specific dependency tracking (recommended)
HAVE_GCC3 = 1

#######################################################################
# Control which modules are built - uncomment any to disable module   #
#######################################################################

DISABLE_SCALERS = 1
DISABLE_HQ_SCALERS = 1

#DISABLE_SCUMM = 1
#DISABLE_SCUMM_7_8 = 1
DISABLE_HE = 1
DISABLE_SIMON = 1
DISABLE_SKY = 1
DISABLE_QUEEN = 1


# The engines below are not supported on the GP32 port so there is
# no point compiling support into the binary.
DISABLE_SWORD1 = 1
DISABLE_SWORD2 = 1

# In-development engines below.
# Disable for ALL release builds.
DISABLE_SAGA = 1
DISABLE_KYRA = 1
DISABLE_GOB = 1

#######################################################################
# Misc stuff - you should normally never have to edit this            #
#######################################################################

EXECUTABLE := scummvm$(EXEEXT)

include Makefile.common

dist:
	$(RM) $(ZIPFILE)
	$(ZIP) $(ZIPFILE) $(DISTFILES)

install: $(FXE)
		$(EXECUTE_TOOL) $(BIN)	
