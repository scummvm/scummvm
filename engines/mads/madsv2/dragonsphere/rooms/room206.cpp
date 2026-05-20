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
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section2.h"
#include "mads/madsv2/dragonsphere/rooms/room206.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];

	int16 left_worker_frame;
	int16 left_worker_talk_count;
	int16 anim_0_running;

	int16 right_worker_frame;
	int16 right_worker_talk_count;
	int16 anim_1_running;

	int16 lady_frame;
	int16 lady_action;
	int16 lady_talk_count;
	int16 anim_2_running;

	int16 end_frame;
	int16 anim_3_running;

	int16 prevent;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_take                 0

#define ROOM_206_YOU_TALK       60
#define ROOM_206_END_GAME       70

#define PLAYER_X_FROM_205       167
#define PLAYER_Y_FROM_205       146

#define FREEZE                  0
#define TALK                    1
#define GET_UP                  2
#define SIT_DOWN                3
#define GIVE                    4

#define CONV_53_PID             53

#define SHIFTER_X               219
#define SHIFTER_Y               111


static void handle_anim_left_worker() {
	int left_worker_reset_frame;
	int it;

	if (kernel_anim[aa[0]].frame != local->left_worker_frame) {
		local->left_worker_frame = kernel_anim[aa[0]].frame;
		left_worker_reset_frame = -1;

		switch (local->left_worker_frame) {
		case 1:  /* end of freeze */
		case 2:  /* end of freeze */
		case 3:  /* end of freeze */
		case 10: /* end of rake   */
		case 33: /* end of rake   */
			++local->left_worker_talk_count;

			if (local->left_worker_talk_count > imath_random(3, 4)) {
				it = imath_random(1, 3);
				local->left_worker_talk_count = 0;

				switch (it) {
					case 1:
						left_worker_reset_frame = 3;
						local->left_worker_talk_count = 30;
						break;

					case 2:
						left_worker_reset_frame = 10;
						break;

					case 3:
						left_worker_reset_frame = imath_random(0, 2);
						break;
				}

			} else {
				left_worker_reset_frame = imath_random(0, 2);
			}
			break;

		case 20: /* end of rake   */
		case 21: /* end of freeze */
		case 22: /* end of freeze */
			++ local->left_worker_talk_count;

			if (local->left_worker_talk_count > imath_random(3, 4)) {
				it = imath_random(1, 2);
				local->left_worker_talk_count = 0;

				switch (it) {
				case 1:
					left_worker_reset_frame = 22;
					local->left_worker_talk_count = 30;
					break;

				case 3:
					left_worker_reset_frame = imath_random(20, 21);
					break;
				}

			} else {
				left_worker_reset_frame = imath_random(20, 21);
			}
			break;
		}

		if (left_worker_reset_frame >= 0) {
			kernel_reset_animation(aa[0], left_worker_reset_frame);
			local->left_worker_frame = left_worker_reset_frame;
		}
	}
}

static void handle_anim_right_worker() {
	int right_worker_reset_frame;
	int it;

	if (kernel_anim[aa[1]].frame != local->right_worker_frame) {
		local->right_worker_frame = kernel_anim[aa[1]].frame;
		right_worker_reset_frame = -1;

		switch (local->right_worker_frame) {
		case 1:  /* end of freeze */
		case 9:  /* end of rake   */
			++ local->right_worker_talk_count;
			if (local->right_worker_talk_count > imath_random(3, 4)) {
				it = imath_random(1, 3);
				switch (it) {
					case 1:
						right_worker_reset_frame = 1;  /* rake */
						break;

					default:
						right_worker_reset_frame = 0;  /* freeze */
						break;
				}

			} else if (local->right_worker_frame == 1) {
				right_worker_reset_frame = 0;

			} else {
				right_worker_reset_frame = 1;
			}
			break;
		}

		if (right_worker_reset_frame >= 0) {
			kernel_reset_animation(aa[1], right_worker_reset_frame);
			local->right_worker_frame = right_worker_reset_frame;
		}
	}
}

static void handle_anim_lady() {
	int lady_reset_frame;
	int it;

	if (kernel_anim[aa[2]].frame != local->lady_frame) {
		local->lady_frame = kernel_anim[aa[2]].frame;
		lady_reset_frame = -1;

		switch (local->lady_frame) {
		case 1:  /* end of freeze */
		case 2:  /* end of freeze */
		case 3:  /* end of freeze */
		case 12: /* end of talk   */
		case 56: /* end of sit    */
			switch (local->lady_action) {
			case FREEZE:
				if (local->lady_frame > 3) {
					local->lady_frame = 1;
				}

				++ local->lady_talk_count;
				if (local->lady_talk_count > imath_random(3, 6)) {
					if (local->lady_frame == 1) {
						lady_reset_frame = imath_random(0, 1);
					} else if (local->lady_frame == 2) {
						lady_reset_frame = imath_random(0, 2);
					} else if (local->lady_frame == 3) {
						lady_reset_frame = imath_random(1, 2);
					}
					local->lady_talk_count = 0;

				} else {
					lady_reset_frame = local->lady_frame - 1;
				}
				break;

			case GET_UP:
				lady_reset_frame   = 12;
				local->lady_action = FREEZE;
				break;

			case TALK:
				lady_reset_frame = 3;
				break;
			}
			break;

		case 7:  /* end of come to talk */
		case 8:  /* end of talk         */
		case 9:  /* end of talk         */
		case 10: /* end of talk         */
			switch (local->lady_action) {
			case TALK:
				lady_reset_frame = imath_random(7, 9);
				++ local->lady_talk_count;
				if (local->lady_talk_count > 15) {
					local->lady_action     = FREEZE;
					local->lady_talk_count = 0;
					lady_reset_frame       = 10; /* make lady shut up */
				}
				break;

			default:
				lady_reset_frame = 10; /* make lady shut up */
				break;
			}
			break;

		case 41:  /* lady has hand almost all the way out */
			player.walker_visible = false;
			seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 7, 0, 0, 2);
			kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 4, 1);
			kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 2);
			kernel_seq_depth(seq[fx_take], 3);
			kernel_seq_player(seq[fx_take], true);
			kernel_seq_loc(seq[fx_take], player.x + 5, player.y + 4);
			kernel_seq_range(seq[fx_take], KERNEL_FIRST, KERNEL_LAST);
			break;

		case 42:
			if (local->lady_action == GIVE) {
				lady_reset_frame = 41;
			}
			break;

		case 55:  /* almost end of sit down */
		case 19:  /* almost end of get up   */
			conv_release();
			break;

		case 20:  /* end of get up          */
		case 44:  /* end of give            */
		case 25:  /* end of freeze and talk */
		case 33:  /* end of other talk      */
			switch (local->lady_action) {
			case FREEZE:
				lady_reset_frame = 24;
				break;

			case SIT_DOWN:
				lady_reset_frame   = 47;
				local->lady_action = FREEZE;
				break;

			case GIVE:
				lady_reset_frame = 33;
				break;

			case TALK:
				it = imath_random(1, 3);
				switch (it) {
					case 1: lady_reset_frame = 20; break;
					case 2: lady_reset_frame = 25; break;
					case 3: lady_reset_frame = 44; break;
				}
				break;
			}
			break;

		case 21:  /* end of come to talk and talk */
		case 22:  /* end of talk                  */
		case 23:  /* end of talk                  */
		case 24:  /* end of talk                  */
			switch (local->lady_action) {
			case TALK:
				lady_reset_frame = imath_random(20, 23);
				++ local->lady_talk_count;
				if (local->lady_talk_count > 15) {
					local->lady_action     = FREEZE;
					local->lady_talk_count = 0;
					lady_reset_frame       = 24; /* make lady shut up */
				}
				break;

			default:
				lady_reset_frame = 24; /* make lady shut up */
				break;
			}
			break;

		case 45:  /* end of come to talk and talk */
		case 46:  /* end of talk                  */
		case 47:  /* end of talk                  */
			switch (local->lady_action) {
			case TALK:
				lady_reset_frame = imath_random(44, 46);
				++ local->lady_talk_count;
				if (local->lady_talk_count > 15) {
					local->lady_action     = FREEZE;
					local->lady_talk_count = 0;
					lady_reset_frame       = 24; /* make lady shut up */
				}
				break;

			default:
				lady_reset_frame = 24; /* make lady shut up */
				break;
			}
			break;

		case 28:  /* end of come to talk */
		case 29:  /* end of talk         */
		case 30:  /* end of talk         */
			switch (local->lady_action) {
			case TALK:
				lady_reset_frame = imath_random(28, 29);
				++ local->lady_talk_count;
				if (local->lady_talk_count > 15) {
					local->lady_action     = FREEZE;
					local->lady_talk_count = 0;
					lady_reset_frame       = 30; /* make lady shut up */
				}
				break;

			default:
				lady_reset_frame = 30; /* make lady shut up */
				break;
			}
			break;
		}

		if (lady_reset_frame >= 0) {
			kernel_reset_animation(aa[2], lady_reset_frame);
			local->lady_frame = lady_reset_frame;
		}
	}
}

static void handle_anim_end() {
	int end_reset_frame;

	if (kernel_anim[aa[3]].frame != local->end_frame) {
		local->end_frame = kernel_anim[aa[3]].frame;
		end_reset_frame = -1;

		switch (local->end_frame) {
		case 84:
			kernel_timing_trigger(12, ROOM_206_END_GAME + 1);
			break;

		case 85:
			end_reset_frame = 84;
			break;
		}

		if (end_reset_frame >= 0) {
			kernel_reset_animation(aa[3], end_reset_frame);
			local->end_frame = end_reset_frame;
		}
	}
}

static void room_206_init() {
	if (global[end_of_game]) {
		player.commands_allowed = false;
		player.walker_visible   = false;

		viewing_at_y = ((video_y - display_y) >> 1);
		kernel_init_dialog();
		kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

		local->anim_2_running = false;

		local->left_worker_talk_count  = 0;
		local->right_worker_talk_count = 0;
		local->lady_talk_count         = 0;
		local->prevent                 = false;

		aa[0]                 = kernel_run_animation(kernel_name('w', 1), 0);
		local->anim_0_running = true;

		aa[1]                 = kernel_run_animation(kernel_name('w', 2), 0);
		local->anim_1_running = true;

		aa[3]                 = kernel_run_animation(kernel_name('e', 1), 0);
		local->anim_3_running = true;

	} else {
		kernel_flip_hotspot(words_Greta, false);
		kernel_flip_hotspot_loc(words_shifter, false, SHIFTER_X, SHIFTER_Y);

		if (global[talked_to_greta]) {
			kernel_flip_hotspot(words_Greta, true);
		} else {
			kernel_flip_hotspot(words_shifter, true);
		}

		if (previous_room != KERNEL_RESTORING_GAME) {
			local->anim_0_running = false;
			local->anim_1_running = false;
			local->anim_2_running = false;
			local->anim_3_running = false;
		}

		conv_get(CONV_53_PID);

		ss[fx_take] = kernel_load_series("*PDRD_9", false);

		local->left_worker_talk_count  = 0;
		local->right_worker_talk_count = 0;
		local->lady_talk_count         = 0;
		local->prevent                 = false;

		aa[0]                 = kernel_run_animation(kernel_name('w', 1), 0);
		local->anim_0_running = true;

		aa[1]                 = kernel_run_animation(kernel_name('w', 2), 0);
		local->anim_1_running = true;

		aa[2]                 = kernel_run_animation(kernel_name('l', 1), 0);
		local->lady_action    = FREEZE;
		local->anim_2_running = true;

		if (previous_room == 205 || previous_room != KERNEL_RESTORING_GAME) {
			player.x      = PLAYER_X_FROM_205;
			player.y      = PLAYER_Y_FROM_205;
			player.facing = FACING_NORTHEAST;
		}
	}

	section_2_music();
}

static void room_206_daemon() {
	int score;

	if (local->anim_0_running) {
		handle_anim_left_worker();
	}

	if (local->anim_1_running) {
		handle_anim_right_worker();
	}

	if (local->anim_2_running) {
		handle_anim_lady();
	}

	if (local->anim_3_running) {
		handle_anim_end();
	}

	switch (kernel.trigger) {
		case 1:
			if (local->prevent) {
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(shifter_ring);
				object_examine(shifter_ring, 20613, 0);
				local->lady_action = FREEZE;
				++ global[player_score];
			}
			local->prevent = true;
			break;

		case 2:
			player.walker_visible   = true;
			player.commands_allowed = true;
			local->prevent          = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);
			conv_release();
			break;
	}

	if (kernel.trigger == ROOM_206_END_GAME + 1) {

		score = global[player_score];
		if (score > 250) score = 250;

		text_index[0] = score;
		text_index[1] = 250;

		if (score <= 25) {
			text_index[2] = 1;
		} else if (score <= 50) {
			text_index[2] = 2;
		} else if (score <= 75) {
			text_index[2] = 3;
		} else if (score <= 100) {
			text_index[2] = 4;
		} else if (score <= 150) {
			text_index[2] = 5;
		} else if (score <= 200) {
			text_index[2] = 6;
		} else if (score <= 249) {
			text_index[2] = 7;
		} else if (score <= 250) {
			text_index[2] = 8;
		} else {
			text_index[2] = 9;
		}

		text_show(99);
		game.going = false;
		win_status = 1;
	}
}

static void room_206_pre_parser() {
}

static void process_conv_lady() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	if (player_verb == conv053_thanks_only || player_verb == conv053_restart_only) {
		global[perform_displacements] = true;
	}

	if (player_verb == conv053_story_b_b) {
		conv_hold();
		you_trig_flag      = true;
		me_trig_flag       = true;
		local->lady_action = GET_UP;
	}

	if (player_verb == conv053_thanks_b_b) {
		conv_hold();
		you_trig_flag      = true;
		me_trig_flag       = true;
		local->lady_action = SIT_DOWN;
	}

	if (player_verb == conv053_focus_b_b) {
		conv_hold();
		you_trig_flag      = true;
		me_trig_flag       = true;
		local->lady_action = GIVE;
	}

	if (kernel.trigger == ROOM_206_YOU_TALK) {
		local->lady_action = TALK;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_206_YOU_TALK);
	}

	local->lady_talk_count = 0;
}

static void room_206_parser() {
	if (conv_control.running == CONV_53_PID) {
		process_conv_lady();
		goto handled;
	}

	if (player_said_2(talk_to, shifter) ||
	    player_said_2(talk_to, Greta)) {

		if (inter_point_x < 140) {
			text_show(20615);

		} else {
			global[perform_displacements] = false;
			conv_run(CONV_53_PID);

			global[talked_to_greta] = true;
			kernel_flip_hotspot(words_Greta, true);
			kernel_flip_hotspot_loc(words_shifter, false, SHIFTER_X, SHIFTER_Y);
		}
		goto handled;
	}

	if (player_said_2(walk_down, path_to_south)) {
		new_room = 205;
		goto handled;
	}

	if (player.look_around) {
		text_show(20601);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(wrecked_bridge)) {
			text_show(20602);
			goto handled;
		}

		if (player_said_1(wrecked_shack)) {
			if (global[talked_to_greta]) {
				text_show(20610);
			} else {
				text_show(20604);
			}
			goto handled;
		}

		if (player_said_1(river)) {
			text_show(20605);
			goto handled;
		}

		if (player_said_1(shack)) {
			text_show(20606);
			goto handled;
		}

		if (player_said_1(path_to_south)) {
			text_show(20607);
			goto handled;
		}

		if (player_said_1(shifter_village)) {
			text_show(20608);
			goto handled;
		}

		if (player_said_1(shifter) || player_said_1(Greta)) {
			if (inter_point_x < 140) {
				text_show(20614);
			} else if (global[talked_to_greta]) {
				text_show(20611);
			} else {
				text_show(20609);
			}
			goto handled;
		}
	}

	if (player_said_2(throw, shifter) && inter_point_x < 140) {
		text_show(20616);
		goto handled;
	}

	if (player_said_2(close, wrecked_bridge)) {
		text_show(20603);
		goto handled;
	}

	if (player_said_2(give, Greta)) {
		text_show(20612);
		goto handled;
	}

	if (player_said_1(shack)) {
		if ((inter_point_x < 182) ||
		   (inter_point_x > 166 && inter_point_y < 54)) {
			text_show(20618);
			goto handled;
		}
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_206_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.left_worker_frame);
	s.syncAsSint16LE(scratch.left_worker_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.right_worker_frame);
	s.syncAsSint16LE(scratch.right_worker_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.lady_frame);
	s.syncAsSint16LE(scratch.lady_action);
	s.syncAsSint16LE(scratch.lady_talk_count);
	s.syncAsSint16LE(scratch.anim_2_running);
	s.syncAsSint16LE(scratch.end_frame);
	s.syncAsSint16LE(scratch.anim_3_running);
	s.syncAsSint16LE(scratch.prevent);
}

void room_206_preload() {
	room_init_code_pointer       = room_206_init;
	room_pre_parser_code_pointer = room_206_pre_parser;
	room_parser_code_pointer     = room_206_parser;
	room_daemon_code_pointer     = room_206_daemon;

	if (kernel.teleported_in) {
		global[player_persona] = PLAYER_IS_PID;
	}

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
