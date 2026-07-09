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

#include "mads/madsv2/core/attr.h"
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/hspot.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/magic.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/core/video.h"
#include "mads/madsv2/nebular/mads/quotes.h"
#include "mads/madsv2/nebular/menus.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

#define MENU_MESSAGE_COLOR_BASE       10    /* 1st color to use for messages          */
#define MENU_MESSAGE_COLOR_BASE_2     12    /* 1st color to use for second message    */
#define MENU_MESSAGE_COLOR_BASE_3     14    /* 1st color to use for third  message    */

#define MENU_MESSAGE_COLOR    (((MENU_MESSAGE_COLOR_BASE   + 1) << 8) + MENU_MESSAGE_COLOR_BASE)
#define MENU_MESSAGE_COLOR_2  (((MENU_MESSAGE_COLOR_BASE_2 + 1) << 8) + MENU_MESSAGE_COLOR_BASE_2)
#define MENU_MESSAGE_COLOR_3  (((MENU_MESSAGE_COLOR_BASE_3 + 1) << 8) + MENU_MESSAGE_COLOR_BASE_3)

enum {
	SAVE_SUCCESSFUL     = 1,
	RESTORE_SUCCESSFUL  = 2,
	SAVE_FAILED         = 3,
	RESTORE_FAILED      = 4
};

struct MenuMessage {
	int active;         /* Item is present/not present   */
	int status;         /* Item is selected/not selected */
	int x, y;           /* Item position                 */
	int handle;         /* Matte message handle          */
	char text[80];      /* Text of message string        */
	FontPtr font;       /* Font to display message       */
	int spacing;        /* Font auto spacing             */
};

#define MAX_MENU_MESSAGE 20

static MenuMessage *menu = NULL;
static byte *save_menu = NULL;
static byte *trash_bag = NULL;
static int game_menu_num_items;
static int game_menu_scan_items;
static int trash_num_items;

static int game_menu_changed;
static int game_menu_current_item;
static int game_menu_selected_item;
static int game_menu_return_key;

static int game_menu_series_id;

static int game_menu_save_top = 0;
static int game_menu_save_select = 0;
static int game_menu_save_stroke = 0;

static int game_menu_direct_jump;

static int game_save_menu_virgin;

static int game_been_to_save_screen;
static int game_save_status = RESTORE_FAILED;

static int  game_menu_save_dirty;
static char *game_menu_save_buffer;
static char *game_menu_save_pointer;
static int menu_room_id;
static int menu_series_handle;



void global_emergency_save() {
	game_save_name(0);

	if (scr_orig.data != NULL)
		mem_free(scr_orig.data);

	g_engine->saveAutosaveIfEnabled();
}

static void choose_menu_background() {
	switch (section_id) {
	case 1:
	case 2:
		menu_room_id = 921;
		break;
	case 3:
	case 4:
		menu_room_id = 922;
		break;
	case 5:
	case 6:
	case 7:
		menu_room_id = 923;
		break;
	case 8:
		menu_room_id = 924;
		break;
	default:
		menu_room_id = 920;
		break;
	}
}

static void game_menu_setup() {
	Palette specialPal;

	MADS::MADSV2::game_menu_setup();

	menu = (MenuMessage *)malloc(MAX_MENU_MESSAGE * sizeof(MenuMessage));

	choose_menu_background();

	kernel.quotes = quote_load(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
		21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 0);

	
	cursor_id = MAX(CURSOR_WAIT, cursor->num_sprites);
	if (cursor_id != cursor_last) {
		mouse_cursor_sprite(cursor, cursor_id);
		cursor_last = cursor_id;
	}

	vocab_clear_active();
	kernel_init_dynamic();

	text_default_x = text_default_y = -1;

	int newRoom = new_room;
	int roomId = room_id;
	int previousRoom = previous_room;

	inter_input_mode = INTER_BUILDING_SENTENCES;

	bool palFlag = kernel.activate_menu == GAME_DIFFICULTY_MENU;
	if (!palFlag)
		player_preserve_palette();

	game.going = (byte)!kernel_room_startup(menu_room_id, 0, kernel.interface, palFlag);

	previous_room = previousRoom;
	room_id = roomId;
	new_room = newRoom;

	viewing_at_y = 22;
	sound_queue_hold();

	Series *menuSprites = sprite_series_load("*MENU", 0);
	if (!menuSprites) {
		error_report(-7, 3, 23, 0, 0);
		return;
	}

	menu_series_handle = matte_allocate_series(menuSprites, 0);

	Common::fill(magic_color_flags, magic_color_flags + 3, 0);
	Common::fill(magic_color_values, magic_color_values + 3, 0);

	if (config_file.screen_fade) {
		memset(&specialPal[0], 0, Graphics::PALETTE_SIZE);
		mcga_setpal(&specialPal);
	} else {
		mcga_getpal(&specialPal);
		magic_fade_to_grey(specialPal, NULL, 0, 256, 0, 1, 1, 16);
	}

	mouse_hide();
	buffer_fill(scr_main, 0);
	video_update(&scr_main, 0, 0, 0, 0, 320, 200);

	buffer_rect_fill(scr_work, 0, 0, 320, 1, 2);
	video_update(&scr_work, 0, 0, 0, viewing_at_y - 2, 320, 1);
	video_update(&scr_work, 0, 0, 0, viewing_at_y + 157, 320, 1);

	kernel.fx = config_file.screen_fade ? MATTE_FX_FAST_THRU_BLACK : MATTE_FX_FADE_FROM_BLACK;
	kernel.trigger = 0;

	game_emergency_save = global_emergency_save;

	cursor_id = (cursor->num_sprites > 1) ? CURSOR_NORMAL : cursor->num_sprites;
	if (cursor_id != cursor_last) {
		mouse_cursor_sprite(cursor, cursor_id);
		cursor_last = cursor_id;
	}

	pal_change_color(10, 0, 63, 0);
	pal_change_color(11, 0, 45, 0);
	pal_change_color(12, 63, 63, 0);
	pal_change_color(13, 45, 45, 0);
	pal_change_color(14, 63, 63, 63);
	pal_change_color(15, 45, 45, 45);

	mouse_show();
}

static void game_menu_shutdown() {
	MADS::MADSV2::game_menu_shutdown();

	viewing_at_y = 0;

	matte_deallocate_series(menu_series_handle, true);
	kernel_room_shutdown();

	mem_free(kernel.quotes);
	kernel.quotes = nullptr;

	free(menu);
	menu = nullptr;

	previous_room = room_id = KERNEL_RESTORING_GAME;

	if (!game.going)
		room_id = new_room;
}

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

static char *game_menu_save_string(int id) {
	char *string;

	string = (char *)save_menu + (id * (GAME_MAX_SAVE_LENGTH + 1));

	return string;
}

static void game_menu_write_message(FontPtr font, char *string,
	int special, int x, int y) {
	int this_x;
	int width;
	int keep_me;
	char *look;
	byte *trash_pointer;
	MenuMessage *message;

	if (game_menu_scan_items < game_menu_num_items) {
		message = &menu[game_menu_scan_items];
		keep_me = (strcmp(message->text, string) == 0);
		if (!keep_me) {
			if (message->handle >= 0) {
				if (message_list[message->handle].active) {
					message_list[message->handle].status = -1;
					if (trash_num_items < GAME_MENU_MAX_ITEMS) {
						trash_pointer = trash_bag + (80 * trash_num_items);
						Common::strcpy_s((char *)trash_pointer, 80, message_list[message->handle].text);
						message_list[message->handle].text = (char *)trash_pointer;
						trash_num_items++;
					}
				}
			}
		} else {
			goto done;
		}
	} else {
		if (game_menu_num_items < GAME_MENU_MAX_ITEMS) {
			message = &menu[game_menu_num_items++];
		} else {
			goto done;
		}
	}

	Common::strcpy_s(message->text, string);
	message->font = font;
	message->spacing = -1;
	message->active = true;
	message->status = 0;
	message->y = y;
	message->handle = -1;

	this_x = x;
	if (special == GAME_MENU_CENTER) {
		width = font_string_width(font, message->text, message->spacing);
		this_x += (video_x >> 1) - (width >> 1);
	} else if (special == GAME_MENU_SPECIAL) {
		look = strchr(message->text, '@');
		if (look != NULL) {
			*look = 0;
			width = font_string_width(font, message->text, message->spacing);
			Common::strcpy_s(look, 80, look + 1);
			this_x += (video_x >> 1) - width;
		}
	} else if (special == GAME_MENU_RIGHT) {
		width = font_string_width(font, message->text, message->spacing);
		this_x -= width;
	}
	message->x = this_x;

done:
	game_menu_scan_items++;
}


static void game_menu_make_message(FontPtr font, int quote_1, int quote_2,
	int special, int x, int y) {
	char work_buf[80];

	Common::strcpy_s(work_buf, quote_string(kernel.quotes, quote_1));
	if (quote_2 > 0) {
		Common::strcat_s(work_buf, " ");
		Common::strcat_s(work_buf, quote_string(kernel.quotes, quote_2));
	}
	game_menu_write_message(font, work_buf, special, x, y);
}

static void game_save_restore_message(int id, int select, int entries_x, int entries_y, int *menu_scroll) {
	int width;
	int ok;
	char temp_buf[80];

	if ((id + game_menu_save_top) < GAME_MAX_SAVE_SLOTS) {
		Common::strcpy_s(temp_buf, game_menu_save_string(game_menu_save_top + id));

		if (id + game_menu_save_top == select) {
			Common::strcat_s(temp_buf, "_");
		} else {
			if (!strlen(temp_buf))
				Common::strcat_s(temp_buf, quote_string(kernel.quotes, quote_menu_empty));
		}

	} else {
		temp_buf[0] = 0;
	}

	do {
		width = font_string_width(font_conv, temp_buf, -1);
		ok = !strlen(temp_buf) || (width <= GAME_MAX_SAVE_WIDTH);
		if (!ok) {
			temp_buf[strlen(temp_buf) - 1] = 0;
		}
	} while (!ok);

	game_menu_write_message(font_conv, temp_buf, 0, entries_x, *menu_scroll += entries_y);
}

static void game_save_restore_number(int id, int entries_x, int entries_y, int *menu_scroll) {
	char temp_buf[80];

	if ((id + game_menu_save_top) < GAME_MAX_SAVE_SLOTS) {
		mads_itoa(game_menu_save_top + id + 1, temp_buf, 10);
	} else {
		temp_buf[0] = 0;
	}

	game_menu_write_message(font_conv, temp_buf, GAME_MENU_RIGHT, entries_x, *menu_scroll += entries_y);
}

static void game_menu_first_init() {
	int count;

	game_menu_num_items = 0;
	game_menu_save_stroke = 0;

	for (count = 0; count < GAME_MENU_MAX_ITEMS; count++) {
		menu[count].active = false;
	}

	for (count = 0; count < MESSAGE_LIST_SIZE; count++) {
		message_list[count].active = false;
	}

	image_marker = 1;
	image_list[0].flags = IMAGE_REFRESH;
	image_list[0].segment_id = (byte)-1;
}

static void game_menu_general_init() {
	game_menu_current_item = -1;
	game_menu_selected_item = -1;
	game_menu_scan_items = 0;

	game_menu_return_key = false;

	trash_num_items = 0;
}

static void game_menu_main_init() {
	int menu_base;
	int height;
	int unit, title;
	int items;
	int count;

	game_menu_general_init();

	items = 6;
	unit = (font_conv->max_y_size + 1);
	title = 6;
	height = (unit * items) + title;
	menu_base = (display_y >> 1) - (height >> 1);

	game_menu_make_message(font_conv, quote_main_title, 0, GAME_MENU_CENTER, 0, menu_base);

	menu_base += title;
	for (count = 0; count < 5; count++) {
		menu_base += unit;
		game_menu_make_message(font_conv, quote_main_item1 + count, 0, GAME_MENU_CENTER, 0, menu_base);
	}
}

static void game_menu_difficulty_init() {
	int menu_base;
	int height;
	int unit, title;
	int items;
	int count;

	game_menu_general_init();

	items = 4;
	unit = (font_conv->max_y_size + 1);
	title = 6;
	height = (unit * items) + title;
	menu_base = (display_y >> 1) - (height >> 1);

	game_menu_make_message(font_conv, quote_difficulty_title, 0, GAME_MENU_CENTER, 0, menu_base);

	menu_base += title;
	for (count = 0; count < 3; count++) {
		menu_base += unit;
		game_menu_make_message(font_conv, quote_difficulty_item1 + count, 0, GAME_MENU_CENTER, 0, menu_base);
	}
}

static void game_menu_alert_init() {
	int menu_base;
	int height;
	int unit, title;
	int items;
	int quote;

	game_menu_general_init();

	items = 3;
	unit = (font_conv->max_y_size + 1);
	title = 6;
	height = (unit * items) + title;
	menu_base = (display_y >> 1) - (height >> 1);

	switch (game_save_status) {
	case SAVE_SUCCESSFUL:
		quote = quote_save_successful;
		break;

	case RESTORE_SUCCESSFUL:
		quote = quote_restore_successful;
		break;

	case SAVE_FAILED:
		quote = quote_save_failed;
		break;

	case RESTORE_FAILED:
	default:
		quote = quote_restore_failed;
		break;
	}

	game_menu_make_message(font_conv, quote, 0, GAME_MENU_CENTER, 0, menu_base);
	menu_base += unit;

	mads_strlwr(&save_game_buf[1]);
	game_menu_write_message(font_conv, save_game_buf, GAME_MENU_CENTER, 0, menu_base);

	menu_base += title;
	menu_base += unit;
	game_menu_make_message(font_conv, quote_menu_ok, 0, GAME_MENU_CENTER, 0, menu_base);
}

static void game_menu_options_init() {
	int menu_base;
	int height;
	int unit, title;
	int items;
	int quote;

	game_menu_general_init();

	items = 9;
	unit = (font_conv->max_y_size + 1);
	title = 6;
	height = (unit * items) + (title * 2);
	menu_base = (display_y >> 1) - (height >> 1);

	game_menu_make_message(font_conv, quote_options_title, 0, GAME_MENU_CENTER, 0, menu_base);

	quote = config_file.music_flag ? quote_options_item1a : quote_options_item1b;
	game_menu_make_message(font_conv, quote_options_item1, quote, GAME_MENU_SPECIAL, 0, menu_base += (title + unit));

	quote = config_file.sound_flag ? quote_options_item2a : quote_options_item2b;
	game_menu_make_message(font_conv, quote_options_item2, quote, GAME_MENU_SPECIAL, 0, menu_base += unit);

	quote = quote_options_item3a + config_file.interface_hotspots;
	game_menu_make_message(font_conv, quote_options_item3, quote, GAME_MENU_SPECIAL, 0, menu_base += unit);

	quote = quote_options_item4a + config_file.inventory_mode;
	game_menu_make_message(font_conv, quote_options_item4, quote, GAME_MENU_SPECIAL, 0, menu_base += unit);

	quote = quote_options_item5a + config_file.animated_interface;
	game_menu_make_message(font_conv, quote_options_item5, quote, GAME_MENU_SPECIAL, 0, menu_base += unit);

	quote = quote_options_item6a + config_file.screen_fade;
	game_menu_make_message(font_conv, quote_options_item6, quote, GAME_MENU_SPECIAL, 0, menu_base += unit);

	quote = (config_file.naughtiness == STORYMODE_NAUGHTY) ? quote_options_item7a : quote_options_item7b;
	game_menu_make_message(font_conv, quote_options_item7, quote, GAME_MENU_SPECIAL, 0, menu_base += unit);

	game_menu_make_message(font_conv, quote_menu_done, 0, GAME_MENU_CENTER, -54, menu_base += (unit + title));
	game_menu_make_message(font_conv, quote_menu_cancel, 0, GAME_MENU_CENTER, 54, menu_base);
}

static void game_menu_save_init() {
	int menu_base;
	int menu_scroll;
	int height;
	int unit, title;
	int items;
	int count;
	int numbers_x, entries_x;
	int save_x, cancel_x, clear_x;

	game_menu_general_init();

	items = 9;
	unit = (font_conv->max_y_size + 1);
	title = 6;
	height = (unit * items) + (title * 2);
	menu_base = (display_y >> 1) - (height >> 1);

	numbers_x = 24;       /* Main table number locations */
	entries_x = 32;       /* Main table entry locations  */

	save_x = -103;     /* Button locations */
	clear_x = 0;
	cancel_x = 103;

	game_menu_make_message(font_conv, quote_save_title, 0, GAME_MENU_CENTER, 0, menu_base);

	menu_base += title;

	menu_scroll = menu_base;
	for (count = 0; count < GAME_MAX_SAVES_ON_SCREEN; count++) {
		game_save_restore_message(count, game_menu_save_select, entries_x, unit, &menu_scroll);
	}

	menu_scroll = menu_base;
	for (count = 0; count < GAME_MAX_SAVES_ON_SCREEN; count++) {
		game_save_restore_number(count, numbers_x, unit, &menu_scroll);
	}

	menu_base = menu_scroll + title + unit;

	game_menu_make_message(font_conv, quote_menu_save, 0, GAME_MENU_CENTER, save_x, menu_base);
	game_menu_make_message(font_conv, quote_menu_clear, 0, GAME_MENU_CENTER, clear_x, menu_base);
	game_menu_make_message(font_conv, quote_menu_cancel, 0, GAME_MENU_CENTER, cancel_x, menu_base);
}

static void game_menu_restore_init() {
	int menu_base;
	int menu_scroll;
	int height;
	int unit, title;
	int items;
	int count;
	int numbers_x, entries_x;
	int save_x, cancel_x, clear_x;

	game_menu_general_init();

	items = 9;
	unit = (font_conv->max_y_size + 1);
	title = 6;
	height = (unit * items) + (title * 2);
	menu_base = (display_y >> 1) - (height >> 1);

	numbers_x = 24;       /* Main table number locations */
	entries_x = 32;       /* Main table entry locations  */

	save_x = -103;     /* Button locations */
	clear_x = 0;
	cancel_x = 103;

	game_menu_make_message(font_conv, quote_restore_title, 0, GAME_MENU_CENTER, 0, menu_base);

	menu_base += title;

	menu_scroll = menu_base;
	for (count = 0; count < GAME_MAX_SAVES_ON_SCREEN; count++) {
		game_save_restore_message(count, -1, entries_x, unit, &menu_scroll);
	}

	menu_scroll = menu_base;
	for (count = 0; count < GAME_MAX_SAVES_ON_SCREEN; count++) {
		game_save_restore_number(count, numbers_x, unit, &menu_scroll);
	}

	menu_base = menu_scroll + title + unit;

	game_menu_make_message(font_conv, quote_menu_restore, 0, GAME_MENU_CENTER, save_x, menu_base);
	game_menu_make_message(font_conv, quote_menu_clear, 0, GAME_MENU_CENTER, clear_x, menu_base);
	game_menu_make_message(font_conv, quote_menu_cancel, 0, GAME_MENU_CENTER, cancel_x, menu_base);
}

static void game_menu_hotspot_init() {
	int count;
	int x1, y1, x2, y2;
	int width, height;

	numspots = 0;

	for (count = 0; count < GAME_MENU_MAX_ITEMS; count++) {
		if (menu[count].active) {
			x1 = menu[count].x;
			y1 = menu[count].y;
			width = font_string_width(menu[count].font, menu[count].text, menu[count].spacing);
			height = menu[count].font->max_y_size;
			x2 = x1 + width - 1;
			y2 = y1 + height - 1;
			hspot_add(x1, y1, x2, y2, 1, count, mcga_mode);
		}
	}

	if ((kernel.activate_menu == GAME_SAVE_MENU) ||
		(kernel.activate_menu == GAME_RESTORE_MENU)) {
		/* Arrow hot spots                             */
		/*         x1   y1  x2   y2 clas num  mode     */
		hspot_add(293, 26, 312, 75, 2, 50, mcga_mode);
		hspot_add(293, 78, 312, 127, 2, 51, mcga_mode);
	}
}

static void game_menu_process_hotspots() {
	int myspot;
	int count;
	int old_select;
	int stroke_check;

	old_select = game_menu_current_item;

	for (count = 0; count < game_menu_num_items; count++) {
		menu[count].status = 0;
	}

	myspot = hspot_which(mouse_x, mouse_y - viewing_at_y, mcga_mode);

	if (game_menu_save_stroke) {
		if ((mouse_y - viewing_at_y) < spot[2].ul_y) {  /* Hack: drag scroll up   */
			if (!mouse_stop_stroke) menu[1].status = 1;
			myspot = 19; /* Up arrow */
		}
		if ((mouse_y - viewing_at_y) > spot[8].lr_y) {  /* Hack: drag scroll down */
			if (!mouse_stop_stroke) menu[7].status = 1;
			myspot = 20; /* Down arrow */
		}
	}

	if ((myspot > 0) && mouse_any_stroke) {
		game_menu_current_item = spot[myspot].num;

		if ((kernel.activate_menu == GAME_SAVE_MENU) || (kernel.activate_menu == GAME_RESTORE_MENU)) {
			if ((game_menu_current_item > GAME_MAX_SAVES_ON_SCREEN) && (game_menu_current_item <= (GAME_MAX_SAVES_ON_SCREEN << 1))) {
				menu[game_menu_current_item].status = 1;
				game_menu_current_item -= GAME_MAX_SAVES_ON_SCREEN;
			}

			stroke_check = (game_menu_current_item > 0) && (game_menu_current_item <= GAME_MAX_SAVES_ON_SCREEN);
			if (mouse_start_stroke) {
				game_menu_save_stroke = stroke_check;
			}
		}

		if (spot[myspot]._class == 1) {
			menu[game_menu_current_item].status = 1;
		}
	} else {
		game_menu_current_item = -1;
	}

	if (game_menu_current_item == 0) game_menu_current_item = -1;
	if (kernel.activate_menu == GAME_ALERT_MENU) {
		if (game_menu_current_item == 1) game_menu_current_item = -1;
	}

	if (mouse_stop_stroke) {
		if (!game_menu_save_stroke || (game_menu_current_item <= 18)) {
			game_menu_selected_item = game_menu_current_item;
		}
		game_menu_changed = true;
	}

	if ((old_select != game_menu_current_item) || (game_menu_selected_item >= 0)) {
		game_menu_changed = true;
	}
}

static void game_menu_generate_messages() {
	int count;
	int my_color;
	int keep_me;

	for (count = 0; count < GAME_MENU_MAX_ITEMS; count++) {
		if (menu[count].active) {

			switch (menu[count].status) {
			case 0:
				my_color = MENU_MESSAGE_COLOR;
				break;

			case 1:
				my_color = MENU_MESSAGE_COLOR_2;
				break;

			case 2:
			default:
				my_color = MENU_MESSAGE_COLOR_3;
				break;
			}

			keep_me = false;

			if (menu[count].handle >= 0) {
				if (message_list[menu[count].handle].main_color == my_color) {
					keep_me = true;
				} else {
					matte_clear_message(menu[count].handle);
					menu[count].handle = -1;
				}
			}

			if (!keep_me) {
				menu[count].handle = matte_add_message(menu[count].font,
					menu[count].text,
					menu[count].x,
					menu[count].y,
					my_color,
					menu[count].spacing);
			}
		}
	}
}

static int game_menu_standard_keyboard(int mykey, int going) {
	switch (mykey) {
	case esc_key:
		going = false;
		game_menu_selected_item = 0;
		break;

	case alt_x_key:
	case alt_q_key:
	case ctrl_x_key:
	case ctrl_q_key:
		game_menu_selected_item = 0;
		g_engine->quitGame();
		going = false;
		game.going = false;
		break;

	case enter_key:
		game_menu_selected_item = 0;
		game_menu_return_key = true;
		going = false;
		break;
	}

	return going;
}

static bool game_menu_keyboard(int going) {
	int mykey;

	if (keys_any()) {
		mykey = keys_get();
		going = game_menu_standard_keyboard(mykey, going);
	}

	return going;
}

static int game_menu_sprite(int sprite, int depth) {
	int id;

	id = matte_allocate_image();
	image_list[id].flags = IMAGE_UPDATE;
	image_list[id].segment_id = (byte)1;
	image_list[id].series_id = (byte)game_menu_series_id;
	image_list[id].sprite_id = sprite;
	image_list[id].x = series_list[game_menu_series_id]->index[sprite - 1].x;
	image_list[id].y = series_list[game_menu_series_id]->index[sprite - 1].y;
	image_list[id].depth = (byte)depth;
	image_list[id].scale = 100;

	return id;
}

static void game_menu_main() {
	int going = true;
	int first_time = true;
	int frame_id;
	long menu_clock = 0;
	long now_clock;

	game_menu_first_init();

	frame_id = game_menu_sprite(1, 2);

	while (kernel.activate_menu == GAME_MAIN_MENU) {

		going = true;

		game_menu_main_init();
		game_menu_hotspot_init();

		mouse_init_cycle();

		game_menu_changed = true;

		while (going && (game_menu_selected_item < 1)) {

			going = game_menu_keyboard(going) && !g_engine->shouldQuit();

			now_clock = timer_read();

			if (now_clock != menu_clock) {
				menu_clock = now_clock;

				mouse_begin_cycle(false);

				game_menu_process_hotspots();

				if (game_menu_changed) {

					if (game_menu_current_item == 0) game_menu_current_item = -1;

					game_menu_generate_messages();

					/* Matte out the next graphics frame */

					matte_frame(kernel.fx, false);

					if (first_time) {
						/* Hack to prevent matte manager from erasing big frame sprite */
						/* but to allow it to remain on the screen as long as nothing  */
						/* overlaps with it.                                           */
						image_marker = 0;
						first_time = false;
					}

					game_menu_changed = false;
				}

				mouse_end_cycle(false, false);

				kernel.fx = 0;
			}
		}

		switch (game_menu_selected_item) {
		case 1:
			kernel.activate_menu = GAME_SAVE_MENU;
			break;

		case 2:
			kernel.activate_menu = GAME_RESTORE_MENU;
			break;

		case 3:
			kernel.activate_menu = GAME_OPTIONS_MENU;
			break;

		case 4:
			kernel.activate_menu = 0;
			break;

		case 5:
			kernel.activate_menu = 0;
			game.going = false;
			break;

		default:
			kernel.activate_menu = 0;
			break;
		}
	}
}

static void game_menu_options() {
	int going = true;
	int first_time = true;
	int frame_id;
	long menu_clock = 0;
	long now_clock;
	ConfigFile save_config;

	save_config = config_file;

	game_menu_first_init();

	frame_id = game_menu_sprite(2, 2);

	while (kernel.activate_menu == GAME_OPTIONS_MENU) {
		going = true;

		game_menu_options_init();
		game_menu_hotspot_init();

		mouse_init_cycle();

		game_menu_changed = true;

		while (going && (game_menu_selected_item < 1)) {

			going = game_menu_keyboard(going) && !g_engine->shouldQuit();

			now_clock = timer_read();

			if (now_clock != menu_clock) {
				menu_clock = now_clock;

				mouse_begin_cycle(false);

				game_menu_process_hotspots();

				if (game_menu_changed) {

					if (game_menu_current_item == 0) game_menu_current_item = -1;

					game_menu_generate_messages();

					/* Matte out the next graphics frame */

					matte_frame(kernel.fx, false);

					if (first_time) {
						/* Hack to prevent matte manager from erasing big frame sprite */
						/* but to allow it to remain on the screen as long as nothing  */
						/* overlaps with it.                                           */
						image_marker = 0;
						first_time = false;
					}

					game_menu_changed = false;
				}

				mouse_end_cycle(false, false);

				kernel.fx = 0;
			}
		}

		switch (game_menu_selected_item) {
		case 0:
			game_menu_selected_item = game_menu_return_key ? 8 : 9;
			break;

		case 1:
			config_file.music_flag = !config_file.music_flag;
			break;

		case 2:
			config_file.sound_flag = !config_file.sound_flag;
			break;

		case 3:
			if (config_file.interface_hotspots == INTERFACE_MACINTOSH) {
				config_file.interface_hotspots = INTERFACE_BRAINDEAD;
			} else {
				config_file.interface_hotspots = INTERFACE_MACINTOSH;
			}
			break;

		case 4:
			if (config_file.inventory_mode == INVENTORY_SPINNING) {
				config_file.inventory_mode = INVENTORY_SQUAT;
			} else {
				config_file.inventory_mode = INVENTORY_SPINNING;
			}
			break;

		case 5:
			if (config_file.animated_interface == INTERFACE_ANIMATED) {
				config_file.animated_interface = INTERFACE_STILL;
			} else {
				config_file.animated_interface = INTERFACE_ANIMATED;
			}
			break;

		case 6:
			config_file.screen_fade++;
			if (config_file.screen_fade > SCREEN_FADE_FAST) {
				config_file.screen_fade = SCREEN_FADE_SMOOTH;
			}
			break;

		case 7:
			if (config_file.naughtiness == STORYMODE_NICE) {
				config_file.naughtiness = STORYMODE_NAUGHTY;
			} else {
				if (config_file.naughtiness == STORYMODE_NAUGHTY) {
					config_file.naughtiness = STORYMODE_NICE;
				}
			}
			break;
		}

		kernel.activate_menu = (game_menu_selected_item > 7) ? GAME_MAIN_MENU : GAME_OPTIONS_MENU;
	}

	if (game_menu_selected_item == 9) {
		save_config = config_file;
	} else {
		write_config_file();
		//game_load_config_parameters();
	}
}

static void game_menu_select_slot(int slot) {
	game_menu_save_select = slot;

	if ((game_menu_save_select >= game_menu_save_top) &&
		(game_menu_save_select < (game_menu_save_top + GAME_MAX_SAVES_ON_SCREEN))) {
		goto done;
	}

	game_menu_save_top = game_menu_save_select - 2;
	game_menu_save_top = MAX(game_menu_save_top, 0);

done:
	game_menu_save_select = MAX(game_menu_save_select, 0);
	game_menu_save_select = MIN(game_menu_save_select, GAME_MAX_SAVE_SLOTS - 1);
}

static void game_menu_select_free_slot() {
	int count;
	int match = -1;
	char *pointer;

	for (count = 0; (match < 0) && (count < GAME_MAX_SAVE_SLOTS); count++) {
		pointer = game_menu_save_string(count);
		if (!*pointer) match = count;
	}

	if (match >= 0) {
		game_menu_select_slot(match);
	}
}

static void game_menu_select_last() {
	int match;
	int count;
	char *string;

	if (game.last_save < 0) {
		match = -1;
		for (count = GAME_MAX_SAVE_SLOTS - 1; (match < 0) && (count >= 0); count--) {
			string = game_menu_save_string(count);
			if (*string) {
				match = count;
			}
		}
		if (match >= 0) {
			game.last_save = match;
		} else {
			game.last_save = 0;
		}
	}

	game_menu_select_slot(game.last_save);

	string = game_menu_save_string(game_menu_save_select);
	if (!*string) {
		game_menu_select_free_slot();
	}
}

static int game_menu_save_keyboard(int going) {
	int mykey;
	int width;
	char teeny[2];
	char *string;
	char work_buf[80];

	if (keys_any()) {
		mykey = keys_get();
		going = game_menu_standard_keyboard(mykey, going);

		if (going || game_menu_return_key) {

			if (mykey == bksp_key) {
				string = game_menu_save_string(game_menu_save_select);

				if (*string) {
					if (!game_menu_save_dirty) {
						Common::strcpy_s(game_menu_save_buffer, GAME_MAX_SAVE_LENGTH, string);
						game_menu_save_pointer = string;
						game_menu_save_dirty = true;
					}
					string[strlen(string) - 1] = 0;
				}

				game_save_menu_virgin = false;
				going = false;
			}

			if (mykey == enter_key) {
				game_menu_selected_item = 15;
				game_save_menu_virgin = false;
				going = false;
			}

			if (mykey == ctrl_c_key) {
				string = game_menu_save_string(game_menu_save_select);
				if (!game_menu_save_dirty) {
					Common::strcpy_s(game_menu_save_buffer, GAME_MAX_SAVE_LENGTH, string);
					game_menu_save_pointer = string;
					game_menu_save_dirty = true;
				}
				*string = 0;
			}

			if (mykey == ctrl_r_key) {
				game_menu_save_dirty = false;
			}

			if (Common::isPrint(mykey)) {
				if (game_save_menu_virgin) {
					game_menu_select_free_slot();
				}

				string = game_menu_save_string(game_menu_save_select);

				if (strlen(string) < GAME_MAX_SAVE_LENGTH) {
					teeny[0] = (char)mykey;
					teeny[1] = 0;
					Common::strcpy_s(work_buf, string);
					Common::strcat_s(work_buf, teeny);
					width = font_string_width(font_conv, work_buf, -1);
					if (width <= GAME_MAX_SAVE_WIDTH) {
						if (!game_menu_save_dirty) {
							Common::strcpy_s(game_menu_save_buffer, GAME_MAX_SAVE_LENGTH, string);
							game_menu_save_pointer = string;
							game_menu_save_dirty = true;
						}
						Common::strcat_s(string, GAME_MAX_SAVE_LENGTH, teeny);
					}
				}

				game_save_menu_virgin = false;
				game_menu_changed = true;
				going = false;
			}

		}
	}

	return going;
}

static void game_menu_save() {
	int going = true;
	int first_time = true;
	int frame_id;
	int id;
	int special_sprite;
	int let_scroll_continue;
	int old_scrolling;
	int new_scrolling;
	long menu_clock = 0;
	long now_clock;
	long scroll_base_clock = 0;
	char *string;

	game_menu_select_last();

	game_save_menu_virgin = true;

	game_been_to_save_screen = true;

	game_menu_save_stroke = 0;

	game_menu_first_init();

	frame_id = game_menu_sprite(3, 2);

	game_menu_save_dirty = false;

	while (kernel.activate_menu == GAME_SAVE_MENU) {

		going = true;

		game_menu_save_select = MAX(game_menu_save_select, game_menu_save_top);
		game_menu_save_select = MIN(game_menu_save_select, game_menu_save_top + GAME_MAX_SAVES_ON_SCREEN - 1);
		game_menu_save_select = MIN(game_menu_save_select, GAME_MAX_SAVE_SLOTS - 1);

		let_scroll_continue = 0;
		if (!first_time && (game_menu_selected_item >= 50)) {
			let_scroll_continue = game_menu_selected_item;
		}

		game_menu_save_init();
		game_menu_hotspot_init();

		if (!let_scroll_continue) {
			mouse_init_cycle();
		} else {
			game_menu_current_item = let_scroll_continue;
		}

		game_menu_changed = true;

		while (going && (game_menu_selected_item < 1)) {

			going = game_menu_save_keyboard(going);

			now_clock = timer_read();

			if (now_clock != menu_clock) {
				menu_clock = now_clock;

				mouse_begin_cycle(false);

				old_scrolling = (game_menu_current_item >= 50);

				game_menu_process_hotspots();

				new_scrolling = (game_menu_current_item >= 50);

				if (new_scrolling) {
					if (!old_scrolling) {
						scroll_base_clock = now_clock + GAME_MENU_SCROLL_FIRST;
						let_scroll_continue = 0;
					} else {
						if (now_clock >= scroll_base_clock) {
							game_menu_selected_item = game_menu_current_item;
							game_menu_changed = true;
							scroll_base_clock = now_clock + GAME_MENU_SCROLL_SECOND;
						} else if (let_scroll_continue && (game_menu_selected_item >= 0)) {
							game_menu_selected_item = -1;
						}
					}
				}

				if (game_menu_changed) {

					if (game_menu_current_item == 0) game_menu_current_item = -1;

					id = (game_menu_save_select - game_menu_save_top) + 1;
					if (!menu[id].status) menu[id].status = 2;

					game_menu_generate_messages();

					special_sprite = (game_menu_current_item == 50) ? 2 : 0;
					id = game_menu_sprite(4 + special_sprite, 1);

					special_sprite = (game_menu_current_item == 51) ? 2 : 0;
					id = game_menu_sprite(5 + special_sprite, 1);

					/* Matte out the next graphics frame */

					matte_frame(kernel.fx, false);

					/* Hack to prevent matte manager from erasing big frame sprite */
					/* but to allow it to remain on the screen as long as nothing  */
					/* overlaps with it.                                           */
					image_marker = 0;
					first_time = 0;

					if (special_sprite >= 0) {
						image_list[special_sprite].flags = IMAGE_ERASE;
					}

					game_menu_changed = false;
				}

				mouse_end_cycle(false, false);

				kernel.fx = 0;
			}
		}

		game_save_menu_virgin = false;

		if ((game_menu_selected_item > 0) &&
			(game_menu_selected_item <= GAME_MAX_SAVES_ON_SCREEN)) {
			game_menu_save_select = game_menu_save_top + game_menu_selected_item - 1;
		}

		switch (game_menu_selected_item) {
		case 0:
			game_menu_selected_item = 17;
			break;

		case 15:
			game_menu_save_dirty = false;
			break;

		case 16:
			string = game_menu_save_string(game_menu_save_select);
			*string = 0;
			game_menu_save_dirty = false;
			break;

		case 50:
			if (game_menu_save_top > 0) {
				game_menu_save_top--;
			}
			break;

		case 51:
			if (game_menu_save_top < GAME_MAX_SAVE_SLOTS - 1) {
				game_menu_save_top++;
			}
			break;

		default:
			break;
		}

		kernel.activate_menu = ((game_menu_selected_item == 15) ||
			(game_menu_selected_item == 17)) ? GAME_MAIN_MENU : GAME_SAVE_MENU;

		if (game_menu_save_dirty) {
			if ((kernel.activate_menu != GAME_SAVE_MENU) ||
				(game_menu_selected_item > 0)) {
				Common::strcpy_s(game_menu_save_pointer, GAME_MAX_SAVE_LENGTH, game_menu_save_buffer);
				game_menu_save_dirty = false;
			}
		}
	}

	if (game_menu_selected_item == 15) {
		id = game_menu_save_select;
		game.last_save = id;
		global_save(id, game_menu_save_buffer);
		kernel.activate_menu = GAME_ALERT_MENU;
	}
}

static int game_menu_restore_keyboard(int going) {
	int mykey;

	if (keys_any()) {
		mykey = keys_get();
		going = game_menu_standard_keyboard(mykey, going);

		if (going || game_menu_return_key) {
			if (mykey == enter_key) {
				game_menu_selected_item = 15;
				going = false;
			}
		}
	}

	return going;
}

static void game_menu_restore() {
	int going = true;
	int first_time = true;
	int frame_id;
	int id;
	int special_sprite;
	int let_scroll_continue;
	int old_scrolling;
	int new_scrolling;
	long menu_clock = 0;
	long now_clock;
	long scroll_base_clock = 0;
	char *string;

	game_been_to_save_screen = true;

	game_menu_select_last();

	game_menu_save_stroke = 0;

	game_menu_first_init();

	frame_id = game_menu_sprite(3, 2);

	while (kernel.activate_menu == GAME_RESTORE_MENU) {

		going = true;

		game_menu_save_select = MAX(game_menu_save_select, game_menu_save_top);
		game_menu_save_select = MIN(game_menu_save_select, game_menu_save_top + GAME_MAX_SAVES_ON_SCREEN - 1);
		game_menu_save_select = MIN(game_menu_save_select, GAME_MAX_SAVE_SLOTS - 1);

		let_scroll_continue = 0;
		if (!first_time && (game_menu_selected_item >= 50)) {
			let_scroll_continue = game_menu_selected_item;
		}

		game_menu_restore_init();
		game_menu_hotspot_init();

		if (!let_scroll_continue) {
			mouse_init_cycle();
		} else {
			game_menu_current_item = let_scroll_continue;
		}

		game_menu_changed = true;

		while (going && (game_menu_selected_item < 1)) {

			going = game_menu_restore_keyboard(going);

			now_clock = timer_read();

			if (now_clock != menu_clock) {
				menu_clock = now_clock;

				mouse_begin_cycle(false);

				old_scrolling = (game_menu_current_item >= 50);

				game_menu_process_hotspots();

				new_scrolling = (game_menu_current_item >= 50);

				if (new_scrolling) {
					if (!old_scrolling) {
						scroll_base_clock = now_clock + GAME_MENU_SCROLL_FIRST;
						let_scroll_continue = 0;
					} else {
						if (now_clock >= scroll_base_clock) {
							game_menu_selected_item = game_menu_current_item;
							game_menu_changed = true;
							scroll_base_clock = now_clock + GAME_MENU_SCROLL_SECOND;
						} else if (let_scroll_continue && (game_menu_selected_item >= 0)) {
							game_menu_selected_item = -1;
						}
					}
				}

				if (game_menu_changed) {

					if (game_menu_current_item == 0) game_menu_current_item = -1;

					id = (game_menu_save_select - game_menu_save_top) + 1;
					if (!menu[id].status) menu[id].status = 2;

					game_menu_generate_messages();

					special_sprite = (game_menu_current_item == 50) ? 2 : 0;
					id = game_menu_sprite(4 + special_sprite, 1);

					special_sprite = (game_menu_current_item == 51) ? 2 : 0;
					id = game_menu_sprite(5 + special_sprite, 1);

					/* Matte out the next graphics frame */

					matte_frame(kernel.fx, false);

					/* Hack to prevent matte manager from erasing big frame sprite */
					/* but to allow it to remain on the screen as long as nothing  */
					/* overlaps with it.                                           */
					image_marker = 0;
					first_time = 0;

					if (special_sprite >= 0) {
						image_list[special_sprite].flags = IMAGE_ERASE;
					}

					game_menu_changed = false;
				}

				mouse_end_cycle(false, false);

				kernel.fx = 0;
			}
		}

		if ((game_menu_selected_item > 0) &&
			(game_menu_selected_item <= GAME_MAX_SAVES_ON_SCREEN)) {
			game_menu_save_select = game_menu_save_top + game_menu_selected_item - 1;
		}

		switch (game_menu_selected_item) {
		case 0:
			game_menu_selected_item = 17;
			break;

		case 15:
			break;

		case 16:
			string = game_menu_save_string(game_menu_save_select);
			*string = 0;
			break;

		case 50:
			if (game_menu_save_top > 0) {
				game_menu_save_top--;
			}
			break;

		case 51:
			if (game_menu_save_top < GAME_MAX_SAVE_SLOTS - 1) {
				game_menu_save_top++;
			}
			break;

		default:
			break;
		}

		kernel.activate_menu = ((game_menu_selected_item == 15) ||
			(game_menu_selected_item == 17)) ? GAME_MAIN_MENU : GAME_RESTORE_MENU;
	}

	if (game_menu_selected_item == 15) {
		id = game_menu_save_select;
		game.last_save = id;
		global_restore(id);
		kernel.activate_menu = GAME_ALERT_MENU;
	}
}

static void game_menu_difficulty() {
	int going = true;
	int first_time = true;
	int frame_id;
	long menu_clock = 0;
	long now_clock;

	game_menu_first_init();

	frame_id = game_menu_sprite(8, 2);

	while (kernel.activate_menu == GAME_DIFFICULTY_MENU) {

		going = true;

		game_menu_difficulty_init();
		game_menu_hotspot_init();

		mouse_init_cycle();

		game_menu_changed = true;

		while (going && (game_menu_selected_item < 1)) {
			going = game_menu_keyboard(going) && !g_engine->shouldQuit();

			now_clock = timer_read();

			if (now_clock != menu_clock) {
				menu_clock = now_clock;

				mouse_begin_cycle(false);

				game_menu_process_hotspots();

				if (game_menu_changed) {

					if (game_menu_current_item == 0) game_menu_current_item = -1;

					game_menu_generate_messages();

					/* Matte out the next graphics frame */

					matte_frame(kernel.fx, false);

					if (first_time) {
						/* Hack to prevent matte manager from erasing big frame sprite */
						/* but to allow it to remain on the screen as long as nothing  */
						/* overlaps with it.                                           */
						image_marker = 0;
						first_time = false;
					}

					game_menu_changed = false;
				}

				mouse_end_cycle(false, false);

				kernel.fx = 0;
			}
		}

		if (game_menu_return_key) {
			game_menu_selected_item = 1;
		}

		switch (game_menu_selected_item) {
		case 1:
			game.difficulty = DIFFICULTY_EASY;
			kernel.activate_menu = 0;
			break;

		case 2:
			game.difficulty = DIFFICULTY_MEDIUM;
			kernel.activate_menu = 0;
			break;

		case 3:
			game.difficulty = DIFFICULTY_HARD;
			kernel.activate_menu = 0;
			break;

		default:
			game.going = false;
			kernel.activate_menu = 0;
			break;
		}
	}
}

static void game_menu_alert() {
	int going = true;
	int first_time = true;
	int frame_id;
	long menu_clock = 0;
	long now_clock;

	game_menu_first_init();

	frame_id = game_menu_sprite(9, 2);

	while (kernel.activate_menu == GAME_ALERT_MENU) {

		going = true;

		game_menu_alert_init();
		game_menu_hotspot_init();

		mouse_init_cycle();

		game_menu_changed = true;

		while (going && (game_menu_selected_item < 1)) {

			going = game_menu_keyboard(going) && !g_engine->shouldQuit();

			now_clock = timer_read();

			if (now_clock != menu_clock) {
				menu_clock = now_clock;

				mouse_begin_cycle(false);

				game_menu_process_hotspots();

				if (game_menu_changed) {

					if (game_menu_current_item < 2) game_menu_current_item = -1;

					game_menu_generate_messages();

					/* Matte out the next graphics frame */

					matte_frame(kernel.fx, false);

					if (first_time) {
						/* Hack to prevent matte manager from erasing big frame sprite */
						/* but to allow it to remain on the screen as long as nothing  */
						/* overlaps with it.                                           */
						image_marker = 0;
						first_time = false;
					}

					game_menu_changed = false;
				}

				mouse_end_cycle(false, false);

				kernel.fx = 0;
			}
		}

		if (game_menu_return_key) {
			game_menu_selected_item = 1;
		}

		kernel.activate_menu = 0;
	}
}

void global_menu_system_init() {
	// No implementation
}

void global_menu_system_shutdown() {
	// No implementation
}

void global_game_menu() {


	while (keys_any())
		keys_get();

	game_menu_setup();

	game_menu_direct_jump = (kernel.activate_menu != GAME_MAIN_MENU);

	while (kernel.activate_menu && game.going) {

		switch (kernel.activate_menu) {
		case GAME_MAIN_MENU:
			game_menu_main();
			break;

		case GAME_SAVE_MENU:
			game_menu_save();
			break;

		case GAME_RESTORE_MENU:
			game_menu_restore();
			break;

		case GAME_OPTIONS_MENU:
			game_menu_options();
			break;

		case GAME_DIFFICULTY_MENU:
			game_menu_difficulty();
			break;

		case GAME_ALERT_MENU:
			game_menu_alert();
			break;

		default:
			kernel.activate_menu = 0;
		}

		if ((kernel.activate_menu == GAME_MAIN_MENU) && game_menu_direct_jump) {
			kernel.activate_menu = 0;
		}
	}

	game_menu_shutdown();
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
