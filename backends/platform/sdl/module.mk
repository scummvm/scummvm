MODULE := backends/platform/sdl

MODULE_OBJS := \
	sdl.o \
	sdl-window.o

ifdef KOLIBRIOS
MODULE_OBJS += \
	kolibrios/kolibrios-main.o \
	kolibrios/kolibrios.o
endif

ifdef POSIX
MODULE_OBJS += \
	posix/posix-main.o \
	posix/posix.o
endif

ifdef MACOSX
MODULE_OBJS += \
	macosx/macosx-main.o \
	macosx/macosx.o \
	macosx/macosx-touchbar.o \
	macosx/macosx-window.o \
	macosx/macosx_wrapper.o \
	macosx/macosx_osys_misc.o \
	macosx/appmenu_osx.o
endif

ifdef WIN32
MODULE_OBJS += \
	win32/win32-main.o \
	win32/win32-window.o \
	win32/win32_wrapper.o \
	win32/win32.o
endif

ifdef AMIGAOS
MODULE_OBJS += \
	amigaos/amigaos-main.o \
	amigaos/amigaos.o
endif

ifdef RISCOS
MODULE_OBJS += \
	riscos/riscos-main.o \
	riscos/riscos-utils.o \
	riscos/riscos.o
endif

ifdef MIYOO
MODULE_OBJS += \
	miyoo/miyoo-main.o \
	miyoo/miyoo.o
endif

ifdef MORPHOS
MODULE_OBJS += \
	morphos/morphos-main.o \
	morphos/morphos.o
endif

ifdef OPENDINGUX
MODULE_OBJS += \
	opendingux/opendingux-main.o \
	opendingux/opendingux.o
endif

ifdef PLAYSTATION3
MODULE_OBJS += \
	ps3/ps3-main.o \
	ps3/ps3.o
endif

ifdef PSP2
CC=arm-vita-eabi-gcc
MODULE_OBJS += \
	psp2/psp2-main.o \
	psp2/psp2.o
endif

ifdef SAILFISH
MODULE_OBJS += \
	sailfish/sailfish-main.o \
	sailfish/sailfish-window.o \
	sailfish/sailfish.o
endif

ifdef SWITCH
MODULE_OBJS += \
	switch/switch-main.o \
	switch/switch.o
endif

ifdef EMSCRIPTEN
MODULE_OBJS += \
	emscripten/emscripten-main.o \
	emscripten/emscripten.o
endif

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
