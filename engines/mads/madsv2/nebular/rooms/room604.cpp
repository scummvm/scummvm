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

Scene604::Scene604(RexNebularEngine *vm) : Scene6xx(vm) {
	_timebombHotspotId = -1;
	_bombMode = -1;
	_monsterFrame = -1;

	_monsterTimer = 0;

	_monsterActive = false;
	_animationActiveFl = false;
}

void room_604_synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsSint16LE(_timebombHotspotId);
	s.syncAsSint16LE(_bombMode);
	s.syncAsSint16LE(_monsterFrame);

	s.syncAsUint32LE(_monsterTimer);

	s.syncAsByte(_monsterActive);
	s.syncAsByte(_animationActiveFl);
}

void Scene604::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(NOUN_SEA_MONSTER);
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_TIMEBOMB);
}

static void room_604_init() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[4] = _scene->_sprites.addSprites("*RXCD_9");
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(Resources::formatName(620, 'b', 0, EXT_SS, ""));
	_globals._spriteIndexes[5] = _scene->_sprites.addSprites("*RXMRC_9");

	if (_globals[kTimebombStatus] == 1) {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, -1);
		_timebombHotspotId = _scene->_dynamicHotspots.add(NOUN_TIMEBOMB, VERB_WALKTO, _globals._sequenceIndexes[6], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_timebombHotspotId, Common::Point(166, 118), FACING_NORTHEAST);
	}

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_TIMEBOMB);

	_vm->_palette->setEntry(252, 63, 37, 26);
	_vm->_palette->setEntry(253, 45, 24, 17);
	_animationActiveFl = false;

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(72, 149);
		_game._player._facing = FACING_NORTHEAST;
		_game._player._visible = false;
		_game._player._stepEnabled = false;
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_scene->loadAnimation(formAnimName('R', 1), 70);
		_animationActiveFl = true;
	} else {
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
	}

	_monsterTimer = _scene->_frameStartTime;
	_monsterActive = false;

	sceneEntrySound();
	_game.loadQuoteSet(0x2E7, 0x2E8, 0x2E9, 0x2EA, 0x2EB, 0x2EC, 0x2ED, 0x2EE, 0x2EF, 0x2F0, 0);
}

void Scene604::step() {
	switch (_game._trigger) {
	case 70:
		_game._player._visible = true;
		_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
		_scene->_sequences.addTimer(30, 71);
		break;

	case 71:
		_scene->_sequences.remove(_globals._sequenceIndexes[2]);
		_globals._sequenceIndexes[2] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -2);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
		_game._player._stepEnabled = true;
		_animationActiveFl = false;
		break;

	default:
		break;
	}

	if (_monsterActive && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _monsterFrame) {
			_monsterFrame = _scene->_animation[0]->getCurrentFrame();
			int nextMonsterFrame = -1;

			switch (_monsterFrame) {
			case 50:
			case 137:
			case 174:
			{
				int randVal = _vm->getRandomNumber(1, 1000);
				if ((randVal <= 450) && (_game._player._special)) {
					if (_game._player._special == 1)
						nextMonsterFrame = 50;
					else if (_game._player._special == 2)
						nextMonsterFrame = 84;
					else
						nextMonsterFrame = 137;
				} else if (randVal <= 150)
					nextMonsterFrame = 50;
				else if (randVal <= 300)
					nextMonsterFrame = 84;
				else if (randVal <= 450)
					nextMonsterFrame = 137;
				else if (randVal < 750)
					nextMonsterFrame = 13;
				else
					nextMonsterFrame = 114;

			}
			break;

			case 84:
				nextMonsterFrame = 14;
				break;

			default:
				break;
			}

			if ((nextMonsterFrame >= 0) && (nextMonsterFrame != _monsterFrame)) {
				_scene->_animation[0]->setCurrentFrame(nextMonsterFrame);
				_monsterFrame = nextMonsterFrame;
			}
		}
	}

	if ((!_monsterActive && !_animationActiveFl) && (_scene->_frameStartTime > (_monsterTimer + 4))) {
		_monsterTimer = _scene->_frameStartTime;
		if ((_vm->getRandomNumber(1, 1000) < 25) || !_game._visitedScenes._sceneRevisited) {
			_monsterActive = true;
			_scene->freeAnimation();
			_scene->loadAnimation(formAnimName('m', -1));
		}
	}
}

void Scene604::handleBombActions() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		_game._player._visible = false;
		_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 9, 1, 0, 0);
		_scene->_sequences.setAnimRange(_globals._sequenceIndexes[5], 1, 3);
		_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[5]);
		if (_bombMode == 1)
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 3, 1);
		else
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 3, 2);

		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		break;

	case 1:
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, -1);
		_timebombHotspotId = _scene->_dynamicHotspots.add(NOUN_TIMEBOMB, VERB_WALKTO, _globals._sequenceIndexes[6], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(_timebombHotspotId, Common::Point(166, 118), FACING_NORTHEAST);
		_game._objects.setRoom(OBJ_TIMEBOMB, _scene->_currentSceneId);
		break;

	case 2:
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_scene->_dynamicHotspots.remove(_timebombHotspotId);
		_game._objects.addToInventory(OBJ_TIMEBOMB);
		break;

	case 3:
		_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[5]);
		_game._player._visible = true;
		_game._player._stepEnabled = true;
		if (_bombMode == 1) {
			_vm->_dialogs->show(60421);
			_globals[kTimebombStatus] = TIMEBOMB_ACTIVATED;
			_globals[kTimebombTimer] = 0;
		} else {
			_vm->_dialogs->show(60423);
			_globals[kTimebombStatus] = TIMEBOMB_DEACTIVATED;
			_globals[kTimebombTimer] = 0;
		}
		break;

	default:
		break;
	}
}

static void room_604_parser() {
	if (_action.isAction(VERB_GET_INSIDE, NOUN_CAR)) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_scene->_sequences.remove(_globals._sequenceIndexes[2]);
			_globals._sequenceIndexes[2] = _scene->_sequences.addReverseSpriteCycle(_globals._spriteIndexes[2], false, 6, 1, 0, 0);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = _globals._sequenceIndexes[2];
			_globals._sequenceIndexes[2] = _scene->_sequences.startCycle(_globals._spriteIndexes[2], false, -1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[2], 1);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[2], syncIdx);
			_scene->_sequences.addTimer(6, 2);
		}
		break;

		case 2:
			_game._player._visible = false;
			_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 10, 1, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = _globals._sequenceIndexes[4];
			_globals._sequenceIndexes[4] = _scene->_sequences.startCycle(_globals._spriteIndexes[4], false, -2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[4]);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[4], syncIdx);
			_scene->_nextSceneId = 504;
		}
		break;

		default:
			break;
		}
	} else if ((_action.isAction(VERB_PUT, NOUN_LEDGE) || _action.isAction(VERB_PUT, NOUN_VIEWPORT) || _action.isAction(VERB_THROW, NOUN_VIEWPORT))
		&& (_action.isObject(NOUN_BOMB) || _action.isObject(NOUN_BOMBS)))
		_vm->_dialogs->show(60420);
	else if (_action.isAction(VERB_PUT, NOUN_TIMEBOMB, NOUN_LEDGE) || _action.isAction(VERB_PUT, NOUN_TIMEBOMB, NOUN_VIEWPORT)) {
		_bombMode = 1;
		if ((_game._difficulty == DIFFICULTY_HARD) || _globals[kWarnedFloodCity])
			handleBombActions();
		else if (
			(_game._objects.isInInventory(OBJ_POLYCEMENT) && (_game._objects.isInInventory(OBJ_CHICKEN) || _game._objects.isInInventory(OBJ_CHICKEN_BOMB)))
			&& (_globals[kLineStatus] == LINE_TIED || (_game._difficulty == DIFFICULTY_EASY && !_globals[kBoatRaised]))
			)
			// The original can get in an impossible state at this point, if the player has
			// combined the chicken with the bomb before placing the timer bomb on the ledge.
			// Therefore, we also allow the player to place the bomb if the chicken bomb is
			// in the inventory.
			handleBombActions();
		else if (_game._difficulty == DIFFICULTY_EASY)
			_vm->_dialogs->show(60424);
		else {
			_vm->_dialogs->show(60425);
			_globals[kWarnedFloodCity] = true;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_TIMEBOMB)) {
		if (_game._trigger || !_game._objects.isInInventory(OBJ_TIMEBOMB)) {
			_bombMode = 2;
			handleBombActions();
		}
	} else if (_action._lookFlag)
		_vm->_dialogs->show(60411);
	else if (_action.isAction(VERB_LOOK, NOUN_VIEWPORT)) {
		if (_monsterActive) {
			_vm->_dialogs->show(60413);
		} else {
			_vm->_dialogs->show(60412);
		}
	} else if (_action.isAction(VERB_LOOK, NOUN_WALL))
		_vm->_dialogs->show(60414);
	else if (_action.isAction(VERB_LOOK, NOUN_VENT))
		_vm->_dialogs->show(60415);
	else if (_action.isAction(VERB_LOOK, NOUN_INDICATOR))
		_vm->_dialogs->show(60416);
	else if (_action.isAction(VERB_LOOK, NOUN_SCULPTURE))
		_vm->_dialogs->show(60417);
	else if (_action.isAction(VERB_LOOK, NOUN_CAR))
		_vm->_dialogs->show(60418);
	else if (_action.isAction(VERB_LOOK, NOUN_FOUNTAIN))
		_vm->_dialogs->show(60419);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
