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
#include "mads/nebular/rooms/section7.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_702_init() {
	g_sprite_ids[12] = kernel_load_series("*RXMBD_8", 0);

	if (previous_room == 701) {
		player.x = 13;
		player.y = 145;
		player.facing = FACING_EAST;
	} else if (previous_room != KERNEL_RESTORING_GAME && previous_room != 620) {
		player.x = 289;
		player.y = 138;
		player_walk(262, 148, FACING_WEST);
		player.facing = FACING_WEST;
		player.walker_visible = true;
	}

	if (global[kTeleporterCommand]) {
		switch (global[kTeleporterCommand]) {
		case TELEPORTER_BEAM_OUT:
		case TELEPORTER_WRONG:
		case TELEPORTER_STEP_OUT:
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		default:
			break;
		}

		global[kTeleporterCommand] = TELEPORTER_NONE;
	}

	section_7_music();
}

static void room_702_pre_parser() {
	if (player_said_2(walkto, west_end_of_platform))
		player.walk_off_edge_to_room = 701;
}

static void room_702_parser() {
	if (player_said_2(walk_along, platform))
		; // Only set the action as finished
	else if (player_said_2(step_into, teleporter)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		new_room = 711;
	} else if (player_said_2(take, bones) && (player.main_object_source == STROKE_INTERFACE) && (!player_has(OBJ_BONES) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[12] = kernel_seq_pingpong(g_sprite_ids[12], false, 5, 0, 0, 2);
			kernel_seq_player(g_sequence_ids[12], false);
			kernel_seq_trigger(g_sequence_ids[12], KERNEL_TRIGGER_SPRITE, 4, 1);
			kernel_seq_trigger(g_sequence_ids[12], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			g_engine->_soundManager->command(0xF, 0);
			if (player_has(OBJ_BONE))
				inter_move_object(OBJ_BONE, 1);
			inter_give_to_player(OBJ_BONES);
			object_examine(OBJ_BONES, 70218, 0);
			break;
		case 2:
			kernel_seq_timeout(g_sequence_ids[12], -1);
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		default:
			break;
		}
	} else if (player.look_around)
		text_show(70210);
	else if (player_said_2(look, platform))
		text_show(70211);
	else if (player_said_2(look, cement_block))
		text_show(70212);
	else if (player_said_2(look, rock))
		text_show(70213);
	else if (player_said_2(take, rock))
		text_show(70214);
	else if (player_said_2(look, west_end_of_platform))
		text_show(70215);
	else if (player_said_2(look, teleporter))
		text_show(70216);
	else if (player_said_2(look, bones) && (player.main_object_source == STROKE_INTERFACE))
		text_show(70217);
	else if (player_said_2(take, bones) && (player.main_object_source == STROKE_INTERFACE)) {
		if (player_has(OBJ_BONES))
			text_show(70219);
	} else if (player_said_2(look, submerged_city))
		text_show(70220);
	else
		return;

	player.command_ready = false;
}

void room_702_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_702_preload() {
	room_init_code_pointer = room_702_init;
	room_pre_parser_code_pointer = room_702_pre_parser;
	room_parser_code_pointer = room_702_parser;

	section_7_walker();
	section_7_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
