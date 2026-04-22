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

#include "engines/util.h"
#include "mads/madsv2/console.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/copy.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/himem.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/phantom.h"
#include "mads/madsv2/phantom/main.h"
#include "mads/madsv2/phantom/sound_phantom.h"
#include "mads/madsv2/phantom/catacombs.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/section2.h"
#include "mads/madsv2/phantom/rooms/section3.h"
#include "mads/madsv2/phantom/rooms/section4.h"
#include "mads/madsv2/phantom/rooms/section5.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

Common::Error PhantomEngine::run() {
	initGraphics(320, 200);
	_screen = new Graphics::Screen();
	scr_live.data = (byte *)_screen->getPixels();

	// Create a debugger console
	setDebugger(new Console());

	// Set up sound manager
	_soundManager = new PhantomSoundManager(_mixer, _soundFlag);
	_soundManager->validate();

	// Run the game
	Phantom::phantom_main();

	return Common::kNoError;
}

void PhantomEngine::global_init_code() {
	Common::fill(global, global + GLOBAL_LIST_SIZE, 0);

	global_catacombs_setup();

	/* Section 1 Initialization */

	player.facing = FACING_NORTH;
	player.turn_to_facing = FACING_NORTH;

	global[temp_var] = false;
	global[room_103_104_transition] = NEW_ROOM;
	global[current_year] = 1993;
	global[trap_door_status] = TRAP_DOOR_IS_CLOSED;
	global[christine_door_status] = CHRIS_IS_IN;
	global[sandbag_status] = SANDBAG_SECURE;
	global[jacques_status] = JACQUES_IS_ALIVE;
	global[chris_f_status] = CHRIS_F_IS_ALIVE;
	global[brie_talk_status] = BEFORE_BRIE_MOTIONS;
	global[panel_in_206] = PANEL_UNDISCOVERED;
	global[fight_status] = FIGHT_NOT_HAPPENED;
	global[julies_door] = CRACKED_OPEN;
	global[prompter_stand_status] = PROMPT_LEFT;
	global[chris_d_status] = BEFORE_LOVE;
	global[julie_name_is_known] = JULIE_NO;
	global[doors_in_205] = BOTH_LOCKED;
	global[madame_giry_loc] = MIDDLE;
	global[ticket_people_here] = NEITHER;
	global[coffin_status] = COFFIN_CLOSED;
	global[done_brie_conv_203] = NO;
	global[florent_name_is_known] = NO;
	global[degas_name_is_known] = NO;
	global[madame_giry_shows_up] = false;
	global[jacques_name_is_known] = NO;
	global[charles_name_is_known] = false;
	global[top_floor_locked] = true;
	global[madame_name_is_known] = NO;
	global[chris_kicked_raoul_out] = false;
	global[looked_at_case] = false;
	global[ring_is_on_finger] = false;
	global[he_listened] = false;
	global[knocked_over_head] = false;
	global[observed_phan_104] = false;
	global[read_book] = false;
	global[can_find_book_library] = false;
	global[looked_at_skull_face] = false;
	global[scanned_bookcase] = false;
	global[ran_conv_in_205] = false;
	global[done_rich_conv_203] = false;
	global[hint_that_daae_is_home_1] = false;
	global[hint_that_daae_is_home_2] = false;
	global[make_brie_leave_203] = false;
	global[make_rich_leave_203] = false;
	global[came_from_fade] = false;
	global[christine_told_envelope] = false;
	global[leave_angel_music_on] = false;
	global[door_in_409_is_open] = false;
	global[priest_piston_puke] = false;
	global[cob_web_is_cut] = false;
	global[christine_is_in_boat] = false;
	global[right_door_is_open_504] = false;
	global[chris_left_505] = false;
	global[chris_will_take_seat] = true;
	global[flicked_1] = 0;
	global[flicked_2] = 0;
	global[flicked_3] = 0;
	global[flicked_4] = 0;
	global[player_score] = 0;
	global[player_score_flags] = 0;

	global[music_selected] = imath_random(TOCCATA_FUGUE_D_MINOR, FUGUE_C_MINOR);

	inter_object_routine = global_object_sprite;


	/* Global preload items */

	player_himem_preload("RAL", GLOBAL);
	himem_preload_series("*BOX", GLOBAL);
	himem_preload_series("*LOGO", GLOBAL);
	himem_preload_series("*MENU", GLOBAL);
	himem_preload_series("*CURSOR", GLOBAL);
	himem_preload_series("*FACERAL", GLOBAL);
	himem_preload_series("*RRD_8", GLOBAL);
	himem_preload_series("*RRD_9", GLOBAL);
	himem_preload_series("*RDR_6", GLOBAL);
	himem_preload_series("*RDR_9", GLOBAL);
	himem_preload_series("*RTK_6", GLOBAL);
	himem_preload_series("*RTK_9", GLOBAL);
	himem_preload_series("*RALRH_9", GLOBAL);
}

void PhantomEngine::section_music(int section_num) {
	switch (section_num) {
	case 1: Rooms::section_1_music(); break;
	case 2: Rooms::section_2_music(); break;
	case 3: Rooms::section_3_music(); break;
	case 4: Rooms::section_4_music(); break;
	case 5: Rooms::section_5_music(); break;
	}
}

void PhantomEngine::global_object_sprite() {
	if (inter_object_id == 1 && global[lantern_status] == LANTERN_IS_ON)
		inter_object_id = 31;

	Common::strcpy_s(inter_object_buf, "*OB");
	env_catint(inter_object_buf, inter_object_id, 3);
	Common::strcat_s(inter_object_buf, "I");
}


void PhantomEngine::stop_walker_basic() {
	int random;
	int count;
	int how_many;

	random = imath_random(1, 30000);

	switch (player.facing) {
	case FACING_SOUTH:
		if (random < 500) {
			how_many = imath_random(4, 10);
			for (count = 0; count < how_many; count++) {
				player_add_stop_walker((random < 250) ? 1 : 2, 0);
			}
		} else if (random < 750) {
			for (count = 0; count < 4; count++) {
				player_add_stop_walker(1, 0);
			}

			player_add_stop_walker(0, 0);

			for (count = 0; count < 4; count++) {
				player_add_stop_walker(2, 0);
			}

			player_add_stop_walker(0, 0);
		}
		break;

	case FACING_SOUTHEAST:
	case FACING_SOUTHWEST:
	case FACING_NORTHEAST:
	case FACING_NORTHWEST:
		if (random < 150) {
			player_add_stop_walker(-1, 0);
			player_add_stop_walker(1, 0);
			for (count = 0; count < 6; count++) {
				player_add_stop_walker(0, 0);
			}
		}
		break;

	case FACING_EAST:
	case FACING_WEST:
		if (random < 250) {
			player_add_stop_walker(-1, 0);
			how_many = imath_random(2, 6);
			for (count = 0; count < how_many; count++) {
				player_add_stop_walker(2, 0);
			}
			player_add_stop_walker(1, 0);
			player_add_stop_walker(0, 0);
			player_add_stop_walker(0, 0);
		} else if (random < 500) {
			WRITE_LE_UINT32(&global[walker_timing], kernel.clock);
		}
		break;

	case FACING_NORTH:
		if (random < 250) {
			player_add_stop_walker(-1, 0);
			how_many = imath_random(3, 7);
			for (count = 0; count < how_many; count++) {
				player_add_stop_walker(2, 0);
			}
			player_add_stop_walker(1, 0);
			player_add_stop_walker(0, 0);
		}
		break;

	}
}

void PhantomEngine::stop_walker_tricks() {
	int state;
	int cmd;
	int random;

	state = global[walker_converse_state];
	cmd = global[walker_converse];

	global[walker_converse_now] = state;

	if ((player.facing != FACING_NORTHEAST) &&
		(player.facing != FACING_NORTHWEST)) {
		state = CONVERSE_NONE;
		cmd = CONVERSE_NONE;
	}

	switch (state) {
	case CONVERSE_LEAN:
		switch (cmd) {
		case CONVERSE_LEAN:
			player_add_stop_walker(3, 0);
			break;

		case CONVERSE_HAND_WAVE:
		case CONVERSE_HAND_WAVE_2:
			player_add_stop_walker(6, 0);
			player_add_stop_walker(5, 0);
			player_add_stop_walker(4, 0);
			state = CONVERSE_HAND_WAVE;
			break;

		case CONVERSE_HAND_CHIN:
			player_add_stop_walker(8, 0);
			player_add_stop_walker(4, 0);
			state = CONVERSE_HAND_CHIN;
			break;

		case CONVERSE_NONE:
		default:
			player_add_stop_walker(-2, 0);
			state = CONVERSE_NONE;
			break;
		}
		break;

	case CONVERSE_HAND_WAVE:
	case CONVERSE_HAND_WAVE_2:
		switch (cmd) {
		case CONVERSE_HAND_WAVE:
		case CONVERSE_HAND_WAVE_2:
			random = imath_random(1, 30000);

			if (state == CONVERSE_HAND_WAVE) {
				if (random < 2000) {
					player_add_stop_walker(10, 0);
					player_add_stop_walker(7, 0);
					state = CONVERSE_HAND_WAVE_2;
				} else {
					player_add_stop_walker(6, 0);
				}
			} else {
				if (random < 1000) {
					player_add_stop_walker(6, 0);
					player_add_stop_walker(7, 0);
					state = CONVERSE_HAND_WAVE;
				} else {
					player_add_stop_walker(10, 0);
				}
			}
			break;

		case CONVERSE_LEAN:
		case CONVERSE_HAND_CHIN:
		case CONVERSE_NONE:
		default:
			player_add_stop_walker(-4, 0);
			player_add_stop_walker(-5, 0);
			if (state == CONVERSE_HAND_WAVE_2) {
				player_add_stop_walker(6, 0);
				player_add_stop_walker(7, 0);
			}
			state = CONVERSE_LEAN;
			break;
		}
		break;

	case CONVERSE_HAND_CHIN:
		switch (cmd) {
		case CONVERSE_HAND_CHIN:
			player_add_stop_walker(9, 0);
			break;

		case CONVERSE_LEAN:
		case CONVERSE_HAND_WAVE:
		case CONVERSE_HAND_WAVE_2:
		case CONVERSE_NONE:
		default:
			player_add_stop_walker(-4, 0);
			player_add_stop_walker(-8, 0);
			state = CONVERSE_LEAN;
			break;
		}
		break;

	case CONVERSE_NONE:
	default:
		switch (cmd) {
		case CONVERSE_LEAN:
		case CONVERSE_HAND_WAVE:
		case CONVERSE_HAND_WAVE_2:
		case CONVERSE_HAND_CHIN:
			player_add_stop_walker(2, 0);
			state = CONVERSE_LEAN;
			break;

		case CONVERSE_NONE:
		default:
			stop_walker_basic();
			break;
		}
		break;
	}

	global[walker_converse] = cmd;
	global[walker_converse_state] = state;
}

void PhantomEngine::global_section_constructor() {
	Phantom::global_section_constructor();
}

void PhantomEngine::global_daemon_code() {
	if (player.walker_visible && !global[stop_walker_disabled] && (player.commands_allowed || (conv_control.running >= 0)) && !player.walking &&
		(player.facing == player.turn_to_facing)) {
		if (kernel.clock >= READ_LE_INT32(&global[walker_timing])) {
			if (!player.stop_walker_pointer) {
				stop_walker_tricks();
			}

			WRITE_LE_INT32(&global[walker_timing], READ_LE_INT32(&global[walker_timing]) + 6);
		}
	}
}

void PhantomEngine::global_pre_parser_code() {
	if (player_said_1(look) || player_said_1(throw)) {
		player.need_to_walk = false;
	}
}

void PhantomEngine::global_parser_code() {
	if (room_id >= 401 && room_id <= 456) {
		if (player_said_1(red_frame) ||
			player_said_1(yellow_frame) ||
			player_said_1(green_frame) ||
			player_said_1(blue_frame)) {

			if (player_said_1(put)) {

				if (player_said_1(unlucky_adventurer)) {
					text_show(text_000_35);
					goto handled;

				} else if (player_said_1(hole)) {
					text_show(text_000_36);
					goto handled;

				} else if (player_said_1(grate)) {
					text_show(text_000_37);
					goto handled;

				} else if (player_said_1(wall)) {
					text_show(text_000_38);
					goto handled;
				}
			}
		}
	}

	if (player.look_around) {
		text_show(text_008_10);
		goto handled;
	}

	if (player_said_2(put, red_frame) ||
		player_said_2(put, blue_frame) ||
		player_said_2(put, yellow_frame) ||
		player_said_2(put, green_frame)) {
		if (player_said_1(puddle)) {
			text_show(text_401_24);
		} else {
			text_show(text_401_25);
		}
		goto handled;
	}

	if (player_said_2(wear, wedding_ring)) {
		if (global[ring_is_on_finger]) {
			text_show(text_008_49);
		} else {
			text_show(text_008_35);
			global[ring_is_on_finger] = true;
		}
		goto handled;
	}

	if (player_said_2(remove, wedding_ring)) {
		if (!global[ring_is_on_finger]) {
			text_show(text_008_48);
		} else {
			text_show(text_008_36);
			global[ring_is_on_finger] = false;
		}
		goto handled;
	}

	if (player_said_2(look, archway_to_west) || player_said_2(look, archway_to_east) ||
		player_said_2(look, archway_to_north)) {
		text_show(text_000_34);
		goto handled;
	}

	if (player_said_2(attack, Christine)) {
		text_show(text_000_33);
		goto handled;
	}

	if (player_said_2(look, key)) {
		object_examine(key, text_008_00, 0);
		goto handled;
	}

	if (player_said_2(look, sandbag)) {
		object_examine(sandbag, text_008_03, 0);
		goto handled;
	}

	if (player_said_2(look, small_note) || player_said_2(read, small_note)) {
		object_examine(small_note, text_008_06, 2);
		goto handled;
	}

	if (player_said_2(look, rope)) {
		object_examine(rope, text_008_07, 0);
		goto handled;
	}

	if (player_said_2(look, sword)) {
		object_examine(sword, text_008_08, 0);
		goto handled;
	}

	if (player_said_2(look, envelope) || player_said_2(read, envelope)) {
		object_examine(envelope, text_008_09, 0);
		goto handled;
	}

	if (player_said_2(look, ticket) || player_said_2(read, ticket)) {
		object_examine(ticket, text_008_10, 0);
		goto handled;
	}

	if (player_said_2(look, piece_of_paper) || player_said_2(read, piece_of_paper)) {
		object_examine(piece_of_paper, text_008_11, 1);
		goto handled;
	}

	if (player_said_2(look, parchment) || player_said_2(read, parchment)) {
		object_examine(parchment, text_008_12, 3);
		goto handled;
	}

	if (player_said_2(look, letter) || player_said_2(read, letter)) {
		object_examine(letter, text_008_13, 4);
		goto handled;
	}

	if (player_said_2(look, notice) || player_said_2(read, notice)) {
		object_examine(notice, text_008_14, 5);
		goto handled;
	}

	if (player_said_2(look, book) || player_said_2(read, book)) {
		object_examine(book, text_008_15, 0);
		goto handled;
	}

	if (player_said_2(look, crumpled_note) || player_said_2(read, crumpled_note)) {
		object_examine(crumpled_note, text_008_16, 6);
		goto handled;
	}

	if (player_said_2(look, large_note) || player_said_2(read, large_note)) {
		object_examine(large_note, text_008_18, 7);
		goto handled;
	}

	if (player_said_2(look, music_score) || player_said_2(read, music_score)) {
		object_examine(music_score, text_008_20, 0);
		goto handled;
	}

	if (player_said_2(look, wedding_ring)) {
		object_examine(wedding_ring, text_008_21, 0);
		goto handled;
	}

	if (player_said_2(look, cable_hook)) {
		object_examine(cable_hook, text_008_22, 0);
		goto handled;
	}

	if (player_said_2(look, rope_with_hook)) {
		object_examine(rope_with_hook, text_008_23, 0);
		goto handled;
	}

	if (player_said_2(look, oar)) {
		object_examine(oar, text_008_24, 0);
		goto handled;
	}


	if (player_said_1(look) && player_has(object_named(player_main_noun))) {
		object_examine(oar, text_008_24, 0);
		goto handled;
	}

	if (player_said_2(turn_on, lantern)) {
		if ((global[lantern_status] == LANTERN_IS_ON) && !kernel.trigger) {
			text_show(text_008_28);
			/* lantern is already on */
		} else {
			switch (kernel.trigger) {
			case 0:
				kernel_timing_trigger(4, 1);
				global[lantern_status] = LANTERN_IS_ON;
				inter_spin_object(lantern);
				break;

			case 1:
				text_show(text_008_25);
				break;
			}
		}
		goto handled;
	}

	if (player_said_2(turn_off, lantern)) {
		if ((global[lantern_status] == LANTERN_IS_OFF) && !kernel.trigger) {
			text_show(text_008_29);
			/* lantern is already off */
		} else {
			if (section_id == 4) {
				text_show(text_008_26);
				/* you are in catacombs, not a good idea */
			} else {
				switch (kernel.trigger) {
				case 0:
					kernel_timing_trigger(4, 1);
					global[lantern_status] = LANTERN_IS_OFF;
					inter_spin_object(lantern);
					break;

				case 1:
					text_show(text_008_27);
					break;
				}
			}
		}
		goto handled;
	}

	if (player_said_2(open, envelope)) {
		inter_move_object(envelope, NOWHERE);
		inter_give_to_player(ticket);
		inter_give_to_player(piece_of_paper);
		text_show(text_008_33);
		goto handled;
	}

	if (player_said_3(attach, cable_hook, rope)) {
		if (!player_has(rope)) {
			text_show(text_114_38);
		} else {
			inter_move_object(cable_hook, NOWHERE);
			inter_move_object(rope, NOWHERE);
			inter_give_to_player(rope_with_hook);
			object_examine(rope_with_hook, text_008_23, 0);
			/* text_show (text_008_23); */
		}
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void PhantomEngine::global_error_code() {
	int show_me = 0;
	int item;
	int random;

	random = imath_random(1, 1000);

	if (player_said_3(put, chandelier, seats)) {
		text_show(text_101_23);
		goto done;
	}

	if (player_said_1(take)) {
		item = object_named(player_main_noun);
		if (player_has(item)) {
			show_me = text_000_25;
			/* player already has it */

		} else {
			if (random <= 333) {
				show_me = text_000_01;
			} else if (random <= 666) {
				show_me = text_000_02;
			} else {
				show_me = text_000_03;
			}
		}
		goto done;
	}

	if (player_said_1(push)) {
		if (random < 750) {
			show_me = text_000_04;
		} else {
			show_me = text_000_05;
		}
		goto done;
	}

	if (player_said_1(pull)) {
		if (random < 750) {
			show_me = text_000_06;
		} else {
			show_me = text_000_07;
		}
		goto done;
	}

	if (player_said_1(open)) {
		if (random <= 500) {
			show_me = text_000_08;
		} else if (random <= 750) {
			show_me = text_000_09;
		} else {
			show_me = text_000_10;
		}
		goto done;
	}

	if (player_said_1(close)) {
		if (random <= 500) {
			show_me = text_000_11;
		} else if (random <= 750) {
			show_me = text_000_12;
		} else {
			show_me = text_000_13;
		}
		goto done;
	}

	if (player_said_1(put)) {
		item = object_named(player_main_noun);
		if (player_has(item)) {
			show_me = text_000_26;
		} else if (random < 500) {
			show_me = text_000_14;
		} else {
			show_me = text_000_15;
		}
		goto done;
	}

	if (player_said_1(talk_to)) {
		if (random <= 500) {
			show_me = text_000_16;
		} else {
			show_me = text_000_17;
		}
		goto done;
	}

	if (player_said_1(give)) {
		item = object_named(player_main_noun);
		if (player_has(item)) {
			show_me = text_000_27;
		} else {
			show_me = text_000_18;
		}
		goto done;
	}

	if (player_said_1(throw)) {
		item = object_named(player_main_noun);
		if (player_has(item)) {
			show_me = text_000_19;
		} else {
			show_me = text_000_28;
		}
		goto done;
	}

	if (player_said_1(look)) {
		item = object_named(player_main_noun);
		if (random <= 333) {
			show_me = text_000_20;
		} else if (random <= 666) {
			show_me = text_000_21;
		} else {
			show_me = text_000_22;
		}
		goto done;
	}

	if (player_said_1(unlock) || player_said_1(lock)) {
		if (player_said_1(door) || player_said_1(left_door) ||
			player_said_1(middle_door) || player_said_1(right_door) ||
			player_said_1(trap_door)) {
			show_me = text_000_32;
			goto done;
		}
	}

	if (!player_said_1(walk_to) && !player_said_1(walk_across) &&
		!player_said_1(walk_down) && !player_said_1(walk_up)) {
		if (random < 500) {
			show_me = text_000_23;
		} else {
			show_me = text_000_24;
		}
		goto done;
	}


done:
	if (show_me) text_show(show_me);
}

void PhantomEngine::global_room_init() {
	pal_change_color(16, 45, 45, 55);
	pal_change_color(17, 30, 30, 45);
	global[walker_converse] = 0;
	global[walker_converse_state] = 0;
}

void PhantomEngine::global_sound_driver() {
	Common::strcpy_s(kernel.sound_driver, "*#SOUND.PH");
	env_catint(kernel.sound_driver, new_section, 1);
}

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
