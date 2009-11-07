MODULE := backends/platform/linuxmoto

MODULE_OBJS := \
	linuxmoto-main.o \
	linuxmoto-events.o \
	linuxmoto-sdl.o \
	linuxmoto-scaler.o \
	linuxmoto-graphics.o \
	hardwarekeys.o \
	../wince/ARMscaler.o

MODULE_DIRS += \
	backends/platform/linuxmoto/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)

# HACK: The linuxmoto backend is based on the SDL one, so we load that, too.
include $(srcdir)/backends/platform/sdl/module.mk
