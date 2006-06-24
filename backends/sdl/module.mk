MODULE := backends/sdl

MODULE_OBJS := \
	events.o \
	graphics.o \
	sdl.o

MODULE_DIRS += \
	backends/sdl/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
