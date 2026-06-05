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

Scene608::Scene608(RexNebularEngine *vm) : Scene6xx(vm) {
	_carMode = -1;
	_carFrame = -1;
	_carMoveMode = -1;
	_dogDeathMode = -1;
	_carHotspotId = -1;
	_barkCount = -1;
	_polycementHotspotId = -1;
	_animationMode = -1;
	_nextTrigger = -1;
	_throwMode = -1;

	_resetPositionsFl = false;
	_dogActiveFl = false;
	_dogBarkingFl = false;
	_dogFirstEncounter = false;
	_rexBeingEaten = false;
	_dogHitWindow = false;
	_checkFl = false;
	_dogSquashFl = false;
	_dogSafeFl = false;
	_buttonPressedonTimeFl = false;
	_dogUnderCar = false;
	_dogYelping = false;

	_dogWindowTimer = -1;
	_dogRunTimer = -1;

	_dogTimer1 = 0;
	_dogTimer2 = 0;
}

void Scene608::synchronize(Common::Serializer &s) {
	Scene6xx::synchronize(s);

	s.syncAsSint16LE(_carMode);
	s.syncAsSint16LE(_carFrame);
	s.syncAsSint16LE(_carMoveMode);
	s.syncAsSint16LE(_dogDeathMode);
	s.syncAsSint16LE(_carHotspotId);
	s.syncAsSint16LE(_barkCount);
	s.syncAsSint16LE(_polycementHotspotId);
	s.syncAsSint16LE(_animationMode);
	s.syncAsSint16LE(_nextTrigger);
	s.syncAsSint16LE(_throwMode);

	s.syncAsByte(_resetPositionsFl);
	s.syncAsByte(_dogActiveFl);
	s.syncAsByte(_dogBarkingFl);
	s.syncAsByte(_dogFirstEncounter);
	s.syncAsByte(_rexBeingEaten);
	s.syncAsByte(_dogHitWindow);
	s.syncAsByte(_checkFl);
	s.syncAsByte(_dogSquashFl);
	s.syncAsByte(_dogSafeFl);
	s.syncAsByte(_buttonPressedonTimeFl);
	s.syncAsByte(_dogUnderCar);
	s.syncAsByte(_dogYelping);

	s.syncAsSint32LE(_dogWindowTimer);
	s.syncAsSint32LE(_dogRunTimer);

	s.syncAsUint32LE(_dogTimer1);
	s.syncAsUint32LE(_dogTimer2);
}

void Scene608::setup() {
	setPlayerSpritesPrefix();
	setAAName();
	_scene->addActiveVocab(VERB_WALKTO);
	_scene->addActiveVocab(NOUN_POLYCEMENT);
	_scene->addActiveVocab(NOUN_CAR);
	_scene->addActiveVocab(NOUN_OBNOXIOUS_DOG);
}

void Scene608::resetDogVariables() {
	_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
	int idx = _scene->_dynamicHotspots.add(NOUN_OBNOXIOUS_DOG, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(194, 142), FACING_EAST);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 4);
	_dogBarkingFl = false;
	_dogFirstEncounter = false;
}

void Scene608::restoreAnimations() {
	_scene->freeAnimation();
	_carMode = 0;
	_game._player._stepEnabled = true;
	if (_throwMode == 6)
		_dogSquashFl = true;

	if (_globals[kCarStatus] == CAR_UP) {
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_scene->_sequences.remove(_globals._sequenceIndexes[7]);
		_scene->loadAnimation(formAnimName('A', -1));
	} else {
		_scene->_sequences.remove(_globals._sequenceIndexes[8]);
		_scene->_sequences.remove(_globals._sequenceIndexes[6]);
		_scene->_sequences.remove(_globals._sequenceIndexes[7]);
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(6);
	}
}

void Scene608::setCarAnimations() {
	_scene->freeAnimation();
	if (_globals[kCarStatus] == CAR_UP) {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[6], Common::Point(143, 98));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 6);
		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[7], Common::Point(141, 67));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
	} else {
		_globals._sequenceIndexes[6] = _scene->_sequences.startCycle(_globals._spriteIndexes[6], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[6], Common::Point(143, 128));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[6], 6);
		_globals._sequenceIndexes[7] = _scene->_sequences.startCycle(_globals._spriteIndexes[7], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[7], Common::Point(141, 97));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[7], 15);
		_globals._sequenceIndexes[8] = _scene->_sequences.startCycle(_globals._spriteIndexes[8], false, 1);
		_scene->_sequences.setPosition(_globals._sequenceIndexes[8], Common::Point(144, 126));
		_scene->_sequences.setDepth(_globals._sequenceIndexes[8], 5);
	}
}

void Scene608::handleThrowingBone() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		setCarAnimations();
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		_animationMode = -1;
		_game._player._visible = false;
		_carMode = _throwMode;
		if (_throwMode == 4)
			_scene->loadAnimation(formAnimName('X', 2), 1);
		else if (_throwMode == 5)
			_scene->loadAnimation(formAnimName('X', 1), 1);
		else
			_scene->loadAnimation(formAnimName('X', 3), 1);
		break;

	case 1:
		_nextTrigger = 1;
		_scene->_sequences.addTimer(1, 2);
		break;

	case 2:
		if (_nextTrigger != 2)
			_scene->_sequences.addTimer(1, 2);
		else {
			if (_game._objects.isInInventory(OBJ_BONE))
				_game._objects.setRoom(OBJ_BONE, 1);
			else {
				_game._objects.setRoom(OBJ_BONES, 1);
				_game._objects.addToInventory(OBJ_BONE);
			}
			_scene->_sequences.addTimer(60, 3);
		}
		break;

	case 3:
		if (_throwMode != 6) {
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x304));
			_scene->_sequences.addTimer(120, 4);
		} else
			restoreAnimations();
		break;

	case 4:
		restoreAnimations();
		break;

	default:
		break;
	}
}

void Scene608::enter() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRD_7");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXMRC_9");
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('c', 2));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('c', 1));

	if (_game._objects.isInRoom(OBJ_POLYCEMENT)) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('g', -1));
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		int idx = _scene->_dynamicHotspots.add(NOUN_POLYCEMENT, VERB_WALKTO, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		_polycementHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(249, 129), FACING_NORTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
	}

	if (_game._objects.isInRoom(OBJ_REARVIEW_MIRROR)) {
		_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('m', -1));
		_globals._sequenceIndexes[12] = _scene->_sequences.startCycle(_globals._spriteIndexes[12], false, 1);
		int idx = _scene->_dynamicHotspots.add(NOUN_REARVIEW_MIRROR, VERB_WALKTO, _globals._sequenceIndexes[12], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(71, 113), FACING_NORTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 15);
	}

	if (_game._difficulty == DIFFICULTY_HARD) {
		_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('g', 0));
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('g', 1));
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('g', 2));
		_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('h', 2));
		_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('h', 3));
		_rexBeingEaten = false;

		if (!_game._visitedScenes._sceneRevisited) {
			_globals[kDogStatus] = DOG_GONE;
			_dogActiveFl = true;
		} else
			_dogActiveFl = (_globals[kDogStatus] != DOG_DEAD);
	} else {
		_globals[kDogStatus] = DOG_DEAD;
		_dogActiveFl = false;
	}

	_dogSquashFl = false;
	_buttonPressedonTimeFl = false;
	_dogWindowTimer = 0;
	_dogRunTimer = 0;
	_dogHitWindow = false;
	_checkFl = false;
	_dogUnderCar = false;
	_dogYelping = false;


	if (!_game._visitedScenes._sceneRevisited)
		_globals[kCarStatus] = CAR_UP;

	_animationMode = 0;
	_carMoveMode = 0;
	_carFrame = -1;

	if (_globals[kCarStatus] == CAR_UP) {
		_carMode = 0;
		_dogDeathMode = 0;
		_resetPositionsFl = false;
		int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('A', -1));
	} else if (_globals[kCarStatus] == CAR_DOWN) {
		_carMode = 0;
		_dogDeathMode = 0;
		_resetPositionsFl = false;
		int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(100, 100, 100 + 82, 100 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(6);
	} else if (_globals[kCarStatus] == CAR_SQUASHES_DOG) {
		_carMode = 2;
		_dogDeathMode = 0;
		_resetPositionsFl = false;
		int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('C', -1));
	} else if (_globals[kCarStatus] == CAR_SQUASHES_DOG_AGAIN) {
		_carMode = 1;
		_dogDeathMode = 2;
		_resetPositionsFl = true;
		int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('B', -1));
	} else {
		_carMode = 3;
		_dogDeathMode = 2;
		_resetPositionsFl = true;
		int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(100, 100, 100 + 82, 100 + 25));
		_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('D', -1));
	}

	_vm->_palette->setEntry(252, 63, 44, 30);
	_vm->_palette->setEntry(253, 63, 20, 22);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(46, 132);
		_game._player._facing = FACING_EAST;
		if (_game._difficulty == DIFFICULTY_HARD) {
			if (!_game._visitedScenes._sceneRevisited)
				_dogFirstEncounter = true;
			else if (_dogActiveFl)
				resetDogVariables();
		}
	} else if ((_game._difficulty == DIFFICULTY_HARD) && !_dogFirstEncounter && _dogActiveFl) {
		if (!_dogUnderCar)
			resetDogVariables();
		else {
			_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 9, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 10, 11);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
		}
	}

	sceneEntrySound();

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_BONES);

	_game.loadQuoteSet(0x2FB, 0x2FC, 0x2FE, 0x2FD, 0x2FF, 0x300, 0x301, 0x302, 0x303, 0x304, 0);
}

void Scene608::step() {
	if (_dogFirstEncounter) {
		long diff = _scene->_frameStartTime - _dogTimer1;
		if ((diff >= 0) && (diff <= 1))
			_dogWindowTimer += diff;
		else
			_dogWindowTimer++;

		_dogTimer1 = _scene->_frameStartTime;
	}

	if (_dogActiveFl && (_dogWindowTimer >= 2) && !_dogHitWindow) {
		_dogHitWindow = true;
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 11, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
		_vm->_sound->command(14);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		_dogWindowTimer = 0;
	}

	if (_game._trigger == 70)
		resetDogVariables();

	if ((_game._difficulty == DIFFICULTY_HARD) && !_animationMode && _dogActiveFl && !_dogFirstEncounter && !_dogUnderCar) {
		if (!_dogBarkingFl) {
			if (_vm->getRandomNumber(1, 50) == 10) {
				_dogBarkingFl = true;
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 5, 8, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 4);
				int idx = _scene->_dynamicHotspots.add(NOUN_OBNOXIOUS_DOG, VERB_WALKTO, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(194, 142), FACING_EAST);
				_barkCount = 0;
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 2, 100);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 60);
			}
		} else if (_game._trigger == 60) {
			int syncIdx = _globals._sequenceIndexes[5];
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 6);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], syncIdx);
			_scene->_kernelMessages.reset();
			_dogBarkingFl = false;
		}
	}

	if ((_game._trigger == 100) && _dogBarkingFl) {
		_vm->_sound->command(12);
		_barkCount++;

		if ((_barkCount >= 1) && (_barkCount <= 4)) {
			Common::Point _barkPos(0, 0);
			switch (_barkCount) {
			case 1:
				_barkPos = Common::Point(197, 66);
				break;

			case 2:
				_barkPos = Common::Point(230, 76);
				break;

			case 3:
				_barkPos = Common::Point(197, 86);
				break;

			case 4:
				_barkPos = Common::Point(230, 97);
				break;

			default:
				break;
			}
			_scene->_kernelMessages.add(_barkPos, 0xFDFC, 0, 0, 120, _game.getQuote(0x2FB));
		}
	}

	if (_dogSquashFl && !_dogFirstEncounter && _dogUnderCar && _dogActiveFl) {
		long diff = _scene->_frameStartTime - _dogTimer2;
		if ((diff >= 0) && (diff <= 4))
			_dogRunTimer += diff;
		else
			_dogRunTimer++;

		_dogTimer2 = _scene->_frameStartTime;
	}

	if (_dogRunTimer >= 480 && !_checkFl && !_buttonPressedonTimeFl && !_dogFirstEncounter && _dogUnderCar && _dogActiveFl) {
		_checkFl = true;
		_dogSquashFl = false;
		_dogSafeFl = true;
		_checkFl = false;
		_dogRunTimer = 0;
	} else {
		_dogSafeFl = false;
		if (_game._player._moving && (_game._difficulty == DIFFICULTY_HARD) && _dogActiveFl && (_scene->_rails.getNext() > 0) && _dogUnderCar)
			_dogSafeFl = true;
	}

	if (_dogActiveFl && _dogSafeFl && !_buttonPressedonTimeFl) {
		_dogDeathMode = 0;
		_globals[kCarStatus] = CAR_UP;
		_carMode = 0;
		_scene->_sequences.remove(_globals._sequenceIndexes[10]);
		_dogUnderCar = false;
		_dogYelping = false;
		_scene->_kernelMessages.reset();
		_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 92);
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FF));
	}

	if (_game._trigger == 92) {
		resetDogVariables();
		_animationMode = 0;
	}

	if ((_carMode == 4) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();

			if (_carFrame == 10) {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			} else if (_carFrame == 56) {
				resetDogVariables();
				_animationMode = 0;
				_nextTrigger = 2;
			}
		}
	}

	if ((_carMode == 5) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();
			if (_carFrame == 10) {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			} else if (_carFrame == 52) {
				resetDogVariables();
				_animationMode = 0;
				_nextTrigger = 2;
			}
		}
	}

	if ((_carMode == 6) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();

			if (_carFrame == 11) {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			} else if (_carFrame == 41) {
				_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 9, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 10, 11);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
				_dogUnderCar = true;
				_nextTrigger = 2;
			}
		}
	}

	if (_dogUnderCar) {
		if (!_dogYelping) {
			if (_vm->getRandomNumber(1, 50) == 10) {
				_dogYelping = true;
				_barkCount = 0;
				_scene->_sequences.addTimer(12, 110);
				_scene->_sequences.addTimer(22, 111);
				_scene->_sequences.addTimer(120, 112);
			}
			_scene->_kernelMessages.reset();
		}
	} else
		_dogYelping = false;

	if (_game._trigger == 110) {
		_vm->_sound->command(12);
		_scene->_kernelMessages.add(Common::Point(150, 97), 0xFDFC, 0, 0, 60, _game.getQuote(0x303));
	}

	if (_game._trigger == 111) {
		_vm->_sound->command(12);
		_scene->_kernelMessages.add(Common::Point(183, 93), 0xFDFC, 0, 0, 60, _game.getQuote(0x303));
	}

	if (_game._trigger == 112)
		_dogYelping = false;

	if ((_carMode == 0) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			if ((_globals[kCarStatus] == CAR_UP) || (_globals[kCarStatus] == CAR_DOWN)) {
				switch (_carMoveMode) {
				case 0:
					if (_globals[kCarStatus] == CAR_UP)
						nextFrame = 0;
					else
						nextFrame = 6;
					break;

				case 1:
					if (_scene->_animation[0]->getCurrentFrame() >= 12) {
						nextFrame = 0;
						_carMoveMode = 0;
						_globals[kCarStatus] = CAR_UP;
					}
					break;

				case 2:
					if (_scene->_animation[0]->getCurrentFrame() >= 6) {
						nextFrame = 6;
						_carMoveMode = 0;
						_globals[kCarStatus] = CAR_DOWN;
					}
					break;

				default:
					break;
				}
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}

	if ((_carMode == 2) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			if (_carMoveMode == 0)
				nextFrame = 28;
			else if (_scene->_animation[0]->getCurrentFrame() >= 28) {
				nextFrame = 28;
				_carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}

	if ((_carMode == 3) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			if (_resetPositionsFl) {
				nextFrame = 0;
				_carMoveMode = 0;
			} else if (_carMoveMode == 0)
				nextFrame = 6;
			else if (_scene->_animation[0]->getCurrentFrame() >= 6) {
				nextFrame = 6;
				_carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}


	if ((_carMode == 1) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != _carFrame) {
			_carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			if (_resetPositionsFl) {
				nextFrame = 0;
				_carMoveMode = 0;
			} else if (_carMoveMode == 0)
				nextFrame = 6;
			else if (_scene->_animation[0]->getCurrentFrame() >= 6) {
				nextFrame = 6;
				_carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				_carFrame = nextFrame;
			}
		}
	}

	if (_game._player._moving && (_game._difficulty == DIFFICULTY_HARD) && _dogActiveFl && (_scene->_rails.getNext() > 0)) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(194, 142), FACING_EAST);
		_scene->_rails.resetNext();
		if (_dogUnderCar)
			_dogSafeFl = true;
	}

	if (_game._player._special > 0 && (_game._difficulty == DIFFICULTY_HARD) && _dogActiveFl && _game._player._stepEnabled)
		_game._player._stepEnabled = false;

	if ((_game._difficulty == DIFFICULTY_HARD) && _dogActiveFl && (_game._player._playerPos == Common::Point(194, 142))
		&& (_game._trigger || !_rexBeingEaten)) {
		_rexBeingEaten = true;
		switch (_game._trigger) {
		case 0:
			_scene->_sequences.remove(_globals._sequenceIndexes[5]);
			_animationMode = 1;
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 10, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 1, 2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 5);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 80);
			_scene->_sequences.addTimer(10, 85);
			break;

		case 80:
			_game._player._visible = false;
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 10, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 3, 5);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 5);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], -1);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FC));
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 81);
			break;

		case 81:
		{
			int syncIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[9], false, 5, 1, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 6, 38);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 5);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], syncIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 82);
		}
		break;

		case 82:
		{
			int syncIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[9], false, 15, 5, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[9], 39, 40);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 5);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], syncIdx);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[9], SEQUENCE_TRIGGER_EXPIRE, 0, 83);
		}
		break;

		case 83:
		{
			_animationMode = 2;
			int syncIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 5);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], syncIdx);
			_scene->_sequences.addTimer(60, 84);
		}
		break;

		case 84:
			_rexBeingEaten = false;
			_animationMode = 0;
			_scene->_reloadSceneFlag = true;
			_game._player._stepEnabled = true;
			break;

		case 85:
			if (_rexBeingEaten && (_animationMode == 1)) {
				_vm->_sound->command(12);
				_scene->_sequences.addTimer(10, 85);
			}
			break;

		default:
			break;
		}
	}
}

void Scene608::preActions() {
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;

	if ((_action.isAction(VERB_THROW, NOUN_BONE, NOUN_REAR_OF_GARAGE) || _action.isAction(VERB_THROW, NOUN_BONES, NOUN_REAR_OF_GARAGE)
		|| _action.isAction(VERB_THROW, NOUN_BONE, NOUN_FRONT_OF_GARAGE) || _action.isAction(VERB_THROW, NOUN_BONES, NOUN_FRONT_OF_GARAGE)
		|| _action.isAction(VERB_THROW, NOUN_BONES, NOUN_OBNOXIOUS_DOG) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_OBNOXIOUS_DOG)) && _dogActiveFl) {
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(56, 146), FACING_EAST);
	}

	if ((_action.isAction(VERB_THROW, NOUN_BONES, NOUN_AREA_BEHIND_CAR) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_AREA_BEHIND_CAR)
		|| _action.isAction(VERB_THROW, NOUN_BONES, NOUN_DANGER_ZONE) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_DANGER_ZONE)) && _dogActiveFl) {
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(75, 136), FACING_EAST);
	}

	if (_action.isAction(VERB_PUSH, NOUN_DOWN_BUTTON) && _dogUnderCar) {
		_buttonPressedonTimeFl = true;
		_dogDeathMode = 1;
	} else
		_buttonPressedonTimeFl = false;
}

void Scene608::actions() {
	if (_action.isAction(VERB_WALK_THROUGH, NOUN_DOORWAY))
		_scene->_nextSceneId = 607;
	else if (_action.isAction(VERB_PUSH, NOUN_DOWN_BUTTON)) {
		_game._player._stepEnabled = true;
		switch (_game._trigger) {
		case 0:
			if ((_globals[kCarStatus] == CAR_UP) || (_globals[kCarStatus] == CAR_SQUASHES_DOG) || (_globals[kCarStatus] == CAR_SQUASHES_DOG_AGAIN)) {
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], true, 6, 2, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 2);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			} else
				_vm->_dialogs->show(60839);
			break;

		case 1:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
			_game._player._visible = true;
			if (_dogDeathMode == 0)
				_carMode = 0;
			else if (_dogDeathMode == 1) {
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x300));
				_globals[kCarStatus] = CAR_SQUASHES_DOG;
				_carMode = 2;
				_globals[kDogStatus] = DOG_DEAD;
				_dogActiveFl = false;
				_dogUnderCar = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[10]);
				_scene->freeAnimation();
				_scene->loadAnimation(formAnimName('C', -1));
			} else {
				_resetPositionsFl = false;
				_carMode = 1;
				_scene->freeAnimation();
				_scene->loadAnimation(formAnimName('B', -1));
			}

			_carMoveMode = 2;
			_scene->_sequences.addTimer(1, 2);
			break;

		case 2:
			if (_carMoveMode != 0)
				_scene->_sequences.addTimer(1, 2);
			else {
				_scene->_dynamicHotspots.remove(_carHotspotId);
				int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(100, 100, 100 + 82, 100 + 25));
				_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
				if (_globals[kCarStatus] == CAR_SQUASHES_DOG)
					_scene->_sequences.addTimer(120, 3);
				else {
					if (_dogDeathMode == 0)
						_globals[kCarStatus] = CAR_DOWN;
					else {
						_globals[kCarStatus] = CAR_DOWN_ON_SQUASHED_DOG;
						_carMode = 3;
						_dogDeathMode = 2;
					}
					_game._player._stepEnabled = true;
				}
			}
			break;

		case 3:
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x302));
			_globals[kCarStatus] = CAR_DOWN_ON_SQUASHED_DOG;
			_carMode = 3;
			_dogDeathMode = 2;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_PUSH, NOUN_UP_BUTTON)) {
		switch (_game._trigger) {
		case 0:
			if ((_globals[kCarStatus] == CAR_DOWN) || (_globals[kCarStatus] == CAR_DOWN_ON_SQUASHED_DOG)) {
				_game._player._stepEnabled = false;
				_game._player._visible = false;
				_globals._sequenceIndexes[3] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[3], true, 6, 2, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[3], 1, 3);
				_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[3]);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
			} else
				_vm->_dialogs->show(60840);
			break;

		case 1:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[3]);
			_game._player._visible = true;
			if (_dogDeathMode == 0)
				_carMode = 0;
			else {
				_carMode = 3;
				_resetPositionsFl = false;
				_scene->freeAnimation();
				_scene->loadAnimation(formAnimName('D', -1));
			}
			_carMoveMode = 1;
			_scene->_sequences.addTimer(1, 2);
			break;

		case 2:
		{
			if (_carMoveMode != 0)
				_scene->_sequences.addTimer(1, 2);
			else if (_dogDeathMode == 0)
				_globals[kCarStatus] = CAR_UP;
			else if (_dogDeathMode == 2) {
				_globals[kCarStatus] = CAR_SQUASHES_DOG_AGAIN;
				_carMode = 3;
				_dogDeathMode = 2;
			}
			_scene->_dynamicHotspots.remove(_carHotspotId);
			int idx = _scene->_dynamicHotspots.add(NOUN_CAR, VERB_WALKTO, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
			_carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
			_game._player._stepEnabled = true;
		}
		break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_THROW, NOUN_BONE, NOUN_REAR_OF_GARAGE) || _action.isAction(VERB_THROW, NOUN_BONES, NOUN_REAR_OF_GARAGE)
		|| _action.isAction(VERB_THROW, NOUN_BONES, NOUN_OBNOXIOUS_DOG) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_OBNOXIOUS_DOG)) {
		_game._player._stepEnabled = true;
		if (_dogActiveFl) {
			if (_game._trigger == 0) {
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FE));
			}
			_throwMode = 4;
			handleThrowingBone();
		} else
			_vm->_dialogs->show(60841);
	} else if (_action.isAction(VERB_THROW, NOUN_BONE, NOUN_FRONT_OF_GARAGE) || _action.isAction(VERB_THROW, NOUN_BONES, NOUN_FRONT_OF_GARAGE)) {
		_game._player._stepEnabled = true;
		if (_dogActiveFl) {
			if (_game._trigger == 0) {
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FD));
			}
			_throwMode = 5;
			handleThrowingBone();
		} else
			_vm->_dialogs->show(60841);
	} else if (_action.isAction(VERB_THROW, NOUN_BONES, NOUN_AREA_BEHIND_CAR) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_AREA_BEHIND_CAR)
		|| _action.isAction(VERB_THROW, NOUN_BONES, NOUN_DANGER_ZONE) || _action.isAction(VERB_THROW, NOUN_BONE, NOUN_DANGER_ZONE)) {
		_game._player._stepEnabled = true;
		if ((_globals[kCarStatus] == CAR_UP) && _dogActiveFl) {
			if (_dogActiveFl) {
				if (_game._trigger == 0) {
					_scene->_kernelMessages.reset();
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x301));
				}
				_throwMode = 6;
				handleThrowingBone();
			} else
				_vm->_dialogs->show(60841);
		} else
			_vm->_dialogs->show(60842);
	} else if (_action.isAction(VERB_TAKE, NOUN_POLYCEMENT) && (_game._trigger || !_game._objects.isInInventory(OBJ_POLYCEMENT))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], true, 6, 2, 0, 0);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 4, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_vm->_sound->command(9);
			_scene->_sequences.remove(_globals._sequenceIndexes[1]);
			_scene->_dynamicHotspots.remove(_polycementHotspotId);
			break;

		case 2:
			_game._objects.addToInventory(OBJ_POLYCEMENT);
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			_vm->_dialogs->showItem(OBJ_POLYCEMENT, 60833);
			break;

		default:
			break;
		}
	} else if (_action.isAction(VERB_TAKE, NOUN_REARVIEW_MIRROR) && (_game._trigger || !_game._objects.isInInventory(OBJ_REARVIEW_MIRROR))) {
		switch (_game._trigger) {
		case 0:
			_game._player._stepEnabled = false;
			_game._player._visible = false;
			_globals._sequenceIndexes[2] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[2], true, 6, 2, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[2], 1, 2);
			_scene->_sequences.setMsgLayout(_globals._sequenceIndexes[2]);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_SPRITE, 2, 1);
			_scene->_sequences.addSubEntry(_globals._sequenceIndexes[2], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			_vm->_sound->command(9);
			_scene->_sequences.remove(_globals._sequenceIndexes[12]);
			_game._objects.addToInventory(OBJ_REARVIEW_MIRROR);
			_vm->_dialogs->showItem(OBJ_REARVIEW_MIRROR, 60827);
			break;

		case 2:
			_scene->_sequences.updateTimeout(-1, _globals._sequenceIndexes[2]);
			_game._player._visible = true;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (_action._lookFlag) {
		if (_game._difficulty != DIFFICULTY_HARD)
			_vm->_dialogs->show(60810);
		else if (_globals[kDogStatus] == DOG_DEAD)
			_vm->_dialogs->show(60812);
		else
			_vm->_dialogs->show(60811);
	} else if (_action.isAction(VERB_LOOK) && (_action.isObject(NOUN_MUFFLER) || _action.isObject(NOUN_CAR_SEAT) || _action.isObject(NOUN_HUBCAP)
		|| _action.isObject(NOUN_COILS) || _action.isObject(NOUN_QUARTER_PANEL)))
		_vm->_dialogs->show(60813);
	else if (_action.isAction(VERB_TAKE) && (_action.isObject(NOUN_MUFFLER) || _action.isObject(NOUN_CAR_SEAT) || _action.isObject(NOUN_HUBCAP)
		|| _action.isObject(NOUN_COILS) || _action.isObject(NOUN_QUARTER_PANEL)))
		_vm->_dialogs->show(60814);
	else if (_action.isAction(VERB_LOOK, NOUN_GARAGE_FLOOR) || _action.isAction(VERB_LOOK, NOUN_FRONT_OF_GARAGE) || _action.isAction(VERB_LOOK, NOUN_REAR_OF_GARAGE)) {
		if (_dogActiveFl)
			_vm->_dialogs->show(60815);
		else
			_vm->_dialogs->show(60816);
	} else if (_action.isAction(VERB_LOOK, NOUN_SPARE_RIBS))
		_vm->_dialogs->show(60817);
	else if (_action.isAction(VERB_TAKE, NOUN_SPARE_RIBS)) {
		if (_game._difficulty == DIFFICULTY_HARD)
			_vm->_dialogs->show(60818);
		else
			_vm->_dialogs->show(60819);
	} else if (_action.isAction(VERB_LOOK, NOUN_UP_BUTTON))
		_vm->_dialogs->show(60820);
	else if (_action.isAction(VERB_LOOK, NOUN_DOWN_BUTTON))
		_vm->_dialogs->show(60821);
	else if (_action.isAction(VERB_LOOK, NOUN_TRASH_CAN))
		_vm->_dialogs->show(60822);
	else if (_action.isAction(VERB_LOOK, NOUN_CALENDAR))
		_vm->_dialogs->show(60823);
	else if (_action.isAction(VERB_LOOK, NOUN_STORAGE_BOX)) {
		if (_game._objects[OBJ_REARVIEW_MIRROR]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60825);
		else
			_vm->_dialogs->show(60824);
	} else if (_action.isAction(VERB_OPEN, NOUN_STORAGE_BOX))
		_vm->_dialogs->show(60826);
	else if (_action.isAction(VERB_LOOK, NOUN_REARVIEW_MIRROR) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(60828);
	else if (_action.isAction(VERB_LOOK, NOUN_TOOL_BOX)) {
		if (_game._objects[OBJ_POLYCEMENT]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60829);
		else
			_vm->_dialogs->show(60830);
	} else if (_action.isAction(VERB_OPEN, NOUN_TOOL_BOX))
		_vm->_dialogs->show(60831);
	else if ((_action.isAction(VERB_LOOK, NOUN_POLYCEMENT)) && (_game._objects.isInRoom(OBJ_POLYCEMENT)))
		_vm->_dialogs->show(60832);
	else if (_action.isAction(VERB_LOOK, NOUN_GREASE_CAN) || _action.isAction(VERB_LOOK, NOUN_OIL_CAN))
		_vm->_dialogs->show(60834);
	else if (_action.isAction(VERB_LOOK, NOUN_CAR_LIFT))
		_vm->_dialogs->show(60835);
	else if (_action.isAction(VERB_LOOK, NOUN_CHAIR) || _action.isAction(VERB_LOOK, NOUN_HAT))
		_vm->_dialogs->show(60836);
	else if (_action.isAction(VERB_LOOK, NOUN_DANGER_ZONE))
		_vm->_dialogs->show(60838);
	else
		return;

	_action._inProgress = false;
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
