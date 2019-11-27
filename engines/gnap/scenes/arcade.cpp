/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"
#include "gnap/scenes/arcade.h"

namespace Gnap {

static const ObstacleDef kObstacleDefs[] = {
	{0xB4, 15}, {0xCB, 14}, {0xCD, 13}, {0xCF, 15}, {0xBA, 14},
	{0xCD, 13}, {0xCF, 12}, {0xCB, 15}, {0xBD, 13}, {0xCF, 12},
	{0xCD, 11}, {0xCB, 15}, {0xB7, 12}, {0xCD, 11}, {0xCB, 10},
	{0xCF, 15}, {0xCF, 14}, {0xBD, 13}, {0xCF, 12}, {0xCD, 11},
	{0xCB, 15}, {0xCB, 13}, {0xB4, 12}, {0xCB, 11}, {0xCD, 10},
	{0xCF, 15}, {0xCD, 12}, {0xBA, 12}, {0xCD, 12}, {0xCF, 12},
	{0xCB, 15}, {0xCB,  9}, {0xCD,  9}, {0xCF,  9}, {0xCD,  9},
	{0xCB,  9}, {0xCD,  9}, {0xCF,  5}, {0xBD, 13}, {0xCF,  8},
	{0xCB,  8}, {0xCD, 15}, {0xB4,  1}, {0xBD,  7}, {0xCF,  7},
	{0xCD,  7}, {0xCB,  7}, {0xCD,  7}, {0xCF, 15}, {0xCF, 15}
};

Scene49::Scene49(GnapEngine *vm) : Scene(vm) {
	_scoreBarFlash = false;
	_scoreBarPos = -1;
	_scoreLevel = -1;
	_obstacleIndex = -1;
	_truckSequenceId = -1;
	_truckId = -1;
	_truckLaneNum = -1;

	for (int i = 0; i < 5; i++) {
		_obstacles[i]._currSequenceId = -1;
		_obstacles[i]._closerSequenceId = -1;
		_obstacles[i]._passedSequenceId = -1;
		_obstacles[i]._splashSequenceId = -1;
		_obstacles[i]._collisionSequenceId = -1;
		_obstacles[i]._prevId = -1;
		_obstacles[i]._currId = -1;
		_obstacles[i]._laneNum = -1;
	}
}

int Scene49::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	for (int i = 0; i < 5; ++i)
		gameSys.setAnimation(0, 0, i + 2);
	_vm->_timers[2] = 0;
	_vm->_timers[0] = 0;
	_vm->_timers[1] = 0;
	_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
	_vm->clearKeyStatus1(Common::KEYCODE_RIGHT);
	_vm->clearKeyStatus1(Common::KEYCODE_LEFT);
	return 0xD5;
}

void Scene49::updateHotspots() {
	_vm->_hotspotsCount = 0;
}

void Scene49::checkObstacles() {
	if (_vm->_timers[2] == 0) {
		if (_vm->_timers[3] == 0) {
			for (int i = 0; i < 5; ++i)
				clearObstacle(i);
		}

		for (int j = 0; j < 5; ++j) {
			if (_obstacles[j]._currSequenceId == 0) {
				_vm->_timers[3] = 35;
				_obstacles[j]._currSequenceId = kObstacleDefs[_obstacleIndex]._sequenceId;
				switch (_obstacles[j]._currSequenceId) {
				case 0xB4:
					_obstacles[j]._laneNum = 1;
					_obstacles[j]._closerSequenceId = 180;
					_obstacles[j]._passedSequenceId = 181;
					_obstacles[j]._splashSequenceId = 182;
					_obstacles[j]._collisionSequenceId = 192;
					break;
				case 0xB7:
					_obstacles[j]._laneNum = 2;
					_obstacles[j]._closerSequenceId = 183;
					_obstacles[j]._passedSequenceId = 184;
					_obstacles[j]._splashSequenceId = 185;
					_obstacles[j]._collisionSequenceId = 193;
					break;
				case 0xBD:
					_obstacles[j]._laneNum = 3;
					_obstacles[j]._closerSequenceId = 189;
					_obstacles[j]._passedSequenceId = 190;
					_obstacles[j]._splashSequenceId = 191;
					_obstacles[j]._collisionSequenceId = 195;
					break;
				case 0xBA:
					_obstacles[j]._laneNum = 2;
					_obstacles[j]._closerSequenceId = 186;
					_obstacles[j]._passedSequenceId = 187;
					_obstacles[j]._splashSequenceId = 188;
					_obstacles[j]._collisionSequenceId = 194;
					break;
				case 0xCB:
					_obstacles[j]._laneNum = 1;
					_obstacles[j]._closerSequenceId = 203;
					_obstacles[j]._passedSequenceId = 204;
					_obstacles[j]._splashSequenceId = 0;
					_obstacles[j]._collisionSequenceId = 209;
					break;
				case 0xCD:
					_obstacles[j]._laneNum = 2;
					_obstacles[j]._closerSequenceId = 205;
					_obstacles[j]._passedSequenceId = 206;
					_obstacles[j]._splashSequenceId = 0;
					_obstacles[j]._collisionSequenceId = 210;
					break;
				case 0xCF:
					_obstacles[j]._laneNum = 3;
					_obstacles[j]._closerSequenceId = 207;
					_obstacles[j]._passedSequenceId = 208;
					_obstacles[j]._splashSequenceId = 0;
					_obstacles[j]._collisionSequenceId = 211;
					break;
				default:
					break;
				}
				_obstacles[j]._prevId = _truckId;
				_obstacles[j]._currId = _obstacles[j]._prevId;
				_vm->_gameSys->setAnimation(_obstacles[j]._currSequenceId, _obstacles[j]._currId, j + 2);
				_vm->_gameSys->insertSequence(_obstacles[j]._currSequenceId, _obstacles[j]._currId, 0, 0, kSeqNone, 0, 0, -50);
				_vm->_timers[2] = kObstacleDefs[_obstacleIndex]._ticks;
				++_obstacleIndex;
				if (_obstacleIndex == 50)
					_obstacleIndex = 0;
				break;
			}
		}
	}
}

void Scene49::updateObstacle(int id) {
	GameSys& gameSys = *_vm->_gameSys;

	Scene49Obstacle &obstacle = _obstacles[id];
	obstacle._currId = obstacle._prevId;

	switch (obstacle._laneNum) {
	case 1:
		obstacle._prevId = _truckId + 1;
		break;
	case 2:
		if (_truckLaneNum != 2 && _truckLaneNum != 3)
			obstacle._prevId = _truckId - 1;
		else
			obstacle._prevId = _truckId + 1;
		break;
	case 3:
		if (_truckLaneNum != 1 && _truckLaneNum != 2)
			obstacle._prevId = _truckId;
		else
			obstacle._prevId = _truckId - 1;
		break;
	default:
		break;
	}

	if (obstacle._currSequenceId == obstacle._closerSequenceId) {
		if (_truckLaneNum == obstacle._laneNum) {
			if (obstacle._splashSequenceId) {
				gameSys.setAnimation(obstacle._collisionSequenceId, obstacle._prevId, id + 2);
				gameSys.insertSequence(obstacle._collisionSequenceId, obstacle._prevId,
					obstacle._currSequenceId, obstacle._currId,
					kSeqSyncWait, 0, 0, -50);
				obstacle._currSequenceId = obstacle._collisionSequenceId;
				_vm->playSound(0xE0, false);
				increaseScore(30);
			} else if ((obstacle._laneNum == 1 && _truckSequenceId == 0xB0) ||
				(obstacle._laneNum == 2 && (_truckSequenceId == 0xB1 || _truckSequenceId == 0xB2)) ||
				(obstacle._laneNum == 3 && _truckSequenceId == 0xB3)) {
				gameSys.setAnimation(obstacle._passedSequenceId, obstacle._prevId, id + 2);
				gameSys.insertSequence(obstacle._passedSequenceId, obstacle._prevId,
					obstacle._currSequenceId, obstacle._currId,
					kSeqSyncWait, 0, 0, -50);
				obstacle._currSequenceId = obstacle._passedSequenceId;
			} else {
				gameSys.setAnimation(obstacle._collisionSequenceId, 256, 0);
				gameSys.setAnimation(obstacle._passedSequenceId, obstacle._prevId, id + 2);
				gameSys.insertSequence(obstacle._passedSequenceId, obstacle._prevId,
					obstacle._currSequenceId, obstacle._currId,
					kSeqSyncWait, 0, 0, -50);
				gameSys.insertSequence(obstacle._collisionSequenceId, 256,
					_truckSequenceId, _truckId,
					kSeqSyncExists, 0, 0, -50);
				_truckSequenceId = obstacle._collisionSequenceId;
				_truckId = 256;
				obstacle._currSequenceId = obstacle._passedSequenceId;
				_vm->playSound(0xE1, false);
				decreaseScore(30);
			}
		} else {
			gameSys.setAnimation(obstacle._passedSequenceId, obstacle._prevId, id + 2);
			gameSys.insertSequence(obstacle._passedSequenceId, obstacle._prevId,
				obstacle._currSequenceId, obstacle._currId,
				kSeqSyncWait, 0, 0, -50);
			obstacle._currSequenceId = obstacle._passedSequenceId;
		}
	} else if (obstacle._currSequenceId == obstacle._passedSequenceId) {
		if (_truckLaneNum == obstacle._laneNum) {
			if (obstacle._splashSequenceId) {
				gameSys.setAnimation(obstacle._collisionSequenceId, obstacle._prevId, id + 2);
				gameSys.insertSequence(obstacle._collisionSequenceId, obstacle._prevId,
					obstacle._currSequenceId, obstacle._currId,
					kSeqSyncWait, 0, 0, -50);
				obstacle._currSequenceId = obstacle._collisionSequenceId;
				_vm->playSound(0xE0, false);
				increaseScore(30);
			}
		} else if (obstacle._splashSequenceId) {
			gameSys.setAnimation(obstacle._splashSequenceId, obstacle._prevId, id + 2);
			gameSys.insertSequence(obstacle._splashSequenceId, obstacle._prevId,
				obstacle._currSequenceId, obstacle._currId,
				kSeqSyncWait, 0, 0, -50);
			obstacle._currSequenceId = obstacle._splashSequenceId;
		}
	} else {
		gameSys.setAnimation(0, 0, id + 2);
		clearObstacle(id);
	}
}

void Scene49::increaseScore(int amount) {
	if (_scoreBarPos + amount <= 556) {
		_scoreBarPos += amount;
		_vm->_gameSys->fillSurface(nullptr, _scoreBarPos, 508, amount, 22, 255, 0, 0);
	}

	_scoreLevel = (_scoreBarPos + amount >= 556) ? 1 : 0;
}

void Scene49::decreaseScore(int amount) {
	if (_scoreBarPos >= 226 && _scoreLevel == 0) {
		if (_scoreBarFlash)
			refreshScoreBar();
		_vm->_gameSys->fillSurface(nullptr, _scoreBarPos, 508, amount, 22, 89, 0, 5);
		_scoreBarPos -= amount;
		_scoreLevel = 0;
	}
}

void Scene49::refreshScoreBar() {
	if (_scoreBarFlash)
		_vm->_gameSys->fillSurface(nullptr, 226, 508, 330, 22, 255, 0, 0);
	else
		_vm->_gameSys->fillSurface(nullptr, 226, 508, 330, 22, 89, 0, 5);
	_scoreBarFlash = !_scoreBarFlash;
}

void Scene49::clearObstacle(int index) {
	_obstacles[index]._currSequenceId = 0;
	_obstacles[index]._closerSequenceId = 0;
	_obstacles[index]._passedSequenceId = 0;
	_obstacles[index]._splashSequenceId = 0;
	_obstacles[index]._collisionSequenceId = 0;
	_obstacles[index]._prevId = 0;
	_obstacles[index]._currId = 0;
	_obstacles[index]._laneNum = 0;
}

void Scene49::run() {
	GameSys& gameSys = *_vm->_gameSys;

	bool animToggle6 = false;
	bool animToggle5 = false;
	bool animToggle4 = false;
	bool animToggle3 = false;
	bool streetAnimToggle = false;
	bool bgAnimToggle = false;

	_vm->playSound(0xE2, true);
	_vm->setSoundVolume(0xE2, 75);

	_vm->hideCursor();
	_vm->setGrabCursorSprite(-1);

	_scoreBarPos = 196;
	_scoreLevel = 0;
	_scoreBarFlash = false;

	switch (_vm->getRandom(3)) {
	case 0:
		_truckSequenceId = 0xAD;
		_truckLaneNum = 1;
		break;
	case 1:
		_truckSequenceId = 0xAE;
		_truckLaneNum = 2;
		break;
	case 2:
		_truckSequenceId = 0xAF;
		_truckLaneNum = 3;
		break;
	default:
		break;
	}

	int bgWidth1 = gameSys.getSpriteWidthById(0x5E);
	int bgX1 = 600;

	int bgWidth2 = gameSys.getSpriteWidthById(0x5F);
	int bgX2 = 400;

	int bgWidth3 = gameSys.getSpriteWidthById(4);
	int bgX3 = 700;

	int bgWidth4 = gameSys.getSpriteWidthById(5);
	int bgX4 = 500;

	int bgWidth5 = gameSys.getSpriteWidthById(6);
	int bgX5 = 300;

	int bgWidth6 = gameSys.getSpriteWidthById(7);
	int bgX6 = 100;

	gameSys.setAnimation(0xC8, 251, 1);
	gameSys.setAnimation(_truckSequenceId, 256, 0);
	gameSys.insertSequence(0xC9, 256, 0, 0, kSeqNone, 0, 600, 85);
	gameSys.insertSequence(0xCA, 257, 0, 0, kSeqNone, 0, 400, 100);
	gameSys.insertSequence(0xC4, 256, 0, 0, kSeqNone, 0, 700, 140);
	gameSys.insertSequence(0xC5, 257, 0, 0, kSeqNone, 0, 500, 160);
	gameSys.insertSequence(0xC6, 258, 0, 0, kSeqNone, 0, 300, 140);
	gameSys.insertSequence(0xC7, 259, 0, 0, kSeqNone, 0, 100, 140);
	gameSys.insertSequence(0xC8, 251, 0, 0, kSeqNone, 0, 0, -50);
	gameSys.insertSequence(_truckSequenceId, 256, 0, 0, kSeqNone, 0, 0, -50);

	_vm->_timers[0] = 2;

	for (int i = 0; i < 5; ++i)
		clearObstacle(i);

	_obstacleIndex = 0;

	_vm->_timers[2] = _vm->getRandom(20) + 10;

	_truckId = 256;
	_vm->_timers[3] = 35;

	while (!_vm->_sceneDone) {
		if (_vm->_timers[0] == 0) {
			// Update background animations (clouds etc.)
			--bgX1;
			bgX2 -= 2;
			bgX3 -= 5;
			--bgX4;
			--bgX5;
			--bgX6;
			if (bgX1 <= -bgWidth1)
				bgX1 = 799;
			if (bgX2 <= -bgWidth2)
				bgX2 = 799;
			if (bgX3 <= -bgWidth3)
				bgX3 = 799;
			if (bgX4 <= -bgWidth4)
				bgX4 = 799;
			if (bgX5 <= -bgWidth5)
				bgX5 = 799;
			if (bgX6 <= -bgWidth6)
				bgX6 = 799;
			bgAnimToggle = !bgAnimToggle;
			gameSys.insertSequence(0xC9, (bgAnimToggle ? 1 : 0) + 256, 0xC9, (bgAnimToggle ? 0 : 1) + 256, kSeqSyncWait, 0, bgX1, 85);
			gameSys.insertSequence(0xCA, (bgAnimToggle ? 1 : 0) + 257, 0xCA, (bgAnimToggle ? 0 : 1) + 257, kSeqSyncWait, 0, bgX2, 100);
			gameSys.insertSequence(0xC4, (bgAnimToggle ? 1 : 0) + 256, 0xC4, (bgAnimToggle ? 0 : 1) + 256, kSeqSyncWait, 0, bgX3, 140);
			gameSys.insertSequence(0xC5, (bgAnimToggle ? 1 : 0) + 257, 0xC5, (bgAnimToggle ? 0 : 1) + 257, kSeqSyncWait, 0, bgX4, 160);
			gameSys.insertSequence(0xC6, (bgAnimToggle ? 1 : 0) + 258, 0xC6, (bgAnimToggle ? 0 : 1) + 258, kSeqSyncWait, 0, bgX5, 140);
			gameSys.insertSequence(0xC7, (bgAnimToggle ? 1 : 0) + 259, 0xC7, (bgAnimToggle ? 0 : 1) + 259, kSeqSyncWait, 0, bgX6, 140);
			_vm->_timers[0] = 2;
		}

		if (gameSys.getAnimationStatus(1) == 2) {
			streetAnimToggle = !streetAnimToggle;
			gameSys.setAnimation(0xC8, (streetAnimToggle ? 1 : 0) + 251, 1);
			gameSys.insertSequence(0xC8, (streetAnimToggle ? 1 : 0) + 251, 200, (streetAnimToggle ? 0 : 1) + 251, kSeqSyncWait, 0, 0, -50);
		}

		checkObstacles();

		if (gameSys.getAnimationStatus(0) == 2) {
			switch (_truckSequenceId) {
			case 0xB1:
				_truckLaneNum = 1;
				break;
			case 0xB0:
			case 0xB3:
				_truckLaneNum = 2;
				break;
			case 0xB2:
				_truckLaneNum = 3;
				break;
			default:
				break;
			}
			animToggle3 = !animToggle3;
			if (_truckLaneNum == 1) {
				gameSys.setAnimation(0xAD, (animToggle3 ? 1 : 0) + 256, 0);
				gameSys.insertSequence(0xAD, (animToggle3 ? 1 : 0) + 256, _truckSequenceId, _truckId, kSeqSyncWait, 0, 0, -50);
				_truckSequenceId = 0xAD;
			} else if (_truckLaneNum == 2) {
				gameSys.setAnimation(0xAE, (animToggle3 ? 1 : 0) + 256, 0);
				gameSys.insertSequence(0xAE, (animToggle3 ? 1 : 0) + 256, _truckSequenceId, _truckId, kSeqSyncWait, 0, 0, -50);
				_truckSequenceId = 0xAE;
			} else {
				gameSys.setAnimation(0xAF, (animToggle3 ? 1 : 0) + 256, 0);
				gameSys.insertSequence(0xAF, (animToggle3 ? 1 : 0) + 256, _truckSequenceId, _truckId, kSeqSyncWait, 0, 0, -50);
				_truckSequenceId = 0xAF;
			}
			_truckId = (animToggle3 ? 1 : 0) + 256;
			if (_scoreLevel == 1) {
				if (!gameSys.isSequenceActive(0xD4, 266)) {
					gameSys.setAnimation(0xD4, 266, 8);
					gameSys.insertSequence(0xD4, 266, 0, 0, kSeqNone, 0, 0, -50);
				}
				++_scoreLevel;
				_vm->_timers[1] = 2;
				animToggle4 = false;
				animToggle5 = false;
				animToggle6 = false;
				_scoreBarFlash = false;
			}
		}

		if (_scoreLevel != 0 && !_vm->_timers[1]) {
			refreshScoreBar();
			_vm->_timers[1] = 8;
			if (animToggle6) {
				if (animToggle5) {
					if (animToggle4 && !gameSys.isSequenceActive(212, 266))
						gameSys.insertSequence(212, 266, 0, 0, kSeqNone, 0, 0, -50);
					animToggle4 = !animToggle4;
				}
				animToggle5 = !animToggle5;
			}
			animToggle6 = !animToggle6;
		}

		updateAnimations();

		if (clearKeyStatus()) {
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 2;
			_vm->_newCursorValue = 1;
		}

		if (_vm->isKeyStatus1(Common::KEYCODE_RIGHT)) {
			// Steer right
			if (_truckSequenceId == 0xB3)
				_truckLaneNum = 2;
			if (_truckSequenceId == 0xB1)
				_truckLaneNum = 1;
			if (_truckLaneNum != 3 && _truckLaneNum != 2) {
				if (_scoreLevel) {
					_vm->_sceneDone = true;
					_vm->_newSceneNum = 47;
				}
			} else {
				int steerSequenceId = (_truckLaneNum == 3) ? 0xB3 : 0xB1;
				if (_truckSequenceId == 0xAE || _truckSequenceId == 0xAF) {
					gameSys.setAnimation(steerSequenceId, 256, 0);
					gameSys.insertSequence(steerSequenceId, 256, _truckSequenceId, _truckId, kSeqSyncExists, 0, 0, -50);
					_truckSequenceId = steerSequenceId;
					_truckId = 256;
				}
			}
			_vm->clearKeyStatus1(Common::KEYCODE_RIGHT);
		}

		if (_vm->isKeyStatus1(Common::KEYCODE_LEFT)) {
			// Steer left
			if (_truckSequenceId == 0xB0)
				_truckLaneNum = 2;
			if (_truckSequenceId == 0xB2)
				_truckLaneNum = 3;
			if (_truckLaneNum == 1 || _truckLaneNum == 2) {
				int steerSequenceId = (_truckLaneNum == 1) ? 0xB0 : 0xB2;
				if (_truckSequenceId == 0xAD || _truckSequenceId == 0xAE) {
					gameSys.setAnimation(steerSequenceId, 256, 0);
					gameSys.insertSequence(steerSequenceId, 256, _truckSequenceId, _truckId, kSeqSyncExists, 0, 0, -50);
					_truckSequenceId = steerSequenceId;
					_truckId = 256;
				}
			}
			_vm->clearKeyStatus1(Common::KEYCODE_LEFT);
		}
		_vm->gameUpdateTick();
	}
	_vm->stopSound(0xE2);
}

void Scene49::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	for (int i = 0; i < 5; ++i) {
		if (gameSys.getAnimationStatus(i + 2) == 2) {
			if (_obstacles[i]._currSequenceId)
				updateObstacle(i);
		}
	}

	if (gameSys.getAnimationStatus(8) == 2) {
		_vm->_sceneDone = true;
		_vm->_newSceneNum = 47;
	}
}

/*****************************************************************************/

Scene50::Scene50(GnapEngine *vm) : Scene(vm) {
	_fightDone = false;

	_roundNum = -1;
	_timeRemaining = -1;
	_leftTongueRoundsWon = -1;
	_rightTongueRoundsWon = -1;
	_leftTongueSequenceId = -1;
	_leftTongueId = -1;
	_leftTongueNextSequenceId = -1;
	_leftTongueNextId = -1;
	_rightTongueSequenceId = -1;
	_rightTongueId = -1;
	_rightTongueNextSequenceId = -1;
	_rightTongueNextId = -1;
	_leftTongueEnergy = -1;
	_rightTongueEnergy = -1;

	_timesPlayed = 0;
	_timesPlayedModifier = 0;
	_attackCounter = 0;
	_leftTongueEnergyBarPos = 10;
	_leftTongueNextIdCtr = 0;
	_rightTongueEnergyBarPos = 10;
	_rightTongueNextIdCtr = 0;
}

int Scene50::init() {
	return 0xC7;
}

void Scene50::updateHotspots() {
	_vm->_hotspotsCount = 0;
}

bool Scene50::tongueWinsRound(int tongueNum) {
	if (tongueNum == 1)
		++_leftTongueRoundsWon;
	else
		++_rightTongueRoundsWon;
	playWinBadgeAnim(tongueNum);
	bool fightOver = _rightTongueRoundsWon == 2 || _leftTongueRoundsWon == 2;
	playWinAnim(tongueNum, fightOver);
	return fightOver;
}

void Scene50::playWinAnim(int tongueNum, bool fightOver) {
	if (tongueNum == 1) {
		if (fightOver) {
			_vm->_gameSys->insertSequence(0xAD, 140, 0xAC, 140, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xB4, 100, _leftTongueSequenceId, _leftTongueId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xBD, 100, _rightTongueSequenceId, _rightTongueId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xBC, 100, 0xBD, 100, kSeqSyncWait, 0, 0, 0);
			_leftTongueSequenceId = 0xB4;
			_rightTongueSequenceId = 0xBC;
			_rightTongueId = 100;
			_leftTongueId = 100;
			_vm->_gameSys->setAnimation(0xB4, 100, 6);
			_vm->_gameSys->setAnimation(_rightTongueSequenceId, 100, 5);
			waitForAnim(6);
			waitForAnim(5);
			_vm->invAdd(kItemGum);
			_vm->setFlag(kGFUnk13);
		} else {
			_vm->_gameSys->insertSequence(0xB4, 100, _leftTongueSequenceId, _leftTongueId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xBD, 100, _rightTongueSequenceId, _rightTongueId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xBC, 100, 0xBD, 100, kSeqSyncWait, 0, 0, 0);
			_leftTongueSequenceId = 0xB4;
			_rightTongueSequenceId = 0xBC;
			_rightTongueId = 100;
			_leftTongueId = 100;
			_vm->_gameSys->setAnimation(0xB4, 100, 6);
			_vm->_gameSys->setAnimation(_rightTongueSequenceId, 100, 5);
			waitForAnim(6);
			waitForAnim(5);
		}
	} else {
		_vm->_gameSys->insertSequence(0xBE, 100, _rightTongueSequenceId, _rightTongueId, kSeqSyncWait, 0, 0, 0);
		_vm->_gameSys->setAnimation(0xBE, 100, 5);
		waitForAnim(5);
		_vm->_gameSys->insertSequence(0xBF, 100, 0xBE, 100, kSeqSyncWait, 0, 0, 0);
		_vm->_gameSys->insertSequence(0xB5, 100, _leftTongueSequenceId, _leftTongueId, kSeqSyncWait, 0, 0, 0);
		_rightTongueSequenceId = 0xBF;
		_leftTongueSequenceId = 0xB5;
		_rightTongueId = 100;
		_leftTongueId = 100;
		_vm->_gameSys->setAnimation(0xB5, 100, 6);
		_vm->_gameSys->setAnimation(_rightTongueSequenceId, 100, 5);
		waitForAnim(6);
		waitForAnim(5);
	}
	_vm->delayTicksA(1, 7);
}

void Scene50::delayTicks() {
	_vm->delayTicksA(3, 7);
}

void Scene50::initRound() {
	_leftTongueEnergy = 10;
	_rightTongueEnergy = 10;
	_fightDone = false;
	_vm->_timers[3] = getRightTongueActionTicks();
	_vm->_timers[4] = 0;
	_vm->_timers[6] = 0;
	_vm->_gameSys->fillSurface(nullptr, 91, 73, 260, 30, 212, 0, 0);
	_vm->_gameSys->fillSurface(nullptr, 450, 73, 260, 30, 212, 0, 0);
	_timeRemaining = 40;
	drawCountdown(40);
}

bool Scene50::updateCountdown() {
	if (!_vm->_timers[5]) {
		--_timeRemaining;
		if (_timeRemaining < 0) {
			return true;
		} else {
			_vm->_timers[5] = 15;
			drawCountdown(_timeRemaining);
		}
	}
	return false;
}

void Scene50::drawCountdown(int value) {
	char str[8];
	sprintf(str, "%02d", value);
	_vm->_gameSys->fillSurface(nullptr, 371, 505, 50, 27, 0, 0, 0);
	_vm->_gameSys->drawTextToSurface(nullptr, 381, 504, 255, 255, 255, str);
}

void Scene50::playTonguesIdle() {
	_vm->_gameSys->insertSequence(0xBA, 100, _leftTongueSequenceId, _leftTongueId, kSeqSyncWait, 0, 0, 0);
	_vm->_gameSys->insertSequence(0xC2, 100, _rightTongueSequenceId, _rightTongueId, kSeqSyncWait, 0, 0, 0);
	_leftTongueSequenceId = 0xBA;
	_rightTongueSequenceId = 0xC2;
	_rightTongueNextSequenceId = -1;
	_leftTongueNextSequenceId = -1;
	_leftTongueId = 100;
	_rightTongueId = 100;
	_vm->_gameSys->setAnimation(0xC2, 100, 5);
	_vm->_gameSys->setAnimation(_leftTongueSequenceId, _leftTongueId, 6);
}

void Scene50::playRoundAnim(int roundNum) {
	int sequenceId = 0;

	switch (roundNum) {
	case 1:
		sequenceId = 0xAF;
		break;
	case 2:
		sequenceId = 0xB0;
		break;
	case 3:
		sequenceId = 0xB1;
		break;
	default:
		break;
	}

	_vm->_gameSys->insertSequence(sequenceId, 256, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_gameSys->setAnimation(sequenceId, 256, 7);
	waitForAnim(7);

	_vm->_gameSys->insertSequence(0xAB, 256, sequenceId, 256, kSeqSyncWait, 0, 0, 0);
	_vm->_gameSys->setAnimation(0xAB, 256, 7);
	waitForAnim(7);
}

bool Scene50::updateEnergyBars(int newLeftBarPos, int newRightBarPos) {
	if (newLeftBarPos != _leftTongueEnergyBarPos) {
		if (newLeftBarPos < 0)
			newLeftBarPos = 0;
		_leftTongueEnergyBarPos = newLeftBarPos;
		_vm->_gameSys->fillSurface(nullptr, 26 * newLeftBarPos + 91, 73, 260 - 26 * newLeftBarPos, 30, 0, 0, 0);
	}

	if (newRightBarPos != _rightTongueEnergyBarPos) {
		if (newRightBarPos < 0)
			newRightBarPos = 0;
		_rightTongueEnergyBarPos = newRightBarPos;
		if (newRightBarPos != 10)
			_vm->_gameSys->fillSurface(nullptr, 26 * (9 - newRightBarPos) + 450, 73, 26, 30, 0, 0, 0);
	}

	if (newLeftBarPos * newRightBarPos > 0)
		return false;

	_leftTongueEnergyBarPos = 10;
	_rightTongueEnergyBarPos = 10;
	return true;
}

void Scene50::waitForAnim(int animationIndex) {
	GameSys& gameSys = *_vm->_gameSys;

	while (gameSys.getAnimationStatus(animationIndex) != 2 && !_vm->_gameDone)
		_vm->gameUpdateTick();

	gameSys.setAnimation(0, 0, animationIndex);
}

int Scene50::checkInput() {
	int sequenceId = -1;

	if (_vm->isKeyStatus1(Common::KEYCODE_RIGHT)) {
		_vm->clearKeyStatus1(Common::KEYCODE_RIGHT);
		sequenceId = 0xB6;
	} else if (_vm->isKeyStatus1(Common::KEYCODE_LEFT)) {
		_vm->clearKeyStatus1(Common::KEYCODE_LEFT);
		sequenceId = 0xB3;
	} else if (_vm->isKeyStatus1(Common::KEYCODE_ESCAPE)) {
		_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
		_fightDone = true;
	}

	return sequenceId;
}

int Scene50::getRightTongueAction() {
	int sequenceId = -1;

	if (!_vm->_timers[3]) {
		_vm->_timers[3] = getRightTongueActionTicks();
		if (_rightTongueEnergy >= _leftTongueEnergy) {
			switch (_vm->getRandom(5)) {
			case 0:
				sequenceId = 0xBE;
				break;
			case 1:
				sequenceId = 0xBE;
				break;
			case 2:
				sequenceId = 0xBB;
				break;
			case 3:
				sequenceId = 0xBB;
				break;
			case 4:
				sequenceId = 0xBB;
				break;
			default:
				break;
			}
		} else {
			switch (_vm->getRandom(4)) {
			case 0:
				sequenceId = 0xBE;
				break;
			case 1:
				sequenceId = 0xBB;
				break;
			case 2:
				sequenceId = 0xBE;
				break;
			case 3:
				sequenceId = 0xBE;
				break;
			default:
				break;
			}
		}
	}

	return sequenceId;
}

void Scene50::updateAnimations() {
	if (!_vm->_timers[4])
		_attackCounter = 0;

	if (_vm->_gameSys->getAnimationStatus(5) == 2) {
		if (_rightTongueSequenceId == 0xBE) {
			if (_leftTongueSequenceId != 0xB3 && _leftTongueSequenceId != 0xB8)
				_rightTongueNextSequenceId = 0xBF;
			else
				_rightTongueNextSequenceId = 0xC0;
		}
		if (_rightTongueNextSequenceId == -1)
			_rightTongueNextSequenceId = 0xC2;
		if (_rightTongueNextSequenceId == 0xBF) {
			_leftTongueNextId = getLeftTongueNextId();
			_rightTongueNextId = getRightTongueNextId();
			_vm->_gameSys->setAnimation(_rightTongueNextSequenceId, _rightTongueNextId, 5);
			_vm->_gameSys->setAnimation(0xB9, _leftTongueNextId, 6);
			_vm->_gameSys->insertSequence(_rightTongueNextSequenceId, _rightTongueNextId, _rightTongueSequenceId, _rightTongueId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xB9, _leftTongueNextId, _leftTongueSequenceId, _leftTongueId, kSeqSyncExists, 0, 0, 0);
			_rightTongueSequenceId = _rightTongueNextSequenceId;
			_rightTongueNextSequenceId = -1;
			_leftTongueSequenceId = 0xB9;
			_leftTongueNextSequenceId = -1;
			_rightTongueId = _rightTongueNextId;
			_leftTongueId = _leftTongueNextId;
			_leftTongueEnergy -= _vm->getRandom(1) + 1;
		} else {
			_rightTongueNextId = getRightTongueNextId();
			_vm->_gameSys->setAnimation(_rightTongueNextSequenceId, _rightTongueNextId, 5);
			_vm->_gameSys->insertSequence(_rightTongueNextSequenceId, _rightTongueNextId, _rightTongueSequenceId, _rightTongueId, kSeqSyncWait, 0, 0, 0);
			_rightTongueSequenceId = _rightTongueNextSequenceId;
			_rightTongueNextSequenceId = -1;
			_rightTongueId = _rightTongueNextId;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(6) == 2) {
		if (_leftTongueSequenceId == 0xB6) {
			++_attackCounter;
			if (_timesPlayedModifier + 3 <= _attackCounter) {
				_leftTongueNextSequenceId = 0xB8;
			} else {
				_vm->_timers[4] = 20;
				if (_rightTongueSequenceId != 0xBB && _rightTongueSequenceId != 0xC0 && _vm->getRandom(7) != _roundNum)
					_leftTongueNextSequenceId = 0xB7;
				else
					_leftTongueNextSequenceId = 0xB8;
			}
		}
		if (_leftTongueNextSequenceId == 0xB3)
			--_attackCounter;
		if (_leftTongueNextSequenceId == -1)
			_leftTongueNextSequenceId = 0xBA;
		if (_leftTongueNextSequenceId == 0xB7) {
			_leftTongueNextId = getLeftTongueNextId();
			_rightTongueNextId = getRightTongueNextId();
			_vm->_gameSys->setAnimation(_leftTongueNextSequenceId, _leftTongueNextId, 6);
			_vm->_gameSys->setAnimation(0xC1, _rightTongueNextId, 5);
			_vm->_gameSys->insertSequence(_leftTongueNextSequenceId, _leftTongueNextId, _leftTongueSequenceId, _leftTongueId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xC1, _rightTongueNextId, _rightTongueSequenceId, _rightTongueId, kSeqSyncExists, 0, 0, 0);
			_leftTongueSequenceId = _leftTongueNextSequenceId;
			_leftTongueNextSequenceId = -1;
			_rightTongueSequenceId = 0xC1;
			_rightTongueNextSequenceId = -1;
			_rightTongueId = _rightTongueNextId;
			_leftTongueId = _leftTongueNextId;
			--_rightTongueEnergy;
		} else if (_leftTongueNextSequenceId != 0xB8 || _rightTongueSequenceId != 0xC2) {
			_leftTongueNextId = getLeftTongueNextId();
			_vm->_gameSys->setAnimation(_leftTongueNextSequenceId, _leftTongueNextId, 6);
			_vm->_gameSys->insertSequence(_leftTongueNextSequenceId, _leftTongueNextId, _leftTongueSequenceId, _leftTongueId, kSeqSyncWait, 0, 0, 0);
			_leftTongueSequenceId = _leftTongueNextSequenceId;
			_leftTongueNextSequenceId = -1;
			_leftTongueId = _leftTongueNextId;
		} else {
			_leftTongueNextId = getLeftTongueNextId();
			_rightTongueNextId = getRightTongueNextId();
			_vm->_gameSys->setAnimation(0xBB, _rightTongueNextId, 5);
			_vm->_gameSys->setAnimation(_leftTongueNextSequenceId, _leftTongueNextId, 6);
			_vm->_gameSys->insertSequence(_leftTongueNextSequenceId, _leftTongueNextId, _leftTongueSequenceId, _leftTongueId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0xBB, _rightTongueNextId, _rightTongueSequenceId, _rightTongueId, kSeqSyncExists, 0, 0, 0);
			_rightTongueSequenceId = 0xBB;
			_rightTongueId = _rightTongueNextId;
			_rightTongueNextSequenceId = -1;
			_leftTongueSequenceId = _leftTongueNextSequenceId;
			_leftTongueNextSequenceId = -1;
			_leftTongueId = _leftTongueNextId;
		}
	}
}

int Scene50::getRightTongueActionTicks() {
	return 15 - 5 * _roundNum + 1;
}

int Scene50::getLeftTongueNextId() {
	_leftTongueNextIdCtr = (_leftTongueNextIdCtr + 1) % 3;
	return _leftTongueNextIdCtr + 100;
}

int Scene50::getRightTongueNextId() {
	_rightTongueNextIdCtr = (_rightTongueNextIdCtr + 1) % 3;
	return _rightTongueNextIdCtr + 100;
}

void Scene50::playWinBadgeAnim(int tongueNum) {
	int sequenceId;

	if (tongueNum == 1) {
		if (_leftTongueRoundsWon == 1)
	  		sequenceId = 0xC3;
		else
	  		sequenceId = 0xC4;
	} else {
		if (_rightTongueRoundsWon == 1)
			sequenceId = 0xC5;
		else
			sequenceId = 0xC6;
	}

	_vm->_gameSys->setAnimation(sequenceId, 120, 7);
	_vm->_gameSys->insertSequence(sequenceId, 120, 0, 0, kSeqNone, 0, 0, 0);
	waitForAnim(7);
}

void Scene50::run() {
	++_timesPlayed;
	_timesPlayedModifier = _timesPlayed / 4;
	_leftTongueRoundsWon = 0;
	_rightTongueRoundsWon = 0;
	_leftTongueSequenceId = 186;
	_rightTongueSequenceId = 194;
	_rightTongueNextSequenceId = -1;
	_leftTongueNextSequenceId = -1;
	_leftTongueId = 100;
	_rightTongueId = 100;

	_vm->_gameSys->setAnimation(194, 100, 5);
	_vm->_gameSys->setAnimation(_leftTongueSequenceId, _leftTongueId, 6);
	_vm->_gameSys->insertSequence(_leftTongueSequenceId, _leftTongueId, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_gameSys->insertSequence(_rightTongueSequenceId, _rightTongueId, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_gameSys->insertSequence(172, 140, 0, 0, kSeqNone, 0, 0, 0);
	_vm->endSceneInit();

	initRound();

	_roundNum = 1;

	_vm->setGrabCursorSprite(-1);
	_vm->hideCursor();

	_vm->delayTicksA(1, 7);

	playRoundAnim(_roundNum);

	_vm->_timers[5] = 15;

	while (!_fightDone && !_vm->_gameDone) {
		int playerSequenceId = checkInput();
		if (playerSequenceId != -1)
			_leftTongueNextSequenceId = playerSequenceId;

		int rightSequenceId = getRightTongueAction();
		if (rightSequenceId != -1)
			_rightTongueNextSequenceId = rightSequenceId;

		updateAnimations();

		if (updateCountdown() ||
			updateEnergyBars(_leftTongueEnergy, _rightTongueEnergy)) {
			bool v0;
			if (_rightTongueEnergy < _leftTongueEnergy)
				v0 = tongueWinsRound(1);
			else
				v0 = tongueWinsRound(2);
			if (v0) {
				delayTicks();
				_fightDone = true;
			} else {
				++_roundNum;
				initRound();
				playTonguesIdle();
				updateEnergyBars(_leftTongueEnergy, _rightTongueEnergy);
				playRoundAnim(_roundNum);
				_vm->_timers[5] = 15;
			}
		}
		_vm->gameUpdateTick();
	}

	_vm->_gameSys->setAnimation(0, 0, 7);
	_vm->_gameSys->setAnimation(0, 0, 6);
	_vm->_gameSys->setAnimation(0, 0, 5);
	_vm->_gameSys->setAnimation(0, 0, 3);

	_vm->showCursor();
}

/*****************************************************************************/

static const int kDigitSequenceIds[] = {
	0xCA, 0xCB, 0xCC, 0xCD, 0xCE,
	0xCF, 0xD0, 0xD1, 0xD2, 0xD3
};

static const int kDigitPositions[4] = {
	0, 34, 83, 119
};

/*
	0xBA	Falling banana peel
	0xBC	Banana peel goes away
	0xBD	Falling coin
	0xBE	Fallen coin
	0xC0	Falling banknote
	0xB6	Platypus tripping (right)
	0xB7	Platypus tripping (left)
	0x76	Platypus jumping (right)
*/

Scene51::Scene51(GnapEngine *vm) : Scene(vm) {
	_dropLoseCash = false;

	_cashAmount = -1;
	_guySequenceId = -1;
	_guyNextSequenceId = -1;
	_itemsCaughtCtr = -1;
	_dropSpeedTicks = -1;
	_nextDropItemKind = -1;
	_itemInsertX = -1;
	_itemInsertDirection = -1;
	_platypusSequenceId = -1;
	_platypusNextSequenceId = -1;
	_platypusJumpSequenceId = -1;
	_itemsCtr = -1;
	_itemsCtr1 = -1;
	_itemsCtr2 = -1;

	for (int i = 0; i < 4; i++) {
		_digits[i] = 0;
		_digitSequenceIds[i] = -1;
	}

	for (int i = 0; i < 6; i++) {
		_items[i]._currSequenceId = -1;
		_items[i]._droppedSequenceId = 0;
		_items[i]._x = 0;
		_items[i]._y = 0;
		_items[i]._collisionX = 0;
		_items[i]._canCatch = false;
		_items[i]._isCollision = false;
		_items[i]._x2 = 0;
		_items[i]._id = -1;
	}
}

int Scene51::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	for (int i = 0; i < 6; ++i)
		_vm->_gameSys->setAnimation(0, 0, i + 1);
	return 0xD4;
}

void Scene51::updateHotspots() {
	_vm->_hotspotsCount = 0;
}

void Scene51::clearItem(Scene51Item *item) {
	item->_currSequenceId = 0;
	item->_droppedSequenceId = 0;
	item->_x = 0;
	item->_y = 0;
	item->_x2 = 0;
	item->_collisionX = 0;
	item->_canCatch = false;
}

void Scene51::dropNextItem() {
	if (_vm->_timers[0])
		return;

	int index = 0;
	while (index < 6 && _items[index]._currSequenceId)
		++index;

	if (index == 6)
		return;

	switch (_nextDropItemKind) {
	case 0:
		if (_vm->getRandom(10) != 0 || _itemsCtr2 >= 2) {
			_items[index]._currSequenceId = 0xBD;
		} else {
			--_itemsCtr1;
			_items[index]._currSequenceId = 0xBA;
			++_itemsCtr2;
		}
		break;

	case 1:
		if (_vm->getRandom(8) != 0 || _itemsCtr2 >= 2) {
			if (_vm->getRandom(5) == 0) {
				if (_itemInsertDirection)
					_itemInsertX -= 70;
				else
					_itemInsertX += 70;
			}
			_items[index]._currSequenceId = 0xBD;
		} else {
			--_itemsCtr1;
			_items[index]._currSequenceId = 0xBA;
			++_itemsCtr2;
		}
		break;

	case 2:
		if (_vm->getRandom(6) != 0 || _itemsCtr2 >= 2) {
			_items[index]._currSequenceId = 0xBD;
		} else {
			--_itemsCtr1;
			_items[index]._currSequenceId = 0xBA;
			++_itemsCtr2;
		}
		break;

	case 3:
	case 4:
		if (_itemsCtr == 0)
			_itemsCtr1 = 3;
		_items[index]._currSequenceId = 0xC0;
		break;

	case 5:
	case 6:
		if (_vm->getRandom(5) != 0 || _itemsCtr2 >= 2) {
			if (_vm->getRandom(5) != 0)
				_items[index]._currSequenceId = 0xBD;
			else
				_items[index]._currSequenceId = 0xC0;
		} else {
			--_itemsCtr1;
			_items[index]._currSequenceId = 0xBA;
			++_itemsCtr2;
		}
		break;

	case 7:
		if (_vm->getRandom(5) != 0 || _itemsCtr2 >= 2) {
			if (_vm->getRandom(5) == 0) {
				if (_itemInsertDirection)
					_itemInsertX -= 40;
				else
					_itemInsertX += 40;
			}
			if (_vm->getRandom(9) != 0)
				_items[index]._currSequenceId = 0xBD;
			else
				_items[index]._currSequenceId = 0xC0;
		} else {
			--_itemsCtr1;
			_items[index]._currSequenceId = 0xBA;
			++_itemsCtr2;
		}
		break;

	default:
		if (_vm->getRandom(4) != 0 || _itemsCtr2 >= 2) {
			if (_vm->getRandom(9) != 0)
				_items[index]._currSequenceId = 0xBD;
			else
				_items[index]._currSequenceId = 0xC0;
		} else {
			--_itemsCtr1;
			_items[index]._currSequenceId = 0xBA;
			++_itemsCtr2;
		}
		break;
	}

	if (_itemInsertDirection) {
		_itemInsertX -= 73;
		if (_itemInsertX < 129) {
			_itemInsertX += 146;
			_itemInsertDirection = 0;
		}
	} else {
		_itemInsertX += 73;
		if (_itemInsertX > 685) {
			_itemInsertX -= 146;
			_itemInsertDirection = 1;
		}
	}

	if (_itemInsertX > 685)
		_itemInsertX = 685;

	if (_itemInsertX < 129)
		_itemInsertX = 129;

	if (_items[index]._currSequenceId == 0xBA) {
		_items[index]._x2 = _vm->getRandom(350) + 200;
		_items[index]._x = _items[index]._x2 - 362;
		_items[index]._y = 15;
		_items[index]._id = 249 - index;
	} else {
		_items[index]._collisionX = _itemInsertX;
		_items[index]._x = _items[index]._collisionX - 395;
		if (_items[index]._currSequenceId == 0xC0)
			_items[index]._x -= 65;
		_items[index]._id = index + 250;
		_items[index]._canCatch = true;
	}

	_vm->_gameSys->setAnimation(_items[index]._currSequenceId, _items[index]._id, index + 1);
	_vm->_gameSys->insertSequence(_items[index]._currSequenceId, _items[index]._id, 0, 0,
		kSeqNone, 0, _items[index]._x, _items[index]._y);

	_vm->_timers[0] = _dropSpeedTicks;

	if (_nextDropItemKind >= 3)
		_vm->_timers[0] = 20;

	if (_nextDropItemKind >= 5)
		_vm->_timers[0] = 5;

	if (_nextDropItemKind == 8)
		_vm->_timers[0] = 4;

	++_itemsCtr;
}

void Scene51::updateItemAnimations() {
	for (int i = 0; i < 6; ++i) {
		if (_vm->_gameSys->getAnimationStatus(i + 1) == 2)
			updateItemAnimation(&_items[i], i);
	}
}

int Scene51::checkCollision(int sequenceId) {
	if (!isJumping(sequenceId))
		return false;

	bool jumpingLeft = false, jumpingRight = false;
	int v8 = 0, v4 = 0;
	int result = 0;

	bool checkFl = false;
	for (int i = 0; i < 6; i++)
		checkFl |= _items[i]._isCollision;

	if (!checkFl)
		return false;

	if (isJumpingRight(sequenceId)) {
		v8 = getPosRight(sequenceId);
		v4 = getPosRight(sequenceId + 1);
		jumpingRight = true;
	} else if (isJumpingLeft(sequenceId)) {
		v4 = getPosLeft(sequenceId - 1) + 33;
		v8 = getPosLeft(sequenceId) + 33;
		jumpingLeft = true;
	}

	if (jumpingRight || jumpingLeft) {
		int v5 = 0;
		int i;
		for (i = 0; i < 6; ++i) {
			if (_items[i]._isCollision) {
				if (jumpingRight && _items[i]._x2 > v8 && _items[i]._x2 < v4) {
					v5 = v8 - 359;
					if (v5 == 0)
						v5 = 1;
					_platypusNextSequenceId = 0xB6;
					break;
				} else if (jumpingLeft && _items[i]._x2 < v4 && _items[i]._x2 > v8) {
					v5 = v8 - 344;
					if (v5 == 0)
						v5 = 1;
					_platypusNextSequenceId = 0xB7;
					break;
				}
			}
		}
		if (v5) {
			_vm->_gameSys->setAnimation(0xBC, _items[i]._id, i + 1);
			_vm->_gameSys->insertSequence(0xBC, _items[i]._id, _items[i]._currSequenceId, _items[i]._id, kSeqSyncWait, 0, _items[i]._x, 15);
			_items[i]._isCollision = false;
			_items[i]._currSequenceId = 0xBC;
			--_itemsCtr2;
		}
		result = v5;
	}

	return result;
}

void Scene51::updateItemAnimation(Scene51Item *item, int index) {

	switch (item->_currSequenceId) {
	case 0xBD:
	case 0xC0:
	case 0xC1:
		// Falling coin and banknote
		if (!itemIsCaught(item)) {
			if (_dropLoseCash) {
				if (item->_currSequenceId == 0xBD)
					_cashAmount -= 2;
				else
					_cashAmount -= 25;
				if (_cashAmount < 0)
					_cashAmount = 0;
				updateCash(_cashAmount);
			}
			item->_droppedSequenceId = item->_currSequenceId + 1;
			if (item->_currSequenceId != 0xC0) {
				item->_canCatch = false;
				_dropLoseCash = true;
				_itemsCtr = 0;
				_vm->_timers[0] = 10;
			}
			if (item->_droppedSequenceId) {
				_vm->_gameSys->setAnimation(item->_droppedSequenceId, item->_id, index + 1);
				_vm->_gameSys->insertSequence(item->_droppedSequenceId, item->_id, item->_currSequenceId, item->_id, kSeqSyncWait, 0, item->_x, item->_y);
				item->_currSequenceId = item->_droppedSequenceId;
				item->_y = 0;
			}
		} else {
			_vm->_gameSys->removeSequence(item->_currSequenceId, item->_id, true);
			_vm->_gameSys->setAnimation(0, 0, index + 1);
			_vm->playSound(0xDA, false);
			if (incCashAmount(item->_currSequenceId) == 1995) {
				winMinigame();
				_vm->_sceneDone = true;
			} else {
				clearItem(item);
				++_itemsCaughtCtr;
				if (_itemsCaughtCtr == 5)
					--_dropSpeedTicks;
				if (_itemsCaughtCtr == 8)
					--_dropSpeedTicks;
				if (_itemsCaughtCtr == 11)
					--_dropSpeedTicks;
				if (_itemsCaughtCtr == 14)
					--_dropSpeedTicks;
				if (_itemsCaughtCtr >= 15 && _dropSpeedTicks > 4)
					--_dropSpeedTicks;
				if (_itemsCtr1 <= _itemsCaughtCtr) {
					++_nextDropItemKind;
					_dropSpeedTicks = 10;
					_itemsCtr = 0;
					_itemsCtr1 = 20;
					_dropLoseCash = false;
					_itemsCaughtCtr = 0;
					removeCollidedItems();
				}
			}
		}
		break;

	case 0xBE:
		// Fallen coin
		item->_droppedSequenceId = item->_currSequenceId + 1;
		if (item->_droppedSequenceId) {
			_vm->_gameSys->setAnimation(item->_droppedSequenceId, item->_id, index + 1);
			_vm->_gameSys->insertSequence(item->_droppedSequenceId, item->_id, item->_currSequenceId, item->_id, kSeqSyncWait, 0, item->_x, item->_y);
			item->_currSequenceId = item->_droppedSequenceId;
			item->_y = 0;
		}
		break;

	case 0xBF:
	case 0xC2:
		// Bouncing coin and banknote
		_vm->_gameSys->setAnimation(0, 0, index + 1);
		_vm->_gameSys->removeSequence(item->_currSequenceId, item->_id, true);
		clearItem(item);
		break;

	case 0xBA:
		// Falling banana peel
		item->_droppedSequenceId = 0xBB;
		item->_y = 15;
		if (item->_droppedSequenceId) {
			_vm->_gameSys->setAnimation(item->_droppedSequenceId, item->_id, index + 1);
			_vm->_gameSys->insertSequence(item->_droppedSequenceId, item->_id, item->_currSequenceId, item->_id, kSeqSyncWait, 0, item->_x, item->_y);
			item->_currSequenceId = item->_droppedSequenceId;
			item->_y = 0;
		}
		break;

	case 0xBB:
		item->_isCollision = true;
		item->_droppedSequenceId = 0;
		_vm->_gameSys->setAnimation(0, 0, index + 1);
		break;

	case 0xBC:
		_vm->_gameSys->removeSequence(item->_currSequenceId, item->_id, true);
		_vm->_gameSys->setAnimation(0, 0, index + 1);
		clearItem(item);
		break;

	default:
		if (item->_droppedSequenceId) {
			_vm->_gameSys->setAnimation(item->_droppedSequenceId, item->_id, index + 1);
			_vm->_gameSys->insertSequence(item->_droppedSequenceId, item->_id, item->_currSequenceId, item->_id, kSeqSyncWait, 0, item->_x, item->_y);
			item->_currSequenceId = item->_droppedSequenceId;
			item->_y = 0;
		}
		break;
	}
}

void Scene51::removeCollidedItems() {
	for (int i = 0; i < 6; ++i) {
		if (_items[i]._isCollision) {
			_vm->_gameSys->removeSequence(_items[i]._currSequenceId, _items[i]._id, true);
			_vm->_gameSys->setAnimation(0, 0, i + 1);
			clearItem(&_items[i]);
		}
	}
	_itemsCtr2 = 0;
}

int Scene51::itemIsCaught(Scene51Item *item) {
	if (!item->_canCatch)
		return 0;

	if (isJumpingRight(_platypusJumpSequenceId)) {
		int v4 = getPosRight(_platypusJumpSequenceId) + 97;
		if (item->_collisionX < v4 && v4 - item->_collisionX < 56)
			return 1;
	} else {
		int v2 = getPosLeft(_platypusJumpSequenceId);
		if (item->_collisionX > v2 && item->_collisionX - v2 < 56)
			return 1;
	}

	if (item->_currSequenceId == 0xC1) {
		int v3 = item->_collisionX + 100;
		if (isJumpingRight(_platypusJumpSequenceId)) {
			if (ABS(getPosRight(_platypusJumpSequenceId) + 46 - v3) < 56)
				return 1;
		} else if (ABS(getPosLeft(_platypusJumpSequenceId) + 46 - v3) < 56) {
			return 1;
		}
	}

	return 0;
}

bool Scene51::isJumpingRight(int sequenceId) {
	return sequenceId >= 0x76 && sequenceId <= 0x95;
}

bool Scene51::isJumpingLeft(int sequenceId) {
	return sequenceId >= 0x96 && sequenceId <= 0xB5;
}

bool Scene51::isJumping(int sequenceId) {
	return sequenceId >= 0x76 && sequenceId <= 0xB5;
}

void Scene51::waitForAnim(int animationIndex) {
	while (_vm->_gameSys->getAnimationStatus(animationIndex) != 2 && _vm->_gameDone) {
		updateItemAnimations();
		_vm->gameUpdateTick();
	}
}

int Scene51::getPosRight(int sequenceId) {
	static const int kRightPosTbl[] = {
		131, 159, 178, 195, 203, 219, 238, 254,
		246, 274, 293, 310, 318, 334, 353, 369,
		362, 390, 409, 426, 434, 450, 469, 485,
		477, 505, 524, 541, 549, 565, 584, 600
	};

	if (sequenceId >= 118 && sequenceId <= 149)
		return kRightPosTbl[sequenceId - 118];
	return -1;
}

int Scene51::getPosLeft(int sequenceId) {
	static const int kLeftPosTbl[] = {
		580, 566, 550, 536, 526, 504, 488, 469,
		460, 446, 430, 416, 406, 384, 368, 349,
		342, 328, 312, 298, 288, 266, 250, 231,
		220, 206, 190, 176, 166, 144, 128, 109
	};

	if (sequenceId >= 150 && sequenceId <= 181)
		return kLeftPosTbl[sequenceId - 150];
	return -1;
}

void Scene51::playIntroAnim() {
	int soundCtr = 0;

	_platypusSequenceId = 0x76;
	_platypusNextSequenceId = 0x76;

	for (int i = 0; i < 6; ++i)
		clearItem(&_items[i]);

	_items[0]._currSequenceId = 0xBA;
	_items[0]._x2 = 320;
	_items[0]._x = -42;
	_items[0]._y = 15;
	_items[0]._id = 249;
	_items[0]._isCollision = true;

	_vm->_gameSys->insertSequence(_platypusSequenceId, 256, 0, 0, kSeqNone, 0, -179, 0);
	_vm->_gameSys->insertSequence(0xBA, 249, 0, 0, kSeqNone, 0, _items[0]._x, _items[0]._y);
	_vm->_gameSys->setAnimation(0xBA, 249, 1);
	_vm->_gameSys->setAnimation(_platypusSequenceId, 256, 0);

	while (_platypusSequenceId < 0x80) {
		waitForAnim(0);
		++_platypusNextSequenceId;
		_vm->_gameSys->setAnimation(_platypusNextSequenceId, 256, 0);
		_vm->_gameSys->insertSequence(_platypusNextSequenceId, 256, _platypusSequenceId, 256, kSeqSyncWait, 0, -179, 0);
		_platypusSequenceId = _platypusNextSequenceId;
		++soundCtr;
		if (soundCtr % 4 == 0)
			_vm->playSound(0xD6, false);
	}

	_platypusNextSequenceId = 0x75;

	while (_platypusSequenceId != 0x84) {
		waitForAnim(0);
		++_platypusNextSequenceId;
		int oldSequenceId = _platypusNextSequenceId;
		int v0 = checkCollision(_platypusNextSequenceId);
		_vm->_gameSys->setAnimation(_platypusNextSequenceId, 256, 0);
		_vm->_gameSys->insertSequence(_platypusNextSequenceId, 256, _platypusSequenceId, 256, kSeqSyncWait, 0, v0, 0);
		_platypusSequenceId = _platypusNextSequenceId;
		if (v0) {
			_platypusNextSequenceId = oldSequenceId;
		} else {
			++soundCtr;
			if (soundCtr % 4 == 0)
				_vm->playSound(0xD6, false);
		}
	}
	waitForAnim(0);
}

void Scene51::updateGuyAnimation() {
	if (!_vm->_timers[4]) {
		_vm->_timers[4] = _vm->getRandom(20) + 60;

		switch (_vm->getRandom(5)) {
		case 0:
			_guyNextSequenceId = 0xC3;
			break;
		case 1:
			_guyNextSequenceId = 0xC4;
			break;
		case 2:
			_guyNextSequenceId = 0xC5;
			break;
		case 3:
			_guyNextSequenceId = 0xC6;
			break;
		case 4:
			_guyNextSequenceId = 0xC7;
			break;
		default:
			break;
		}

		_vm->_gameSys->insertSequence(_guyNextSequenceId, 39, _guySequenceId, 39, kSeqSyncWait, 0, 0, 0);
		_guySequenceId = _guyNextSequenceId;
		_guyNextSequenceId = -1;
	}
}

int Scene51::incCashAmount(int sequenceId) {
	switch (sequenceId) {
	case 0xBD:
		_cashAmount += 10;
		break;
	case 0xC0:
	case 0xC1:
		_cashAmount += 100;
		break;
	case 0xB6:
	case 0xB7:
		_cashAmount -= 10 * _vm->getRandom(5) + 50;
		if (_cashAmount < 0)
			_cashAmount = 0;
		break;
	default:
		break;
	}
	if (_cashAmount > 1995)
		_cashAmount = 1995;
	updateCash(_cashAmount);
	return _cashAmount;
}

void Scene51::winMinigame() {
	updateCash(1995);
	_vm->playSound(0xDA, false);
	_vm->delayTicksA(1, 5);
	_vm->_newSceneNum = 48;
	_vm->invRemove(kItemBanana);
}

void Scene51::playCashAppearAnim() {
	_vm->_gameSys->setAnimation(0xC8, 252, 0);
	_vm->_gameSys->insertSequence(0xC8, 252, 0, 0, kSeqNone, 0, -20, -20);

	while (_vm->_gameSys->getAnimationStatus(0) != 2 && !_vm->_gameDone)
		_vm->gameUpdateTick();
}

void Scene51::updateCash(int amount) {
	drawDigit(amount / 1000, 0);
	drawDigit(amount / 100 % 10, 1);
	drawDigit(amount / 10 % 10, 2);
	drawDigit(amount % 10, 3);
}

void Scene51::drawDigit(int digit, int position) {
	if (digit != _digits[position]) {
		_vm->_gameSys->insertSequence(kDigitSequenceIds[digit], 253 + position,
			_digitSequenceIds[position], 253 + position,
			kSeqSyncWait, 0, kDigitPositions[position] - 20, -20);
		_digitSequenceIds[position] = kDigitSequenceIds[digit];
		_digits[position] = digit;
	}
}

void Scene51::initCashDisplay() {
	for (int position = 0; position < 4; ++position) {
		_digits[position] = 0;
		_digitSequenceIds[position] = kDigitSequenceIds[0];
		_vm->_gameSys->insertSequence(kDigitSequenceIds[0], 253 + position, 0, 0, kSeqNone, 0, kDigitPositions[position] - 20, -20);
	}
	_cashAmount = 0;
}

void Scene51::run() {
	int soundCtr = 0;
	bool isIdle = true;

	_itemsCtr = 0;
	_vm->_newSceneNum = _vm->_prevSceneNum;
	_cashAmount = 0;
	_platypusJumpSequenceId = 0x84;
	_vm->endSceneInit();

	_vm->hideCursor();
	_vm->setGrabCursorSprite(-1);

	_guySequenceId = 0xC3;
	_guyNextSequenceId = -1;

	_vm->_gameSys->insertSequence(0xC3, 39, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_timers[4] = _vm->getRandom(20) + 60;

	playCashAppearAnim();
	initCashDisplay();
	playIntroAnim();

	_platypusNextSequenceId = 0x74;
	_vm->_gameSys->setAnimation(0x74, 256, 0);
	_vm->_gameSys->insertSequence(_platypusNextSequenceId, 256, _platypusSequenceId, 256, kSeqSyncWait, 0, getPosRight(_platypusJumpSequenceId) - 362, 0);
	_platypusSequenceId = _platypusNextSequenceId;

	_itemInsertDirection = 0;
	_itemInsertX = 685;
	_dropSpeedTicks = 10;
	_nextDropItemKind = 0;

	for (int i = 0; i < 6; ++i)
		clearItem(&_items[i]);

	_itemInsertX = _vm->getRandom(556) + 129;
	_vm->_timers[0] = 15;

	_itemsCaughtCtr = 0;
	_dropLoseCash = false;
	_itemsCtr1 = 20;

	_vm->clearKeyStatus1(Common::KEYCODE_RIGHT);
	_vm->clearKeyStatus1(Common::KEYCODE_LEFT);
	_vm->clearKeyStatus1(Common::KEYCODE_UP);
	_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
	_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);

	bool isCollision = false;
	bool startWalk = true;

	while (!_vm->_sceneDone) {
		if (clearKeyStatus())
			_vm->_sceneDone = true;

		_vm->gameUpdateTick();

		updateGuyAnimation();
		dropNextItem();
		updateItemAnimations();

		if (_vm->isKeyStatus2(Common::KEYCODE_UP) || _vm->isKeyStatus2(Common::KEYCODE_SPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_UP);
			_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
			if (isJumpingRight(_platypusJumpSequenceId)) {
				waitForAnim(0);
				_vm->_gameSys->setAnimation(0xB8, 256, 0);
				_vm->_gameSys->insertSequence(0xB8, 256, _platypusSequenceId, 256, kSeqSyncWait, 0, getPosRight(_platypusJumpSequenceId) - 348, 0);
				_platypusSequenceId = 0xB8;
				waitForAnim(0);
				_platypusNextSequenceId += 6;
				if (_platypusNextSequenceId > 0x95)
					_platypusNextSequenceId = 0x95;
				_platypusJumpSequenceId = _platypusNextSequenceId;
			} else {
				waitForAnim(0);
				_vm->_gameSys->setAnimation(0xB9, 256, 0);
				_vm->_gameSys->insertSequence(0xB9, 256, _platypusSequenceId, 256, kSeqSyncWait, 0, getPosLeft(_platypusJumpSequenceId) - 338, 0);
				_platypusSequenceId = 0xB9;
				waitForAnim(0);
				_platypusNextSequenceId += 6;
				if (_platypusNextSequenceId > 0xB5)
					_platypusNextSequenceId = 0xB5;
				_platypusJumpSequenceId = _platypusNextSequenceId;
			}
			isIdle = false;
		}

		while (_vm->isKeyStatus2(Common::KEYCODE_RIGHT) && _platypusNextSequenceId != 0x96 && !_vm->_gameDone) {
			if (_platypusNextSequenceId == 0xB6)
				_platypusNextSequenceId = 0x76;
			updateItemAnimations();
			if (startWalk) {
				_platypusNextSequenceId = 0x86;
				startWalk = false;
			}

			if (_vm->_gameSys->getAnimationStatus(0) == 2) {
				int collisionX = checkCollision(_platypusNextSequenceId);
				if (collisionX)
					incCashAmount(_platypusNextSequenceId);
				_vm->_gameSys->setAnimation(_platypusNextSequenceId, 256, 0);
				_vm->_gameSys->insertSequence(_platypusNextSequenceId, 256, _platypusSequenceId, 256, kSeqSyncWait, 0, collisionX, 0);
				_platypusSequenceId = _platypusNextSequenceId;
				if (collisionX) {
					isCollision = true;
					++_platypusJumpSequenceId;
					_platypusNextSequenceId = _platypusJumpSequenceId;
				} else {
					_platypusJumpSequenceId = _platypusNextSequenceId;
				}
				if (isJumpingRight(_platypusJumpSequenceId)) {
					++_platypusNextSequenceId;
					if (!isCollision) {
						if (_vm->isKeyStatus2(Common::KEYCODE_UP) || _vm->isKeyStatus2(Common::KEYCODE_SPACE)) {
							_vm->clearKeyStatus1(Common::KEYCODE_UP);
							_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
							waitForAnim(0);
							_vm->_gameSys->setAnimation(0xB8, 256, 0);
							_vm->_gameSys->insertSequence(0xB8, 256, _platypusSequenceId, 256, kSeqSyncWait, 0, getPosRight(_platypusJumpSequenceId) - 348, 0);
							_platypusSequenceId = 0xB8;
							waitForAnim(0);
							_platypusNextSequenceId += 6;
							if (_platypusNextSequenceId > 0x95)
								_platypusNextSequenceId = 0x95;
							_platypusJumpSequenceId = _platypusNextSequenceId;
						} else {
							++soundCtr;
							if (soundCtr % 4 == 0)
								_vm->playSound(0xD6, false);
						}
					}
				} else {
					_platypusNextSequenceId = 150 - (_platypusJumpSequenceId - 150);
				}
				isCollision = false;
				isIdle = false;
			}
			_vm->gameUpdateTick();
		}

		while (_vm->isKeyStatus2(Common::KEYCODE_LEFT) && _platypusNextSequenceId != 0xB6 && !_vm->_gameDone) {
			updateItemAnimations();
			if (startWalk) {
				_platypusNextSequenceId = 0xA5;
				startWalk = false;
			}

			if (_vm->_gameSys->getAnimationStatus(0) == 2) {
				int collisionX = checkCollision(_platypusNextSequenceId);
				if (collisionX)
					incCashAmount(_platypusNextSequenceId);
				_vm->_gameSys->setAnimation(_platypusNextSequenceId, 256, 0);
				_vm->_gameSys->insertSequence(_platypusNextSequenceId, 256, _platypusSequenceId, 256, kSeqSyncWait, 0, collisionX, 0);
				_platypusSequenceId = _platypusNextSequenceId;
				if (collisionX) {
					isCollision = true;
					++_platypusJumpSequenceId;
					_platypusNextSequenceId = _platypusJumpSequenceId;
				} else {
					_platypusJumpSequenceId = _platypusNextSequenceId;
				}
				if (isJumpingLeft(_platypusJumpSequenceId)) {
					++_platypusNextSequenceId;
					if (!isCollision) {
						if (_vm->isKeyStatus2(Common::KEYCODE_UP) || _vm->isKeyStatus2(Common::KEYCODE_SPACE)) {
							_vm->clearKeyStatus1(Common::KEYCODE_UP);
							_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
							waitForAnim(0);
							_vm->_gameSys->setAnimation(0xB9, 256, 0);
							_vm->_gameSys->insertSequence(0xB9, 256, _platypusSequenceId, 256, kSeqSyncWait, 0, getPosLeft(_platypusJumpSequenceId) - 338, 0);
							_platypusSequenceId = 0xB9;
							waitForAnim(0);
							_platypusNextSequenceId += 6;
							if (_platypusNextSequenceId > 0xB5)
								_platypusNextSequenceId = 0xB5;
							_platypusJumpSequenceId = _platypusNextSequenceId;
						} else {
							++soundCtr;
							if (soundCtr % 4 == 0)
								_vm->playSound(0xD6, false);
						}
					}
				} else {
					_platypusNextSequenceId = 182 - (_platypusJumpSequenceId - 118);
				}
				isCollision = false;
				isIdle = false;
			}
			_vm->gameUpdateTick();
		}

		if (!isIdle && _vm->_gameSys->getAnimationStatus(0) == 2) {
			if (isJumpingRight(_platypusJumpSequenceId)) {
				_vm->_gameSys->setAnimation(0x74, 256, 0);
				_vm->_gameSys->insertSequence(0x74, 256, _platypusSequenceId, 256, kSeqSyncWait, 0, getPosRight(_platypusJumpSequenceId) - 362, 0);
				_platypusSequenceId = 0x74;
			} else {
				_vm->_gameSys->setAnimation(0x75, 256, 0);
				_vm->_gameSys->insertSequence(0x75, 256, _platypusSequenceId, 256, kSeqSyncWait, 0, getPosLeft(_platypusJumpSequenceId) - 341, 0);
				_platypusSequenceId = 0x75;
			}
			waitForAnim(0);
			isIdle = true;
		}
	}

	_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
	_vm->clearKeyStatus1(Common::KEYCODE_UP);
	_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
	_vm->clearKeyStatus1(Common::KEYCODE_RIGHT);
	_vm->clearKeyStatus1(Common::KEYCODE_LEFT);

	_vm->_gameSys->setAnimation(0, 0, 0);
	for (int i = 0; i < 6; ++i)
		_vm->_gameSys->setAnimation(0, 0, i + 1);

	_vm->showCursor();
}

/*****************************************************************************/

Scene52::Scene52(GnapEngine *vm) : Scene(vm) {
	_gameScore = 0;
	_aliensInitialized = false;
	_alienDirection = 0;
	_soundToggle = false;
	_arcadeScreenBottom = 0;
	_shipsLeft = 0;
	_shipPosX = 0;
	_shipCannonPosX = 0;
	_shipCannonPosY = 0;
	_shipCannonFiring = false;
	_shipCannonFired = false;
	_shipCannonWidth = 0;
	_shipCannonHeight = 0;
	_shipCannonTopY = 0;
	_shipMidX = 0;
	_shipMidY = 0;
	_shipFlag = false;
	_alienSpeed = 0;
	_alienWidth = 0;
	_alienHeight = 0;
	_alienLeftX = 0;
	_alienTopY = 0;
	_alienRowDownCtr = 0;
	_alienWave = false;
	_alienSingle = false;
	_alienCounter = 0;
	_bottomAlienFlag = false;
	_aliensCount = 0;
	_nextUfoSequenceId = -1;
	_ufoSequenceId = -1;
	_liveAlienRows = 0;
}

int Scene52::init() {
	initAnims();
	return 0x2B;
}

void Scene52::updateHotspots() {
	_vm->_hotspotsCount = 0;
}

void Scene52::update() {
	for (int rowNum = 0; rowNum < 7 && !_vm->_gameDone; ++rowNum) {
		_vm->gameUpdateTick();
		if (_vm->_gameSys->getAnimationStatus(_alienRowAnims[rowNum]) == 2) {
			updateAlienRow(rowNum);
			rowNum = 0;
		}
	}

	if (_liveAlienRows == 0 && !_alienSingle) {
		_alienWave = false;
		_vm->playSound(0x30, false);
		++_alienCounter;
		if (_alienCounter != 3) {
			_vm->_timers[0] = 50;
			_vm->_timers[2] = 100;
			_alienRowDownCtr = 0;
			_alienSingle = true;
		}
	}

	if (_alienSingle && !_vm->_timers[0]) {
		initAliens();
		_alienSingle = false;
		_vm->_timers[2] = 5;
		_alienWave = true;
	}

	if ((_alienRowDownCtr || _liveAlienRows == 0) && !_alienSingle) {
		moveDownAlienRow();
		_alienRowDownCtr = 0;
	}

	if (_vm->isKeyStatus1(Common::KEYCODE_UP) || _vm->isKeyStatus1(Common::KEYCODE_SPACE)) {
		_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
		_vm->clearKeyStatus1(Common::KEYCODE_UP);
		if (!_aliensCount)
			fireShipCannon(_shipPosX);
	}

	if (_shipCannonFiring)
		updateShipCannon();

	fireAlienCannon();
	updateAlienCannons();

	if (_aliensCount == 1) {
		_alienWave = false;
		_vm->_timers[3] = 20;
		_vm->_timers[2] = 100;
		++_aliensCount;
	}

	if (_aliensCount && !_vm->_timers[3]) {
		updateAliens();
		loseShip();
		if (_shipsLeft != 0) {
			_vm->_timers[3] = 40;
			while (_vm->_timers[3] && !_vm->_gameDone) {
				updateAlienCannons();
				if (_shipCannonFiring)
					updateShipCannon();
				_vm->gameUpdateTick();
			}
			initAliens();
			_shipPosX = (800 - _shipMidX) / 2;
			_vm->_gameSys->setAnimation(_nextUfoSequenceId, 256, 7);
			_vm->_gameSys->insertSequence(_nextUfoSequenceId, 256, 0, 0, kSeqNone, 0, _shipPosX, _arcadeScreenBottom);
			_ufoSequenceId = _nextUfoSequenceId;
			_vm->_timers[2] = 5;
			_alienWave = true;
		} else {
			_vm->_sceneDone = true;
		}
	}

	_nextUfoSequenceId = 34;
	if (_ufoSequenceId != 34)
		_shipFlag = true;

	if (_shipFlag) {
		if (_vm->_gameSys->getAnimationStatus(7) == 2) {
			_vm->_gameSys->setAnimation(_nextUfoSequenceId, 256, 7);
			_vm->_gameSys->insertSequence(_nextUfoSequenceId, 256, _ufoSequenceId, 256, kSeqSyncWait, 0, _shipPosX, _arcadeScreenBottom);
			_ufoSequenceId = _nextUfoSequenceId;
		}
		_shipFlag = false;
	}

	if (_alienWave && !_vm->_timers[0]) {
		playSound();
		int delay = CLIP(_alienSpeed, 2, 10);
		_vm->_timers[0] = delay;
	}
}

void Scene52::initShipCannon(int bottomY) {
	_shipCannonFired = false;
	_shipCannonWidth = MAX(_vm->_gameSys->getSpriteWidthById(14), _vm->_gameSys->getSpriteWidthById(16));
	_shipCannonHeight = MAX(_vm->_gameSys->getSpriteHeightById(14), _vm->_gameSys->getSpriteHeightById(16));
	_shipCannonTopY = bottomY - _shipCannonHeight;
	_shipCannonFiring = false;
}

void Scene52::initAlienCannons() {
	for (int i = 0; i < 3; ++i) {
		_alienCannonIds[i] = 0;
		_alienCannonFired[i] = 0;
	}
	_alienCannonSequenceIds[0] = 30;
	_alienCannonSequenceIds[1] = 31;
	_alienCannonSequenceIds[2] = 32;
}

void Scene52::fireShipCannon(int posX) {
	if (_vm->_timers[1])
		return;

	int cannonNum = getFreeShipCannon();
	if (cannonNum != -1) {
		_shipCannonPosX = _shipMidX / 2 + posX - _shipCannonWidth / 2;
		_shipCannonPosY = _shipCannonTopY;
		_vm->_gameSys->setAnimation(0x23, cannonNum + 256, cannonNum + 8);
		_vm->_gameSys->insertSequence(0x23, cannonNum + 256, 0, 0, kSeqNone, 0, _shipCannonPosX, _shipCannonPosY);
		_vm->playSound(0x2D, false);
		if (shipCannonHitShield(cannonNum)) {
			_vm->_gameSys->setAnimation(0, 0, cannonNum + 8);
			_vm->_gameSys->removeSequence(0x23, cannonNum + 256, true);
		} else {
			_shipCannonFired = true;
			_shipCannonPosY -= 13;
			_shipCannonFiring = true;
		}
		_vm->_timers[1] = 5;
	}
}

void Scene52::fireAlienCannon() {
	if (_vm->_timers[2])
		return;

	int cannonNum = getFreeAlienCannon();
	if (cannonNum != -1) {
		int alienX1 = _alienLeftX + _alienRowXOfs[0];
		int alienX2 = _alienLeftX + _alienRowXOfs[0] + 5 * _alienWidth - (_alienWidth / 2 - 15);
		_alienCannonPosX[cannonNum] = _vm->getRandom(alienX2 - alienX1) + alienX1;
		_alienCannonPosY[cannonNum] = 104;
		_alienCannonFired[cannonNum] = 1;
		_vm->_gameSys->setAnimation(_alienCannonSequenceIds[cannonNum], _alienCannonIds[cannonNum] + 256, cannonNum + 9);
		_vm->_gameSys->insertSequence(_alienCannonSequenceIds[cannonNum], _alienCannonIds[cannonNum] + 256, 0, 0,
			kSeqNone, 0, _alienCannonPosX[cannonNum], _alienCannonPosY[cannonNum]);
		_alienCannonPosY[cannonNum] -= 13;
		_vm->_timers[2] = 5;
	}
}

int Scene52::getFreeShipCannon() {
	if (!_shipCannonFired)
		return 0;
	return -1;
}

int Scene52::getFreeAlienCannon() {
	for (int i = 0; i < 3; ++i)
		if (!_alienCannonFired[i])
			return i;
	return -1;
}

void Scene52::updateShipCannon() {
	if (_shipCannonFired && _vm->_gameSys->getAnimationStatus(8) == 2) {
		_shipCannonPosY -= 13;
		if (_shipCannonPosY - 13 >= 135) {
			if (updateHitAlien()) {
				_vm->_gameSys->setAnimation(0, 0, 8);
				_vm->_gameSys->removeSequence(35, 256, true);
				_shipCannonFired = false;
				drawScore(_gameScore);
			} else {
				_vm->_gameSys->setAnimation(35, 256, 8);
				_vm->_gameSys->insertSequence(35, 256, 35, 256, kSeqSyncWait, 0, _shipCannonPosX, _shipCannonPosY);
				_shipCannonPosY -= 13;
			}
		} else {
			_vm->_gameSys->setAnimation(0, 0, 8);
			_vm->_gameSys->removeSequence(35, 256, true);
			_shipCannonFired = false;
		}
	}
}

void Scene52::updateAlienCannons() {
	for (int i = 0; i < 3; ++i) {
		if (_alienCannonFired[i] && _vm->_gameSys->getAnimationStatus(i + 9) == 2) {
			_alienCannonPosY[i] += 13;
			if (_shipCannonHeight + _alienCannonPosY[i] + 13 <= 550) {
				if (alienCannonHitShip(i)) {
					_vm->_gameSys->setAnimation(0, 0, i + 9);
					_alienCannonFired[i] = 0;
					shipExplode();
				} else if (alienCannonHitShield(i)) {
					_alienCannonFired[i] = 0;
				} else {
					_vm->_gameSys->insertSequence(_alienCannonSequenceIds[i], 1 - _alienCannonIds[i] + 256, 0, 0,
						kSeqNone, 0, _alienCannonPosX[i], _alienCannonPosY[i]);
					_vm->_gameSys->setAnimation(_alienCannonSequenceIds[i], 1 - _alienCannonIds[i] + 256, i + 9);
					_alienCannonIds[i] = 1 - _alienCannonIds[i];
					_alienCannonPosY[i] += 13;
				}
			} else {
				_vm->_gameSys->setAnimation(0, 0, i + 9);
				_alienCannonFired[i] = 0;
			}
		}
	}
}

void Scene52::initAliens() {
	if (!_aliensInitialized) {
		initAlienSize();
		_aliensInitialized = true;
	}

	_liveAlienRows = 0;
	_alienSpeed = 0;
	_bottomAlienFlag = false;
	_aliensCount = 0;
	_alienSingle = false;
	_alienRowDownCtr = 0;

	initShields();

	_alienRowKind[0] = -1;
	_alienRowKind[1] = -1;
	_alienRowKind[2] = -1;
	_alienRowKind[3] = -1;
	_alienRowKind[4] = _vm->getRandom(2) != 0 ? 24 : 27;
	_alienRowKind[5] = _vm->getRandom(2) != 0 ? 25 : 28;
	_alienRowKind[6] = _vm->getRandom(2) != 0 ? 26 : 29;

	for (int i = 0; i < 7; ++i) {
		_alienRowAnims[i] = i;
		_alienRowXOfs[i] = 0;
		initAlienRowKind(i, _alienRowKind[i]);
		insertAlienRow(i);
	}
}

void Scene52::initAlienRowKind(int rowNum, int alienKind) {
	for (int i = 0; i < 5; ++i)
		_items[rowNum][i] = alienKind;
}

void Scene52::insertAlienRow(int rowNum) {
	if (_alienRowKind[rowNum] >= 0) {
		insertAlienRowAliens(rowNum);
		_alienRowIds[rowNum] = 256;
		_vm->_gameSys->setAnimation(_alienRowKind[rowNum], _alienRowIds[rowNum], _alienRowAnims[rowNum]);
		++_liveAlienRows;
	}
}

void Scene52::insertAlienRowAliens(int rowNum) {
	int xOffs = _alienLeftX;
	int yOffs = _alienTopY - 52 * rowNum - _alienHeight + 10;
	for (int i = 0; i < 5; ++i) {
		if (_items[rowNum][i] >= 0) {
			_vm->_gameSys->insertSequence(_items[rowNum][i], i + 256, 0, 0, kSeqNone, 0, xOffs, yOffs);
			++_alienSpeed;
		}
		xOffs += _alienWidth;
	}
}

void Scene52::updateAlienRow(int rowNum) {
	if (_alienRowKind[rowNum] != -1 && !checkAlienRow(rowNum)) {
		updateAlienRowXOfs();
		_alienRowIds[rowNum] = -1;
		int xOffs = _alienLeftX + _alienRowXOfs[rowNum];
		int yOffs = _alienTopY - 52 * rowNum - _alienHeight + 10;
		for (int i = 0; i < 5; ++i) {
			if (_items[rowNum][i] >= 0) {
				_vm->_gameSys->insertSequence(_items[rowNum][i], i + 256, _items[rowNum][i], i + 256, kSeqSyncWait, 0, xOffs, yOffs);
				if (_alienRowIds[rowNum] == -1)
					_alienRowIds[rowNum] = i + 256;
			} else if (_items[rowNum][i] == -2) {
				_vm->_gameSys->removeSequence(_alienRowKind[rowNum], i + 256, true);
				_items[rowNum][i] = -1;
				--_alienSpeed;
			}
			xOffs += _alienWidth;
		}
		if (_alienRowIds[rowNum] == -1) {
			_vm->_gameSys->setAnimation(0, 0, _alienRowAnims[rowNum]);
			// MessageBoxA(0, "No live aliens!", "Error 3:", 0x30u);
		} else {
			_vm->_gameSys->setAnimation(_alienRowKind[rowNum], _alienRowIds[rowNum], _alienRowAnims[rowNum]);
		}
		if (rowNum == 1) {
			for (int j = 0; j < 3; ++j) {
				if (_shieldSpriteIds[j] != -1) {
					_vm->_gameSys->fillSurface(nullptr, _shieldPosX[j], _arcadeScreenBottom - 44, 33, 44, 0, 0, 0);
					_shieldSpriteIds[j] = -1;
				}
			}
		}
		if (rowNum == 0 && _bottomAlienFlag)
			shipExplode();
	}
}

void Scene52::moveDownAlienRow() {
	int v2[5], v3, v1, v0, v4;

	for (int i = 0; i < 5; ++i)
		v2[i] = _items[0][i];

	v3 = _alienRowIds[0];
	v1 = _alienRowAnims[0];
	v0 = _alienRowKind[0];
	v4 = _alienRowXOfs[0];

	for (int j = 0; j < 7; ++j) {
		for (int i = 0; i < 5; ++i)
			_items[j][i] = _items[j + 1][i];
		_alienRowIds[j] = _alienRowIds[j + 1];
		_alienRowAnims[j] = _alienRowAnims[j + 1];
		_alienRowKind[j] = _alienRowKind[j + 1];
		_alienRowXOfs[j] = _alienRowXOfs[j + 1];
	}

	for (int i = 0; i < 5; ++i)
		_items[6][i] = v2[i];

	_alienRowIds[6] = v3;
	_alienRowAnims[6] = v1;
	_alienRowKind[6] = v0;
	_alienRowXOfs[6] = v4;

	updateAlien(6);
	initAlienRowKind(6, _alienRowKind[6]);
	insertAlienRow(6);

	_bottomAlienFlag = _alienRowKind[0] > -1;
}

int Scene52::updateHitAlien() {
	int result = 0, rowNum, ya;

	int y = _shipCannonTopY - _shipCannonPosY;

	if (y == 26) {
		rowNum = 1;
		ya = _shipCannonPosY + 26;
	} else {
		if (y % 52)
			return 0;
		rowNum = y / 52 + 1;
		ya = _shipCannonPosY;
	}

	if (rowNum < 7) {
		int hitAlienNum = getHitAlienNum(rowNum);
		if (hitAlienNum != -1 && _items[rowNum][hitAlienNum] >= 0) {
			_gameScore = ((_items[rowNum][hitAlienNum] - 24) % 3 + _gameScore + 1) % 1000;
			_items[rowNum][hitAlienNum] = -2;
			_vm->playSound(0x2C, false);
			_vm->_gameSys->insertSequence(0x21, 266, 0, 0,
				kSeqNone, 0, _alienLeftX + hitAlienNum * _alienWidth + _alienRowXOfs[rowNum] - 10, ya - _alienHeight);
			result = 1;
		}
	}

	return result;
}

int Scene52::getHitAlienNum(int rowNum) {
	int result = -1;

	int v3 = _alienLeftX + _alienRowXOfs[rowNum];

	if (_shipCannonPosX >= v3) {
		int v8 = _alienWidth / 2 - 15;
		if (v3 + 5 * _alienWidth - v8 >= _shipCannonPosX) {
			int v4 = v3 + _alienWidth;
			if (_shipCannonPosX >= v4 - v8) {
				int v5 = v4 + _alienWidth;
				if (_shipCannonPosX >= v5 - v8) {
					int v6 = v5 + _alienWidth;
					if (_shipCannonPosX >= v6 - v8) {
						int v7 = v6 + _alienWidth;
						if (_shipCannonPosX >= v7 - v8) {
							if (_shipCannonPosX >= v7 + _alienWidth - v8)
								result = -1;
							else
								result = 4;
						} else {
							result = 3;
						}
					} else {
						result = 2;
					}
				} else {
					result = 1;
				}
			} else {
				result = 0;
			}
		} else {
			result = -1;
		}
	} else {
		result = -1;
	}
	return result;
}

int Scene52::alienCannonHitShip(int cannonNum) {
	int result = 0;

	if (_aliensCount) {
		result = 0;
	} else {
		int cannonY = _alienCannonPosY[cannonNum] - 13;
		if (_arcadeScreenBottom <= cannonY) {
			if (_shipMidY + _arcadeScreenBottom > cannonY) {
				if (_alienCannonPosX[cannonNum] >= _shipPosX)
					result = _alienCannonPosX[cannonNum] < _shipMidX + _shipPosX;
				else
					result = 0;
			} else {
				result = 0;
			}
		} else {
			result = 0;
		}
	}
	return result;
}

int Scene52::alienCannonHitShield(int cannonNum) {
	int result = 0;

	int v3 = _alienCannonPosY[cannonNum] + 39;
	if (_arcadeScreenBottom - 44 > v3)
		return 0;

	if (_arcadeScreenBottom <= v3)
		return 0;

	if (_alienCannonPosX[cannonNum] < _shieldPosX[0])
		return 0;

	if (_alienCannonPosX[cannonNum] > _shieldPosX[2] + 33)
		return 0;

	int shieldNum = -1;
	if (_alienCannonPosX[cannonNum] < _shieldPosX[0] + 33)
		shieldNum = 0;

	if (shieldNum < 0 && _alienCannonPosX[cannonNum] < _shieldPosX[1])
		return 0;

	if (shieldNum < 0 && _alienCannonPosX[cannonNum] < _shieldPosX[1] + 33)
		shieldNum = 1;

	if (shieldNum < 0) {
		if (_alienCannonPosX[cannonNum] < _shieldPosX[2])
			return 0;
		shieldNum = 2;
	}

	if (_shieldSpriteIds[shieldNum] == -1) {
		result = 0;
	} else {
		++_shieldSpriteIds[shieldNum];
		if (_shieldSpriteIds[shieldNum] <= 21) {
			_vm->_gameSys->drawSpriteToBackground(_shieldPosX[shieldNum], _arcadeScreenBottom - 44, _shieldSpriteIds[shieldNum]);
		} else {
			_vm->_gameSys->fillSurface(nullptr, _shieldPosX[shieldNum], _arcadeScreenBottom - 44, 33, 44, 0, 0, 0);
			_shieldSpriteIds[shieldNum] = -1;
		}
		_vm->_gameSys->setAnimation(0, 0, cannonNum + 9);
		_vm->_gameSys->insertSequence(0x21, shieldNum + 257, 0, 0, kSeqNone, 0, _alienCannonPosX[cannonNum] - 18, _arcadeScreenBottom - 44);
		_vm->playSound(0x2C, false);
		result = 1;
	}

	return result;
}

bool Scene52::shipCannonHitShield(int cannonNum) {
	bool result = false;

	if (_shipCannonPosX < _shieldPosX[0])
		return result;

	if (_shipCannonPosX > _shieldPosX[2] + 33)
		return result;

	int shieldNum = -1;
	if (_shipCannonPosX < _shieldPosX[0] + 33)
		shieldNum = 0;

	if (shieldNum < 0 && _shipCannonPosX < _shieldPosX[1])
		return result;

	if (shieldNum < 0 && _shipCannonPosX < _shieldPosX[1] + 33)
		shieldNum = 1;

	if (shieldNum < 0) {
		if (_shipCannonPosX < _shieldPosX[2])
			return result;
		shieldNum = 2;
	}

	if (_shieldSpriteIds[shieldNum] == -1) {
		result = false;
	} else {
		++_shieldSpriteIds[shieldNum];
		if (_shieldSpriteIds[shieldNum] <= 21) {
			_vm->_gameSys->drawSpriteToBackground(_shieldPosX[shieldNum], _arcadeScreenBottom - 44, _shieldSpriteIds[shieldNum]);
		} else {
			_vm->_gameSys->fillSurface(nullptr, _shieldPosX[shieldNum], _arcadeScreenBottom - 44, 33, 44, 0, 0, 0);
			_shieldSpriteIds[shieldNum] = -1;
		}
		_vm->_gameSys->insertSequence(0x21, shieldNum + 257, 0, 0, kSeqNone, 0, _shipCannonPosX - 18, _arcadeScreenBottom - 44);
		_vm->playSound(0x2C, false);
		result = true;
	}

	return result;
}

bool Scene52::shipCannonHitAlien() {
	bool result = false;

	if (_aliensCount || checkAlienRow(0))
		return false;

	int alienNextX = _alienLeftX + _alienRowXOfs[0];
	if (_shipMidX + _shipPosX >= alienNextX) {
		int startX = _alienWidth / 2 - 15;
		if (alienNextX + 5 * _alienWidth - startX >= _shipPosX) {
			int alienNextDeltaX = alienNextX + _alienWidth;
			if (_items[0][0] <= -1 || alienNextDeltaX - startX <= _shipPosX) {
				alienNextDeltaX += _alienWidth;
				if (_items[0][1] <= -1 || alienNextDeltaX - startX <= _shipPosX) {
					alienNextDeltaX += _alienWidth;
					if (_items[0][2] <= -1 || alienNextDeltaX - startX <= _shipPosX) {
						alienNextDeltaX += _alienWidth;
						if (_items[0][3] <= -1 || alienNextDeltaX - startX <= _shipPosX) {
							alienNextDeltaX += _alienWidth;
							result = _items[0][4] > -1 && alienNextDeltaX - startX > _shipPosX;
						} else {
							result = true;
						}
					} else {
						result = true;
					}
				} else {
					result = true;
				}
			} else {
				result = true;
			}
		} else {
			result = false;
		}
	} else {
		result = false;
	}

	return result;
}

void Scene52::shipExplode() {
	if (!_aliensCount) {
		_vm->_gameSys->setAnimation(0, 0, 7);
		_vm->_gameSys->removeSequence(_ufoSequenceId, 256, true);
		_vm->playSound(0x2C, false);
		_vm->_gameSys->insertSequence(0x21, 266, 0, 0, kSeqNone, 0, _shipPosX, _arcadeScreenBottom);
		_aliensCount = 1;
		_vm->playSound(0x31, false);
	}
}

bool Scene52::checkAlienRow(int rowNum) {
	for (int i = 0; i < 5; ++i) {
		if (_items[rowNum][i] >= 0)
			return false;
	}

	bool found = false;
	for (int j = 0; j < 5; ++j)
		if (_items[rowNum][j] == -2) {
			_vm->_gameSys->removeSequence(_alienRowKind[rowNum], j + 256, true);
			_items[rowNum][j] = -1;
			--_alienSpeed;
			found = true;
		}

	if (found) {
		_vm->_gameSys->setAnimation(0, 0, _alienRowAnims[rowNum]);
		--_liveAlienRows;
	}

	if (_liveAlienRows < 0)
		_liveAlienRows = 0;

	return true;
}

void Scene52::updateAlienRowXOfs() {
	int amount = 2 * (3 - _liveAlienRows) + 1;

	if (_alienSpeed == 2)
		amount *= 4;
	else if (_alienSpeed == 1)
		amount *= 10;

	if (_alienDirection) {
		for (int i = 0; i < 7; ++i) {
			_alienRowXOfs[i] -= amount;
			if (_alienRowXOfs[i] <= -100) {
				_alienRowXOfs[i] = -100;
				_alienDirection = 0;
				++_alienRowDownCtr;
			}
		}
	} else {
		for (int j = 0; j < 7; ++j) {
			_alienRowXOfs[j] += amount;
			if (_alienRowXOfs[j] >= 100) {
				_alienRowXOfs[j] = 100;
				_alienDirection = 1;
				++_alienRowDownCtr;
			}
		}
	}
}

void Scene52::initAlienSize() {
	_alienWidth = _vm->_gameSys->getSpriteWidthById(0);
	if (_vm->_gameSys->getSpriteWidthById(1) > _alienWidth)
		_alienWidth = _vm->_gameSys->getSpriteWidthById(1);
	if (_vm->_gameSys->getSpriteWidthById(4) > _alienWidth)
		_alienWidth = _vm->_gameSys->getSpriteWidthById(4);
	if (_vm->_gameSys->getSpriteWidthById(5) > _alienWidth)
		_alienWidth = _vm->_gameSys->getSpriteWidthById(5);
	if (_vm->_gameSys->getSpriteWidthById(12) > _alienWidth)
		_alienWidth = _vm->_gameSys->getSpriteWidthById(12);
	if (_vm->_gameSys->getSpriteWidthById(13) > _alienWidth)
		_alienWidth = _vm->_gameSys->getSpriteWidthById(13);

	_alienHeight = _vm->_gameSys->getSpriteHeightById(0);
	if (_vm->_gameSys->getSpriteHeightById(1) > _alienHeight)
		_alienHeight = _vm->_gameSys->getSpriteHeightById(1);
	if (_vm->_gameSys->getSpriteHeightById(4) > _alienHeight)
		_alienHeight = _vm->_gameSys->getSpriteHeightById(4);
	if (_vm->_gameSys->getSpriteHeightById(5) > _alienHeight)
		_alienHeight = _vm->_gameSys->getSpriteHeightById(5);
	if (_vm->_gameSys->getSpriteHeightById(12) > _alienHeight)
		_alienHeight = _vm->_gameSys->getSpriteHeightById(12);
	if (_vm->_gameSys->getSpriteHeightById(13) > _alienHeight)
		_alienHeight = _vm->_gameSys->getSpriteHeightById(13);

	_alienTopY = _shipCannonTopY + 52;
	_alienLeftX = (800 - 5 * _alienWidth) / 2;
}

void Scene52::playSound() {
	if (_soundToggle) {
		_vm->playSound(0x2F, false);
		_soundToggle = false;
	} else {
		_vm->playSound(0x2E, false);
		_soundToggle = true;
	}
}

void Scene52::updateAliens() {
	for (int i = 0; i < 7; ++i)
		updateAlien(i);
}

void Scene52::updateAlien(int rowNum) {
	if (_alienRowKind[rowNum] >= 0 && !checkAlienRow(rowNum)) {
		for (int i = 0; i < 5; ++i) {
			if (_items[rowNum][i] >= 0)
				_items[rowNum][i] = -2;
		}
		checkAlienRow(rowNum);
	}
}

void Scene52::loseShip() {
	--_shipsLeft;
	if (_shipsLeft == 2) {
		_vm->_gameSys->fillSurface(nullptr, 120, 140, _shipMidX, _shipMidY, 0, 0, 0);
	} else if (_shipsLeft == 1) {
		_vm->_gameSys->fillSurface(nullptr, 120, 185, _shipMidX, _shipMidY, 0, 0, 0);
	}
}

void Scene52::initShields() {
	for (int i = 0; i < 3; ++i) {
		_vm->_gameSys->drawSpriteToBackground(_shieldPosX[i], _arcadeScreenBottom - 44, 17);
		_shieldSpriteIds[i] = 17;
	}
}

void Scene52::initAnims() {
	for (int i = 0; i < 7; ++i)
		_vm->_gameSys->setAnimation(0, 0, i);
	_vm->_gameSys->setAnimation(0, 0, 7);
	for (int j = 0; j < 1; ++j)
		_vm->_gameSys->setAnimation(0, 0, j + 8);
	for (int k = 0; k < 3; ++k)
		_vm->_gameSys->setAnimation(0, 0, k + 9);
}

void Scene52::drawScore(int score) {
	char str[4];
	sprintf(str, "%03d", score);
	_vm->_gameSys->fillSurface(nullptr, 420, 80, 48, 30, 0, 0, 0);
	_vm->_gameSys->drawTextToSurface(nullptr, 420, 80, 255, 255, 255, str);
}

void Scene52::run() {
	_vm->_timers[1] = 0;

	_vm->hideCursor();

	_gameScore = 0;
	_vm->_gameSys->drawTextToSurface(nullptr, 300, 80, 255, 255, 255, "SCORE");
	_vm->_gameSys->drawTextToSurface(nullptr, 468, 80, 255, 255, 255, "0");

	drawScore(0);

	_shipMidX = 33;
	_shipMidY = _vm->_gameSys->getSpriteHeightById(15);
	_shipPosX = (800 - _shipMidX) / 2;
	_arcadeScreenBottom = 496;
	int arcadeScreenRight = 595 - _shipMidX;
	int arcadeScreenLeft = 210;
	_shipsLeft = 3;
	_alienCounter = 0;

	_shieldPosX[0] = 247;
	_shieldPosX[1] = 387;
	_shieldPosX[2] = 525;

	for (int i = 0; i < 3; ++i)
		_shieldSpriteIds[i] = -1;

	_vm->_gameSys->drawSpriteToBackground(120, 140, 0xF);
	_vm->_gameSys->drawSpriteToBackground(120, 185, 0xF);

	initShipCannon(_arcadeScreenBottom);
	initAlienCannons();
	initAliens();

	_nextUfoSequenceId = 0x22;
	_vm->_gameSys->setAnimation(0x22, 256, 7);
	_vm->_gameSys->insertSequence(_nextUfoSequenceId, 256, 0, 0, kSeqNone, 0, _shipPosX, _arcadeScreenBottom);

	_ufoSequenceId = _nextUfoSequenceId;

	_vm->clearKeyStatus1(Common::KEYCODE_RIGHT);
	_vm->clearKeyStatus1(Common::KEYCODE_LEFT);
	_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
	_vm->clearKeyStatus1(Common::KEYCODE_UP);
	_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);

	_vm->_timers[2] = 5;
	_shipFlag = false;

	_vm->_timers[0] = 10;
	_alienWave = true;

	while (!_vm->_sceneDone) {
		_vm->gameUpdateTick();

		while (_vm->isKeyStatus2(Common::KEYCODE_RIGHT)) {
			update();
			if (_vm->_gameSys->getAnimationStatus(7) == 2) {
				if (_shipPosX < arcadeScreenRight) {
					_shipPosX += 15;
					if (_shipPosX > arcadeScreenRight)
						_shipPosX = arcadeScreenRight;
					_vm->_gameSys->setAnimation(_nextUfoSequenceId, 256, 7);
					_vm->_gameSys->insertSequence(_nextUfoSequenceId, 256, _ufoSequenceId, 256, kSeqSyncWait, 0, _shipPosX, _arcadeScreenBottom);
					_ufoSequenceId = _nextUfoSequenceId;
					if (_bottomAlienFlag && shipCannonHitAlien())
						shipExplode();
				}
				break;
			}
		}

		while (_vm->isKeyStatus2(Common::KEYCODE_LEFT)) {
			update();
			if (_vm->_gameSys->getAnimationStatus(7) == 2) {
				if (_shipPosX > arcadeScreenLeft) {
					_shipPosX -= 15;
					if (_shipPosX < arcadeScreenLeft)
						_shipPosX = arcadeScreenLeft;
					_vm->_gameSys->setAnimation(_nextUfoSequenceId, 256, 7);
					_vm->_gameSys->insertSequence(_nextUfoSequenceId, 256, _ufoSequenceId, 256, kSeqSyncWait, 0, _shipPosX, _arcadeScreenBottom);
					_ufoSequenceId = _nextUfoSequenceId;
					if (_bottomAlienFlag && shipCannonHitAlien())
						shipExplode();
				}
				break;
			}
		}

		update();

		if (clearKeyStatus()) {
			_alienWave = false;
			_vm->_gameSys->waitForUpdate();
			initAnims();
			_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
			_vm->_sceneDone = true;
		}
	}
	_vm->_gameSys->waitForUpdate();
}

} // End of namespace Gnap
