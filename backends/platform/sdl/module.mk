MODULE := backends/platform/sdl

MODULE_OBJS := \
	driver_gl.o \
	driver_sdl.o \
	driver_tinygl.o

MODULE_DIRS += \
	backends/platform/sdl/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
