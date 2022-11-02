MODULE := backends/platform/ds

MODULE_OBJS := \
	background.o \
	blitters_arm.o \
	ds-graphics.o \
	dsmain.o \
	keyboard.o \
	osystem_ds.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
