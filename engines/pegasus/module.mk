MODULE := engines/pegasus

MODULE_OBJS = \
	console.o \
	credits.o \
	cursor.o \
	detection.o \
	elements.o \
	fader.o \
	gamestate.o \
	graphics.o \
	hotspot.o \
	input.o \
	menu.o \
	notification.o \
	overview.o \
	pegasus.o \
	sound.o \
	timers.o \
	transition.o \
	util.o \
	video.o \
	items/inventory.o \
	items/item.o \
	items/itemlist.o \
	items/biochips/biochipitem.o \
	items/inventory/inventoryitem.o \
	neighborhood/door.o \
	neighborhood/exit.o \
	neighborhood/extra.o \
	neighborhood/hotspotinfo.o \
	neighborhood/neighborhood.o \
	neighborhood/spot.o \
	neighborhood/turn.o \
	neighborhood/view.o \
	neighborhood/zoom.o


# This module can be built as a plugin
ifeq ($(ENABLE_PEGASUS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
