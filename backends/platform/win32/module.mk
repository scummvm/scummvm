MODULE := backends/platform/win32

MODULE_OBJS := \
	main.o \
	win32.o

MODULE_DIRS += \
	backends/platform/win32/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)