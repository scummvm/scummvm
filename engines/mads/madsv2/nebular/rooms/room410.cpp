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
#include "mads/madsv2/nebular/rooms/section4.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static void room_410_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('y', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*ROXRC_7");

	if (_game._objects.isInRoom(OBJ_CHARGE_CASES))
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	else
		_scene->_hotspots.activate(words_charge_cases, false);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(155, 150);
		_game._player._facing = FACING_NORTH;
	}

	section_4_music();

	_scene->loadAnimation(Resources::formatName(410, 'r', -1, EXT_AA, ""));
	_scene->_animation[0]->_resetFlag = true;
}

static void room_410_daemon() {
	if (_scene->_animation[0]->getCurrentFrame() == 1) {
		if (_vm->getRandomNumber(1, 30) == 1)
			_scene->_animation[0]->setCurrentFrame(2);
		else
			_scene->_animation[0]->setCurrentFrame(0);
	}

	if (_scene->_animation[0]->getCurrentFrame() == 9) {
		if (_vm->getRandomNumber(1, 30) == 1)
			_scene->_animation[0]->setCurrentFrame(10);
		else
			_scene->_animation[0]->setCurrentFrame(8);
	}

	if (_scene->_animation[0]->getCurrentFrame() == 5) {
		if (_vm->getRandomNumber(1, 30) == 1)
			_scene->_animation[0]->setCurrentFrame(6);
		else
			_scene->_animation[0]->setCurrentFrame(4);
	}

	if (_scene->_animation[0]->getCurrentFrame() == 3) {
		if (_vm->getRandomNumber(1, 2) == 1)
			_scene->_animation[0]->setCurrentFrame(4);
		else // == 2
			_scene->_animation[0]->setCurrentFrame(8);
	}
}

static void room_410_pre_parser() {
	if (_action.isAction(words_take) && !_action.isObject(words_charge_cases))
		_game._player._needToWalk = false;

	if (_action.isAction(words_look, words_charge_cases) && _game._objects.isInRoom(OBJ_CHARGE_CASES))
		_game._player._needToWalk = true;

	if (_action.isAction(words_open, words_sacks) || _action.isAction(words_open, words_sack))
		_game._player._needToWalk = false;

	if (_action.isAction(words_look, words_can))
		_game._player._needToWalk = true;
}

static void room_410_parser() {
	if (_action.isAction(words_walk_into, words_corridor_to_south))
		_scene->_nextSceneId = 406;
	else if (_action.isAction(words_take, words_charge_cases) && (_game._objects.isInRoom(OBJ_CHARGE_CASES) || _game._trigger)) {
		switch (_game._trigger) {
		case 0:
			_vm->_sound->command(57);
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 7, 2, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 3);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 3, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_scene->_hotspots.activate(words_charge_cases, false);
			_game._objects.addToInventory(OBJ_CHARGE_CASES);
			_vm->_dialogs->showItem(OBJ_CHARGE_CASES, 41032);
			break;

		case 2:
			_game._player._priorTimer = _game._player._ticksAmount + _scene->_frameStartTime;
			_game._player._visible = true;
			_scene->_sequences.addTimer(20, 3);
			break;

		case 3:
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(words_look, words_barrel))
		_vm->_dialogs->show(41010);
	else if (_action.isAction(words_take, words_barrel))
		_vm->_dialogs->show(41011);
	else if (_action.isAction(words_open, words_barrel))
		_vm->_dialogs->show(41012);
	else if (_action.isAction(words_look, words_rug))
		_vm->_dialogs->show(41013);
	else if (_action.isAction(words_take, words_rug))
		_vm->_dialogs->show(41014);
	else if (_action.isAction(words_look, words_carton) || _action.isAction(words_open, words_carton)) {
		if (_game._objects.isInRoom(OBJ_CHARGE_CASES))
			_vm->_dialogs->show(41015);
		else
			_vm->_dialogs->show(41016);
	} else if (_action.isAction(words_look, words_flour))
		_vm->_dialogs->show(41017);
	else if (_action.isAction(words_take, words_flour))
		_vm->_dialogs->show(41018);
	else if (_action.isAction(words_look, words_sacks))
		_vm->_dialogs->show(41019);
	else if (_action.isAction(words_look, words_sack))
		_vm->_dialogs->show(41019);
	else if (_action.isAction(words_open, words_sacks))
		_vm->_dialogs->show(41020);
	else if (_action.isAction(words_open, words_sack))
		_vm->_dialogs->show(41020);
	else if (_action.isAction(words_look, words_bucket_of_tar))
		_vm->_dialogs->show(41021);
	else if (_action.isAction(words_take, words_bucket_of_tar))
		_vm->_dialogs->show(41022);
	else if (_action.isAction(words_look, words_can))
		_vm->_dialogs->show(41023);
	else if (_action.isAction(words_take, words_can))
		_vm->_dialogs->show(41024);
	else if (_action.isAction(words_look, words_charge_cases) && _game._objects.isInRoom(OBJ_CHARGE_CASES))
		_vm->_dialogs->show(41025);
	else if (_action.isAction(words_look, words_fence))
		_vm->_dialogs->show(41027);
	else if (_action.isAction(words_look, words_shelves))
		_vm->_dialogs->show(41028);
	else if (_action.isAction(words_look, words_rat))
		_vm->_dialogs->show(41029);
	else if (_action.isAction(words_take, words_rat))
		_vm->_dialogs->show(41030);
	else if (_action.isAction(words_throw, words_rat))
		_vm->_dialogs->show(41031);
	else if (_action._lookFlag)
		_vm->_dialogs->show(41033);
	else
		return;

	_action._inProgress = false;
}

void room_410_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_410_preload() {
	room_init_code_pointer = room_410_init;
	room_pre_parser_code_pointer = room_410_pre_parser;
	room_parser_code_pointer = room_410_parser;
	room_daemon_code_pointer = room_410_daemon;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
