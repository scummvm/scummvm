MODULE := common

MODULE_OBJS := \
	common/config-manager.o \
	common/file.o \
	common/scaler.o \
	common/str.o \
	common/timer.o \
	common/util.o \
	common/savefile.o \
	common/system.o \
	common/scaler/2xsai.o \
	common/scaler/aspect.o \
	common/scaler/hq2x.o \
	common/scaler/hq3x.o

MODULE_DIRS += \
	common \
	common/scaler

# Include common rules 
include common.rules
