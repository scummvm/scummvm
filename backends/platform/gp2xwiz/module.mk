MODULE := backends/platform/gp2xwiz

MODULE_OBJS := \
	gp2xwiz-scaler.o \
	../wince/ARMscaler.o \
	gp2xwiz-graphics.o \
	gp2xwiz-events.o \
	gp2xwiz-hw.o \
	gp2xwiz-main.o \

MODULE_DIRS += \
	backends/platform/gp2xwiz/ 

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)

# Hack to ensure the SDL backend is built so we can use OSystem_SDL.
-include $(srcdir)/backends/platform/sdl/module.mk