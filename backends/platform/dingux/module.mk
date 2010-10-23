MODULE := backends/platform/dingux

MODULE_OBJS := \
	main.o \
	dingux.o 

MODULE_DIRS += \
	backends/platform/dingux/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
