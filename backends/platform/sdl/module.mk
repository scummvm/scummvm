MODULE := backends/platform/sdl

MODULE_OBJS := \
	posix/main.o \
	posix/posix.o \
	win32/main.o \
	win32/win32.o \
	hardwarekeys.o \
	main.o \
	sdl.o

MODULE_DIRS += \
	backends/platform/sdl/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
