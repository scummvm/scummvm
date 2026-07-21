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

struct Scratch {
	int16 _fishingLineId;
};

static Scratch local;


static void room_701_init() {
	_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('x', 0));
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('b', 5));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('b', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('b', 1));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RM202A1");
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('b', 8));

	if (_scene->_roomChanged) {
		_game._objects.addToInventory(OBJ_BINOCULARS);
		_game._objects.addToInventory(OBJ_TWINKIFRUIT);
		_game._objects.addToInventory(OBJ_BOMB);
		_game._objects.addToInventory(OBJ_CHICKEN);
		_game._objects.addToInventory(OBJ_BONES);

		_globals[kCityFlooded] = true;
		_globals[kLineStatus] = LINE_TIED;
		_globals[kBoatRaised] = false;
	}

	if (_globals[kBoatStatus] == BOAT_UNFLOODED) {
		if (_globals[kBoatRaised])
			_globals[kBoatStatus] = BOAT_GONE;
		else if (_globals[kLineStatus] == LINE_TIED)
			_globals[kBoatStatus] = BOAT_TIED_FLOATING;
		else if (_game._difficulty == DIFFICULTY_HARD)
			_globals[kBoatStatus] = BOAT_ADRIFT;
		else
			_globals[kBoatStatus] = BOAT_TIED;
	}

	_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
	_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
	_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);

	int boatStatus = (_scene->_priorSceneId == 703) ? BOAT_GONE : _globals[kBoatStatus];

	switch (boatStatus) {
	case BOAT_TIED_FLOATING:
		_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 20, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 10);
		break;
	case BOAT_ADRIFT:
		_globals._sequenceIndexes[6] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[6], false, 20, 0, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 10);
		break;
	case BOAT_TIED:
	{
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		int idx = _scene->_dynamicHotspots.add(words_boat, words_climb_into, _globals._sequenceIndexes[2], Common::Rect());
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(231, 127), FACING_NORTH);
		break;
	}
	case BOAT_GONE:
		_scene->_hotspots.activate(words_boat, false);
		break;
	default:
		break;
	}

	if (_globals[kLineStatus] == LINE_DROPPED || _globals[kLineStatus] == LINE_TIED) {
		_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[3], 8);
		int idx = _scene->_dynamicHotspots.add(words_fishing_line, words_walkto, _globals._sequenceIndexes[3], Common::Rect(0, 0, 0, 0));
		local._fishingLineId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(234, 129), FACING_NORTHEAST);
	}

	if (_scene->_priorSceneId == 702) {
		_game._player._playerPos = Common::Point(309, 138);
		_game._player._facing = FACING_WEST;
	} else if (_scene->_priorSceneId == 710) {
		_game._player._playerPos = Common::Point(154, 129);
		_game._player._facing = FACING_NORTH;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(155, 129));
		_scene->_sequences.addTimer(15, 60);
	} else if (_scene->_priorSceneId == 703) {
		_game._player._playerPos = Common::Point(231, 127);
		_game._player._facing = FACING_SOUTH;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->loadAnimation(formAnimName('B', 1), 80);
		_vm->_sound->command(28);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG && _scene->_priorSceneId != 620) {
		_game._player._playerPos = Common::Point(22, 131);
		_game._player._facing = FACING_EAST;
		_game._player._stepEnabled = false;
		_scene->_sequences.addTimer(60, 70);
	}

	_game.loadQuoteSet(0x310, 0x30F, 0);
	section_7_music();
}

static void room_701_daemon() {
	switch (_game._trigger) {
	case 60:
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		_globals._sequenceIndexes[5] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[5], false, 6, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(155, 129));
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		break;

	case 61:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[5]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		break;

	case 70:
		_vm->_sound->command(16);
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 71);
		break;

	case 71:
		_game._player.walk(Common::Point(61, 131), FACING_EAST);
		_scene->_sequences.addTimer(120, 72);
		break;

	case 72:
		_vm->_sound->command(17);
		_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 73);
		break;

	case 73:
		_game._player._stepEnabled = true;
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
		_scene->_kernelMessages.reset();
		break;

	case 80:
	{
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_frameStartTime - _game._player._ticksAmount;
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
		int idx = _scene->_dynamicHotspots.add(words_boat, words_climb_into, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(234, 129), FACING_NORTH);
		_globals[kBoatStatus] = BOAT_TIED;
		_game._player._stepEnabled = true;
	}
	break;

	default:
		break;
	}
}

static void room_701_pre_parser() {
	if (player_said_2(walkto, east_end_of_platform))
		_game._player._walkOffScreenSceneId = 702;

	if (player_said_2(look, building))
		_game._player.walk(Common::Point(154, 129), FACING_NORTHEAST);

	if (player_said_3(look, binoculars, building))
		_game._player.walk(Common::Point(154, 129), FACING_NORTH);
}

static void room_701_parser() {
	if (player_said_2(walk_along, platform)) {
	} else if (player_said_3(look, binoculars, building) && _game._objects[OBJ_VASE]._roomNumber == 706) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 6, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(155, 129));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int temp = _globals._sequenceIndexes[5];
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, -2);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[5], Common::Point(155, 129));
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], temp);
			_scene->_sequences.addTimer(15, 2);
		}
		break;

		case 2:
			_scene->_nextSceneId = 710;
			break;

		default:
			break;
		}
	} else if (player_said_2(step_into, elevator)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_vm->_sound->command(16);
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x310, 34, 0, 120, _game.getQuote(0x30D));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			_game._player.walk(Common::Point(22, 131), FACING_EAST);
			_scene->_sequences.addTimer(120, 3);
			break;

		case 3:
			_vm->_sound->command(17);
			_globals._sequenceIndexes[1] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[1], false, 5, 1, 0, 0);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 4);
			break;

		case 4:
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, -1);
			_scene->_sequences.setPosition(_globals._sequenceIndexes[1], Common::Point(48, 136));
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 10);
			_globals[kResurrectRoom] = 701;
			_scene->_nextSceneId = 605;
			break;

		default:
			break;
		}
	} else if ((player_said_2(pull, boat) || player_said_2(take, boat) ||
		player_said_2(pull, fishing_line) || player_said_2(take, fishing_line)) &&
		!_game._objects.isInInventory(OBJ_FISHING_LINE)) {
		if (_globals[kBoatStatus] == BOAT_TIED_FLOATING) {
			switch (_game._trigger) {
			case 0:
				_game._player._stepEnabled = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[4]);
				_scene->_sequences.remove(_globals._sequenceIndexes[3]);
				_scene->_dynamicHotspots.remove(local._fishingLineId);
				_scene->_hotspots.activate(words_boat, false);
				_game._player._visible = false;
				_scene->loadAnimation(formAnimName('E', -1), 1);
				break;

			case 1:
			{
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
				_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 9);
				int idx = _scene->_dynamicHotspots.add(words_boat, words_climb_into, _globals._sequenceIndexes[2], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(231, 127), FACING_NORTH);
				_scene->_sequences.addTimer(15, 2);
			}
			break;

			case 2:
				_globals[kBoatStatus] = BOAT_TIED;
				_globals[kLineStatus] = LINE_NOW_UNTIED;
				_game._player._stepEnabled = true;
				break;

			default:
				break;
			}
		} else if (_globals[kBoatStatus] == BOAT_TIED) {
			_vm->_dialogs->show(70125);
		} else if (_globals[kLineStatus] == LINE_DROPPED) {
			_globals[kLineStatus] = LINE_NOW_UNTIED;
			_game._objects.addToInventory(OBJ_FISHING_LINE);
			_vm->_sound->command(15);
			_scene->_sequences.remove(_globals._sequenceIndexes[3]);
			_vm->_dialogs->showItem(OBJ_FISHING_LINE, 70126);
		} else {
			_vm->_dialogs->show(70127);
		}
	} else if (player_said_2(climb_into, boat) && _globals[kBoatStatus] == BOAT_TIED) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_game._player._visible = false;
			_scene->loadAnimation(formAnimName('B', 0), 1);
			break;

		case 1:
			_scene->_nextSceneId = 703;
			break;

		default:
			break;
		}
	} else if (_action._lookFlag) {
		if (_globals[kBoatStatus] != BOAT_GONE) {
			if (_globals[kBoatStatus] == BOAT_TIED)
				_vm->_dialogs->show(70128);
			else
				_vm->_dialogs->show(70110);
		} else
			_vm->_dialogs->show(70111);
	} else if (player_said_2(look, submerged_city))
		_vm->_dialogs->show(70112);
	else if (player_said_2(look, elevator))
		_vm->_dialogs->show(70113);
	else if (player_said_2(look, platform))
		_vm->_dialogs->show(70114);
	else if (player_said_2(look, cement_pylon))
		_vm->_dialogs->show(70115);
	else if (player_said_2(look, hook)) {
		if (_globals[kLineStatus] == LINE_NOT_DROPPED || _globals[kLineStatus] == LINE_NOW_UNTIED)
			_vm->_dialogs->show(70116);
		else
			_vm->_dialogs->show(70117);
	} else if (player_said_2(look, rock))
		_vm->_dialogs->show(70118);
	else if (player_said_2(take, rock))
		_vm->_dialogs->show(70119);
	else if (player_said_2(look, east_end_of_platform))
		_vm->_dialogs->show(70120);
	else if (player_said_2(look, building))
		_vm->_dialogs->show(70121);
	else if (player_said_2(look, boat)) {
		if (_globals[kBoatStatus] == BOAT_ADRIFT || _globals[kBoatStatus] == BOAT_TIED_FLOATING)
			_vm->_dialogs->show(70122);
		else
			_vm->_dialogs->show(70123);
	} else if (player_said_3(cast, fishing_rod, boat) && _game._objects.isInInventory(OBJ_FISHING_LINE))
		_vm->_dialogs->show(70124);
	else
		return;

	_action._inProgress = false;
}

void room_701_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._fishingLineId);
}

void room_701_preload() {
	room_init_code_pointer = room_701_init;
	room_daemon_code_pointer = room_701_daemon;
	room_pre_parser_code_pointer = room_701_pre_parser;
	room_parser_code_pointer = room_701_parser;

	section_7_walker();
	section_7_interface();
	_scene->addActiveVocab(words_boat);
	_scene->addActiveVocab(words_climb_into);
	_scene->addActiveVocab(words_fishing_line);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
