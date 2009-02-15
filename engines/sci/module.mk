MODULE := engines/sci

MODULE_OBJS = \
	detection.o \
	sci.o \
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
	engine/kmovement.o \
	engine/kpathing.o \
	engine/kscripts.o \
	engine/ksound.o \
	engine/kstring.o \
	engine/message.o \
	engine/said.o \
	engine/savegame.o \
	engine/scriptconsole.o \
	engine/scriptdebug.o \
	engine/seg_manager.o \
	engine/sys_strings.o \
	engine/vm.o \
	gfx/antialias.o \
	gfx/font.o \
	gfx/font-5x8.o \
	gfx/font-6x10.o \
	gfx/gfx_driver.o \
	gfx/gfx_res_options.o \
	gfx/gfx_resource.o \
	gfx/gfx_support.o \
	gfx/gfx_tools.o \
	gfx/menubar.o \
	gfx/operations.o \
	gfx/resmgr.o \
	gfx/sbtree.o \
	gfx/sci_widgets.o \
	gfx/widgets.o \
	gfx/resource/sci_cursor_0.o \
	gfx/resource/sci_font.o \
	gfx/resource/sci_pal_1.o \
	gfx/resource/sci_pic_0.o \
	gfx/resource/sci_resmgr.o \
	gfx/resource/sci_view_0.o \
	gfx/resource/sci_view_1.o \
	scicore/aatree.o \
	scicore/console.o \
	scicore/decompress0.o \
	scicore/decompress01.o \
	scicore/decompress1.o \
	scicore/decompress11.o \
	scicore/exe.o \
	scicore/exe_lzexe.o \
	scicore/exe_raw.o \
	scicore/int_hashmap.o \
	scicore/reg_t_hashmap.o \
	scicore/resource.o \
	scicore/resource_map.o \
	scicore/resource_patch.o \
	scicore/sci_memory.o \
	scicore/script.o \
	scicore/tools.o \
	scicore/versions.o \
	scicore/vocab.o \
	scicore/vocab_debug.o \
	sfx/adlib.o \
	sfx/core.o \
	sfx/iterator.o \
	sfx/pcm-iterator.o \
	sfx/songlib.o \
	sfx/time.o \
	sfx/device/devices.o \
	sfx/mixer/mixers.o \
	sfx/mixer/soft.o \
	sfx/pcm_device/pcm_devices.o \
	sfx/pcm_device/scummvm.o \
	sfx/player/players.o \
	sfx/player/polled.o \
	sfx/player/realtime.o \
	sfx/seq/sequencers.o \
	sfx/softseq/amiga.o \
	sfx/softseq/opl2.o \
	sfx/softseq/pcspeaker.o \
	sfx/softseq/SN76496.o \
	sfx/softseq/softsequencers.o \
	sfx/timer/scummvm.o \
	sfx/timer/timers.o

# FIXME: The following is supposed to be a set of *temporary* hacks
CXXFLAGS += -Wno-variadic-macros
CPPFLAGS += -DSCUMMVM

# Generate savegame.cpp
$(srcdir)/engines/sci/engine/savegame.c: $(srcdir)/engines/sci/engine/savegame.cfsml
	cat $< | perl $(srcdir)/engines/sci/engine/cfsml.pl -f savegame.cfsml > $@

# This module can be built as a plugin
ifeq ($(ENABLE_SCI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
