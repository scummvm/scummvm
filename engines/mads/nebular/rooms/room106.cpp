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
#include "mads/nebular/rooms/section1.h"
#include "mads/nebular/rooms/thunks.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _backToShipFl = false;
	bool _shadowFl = false;
	bool _firstEmergingFl = false;
	long _positionY = 0;
};

static Scratch local;


static void room_106_init() {
	_globals._spriteIndexes[0] = _scene->_sprites.addSprites(formAnimName('H', -1));

	if (_game._objects.isInInventory(OBJ_REBREATHER) || (_scene->_priorSceneId != 102) || _scene->_roomChanged) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('A', 0));
		_globals._spriteIndexes[3] = _scene->_sprites.addSprites(formAnimName('A', 1));
	}

	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('G', -1));
	_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], false, 21, 0, 0, 0);
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('I', -1));
	_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 6, 0, 32, 47);

	if (_scene->_priorSceneId == 102) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 1, 4, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[0], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_game._player._facing = FACING_EAST;
		_game._player._playerPos = Common::Point(106, 69);
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		if (_scene->_priorSceneId == 107) {
			_game._player._playerPos = Common::Point(319, 84);
			_game._player._facing = _game._player._prepareWalkFacing = FACING_WEST;
		} else {
			_game._player._playerPos = Common::Point(319, 44);
			_game._player._facing = _game._player._prepareWalkFacing = FACING_SOUTHWEST;
			_scene->_sprites[_game._player._spritesStart + 3]->_charInfo->_velocity = 24;
		}

		_game._player._prepareWalkPos = Common::Point(246, 69);
		_game._player._needToWalk = true;
		_game._player._readyToWalk = true;
	}

	if (_scene->_priorSceneId != 102) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -2, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);
	}

	local._backToShipFl = false;
	local._shadowFl = false;
	local._firstEmergingFl = false;

	_game.loadQuoteSet(0x31, 0x32, 0x34, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0);
	section_1_music();
}

static void room_106_daemon() {
	if (_game._trigger == 70) {
		_globals._sequenceIndexes[0] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[0], false, 6, 0, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[0], -2, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[0], 14);

		if (!_game._objects.isInInventory(OBJ_REBREATHER) && !_scene->_roomChanged) {
			_scene->loadAnimation(Resources::formatName(106, 'A', -1, EXT_AA, ""), 75);
		} else {
			_globals._sequenceIndexes[1] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[1], false, 4, 1, 0, 0);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 28, 71);
		}
	}

	if (_game._trigger == 71) {
		_game._player._prepareWalkPos = Common::Point(246, 69);
		_game._player._prepareWalkFacing = FACING_EAST;
		_game._player._needToWalk = true;
		_game._player._readyToWalk = true;
		_game._player._visible = true;

		if (_game._visitedScenes._sceneRevisited) {
			_game._player._stepEnabled = true;
		} else {
			_game._player._prepareWalkFacing = FACING_SOUTHWEST;
			local._firstEmergingFl = true;
			_scene->loadAnimation(Resources::formatName(106, 'B', -1, EXT_AA, ""), 80);
		}
	}

	if (local._firstEmergingFl && (_scene->_animation[0]->getCurrentFrame() >= 19)) {
		local._firstEmergingFl = false;
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(52));
	}

	if ((_game._trigger >= 80) && (_game._trigger <= 87)) {
		int tmpVal = _game._trigger - 80;
		int msgId = -1;
		switch (tmpVal) {
		case 0:
			local._positionY = 26;
			msgId = 49;
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			msgId = 76 + tmpVal;
			break;

		case 6:
			msgId = 50;
			break;

		default:
			msgId = -1;
			_game._player._stepEnabled = true;
			break;
		}

		if (msgId >= 0) {
			int nextTrigger = _game._trigger + 1;
			_scene->_kernelMessages.add(Common::Point(15, local._positionY), 0x1110, 0, 0, 360, _game.getQuote(msgId));
			_scene->_sequences.addTimer(150, nextTrigger);
			local._positionY += 14;
		}
	}

	if (local._backToShipFl) {
		if (!local._shadowFl) {
			if (_game._player._playerPos.x < 204) {
				local._shadowFl = true;
				_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 4, 1, 0, 0);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_SPRITE, 44, 73);
			}
		} else if (_game._trigger == 73)
			_game._player._visible = false;
		else if (_game._trigger == 72)
			_scene->_sequences.addTimer(24, 74);
		else if (_game._trigger == 74)
			_scene->_nextSceneId = 102;
	}

	if (_game._trigger == 75) {
		_game._visitedScenes.pop_back();
		_scene->_nextSceneId = 102;
	}
}

static void room_106_pre_parser() {
	if (player_said_2(swim_towards, sea_cliff) || player_said_2(swim_towards, seaweed_bank)) {
		_game._player._stepEnabled = false;
		_scene->_sprites[_game._player._spritesStart + 1]->_charInfo->_velocity = 24;
		_game._player._walkOffScreenSceneId = 104;
	}

	if (player_said_2(swim_towards, open_area_to_east))
		_game._player._walkOffScreenSceneId = 107;
}

static void room_106_parser() {
	if (_action._lookFlag)
		_vm->_dialogs->show(10614);
	else if (player_said_2(swim_to, main_airlock)) {
		_game._player._stepEnabled = false;
		_game._player._prepareWalkPos = Common::Point(95, 72);
		_game._player._prepareWalkFacing = FACING_WEST;
		_game._player._needToWalk = true;
		_game._player._readyToWalk = true;
		_game._player._frameNumber = 9;
		local._backToShipFl = true;
	} else if (player_said_2(look, anemone) || player_said_2(look_at, anemone))
		_vm->_dialogs->show(10601);
	else if (player_said_2(take, anemone))
		_vm->_dialogs->show(10602);
	else if (player_said_2(look, seaweed) || player_said_2(look, seaweed_bank))
		_vm->_dialogs->show(10603);
	else if (player_said_2(take, seaweed) || player_said_2(take, seaweed_bank))
		_vm->_dialogs->show(10604);
	else if (player_said_2(look, open_area_to_east))
		_vm->_dialogs->show(10605);
	else if (player_said_2(look, pile_of_rocks) || player_said_2(look_at, pile_of_rocks))
		_vm->_dialogs->show(10606);
	else if (player_said_1(pile_of_rocks) && (player_said_1(push) || player_said_1(pull) || player_said_1(take)))
		_vm->_dialogs->show(10607);
	else if (player_said_2(look, ship) || player_said_2(look_at, ship))
		_vm->_dialogs->show(10608);
	else if (player_said_2(look, main_airlock))
		_vm->_dialogs->show(10609);
	else if (player_said_2(open, main_airlock))
		_vm->_dialogs->show(10610);
	else if (player_said_2(close, main_airlock))
		_vm->_dialogs->show(10611);
	else if (player_said_2(look, sea_cliff))
		_vm->_dialogs->show(10612);
	else if (player_said_2(look, ocean_floor))
		_vm->_dialogs->show(10613);
	else
		return;

	_action._inProgress = false;
}

void room_106_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._backToShipFl);
	s.syncAsByte(local._shadowFl);
	s.syncAsByte(local._firstEmergingFl);
	s.syncAsSint32LE(local._positionY);
}

void room_106_preload() {
	room_init_code_pointer = room_106_init;
	room_pre_parser_code_pointer = room_106_pre_parser;
	room_parser_code_pointer = room_106_parser;
	room_daemon_code_pointer = room_106_daemon;

	section_1_walker();
	section_1_interface();

	if ((_scene->_priorSceneId == 102) && !_game._objects.isInInventory(OBJ_REBREATHER) && !_scene->_roomChanged)
		*player.series_name = '\0';

	_vm->_dialogs->_defaultPosition.y = 100;

}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
