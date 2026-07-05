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

#include "common/textconsole.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/forest/midi.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

int int_sprite[6];
bool knuthole_flag;
int paul_object_showing;

void init_extra() {
	Common::fill(int_sprite, int_sprite + 6, 0);
	knuthole_flag = false;
	paul_object_showing = -1;
}

void clear_selected_item() {
	inter_turn_off_object();
	buffer_rect_copy_2(scr_inter_orig, scr_live, 138, 0, 138, 156, 55, 42);
}

void solve_me_selected() {
	global[walker_converse_state] = -1;
	open_interface(CANDLE_FLY);
	game_exec_function(room_parser_code_pointer);
}

void door_selected() {
	open_interface(DOOR_FLY);
	kernel.activate_menu = GAME_MAIN_MENU;
}

void stamp_sprite_to_interface(int x, int y, int sprite, int series) {
	ImageInter &ii = image_inter_list[image_inter_marker++];

	ii.flags = IMAGE_UPDATE;
	ii.segment_id = 200;
	ii.sprite_id = sprite;
	ii.series_id = series;
	ii.x = x;
	ii.y = y;
}

void delete_sprite_in_interface(int series) {
	matte_deallocate_series(series, false);

	// WORKAROUND: Remove any pending interface entries referencing the now-freed series.
	// This fixes a game crash when opening the inventory, since there were pending items
	// in image_inter_list for the series_id that had been deallocated
	for (int i = (int)image_inter_marker - 1; i >= 0; --i) {
		if (image_inter_list[i].series_id == series) {
			for (int j = i; j < (int)image_inter_marker - 1; ++j)
				image_inter_list[j] = image_inter_list[j + 1];
			--image_inter_marker;
		}
	}
}

void load_interface() {
	int_sprite[fx_int_journal] = kernel_load_series("*journal", false);
	int_sprite[fx_int_backpack] = kernel_load_series("*backpack", false);
	int_sprite[fx_int_candle] = kernel_load_series("*candle", false);
	int_sprite[fx_int_exit] = kernel_load_series("*door", false);
	int_sprite[fx_int_dooropen] = kernel_load_series("*dooropen", false);
	int_sprite[fx_int_candle_on] = kernel_load_series("*candleon", false);
}

void unload_interface() {
	if (int_sprite[fx_int_journal] != -1) {
		matte_deallocate_series(int_sprite[fx_int_candle_on], true);
		matte_deallocate_series(int_sprite[fx_int_dooropen], true);
		matte_deallocate_series(int_sprite[fx_int_exit], true);
		matte_deallocate_series(int_sprite[fx_int_candle], true);
		matte_deallocate_series(int_sprite[fx_int_backpack], true);
		matte_deallocate_series(int_sprite[fx_int_journal], true);
		int_sprite[fx_int_journal] = -1;
	}
}

void draw_interface() {
	// Draw backpack icon
	stamp_sprite_to_interface(BP_X, BP_Y, 1, int_sprite[fx_int_backpack]);

	// Draw candle cion
	if (global[Forest::walker_converse_state]) {
		stamp_sprite_to_interface(CANDLE_X, CANDLE_Y, 1, int_sprite[fx_int_candle_on]);
	} else {
		stamp_sprite_to_interface(CANDLE_X, CANDLE_Y, 1, int_sprite[fx_int_candle]);
	}

	// Draw door icon
	stamp_sprite_to_interface(DOOR_X, DOOR_Y, 1, int_sprite[fx_int_exit]);

	// Draw journal icon, except when in journal room
	if (room_id != 199) {
		stamp_sprite_to_interface(JOURNAL_X, JOURNAL_Y, 1, int_sprite[fx_int_journal]);
	}
}

void do_interface() {
	if (mouse_y > 156 && mouse_stop_stroke && player.commands_allowed &&
			!kernel.trigger && inter_input_mode == INTER_LIMITED_SENTENCES &&
			!global[inventory_is_displayed]) {
		// Interface click handling
		if (room_id == 199) {
			leave_journal();
		} else if (mouse_x < 64) {
			display_journal();
		} else if (mouse_x < 139) {
			display_inventory();
		} else if (mouse_x < 195) {
		} else if (mouse_x < 250) {
			solve_me_selected();
		} else {
			door_selected();
		}
	}

	if (kernel.trigger == 40) {
		display_inventory();
	}
}

void inter_update_series(int series_id) {
	for (int count = 0; count < (int)image_inter_marker; count++) {
		ImageInter &ii = image_inter_list[count];

		if (ii.series_id == series_id)
			ii.flags = IMAGE_ERASE;
		else if (ii.flags != IMAGE_ERASE)
			ii.flags = IMAGE_UPDATE;
	}
}

void open_interface(InterfaceButton button) {
	mouse_hide();

	int count = 0;
	do {
		digi_read_another_chunk();
		if (global[g009])
			midi_loop();

		long current_time = timer_read();

		buffer_rect_copy(scr_inter_orig, scr_inter, 0, 0, 165, 44);

		if (button == JOURNAL_FLY) {
			if (count != 23) {
				int y = count * 2 + 46;
				int scale = 100 - count * 9;
				sprite_draw_scaled(series_list[0], 1, &scr_inter, 30, y, scale);
			}
		} else {
			sprite_draw(series_list[0], 1, &scr_inter, JOURNAL_X, JOURNAL_Y);
		}

		if (button == BP_FLY) {
			if (count != 23) {
				int y = count * 2 + 40;
				int scale = 100 - count * 9;
				sprite_draw_scaled(series_list[1], 1, &scr_inter, 96, y, scale);
			}
		} else {
			sprite_draw(series_list[1], 1, &scr_inter, BP_X, BP_Y);
		}

		buffer_rect_copy_2(scr_inter, scr_main, 0, 0, 0, 156, 165, 44);
		buffer_rect_copy(scr_inter_orig, scr_inter, 168, 0, 152, 44);

		if (button == CANDLE_FLY)
			count = 24;

		if (global[walker_converse_state])
			sprite_draw(series_list[5], 1, &scr_inter, CANDLE_X, CANDLE_Y);
		else
			sprite_draw(series_list[2], 1, &scr_inter, CANDLE_X, CANDLE_Y);

		if (button == DOOR_FLY) {
			sprite_draw(series_list[4], 1, &scr_inter, DOOR_X, DOOR_Y);
			count = 24;
		} else {
			sprite_draw(series_list[3], 1, &scr_inter, DOOR_X, DOOR_Y);
		}

		buffer_rect_copy_2(scr_inter, scr_main, 168, 0, 168, 156, 152, 44);
		buffer_rect_copy_2(scr_main, scr_live, 0, 156, 0, 156, 320, 44);

		while (timer_read() - current_time < 2) {
		}

		count++;
	} while (count < 24);

	mouse_show();
}

void close_interface(InterfaceButton button) {
	mouse_hide();

	int count = 24;
	do {
		digi_read_another_chunk();
		if (global[g009])
			midi_loop();

		long current_time = timer_read();

		buffer_rect_copy(scr_inter_orig, scr_inter, 0, 0, 165, 44);

		if (button == JOURNAL_FLY) {
			if (count != 0) {
				int y = count * 2 + 43;
				int scale = 100 - count * 9;
				sprite_draw_scaled(series_list[0], 1, &scr_inter, 30, y, scale);
			} else {
				sprite_draw(series_list[0], 1, &scr_inter, JOURNAL_X, JOURNAL_Y);
			}
		} else {
			sprite_draw(series_list[0], 1, &scr_inter, JOURNAL_X, JOURNAL_Y);
		}

		if (button == BP_FLY) {
			if (count != 0) {
				int y = count * 2 + 37;
				int scale = 100 - count * 9;
				sprite_draw_scaled(series_list[1], 1, &scr_inter, 96, y, scale);
			} else {
				sprite_draw(series_list[1], 1, &scr_inter, BP_X, BP_Y);
			}
		} else {
			sprite_draw(series_list[1], 1, &scr_inter, BP_X, BP_Y);
		}

		buffer_rect_copy_2(scr_inter, scr_main, 0, 0, 0, 156, 165, 44);
		buffer_rect_copy(scr_inter_orig, scr_inter, 168, 0, 152, 44);

		if (button == CANDLE_FLY)
			count = 0;

		if (global[walker_converse_state])
			sprite_draw(series_list[5], 1, &scr_inter, CANDLE_X, CANDLE_Y);
		else
			sprite_draw(series_list[2], 1, &scr_inter, CANDLE_X, CANDLE_Y);

		sprite_draw(series_list[3], 1, &scr_inter, DOOR_X, DOOR_Y);

		buffer_rect_copy_2(scr_inter, scr_main, 168, 0, 168, 156, 152, 44);
		buffer_rect_copy_2(scr_main, scr_live, 0, 156, 0, 156, 320, 44);

		while (timer_read() - current_time < 2) {
		}

		count--;
	} while (count >= 0);

	mouse_show();
}

void extra_change_animation(int handle, int x, int y, int scale, int depth) {
	Animation &k_anim = kernel_anim[handle];
	Anim *anim = k_anim.anim;

	for (int count = 0; count < anim->num_frames; ++count) {
		Image &image = anim->image[count];
		image.x = x;
		image.y = y;
		image.scale = scale;
		image.depth = depth;
	}
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
