srcdir      ?= .
DEPDIR      := .deps

#DYNAMIC_MODULES = 1
libndsdir = $(DEVKITPRO)/libnds
#libndsdir = /home/neil/devkitpror21/libnds

# Select the build by setting SCUMM_BUILD to a,b,c,d,e,f or g.
# Anything else gets build a.

ifeq ($(SCUMM_BUILD),i)
        DS_BUILD_I = 1
else
 ifeq ($(SCUMM_BUILD),h)
         DS_BUILD_H = 1
  else
   ifeq ($(SCUMM_BUILD),g)
         DS_BUILD_G = 1
   else
    ifeq ($(SCUMM_BUILD),f)
         DS_BUILD_F = 1
    else
     ifeq ($(SCUMM_BUILD),e)
         DS_BUILD_E = 1
     else
      ifeq ($(SCUMM_BUILD),d)
         DS_BUILD_D = 1
      else
       ifeq ($(SCUMM_BUILD),c)
         DS_BUILD_C = 1
       else
        ifeq ($(SCUMM_BUILD),b)
         DS_BUILD_B = 1
        else
         DS_BUILD_A = 1
        endif
       endif
      endif
     endif
    endif
   endif
  endif
 endif
# To do:
# - FAT cache?



# Uncomment the following line to build in support for MP3 audio
# using libmad:

ifdef DS_BUILD_F
	# TODO: Fix this.  When libmad is compiled in, the Kyrandia resource loading
	# searches through it's entire index to find an mp3 each time a voice sample is requested
	# this causes a nasty pause.
else
	ifdef DS_BUILD_E
	# TODO: Inherit the earth uses so much RAM that I have removed libmad in order to
	# claw some back.


	else
		USE_MAD = 1
	endif
endif

# Uncomment the following line to enable support for the
# ace DS Debugger (remembering to make the same change in the arm7 makefile):
#USE_DEBUGGER = 1

# Uncomment the following line to enable the profiler
#USE_PROFILER = 1

# NOTE: The header and libs for the debugger is assumed to be in the libnds
# folder.

vpath %.h $(srcdir)
vpath %.cpp $(srcdir)
vpath %.c $(srcdir)
vpath %.m $(srcdir)
vpath %.asm $(srcdir)
vpath %.s $(srcdir)

# Command to build libmad is:
# ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork'
#
# I actually had to use
# ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork' LDFLAGS='C:/Progra~1/devkitpro/libnds/lib/libnds9.a' --disable-shared --disable-debugging

ARM = 1
USE_ARM_SOUND_ASM = 1
USE_ARM_COSTUME_ASM = 1
#WRAP_MALLOC = 1

ifdef DS_BUILD_A
	DEFINES = -DDS_SCUMM_BUILD -DDS_BUILD_A -DUSE_ARM_GFX_ASM -DUSE_ARM_COSTUME_ASM
	LOGO = logoa.bmp
	ENABLE_SCUMM = STATIC_PLUGIN
	DEFINES += -DENABLE_SCUMM=STATIC_PLUGIN
	MODULES += engines/scumm

	USE_ARM_GFX_ASM = 1
	BUILD=scummvm-A
endif

ifdef DS_BUILD_B
	DEFINES = -DDS_NON_SCUMM_BUILD -DDS_BUILD_B
	LOGO = logob.bmp
	ENABLE_SKY = STATIC_PLUGIN
	ENABLE_QUEEN = STATIC_PLUGIN
	BUILD=scummvm-B
endif

ifdef DS_BUILD_C
	DEFINES = -DDS_NON_SCUMM_BUILD -DDS_BUILD_C
	LOGO = logoc.bmp
	ENABLE_AGOS = STATIC_PLUGIN
	BUILD=scummvm-C
endif

ifdef DS_BUILD_D
	DEFINES = -DDS_NON_SCUMM_BUILD -DDS_BUILD_D
	LOGO = logod.bmp
	ENABLE_GOB = STATIC_PLUGIN
	ENABLE_CINE = STATIC_PLUGIN
	ENABLE_AGI = STATIC_PLUGIN
	BUILD=scummvm-D
endif

ifdef DS_BUILD_E
	DEFINES = -DDS_NON_SCUMM_BUILD -DDS_BUILD_E
	LOGO = logoe.bmp
	ENABLE_SAGA = STATIC_PLUGIN
	BUILD=scummvm-E
endif

ifdef DS_BUILD_F
	DEFINES = -DDS_NON_SCUMM_BUILD -DDS_BUILD_F
	LOGO = logof.bmp
	ENABLE_KYRA = STATIC_PLUGIN
	BUILD=scummvm-F
endif

ifdef DS_BUILD_G
	DEFINES = -DDS_NON_SCUMM_BUILD -DDS_BUILD_G
	LOGO = logog.bmp
	ENABLE_LURE = STATIC_PLUGIN
	BUILD=scummvm-G
endif

ifdef DS_BUILD_H
	DEFINES = -DDS_NON_SCUMM_BUILD -DDS_BUILD_H
	LOGO = logog.bmp
	ENABLE_PARALLACTION = STATIC_PLUGIN
	BUILD=scummvm-H
endif

ifdef DS_BUILD_I
	DEFINES = -DDS_NON_SCUMM_BUILD -DDS_BUILD_H
	LOGO = logog.bmp
	ENABLE_TUCKER = STATIC_PLUGIN
	BUILD=scummvm-H
endif

ARM7BIN	:= -7 $(CURDIR)/../../arm7/arm7.bin
ICON		:= -b ../../../logo.bmp "ScummVM;By Neil Millstone;"

CC      = arm-eabi-gcc
CXX     = arm-eabi-g++

CFLAGS	=	-Wno-multichar -Wall\
		-Wno-multichar -mcpu=arm9tdmi -mtune=arm9tdmi \
		-mcpu=arm9tdmi -mtune=arm9tdmi -fomit-frame-pointer\
		-mthumb-interwork -DUSE_ARM_COSTUME_ASM=1


# -ffast-math 

ifdef USE_DEBUGGER
	DEFINES += -DUSE_DEBUGGER 
	CFLAGS += -g
endif

ifdef USE_PROFILER
	CFLAGS += -mpoke-function-name -finstrument-functions -g
	DEFINES += -DUSE_PROFILER
endif

CXXFLAGS=	$(CFLAGS) -Wno-non-virtual-dtor -Wno-unknown-pragmas -Wno-reorder  \
		-fno-exceptions -fno-rtti -mthumb-interwork -ffunction-sections -fdata-sections

#		-mthumb

#		-fno-gcse\
#		-fno-schedule-insns2

#		-fno-inline-functions-called-once\
#		-fno-gcse-lm\
#		-fno-rerun-cse-after-loop\
#		-fno-cse-follow-jumps -fno-cse-skip-blocks\


ASFLAGS = -mcpu=arm9tdmi -mthumb-interwork
DEFINES += -D__DS__ -DNDS -DARM9 -DNONSTANDARD_PORT -DDISABLE_FANCY_THEMES -DVECTOR_RENDERER_FORMAT=1555 -DDISABLE_DOSBOX_OPL -DDISABLE_DEFAULT_SAVEFILEMANAGER -DARM
ifdef USE_MAD
	DEFINES += -DUSE_MAD
endif

DEFINES += -DREDUCE_MEMORY_USAGE

# Removed, as these are done in portdefs.h
# -DDISABLE_TEXT_CONSOLE -DDISABLE_COMMAND_LINE 

LDFLAGS = -specs=ds_arm9.specs -mthumb-interwork -mno-fpu -Wl,-Map,map.txt -Wl,--gc-sections

ifdef WRAP_MALLOC
	LDFLAGS += -Wl,--wrap,malloc
	DEFINES += -DWRAP_MALLOC
endif

INCLUDES= -I$(portdir)/$(BUILD) -I$(srcdir) -I$(srcdir)/engines \
			-I$(portdir)/data -I$(portdir)/../commoninclude \
			-I$(portdir)/source -I$(portdir)/source/mad -I$(portdir)/source/libcartreset \
			-I$(libndsdir)/include -include $(srcdir)/common/scummsys.h


LIBS	= -lm -L$(libndsdir)/lib -L$(portdir)/lib -lnds9
ifdef USE_MAD
	LIBS += -lmad
endif
ifdef USE_DEBUGGER
	LIBS += -ldsdebugger -ldswifi9
endif

#-Lscumm -lscumm -Lbase -lbase -Lcommon -lcommon -Lgraphics -lgraphics -Lgui -lgui -Lsound -lsound
EXECUTABLE = scummvm.elf
PLUGIN_PREFIX =
PLUGIN_SUFFIX = .plg
PLUGIN_EXTRA_DEPS = plugin.x plugin.syms scummvm.elf
PLUGIN_LDFLAGS = -nostartfiles -Wl,-q,-Tplugin.x,--just-symbols,scummvm.elf,--retain-symbols-file,plugin.syms -L$(ronindir)/lib
MKDIR = mkdir -p
RM = rm -f
RM_REC = rm -rf
AR = arm-eabi-ar cru
RANLIB = arm-eabi-ranlib
OBJCOPY = arm-eabi-objcopy
AS = arm-eabi-as
HAVE_GCC3 = true
DISABLE_SCALERS = true

ifdef DYNAMIC_MODULES
DEFINES += -DDYNAMIC_MODULES
PRE_OBJS_FLAGS = -Wl,--whole-archive
POST_OBJS_FLAGS = -Wl,--no-whole-archive
endif

PORT_OBJS :=	$(portdir)/source/blitters_arm.o $(portdir)/source/cdaudio.o $(portdir)/source/dsmain.o \
		$(portdir)/../../../fs/ds/ds-fs.o $(portdir)/source/gbampsave.o $(portdir)/source/scummhelp.o\
		$(portdir)/source/osystem_ds.o $(portdir)/source/portdefs.o $(portdir)/source/ramsave.o\
		$(portdir)/source/touchkeyboard.o $(portdir)/source/zipreader.o\
		$(portdir)/source/dsoptions.o $(portdir)/source/keys.o $(portdir)/source/wordcompletion.o\
		$(portdir)/source/interrupt.o

ifdef USE_PROFILER
	PORT_OBJS += $(portdir)/source/profiler/cyg-profile.o
endif


DATA_OBJS := $(portdir)/data/icons.o $(portdir)/data/keyboard.o $(portdir)/data/keyboard_pal.o $(portdir)/data/default_font.o $(portdir)/data/8x8font_tga.o

COMPRESSOR_OBJS := 
#$(portdir)/source/compressor/lz.o

FAT_OBJS :=  $(portdir)/source/fat/disc_io.o $(portdir)/source/fat/gba_nds_fat.o\
			$(portdir)/source/fat/io_fcsr.o $(portdir)/source/fat/io_m3cf.o\
			$(portdir)/source/fat/io_mpcf.o $(portdir)/source/fat/io_sccf.o\
			$(portdir)/source/fat/io_m3sd.o\
			$(portdir)/source/fat/io_nmmc.o $(portdir)/source/fat/io_scsd.o \
			$(portdir)/source/fat/io_scsd_asm.o \
			$(portdir)/source/fat/io_njsd.o \
			$(portdir)/source/fat/io_mmcf.o \
			$(portdir)/source/fat/io_sd_common.o \
			$(portdir)/source/fat/io_m3_common.o \
			$(portdir)/source/fat/io_dldi.o \
			$(portdir)/source/fat/m3sd.o


#			$(portdir)/source/fat/io_cf_common.o $(portdir)/source/fat/io_m3_common.o\
#			$(portdir)/source/fat/io_sd_common.o $(portdir)/source/fat/io_scsd_s.o \
#			$(portdir)/source/fat/io_sc_common.o $(portdir)/source/fat/io_sd_common.o

LIBCARTRESET_OBJS := 
#$(portdir)/source/libcartreset/cartreset.o

# Files in this list will be optimisied for speed, otherwise they will be optimised for space
OPTLIST := actor.cpp ds_main.cpp osystem_ds.cpp blitters.cpp mame.cpp rate.cpp isomap.cpp image.cpp gfx.cpp sprite.cpp actor_path.cpp actor_walk.cpp
#OPTLIST :=

# Compiler options for files which should be optimised for speed
OPT_SPEED := -O3
#OPT_SPEED := -Os -mthumb

# Compiler options for files which should be optimised for space
OPT_SIZE := -Os -mthumb


#-mthumb -fno-gcse -fno-schedule-insns2


OBJS := $(DATA_OBJS) $(LIBCARTRESET_OBJS) $(PORT_OBJS) $(COMPRESSOR_OBJS) $(FAT_OBJS)



MODULE_DIRS += .

ndsall:
	@[ -d $(BUILD) ] || mkdir -p $(BUILD)
	make -C ./$(BUILD) -f ../makefile scummvm.nds

include $(srcdir)/Makefile.common

semiclean:
	$(RM) $(portdir)/source/dsoptions.o $(portdir)/source/dsmain.o $(FAT_OBJS) $(DATA_OBJS) $(portdir)/source/wordcompletion.o $(portdir)/source/dsoptions.o 

clean:
	$(RM) $(OBJS) $(EXECUTABLE)
	rm -fr $(BUILD)

plugin_dist :
	find . -name '*.plg' | while read p; do \
	  sh-elf-strip -g -o "`basename \"$$p\" | tr '[:lower:]' '[:upper:]'`" "$$p"; \
	done

dist : SCUMMVM.BIN plugins plugin_dist


#---------------------------------------------------------------------------------
# canned command sequence for binary data
#---------------------------------------------------------------------------------
#define bin2o
#	bin2s $< | $(AS) -mthumb -mthumb-interwork -o $(@)
#	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_raw_end[];" > `(echo $(<F) | tr . _)`.h
#	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_raw[];" >> `(echo $(<F) | tr . _)`.h
#	echo "extern const u32" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_raw_size";" >> `(echo $(<F) | tr . _)`.h
#endef


define bin2o
	bin2s $< | $(AS) -mthumb -mthumb-interwork -o $(@)
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(<F) | tr . _)`.h
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(<F) | tr . _)`.h
	echo "extern const u32" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(<F) | tr . _)`.h
endef


#define bin2o
#	@echo $(*)
#	cp $(<) $(*).tmp
#	$(OBJCOPY) -I binary -O elf32-littlearm -B arm \
#	--rename-section .data=.rodata \
#	--redefine-sym _binary_$(subst .,_,$(subst /,_,$(*)))_tmp_start=$(notdir $*)\
#	--redefine-sym _binary_$(subst .,_,$(subst /,_,$(*)))_tmp_end=$(notdir $*)_end\
#	$(*).tmp $(@)
#	echo "extern const u8" $(notdir $*)"[] __attribute__ ((aligned (4)));" > $(*).h
#	echo "extern const u32" $(notdir $(*))_size[]";" >> $(*).h
#
#	echo $(*).h
#	rm $(*).tmp
#endef


##############
# Replacement rule for the one in makefile.common
##############
%.o: %.cpp
#	echo !!!!!!!!!!!! $(notdir $<)
#	ifeq ( $(notdir $<), $(findstring $(notdir $<), $(OPTLIST)) )
#	 OPTFLAG=-O3
#	else
#	 OPTFLAG=-Os
#	endif
#	export OPTFLAG = ;
#	echo !!!!!!!! $(OPTFLAG)
	$(MKDIR) $(*D)/$(DEPDIR)
	$(CXX) -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP $(if $(findstring $(notdir $<), $(OPTLIST)), $(OPT_SPEED), $(OPT_SIZE)) $(CXXFLAGS) $(CPPFLAGS) -c $(<) -o $*.o 


#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
%.o	:	%.pcx
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.raw
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.pal
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.map
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.mdl
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.nds: %.bin
	@echo ndstool -c $@ -9 scummvm.bin $(ARM7BIN) -b ../../$(LOGO) "$(shell basename $@);ScummVM $(VERSION);DS Port"
	ndstool -c $@ -9 scummvm.bin $(ARM7BIN) -b ../../$(LOGO) "$(shell basename $@);ScummVM $(VERSION);DS Port"
	dsbuild $@ -l ../ndsloader.bin

	padbin 16 $(basename $@).ds.gba

#---------------------------------------------------------------------------------
# FIXME: The following rule hardcodes the input & output filename -- shouldn't it use $< and $@ instead?
%.bin: %.elf
	$(OBJCOPY) -S scummvm.elf scummvm-stripped.elf
	$(OBJCOPY) -O binary scummvm-stripped.elf scummvm.bin

#%.o: %.s
#	$(MKDIR) $(*D)/$(DEPDIR)
#	$(CXX) -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP $(CXXFLAGS) $(CPPFLAGS) -c $(<) -o $*.o
