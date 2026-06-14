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
#include "mads/madsv2/nebular/rooms/section2.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_215_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('e', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('a', 0));

	_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 7, 0, 0, 0);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(235, 83));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 15);

	if (_globals[kSexOfRex] == REX_MALE)
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRC_9");
	else
		_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*ROXRC_9");

	if (_scene->_priorSceneId == 216) {
		_game._player._playerPos = Common::Point(140, 119);
		_game._player._facing = FACING_SOUTHWEST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, 1);
		_scene->_sequences.addTimer(120, 70);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(204, 152);
		_game._player._facing = FACING_NORTH;
	}

	_game.loadQuoteSet(0xA9, 0xAA, 0);
	section_2_music();
}

static void room_215_daemon() {
	if (_game._trigger == 70) {
		_scene->_sequences.remove(_globals._sequenceIndexes[3]);
		_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
	}

	if (_game._trigger == 71) {
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
	}
}

static void room_215_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(21509);
	else if (_action.isAction(VERB_TAKE, NOUN_TWINKIFRUIT)) {
		if (!_game._objects.isInInventory(OBJ_TWINKIFRUIT) || _game._trigger) {
			switch (_game._trigger) {
			case 0:
				if (_globals[kSexOfRex] == REX_MALE) {
					_game._player._visible = false;
					_game._player._stepEnabled = false;
					_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 6, 2, 0, 0);
					_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 4);
					_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_LOOP, 0, 1);
					_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				} else {
					_game._objects.addToInventory(OBJ_TWINKIFRUIT);
					_vm->_dialogs->showItem(OBJ_TWINKIFRUIT, 0x5404);
				}
				break;

			case 1:
				if (!_game._objects.isInInventory(OBJ_TWINKIFRUIT)) {
					_game._objects.addToInventory(OBJ_TWINKIFRUIT);
					_vm->_dialogs->showItem(OBJ_TWINKIFRUIT, 0x5404);
				}
				break;

			case 2:
				_game._player._visible = true;
				_game._player._stepEnabled = true;
				_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
				break;

			default:
				break;
			}
		} else {
			int idx = _vm->getRandomNumber(169, 170);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(idx));
		}
	} else if (_action.isAction(VERB_WALK_OUTSIDE, NOUN_HUT))
		_scene->_nextSceneId = 210;
	else if (_action.isAction(VERB_LOOK, NOUN_BEAR_RUG))
		_vm->_dialogs->show(21501);
	else if (_action.isAction(VERB_LOOK, NOUN_BED))
		_vm->_dialogs->show(21502);
	else if (_action.isAction(VERB_LOOK, NOUN_WELCOME_MAT))
		_vm->_dialogs->show(21503);
	else if (_action.isAction(VERB_LOOK, NOUN_LOVE_ALTAR))
		_vm->_dialogs->show(21504);
	else if (_action.isAction(VERB_LOOK, NOUN_WINDOW))
		_vm->_dialogs->show(21505);
	else if (_action.isAction(VERB_LOOK, NOUN_PICTURE))
		_vm->_dialogs->show(21506);
	else if (_action.isAction(VERB_LOOK, NOUN_TWINKIFRUIT) && (_action._savedFields._mainObjectSource == 4))
		_vm->_dialogs->show(21507);
	else if (_action.isAction(VERB_TAKE, NOUN_BEAR_RUG))
		_vm->_dialogs->show(21510);
	else if (_action.isAction(VERB_TAKE, NOUN_LOVE_ALTAR))
		_vm->_dialogs->show(21511);
	else if (_action.isAction(VERB_LOOK, NOUN_BAG_OF_TWINKIFRUITS))
		_vm->_dialogs->show(21512);
	else if (_action.isAction(VERB_TAKE, NOUN_BAG_OF_TWINKIFRUITS))
		_vm->_dialogs->show(21513);
	else if (_action.isAction(VERB_TAKE, NOUN_WELCOME_MAT))
		_vm->_dialogs->show(21514);
	else
		return;

	_action._inProgress = false;
}

void room_215_preload() {
	room_init_code_pointer = room_215_init;
	room_parser_code_pointer = room_215_parser;
	room_daemon_code_pointer = room_215_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
