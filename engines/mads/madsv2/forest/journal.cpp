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

#include "mads/madsv2/forest/journal.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/timer.h"
#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/midi.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

static int prior_room;

static void change_to_journal() {
	save_player();
	prior_room = room_id;
	new_room = 199;
}

static int get_prior_room() {
	return prior_room;
}

void display_journal() {
	open_journal(1);
	change_to_journal();
}

void leave_journal() {
	close_journal(1);
	global[g009] = 0;
	midi_stop();
	new_room = get_prior_room();
}

void open_journal(int mode) {
	mouse_hide();

	int count = 0;
	do {
		digi_read_another_chunk();
		if (global[g009])
			midi_loop();

		long current_time = timer_read();

		buffer_rect_copy(scr_inter_orig, scr_inter, 0, 0, 165, 44);

		if (mode == 1) {
			if (count != 23) {
				int x = count * 2 + 46;
				int clip_x = 100 - count * 9;
				sprite_draw_clipped(series_list[0], 1, clip_x, &scr_inter, x, 30);
			}
		} else {
			sprite_draw(series_list[0], 3, &scr_inter, 1, 8);
		}

		if (mode == 2) {
			if (count != 23) {
				int x = count * 2 + 40;
				int clip_x = 100 - count * 9;
				sprite_draw_clipped(series_list[1], 1, clip_x, &scr_inter, x, 96);
			}
		} else {
			sprite_draw(series_list[1], 1, &scr_inter, 1, 68);
		}

		buffer_rect_copy_2(scr_inter, scr_main, 0, 0, 0, 156, 165, 44);
		buffer_rect_copy(scr_inter_orig, scr_inter, 168, 0, 152, 44);

		if (mode == 3)
			count = 24;

		if (global[walker_converse_state])
			sprite_draw(series_list[5], 4, &scr_inter, 1, 194);
		else
			sprite_draw(series_list[2], 4, &scr_inter, 1, 194);

		if (mode == 4) {
			sprite_draw(series_list[4], 2, &scr_inter, 1, 264);
			count = 24;
		} else {
			sprite_draw(series_list[3], 2, &scr_inter, 1, 264);
		}

		buffer_rect_copy_2(scr_inter, scr_main, 168, 0, 168, 156, 152, 44);

		while (timer_read() - current_time < 2) {}

		count++;
	} while (count < 24);

	mouse_show();
}

void close_journal(int mode) {
	mouse_hide();

	int count = 24;
	do {
		digi_read_another_chunk();
		if (global[g009])
			midi_loop();

		long current_time = timer_read();

		buffer_rect_copy(scr_inter_orig, scr_inter, 0, 0, 165, 44);

		if (mode == 1) {
			if (count != 0) {
				int x = count * 2 + 43;
				int clip_x = 100 - count * 9;
				sprite_draw_clipped(series_list[0], 1, clip_x, &scr_inter, x, 30);
			} else {
				sprite_draw(series_list[0], 3, &scr_inter, 1, 8);
			}
		} else {
			sprite_draw(series_list[0], 3, &scr_inter, 1, 8);
		}

		if (mode == 2) {
			if (count != 0) {
				int x = count * 2 + 37;
				int clip_x = 100 - count * 9;
				sprite_draw_clipped(series_list[1], 1, clip_x, &scr_inter, x, 96);
			} else {
				sprite_draw(series_list[1], 1, &scr_inter, 1, 68);
			}
		} else {
			sprite_draw(series_list[1], 1, &scr_inter, 1, 68);
		}

		buffer_rect_copy_2(scr_inter, scr_main, 0, 0, 0, 156, 165, 44);
		buffer_rect_copy(scr_inter_orig, scr_inter, 168, 0, 152, 44);

		if (mode == 3)
			count = 0;

		if (global[walker_converse_state])
			sprite_draw(series_list[5], 4, &scr_inter, 1, 194);
		else
			sprite_draw(series_list[2], 4, &scr_inter, 1, 194);

		sprite_draw(series_list[3], 2, &scr_inter, 1, 264);

		buffer_rect_copy_2(scr_inter, scr_main, 168, 0, 168, 156, 152, 44);

		while (timer_read() - current_time < 2) {}

		count--;
	} while (count >= 0);

	mouse_show();
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
