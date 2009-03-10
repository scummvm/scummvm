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
	engine/stringfrag.o \
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
	gfx/palette.o \
	gfx/resmgr.o \
	gfx/sbtree.o \
	gfx/sci_widgets.o \
	gfx/resource/res_cursor.o \
	gfx/resource/res_font.o \
	gfx/resource/res_manager.o \
	gfx/resource/res_pal.o \
	gfx/resource/res_pic.o \
	gfx/resource/res_view0.o \
	gfx/resource/res_view1.o \
	scicore/decompress0.o \
	scicore/decompress01.o \
	scicore/decompress1.o \
	scicore/decompress11.o \
	scicore/resource.o \
	scicore/sciconsole.o \
	scicore/versions.o \
	scicore/vocabulary.o \
	scicore/vocab_debug.o \
	sfx/adlib.o \
	sfx/core.o \
	sfx/iterator.o \
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

engines/sci/engine/savegame.cpp: engines/sci/engine/savegame.cpp \
  common/stream.h common/scummsys.h config.h common/system.h \
  common/noncopyable.h common/rect.h common/util.h common/str.h \
  common/array.h common/algorithm.h common/debug.h common/list.h \
  common/list_intern.h graphics/pixelformat.h engines/sci/sci_memory.h \
  engines/sci/gfx/operations.h engines/sci/gfx/gfx_resmgr.h \
  engines/sci/gfx/gfx_resource.h engines/sci/gfx/gfx_system.h \
  engines/sci/tools.h common/endian.h engines/sci/gfx/palette.h \
  engines/sci/gfx/gfx_driver.h engines/sci/uinput.h \
  engines/sci/gfx/sbtree.h engines/sci/gfx/gfx_tools.h \
  engines/sci/gfx/gfx_options.h engines/sci/gfx/gfx_res_options.h \
  engines/sci/gfx/menubar.h engines/sci/engine/vm_types.h \
  engines/sci/gfx/gfx_widgets.h engines/sci/gfx/gfx_state_internal.h \
  engines/sci/sfx/core.h engines/sci/sfx/sfx.h engines/sci/sfx/songlib.h \
  sound/timestamp.h engines/sci/scicore/resource.h common/file.h \
  common/archive.h common/hash-str.h common/hashmap.h common/func.h \
  common/memorypool.h common/ptr.h common/singleton.h \
  engines/sci/sfx/iterator.h engines/sci/sfx/sfx_pcm.h \
  engines/sci/engine/state.h engines/sci/scicore/vocabulary.h \
  engines/sci/scicore/versions.h engines/sci/engine/script.h \
  engines/sci/scicore/sciconsole.h engines/sci/engine/seg_manager.h \
  engines/sci/engine/vm.h engines/sci/engine/heapmgr.h \
  engines/sci/engine/intmap.h


# This module can be built as a plugin
ifeq ($(ENABLE_SCI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
