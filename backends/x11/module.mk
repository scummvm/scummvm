MODULE := backends/x11

MODULE_OBJS := \
	x11.o

MODULE_DIRS += \
	backends/x11

# We don't use the common.rules here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
