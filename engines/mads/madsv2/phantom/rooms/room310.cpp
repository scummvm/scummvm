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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/phantom/mads/quotes.h"
#include "mads/madsv2/phantom/rooms/section3.h"
#include "mads/madsv2/phantom/rooms/room310.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_310_init() {
	viewing_at_y = ((video_y - display_y) >> 1);

	for (int count = 0; count < NUM_MULTIPLANES; count++) {
		ss[fx_multiplane_0 + count] = kernel_load_series(kernel_name('f', count), false);
		seq[fx_multiplane_0 + count] = -1;
	}


	local->multiplane_base[0] = 100;
	local->multiplane_base[1] = 210;
	local->multiplane_base[2] = 320;
	local->multiplane_base[3] = 472;


	kernel.quotes = quote_load(quote_310a0, quote_310a1, quote_310b0,
		quote_310b1, quote_310c0, quote_310c1,
		quote_310d0, quote_310d1, quote_310e0,
		quote_310e1, quote_310f0, quote_310d2,
		quote_310a2, quote_310e2, quote_310b2, 0);

	player.commands_allowed = false;
	player.walker_visible = false;
	aa[0] = kernel_run_animation(kernel_name('l', 1), ROOM_310_DONE);
	kernel_anim[aa[0]].view_changes = true;
	camera_x.pan_mode = CAMERA_MANUAL;

	local->message_1 = (1 << 8) + 2;
	local->message_2 = MESSAGE_COLOR;

	kernel_init_dialog();  /* clear interface */
	kernel_set_interface_mode(INTER_CONVERSATION);

	section_3_music();
}

void set_multiplane_positions_310(int x_new) {
	int multiplane;
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int y;
	int xs;

	center = x_new + (video_x >> 1);

	for (multiplane = 0; multiplane < NUM_MULTIPLANES; multiplane++) {

		if (seq[multiplane + fx_multiplane_0] >= 0) {
			kernel_seq_delete(seq[multiplane + fx_multiplane_0]);
		}

		difference = center - local->multiplane_base[multiplane];
		direction = neg(sgn(difference));
		distance = abs(difference);

		displace = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
		displace = sgn_in(displace, direction);

		x = local->multiplane_base[multiplane] + displace - 1;
		y = series_list[ss[fx_multiplane_0 + multiplane]]->index[0].ys - 1 + 30;
		xs = series_list[ss[fx_multiplane_0 + multiplane]]->index[0].xs;

		if (((x - ((xs >> 1) + 1)) >= (x_new + video_x)) ||
			((x + ((xs >> 1) + 1)) < x_new)) {
			seq[multiplane + fx_multiplane_0] = -1;
		} else {
			/*
			if (local->dyn_multiplanes[multiplane] != -1) {
		  kernel_delete_dynamic (local->dyn_multiplanes[multiplane]);
			}

			local->dyn_multiplanes[multiplane] = kernel_add_dynamic (words_multiplane, words_look_at, SYNTAX_SINGULAR, KERNEL_NONE,
			  x - 8, y - 12, 16, 13);
			*/

			seq[multiplane + fx_multiplane_0] =
				kernel_seq_stamp(ss[fx_multiplane_0 + multiplane], false, 1);
			kernel_seq_loc(seq[multiplane + fx_multiplane_0], x, y);
			kernel_seq_depth(seq[multiplane + fx_multiplane_0], 1);
		}
	}
}

static void handle_animation_lake() {
	int lake_reset_frame;
	int id;

	if (kernel_anim[aa[0]].frame != local->lake_frame) {
		local->lake_frame = kernel_anim[aa[0]].frame;
		lake_reset_frame = -1;

		switch (local->lake_frame) {
		case 60:
			id = kernel_message_add(quote_string(kernel.quotes, quote_310a0),
				-142, 0, local->message_2, TEN_SECONDS, ROOM_310_CONV + 1, 0);
			kernel_message_anim(id, aa[0], 0);
			id = kernel_message_add(quote_string(kernel.quotes, quote_310a1),
				-142, 15, local->message_2, TEN_SECONDS, 0, 0);
			kernel_message_anim(id, aa[0], 0);
			id = kernel_message_add(quote_string(kernel.quotes, quote_310a2),
				-142, 30, local->message_2, TEN_SECONDS, 0, 0);
			kernel_message_anim(id, aa[0], 0);
			break;

		case 120:
			kernel_message_purge();
			break;

		case 140:
			id = kernel_message_add(quote_string(kernel.quotes, quote_310b0),
				-120, 0, local->message_2, SIX_SECONDS, ROOM_310_CONV + 3, 0);
			kernel_message_anim(id, aa[0], 0);
			id = kernel_message_add(quote_string(kernel.quotes, quote_310b1),
				-120, 15, local->message_2, SIX_SECONDS, 0, 0);
			kernel_message_anim(id, aa[0], 0);
			id = kernel_message_add(quote_string(kernel.quotes, quote_310b2),
				-120, 30, local->message_2, SIX_SECONDS, 0, 0);
			kernel_message_anim(id, aa[0], 0);
			break;

		case 200:
			kernel_message_purge();
			break;

		case 220:
			id = kernel_message_add(quote_string(kernel.quotes, quote_310c0),
				-32, 30, local->message_2, FOUR_SECONDS, ROOM_310_CONV + 5, 0);
			kernel_message_anim(id, aa[0], 0);
			id = kernel_message_add(quote_string(kernel.quotes, quote_310c1),
				-32, 45, local->message_2, FOUR_SECONDS, 0, 0);
			kernel_message_anim(id, aa[0], 0);
			break;

		case 280:
			kernel_message_purge();
			break;

		case 300:
			id = kernel_message_add(quote_string(kernel.quotes, quote_310d0),
				101, 0, local->message_1, SIX_SECONDS, ROOM_310_CONV + 7, 0);
			kernel_message_anim(id, aa[0], 0);
			id = kernel_message_add(quote_string(kernel.quotes, quote_310d1),
				101, 15, local->message_1, SIX_SECONDS, 0, 0);
			kernel_message_anim(id, aa[0], 0);
			id = kernel_message_add(quote_string(kernel.quotes, quote_310d2),
				101, 30, local->message_1, SIX_SECONDS, 0, 0);
			kernel_message_anim(id, aa[0], 0);
			break;

		case 360:
			kernel_message_purge();
			break;

		case 380:
			id = kernel_message_add(quote_string(kernel.quotes, quote_310e0),
				107, 0, local->message_2, SIX_SECONDS, ROOM_310_CONV + 9, 0);
			kernel_message_anim(id, aa[0], 0);
			id = kernel_message_add(quote_string(kernel.quotes, quote_310e1),
				107, 15, local->message_2, SIX_SECONDS, 0, 0);
			kernel_message_anim(id, aa[0], 0);
			id = kernel_message_add(quote_string(kernel.quotes, quote_310e2),
				107, 30, local->message_2, SIX_SECONDS, 0, 0);
			kernel_message_anim(id, aa[0], 0);
			break;

		case 440:
			kernel_message_purge();
			break;

		case 460:
			id = kernel_message_add(quote_string(kernel.quotes, quote_310f0),
				107, 7, local->message_2, THREE_SECONDS, 0, 0);
			kernel_message_anim(id, aa[0], 0);
			break;

		case 510:
			kernel_message_purge();
			break;
		}

		if (lake_reset_frame >= 0) {
			kernel_reset_animation(aa[0], lake_reset_frame);
			local->lake_frame = lake_reset_frame;
		}
	}
}

void room_310_daemon() {
	int frame;
	int x;
	int positions_set;
	long clock;

	handle_animation_lake();

	if (kernel.trigger == ROOM_310_DONE) {
		new_room = 309;
	}

	positions_set = false;

	if (aa[0] >= 0) {
		frame = kernel_anim[aa[0]].frame;
		clock = kernel_anim[aa[0]].next_clock;
		if (frame > 0) {
			if (kernel.clock >= clock) {
				x = kernel_anim[aa[0]].anim->frame[frame].view_x;
				if (x != picture_view_x) {
					set_multiplane_positions_310(x);
					positions_set = true;
				}
			}
		}
	}

	if (!positions_set && kernel.fx) {
		set_multiplane_positions_310(video_x);
	}
}

void room_310_preload() {
	room_init_code_pointer = room_310_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_daemon_code_pointer = room_310_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
