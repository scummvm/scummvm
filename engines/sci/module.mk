MODULE := engines/sci

MODULE_OBJS = \
	console.o \
	detection.o \
	exereader.o \
	sci.o \
	tools.o \
	engine/game.o \
	engine/gc.o \
	engine/grammar.o \
	engine/intmap.o \
	engine/kdebug.o \
	engine/kernel.o \
	engine/kevent.o \
	engine/kfile.o \
	engine/kgraphics.o \
	engine/klists.o \
	engine/kmath.o \
	engine/kmenu.o \
	engine/kmovement.o \
	engine/kpathing.o \
	engine/kscripts.o \
	engine/ksound.o \
	engine/kstring.o \
	engine/message.o \
	engine/said.o \
	engine/savegame.o \
	engine/script.o \
	engine/scriptconsole.o \
	engine/scriptdebug.o \
	engine/seg_manager.o \
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
	gfx/res_view0.o \
	gfx/res_view1.o \
	scicore/decompressor.o \
	scicore/resource.o \
	scicore/sciconsole.o \
	scicore/versions.o \
	scicore/vocabulary.o \
	scicore/vocab_debug.o \
	sfx/adlib_sbi.o \
	sfx/core.o \
	sfx/iterator.o \
	sfx/songlib.o \
	sfx/device/devices.o \
	sfx/player/player.o \
	sfx/player/players.o \
	sfx/player/polled.o \
	sfx/player/realtime.o \
	sfx/seq/gm.o \
	sfx/seq/instrument-map.o \
	sfx/seq/map-mt32-to-gm.o \
	sfx/seq/sequencers.o \
	sfx/softseq/adlib.o \
	sfx/softseq/amiga.o \
	sfx/softseq/pcjr.o \
	sfx/softseq/opl2.o \
	sfx/softseq/pcspeaker.o \
	sfx/softseq/SN76496.o \
	sfx/softseq/softsequencers.o

# This module can be built as a plugin
ifeq ($(ENABLE_SCI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
