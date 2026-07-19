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
	int32 _dogTimer;
	int32 _lastFrameTime;
	bool _dogLoop;
	bool _dogEatsRex;
	bool _dogBarking;
	bool _shopAvailable;
	int16 _animationMode;
	int16 _animationActive;
	int16 _counter;
};

static Scratch local;


static void room_607_init() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCD_3");

	if (!_game._visitedScenes._sceneRevisited && (_scene->_priorSceneId != 608))
		_globals[kDogStatus] = DOG_PRESENT;

	if ((_scene->_priorSceneId == 608) && (_globals[kDogStatus] < DOG_GONE))
		_globals[kDogStatus] = DOG_GONE;

	local._animationActive = 0;

	if ((_globals[kDogStatus] == DOG_PRESENT) && (_game._difficulty != DIFFICULTY_EASY)) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('g', 3));
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('g', 7));
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('g', 0));

		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		local._dogBarking = false;
		local._dogLoop = false;
		local._shopAvailable = false;
		local._dogEatsRex = false;
		local._dogTimer = 0;
	} else
		_scene->_hotspots.activate(words_obnoxious_dog, false);

	_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);

	if (_scene->_priorSceneId == 608) {
		_game._player._playerPos = Common::Point(297, 50);
		_game._player._facing = FACING_SOUTHEAST;
	} else if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(40, 104);
		_game._player._facing = FACING_SOUTHEAST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
		_scene->loadAnimation(formAnimName('R', 1), 80);
	} else if (_globals[kDogStatus] == DOG_LEFT) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('g', 3));
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('g', 7));
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('g', 0));
	}

	section_6_music();

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_BONES);

	_vm->_palette->setEntry(252, 63, 44, 30);
	_vm->_palette->setEntry(253, 63, 20, 22);
	_game.loadQuoteSet(0x2F8, 0x2F7, 0x2F6, 0x2F9, 0x2FA, 0);
}

static void room_607_daemon() {
	if (_globals[kDogStatus] == DOG_LEFT) {
		int32 diff = _scene->_frameStartTime - local._lastFrameTime;
		if ((diff >= 0) && (diff <= 4))
			local._dogTimer += diff;
		else
			local._dogTimer++;

		local._lastFrameTime = _scene->_frameStartTime;
	}

	if ((local._dogTimer >= 480) && !local._dogLoop && !local._shopAvailable && (_globals[kDogStatus] == DOG_LEFT) && !_game._player._special) {
		_vm->_sound->command(14);
		local._dogLoop = true;
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 10, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 91);
		local._dogLoop = false;
		local._dogTimer = 0;
	}

	if (_game._trigger == 91) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		local._dogBarking = false;
		_globals[kDogStatus] = DOG_PRESENT;
		_scene->_hotspots.activate(words_obnoxious_dog, true);
	}

	if (!local._dogEatsRex && (_game._difficulty != DIFFICULTY_EASY) && !local._animationActive && (_globals[kDogStatus] == DOG_PRESENT)
		&& !local._dogBarking && (_vm->getRandomNumber(1, 50) == 10)) {
		local._dogBarking = true;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 5, 8, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		_scene->_kernelMessages.reset();
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 2, 100);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		local._counter = 0;
	}

	if ((_game._trigger == 70) && !local._dogEatsRex && (_globals[kDogStatus] == DOG_PRESENT) && !local._animationActive) {
		int syncIdx = _globals._sequenceIndexes[1];
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncIdx);
		_scene->_kernelMessages.reset();
		local._dogBarking = false;
	}

	if (_game._trigger == 100) {
		local._counter++;
		_vm->_sound->command(12);

		if ((local._counter >= 1) && (local._counter <= 4)) {
			Common::Point pos(0, 0);
			switch (local._counter) {
			case 1:
				pos = Common::Point(237, 5);
				break;

			case 2:
				pos = Common::Point(270, 15);
				break;

			case 3:
				pos = Common::Point(237, 25);
				break;

			case 4:
				pos = Common::Point(270, 36);
				break;

			default:
				break;
			}
			_scene->_kernelMessages.add(pos, 0xFDFC, 0, 0, 120, _game.getQuote(0x2F9));
		}
	}

	if (_game._player._moving && (_game._difficulty != DIFFICULTY_EASY) && !local._shopAvailable && (_globals[kDogStatus] == DOG_PRESENT) && (_scene->_rails.getNext() > 0)) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(268, 72), FACING_NORTHEAST);
		_scene->_rails.resetNext();
	}

	if ((_game._player._special > 0) && (_game._difficulty != DIFFICULTY_EASY) && (_globals[kDogStatus] == DOG_PRESENT) && _game._player._stepEnabled)
		_game._player._stepEnabled = false;

	if ((_game._difficulty != DIFFICULTY_EASY) && (_globals[kDogStatus] == DOG_PRESENT) && (_game._player._playerPos == Common::Point(268, 72))
		&& (_game._trigger || !local._dogEatsRex)) {
		local._dogEatsRex = true;
		switch (_game._trigger) {
		case 91:
		case 0:
			local._animationActive = 1;
			_game._player._visible = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 10, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], -1, 7);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FA));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 60);
			_scene->_sequences.addTimer(10, 64);
			break;

		case 60:
		{
			int syncIdx = _globals._sequenceIndexes[4];
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 5, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 8, 45);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 61);
		}
		break;

		case 61:
		{
			int syncIdx = _globals._sequenceIndexes[4];
			_globals._sequenceIndexes[4] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[4], false, 15, 3, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[4], 46, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 62);
		}
		break;

		case 62:
		{
			int syncIdx = _globals._sequenceIndexes[4];
			local._animationActive = 2;
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_sequences.addTimer(60, 63);
		}
		break;

		case 63:
			_vm->_dialogs->show(60729);
			local._animationActive = 0;
			local._dogEatsRex = false;
			_scene->_reloadSceneFlag = true;
			_game._player._stepEnabled = true;
			break;

		case 64:
			if (local._dogEatsRex && (local._animationActive == 1)) {
				_vm->_sound->command(12);
				_scene->_sequences.addTimer(10, 64);
			}
			break;

		default:
			break;
		}
	}

	switch (_game._trigger) {
	case 80:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(6, 81);
		break;

	case 81:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 82);
		break;

	case 82:
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
		_game._player._stepEnabled = true;
		break;

	default:
		break;
	}
}

static void handleThrowingBone() {
	local._animationActive = -1;
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_game._player._visible = false;
		_scene->loadAnimation(formAnimName('D', local._animationMode), 1);
		break;

	case 1:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;

		if (local._animationMode != 1)
			_scene->_hotspots.activate(words_obnoxious_dog, false);
		else {
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		}

		local._dogBarking = false;
		if (_game._objects.isInInventory(OBJ_BONE)) {
			_game._objects.setRoom(OBJ_BONE, 1);
			if (local._animationMode == 1)
				_globals[kBone202Status] = 0;
		} else {
			_game._objects.setRoom(OBJ_BONES, 1);
			_game._objects.addToInventory(OBJ_BONE);
		}

		_scene->_sequences.addTimer(60, 2);
		break;

	case 2:
	{
		int quoteId = 0x2F8;
		if (local._animationMode == 1)
			quoteId = 0x2F7;

		if (local._animationMode == 2) {
			_globals[kDogStatus] = DOG_LEFT;
			local._dogTimer = 0;
		}

		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(quoteId));
		_scene->_sequences.addTimer(60, 3);
	}
	break;

	case 3:
		_game._player._stepEnabled = true;
		local._animationActive = 0;
		break;

	default:
		break;
	}
}

static void room_607_pre_parser() {
	if (player_said_2(talkto, obnoxious_dog))
		_game._player._needToWalk = false;

	if (player_said_2(walk_through, side_entrance) && (_globals[kDogStatus] == DOG_LEFT) && (_game._difficulty != DIFFICULTY_EASY)) {
		local._shopAvailable = true;
		local._dogTimer = 0;
	}

	if (player_said_3(throw, bones, obnoxious_dog) || player_said_3(throw, bone, obnoxious_dog))
		_game._player.walk(Common::Point(193, 100), FACING_NORTHEAST);

	if (player_said_3(throw, bones, fence) || player_said_3(throw, bone, fence))
		_game._player.walk(Common::Point(201, 107), FACING_SOUTHEAST);
}

static void room_607_parser() {
	if (player_said_2(walk_through, side_entrance))
		_scene->_nextSceneId = 608;
	else if (player_said_2(get_inside, car)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 4);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
			_scene->_sequences.addTimer(6, 2);
		}
		break;

		case 2:
			_game._player._visible = false;
			_globals._sequenceIndexes[3] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[3], false, 10, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = _globals._sequenceIndexes[3];
			_globals._sequenceIndexes[3] = _scene->_sequences.startCycle(_globals._spriteIndexes[3], false, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[3], syncIdx);
			_scene->_nextSceneId = 504;
		}
		break;

		default:
			break;
		}
	} else if (player_said_3(throw, bones, obnoxious_dog) || player_said_3(throw, bone, obnoxious_dog)) {
		if (_game._difficulty != DIFFICULTY_EASY) {
			local._animationMode = 1;
			_scene->_kernelMessages.reset();
			if (_game._trigger == 0)
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F6));

			handleThrowingBone();
		}
	} else if ((player_said_3(throw, bones, fence) || player_said_3(throw, bone, fence)) && (_game._difficulty != DIFFICULTY_EASY)
		&& ((_globals[kDogStatus] == DOG_PRESENT) || _game._trigger)) {
		local._animationMode = 2;
		if (_game._trigger == 0) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F6));
		}
		handleThrowingBone();
	} else if (_action._lookFlag || player_said_2(look, street)) {
		if ((_globals[kDogStatus] == DOG_PRESENT) || (_game._difficulty == DIFFICULTY_EASY))
			_vm->_dialogs->show(60710);
		else
			_vm->_dialogs->show(60711);
	} else if (player_said_2(look, wall))
		_vm->_dialogs->show(60712);
	else if (player_said_2(look, fence))
		_vm->_dialogs->show(60713);
	else if (player_said_2(look, car))
		_vm->_dialogs->show(60714);
	else if (player_said_2(look, manhole))
		_vm->_dialogs->show(60715);
	else if (player_said_2(look, fire_hydrant) && (_globals[kDogStatus] == DOG_PRESENT))
		_vm->_dialogs->show(60716);
	else if (player_said_2(look, sign))
		_vm->_dialogs->show(60717);
	else if (player_said_2(look, broken_window))
		_vm->_dialogs->show(60718);
	else if (player_said_2(look, garage_door))
		_vm->_dialogs->show(60719);
	else if (player_said_2(look, sidewalk))
		_vm->_dialogs->show(60720);
	else if (player_said_2(look, air_hose))
		_vm->_dialogs->show(60721);
	else if (player_said_2(look, auto_shop)) {
		if (_globals[kDogStatus] == DOG_PRESENT)
			_vm->_dialogs->show(60723);
		else
			_vm->_dialogs->show(60722);
	} else if (player_said_2(look, side_entrance)) {
		if (_globals[kDogStatus] == DOG_PRESENT)
			_vm->_dialogs->show(60725);
		else
			_vm->_dialogs->show(60724);
	} else if (player_said_2(look, obnoxious_dog))
		_vm->_dialogs->show(60726);
	else if (player_said_2(talkto, obnoxious_dog))
		_vm->_dialogs->show(60727);
	else if (player_said_2(look, barricade))
		_vm->_dialogs->show(60728);
	else if (player_said_2(walk_down, street))
		_vm->_dialogs->show(60730);
	else if (player_said_1(garage_door) && (player_said_1(open) || player_said_1(push) || player_said_1(pull)))
		_vm->_dialogs->show(60731);
	else
		return;

	_action._inProgress = false;
}

void room_607_synchronize(Common::Serializer &s) {
	s.syncAsUint32LE(local._dogTimer);
	s.syncAsUint32LE(local._lastFrameTime);

	s.syncAsByte(local._dogLoop);
	s.syncAsByte(local._dogEatsRex);
	s.syncAsByte(local._dogBarking);
	s.syncAsByte(local._shopAvailable);

	s.syncAsSint16LE(local._animationMode);
	s.syncAsSint16LE(local._animationActive);
	s.syncAsSint16LE(local._counter);
}

void room_607_preload() {
	room_init_code_pointer = room_607_init;
	room_daemon_code_pointer = room_607_daemon;
	room_pre_parser_code_pointer = room_607_pre_parser;
	room_parser_code_pointer = room_607_parser;

	section_6_walker();
	section_6_interface();
	_scene->addActiveVocab(words_obnoxious_dog);
	_scene->addActiveVocab(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
