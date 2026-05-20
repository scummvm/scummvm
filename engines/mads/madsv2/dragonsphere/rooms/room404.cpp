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

#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/room404.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];
	int16 dune_1_base;
	int16 dyn_dune_1;
	int16 dune_2_base;
	int16 dyn_dune_2;
	int16 mount_left_base;
	int16 dyn_mount_left;
	int16 mount_right_base;
	int16 dyn_mount_right;
	int16 king_frame;
	int16 anim_0_running;
	int16 death_frame;
	int16 anim_1_running;
	int16 num;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_dune_1               0
#define fx_dune_2               1
#define fx_lean_to              2
#define fx_mount_left           3
#define fx_mount_right          4
#define fx_cloud_left           5
#define fx_cloud_right          6

#define MUSIC                   60

#define camera_ratio_1          1
#define camera_ratio_2          2

#define WALK_3_X                121
#define WALK_3_Y                112
#define WALK_4_X                1
#define WALK_4_Y                104

static Scratch scratch;


static void set_dune_1_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int y;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_dune_1] >= 0) {
		kernel_seq_delete(seq[fx_dune_1]);
	}

	difference = center - local->dune_1_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->dune_1_base + displace - 1;
	y          = series_list[ss[fx_dune_1]]->index[0].ys + 114;
	xs         = series_list[ss[fx_dune_1]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_dune_1] = -1;
	} else {
		seq[fx_dune_1] = kernel_seq_stamp(ss[fx_dune_1], false, 1);
		kernel_seq_loc(seq[fx_dune_1], x, y);
		kernel_seq_depth(seq[fx_dune_1], 1);
	}
}

static void set_dune_2_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int y;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_dune_2] >= 0) {
		kernel_seq_delete(seq[fx_dune_2]);
	}

	difference = center - local->dune_2_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->dune_2_base + displace - 1;
	y          = series_list[ss[fx_dune_2]]->index[0].ys + 126;
	xs         = series_list[ss[fx_dune_2]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_dune_2] = -1;
	} else {
		seq[fx_dune_2] = kernel_seq_stamp(ss[fx_dune_2], false, 1);
		kernel_seq_loc(seq[fx_dune_2], x, y);
		kernel_seq_depth(seq[fx_dune_2], 1);
	}
}

static void set_404_mount_left_position() {
	int difference;
	int x;

	difference = ((280 - picture_view_x) * 4) / 5;
	x = series_list[ss[fx_mount_left]]->index[0].x - difference;
	x += 1;

	kernel_seq_delete(seq[fx_mount_left]);
	seq[fx_mount_left] = kernel_seq_stamp(ss[fx_mount_left], false, 1);
	kernel_seq_loc(seq[fx_mount_left], x, 85);
	kernel_seq_depth(seq[fx_mount_left], 12);
}

static void set_404_mount_right_position() {
	int difference;
	int x;

	difference = ((280 - picture_view_x) * 4) / 5;
	x = series_list[ss[fx_mount_right]]->index[0].x - difference;
	x += 320;

	kernel_seq_delete(seq[fx_mount_right]);
	seq[fx_mount_right] = kernel_seq_stamp(ss[fx_mount_right], false, 1);
	kernel_seq_loc(seq[fx_mount_right], x, 85);
	kernel_seq_depth(seq[fx_mount_right], 12);
}

static void set_404_cloud_left_position() {
	int difference;
	int x;

	difference = ((280 - picture_view_x) * 10) / 11;
	x = series_list[ss[fx_cloud_left]]->index[0].x - difference;
	x += -38;

	kernel_seq_delete(seq[fx_cloud_left]);
	seq[fx_cloud_left] = kernel_seq_stamp(ss[fx_cloud_left], false, 1);
	kernel_seq_loc(seq[fx_cloud_left], x, 80);
	kernel_seq_depth(seq[fx_cloud_left], 15);
}

static void set_404_cloud_right_position() {
	int difference;
	int x;

	difference = ((280 - picture_view_x) * 10) / 11;
	x = series_list[ss[fx_cloud_right]]->index[0].x - difference;
	x += 280;

	kernel_seq_delete(seq[fx_cloud_right]);
	seq[fx_cloud_right] = kernel_seq_stamp(ss[fx_cloud_right], false, 1);
	kernel_seq_loc(seq[fx_cloud_right], x, 82);
	kernel_seq_depth(seq[fx_cloud_right], 3);
}

static void handle_animation_king() {
	int king_reset_frame;

	if (kernel_anim[aa[0]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[0]].frame;
		king_reset_frame = -1;

		switch (local->king_frame) {

		case 132:
			king_reset_frame = 463;
			break;

		case 475:
			king_reset_frame = 132;
			break;

		case 245:
			camera_pan_to(&camera_x, 0);
			break;

		case 463:
			if (global[talked_to_soptus]) {
				text_show(40401);
			} else {
				text_show(40402);
			}
			text_show(40403);
			camera_x.pan_mode      = CAMERA_PLAYER;
			global[from_direction] = FROM_EAST;
			global[no_load_walker] = false;
			new_room               = 401;
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[0], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}

static void handle_animation_death() {
	int death_reset_frame;

	if (kernel_anim[aa[1]].frame != local->death_frame) {
		local->death_frame = kernel_anim[aa[1]].frame;
		death_reset_frame = -1;

		switch (local->death_frame) {
		case 149:
			++ local->num;
			if (local->num < 4) {
				death_reset_frame = 147;
			}
			break;
		}

		if (death_reset_frame >= 0) {
			kernel_reset_animation(aa[1], death_reset_frame);
			local->death_frame = death_reset_frame;
		}
	}
}

static void room_404_init() {
	global[perform_displacements] = true;

	player.commands_allowed = false;
	player.walker_visible = false;

	camera_x.pan_mode = CAMERA_MANUAL;
	camera_jump_to(280, 0);

	local->anim_0_running = false;
	local->anim_1_running = false;
	local->num = 0;

	ss[fx_mount_left] = kernel_load_series(kernel_name('x', 6), false);
	ss[fx_mount_right] = kernel_load_series(kernel_name('x', 7), false);

	ss[fx_cloud_left] = kernel_load_series(kernel_name('c', 0), false);
	ss[fx_cloud_right] = kernel_load_series(kernel_name('c', 1), false);

	if (global[desert_counter] == 6) {
		aa[1] = kernel_run_animation(kernel_name('d', 1), 1);
		local->anim_1_running = true;
		kernel_reset_animation(aa[25], 30);

	} else if (previous_room == 120 || previous_room != KERNEL_RESTORING_GAME) {
		ss[fx_dune_1] = kernel_load_series(kernel_name('x', 0), false);
		ss[fx_dune_2] = kernel_load_series(kernel_name('x', 3), false);
		ss[fx_lean_to] = kernel_load_series(kernel_name('x', 1), false);

		aa[0] = kernel_run_animation(kernel_name('w', 1), 0);
		local->anim_0_running = true;
		kernel_reset_animation(aa[0], 30);

		seq[fx_lean_to] = kernel_seq_stamp(ss[fx_lean_to], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_lean_to], 4);

		seq[fx_dune_1] = -1;
		local->dyn_dune_1 = -1;
		local->dune_1_base = 84;

		seq[fx_dune_2] = -1;
		local->dyn_dune_2 = -1;
		local->dune_2_base = 500;

		set_dune_1_position();
		set_dune_2_position();
	}

	seq[fx_mount_left] = kernel_seq_stamp(ss[fx_mount_left], false, 1);
	kernel_seq_loc(seq[fx_mount_left], 270, 85);
	kernel_seq_depth(seq[fx_mount_left], 12);

	seq[fx_mount_right] = kernel_seq_stamp(ss[fx_mount_right], false, 1);
	kernel_seq_loc(seq[fx_mount_right], 480, 85);
	kernel_seq_depth(seq[fx_mount_right], 12);

	seq[fx_cloud_left] = kernel_seq_stamp(ss[fx_cloud_left], false, 1);
	kernel_seq_loc(seq[fx_cloud_left], 258, 80);
	kernel_seq_depth(seq[fx_cloud_left], 15);

	seq[fx_cloud_right] = kernel_seq_stamp(ss[fx_cloud_right], false, 1);
	kernel_seq_loc(seq[fx_cloud_right], 441, 82);
	kernel_seq_depth(seq[fx_cloud_right], 3);

	viewing_at_y = ((video_y - display_y) >> 1);

	if (previous_room == 120) {
		kernel_timing_trigger(10, MUSIC);
	}

	section_4_music();
}

static void room_404_daemon() {
	if (local->anim_0_running) {
		handle_animation_king();
	}

	if (local->anim_1_running) {
		handle_animation_death();
	}

	if (camera_x.pan_this_frame) {
		set_404_mount_left_position();
		set_404_mount_right_position();
		set_404_cloud_left_position();
		set_404_cloud_right_position();
		set_dune_1_position();
		set_dune_2_position();
	}

	if (kernel.trigger == 1) {
		text_show(40404);
		global[desert_room]    = 42;
		new_room               = 401;
		global[no_load_walker] = false;
	}

	if (kernel.trigger == MUSIC) {
		sound_play(N_Bk404Music);
	}
}

static void room_404_pre_parser() {
}

static void room_404_parser() {
}

void room_404_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.dune_1_base);
	s.syncAsSint16LE(scratch.dyn_dune_1);
	s.syncAsSint16LE(scratch.dune_2_base);
	s.syncAsSint16LE(scratch.dyn_dune_2);
	s.syncAsSint16LE(scratch.mount_left_base);
	s.syncAsSint16LE(scratch.dyn_mount_left);
	s.syncAsSint16LE(scratch.mount_right_base);
	s.syncAsSint16LE(scratch.dyn_mount_right);
	s.syncAsSint16LE(scratch.king_frame);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.death_frame);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.num);
}

void room_404_preload() {
	room_init_code_pointer       = room_404_init;
	room_pre_parser_code_pointer = room_404_pre_parser;
	room_parser_code_pointer     = room_404_parser;
	room_daemon_code_pointer     = room_404_daemon;

	global[no_load_walker] = true;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
