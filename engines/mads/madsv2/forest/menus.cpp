/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/echo.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/mads/quotes.h"
#include "mads/madsv2/forest/menus.h"
#include "mads/madsv2/forest/midi.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

#define SAVE_SUCCESSFUL                 1
#define RESTORE_SUCCESSFUL              2
#define SAVE_FAILED                     3
#define RESTORE_FAILED                  4


int global_menu_direct_jump;
int global_menu_force_restart;

char *menu_quotes = NULL;


static int global_save(int id, const char *save_game_name) {
	int status;

	game_save_name(id + 1);

	if (g_engine->saveGameState(id + 1, save_game_name).getCode() == Common::kNoError)
		status = SAVE_SUCCESSFUL;
	else
		status = SAVE_FAILED;

	return status;
}

static int global_restore(int id) {
	int status;

	if (g_engine->loadGameState(id + 1).getCode() == Common::kNoError)
		status = RESTORE_SUCCESSFUL;
	else
		status = RESTORE_FAILED;

	WRITE_LE_UINT32(&global[walker_timing], 0);

	return status;
}

void global_menu_system_init() {
	menu_quotes = quote_load(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 17, 18, 14, 16, 19, 20,
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0);
}

void global_menu_system_shutdown() {
	if (menu_quotes != NULL)
		mem_free(menu_quotes);
}

static char *menu_quote(int id) {
	return quote_string(menu_quotes, id);
}

void global_emergency_save() {
	game_save_name(0);

	if (scr_orig.data != NULL) mem_free(scr_orig.data);

	g_engine->saveAutosaveIfEnabled();
}

static void global_alert(int status) {
	int quote_id;
	switch (status) {
	case SAVE_SUCCESSFUL:    quote_id = quote_alert_save_ok;      break;
	case RESTORE_SUCCESSFUL: quote_id = quote_alert_restore_ok;   break;
	case SAVE_FAILED:        quote_id = quote_alert_save_fail;    break;
	case RESTORE_FAILED:     quote_id = quote_alert_restore_fail; break;
	default: return;
	}

	popup_dialog_create(game_menu_popup, 0x1000, 20);
	popup_message(menu_quote(quote_id), 0x8000, -1);

	if (status == SAVE_SUCCESSFUL) {
		mads_strlwr(save_game_buf);
		popup_message(save_game_buf, 0x8000, -1);
	}

	popup_button(menu_quote(quote_menu_ok), 0x8000);
	popup_execute();
	popup_dialog_destroy();
}

static void global_menu_save_restore(int save) {
	int status = -1;
	PopupItem *save_list;
	PopupItem *go_button;
	PopupItem *cancel_button;
	PopupItem *result;

	popup_dialog_create(game_menu_popup, 0x1000, 20);

	popup_message(menu_quote(save ? quote_save_title : quote_restore_title), 0x8000, -1);
	popup_blank(4);

	save_list = popup_savelist(game_save_directory, menu_quote(quote_menu_empty),
		99, 64, 63, 200, 10, save, game.last_save);

	if (save) {
		go_button = popup_button(menu_quote(quote_menu_save), 0x400);
		game_menu_popup->clear_item = popup_button(menu_quote(quote_menu_clear), 0x8000);
	} else {
		go_button = popup_button(menu_quote(quote_menu_restore), 0x400);
	}
	cancel_button = popup_cancel_button(menu_quote(quote_menu_cancel));

	result = popup_execute();

	if (result == cancel_button) {
		int key = game_menu_popup->key;
		if (key == 301 || key == 17 || key == 24 || key == 272) {
			game.going = false;
			kernel.activate_menu = GAME_NO_MENU;
		} else if (key == 316) {
			kernel.activate_menu = (section_id != 9 && !save) ? GAME_SAVE_MENU : GAME_MAIN_MENU;
		} else if (key == 317) {
			kernel.activate_menu = save ? GAME_RESTORE_MENU : GAME_MAIN_MENU;
		} else {
			kernel.activate_menu = GAME_MAIN_MENU;
		}
	} else {
		kernel.activate_menu = GAME_NO_MENU;
	}

	popup_dialog_destroy();

	if (result == go_button) {
		int selection = save_list->list->picked_element;
		game.last_save = selection;

		if (save) {
			char *save_game_name = game_save_directory + selection * 64;
			if (!strlen(save_game_name))
				Common::strcpy_s(save_game_name, 64, menu_quote(quote_menu_unnamed));
			status = global_save(selection, save_game_name);
		} else {
			status = global_restore(selection);
		}
	}

	if (status >= 0)
		global_alert(status);
}

static void global_menu_options() {
	PopupItem *music_item;
	PopupItem *fade_item;
	PopupItem *done_button;
	PopupItem *cancel_button;
	PopupItem *result;

	global_unload_config_parameters();
	box_param.menu_text_y_offset = 3;
	box_param.menu_text_x_bonus = 0;

	popup_dialog_create(game_menu_popup, 0x1000, 20);

	popup_blank(1);
	popup_blank(2);

	int initial_music = config_file.music_flag ? 0 : 1;
	int former_screen_fade = config_file.screen_fade;

	music_item = popup_menu(menu_quote(quote_options_music), 0x8000, -1, 160, 10, 2, 40, initial_music);
	popup_menu_option(music_item, menu_quote(quote_options_music_on));
	popup_menu_option(music_item, menu_quote(quote_options_music_off));

	popup_blank(1);

	fade_item = popup_menu(menu_quote(quote_options_fade), 0x8000, -1, 160, 10, 3, 40, former_screen_fade);
	popup_menu_option(fade_item, menu_quote(quote_options_fade_1));
	popup_menu_option(fade_item, menu_quote(quote_options_fade_2));
	popup_menu_option(fade_item, menu_quote(quote_options_fade_3));

	done_button   = popup_button(menu_quote(quote_menu_done), 5);
	cancel_button = popup_cancel_button(menu_quote(quote_menu_cancel));

	popup_width_force(170);

	result = popup_execute();

	kernel.activate_menu = GAME_MAIN_MENU;

	if (result == cancel_button) {
		int key = game_menu_popup->key;
		if (key == 17 || key == 24 || key == 272 || key == 301) {
			game.going = false;
			kernel.activate_menu = GAME_NO_MENU;
		} else if (key == 316) {
			kernel.activate_menu = GAME_SAVE_MENU;
		} else if (key == 317) {
			kernel.activate_menu = GAME_RESTORE_MENU;
		} else if (key == 320) {
			kernel.activate_menu = 8;
		}
	}

	if (result == done_button) {
		config_file.music_flag    = (music_item->list->picked_element == 0) ? 1 : 0;
		config_file.screen_fade   = fade_item->list->picked_element;
		write_config_file();
		global_load_config_parameters();
		kernel.activate_menu = GAME_NO_MENU;
	}

	popup_dialog_destroy();
}

static void global_menu_main() {
	PopupItem *save_item;
	PopupItem *restore_item;
	PopupItem *resume_item;
	PopupItem *exit_item;
	PopupItem *result;
	Buffer scr_live = { video_y, video_x, mcga_video };

	box_param.menu_text_y_offset = 3;
	box_param.menu_text_x_bonus = 0;

	popup_dialog_create(game_menu_popup, 0x1000, 20);

	popup_blank(1);
	popup_blank(2);

	save_item    = popup_menu(menu_quote(quote_main_save),    0x8000, -1, 140, 0, 0, 0, 0);
	popup_blank(1);
	restore_item = popup_menu(menu_quote(quote_main_restore), 0x8000, -1, 140, 0, 0, 0, 0);
	popup_blank(1);
	resume_item  = popup_menu(menu_quote(quote_main_resume),  0x8000, -1, 140, 0, 0, 0, 0);
	popup_blank(1);
	exit_item    = popup_menu(menu_quote(quote_main_exit),    0x8000, -1, 140, 0, 0, 0, 0);

	popup_width_force(160);
	game_menu_popup->cancel_item = resume_item;

	result = popup_execute();

	if (result == save_item) {
		kernel.activate_menu = GAME_SAVE_MENU;
	} else if (result == restore_item) {
		kernel.activate_menu = GAME_RESTORE_MENU;
	} else {
		kernel.activate_menu = GAME_NO_MENU;
	}

	if (result == exit_item) {
		g_engine->saveAutosaveIfEnabled();
		new_room = 904;
	}

	int key = game_menu_popup->key;
	if (key == 301 || key == 17 || key == 24 || key == 272) {
		game.going = false;
		kernel.activate_menu = GAME_NO_MENU;
	} else if (key == 27 || key == 315) {
		if (section_id != 9)
			kernel.activate_menu = GAME_NO_MENU;
	} else if (key == 316) {
		if (section_id != 9)
			kernel.activate_menu = GAME_SAVE_MENU;
	} else if (key == 317) {
		kernel.activate_menu = GAME_RESTORE_MENU;
	}

	popup_dialog_destroy();
	sprite_draw(series_list[6], 2, &scr_inter, 1, 264);
	buffer_rect_copy_2(scr_inter, scr_live, 168, 0, 168, 156, 152, 44);
}

void global_game_menu() {
	bool loaded = false;
	bool music = true;

	if (box_param.series == NULL) {
		Common::strcpy_s(box_param.name, "*BOX");
		if (popup_box_load())
			goto done;
		loaded = true;
	}

	game_menu_setup();
	g_engine->flushKeys();

	do {
		switch (kernel.activate_menu) {
		case GAME_MAIN_MENU:
			global_menu_main();
			break;
		case GAME_SAVE_MENU:
			if (config_file.original_save_load) {
				global_menu_save_restore(true);
			} else {
				kernel.activate_menu = GAME_NO_MENU;
				g_engine->saveGameDialog();
			}
			break;
		case GAME_RESTORE_MENU:
			if (config_file.original_save_load) {
				global_menu_save_restore(false);
			} else {
				kernel.activate_menu = GAME_NO_MENU;
				if (g_engine->loadGameDialog())
					// Dummy name to flag that load was successful
					Common::strcpy_s(save_game_buf, "OK");
			}
			break;
		case GAME_OPTIONS_MENU:
			if (section_id != 9)
				global_menu_options();
			break;
		default:
			kernel.activate_menu = GAME_NO_MENU;
			break;
		}
	} while (!g_engine->shouldQuit() && game.going && kernel.activate_menu != GAME_NO_MENU);

	game_menu_shutdown();

	if (loaded) {
		sprite_free(&box_param.menu, true);
		sprite_free(&box_param.logo, true);
		sprite_free(&box_param.series, true);
	}

done:
	if (!music) {
		config_file.music_flag = true;
		midi_stop();
		config_file.music_flag = false;
		midi_playing = false;
	}
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
