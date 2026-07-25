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
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_405_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[3] = kernel_load_series("*ROXCL_8", 0);

	if (previous_room == 401) {
		player.x = 23;
		player.y = 123;
		player.facing = FACING_EAST;
	} else if (previous_room == 406) {
		player.x = 300;
		player.y = 128;
		player.facing = FACING_WEST;
	} else if (previous_room == 408) {
		player.x = 154;
		player.y = 109;
		player.facing = FACING_SOUTH;
	} else if (previous_room == 413) {
		player.x = 284;
		player.y = 109;
		player.facing = FACING_SOUTH;
	} else if (previous_room != RETURNING_FROM_DIALOG) {
		player.x = 23;
		player.y = 123;
		player.facing = FACING_EAST;
	}

	if (global[kArmoryDoorOpen])
		g_sequence_ids[2] = _scene->_sequences.startCycle(g_sprite_ids[2], false, 1);
	else
		g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, 1);

	if (kernel.teleported_in) {
		global[kArmoryDoorOpen] = false;
		inter_give_to_player(OBJ_SECURITY_CARD);
	}

	kernel.quotes = quote_load(0x24F, 0);
	section_4_music();
}

static void room_405_daemon() {
	if (kernel.trigger == 80) {
		_scene->_sequences.addTimer(20, 81);
		player.clock = kernel.clock + player.frame_delay;
		player.walker_visible = true;
	}

	if (kernel.trigger == 81) {
		player.commands_allowed = true;
		text_show(40525);
	}

	if (kernel.trigger == 70) {
		player.clock = kernel.clock + player.frame_delay;
		player.walker_visible = true;
		g_sequence_ids[1] = _scene->_sequences.addReverseSpriteCycle(g_sprite_ids[1], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(g_sequence_ids[1], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		g_engine->_soundManager->command(19, 0);
	}

	if (kernel.trigger == 71) {
		g_sequence_ids[1] = _scene->_sequences.startCycle(g_sprite_ids[1], false, 1);
		global[kArmoryDoorOpen] = false;
		_scene->_sequences.remove(g_sequence_ids[2]);
		player.commands_allowed = true;
	}

	if (kernel.trigger == 75) {
		player.clock = kernel.clock + player.frame_delay;
		player.walker_visible = true;
		_scene->_sequences.remove(g_sequence_ids[1]);
		g_sequence_ids[1] = _scene->_sequences.addSpriteCycle(g_sprite_ids[1], false, 6, 1, 0, 0);
		global[kArmoryDoorOpen] = true;
		player.commands_allowed = true;
		g_sequence_ids[2] = _scene->_sequences.startCycle(g_sprite_ids[2],
			false, 1);
		g_engine->_soundManager->command(19, 0);
	}
}

static void room_405_pre_parser() {
	if (player_said_1(take))
		player.need_to_walk = false;

	if (player_said_2(walk_down, corridor_to_west))
		player.walk_off_edge_to_room = 401;

	if (player_said_2(walk_down, corridor_to_east))
		player.walk_off_edge_to_room = 406;

	if (player_said_2(close, wide_door) && global[kArmoryDoorOpen])
		player_walk(212, 113, FACING_NORTH);
}

static void room_405_parser() {
	if (player_said_2(walk_through, door))
		new_room = 413;
	else if (player_said_2(walk_through, wide_door) && global[kArmoryDoorOpen])
		new_room = 408;
	else if (player_said_2(walk_through, wide_door) && !global[kArmoryDoorOpen])
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 60, quote_string(kernel.quotes, 0x24F));
	else if (player_said_3(put, security_card, card_slot) && !global[kArmoryDoorOpen]) {
		player.commands_allowed = false;
		player.walker_visible = false;
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		g_sequence_ids[3] = _scene->_sequences.startPingPongCycle(g_sprite_ids[3], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[3], 1, 2);
		_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
		Common::Point msgPos = Common::Point(player.x, player.y + 1);
		_scene->_sequences.setPosition(g_sequence_ids[3], msgPos);
		_scene->_sequences.setScale(g_sequence_ids[3], 87);
	} else if ((player_said_3(put, security_card, card_slot) || player_said_2(close, wide_door)) && global[kArmoryDoorOpen]) {
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[3] = _scene->_sequences.startPingPongCycle(g_sprite_ids[3], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[3], 1, 2);
		_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		_scene->_sequences.setPosition(g_sequence_ids[3], Common::Point(player.x, player.y));
		_scene->_sequences.setScale(g_sequence_ids[3], 87);
	} else if (player_said_2(put, card_slot)) {
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[3] = _scene->_sequences.startPingPongCycle(g_sprite_ids[3], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(g_sequence_ids[3], 1, 2);
		_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
		_scene->_sequences.setPosition(g_sequence_ids[3], Common::Point(player.x, player.y));
		_scene->_sequences.setScale(g_sequence_ids[3], 87);
	} else if (player_said_2(look, cannon_balls))
		text_show(40510);
	else if (player_said_2(take, cannon_balls))
		text_show(40511);
	else if (player_said_2(look, water_fountain))
		text_show(40512);
	else if (player_said_2(look, backboard) || player_said_2(look, hoop))
		text_show(40513);
	else if (player_said_2(look, light))
		text_show(40514);
	else if (player_said_2(look, card_slot))
		text_show(40515);
	else if (player_said_2(look, corridor_to_east))
		text_show(40516);
	else if (player_said_2(look, corridor_to_west))
		text_show(40517);
	else if (player_said_2(look, monitor))
		text_show(40518);
	else if (player_said_2(look, corridor) || player.look_around)
		text_show(40519);
	else if (player_said_2(look, wide_door)) {
		if (global[kArmoryDoorOpen])
			text_show(40521);
		else
			text_show(40520);
	} else if (player_said_2(look, door))
		text_show(40522);
	else if (player_said_2(look, coach_lamp))
		text_show(40523);
	else if (player_said_2(look, support))
		text_show(40524);
	else
		return;

	player.command_ready = false;
}

void room_405_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_405_preload() {
	room_init_code_pointer = room_405_init;
	room_pre_parser_code_pointer = room_405_pre_parser;
	room_parser_code_pointer = room_405_parser;
	room_daemon_code_pointer = room_405_daemon;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
