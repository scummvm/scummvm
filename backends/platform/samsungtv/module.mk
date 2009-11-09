MODULE := backends/platform/samsungtv

MODULE_OBJS := \
	events.o \
	graphics.o \
	hardwarekeys.o \
	main.o \
	sdl.o

MODULE_DIRS += \
	backends/platform/samsungtv/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
