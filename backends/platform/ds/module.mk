MODULE := backends/platform/ds

MODULE_OBJS := \
	arm9/source/blitters_arm.o \
	arm9/source/dsmain.o \
	arm9/source/osystem_ds.o \
	arm9/source/dsoptions.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
