MODULE := common/compression

MODULE_OBJS := \
	clickteam.o \
	dcl.o \
	gentee_installer.o \
	gzio.o \
	installshield_cab.o \
	installshieldv3_archive.o \
	packice.o \
	powerpacker.o \
	rnc_deco.o \
	stuffit.o \
	unarj.o \
	unzip.o \
	vise.o

ifdef USE_UNP64
MODULE_OBJS += \
	unp64/unp64.o \
	unp64/6502_emu.o \
	unp64/exo_util.o \
	unp64/scanners/scanners.o \
	unp64/scanners/action_packer.o \
	unp64/scanners/action_replay.o \
	unp64/scanners/byte_boiler.o \
	unp64/scanners/caution.o \
	unp64/scanners/ccs.o \
	unp64/scanners/cruel.o \
	unp64/scanners/eca.o \
	unp64/scanners/exomizer.o \
	unp64/scanners/expert.o \
	unp64/scanners/master_compressor.o \
	unp64/scanners/megabyte.o \
	unp64/scanners/pu_crunch.o \
	unp64/scanners/section8.o \
	unp64/scanners/tbc_multicomp.o \
	unp64/scanners/tcs_crunch.o \
	unp64/scanners/xtc.o
endif

ifdef USE_ZLIB
MODULE_OBJS += \
	zlib.o
endif


# Include common rules
include $(srcdir)/rules.mk
