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
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/screen.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/dragonsphere.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/main.h"
#include "mads/madsv2/dragonsphere/sound_dragonsphere.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/section2.h"
#include "mads/madsv2/dragonsphere/rooms/section3.h"
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/section5.h"
#include "mads/madsv2/dragonsphere/rooms/section6.h"
#include "mads/madsv2/dragonsphere/rooms/section9.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {

Common::Error DragonsphereEngine::run() {
	initGraphics(320, 200);
	_screen = new Graphics::Screen();
	scr_live.data = (byte *)_screen->getPixels();

	// Create a debugger console
	setDebugger(new Console());

	// Set up sound manager
	_soundManager = new DragonSoundManager(_mixer, _soundFlag);
	_soundManager->validate();

	// Run the game
	Dragonsphere::dragonsphere_main();

	return Common::kNoError;
}

void DragonsphereEngine::global_init_code() {
	int count;

	for (count = 0; count < GLOBAL_LIST_SIZE; count++) {
		global[count] = 0;
	}

	/* initialize_grid (); */

	global[grid_position] = 5;
	global[grid_position + 1] = 4;
	global[grid_position + 2] = 9;
	global[grid_position + 3] = 10;
	global[grid_position + 4] = 15;
	global[grid_position + 5] = 20;
	global[grid_position + 6] = 19;
	global[grid_position + 7] = 24;
	global[grid_position + 8] = 23;
	global[grid_position + 9] = 18;

	global[grid_position + 10] = 13;
	global[grid_position + 11] = 8;
	global[grid_position + 12] = 3;
	global[grid_position + 13] = 2;
	global[grid_position + 14] = 1;
	global[grid_position + 15] = 6;
	global[grid_position + 16] = 7;
	global[grid_position + 17] = 12;

	global[grid_position + 18] = 17;
	global[grid_position + 19] = 22;
	global[grid_position + 20] = 21;
	global[grid_position + 21] = 16;
	global[grid_position + 22] = 11;

	global[max_grid_value] = grid_position + 22;

	global[oasis] = imath_random(1, 11);
	switch (global[oasis]) {
	case 1:  global[oasis] = 7;  break;
	case 2:  global[oasis] = 13; break;
	case 3:  global[oasis] = 19; break;
	case 4:  global[oasis] = 25; break;
	case 5:  global[oasis] = 31; break;
	case 6:  global[oasis] = 37; break;
	case 7:  global[oasis] = 45; break;
	case 8:  global[oasis] = 53; break;
	case 9:  global[oasis] = 61; break;
	case 10: global[oasis] = 69; break;
	case 11: global[oasis] = 77; break;
	}

	do {
		global[fire_holes] = imath_random(1, 11);
		switch (global[fire_holes]) {
		case 1:  global[fire_holes] = 7;  break;
		case 2:  global[fire_holes] = 13; break;
		case 3:  global[fire_holes] = 19; break;
		case 4:  global[fire_holes] = 25; break;
		case 5:  global[fire_holes] = 31; break;
		case 6:  global[fire_holes] = 37; break;
		case 7:  global[fire_holes] = 45; break;
		case 8:  global[fire_holes] = 53; break;
		case 9:  global[fire_holes] = 61; break;
		case 10: global[fire_holes] = 69; break;
		case 11: global[fire_holes] = 77; break;
		}
	} while (global[fire_holes] == global[oasis]);

	/* Section 1 Initialization */

	global[player_score] = 0;
	global[player_score_flags] = 0;

	global[dragon_high_scene] = 0;
	global[dragon_my_scene] = 0;
	global[player_persona] = PLAYER_IS_KING;
	global[king_status] = KING_CAPTIVE;
	global[ward_status] = WARD_PRESENT;
	global[amulet_status] = AMULET_NOT_CORRECT_TIME;
	global[books_status] = BOOKS_NOT_PRESENT;
	global[tapestry_status] = TAPESTRY_CLOSED;
	global[talked_to_status] = TALKED_TO_NONE;
	global[guard_pid_status] = GUARD_NEVER_HEALED;

	global[crawled_out_of_bed_101] = false;
	global[statue_is_on_stairway_114] = false;
	global[dog_is_asleep] = false;
	global[crystal_ball_dead] = false;
	global[threw_bone] = false;
	global[no_load_walker] = false;
	global[wooden_door_open] = false;
	global[king_is_in_stairwell] = false;
	global[llanie_status] = BEFORE_FALL;
	global[no_talk_to_guard] = false;
	global[sorceror_defeated] = false;
	global[waterfall_diverted] = false;
	global[shak_status] = SHAK_NEVER_MET;
	global[move_direction_510] = true;
	global[move_direction_409] = true;
	global[shak_506_angry] = false;
	global[monster_is_dead] = false;
	global[done_talking_lani_502] = false;
	global[found_lani_504] = false;
	global[said_use_sword_shak] = false;
	global[goblet_filled_soporific] = false;
	global[been_in_504_as_pid] = false;
	global[seen_lani_dead_1st_time] = false;
	global[said_poem_in_504] = false;
	global[tried_to_heal_llanie_504] = false;
	global[put_bundle_on_llanie_504] = false;
	global[mud_is_in_eye_603] = false;
	global[rope_is_alive] = true;
	global[make_504_empty] = false;
	global[rat_cage_is_open] = false;
	global[flask_on_plate] = NONE;
	global[fluid_is_dripping] = false;
	global[hole_is_in_607] = false;
	global[rope_is_hanging_in_607] = false;
	global[object_is_in_freezer_605] = NEVER_USED_FREEZER;
	global[has_taken_mud] = false;
	global[desert_room] = 42;
	global[object_imitated] = -1;
	global[has_red] = false;
	global[has_yellow] = false;
	global[has_blue] = false;
	global[wizard_dead] = false;
	global[vine_will_grab] = true;
	global[desert_counter] = 0;
	global[floor_is_cool] = false;
	global[rat_melted] = false;
	global[door_is_cool] = false;
	global[used_elevator] = false;
	global[been_on_top_floor] = false;
	global[torch_is_in_609] = false;
	global[platform_clicked_606] = false;
	global[perform_displacements] = true;
	global[had_spirit_bundle] = false;
	global[heal_verbs_visible] = false;
	global[pid_talk_shamon] = false;
	global[talked_to_soptus] = false;
	global[pid_just_died] = false;
	global[grapes_have_grown] = GRAPES_NOT_THERE;
	global[grapes_are_dead] = false;
	global[roc_is_chewing_dates] = false;
	global[wins_in_desert] = 0;
	global[wins_till_prize] = 3;
	global[game_points] = 0;
	global[dance_points] = 0;
	global[clue_points] = 0;
	global[prizes_owed_to_player] = 0; /* for king */
	global[pid_has_been_healed_sop] = false;
	global[object_flags] = 0;
	global[save_wins_in_desert] = 0;
	global[object_given_201] = -1;
	global[king_got_stabbed] = false;
	global[given_object_before] = false;
	global[guards_are_asleep] = false;
	global[dome_up] = false;
	global[talked_to_wise] = false;
	global[talked_to_shifter] = false;
	global[talked_to_merchant] = false;
	global[doll_given] = false;
	global[reset_conv] = -1;
	global[reset_conv_2] = -1;
	global[talked_to_greta] = false;
	global[slime_healed] = false;
	global[dance_music_on] = false;
	global[bubbles_up_in_301] = true;
	global[bubble_wont_attack] = false;
	global[can_view_crown_hole] = false;
	global[player_is_seal] = false;
	global[vines_have_player] = false;
	global[end_of_game] = false;
	global[pid_looked_at_doll] = false;
	global[invoked_from_111] = false;

	player.facing = FACING_NORTH;
	player.turn_to_facing = FACING_NORTH;
}

void DragonsphereEngine::section_music(int section_num) {
	switch (section_num) {
	case 1: Rooms::section_1_music(); break;
	case 2: Rooms::section_2_music(); break;
	case 3: Rooms::section_3_music(); break;
	case 4: Rooms::section_4_music(); break;
	case 5: Rooms::section_5_music(); break;
	case 6: Rooms::section_6_music(); break;
	case 9: Rooms::section_9_music(); break;
	}
}

void DragonsphereEngine::global_section_constructor() {
	Dragonsphere::global_section_constructor();
}

void DragonsphereEngine::syncRoom(Common::Serializer &s) {
	Dragonsphere::sync_room(s);
}

void DragonsphereEngine::global_daemon_code() {
	int random;
	int count;
	int how_many;

	if (player.walker_visible && (player.commands_allowed || (conv_control.running >= 0)) && !player.walking &&
		(player.facing == player.turn_to_facing) && global[perform_displacements]) {

		if (kernel.clock >= READ_LE_INT32(&global[walker_timing])) {
			if (!player.stop_walker_pointer) {
				random = imath_random(1, 30000);

				if (global[player_persona] == PLAYER_IS_KING) {
					if (random < 500) {
						switch (player.facing) {
						case FACING_SOUTHEAST:
						case FACING_SOUTHWEST:
							how_many = imath_random(0, 3);
							if (how_many <= 2) {
								for (count = 0; count < imath_random(5, 7); count++) {
									player_add_stop_walker(how_many, 0);
								}
							} else {
								player_add_stop_walker(-3, 0);
								for (count = 0; count < imath_random(5, 7); count++) {
									player_add_stop_walker(4, 0);
								}
								player_add_stop_walker(3, 0);
							}
							break;

						case FACING_NORTHEAST:
						case FACING_NORTHWEST:
							how_many = imath_random(0, 1);
							for (count = 0; count < imath_random(5, 7); count++) {
								player_add_stop_walker(how_many, 0);
							}
							break;

						case FACING_SOUTH:
							how_many = imath_random(0, 3);
							if (how_many <= 2) {
								for (count = 0; count < imath_random(5, 7); count++) {
									player_add_stop_walker(how_many, 0);
								}

							} else {
								player_add_stop_walker(-3, 0);
								for (count = 0; count < imath_random(5, 7); count++) {
									player_add_stop_walker(4, 0);
								}
								player_add_stop_walker(3, 0);
							}
							break;

						case FACING_NORTH:
							how_many = imath_random(0, 2);
							for (count = 0; count < imath_random(5, 7); count++) {
								player_add_stop_walker(how_many, 0);
							}
							break;

						case FACING_EAST:
						case FACING_WEST:
						default:
							how_many = imath_random(0, 2);
							if (how_many <= 1) {
								for (count = 0; count < imath_random(5, 7); count++) {
									player_add_stop_walker(how_many, 0);
								}
							} else {
								player_add_stop_walker(-2, 0);
								for (count = 0; count < imath_random(5, 7); count++) {
									player_add_stop_walker(3, 0);
								}
								player_add_stop_walker(2, 0);
							}
							break;
						}
					}

				} else {  /* player is PID */

					if (random < 500) {
						switch (player.facing) {
						case FACING_SOUTHEAST:
						case FACING_SOUTHWEST:
							how_many = imath_random(0, 3);
							if (how_many <= 2) {
								for (count = 0; count < imath_random(5, 7); count++) {
									player_add_stop_walker(how_many, 0);
								}
							} else {
								player_add_stop_walker(-3, 0);
								for (count = 0; count < imath_random(5, 7); count++) {
									player_add_stop_walker(4, 0);
								}
								player_add_stop_walker(3, 0);
							}
							break;

						case FACING_NORTHEAST:
						case FACING_NORTHWEST:
							how_many = imath_random(0, 2);
							for (count = 0; count < imath_random(5, 7); count++) {
								player_add_stop_walker(how_many, 0);
							}
							break;

						case FACING_SOUTH:
							how_many = imath_random(0, 3);
							if (how_many <= 2) {
								for (count = 0; count < imath_random(5, 7); count++) {
									player_add_stop_walker(how_many, 0);
								}
							} else {
								player_add_stop_walker(-3, 0);
								player_add_stop_walker(3, 0);
							}
							break;

						case FACING_NORTH:
							how_many = imath_random(0, 2);
							for (count = 0; count < imath_random(5, 7); count++) {
								player_add_stop_walker(how_many, 0);
							}
							break;

						case FACING_EAST:
						case FACING_WEST:
						default:
							how_many = imath_random(0, 2);
							if (how_many <= 1) {
								for (count = 0; count < imath_random(5, 7); count++) {
									player_add_stop_walker(how_many, 0);
								}
							} else {
								player_add_stop_walker(3, 0);
								player_add_stop_walker(2, 0);
							}
							break;
						}

					} else if (random < 1000) {
						WRITE_LE_UINT32(&global[walker_timing], kernel.clock);
					}
				}

			}

			WRITE_LE_UINT32(&global[walker_timing], READ_LE_UINT32(&global[walker_timing]) + 10);
		}
	}
}

void DragonsphereEngine::global_pre_parser_code() {
	if (player_said_1(look) || player_said_1(throw)) {
		player.need_to_walk = false;
	}
}

void DragonsphereEngine::global_parser_code() {
	int id;
	int idd;

	id = object_named(player_second_noun);
	idd = object_named(player_main_noun);

	if (player_said_1(revert) && idd == polystone) {
		if (global[object_imitated] == -1 || global[object_imitated] == 9) {
			text_show(708);

		} else {
			global[object_imitated] = 9;
			inter_move_object(idd, NOWHERE);
			inter_give_to_player(idd);
			object_examine(polystone, 701, 0);
		}
		goto handled;
	}

	if (player_said_1(mimic)) {
		if (player_has(id)) {
			if (room_id == 614 && id == blue_powerstone) {
				text_show(61422);
			}

			if ((id == 9 && idd == 9) && (global[object_imitated] == -1 || global[object_imitated] == 9)) {
				text_show(708);

			} else {
				global[object_imitated] = id;
				
				inter_move_object(id, NOWHERE);
				inter_give_to_player(id);
				if (id == 9) {
					object_examine(polystone, 701, 0);
				} else {
					object_examine(polystone, 933, 0);
				}
			}

		} else {
			text_show(707);
		}
		goto handled;
	}

	if (player_said_2(polish, signet_ring)) {
		object_examine(signet_ring, 901, 0);
		goto handled;
	}

	if (player_said_2(invoke, red_stone) ||
		player_said_2(invoke, yellow_stone) ||
		player_said_2(invoke, blue_stone)) {
		text_show(705);
		goto handled;
	}

	if (player_said_1(pour_contents_of)) {
		if (id == flask_full_of_acid && idd == flask_full_of_acid) {
			text_show(23);
		} else {
			text_show(958);
		}
		goto handled;
	}

	if (player_said_1(drink) && idd == flask_full_of_acid) {
		text_show(957);
		goto handled;
	}

	if (player_said_2(put_magic_into, partial_bundle)) {
		if (player_has(partly_built_bundle)) {
			if (global[had_spirit_bundle]) {
				inter_move_object(partly_built_bundle, NOWHERE);
				inter_give_to_player(new_bundle);
				object_examine(new_bundle, 977, 0);

			} else {
				text_show(998);
			}
		}
		goto handled;
	}

	if (player_said_1(put_magic_into)) {
		text_show(978);
		goto handled;
	}

	if (player_said_2(polish, red_stone)) {
		object_examine(red_powerstone, 935, 0);
		goto handled;
	}

	if (player_said_2(polish, yellow_stone)) {
		object_examine(yellow_powerstone, 703, 0);
		goto handled;
	}

	if (player_said_2(polish, blue_stone)) {
		object_examine(blue_powerstone, 704, 0);
		goto handled;
	}

	if (player_said_2(give, signet_ring)) {
		if (player_said_1(king)) {
			text_show(902);
		} else {
			text_show(917);
		}
		goto handled;
	}

	if (player_said_2(open, music_box)) {
		sound_play(N_MusicBoxOn);
		object_examine(magic_music_box, 843, 0);
		sound_play(N_MusicBoxOff);
		goto handled;
	}

	if (player_said_2(rub, bird_figurine)) {
		object_examine(bird_figurine, 903, 0);
		goto handled;
	}

	if (player_said_2(talk_to, bird_figurine)) {
		object_examine(bird_figurine, 904, 0);
		goto handled;
	}

	if (player_said_2(rub, birdcall)) {
		object_examine(birdcall, 906, 0);
		goto handled;
	}

	if (player_said_2(polish, shieldstone)) {
		object_examine(shieldstone, 907, 0);
		goto handled;
	}

	if (player_said_2(throw, shieldstone)) {
		text_show(908);
		goto handled;
	}

	if (player_said_1(heal)) {
		if (player_said_1(Llanie) ||
			player_said_1(MacMorn) ||
			player_said_1(shak) ||
			player_said_1(wise_shifter) ||
			player_said_1(Roc) ||
			player_said_1(shifting_monster)) {
			text_show(927);
			goto handled;
		}
	}

	if (player_said_1(heal)) {
		if (player_said_1(Queen_Mother) ||
			player_said_1(stranger) ||
			player_said_1(Ner_Tom) ||
			player_said_1(hermit) ||
			player_said_1(shifter)) {
			text_show(927);
			goto handled;
		}
	}

	if (player_said_1(heal)) {
		if (player_said_1(trader) ||
			player_said_1(lizard) ||
			player_said_1(faerie) ||
			player_said_1(shaman) ||
			player_said_1(guard) ||
			player_said_1(shapechanger) ||
			player_said_1(Soptus_Ecliptus) ||
			player_said_1(merchant)) {
			text_show(927);
			goto handled;
		}
	}

	if (player_said_1(heal)) {
		if (player_said_1(rat) ||
			player_said_1(Sanwe) ||
			player_said_1(king) ||
			player_said_1(guard_captain) ||
			player_said_1(beast) ||
			player_said_1(darkness_beast)) {
			text_show(927);
			goto handled;

		} else {
			text_show(928);
			goto handled;
		}
	}

	if (player_said_1(heal_self)) {
		text_show(929);
		goto handled;
	}

	if (player_said_2(fill, goblet)) {
		if (player_said_1(soporific)) {
			if (global[goblet_filled_soporific]) {
				object_examine(goblet, 994, 0);
			} else {
				object_examine(goblet, 914, 0);
				global[goblet_filled_soporific] = true;
			}
		} else if (player_said_1(flask_full_of_acid)) {
			object_examine(flask_full_of_acid, 993, 0);
		} else {
			object_examine(goblet, 913, 0);
		}
		goto handled;
	}

	if (player_said_2(drink_from, goblet)) {
		if (global[goblet_filled_soporific]) {
			object_examine(goblet, 916, 0);
		} else {
			text_show(915);
		}
		goto handled;
	}

	if (player_said_2(gnaw, bone)) {
		object_examine(bone, 918, 0);
		goto handled;
	}

	if (player_said_2(eat, fruit)) {
		object_examine(fruit, 919, 0);
		goto handled;
	}

	if (player_said_2(throw, fruit)) {
		object_examine(fruit, 921, 0);
		goto handled;
	}

	if (player_said_2(play_with, doll)) {
		object_examine(words_doll, 922, 0);
		goto handled;
	}

	if (player_said_2(talk_to, doll)) {
		object_examine(words_doll, 931, 0);
		goto handled;
	}

	if (player_said_2(open, doll)) {
		object_examine(words_doll, 932, 0);
		goto handled;
	}

	if (player_said_1(polystone)) {
		if (player_said_1(push) || player_said_1(pull)) {
			object_examine(polystone, 934, 0);
			goto handled;
		}
	}

	if (player_said_2(wear, key_crown)) {
		object_examine(key_crown, 936, 0);
		goto handled;
	}

	if (player_said_2(eat, dates)) {
		text_show(938);
		goto handled;
	}

	if (player_said_2(polish, statue)) {
		object_examine(statue, 939, 0);
		goto handled;
	}

	if (player_said_2(listen_to, flies)) {
		sound_play(N_ListenToFlies);
		text_show(940);
		goto handled;
	}

	if (player_said_2(open, flies)) {
		text_show(941);
		goto handled;
	}

	if (player_said_2(talk_to, flies)) {
		text_show(942);
		goto handled;
	}

	if (player_said_2(break, soul_egg)) {
		object_examine(soul_egg, 943, 0);
		goto handled;
	}

	if (player_said_2(adjust, magic_belt)) {
		object_examine(magic_belt, 944, 0);
		goto handled;
	}

	if (player_said_2(feel, mud)) {
		text_show(947);
		goto handled;
	}

	if (player_said_2(taste, mud)) {
		text_show(948);
		goto handled;
	}

	if (player_said_2(throw, mud)) {
		text_show(949);
		goto handled;
	}

	if (player_said_2(feel, feathers)) {
		text_show(950);
		goto handled;
	}

	if (player_said_3(put, feathers, bone)) {
		if (player_has(feathers) && player_has(bone)) {
			inter_move_object(feathers, NOWHERE);
			inter_move_object(bone, NOWHERE);
			inter_give_to_player(partly_built_bundle);
			object_examine(partly_built_bundle, 952, 0);
			goto handled;
		}
	}

	if (player_said_2(wave, torch)) {
		text_show(953);
		goto handled;
	}

	if (player_said_2(throw, torch)) {
		text_show(954);
		goto handled;
	}

	if (player_said_2(polish, flask)) {
		text_show(955);
		goto handled;
	}

	if (player_said_2(open, flask_full_of_acid)) {
		text_show(956);
		goto handled;
	}

	if (player_said_2(drink, flask_full_of_acid)) {
		text_show(957);
		goto handled;
	}

	if (player_said_2(pour, flask_full_of_acid)) {
		text_show(958);
		goto handled;
	}

	if (player_said_2(throw, flask_full_of_acid)) {
		text_show(959);
		goto handled;
	}

	if (player_said_2(tie, rope)) {
		text_show(960);
		goto handled;
	}

	if (player_said_2(vortex_stone, take_magic_from)) {
		if (player_said_1(signet_ring) ||
			player_said_1(bird_figurine) ||
			player_said_1(birdcall) ||
			player_said_1(shieldstone) ||
			player_said_1(key_crown) ||
			player_said_1(polystone) ||
			player_said_1(red_stone) ||
			player_said_1(blue_stone) ||
			player_said_1(yellow_stone) ||
			player_said_1(soul_egg) ||
			player_said_1(magic_belt) ||
			player_said_1(crystal_ball) ||
			player_said_1(shifter_ring)) {

			text_show(962);
			goto handled;
			/* then what good would it do you */
		}
	}

	if (player_said_2(vortex_stone, take_magic_from)) {
		if (player_said_1(signet_ring) ||
			player_said_1(doll) ||
			player_said_1(door_frame) ||
			player_said_1(new_bundle) ||
			player_said_1(tower_door) ||
			player_said_1(vortex_stone) ||
			player_said_1(spirit_bundle) ||
			player_said_1(freezer) ||
			player_said_1(music_box) ||
			player_said_1(amulet)) {

			text_show(962);
			goto handled;
			/* then what good would it do you */
		}
	}


	if (player_said_2(vortex_stone, take_magic_from)) {
		if ((player_said_1(cave) && room_id == 203) ||
			(player_said_1(torch) && player.second_object_source != STROKE_INTERFACE) ||
			(player_said_1(torch) && room_id == 607) ||
			(player_said_1(torch) && room_id == 609) ||
			player_said_1(teleport_door)) {

			text_show(962);
			/* then what good would it do you */

		} else if (player_said_1(eye) ||
			player_said_1(floating_disk) ||
			player_said_1(spirit_plane) ||
			player_said_1(waterfall) ||
			player_said_1(darkness_beast) ||
			player_said_1(ward) ||
			player_said_1(teleportal) ||
			player_said_1(tangle) ||
			player_said_1(Roc) ||
			player_said_1(Dragonsphere) ||
			player_said_1(spell_shield) ||
			player_said_1(glowing_floor) ||
			(player_said_1(pedestal) && room_id == 116) ||
			player_said_1(circle_of_spheres)) {

			text_show(963);
			/* too strong it fails */

		} else {
			text_show(961);
		}
		goto handled;
	}

	if (player_said_2(pet, dead_rat)) {
		object_examine(dead_rat, 964, 0);
		goto handled;
	}

	if (player_said_2(open, dead_rat)) {
		text_show(965);
		goto handled;
	}

	if (player_said_2(fold, map)) {
		text_show(966);
		goto handled;
	}

	if (player_said_2(gaze_into, crystal_ball)) {
		object_examine(crystal_ball, 967, 0);
		goto handled;
	}

	if (player_said_2(talk_to, crystal_ball)) {
		object_examine(crystal_ball, 971, 0);
		goto handled;
	}

	if (player_said_2(polish, black_sphere)) {
		object_examine(black_sphere, 972, 0);
		goto handled;
	}

	if (player_said_2(drink, soporific)) {
		text_show(973);
		goto handled;
	}

	if (player_said_1(shake)) {
		if (idd == partly_built_bundle) {
			text_show(976);

		} else if (idd == medicine_bundle ||
			idd == new_bundle) {
			if (game.difficulty == HARD_MODE) {
				text_show(974);

			} else {
				text_show(975);
			}
		}
		goto handled;
	}

	if (player_said_2(lick, ratsicle)) {
		text_show(979);
		goto handled;
	}

	if (player_said_2(chew, tentacle_parts)) {
		text_show(980);
		goto handled;
	}

	if (player_said_2(put, tentacle_parts)) {

		if (id == tentacle_parts) {
			text_show(23);

		} else {
			text_show(981);
		}
		goto handled;
	}

	if (player_said_2(unroll, teleport_door)) {
		text_show(982);
		goto handled;
	}

	if (player_said_1(admire)) {
		text_show(984);
		goto handled;
	}


	if (player_said_2(make_noise, birdcall)) {
		if (global[shak_status] == SHAK_MET) {
			sound_play(N_BlowBirdCall);
			text_show(50621);
		} else if (player_has_been_in_room(509)) {
			sound_play(N_BlowBirdCall);
			text_show(992);
		} else if (global[player_persona] == PLAYER_IS_KING) {
			sound_play(N_BlowBirdCall);
			text_show(991);
		} else {
			sound_play(N_BlowBirdCall);
			text_show(992);
		}
		goto handled;
	}

	if (player_said_2(look, new_bundle)) {
		object_examine(new_bundle, 890, 0);
		goto handled;
	}

	if (player_said_1(look) && player_has(object_named(player_main_noun))) {
		global_object_examine();
		goto handled;
	}

	if (player_said_2(invoke, amulet)) {
		if (global[amulet_status] == AMULET_NOT_CORRECT_TIME) {
			object_examine(amulet, 945, 0);
		}
		goto handled;
	}

	if (player_said_2(thrust, sword) ||
		player_said_2(carve_up, sword) ||
		player_said_2(attack, sword)) {

		if (player_said_1(bird_figurine) && player_has(bird_figurine)) {

			inter_move_object(bird_figurine, NOWHERE);
			inter_give_to_player(birdcall);
			object_examine(birdcall, 912, 0);

		} else {
			text_show(911);
		}
		goto handled;
	}

	if (player_said_1(shift_into_bear) || player_said_1(shift_into_seal) ||
		player_said_1(shift_into_snake)) {
		text_show(988);
		goto handled;
	}

	if (player_said_1(revert)) {
		text_show(989);
		goto handled;
	}

	if (player_said_2(invoke, signet_ring)) {
		if (room_id == 408 || room_id == 409 || room_id == 410 || room_id == 411) {
			text_show(702);
		} else if (room_id == 110) {
			text_show(996);
		} else {
			text_show(900);
			global[desert_counter] = 0;
			global[move_direction_510] = true;
			new_room = 110;
		}
		goto handled;
	}

	if (player_said_2(speak_words_on, parchment)) {
		text_show(985);
		goto handled;
	}

	if (player_said_2(invoke_power_of, crystal_ball)) {
		object_examine(crystal_ball, 969, 0);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void DragonsphereEngine::global_object_examine() {
	int id;

	id = object_named(player_main_noun);

	/* exceptions here */

	if (player_said_1(music_box)) {
		sound_play(N_MusicBoxOn);
	}

	if (player_said_1(doll)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			object_examine(pid_doll, 808, 0);

		} else { /* pid */
			if (game.difficulty == EASY_MODE) {
				if (global[pid_looked_at_doll]) {
					object_examine(pid_doll, 896, 0);
				} else {
					object_examine(pid_doll, 895, 0);
					global[pid_looked_at_doll] = true;
					global[heal_verbs_visible] = true;
					inter_move_object(pid_doll, NOWHERE);
					inter_give_to_player(pid_doll);
				}

			} else {
				if (global[pid_looked_at_doll]) {
					object_examine(pid_doll, 896, 0);
				} else {
					object_examine(pid_doll, 706, 0);
					global[pid_looked_at_doll] = true;
					global[heal_verbs_visible] = true;
					inter_move_object(pid_doll, NOWHERE);
					inter_give_to_player(pid_doll);
				}
			}
		}

	} else if (player_said_1(polystone) && global[object_imitated] == -1) {
		object_examine(polystone, 809, 0);

	} else if (player_said_1(polystone) && global[object_imitated] == 9) {
		object_examine(polystone, 809, 0);

	} else if (player_said_1(polystone) && global[object_imitated] >= 0) {
		object_examine(polystone, 894, 0);

	} else if (player_said_1(medicine_bundle)) {
		object_examine(medicine_bundle, 890, 0);

	} else {
		object_examine(id, 800 + id, 0);
	}

	if (player_said_1(music_box)) {
		sound_play(N_MusicBoxOff);
	}
}

void DragonsphereEngine::global_error_code() {
	int show_me = 0;
	int item;
	int random;

	random = imath_random(1, 1000);

	if (player_said_3(put, king, pedestal)) {
		if (global[king_status] != KING_CAPTIVE) {
			text_show(43);
			goto done;
		}
	}

	if (player_said_1(take)) {
		item = object_named(player_main_noun);
		if (player_has(item) && player.main_object_source != STROKE_INTERFACE) {
			show_me = 25;
			/* player already has it */

		} else {
			if (random <= 333) {
				show_me = 1;
			} else if (random <= 666) {
				show_me = 2;
			} else {
				show_me = 3;
			}
		}
		goto done;
	}

	if (player_said_1(push)) {
		if (random < 750) {
			show_me = 4;
		} else {
			show_me = 5;
		}
		goto done;
	}

	if (player_said_1(pull)) {
		if (random < 750) {
			show_me = 6;
		} else {
			show_me = 7;
		}
		goto done;
	}

	if (player_said_1(open)) {
		if (random <= 500) {
			show_me = 8;
		} else if (random <= 750) {
			show_me = 9;
		} else {
			show_me = 10;
		}
		goto done;
	}

	if (player_said_1(close)) {
		if (random <= 500) {
			show_me = 11;
		} else if (random <= 750) {
			show_me = 12;
		} else {
			show_me = 13;
		}
		goto done;
	}

	if (player_said_1(put)) {
		item = object_named(player_main_noun);
		if (player_has(item)) {
			show_me = 26;

		} else if (player.main_object_source == STROKE_INTERFACE &&
			player.second_object_source == STROKE_INTERFACE) {
			show_me = 28;

		} else if (random < 500) {
			show_me = 14;

		} else {
			show_me = 15;
		}
		goto done;
	}

	if (player_said_1(talk_to)) {
		if (random <= 500) {
			show_me = 16;
		} else {
			show_me = 17;
		}
		goto done;
	}

	if (player_said_1(give)) {
		item = object_named(player_main_noun);
		if (player_has(item)) {
			show_me = 27;

		} else if (player.main_object_source == STROKE_INTERFACE &&
			player.second_object_source == STROKE_INTERFACE) {
			show_me = 28;

		} else {
			show_me = 18;
		}
		goto done;
	}

	if (player_said_1(throw)) {
		item = object_named(player_main_noun);
		if (player_has(item)) {
			show_me = 19;
		} else {
			show_me = 28;
		}
		goto done;
	}

	if (player_said_1(look)) {
		item = object_named(player_main_noun);
		if (random <= 333) {
			show_me = 20;
		} else if (random <= 666) {
			show_me = 21;
		} else {
			show_me = 22;
		}
		goto done;
	}

	if (!player_said_1(walk_to) && !player_said_1(walk_across) &&
		!player_said_1(walk_down) && !player_said_1(walk_behind) &&
		!player_said_1(cross)) {
		if (random < 500) {
			show_me = 23;
		} else {
			show_me = 24;
		}
		goto done;
	}

done:
	if (show_me) text_show(show_me);
}

void DragonsphereEngine::global_room_init() {
	switch (global[player_persona]) {
	case PLAYER_IS_KING:
		pal_change_color(KERNEL_MESSAGE_COLOR_BASE, 63, 0, 0);
		pal_change_color(KERNEL_MESSAGE_COLOR_BASE + 1, 45, 0, 0);
		break;

	case PLAYER_IS_PID:
	default:
		pal_change_color(KERNEL_MESSAGE_COLOR_BASE, 63, 50, 42);
		pal_change_color(KERNEL_MESSAGE_COLOR_BASE + 1, 45, 30, 20);
		break;
	}
}

void DragonsphereEngine::global_sound_driver() {
	Common::strcpy_s(kernel.sound_driver, "/");
	env_catint(kernel.sound_driver, new_section, 1);
}

} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
