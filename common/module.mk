MODULE := common

MODULE_OBJS := \
	common/config-file.o \
	common/engine.o \
	common/file.o \
	common/plugins.o \
	common/scaler.o \
	common/str.o \
	common/timer.o \
	common/util.o \
	common/savefile.o

MODULE_DIRS += \
	common

# Include common rules 
include common.rules
