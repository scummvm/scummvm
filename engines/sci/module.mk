MODULE := engines/sci

MODULE_OBJS = \
	console.o \
	detection.o \
	exereader.o \
	sci.o \
	sci_memory.o \
	tools.o \
	engine/aatree.o \
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
	engine/state.o \
	engine/vm.o \
	gfx/font.o \
	gfx/gfx_driver.o \
	gfx/gfx_res_options.o \
	gfx/gfx_resource.o \
	gfx/gfx_support.o \
	gfx/gfx_tools.o \
	gfx/gfx_widgets.o \
	gfx/menubar.o \
	gfx/operations.o \
	gfx/resmgr.o \
	gfx/sbtree.o \
	gfx/sci_widgets.o \
	gfx/resource/sci_cursor_0.o \
	gfx/resource/sci_font.o \
	gfx/resource/sci_pal_1.o \
	gfx/resource/sci_pic_0.o \
	gfx/resource/sci_resmgr.o \
	gfx/resource/sci_view_0.o \
	gfx/resource/sci_view_1.o \
	scicore/sciconsole.o \
	scicore/decompress0.o \
	scicore/decompress01.o \
	scicore/decompress1.o \
	scicore/decompress11.o \
	scicore/resource.o \
	scicore/resource_map.o \
	scicore/resource_patch.o \
	scicore/versions.o \
	scicore/vocab.o \
	scicore/vocab_debug.o \
	sfx/adlib.o \
	sfx/core.o \
	sfx/iterator.o \
	sfx/mixer.o \
	sfx/songlib.o \
	sfx/device/devices.o \
	sfx/player/players.o \
	sfx/player/polled.o \
	sfx/player/realtime.o \
	sfx/seq/gm.o \
	sfx/seq/instrument-map.o \
	sfx/seq/map-mt32-to-gm.o \
	sfx/seq/sequencers.o \
	sfx/softseq/amiga.o \
	sfx/softseq/opl2.o \
	sfx/softseq/pcspeaker.o \
	sfx/softseq/SN76496.o \
	sfx/softseq/softsequencers.o

# Generate savegame.cpp
$(srcdir)/engines/sci/engine/savegame.cpp: $(srcdir)/engines/sci/engine/savegame.cfsml $(srcdir)/tools/cfsml.pl
	cat $< | perl $(srcdir)/tools/cfsml.pl -f engines/sci/engine/savegame.cfsml > $@

# This module can be built as a plugin
ifeq ($(ENABLE_SCI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
