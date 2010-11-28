MODULE := backends/platform/sdl

MODULE_OBJS := \
	hardwarekeys.o \
	main.o \
	sdl.o

ifdef UNIX
MODULE_OBJS += \
	posix/posix-main.o \
	posix/posix.o
endif

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

ifdef AMIGAOS
MODULE_OBJS += \
	amigaos/amigaos-main.o \
	amigaos/amigaos.o
endif

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
