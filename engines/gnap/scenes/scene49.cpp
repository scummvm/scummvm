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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"
#include "gnap/scenes/scene49.h"

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
	_s49_scoreBarFlash = false;
	_s49_scoreBarPos = -1;
	_s49_scoreLevel = -1;
	_s49_obstacleIndex = -1;
	_s49_truckSequenceId = -1;
	_s49_truckId = -1;
	_s49_truckLaneNum = -1;

	for (int i = 0; i < 5; i++) {
		_s49_obstacles[i]._currSequenceId = -1;
		_s49_obstacles[i]._closerSequenceId = -1;
		_s49_obstacles[i]._passedSequenceId = -1;
		_s49_obstacles[i]._splashSequenceId = -1;
		_s49_obstacles[i]._collisionSequenceId = -1;
		_s49_obstacles[i]._prevId = -1;
		_s49_obstacles[i]._currId = -1;
		_s49_obstacles[i]._laneNum = -1;
	}
}

int Scene49::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	for (int i = 0; i < 5; ++i)
		_vm->_gameSys->setAnimation(0, 0, i + 2);
	_vm->_timers[2] = 0;
	_vm->_timers[0] = 0;
	_vm->_timers[1] = 0;
	_vm->clearKeyStatus1(28);
	_vm->clearKeyStatus1(54);
	_vm->clearKeyStatus1(52);
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
			if (_s49_obstacles[j]._currSequenceId == 0) {
				_vm->_timers[3] = 35;
				_s49_obstacles[j]._currSequenceId = kObstacleDefs[_s49_obstacleIndex]._sequenceId;
				switch (_s49_obstacles[j]._currSequenceId) {
				case 0xB4:
					_s49_obstacles[j]._laneNum = 1;
					_s49_obstacles[j]._closerSequenceId = 180;
					_s49_obstacles[j]._passedSequenceId = 181;
					_s49_obstacles[j]._splashSequenceId = 182;
					_s49_obstacles[j]._collisionSequenceId = 192;
					break;
				case 0xB7:
					_s49_obstacles[j]._laneNum = 2;
					_s49_obstacles[j]._closerSequenceId = 183;
					_s49_obstacles[j]._passedSequenceId = 184;
					_s49_obstacles[j]._splashSequenceId = 185;
					_s49_obstacles[j]._collisionSequenceId = 193;
					break;
				case 0xBD:
					_s49_obstacles[j]._laneNum = 3;
					_s49_obstacles[j]._closerSequenceId = 189;
					_s49_obstacles[j]._passedSequenceId = 190;
					_s49_obstacles[j]._splashSequenceId = 191;
					_s49_obstacles[j]._collisionSequenceId = 195;
					break;
				case 0xBA:
					_s49_obstacles[j]._laneNum = 2;
					_s49_obstacles[j]._closerSequenceId = 186;
					_s49_obstacles[j]._passedSequenceId = 187;
					_s49_obstacles[j]._splashSequenceId = 188;
					_s49_obstacles[j]._collisionSequenceId = 194;
					break;
				case 0xCB:
					_s49_obstacles[j]._laneNum = 1;
					_s49_obstacles[j]._closerSequenceId = 203;
					_s49_obstacles[j]._passedSequenceId = 204;
					_s49_obstacles[j]._splashSequenceId = 0;
					_s49_obstacles[j]._collisionSequenceId = 209;
					break;
				case 0xCD:
					_s49_obstacles[j]._laneNum = 2;
					_s49_obstacles[j]._closerSequenceId = 205;
					_s49_obstacles[j]._passedSequenceId = 206;
					_s49_obstacles[j]._splashSequenceId = 0;
					_s49_obstacles[j]._collisionSequenceId = 210;
					break;
				case 0xCF:
					_s49_obstacles[j]._laneNum = 3;
					_s49_obstacles[j]._closerSequenceId = 207;
					_s49_obstacles[j]._passedSequenceId = 208;
					_s49_obstacles[j]._splashSequenceId = 0;
					_s49_obstacles[j]._collisionSequenceId = 211;
					break;
				}
				_s49_obstacles[j]._prevId = _s49_truckId;
				_s49_obstacles[j]._currId = _s49_obstacles[j]._prevId;
				_vm->_gameSys->setAnimation(_s49_obstacles[j]._currSequenceId, _s49_obstacles[j]._currId, j + 2);
				_vm->_gameSys->insertSequence(_s49_obstacles[j]._currSequenceId, _s49_obstacles[j]._currId, 0, 0, kSeqNone, 0, 0, -50);
				_vm->_timers[2] = kObstacleDefs[_s49_obstacleIndex]._ticks;
				++_s49_obstacleIndex;
				if (_s49_obstacleIndex == 50)
					_s49_obstacleIndex = 0;
				break;
			}
		}

	}

}

void Scene49::updateObstacle(int id) {
	Scene49Obstacle &obstacle = _s49_obstacles[id];
	obstacle._currId = obstacle._prevId;

	switch (obstacle._laneNum) {
	case 1:
		obstacle._prevId = _s49_truckId + 1;
		break;
	case 2:
		if (_s49_truckLaneNum != 2 && _s49_truckLaneNum != 3)
			obstacle._prevId = _s49_truckId - 1;
		else
			obstacle._prevId = _s49_truckId + 1;
		break;
	case 3:
		if (_s49_truckLaneNum != 1 && _s49_truckLaneNum != 2)
			obstacle._prevId = _s49_truckId;
		else
			obstacle._prevId = _s49_truckId - 1;
		break;
	}

	if (obstacle._currSequenceId == obstacle._closerSequenceId) {
		if (_s49_truckLaneNum == obstacle._laneNum) {
			if (obstacle._splashSequenceId) {
				_vm->_gameSys->setAnimation(obstacle._collisionSequenceId, obstacle._prevId, id + 2);
				_vm->_gameSys->insertSequence(obstacle._collisionSequenceId, obstacle._prevId,
					obstacle._currSequenceId, obstacle._currId,
					kSeqSyncWait, 0, 0, -50);
				obstacle._currSequenceId = obstacle._collisionSequenceId;
				_vm->playSound(224, false);
				increaseScore(30);
			} else if ((obstacle._laneNum == 1 && _s49_truckSequenceId == 0xB0) ||
				(obstacle._laneNum == 2 && (_s49_truckSequenceId == 0xB1 || _s49_truckSequenceId == 0xB2)) ||
				(obstacle._laneNum == 3 && _s49_truckSequenceId == 0xB3)) {
				_vm->_gameSys->setAnimation(obstacle._passedSequenceId, obstacle._prevId, id + 2);
				_vm->_gameSys->insertSequence(obstacle._passedSequenceId, obstacle._prevId,
					obstacle._currSequenceId, obstacle._currId,
					kSeqSyncWait, 0, 0, -50);
				obstacle._currSequenceId = obstacle._passedSequenceId;
			} else {
				_vm->_gameSys->setAnimation(obstacle._collisionSequenceId, 256, 0);
				_vm->_gameSys->setAnimation(obstacle._passedSequenceId, obstacle._prevId, id + 2);
				_vm->_gameSys->insertSequence(obstacle._passedSequenceId, obstacle._prevId,
					obstacle._currSequenceId, obstacle._currId,
					kSeqSyncWait, 0, 0, -50);
				_vm->_gameSys->insertSequence(obstacle._collisionSequenceId, 256,
					_s49_truckSequenceId, _s49_truckId,
					kSeqSyncExists, 0, 0, -50);
				_s49_truckSequenceId = obstacle._collisionSequenceId;
				_s49_truckId = 256;
				obstacle._currSequenceId = obstacle._passedSequenceId;
				_vm->playSound(225, false);
				decreaseScore(30);
			}
		} else {
			_vm->_gameSys->setAnimation(obstacle._passedSequenceId, obstacle._prevId, id + 2);
			_vm->_gameSys->insertSequence(obstacle._passedSequenceId, obstacle._prevId,
				obstacle._currSequenceId, obstacle._currId,
				kSeqSyncWait, 0, 0, -50);
			obstacle._currSequenceId = obstacle._passedSequenceId;
		}
	} else if (obstacle._currSequenceId == obstacle._passedSequenceId) {
		if (_s49_truckLaneNum == obstacle._laneNum) {
			if (obstacle._splashSequenceId) {
				_vm->_gameSys->setAnimation(obstacle._collisionSequenceId, obstacle._prevId, id + 2);
				_vm->_gameSys->insertSequence(obstacle._collisionSequenceId, obstacle._prevId,
					obstacle._currSequenceId, obstacle._currId,
					kSeqSyncWait, 0, 0, -50);
				obstacle._currSequenceId = obstacle._collisionSequenceId;
				_vm->playSound(224, false);
				increaseScore(30);
			}
		} else if (obstacle._splashSequenceId) {
			_vm->_gameSys->setAnimation(obstacle._splashSequenceId, obstacle._prevId, id + 2);
			_vm->_gameSys->insertSequence(obstacle._splashSequenceId, obstacle._prevId,
				obstacle._currSequenceId, obstacle._currId,
				kSeqSyncWait, 0, 0, -50);
			obstacle._currSequenceId = obstacle._splashSequenceId;
		}
	} else {
		_vm->_gameSys->setAnimation(0, 0, id + 2);
		clearObstacle(id);
	}
}

void Scene49::increaseScore(int amount) {
	if (_s49_scoreBarPos + amount <= 556) {
		_s49_scoreBarPos += amount;
		_vm->_gameSys->fillSurface(0, _s49_scoreBarPos, 508, amount, 22, 255, 0, 0);
	}
	_s49_scoreLevel = _s49_scoreBarPos + amount >= 556;
}

void Scene49::decreaseScore(int amount) {
	if (_s49_scoreBarPos >= 226 && _s49_scoreLevel == 0) {
		if (_s49_scoreBarFlash)
			refreshScoreBar();
		_vm->_gameSys->fillSurface(0, _s49_scoreBarPos, 508, amount, 22, 89, 0, 5);
		_s49_scoreBarPos -= amount;
		_s49_scoreLevel = 0;
	}
}

void Scene49::refreshScoreBar() {
	if (_s49_scoreBarFlash)
		_vm->_gameSys->fillSurface(0, 226, 508, 330, 22, 255, 0, 0);
	else
		_vm->_gameSys->fillSurface(0, 226, 508, 330, 22, 89, 0, 5);
	_s49_scoreBarFlash = !_s49_scoreBarFlash;
}

void Scene49::clearObstacle(int index) {
	_s49_obstacles[index]._currSequenceId = 0;
	_s49_obstacles[index]._closerSequenceId = 0;
	_s49_obstacles[index]._passedSequenceId = 0;
	_s49_obstacles[index]._splashSequenceId = 0;
	_s49_obstacles[index]._collisionSequenceId = 0;
	_s49_obstacles[index]._prevId = 0;
	_s49_obstacles[index]._currId = 0;
	_s49_obstacles[index]._laneNum = 0;
}

void Scene49::run() {
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

	_s49_scoreBarPos = 196;
	_s49_scoreLevel = 0;
	_s49_scoreBarFlash = false;

	switch (_vm->getRandom(3)) {
	case 0:
		_s49_truckSequenceId = 0xAD;
		_s49_truckLaneNum = 1;
		break;
	case 1:
		_s49_truckSequenceId = 0xAE;
		_s49_truckLaneNum = 2;
		break;
	case 2:
		_s49_truckSequenceId = 0xAF;
		_s49_truckLaneNum = 3;
		break;
	}

	int bgWidth1 = _vm->_gameSys->getSpriteWidthById(0x5E);
	int bgX1 = 600;

	int bgWidth2 = _vm->_gameSys->getSpriteWidthById(0x5F);
	int bgX2 = 400;

	int bgWidth3 = _vm->_gameSys->getSpriteWidthById(4);
	int bgX3 = 700;

	int bgWidth4 = _vm->_gameSys->getSpriteWidthById(5);
	int bgX4 = 500;

	int bgWidth5 = _vm->_gameSys->getSpriteWidthById(6);
	int bgX5 = 300;

	int bgWidth6 = _vm->_gameSys->getSpriteWidthById(7);
	int bgX6 = 100;

	_vm->_gameSys->setAnimation(0xC8, 251, 1);
	_vm->_gameSys->setAnimation(_s49_truckSequenceId, 256, 0);
	_vm->_gameSys->insertSequence(0xC9, 256, 0, 0, kSeqNone, 0, 600, 85);
	_vm->_gameSys->insertSequence(0xCA, 257, 0, 0, kSeqNone, 0, 400, 100);
	_vm->_gameSys->insertSequence(0xC4, 256, 0, 0, kSeqNone, 0, 700, 140);
	_vm->_gameSys->insertSequence(0xC5, 257, 0, 0, kSeqNone, 0, 500, 160);
	_vm->_gameSys->insertSequence(0xC6, 258, 0, 0, kSeqNone, 0, 300, 140);
	_vm->_gameSys->insertSequence(0xC7, 259, 0, 0, kSeqNone, 0, 100, 140);
	_vm->_gameSys->insertSequence(0xC8, 251, 0, 0, kSeqNone, 0, 0, -50);
	_vm->_gameSys->insertSequence(_s49_truckSequenceId, 256, 0, 0, kSeqNone, 0, 0, -50);

	_vm->_timers[0] = 2;

	for (int i = 0; i < 5; ++i)
		clearObstacle(i);

	_s49_obstacleIndex = 0;

	_vm->_timers[2] = _vm->getRandom(20) + 10;

	_s49_truckId = 256;
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
			_vm->_gameSys->insertSequence(0xC9, (bgAnimToggle ? 1 : 0) + 256, 0xC9, (bgAnimToggle ? 0 : 1) + 256, kSeqSyncWait, 0, bgX1, 85);
			_vm->_gameSys->insertSequence(0xCA, (bgAnimToggle ? 1 : 0) + 257, 0xCA, (bgAnimToggle ? 0 : 1) + 257, kSeqSyncWait, 0, bgX2, 100);
			_vm->_gameSys->insertSequence(0xC4, (bgAnimToggle ? 1 : 0) + 256, 0xC4, (bgAnimToggle ? 0 : 1) + 256, kSeqSyncWait, 0, bgX3, 140);
			_vm->_gameSys->insertSequence(0xC5, (bgAnimToggle ? 1 : 0) + 257, 0xC5, (bgAnimToggle ? 0 : 1) + 257, kSeqSyncWait, 0, bgX4, 160);
			_vm->_gameSys->insertSequence(0xC6, (bgAnimToggle ? 1 : 0) + 258, 0xC6, (bgAnimToggle ? 0 : 1) + 258, kSeqSyncWait, 0, bgX5, 140);
			_vm->_gameSys->insertSequence(0xC7, (bgAnimToggle ? 1 : 0) + 259, 0xC7, (bgAnimToggle ? 0 : 1) + 259, kSeqSyncWait, 0, bgX6, 140);
			_vm->_timers[0] = 2;
		}

		if (_vm->_gameSys->getAnimationStatus(1) == 2) {
			streetAnimToggle = !streetAnimToggle;
			_vm->_gameSys->setAnimation(0xC8, (streetAnimToggle ? 1 : 0) + 251, 1);
			_vm->_gameSys->insertSequence(0xC8, (streetAnimToggle ? 1 : 0) + 251, 200, (streetAnimToggle ? 0 : 1) + 251, kSeqSyncWait, 0, 0, -50);
		}

		checkObstacles();

		if (_vm->_gameSys->getAnimationStatus(0) == 2) {
			switch (_s49_truckSequenceId) {
			case 0xB1:
				_s49_truckLaneNum = 1;
				break;
			case 0xB0:
			case 0xB3:
				_s49_truckLaneNum = 2;
				break;
			case 0xB2:
				_s49_truckLaneNum = 3;
				break;
			}
			animToggle3 = !animToggle3;
			if (_s49_truckLaneNum == 1) {
				_vm->_gameSys->setAnimation(0xAD, (animToggle3 ? 1 : 0) + 256, 0);
				_vm->_gameSys->insertSequence(0xAD, (animToggle3 ? 1 : 0) + 256, _s49_truckSequenceId, _s49_truckId, kSeqSyncWait, 0, 0, -50);
				_s49_truckSequenceId = 0xAD;
			} else if (_s49_truckLaneNum == 2) {
				_vm->_gameSys->setAnimation(0xAE, (animToggle3 ? 1 : 0) + 256, 0);
				_vm->_gameSys->insertSequence(0xAE, (animToggle3 ? 1 : 0) + 256, _s49_truckSequenceId, _s49_truckId, kSeqSyncWait, 0, 0, -50);
				_s49_truckSequenceId = 0xAE;
			} else {
				_vm->_gameSys->setAnimation(0xAF, (animToggle3 ? 1 : 0) + 256, 0);
				_vm->_gameSys->insertSequence(0xAF, (animToggle3 ? 1 : 0) + 256, _s49_truckSequenceId, _s49_truckId, kSeqSyncWait, 0, 0, -50);
				_s49_truckSequenceId = 0xAF;
			}
			_s49_truckId = (animToggle3 ? 1 : 0) + 256;
			if (_s49_scoreLevel == 1) {
				if (!_vm->_gameSys->isSequenceActive(0xD4, 266)) {
					_vm->_gameSys->setAnimation(0xD4, 266, 8);
					_vm->_gameSys->insertSequence(0xD4, 266, 0, 0, kSeqNone, 0, 0, -50);
				}
				++_s49_scoreLevel;
				_vm->_timers[1] = 2;
				animToggle4 = false;
				animToggle5 = false;
				animToggle6 = false;
				_s49_scoreBarFlash = false;
			}
		}

		if (_s49_scoreLevel != 0 && !_vm->_timers[1]) {
			refreshScoreBar();
			_vm->_timers[1] = 8;
			if (animToggle6) {
				if (animToggle5) {
					if (animToggle4 && !_vm->_gameSys->isSequenceActive(212, 266))
						_vm->_gameSys->insertSequence(212, 266, 0, 0, kSeqNone, 0, 0, -50);
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
			if (_s49_truckSequenceId == 0xB3)
				_s49_truckLaneNum = 2;
			if (_s49_truckSequenceId == 0xB1)
				_s49_truckLaneNum = 1;
			if (_s49_truckLaneNum != 3 && _s49_truckLaneNum != 2) {
				if (_s49_scoreLevel) {
					_vm->_sceneDone = true;
					_vm->_newSceneNum = 47;
				}
			} else {
				int steerSequenceId = (_s49_truckLaneNum == 3) ? 0xB3 : 0xB1;
				if (_s49_truckSequenceId == 0xAE || _s49_truckSequenceId == 0xAF) {
					_vm->_gameSys->setAnimation(steerSequenceId, 256, 0);
					_vm->_gameSys->insertSequence(steerSequenceId, 256, _s49_truckSequenceId, _s49_truckId, kSeqSyncExists, 0, 0, -50);
					_s49_truckSequenceId = steerSequenceId;
					_s49_truckId = 256;
				}
			}
			_vm->clearKeyStatus1(Common::KEYCODE_RIGHT);
		}

		if (_vm->isKeyStatus1(Common::KEYCODE_LEFT)) {
			// Steer left
			if (_s49_truckSequenceId == 0xB0)
				_s49_truckLaneNum = 2;
			if (_s49_truckSequenceId == 0xB2)
				_s49_truckLaneNum = 3;
			if (_s49_truckLaneNum == 1 || _s49_truckLaneNum == 2) {
				int steerSequenceId = (_s49_truckLaneNum == 1) ? 0xB0 : 0xB2;
				if (_s49_truckSequenceId == 0xAD || _s49_truckSequenceId == 0xAE) {
					_vm->_gameSys->setAnimation(steerSequenceId, 256, 0);
					_vm->_gameSys->insertSequence(steerSequenceId, 256, _s49_truckSequenceId, _s49_truckId, kSeqSyncExists, 0, 0, -50);
					_s49_truckSequenceId = steerSequenceId;
					_s49_truckId = 256;
				}
			}
			_vm->clearKeyStatus1(Common::KEYCODE_LEFT);
		}
		_vm->gameUpdateTick();
	}
	_vm->stopSound(0xE2);  
}

void Scene49::updateAnimations() {
	for (int i = 0; i < 5; ++i) {
		if (_vm->_gameSys->getAnimationStatus(i + 2) == 2) {
			if (_s49_obstacles[i]._currSequenceId)
				updateObstacle(i);
		}
	}

	if (_vm->_gameSys->getAnimationStatus(8) == 2) {
		_vm->_sceneDone = true;
		_vm->_newSceneNum = 47;
	}
}

} // End of namespace Gnap
