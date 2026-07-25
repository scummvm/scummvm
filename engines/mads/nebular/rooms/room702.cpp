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
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_702_init() {
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites("*RXMBD_8");

	if (_scene->_priorSceneId == 701) {
		player.x = 13;
		player.y = 145;
		player.facing = FACING_EAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG && _scene->_priorSceneId != 620) {
		player.x = 289;
		player.y = 138;
		player_walk(262, 148, FACING_WEST);
		player.facing = FACING_WEST;
		player.walker_visible = true;
	}

	if (_globals[kTeleporterCommand]) {
		switch (_globals[kTeleporterCommand]) {
		case TELEPORTER_BEAM_OUT:
		case TELEPORTER_WRONG:
		case TELEPORTER_STEP_OUT:
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		default:
			break;
		}

		_globals[kTeleporterCommand] = TELEPORTER_NONE;
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
		_scene->_nextSceneId = 711;
	} else if (player_said_2(take, bones) && (_action._mainObjectSource == CAT_HOTSPOT) && (!player_has(OBJ_BONES) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			g_engine->_soundManager->command(0xF, 0);
			if (player_has(OBJ_BONE))
				inter_move_object(OBJ_BONE, 1);
			inter_give_to_player(OBJ_BONES);
			object_examine(OBJ_BONES, 70218, 0);
			break;
		case 2:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[12]);
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		default:
			break;
		}
	} else if (_action._lookFlag)
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
	else if (player_said_2(look, bones) && (_action._mainObjectSource == CAT_HOTSPOT))
		text_show(70217);
	else if (player_said_2(take, bones) && (_action._mainObjectSource == CAT_HOTSPOT)) {
		if (player_has(OBJ_BONES))
			text_show(70219);
	} else if (player_said_2(look, submerged_city))
		text_show(70220);
	else
		return;

	_action._inProgress = false;
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
