MODULE := backends

MODULE_OBJS := \
	base-backend.o \
	modular-backend.o \
	audiocd/default/default-audiocd.o \
	audiocd/sdl/sdl-audiocd.o \
	events/default/default-events.o \
	events/dinguxsdl/dinguxsdl-events.o \
	events/gp2xsdl/gp2xsdl-events.o \
	events/gph/gph-events.o \
	events/linuxmotosdl/linuxmotosdl-events.o \
	events/openpandora/op-events.o \
	events/samsungtvsdl/samsungtvsdl-events.o \
	events/sdl/sdl-events.o \
	events/symbiansdl/symbiansdl-events.o \
	events/wincesdl/wincesdl-events.o \
	fs/abstract-fs.o \
	fs/stdiostream.o \
	fs/amigaos4/amigaos4-fs-factory.o \
	fs/posix/posix-fs-factory.o \
	fs/symbian/symbian-fs-factory.o \
	fs/windows/windows-fs-factory.o \
	graphics/dinguxsdl/dinguxsdl-graphics.o \
	graphics/gp2xsdl/gp2xsdl-graphics.o \
	graphics/gph/gph-graphics.o \
	graphics/linuxmotosdl/linuxmotosdl-graphics.o \
	graphics/opengl/glerrorcheck.o \
	graphics/opengl/gltexture.o \
	graphics/opengl/opengl-graphics.o \
	graphics/openglsdl/openglsdl-graphics.o \
	graphics/openpandora/op-graphics.o \
	graphics/sdl/sdl-graphics.o \
	graphics/symbiansdl/symbiansdl-graphics.o \
	graphics/wincesdl/wincesdl-graphics.o \
	keymapper/action.o \
	keymapper/keymap.o \
	keymapper/keymapper.o \
	keymapper/remap-dialog.o \
	log/log.o \
	midi/alsa.o \
	midi/camd.o \
	midi/coreaudio.o \
	midi/coremidi.o \
	midi/seq.o \
	midi/stmidi.o \
	midi/timidity.o \
	midi/dmedia.o \
	midi/windows.o \
	mixer/doublebuffersdl/doublebuffersdl-mixer.o \
	mixer/sdl/sdl-mixer.o \
	mixer/symbiansdl/symbiansdl-mixer.o \
	mixer/wincesdl/wincesdl-mixer.o \
	mutex/sdl/sdl-mutex.o \
	plugins/elf/elf-loader.o \
	plugins/elf/mips-loader.o \
	plugins/elf/shorts-segment-manager.o \
	plugins/elf/ppc-loader.o \
	plugins/elf/arm-loader.o \
	plugins/elf/elf-provider.o \
	plugins/elf/memory-manager.o \
	plugins/elf/version.o \
	plugins/posix/posix-provider.o \
	plugins/sdl/sdl-provider.o \
	plugins/win32/win32-provider.o \
	saves/savefile.o \
	saves/default/default-saves.o \
	saves/posix/posix-saves.o \
	timer/default/default-timer.o \
	timer/sdl/sdl-timer.o \
	vkeybd/image-map.o \
	vkeybd/polygon.o \
	vkeybd/virtual-keyboard.o \
	vkeybd/virtual-keyboard-gui.o \
	vkeybd/virtual-keyboard-parser.o

ifeq ($(BACKEND),dc)
MODULE_OBJS += \
	plugins/dc/dc-provider.o
endif

ifeq ($(BACKEND),ds)
MODULE_OBJS += \
	fs/ds/ds-fs-factory.o \
	fs/ds/ds-fs.o \
	plugins/ds/ds-provider.o
endif

ifeq ($(BACKEND),n64)
MODULE_OBJS += \
	fs/n64/n64-fs-factory.o \
	fs/n64/romfsstream.o
endif

ifeq ($(BACKEND),ps2)
MODULE_OBJS += \
	fs/ps2/ps2-fs-factory.o \
	plugins/ps2/ps2-provider.o
endif

ifeq ($(BACKEND),psp)
MODULE_OBJS += \
	fs/psp/psp-fs-factory.o \
	fs/psp/psp-stream.o \
	plugins/psp/psp-provider.o \
	saves/psp/psp-saves.o \
	timer/psp/timer.o
endif

ifeq ($(BACKEND),wii)
MODULE_OBJS += \
	fs/wii/wii-fs-factory.o \
	plugins/wii/wii-provider.o
endif

# Include common rules
include $(srcdir)/rules.mk
