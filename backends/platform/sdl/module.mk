MODULE := backends/platform/sdl

MODULE_OBJS := \
	posix/posix-main.o \
	posix/posix.o \
	hardwarekeys.o \
	main.o \
	sdl.o

ifdef MACOSX
MODULE_OBJS += \
	macosx/macosx-main.o \
	macosx/macosx.o
endif

ifdef WIN32
MODULE_OBJS += \
	win32/win32-main.o \
	win32/win32.o
endif

# We don't use the rules.mk here on purpose
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))