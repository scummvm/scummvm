MODULE := engines/cge
 
MODULE_OBJS := \
	bitmap.o \
	bitmaps.o \
	cfile.o \
	cge.o \
	cge_main.o \
	console.o \
	detection.o \
	game.o \
	gettext.o \
	keybd.o \
	mixer.o \
	mouse.o \
	snail.o \
	sound.o \
	startup.o \
	talk.o \
	text.o \
	vga13h.o \
	vmenu.o \
	vol.o

MODULE_DIRS += \
	engines/cge
 
# This module can be built as a plugin
ifeq ($(ENABLE_CGE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
