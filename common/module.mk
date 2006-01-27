MODULE := common

MODULE_OBJS := \
	common/config-file.o \
	common/config-manager.o \
	common/file.o \
	common/md5.o \
	common/mutex.o \
	common/str.o \
	common/stream.o \
	common/timer.o \
	common/util.o \
	common/savefile.o \
	common/system.o \
	common/scaler.o \
	common/scaler/thumbnail.o \
	common/unzip.o

ifndef DISABLE_SCALERS
MODULE_OBJS += \
	common/scaler/2xsai.o \
	common/scaler/aspect.o \
	common/scaler/scale2x.o \
	common/scaler/scale3x.o \
	common/scaler/scalebit.o

ifndef DISABLE_HQ_SCALERS
MODULE_OBJS += \
	common/scaler/hq2x.o \
	common/scaler/hq3x.o

ifdef HAVE_NASM
MODULE_OBJS += \
	common/scaler/hq2x_i386.o \
	common/scaler/hq3x_i386.o
endif

endif

endif

MODULE_DIRS += \
	common \
	common/scaler

# Include common rules 
include $(srcdir)/common.rules
