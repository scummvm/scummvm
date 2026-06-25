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

#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/inventory.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/midi.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

char interface_name[40];

void init_inventory() {
	Common::fill(interface_name, interface_name + 40, 0);
}

static Series *object_icon(int num) {
	Common::String name = Common::String::format("*ob%03di", num);
	return sprite_series_load(name.c_str(), 0);
}

static int inventory_get_item(int num) {
	if (!mouse_stop_stroke)
		return 0;
	if (num < 1)
		return -1;

	int var_6 = 0;
	for (int dx = 1; dx <= num; dx++) {
		int cx;
		switch (dx) {
		case 1:  case 8:  cx = 9;   break;
		case 2:  case 9:  cx = 51;  break;
		case 3:  case 10: cx = 93;  break;
		case 4:  case 11: cx = 135; break;
		case 5:  case 12: cx = 177; break;
		case 6:  case 13: cx = 219; break;
		case 7:  case 14: cx = 261; break;
		default:          cx = var_6; break;
		}
		int bx = cx + 42;
		int di = (dx >= 8) ? 108 : 72;
		int ax = di + 36;
		var_6 = cx;

		if (mouse_x >= cx && mouse_x <= bx && mouse_y >= di && mouse_y <= ax)
			return dx;
	}
	return -1;
}

static void kernel_load_interface(void) {
	char temp_buf[80];
	char *mark;

	Common::strcpy_s(temp_buf, kernel.interface);
	mark = strchr(temp_buf, '.');
	if (mark != NULL) {
		*mark = 0;
	}
	if (inter_input_mode != INTER_BUILDING_SENTENCES) {
		Common::strcat_s(temp_buf, "A");
	}
	Common::strcat_s(temp_buf, ".INT");

	if (strcmp(interface_name, temp_buf)) {
		buffer_free(&scr_inter_orig);
		pal_activate_shadow(&kernel_shadow_inter);
		if (inter_load_background(temp_buf, &scr_inter_orig)) {
			error_report(ERROR_KERNEL_NO_INTERFACE, SEVERE, MODULE_KERNEL, inter_input_mode, 0);
		}
		Common::strcpy_s(interface_name, temp_buf);
		pal_activate_shadow(&kernel_shadow_main);
	}
}

static void inven_set_interface_mode(int mode) {
	inter_input_mode = 2;
	kernel_load_interface();
	inter_base_time = timer_read();
	left_command = -1;
	left_action  = -1;
	left_inven   = -1;

	if (!viewing_at_y) {
		if (mode != 0)
			buffer_rect_copy(scr_inter_orig, scr_inter, 138, 0, 58, 44);
		else
			buffer_rect_copy(scr_inter_orig, scr_inter, 0, 0, 320, 44);

		if (kernel_mode == 5)
			matte_inter_frame(0, 0);
	}

	inter_init_sentence();
	inter_setup_hotspots();

	if (!viewing_at_y)
		inter_prepare_background();

	kernel_refresh_dynamic();
}

void display_inventory() {
	int var_10 = 17;
	int var_12 = 72;
	int object_id  = -1;
	int var_22 = -1;
	int var_6  = 0;
	int var_E  = 0;
	int var_20 = 0;
	int var_4  = 0;
	int var_1E = 0;
	long var_1C_1A = 0;
	int var_8  = 0;

	if (kernel.trigger != 0)
		return;

	inter_turn_off_object();
	buffer_rect_copy_2(scr_inter_orig, scr_live, 138, 0, 138, 156, 55, 42);
	inter_update_series(int_sprite[fx_int_journal]);
	inter_update_series(int_sprite[fx_int_backpack]);
	inter_update_series(int_sprite[fx_int_candle]);
	inter_update_series(int_sprite[fx_int_exit]);
	inven_set_interface_mode(-1);

	open_journal(2);
	cursor_id = 2;
	if (cursor_last != 2) {
		mouse_cursor_sprite(cursor, 2);
		cursor_last = cursor_id;
	}

	for (int on = 0; on < 16; on++) {
		if (object[on].location == 2)
			var_20++;
	}

	global[inventory_is_displayed] = -1;
	mouse_hide();

	// Draw inventory background
	SeriesPtr bg_series = sprite_series_load("*backinv", 0);
	mcga_setpal(&master_palette);
	sprite_draw(bg_series, 1, &scr_live, 1, 53);
	sprite_free(&bg_series, -1);

	// Draw inventory item icons in a 7-per-row grid
	int object_num = 0;
	while (object_num < 16) {
		if (object[object_num].location != 2) {
			object_num++;
			continue;
		}
		SeriesPtr icon_series = object_icon(object_num);
		mcga_setpal(&master_palette);
		var_6++;
		if (var_6 % 8 == 0) {
			var_12 += 40;
			var_10 = 17;
		} else if (var_6 != 1) {
			var_10 += 41;
		}
		sprite_draw(icon_series, 1, &scr_live, var_10, var_12);
		sprite_free(&icon_series, -1);
		object_num++;
	}

	mouse_cursor_sprite(cursor, 1);
	mouse_show();

	// Main event loop
	for (;;) {
		digi_read_another_chunk();
		if (global[g009])
			midi_loop();
		mouse_begin_cycle(0);
		// TODO: delay_to_expiry(0, -1) — not yet declared

		if (var_1E) {
			long current = timer_read();
			if (current > var_1C_1A + (int16)var_8)
				goto exit_inventory;
		}

		if (!player.commands_allowed)
			continue;

		var_4 = inventory_get_item(var_20);
		if (var_4 == 0)
			continue;

		if (var_4 == -1) {
			kernel_force_refresh();
			goto cancel_inventory;
		}

		// Find the var_4-th inventory item
		object_num = 0;
		var_E = 0;
		while (object_num < 16) {
			if (object[object_num].location == 2) {
				var_E++;
				if (var_E == var_4) {
					object_id = object_num;
					break;
				}
			}
			object_num++;
		}
		if (object_num >= 16) {
			var_E = 0;
			var_4 = 0;
			continue;
		}

		kernel_force_refresh();

		// Determine usability based on object number and room
		if (object_id <= 15) {
			switch (object_id) {
			case 0:
				var_22 = (room_id == 521) ? 0 : -1;
				break;
			case 1: case 10: case 15:
				var_22 = (room_id == 308) ? 0 : -1;
				break;
			case 2: case 3: case 6: case 7: case 9: case 11: case 12:
				var_22 = (room_id == 521) ? 0 : -1;
				break;
			case 4: case 14:
				var_22 = (room_id == 307 || room_id == 322 || room_id == 308 ||
				          room_id == 401 || room_id == 512) ? 0 : -1;
				break;
			case 5:
				var_22 = (room_id == 203 || room_id == 308) ? 0 : -1;
				break;
			case 8:
				var_22 = (room_id == 203) ? 0 : -1;
				break;
			case 13:
				var_22 = (room_id == 404) ? 0 : -1;
				break;
			default:
				break;
			}
		}

		if (object_check_quality(object_id, 0))
			var_22 = -1;

		// Room 203: special quality-gated overrides
		if (room_id == 203 && !global[g046]) {
			if (object_id == 8 || object_id == 5) {
				if (object_check_quality(object_id, 0))
					var_22 = -1;
				else
					var_22 = 0;
			} else {
				var_22 = 0;
			}
		}

		// Room 402
		if (room_id == 402) {
			if (global[g083] >= 3)
				var_22 = -1;
			else
				var_22 = 0;
		}

		// Room 308
		if (room_id == 308) {
			if (object_id == 1 || object_id == 15 || object_id == 5) {
				if (object_check_quality(object_id, 0))
					var_22 = -1;
				else
					var_22 = 0;
			} else {
				var_22 = 0;
			}
		}

		if (object_id == -1 || var_4 == -1 || var_22 != 0) {
			var_E = 0;
			var_4 = 0;
			continue;
		}

		// var_22 == 0: item may be usable here — check room-specific audio/action
		switch (room_id) {
		case 221:
			digi_play_build(221, 'e', 5, 2);
			var_22 = -1;
			break;
		case 203:
			if (object_id != 5 && object_id != 8) {
				digi_play_build(203, 'R', 5, 2);
				var_22 = -1;
			}
			break;
		case 308:
			if (object_id == 1 || object_id == 15) {
				var_8 = 90;
				digi_play_build(308, 'e', 4, 1);
				var_1C_1A = timer_read();
				var_1E = -1;
				player.commands_allowed = false;
				mouse_cursor_sprite(cursor, 2);
				continue;
			} else if (object_id == 5) {
				var_8 = 180;
				digi_play_build(308, 'r', 2, 1);
				var_1C_1A = timer_read();
				var_1E = -1;
				player.commands_allowed = false;
				mouse_cursor_sprite(cursor, 2);
				continue;
			} else {
				digi_play_build(308, 'r', 3, 1);
				var_22 = -1;
			}
			break;
		case 402:
			digi_play_build(203, 'R', 5, 2);
			var_22 = -1;
			break;
		default:
			break;
		}

		if (var_22 != 0) {
			var_E = 0;
			var_4 = 0;
			continue;
		}

		// Item is usable — exit inventory with selection
		mouse_cursor_sprite(cursor, 2);

exit_inventory:
		global[inventory_is_displayed] = 0;
		var_6 = var_20;
		close_journal(2);
		knuthole_flag = true;
		inter_spin_object(object_id);
		inter_move_object(object_id, PLAYER);
		global[player_selected_object] = object_id;

		player_verb        = -1;
		player_main_noun   = -1;
		player_second_noun = -1;
		inv_enable_command = true;

		game_exec_function(room_pre_parser_code_pointer);
		game_exec_function(room_parser_code_pointer);
		goto restore_interface;

cancel_inventory:
		mouse_cursor_sprite(cursor, 2);
		global[inventory_is_displayed] = 0;
		var_6 = var_20;
		close_journal(2);

restore_interface:
		stamp_sprite_to_interface(JOURNAL_X, JOURNAL_Y, 1, int_sprite[fx_int_journal]);
		stamp_sprite_to_interface(BP_X,      BP_Y,      1, int_sprite[fx_int_backpack]);
		stamp_sprite_to_interface(CANDLE_X,  CANDLE_Y,  1, int_sprite[fx_int_candle]);
		stamp_sprite_to_interface(DOOR_X,    DOOR_Y,    1, int_sprite[fx_int_exit]);
		if (inv_enable_command) {
			mouse_cursor_sprite(cursor, 1);
			player.commands_allowed = true;
		}
		return;
	}
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
