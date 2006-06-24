MODULE := common

MODULE_OBJS := \
	config-file.o \
	config-manager.o \
	file.o \
	fs.o \
	hashmap.o \
	md5.o \
	mutex.o \
	str.o \
	stream.o \
	util.o \
	savefile.o \
	system.o \
	unzip.o

# Include common rules 
include $(srcdir)/rules.mk
