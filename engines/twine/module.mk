MODULE := engines/twine

MODULE_OBJS := \
	audio/music.o \
	audio/sound.o \
	\
	debugger/console.o \
	debugger/debug.o \
	debugger/debug_grid.o \
	debugger/debug_scene.o \
	\
	menu/interface.o \
	menu/menu.o \
	menu/menuoptions.o \
	\
	parser/anim.o \
	parser/anim3ds.o \
	parser/blocklibrary.o \
	parser/body.o \
	parser/entity.o \
	parser/holomap.o \
	parser/parser.o \
	parser/sprite.o \
	parser/text.o \
	\
	renderer/redraw.o \
	renderer/renderer.o \
	renderer/screens.o \
	\
	scene/actor.o \
	scene/animations.o \
	scene/buggy.o \
	scene/collision.o \
	scene/dart.o \
	scene/extra.o \
	scene/gamestate.o \
	scene/grid.o \
	scene/movements.o \
	scene/rain.o \
	scene/scene.o \
	scene/wagon.o \
	\
	script/script_life.o \
	script/script_move.o \
	script/script_life_v1.o \
	script/script_life_v2.o \
	script/script_move_v1.o \
	script/script_move_v2.o \
	\
	resources/hqr.o \
	resources/lzss.o \
	resources/resources.o \
	\
	movies.o \
	holomap_v1.o \
	holomap_v2.o \
	input.o \
	metaengine.o \
	shared.o \
	slideshow.o \
	text.o \
	twine.o

# This module can be built as a plugin
ifeq ($(ENABLE_TWINE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
