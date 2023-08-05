MODULE := engines/crab

MODULE_OBJS = \
	crab.o \
	console.o \
	metaengine.o \
	filesystem.o \
	XMLDoc.o \
	loaders.o \
	ScreenSettings.o \
	collision.o \
	game.o \
	GameParam.o \
	ai/moveeffect.o \
	ai/movement.o \
	ai/spriteai.o \
	ai/SpriteConstant.o \
	animation/animation.o \
	animation/AnimationFrame.o \
	animation/animframe.o \
	animation/animset.o \
	animation/fightanim.o \
	animation/fightmove.o \
	animation/fightmoves.o \
	animation/imageeffect.o \
	animation/PopUp.o \
	animation/sprite.o \
	animation/walkframes.o \
	event/conversationdata.o \
	event/effect.o \
	event/EventSeqGroup.o \
	event/EventSequence.o \
	event/eventstore.o \
	event/gameevent.o \
	event/GameEventInfo.o \
	event/gameeventmanager.o \
	event/quest.o \
	event/trigger.o \
	event/triggerset.o \
	image/Image.o \
	image/ImageManager.o \
	input/cursor.o \
	input/fightinput.o \
	input/hotkey.o \
	input/input.o \
	item/Item.o \
	item/ItemCollection.o \
	item/ItemMenu.o \
	item/ItemSlot.o \
	item/StatPreview.o \
	level/level_ai.o \
	level/level_collide.o \
	level/level_draw.o \
	level/level_load.o \
	level/level_objects.o \
	level/level.o \
	level/LevelExit.o \
	level/MusicArea.o \
	level/Stairs.o \
	level/talknotify.o \
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
	TMX/TMXMap.o \
	ui/AlphaImage.o \
	ui/button.o \
	ui/Caption.o \
	ui/ChapterIntro.o \
	ui/ClipButton.o \
	ui/CreditScreen.o \
	ui/DevConsole.o \
	ui/dialogbox.o \
	ui/element.o \
	ui/emotion.o \
	ui/FileData.o \
	ui/FileMenu.o \
	ui/GameOverMenu.o \
	ui/GeneralSettingMenu.o \
	ui/GfxSettingMenu.o \
	ui/HealthIndicator.o \
	ui/HoverInfo.o \
	ui/hud.o \
	ui/ImageData.o \
	ui/Inventory.o \
	ui/journal.o \
	ui/KeyBindMenu.o \
	ui/map.o \
	ui/MapData.o \
	ui/MapMarkerMenu.o \
	ui/ModMenu.o \
	ui/OptionMenu.o \
	ui/OptionSelect.o \
	ui/ParagraphData.o \
	ui/PauseMenu.o \
	ui/PersonHandler.o \
	ui/PersonScreen.o \
	ui/ProgressBar.o \
	ui/questmenu.o \
	ui/QuestText.o \
	ui/ReplyButton.o \
	ui/ReplyMenu.o \
	ui/ResolutionMenu.o \
	ui/SaveGameMenu.o \
	ui/SectionHeader.o \
	ui/slider.o \
	ui/SlideShow.o \
	ui/StateButton.o \
	ui/textarea.o \
	ui/textdata.o \
	ui/ToggleButton.o \
	ui/TraitButton.o \
	ui/TraitMenu.o \
	timer.o \
	app.o \
	splash.o \
	LoadingScreen.o \
	mainmenu.o \
	PathfindingAgent.o \
	PathfindingGraphNode.o \
	PathfindingGrid.o \
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
