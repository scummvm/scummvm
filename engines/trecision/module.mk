MODULE := engines/trecision

MODULE_OBJS = \
	trecision.o \
	metaengine.o \
	graphics.o \
	video.o \
	inventory.o \
	logicnl.o \
	script.o \
	utils.o \
	nl/classes.o \
	nl/dialog.o \
	nl/do.o \
	nl/globvar.o \
	nl/initmain.o \
	nl/process.o \
	nl/regen.o \
	nl/schedule.o \
	nl/string.o \
	nl/utility.o \
	nl/3d/3dact.o \
	nl/3d/3drend.o \
	nl/3d/3dvar.o \
	nl/3d/3dwalk.o \
	nl/ll/llanim.o \
	nl/ll/llasm.o \
	nl/ll/lldec.o \
	nl/ll/llmouse.o \
	nl/ll/llscreen.o \
	nl/ll/llvideo.o \
	nl/sys/anim.o \
	nl/sys/ffile.o \
	nl/sys/main.o \
	nl/sys/sound.o

# This module can be built as a plugin
ifeq ($(ENABLE_TRECISION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
