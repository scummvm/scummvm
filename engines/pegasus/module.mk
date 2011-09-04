MODULE := engines/pegasus

MODULE_OBJS = \
	console.o \
	credits.o \
	detection.o \
	gamestate.o \
	graphics.o \
	menu.o \
	overview.o \
	pegasus.o \
	sound.o \
	video.o \
	items/inventory.o \
	items/item.o \
	items/itemlist.o \
	items/biochips/biochipitem.o \
	items/inventory/inventoryitem.o \
	MMShell/Base_Classes/MMFunctionPtr.o \
	MMShell/Notification/MMNotification.o \
	MMShell/Notification/MMNotificationManager.o \
	MMShell/Notification/MMNotificationReceiver.o \
	MMShell/Utilities/MMResourceFile.o \
	MMShell/Utilities/MMTimeValue.o \
	MMShell/Utilities/MMUtilities.o \
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
