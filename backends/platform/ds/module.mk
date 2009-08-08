MODULE := backends/platform/ds

ARM7_MODULE_OBJS := \
	arm7/source/main.o \
	arm7/source/libcartreset/cartreset.o \

PORT_OBJS := \
	arm9/source/blitters_arm.o \
	arm9/source/cdaudio.o \
	arm9/source/dsmain.o \
	../../fs/ds/ds-fs.o \
	arm9/source/gbampsave.o \
	arm9/source/scummhelp.o \
	arm9/source/osystem_ds.o \
	arm9/source/portdefs.o \
	arm9/source/ramsave.o \
	arm9/source/touchkeyboard.o \
	arm9/source/zipreader.o \
	arm9/source/dsoptions.o \
	arm9/source/keys.o \
	arm9/source/wordcompletion.o \
	arm9/source/interrupt.o

ifdef USE_PROFILER
	PORT_OBJS += arm9/source/profiler/cyg-profile.o
endif

DATA_OBJS := \
	arm9/data/icons.o \
	arm9/data/keyboard.o \
	arm9/data/keyboard_pal.o \
	arm9/data/default_font.o \
	arm9/data/8x8font_tga.o

COMPRESSOR_OBJS := #arm9/source/compressor/lz.o

FAT_OBJS :=  arm9/source/fat/disc_io.o arm9/source/fat/gba_nds_fat.o\
	arm9/source/fat/io_fcsr.o \
	arm9/source/fat/io_m3cf.o \
	arm9/source/fat/io_mpcf.o \
	arm9/source/fat/io_sccf.o \
	arm9/source/fat/io_m3sd.o \
	arm9/source/fat/io_nmmc.o \
	arm9/source/fat/io_scsd.o \
	arm9/source/fat/io_scsd_asm.o \
	arm9/source/fat/io_njsd.o \
	arm9/source/fat/io_mmcf.o \
	arm9/source/fat/io_sd_common.o \
	arm9/source/fat/io_m3_common.o \
	arm9/source/fat/io_dldi.o \
	arm9/source/fat/m3sd.o


#	arm9/source/fat/io_cf_common.o arm9/source/fat/io_m3_common.o\
#	arm9/source/fat/io_sd_common.o arm9/source/fat/io_scsd_s.o \
#	arm9/source/fat/io_sc_common.o arm9/source/fat/io_sd_common.o

LIBCARTRESET_OBJS := 
	#arm9/source/libcartreset/cartreset.o


MODULE_OBJS :=



# TODO: Should add more dirs to MODULE_DIRS so that "make distclean" can remove .deps dirs.
MODULE_DIRS += \
	backends/platform/ds/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
