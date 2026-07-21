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
#include "mads/nebular/rooms/section6.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _compactCaseHotspotId;
	int16 _noteHotspotId;
};

static Scratch local;


static void room_603_init() {
	if (_game._objects[OBJ_COMPACT_CASE]._roomNumber == _scene->_currentSceneId) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXMRD_3");
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', -1));
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 1);
		local._compactCaseHotspotId = _scene->_dynamicHotspots.add(words_compact_case, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(local._compactCaseHotspotId, Common::Point(250, 152), FACING_SOUTHEAST);
	}

	if ((_game._difficulty != DIFFICULTY_HARD) && (_game._objects[OBJ_NOTE]._roomNumber == _scene->_currentSceneId)) {
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXMRC_9");
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('p', -1));
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 14);
		local._noteHotspotId = _scene->_dynamicHotspots.add(words_note, words_walkto, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(local._noteHotspotId, Common::Point(242, 118), FACING_NORTHEAST);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG)
		_game._player._playerPos = Common::Point(113, 134);

	section_6_music();
}

static void room_603_parser() {
	if (player_said_2(walk_towards, livingroom))
		_scene->_nextSceneId = 602;
	else if (player_said_2(take, compact_case)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_COMPACT_CASE)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 8, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 5);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_vm->_sound->command(9);
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(local._compactCaseHotspotId);
				_game._objects.addToInventory(OBJ_COMPACT_CASE);
				_vm->_dialogs->showItem(OBJ_COMPACT_CASE, 60330);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[4]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
	} else if (player_said_2(take, note)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_NOTE)) {
			if (_game._trigger == 0) {
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
				_scene->_sequences.addTimer(15, 1);
			} else if (_game._trigger == 1) {
				_vm->_sound->command(9);
				_scene->_sequences.remove(_globals._sequenceIndexes[2]);
				_scene->_dynamicHotspots.remove(local._noteHotspotId);
				_game._objects.addToInventory(OBJ_NOTE);
				_scene->_sequences.remove(_globals._sequenceIndexes[3]);
				_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
				_game._player._visible = true;
				_game._player._stepEnabled = true;
			}
		} else
			_vm->_dialogs->show(60323);
	} else if (_action._lookFlag)
		_vm->_dialogs->show(60310);
	else if (player_said_2(look, bed))
		_vm->_dialogs->show(60311);
	else if (player_said_2(look, wig_stand))
		_vm->_dialogs->show(60312);
	else if (player_said_2(take, wig_stand))
		_vm->_dialogs->show(60313);
	else if (player_said_2(look, review))
		_vm->_dialogs->show(60314);
	else if (player_said_2(look, souvenir_tickets))
		_vm->_dialogs->show(60315);
	else if (player_said_2(look, photograph))
		_vm->_dialogs->show(60316);
	else if (player_said_2(look, lamp))
		_vm->_dialogs->show(60317);
	else if (player_said_2(look, directors_slate) || player_said_2(look, crop) || player_said_2(look, megaphone))
		_vm->_dialogs->show(60318);
	else if (player_said_2(look, snapshot))
		_vm->_dialogs->show(60319);
	else if (player_said_2(take, snapshot))
		_vm->_dialogs->show(60320);
	else if (player_said_2(look, perfume))
		_vm->_dialogs->show(60321);
	else if (player_said_2(take, perfume))
		_vm->_dialogs->show(60322);
	else if (player_said_2(look, note)) {
		if (_game._objects[OBJ_NOTE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60324);
	} else if (player_said_2(look, corner_table)) {
		if (_game._objects[OBJ_NOTE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60326);
		else
			_vm->_dialogs->show(60325);
	} else if (player_said_2(look, vanity)) {
		if (_game._objects[OBJ_COMPACT_CASE]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60327);
		else
			_vm->_dialogs->show(60328);
	} else if (player_said_2(look, compact_case) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(60329);
	// For the next two checks, the second part of the check wasn't surrounded par parenthesis, which was obviously wrong
	else if (player_said_1(look) && (player_said_1(bra) || player_said_1(boa) || player_said_1(slip)))
		_vm->_dialogs->show(60331);
	else if (player_said_1(take) && (player_said_1(bra) || player_said_1(boa) || player_said_1(slip)))
		_vm->_dialogs->show(60332);
	else
		return;

	_action._inProgress = false;
}

void room_603_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._compactCaseHotspotId);
	s.syncAsSint16LE(local._noteHotspotId);
}

void room_603_preload() {
	room_init_code_pointer = room_603_init;
	room_parser_code_pointer = room_603_parser;

	section_6_walker();
	section_6_interface();
	_scene->addActiveVocab(words_walkto);
	_scene->addActiveVocab(words_compact_case);
	_scene->addActiveVocab(words_note);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
