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
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/menus.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/phantom/mads/quotes.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

#define SAVE_MENU_PIXEL_WIDTH           200
#define MAX_SAVES_ON_SCREEN             10

#define SPACE_BETWEEN                   -1

#define MAIN_MENU_ITEM_WIDTH            140
#define MAIN_MENU_FORCE_WIDTH           160

#define DIFFICULTY_MENU_FORCE_WIDTH     140

#define OPTIONS_MENU_ITEM_WIDTH         160
#define OPTIONS_MENU_FORCE_WIDTH        170
#define OPTIONS_MENU_OFF_CENTER         10


#define SAVE_SUCCESSFUL                 1
#define RESTORE_SUCCESSFUL              2
#define SAVE_FAILED                     3
#define RESTORE_FAILED                  4


int global_menu_direct_jump;
int global_menu_force_restart;

char *menu_quotes = NULL;


static int global_save(int id) {
	int status;

	game_save_name(id + 1);

	if (g_engine->saveGameState(id, save_game_buf).getCode() == Common::kNoError)
		status = SAVE_SUCCESSFUL;
	else
		status = SAVE_FAILED;

	return status;
}

static int global_restore(int id) {
	int status;

	if (g_engine->loadGameState(id).getCode() == Common::kNoError)
		status = RESTORE_SUCCESSFUL;
	else
		status = RESTORE_FAILED;

	WRITE_LE_UINT32(&global[walker_timing], 0);

	return status;
}

void global_menu_system_init() {
	menu_quotes = quote_load(quote_menu_done, quote_menu_cancel,
		quote_menu_save, quote_menu_restore,
		quote_menu_clear, quote_menu_yes,
		quote_menu_no, quote_menu_ok,
		quote_menu_empty,
		quote_menu_unnamed,
		quote_main_title,
		quote_main_item1, quote_main_item2,
		quote_main_item3, quote_main_item4,
		quote_main_item5, quote_main_item6,
		quote_options_title,
		quote_options_item1,
		quote_options_item2,
		quote_options_item3,
		quote_options_item4,
		quote_options_item5,
		quote_options_item6,
		quote_options_item1a,
		quote_options_item1b,
		quote_options_item2a,
		quote_options_item2b,
		quote_options_item3a,
		quote_options_item3b,
		quote_options_item4a,
		quote_options_item4b,
		quote_options_item4c,
		quote_options_item5a,
		quote_options_item5b,
		quote_options_item5c,
		quote_options_item6a,
		quote_options_item6b,
		quote_save_title, quote_restore_title,
		quote_difficulty_title,
		quote_difficulty_item1, quote_difficulty_item2,
		quote_save_successful,
		quote_restore_successful,
		quote_save_failed,
		quote_restore_failed,
		quote_emergency_save_1a,
		quote_emergency_save_1b,
		quote_emergency_save_success,
		quote_emergency_save_failure,
		quote_emergency_save_attempt,
		quote_emergency_save_resume,
		0);
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

	echo(" ", true);
	echo(menu_quote(quote_emergency_save_1a), false);
	echo(save_game_buf, false);
	echo(menu_quote(quote_emergency_save_1b), false);

	if (scr_orig.data != NULL) mem_free(scr_orig.data);

	g_engine->saveAutosaveIfEnabled();
}

static void global_alert(int status) {
	int id = 0;

	switch (status) {
	case SAVE_SUCCESSFUL:
		id = quote_save_successful;
		break;

	case SAVE_FAILED:
		id = quote_save_failed;
		break;

	case RESTORE_SUCCESSFUL:
		id = quote_restore_successful;
		break;

	case RESTORE_FAILED:
		id = quote_restore_failed;
		break;
	}

	popup_dialog_create(game_menu_popup, GAME_DIALOG_HEAP, 20);

	popup_message(menu_quote(id), POPUP_CENTER, POPUP_FILL);

	if (status == SAVE_SUCCESSFUL) {
		mads_strlwr(&save_game_buf[1]);
		popup_message(save_game_buf, POPUP_CENTER, POPUP_FILL);
	}

	popup_button(menu_quote(quote_menu_ok), POPUP_CENTER);

	popup_execute();
	popup_dialog_destroy();
}

static void global_menu_score() {
	int score;

	score = global[player_score];
	if (score > 250) score = 250;

	text_index[0] = score;
	text_index[1] = 250;

	if (score <= 25) {
		text_index[2] = 1;          /* Stage sweeper */
	} else if (score <= 50) {
		text_index[2] = 2;          /* Dresser */
	} else if (score <= 75) {
		text_index[2] = 3;          /* Usher */
	} else if (score <= 100) {
		text_index[2] = 4;          /* Stagehand */
	} else if (score <= 150) {
		text_index[2] = 5;          /* Chorus Member */
	} else if (score <= 200) {
		text_index[2] = 6;          /* Supporting Player */
	} else if (score <= 249) {
		text_index[2] = 7;          /* Star Player */
	} else if (score <= 250) {
		text_index[2] = 8;          /* Director */
	} else {
		text_index[2] = 9;          /* Bug Finder! */
	}

	text_show(99);

	kernel.activate_menu = GAME_NO_MENU;
}

static void global_menu_save_restore(int save) {
	int status = -1;
	int selection;
	char *save_game_name;
	PopupItem *save_list;
	PopupItem *go_button;
	PopupItem *clear_button;
	PopupItem *cancel_button;
	PopupItem *result;

	popup_dialog_create(game_menu_popup, GAME_DIALOG_HEAP, 20);

	if (save) {
		popup_message(menu_quote(quote_save_title), POPUP_CENTER, POPUP_FILL);
	} else {
		popup_message(menu_quote(quote_restore_title), POPUP_CENTER, POPUP_FILL);
	}
	popup_blank(4);

	save_list = popup_savelist(nullptr, menu_quote(quote_menu_empty),
		GAME_MAX_SAVE_SLOTS,
		GAME_MAX_SAVE_LENGTH + 1,
		GAME_MAX_SAVE_LENGTH,
		SAVE_MENU_PIXEL_WIDTH,
		MAX_SAVES_ON_SCREEN,
		save, game.last_save);

	if (save) {
		go_button = popup_button(menu_quote(quote_menu_save), POPUP_BUTTON_LEFT);
		clear_button = popup_button(menu_quote(quote_menu_clear), POPUP_CENTER);
		game_menu_popup->clear_item = clear_button;
	} else {
		go_button = popup_button(menu_quote(quote_menu_restore), POPUP_BUTTON_LEFT);
	}
	cancel_button = popup_cancel_button(menu_quote(quote_menu_cancel));

	result = popup_execute();

	if (result == cancel_button) {
		switch (game_menu_popup->key) {
		case alt_x_key:
		case ctrl_x_key:
		case alt_q_key:
		case ctrl_q_key:
			game.going = false;
			kernel.activate_menu = GAME_NO_MENU;
			break;

		case f1_key:
			kernel.activate_menu = GAME_MAIN_MENU;
			break;

		case f2_key:
			if (save) {
				kernel.activate_menu = GAME_MAIN_MENU;
			} else {
				kernel.activate_menu = GAME_SAVE_MENU;
			}
			break;

		case f3_key:
			if (!save) {
				kernel.activate_menu = GAME_MAIN_MENU;
			} else {
				kernel.activate_menu = GAME_RESTORE_MENU;
			}
			break;

		case f4_key:
			kernel.activate_menu = GAME_SCORE_MENU;
			break;

		case f5_key:
			kernel.activate_menu = GAME_OPTIONS_MENU;
			break;

		default:
			kernel.activate_menu = GAME_MAIN_MENU;
			break;
		}
	} else {
		kernel.activate_menu = GAME_NO_MENU;
	}

	popup_dialog_destroy();

	if (result == go_button) {
		selection = save_list->list->picked_element;
		game.last_save = selection;
		if (save) {
			save_game_name = game_save_directory + ((GAME_MAX_SAVE_LENGTH + 1) * selection);
			if (!strlen(save_game_name)) {
				Common::strcpy_s(save_game_name, GAME_MAX_SAVE_LENGTH, menu_quote(quote_menu_unnamed));
			}
			status = global_save(selection);

		} else {
			status = global_restore(selection);
		}
	}

	if (status >= 0) {
		global_alert(status);
	}
}

static void global_menu_options() {
	int initial_1, initial_2, initial_3;
	int initial_4, initial_5, initial_6;
	int former_music;
	int former_sound;
	PopupItem *music_item;
	PopupItem *sound_item;
	PopupItem *interface_item;
	PopupItem *fade_item;
	PopupItem *panning_item;
	PopupItem *speech_item;
	PopupItem *done_button;
	PopupItem *cancel_button;
	PopupItem *result;

	global_unload_config_parameters();

	popup_dialog_create(game_menu_popup, GAME_DIALOG_HEAP, 20);

	popup_sprite(box_param.logo, 1, POPUP_CENTER, POPUP_FILL);
	popup_blank(2);

	former_music = config_file.music_flag;
	former_sound = config_file.sound_flag;

	initial_1 = config_file.music_flag ? 0 : 1;
	initial_2 = config_file.sound_flag ? 0 : 1;
	initial_6 = config_file.speech_flag ? 0 : 1;
	initial_3 = config_file.interface_hotspots;
	initial_4 = config_file.screen_fade;
	initial_5 = config_file.panning_speed;

	music_item = popup_menu(menu_quote(quote_options_item1),
		POPUP_CENTER, POPUP_FILL, OPTIONS_MENU_ITEM_WIDTH,
		OPTIONS_MENU_OFF_CENTER,
		2, 40, initial_1);
	popup_menu_option(music_item, menu_quote(quote_options_item1a));
	popup_menu_option(music_item, menu_quote(quote_options_item1b));

	popup_blank(SPACE_BETWEEN);


	sound_item = popup_menu(menu_quote(quote_options_item2),
		POPUP_CENTER, POPUP_FILL, OPTIONS_MENU_ITEM_WIDTH,
		OPTIONS_MENU_OFF_CENTER,
		2, 40, initial_2);
	popup_menu_option(sound_item, menu_quote(quote_options_item2a));
	popup_menu_option(sound_item, menu_quote(quote_options_item2b));

	popup_blank(SPACE_BETWEEN);


	speech_item = popup_menu(menu_quote(quote_options_item6),
		POPUP_CENTER, POPUP_FILL, OPTIONS_MENU_ITEM_WIDTH,
		OPTIONS_MENU_OFF_CENTER,
		2, 40, initial_6);
	popup_menu_option(speech_item, menu_quote(quote_options_item6a));
	popup_menu_option(speech_item, menu_quote(quote_options_item6b));

	popup_blank(SPACE_BETWEEN);


	interface_item = popup_menu(menu_quote(quote_options_item3),
		POPUP_CENTER, POPUP_FILL, OPTIONS_MENU_ITEM_WIDTH,
		OPTIONS_MENU_OFF_CENTER,
		2, 40, initial_3);
	popup_menu_option(interface_item, menu_quote(quote_options_item3a));
	popup_menu_option(interface_item, menu_quote(quote_options_item3b));

	popup_blank(SPACE_BETWEEN);


	fade_item = popup_menu(menu_quote(quote_options_item4),
		POPUP_CENTER, POPUP_FILL, OPTIONS_MENU_ITEM_WIDTH,
		OPTIONS_MENU_OFF_CENTER,
		3, 40, initial_4);
	popup_menu_option(fade_item, menu_quote(quote_options_item4a));
	popup_menu_option(fade_item, menu_quote(quote_options_item4b));
	popup_menu_option(fade_item, menu_quote(quote_options_item4c));

	popup_blank(SPACE_BETWEEN);


	panning_item = popup_menu(menu_quote(quote_options_item5),
		POPUP_CENTER, POPUP_FILL, OPTIONS_MENU_ITEM_WIDTH,
		OPTIONS_MENU_OFF_CENTER,
		3, 40, initial_5);
	popup_menu_option(panning_item, menu_quote(quote_options_item5a));
	popup_menu_option(panning_item, menu_quote(quote_options_item5b));
	popup_menu_option(panning_item, menu_quote(quote_options_item5c));



	done_button = popup_button(menu_quote(quote_menu_done), POPUP_LEFT);
	cancel_button = popup_cancel_button(menu_quote(quote_menu_cancel));

	popup_width_force(OPTIONS_MENU_FORCE_WIDTH);

	result = popup_execute();

	kernel.activate_menu = GAME_MAIN_MENU;

	if (result == cancel_button) {
		switch (game_menu_popup->key) {
		case alt_x_key:
		case ctrl_x_key:
		case alt_q_key:
		case ctrl_q_key:
			game.going = false;
			kernel.activate_menu = GAME_NO_MENU;
			break;

		case f1_key:
			kernel.activate_menu = GAME_MAIN_MENU;
			break;

		case f2_key:
			kernel.activate_menu = GAME_SAVE_MENU;
			break;

		case f3_key:
			kernel.activate_menu = GAME_RESTORE_MENU;
			break;

		case f4_key:
			kernel.activate_menu = GAME_SCORE_MENU;
			break;

		case f5_key:
			kernel.activate_menu = GAME_MAIN_MENU;
			break;

		default:
			kernel.activate_menu = GAME_MAIN_MENU;
			break;
		}
	}

	if (result == done_button) {
		config_file.music_flag = !music_item->list->picked_element;
		config_file.sound_flag = !sound_item->list->picked_element;
		config_file.speech_flag = !speech_item->list->picked_element;
		config_file.interface_hotspots = interface_item->list->picked_element;
		config_file.screen_fade = fade_item->list->picked_element;
		config_file.panning_speed = panning_item->list->picked_element;

		global_write_config_file();
		global_load_config_parameters();

		kernel.activate_menu = GAME_NO_MENU;
	}

	if ((former_music != config_file.music_flag) ||
		(former_sound != config_file.sound_flag)) {
		game_exec_function(section_music_reset_pointer);
	}

	if (!config_file.speech_flag) {
		if (speech_system_active)
			speech_all_off();
	}

	popup_dialog_destroy();
}

static void global_menu_difficulty() {
	PopupItem *easy_item;
	PopupItem *result;

	popup_dialog_create(game_menu_popup, GAME_DIALOG_HEAP, 20);

	popup_message(menu_quote(quote_difficulty_title), POPUP_CENTER, POPUP_FILL);
	popup_blank(6);

	easy_item = popup_menu(menu_quote(quote_difficulty_item1),
		POPUP_CENTER, POPUP_FILL, MAIN_MENU_ITEM_WIDTH, 0,
		0, 0, 0);

	popup_blank(SPACE_BETWEEN);

	(void)popup_menu(menu_quote(quote_difficulty_item2),
		POPUP_CENTER, POPUP_FILL, MAIN_MENU_ITEM_WIDTH, 0,
		0, 0, 0);


	popup_width_force(DIFFICULTY_MENU_FORCE_WIDTH);

	game_menu_popup->cancel_item = NULL;

	result = popup_execute();

	kernel.activate_menu = GAME_NO_MENU;

	if (result == easy_item) {
		game.difficulty = EASY_MODE;
	} else {
		game.difficulty = HARD_MODE;
	}

	if (result == NULL) {
		game.going = false;
	}

	switch (game_menu_popup->key) {
	case alt_x_key:
	case ctrl_x_key:
	case alt_q_key:
	case ctrl_q_key:
	case esc_key:
		game.going = false;
		break;
	}

	popup_dialog_destroy();
}

static void global_menu_main() {
	PopupItem *save_item;
	PopupItem *restore_item;
	PopupItem *score_item;
	PopupItem *options_item;
	PopupItem *resume_item;
	PopupItem *quit_item;
	PopupItem *result;

	popup_dialog_create(game_menu_popup, GAME_DIALOG_HEAP, 20);

	popup_sprite(box_param.logo, 1, POPUP_CENTER, POPUP_FILL);
	popup_blank(6);

	save_item = popup_menu(menu_quote(quote_main_item1),
		POPUP_CENTER, POPUP_FILL, MAIN_MENU_ITEM_WIDTH, 0,
		0, 0, 0);

	popup_blank(SPACE_BETWEEN);

	restore_item = popup_menu(menu_quote(quote_main_item2),
		POPUP_CENTER, POPUP_FILL, MAIN_MENU_ITEM_WIDTH, 0,
		0, 0, 0);

	popup_blank(SPACE_BETWEEN);

	score_item = popup_menu(menu_quote(quote_main_item6),
		POPUP_CENTER, POPUP_FILL, MAIN_MENU_ITEM_WIDTH, 0,
		0, 0, 0);

	popup_blank(SPACE_BETWEEN);

	options_item = popup_menu(menu_quote(quote_main_item3),
		POPUP_CENTER, POPUP_FILL, MAIN_MENU_ITEM_WIDTH, 0,
		0, 0, 0);

	popup_blank(SPACE_BETWEEN);

	resume_item = popup_menu(menu_quote(quote_main_item4),
		POPUP_CENTER, POPUP_FILL, MAIN_MENU_ITEM_WIDTH, 0,
		0, 0, 0);

	popup_blank(SPACE_BETWEEN);

	quit_item = popup_menu(menu_quote(quote_main_item5),
		POPUP_CENTER, POPUP_FILL, MAIN_MENU_ITEM_WIDTH, 0,
		0, 0, 0);

	popup_width_force(MAIN_MENU_FORCE_WIDTH);
	game_menu_popup->cancel_item = resume_item;

	result = popup_execute();

	if (result == save_item) {
		kernel.activate_menu = GAME_SAVE_MENU;
	} else if (result == restore_item) {
		kernel.activate_menu = GAME_RESTORE_MENU;
	} else if (result == options_item) {
		kernel.activate_menu = GAME_OPTIONS_MENU;
	} else if (result == score_item) {
		kernel.activate_menu = GAME_SCORE_MENU;
	} else {
		kernel.activate_menu = GAME_NO_MENU;
	}

	if (result == quit_item)
		game.going = false;

	switch (game_menu_popup->key) {
	case alt_q_key:
	case alt_x_key:
	case ctrl_q_key:
		game.going = false;
		kernel.activate_menu = GAME_NO_MENU;
		break;

	case f5_key:
		kernel.activate_menu = GAME_SAVE_MENU;
		break;

	case f7_key:
		kernel.activate_menu = GAME_RESTORE_MENU;
		break;

	case f8_key:
		kernel.activate_menu = GAME_SCORE_MENU;
		break;

	case f10_key:
		kernel.activate_menu = GAME_OPTIONS_MENU;
		break;

	default:
		break;
	}
}

void global_game_menu() {
	bool loaded = false;

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
			global_menu_save_restore(true);
			break;
		case GAME_RESTORE_MENU:
			global_menu_save_restore(false);
			break;
		case GAME_OPTIONS_MENU:
			global_menu_options();
			break;
		case GAME_DIFFICULTY_MENU:
			global_menu_difficulty();
			break;
		case GAME_SCORE_MENU:
			global_menu_score();
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
	;
}

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
