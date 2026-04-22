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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/rooms/section2.h"
#include "mads/madsv2/phantom/rooms/room201.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

static void handle_animation_raoul() {
	int random;
	int raoul_reset_frame;

	if (kernel_anim[aa[0]].frame != local->raoul_frame) {
		local->raoul_frame = kernel_anim[aa[0]].frame;
		raoul_reset_frame  = -1;

		switch (local->raoul_frame) {
		case 9:   /* end of take envelope */
			player.walker_visible   = true;
			local->anim_0_running   = false;
			player.commands_allowed = true;
			raoul_reset_frame       = 49;
			break;

		case 1:   /* end of freeze standing upright */
		case 19:  /* end talk                       */
		case 49:  /* end of coming from lean        */

			random = imath_random(4, 50);

			switch (local->raoul_action) {
			case CONV16_RAOUL_TALK:
				random = 1;
				break;

			case CONV16_RAOUL_TAKE_IT:
				random                  = 2;
				player.commands_allowed = false;
				break;

			case CONV16_RAOUL_QUIT:
				random = 3;
				break;
			}

			switch (random) {
			case 1:
				raoul_reset_frame   = 9;
				local->raoul_action = CONV16_RAOUL_SHUT_UP;
				break; /* do talk (standing upright) */

			case 2:
				raoul_reset_frame = 1;
				break; /* take envelope */

			case 3:
				player.walker_visible   = true;
				local->seller_action    = CONV16_SELLER_LOOK_DOWN;
				local->anim_0_running   = false;
				raoul_reset_frame       = 49;
				player.commands_allowed = true;
				local->want_to_hold     = false;
				break; /* quit */

			case 4:
				raoul_reset_frame = 19;
				break; /* lean (new node) */

			default:
				raoul_reset_frame = 0;
				break; /* freeze while standing upright */
			}
			break;

		case 23:    /* end of coming to lean and freeze */
		case 35:    /* end of talk                      */
		case 45:    /* end of hand on chin              */
			random = imath_random(3, 70);

			switch (local->raoul_action) {
			case CONV16_RAOUL_TAKE_IT:
			case CONV16_RAOUL_QUIT:
				random = 1;
				break;

			case CONV16_RAOUL_TALK:
				random = 2;
				break;
			}

			switch (random) {
			case 1:
				raoul_reset_frame = 45;
				break; /* go to stand upright (new node) */

			case 2:
				raoul_reset_frame   = 23;
				local->raoul_action = CONV16_RAOUL_SHUT_UP;
				break; /* talk while leaning */

			case 3:
				raoul_reset_frame = 35;
				break; /* hand on chin */

			default:
				raoul_reset_frame = 22;
				break; /* freeze while slouch 1 */
			}
			break;

		case 5:
			kernel_seq_delete(seq[fx_envelope]);
			inter_give_to_player(envelope);
			sound_play(N_TakeObjectSnd);
			object_examine(envelope, text_008_34, 0);
			break;
		}

		if (raoul_reset_frame >= 0) {
			kernel_reset_animation(aa[0], raoul_reset_frame);
			local->raoul_frame = raoul_reset_frame;
		}
	}
}

static void handle_animation_seller() {
	int random = 0;
	int seller_reset_frame;

	if (kernel_anim[aa[1]].frame != local->seller_frame) {
		local->seller_frame = kernel_anim[aa[1]].frame;
		seller_reset_frame  = -1;

		switch (local->seller_frame) {
		case 1:   /* end of talk 1        */
		case 2:   /* end of talk 2        */
		case 3:   /* end of talk 3        */
		case 4:   /* end of talk 4        */
		case 5:   /* end of talk 5        */
		case 6:   /* end of look down 1   */
		case 7:   /* end of look down 2   */
		case 10:  /* end of give envelope */

			if (local->seller_frame == 10) {
				local->raoul_action = CONV16_RAOUL_TAKE_IT;
			}

			switch (local->seller_action) {
			case CONV16_SELLER_TALK:
				random = imath_random(1, 5);
				++local->seller_talk_count;
				if (local->seller_talk_count > 30) {
					local->seller_action = CONV16_SELLER_SHUT_UP;
					random = 6;
				}
				break;

			case CONV16_SELLER_LOOK_DOWN:
				if (local->seller_frame == 6)      random = 6;
				else if (local->seller_frame == 7) random = 7;
				else                               random = imath_random(6, 7);

				++local->seller_talk_count;
				if (local->seller_talk_count > 30) {
					local->seller_talk_count = 0;
					random                   = imath_random(6, 7);
				}
				break;

			case CONV16_SELLER_SHUT_UP:
				random = 1;
				break;

			case CONV16_SELLER_GIVE:
				random = 8;
				break;
			}

			switch (random) {
			case 1:
				seller_reset_frame = 0;
				break; /* do talk 1 and shut up */

			case 2:
				seller_reset_frame = 1;
				break; /* do talk 2 */

			case 3:
				seller_reset_frame = 2;
				break; /* do talk 3 */

			case 4:
				seller_reset_frame = 3;
				break; /* do talk 4 */

			case 5:
				seller_reset_frame = 4;
				break; /* do talk 5 */

			case 6:
				seller_reset_frame = 5;
				break; /* do look down 1 */

			case 7:
				seller_reset_frame = 6;
				break; /* do look down 2 */

			case 8:
				seller_reset_frame = 7;
				break; /* give envelope */
			}
			break;

		case 9:
			seq[fx_envelope] = kernel_seq_stamp(ss[fx_envelope], false, 1);
			kernel_seq_depth(seq[fx_envelope], 1);
			local->seller_action = CONV16_SELLER_LOOK_DOWN;
			break;
		}

		if (seller_reset_frame >= 0) {
			kernel_reset_animation(aa[1], seller_reset_frame);
			local->seller_frame = seller_reset_frame;
		}
	}
}

void room_201_init() {
	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
	}

	local->seller_talk_count = 0;
	local->want_to_hold      = false;

	/* =================== Load conversation ======================== */

	conv_get(CONV_TICKET_16);


	/* =================== Load Sprite Series ======================= */

	ss[fx_curtain]  = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_envelope] = kernel_load_series(kernel_name('f', 0), false);


	/* ================ Start anim of ticket seller ================= */

	if (global[ticket_people_here]) {
		aa[1]                 = kernel_run_animation(kernel_name('s', 1), 0);
		local->anim_1_running = true;
		local->seller_action  = CONV16_SELLER_LOOK_DOWN;
		kernel_reset_animation(aa[1], 5);  /* make ticket seller look down */

	} else {
		kernel_flip_hotspot(words_ticket_seller, false);
		seq[fx_curtain] = kernel_seq_stamp(ss[fx_curtain], false, 1);
		kernel_seq_depth(seq[fx_curtain], 14);
	}

	if (conv_restore_running == CONV_TICKET_16) {
		player.x              = WINDOW_X;
		player.y              = WINDOW_Y;
		player.facing         = FACING_NORTHWEST;
		aa[0]                 = kernel_run_animation(kernel_name('r', 1), 0);
		local->anim_0_running = true;
		local->raoul_action   = CONV16_RAOUL_SHUT_UP;
		player.walker_visible = false;

		conv_run(CONV_TICKET_16);
		conv_export_pointer(&global[player_score]);
		conv_export_pointer(&global[christine_told_envelope]);
	}


	/* ========================= Previous Rooms ===================== */

	if ((previous_room == 202) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x = PLAYER_X_FROM_202;
		player.y = PLAYER_Y_FROM_202;
		player_walk(WALK_TO_X_FROM_202, WALK_TO_Y_FROM_202, FACING_SOUTHWEST);
	}

	section_2_music();
}

void room_201_daemon() {
	if (local->anim_0_running) {
		handle_animation_raoul();
	}

	if (local->anim_1_running) {
		handle_animation_seller();
	}

	if ((local->want_to_hold) && (conv_control.running != CONV_TICKET_16)) {
		player.commands_allowed = false;
		local->want_to_hold     = false;
	}
}

static void process_conversation_16() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	switch (player_verb) {
	case conv016_take_b_b:
		local->seller_action = CONV16_SELLER_GIVE;
		local->want_to_hold  = true;
		you_trig_flag        = true;
		me_trig_flag         = true;
		break;

	case conv016_buy_four:
	case conv016_kind_five:
	case conv016_money_cash:
	case conv016_money_lend:
		conv_you_trigger(ROOM_201_END);
		local->want_to_hold = true;
		you_trig_flag       = true;
		break;
	}

	switch (kernel.trigger) {
	case ROOM_201_END:
		conv_me_trigger(ROOM_201_END + 1);
		me_trig_flag = true;
		break;

	case ROOM_201_END + 1:
		local->raoul_action = CONV16_RAOUL_QUIT;
		me_trig_flag        = true;
		you_trig_flag       = true;
		break;

	case ROOM_201_SELLER_TALK:
		if (local->seller_action != CONV16_SELLER_GIVE) {
			local->seller_action = CONV16_SELLER_TALK;
		}
		break;

	case ROOM_201_ME_TALK:
		if (local->seller_action != CONV16_SELLER_GIVE) {
			local->seller_action = CONV16_SELLER_SHUT_UP;
			local->raoul_action  = CONV16_RAOUL_TALK;
		}
		break;
	}


	/* ================= Set up me and you triggers ================ */

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_201_ME_TALK);
	} /* if me_trig_flag == true, then a me trigger is called from above, not here. */

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_201_SELLER_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->seller_talk_count = 0;
}

void room_201_pre_parser() {
	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(placard)) {
			player_walk(SIGN_X, SIGN_Y, FACING_NORTHWEST);
		}
	}
}

void room_201_parser() {
	if (conv_control.running == CONV_TICKET_16) {
		process_conversation_16();
		goto handled;
	}

	if (player_said_2(walk_through, archway)) {
		new_room = 202;
		goto handled;
	}

	if (player_said_2(talk_to, ticket_seller)) {
		aa[0]                 = kernel_run_animation(kernel_name('r', 1), 0);
		local->anim_0_running = true;
		local->raoul_action   = CONV16_RAOUL_SHUT_UP;
		conv_run(CONV_TICKET_16);
		conv_export_pointer(&global[player_score]);
		conv_export_pointer(&global[christine_told_envelope]);
		player.walker_visible = false;
		goto handled;
	}

	if (player.look_around) {
		if (global[current_year] == 1993) {
			text_show(text_201_10);
		} else {
			text_show(text_201_11);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(floor)) {
			text_show(text_201_12);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_201_13);
			goto handled;
		}

		if (player_said_1(bust)) {
			text_show(text_201_14);
			goto handled;
		}

		if (player_said_1(pedestal)) {
			text_show(text_201_15);
			goto handled;
		}

		if (player_said_1(barrier)) {
			text_show(text_201_16);
			goto handled;
		}

		if (player_said_1(placard)) {
			if (global[current_year] == 1993) {
				text_show(text_201_17);
			} else {
				text_show(text_201_18);
			}
			goto handled;
		}

		if (player_said_1(ticket_window)) {
			if (global[current_year] == 1993) {
				text_show(text_201_19);
			} else {
				text_show(text_201_20);
			}
			goto handled;
		}

		if (player_said_1(archway)) {
			text_show(text_201_21);
			goto handled;
		}

		if (player_said_1(ticket_seller)) {
			text_show(text_201_23);
			goto handled;
		}
	}

	if (player_said_2(take, ticket_seller)) {
		text_show(text_201_24);
		goto handled;
	}

	if (player_said_2(talk_to, bust)) {
		text_show(text_201_22);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_201_preload() {
	room_init_code_pointer       = room_201_init;
	room_pre_parser_code_pointer = room_201_pre_parser;
	room_parser_code_pointer     = room_201_parser;
	room_daemon_code_pointer     = room_201_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
