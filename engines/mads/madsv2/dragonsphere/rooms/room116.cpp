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
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room116.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[9];     /* Animation handles     */

	int16 temp;             /* for synching sprites */
	int16 kingsicle_id;     /* hotspot id for frozen king */
	int16 bear_status;      /* for when player is a bear */
	int16 animation_running;
	int16 current_frame;    /* helps keep track of frame animation is on */
	int16 just_melted;      /* the king just melted - free up player commands */

	int16 king_frame;       /* animation frame being held for King stuff */
	int16 king_action;      /* Type of action to run for King animation */
	int16 king_talk_count;  /* counter for King talking */
	int16 anim_1_running;

	int16 lift_frame;       /* animation frame being held for lifting King stuff */
	int16 lift_action;      /* Type of action to run for lifting King animation */
	int16 anim_2_running;

	int16 invoked_ring;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;


/* ========================= Sprites ========================= */

#define fx_door                 1  /* rm116x  */
#define fx_open_door            2  /* rm116g  */
#define fx_kingsicle            3  /* rm116a4 */
#define fx_bear_morph           4  /* rm116b1 */
#define fx_give_soul            5  /* pdrm_6  */
#define fx_spill                6  /* rm116x1 */
#define fx_smoke                7  /* rm116a5 */
#define fx_single_king          8  /* rm116a3 */


/* ======================== Triggers ========================= */

#define ROOM_116_DOOR_CLOSES    70
#define ROOM_116_YOU_TALK       78
#define ROOM_116_ME_TALK        79
#define ROOM_116_LEAVE_ROOM     81


/* walk points */
#define START_X_ROOM_119        291
#define START_Y_ROOM_119        84
#define START_X_ROOM_115        196
#define START_Y_ROOM_115        152

#define WALK_TO_X_FROM_119      246
#define WALK_TO_Y_FROM_119      98

#define WALK_TO_KINGSICLE_X     165
#define WALK_TO_KINGSICLE_Y     145

#define WALK_TO_KING_SOUL_X     152
#define WALK_TO_KING_SOUL_Y     145

/* conversations */
/* conv013.con   */
#define CONVERSATION_WITH_KING  13

/* dynamic hotspots */
#define KING_X                  120
#define KING_Y                  95
#define KING_X_SIZE             13
#define KING_Y_SIZE             50

/* animation descriptions */
#define KING_NO_ANIMATION       0
#define KING_OFF_ICE            1
#define KING_THRU_DOOR          2

/* animations */
/* rm116a.aa - bear pulls king off pedestal */

/* for bear_status */
#define HAS_NEVER_BEEN_A_BEAR   0
#define FIRST_TIME_BEAR         1
#define IS_PID_AGAIN            2
#define IS_A_BEAR_AGAIN         3

#define KING_SHUT_UP            0
#define KING_TALK               1


static void room_116_init() {
	local->invoked_ring = false;

	ss[fx_spill] = kernel_load_series(kernel_name('x', 1), false);

	if (global[king_is_in_stairwell]) {
		seq[fx_spill] = kernel_seq_stamp(ss[fx_spill], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_spill], 15);
	}

	ss[fx_open_door] = kernel_load_series(kernel_name('g', -1), false);
	ss[fx_door] = kernel_load_series(kernel_name('x', -1), false);
	ss[fx_single_king] = kernel_load_series(kernel_name('a', 3), false);

	if (global[king_status] == KING_CAPTIVE) {
		ss[fx_bear_morph] = kernel_load_series(kernel_name('b', 1), false);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->bear_status = HAS_NEVER_BEEN_A_BEAR;
		local->just_melted = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->animation_running = KING_NO_ANIMATION;
	}

	conv_get(CONVERSATION_WITH_KING);

	if (global[king_status] == KING_CAPTIVE) {
		ss[fx_give_soul] = kernel_load_series("*PDRM_6", false);
		ss[fx_kingsicle] = kernel_load_series(kernel_name('a', 4), false);
		seq[fx_kingsicle] = kernel_seq_stamp(ss[fx_kingsicle], false, KERNEL_FIRST);


		local->kingsicle_id = kernel_add_dynamic(words_king, words_walk_to, SYNTAX_MASC_NOT_PROPER,
			seq[fx_kingsicle], 0, 0, 0, 0);
		kernel_dynamic_hot[local->kingsicle_id].prep = PREP_ON;
		kernel_seq_depth(seq[fx_kingsicle], 5);
		kernel_dynamic_walk(local->kingsicle_id, WALK_TO_KINGSICLE_X, WALK_TO_KINGSICLE_Y, FACING_NORTHWEST);

	} else if (!global[king_is_in_stairwell]) {
		if (global[king_status] == KING_WITHOUT_SOUL) {
			ss[fx_give_soul] = kernel_load_series("*PDRM_6", false);
		}
		aa[1] = kernel_run_animation(kernel_name('C', -1), 0);
		local->anim_1_running = true;
		local->king_action = KING_SHUT_UP;
		local->kingsicle_id = kernel_add_dynamic(words_king, words_walk_to, SYNTAX_MASC_NOT_PROPER,
			KERNEL_NONE, KING_X, KING_Y, KING_X_SIZE,
			KING_Y_SIZE);
		kernel_dynamic_walk(local->kingsicle_id, WALK_TO_KINGSICLE_X, WALK_TO_KINGSICLE_Y, FACING_WEST);
	}

	if (previous_room == 119) {        /* Player comes from Stairwell*/
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door], 14);
		player_first_walk(START_X_ROOM_119, START_Y_ROOM_119, FACING_SOUTHWEST,
			WALK_TO_X_FROM_119, WALK_TO_Y_FROM_119, FACING_SOUTHWEST,
			false);
		player_walk_trigger(ROOM_116_DOOR_CLOSES);

	} else if (previous_room != KERNEL_RESTORING_GAME) {  /* Player comes from Darkness Beast room 115*/
		player.x = START_X_ROOM_115;
		player.y = START_Y_ROOM_115;
		player.facing = FACING_NORTH;
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);

	} else {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);
		if (local->bear_status == FIRST_TIME_BEAR || local->bear_status == IS_A_BEAR_AGAIN) {
			player.walker_visible = false;
			seq[fx_bear_morph] = kernel_seq_stamp(ss[fx_bear_morph], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_bear_morph], 4);
		}
		if (conv_restore_running == CONVERSATION_WITH_KING) {
			local->king_action = KING_SHUT_UP;
			player.facing = FACING_WEST;
			conv_run(CONVERSATION_WITH_KING);
			if (global[king_status] == KING_WITH_SOUL) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}
		}
	}

	section_1_music();
}

static void handle_animation_lift() {
	int lift_reset_frame;

	if (kernel_anim[aa[2]].frame != local->lift_frame) {
		local->lift_frame = kernel_anim[aa[2]].frame;
		lift_reset_frame = -1;

		switch (local->lift_frame) {
		case 9:
			kernel_seq_delete(seq[fx_kingsicle]);
			kernel_delete_dynamic(local->kingsicle_id);
			break;

		case 26:
			seq[fx_single_king] = kernel_seq_stamp(ss[fx_single_king], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_single_king], 1);
			kernel_synch(KERNEL_SERIES, seq[fx_single_king], KERNEL_ANIM, aa[2]);
			break;

		case 31:
			if (player_said_1(crystal_ball)) {
				text_show(11617);
				inter_move_object(crystal_ball, NOWHERE);
				text_show(970);
			}
			kernel_abort_animation(aa[2]);
			aa[0] = kernel_run_animation(kernel_name('k', -1), 0);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
			kernel_reset_animation(aa[0], 33);
			local->just_melted = true;
			local->animation_running = KING_OFF_ICE;
			local->anim_2_running = false;
			lift_reset_frame = -1;
			break;
		}

		if (lift_reset_frame >= 0) {
			kernel_reset_animation(aa[2], lift_reset_frame);
			local->lift_frame = lift_reset_frame;
		}
	}
}


static void handle_animation_king() {
	int king_reset_frame;

	if (kernel_anim[aa[1]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[1]].frame;
		king_reset_frame = -1;

		switch (local->king_frame) {
		case 1:  /* end of talk 1 */
		case 2:  /* end of talk 2 */
		case 3:  /* end of talk 3 */
			if (local->king_action == KING_TALK) {

				king_reset_frame = imath_random(0, 2);
				++local->king_talk_count;
				if (local->king_talk_count > 15) {
					local->king_action = KING_SHUT_UP;
					local->king_talk_count = 0;
					king_reset_frame = 0; /* freeze */
				}

			} else {
				king_reset_frame = 0; /* freeze */
			}
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[1], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}

static void room_116_daemon() {
	int reset_frame;

	if (local->anim_1_running) {
		handle_animation_king();
	}

	if (local->anim_2_running) {
		handle_animation_lift();
	}

	if (local->animation_running == KING_OFF_ICE) {
		if (kernel_anim[aa[0]].frame != local->current_frame) {
			local->current_frame = kernel_anim[aa[0]].frame;
			reset_frame = -1;

			switch (local->current_frame) {
			case 34:
				kernel_seq_delete(seq[fx_single_king]);
				break; /* delete king on ice so he can melt */

			case 43:
				player.walker_visible = true;
				player_demand_facing(FACING_WEST);
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
				break;

			case 73:
				if (local->just_melted) {
					kernel_load_variant(1);
					local->just_melted = false;
					local->bear_status = IS_PID_AGAIN;
					global[king_status] = KING_WITHOUT_SOUL;
					kernel_delete_dynamic(local->kingsicle_id);
					local->kingsicle_id = kernel_add_dynamic(words_king, words_walk_to, SYNTAX_MASC_NOT_PROPER,
						KERNEL_NONE, KING_X, KING_Y, KING_X_SIZE,
						KING_Y_SIZE);
					kernel_dynamic_hot[local->kingsicle_id].prep = PREP_ON;
					kernel_dynamic_walk(local->kingsicle_id, WALK_TO_KINGSICLE_X, WALK_TO_KINGSICLE_Y, FACING_WEST);
					player.commands_allowed = true;

					kernel_abort_animation(aa[0]);
					reset_frame = -1;
					local->animation_running = 0;
					aa[1] = kernel_run_animation(kernel_name('c', -1), 0);
					local->king_action = KING_SHUT_UP;
					local->anim_1_running = true;

					conv_run(CONVERSATION_WITH_KING);
					conv_export_value(0);
				}
				break;
			}

			if (reset_frame >= 0) {
				if (reset_frame != kernel_anim[aa[0]].frame) {
					kernel_reset_animation(aa[0], reset_frame);
					local->current_frame = reset_frame;
				}
			}
		}
	}

	if (kernel.trigger >= ROOM_116_DOOR_CLOSES) {
		switch (kernel.trigger) {
		case ROOM_116_DOOR_CLOSES:
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorCloses);
			seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
				7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_seq_range(seq[fx_door], 1, 5);
			kernel_seq_trigger(seq[fx_door],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_116_DOOR_CLOSES + 1);
			break;

		case ROOM_116_DOOR_CLOSES + 1:
			local->temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, local->temp);
			player.commands_allowed = true;
			break;
		}
	}
}

static void room_116_pre_parser() {
	if (!player_parse(words_look, 0) &&
	    !player_parse(words_push, words_king, 0) &&
	    !player_parse(7, words_king, words_cave_floor, 0) &&
	    !player_parse(words_pull, words_king, 0) &&
	    !player_parse(words_take, words_king, 0) &&
	    !player_parse(words_walk_to, words_king, 0) &&
	    (local->bear_status == FIRST_TIME_BEAR || local->bear_status == IS_A_BEAR_AGAIN) &&
	    global[king_status] == KING_CAPTIVE &&
	    (player.need_to_walk || player_parse(words_revert, 0))) {

		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.ready_to_walk = false;
			kernel_seq_delete(seq[fx_bear_morph]);
			seq[fx_bear_morph] = kernel_seq_backward(ss[fx_bear_morph], false, 8, 1, 0, 0);
			kernel_seq_depth(seq[fx_bear_morph], 4);
			kernel_seq_trigger(seq[fx_bear_morph], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			local->bear_status = IS_PID_AGAIN;
			player.walker_visible = true;
			player.commands_allowed = true;
			if (!player_parse(words_revert, 0)) {
				player.ready_to_walk = true;
			}
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_bear_morph]);
			break;
		}
	}

	if (player_parse(words_invoke_power_of, words_crystal_ball, 0)) {
		if (local->bear_status == HAS_NEVER_BEEN_A_BEAR ||
		    local->bear_status == IS_PID_AGAIN) {
			if (global[king_status] == KING_CAPTIVE ||
			    global[king_status] == KING_WITHOUT_SOUL) {
				player_walk(WALK_TO_KINGSICLE_X, WALK_TO_KINGSICLE_Y, FACING_WEST);
			}
		}
	}
}

static void room_116_conversation_with_king() {
	bool flag1 = false, flag2 = false;

	if (player_verb == 3) {
		flag1 = flag2 = true;

		if (kernel.trigger == 0)
			conv_me_trigger(81);
	}

	if (kernel.trigger == 81)
		new_room = local->invoked_ring ? 115 : 110;

	if (kernel.trigger == 78)
		local->king_action = 1;
	if (kernel.trigger == 79)
		local->king_action = 0;

	if (!flag2)
		conv_me_trigger(79);
	if (!flag1)
		conv_you_trigger(78);

	local->king_talk_count = 0;
}

static void room_116_parser() {
	if (conv_control.running == CONVERSATION_WITH_KING) {
		room_116_conversation_with_king();
		goto handled;
	}

	if (player_parse(words_revert, 0) &&
	    (local->bear_status == FIRST_TIME_BEAR || local->bear_status == IS_A_BEAR_AGAIN)) {
		goto handled;
	}

	if (player.look_around) {
		text_show(global[king_status] == KING_CAPTIVE ? 11601 : 11608);
		goto handled;
	}

	if (player_parse(words_talk_to, words_king, 0)) {
		if (global[king_status] == KING_CAPTIVE) {
			text_show(11626);
		} else {
			conv_run(CONVERSATION_WITH_KING);
			conv_export_value(global[king_status] == KING_WITH_SOUL ? 1 : 0);
			conv_export_value(0);
		}
		goto handled;
	}

	if (player_parse(words_walk_through, words_door_to_north, 0) ||
	    player_parse(words_open, words_door_to_north, 0) ||
	    player_parse(words_pull, words_door_to_north, 0)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;

			if (global[king_status] == KING_WITH_SOUL && !global[king_is_in_stairwell]) {
				kernel_abort_animation(aa[1]);
				local->anim_1_running = false;
				aa[0] = kernel_run_animation(kernel_name('B', -1), 0);
				local->animation_running = KING_THRU_DOOR;
			}

			player.walker_visible = false;
			seq[fx_open_door] = kernel_seq_pingpong(ss[fx_open_door], false, 8, 2, 0, 0);
			kernel_seq_player(seq[fx_open_door], -1);
			kernel_seq_trigger(seq[fx_open_door], KERNEL_TRIGGER_SPRITE, 2, 1);
			kernel_seq_trigger(seq[fx_open_door], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorOpens);
			seq[fx_open_door] = kernel_seq_forward(ss[fx_door], false, 7, 0, 0, 0);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;
		case 2:
			local->temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, local->temp);
			break;
		case 3:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_open_door]);
			kernel_timing_trigger(1, 4);
			break;
		case 4:
			player_walk(START_X_ROOM_119, START_Y_ROOM_119, FACING_EAST);

			if (global[king_status] == KING_WITH_SOUL && global[king_is_in_stairwell])
				player_walk_trigger(5);
			break;
		case 5:
			if (global[king_status] != KING_CAPTIVE || global[king_is_in_stairwell]) {
				seq[fx_spill] = kernel_seq_stamp(ss[fx_spill], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_spill], 10);
			}

			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorCloses);
			seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, 6);
			break;
		case 6:
			local->temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, local->temp);
			new_room = 119;
			break;
		}
		goto handled;
	}

	if (player_parse(words_walk_through, words_doorway_to_south, 0) ||
			player_parse(words_open, words_doorway_to_south, 0) ||
			player_parse(words_pull, words_doorway_to_south, 0)) {
		if (global[king_status] == KING_WITH_SOUL && !global[king_is_in_stairwell]) {
			conv_run(CONVERSATION_WITH_KING);
			conv_export_value(global[king_status] == KING_WITH_SOUL ? 1 : 0);
			conv_export_value(1);
		} else {
			new_room = 115;
		}
		goto handled;
	}

	// Can't shift into seal/snake while a bear
	if ((player_parse(words_shift_into_seal, 0) || player_parse(words_shift_into_snake, 0)) &&
	    (local->bear_status == FIRST_TIME_BEAR || local->bear_status == IS_A_BEAR_AGAIN)) {
		text_show(990);
		goto handled;
	}

	// Shift into bear / invoke crystal ball (bear transformation)
	if (player_parse(words_shift_into_bear, 0) ||
	    player_parse(words_invoke_power_of, words_crystal_ball, 0)) {
		if (player_parse(words_shift_into_bear, 0) &&
				(local->bear_status == FIRST_TIME_BEAR || local->bear_status == IS_A_BEAR_AGAIN)) {
			text_show(11629);
			goto handled;
		} else if ((local->bear_status == HAS_NEVER_BEEN_A_BEAR || local->bear_status == IS_PID_AGAIN) &&
				global[king_status] == KING_CAPTIVE) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player_walk(WALK_TO_KINGSICLE_X, WALK_TO_KINGSICLE_Y, FACING_NORTHWEST);
				player_walk_trigger(1);
				break;

			case 1:
				if (player_parse(words_invoke_power_of, 0))
					sound_play(N_InvokeCrystalBall);

				player.walker_visible = false;
				seq[fx_bear_morph] = kernel_seq_forward(ss[fx_bear_morph], false, 8, 0, 0, 1);
				kernel_seq_depth(seq[fx_bear_morph], 4);
				kernel_seq_range(seq[fx_bear_morph], KERNEL_FIRST, KERNEL_LAST);
				kernel_synch(1, seq[fx_bear_morph], 2, 0);
				kernel_seq_trigger(seq[fx_bear_morph], KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				local->temp = seq[fx_bear_morph];

				if (player_parse(words_crystal_ball, 0)) {
					aa[fx_open_door] = kernel_run_animation(kernel_name('a', -1), 0);
					local->anim_2_running = true;
					global[crystal_ball_dead] = true;
				} else {
					seq[fx_bear_morph] = kernel_seq_stamp(ss[fx_bear_morph], 0, KERNEL_LAST);
					kernel_seq_depth(seq[fx_bear_morph], 4);
					kernel_synch(KERNEL_SERIES, seq[fx_bear_morph], KERNEL_SERIES, local->temp);
					player.commands_allowed = true;
				}

				if (local->bear_status == HAS_NEVER_BEEN_A_BEAR) {
					global[player_score] += 5;
					local->bear_status = FIRST_TIME_BEAR;
				} else if (local->bear_status == IS_PID_AGAIN) {
					local->bear_status = IS_A_BEAR_AGAIN;
				}

				if (local->bear_status == HAS_NEVER_BEEN_A_BEAR)
					local->bear_status = FIRST_TIME_BEAR;
				else
					local->bear_status = IS_A_BEAR_AGAIN;
				break;
			}
		}

		goto handled;
	}

	// Bear grabs king / gives soul to king
	if ((player_parse(words_push, words_king, 0) ||
		player_parse(words_pull, words_king, 0) ||
		player_parse(words_take, words_king, 0) ||
		player_parse(words_put, words_king, words_cave_floor)) &&
		global[king_status] == KING_CAPTIVE) {
		if (local->bear_status == HAS_NEVER_BEEN_A_BEAR) {
			kernel_seq_delete(seq[fx_bear_morph]);
			player.commands_allowed = false;
			local->anim_2_running = true;
			aa[fx_bear_morph] = kernel_run_animation(kernel_name('a', -1), 0);
		} else {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player_walk(WALK_TO_KINGSICLE_X, WALK_TO_KINGSICLE_Y, FACING_NORTH);
				player_walk_trigger(1);
				break;

			case 1:
				player.walker_visible = false;
				seq[fx_open_door] = kernel_seq_pingpong(ss[fx_open_door], 0, 9, 0, 0, 2);
				kernel_seq_player(seq[fx_open_door], -1);
				kernel_seq_trigger(seq[fx_open_door], 0, 0, 8);
				break;

			case 2:
				player.walker_visible = true;
				kernel_synch(2, 0, 1, seq[fx_open_door]);
				kernel_timing_trigger(12, 3);
				sound_play(N_GrabKing);
				break;

			case 3:
				text_show(game.difficulty == 0 ? 11612 : 11613);
				player.commands_allowed = true;
				break;
			}
		}

		goto handled;
	}

	if (player_parse(words_give, 82, words_king) ||
			player_parse(words_invoke_power_of, words_crystal_ball) ||
			player_parse(words_put, 82, words_king))  {
		if ((player_parse(words_give, 82, words_king) || player_parse(words_put, 82, words_king)) &&
				global[king_status] == KING_CAPTIVE) {
			text_show(11630);
		} else if (global[king_status] == KING_WITHOUT_SOUL) {
			switch (kernel.trigger) {
			case 0:
				if (player_parse(words_crystal_ball, 0)) {
					sound_play(N_InvokeCrystalBall);
					kernel_timing_trigger(1, 3);
				} else {
					player.commands_allowed = false;
					player_walk(WALK_TO_KING_SOUL_X, WALK_TO_KING_SOUL_Y, FACING_WEST);
					player_walk_trigger(1);
				}
				break;

			case 1:
				if (!player_parse(words_crystal_ball, 0)) {
					player.walker_visible = false;
					seq[fx_give_soul] = kernel_seq_pingpong(ss[fx_give_soul], -1, 8, 0, 0, 2);
					kernel_seq_player(seq[fx_give_soul], -1);
					kernel_seq_trigger(seq[fx_give_soul], 2, 4, 2);
					kernel_seq_trigger(seq[fx_give_soul], 0, 0, 3);
				}
				break;

			case 2:
				sound_play(97);
				ss[fx_smoke] = kernel_load_series(kernel_name('a', 5), false);
				seq[fx_smoke] = kernel_seq_forward(ss[fx_smoke], false, 8, 0, 0, 1);
				kernel_seq_depth(seq[fx_smoke], 1);
				kernel_seq_range(seq[fx_smoke], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_smoke], KERNEL_TRIGGER_EXPIRE, 0, 4);
				break;

			case 3:
				if (player_parse(109, 0)) {
					text_show(11618);
					inter_move_object(crystal_ball, NOWHERE);
					text_show(970);
					kernel_timing_trigger(1, 6);
				} else {
					player.walker_visible = true;
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_give_soul]);
				}
				break;

			case 4:
				matte_deallocate_series(ss[fx_smoke], -1);
				kernel_timing_trigger(1, 5);
				break;

			case 5:
				kernel_timing_trigger(10, 6);
				break;

			case 6:
				if (!player_parse(crystal_ball, 0)) {
					inter_move_object(soul_egg, NOWHERE);
					text_show(11614);
				}

				global[king_status] = KING_WITH_SOUL;
				global[player_score] += 5;
				player.commands_allowed = true;
				local->king_action = 0;
				conv_run(CONVERSATION_WITH_KING);
				conv_export_value(global[king_status] == KING_WITH_SOUL ? 1 : 0);
				break;
			}
		}

		goto handled;
	}

	if (player_parse(words_invoke, words_signet_ring, 0)) {
		if (global[king_status] == KING_WITH_SOUL && !global[king_is_in_stairwell]) {
			conv_run(CONVERSATION_WITH_KING);
			conv_export_value(global[king_status] == KING_WITH_SOUL ? 1 : 0);
			conv_export_value(1);
			local->invoked_ring = true;
		} else {
			new_room = 110;
		}

		goto handled;
	}

	if (player_parse(words_look, 0) || player_parse(words_look_at, 0)) {
		if (player_parse(words_cave_floor, 0)) {
			text_show(11602);
			goto handled;
		} else if (player_parse(words_cave_wall, 0)) {
			text_show(11603);
			goto handled;
		} else if (player_parse(229, 0)) {
			text_show(global[king_status] == KING_CAPTIVE ? 11604 : 11609);
			goto handled;
		} else if (player_parse(words_door_to_north, 0)) {
			text_show(11605);
			goto handled;
		} else if (player_parse(words_doorway_to_south, 0)) {
			text_show(11606);
			goto handled;
		} else if (player_parse(words_king, 0) && global[king_status] == KING_CAPTIVE) {
			text_show(11607);
			goto handled;
		} else if (player_parse(words_king, 0) && global[king_status] == KING_OFF_ICE) {
			text_show(11610);
			goto handled;
		} else if (player_parse(words_king, 0) && conv_control.running != 13) {
			text_show(11611);
			goto handled;
		}
	}

	if (player_parse(words_gaze_into, words_crystal_ball, 0) && global[king_status] == KING_CAPTIVE) {
		text_show(11615);
		goto handled;
	}
	if (player_parse(words_gaze_into, words_crystal_ball, 0) && global[king_status] == KING_WITHOUT_SOUL) {
		text_show(11616);
		goto handled;
	}

	if (player_parse(words_close, words_door_to_north, 0)) {
		text_show(42);
		goto handled;
	}

	if ((local->bear_status != FIRST_TIME_BEAR || local->bear_status != IS_A_BEAR_AGAIN) &&
			!player_parse(words_look, 0) &&
			!player_parse(words_take, 0) &&
			!player_parse(words_push, 0) &&
			!player_parse(words_open, 0) &&
			!player_parse(words_put, 0) &&
			!player_parse(words_talk_to, 0) &&
			!player_parse(words_give, 0) &&
			!player_parse(words_pull, 0) &&
			!player_parse(words_close, 0) &&
			!player_parse(words_throw, 0) &&
			!player_parse(words_swim_to, 0) &&
			!player_parse(words_swim_towards, 0)) {
		text_show(32);
		goto handled;
	}

	if (player_parse(words_heal, words_king) && global[king_status] != KING_WITH_SOUL) {
		text_show(11622);
		goto handled;
	}

	if (player_parse(words_sword, words_attack, words_king, 0) ||
			player_parse(words_sword, words_carve_up, words_king, 0) ||
			player_parse(words_sword, words_thrust, words_king, 0)) {
		if (global[king_status] == KING_CAPTIVE) {
			text_show(11621);
			goto handled;
		} else {
			goto done;
		}
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_116_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.temp);
	s.syncAsSint16LE(scratch.kingsicle_id);
	s.syncAsSint16LE(scratch.bear_status);
	s.syncAsSint16LE(scratch.animation_running);
	s.syncAsSint16LE(scratch.current_frame);
	s.syncAsSint16LE(scratch.just_melted);
	s.syncAsSint16LE(scratch.king_frame);
	s.syncAsSint16LE(scratch.king_action);
	s.syncAsSint16LE(scratch.king_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.lift_frame);
	s.syncAsSint16LE(scratch.lift_action);
	s.syncAsSint16LE(scratch.anim_2_running);
	s.syncAsSint16LE(scratch.invoked_ring);
}

void room_116_preload() {
	room_init_code_pointer = room_116_init;
	room_pre_parser_code_pointer = room_116_pre_parser;
	room_parser_code_pointer = room_116_parser;
	room_daemon_code_pointer = room_116_daemon;

	if (kernel.teleported_in) {
		inter_give_to_player(key_crown);
		inter_give_to_player(crystal_ball);
		inter_give_to_player(shifter_ring);
		inter_give_to_player(soul_egg);
		inter_give_to_player(amulet);
		global[player_persona] = PLAYER_IS_PID;
	}

	section_1_walker();
	section_1_interface();

	if ((global[king_status] != KING_CAPTIVE) && (!global[king_is_in_stairwell])) {
		kernel_initial_variant = 1;
	}

	vocab_make_active(words_king);
	vocab_make_active(words_walk_to);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
