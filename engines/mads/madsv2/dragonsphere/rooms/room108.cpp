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

#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room108.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 eye_frame;        /* Which animation frame is being held for blinking eyes */
	int16 temp;             /* used for synching purposes */
};

/* ========================= Sprites ========================= */

#define fx_door              1   /* rm108x2 */
#define fx_fire_right        2   /* rm108x0 */
#define fx_fire_left         3   /* rm108x1 */
#define fx_eyes_0            4   /* rm108y0 */ 
#define fx_eyes_1            5   /* rm108y1 */ 
#define fx_eyes_2            6   /* rm108y2 */ 
#define fx_eyes_3            7   /* rm108y3 */ 
#define fx_eyes_4            8   /* rm108y4 */ 
#define fx_eyes_5            9   /* rm108y5 */ 
#define fx_go_downstairs     10  /* rm108a0 */
#define fx_go_upstairs       11  /* rm108a1 */
#define fx_open_door         12  /* kgdrd_8 */


/* ======================== Triggers ========================= */

#define ROOM_108_DOOR_CLOSES 70

/* walk points */
#define START_X_ROOM_107     33
#define START_Y_ROOM_107     34
#define START_X_ROOM_109     289
#define START_Y_ROOM_109     98

#define WALK_TO_X_FROM_107   50
#define WALK_TO_Y_FROM_107   40

#define OPEN_DOOR_X_ROOM_107 33
#define OPEN_DOOR_Y_ROOM_107 40

#define BOTTOM_OF_STAIRS_X   154
#define BOTTOM_OF_STAIRS_Y   89

static Scratch scratch;

static void room_108_init() {
	/* Load sprite series */
	ss[fx_door] = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_fire_right] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_fire_left] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_go_downstairs] = kernel_load_series(kernel_name('a', 0), false);
	ss[fx_go_upstairs] = kernel_load_series(kernel_name('a', 1), false);
	ss[fx_open_door] = kernel_load_series("*KGDRD_8", false);

	/* Start continuous sequences */

	seq[fx_fire_left] = kernel_seq_forward(ss[fx_fire_left], false, 7, 0, 0, 0);
	seq[fx_fire_right] = kernel_seq_forward(ss[fx_fire_right], false, 7, 3, 0, 0);

	if (previous_room == 109) {               /* Player comes from dungeon */
		player.x = START_X_ROOM_109;
		player.y = START_Y_ROOM_109;
		player.facing = FACING_SOUTHWEST;
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 13);

	} else if (previous_room != KERNEL_RESTORING_GAME) { /* Player comes from throne room rm107 */
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door], 10);
		player_first_walk(START_X_ROOM_107, START_Y_ROOM_107, FACING_SOUTH,
			WALK_TO_X_FROM_107, WALK_TO_Y_FROM_107, FACING_EAST,
			false);
		player_walk_trigger(ROOM_108_DOOR_CLOSES);

	} else {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 13);
	}

	aa[0] = kernel_run_animation(kernel_name('A', -1), 0);

	section_1_music();
}

static void get_random_eyes(int16 *new_frame) {
	int random;

	random = imath_random(0, 15);
	switch (random) {

	case 0:
		*new_frame = 0;
		break;

	case 1:
		*new_frame = 5;
		break;

	case 2:
		*new_frame = 10;
		break;

	case 3:
		*new_frame = 15;
		break;

	case 4:
		*new_frame = 20;
		break;

	case 5:
		*new_frame = 25;
		break;

	case 6:
		*new_frame = 30;
		break;

	case 7:
		*new_frame = 35;
		break;

	case 8:
		*new_frame = 40;
		break;

	case 9:
		*new_frame = 45;
		break;

	case 10:
		*new_frame = 50;
		break;

	case 11:
		*new_frame = 55;
		break;

	case 12:
		*new_frame = 60;
		break;

	case 13:
		*new_frame = 65;
		break;

	case 14:
		*new_frame = 70;
		break;

	case 15:
		*new_frame = 75;
		break;
	}
}

static void room_108_daemon() {
	int16 reset_frame;
	int16 new_frame;

	/* Randomly put different blinking eyes sequences up on screen */
	if (kernel_anim[aa[0]].anim != NULL) {
		if (kernel_anim[aa[0]].frame != local->eye_frame) {
			local->eye_frame = kernel_anim[aa[0]].frame;
			reset_frame = -1;

			get_random_eyes(&new_frame);

			switch (local->eye_frame) {
			case  5:
			case 10:
			case 15:
			case 20:
			case 25:
			case 30:
			case 35:
			case 40:
			case 45:
			case 50:
			case 55:
			case 60:
			case 65:
			case 70:
			case 75:
			case 80:
				reset_frame = new_frame;
				break;
			}

			if (reset_frame >= 0) {
				if (reset_frame != kernel_anim[aa[0]].frame) {
					kernel_reset_animation(aa[0], reset_frame);
					local->eye_frame = reset_frame;
				}
			}
		}
	}

	/* Close the door when player enters from 107 */
	if (kernel.trigger >= ROOM_108_DOOR_CLOSES) {
		switch (kernel.trigger) {
		case ROOM_108_DOOR_CLOSES:
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorCloses);
			seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
				6, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 13);
			kernel_seq_trigger(seq[fx_door],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_108_DOOR_CLOSES + 1);
			player.walker_visible = false;
			aa[1] = kernel_run_animation(kernel_name('d', 1), ROOM_108_DOOR_CLOSES + 2);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_PLAYER, 0);
			break;

		case ROOM_108_DOOR_CLOSES + 1:
			local->temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door], 13);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, local->temp);
			break;

		case ROOM_108_DOOR_CLOSES + 2:
			player.x = BOTTOM_OF_STAIRS_X;
			player.y = BOTTOM_OF_STAIRS_Y;
			player.walker_visible = true;
			player.commands_allowed = true;
			player_demand_facing(FACING_EAST);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			break;
		}
	}
}

void room_108_pre_parser(void) {
	if (player_said_1(doorway_to_cell) && player.need_to_walk) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_2(walk_through, doorway_to_cell)) {
		player.walk_off_edge_to_room = 109;
	}
}


void room_108_parser(void) {
	int temp;

	if (player.look_around) {
		text_show(10801);
		goto handled;
	}

	if (player_said_2(walk_through, door_to_council_room) || player_said_2(open, door_to_council_room) ||
		player_said_2(pull, door_to_council_room)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			aa[1] = kernel_run_animation(kernel_name('u', 1), 1);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_PLAYER, 0);
			break;

		case 1:
			player.x = WALK_TO_X_FROM_107;
			player.y = WALK_TO_Y_FROM_107;
			player_demand_facing(FACING_WEST);
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
			player_walk(OPEN_DOOR_X_ROOM_107, OPEN_DOOR_Y_ROOM_107, FACING_NORTH);
			player_walk_trigger(2);
			break;

		case 2:
			player.walker_visible = false;
			seq[fx_open_door] = kernel_seq_pingpong(ss[fx_open_door],
				false, 8, 0, 0, 2);
			kernel_seq_player(seq[fx_open_door], true);
			kernel_seq_trigger(seq[fx_open_door],
				KERNEL_TRIGGER_SPRITE, 2, 3);
			kernel_seq_trigger(seq[fx_open_door],
				KERNEL_TRIGGER_EXPIRE, 0, 4);
			break;

		case 3:
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorOpens);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false,
				6, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 13);
			break;

		case 4:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_open_door]);
			kernel_timing_trigger(TENTH_SECOND, 5);
			break;

		case 5:
			player_walk(START_X_ROOM_107, START_Y_ROOM_107, FACING_NORTH);
			player_walk_trigger(6);
			break;

		case 6:
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorCloses);
			seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 1);
			kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, 7);
			break;

		case 7:
			temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
			kernel_timing_trigger(6, 8);
			break;

		case 8:
			new_room = 107;
			break;
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(dungeon_floor)) {
			text_show(10802);
			goto handled;
		}

		if (player_said_1(dungeon_wall)) {
			text_show(10803);
			goto handled;
		}

		if (player_said_1(rocks)) {
			text_show(10804);
			goto handled;
		}

		if (player_said_1(doorway_to_cell)) {
			text_show(10805);
			goto handled;
		}

		if (player_said_1(guard_station)) {
			text_show(10806);
			goto handled;
		}

		if (player_said_1(guard_stool)) {
			text_show(10807);
			goto handled;
		}

		if (player_said_1(door_to_dungeon_cell)) {
			text_show(10808);
			goto handled;
		}

		if (player_said_1(door_to_council_room)) {
			text_show(10809);
			goto handled;
		}

		if (player_said_1(sconce)) {
			text_show(10810);
			goto handled;
		}
	}

	if (player_said_2(open, door_to_dungeon_cell)) {
		text_show(41);
		goto handled;
	}

	if (player_said_2(close, door_to_dungeon_cell)) {
		text_show(10812);
		goto handled;
	}

	if (player_said_2(close, door_to_council_room)) {
		text_show(44);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}


void room_108_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.eye_frame);
	s.syncAsSint16LE(scratch.temp);
}

void room_108_preload() {
	room_init_code_pointer = room_108_init;
	room_pre_parser_code_pointer = room_108_pre_parser;
	room_parser_code_pointer = room_108_parser;
	room_daemon_code_pointer = room_108_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
