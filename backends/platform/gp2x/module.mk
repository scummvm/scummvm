MODULE := backends/platform/gp2x

MODULE_OBJS := \
	gp2x-hw.o \
	gp2x-mem.o \
	events.o \
	graphics.o \
	gp2x.o \

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
