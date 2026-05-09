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
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room103.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];    // ss[] — series handles
	int16 sequence[15];  // seq[] — sequence handles
	int16 animation[4];  // aa[]  — animation handles (unused in room 103)
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

/* ========================= Sprite Series =================== */

#define fx_candle_flame_1   1  /* rm103y0 */
#define fx_fire_sconce_1    2  /* rm103y1 */
#define fx_fire_sconce_2    3  /* rm103y2 */
#define fx_candle_flame_2   4  /* rm103y3 */
#define fx_candle_flame_3   5  /* rm103y4 */
#define fx_candle_flame_4   6  /* rm103y5 */
#define fx_candle_flame_5   7  /* rm103y6 */
#define fx_fire_sconce_3    8  /* rm103y7 */
#define fx_door_101         9  /* rm103x0 */
#define fx_door_102        10  /* rm103x1 */
#define fx_open_door       11  /* kgrd_8; aKgrd6+4 in disasm — TODO: confirm string */

/* ========================== Triggers ======================= */

#define ROOM_103_DOOR_CLOSES  70

/* Walk points */
#define START_X_ROOM_101      173
#define START_Y_ROOM_101      108
#define START_X_ROOM_102      471
#define START_Y_ROOM_102      108
#define START_X_ROOM_104      130
#define START_Y_ROOM_104      152
#define START_X_ROOM_105      517
#define START_Y_ROOM_105      152

#define WALK_TO_X_FROM_101    162
#define WALK_TO_Y_FROM_101    127
#define WALK_TO_X_FROM_102    482
#define WALK_TO_Y_FROM_102    128

/* Cursor points */
#define END_HALL_TAPESTRY_X    75
#define END_HALL_TAPESTRY_Y   130

/* Camera defines */
#define RIGHT_HALLWAY         320


static Scratch scratch;

void room_103_init() {
	kernel.disable_fastwalk = true;

	// Load series in disassembly order: y0..y7, x0, x1, open_door
	ss[fx_candle_flame_1] = kernel_load_series(kernel_name('y', 0), 0);
	ss[fx_fire_sconce_1]  = kernel_load_series(kernel_name('y', 1), 0);
	ss[fx_fire_sconce_2]  = kernel_load_series(kernel_name('y', 2), 0);
	ss[fx_candle_flame_2] = kernel_load_series(kernel_name('y', 3), 0);
	ss[fx_candle_flame_3] = kernel_load_series(kernel_name('y', 4), 0);
	ss[fx_candle_flame_4] = kernel_load_series(kernel_name('y', 5), 0);
	ss[fx_candle_flame_5] = kernel_load_series(kernel_name('y', 6), 0);
	ss[fx_fire_sconce_3]  = kernel_load_series(kernel_name('y', 7), 0);
	ss[fx_door_101]       = kernel_load_series(kernel_name('x', 0), 0);
	ss[fx_door_102]       = kernel_load_series(kernel_name('x', 1), 0);
	ss[fx_open_door]      = kernel_load_series("kgrd_8", 0);

	// Start ambient loops in disassembly order: y0..y7 with phase offsets
	seq[fx_candle_flame_1] = kernel_seq_forward(ss[fx_candle_flame_1], false, 7, 0, 0, 0);
	seq[fx_fire_sconce_1]  = kernel_seq_forward(ss[fx_fire_sconce_1],  false, 7, 0, 0, 0);
	seq[fx_fire_sconce_2]  = kernel_seq_forward(ss[fx_fire_sconce_2],  false, 7, 4, 0, 0);
	seq[fx_candle_flame_2] = kernel_seq_forward(ss[fx_candle_flame_2], false, 7, 3, 0, 0);
	seq[fx_candle_flame_3] = kernel_seq_forward(ss[fx_candle_flame_3], false, 7, 2, 0, 0);
	seq[fx_candle_flame_4] = kernel_seq_forward(ss[fx_candle_flame_4], false, 7, 0, 0, 0);
	seq[fx_candle_flame_5] = kernel_seq_forward(ss[fx_candle_flame_5], false, 7, 5, 0, 0);
	seq[fx_fire_sconce_3]  = kernel_seq_forward(ss[fx_fire_sconce_3],  false, 7, 0, 0, 0);

	if (previous_room == 104 || previous_room == 105) {
		seq[fx_door_101] = kernel_seq_stamp(ss[fx_door_101], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_101], 8);
		seq[fx_door_102] = kernel_seq_stamp(ss[fx_door_102], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_102], 8);
	}

	if (previous_room == 102) {
		seq[fx_door_101] = kernel_seq_stamp(ss[fx_door_101], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_101], 6);
		seq[fx_door_102] = kernel_seq_stamp(ss[fx_door_102], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door_102], 14);
		camera_jump_to(RIGHT_HALLWAY, 0);
		player_walk(WALK_TO_X_FROM_102, WALK_TO_Y_FROM_102, FACING_SOUTH);
		player_first_walk(START_X_ROOM_102, START_Y_ROOM_102, FACING_SOUTH,
		                  WALK_TO_X_FROM_102, WALK_TO_Y_FROM_102, FACING_SOUTH, false);
		player_walk_trigger(ROOM_103_DOOR_CLOSES + 2);
	} else if (previous_room == 104) {
		player.x      = START_X_ROOM_104;
		player.y      = START_Y_ROOM_104;
		player.facing = FACING_NORTHEAST;
	} else if (previous_room == 105) {
		player.x      = START_X_ROOM_105;
		player.y      = START_Y_ROOM_105;
		player.facing = FACING_NORTHWEST;
		camera_jump_to(RIGHT_HALLWAY, 0);
	} else if (previous_room == 101 || previous_room != KERNEL_RESTORING_GAME) {
		seq[fx_door_102] = kernel_seq_stamp(ss[fx_door_102], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_102], 6);
		seq[fx_door_101] = kernel_seq_stamp(ss[fx_door_101], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door_101], 14);
		player_first_walk(START_X_ROOM_101, START_Y_ROOM_101, FACING_SOUTH,
		                  WALK_TO_X_FROM_101, WALK_TO_Y_FROM_101, FACING_SOUTH, false);
		player_walk_trigger(ROOM_103_DOOR_CLOSES);
	} else {
		seq[fx_door_102] = kernel_seq_stamp(ss[fx_door_102], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_102], 6);
		seq[fx_door_101] = kernel_seq_stamp(ss[fx_door_101], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_101], 6);
	}

	section_1_music();
}

void room_103_daemon() {
	int16 temp;

	switch (kernel.trigger) {
	case ROOM_103_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_door_101]);
		sound_play(N_DoorCloses);
		seq[fx_door_101] = kernel_seq_backward(ss[fx_door_101], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_door_101], 6);
		kernel_seq_range(seq[fx_door_101], 1, 3);
		kernel_seq_trigger(seq[fx_door_101], KERNEL_TRIGGER_EXPIRE, 0, ROOM_103_DOOR_CLOSES + 1);
		break;

	case ROOM_103_DOOR_CLOSES + 1:
		temp             = seq[fx_door_101];
		seq[fx_door_101] = kernel_seq_stamp(ss[fx_door_101], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_101], 6);
		kernel_synch(KERNEL_SERIES, seq[fx_door_101], KERNEL_SERIES, temp);
		player.commands_allowed = true;
		break;

	case ROOM_103_DOOR_CLOSES + 2:
		kernel_seq_delete(seq[fx_door_102]);
		sound_play(N_DoorCloses);
		seq[fx_door_102] = kernel_seq_backward(ss[fx_door_102], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_door_102], 6);
		kernel_seq_range(seq[fx_door_102], 1, 3);
		kernel_seq_trigger(seq[fx_door_102], KERNEL_TRIGGER_EXPIRE, 0, ROOM_103_DOOR_CLOSES + 3);
		break;

	case ROOM_103_DOOR_CLOSES + 3:
		temp             = seq[fx_door_102];
		seq[fx_door_102] = kernel_seq_stamp(ss[fx_door_102], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door_102], 6);
		kernel_synch(KERNEL_SERIES, seq[fx_door_102], KERNEL_SERIES, temp);
		player.commands_allowed = true;
		break;
	}
}

void room_103_pre_parser() {
}

void room_103_parser() {
	int16 temp;

	if (player.look_around) {
		text_show(10301);
		player.command_ready = false;
		return;
	}

	if (player_parse(37, 245, 0) || player_parse(6, 245, 0) || player_parse(10, 245, 0)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_open_door]       = kernel_seq_pingpong(ss[fx_open_door], false, 7, 0, 0, 2);
			kernel_seq_player(seq[fx_open_door], true);
			kernel_seq_trigger(seq[fx_open_door], KERNEL_TRIGGER_SPRITE, 2, 1);
			kernel_seq_trigger(seq[fx_open_door], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			kernel_seq_delete(seq[fx_door_101]);
			sound_play(N_DoorOpens);
			seq[fx_door_101] = kernel_seq_forward(ss[fx_door_101], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_101], 8);
			kernel_seq_trigger(seq[fx_door_101], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			temp             = seq[fx_door_101];
			seq[fx_door_101] = kernel_seq_stamp(ss[fx_door_101], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_door_101], 14);
			kernel_synch(KERNEL_SERIES, seq[fx_door_101], KERNEL_SERIES, temp);
			break;

		case 3:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_open_door]);
			kernel_timing_trigger(1, 4);
			break;

		case 4:
			player_walk(START_X_ROOM_101, START_Y_ROOM_101, FACING_NORTH);
			player_walk_trigger(5);
			break;

		case 5:
			kernel_seq_delete(seq[fx_door_101]);
			sound_play(N_DoorCloses);
			seq[fx_door_101] = kernel_seq_backward(ss[fx_door_101], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_101], 1);
			kernel_seq_trigger(seq[fx_door_101], KERNEL_TRIGGER_EXPIRE, 0, 6);
			break;

		case 6:
			temp             = seq[fx_door_101];
			seq[fx_door_101] = kernel_seq_stamp(ss[fx_door_101], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door_101], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_door_101], KERNEL_SERIES, temp);
			kernel_timing_trigger(6, 7);
			break;

		case 7:
			new_room = 101;
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_parse(37, 36, 0) || player_parse(6, 36, 0) || player_parse(10, 36, 0)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_open_door]       = kernel_seq_pingpong(ss[fx_open_door], true, 7, 0, 0, 2);
			kernel_seq_player(seq[fx_open_door], true);
			kernel_seq_trigger(seq[fx_open_door], KERNEL_TRIGGER_SPRITE, 2, 1);
			kernel_seq_trigger(seq[fx_open_door], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			kernel_seq_delete(seq[fx_door_102]);
			sound_play(N_DoorOpens);
			seq[fx_door_102] = kernel_seq_forward(ss[fx_door_102], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_102], 8);
			kernel_seq_trigger(seq[fx_door_102], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			temp             = seq[fx_door_102];
			seq[fx_door_102] = kernel_seq_stamp(ss[fx_door_102], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_door_102], 14);
			kernel_synch(KERNEL_SERIES, seq[fx_door_102], KERNEL_SERIES, temp);
			break;

		case 3:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_open_door]);
			kernel_timing_trigger(1, 4);
			break;

		case 4:
			player_walk(START_X_ROOM_102, START_Y_ROOM_102, FACING_NORTH);
			player_walk_trigger(5);
			break;

		case 5:
			kernel_seq_delete(seq[fx_door_102]);
			sound_play(N_DoorCloses);
			seq[fx_door_102] = kernel_seq_backward(ss[fx_door_102], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_102], 1);
			kernel_seq_trigger(seq[fx_door_102], KERNEL_TRIGGER_EXPIRE, 0, 6);
			break;

		case 6:
			temp             = seq[fx_door_102];
			seq[fx_door_102] = kernel_seq_stamp(ss[fx_door_102], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door_102], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_door_102], KERNEL_SERIES, temp);
			kernel_timing_trigger(6, 7);
			break;

		case 7:
			new_room = 102;
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_parse(37, 249, 0) || player_parse(6, 249, 0) || player_parse(10, 249, 0)) {
		new_room = 104;
		player.command_ready = false;
		return;
	}

	if (player_parse(37, 250, 0) || player_parse(6, 250, 0) || player_parse(10, 250, 0)) {
		new_room = 105;
		player.command_ready = false;
		return;
	}

	if (player_parse(3, 0) || player_parse(30, 0)) {
		if (player_parse(26, 0)) {
			if (inter_point_x <= END_HALL_TAPESTRY_X && inter_point_y <= END_HALL_TAPESTRY_Y)
				text_show(10302);
			else
				text_show(10303);
			player.command_ready = false;
			return;
		}
		if (player_parse(246, 0)) { text_show(10305); player.command_ready = false; return; }
		if (player_parse(36, 0))  { text_show(10307); player.command_ready = false; return; }
		if (player_parse(245, 0)) { text_show(10308); player.command_ready = false; return; }
		if (player_parse(570, 0)) { text_show(10309); player.command_ready = false; return; }
		if (player_parse(198, 0)) { text_show(10311); player.command_ready = false; return; }
		if (player_parse(248, 0)) { text_show(10312); player.command_ready = false; return; }
		if (player_parse(247, 0)) { text_show(10314); player.command_ready = false; return; }
		if (player_parse(244, 0)) { text_show(10315); player.command_ready = false; return; }
		if (player_parse(44, 0))  { text_show(10317); player.command_ready = false; return; }
		if (player_parse(41, 0))  { text_show(10320); player.command_ready = false; return; }
		if (player_parse(40, 0))  { text_show(10322); player.command_ready = false; return; }
		if (player_parse(250, 0)) { text_show(10323); player.command_ready = false; return; }
		if (player_parse(249, 0)) { text_show(10324); player.command_ready = false; return; }
	}

	if ((player_parse(4, 0) || player_parse(10, 0)) && player_parse(244, 0)) {
		text_show(10316);
		player.command_ready = false;
		return;
	}

	if ((player_parse(5, 0) || player_parse(10, 0)) && player_parse(26, 0)) {
		text_show(10304);
		player.command_ready = false;
		return;
	}

	if (player_parse(10, 246, 0)) {
		text_show(10306);
		player.command_ready = false;
		return;
	}

	if (player_parse(6, 570, 0)) {
		text_show(10310);
		player.command_ready = false;
		return;
	}

	if (player_parse(6, 248, 0) || player_parse(6, 247, 0)) {
		text_show(10313);
		player.command_ready = false;
		return;
	}
}

void room_103_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
}

void room_103_preload() {
	room_init_code_pointer       = room_103_init;
	room_pre_parser_code_pointer = room_103_pre_parser;
	room_parser_code_pointer     = room_103_parser;
	room_daemon_code_pointer     = room_103_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
