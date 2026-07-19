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
#include "mads/madsv2/nebular/rooms/section6.h"
#include "mads/madsv2/nebular/rooms/thunks.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int16 _actionMode;
	int16 _cycleIndex;
};

static Scratch local;


static void handleWinchMovement() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 10, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 1, 5);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_SPRITE, 5, 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
		// CHECKME: Is the "else" block useful as action is always equal to 1 at this point?
		// Or is it a missing bit of code we could fix?
		if (local._actionMode == 1) {
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 17, 7, 0, 0);
			_vm->_sound->command(19);
			_game._objects.setRoom(OBJ_PADLOCK_KEY, 1);
			_globals[kBoatRaised] = false;
		} else {
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 17, 9, 0, 0);
			_vm->_sound->command(18);
		}
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		break;

	case 2:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[4]);
		_game._player._visible = true;
		break;

	case 3:
	{
		int syncIdx = _globals._sequenceIndexes[2];
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, local._cycleIndex);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F4));
		_game._player._stepEnabled = true;

		_vm->_dialogs->show(61217);
	}
	break;

	default:
		break;
	}
}

static void room_612_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('c', -1));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('p', -1));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCD_3");
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXRC_6");

	if ((_globals[kLineStatus] == 2) || (_globals[kLineStatus] == 3)) {
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('f', -1));
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		int idx = _scene->_dynamicHotspots.add(words_fishing_line, words_walkto, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(34, 117), FACING_SOUTHEAST);
	}

	if (_globals[kBoatRaised])
		local._cycleIndex = -2;
	else
		local._cycleIndex = -1;

	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, local._cycleIndex);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(280, 75);
		_game._player._facing = FACING_SOUTHWEST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 3);
		_scene->loadAnimation(formAnimName('R', 1), 70);
	}

	section_6_music();

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_PADLOCK_KEY);

	_game.loadQuoteSet(0x2F5, 0x2F4, 0);
}

static void room_612_daemon() {
	switch (_game._trigger) {
	case 70:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(6, 71);
		break;

	case 71:
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

static void room_612_parser() {
	if (_action.isAction(words_get_inside, words_car)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 2);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[1];
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 3);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncIdx);
			_scene->_sequences.addTimer(6, 2);
		}
		break;

		case 2:
			_game._player._visible = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], true, 10, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], true, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			_scene->_nextSceneId = 504;
		}
		break;

		default:
			break;
		}
	} else if (_action.isAction(words_unlock, words_padlock_key, words_control_box)) {
		local._cycleIndex = -2;
		local._actionMode = 1;
		handleWinchMovement();
	} else if (_action._lookFlag || _action.isAction(words_look, words_expressway))
		_vm->_dialogs->show(61210);
	else if (_action.isAction(words_look, words_rope) || _action.isAction(words_look, words_armature)) {
		if (_globals[kBoatRaised])
			_vm->_dialogs->show(61211);
		else
			_vm->_dialogs->show(61212);
	} else if (_action.isAction(words_take, words_rope))
		_vm->_dialogs->show(61213);
	else if (_action.isAction(words_look, words_control_box)) {
		if (_globals[kBoatRaised])
			_vm->_dialogs->show(61214);
		else
			_vm->_dialogs->show(61216);
	} else if (_action.isAction(words_open, words_control_box))
		_vm->_dialogs->show(61215);
	else if (_action.isAction(words_look, words_buildings))
		_vm->_dialogs->show(61218);
	else if (_action.isAction(words_look, words_dome))
		_vm->_dialogs->show(61219);
	else if (_action.isAction(words_look, words_statue))
		_vm->_dialogs->show(61220);
	else if (_action.isAction(words_look, words_maintenance_building))
		_vm->_dialogs->show(61221);
	else if (_action.isAction(words_open, words_maintenance_building))
		_vm->_dialogs->show(61222);
	else if (_action.isAction(words_look, words_wall))
		_vm->_dialogs->show(61223);
	else if (_action.isAction(words_look, words_support))
		_vm->_dialogs->show(61224);
	else if (_action.isAction(words_walk_down, words_expressway_to_east) || _action.isAction(words_walk_down, words_expressway_to_west))
		_vm->_dialogs->show(61225);
	else
		return;

	_action._inProgress = false;
}

void room_612_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._actionMode);
	s.syncAsSint16LE(local._cycleIndex);
}

void room_612_preload() {
	room_init_code_pointer = room_612_init;
	room_daemon_code_pointer = room_612_daemon;
	room_parser_code_pointer = room_612_parser;

	section_6_walker();
	section_6_interface();
	_scene->addActiveVocab(words_fishing_line);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
