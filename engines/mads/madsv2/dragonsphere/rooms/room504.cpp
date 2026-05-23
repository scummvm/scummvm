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
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/quotes.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section5.h"
#include "mads/madsv2/dragonsphere/rooms/room504.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[5];

	int16 lani_pid_frame;
	int16 lani_pid_action;
	int16 lani_pid_talk_count;
	int16 anim_0_running;

	int16 poking_frame;
	int16 poking_action;
	int16 poking_talk_count;
	int16 anim_1_running;

	int16 heal_frame;
	int16 heal_action;
	int16 heal_talk_count;
	int16 anim_2_running;

	int16 reveal_frame;
	int16 reveal_action;
	int16 reveal_talk_count;
	int16 anim_3_running;

	int16 tom_talk_frame;
	int16 tom_talk_action;
	int16 tom_talk_talk_count;
	int16 anim_4_running;

	int16 ready_to_heal;
	int16 prevent;
	int16 dynamic_hs;
	int16 pid_is_kneeling;
	int16 purpose_for_healing;
	int16 got_it;

	char  line_1[70];
	char  line_2[70];
	char  line_3[70];
	int16 working_on_line;
	int16 quote_id;

	int16 shit;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_fire           0
#define fx_look_at_heal   1
#define fx_lani           2

#define ROOM_504_DOOR_CLOSES    60
#define ROOM_504_YOU_TALK       65
#define ROOM_504_ME_TALK        67
#define ROOM_504_DONE_HEALING   73
#define ROOM_504_TALK_TO_HERMIT 77
#define ROOM_504_RUN_POEM_CONV  90
#define ROOM_504_END_POEM       92

#define NOTHING           0
#define BUNDLE            1
#define HEAL              2
#define HEAL_CRYSTAL      3
#define POEM              4
#define NEW               5

#define PLAYER_X_FROM_503  114
#define PLAYER_Y_FROM_503  147

#define LANI_X             184
#define LANI_Y             147

#define CONV_24_KING            24
#define CONV_25_KING_AFTER_FALL 25
#define CONV_26_PID_PRE_HEAL    26
#define CONV_27_PID_HEAL        27
#define CONV_42_POEM            42

#define LANI_DEAD          0
#define PID_KNEEL          1
#define PID_LOOK_TOM       2
#define PID_TALK_TO_LANI   3
#define LANI_TALK          4
#define BOTH_SHUT_UP       5
#define LANI_KISS          6
#define PID_HEAL           7
#define GIVE               8
#define PID_GET_UP         9
#define LANI_SIT_UP        10
#define LANI_LAY_DOWN      11
#define PID_POEM           12

#define TOM_SHUT_UP        0
#define TOM_TALK           1
#define TOM_LOOK_AT_HEAL   2
#define TOM_REVEAL         3
#define TOM_HIDE           4

#define WALK_TO_HERMIT_X   143
#define WALK_TO_HERMIT_Y   134


static void handle_animation_lani_pid() {
	int lani_pid_reset_frame;
	int id;

	if (kernel_anim[aa[0]].frame != local->lani_pid_frame) {
		local->lani_pid_frame = kernel_anim[aa[0]].frame;
		lani_pid_reset_frame  = -1;

		switch (local->lani_pid_frame) {
		case 99:
			if (!player_has(piece_of_paper)) {
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(piece_of_paper);
				object_examine(piece_of_paper, 845, 0);
				global[player_score] += 1;
				local->shit = true;

			} else if (!player_has(amulet)) {
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(amulet);
				object_examine(amulet, 819, 0);
				global[player_score] += 2;
			}
			break;

		case 100:
			if (local->shit) {
				local->shit = false;
				sound_play(N_KissMusic);
			}
			break;

		case 165:
			lani_pid_reset_frame = 164;
			break;

		case 114:
		case 122:
			if (local->lani_pid_frame == 122) {
				if (global[llanie_status] == IS_SAVED) {
					*conv_my_next_start = conv027_restart;
				}
				conv_abort();
				kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
				player.walker_visible  = true;
				local->pid_is_kneeling = false;
				local->lani_pid_action = LANI_DEAD;
				local->tom_talk_action = TOM_HIDE;
				player.ready_to_walk   = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
			}

			switch (local->lani_pid_action) {
			case LANI_DEAD:
				lani_pid_reset_frame = 113;
				break;

			case PID_HEAL:
			case PID_KNEEL:
			case BOTH_SHUT_UP:
				local->pid_is_kneeling = true;
				player.walker_visible  = false;
				lani_pid_reset_frame   = 20;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
				break;
			}
			break;

		case 32:
			lani_pid_reset_frame = imath_random(31, 32);
			++local->lani_pid_talk_count;
			if (local->lani_pid_talk_count > 15) {
				local->lani_pid_talk_count = 0;
				lani_pid_reset_frame       = 34;
			}
			break;

		case 33:
			lani_pid_reset_frame = imath_random(31, 33);
			++local->lani_pid_talk_count;
			if (local->lani_pid_talk_count > 15) {
				local->lani_pid_talk_count = 0;
				lani_pid_reset_frame       = 34;
			}
			break;

		case 34:
			lani_pid_reset_frame = imath_random(32, 33);
			++local->lani_pid_talk_count;
			if (local->lani_pid_talk_count > 15) {
				local->lani_pid_talk_count = 0;
				lani_pid_reset_frame       = 34;
			}
			break;

		case 159:
		case 39:
		case 28:
		case 31:
		case 107:
		case 108:
		case 109:
		case 110:
		case 113:
			if (local->purpose_for_healing != POEM) {
				if (local->lani_pid_frame == 39) {
					if ((!global[said_poem_in_504] || !global[put_bundle_on_llanie_504]) ||
					    (local->purpose_for_healing < HEAL || local->purpose_for_healing == NEW)) {

						kernel_seq_delete(seq[fx_look_at_heal]);

						aa[1] = kernel_run_animation(kernel_name('t', 1), 0);
						kernel_reset_animation(aa[1], 5);

						local->poking_action     = TOM_SHUT_UP;
						local->anim_1_running    = true;
						local->prevent           = false;
						local->lani_pid_action   = BOTH_SHUT_UP;
						player.commands_allowed  = true;

						id = kernel_add_dynamic(words_hermit, words_walk_to, SYNTAX_MASC_NOT_PROPER, KERNEL_NONE,
						                        0, 0, 0, 0);
						kernel_dynamic_hot[id].prep = PREP_ON;
						kernel_dynamic_walk(id, WALK_TO_HERMIT_X, WALK_TO_HERMIT_Y, FACING_NORTHWEST);
						kernel_dynamic_anim(id, aa[1], 0);

						if ((local->purpose_for_healing == NOTHING) && (!global[tried_to_heal_llanie_504])) {
							global[tried_to_heal_llanie_504] = true;
							local->lani_pid_action           = PID_LOOK_TOM;
							conv_run(CONV_26_PID_PRE_HEAL);
							conv_export_value(global[put_bundle_on_llanie_504]);
							conv_export_value(global[said_poem_in_504]);
							conv_export_value(global[tried_to_heal_llanie_504]);
							conv_export_value(player_has(shifter_ring));
						}

					} else if (global[llanie_status] != IS_SAVED) {

						global[tried_to_heal_llanie_504] = true;
						global[llanie_status]            = IS_SAVED;
						local->lani_pid_action           = LANI_TALK;
						local->poking_action             = TOM_SHUT_UP;
						local->anim_1_running            = true;
						local->prevent                   = false;
						global[player_score]            += 5;

						kernel_seq_delete(seq[fx_look_at_heal]);

						aa[1] = kernel_run_animation(kernel_name('t', 1), 0);
						kernel_reset_animation(aa[1], 5);

						kernel_init_dialog();
						kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
						conv_run(CONV_27_PID_HEAL);
						conv_export_value(player_has(amulet));
					}
				}
			}

			switch (local->lani_pid_action) {
			case PID_TALK_TO_LANI:
				lani_pid_reset_frame   = 31;
				local->lani_pid_action = BOTH_SHUT_UP;
				break;

			case LANI_TALK:
				lani_pid_reset_frame = imath_random(107, 109);
				++local->lani_pid_talk_count;
				if (local->lani_pid_talk_count > 15) {
					local->lani_pid_action     = BOTH_SHUT_UP;
					local->lani_pid_talk_count = 0;
					lani_pid_reset_frame       = 110;
				}
				break;

			case PID_LOOK_TOM:
				lani_pid_reset_frame = 28;
				break;

			case LANI_SIT_UP:
				lani_pid_reset_frame = 70;
				break;

			case PID_HEAL:
				if (player_said_2(talk_to, Llanie)) {
					local->lani_pid_talk_count = 0;
					lani_pid_reset_frame       = 31;
					local->lani_pid_action     = BOTH_SHUT_UP;

				} else {
					aa[2]                  = kernel_run_animation(kernel_name('h', 1), ROOM_504_DONE_HEALING);
					local->anim_2_running  = true;
					lani_pid_reset_frame   = 113;
					local->lani_pid_action = LANI_DEAD;
				}
				break;

			case PID_GET_UP:
				lani_pid_reset_frame = 115;
				break;

			case PID_POEM:
				local->lani_pid_talk_count = 0;
				lani_pid_reset_frame       = 31;
				local->lani_pid_action     = BOTH_SHUT_UP;
				break;

			default:
				if (local->lani_pid_action == BOTH_SHUT_UP) {
					if (player_said_2(talk_to, Llanie) && !global[said_poem_in_504]) {
						text_show(50425);
						kernel_init_dialog();
						kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
						player.commands_allowed = false;
						conv_run(CONV_42_POEM);
						conv_export_value(0);
						conv_export_value(game.difficulty);
						conv_show_boxes = false;
					}
				}
				lani_pid_reset_frame = 27;
				break;
			}
			break;

		case 30:
			if (local->lani_pid_action == PID_LOOK_TOM) {
				lani_pid_reset_frame = 29;
			} else {
				lani_pid_reset_frame = 30;
			}
			break;

		case 93:
		case 103:
			conv_release();
			break;

		case 72:
		case 73:
		case 74:
		case 75:
		case 76:
		case 77:
		case 94:
		case 167:
		case 104:
		case 160:
		case 161:
		case 162:
			if (local->lani_pid_frame == 72) {
				local->lani_pid_action = PID_LOOK_TOM;
			}

			switch (local->lani_pid_action) {
			case LANI_TALK:
				lani_pid_reset_frame = imath_random(72, 76);
				++local->lani_pid_talk_count;
				if (local->lani_pid_talk_count > 15) {
					local->lani_pid_action     = BOTH_SHUT_UP;
					local->lani_pid_talk_count = 0;
					lani_pid_reset_frame       = 103;
				}
				break;

			case LANI_KISS:
				lani_pid_reset_frame   = 78;
				local->lani_pid_action = BOTH_SHUT_UP;
				break;

			case LANI_LAY_DOWN:
				lani_pid_reset_frame   = 104;
				local->lani_pid_action = BOTH_SHUT_UP;
				break;

			case PID_LOOK_TOM:
				lani_pid_reset_frame = 166;
				break;

			case GIVE:
				lani_pid_reset_frame   = 94;
				local->lani_pid_action = BOTH_SHUT_UP;
				break;

			case PID_TALK_TO_LANI:
				lani_pid_reset_frame = imath_random(159, 161);
				++local->lani_pid_talk_count;
				if (local->lani_pid_talk_count > 15) {
					local->lani_pid_action     = BOTH_SHUT_UP;
					local->lani_pid_talk_count = 0;
					lani_pid_reset_frame       = 103;
				}
				break;

			case PID_GET_UP:
				lani_pid_reset_frame = 104;
				break;

			default:
				lani_pid_reset_frame = 103;
				break;
			}
			break;
		}

		if (lani_pid_reset_frame >= 0) {
			kernel_reset_animation(aa[0], lani_pid_reset_frame);
			local->lani_pid_frame = lani_pid_reset_frame;
		}
	}
}

static void handle_animation_tom_poking() {
	int poking_reset_frame;
	int random;

	if (kernel_anim[aa[1]].frame != local->poking_frame) {
		local->poking_frame = kernel_anim[aa[1]].frame;
		poking_reset_frame  = -1;

		switch (local->poking_frame) {
		case 1:
		case 44:
			switch (local->poking_action) {
			case TOM_SHUT_UP:
				++local->poking_talk_count;
				if (local->poking_talk_count > imath_random(20, 30)) {
					local->poking_talk_count = 0;
					poking_reset_frame       = imath_random(0, 1);
				} else {
					poking_reset_frame = 0;
				}
				break;

			default:
				poking_reset_frame = 1;
				break;
			}
			break;

		case 3:
		case 42:
		case 53:
		case 23:
			switch (local->poking_action) {
			case TOM_SHUT_UP:
				++local->poking_talk_count;
				if (local->poking_talk_count > imath_random(12, 20)) {
					random = imath_random(1, 5);
					switch (random) {
					case 1: poking_reset_frame = 42;  break;
					case 2: poking_reset_frame = 2;   break;
					case 3: poking_reset_frame = 19;  break;
					case 4: poking_reset_frame = 23;  break;
					case 5: poking_reset_frame = 3;   break;
					}
					local->poking_talk_count = 0;

					if (local->prevent) {
						poking_reset_frame = 3;
					}

					if (local->pid_is_kneeling) {
						if (poking_reset_frame == 23) {
							poking_reset_frame = 3;
						}
					}

				} else {
					poking_reset_frame = 2;
				}
				break;

			case TOM_REVEAL:
				*conv_my_next_start = conv027_quickie_only;
				conv_abort();

				kernel_abort_animation(aa[1]);
				aa[3] = kernel_run_animation(kernel_name('t', 2), 0);
				kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);

				local->tom_talk_action = TOM_TALK;
				local->anim_3_running  = true;
				local->anim_1_running  = false;
				poking_reset_frame     = -1;
				break;

			default:
				poking_reset_frame = 3;
				break;
			}
			break;

		case 5:
		case 6:
		case 7:
		case 19:
			switch (local->poking_action) {
			case TOM_SHUT_UP:
				++local->poking_talk_count;
				if (local->poking_talk_count > imath_random(15, 30)) {
					local->poking_talk_count = 0;
					random = imath_random(1, 2);
					if (random == 1) {
						poking_reset_frame = 4;
					} else {
						poking_reset_frame = 39;
					}
				} else {
					poking_reset_frame = 4;
				}
				break;

			case TOM_REVEAL:
				poking_reset_frame = 39;
				break;

			case TOM_TALK:
				poking_reset_frame = imath_random(4, 6);
				++local->poking_talk_count;
				if (local->poking_talk_count > 17) {
					local->poking_action     = TOM_SHUT_UP;
					local->poking_talk_count = 0;
					poking_reset_frame       = 39;
				}
				break;

			case TOM_LOOK_AT_HEAL:
				if (!local->ready_to_heal) {
					poking_reset_frame = 4;

				} else {

					kernel_abort_animation(aa[1]);
					local->anim_1_running  = false;
					poking_reset_frame     = -1;
					local->ready_to_heal   = false;
					player.walker_visible  = false;
					local->lani_pid_action = PID_HEAL;

					seq[fx_look_at_heal] = kernel_seq_stamp(ss[fx_look_at_heal], false, KERNEL_FIRST);
					kernel_seq_depth(seq[fx_look_at_heal], 14);
					kernel_synch(KERNEL_SERIES, seq[fx_look_at_heal], KERNEL_NOW, 0);

					if (local->pid_is_kneeling) {
						kernel_reset_animation(aa[0], 27);
					} else {
						kernel_reset_animation(aa[0], 20);
					}
					local->pid_is_kneeling = true;
					kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
				}
				break;
			}
			break;

		case 12:
		case 13:
		case 14:
			switch (local->poking_action) {
			case TOM_TALK:
				poking_reset_frame = imath_random(11, 13);
				++local->poking_talk_count;
				if (local->poking_talk_count > 17) {
					local->poking_action     = TOM_SHUT_UP;
					local->poking_talk_count = 0;
					poking_reset_frame       = 14;
				}
				break;

			default:
				poking_reset_frame = 14;
				break;
			}
			break;

		case 21:
			switch (local->poking_action) {
			case TOM_SHUT_UP:
				++local->poking_talk_count;
				if (local->poking_talk_count > imath_random(5, 9)) {
					local->poking_talk_count = 0;
					poking_reset_frame       = imath_random(20, 21);
				} else {
					poking_reset_frame = 20;
				}
				break;

			default:
				poking_reset_frame = 21;
				break;
			}
			break;

		case 37:
			switch (local->poking_action) {
			case TOM_SHUT_UP:
				++local->poking_talk_count;
				if (local->poking_talk_count > imath_random(10, 14)) {
					random = imath_random(1, 5);
					switch (random) {
					case 1:
						poking_reset_frame       = 36;
						local->poking_talk_count = 0;
						break;
					case 2:
						poking_reset_frame       = 44;
						local->poking_talk_count = 0;
						break;
					default:
						poking_reset_frame       = 31;
						local->poking_talk_count = 40;
						break;
					}
				} else {
					poking_reset_frame = 36;
				}
				break;

			default:
				poking_reset_frame = 44;
				break;
			}
			break;
		}

		if (poking_reset_frame >= 0) {
			kernel_reset_animation(aa[1], poking_reset_frame);
			local->poking_frame = poking_reset_frame;
		}
	}
}

static void handle_animation_reveal() {
	int reveal_reset_frame;
	int id;

	if (kernel_anim[aa[3]].frame != local->reveal_frame) {
		local->reveal_frame = kernel_anim[aa[3]].frame;
		reveal_reset_frame  = -1;

		switch (local->reveal_frame) {
		case 7:
			local->lani_pid_action = LANI_SIT_UP;
			break;

		case 24:
			kernel_abort_animation(aa[3]);
			aa[4] = kernel_run_animation(kernel_name('t', 3), 0);
			kernel_reset_animation(aa[4], 25);
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);

			local->poking_action  = TOM_SHUT_UP;
			local->anim_4_running = true;
			local->anim_3_running = false;
			reveal_reset_frame    = -1;

			conv_run(CONV_27_PID_HEAL);
			conv_export_value(player_has(amulet));
			break;

		case 63:
			kernel_abort_animation(aa[3]);
			aa[1] = kernel_run_animation(kernel_name('t', 1), 0);
			kernel_reset_animation(aa[1], 3);
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);

			local->poking_action    = TOM_SHUT_UP;
			local->anim_1_running   = true;
			local->anim_3_running   = false;
			reveal_reset_frame      = -1;
			player.commands_allowed = true;

			id = kernel_add_dynamic(words_Ner_Tom, words_walk_to, SYNTAX_SINGULAR_MASC, KERNEL_NONE,
			                        0, 0, 0, 0);
			kernel_dynamic_hot[id].prep = PREP_ON;
			kernel_dynamic_walk(id, WALK_TO_HERMIT_X, WALK_TO_HERMIT_Y, FACING_NORTHWEST);
			kernel_dynamic_anim(id, aa[1], 0);

			if (global[llanie_status] != IS_SAVED) {
				conv_run(CONV_26_PID_PRE_HEAL);
				conv_export_value(global[put_bundle_on_llanie_504]);
				conv_export_value(global[said_poem_in_504]);
				conv_export_value(global[tried_to_heal_llanie_504]);
				conv_export_value(player_has(shifter_ring));
			}
			break;
		}

		if (reveal_reset_frame >= 0) {
			kernel_reset_animation(aa[3], reveal_reset_frame);
			local->reveal_frame = reveal_reset_frame;
		}
	}
}

static void handle_animation_tom_talk() {
	int tom_talk_reset_frame;
	int random;

	if (kernel_anim[aa[4]].frame != local->tom_talk_frame) {
		local->tom_talk_frame = kernel_anim[aa[4]].frame;
		tom_talk_reset_frame  = -1;

		switch (local->tom_talk_frame) {

		case 25:
		case 38:
		case 39:
			switch (local->tom_talk_action) {
			case TOM_SHUT_UP:
				++local->tom_talk_talk_count;
				if (local->tom_talk_talk_count > imath_random(12, 20)) {
					local->tom_talk_talk_count = 0;
					random = imath_random(1, 3);
					switch (random) {
					case 1: tom_talk_reset_frame = 24;  break;
					case 2: tom_talk_reset_frame = 37;  break;
					case 3: tom_talk_reset_frame = 38;  break;
					}
				} else {
					tom_talk_reset_frame = local->tom_talk_frame - 1;
				}
				break;

			case TOM_TALK:
				tom_talk_reset_frame = 25;
				break;

			case TOM_HIDE:
				kernel_abort_animation(aa[4]);
				aa[3] = kernel_run_animation(kernel_name('t', 2), 0);
				kernel_reset_animation(aa[3], 40);
				kernel_synch(KERNEL_ANIM, aa[3], KERNEL_NOW, 0);

				local->poking_action  = TOM_SHUT_UP;
				local->anim_3_running = true;
				local->anim_4_running = false;
				tom_talk_reset_frame  = -1;
				break;
			}
			break;

		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
			switch (local->tom_talk_action) {
			case TOM_TALK:
				tom_talk_reset_frame = imath_random(27, 32);
				++local->tom_talk_talk_count;
				if (local->tom_talk_talk_count > 17) {
					local->tom_talk_action     = TOM_SHUT_UP;
					local->tom_talk_talk_count = 0;
					tom_talk_reset_frame       = 33;
				}
				break;

			case TOM_SHUT_UP:
				tom_talk_reset_frame = 33;
				break;
			}
			break;
		}

		if (tom_talk_reset_frame >= 0) {
			kernel_reset_animation(aa[4], tom_talk_reset_frame);
			local->tom_talk_frame = tom_talk_reset_frame;
		}
	}
}

static void handle_animation_heal() {
	int heal_reset_frame;

	if (kernel_anim[aa[2]].frame != local->heal_frame) {
		local->heal_frame = kernel_anim[aa[2]].frame;
		heal_reset_frame  = -1;

		switch (local->heal_frame) {
		case 3:
			local->heal_talk_count = 0;
			if (local->purpose_for_healing < HEAL || local->purpose_for_healing == NEW) {
				heal_reset_frame = 31;
			}
			if (local->purpose_for_healing == BUNDLE ||
			    local->purpose_for_healing == NEW) {
				sound_play(N_FarEchos);
			}
			break;

		case 4:
			kernel_reset_animation(aa[0], 164);
			break;

		case 32:
			if (local->purpose_for_healing == BUNDLE) {
				inter_move_object(medicine_bundle, 504);
				text_show(50427);
				global[player_score] += 2;
				sound_play(N_TenseMusic);

			} else if (local->purpose_for_healing == NEW) {
				inter_move_object(new_bundle, 504);
				text_show(50427);
				global[player_score] += 2;
				sound_play(N_TenseMusic);

			} else if (local->purpose_for_healing == POEM) {
				/* no action */

			} else if (global[put_bundle_on_llanie_504] &&
			           global[said_poem_in_504]) {
				if (local->purpose_for_healing == HEAL) {
					text_show(50428);
				} else if (local->purpose_for_healing == HEAL_CRYSTAL) {
					text_show(50439);
					inter_move_object(crystal_ball, NOWHERE);
					text_show(970);
				}

			} else {
				++local->heal_talk_count;
				if (local->heal_talk_count < 10) {
					heal_reset_frame = 31;

				} else if (player_said_1(crystal_ball)) {
					text_show(50438);

				} else if (game.difficulty == HARD_MODE) {
					text_show(50430);

				} else {
					text_show(50429);
				}
			}
			break;
		}

		if (heal_reset_frame >= 0) {
			kernel_reset_animation(aa[2], heal_reset_frame);
			local->heal_frame = heal_reset_frame;
		}
	}
}

static void room_504_init() {
	int id;

	if (!player.been_here_before) ++global[dragon_high_scene];
	if (!player.been_here_before) ++global[player_score];

	if (global[monster_is_dead]) {
		global[found_lani_504] = true;
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running      = false;
		local->anim_1_running      = false;
		local->anim_2_running      = false;
		local->anim_3_running      = false;
		local->anim_4_running      = false;
		local->ready_to_heal       = false;
		local->prevent             = false;
		local->purpose_for_healing = false;
		local->pid_is_kneeling     = false;
		local->line_1[0]           = '\0';
		local->line_2[0]           = '\0';
		local->line_3[0]           = '\0';
		local->working_on_line     = 1;
		local->got_it              = 0;
		local->shit                = false;
	}

	if (global[player_persona] == PLAYER_IS_PID)
		kernel.quotes = quote_load(quote_poem_one_how,
		                           quote_poem_one_thou,
		                           quote_poem_one_soft,
		                           quote_poem_two_art,
		                           quote_poem_two_do,
		                           quote_poem_two_what,
		                           quote_poem_three_light,
		                           quote_poem_three_i,
		                           quote_poem_three_a,
		                           quote_poem_four_through,
		                           quote_poem_four_rose,
		                           quote_poem_four_love,
		                           quote_poem_five_but,
		                           quote_poem_five_yonder,
		                           quote_poem_five_thee,
		                           quote_poem_six_let,
		                           quote_poem_six_no,
		                           quote_poem_six_window,
		                           quote_poem_seven_breaks,
		                           quote_poem_seven_for,
		                           quote_poem_seven_me,
		                           quote_poem_eight_no,
		                           quote_poem_eight_count,
		                           quote_poem_eight_tis,
		                           quote_poem_nine_rose,
		                           quote_poem_nine_the,
		                           quote_poem_nine_bird,
		                           quote_poem_ten_ways,
		                           quote_poem_ten_and,
		                           quote_poem_ten_as,
		                           quote_poem_eleven_and,
		                           quote_poem_eleven_fair,
		                           quote_poem_eleven_one,
		                           quote_poem_twelve_did,
		                           quote_poem_twelve_bird,
		                           quote_poem_twelve_two,
		                           quote_poem_thirteen_ever,
		                           quote_poem_thirteen_three,
		                           quote_poem_thirteen_bird,
		                           quote_poem_fourteen_are,
		                           quote_poem_fourteen_is,
		                           quote_poem_fourteen_grow,
		                           quote_poem_easy_14_are,
		                           quote_poem_easy_14_is,
		                           quote_poem_easy_14_grow,
		                           quote_poem_fifteen_enough,
		                           quote_poem_fifteen_in,
		                           quote_poem_fifteen_the,
		                           quote_poem_sixteen_for,
		                           quote_poem_sixteen_any,
		                           quote_poem_sixteen_sun,
		                           quote_poem_seventeen_thee,
		                           quote_poem_seventeen_land,
		                           quote_poem_seventeen_oh, 0);

	ss[fx_look_at_heal] = kernel_load_series(kernel_name('e', 3), false);
	ss[fx_lani]         = kernel_load_series(kernel_name('e', 0), false);
	ss[fx_fire]         = kernel_load_series(kernel_name('x', 1), false);

	seq[fx_fire] = kernel_seq_forward(ss[fx_fire], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire], 14);
	kernel_seq_range(seq[fx_fire], KERNEL_FIRST, KERNEL_LAST);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x      = PLAYER_X_FROM_503;
		player.y      = PLAYER_Y_FROM_503;
		player.facing = FACING_NORTHEAST;
	}

	if (global[player_persona] == PLAYER_IS_PID) {

		if (!global[make_504_empty]) {

			conv_get(CONV_42_POEM);
			conv_get(CONV_26_PID_PRE_HEAL);
			conv_get(CONV_27_PID_HEAL);

			aa[0]                  = kernel_run_animation(kernel_name('l', 1), 0);
			local->anim_0_running  = true;
			local->lani_pid_action = LANI_DEAD;

			aa[1]                = kernel_run_animation(kernel_name('t', 1), 0);
			local->poking_action = TOM_SHUT_UP;
			local->anim_1_running = true;

			if (global[llanie_status] == IS_SAVED) {
				local->dynamic_hs = kernel_add_dynamic(words_Ner_Tom, words_walk_to, SYNTAX_SINGULAR_MASC, KERNEL_NONE,
				                                       0, 0, 0, 0);
			} else {
				local->dynamic_hs = kernel_add_dynamic(words_hermit, words_walk_to, SYNTAX_MASC_NOT_PROPER, KERNEL_NONE,
				                                       0, 0, 0, 0);
			}
			kernel_dynamic_hot[local->dynamic_hs].prep = PREP_ON;
			kernel_dynamic_walk(local->dynamic_hs, WALK_TO_HERMIT_X, WALK_TO_HERMIT_Y, FACING_NORTHWEST);
			kernel_dynamic_anim(local->dynamic_hs, aa[1], 0);

			if (conv_restore_running == CONV_26_PID_PRE_HEAL) {
				conv_run(CONV_26_PID_PRE_HEAL);
				conv_export_value(global[put_bundle_on_llanie_504]);
				conv_export_value(global[said_poem_in_504]);
				conv_export_value(global[tried_to_heal_llanie_504]);
				conv_export_value(player_has(shifter_ring));
			}

			if (conv_restore_running == CONV_42_POEM) {
				kernel_init_dialog();
				kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
				player.commands_allowed = false;
				conv_run(CONV_42_POEM);
				conv_export_value(0);
				conv_export_value(game.difficulty);
				conv_show_boxes = false;
				if (local->line_1[0] != '\0') {
					local->quote_id = kernel_message_add(local->line_1,
					                                     8, 5, MESSAGE_COLOR, 999999, 0, 0);
				}
				if (local->line_2[0] != '\0') {
					local->quote_id = kernel_message_add(local->line_2,
					                                     8, 22, MESSAGE_COLOR, 999999, 0, 0);
				}
				if (local->line_3[0] != '\0') {
					local->quote_id = kernel_message_add(local->line_3,
					                                     8, 39, MESSAGE_COLOR, 999999, 0, 0);
				}
			}

			if (local->pid_is_kneeling) {
				kernel_reset_animation(aa[0], 28);
				player.x              = LANI_X;
				player.y              = LANI_Y;
				player.facing         = FACING_NORTHEAST;
				player.walker_visible = false;
			} else {
				kernel_reset_animation(aa[0], 114);
			}

			if (!global[been_in_504_as_pid]) {
				global[been_in_504_as_pid] = true;
				player.commands_allowed = false;
				player_walk(WALK_TO_HERMIT_X, WALK_TO_HERMIT_Y, FACING_NORTHWEST);
				player_walk_trigger(ROOM_504_TALK_TO_HERMIT);
			}

		} else {
			kernel_flip_hotspot(words_blanket, false);
			kernel_flip_hotspot(words_Llanie, false);
		}

	} else {
		if (global[llanie_status] == SHE_FELL) {
			conv_get(CONV_25_KING_AFTER_FALL);

			aa[0]                  = kernel_run_animation(kernel_name('l', 1), 0);
			local->anim_0_running  = true;
			local->lani_pid_action = LANI_DEAD;
			kernel_reset_animation(aa[0], 114);

			if (!global[seen_lani_dead_1st_time]) {
				global[seen_lani_dead_1st_time] = true;
				player.commands_allowed = false;
				player_walk(WALK_TO_HERMIT_X, WALK_TO_HERMIT_Y, FACING_NORTHWEST);
				player_walk_trigger(ROOM_504_TALK_TO_HERMIT + 1);
			}

		} else {
			conv_get(CONV_24_KING);
			kernel_flip_hotspot(words_blanket, false);
			kernel_flip_hotspot(words_Llanie, false);
		}

		aa[1]                = kernel_run_animation(kernel_name('t', 1), 0);
		local->poking_action = TOM_SHUT_UP;
		local->anim_1_running = true;

		id = kernel_add_dynamic(words_hermit, words_walk_to, SYNTAX_MASC_NOT_PROPER, KERNEL_NONE,
		                        0, 0, 0, 0);
		kernel_dynamic_hot[id].prep = PREP_ON;
		kernel_dynamic_walk(id, WALK_TO_HERMIT_X, WALK_TO_HERMIT_Y, FACING_NORTHWEST);
		kernel_dynamic_anim(id, aa[1], 0);

		if (conv_restore_running == CONV_25_KING_AFTER_FALL) {
			conv_run(CONV_25_KING_AFTER_FALL);
		} else if (conv_restore_running == CONV_24_KING) {
			conv_run(CONV_24_KING);
		}
	}

	if (object_is_here(medicine_bundle) || object_is_here(new_bundle)) {
		if (global[llanie_status] != IS_SAVED) {
			sound_play(N_TenseMusic);
		} else {
			section_5_music();
		}
	} else {
		section_5_music();
	}
}

static void room_504_daemon() {
	if (kernel.trigger == ROOM_504_DONE_HEALING) {
		kernel_abort_animation(aa[2]);
		local->anim_2_running = false;
		kernel_reset_animation(aa[0], 37);
		kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
	}

	if (local->anim_0_running) {
		handle_animation_lani_pid();
	}

	if (local->anim_1_running) {
		handle_animation_tom_poking();
	}

	if (local->anim_2_running) {
		handle_animation_heal();
	}

	if (local->anim_3_running) {
		handle_animation_reveal();
	}

	if (local->anim_4_running) {
		handle_animation_tom_talk();
	}

	if (kernel.trigger == ROOM_504_TALK_TO_HERMIT) {
		player.commands_allowed = true;
		conv_run(CONV_26_PID_PRE_HEAL);
		conv_export_value(global[put_bundle_on_llanie_504]);
		conv_export_value(global[said_poem_in_504]);
		conv_export_value(global[tried_to_heal_llanie_504]);
		conv_export_value(player_has(shifter_ring));
	}

	if (kernel.trigger == ROOM_504_TALK_TO_HERMIT + 1) {
		player.commands_allowed = true;
		conv_run(CONV_25_KING_AFTER_FALL);
	}
}

static void process_conv_poem() {
	int me_trig_flag  = false;
	int selected      = 0;

	switch (player_verb) {
	case conv042_one_restart:
	case conv042_one_abort:
	case conv042_two_restart:
	case conv042_two_abort:
	case conv042_three_restart:
	case conv042_three_abort:
	case conv042_four_restart:
	case conv042_four_abort:
	case conv042_five_restart:
	case conv042_five_abort:
	case conv042_six_restart:
	case conv042_six_abort:
	case conv042_seven_restart:
	case conv042_seven_abort:
	case conv042_eight_restart:
	case conv042_eight_abort:
	case conv042_nine_restart:
	case conv042_nine_abort:
	case conv042_ten_restart:
	case conv042_ten_abort:
	case conv042_eleven_restart:
	case conv042_eleven_abort:
	case conv042_twelve_restart:
	case conv042_twelve_abort:
	case conv042_thirteen_restart:
	case conv042_thirteen_abort:
	case conv042_fourteen_restart:
	case conv042_fourteen_abort:
	case conv042_easy_14_restart:
	case conv042_easy_14_abort:
	case conv042_fifteen_restart:
	case conv042_fifteen_abort:
	case conv042_sixteen_restart:
	case conv042_sixteen_abort:
	case conv042_seventeen_restart:
	case conv042_seventeen_abort:
		conv_show_boxes = true;
		break;
	}

	if (kernel.trigger == ROOM_504_ME_TALK) {
		switch (player_verb) {
		case conv042_one_first:        selected = quote_poem_one_how;        break;
		case conv042_one_second:       selected = quote_poem_one_thou;       ++local->got_it; break;
		case conv042_one_third:        selected = quote_poem_one_soft;       break;

		case conv042_two_first:        selected = quote_poem_two_art;        ++local->got_it; break;
		case conv042_two_second:       selected = quote_poem_two_do;         break;
		case conv042_two_third:        selected = quote_poem_two_what;       break;

		case conv042_three_first:      selected = quote_poem_three_light;    break;
		case conv042_three_second:     selected = quote_poem_three_i;        break;
		case conv042_three_third:      selected = quote_poem_three_a;        ++local->got_it; break;

		case conv042_four_first:       selected = quote_poem_four_through;   break;
		case conv042_four_second:      selected = quote_poem_four_rose;      ++local->got_it; break;
		case conv042_four_third:       selected = quote_poem_four_love;      break;

		case conv042_five_first:       selected = quote_poem_five_but;       ++local->got_it; break;
		case conv042_five_second:      selected = quote_poem_five_yonder;    break;
		case conv042_five_third:       selected = quote_poem_five_thee;      break;

		case conv042_six_first:        selected = quote_poem_six_let;        break;
		case conv042_six_second:       selected = quote_poem_six_no;         ++local->got_it; break;
		case conv042_six_third:        selected = quote_poem_six_window;     break;

		case conv042_seven_first:      selected = quote_poem_seven_breaks;   break;
		case conv042_seven_second:     selected = quote_poem_seven_for;      ++local->got_it; break;
		case conv042_seven_third:      selected = quote_poem_seven_me;       break;

		case conv042_eight_first:      selected = quote_poem_eight_no;       ++local->got_it; break;
		case conv042_eight_second:     selected = quote_poem_eight_count;    break;
		case conv042_eight_third:      selected = quote_poem_eight_tis;      break;

		case conv042_nine_first:       selected = quote_poem_nine_rose;      ++local->got_it; break;
		case conv042_nine_second:      selected = quote_poem_nine_the;       break;
		case conv042_nine_third:       selected = quote_poem_nine_bird;      break;

		case conv042_ten_first:        selected = quote_poem_ten_ways;       break;
		case conv042_ten_second:       selected = quote_poem_ten_and;        break;
		case conv042_ten_third:        selected = quote_poem_ten_as;         ++local->got_it; break;

		case conv042_eleven_first:     selected = quote_poem_eleven_and;     break;
		case conv042_eleven_second:    selected = quote_poem_eleven_fair;    ++local->got_it; break;
		case conv042_eleven_third:     selected = quote_poem_eleven_one;     break;

		case conv042_twelve_first:     selected = quote_poem_twelve_did;     ++local->got_it; break;
		case conv042_twelve_second:    selected = quote_poem_twelve_bird;    break;
		case conv042_twelve_third:     selected = quote_poem_twelve_two;     break;

		case conv042_thirteen_first:   selected = quote_poem_thirteen_ever;  ++local->got_it; break;
		case conv042_thirteen_second:  selected = quote_poem_thirteen_three; break;
		case conv042_thirteen_third:   selected = quote_poem_thirteen_bird;  break;

		case conv042_fourteen_first:   selected = quote_poem_fourteen_are;   break;
		case conv042_fourteen_second:  selected = quote_poem_fourteen_is;    break;
		case conv042_fourteen_third:   selected = quote_poem_fourteen_grow;  ++local->got_it; break;

		case conv042_easy_14_first:    selected = quote_poem_easy_14_are;    break;
		case conv042_easy_14_second:   selected = quote_poem_easy_14_is;     break;
		case conv042_easy_14_third:    selected = quote_poem_easy_14_grow;   ++local->got_it; break;

		case conv042_fifteen_first:    selected = quote_poem_fifteen_enough; break;
		case conv042_fifteen_second:   selected = quote_poem_fifteen_in;     ++local->got_it; break;
		case conv042_fifteen_third:    selected = quote_poem_fifteen_the;    break;

		case conv042_sixteen_first:    selected = quote_poem_sixteen_for;    break;
		case conv042_sixteen_second:   selected = quote_poem_sixteen_any;    ++local->got_it; break;
		case conv042_sixteen_third:    selected = quote_poem_sixteen_sun;    break;

		case conv042_seventeen_first:  selected = quote_poem_seventeen_thee; break;
		case conv042_seventeen_second: selected = quote_poem_seventeen_land; ++local->got_it; break;
		case conv042_seventeen_third:  selected = quote_poem_seventeen_oh;   break;

		case conv042_whoops_b_b:
			kernel_message_purge();
			kernel_timing_trigger(HALF_SECOND, ROOM_504_RUN_POEM_CONV);
			local->working_on_line = 1;
			local->line_1[0] = '\0';
			local->line_2[0] = '\0';
			local->line_3[0] = '\0';
			local->got_it    = 0;
			break;

		case conv042_later_b_b:
			kernel_timing_trigger(HALF_SECOND, ROOM_504_END_POEM + 1);
			local->working_on_line = 1;
			local->line_1[0] = '\0';
			local->line_2[0] = '\0';
			local->line_3[0] = '\0';
			local->got_it    = 0;
			break;
		}

		if (selected) {
			local->lani_pid_action = PID_POEM;

			if (local->working_on_line == 1) {
				if (font_string_width(kernel_message_font, local->line_1, -1) > 278) {
					++local->working_on_line;
				} else {
					if (local->line_1[0] != '\0') {
						kernel_message_delete(local->quote_id);
					}
					Common::strcat_s(local->line_1, quote_string(kernel.quotes, selected));
					Common::strcat_s(local->line_1, " ");
					local->quote_id = kernel_message_add(local->line_1,
					                                     8, 5, MESSAGE_COLOR, 999999, 0, 0);
				}
			}

			if (local->working_on_line == 2) {
				if (font_string_width(kernel_message_font, local->line_2, -1) > 278) {
					++local->working_on_line;
				} else {
					if (local->line_2[0] != '\0') {
						kernel_message_delete(local->quote_id);
					}
					Common::strcat_s(local->line_2, quote_string(kernel.quotes, selected));
					Common::strcat_s(local->line_2, " ");
					local->quote_id = kernel_message_add(local->line_2,
					                                     8, 22, MESSAGE_COLOR, 999999, 0, 0);
				}
			}

			if (local->working_on_line == 3) {
				if (font_string_width(kernel_message_font, local->line_3, -1) > 278) {
					++local->working_on_line;
				} else {
					if (local->line_3[0] != '\0') {
						kernel_message_delete(local->quote_id);
					}
					Common::strcat_s(local->line_3, quote_string(kernel.quotes, selected));
					Common::strcat_s(local->line_3, " ");
					local->quote_id = kernel_message_add(local->line_3,
					                                     8, 39, MESSAGE_COLOR, 999999, 0, 0);
				}
			}
		}
	}

	if (player_verb == conv042_exit_b_b && !kernel.trigger) {
		*conv_my_next_start = conv042_pre_poem;
		conv_abort();
		me_trig_flag = true;

		if ((game.difficulty == EASY_MODE && local->got_it == 14) ||
		    (local->got_it == 17)) {
			kernel_timing_trigger(HALF_SECOND, ROOM_504_END_POEM);
			global[player_score] += 8;

		} else {
			local->working_on_line = 1;
			local->line_1[0] = '\0';
			local->line_2[0] = '\0';
			local->line_3[0] = '\0';
			local->got_it    = 0;
			kernel_timing_trigger(HALF_SECOND, ROOM_504_END_POEM);
		}
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_504_ME_TALK);
	}
}

static void process_conv_lani_pid() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	switch (player_verb) {
	case conv027_inter2_b_b:
	case conv027_amulet_b_b:
		conv_hold();
		you_trig_flag          = true;
		me_trig_flag           = true;
		local->lani_pid_action = GIVE;
		break;

	case conv027_exit_b_b:
		conv_hold();
		you_trig_flag          = true;
		me_trig_flag           = true;
		local->lani_pid_action = PID_GET_UP;
		break;

	case conv027_kiss_b_b:
		conv_hold();
		you_trig_flag          = true;
		me_trig_flag           = true;
		local->lani_pid_action = LANI_KISS;
		break;

	case conv027_quickie_b_b:
		conv_hold();
		you_trig_flag        = true;
		me_trig_flag         = true;
		local->poking_action = TOM_REVEAL;
		break;
	}

	if (kernel.trigger == ROOM_504_YOU_TALK) {
		switch (player_verb) {
		case conv027_seven_only:
		case conv027_amulet_only:
		case conv027_stopper_monk:
		case conv027_inter2_only:
		case conv027_hello_only:
		case conv027_explain_one:
		case conv027_next_one:
		case conv027_last_one:
		case conv027_two_only:
		case conv027_four_only:
		case conv027_six_only:
		case conv027_eight_only:
		case conv027_nogood_hidewhat:
			if (local->lani_pid_action != LANI_KISS &&
			    local->lani_pid_action != GIVE       &&
			    local->lani_pid_action != PID_GET_UP) {
				local->lani_pid_action = LANI_TALK;
			}
			local->tom_talk_action = TOM_SHUT_UP;
			break;

		case conv027_restart_only:
			local->poking_action = TOM_TALK;
			break;

		case conv027_ten_only:
			break;

		default:
			if (local->lani_pid_action != LANI_KISS &&
			    local->lani_pid_action != GIVE       &&
			    local->lani_pid_action != PID_GET_UP) {
				local->lani_pid_action = PID_LOOK_TOM;
			}
			local->poking_action   = TOM_TALK;
			local->tom_talk_action = TOM_TALK;
			break;
		}
	}

	if (kernel.trigger == ROOM_504_ME_TALK) {
		local->lani_pid_action = PID_TALK_TO_LANI;
		local->tom_talk_action = TOM_SHUT_UP;
		if (player_verb == conv027_quickie_only ||
		    player_verb == conv027_hem_only      ||
		    player_verb == conv027_one_only      ||
		    player_verb == conv027_two_only) {
			local->lani_pid_action = PID_LOOK_TOM;
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_504_YOU_TALK);
	}

	if (!me_trig_flag) {
		if (player_verb != conv027_restart_only && player_verb != conv027_ten_only) {
			conv_me_trigger(ROOM_504_ME_TALK);
		}
	}

	local->lani_pid_talk_count   = 0;
	local->tom_talk_talk_count   = 0;
	local->poking_talk_count     = 0;
}

static void process_conv_king() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	if (player_verb == conv026_exit_b_b) {
		if (local->pid_is_kneeling) {
			local->lani_pid_action = BOTH_SHUT_UP;
		}
	}

	if (conv_control.running == CONV_26_PID_PRE_HEAL && player_verb == conv026_last_one) {
		global[heal_verbs_visible] = true;
	}

	if (kernel.trigger == ROOM_504_YOU_TALK) {
		local->poking_action = TOM_TALK;
	}

	if (kernel.trigger == ROOM_504_ME_TALK) {
		local->poking_action = TOM_SHUT_UP;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_504_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_504_ME_TALK);
	}

	local->poking_talk_count = 0;
}

static void room_504_pre_parser() {
	if (player_said_2(heal, Llanie)) {
		if (global[llanie_status] != IS_SAVED) {
			local->prevent = true;
		}
	}

	if (player_said_2(invoke_power_of, crystal_ball) ||
	    player_said_3(put, spirit_bundle, blanket)    ||
	    player_said_3(put, new_bundle, blanket)        ||
	    player_said_2(talk_to, Llanie)) {
		if (global[llanie_status] != IS_SAVED) {
			player_walk(LANI_X, LANI_Y, FACING_NORTHEAST);
		}
	}

	if (local->pid_is_kneeling && player.need_to_walk) {
		if (player_said_3(put, spirit_bundle, Llanie)  ||
		    player_said_3(put, spirit_bundle, blanket)  ||
		    player_said_3(give, spirit_bundle, Llanie)  ||
		    player_said_3(put, new_bundle, Llanie)      ||
		    player_said_3(put, new_bundle, blanket)     ||
		    player_said_3(give, new_bundle, Llanie)     ||
		    player_said_2(heal, Llanie)                 ||
		    player_said_2(invoke_power_of, crystal_ball)||
		    player_said_2(talk_to, hermit)              ||
		    player_said_2(talk_to, Llanie)) {

			player.need_to_walk = false;

		} else {
			player.ready_to_walk   = false;
			local->lani_pid_action = PID_GET_UP;
		}
	}
}

static void room_504_parser() {
	if (kernel.trigger == ROOM_504_RUN_POEM_CONV) {
		conv_run(CONV_42_POEM);
		conv_export_value(0);
		conv_export_value(game.difficulty);
		conv_show_boxes = false;
		player.command_ready = false;
		return;
	}

	if (kernel.trigger == ROOM_504_END_POEM + 1) {
		kernel_message_purge();
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		player.commands_allowed = true;
		player.command_ready = false;
		return;
	}

	if (kernel.trigger == ROOM_504_END_POEM) {
		player.commands_allowed = true;
		kernel_message_purge();
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);

		if ((game.difficulty == EASY_MODE && local->got_it == 14) ||
		    (local->got_it == 17)) {
			global[said_poem_in_504] = true;
			text_show(50426);

		} else {
			global[tried_to_heal_llanie_504] = true;

			conv_run(CONV_42_POEM);
			conv_export_value(1);
			conv_export_value(game.difficulty);
		}
		player.command_ready = false;
		return;
	}

	if (conv_control.running == CONV_27_PID_HEAL) {
		process_conv_lani_pid();
		player.command_ready = false;
		return;
	}

	if (conv_control.running == CONV_42_POEM) {
		process_conv_poem();
		player.command_ready = false;
		return;
	}

	if (conv_control.running == CONV_24_KING          ||
	    conv_control.running == CONV_25_KING_AFTER_FALL||
	    conv_control.running == CONV_26_PID_PRE_HEAL) {
		process_conv_king();
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_through, cave_entrance)) {
		new_room = 503;
		player.command_ready = false;
		return;
	}

	if (player.look_around) {
		if (global[make_504_empty]) {
			text_show(50441);

		} else if (global[player_persona] == PLAYER_IS_KING) {
			if (global[llanie_status] == BEFORE_FALL) {
				text_show(50401);
			} else if (global[llanie_status] == SHE_FELL) {
				text_show(50411);
			}

		} else if (global[llanie_status] == IS_SAVED) {
			text_show(50431);

		} else {
			text_show(50421);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (global[make_504_empty]) {
			text_show(50442);
			player.command_ready = false;
			return;
		}

		if (player_said_1(pallet)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				if (global[llanie_status] == BEFORE_FALL) {
					text_show(50402);
				} else if (global[llanie_status] == SHE_FELL) {
					text_show(50412);
				}
			} else if (global[llanie_status] == IS_SAVED) {
				text_show(50432);
			} else {
				text_show(50422);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(Llanie)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(50412);
			} else if (global[llanie_status] == IS_SAVED) {
				text_show(50432);
			} else {
				text_show(50422);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(blanket)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				if (global[llanie_status] == BEFORE_FALL) {
					text_show(50404);
					player.command_ready = false;
					return;
				} else if (global[llanie_status] == SHE_FELL) {
					text_show(50414);
					player.command_ready = false;
					return;
				}
			}
		}

		if (player_said_1(firepit)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				if (global[llanie_status] == BEFORE_FALL) {
					text_show(50407);
					player.command_ready = false;
					return;
				} else if (global[llanie_status] == SHE_FELL) {
					text_show(50415);
					player.command_ready = false;
					return;
				}
			} else if (global[llanie_status] == IS_SAVED) {
				text_show(50434);
			} else {
				text_show(50415);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(flat_stone)) {
			text_show(50408);
			player.command_ready = false;
			return;
		}

		if (player_said_1(cave_entrance)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(50409);
			} else {
				text_show(50424);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(hermit)) {
			if (global[llanie_status] == BEFORE_FALL) {
				text_show(50410);
			} else if (global[llanie_status] == SHE_FELL) {
				text_show(50416);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(Ner_Tom)) {
			text_show(50433);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_1(Llanie) || player_said_1(pallet) || player_said_1(blanket)) {
		if (player_said_1(take) || player_said_1(push) || player_said_1(pull)) {
			if (global[llanie_status] == SHE_FELL) {
				text_show(50413);
				player.command_ready = false;
				return;
			}
		}
	}

	if (player_said_3(give, soul_egg, Llanie)) {
		text_show(50435);
		player.command_ready = false;
		return;
	}

	if (player_said_2(talk_to, hermit)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			if (global[llanie_status] == SHE_FELL) {
				conv_run(CONV_25_KING_AFTER_FALL);
			} else {
				conv_run(CONV_24_KING);
			}

		} else {
			if (local->pid_is_kneeling) {
				local->lani_pid_action = PID_LOOK_TOM;
			}
			conv_run(CONV_26_PID_PRE_HEAL);
			conv_export_value(global[put_bundle_on_llanie_504]);
			conv_export_value(global[said_poem_in_504]);
			conv_export_value(global[tried_to_heal_llanie_504]);
			conv_export_value(player_has(shifter_ring));
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(talk_to, Ner_Tom)) {
		conv_run(CONV_27_PID_HEAL);
		conv_export_value(player_has(amulet));
		player.command_ready = false;
		return;
	}

	if (player_said_3(put, spirit_bundle, Llanie)  ||
	    player_said_3(put, spirit_bundle, blanket)  ||
	    player_said_3(give, spirit_bundle, Llanie)  ||
	    player_said_3(put, new_bundle, Llanie)      ||
	    player_said_3(put, new_bundle, blanket)     ||
	    player_said_3(give, new_bundle, Llanie)) {

		if (global[put_bundle_on_llanie_504]) {
			text_show(50445);

		} else if (global[player_persona] == PLAYER_IS_PID) {
			if (player_said_1(new_bundle)) {
				local->purpose_for_healing = NEW;
			} else {
				local->purpose_for_healing = BUNDLE;
			}
			global[put_bundle_on_llanie_504] = true;
			local->ready_to_heal    = true;
			local->poking_action    = TOM_LOOK_AT_HEAL;
			player.commands_allowed = false;

		} else {
			if (game.difficulty == HARD_MODE) {
				text_show(50419);
			} else {
				text_show(50420);
			}
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(heal, Llanie) || player_said_2(invoke_power_of, crystal_ball)) {
		if (global[llanie_status] == IS_SAVED) {
			if (player_said_1(heal)) {
				text_show(927);
				player.command_ready = false;
				return;
			}
		} else {
			local->ready_to_heal    = true;
			local->poking_action    = TOM_LOOK_AT_HEAL;
			player.commands_allowed = false;

			if (!global[said_poem_in_504] || !global[put_bundle_on_llanie_504]) {
				local->purpose_for_healing = NOTHING;

			} else if (player_said_1(crystal_ball)) {
				sound_play(N_InvokeCrystalBall);
				local->purpose_for_healing = HEAL_CRYSTAL;
			} else {
				local->purpose_for_healing = HEAL;
			}

			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(talk_to, Llanie)) {
		if (global[player_persona] == PLAYER_IS_PID) {
			if (global[llanie_status] == IS_SAVED) {
				conv_run(CONV_27_PID_HEAL);
				conv_export_value(player_has(amulet));

			} else if (global[said_poem_in_504]) {
				text_show(50440);

			} else {
				local->lani_pid_action     = BOTH_SHUT_UP;
				local->purpose_for_healing = POEM;
				player.commands_allowed    = false;
			}

		} else {
			text_show(50418);
		}

		player.command_ready = false;
		return;
	}

	if (player_said_2(give, Llanie)) {
		if (global[player_persona] == PLAYER_IS_PID) {
			if (global[llanie_status] == IS_SAVED) {
				text_show(50443);
			} else {
				text_show(50423);
			}
		} else {
			text_show(50417);
		}

		player.command_ready = false;
		return;
	}

	if (player_said_2(put, firepit) || player_said_2(throw, firepit)) {
		text_show(50444);
		player.command_ready = false;
		return;
	}

	if (player_said_2(gaze_into, crystal_ball)) {
		if (global[player_persona] == PLAYER_IS_PID) {
			if (!global[said_poem_in_504] || !global[put_bundle_on_llanie_504]) {
				text_show(50436);
				player.command_ready = false;
				return;

			} else if (global[said_poem_in_504] && global[put_bundle_on_llanie_504] &&
			           global[llanie_status] != IS_SAVED) {
				text_show(50437);
				player.command_ready = false;
				return;
			}
		}
	}
}

void room_504_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(local->lani_pid_frame);
	s.syncAsSint16LE(local->lani_pid_action);
	s.syncAsSint16LE(local->lani_pid_talk_count);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->poking_frame);
	s.syncAsSint16LE(local->poking_action);
	s.syncAsSint16LE(local->poking_talk_count);
	s.syncAsSint16LE(local->anim_1_running);
	s.syncAsSint16LE(local->heal_frame);
	s.syncAsSint16LE(local->heal_action);
	s.syncAsSint16LE(local->heal_talk_count);
	s.syncAsSint16LE(local->anim_2_running);
	s.syncAsSint16LE(local->reveal_frame);
	s.syncAsSint16LE(local->reveal_action);
	s.syncAsSint16LE(local->reveal_talk_count);
	s.syncAsSint16LE(local->anim_3_running);
	s.syncAsSint16LE(local->tom_talk_frame);
	s.syncAsSint16LE(local->tom_talk_action);
	s.syncAsSint16LE(local->tom_talk_talk_count);
	s.syncAsSint16LE(local->anim_4_running);
	s.syncAsSint16LE(local->ready_to_heal);
	s.syncAsSint16LE(local->prevent);
	s.syncAsSint16LE(local->dynamic_hs);
	s.syncAsSint16LE(local->pid_is_kneeling);
	s.syncAsSint16LE(local->purpose_for_healing);
	s.syncAsSint16LE(local->got_it);
	s.syncBytes((byte *)local->line_1, sizeof(local->line_1));
	s.syncBytes((byte *)local->line_2, sizeof(local->line_2));
	s.syncBytes((byte *)local->line_3, sizeof(local->line_3));
	s.syncAsSint16LE(local->working_on_line);
	s.syncAsSint16LE(local->quote_id);
	s.syncAsSint16LE(local->shit);
}

void room_504_preload() {
	room_init_code_pointer       = room_504_init;
	room_pre_parser_code_pointer = room_504_pre_parser;
	room_parser_code_pointer     = room_504_parser;
	room_daemon_code_pointer     = room_504_daemon;

	if (global[make_504_empty]) {
		kernel_initial_variant = 1;
	}

	section_5_walker();
	section_5_interface();

	vocab_make_active(words_hermit);
	vocab_make_active(words_Ner_Tom);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
