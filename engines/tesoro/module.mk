MODULE := engines/tesoro

MODULE_OBJS = \
	console.o \
	core/animbank.o \
	core/animimage.o \
	core/animset.o \
	core/archive.o \
	core/camera.o \
	core/clock.o \
	core/collision.o \
	core/connectfour.o \
	core/connectfourmatch.o \
	core/display.o \
	core/level.o \
	core/namekey.o \
	core/object.o \
	core/objectlist.o \
	core/renderqueue.o \
	core/room.o \
	core/scenes.o \
	core/scriptblock.o \
	core/scriptrunner.o \
	core/sounddevice.o \
	core/speaker.o \
	core/speechtable.o \
	core/stdiotext.o \
	core/textqueue.o \
	core/walkarea.o \
	kinds/actionstrip.o \
	kinds/boardcolumn.o \
	kinds/cameraframe.o \
	kinds/character.o \
	kinds/clickcatcher.o \
	kinds/cursor.o \
	kinds/exitdoor.o \
	kinds/interactive.o \
	kinds/inventoryitem.o \
	kinds/jim.o \
	kinds/mapplace.o \
	kinds/menubutton.o \
	kinds/menuitem.o \
	kinds/noteobject.o \
	kinds/objectgroup.o \
	kinds/pathobject.o \
	kinds/slotlabel.o \
	kinds/spriteobject.o \
	kinds/storycharacter.o \
	kinds/storyprop.o \
	kinds/walkfloor.o \
	kinds/wanderingsprite.o \
	metaengine.o \
	tesoro.o \
	videoplayer.o \
	world.o

# This module can be built as a plugin
ifeq ($(ENABLE_TESORO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
