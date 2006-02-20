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
	unzip.o

MODULE_DIRS += \
	common

# Include common rules 
include $(srcdir)/common.rules
