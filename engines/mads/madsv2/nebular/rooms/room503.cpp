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

#include "math/utils.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/nebular.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/nebular/mads/words.h"
#include "mads/madsv2/nebular/rooms/section5.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _detonatorHotspotId;
};

static Scratch local;


static void room_503_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', -1));

	if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMBD_2");
	else
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*ROXBD_2");

	if (_game._objects[OBJ_DETONATORS]._roomNumber == _scene->_currentSceneId) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		local._detonatorHotspotId = _scene->_dynamicHotspots.add(words_detonators, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(local._detonatorHotspotId, Common::Point(254, 135), FACING_SOUTH);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(191, 152);
		_game._player._facing = FACING_NORTHWEST;
	}

	section_5_music();
}

static void room_503_parser() {
	if (player_said_2(walk, outside))
		_scene->_nextSceneId = 501;
	else if (player_said_2(take, detonators)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_DETONATORS)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				if (_globals[kSexOfRex] == REX_MALE) {
					_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 8, 1, 0, 0);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 3);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 3, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], true, 8, 1, 0, 0);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 4);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 4, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				}
				break;

			case 1:
				_vm->_sound->command(9);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(local._detonatorHotspotId);
				_game._objects.addToInventory(OBJ_DETONATORS);
				_vm->_dialogs->showItem(OBJ_DETONATORS, 50326);
				break;

			case 2:
				if (_globals[kSexOfRex] == REX_MALE)
					_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				else
					_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);

				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(50328);
	else if (player_said_2(look, monitoring_equipment))
		_vm->_dialogs->show(50310);
	else if (player_said_2(look, photon_rifles))
		_vm->_dialogs->show(50311);
	else if (player_said_2(take, photon_rifles) || player_said_2(take, nuclear_slingshot))
		_vm->_dialogs->show(50312);
	else if (player_said_2(look, display_case))
		_vm->_dialogs->show(50313);
	else if (player_said_2(look, nuclear_slingshot))
		_vm->_dialogs->show(50314);
	else if (player_said_2(look, water_cooler))
		_vm->_dialogs->show(50315);
	else if (player_said_2(look, storage_box))
		_vm->_dialogs->show(50316);
	else if (player_said_2(open, storage_box))
		_vm->_dialogs->show(50317);
	else if (player_said_2(look, warning_label))
		_vm->_dialogs->show(50318);
	else if (player_said_2(look, desk))
		_vm->_dialogs->show(50319);
	else if (player_said_2(look, monitor))
		_vm->_dialogs->show(50320);
	else if (player_said_2(look, file_cabinets))
		_vm->_dialogs->show(50322);
	else if (player_said_2(look, box)) {
		if (_game._objects.isInRoom(OBJ_DETONATORS))
			_vm->_dialogs->show(50323);
		else
			_vm->_dialogs->show(50324);
	} else if (player_said_2(look, detonators) && (_action._savedFields._mainObjectSource == 4))
		_vm->_dialogs->show(50325);
	else if (player_said_2(look, windows))
		_vm->_dialogs->show(50327);
	else if (player_said_2(open, display_case))
		_vm->_dialogs->show(50329);
	else if (player_said_2(throw, display_case) && _game._objects.isInInventory(_game._objects.getIdFromDesc(_action._activeAction._objectNameId)))
		_vm->_dialogs->show(50330);
	else
		return;

	_action._inProgress = false;
}

void room_503_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._detonatorHotspotId);
}

void room_503_preload() {
	room_init_code_pointer = room_503_init;
	room_parser_code_pointer = room_503_parser;

	section_5_walker();
	section_5_interface();
	_scene->addActiveVocab(words_detonators);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
