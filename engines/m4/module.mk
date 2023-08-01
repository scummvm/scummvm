MODULE := engines/m4

MODULE_OBJS = \
	m4.o \
	console.o \
	metaengine.o \
	vars.o \
	adv_db_r/db_catalog.o \
	adv_r/adv_background.o \
	adv_r/adv_been.o \
	adv_r/adv_chk.o \
	adv_r/adv_control.o \
	adv_r/adv_file.o \
	adv_r/adv_game.o \
	adv_r/adv_hotspot.o \
	adv_r/adv_interface.o \
	adv_r/adv_inv.o \
	adv_r/adv_player.o \
	adv_r/adv_rails.o \
	adv_r/adv_scale.o \
	adv_r/adv_trigger.o \
	adv_r/adv_walk.o \
	adv_r/chunk_ops.o \
	adv_r/conv.o \
	adv_r/conv_io.o \
	adv_r/db_env.o \
	adv_r/kernel.o \
	adv_r/other.o \
	core/cstring.o \
	core/errors.o \
	core/globals.o \
	core/imath.o \
	core/mouse.o \
	core/param.o \
	core/rooms.o \
	core/term.o \
	dbg/dbg_wscript.o \
	fileio/extensions.o \
	fileio/fileio.o \
	fileio/fstream.o \
	fileio/info.o \
	fileio/sys_file.o \
	graphics/gr_buff.o \
	graphics/gr_draw.o \
	graphics/gr_color.o \
	graphics/gr_font.o \
	graphics/gr_line.o \
	graphics/gr_pal.o \
	graphics/gr_series.o \
	graphics/gr_sprite.o \
	graphics/gr_surface.o \
	graphics/graphics.o \
	graphics/krn_pal.o \
	graphics/rend.o \
	gui/gui_buffer.o \
	gui/gui_dialog.o \
	gui/gui_item.o \
	gui/gui_mouse.o \
	gui/gui_sys.o \
	gui/gui_vmng_core.o \
	gui/gui_vmng_rectangles.o \
	gui/gui_vmng_screen.o \
	gui/hotkeys.o \
	mem/mem.o \
	mem/memman.o \
	mem/reloc.o \
	mem/res.o \
	platform/draw.o \
	platform/events.o \
	platform/sound.o \
	platform/timer.o \
	platform/sound/digi.o \
	platform/sound/midi.o \
	platform/tile/tile_read.o \
	wscript/ws_cruncher.o \
	wscript/ws_hal.o \
	wscript/ws_load.o \
	wscript/ws_machine.o \
	wscript/ws_timer.o \
	wscript/wscript.o \
	wscript/wst_regs.o \
	burger/core/release_trigger.o \
	burger/core/play_break.o \
	burger/core/stream_break.o \
	burger/gui/game_menu.o \
	burger/gui/gui_cheapo.o \
	burger/gui/interface.o \
	burger/rooms/room.o \
	burger/rooms/section.o \
	burger/rooms/section1/section1.o \
	burger/rooms/section1/room101.o \
	burger/rooms/section9/section9.o \
	burger/rooms/section9/menu_room.o \
	burger/rooms/section9/room901.o \
	burger/rooms/section9/room902.o \
	burger/rooms/section9/room903.o \
	burger/rooms/section9/room904.o \
	burger/rooms/section9/room951.o \
	burger/rooms/section9/room971.o \
	burger/burger.o \
	burger/flags.o \
	burger/hotkeys.o \
	burger/inventory.o \
	burger/other.o \
	burger/series_player.o \
	burger/vars.o \
	burger/walker.o \
	burger/wilbur.o

# This module can be built as a plugin
ifeq ($(ENABLE_M4), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
