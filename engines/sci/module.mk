MODULE := engines/sci

MODULE_OBJS = \
	console.o \
	decompressor.o \
	detection.o \
	resource.o \
	sci.o \
	tools.o \
	vocabulary.o \
	engine/game.o \
	engine/gc.o \
	engine/grammar.o \
	engine/kernel.o \
	engine/kevent.o \
	engine/kfile.o \
	engine/kgraphics.o \
	engine/klists.o \
	engine/kmath.o \
	engine/kmenu.o \
	engine/kmisc.o \
	engine/kmovement.o \
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
	engine/stringfrag.o \
	engine/state.o \
	engine/vm.o \
	gfx/font.o \
	gfx/gfx_driver.o \
	gfx/gfx_gui.o \
	gfx/gfx_pixmap_scale.o \
	gfx/gfx_res_options.o \
	gfx/gfx_resmgr.o \
	gfx/gfx_resource.o \
	gfx/gfx_support.o \
	gfx/gfx_tools.o \
	gfx/gfx_widgets.o \
	gfx/menubar.o \
	gfx/operations.o \
	gfx/palette.o \
	gfx/res_cursor.o \
	gfx/res_font.o \
	gfx/res_pal.o \
	gfx/res_pic.o \
	gfx/res_view.o \
	gfx/seq_decoder.o \
	gui/gui.o \
	gui/gui_font.o \
	gui/gui_gfx.o \
	gui/gui_palette.o \
	gui/gui_picture.o \
	gui/gui_screen.o \
	gui/gui_view.o \
	gui/gui_windowmgr.o \
	gui32/gui32.o \
	sfx/core.o \
	sfx/iterator.o \
	sfx/songlib.o \
	sfx/seq/gm.o \
	sfx/seq/instrument-map.o \
	sfx/seq/map-mt32-to-gm.o \
	sfx/seq/sequencers.o \
	sfx/softseq/adlib.o \
	sfx/softseq/amiga.o \
	sfx/softseq/pcjr.o
	
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
