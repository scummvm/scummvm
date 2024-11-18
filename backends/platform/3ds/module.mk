MODULE := backends/platform/3ds

MODULE_OBJS := \
	main.o \
	allocHeapsOverride.o \
	shader.shbin.o \
	sprite.o \
	options.o \
	osystem.o \
	osystem-graphics.o \
	osystem-audio.o \
	osystem-events.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
