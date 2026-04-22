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
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/rail.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/quotes.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/rooms/section3.h"
#include "mads/madsv2/phantom/rooms/room304.h"
#include "mads/madsv2/core/conv.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_304_init() {
	local->raoul_look_count = 0;
	player.x = 0;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
	}

	/* =================== Load conversations ==================== */

	if (global[right_door_is_open_504]) {
		conv_get(CONV_FIGHT_23);
	}


	/* ===================== Stamp down chan ===================== */

	if (!global[right_door_is_open_504]) {
		ss[fx_chan] = kernel_load_series(kernel_name('f', 1), false);
		seq[fx_chan] = kernel_seq_stamp(ss[fx_chan], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_chan], 1);

		aa[0] = kernel_run_animation(kernel_name('n', 1), 0);
		local->anim_0_running = true;
		local->raoul_action = RAOUL_OTHER;
		player.commands_allowed = false;

	} else {

		kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

		/* viewing_at_y = ((video_y - display_y) >> 1); */

		if (previous_room == 305) {
			ss[fx_chan_tilt] = kernel_load_series(kernel_name('f', 0), false);
			kernel_draw_to_background(ss[fx_chan_tilt], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);

			player.commands_allowed = false;
			aa[1] = kernel_run_animation(kernel_name('f', 1), 0);
			local->anim_1_running = true;
			kernel_reset_animation(aa[1], 138);

		} else {

			ss[fx_chan_tilt] = kernel_load_series(kernel_name('f', 0), false);
			kernel_draw_to_background(ss[fx_chan_tilt], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);

			aa[1] = kernel_run_animation(kernel_name('f', 1), 0);
			local->anim_1_running = true;
			local->phantom_action = CONV23_PHANTOM_DAAE;

			aa[2] = kernel_run_animation(kernel_name('r', 1), 0);
			local->anim_2_running = true;
			local->raoul_fight_action = CONV23_RAOUL_OTHER;

			player.commands_allowed = false;
			local->raoul_fight_talk_count = 0;

			global[player_score] += 10;

			conv_run(CONV_FIGHT_23);
			conv_hold();
		}
	}


	if (previous_room == KERNEL_RESTORING_GAME) {
		if (!global[right_door_is_open_504]) {
			kernel_reset_animation(aa[0], 53);
			player.commands_allowed = true;
		}
	}

	player.walker_visible = false;

	section_3_music();
}

static void process_conversation_23() {
	switch (player_verb) {
	case conv023_unhand_b_b:
		if (local->phantom_action != CONV23_PHANTOM_LET_GO) {
			local->raoul_fight_action = CONV23_RAOUL_TALK;
		}
		break;

	case conv023_okay_b_b:
		if (local->phantom_action != CONV23_PHANTOM_LET_GO) {
			local->raoul_fight_action = CONV23_RAOUL_OTHER;
		}
		conv_hold();
		break;

	case conv023_die_b_b:
		local->phantom_action = CONV23_PHANTOM_FIGHT;
		/* conv_hold (); */
		break;

	case conv023_okay_abc:
		local->phantom_action = CONV23_PHANTOM_LET_GO;
		conv_hold();
		break;

	default:
		break;
	}

	local->raoul_fight_talk_count = 0;
}

void room_304_parser() {
	if (conv_control.running == CONV_FIGHT_23) {
		process_conversation_23();
		goto handled;
	}

	if (player_said_2(climb_through, opening) || player_said_2(climb, chandelier_cable)) {
		local->raoul_action = RAOUL_CLIMB_UP;
		goto handled;
	}

	if (player.look_around) {
		text_show(text_304_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(ceiling)) {
			text_show(text_304_11);
			goto handled;
		}

		if (player_said_1(dome)) {
			text_show(text_304_12);
			goto handled;
		}

		if (player_said_1(latticework)) {
			text_show(text_304_13);
			goto handled;
		}

		if (player_said_1(opening)) {
			text_show(text_304_14);
			goto handled;
		}

		if (player_said_1(chandelier)) {
			text_show(text_304_15);
			goto handled;
		}

		if (player_said_1(chandelier_cable)) {
			text_show(text_304_16);
			goto handled;
		}
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

static void handle_animation_raoul() {
	int random;
	int raoul_reset_frame;

	if (kernel_anim[aa[0]].frame != local->raoul_frame) {
		local->raoul_frame = kernel_anim[aa[0]].frame;
		raoul_reset_frame = -1;

		switch (local->raoul_frame) {
		case 53:  /* end of climbing up rope */
			new_room = 303;
			break;

		case 20:  /* end of climbing down    */
			player.commands_allowed = true;
			raoul_reset_frame = 53;
			break;

		case 54:  /* end of look straight    */
		case 55:  /* end of look left        */
		case 56:  /* end of look right       */

			switch (local->raoul_action) {
			case RAOUL_CLIMB_UP:
				raoul_reset_frame = 20; /* climb up rope */
				player.commands_allowed = false;
				break;

			default:  /* look around */
				random = imath_random(1, 50);
				switch (local->raoul_frame) {
				case 54: /* looking straight */
					if (random == 1) raoul_reset_frame = 54; /* look left */
					else if (random == 2) raoul_reset_frame = 55; /* look right */
					else raoul_reset_frame = local->raoul_frame - 1;
					break;

				case 55: /* looking left */
					if (random == 1) raoul_reset_frame = 54; /* look left */
					else if (random == 2) raoul_reset_frame = 53; /* look straight */
					else raoul_reset_frame = local->raoul_frame - 1;
					break;

				case 56: /* looking right */
					if (random == 1) raoul_reset_frame = 55; /* look right */
					else if (random == 2) raoul_reset_frame = 53; /* look straight */
					else raoul_reset_frame = local->raoul_frame - 1;
					break;
				}
				break;
			}
			break;
		}

		if (raoul_reset_frame >= 0) {
			kernel_reset_animation(aa[0], raoul_reset_frame);
			local->raoul_frame = raoul_reset_frame;
		}
	}
}

static void handle_animation_phantom() {
	int random;
	int phantom_reset_frame;

	if (kernel_anim[aa[1]].frame != local->phantom_frame) {
		local->phantom_frame = kernel_anim[aa[1]].frame;
		phantom_reset_frame = -1;

		switch (local->phantom_frame) {

		case 25:
			conv_release();
			break;

		case 47:
			local->raoul_fight_action = CONV23_RAOUL_OTHER;
			break;

		case 137:
			player.x = 100;
			new_room = 305;
			/* Phantom is on top of Raoul */
			break;

		case 176:
			player.x = 200;
			new_room = 305;
			/* Raoul is on top of Phantom */
			break;

		case 7:  /* end of struggle point */
		case 11: /* end of struggle point */
			phantom_reset_frame = imath_random(6, 7);

			if (local->phantom_action == CONV23_PHANTOM_LET_GO) {
				phantom_reset_frame = 7;
			}
			break;

		case 9:  /* end of struggle point */
		case 15: /* end of struggle point */
			random = imath_random(1, 3);
			switch (random) {
			case 1:
				phantom_reset_frame = 8;
				break;

			case 2:
				phantom_reset_frame = 9;
				break;

			case 3:
				phantom_reset_frame = 11;
				break;
			}

			if (local->phantom_action == CONV23_PHANTOM_LET_GO) {
				phantom_reset_frame = 11;
			}
			break;

		case 13: /* end of struggle point */
		case 24: /* end of struggle point */
			random = imath_random(1, 3);
			switch (random) {
			case 1:
				phantom_reset_frame = 12;
				break;

			case 2:
				phantom_reset_frame = 13;
				break;

			case 3:
				phantom_reset_frame = 16;
				break;
			}

			if (local->phantom_action == CONV23_PHANTOM_LET_GO) {
				phantom_reset_frame = 16;
			}
			break;

		case 20: /* end of struggle point */
			random = imath_random(1, 2);
			switch (random) {
			case 1:
				phantom_reset_frame = 19;
				break;

			case 2:
				phantom_reset_frame = 20;
				break;
			}

			if (local->phantom_action == CONV23_PHANTOM_LET_GO) {
				phantom_reset_frame = 24;
			}
			break;

		case 59:
			if (local->phantom_action == CONV23_PHANTOM_FIGHT) {
				phantom_reset_frame = 59;
			} else {
				phantom_reset_frame = 58;
			}
			break;

		case 60:
			player.commands_allowed = false;
			break;

		case 80:
			inter_move_object(sword, NOWHERE);
			break;

		}

		if (phantom_reset_frame >= 0) {
			kernel_reset_animation(aa[1], phantom_reset_frame);
			local->phantom_frame = phantom_reset_frame;
		}
	}
}

static void handle_animation_raoul_fight() {
	int raoul_fight_reset_frame;

	if (kernel_anim[aa[2]].frame != local->raoul_fight_frame) {
		local->raoul_fight_frame = kernel_anim[aa[2]].frame;
		raoul_fight_reset_frame = -1;

		switch (local->raoul_fight_frame) {
		case 22:  /* almost end of climbing down rope */
			conv_release();
			break;

		case 23:  /* almost end of climbing down rope */
			if (local->raoul_fight_action != CONV23_RAOUL_TALK) {
				raoul_fight_reset_frame = 22;
			}
			break;

		case 25:  /* end of climbing down rope & talk 1*/
		case 26:  /* end of talk 2 */
		case 27:  /* end of talk 3 */
			if (local->raoul_fight_action == CONV23_RAOUL_TALK) {
				raoul_fight_reset_frame = imath_random(24, 26);
				++local->raoul_fight_talk_count;
				if (local->raoul_fight_talk_count > 17) {
					raoul_fight_reset_frame = 24;
				}
			}
			break;

		case 28:
			local->raoul_fight_action = CONV23_RAOUL_TALK;
			break;

		case 45:
			conv_release();
			break;

		case 46:  /* end of jumping down to second level and talk 1 */
		case 47:  /* end of talk 2 */
		case 48:  /* end of talk 3 */
			if (local->raoul_fight_action == CONV23_RAOUL_TALK) {
				raoul_fight_reset_frame = imath_random(45, 47);
				++local->raoul_fight_talk_count;
				if (local->raoul_fight_talk_count > 17) {
					raoul_fight_reset_frame = 45;
				}
			}
			break;
		}

		if (raoul_fight_reset_frame >= 0) {
			kernel_reset_animation(aa[2], raoul_fight_reset_frame);
			local->raoul_fight_frame = raoul_fight_reset_frame;
		}
	}
}

void room_304_daemon() {
	if (local->anim_0_running) {
		handle_animation_raoul();
	}

	if (local->anim_1_running) {
		handle_animation_phantom();
	}

	if (local->anim_2_running) {
		handle_animation_raoul_fight();
	}
}

void room_304_preload() {
	room_init_code_pointer = room_304_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = room_304_parser;
	room_daemon_code_pointer = room_304_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
