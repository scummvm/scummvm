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

#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/core/himem.h"
#include "mads/core/matte.h"
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section1.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	byte _rexThrowingObject;
	byte _hoovicDifficultFl;
	byte _beforeEatingRex;
	byte _eatingRex;
	byte _hungryFl;
	byte _eatingFirstFish;

	int16 _throwingObjectId;
	int16 _hoovicTrigger;
};

static Scratch local;


static void room_109_init() {
	global[kFishIn105] = true;

	g_sprite_ids[0] = kernel_load_series("*RXSWRC_6", 0);
	g_sprite_ids[1] = kernel_load_series(kernel_name('O', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('O', 2), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('O', 0), 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('H', 4), 0);

	local._rexThrowingObject = false;
	local._throwingObjectId = 0;
	local._beforeEatingRex = false;
	local._eatingRex = false;
	local._hungryFl = false;

	if (previous_room == 110) {
		player.x = 248;
		player.y = 38;
		global[kHoovicSated] = 2;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 20;
		player.y = 68;
		player.facing = FACING_EAST;
	}

	if (!global[kHoovicAlive]) {
		g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 6, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[6], 4);
		kernel_seq_range(g_sequence_ids[6], -2, -2);

		int idx = kernel_add_dynamic(words_dead_purple_monster, words_swim_to, 0, -1, 256, 57, 11, 30);
		kernel_dynamic_walk(idx, 241, 91, FACING_NORTHEAST);
		idx = kernel_add_dynamic(words_dead_purple_monster, words_swim_to, 0, -1, 242, 79, 23, 11);
		kernel_dynamic_walk(idx, 241, 91, FACING_NORTHEAST);
		idx = kernel_add_dynamic(words_monster_sludge, words_swim_to, 0, -1, 231, 88, 22, 6);
		kernel_dynamic_walk(idx, 241, 91, FACING_NORTHEAST);
	}

	if (!global[kHoovicAlive] || global[kHoovicSated])
		kernel_load_variant(1);

	if (object_is_here(OBJ_BURGER)) {
		g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 6, 0, 0, 0);
		kernel_seq_range(g_sequence_ids[3], -2, -2);
		int idx = kernel_add_dynamic(words_burger, words_swim_to, 0, g_sequence_ids[3], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, -3, 0, FACING_NORTHEAST);
	} else if (kernel.teleported_in)
		inter_give_to_player(OBJ_BURGER);

	if (kernel.teleported_in) {
		inter_give_to_player(OBJ_DEAD_FISH);
		inter_give_to_player(OBJ_STUFFED_FISH);
	}

	pal_change_color(252, 50, 50, 63);
	pal_change_color(253, 30, 30, 50);

	kernel.quotes = quote_load(83, 82, 84, 85, 86, 87, 88, 0);
	local._eatingFirstFish = (!player.been_here_before) && (previous_room < 110);

	if (local._eatingFirstFish) {
		g_sprite_ids[10] = kernel_load_series(kernel_full_name(105, 'F', 1, "", KERNEL_SS), 0);
		g_sprite_ids[9] = kernel_load_series(kernel_name('H', 1), 0);

		g_sequence_ids[10] = kernel_seq_pingpong(g_sprite_ids[10], true, 4, 0, 0, 0);
		kernel_seq_depth(g_sequence_ids[10], 5);
		kernel_seq_loc(g_sequence_ids[10], 126, 39);
		kernel_seq_motion(g_sequence_ids[10], 0, 200, 0);
		kernel_seq_scale(g_sequence_ids[10], 80);
		player.commands_allowed = false;
	}

	section_1_music();
}

static void room_109_daemon() {
	if (local._beforeEatingRex) {
		if (!local._eatingRex) {
			if (player.x > 205) {
				g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 6, 0, 0, 1);
				kernel_seq_depth(g_sequence_ids[4], 4);
				kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_SPRITE, 6, 70);
				kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 71);

				local._eatingRex = true;
				g_engine->_soundManager->command(34, 0);
			}
		} else {
			switch (kernel.trigger) {
			case 70:
				player.walker_visible = false;
				break;

			case 71:
				kernel.force_restart = true;
				break;

			default:
				break;
			}
		}
	}

	if (local._hungryFl && (Common::Point(player.x, player.y) == Common::Point(160, 32)) && (player.facing == FACING_EAST)) {
		player_walk(226, 24, FACING_EAST);
		player.commands_allowed = false;
		local._hungryFl = false;
		local._beforeEatingRex = true;
		matte_deallocate_series(g_sprite_ids[6], true);
		g_sprite_ids[4] = kernel_load_series(kernel_name('H', 0), 0);
		kernel_new_palette();
	}

	if (player.walking && (player.next_special_code > 0) && global[kHoovicAlive] && !global[kHoovicSated] && !local._hungryFl && !local._beforeEatingRex) {
		player_cancel_command();
		player_start_walking(160, 32, FACING_EAST);
		player.next_special_code = 0;
		local._hungryFl = true;
	}

	if (local._eatingFirstFish && (sequence_list[g_sequence_ids[10]].x >= 178)) {
		g_sequence_ids[9] = kernel_seq_forward(g_sprite_ids[9], false, 4, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[9], 4);
		kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_SPRITE, 29, 72);
		kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_EXPIRE, 29, 73);
		kernel_seq_timeout(g_sequence_ids[9], g_sequence_ids[10]);
		local._eatingFirstFish = false;
		player.commands_allowed = true;
		g_engine->_soundManager->command(34, 0);
	}

	if (kernel.trigger == 72)
		kernel_seq_delete(g_sequence_ids[10]);

	if (kernel.trigger == 73) {
		kernel_seq_delete(g_sequence_ids[9]);
		matte_deallocate_series(g_sprite_ids[9], true);
		matte_deallocate_series(g_sprite_ids[10], true);

		image_marker = 0;
		matte_refresh_work();

		int randVal = g_engine->getRandomNumber(85, 88);
		int idx = kernel_message_add(quote_string(kernel.quotes, randVal), 0, 0, 0x1110, 120, 0, 34);
		kernel_message_teletype(idx, 4, true);
		kernel_message[idx].update_time = kernel.clock + 4;
	}
}

static void room_109_pre_parser() {
	if (player_said_2(swim_under, overhang_to_west))
		player.walk_off_edge_to_room = 108;

	if ((player_said_1(throw) || player_said_1(give) || player_said_1(put))
		&& (player_said_1(small_hole) || player_said_1(tunnel))
		&& (player_said_1(dead_fish) || player_said_1(stuffed_fish) || player_said_1(burger))) {
		int idx = object_named(player2.words[1]);
		if ((idx >= 0) && player_has(idx)) {
			player.prepare_walk_x = 106;
			player.prepare_walk_y = 38;
			player.prepare_walk_facing = FACING_EAST;
			player.need_to_walk = true;
			player.ready_to_walk = true;
		}
	}

	if ((player_said_2(swim_into, tunnel) || player_said_2(swim_to, small_hole))
		&& (!global[kHoovicAlive] || global[kHoovicSated]) && (player_said_1(tunnel)))
		player.walk_off_edge_to_room = 110;

	local._hungryFl = false;
}

static void room_109_parser() {
	if (player.look_around) {
		text_show(10912);
		player.command_ready = false;
		return;
	}

	if ((player_said_1(throw) || player_said_1(give)) && (player_said_1(small_hole) || player_said_1(tunnel))) {
		if (player_said_1(dead_fish) || player_said_1(stuffed_fish) || player_said_1(burger)) {
			local._throwingObjectId = object_named(player2.words[1]);
			if (local._throwingObjectId >= 0) {
				if ((player_has(local._throwingObjectId) && global[kHoovicAlive]) || local._rexThrowingObject) {
					switch (kernel.trigger) {
					case 0:
						local._rexThrowingObject = true;
						local._hoovicDifficultFl = false;
						inter_move_object(local._throwingObjectId, NOWHERE);
						g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0], false, 4, 0, 0, 1);
						kernel_seq_player(g_sequence_ids[0], false);
						kernel_seq_trigger(g_sequence_ids[0], KERNEL_TRIGGER_EXPIRE, 0, 1);
						player.walker_visible = false;
						player.commands_allowed = false;

						switch (local._throwingObjectId) {
						case OBJ_DEAD_FISH:
						case OBJ_STUFFED_FISH:
							g_sprite_ids[8] = kernel_load_series(kernel_name('H', 1), 0);
							break;

						case OBJ_BURGER:
							local._hoovicDifficultFl = (game.difficulty == DIFFICULTY_HARD);
							g_sprite_ids[8] = kernel_load_series(kernel_name('H', (local._hoovicDifficultFl ? 3 : 1)), 0);
							break;

						default:
							break;
						}

						kernel_new_palette();
						break;

					case 1:
						player.walker_visible = true;
						local._hoovicTrigger = 4;
						switch (local._throwingObjectId) {
						case OBJ_BURGER:
							g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, (local._hoovicDifficultFl ? 4 : 6), 0, 0, 1);
							kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_SPRITE, 2, 2);
							if (local._hoovicDifficultFl) {
								kernel_seq_range(g_sequence_ids[3], 1, 30);
								kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 5);
							} else {
								kernel_seq_range(g_sequence_ids[3], 1, 4);
								kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 8);
								local._hoovicTrigger = 3;
							}
							break;
						case OBJ_DEAD_FISH:
							g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 4, 0, 0, 1);
							kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_SPRITE, 2, 2);
							break;
						case OBJ_STUFFED_FISH:
							g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 4, 0, 0, 1);
							kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, 2, 2);
							local._hoovicTrigger = 3;
							break;
						default:
							break;
						}
						break;

					case 2:
						if (local._hoovicDifficultFl)
							g_sequence_ids[8] = kernel_seq_pingpong(g_sprite_ids[8], false, 4, 0, 0, 2);
						else
							g_sequence_ids[8] = kernel_seq_forward(g_sprite_ids[8], false, 4, 0, 0, 1);

						kernel_seq_depth(g_sequence_ids[8], 4);
						kernel_seq_trigger(g_sequence_ids[8], KERNEL_TRIGGER_EXPIRE, 0, local._hoovicTrigger);
						g_engine->_soundManager->command(34, 0);
						break;

					case 3:
						kernel_run_animation(kernel_full_name(109, 'H', 2, "", KERNEL_AA), 4);
						g_engine->_soundManager->command(35, 0);
						global[kHoovicAlive] = false;
						break;

					case 4:
						if (!global[kHoovicAlive]) {
							g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 6, 0, 0, 1);
							kernel_seq_depth(g_sequence_ids[6], 4);
							kernel_seq_range(g_sequence_ids[6], -2, -2);
							int idx = kernel_add_dynamic(words_dead_purple_monster, words_swim_to, 0, -1, 256, 57, 12, 31);
							kernel_dynamic_walk(idx, 241, 91, FACING_NORTHEAST);
							idx = kernel_add_dynamic(words_dead_purple_monster, words_swim_to, 0, -1, 242, 79, 24, 12);
							kernel_dynamic_walk(idx, 241, 91, FACING_NORTHEAST);
							idx = kernel_add_dynamic(words_monster_sludge, words_swim_to, 0, -1, 231, 88, 23, 7);
							kernel_dynamic_walk(idx, 241, 91, FACING_NORTHEAST);
							kernel_load_variant(1);
						} else {
							if (local._throwingObjectId == OBJ_DEAD_FISH) {
								++global[kHoovicFishEaten];
								int threshold;
								switch (game.difficulty) {
								case DIFFICULTY_HARD:
									threshold = 1;
									break;
								case DIFFICULTY_MEDIUM:
									threshold = 3;
									break;
								default:
									threshold = 50;
									break;
								}

								if (global[kHoovicFishEaten] >= threshold) {
									int randVal = g_engine->getRandomNumber(83, 84);
									kernel_message_add(quote_string(kernel.quotes, randVal), 230, 24, 0xFDFC, 120, 0, 0);
									global[kHoovicFishEaten] = 0;
									global[kHoovicSated] = 1;
									kernel_load_variant(1);
								}
							}
						}
						kernel_abort_animation(0);
						kernel_seq_delete(g_sequence_ids[8]);
						matte_deallocate_series(g_sprite_ids[8], true);
						image_marker = 0;
						matte_refresh_work();
						kernel_seq_full_update();
						if (player.walker_visible) {
							player.sprite_changed = true;
							player_set_image();
						}

						player.commands_allowed = true;
						local._rexThrowingObject = false;
						break;

					case 5:
					{
						inter_move_object(OBJ_BURGER, room_id);
						g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 6, 0, 0, 0);
						kernel_seq_range(g_sequence_ids[3], 30, 30);
						int idx = kernel_add_dynamic(words_burger, words_swim_to, 0, g_sequence_ids[3], 0, 0, 0, 0);
						kernel_dynamic_walk(idx, -3, 0, FACING_NORTHEAST);
						kernel_timing_trigger(65, 6);
					}
					break;

					case 6:
					{
						kernel_seq_delete(g_sequence_ids[3]);
						g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 6, 0, 0, 1);
						kernel_seq_range(g_sequence_ids[3], 31, 46);
						int idx = kernel_add_dynamic(words_burger, words_swim_to, 0, g_sequence_ids[3], 0, 0, 0, 0);
						kernel_dynamic_walk(idx, -3, 0, FACING_NORTHEAST);
						kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 7);
					}
					break;

					case 7:
					{
						kernel_seq_delete(g_sequence_ids[3]);
						g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -2);
						int idx = kernel_add_dynamic(words_burger, words_swim_to, 0, g_sequence_ids[3], 0, 0, 0, 0);
						kernel_dynamic_walk(idx, -3, 0, FACING_NORTHEAST);
						text_show(10915);
					}
					break;

					case 8:
						g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 7, 0, 0, 1);
						kernel_seq_range(g_sequence_ids[3], 5, 16);
						break;

					default:
						break;
					}
					player.command_ready = false;
					return;
				} else if (player_has(local._throwingObjectId)) {
					// Nothing.
				}
			}
		}
	}

	if (player_said_2(take, burger) && object_is_here(OBJ_BURGER)) {
		kernel_seq_delete(g_sequence_ids[3]);
		inter_give_to_player(OBJ_BURGER);
	} else if (player_said_2(look, ocean_floor))
		text_show(10901);
	else if (player_said_2(look, coral))
		text_show(10902);
	else if ((player_said_1(take) || player_said_1(pull)) && player_said_1(coral))
		text_show(10903);
	else if (player_said_2(look, rocks))
		text_show(10904);
	else if (player_said_2(take, rocks))
		text_show(10905);
	else if (player_said_2(look, cave_wall))
		text_show(10906);
	else if (player_said_2(look, tunnel)) {
		if (global[kHoovicAlive])
			text_show(10907);
		else
			text_show(10913);
	} else if (player_said_2(look, small_hole))
		text_show(10908);
	else if (player_said_2(look, overhang_to_west))
		text_show(10911);
	else if (player_said_2(put, small_hole))
		text_show(10910);
	else if (player_said_2(look, dead_purple_monster))
		text_show(10914);
	else
		return;

	player.command_ready = false;
}

void room_109_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._rexThrowingObject);
	s.syncAsByte(local._hoovicDifficultFl);
	s.syncAsByte(local._beforeEatingRex);
	s.syncAsByte(local._eatingRex);
	s.syncAsByte(local._hungryFl);
	s.syncAsByte(local._eatingFirstFish);
	s.syncAsSint32LE(local._throwingObjectId);
	s.syncAsSint32LE(local._hoovicTrigger);
}

void room_109_preload() {
	room_init_code_pointer = room_109_init;
	room_pre_parser_code_pointer = room_109_pre_parser;
	room_parser_code_pointer = room_109_parser;
	room_daemon_code_pointer = room_109_daemon;

	for (int count = 0; count < 4; ++count) {
		himem_preload_series(kernel_full_name(109, 'H', count, nullptr, 0), 3);
	}

	vocab_make_active(words_dead_purple_monster);
	vocab_make_active(words_monster_sludge);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
