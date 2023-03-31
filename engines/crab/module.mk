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
	animation/animation.o \
	animation/AnimationFrame.o \
	animation/animframe.o \
	animation/imageeffect.o \
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
	input/fightinput.o \
	input/hotkey.o \
	input/input.o \
	input/inputval.o \
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
	TMX/TMXLayer.o \
	TMX/TMXTileSet.o \
	ui/button.o \
	ui/ClipButton.o \
	ui/CreditScreen.o \
	ui/Caption.o \
	ui/element.o \
	ui/HealthIndicator.o \
	ui/HoverInfo.o \
	ui/ImageData.o \
	ui/MapData.o \
	ui/ParagraphData.o \
	ui/ProgressBar.o \
	ui/QuestText.o \
	ui/SectionHeader.o \
	ui/slider.o \
	ui/SlideShow.o \
	ui/StateButton.o \
	ui/textarea.o \
	ui/textdata.o \
	ui/ToggleButton.o \
	ui/TraitButton.o \
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
