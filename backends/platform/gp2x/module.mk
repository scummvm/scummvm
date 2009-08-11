MODULE := backends/platform/gp2x

MODULE_OBJS := \
	gp2x-hw.o \
	gp2x-mem.o \
	events.o \
	graphics.o \
	gp2x.o \

MODULE_DIRS += \
	backends/platform/gp2x/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
