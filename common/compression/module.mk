MODULE := common/compression

MODULE_OBJS := \
	clickteam.o \
	dcl.o \
	gzio.o \
	installshield_cab.o \
	installshieldv3_archive.o \
	powerpacker.o \
	rnc_deco.o \
	stuffit.o \
	unarj.o \
	unzip.o \
	vise.o \
	zlib.o

# Include common rules
include $(srcdir)/rules.mk
