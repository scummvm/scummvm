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
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section5.h"
#include "mads/madsv2/dragonsphere/rooms/room506.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[5];

	int16 pid_frame;
	int16 pid_action;
	int16 pid_freeze_count;
	int16 anim_0_running;

	int16 king_frame;
	int16 king_action;
	int16 king_talk_count;
	int16 anim_1_running;
	int16 anim_2_running;
	int16 anim_3_running;

	int16 shak_frame;
	int16 shak_action;
	int16 shak_count;
	int16 anim_4_running;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_feather_0   0
#define fx_feather_1   1
#define fx_feather_2   2
#define fx_test_2      3

#define ROOM_506_DONE_KING_ANIM  60
#define ROOM_506_YOU_TALK        65
#define ROOM_506_ME_TALK         67

#define PID_FREEZE      0
#define PID_CLIMB_UP    1
#define PID_CLIMB_DOWN  2
#define PID_REACH       3

#define KING_FREEZE     0
#define KING_REACH      1
#define KING_BLOW       2
#define KING_TALK       3
#define KING_CLIMB_UP   4
#define KING_CLIMB_DOWN 5

#define FEATHER_X       139
#define FEATHER_Y       76

#define SHAK_SHUT_UP    0
#define SHAK_TALK       1
#define SHAK_LEAVE      2

#define CONV_SHAK       28


static void handle_animation_pid() {
	int pid_reset_frame;
	int random;

	if (kernel_anim[aa[0]].frame != local->pid_frame) {
		local->pid_frame = kernel_anim[aa[0]].frame;
		pid_reset_frame  = -1;

		switch (local->pid_frame) {

		case 206:
			kernel_seq_delete(seq[fx_feather_1]);
			kernel_flip_hotspot_loc(words_feathers, false, FEATHER_X, FEATHER_Y);
			sound_play(N_TakeObjectSnd);
			inter_give_to_player(feathers);
			object_examine(feathers, 50611, 0);
			player.commands_allowed = true;
			++global[player_score];
			break;

		case 192:
			new_room = 508;
			break;

		case 431:
			new_room = 502;
			break;

		case 346:
		case 85:
			player.commands_allowed = true;
			break;

		case 347:
		case 86:
		case 219:
		case 224:

			switch (local->pid_action) {

			case PID_FREEZE:
				++local->pid_freeze_count;
				if (local->pid_freeze_count > imath_random(30, 45)) {
					local->pid_freeze_count = 0;
					if (imath_random(1, 2) == 1) {
						pid_reset_frame = 85;
					} else {
						pid_reset_frame = 219;
					}
				} else {
					pid_reset_frame = local->pid_frame - 1;
				}
				break;

			case PID_CLIMB_UP:
				pid_reset_frame = 86;
				break;

			case PID_CLIMB_DOWN:
				pid_reset_frame = 347;
				break;

			case PID_REACH:
				local->pid_action = PID_FREEZE;
				pid_reset_frame   = 192;
				break;
			}
			break;

		case 222:
		case 239:

			switch (local->pid_action) {

			case PID_FREEZE:
				++local->pid_freeze_count;
				if (local->pid_freeze_count > imath_random(30, 45)) {
					local->pid_freeze_count = 0;
					random = imath_random(1, 2) == 1;
					if (random == 1) {
						pid_reset_frame = 221;
					} else if (random == 2) {
						pid_reset_frame = 222;
					} else {
						pid_reset_frame = 224;
					}
				} else {
					pid_reset_frame = local->pid_frame - 1;
				}
				break;

			case PID_CLIMB_DOWN:
			case PID_CLIMB_UP:
				pid_reset_frame = 222;
				break;

			case PID_REACH:
				pid_reset_frame = 224;
				break;
			}
			break;

		case 232:

			switch (local->pid_action) {

			case PID_FREEZE:
				++local->pid_freeze_count;
				if (local->pid_freeze_count > imath_random(30, 45)) {
					local->pid_freeze_count = 0;
					if (imath_random(1, 2) == 1) {
						pid_reset_frame = 231;
					} else {
						pid_reset_frame = 232;
					}
				} else {
					pid_reset_frame = local->pid_frame - 1;
				}
				break;

			case PID_CLIMB_DOWN:
			case PID_CLIMB_UP:
				pid_reset_frame = 232;
				break;

			case PID_REACH:
				local->pid_action = PID_FREEZE;
				pid_reset_frame   = 203;
				break;
			}
			break;
		}

		if (pid_reset_frame >= 0) {
			kernel_reset_animation(aa[0], pid_reset_frame);
			local->pid_frame = pid_reset_frame;
		}
	}
}


static void handle_animation_climbing_lower() {
	int king_reset_frame;

	if (kernel_anim[aa[1]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[1]].frame;
		king_reset_frame  = -1;

		switch (local->king_frame) {

		case 105:
			kernel_abort_animation(aa[1]);
			aa[3] = kernel_run_animation(kernel_name('k', 3), 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
			player.commands_allowed = true;
			king_reset_frame        = -1;
			local->anim_1_running   = false;
			local->anim_3_running   = true;
			local->king_action      = KING_FREEZE;
			local->king_frame       = kernel_anim[aa[3]].frame;
			break;

		case 211:
			new_room = 502;
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[1], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}


static void handle_animation_climbing_upper() {
	int king_reset_frame;

	if (kernel_anim[aa[2]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[2]].frame;
		king_reset_frame  = -1;

		switch (local->king_frame) {

		case 200:
			kernel_abort_animation(aa[2]);
			aa[3] = kernel_run_animation(kernel_name('k', 3), 0);
			kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);
			player.commands_allowed = true;
			king_reset_frame        = -1;
			local->anim_2_running   = false;
			local->anim_3_running   = true;
			local->king_action      = KING_FREEZE;
			local->king_frame       = kernel_anim[aa[3]].frame;
			break;

		case 100:
			if (!player_has_been_in_room(507)) {
				new_room = 507;
			} else {
				new_room = 508;
			}
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[2], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}

static void handle_animation_king() {
	int king_reset_frame;

	if (kernel_anim[aa[3]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[3]].frame;
		king_reset_frame  = -1;

		switch (local->king_frame) {
		case 18:
			kernel_seq_delete(seq[fx_feather_1]);
			kernel_flip_hotspot_loc(words_feathers, false, FEATHER_X, FEATHER_Y);
			sound_play(N_TakeObjectSnd);
			inter_give_to_player(feathers);
			object_examine(feathers, 821, 0);
			player.commands_allowed = true;
			++global[player_score];
			break;

		case 7:
			if (global[shak_status] == SHAK_MET) {
				text_show(50621);
				player.commands_allowed = true;
			} else {
				global[shak_status] = SHAK_MET;
				text_show(50613);
				aa[4] = kernel_run_animation(kernel_name('s', 1), 0);
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
				local->anim_4_running = true;
				local->shak_action    = SHAK_SHUT_UP;
			}
			break;

		case 1:
		case 8:
		case 21:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
			switch (local->king_action) {
			case KING_FREEZE:
				king_reset_frame = 0;
				break;

			case KING_REACH:
				king_reset_frame   = 14;
				local->king_action = KING_FREEZE;
				break;

			case KING_BLOW:
				king_reset_frame   = 1;
				local->king_action = KING_FREEZE;
				break;

			case KING_CLIMB_DOWN:
				kernel_abort_animation(aa[3]);
				aa[1] = kernel_run_animation(kernel_name('k', 1), 0);
				kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);
				kernel_reset_animation(aa[1], 106);
				local->anim_3_running = false;
				local->anim_1_running = true;
				king_reset_frame      = -1;
				break;

			case KING_CLIMB_UP:
				kernel_abort_animation(aa[3]);
				aa[2] = kernel_run_animation(kernel_name('k', 2), 0);
				kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
				local->anim_3_running = false;
				local->anim_2_running = true;
				king_reset_frame      = -1;
				break;

			case KING_TALK:
				king_reset_frame = imath_random(9, 13);
				++local->king_talk_count;
				if (local->king_talk_count > 15) {
					local->king_action = KING_FREEZE;
					king_reset_frame   = 0;
				}
				break;
			}
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[3], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}

static void handle_animation_shak() {
	int shak_reset_frame;
	int random;

	if (kernel_anim[aa[4]].frame != local->shak_frame) {
		local->shak_frame = kernel_anim[aa[4]].frame;
		shak_reset_frame  = -1;

		switch (local->shak_frame) {
		case 148:
			player.commands_allowed = true;
			break;

		case 84:
			conv_run(CONV_SHAK);
			if (player_has(red_powerstone) || player_has(yellow_powerstone) ||
			    player_has(blue_powerstone)) {
				conv_export_value(true);
			} else {
				conv_export_value(false);
			}
			break;

		case 85:
		case 86:
		case 87:
		case 88:
		case 105:
			switch (local->shak_action) {
			case SHAK_SHUT_UP:
				if (imath_random(1, 100) == 1) {
					shak_reset_frame = 88;
				} else {
					shak_reset_frame = 84;
				}
				break;

			case SHAK_LEAVE:
				shak_reset_frame = 106;
				break;

			case SHAK_TALK:
				shak_reset_frame = imath_random(85, 87);
				++local->shak_count;
				if (local->shak_count > 15) {
					local->shak_action = SHAK_SHUT_UP;
					shak_reset_frame   = 84;
				}
				break;
			}
			break;

		case 97:
		case 98:
			if (local->shak_action == SHAK_LEAVE) {
				shak_reset_frame = 98;

			} else {
				++local->shak_count;
				if (local->shak_count > imath_random(8, 15)) {
					local->shak_count = 0;
					random = imath_random(1, 3);
					if (random == 1) {
						shak_reset_frame = 96;
					} else if (random == 2) {
						shak_reset_frame = 97;
					} else {
						shak_reset_frame = 98;
					}
				} else {
					shak_reset_frame = local->shak_frame - 1;
				}
			}
			break;
		}

		if (shak_reset_frame >= 0) {
			kernel_reset_animation(aa[4], shak_reset_frame);
			local->shak_frame = shak_reset_frame;
		}
	}
}

static void room_506_init() {
	conv_get(CONV_SHAK);

	kernel_load_series(kernel_name('a', 4), false);

	local->pid_freeze_count = 0;
	local->king_talk_count  = 0;
	local->shak_count       = 0;
	player.walker_visible   = false;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
		local->anim_4_running = false;
	}

	ss[fx_feather_0]  = kernel_load_series(kernel_name('p', 0), false);
	ss[fx_feather_1]  = kernel_load_series(kernel_name('p', 1), false);
	ss[fx_feather_2]  = kernel_load_series(kernel_name('p', 2), false);

	seq[fx_feather_0] = kernel_seq_stamp(ss[fx_feather_0], false, KERNEL_FIRST);
	kernel_seq_depth(seq[fx_feather_0], 1);
	seq[fx_feather_2] = kernel_seq_stamp(ss[fx_feather_2], false, KERNEL_FIRST);
	kernel_seq_depth(seq[fx_feather_2], 1);

	if (object_is_here(feathers)) {
		seq[fx_feather_1] = kernel_seq_stamp(ss[fx_feather_1], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_feather_1], 1);
	} else {
		kernel_flip_hotspot_loc(words_feathers, false, FEATHER_X, FEATHER_Y);
	}

	if (global[player_persona] == PLAYER_IS_PID) {
		aa[0]                 = kernel_run_animation(kernel_name('p', 1), 0);
		local->anim_0_running = true;
		local->pid_action     = PID_FREEZE;
	}

	if (previous_room == 502) {
		player.commands_allowed = false;
		if (global[player_persona] == PLAYER_IS_KING) {
			aa[1]                 = kernel_run_animation(kernel_name('k', 1), 0);
			local->anim_1_running = true;
		}

	} else if (previous_room == 508) {
		if (global[pid_just_died]) {
			aa[3]                 = kernel_run_animation(kernel_name('k', 3), 0);
			local->anim_3_running = true;
			local->king_action    = KING_FREEZE;
			global[pid_just_died] = false;

		} else {
			player.commands_allowed = false;
			if (global[player_persona] == PLAYER_IS_PID) {
				kernel_reset_animation(aa[0], 241);
			} else {
				aa[2] = kernel_run_animation(kernel_name('k', 2), 0);
				kernel_reset_animation(aa[2], 102);
				local->anim_2_running = true;
			}
		}

	} else if (previous_room == KERNEL_RESTORING_GAME) {
		if (global[player_persona] == PLAYER_IS_PID) {
			kernel_reset_animation(aa[0], 86);

		} else {
			aa[3]                 = kernel_run_animation(kernel_name('k', 3), 0);
			local->anim_3_running = true;
			local->king_action    = KING_FREEZE;

			if (conv_restore_running == CONV_SHAK) {
				aa[4]                   = kernel_run_animation(kernel_name('s', 1), 0);
				local->anim_4_running   = true;
				local->shak_action      = SHAK_SHUT_UP;
				player.commands_allowed = false;
				kernel_reset_animation(aa[4], 84);
				conv_run(CONV_SHAK);
				if (player_has(red_powerstone) || player_has(yellow_powerstone) ||
				    player_has(blue_powerstone)) {
					conv_export_value(true);
				} else {
					conv_export_value(false);
				}
			}
		}

	} else {
		player.commands_allowed = false;
	}

	section_5_music();
}

static void room_506_daemon() {
	if (local->anim_0_running) {
		handle_animation_pid();
	}

	if (local->anim_1_running) {
		handle_animation_climbing_lower();
	}

	if (local->anim_2_running) {
		handle_animation_climbing_upper();
	}

	if (local->anim_3_running) {
		handle_animation_king();
	}

	if (local->anim_4_running) {
		handle_animation_shak();
	}
}

static void process_conversation_shak() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	if (player_verb == conv028_exit_b_b) {
		local->shak_action = SHAK_LEAVE;
		me_trig_flag       = true;
		you_trig_flag      = true;
	}

	if (player_verb == conv028_didnot_innocent) {
		global[shak_506_angry] = true;
	}

	switch (kernel.trigger) {
	case ROOM_506_ME_TALK:
		if (local->shak_action != SHAK_LEAVE) {
			local->shak_action = SHAK_SHUT_UP;
			local->king_action = KING_TALK;
		}
		break;

	case ROOM_506_YOU_TALK:
		if (local->shak_action != SHAK_LEAVE) {
			local->shak_action = SHAK_TALK;
			local->king_action = KING_FREEZE;
		}
		break;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_506_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_506_ME_TALK);
	}

	local->shak_count      = 0;
	local->king_talk_count = 0;
}

static void room_506_pre_parser() {
	player.need_to_walk = false;
}

static void room_506_parser() {
	if (conv_control.running == CONV_SHAK) {
		process_conversation_shak();
		player.command_ready = false;
		return;
	}

	if (player_said_2(climb_up, rough_stone)) {
		if (global[player_persona] == PLAYER_IS_PID) {
			local->pid_action = PID_CLIMB_UP;
		} else {
			local->king_action = KING_CLIMB_UP;
		}
		player.commands_allowed = false;
		player.command_ready = false;
		return;
	}

	if (player_said_2(climb_down, rough_stone)) {
		if (global[player_persona] == PLAYER_IS_PID) {
			local->pid_action = PID_CLIMB_DOWN;
		} else {
			local->king_action = KING_CLIMB_DOWN;
		}
		player.commands_allowed = false;
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, feathers)) {
		if (object_is_here(feathers)) {
			if (inter_point_x < 133) {
				if (game.difficulty == EASY_MODE) {
					text_show(50617);
				} else {
					text_show(50620);
				}
			} else {
				player.commands_allowed = false;
				if (global[player_persona] == PLAYER_IS_KING) {
					local->king_action = KING_REACH;
				} else {
					local->pid_action = PID_REACH;
				}
			}
		} else if (inter_point_x < 133) {
			text_show(50618);
		}
		player.command_ready = false;
		return;
	}

	if (player.look_around) {
		text_show(50601);
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(sky)) {
			text_show(50602);
			player.command_ready = false;
			return;
		}

		if (player_said_1(rough_stone)) {
			if (inter_point_y < 74) {
				text_show(50604);
			} else {
				text_show(50603);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(nest)) {
			if (global[shak_status] == SHAK_NEVER_MET) {
				text_show(50605);
			} else {
				text_show(50606);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(mountainside)) {
			text_show(50609);
			player.command_ready = false;
			return;
		}

		if (player_said_1(feathers)) {
			if (player.main_object_source == STROKE_INTERFACE) {
				if (inter_point_x < 110) {
					text_show(50610);
				} else {
					text_show(50619);
				}
				player.command_ready = false;
				return;
			}
		}

		if (player_said_1(shak)) {
			text_show(50612);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_1(nest)) {
		if (player_said_1(take) || player_said_1(pull)) {
			text_show(50607);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(put, nest)) {
		text_show(50608);
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, feathers)) {
		text_show(50611);
		player.command_ready = false;
		return;
	}

	if (player_said_2(make_noise, birdcall)) {
		if (global[player_persona] == PLAYER_IS_PID) {
			if (global[shak_status] == SHAK_MET) {
				text_show(50621);
				player.command_ready = false;
				return;
			}

		} else {

			if (!(global[player_score_flags] & SCORE_MAKE_NOISE_BIRDCALL)) {
				global[player_score_flags] = global[player_score_flags] | SCORE_MAKE_NOISE_BIRDCALL;
				global[player_score] += 4;
			}

			player.commands_allowed = false;
			local->king_action = KING_BLOW;
			player.command_ready = false;
			return;
		}
	}

	if (player_said_3(throw, dates, shak) ||
	    player_said_3(give, dates, shak)) {
		text_show(50614);
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, bottle_of_flies) ||
	    player_said_3(give, bottle_of_flies, shak)) {
		text_show(50615);
		player.command_ready = false;
		return;
	}

	if (player_said_3(give, feathers, shak)) {
		text_show(50616);
		player.command_ready = false;
		return;
	}
}

void room_506_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(local->pid_frame);
	s.syncAsSint16LE(local->pid_action);
	s.syncAsSint16LE(local->pid_freeze_count);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->king_frame);
	s.syncAsSint16LE(local->king_action);
	s.syncAsSint16LE(local->king_talk_count);
	s.syncAsSint16LE(local->anim_1_running);
	s.syncAsSint16LE(local->anim_2_running);
	s.syncAsSint16LE(local->anim_3_running);
	s.syncAsSint16LE(local->shak_frame);
	s.syncAsSint16LE(local->shak_action);
	s.syncAsSint16LE(local->shak_count);
	s.syncAsSint16LE(local->anim_4_running);
}

void room_506_preload() {
	room_init_code_pointer       = room_506_init;
	room_pre_parser_code_pointer = room_506_pre_parser;
	room_parser_code_pointer     = room_506_parser;
	room_daemon_code_pointer     = room_506_daemon;

	section_5_walker();
	section_5_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
