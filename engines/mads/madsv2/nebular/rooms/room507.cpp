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
	int16 _penlightHotspotId;
};

static Scratch local;


static void room_507_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('p', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRD_3");

	if ((_game._difficulty != DIFFICULTY_EASY) && (_game._objects[OBJ_PENLIGHT]._roomNumber == _scene->_currentSceneId)) {
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 9, 0, 0, 0);
		local._penlightHotspotId = _scene->_dynamicHotspots.add(NOUN_PENLIGHT, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(local._penlightHotspotId, Common::Point(233, 152), FACING_SOUTHEAST);
	}

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(121, 147);
		_game._player._facing = FACING_NORTH;
	}

	section_5_music();
}

static void room_507_parser() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_ENTRANCE))
		_scene->_nextSceneId = 506;
	else if (_action.isAction(VERB_TAKE, NOUN_PENLIGHT)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_PENLIGHT)) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 5);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 5, 1);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				_scene->_sequences.remove(_globals._sequenceIndexes[1]);
				_scene->_dynamicHotspots.remove(local._penlightHotspotId);
				_vm->_sound->command(27);
				_game._objects.addToInventory(OBJ_PENLIGHT);
				_vm->_dialogs->showItem(OBJ_PENLIGHT, 50730);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(50722);
	else if (_action.isAction(VERB_LOOK, NOUN_SWIRLING_LIGHT))
		_vm->_dialogs->show(50710);
	else if (_action.isAction(VERB_TAKE, NOUN_SWIRLING_LIGHT))
		_vm->_dialogs->show(50711);
	else if (_action.isAction(VERB_LOOK, NOUN_OLD_SOFTWARE))
		_vm->_dialogs->show(50712);
	else if (_action.isAction(VERB_TAKE, NOUN_OLD_SOFTWARE))
		_vm->_dialogs->show(50713);
	else if (_action.isAction(VERB_LOOK, NOUN_ADVERTISEMENT))
		_vm->_dialogs->show(50714);
	else if (_action.isAction(VERB_LOOK, NOUN_ADVERTISING_POSTER))
		_vm->_dialogs->show(50715);
	else if (_action.isAction(VERB_LOOK, NOUN_SIGN)) {
		if (_scene->_customDest.x < 100)
			_vm->_dialogs->show(50726);
		else
			_vm->_dialogs->show(50716);
	} else if (_action.isAction(VERB_LOOK, NOUN_HOTTEST_SOFTWARE))
		_vm->_dialogs->show(50717);
	else if (_action.isAction(VERB_LOOK, NOUN_SOFTWARE_SHELF))
		_vm->_dialogs->show(50718);
	else if (_action.isAction(VERB_LOOK, NOUN_SENSOR))
		_vm->_dialogs->show(50719);
	else if (_action.isAction(VERB_LOOK, NOUN_CASH_REGISTER))
		_vm->_dialogs->show(50720);
	else if (_action.isAction(VERB_LOOK, NOUN_PAD_OF_PAPER))
		_vm->_dialogs->show(50721);
	else if (_action.isAction(VERB_OPEN, NOUN_CASH_REGISTER))
		_vm->_dialogs->show(50723);
	else if (_action.isAction(VERB_LOOK, NOUN_BARGAIN_VAT))
		_vm->_dialogs->show(50724);
	else if (_action.isAction(VERB_LOOK, NOUN_WINDOW))
		_vm->_dialogs->show(50725);
	else if (_action.isAction(VERB_WALK_BEHIND, NOUN_COUNTER)) {
		// WORKAROUND: Empty handling to prevent default "can't do that" dialogs showing
	} else if (_action.isAction(VERB_LOOK, NOUN_COUNTER)) {
		if (_game._objects.isInRoom(OBJ_PENLIGHT))
			_vm->_dialogs->show(50728);
		else
			_vm->_dialogs->show(50727);
	} else if (_action.isAction(VERB_LOOK, NOUN_PENLIGHT) && !_game._objects.isInInventory(OBJ_PENLIGHT)) {
		if (_game._objects.isInRoom(OBJ_PENLIGHT))
			_vm->_dialogs->show(50729);
	} else if (_action.isAction(VERB_LOOK, NOUN_EMERGENCY_LIGHT))
		_vm->_dialogs->show(50731);
	else
		return;

	_action._inProgress = false;
}

void room_507_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._penlightHotspotId);
}

void room_507_preload() {
	room_init_code_pointer = room_507_init;
	room_parser_code_pointer = room_507_parser;

	section_5_walker();
	section_5_interface();
	_scene->addActiveVocab(NOUN_PENLIGHT);
	_scene->addActiveVocab(VERB_WALKTO);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
