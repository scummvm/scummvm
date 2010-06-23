MODULE := backends/platform/posix

MODULE_OBJS := \
	main.o \
	posix.o

MODULE_DIRS += \
	backends/platform/posix/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
