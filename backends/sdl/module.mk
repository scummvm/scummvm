MODULE := backends/sdl

MODULE_OBJS := \
	backends/sdl/sdl.o \
	backends/sdl/sdl-common.o

MODULE_DIRS += \
	backend/sdl

# We don't use the common.rules here on purpose
OBJS := $(MODULE_OBJS) $(OBJS)
