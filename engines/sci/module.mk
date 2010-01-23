MODULE := engines/sci

MODULE_OBJS := \
	console.o \
	decompressor.o \
	detection.o \
	event.o \
	resource.o \
	sci.o \
	engine/game.o \
	engine/gc.o \
	engine/kernel.o \
	engine/kevent.o \
	engine/kfile.o \
	engine/kgraphics.o \
	engine/klists.o \
	engine/kmath.o \
	engine/kmenu.o \
	engine/kmisc.o \
	engine/kmovement.o \
	engine/kparse.o \
	engine/kpathing.o \
	engine/kscripts.o \
	engine/ksound.o \
	engine/kstring.o \
	engine/message.o \
	engine/said.o \
	engine/savegame.o \
	engine/script.o \
	engine/scriptdebug.o \
	engine/selector.o \
	engine/seg_manager.o \
	engine/segment.o \
	engine/static_selectors.o \
	engine/state.o \
	engine/vm.o \
	graphics/gui.o \
	graphics/animate.o \
	graphics/controls.o \
	graphics/cursor.o \
	graphics/font.o \
	graphics/gfx.o \
	graphics/menu.o \
	graphics/palette.o \
	graphics/picture.o \
	graphics/portrait.o \
	graphics/robot.o \
	graphics/screen.o \
	graphics/text.o \
	graphics/transitions.o \
	graphics/view.o \
	graphics/windowmgr.o \
	parser/grammar.o \
	parser/said.o \
	parser/vocabulary.o \
	sound/audio.o \
	sound/midiparser_sci.o \
	sound/music.o \
	sound/soundcmd.o \
	sound/iterator/core.o \
	sound/iterator/iterator.o \
	sound/iterator/songlib.o \
	sound/drivers/adlib.o \
	sound/drivers/amiga.o \
	sound/drivers/pcjr.o \
	sound/drivers/midi.o \
	video/seq_decoder.o \
	video/vmd_decoder.o
	
ifdef ENABLE_SCI32
MODULE_OBJS += \
	engine/kernel32.o 
endif

# This module can be built as a plugin
ifeq ($(ENABLE_SCI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
