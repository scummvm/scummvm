MODULE := backends/platform/sdl

MODULE_OBJS := \
	events.o \
	graphics.o \
	hardwarekeys.o \
	main.o \
	sdl.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
