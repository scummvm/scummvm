MODULE := common

MODULE_OBJS := \
	config-file.o \
	config-manager.o \
	file.o \
	md5.o \
	mutex.o \
	str.o \
	stream.o \
	timer.o \
	util.o \
	savefile.o \
	system.o \
	scaler.o \
	scaler/thumbnail.o \
	unzip.o

ifndef DISABLE_SCALERS
MODULE_OBJS += \
	scaler/2xsai.o \
	scaler/aspect.o \
	scaler/scale2x.o \
	scaler/scale3x.o \
	scaler/scalebit.o

ifndef DISABLE_HQ_SCALERS
MODULE_OBJS += \
	scaler/hq2x.o \
	scaler/hq3x.o

ifdef HAVE_NASM
MODULE_OBJS += \
	scaler/hq2x_i386.o \
	scaler/hq3x_i386.o
endif

endif

endif

MODULE_DIRS += \
	common \
	common/scaler

# Include common rules 
include $(srcdir)/common.rules
