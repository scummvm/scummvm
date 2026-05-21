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
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/room454.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];
	int16 dune_3_base;
	int16 dyn_dune_3;
	int16 dune_4_base;
	int16 dyn_dune_4;
	int16 mount_left_base;
	int16 dyn_mount_left;
	int16 mount_right_base;
	int16 dyn_mount_right;
	int16 king_frame;
	int16 anim_0_running;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_dune_3       0
#define fx_dune_4       1
#define fx_oasis        2
#define fx_mount_left   3
#define fx_mount_right  4
#define fx_cloud_left   5
#define fx_cloud_right  6

#define camera_ratio_1  1
#define camera_ratio_2  2

#define WALK_1_X        645
#define WALK_1_Y        130
#define WALK_2_X        202
#define WALK_2_Y        130
#define WALK_3_X        121
#define WALK_3_Y        112
#define WALK_4_X          1
#define WALK_4_Y        104

static Scratch scratch;


static void set_dune_3_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_dune_3] >= 0) {
		kernel_seq_delete(seq[fx_dune_3]);
	}

	difference = center - local->dune_3_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->dune_3_base + displace - 1;
	xs         = series_list[ss[fx_dune_3]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_dune_3] = -1;
	} else {
		seq[fx_dune_3] = kernel_seq_stamp(ss[fx_dune_3], false, 1);
		kernel_seq_loc(seq[fx_dune_3], x, 155);
		kernel_seq_depth(seq[fx_dune_3], 1);
	}
}

static void set_dune_4_position() {
	int center;
	int difference;
	int direction;
	int distance;
	int displace;
	int x;
	int xs;

	center = picture_view_x + (video_x >> 1);

	if (seq[fx_dune_4] >= 0) {
		kernel_seq_delete(seq[fx_dune_4]);
	}

	difference = center - local->dune_4_base;
	direction  = neg(sgn(difference));
	distance   = abs(difference);

	displace   = (int)(((long)distance * camera_ratio_1) / camera_ratio_2);
	displace   = sgn_in(displace, direction);

	x          = local->dune_4_base + displace - 1;
	xs         = series_list[ss[fx_dune_4]]->index[0].xs;

	if (((x - ((xs >> 1) + 1)) >= (picture_view_x + video_x)) ||
	    ((x + ((xs >> 1) + 1)) < picture_view_x)) {
		seq[fx_dune_4] = -1;
	} else {
		seq[fx_dune_4] = kernel_seq_stamp(ss[fx_dune_4], false, 1);
		kernel_seq_loc(seq[fx_dune_4], x, 155);
		kernel_seq_depth(seq[fx_dune_4], 1);
	}
}

static void set_454_mount_left_position() {
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

static void set_454_mount_right_position() {
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

static void set_454_cloud_left_position() {
	int difference;
	int x;

	difference = ((280 - picture_view_x) * 10) / 11;
	x = series_list[ss[fx_cloud_left]]->index[0].x - difference;
	x += -38;

	kernel_seq_delete(seq[fx_cloud_left]);
	seq[fx_cloud_left] = kernel_seq_stamp(ss[fx_cloud_left], false, 1);
	kernel_seq_loc(seq[fx_cloud_left], x, 70);
	kernel_seq_depth(seq[fx_cloud_left], 15);
}

static void set_454_cloud_right_position() {
	int difference;
	int x;

	difference = ((280 - picture_view_x) * 10) / 11;
	x = series_list[ss[fx_cloud_right]]->index[0].x - difference;
	x += 280;

	kernel_seq_delete(seq[fx_cloud_right]);
	seq[fx_cloud_right] = kernel_seq_stamp(ss[fx_cloud_right], false, 1);
	kernel_seq_loc(seq[fx_cloud_right], x, 75);
	kernel_seq_depth(seq[fx_cloud_right], 3);
}

static void handle_animation_king() {
	int king_reset_frame;

	if (kernel_anim[aa[0]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[0]].frame;
		king_reset_frame = -1;

		switch (local->king_frame) {

		case 210:
			camera_pan_to(&camera_x, 0);
			break;

		case 498:
			camera_x.pan_mode      = CAMERA_PLAYER;
			global[from_direction] = FROM_EAST;
			global[no_load_walker] = false;
			new_room               = 405;
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[0], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}

static void room_454_init() {
	global[perform_displacements] = true;

	player.commands_allowed = false;
	player.walker_visible   = false;

	camera_x.pan_mode = CAMERA_MANUAL;
	camera_jump_to(280, 0);

	local->anim_0_running = false;

	ss[fx_mount_left]  = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_mount_right] = kernel_load_series(kernel_name('x', 1), false);

	ss[fx_cloud_left]  = kernel_load_series(kernel_name('c', 0), false);
	ss[fx_cloud_right] = kernel_load_series(kernel_name('c', 1), false);

	if (previous_room != KERNEL_RESTORING_GAME) {
		ss[fx_dune_3] = kernel_load_series(kernel_name('x', 3), false);
		ss[fx_dune_4] = kernel_load_series(kernel_name('x', 2), false);
		ss[fx_oasis]  = kernel_load_series(kernel_name('x', 4), false);

		seq[fx_oasis] = kernel_seq_stamp(ss[fx_oasis], false, 1);
		kernel_seq_depth(seq[fx_oasis], 8);

		seq[fx_dune_3]     = -1;
		local->dyn_dune_3  = -1;
		local->dune_3_base = 110;

		seq[fx_dune_4]     = -1;
		local->dyn_dune_4  = -1;
		local->dune_4_base = 505;

		set_dune_3_position();
		set_dune_4_position();

		aa[0]                 = kernel_run_animation(kernel_name('k', 1), 0);
		kernel_reset_animation(aa[0], 21);
		local->anim_0_running = true;
	}

	seq[fx_mount_left] = kernel_seq_stamp(ss[fx_mount_left], false, 1);
	kernel_seq_loc(seq[fx_mount_left], 270, 85);
	kernel_seq_depth(seq[fx_mount_left], 12);

	seq[fx_mount_right] = kernel_seq_stamp(ss[fx_mount_right], false, 1);
	kernel_seq_loc(seq[fx_mount_right], 480, 85);
	kernel_seq_depth(seq[fx_mount_right], 12);

	seq[fx_cloud_left] = kernel_seq_stamp(ss[fx_cloud_left], false, 1);
	kernel_seq_loc(seq[fx_cloud_left], 258, 70);
	kernel_seq_depth(seq[fx_cloud_left], 15);

	seq[fx_cloud_right] = kernel_seq_stamp(ss[fx_cloud_right], false, 1);
	kernel_seq_loc(seq[fx_cloud_right], 441, 75);
	kernel_seq_depth(seq[fx_cloud_right], 3);

	viewing_at_y = ((video_y - display_y) >> 1);

	section_4_music();
}

static void room_454_daemon() {
	if (local->anim_0_running) {
		handle_animation_king();
	}

	if (camera_x.pan_this_frame) {
		set_454_mount_left_position();
		set_454_mount_right_position();
		set_454_cloud_left_position();
		set_454_cloud_right_position();

		set_dune_3_position();
		set_dune_4_position();
	}
}

static void room_454_pre_parser() {
}

static void room_454_parser() {
}

void room_454_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.dune_3_base);
	s.syncAsSint16LE(scratch.dyn_dune_3);
	s.syncAsSint16LE(scratch.dune_4_base);
	s.syncAsSint16LE(scratch.dyn_dune_4);
	s.syncAsSint16LE(scratch.mount_left_base);
	s.syncAsSint16LE(scratch.dyn_mount_left);
	s.syncAsSint16LE(scratch.mount_right_base);
	s.syncAsSint16LE(scratch.dyn_mount_right);
	s.syncAsSint16LE(scratch.king_frame);
	s.syncAsSint16LE(scratch.anim_0_running);
}

void room_454_preload() {
	room_init_code_pointer       = room_454_init;
	room_pre_parser_code_pointer = room_454_pre_parser;
	room_parser_code_pointer     = room_454_parser;
	room_daemon_code_pointer     = room_454_daemon;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
