MODULE := base

MODULE_OBJS := \
	base/gameDetector.o \
	base/main.o \
	base/plugins.o

MODULE_DIRS += \
	base

# Some of the base files depend on the values of the DISABLE_* flags defined
# in config.mak. Hence we add an explicit make dependency on that file.
base/gameDetector.o base/plugins.o: config.mak

# Include common rules 
include common.rules
