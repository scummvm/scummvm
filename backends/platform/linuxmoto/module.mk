MODULE := backends/platform/linuxmoto

MODULE_OBJS := \
	linuxmoto-events.o \
	linuxmoto-graphics.o \
	linuxmoto-main.o \
	linuxmoto-sdl.o \
	hardwarekeys.o

MODULE_DIRS += \
	backends/platform/linuxmoto/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)

# HACK: The linuxmoto backend is based on the SDL one, so we load that, too.
include $(srcdir)/backends/platform/sdl/module.mk
