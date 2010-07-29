MODULE := backends/platform/linuxmoto

MODULE_OBJS := \
	linuxmoto-events.o \
	linuxmoto-graphics.o \
	linuxmoto-main.o \
	linuxmoto-sdl.o \
	hardwarekeys.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))

# HACK: The linuxmoto backend is based on the SDL one, so we load that, too.
include $(srcdir)/backends/platform/sdl/module.mk
