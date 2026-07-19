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
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section7.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_702_init() {
	_globals._spriteIndexes[12] = _scene->_sprites.addSprites("*RXMBD_8");

	if (_scene->_priorSceneId == 701) {
		_game._player._playerPos = Common::Point(13, 145);
		_game._player._facing = FACING_EAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG && _scene->_priorSceneId != 620) {
		_game._player._playerPos = Common::Point(289, 138);
		_game._player.walk(Common::Point(262, 148), FACING_WEST);
		_game._player._facing = FACING_WEST;
		_game._player._visible = true;
	}

	if (_game._globals[kTeleporterCommand]) {
		switch (_game._globals[kTeleporterCommand]) {
		case TELEPORTER_BEAM_OUT:
		case TELEPORTER_WRONG:
		case TELEPORTER_STEP_OUT:
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		default:
			break;
		}

		_game._globals[kTeleporterCommand] = TELEPORTER_NONE;
	}

	section_7_music();
}

static void room_702_pre_parser() {
	if (_action.isAction(words_walkto, words_west_end_of_platform))
		_game._player._walkOffScreenSceneId = 701;
}

static void room_702_parser() {
	if (_action.isAction(words_walk_along, words_platform))
		; // Only set the action as finished
	else if (_action.isAction(words_step_into, words_teleporter)) {
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_scene->_nextSceneId = 711;
	} else if (_action.isAction(words_take, words_bones) && (_action._mainObjectSource == CAT_HOTSPOT) && (!_game._objects.isInInventory(OBJ_BONES) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[12] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[12], false, 5, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[12]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[12], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;
		case 1:
			_vm->_sound->command(0xF);
			if (_game._objects.isInInventory(OBJ_BONE))
				_game._objects.setRoom(OBJ_BONE, 1);
			_game._objects.addToInventory(OBJ_BONES);
			_vm->_dialogs->show(OBJ_BONES, 70218);
			break;
		case 2:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[12]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;
		default:
			break;
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(70210);
	else if (_action.isAction(words_look, words_platform))
		_vm->_dialogs->show(70211);
	else if (_action.isAction(words_look, words_cement_block))
		_vm->_dialogs->show(70212);
	else if (_action.isAction(words_look, words_rock))
		_vm->_dialogs->show(70213);
	else if (_action.isAction(words_take, words_rock))
		_vm->_dialogs->show(70214);
	else if (_action.isAction(words_look, words_west_end_of_platform))
		_vm->_dialogs->show(70215);
	else if (_action.isAction(words_look, words_teleporter))
		_vm->_dialogs->show(70216);
	else if (_action.isAction(words_look, words_bones) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(70217);
	else if (_action.isAction(words_take, words_bones) && (_action._mainObjectSource == CAT_HOTSPOT)) {
		if (_game._objects.isInInventory(OBJ_BONES))
			_vm->_dialogs->show(70219);
	} else if (_action.isAction(words_look, words_submerged_city))
		_vm->_dialogs->show(70220);
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
} // namespace MADSV2
} // namespace MADS
