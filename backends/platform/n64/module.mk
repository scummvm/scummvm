MODULE := backends/platform/n64

MODULE_OBJS := \
	nintendo64.o

MODULE_DIRS += \
	backends/platform/n64/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
