MODULE := common

MODULE_OBJS = \
	common/config-file.o \
	common/engine.o \
	common/file.o \
	common/gameDetector.o \
	common/main.o \
	common/scaler.o \
	common/str.o \
	common/timer.o \
	common/util.o

# Include common rules 
include common.rules
