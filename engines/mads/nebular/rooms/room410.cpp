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

#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section4.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_410_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('y', -1), 0);
	g_sprite_ids[2] = kernel_load_series("*ROXRC_7", 0);

	if (object_is_here(OBJ_CHARGE_CASES))
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
	else
		kernel_flip_hotspot(words_charge_cases, false);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 155;
		player.y = 150;
		player.facing = FACING_NORTH;
	}

	section_4_music();

	kernel_run_animation(kernel_full_name(410, 'r', -1, "", KERNEL_AA), 0);
	kernel_anim[0].repeat = true;
}

static void room_410_daemon() {
	if (kernel_anim[0].frame == 1) {
		if (g_engine->getRandomNumber(1, 30) == 1)
			kernel_reset_animation(0, 2);
		else
			kernel_reset_animation(0, 0);
	}

	if (kernel_anim[0].frame == 9) {
		if (g_engine->getRandomNumber(1, 30) == 1)
			kernel_reset_animation(0, 10);
		else
			kernel_reset_animation(0, 8);
	}

	if (kernel_anim[0].frame == 5) {
		if (g_engine->getRandomNumber(1, 30) == 1)
			kernel_reset_animation(0, 6);
		else
			kernel_reset_animation(0, 4);
	}

	if (kernel_anim[0].frame == 3) {
		if (g_engine->getRandomNumber(1, 2) == 1)
			kernel_reset_animation(0, 4);
		else // == 2
			kernel_reset_animation(0, 8);
	}
}

static void room_410_pre_parser() {
	if (player_said_1(take) && !player_said_1(charge_cases))
		player.need_to_walk = false;

	if (player_said_2(look, charge_cases) && object_is_here(OBJ_CHARGE_CASES))
		player.need_to_walk = true;

	if (player_said_2(open, sacks) || player_said_2(open, sack))
		player.need_to_walk = false;

	if (player_said_2(look, can))
		player.need_to_walk = true;
}

static void room_410_parser() {
	if (player_said_2(walk_into, corridor_to_south))
		new_room = 406;
	else if (player_said_2(take, charge_cases) && (object_is_here(OBJ_CHARGE_CASES) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			g_engine->_soundManager->command(57, 0);
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], false, 7, 0, 0, 2);
			kernel_seq_range(g_sequence_ids[2], 1, 3);
			kernel_seq_player(g_sequence_ids[2], false);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, 3, 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			kernel_seq_delete(g_sequence_ids[1]);
			kernel_flip_hotspot(words_charge_cases, false);
			inter_give_to_player(OBJ_CHARGE_CASES);
			object_examine(OBJ_CHARGE_CASES, 41032, 0);
			break;

		case 2:
			player.clock = player.frame_delay + kernel.clock;
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(look, barrel))
		text_show(41010);
	else if (player_said_2(take, barrel))
		text_show(41011);
	else if (player_said_2(open, barrel))
		text_show(41012);
	else if (player_said_2(look, rug))
		text_show(41013);
	else if (player_said_2(take, rug))
		text_show(41014);
	else if (player_said_2(look, carton) || player_said_2(open, carton)) {
		if (object_is_here(OBJ_CHARGE_CASES))
			text_show(41015);
		else
			text_show(41016);
	} else if (player_said_2(look, flour))
		text_show(41017);
	else if (player_said_2(take, flour))
		text_show(41018);
	else if (player_said_2(look, sacks))
		text_show(41019);
	else if (player_said_2(look, sack))
		text_show(41019);
	else if (player_said_2(open, sacks))
		text_show(41020);
	else if (player_said_2(open, sack))
		text_show(41020);
	else if (player_said_2(look, bucket_of_tar))
		text_show(41021);
	else if (player_said_2(take, bucket_of_tar))
		text_show(41022);
	else if (player_said_2(look, can))
		text_show(41023);
	else if (player_said_2(take, can))
		text_show(41024);
	else if (player_said_2(look, charge_cases) && object_is_here(OBJ_CHARGE_CASES))
		text_show(41025);
	else if (player_said_2(look, fence))
		text_show(41027);
	else if (player_said_2(look, shelves))
		text_show(41028);
	else if (player_said_2(look, rat))
		text_show(41029);
	else if (player_said_2(take, rat))
		text_show(41030);
	else if (player_said_2(throw, rat))
		text_show(41031);
	else if (player.look_around)
		text_show(41033);
	else
		return;

	player.command_ready = false;
}

void room_410_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_410_preload() {
	room_init_code_pointer = room_410_init;
	room_pre_parser_code_pointer = room_410_pre_parser;
	room_parser_code_pointer = room_410_parser;
	room_daemon_code_pointer = room_410_daemon;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
