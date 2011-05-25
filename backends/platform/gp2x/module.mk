MODULE := backends/platform/gp2x

MODULE_OBJS := \
	gp2x-hw.o \
	gp2x-main.o \
	gp2x-mem.o \
	gp2x.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))

# Hack to ensure the SDL backend is built so we can use OSystem_SDL.
-include $(srcdir)/backends/platform/sdl/module.mk
