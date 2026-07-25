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

struct Scratch {
	bool _hitStorageDoor;
};

static Scratch local;


static void room_406_init() {
	player.walker_visible = true;
	if (_scene->_priorSceneId == 405) {
		player.x = 15;
		player.y = 129;
		player.facing = FACING_EAST;
	} else if (_scene->_priorSceneId == 407) {
		player.x = 270;
		player.y = 127;
		player.facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 410) {
		player.x = 30;
		player.y = 108;
		player.facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId == 411) {
		player.x = 153;
		player.y = 108;
		player.facing = FACING_SOUTH;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		player.x = 15;
		player.y = 129;
		player.facing = FACING_EAST;
	}

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*ROXCL_8");
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(kernel_name('x', 1));

	if (_scene->_roomChanged) {
		_globals[kStorageDoorOpen] = false;
		inter_give_to_player(OBJ_SECURITY_CARD);
	}

	if (!_globals[kStorageDoorOpen])
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);

	if (_scene->_priorSceneId != 411)
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	else {
		player.commands_allowed = false;
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[3] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[3], false, 3, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
		g_engine->_soundManager->command(19, 0);
	}

	kernel.quotes = quote_load(0x24F, 0);
	local._hitStorageDoor = false;
	section_4_music();
}

static void room_406_daemon() {
	if (kernel.trigger == 90) {
		player.commands_allowed = true;
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
	}

	if (kernel.trigger == 80)
		_scene->_nextSceneId = 411;

	if (kernel.trigger == 100) {
		text_show(40622);
		local._hitStorageDoor = true;
	}

	if (kernel.trigger == 110) {
		_scene->_sequences.addTimer(20, 111);
		player.clock = _scene->_frameStartTime + player.frame_delay;
		player.walker_visible = true;
	}

	if (kernel.trigger == 111) {
		player.commands_allowed = true;
		text_show(40613);
	}

	if (kernel.trigger == 70) {
		player.clock = _scene->_frameStartTime + player.frame_delay;
		player.walker_visible = true;
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 4, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		g_engine->_soundManager->command(19, 0);
	}

	if (kernel.trigger == 71) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_globals[kStorageDoorOpen] = false;
		player.commands_allowed = true;
	}

	if (kernel.trigger == 75) {
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 4, 1, 0, 0);
		_globals[kStorageDoorOpen] = true;
		player.commands_allowed = true;
		player.clock = _scene->_frameStartTime + player.frame_delay;
		player.walker_visible = true;
		g_engine->_soundManager->command(19, 0);
	}
}

static void room_406_pre_parser() {
	if (player_said_2(walk_down, corridor_to_west))
		player.walk_off_edge_to_room = 405;

	if (player_said_2(walk_down, corridor_to_east))
		player.walk_off_edge_to_room = 407;

	if (player_said_1(take))
		player.need_to_walk = false;

	if (player_said_2(look, sign) || player_said_2(look, trash))
		player.need_to_walk = true;
}

static void room_406_parser() {
	if (player_said_2(walk_through, door) && (player.target_x > 100)) {
		player.commands_allowed = false;
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 3, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
		g_engine->_soundManager->command(19, 0);
	} else if (player_said_2(walk_through, door) && _globals[kStorageDoorOpen] && (player.target_x < 100))
		_scene->_nextSceneId = 410;
	else if (player_said_2(walk_through, door) && !_globals[kStorageDoorOpen] && (player.target_x < 100)) {
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 60, quote_string(kernel.quotes, 0x24F));
		if (!local._hitStorageDoor) {
			kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addTimer(80, 100);
		}
	} else if (player_said_3(put, security_card, card_slot) && !_globals[kStorageDoorOpen]) {
		player.commands_allowed = false;
		player.walker_visible = false;
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
		Common::Point msgPos = Common::Point(player.x, player.y + 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], msgPos);
		_scene->_sequences.setScale(_globals._sequenceIndexes[2], 87);
	} else if (player_said_3(put, security_card, card_slot) && _globals[kStorageDoorOpen]) {
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		player.commands_allowed = false;
		player.walker_visible = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		Common::Point msgPos = Common::Point(player.x, player.y + 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], msgPos);
		_scene->_sequences.setScale(_globals._sequenceIndexes[2], 87);
	} else if (player_said_2(put, card_slot)) {
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		player.commands_allowed = false;
		player.walker_visible = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 7, 2, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 110);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[2], Common::Point(player.x, player.y));
		_scene->_sequences.setScale(_globals._sequenceIndexes[2], 87);
	} else if (player_said_2(look, trash))
		text_show(40610);
	else if (player_said_2(take, trash))
		text_show(40611);
	else if (player_said_2(look, card_slot))
		text_show(40612);
	else if (player_said_2(look, fire_extinguisher))
		text_show(40614);
	else if (player_said_2(take, fire_extinguisher))
		text_show(40615);
	else if (player_said_2(look, corridor_to_east))
		text_show(40616);
	else if (player_said_2(look, corridor_to_west))
		text_show(40617);
	else if (player_said_2(look, corridor) || player.look_around)
		text_show(40618);
	else if (player_said_2(look, wall))
		text_show(40619);
	else if (player_said_2(look, door)) {
		if (_globals[kStorageDoorOpen])
			text_show(40621);
		else
			text_show(40620);
	} else if (player_said_2(look, monitor))
		text_show(40623);
	else if (player_said_2(look, signpost))
		text_show(40624);
	else if (player_said_2(take, signpost))
		text_show(40625);
	else if (player_said_2(look, boulder))
		text_show(40626);
	else if (player_said_2(take, boulder))
		text_show(40627);
	else if (player_said_2(look, sign))
		text_show(40628);
	else if (player_said_2(take, sign))
		text_show(40629);
	else
		return;

	player.command_ready = false;
}

void room_406_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._hitStorageDoor);
}

void room_406_preload() {
	room_init_code_pointer = room_406_init;
	room_pre_parser_code_pointer = room_406_pre_parser;
	room_parser_code_pointer = room_406_parser;
	room_daemon_code_pointer = room_406_daemon;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
