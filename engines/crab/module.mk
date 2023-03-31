MODULE := engines/crab

MODULE_OBJS = \
	crab.o \
	console.o \
	metaengine.o \
	filesystem.o \
	XMLDoc.o \
	loaders.o \
	ScreenSettings.o \
	GameParam.o \
	event/conversationdata.o \
	event/effect.o \
	event/EventSeqGroup.o \
	event/EventSequence.o \
	event/eventstore.o \
	event/gameevent.o \
	event/GameEventInfo.o \
	event/quest.o \
	event/trigger.o \
	event/triggerset.o \
	image/Image.o \
	image/ImageManager.o \
	input/cursor.o \
	item/Item.o \
	music/MusicManager.o \
	people/opinion.o \
	people/person.o \
	people/personbase.o \
	people/trait.o \
	stat/Stat.o \
	stat/StatDrawHelper.o \
	stat/StatTemplate.o \
	text/color.o \
	text/TextManager.o \
	timer.o \
	app.o \
	splash.o \
	Line.o \
	Polygon.o \
	Rectangle.o \
	Shape.o

# This module can be built as a plugin
ifeq ($(ENABLE_CRAB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
