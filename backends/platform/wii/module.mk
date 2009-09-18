MODULE := backends/platform/wii

MODULE_OBJS := \
	main.o \
	options.o \
	osystem.o \
	osystem_gfx.o \
	osystem_sfx.o \
	osystem_events.o

MODULE_DIRS += \
	backends/platform/wii/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
