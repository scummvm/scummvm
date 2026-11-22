MODULE := engines/tesoro

MODULE_OBJS = \
	tesoro.o \
	console.o \
	world.o \
	metaengine.o \
	videoplayer.o \
	core/animimage.o \
	core/animbank.o \
	core/animset.o \
	core/archive.o \
	core/archiveindex.o \
	kinds/boardcolumn.o \
	core/camera.o \
	kinds/cameraframe.o \
	core/connectfourmatch.o \
	core/connectfour.o \
	kinds/character.o \
	kinds/clickcatcher.o \
	core/clock.o \
	core/collision.o \
	kinds/cursor.o \
	core/display.o \
	kinds/exitdoor.o \
	core/hovertext.o \
	kinds/interactive.o \
	kinds/actionstrip.o \
	kinds/inventoryitem.o \
	kinds/objectgroup.o \
	kinds/jim.o \
	core/level.o \
	kinds/mapplace.o \
	kinds/menuitem.o \
	kinds/menubutton.o \
	kinds/slotlabel.o \
	core/namekey.o \
	kinds/noteobject.o \
	core/object.o \
	core/objectlist.o \
	kinds/pathobject.o \
	core/renderqueue.o \
	core/room.o \
	core/scenes.o \
	core/scriptblock.o \
	core/scriptrunner.o \
	core/sounddevice.o \
	core/speaker.o \
	core/speechtable.o \
	kinds/spriteobject.o \
	core/stdiotext.o \
	kinds/storycharacter.o \
	kinds/storyprop.o \
	core/textqueue.o \
	core/walkarea.o \
	kinds/walkfloor.o \
	kinds/wanderingsprite.o

# This module can be built as a plugin
ifeq ($(ENABLE_TESORO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
