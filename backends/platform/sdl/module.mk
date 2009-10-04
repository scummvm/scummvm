MODULE := backends/platform/sdl

MODULE_OBJS := \
	hardwarekeys.o \
	sdl_driver.o

MODULE_DIRS += \
	backends/platform/sdl/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
