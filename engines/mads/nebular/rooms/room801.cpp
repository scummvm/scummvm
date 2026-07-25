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
#include "mads/nebular/rooms/section8.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _walkThroughDoor;
};

static Scratch local;


static void room_801_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(kernel_name('x', 1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(kernel_name('x', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(kernel_name('x', 2));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(kernel_name('x', 3));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites(kernel_name('a', -1));

	if (_scene->_priorSceneId != 802) {
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);
	}

	if ((_globals[kCameFromCut]) && (_globals[kCutX] != 0)) {
		player.x = _globals[kCutX];
		player.y = _globals[kCutY];
		player.facing = _globals[kCutFacing];
		_globals[kCutX] = 0;
		_globals[kCameFromCut] = false;
		_globals[kReturnFromCut] = false;
		_globals[kBeamIsUp] = false;
		_globals[kForceBeamDown] = false;
		_globals[kDontRepeat] = false;
	} else if (_scene->_priorSceneId == 808) {
		player.x = 148;
		player.y = 110;
		player.facing = FACING_NORTH;
	} else if (_scene->_priorSceneId == 802) {
		player.x = 307;
		player.y = 111;
		player_walk(270, 118, FACING_WEST);
		player.walker_visible = true;
	} else if ((_scene->_priorSceneId != RETURNING_FROM_DIALOG) && !_globals[kTeleporterCommand]) {
		player.x = 8;
		player.y = 117;
		player_walk(41, 115, FACING_EAST);
		player.walker_visible = true;
	}

	_globals[kBetweenRooms] = false;

	if (_globals[kTeleporterCommand]) {
		player.commands_allowed = false;
		switch (_globals[kTeleporterCommand]) {
		case 1:
			player.x = 8;
			player.y = 117;
			_globals[kTeleporterUnderstood] = true;
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 13);
			kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 75);
			g_engine->_soundManager->command(30, 0);
			break;

		case 2:
			player.x = 8;
			player.y = 117;
			_globals[kTeleporterUnderstood] = true;
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[1], 1, 13);
			kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			g_engine->_soundManager->command(30, 0);
			break;

		case 3:
		case 4:
			player.x = 8;
			player.y = 117;
			player_walk(41, 115, FACING_EAST);
			player.walker_visible = true;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
		_globals[kTeleporterCommand] = 0;
	}

	local._walkThroughDoor = false;
	if (_scene->_priorSceneId == 802) {
		player.commands_allowed = false;
		local._walkThroughDoor = true;
	}

	_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 11, 0, 0, 0);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], -1, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 14);

	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 9, 0, 0, 0);
	_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 14);

	section_8_music();
}

static void room_801_daemon() {
	if (kernel.trigger == 75) {
		if (_globals[kSexOfRex] == REX_FEMALE) {
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 8, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 8);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 140);
		} else {
			player.commands_allowed = true;
			player.walker_visible = true;
			player.x = 8;
			player.y = 117;
			player_walk(41, 115, FACING_EAST);
		}
	}

	if (kernel.trigger == 140) {
		g_engine->_soundManager->command(27, 0);
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 8);
		_scene->_sequences.addTimer(100, 141);
	}

	if (kernel.trigger == 141) {
		_scene->_reloadSceneFlag = true;
		_scene->_nextSceneId = _scene->_priorSceneId;
		_globals[kTeleporterCommand] = 0;
	}

	if (kernel.trigger == 80) {
		_globals[kTeleporterCommand] = 1;
		_scene->_nextSceneId = _globals[kTeleporterDestination];
		_scene->_reloadSceneFlag = true;
	}

	if (local._walkThroughDoor && (Common::Point(player.x, player.y) == Common::Point(270, 118))) {
		player.commands_allowed = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		local._walkThroughDoor = false;
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 120);
	}

	if (kernel.trigger == 120) {
		g_engine->_soundManager->command(12, 0);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		player.commands_allowed = true;
	}

	if (kernel.trigger == 90) {
		player_walk(307, 111, FACING_EAST);
		_scene->_sequences.addTimer(80, 130);
	}

	if (kernel.trigger == 130) {
		g_engine->_soundManager->command(12, 0);
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 110);
	}

	if (kernel.trigger == 110) {
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 10);
		_scene->_nextSceneId = 802;
	}
}

static void room_801_pre_parser() {
	if (player_said_2(look, control_panel)) {
		player_walk(148, 110, FACING_NORTH);
		player.need_to_walk = true;
		player.ready_to_walk = true;
	}

	if (player_said_2(walk_inside, teleporter) && _globals[kBeamIsUp]) {
		_globals[kCutX] = player.x;
		_globals[kCutY] = player.y;
		_globals[kCutFacing] = player.facing;
		_globals[kForceBeamDown] = true;
		_globals[kDontRepeat] = true;
		_scene->_nextSceneId = 803;
	}
}

static void room_801_parser() {
	if (player_said_2(look, control_panel))
		_scene->_nextSceneId = 808;
	else if (player_said_2(walk_inside, teleporter)) {
		player.commands_allowed = false;
		player.walker_visible = false;
		_scene->_nextSceneId = 807;
	} else if (player_said_2(walk_through, door) && (Common::Point(player.x, player.y) == Common::Point(270, 118))) {
		player.commands_allowed = false;
		player.facing = FACING_EAST;
		player_select_series();
		_globals[kBetweenRooms] = true;
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 4, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 90);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 13);
		g_engine->_soundManager->command(11, 0);
	} else if (player_said_2(look, ceiling))
		text_show(80110);
	else if (player_said_2(look, monitor))
		text_show(80111);
	else if (player_said_2(look, teleporter))
		text_show(80112);
	else if (player_said_2(look, equipment) || _action._lookFlag)
		text_show(80113);
	else if (player_said_2(look, speaker))
		text_show(80114);
	else if (player_said_2(look, eye_chart))
		text_show(80115);
	else if (player_said_2(look, wall))
		text_show(80116);
	else if (player_said_2(look, door))
		text_show(80117);
	else
		return;

	_action._inProgress = false;
}

void room_801_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._walkThroughDoor);
}

void room_801_preload() {
	room_init_code_pointer = room_801_init;
	room_daemon_code_pointer = room_801_daemon;
	room_pre_parser_code_pointer = room_801_pre_parser;
	room_parser_code_pointer = room_801_parser;

	section_8_walker();
	section_8_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
