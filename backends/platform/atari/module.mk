MODULE := backends/platform/atari

MODULE_OBJS := \
	osystem_atari.o \
	atari-debug.o \
	atari_ikbd.o \
	native_features.o \
	dlmalloc.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
