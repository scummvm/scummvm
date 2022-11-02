MODULE := engines/mutationofjb

MODULE_OBJS := \
	commands/additemcommand.o \
	commands/bitmapvisibilitycommand.o \
	commands/callmacrocommand.o \
	commands/camefromcommand.o \
	commands/changecommand.o \
	commands/command.o \
	commands/conditionalcommand.o \
	commands/definestructcommand.o \
	commands/endblockcommand.o \
	commands/gotocommand.o \
	commands/ifcommand.o \
	commands/ifitemcommand.o \
	commands/ifpiggycommand.o \
	commands/labelcommand.o \
	commands/loadplayercommand.o \
	commands/newroomcommand.o \
	commands/playanimationcommand.o \
	commands/removeallitemscommand.o \
	commands/removeitemcommand.o \
	commands/renamecommand.o \
	commands/saycommand.o \
	commands/seqcommand.o \
	commands/setcolorcommand.o \
	commands/setobjectframecommand.o \
	commands/specialshowcommand.o \
	commands/switchpartcommand.o \
	commands/talkcommand.o \
	commands/randomcommand.o \
	tasks/conversationtask.o \
	tasks/objectanimationtask.o \
	tasks/saytask.o \
	tasks/sequentialtask.o \
	tasks/taskmanager.o \
	widgets/buttonwidget.o \
	widgets/conversationwidget.o \
	widgets/gamewidget.o \
	widgets/imagewidget.o \
	widgets/inventorywidget.o \
	widgets/labelwidget.o \
	widgets/widget.o \
	animationdecoder.o \
	assets.o \
	conversationlinelist.o \
	debug.o \
	encryptedfile.o \
	font.o \
	game.o \
	gamedata.o \
	gamescreen.o \
	guiscreen.o \
	hardcodedstrings.o \
	inventory.o \
	inventoryitemdefinitionlist.o \
	metaengine.o \
	mutationofjb.o \
	room.o \
	script.o \
	timer.o \
	util.o

# This module can be built as a plugin
ifeq ($(ENABLE_MUTATIONOFJB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
