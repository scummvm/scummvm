MODULE := backends/sdl

MODULE_OBJS := \
	backends/sdl/events.o \
	backends/sdl/graphics.o \
	backends/sdl/sdl.o

MODULE_DIRS += \
	backends/sdl

# We don't use the common.rules here on purpose
OBJS := $(MODULE_OBJS) $(OBJS)
