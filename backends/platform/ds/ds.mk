# Repeat "all" target here, to make sure it is the first target
# Currently disabled, so only arm7.bin gets build
#all:



# Files in this list will be optimisied for speed, otherwise they will be optimised for space
OPTLIST := actor.cpp ds_main.cpp osystem_ds.cpp blitters.cpp fmopl.cpp rate.cpp isomap.cpp image.cpp gfx.cpp sprite.cpp actor_path.cpp actor_walk.cpp
#OPTLIST :=

# Compiler options for files which should be optimised for speed
OPT_SPEED := -O3

# Compiler options for files which should be optimised for space
OPT_SIZE := -Os -mthumb


#-mthumb -fno-gcse -fno-schedule-insns2


OBJS := $(DATA_OBJS) $(LIBCARTRESET_OBJS) $(PORT_OBJS) $(COMPRESSOR_OBJS) $(FAT_OBJS)


# TODO: Handle files in OPTLIST.
#   For this, the list should be made explicit. So, replace actor.cpp by path/to/actor.cpp --
#   in fact, there are several actor.cpp files, and right now all are "optimized", but
#   I think Neil only had the SAGA one in mind. Same for gfx.cpp






#############################################################################
#############################################################################
#############################################################################


#ndsdir = $(srcdir)/backends/platform/ds
ndsdir = backends/platform/ds

#############################################################################
#
# ARM7 rules.
# For ARM7 files, we need different compiler flags, which leads to the
# extra rules for .o files below
#
#############################################################################

$(ndsdir)/arm7/arm7.bin: $(ndsdir)/arm7/arm7.elf

$(ndsdir)/arm7/arm7.elf: \
	$(ndsdir)/arm7/source/libcartreset/cartreset.o \
	$(ndsdir)/arm7/source/main.o

# HACK/FIXME: C compiler, for cartreset.c -- we should switch this to use CXX
# as soon as possible.
CC := $(DEVKITARM)/bin/arm-eabi-gcc

# HACK/TODO: Pointer to objcopy. This should really be set by configure
OBJCOPY := $(DEVKITARM)/bin/arm-eabi-objcopy

#
# Set various flags
#
ARM7_ARCH	:=	-mthumb-interwork

# note: arm9tdmi isn't the correct CPU arch, but anything newer and LD
# *insists* it has a FPU or VFP, and it won't take no for an answer!
ARM7_CFLAGS	:=	-g -Wall -O2\
		-mcpu=arm7tdmi -mtune=arm7tdmi -fomit-frame-pointer\
		-ffast-math \
		$(ARM7_ARCH) \
		-I$(srcdir)/$(ndsdir)/arm7/source/libcartreset \
		-I$(srcdir)/$(ndsdir)/commoninclude \
		-I$(DEVKITPRO)/libnds/include \
		-I$(DEVKITPRO)/libnds/include/nds \
		-DARM7

ARM7_CXXFLAGS	:= $(ARM7_CFLAGS) -fno-exceptions -fno-rtti

ARM7_LDFLAGS	:= -g $(ARM7_ARCH) -mno-fpu

# HACK/FIXME: Define a custom build rule for cartreset.c.
# We do this because it is a .c file, not a .cpp file and so is outside our
# regular build system anyway. But this is *bad*. It should be changed into a
# .cpp file and this rule be removed.
%.o: %.c
	$(MKDIR) $(*D)/$(DEPDIR)
	$(CC) -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP $(CXXFLAGS) $(CPPFLAGS) -c $(<) -o $*.o


# Set custom build flags for cartreset.o
$(ndsdir)/arm7/source/libcartreset/cartreset.o: CXXFLAGS=$(ARM7_CFLAGS)
$(ndsdir)/arm7/source/libcartreset/cartreset.o: CPPFLAGS=

# Set custom build flags for main.o
$(ndsdir)/arm7/source/main.o: CXXFLAGS=$(ARM7_CXXFLAGS)
$(ndsdir)/arm7/source/main.o: CPPFLAGS=

# Rule for creating ARM7 .bin files from .elf files
%.bin: %.elf
	@echo ------
	@echo Building $@...
	$(OBJCOPY) -O binary  $< $@

# Rule for creating ARM7 .elf files by linking .o files together with a special linker script
%.elf:
	@echo ------
	@echo Building $@...
	$(CXX)  $(ARM7_LDFLAGS) -specs=ds_arm7.specs $+ -L/opt/devkitPro/libnds/lib -lnds7  -o $@
