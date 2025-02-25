MODULE := engines/bagel

MODULE_OBJS = \
	bagel.o \
	metaengine.o \
	mfc/bitmap.o \
	mfc/button.o \
	mfc/dc.o \
	mfc/document.o \
	mfc/palette.o \
	mfc/wnd.o \
	spacebar/baglib/area_object.o \
	spacebar/baglib/bagel.o \
	spacebar/baglib/base_pda.o \
	spacebar/baglib/bmp_object.o \
	spacebar/baglib/button_object.o \
	spacebar/baglib/character_object.o \
	spacebar/baglib/chat_wnd.o \
	spacebar/baglib/command_object.o \
	spacebar/baglib/cursor.o \
	spacebar/baglib/dev_dlg.o \
	spacebar/baglib/dossier_object.o \
	spacebar/baglib/event_sdev.o \
	spacebar/baglib/exam.o \
	spacebar/baglib/expression.o \
	spacebar/baglib/expression_object.o \
	spacebar/baglib/fmovie.o \
	spacebar/baglib/help.o \
	spacebar/baglib/ifstream.o \
	spacebar/baglib/inv.o \
	spacebar/baglib/link_object.o \
	spacebar/baglib/log_msg.o \
	spacebar/baglib/master_win.o \
	spacebar/baglib/menu_dlg.o \
	spacebar/baglib/moo.o \
	spacebar/baglib/movie_object.o \
	spacebar/baglib/object.o \
	spacebar/baglib/paint_table.o \
	spacebar/baglib/pan_bitmap.o \
	spacebar/baglib/pan_window.o \
	spacebar/baglib/parse_object.o \
	spacebar/baglib/pda.o \
	spacebar/baglib/rp_object.o \
	spacebar/baglib/save_game_file.o \
	spacebar/baglib/sound_object.o \
	spacebar/baglib/sprite_object.o \
	spacebar/baglib/storage_dev_bmp.o \
	spacebar/baglib/storage_dev_win.o \
	spacebar/baglib/text_object.o \
	spacebar/baglib/time_object.o \
	spacebar/baglib/var.o \
	spacebar/baglib/variable_object.o \
	spacebar/baglib/wield.o \
	spacebar/baglib/zoom_pda.o \
	spacebar/boflib/app.o \
	spacebar/boflib/cache.o \
	spacebar/boflib/crc.o \
	spacebar/boflib/dat_file.o \
	spacebar/boflib/debug.o \
	spacebar/boflib/error.o \
	spacebar/boflib/event_loop.o \
	spacebar/boflib/file.o \
	spacebar/boflib/file_functions.o \
	spacebar/boflib/llist.o \
	spacebar/boflib/log.o \
	spacebar/boflib/misc.o \
	spacebar/boflib/options.o \
	spacebar/boflib/queue.o \
	spacebar/boflib/string_functions.o \
	spacebar/boflib/res.o \
	spacebar/boflib/sound.o \
	spacebar/boflib/string.o \
	spacebar/boflib/timer.o \
	spacebar/boflib/vector.o \
	spacebar/boflib/gfx/bitmap.o \
	spacebar/boflib/gfx/cursor.o \
	spacebar/boflib/gfx/palette.o \
	spacebar/boflib/gfx/sprite.o \
	spacebar/boflib/gfx/text.o \
	spacebar/boflib/gui/button.o \
	spacebar/boflib/gui/dialog.o \
	spacebar/boflib/gui/edit_text.o \
	spacebar/boflib/gui/list_box.o \
	spacebar/boflib/gui/movie.o \
	spacebar/boflib/gui/scroll_bar.o \
	spacebar/boflib/gui/text_box.o \
	spacebar/boflib/gui/window.o \
	spacebar/dialogs/credits_dialog.o \
	spacebar/dialogs/next_cd_dialog.o \
	spacebar/dialogs/opt_window.o \
	spacebar/dialogs/quit_dialog.o \
	spacebar/dialogs/restart_dialog.o \
	spacebar/dialogs/restore_dialog.o \
	spacebar/dialogs/save_dialog.o \
	spacebar/dialogs/start_dialog.o \
	spacebar/spacebar.o \
	spacebar/bib_odds_wnd.o \
	spacebar/bibble_window.o \
	spacebar/computer.o \
	spacebar/console.o \
	spacebar/filter.o \
	spacebar/full_wnd.o \
	spacebar/main_link_object.o \
	spacebar/main_window.o \
	spacebar/master_win.o \
	spacebar/music.o \
	spacebar/nav_window.o \
	spacebar/slot_wnd.o \
	spacebar/sraf_computer.o \
	spacebar/thud.o \
	spacebar/vid_wnd.o \
	hodjnpodj/hodjnpodj.o \
	hodjnpodj/libs/dib_api.o \
	hodjnpodj/libs/dib_doc.o \
	hodjnpodj/boflib/bitmaps.o \
	hodjnpodj/boflib/button.o \
	hodjnpodj/boflib/sound.o \
	hodjnpodj/boflib/sprite.o \
	hodjnpodj/mazedoom/main_window.o \
	hodjnpodj/mazedoom/maze_doom.o \
	hodjnpodj/mazedoom/maze_gen.o

# This module can be built as a plugin
ifeq ($(ENABLE_BAGEL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
