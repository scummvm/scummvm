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
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/room408.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 shaman_1_frame;       /* animation frame being held for shaman_1 stuff */
	int16 shaman_1_action;      /* Type of action to run for shaman_1 animation */
	int16 shaman_1_talk_count;  /* counter for shaman_1 talking */
	int16 anim_0_running;

	int16 shaman_2_frame;       /* animation frame being held for shaman_2 stuff */
	int16 shaman_2_action;      /* Type of action to run for shaman_2 animation */
	int16 shaman_2_talk_count;  /* counter for shaman_2 talking */
	int16 anim_1_running;

	int16 face_1_on;
	int16 face_2_on;
	int16 face_3_on;
	int16 face_4_on;
	int16 face_5_on;
	int16 face_6_on;
	int16 face_7_on;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation



/* ========================= Sprite Series =================== */

#define fx_pid                  0       /* rm408b0 */
#define fx_face_1               1       /* rm408z0 */
#define fx_face_2               2       /* rm408z1 */
#define fx_face_3               3       /* rm408z2 */
#define fx_face_4               4       /* rm408z3 */
#define fx_face_5               5       /* rm408z4 */
#define fx_face_6               6       /* rm408z5 */
#define fx_face_7               7       /* rm408z6 */


/* ======================== Triggers ========================= */

#define ROOM_408_ME_TALK        60
#define ROOM_408_YOU_TALK       62

/* ========================= Other Macros ==================== */

#define PLAYER_X_FROM_405       213
#define PLAYER_Y_FROM_405       115

#define PLAYER_X_FROM_409       330
#define PLAYER_Y_FROM_409       115
#define WALK_TO_X_FROM_409      310
#define WALK_TO_Y_FROM_409      115

#define SHUT_UP                 0
#define TALK                    1
#define LEAVE                   2

#define CONV_46_PID             46

static Scratch scratch;


static void room_408_init() {
	global[perform_displacements] = true;

	global[move_direction_409] = true;

	local->face_1_on = false;
	local->face_2_on = false;
	local->face_3_on = false;
	local->face_4_on = false;
	local->face_5_on = false;
	local->face_6_on = false;
	local->face_7_on = false;

	conv_get(CONV_46_PID);

	ss[fx_pid] = kernel_load_series(kernel_name('b', 0), false);
	ss[fx_face_1] = kernel_load_series(kernel_name('z', 0), false);
	ss[fx_face_2] = kernel_load_series(kernel_name('z', 1), false);
	ss[fx_face_3] = kernel_load_series(kernel_name('z', 2), false);
	ss[fx_face_4] = kernel_load_series(kernel_name('z', 3), false);
	ss[fx_face_5] = kernel_load_series(kernel_name('z', 4), false);
	ss[fx_face_6] = kernel_load_series(kernel_name('z', 5), false);
	ss[fx_face_7] = kernel_load_series(kernel_name('z', 6), false);

	aa[0] = kernel_run_animation(kernel_name('s', 1), 0);
	local->anim_0_running = true;
	local->shaman_1_action = SHUT_UP;

	aa[1] = kernel_run_animation(kernel_name('s', 2), 0);
	local->anim_1_running = true;
	local->shaman_2_action = SHUT_UP;

	if (conv_restore_running == CONV_46_PID) {
		player.x = PLAYER_X_FROM_405;
		player.y = PLAYER_Y_FROM_405;
		player.facing = FACING_NORTHWEST;

		kernel_reset_animation(aa[0], 41);
		kernel_reset_animation(aa[1], 8);

		conv_run(CONV_46_PID);
		conv_export_value(player_has(soul_egg));

	} else if (previous_room == 405) {

		player.x = PLAYER_X_FROM_405;
		player.y = PLAYER_Y_FROM_405;
		player.facing = FACING_NORTHWEST;
		player.commands_allowed = false;
		player.walker_visible = false;

		seq[fx_pid] = kernel_seq_forward(ss[fx_pid], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_pid], 2);
		kernel_seq_range(seq[fx_pid], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_pid], KERNEL_TRIGGER_EXPIRE, 0, 1);

		global[perform_displacements] = false;

	} else if (previous_room == 409 || previous_room != KERNEL_RESTORING_GAME) {

		kernel_reset_animation(aa[0], 41);
		kernel_reset_animation(aa[1], 8);

		player_first_walk(PLAYER_X_FROM_409, PLAYER_Y_FROM_409, FACING_WEST,
			WALK_TO_X_FROM_409, WALK_TO_Y_FROM_409, FACING_WEST, true);

	} else if (previous_room == KERNEL_RESTORING_GAME) {
		kernel_reset_animation(aa[0], 41);
		kernel_reset_animation(aa[1], 8);
	}

	if (previous_room == 411) {
		text_show(41150);
	}

	section_4_music();
}

static void handle_animation_shaman_1() {
	int shaman_1_reset_frame;
	int it;

	if (kernel_anim[aa[0]].frame != local->shaman_1_frame) {
		local->shaman_1_frame = kernel_anim[aa[0]].frame;
		shaman_1_reset_frame = -1;

		switch (local->shaman_1_frame) {

		case 40: /* end of invisible */
			shaman_1_reset_frame = 39;
			break;

		case 7:  /* end of talk */
		case 8:  /* end of talk */
		case 9:  /* end of talk */
			if (local->shaman_1_action == TALK) {
				shaman_1_reset_frame = imath_random(6, 8);
				++local->shaman_1_talk_count;
				if (local->shaman_1_talk_count > 24) {
					local->shaman_1_action = SHUT_UP;
					local->shaman_1_talk_count = 0;
					shaman_1_reset_frame = 40; /* make shaman_1 shut up */
				}

			} else {
				shaman_1_reset_frame = 40; /* make shaman_1 shut up */
			}
			break;

		case 6:  /* end of fade in     */
		case 20: /* end of talk        */
		case 33: /* end of talk        */
		case 41: /* end of freeze      */
			switch (local->shaman_1_action) {
			case SHUT_UP:
				shaman_1_reset_frame = 40;
				break;

			case TALK:
				it = imath_random(1, 3);
				if (it == 1) {
					shaman_1_reset_frame = 9;
					local->shaman_1_action = SHUT_UP;
				} else if (it == 2) {
					shaman_1_reset_frame = 20;
					local->shaman_1_action = SHUT_UP;
				} else {
					shaman_1_reset_frame = 6;
				}
				break;

			case LEAVE:
				shaman_1_reset_frame = 33;
				break;
			}
			break;
		}

		if (shaman_1_reset_frame >= 0) {
			kernel_reset_animation(aa[0], shaman_1_reset_frame);
			local->shaman_1_frame = shaman_1_reset_frame;
		}
	}
}

static void handle_animation_shaman_2() {
	int shaman_2_reset_frame;

	if (kernel_anim[aa[1]].frame != local->shaman_2_frame) {
		local->shaman_2_frame = kernel_anim[aa[1]].frame;
		shaman_2_reset_frame = -1;

		switch (local->shaman_2_frame) {
		case 18:
			shaman_2_reset_frame = 17;
			break;

		case 7:  /* end of look around   */
		case 8:  /* end of look around   */
		case 9:  /* end of look around   */
		case 10: /* end of look around   */
			switch (local->shaman_2_action) {
			case SHUT_UP:
				if (local->shaman_2_frame >= 11) {
					local->shaman_2_frame = 7;
				}

				++local->shaman_2_talk_count;
				if (local->shaman_2_talk_count > imath_random(6, 9)) {
					if (local->shaman_2_frame == 7) {
						shaman_2_reset_frame = imath_random(6, 7);
					} else if (local->shaman_2_frame == 8) {
						shaman_2_reset_frame = imath_random(6, 8);
					} else if (local->shaman_2_frame == 9) {
						shaman_2_reset_frame = imath_random(7, 9);
					} else if (local->shaman_2_frame == 10) {
						shaman_2_reset_frame = imath_random(8, 9);
					}
					local->shaman_2_talk_count = 0;

				} else {
					shaman_2_reset_frame = local->shaman_2_frame - 1;
				}
				break;

			case LEAVE:
				shaman_2_reset_frame = 10;
				break;
			}
			break;
		}

		if (shaman_2_reset_frame >= 0) {
			kernel_reset_animation(aa[1], shaman_2_reset_frame);
			local->shaman_2_frame = shaman_2_reset_frame;
		}
	}
}

static void room_408_daemon() {
	if (local->anim_0_running) {
		handle_animation_shaman_1();
	}

	if (local->anim_1_running) {
		handle_animation_shaman_2();
	}

	if (kernel.trigger == 1) {
		player.walker_visible = true;
		player.commands_allowed = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_pid]);
		conv_run(CONV_46_PID);
		conv_export_value(player_has(soul_egg));
	}

	if (!local->face_1_on && imath_random(1, 600) == 1) {
		seq[fx_face_1] = kernel_seq_forward(ss[fx_face_1], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_1], 1);
		kernel_seq_range(seq[fx_face_1], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_face_1], KERNEL_TRIGGER_EXPIRE, 0, 10);
		local->face_1_on = true;
	}

	if (!local->face_2_on && imath_random(1, 600) == 1) {
		seq[fx_face_2] = kernel_seq_forward(ss[fx_face_2], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_2], 1);
		kernel_seq_range(seq[fx_face_2], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_face_2], KERNEL_TRIGGER_EXPIRE, 0, 11);
		local->face_2_on = true;
	}

	if (!local->face_3_on && imath_random(1, 600) == 1) {
		seq[fx_face_3] = kernel_seq_forward(ss[fx_face_3], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_3], 1);
		kernel_seq_range(seq[fx_face_3], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_face_3], KERNEL_TRIGGER_EXPIRE, 0, 12);
		local->face_3_on = true;
	}

	if (!local->face_4_on && imath_random(1, 600) == 1) {
		seq[fx_face_4] = kernel_seq_forward(ss[fx_face_4], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_4], 1);
		kernel_seq_range(seq[fx_face_4], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_face_4], KERNEL_TRIGGER_EXPIRE, 0, 13);
		local->face_4_on = true;
	}

	if (!local->face_5_on && imath_random(1, 600) == 1) {
		seq[fx_face_5] = kernel_seq_forward(ss[fx_face_5], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_5], 1);
		kernel_seq_range(seq[fx_face_5], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_face_5], KERNEL_TRIGGER_EXPIRE, 0, 14);
		local->face_5_on = true;
	}

	if (!local->face_6_on && imath_random(1, 600) == 1) {
		seq[fx_face_6] = kernel_seq_forward(ss[fx_face_6], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_6], 1);
		kernel_seq_range(seq[fx_face_6], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_face_6], KERNEL_TRIGGER_EXPIRE, 0, 15);
		local->face_6_on = true;
	}

	if (!local->face_7_on && imath_random(1, 600) == 1) {
		seq[fx_face_7] = kernel_seq_forward(ss[fx_face_7], false, 9, 0, 0, 1);
		kernel_seq_depth(seq[fx_face_7], 1);
		kernel_seq_range(seq[fx_face_7], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_face_7], KERNEL_TRIGGER_EXPIRE, 0, 16);
		local->face_7_on = true;
	}

	switch (kernel.trigger) {
	case 10:
		local->face_1_on = false;
		break;

	case 11:
		local->face_2_on = false;
		break;

	case 12:
		local->face_3_on = false;
		break;

	case 13:
		local->face_4_on = false;
		break;

	case 14:
		local->face_5_on = false;
		break;

	case 15:
		local->face_6_on = false;
		break;

	case 16:
		local->face_7_on = false;
		break;
	}
}

static void process_conv_pid() {
	int you_trig_flag = false;
	int me_trig_flag = false;

	if (player_verb == conv046_exit_b_b) {
		*conv_my_next_start = conv046_advicelp;
		conv_abort();
		if (!kernel.trigger) {
			local->shaman_1_action = LEAVE;
			local->shaman_2_action = LEAVE;

			player.walker_visible = false;
			player.commands_allowed = false;

			seq[fx_pid] = kernel_seq_backward(ss[fx_pid], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_pid], 1);
			kernel_seq_range(seq[fx_pid], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_pid], KERNEL_TRIGGER_EXPIRE, 0, 2);
			kernel_synch(KERNEL_SERIES, seq[fx_pid], KERNEL_PLAYER, 0);
		}
	}

	if (player_verb == conv046_advicelp_bynow ||
		player_verb == conv046_advicelp_seeya) {
		global[perform_displacements] = true;
	}

	if (kernel.trigger == ROOM_408_YOU_TALK) {
		local->shaman_1_action = TALK;
	}

	if (kernel.trigger == ROOM_408_ME_TALK) {
		local->shaman_1_action = SHUT_UP;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_408_YOU_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_408_ME_TALK);
	} /* if me_trig_flag == true, then a me trigger is called from above, not here. */

	local->shaman_1_talk_count = 0;
}

static void room_408_pre_parser() {
	if (player_said_2(walk_down, path_to_east)) {
		player.walk_off_edge_to_room = 409;
	}
}

static void room_408_parser() {
	if (kernel.trigger == 2) {
		global[perform_displacements] = true;
		kernel_timing_trigger(TWO_SECONDS, 3);
		goto handled;
	}

	if (kernel.trigger == 3) {
		new_room = 405;
		goto handled;
	}

	if (conv_control.running == CONV_46_PID) {
		process_conv_pid();
		goto handled;
	}

	if (player_said_2(talk_to, shaman)) {
		conv_run(CONV_46_PID);
		conv_export_value(player_has(soul_egg));
		global[perform_displacements] = false;
		goto handled;
	}

	if (player.look_around) {
		text_show(40801);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(spirit_tree)) {
			text_show(40802);
			goto handled;
		}

		if (player_said_1(path_to_east)) {
			text_show(40803);
			goto handled;
		}

		if (player_said_1(ledge)) {
			text_show(40804);
			goto handled;
		}

		if (player_said_1(spirit_plane)) {
			text_show(40805);
			goto handled;
		}

		if (player_said_1(shaman)) {
			text_show(40806);
			goto handled;
		}
	}

	if (player_said_1(spirit_tree)) {
		if (player_said_1(push) ||
			player_said_1(pull) ||
			player_said_1(open) ||
			player_said_1(close)) {
			text_show(40807);
			goto handled;
		}
	}

	if (player_said_2(talk_to, spirit_tree)) {
		text_show(40808);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_408_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.shaman_1_frame);
	s.syncAsSint16LE(scratch.shaman_1_action);
	s.syncAsSint16LE(scratch.shaman_1_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.shaman_2_frame);
	s.syncAsSint16LE(scratch.shaman_2_action);
	s.syncAsSint16LE(scratch.shaman_2_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.face_1_on);
	s.syncAsSint16LE(scratch.face_2_on);
	s.syncAsSint16LE(scratch.face_3_on);
	s.syncAsSint16LE(scratch.face_4_on);
	s.syncAsSint16LE(scratch.face_5_on);
	s.syncAsSint16LE(scratch.face_6_on);
	s.syncAsSint16LE(scratch.face_7_on);
}

void room_408_preload() {
	room_init_code_pointer = room_408_init;
	room_pre_parser_code_pointer = room_408_pre_parser;
	room_parser_code_pointer = room_408_parser;
	room_daemon_code_pointer = room_408_daemon;

	if (kernel.teleported_in) {
		global[player_persona] = PLAYER_IS_PID;
	}

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
