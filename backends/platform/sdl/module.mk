MODULE := engine/backend/platform/sdl

MODULE_OBJS := \
	driver_gl.o \
	driver_sdl.o \
	driver_tinygl.o

MODULE_DIRS += \
	engine/backend/platform/sdl/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
