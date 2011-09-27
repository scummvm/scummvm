MODULE := engines/pegasus

MODULE_OBJS = \
	compass.o \
	console.o \
	cursor.o \
	detection.o \
	elements.o \
	energymonitor.o \
	fader.o \
	gamestate.o \
	graphics.o \
	hotspot.o \
	input.o \
	interface.o \
	menu.o \
	movie.o \
	notification.o \
	pegasus.o \
	sound.o \
	surface.o \
	timers.o \
	transition.o \
	util.o \
	ai/ai_action.o \
	ai/ai_area.o \
	ai/ai_condition.o \
	ai/ai_rule.o \
	items/inventory.o \
	items/inventorypicture.o \
	items/item.o \
	items/itemlist.o \
	items/biochips/aichip.o \
	items/biochips/biochipitem.o \
	items/biochips/opticalchip.o \
	items/biochips/pegasuschip.o \
	items/biochips/retscanchip.o \
	items/biochips/shieldchip.o \
	items/inventory/airmask.o \
	items/inventory/gascanister.o \
	items/inventory/inventoryitem.o \
	items/inventory/keycard.o \
	neighborhood/door.o \
	neighborhood/exit.o \
	neighborhood/extra.o \
	neighborhood/hotspotinfo.o \
	neighborhood/neighborhood.o \
	neighborhood/prehistoric.o \
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
