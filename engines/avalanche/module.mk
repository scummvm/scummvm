MODULE := engines/avalanche

MODULE_OBJS = \
	avalanche.o \
	graphics.o \
	parser.o \
	avalot.o \
	console.o \
	detection.o \
	gyro2.o \
	enhanced2.o \
	logger2.o \
	pingo2.o \
	scrolls2.o \
	visa2.o \
	lucerna2.o \
	enid2.o \
	celer2.o \
	sequence2.o \
	timeout2.o \
	trip6.o \
	acci2.o \
	dropdown2.o \
	closing2.o

# This module can be built as a plugin
ifeq ($(ENABLE_AVALANCHE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
