MODULE := backends/platform/openpandora

MODULE_OBJS := \
	op-options.o \
	op-backend.o \
	op-main.o

MODULE_DIRS += \
	backends/platform/openpandora/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)

# Hack to ensure the SDL backend is built so we can use OSystem_SDL.
-include $(srcdir)/backends/platform/sdl/module.mk
