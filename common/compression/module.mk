MODULE := common/compression

MODULE_OBJS := \
	clickteam.o \
	dcl.o \
	gentee_installer.o \
	gzio.o \
	installshield_cab.o \
	installshieldv3_archive.o \
	powerpacker.o \
	rnc_deco.o \
	stuffit.o \
	unarj.o \
	unzip.o \
	vise.o

ifdef USE_ZLIB
MODULE_OBJS += \
	zlib.o
endif


# Include common rules
include $(srcdir)/rules.mk
