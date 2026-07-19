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
	int16 _carMode;
	int16 _carFrame;
	int16 _carMoveMode;
	int16 _dogDeathMode;
	int16 _carHotspotId;
	int16 _barkCount;
	int16 _polycementHotspotId;
	int16 _animationMode;
	int16 _nextTrigger;
	int16 _throwMode;

	bool _resetPositionsFl;
	bool _dogActiveFl;
	bool _dogBarkingFl;
	bool _dogFirstEncounter;
	bool _rexBeingEaten;
	bool _dogHitWindow;
	bool _checkFl;
	bool _dogSquashFl;
	bool _dogSafeFl;
	bool _buttonPressedonTimeFl;
	bool _dogUnderCar;
	bool _dogYelping;

	int32 _dogWindowTimer;
	int32 _dogRunTimer;
	int32 _dogTimer1;
	int32 _dogTimer2;
};

static Scratch local;


static void resetDogVariables() {
	_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
	int idx = _scene->_dynamicHotspots.add(words_obnoxious_dog, words_walkto, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
	_scene->_dynamicHotspots.setPosition(idx, Common::Point(194, 142), FACING_EAST);
	_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 4);
	local._dogBarkingFl = false;
	local._dogFirstEncounter = false;
}

static void restoreAnimations() {
	_scene->freeAnimation();
	local._carMode = 0;
	_game._player._stepEnabled = true;
	if (local._throwMode == 6)
		local._dogSquashFl = true;

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

static void setCarAnimations() {
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

static void handleThrowingBone() {
	switch (_game._trigger) {
	case 0:
		_game._player._stepEnabled = false;
		setCarAnimations();
		_scene->_sequences.remove(_globals._sequenceIndexes[5]);
		local._animationMode = -1;
		_game._player._visible = false;
		local._carMode = local._throwMode;
		if (local._throwMode == 4)
			_scene->loadAnimation(formAnimName('X', 2), 1);
		else if (local._throwMode == 5)
			_scene->loadAnimation(formAnimName('X', 1), 1);
		else
			_scene->loadAnimation(formAnimName('X', 3), 1);
		break;

	case 1:
		local._nextTrigger = 1;
		_scene->_sequences.addTimer(1, 2);
		break;

	case 2:
		if (local._nextTrigger != 2)
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
		if (local._throwMode != 6) {
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

static void room_608_init() {
	_globals._spriteIndexes[2] = _scene->_sprites.addSprites("*RXMRD_7");
	_globals._spriteIndexes[3] = _scene->_sprites.addSprites("*RXMRC_9");
	_globals._spriteIndexes[6] = _scene->_sprites.addSprites(formAnimName('c', 2));
	_globals._spriteIndexes[7] = _scene->_sprites.addSprites(formAnimName('c', 0));
	_globals._spriteIndexes[8] = _scene->_sprites.addSprites(formAnimName('c', 1));

	if (_game._objects.isInRoom(OBJ_POLYCEMENT)) {
		_globals._spriteIndexes[1] = _scene->_sprites.addSprites(formAnimName('g', -1));
		_globals._sequenceIndexes[1] = _scene->_sequences.startCycle(_globals._spriteIndexes[1], false, 1);
		int idx = _scene->_dynamicHotspots.add(words_polycement, words_walkto, _globals._sequenceIndexes[1], Common::Rect(0, 0, 0, 0));
		local._polycementHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(249, 129), FACING_NORTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[1], 6);
	}

	if (_game._objects.isInRoom(OBJ_REARVIEW_MIRROR)) {
		_globals._spriteIndexes[12] = _scene->_sprites.addSprites(formAnimName('m', -1));
		_globals._sequenceIndexes[12] = _scene->_sequences.startCycle(_globals._spriteIndexes[12], false, 1);
		int idx = _scene->_dynamicHotspots.add(words_rearview_mirror, words_walkto, _globals._sequenceIndexes[12], Common::Rect(0, 0, 0, 0));
		_scene->_dynamicHotspots.setPosition(idx, Common::Point(71, 113), FACING_NORTHEAST);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[12], 15);
	}

	if (_game._difficulty == DIFFICULTY_HARD) {
		_globals._spriteIndexes[9] = _scene->_sprites.addSprites(formAnimName('g', 0));
		_globals._spriteIndexes[4] = _scene->_sprites.addSprites(formAnimName('g', 1));
		_globals._spriteIndexes[5] = _scene->_sprites.addSprites(formAnimName('g', 2));
		_globals._spriteIndexes[10] = _scene->_sprites.addSprites(formAnimName('h', 2));
		_globals._spriteIndexes[11] = _scene->_sprites.addSprites(formAnimName('h', 3));
		local._rexBeingEaten = false;

		if (!_game._visitedScenes._sceneRevisited) {
			_globals[kDogStatus] = DOG_GONE;
			local._dogActiveFl = true;
		} else
			local._dogActiveFl = (_globals[kDogStatus] != DOG_DEAD);
	} else {
		_globals[kDogStatus] = DOG_DEAD;
		local._dogActiveFl = false;
	}

	local._dogSquashFl = false;
	local._buttonPressedonTimeFl = false;
	local._dogWindowTimer = 0;
	local._dogRunTimer = 0;
	local._dogHitWindow = false;
	local._checkFl = false;
	local._dogUnderCar = false;
	local._dogYelping = false;


	if (!_game._visitedScenes._sceneRevisited)
		_globals[kCarStatus] = CAR_UP;

	local._animationMode = 0;
	local._carMoveMode = 0;
	local._carFrame = -1;

	if (_globals[kCarStatus] == CAR_UP) {
		local._carMode = 0;
		local._dogDeathMode = 0;
		local._resetPositionsFl = false;
		int idx = _scene->_dynamicHotspots.add(words_car, words_walkto, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
		local._carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('A', -1));
	} else if (_globals[kCarStatus] == CAR_DOWN) {
		local._carMode = 0;
		local._dogDeathMode = 0;
		local._resetPositionsFl = false;
		int idx = _scene->_dynamicHotspots.add(words_car, words_walkto, -1, Common::Rect(100, 100, 100 + 82, 100 + 25));
		local._carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('A', -1));
		_scene->_animation[0]->setCurrentFrame(6);
	} else if (_globals[kCarStatus] == CAR_SQUASHES_DOG) {
		local._carMode = 2;
		local._dogDeathMode = 0;
		local._resetPositionsFl = false;
		int idx = _scene->_dynamicHotspots.add(words_car, words_walkto, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
		local._carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('C', -1));
	} else if (_globals[kCarStatus] == CAR_SQUASHES_DOG_AGAIN) {
		local._carMode = 1;
		local._dogDeathMode = 2;
		local._resetPositionsFl = true;
		int idx = _scene->_dynamicHotspots.add(words_car, words_walkto, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
		local._carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('B', -1));
	} else {
		local._carMode = 3;
		local._dogDeathMode = 2;
		local._resetPositionsFl = true;
		int idx = _scene->_dynamicHotspots.add(words_car, words_walkto, -1, Common::Rect(100, 100, 100 + 82, 100 + 25));
		local._carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
		_scene->loadAnimation(formAnimName('D', -1));
	}

	_vm->_palette->setEntry(252, 63, 44, 30);
	_vm->_palette->setEntry(253, 63, 20, 22);

	if (_scene->_priorSceneId != RETURNING_FROM_DIALOG) {
		_game._player._playerPos = Common::Point(46, 132);
		_game._player._facing = FACING_EAST;
		if (_game._difficulty == DIFFICULTY_HARD) {
			if (!_game._visitedScenes._sceneRevisited)
				local._dogFirstEncounter = true;
			else if (local._dogActiveFl)
				resetDogVariables();
		}
	} else if ((_game._difficulty == DIFFICULTY_HARD) && !local._dogFirstEncounter && local._dogActiveFl) {
		if (!local._dogUnderCar)
			resetDogVariables();
		else {
			_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 9, 0, 0, 0);
			_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 10, 11);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
		}
	}

	section_6_music();

	if (_scene->_roomChanged)
		_game._objects.addToInventory(OBJ_BONES);

	_game.loadQuoteSet(0x2FB, 0x2FC, 0x2FE, 0x2FD, 0x2FF, 0x300, 0x301, 0x302, 0x303, 0x304, 0);
}

static void room_608_daemon() {
	if (local._dogFirstEncounter) {
		long diff = _scene->_frameStartTime - local._dogTimer1;
		if ((diff >= 0) && (diff <= 1))
			local._dogWindowTimer += diff;
		else
			local._dogWindowTimer++;

		local._dogTimer1 = _scene->_frameStartTime;
	}

	if (local._dogActiveFl && (local._dogWindowTimer >= 2) && !local._dogHitWindow) {
		local._dogHitWindow = true;
		_globals._sequenceIndexes[4] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[4], false, 11, 1, 0, 0);
		_scene->_sequences.setDepth(_globals._sequenceIndexes[4], 1);
		_vm->_sound->command(14);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[4], SEQUENCE_TRIGGER_EXPIRE, 0, 70);
		local._dogWindowTimer = 0;
	}

	if (_game._trigger == 70)
		resetDogVariables();

	if ((_game._difficulty == DIFFICULTY_HARD) && !local._animationMode && local._dogActiveFl && !local._dogFirstEncounter && !local._dogUnderCar) {
		if (!local._dogBarkingFl) {
			if (_vm->getRandomNumber(1, 50) == 10) {
				local._dogBarkingFl = true;
				_scene->_sequences.remove(_globals._sequenceIndexes[5]);
				_globals._sequenceIndexes[5] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[5], false, 5, 8, 0, 0);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 4);
				int idx = _scene->_dynamicHotspots.add(words_obnoxious_dog, words_walkto, _globals._sequenceIndexes[5], Common::Rect(0, 0, 0, 0));
				_scene->_dynamicHotspots.setPosition(idx, Common::Point(194, 142), FACING_EAST);
				local._barkCount = 0;
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_SPRITE, 2, 100);
				_scene->_sequences.addSubEntry(_globals._sequenceIndexes[5], SEQUENCE_TRIGGER_EXPIRE, 0, 60);
			}
		} else if (_game._trigger == 60) {
			int syncIdx = _globals._sequenceIndexes[5];
			_globals._sequenceIndexes[5] = _scene->_sequences.startCycle(_globals._spriteIndexes[5], false, 1);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[5], 6);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[5], syncIdx);
			_scene->_kernelMessages.reset();
			local._dogBarkingFl = false;
		}
	}

	if ((_game._trigger == 100) && local._dogBarkingFl) {
		_vm->_sound->command(12);
		local._barkCount++;

		if ((local._barkCount >= 1) && (local._barkCount <= 4)) {
			Common::Point _barkPos(0, 0);
			switch (local._barkCount) {
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

	if (local._dogSquashFl && !local._dogFirstEncounter && local._dogUnderCar && local._dogActiveFl) {
		long diff = _scene->_frameStartTime - local._dogTimer2;
		if ((diff >= 0) && (diff <= 4))
			local._dogRunTimer += diff;
		else
			local._dogRunTimer++;

		local._dogTimer2 = _scene->_frameStartTime;
	}

	if (local._dogRunTimer >= 480 && !local._checkFl && !local._buttonPressedonTimeFl && !local._dogFirstEncounter && local._dogUnderCar && local._dogActiveFl) {
		local._checkFl = true;
		local._dogSquashFl = false;
		local._dogSafeFl = true;
		local._checkFl = false;
		local._dogRunTimer = 0;
	} else {
		local._dogSafeFl = false;
		if (_game._player._moving && (_game._difficulty == DIFFICULTY_HARD) && local._dogActiveFl && (_scene->_rails.getNext() > 0) && local._dogUnderCar)
			local._dogSafeFl = true;
	}

	if (local._dogActiveFl && local._dogSafeFl && !local._buttonPressedonTimeFl) {
		local._dogDeathMode = 0;
		_globals[kCarStatus] = CAR_UP;
		local._carMode = 0;
		_scene->_sequences.remove(_globals._sequenceIndexes[10]);
		local._dogUnderCar = false;
		local._dogYelping = false;
		_scene->_kernelMessages.reset();
		_globals._sequenceIndexes[11] = _scene->_sequences.addSpriteCycle(_globals._spriteIndexes[11], false, 6, 1, 0, 0);
		_scene->_sequences.addSubEntry(_globals._sequenceIndexes[11], SEQUENCE_TRIGGER_EXPIRE, 0, 92);
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FF));
	}

	if (_game._trigger == 92) {
		resetDogVariables();
		local._animationMode = 0;
	}

	if ((local._carMode == 4) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != local._carFrame) {
			local._carFrame = _scene->_animation[0]->getCurrentFrame();

			if (local._carFrame == 10) {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			} else if (local._carFrame == 56) {
				resetDogVariables();
				local._animationMode = 0;
				local._nextTrigger = 2;
			}
		}
	}

	if ((local._carMode == 5) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != local._carFrame) {
			local._carFrame = _scene->_animation[0]->getCurrentFrame();
			if (local._carFrame == 10) {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			} else if (local._carFrame == 52) {
				resetDogVariables();
				local._animationMode = 0;
				local._nextTrigger = 2;
			}
		}
	}

	if ((local._carMode == 6) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != local._carFrame) {
			local._carFrame = _scene->_animation[0]->getCurrentFrame();

			if (local._carFrame == 11) {
				_game._player._visible = true;
				_game._player._priorTimer = _scene->_animation[0]->getNextFrameTimer() - _game._player._ticksAmount;
			} else if (local._carFrame == 41) {
				_globals._sequenceIndexes[10] = _scene->_sequences.startPingPongCycle(_globals._spriteIndexes[10], false, 9, 0, 0, 0);
				_scene->_sequences.setAnimRange(_globals._sequenceIndexes[10], 10, 11);
				_scene->_sequences.setDepth(_globals._sequenceIndexes[10], 6);
				local._dogUnderCar = true;
				local._nextTrigger = 2;
			}
		}
	}

	if (local._dogUnderCar) {
		if (!local._dogYelping) {
			if (_vm->getRandomNumber(1, 50) == 10) {
				local._dogYelping = true;
				local._barkCount = 0;
				_scene->_sequences.addTimer(12, 110);
				_scene->_sequences.addTimer(22, 111);
				_scene->_sequences.addTimer(120, 112);
			}
			_scene->_kernelMessages.reset();
		}
	} else
		local._dogYelping = false;

	if (_game._trigger == 110) {
		_vm->_sound->command(12);
		_scene->_kernelMessages.add(Common::Point(150, 97), 0xFDFC, 0, 0, 60, _game.getQuote(0x303));
	}

	if (_game._trigger == 111) {
		_vm->_sound->command(12);
		_scene->_kernelMessages.add(Common::Point(183, 93), 0xFDFC, 0, 0, 60, _game.getQuote(0x303));
	}

	if (_game._trigger == 112)
		local._dogYelping = false;

	if ((local._carMode == 0) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != local._carFrame) {
			local._carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			if ((_globals[kCarStatus] == CAR_UP) || (_globals[kCarStatus] == CAR_DOWN)) {
				switch (local._carMoveMode) {
				case 0:
					if (_globals[kCarStatus] == CAR_UP)
						nextFrame = 0;
					else
						nextFrame = 6;
					break;

				case 1:
					if (_scene->_animation[0]->getCurrentFrame() >= 12) {
						nextFrame = 0;
						local._carMoveMode = 0;
						_globals[kCarStatus] = CAR_UP;
					}
					break;

				case 2:
					if (_scene->_animation[0]->getCurrentFrame() >= 6) {
						nextFrame = 6;
						local._carMoveMode = 0;
						_globals[kCarStatus] = CAR_DOWN;
					}
					break;

				default:
					break;
				}
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				local._carFrame = nextFrame;
			}
		}
	}

	if ((local._carMode == 2) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != local._carFrame) {
			local._carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			if (local._carMoveMode == 0)
				nextFrame = 28;
			else if (_scene->_animation[0]->getCurrentFrame() >= 28) {
				nextFrame = 28;
				local._carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				local._carFrame = nextFrame;
			}
		}
	}

	if ((local._carMode == 3) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != local._carFrame) {
			local._carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			if (local._resetPositionsFl) {
				nextFrame = 0;
				local._carMoveMode = 0;
			} else if (local._carMoveMode == 0)
				nextFrame = 6;
			else if (_scene->_animation[0]->getCurrentFrame() >= 6) {
				nextFrame = 6;
				local._carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				local._carFrame = nextFrame;
			}
		}
	}


	if ((local._carMode == 1) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() != local._carFrame) {
			local._carFrame = _scene->_animation[0]->getCurrentFrame();
			int nextFrame = -1;

			if (local._resetPositionsFl) {
				nextFrame = 0;
				local._carMoveMode = 0;
			} else if (local._carMoveMode == 0)
				nextFrame = 6;
			else if (_scene->_animation[0]->getCurrentFrame() >= 6) {
				nextFrame = 6;
				local._carMoveMode = 0;
			}

			if ((nextFrame >= 0) && (nextFrame != _scene->_animation[0]->getCurrentFrame())) {
				_scene->_animation[0]->setCurrentFrame(nextFrame);
				local._carFrame = nextFrame;
			}
		}
	}

	if (_game._player._moving && (_game._difficulty == DIFFICULTY_HARD) && local._dogActiveFl && (_scene->_rails.getNext() > 0)) {
		_game._player.cancelCommand();
		_game._player.startWalking(Common::Point(194, 142), FACING_EAST);
		_scene->_rails.resetNext();
		if (local._dogUnderCar)
			local._dogSafeFl = true;
	}

	if (_game._player._special > 0 && (_game._difficulty == DIFFICULTY_HARD) && local._dogActiveFl && _game._player._stepEnabled)
		_game._player._stepEnabled = false;

	if ((_game._difficulty == DIFFICULTY_HARD) && local._dogActiveFl && (_game._player._playerPos == Common::Point(194, 142))
		&& (_game._trigger || !local._rexBeingEaten)) {
		local._rexBeingEaten = true;
		switch (_game._trigger) {
		case 0:
			_scene->_sequences.remove(_globals._sequenceIndexes[5]);
			local._animationMode = 1;
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
			local._animationMode = 2;
			int syncIdx = _globals._sequenceIndexes[9];
			_globals._sequenceIndexes[9] = _scene->_sequences.startCycle(_globals._spriteIndexes[9], false, -2);
			_scene->_sequences.setDepth(_globals._sequenceIndexes[9], 5);
			_scene->_sequences.updateTimeout(_globals._sequenceIndexes[9], syncIdx);
			_scene->_sequences.addTimer(60, 84);
		}
		break;

		case 84:
			local._rexBeingEaten = false;
			local._animationMode = 0;
			_scene->_reloadSceneFlag = true;
			_game._player._stepEnabled = true;
			break;

		case 85:
			if (local._rexBeingEaten && (local._animationMode == 1)) {
				_vm->_sound->command(12);
				_scene->_sequences.addTimer(10, 85);
			}
			break;

		default:
			break;
		}
	}
}

static void room_608_pre_parser() {
	_game._triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;

	if ((player_said_3(throw, bone, rear_of_garage) || player_said_3(throw, bones, rear_of_garage)
		|| player_said_3(throw, bone, front_of_garage) || player_said_3(throw, bones, front_of_garage)
		|| player_said_3(throw, bones, obnoxious_dog) || player_said_3(throw, bone, obnoxious_dog)) && local._dogActiveFl) {
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(56, 146), FACING_EAST);
	}

	if ((player_said_3(throw, bones, area_behind_car) || player_said_3(throw, bone, area_behind_car)
		|| player_said_3(throw, bones, danger_zone) || player_said_3(throw, bone, danger_zone)) && local._dogActiveFl) {
		_game._player._stepEnabled = false;
		_game._player.walk(Common::Point(75, 136), FACING_EAST);
	}

	if (player_said_2(push, down_button) && local._dogUnderCar) {
		local._buttonPressedonTimeFl = true;
		local._dogDeathMode = 1;
	} else
		local._buttonPressedonTimeFl = false;
}

static void room_608_parser() {
	if (player_said_2(walk_through, doorway))
		_scene->_nextSceneId = 607;
	else if (player_said_2(push, down_button)) {
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
			if (local._dogDeathMode == 0)
				local._carMode = 0;
			else if (local._dogDeathMode == 1) {
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x300));
				_globals[kCarStatus] = CAR_SQUASHES_DOG;
				local._carMode = 2;
				_globals[kDogStatus] = DOG_DEAD;
				local._dogActiveFl = false;
				local._dogUnderCar = false;
				_scene->_sequences.remove(_globals._sequenceIndexes[10]);
				_scene->freeAnimation();
				_scene->loadAnimation(formAnimName('C', -1));
			} else {
				local._resetPositionsFl = false;
				local._carMode = 1;
				_scene->freeAnimation();
				_scene->loadAnimation(formAnimName('B', -1));
			}

			local._carMoveMode = 2;
			_scene->_sequences.addTimer(1, 2);
			break;

		case 2:
			if (local._carMoveMode != 0)
				_scene->_sequences.addTimer(1, 2);
			else {
				_scene->_dynamicHotspots.remove(local._carHotspotId);
				int idx = _scene->_dynamicHotspots.add(words_car, words_walkto, -1, Common::Rect(100, 100, 100 + 82, 100 + 25));
				local._carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
				if (_globals[kCarStatus] == CAR_SQUASHES_DOG)
					_scene->_sequences.addTimer(120, 3);
				else {
					if (local._dogDeathMode == 0)
						_globals[kCarStatus] = CAR_DOWN;
					else {
						_globals[kCarStatus] = CAR_DOWN_ON_SQUASHED_DOG;
						local._carMode = 3;
						local._dogDeathMode = 2;
					}
					_game._player._stepEnabled = true;
				}
			}
			break;

		case 3:
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x302));
			_globals[kCarStatus] = CAR_DOWN_ON_SQUASHED_DOG;
			local._carMode = 3;
			local._dogDeathMode = 2;
			_game._player._stepEnabled = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(push, up_button)) {
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
			if (local._dogDeathMode == 0)
				local._carMode = 0;
			else {
				local._carMode = 3;
				local._resetPositionsFl = false;
				_scene->freeAnimation();
				_scene->loadAnimation(formAnimName('D', -1));
			}
			local._carMoveMode = 1;
			_scene->_sequences.addTimer(1, 2);
			break;

		case 2:
		{
			if (local._carMoveMode != 0)
				_scene->_sequences.addTimer(1, 2);
			else if (local._dogDeathMode == 0)
				_globals[kCarStatus] = CAR_UP;
			else if (local._dogDeathMode == 2) {
				_globals[kCarStatus] = CAR_SQUASHES_DOG_AGAIN;
				local._carMode = 3;
				local._dogDeathMode = 2;
			}
			_scene->_dynamicHotspots.remove(local._carHotspotId);
			int idx = _scene->_dynamicHotspots.add(words_car, words_walkto, -1, Common::Rect(99, 69, 99 + 82, 69 + 25));
			local._carHotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(96, 132), FACING_NORTHEAST);
			_game._player._stepEnabled = true;
		}
		break;

		default:
			break;
		}
	} else if (player_said_3(throw, bone, rear_of_garage) || player_said_3(throw, bones, rear_of_garage)
		|| player_said_3(throw, bones, obnoxious_dog) || player_said_3(throw, bone, obnoxious_dog)) {
		_game._player._stepEnabled = true;
		if (local._dogActiveFl) {
			if (_game._trigger == 0) {
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FE));
			}
			local._throwMode = 4;
			handleThrowingBone();
		} else
			_vm->_dialogs->show(60841);
	} else if (player_said_3(throw, bone, front_of_garage) || player_said_3(throw, bones, front_of_garage)) {
		_game._player._stepEnabled = true;
		if (local._dogActiveFl) {
			if (_game._trigger == 0) {
				_scene->_kernelMessages.reset();
				_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x2FD));
			}
			local._throwMode = 5;
			handleThrowingBone();
		} else
			_vm->_dialogs->show(60841);
	} else if (player_said_3(throw, bones, area_behind_car) || player_said_3(throw, bone, area_behind_car)
		|| player_said_3(throw, bones, danger_zone) || player_said_3(throw, bone, danger_zone)) {
		_game._player._stepEnabled = true;
		if ((_globals[kCarStatus] == CAR_UP) && local._dogActiveFl) {
			if (local._dogActiveFl) {
				if (_game._trigger == 0) {
					_scene->_kernelMessages.reset();
					_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, _game.getQuote(0x301));
				}
				local._throwMode = 6;
				handleThrowingBone();
			} else
				_vm->_dialogs->show(60841);
		} else
			_vm->_dialogs->show(60842);
	} else if (player_said_2(take, polycement) && (_game._trigger || !_game._objects.isInInventory(OBJ_POLYCEMENT))) {
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
			_scene->_dynamicHotspots.remove(local._polycementHotspotId);
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
	} else if (player_said_2(take, rearview_mirror) && (_game._trigger || !_game._objects.isInInventory(OBJ_REARVIEW_MIRROR))) {
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
	} else if (player_said_1(look) && (player_said_1(muffler) || player_said_1(car_seat) || player_said_1(hubcap)
		|| player_said_1(coils) || player_said_1(quarter_panel)))
		_vm->_dialogs->show(60813);
	else if (player_said_1(take) && (player_said_1(muffler) || player_said_1(car_seat) || player_said_1(hubcap)
		|| player_said_1(coils) || player_said_1(quarter_panel)))
		_vm->_dialogs->show(60814);
	else if (player_said_2(look, garage_floor) || player_said_2(look, front_of_garage) || player_said_2(look, rear_of_garage)) {
		if (local._dogActiveFl)
			_vm->_dialogs->show(60815);
		else
			_vm->_dialogs->show(60816);
	} else if (player_said_2(look, spare_ribs))
		_vm->_dialogs->show(60817);
	else if (player_said_2(take, spare_ribs)) {
		if (_game._difficulty == DIFFICULTY_HARD)
			_vm->_dialogs->show(60818);
		else
			_vm->_dialogs->show(60819);
	} else if (player_said_2(look, up_button))
		_vm->_dialogs->show(60820);
	else if (player_said_2(look, down_button))
		_vm->_dialogs->show(60821);
	else if (player_said_2(look, trash_can))
		_vm->_dialogs->show(60822);
	else if (player_said_2(look, calendar))
		_vm->_dialogs->show(60823);
	else if (player_said_2(look, storage_box)) {
		if (_game._objects[OBJ_REARVIEW_MIRROR]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60825);
		else
			_vm->_dialogs->show(60824);
	} else if (player_said_2(open, storage_box))
		_vm->_dialogs->show(60826);
	else if (player_said_2(look, rearview_mirror) && (_action._mainObjectSource == CAT_HOTSPOT))
		_vm->_dialogs->show(60828);
	else if (player_said_2(look, tool_box)) {
		if (_game._objects[OBJ_POLYCEMENT]._roomNumber == _scene->_currentSceneId)
			_vm->_dialogs->show(60829);
		else
			_vm->_dialogs->show(60830);
	} else if (player_said_2(open, tool_box))
		_vm->_dialogs->show(60831);
	else if ((player_said_2(look, polycement)) && (_game._objects.isInRoom(OBJ_POLYCEMENT)))
		_vm->_dialogs->show(60832);
	else if (player_said_2(look, grease_can) || player_said_2(look, oil_can))
		_vm->_dialogs->show(60834);
	else if (player_said_2(look, car_lift))
		_vm->_dialogs->show(60835);
	else if (player_said_2(look, chair) || player_said_2(look, hat))
		_vm->_dialogs->show(60836);
	else if (player_said_2(look, danger_zone))
		_vm->_dialogs->show(60838);
	else
		return;

	_action._inProgress = false;
}

void room_608_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._carMode);
	s.syncAsSint16LE(local._carFrame);
	s.syncAsSint16LE(local._carMoveMode);
	s.syncAsSint16LE(local._dogDeathMode);
	s.syncAsSint16LE(local._carHotspotId);
	s.syncAsSint16LE(local._barkCount);
	s.syncAsSint16LE(local._polycementHotspotId);
	s.syncAsSint16LE(local._animationMode);
	s.syncAsSint16LE(local._nextTrigger);
	s.syncAsSint16LE(local._throwMode);

	s.syncAsByte(local._resetPositionsFl);
	s.syncAsByte(local._dogActiveFl);
	s.syncAsByte(local._dogBarkingFl);
	s.syncAsByte(local._dogFirstEncounter);
	s.syncAsByte(local._rexBeingEaten);
	s.syncAsByte(local._dogHitWindow);
	s.syncAsByte(local._checkFl);
	s.syncAsByte(local._dogSquashFl);
	s.syncAsByte(local._dogSafeFl);
	s.syncAsByte(local._buttonPressedonTimeFl);
	s.syncAsByte(local._dogUnderCar);
	s.syncAsByte(local._dogYelping);

	s.syncAsSint32LE(local._dogWindowTimer);
	s.syncAsSint32LE(local._dogRunTimer);

	s.syncAsUint32LE(local._dogTimer1);
	s.syncAsUint32LE(local._dogTimer2);
}

void room_608_preload() {
	room_init_code_pointer = room_608_init;
	room_daemon_code_pointer = room_608_daemon;
	room_pre_parser_code_pointer = room_608_pre_parser;
	room_parser_code_pointer = room_608_parser;

	section_6_walker();
	section_6_interface();
	_scene->addActiveVocab(words_walkto);
	_scene->addActiveVocab(words_polycement);
	_scene->addActiveVocab(words_car);
	_scene->addActiveVocab(words_obnoxious_dog);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
