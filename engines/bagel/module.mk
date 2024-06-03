MODULE := engines/bagel

MODULE_OBJS = \
	bagel.o \
	console.o \
	metaengine.o \
	music.o \
	baglib/area_object.o \
	baglib/bagel.o \
	baglib/base_pda.o \
	baglib/bmp_object.o \
	baglib/button_object.o \
	baglib/character_object.o \
	baglib/chat_wnd.o \
	baglib/command_object.o \
	baglib/cursor.o \
	baglib/dev_dlg.o \
	baglib/dossier_object.o \
	baglib/event_sdev.o \
	baglib/exam.o \
	baglib/expression.o \
	baglib/expression_object.o \
	baglib/fmovie.o \
	baglib/help.o \
	baglib/ifstream.o \
	baglib/inv.o \
	baglib/link_object.o \
	baglib/log_msg.o \
	baglib/master_win.o \
	baglib/menu_dlg.o \
	baglib/moo.o \
	baglib/movie_object.o \
	baglib/object.o \
	baglib/paint_table.o \
	baglib/pan_bitmap.o \
	baglib/pan_window.o \
	baglib/parse_object.o \
	baglib/pda.o \
	baglib/rp_object.o \
	baglib/save_game_file.o \
	baglib/sound_object.o \
	baglib/sprite_object.o \
	baglib/storage_dev_bmp.o \
	baglib/storage_dev_win.o \
	baglib/text_object.o \
	baglib/time_object.o \
	baglib/var.o \
	baglib/variable_object.o \
	baglib/wield.o \
	baglib/zoom_pda.o \
	boflib/app.o \
	boflib/cache.o \
	boflib/crc.o \
	boflib/dat_file.o \
	boflib/debug.o \
	boflib/error.o \
	boflib/event_loop.o \
	boflib/file.o \
	boflib/file_functions.o \
	boflib/llist.o \
	boflib/log.o \
	boflib/misc.o \
	boflib/options.o \
	boflib/queue.o \
	boflib/string_functions.o \
	boflib/res.o \
	boflib/sound.o \
	boflib/string.o \
	boflib/timer.o \
	boflib/vector.o \
	boflib/gfx/bitmap.o \
	boflib/gfx/cursor.o \
	boflib/gfx/palette.o \
	boflib/gfx/sprite.o \
	boflib/gfx/text.o \
	boflib/gui/button.o \
	boflib/gui/dialog.o \
	boflib/gui/edit_text.o \
	boflib/gui/list_box.o \
	boflib/gui/movie.o \
	boflib/gui/scroll_bar.o \
	boflib/gui/text_box.o \
	boflib/gui/window.o \
	dialogs/credits_dialog.o \
	dialogs/next_cd_dialog.o \
	dialogs/opt_window.o \
	dialogs/quit_dialog.o \
	dialogs/restart_dialog.o \
	dialogs/restore_dialog.o \
	dialogs/save_dialog.o \
	dialogs/start_dialog.o \
	spacebar/spacebar.o \
	spacebar/bib_odds_wnd.o \
	spacebar/bibble_window.o \
	spacebar/computer.o \
	spacebar/filter.o \
	spacebar/full_wnd.o \
	spacebar/main_link_object.o \
	spacebar/main_window.o \
	spacebar/master_win.o \
	spacebar/nav_window.o \
	spacebar/slot_wnd.o \
	spacebar/sraf_computer.o \
	spacebar/thud.o \
	spacebar/vid_wnd.o

# This module can be built as a plugin
ifeq ($(ENABLE_BAGEL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
