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

#include "common/scummsys.h"
#include "math/utils.h"
#include "mads/madsv2/nebular/nebular.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

Scene607::Scene607(RexNebularEngine *vm) : Scene6xx(vm) {
	_dogTimer = 0;
	_lastFrameTime = 0;

	_dogLoop = false;
	_dogEatsRex = false;
	_dogBarking = false;
	_shopAvailable = false;

	_animationMode = -1;
	_animationActive = -1;
	_counter = -1;
}

void room_607_synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsUint32LE(_dogTimer);
	s.syncAsUint32LE(_lastFrameTime);

	s.syncAsByte(_dogLoop);
	s.syncAsByte(_dogEatsRex);
	s.syncAsByte(_dogBarking);
	s.syncAsByte(_shopAvailable);

	s.syncAsSint16LE(_animationMode);
	s.syncAsSint16LE(_animationActive);
	s.syncAsSint16LE(_counter);
}

void Scene607::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_OBNOXIOUS_DOG);
	_scene->addActiveVocab(VERB_WALKTO);
}

static void room_607_init() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXCD_3");

	if (!_game._visitedScenes._sceneRevisited && (_scene->_priorSceneId != 608))
		_globals[kDogStatus] = DOG_PRESENT;

	if ((_scene->_priorSceneId == 608) && (_globals[kDogStatus] < DOG_GONE))
		_globals[kDogStatus] = DOG_GONE;

	_animationActive = 0;

	if ((_globals[kDogStatus] == DOG_PRESENT) && (_game._difficulty != DIFFICULTY_EASY)) {
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('g', 3));
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('g', 7));
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('g', 0));

		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		_dogBarking = false;
		_dogLoop = false;
		_shopAvailable = false;
		_dogEatsRex = false;
		_dogTimer = 0;
	} else
		_scene->_hotspots.activate(NOUN_OBNOXIOUS_DOG, false);

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

	sceneEntrySound();

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_BONES);

	_vm->_palette->setEntry(252, 63, 44, 30);
	_vm->_palette->setEntry(253, 63, 20, 22);
	_game.loadQuoteSet(0x2F8, 0x2F7, 0x2F6, 0x2F9, 0x2FA, 0);
}

void Scene607::step() {
	if (_globals[kDogStatus] == DOG_LEFT) {
		int32 diff = _scene->_frameStartTime - _lastFrameTime;
		if ((diff >= 0) && (diff <= 4))
			_dogTimer += diff;
		else
			_dogTimer++;

		_lastFrameTime = _scene->_frameStartTime;
	}

	if ((_dogTimer >= 480) && !_dogLoop && !_shopAvailable && (_globals[kDogStatus] == DOG_LEFT) && !_game._player._special) {
		_vm->_sound->command(14);
		_dogLoop = true;
		_globals._sequenceIndexes[5] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[5], false, 10, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 91);
		_dogLoop = false;
		_dogTimer = 0;
	}

	if (_game._trigger == 91) {
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		_dogBarking = false;
		_globals[kDogStatus] = DOG_PRESENT;
		_scene->_hotspots.activate(NOUN_OBNOXIOUS_DOG, true);
	}

	if (!_dogEatsRex && (_game._difficulty != DIFFICULTY_EASY) && !_animationActive && (_globals[kDogStatus] == DOG_PRESENT)
		&& !_dogBarking && (_vm->getRandomNumber(1, 50) == 10)) {
		_dogBarking = true;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_globals._sequenceIndexes[1] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[1], false, 5, 8, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		_scene->_kernelMessages.reset();
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_SPRITE, 2, 100);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[1], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		_counter = 0;
	}

	if ((_game._trigger == 70) && !_dogEatsRex && (_globals[kDogStatus] == DOG_PRESENT) && !_animationActive) {
		int syncIdx = _globals._sequenceIndexes[1];
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		_scene->_sequences.updateTimeout(_globals._sequenceIndexes[1], syncIdx);
		_scene->_kernelMessages.reset();
		_dogBarking = false;
	}

	if (_game._trigger == 100) {
		_counter++;
		_vm->_sound->command(12);

		if ((_counter >= 1) && (_counter <= 4)) {
			Common::Point pos(0, 0);
			switch (_counter) {
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

	if (_game._player._moving && (_game._difficulty != DIFFICULTY_EASY) && !_shopAvailable && (_globals[kDogStatus] == DOG_PRESENT) && (_scene->_rails.getNext() > 0)) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(268, 72), FACING_NORTHEAST);
		_scene->_rails.resetNext();
	}

	if ((_game._player._special > 0) && (_game._difficulty != DIFFICULTY_EASY) && (_globals[kDogStatus] == DOG_PRESENT) && _game._player._stepEnabled)
		_game._player._stepEnabled = false;

	if ((_game._difficulty != DIFFICULTY_EASY) && (_globals[kDogStatus] == DOG_PRESENT) && (_game._player._playerPos == Common::Point(268, 72))
		&& (_game._trigger || !_dogEatsRex)) {
		_dogEatsRex = true;
		switch (_game._trigger) {
		case 91:
		case 0:
			_animationActive = 1;
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
			_animationActive = 2;
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_sequences.addTimer(60, 63);
		}
		break;

		case 63:
			_vm->_dialogs->show(60729);
			_animationActive = 0;
			_dogEatsRex = false;
			_scene->_reloadSceneFlag = true;
			_game._player._stepEnabled = true;
			break;

		case 64:
			if (_dogEatsRex && (_animationActive == 1)) {
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

void Scene607::handleThrowingBone() {
	_animationActive = -1;
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_scene->_sequences.remove(_globals._sequenceIndexes[1]);
		_game._player._visible = false;
		_scene->loadAnimation(formAnimName('D', _animationMode), 1);
		break;

	case 1:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;

		if (_animationMode != 1)
			_scene->_hotspots.activate(NOUN_OBNOXIOUS_DOG, false);
		else {
			_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
		}

		_dogBarking = false;
		if (_game._objects.isInInventory(OBJ_BONE)) {
			_game._objects.setRoom(OBJ_BONE, 1);
			if (_animationMode == 1)
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
		if (_animationMode == 1)
			quoteId = 0x2F7;

		if (_animationMode == 2) {
			_globals[kDogStatus] = DOG_LEFT;
			_dogTimer = 0;
		}

		_scene->_kernelMessages.reset();
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(quoteId));
		_scene->_sequences.addTimer(60, 3);
	}
	break;

	case 3:
		_game._player._stepEnabled = true;
		_animationActive = 0;
		break;

	default:
		break;
	}
}

static void room_607_pre_parser() {
	if (_action.isAction(VERB_TALKTO, NOUN_OBNOXIOUS_DOG))
		_game._player._needToWalk = false;

	if (_action.isAction(VERB_WALK_THROUGH, NOUN_SIDE_ENTRANCE) && (_globals[kDogStatus] == DOG_LEFT) && (_game._difficulty != DIFFICULTY_EASY)) {
		_shopAvailable = true;
		_dogTimer = 0;
	}

	if (_action.isAction(VERB_THROW, NOUN_BONES, NOUN_OBNOXIOUS_DOG) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_OBNOXIOUS_DOG))
		_game._player.walk(Common::Point(193, 100), FACING_NORTHEAST);

	if (_action.isAction(VERB_THROW, NOUN_BONES, NOUN_FENCE) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_FENCE))
		_game._player.walk(Common::Point(201, 107), FACING_SOUTHEAST);
}

static void room_607_parser() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_SIDE_ENTRANCE))
		_scene->_nextSceneId = 608;
	else if (_action.isAction(VERB_GET_INSIDE, NOUN_CAR)) {
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
	} else if (_action.isAction(VERB_THROW, NOUN_BONES, NOUN_OBNOXIOUS_DOG) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_OBNOXIOUS_DOG)) {
		if (_game._difficulty != DIFFICULTY_EASY) {
			_animationMode = 1;
			_scene->_kernelMessages.reset();
			if (_game._trigger == 0)
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F6));

			handleThrowingBone();
		}
	} else if ((_action.isAction(VERB_THROW, NOUN_BONES, NOUN_FENCE) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_FENCE)) && (_game._difficulty != DIFFICULTY_EASY)
		&& ((_globals[kDogStatus] == DOG_PRESENT) || _game._trigger)) {
		_animationMode = 2;
		if (_game._trigger == 0) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2F6));
		}
		handleThrowingBone();
	} else if (_action._lookFlag || _action.isAction(VERB_LOOK, NOUN_STREET)) {
		if ((_globals[kDogStatus] == DOG_PRESENT) || (_game._difficulty == DIFFICULTY_EASY))
			_vm->_dialogs->show(60710);
		else
			_vm->_dialogs->show(60711);
	} else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(60712);
	else if (_action.isAction(VERB_LOOK, NOUN_FENCE))
		_vm->_dialogs->show(60713);
	else if (_action.isAction(VERB_LOOK, NOUN_CAR))
		_vm->_dialogs->show(60714);
	else if (_action.isAction(VERB_LOOK, NOUN_MANHOLE))
		_vm->_dialogs->show(60715);
	else if (_action.isAction(VERB_LOOK, NOUN_FIRE_HYDRANT) && (_globals[kDogStatus] == DOG_PRESENT))
		_vm->_dialogs->show(60716);
	else if (_action.isAction(VERB_LOOK, NOUN_SIGN))
		_vm->_dialogs->show(60717);
	else if (_action.isAction(VERB_LOOK, NOUN_BROKEN_WINDOW))
		_vm->_dialogs->show(60718);
	else if (_action.isAction(VERB_LOOK, NOUN_GARAGE_DOOR))
		_vm->_dialogs->show(60719);
	else if (_action.isAction(VERB_LOOK, NOUN_SIDEWALK))
		_vm->_dialogs->show(60720);
	else if (_action.isAction(VERB_LOOK, NOUN_AIR_HOSE))
		_vm->_dialogs->show(60721);
	else if (_action.isAction(VERB_LOOK, NOUN_AUTO_SHOP)) {
		if (_globals[kDogStatus] == DOG_PRESENT)
			_vm->_dialogs->show(60723);
		else
			_vm->_dialogs->show(60722);
	} else if (_action.isAction(VERB_LOOK, NOUN_SIDE_ENTRANCE)) {
		if (_globals[kDogStatus] == DOG_PRESENT)
			_vm->_dialogs->show(60725);
		else
			_vm->_dialogs->show(60724);
	} else if (_action.isAction(VERB_LOOK, NOUN_OBNOXIOUS_DOG))
		_vm->_dialogs->show(60726);
	else if (_action.isAction(VERB_TALKTO, NOUN_OBNOXIOUS_DOG))
		_vm->_dialogs->show(60727);
	else if (_action.isAction(VERB_LOOK, NOUN_BARRICADE))
		_vm->_dialogs->show(60728);
	else if (_action.isAction(VERB_WALK_DOWN, NOUN_STREET))
		_vm->_dialogs->show(60730);
	else if (_action.isObject(NOUN_GARAGE_DOOR) && (_action.isAction(VERB_OPEN) || _action.isAction(VERB_PUSH) || _action.isAction(VERB_PULL)))
		_vm->_dialogs->show(60731);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
