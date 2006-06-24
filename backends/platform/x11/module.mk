MODULE := backends/platform/x11

MODULE_OBJS := \
	x11.o

MODULE_DIRS += \
	backends/platform/x11/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
