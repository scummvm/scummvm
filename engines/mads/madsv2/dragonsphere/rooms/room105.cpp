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
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/vocab.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room105.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

// ---------------------------------------------------------------------------
// Scratch layout (matches ROOM105.MAC)
//
//   game.scratch offsets:
//     sprite[0..14]    = 0x00..0x1C
//     sequence[0..14]  = 0x1E..0x3A
//     animation[0..3]  = 0x3C..0x42
//     temp             = 0x44
//     maid_frame       = 0x46
//     new_action       = 0x48
//     last_action      = 0x4A
//     maid_talking     = 0x4C
//     situp            = 0x4E
//     good_number      = 0x50
//     maid_id[0]       = 0x52
//     maid_id[1]       = 0x54
//     bucket_id        = 0x56
//     bone_id          = 0x58
//     goblet_id        = 0x5A
// ---------------------------------------------------------------------------
struct Scratch {
	int16 sprite[15];       // ss[]  — sprite series handles
	int16 sequence[15];     // seq[] — sequence handles
	int16 animation[4];     // aa[]  — animation handles
	int16 temp;             // 0x44
	int16 maid_frame;       // 0x46
	int16 new_action;       // 0x48
	int16 last_action;      // 0x4A
	int16 maid_talking;     // 0x4C
	int16 situp;            // 0x4E
	int16 good_number;      // 0x50
	int16 maid_id[2];       // 0x52
	int16 bucket_id;        // 0x56
	int16 bone_id;          // 0x58
	int16 goblet_id;        // 0x5A
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_fire_left    1
#define fx_fire_right   2
#define fx_door         3
#define fx_open_door    4
#define fx_goblet       5
#define fx_bone         6
#define fx_take_bone    7
#define fx_take_goblet  8

/* walk points */
#define START_X_ROOM_103     120
#define START_Y_ROOM_103     96
#define START_X_ROOM_106     104
#define START_Y_ROOM_106     152

#define WALK_TO_X_FROM_103   117
#define WALK_TO_Y_FROM_103   108

#define WALK_TO_BUCKET_X     174
#define WALK_TO_BUCKET_Y     151

#define WALK_TO_MAID_X       174
#define WALK_TO_MAID_Y       151

#define WALK_TO_BONE_X       255
#define WALK_TO_BONE_Y       145

#define WALK_TO_GOBLET_X     63
#define WALK_TO_GOBLET_Y     142

/* cursor points */
#define LEFT_TABLE_X         98

/* animation controls */
#define NO_ACTION            0
#define SCRUB                1
#define DIP                  2
#define WIPEBROW             3
#define SITUP                4
#define TALK                 5 

/* conversations */
/* conv002.con   */
#define CONVERSATION_WITH_MAID        2

/* dynamic hotspots */
#define MAID_0_X                      189
#define MAID_0_Y                      123
#define MAID_0_X_SIZE                 25
#define MAID_0_Y_SIZE                 11

#define MAID_1_X                      189
#define MAID_1_Y                      134
#define MAID_1_X_SIZE                 38
#define MAID_1_Y_SIZE                 9

#define BUCKET_X                      176
#define BUCKET_Y                      137
#define BUCKET_X_SIZE                 10
#define BUCKET_Y_SIZE                 8

/* random numbers */
#define RANDOM_LOW_NUMBER             1
#define RANDOM_HIGH_NUMBER            30
#define RANDOM_SCRUB_ALLOWED          20
#define RANDOM_DIP_ALLOWED_LOW        19    
#define RANDOM_DIP_ALLOWED_HIGH       27
#define RANDOM_KEEP_SITTING_UP        100   
#define RANDOM_WIPEBROW_HIGH_NUMBER   100

#define ROOM_105_DOOR_CLOSES      70
#define ROOM_105_DOOR_OPEN        71
#define ROOM_105_DOOR_OPEN2       72
#define ROOM_105_DOOR_OPEN3       73
#define ROOM_105_DOOR_OPEN4       74
#define ROOM_105_TAKE_GOBLET      80
#define ROOM_105_TAKE_GOBLET2     81
#define ROOM_105_TAKE_GOBLET3     82
#define ROOM_105_TAKE_BONE        83
#define ROOM_105_TAKE_BONE2       84
#define ROOM_105_TAKE_BONE3       85

static Scratch scratch;


void room_105_init() {
	ss[fx_fire_left]  = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_fire_right] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_door]       = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_open_door]  = kernel_load_series("*KGRD_8", false);

	seq[fx_fire_left]  = kernel_seq_forward(ss[fx_fire_left],  false, 7, 0, 0, 0);
	seq[fx_fire_right] = kernel_seq_forward(ss[fx_fire_right], false, 7, 3, 0, 0);

	local->maid_id[0] = kernel_add_dynamic(words_scullery_maid, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE, 189, 123, 25, 11);
	kernel_dynamic_walk(local->maid_id[0], 174, 151, FACING_NORTHEAST);
	kernel_dynamic_hot[local->maid_id[0]].prep = PREP_ON;

	local->maid_id[1] = kernel_add_dynamic(words_scullery_maid, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE, 189, 134, 38, 9);
	kernel_dynamic_walk(local->maid_id[1], 174, 151, FACING_NORTHEAST);
	kernel_dynamic_hot[local->maid_id[1]].prep = PREP_ON;

	local->bucket_id = kernel_add_dynamic(words_bucket, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE, 176, 137, 10, 8);
	kernel_dynamic_walk(local->bucket_id, 174, 151, FACING_NORTHEAST);

	if (object_is_here(goblet)) {
		ss[fx_goblet]      = kernel_load_series(kernel_name('p', 1), false);
		ss[fx_take_goblet] = kernel_load_series("*KGRM1_8", false);
		seq[fx_goblet]     = kernel_seq_stamp(ss[fx_goblet], false, KERNEL_FIRST);
		local->goblet_id   = kernel_add_dynamic(words_goblet, words_walk_to, SYNTAX_SINGULAR, seq[fx_goblet], 0, 0, 0, 0);
		kernel_seq_depth(seq[fx_goblet], 6);
		kernel_dynamic_walk(local->goblet_id, 63, 142, FACING_WEST);
	}

	if (object_is_here(bone)) {
		ss[fx_bone]      = kernel_load_series(kernel_name('p', 0), false);
		ss[fx_take_bone] = kernel_load_series("*KGRD_8", false);
		seq[fx_bone]     = kernel_seq_stamp(ss[fx_bone], false, KERNEL_FIRST);
		local->bone_id   = kernel_add_dynamic(words_bone, words_walk_to, SYNTAX_SINGULAR, seq[fx_bone], 0, 0, 0, 0);
		kernel_seq_depth(seq[fx_bone], 6);
		kernel_dynamic_walk(local->bone_id, 255, 145, FACING_EAST);
	}

	conv_get(2);
	local->new_action   = 1;
	local->last_action  = 0;
	local->maid_talking = 0;
	local->situp        = 0;
	local->good_number  = 0;
	aa[0] = kernel_run_animation(kernel_name('A', -1), 1);

	if (previous_room == 106) {
		player.x      = 104;
		player.y      = 152;
		player.facing = FACING_NORTHEAST;
		seq[fx_door]  = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 12);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 6);
		kernel_seq_depth(seq[fx_door], 12);
		player_first_walk(120, 96, FACING_SOUTH, 117, 108, FACING_SOUTH, false);
		player_walk_trigger(ROOM_105_DOOR_CLOSES);
	} else {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 15);
	}

	section_1_music();
}

void room_105_get_random() {
	int random;

	local->last_action = local->new_action;
	random = imath_random(RANDOM_LOW_NUMBER, RANDOM_HIGH_NUMBER);
	if (random < RANDOM_SCRUB_ALLOWED) {
		local->new_action = SCRUB;
		local->good_number = true;

	} else if ((random > RANDOM_DIP_ALLOWED_LOW) &&
		(random < RANDOM_DIP_ALLOWED_HIGH) && (local->last_action != DIP)) {
		local->new_action = DIP;
		local->good_number = true;

	} else if (local->last_action != WIPEBROW) {
		local->new_action = WIPEBROW;
		local->good_number = true;
	}
}

void room_105_random_wipebrow() {
	int random;

	local->last_action = local->new_action;
	random = imath_random(RANDOM_LOW_NUMBER, RANDOM_WIPEBROW_HIGH_NUMBER);

	if (random < RANDOM_KEEP_SITTING_UP) {
		local->new_action = SITUP;
		local->good_number = true;

	} else if (local->last_action != WIPEBROW) {
		local->new_action = WIPEBROW;
		local->good_number = true;
	}
}

void room_105_daemon() {
	int reset_frame;

	/* Control scullery maid animation */
	if (kernel_anim[aa[0]].anim != NULL) {
		if (kernel_anim[aa[0]].frame != local->maid_frame) {
			local->maid_frame = kernel_anim[aa[0]].frame;
			reset_frame = -1;

			if (local->situp && !local->maid_talking) {
				local->last_action = local->new_action;
				local->new_action = SITUP;
			}

			if (local->maid_talking) {
				local->last_action = local->new_action;
				local->new_action = TALK;
			}

			switch (local->maid_frame) {
			case 16:
				/* Has finished scrubbing */
				if (local->new_action == SCRUB) {
					local->good_number = false;
					while (!local->good_number) {
						room_105_get_random();
					}
				}
				if (local->new_action == SCRUB) {
					reset_frame = 1;
				}
				if ((local->new_action == WIPEBROW && !local->situp) ||
					local->new_action == SITUP) {
					reset_frame = 53;
				}
				if ((local->new_action == WIPEBROW) && (local->situp)) {
					reset_frame = 24;
				}
				break;

			case 24:
				/* Has finished dip */
				if (local->new_action == DIP) {
					local->good_number = false;
					while (!local->good_number) {
						room_105_get_random();
					}
				}
				if ((local->new_action == WIPEBROW) || (local->new_action == TALK)) {
					reset_frame = 53;
				}
				if (local->new_action == SCRUB) {
					reset_frame = 56;
				}
				if (local->new_action == SITUP) {
					reset_frame = 53;
				}
				break;

			case 32:
				/* Has finished wiping brow */
				if (local->new_action == WIPEBROW) {
					local->good_number = false;
					while (!local->good_number) {
						room_105_get_random();
					}
				}
				if ((local->new_action == SCRUB) || (local->new_action == DIP)) {
					reset_frame = 57;
				}
				if (local->new_action == SITUP) {
					reset_frame = 55;
				}
				break;

				/* Has finished talking */
			case 53:
				local->maid_talking = false;
				local->good_number = false;
				while (!local->good_number) {
					room_105_get_random();
				}
				if (local->new_action == SCRUB) {
					reset_frame = 60;
				}
				if (local->new_action == DIP) {
					reset_frame = 57;
				}
				if (local->new_action == WIPEBROW) {
					reset_frame = 63;
				}
				if (local->new_action == SITUP) {
					reset_frame = 55;
				}
				break;

			case 56:
				if (local->new_action == SITUP) {
					local->good_number = false;
					while (!local->good_number) {
						room_105_random_wipebrow();
					}
				}
				if (local->new_action == SITUP) {
					reset_frame = 55;
				}
				if (local->new_action == TALK) {
					reset_frame = 32;
				}
				if (local->new_action == WIPEBROW) {
					reset_frame = 24;
				}
				break;

			case 57:
				if (local->new_action == SCRUB) {
					reset_frame = 1;
				}
				break;

			case 60:
				if (local->new_action == SCRUB) {
					reset_frame = 1;
				}
				if (local->new_action == DIP) {
					reset_frame = 16;
				}
				break;

			case 63:
				if (local->new_action == SCRUB) {
					reset_frame = 1;
				}
				break;

			case 64:
				/* wipebrow move - didn't check for action status so wouldn't fall */
				/* through code. Will always go to frame 24.                       */
				reset_frame = 24;
				break;
			}

			if (reset_frame >= 0) {
				if (reset_frame != kernel_anim[aa[0]].frame) {
					kernel_reset_animation(aa[0], reset_frame);
					local->maid_frame = reset_frame;
				}
			}
		}
	}

	/* Close the door when player enters */
	if (kernel.trigger >= ROOM_105_DOOR_CLOSES) {
		switch (kernel.trigger) {
		case ROOM_105_DOOR_CLOSES:
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorCloses);
			seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
				6, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 12);
			kernel_seq_range(seq[fx_door], 1, 5);
			kernel_seq_trigger(seq[fx_door],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_105_DOOR_CLOSES + 1);
			break;

		case ROOM_105_DOOR_CLOSES + 1:
			local->temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door], 15);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, local->temp);
			player.commands_allowed = true;
			break;
		}
	}
}

void room_105_conversation() {
	if (player_verb == conv002_counter_only) {
		if (!local->situp) {
			local->situp = true;
		}
	}

	if ((player_verb == conv002_banter_random) ||
		(player_verb == conv002_flirt_rand)) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->maid_talking = true;
			local->situp = false;
			conv_me_trigger(2);
			break;

		case 2:
			local->maid_talking = false;
			break;
		}
	}
}

void room_105_pre_parser() {
	if (player_parse(words_talk_to, words_scullery_maid, 0))
		local->situp = -1;
}

void room_105_parser() {
	int temp;

	if (player.look_around) {
		text_show(10501);
		goto handled;
	}

	if (conv_control.running == CONVERSATION_WITH_MAID) {
		room_105_conversation();
		goto handled;
	}

	if (player_said_2(walk_through, door_to_hallway) || player_said_2(open, door_to_hallway) ||
		player_said_2(pull, door_to_hallway)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			seq[fx_open_door] = kernel_seq_pingpong(ss[fx_open_door],
				false, 8, 0, 0, 2);
			kernel_seq_player(seq[fx_open_door], true);
			kernel_seq_trigger(seq[fx_open_door],
				KERNEL_TRIGGER_SPRITE, 2, 1);
			kernel_seq_trigger(seq[fx_open_door],
				KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorOpens);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false,
				6, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 12);
			kernel_seq_trigger(seq[fx_door],
				KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			local->temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_door], 12);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, local->temp);
			break;

		case 3:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_open_door]);
			kernel_timing_trigger(1, 4);
			break;

		case 4:
			player_walk(START_X_ROOM_103, START_Y_ROOM_103, FACING_NORTH);
			player_walk_trigger(5);
			break;

		case 5:
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorCloses);
			seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 1);
			kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, 6);
			break;

		case 6:
			temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
			kernel_timing_trigger(6, 7);
			break;

		case 7:
			new_room = 103;
			break;
		}
		goto handled;
	}

	if (player_said_2(walk_through, door_to_throne_room) || player_said_2(open, door_to_throne_room) ||
		player_said_2(pull, door_to_throne_room)) {
		new_room = 106;
		goto handled;
	}

	if (player_said_2(talk_to, scullery_maid)) {
		conv_run(CONVERSATION_WITH_MAID);
		goto handled;
	}

	if (player_said_2(take, goblet)) {
		if (kernel.trigger || !player_has(goblet)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_take_goblet] = kernel_seq_pingpong(ss[fx_take_goblet],
					true, 6, 0, 0, 2);
				kernel_seq_range(seq[fx_take_goblet], 1, 7);
				kernel_seq_player(seq[fx_take_goblet], true);
				kernel_seq_trigger(seq[fx_take_goblet],
					KERNEL_TRIGGER_SPRITE, 7, 1);
				kernel_seq_trigger(seq[fx_take_goblet],
					KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				/* sound_queue (N_PickUpObject006);*/
				kernel_seq_delete(seq[fx_goblet]);
				kernel_delete_dynamic(local->goblet_id);
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(goblet);
				object_examine(goblet, 10519, 0);
				break;

			case 2:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_goblet]);
				global[player_score] += 1;
				player.walker_visible = true;
				player.commands_allowed = true;
				break;
			}
			goto handled;
		}
	}

	if (player_said_2(take, bone) && player.main_object_source == STROKE_INTERFACE) {
		if (kernel.trigger || !player_has(bone)) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_take_bone] = kernel_seq_pingpong(ss[fx_take_bone],
					false, 6, 0, 0, 2);
				kernel_seq_player(seq[fx_take_bone], true);
				kernel_seq_trigger(seq[fx_take_bone],
					KERNEL_TRIGGER_SPRITE, 6, 1);
				kernel_seq_trigger(seq[fx_take_bone],
					KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				/* sound_queue (N_PickUpObject006);*/
				kernel_seq_delete(seq[fx_bone]);
				sound_play(N_TakeObjectSnd);
				kernel_delete_dynamic(local->bone_id);
				inter_give_to_player(bone);
				object_examine(bone, 10520, 0);
				break;

			case 2:
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_bone]);
				if (!(global[player_score_flags] & SCORE_TAKE_BONE)) {
					global[player_score_flags] = global[player_score_flags] | SCORE_TAKE_BONE;
					global[player_score] += 1;
				}
				player.walker_visible = true;
				player.commands_allowed = true;
				break;
			}
			goto handled;
		}
	}

	if (player_said_2(take, bone) && player_has(bone) &&
		player.main_object_source == STROKE_INTERFACE) {
		text_show(40112);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(floor)) {
			text_show(10502);
			goto handled;
		}

		if (player_said_1(brazier)) {
			text_show(10503);
			goto handled;
		}

		if (player_said_1(dining_table)) {
			if (inter_point_x <= 98) {
				if (object[goblet].location == HERE) {
					text_show(10505);
				} else {
					text_show(10522);
				}
			} else {
				text_show(10504);
			}
			goto handled;
		}

		if (player_said_1(chair)) {
			text_show(10507);
			goto handled;
		}

		if (player_said_1(window)) {
			text_show(10508);
			goto handled;
		}

		if (player_said_1(tapestry)) {
			text_show(10509);
			goto handled;
		}

		if (player_said_1(door_to_throne_room)) {
			text_show(10511);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(10512);
			goto handled;
		}

		if (player_said_1(door_to_hallway)) {
			text_show(10513);
			goto handled;
		}

		if (player_said_1(dividing_wall)) {
			text_show(10514);
			goto handled;
		}

		if (player_said_1(ceiling)) {
			text_show(10515);
			goto handled;
		}

		if (player_said_1(scullery_maid)) {
			text_show(10516);
			goto handled;
		}

		if (player_said_1(goblet)) {
			if (object_is_here(goblet)) {
				text_show(10517);
				goto handled;
			}
		}

		if (player_said_1(bone)) {
			if (object_is_here(bone)) {
				text_show(10518);
				goto handled;
			}
		}

		if (player_said_1(sconce)) {
			text_show(10524);
			goto handled;
		}

		if (player_said_1(bucket)) {
			text_show(10521);
			goto handled;
		}
	}

	if (player_said_2(take, scullery_maid)) {
		text_show(30);
		goto handled;
	}

	if ((player_said_1(push) || player_said_1(pull)) &&
		player_said_1(dining_table)) {
		text_show(10506);
		goto handled;
	}

	if ((player_said_1(push) || player_said_1(pull)) &&
		player_said_1(tapestry)) {
		text_show(10510);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_105_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.temp);
	s.syncAsSint16LE(scratch.maid_frame);
	s.syncAsSint16LE(scratch.new_action);
	s.syncAsSint16LE(scratch.last_action);
	s.syncAsSint16LE(scratch.maid_talking);
	s.syncAsSint16LE(scratch.situp);
	s.syncAsSint16LE(scratch.good_number);
	for (int16 &v : scratch.maid_id)   s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.bucket_id);
	s.syncAsSint16LE(scratch.bone_id);
	s.syncAsSint16LE(scratch.goblet_id);
}

void room_105_preload() {
	room_init_code_pointer       = room_105_init;
	room_pre_parser_code_pointer = room_105_pre_parser;
	room_parser_code_pointer     = room_105_parser;
	room_daemon_code_pointer     = room_105_daemon;

	section_1_walker();
	section_1_interface();

	vocab_make_active(words_scullery_maid);
	vocab_make_active(words_bucket);
	vocab_make_active(words_walk_to);
	vocab_make_active(words_bone);
	vocab_make_active(words_goblet);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
