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

namespace Gnap {

int GnapEngine::scene52_init() {
	scene52_initAnims();
	return 0x2B;
}

void GnapEngine::scene52_updateHotspots() {
	_hotspotsCount = 0;
}

void GnapEngine::scene52_update() {
	
	for (int rowNum = 0; rowNum < 7; ++rowNum) {
		gameUpdateTick();
		if (_gameSys->getAnimationStatus(_s52_alienRowAnims[rowNum]) == 2) {
			scene52_updateAlienRow(rowNum);
			rowNum = 0;
		}
	}

	if (_s52_liveAlienRows == 0 && !_s52_alienSingle) {
		_s52_alienWave = false;
		playSound(48, 0);
		++_s52_alienCounter;
		if (_s52_alienCounter != 3) {
			_timers[0] = 50;
			_timers[2] = 100;
			_s52_alienRowDownCtr = 0;
			_s52_alienSingle = true;
		}
	}

	if (_s52_alienSingle && !_timers[0]) {
		scene52_initAliens();
		_s52_alienSingle = false;
		_timers[2] = 5;
		_s52_alienWave = true;
	}

	if ((_s52_alienRowDownCtr || _s52_liveAlienRows == 0) && !_s52_alienSingle) {
		scene52_moveDownAlienRow();
		_s52_alienRowDownCtr = 0;
	}

	if (isKeyStatus1(Common::KEYCODE_UP) || isKeyStatus1(Common::KEYCODE_SPACE)) {
		clearKeyStatus1(Common::KEYCODE_SPACE);
		clearKeyStatus1(Common::KEYCODE_UP);
		if (!_s52_aliensCount)
			scene52_fireShipCannon(_s52_shipPosX);
	}

	if (_s52_shipCannonFiring)
		scene52_updateShipCannon();

	scene52_fireAlienCannon();
	scene52_updateAlienCannons();

	if (_s52_aliensCount == 1) {
		_s52_alienWave = false;
		_timers[3] = 20;
		_timers[2] = 100;
		++_s52_aliensCount;
	}

	if (_s52_aliensCount && !_timers[3]) {
		scene52_updateAliens();
		scene52_loseShip();
		if (_s52_shipsLeft != 0) {
			_timers[3] = 40;
			while (_timers[3]) {
				scene52_updateAlienCannons();
				if (_s52_shipCannonFiring)
					scene52_updateShipCannon();
				gameUpdateTick();
			}
			scene52_initAliens();
			_s52_shipPosX = (800 - _s52_shipMidX) / 2;
			_gameSys->setAnimation(_s52_nextUfoSequenceId, 256, 7);
			_gameSys->insertSequence(_s52_nextUfoSequenceId, 256, 0, 0, kSeqNone, 0, _s52_shipPosX, _s52_arcadeScreenBottom);
			_s52_ufoSequenceId = _s52_nextUfoSequenceId;
			_timers[2] = 5;
			_s52_alienWave = true;
		} else {
			_sceneDone = true;
		}
	}

	if (_timers[1] || scene52_getFreeShipCannon() == -1) {
		_s52_nextUfoSequenceId = 34;
		if (_s52_ufoSequenceId != 34)
			_s52_shipFlag = true;
	} else {
		_s52_nextUfoSequenceId = 34;
		if (_s52_ufoSequenceId != 34)
			_s52_shipFlag = true;
	}

	if (_s52_shipFlag) {
		if (_gameSys->getAnimationStatus(7) == 2) {
			_gameSys->setAnimation(_s52_nextUfoSequenceId, 256, 7);
			_gameSys->insertSequence(_s52_nextUfoSequenceId, 256, _s52_ufoSequenceId, 256, kSeqSyncWait, 0, _s52_shipPosX, _s52_arcadeScreenBottom);
			_s52_ufoSequenceId = _s52_nextUfoSequenceId;
		}
		_s52_shipFlag = false;
	}

	if (_s52_alienWave && !_timers[0]) {
		scene52_playSound();
		int v0 = _s52_alienSpeed;
		if (_s52_alienSpeed >= 10)
			v0 = 10;
		int v1 = v0;
		if (v0 < 2)
			v1 = 2;
		_timers[0] = v1;
	}
}

void GnapEngine::scene52_initShipCannon(int bottomY) {
	_s52_shipCannonFired = 0;
	_s52_shipCannonWidth = MAX(_gameSys->getSpriteWidthById(14), _gameSys->getSpriteWidthById(16));
	_s52_shipCannonHeight = MAX(_gameSys->getSpriteHeightById(14), _gameSys->getSpriteHeightById(16));
	_s52_shipCannonTopY = bottomY - _s52_shipCannonHeight;
	_s52_shipCannonFiring = 0;
}

void GnapEngine::scene52_initAlienCannons() {
	for (int i = 0; i < 3; ++i) {
		_s52_alienCannonIds[i] = 0;
		_s52_alienCannonFired[i] = 0;
	}
	_s52_alienCannonSequenceIds[0] = 30;
	_s52_alienCannonSequenceIds[1] = 31;
	_s52_alienCannonSequenceIds[2] = 32;
}

void GnapEngine::scene52_fireShipCannon(int posX) {

	if (_timers[1])
		return;

	int cannonNum = scene52_getFreeShipCannon();
	if (cannonNum != -1) {
		_s52_shipCannonPosX = _s52_shipMidX / 2 + posX - _s52_shipCannonWidth / 2;
		_s52_shipCannonPosY = _s52_shipCannonTopY;
		_gameSys->setAnimation(0x23, cannonNum + 256, cannonNum + 8);
		_gameSys->insertSequence(0x23, cannonNum + 256, 0, 0, kSeqNone, 0, _s52_shipCannonPosX, _s52_shipCannonPosY);
		playSound(0x2D, 0);
		if (scene52_shipCannonHitShield(cannonNum)) {
			_gameSys->setAnimation(0, 0, cannonNum + 8);
			_gameSys->removeSequence(0x23, cannonNum + 256, true);
		} else {
			_s52_shipCannonFired = 1;
			_s52_shipCannonPosY -= 13;
			_s52_shipCannonFiring = 1;
		}
		_timers[1] = 5;
	}

}

void GnapEngine::scene52_fireAlienCannon() {

	if (_timers[2])
		return;

	int cannonNum = scene52_getFreeAlienCannon();
	if (cannonNum != -1) {
		int alienX1 = _s52_alienLeftX + _s52_alienRowXOfs[0];
		int alienX2 = _s52_alienLeftX + _s52_alienRowXOfs[0] + 5 * _s52_alienWidth - (_s52_alienWidth / 2 - 15);
		_s52_alienCannonPosX[cannonNum] = getRandom(alienX2 - alienX1) + alienX1;
		_s52_alienCannonPosY[cannonNum] = 104;
		_s52_alienCannonFired[cannonNum] = 1;
		_gameSys->setAnimation(_s52_alienCannonSequenceIds[cannonNum], _s52_alienCannonIds[cannonNum] + 256, cannonNum + 9);
		_gameSys->insertSequence(_s52_alienCannonSequenceIds[cannonNum], _s52_alienCannonIds[cannonNum] + 256, 0, 0,
			kSeqNone, 0, _s52_alienCannonPosX[cannonNum], _s52_alienCannonPosY[cannonNum]);
		_s52_alienCannonPosY[cannonNum] -= 13;
		_timers[2] = 5;
	}

}

int GnapEngine::scene52_getFreeShipCannon() {
	if (!_s52_shipCannonFired)
		return 0;
	return -1;
}

int GnapEngine::scene52_getFreeAlienCannon() {
	for (int i = 0; i < 3; ++i)
		if (!_s52_alienCannonFired[i])
			return i;
	return -1;
}

void GnapEngine::scene52_updateShipCannon() {
	if (_s52_shipCannonFired && _gameSys->getAnimationStatus(8) == 2) {
		_s52_shipCannonPosY -= 13;
		if (_s52_shipCannonPosY - 13 >= 135) {
			if (scene52_updateHitAlien()) {
				_gameSys->setAnimation(0, 0, 8);
				_gameSys->removeSequence(35, 256, true);
				_s52_shipCannonFired = 0;
				scene52_drawScore(_s52_gameScore);
			} else {
				_gameSys->setAnimation(35, 256, 8);
				_gameSys->insertSequence(35, 256, 35, 256, kSeqSyncWait, 0, _s52_shipCannonPosX, _s52_shipCannonPosY);
				_s52_shipCannonPosY -= 13;
			}
		} else {
			_gameSys->setAnimation(0, 0, 8);
			_gameSys->removeSequence(35, 256, true);
			_s52_shipCannonFired = 0;
		}
	}
}

void GnapEngine::scene52_updateAlienCannons() {
	for (int i = 0; i < 3; ++i) {
		if (_s52_alienCannonFired[i] && _gameSys->getAnimationStatus(i + 9) == 2) {
			_s52_alienCannonPosY[i] += 13;
			if (_s52_shipCannonHeight + _s52_alienCannonPosY[i] + 13 <= 550) {
				if (scene52_alienCannonHitShip(i)) {
					_gameSys->setAnimation(0, 0, i + 9);
					_s52_alienCannonFired[i] = 0;
					scene52_shipExplode();
				} else if (scene52_alienCannonHitShield(i)) {
					_s52_alienCannonFired[i] = 0;
				} else {
					_gameSys->insertSequence(_s52_alienCannonSequenceIds[i], 1 - _s52_alienCannonIds[i] + 256, 0, 0,
						kSeqNone, 0, _s52_alienCannonPosX[i], _s52_alienCannonPosY[i]);
					_gameSys->setAnimation(_s52_alienCannonSequenceIds[i], 1 - _s52_alienCannonIds[i] + 256, i + 9);
					_s52_alienCannonIds[i] = 1 - _s52_alienCannonIds[i];
					_s52_alienCannonPosY[i] += 13;
				}
			} else {
				_gameSys->setAnimation(0, 0, i + 9);
				_s52_alienCannonFired[i] = 0;
			}
		}
	}
}

void GnapEngine::scene52_initAliens() {
	
	if (!_s52_aliensInitialized) {
		scene52_initAlienSize();
		_s52_aliensInitialized = true;
	}

	_s52_liveAlienRows = 0;
	_s52_alienSpeed = 0;
	_s52_bottomAlienFlag = false;
	_s52_aliensCount = 0;
	_s52_alienSingle = false;
	_s52_alienRowDownCtr = 0;

	scene52_initShields();

	_s52_alienRowKind[0] = -1;
	_s52_alienRowKind[1] = -1;
	_s52_alienRowKind[2] = -1;
	_s52_alienRowKind[3] = -1;
	_s52_alienRowKind[4] = getRandom(2) != 0 ? 24 : 27;
	_s52_alienRowKind[5] = getRandom(2) != 0 ? 25 : 28;
	_s52_alienRowKind[6] = getRandom(2) != 0 ? 26 : 29;

	for (int i = 0; i < 7; ++i) {
		_s52_alienRowAnims[i] = i;
		_s52_alienRowXOfs[i] = 0;
		scene52_initAlienRowKind(i, _s52_alienRowKind[i]);
		scene52_insertAlienRow(i);
	}

}

void GnapEngine::scene52_initAlienRowKind(int rowNum, int alienKind) {
	for (int i = 0; i < 5; ++i)
		_s52_items[rowNum][i] = alienKind;
}

void GnapEngine::scene52_insertAlienRow(int rowNum) {
	if (_s52_alienRowKind[rowNum] >= 0) {
		scene52_insertAlienRowAliens(rowNum);
		_s52_alienRowIds[rowNum] = 256;
		_gameSys->setAnimation(_s52_alienRowKind[rowNum], _s52_alienRowIds[rowNum], _s52_alienRowAnims[rowNum]);
		++_s52_liveAlienRows;
	}
}

void GnapEngine::scene52_insertAlienRowAliens(int rowNum) {
	int xOffs = _s52_alienLeftX;
	int yOffs = _s52_alienTopY - 52 * rowNum - _s52_alienHeight + 10;
	for (int i = 0; i < 5; ++i) {
		if (_s52_items[rowNum][i] >= 0) {
			_gameSys->insertSequence(_s52_items[rowNum][i], i + 256, 0, 0, kSeqNone, 0, xOffs, yOffs);
			++_s52_alienSpeed;
		}
		xOffs += _s52_alienWidth;
	}
}

void GnapEngine::scene52_updateAlienRow(int rowNum) {
	
	if (_s52_alienRowKind[rowNum] != -1 && !scene52_checkAlienRow(rowNum)) {
		scene52_updateAlienRowXOfs();
		_s52_alienRowIds[rowNum] = -1;
		int xOffs = _s52_alienLeftX + _s52_alienRowXOfs[rowNum];
		int yOffs = _s52_alienTopY - 52 * rowNum - _s52_alienHeight + 10;
		for (int i = 0; i < 5; ++i) {
			if (_s52_items[rowNum][i] >= 0) {
				_gameSys->insertSequence(_s52_items[rowNum][i], i + 256, _s52_items[rowNum][i], i + 256, kSeqSyncWait, 0, xOffs, yOffs);
				if (_s52_alienRowIds[rowNum] == -1)
					_s52_alienRowIds[rowNum] = i + 256;
			} else if (_s52_items[rowNum][i] == -2) {
				_gameSys->removeSequence(_s52_alienRowKind[rowNum], i + 256, true);
				_s52_items[rowNum][i] = -1;
				--_s52_alienSpeed;
			}
			xOffs += _s52_alienWidth;
		}
		if (_s52_alienRowIds[rowNum] == -1) {
			_gameSys->setAnimation(0, 0, _s52_alienRowAnims[rowNum]);
			// MessageBoxA(0, "No live aliens!", "Error 3:", 0x30u);
		} else {
			_gameSys->setAnimation(_s52_alienRowKind[rowNum], _s52_alienRowIds[rowNum], _s52_alienRowAnims[rowNum]);
		}
		if (rowNum == 1) {
			for (int j = 0; j < 3; ++j) {
				if (_s52_shieldSpriteIds[j] != -1) {
					_gameSys->fillSurface(0, _s52_shieldPosX[j], _s52_arcadeScreenBottom - 44, 33, 44, 0, 0, 0);
					_s52_shieldSpriteIds[j] = -1;
				}
			}
		}
		if (rowNum == 0 && _s52_bottomAlienFlag)
			scene52_shipExplode();
	}
}

void GnapEngine::scene52_moveDownAlienRow() {

	int v2[5], v3, v1, v0, v4;
	
	for (int i = 0; i < 5; ++i)
		v2[i] = _s52_items[0][i];

	v3 = _s52_alienRowIds[0];
	v1 = _s52_alienRowAnims[0];
	v0 = _s52_alienRowKind[0];
	v4 = _s52_alienRowXOfs[0];

	for (int j = 0; j < 7; ++j) {
		for (int i = 0; i < 5; ++i)
			_s52_items[j][i] = _s52_items[j + 1][i];
		_s52_alienRowIds[j] = _s52_alienRowIds[j + 1];
		_s52_alienRowAnims[j] = _s52_alienRowAnims[j + 1];
		_s52_alienRowKind[j] = _s52_alienRowKind[j + 1];
		_s52_alienRowXOfs[j] = _s52_alienRowXOfs[j + 1];
	}

	for (int i = 0; i < 5; ++i)
		_s52_items[6][i] = v2[i];

	_s52_alienRowIds[6] = v3;
	_s52_alienRowAnims[6] = v1;
	_s52_alienRowKind[6] = v0;
	_s52_alienRowXOfs[6] = v4;

	scene52_updateAlien(6);
	scene52_initAlienRowKind(6, _s52_alienRowKind[6]);
	scene52_insertAlienRow(6);

	_s52_bottomAlienFlag = _s52_alienRowKind[0] > -1;
}

int GnapEngine::scene52_updateHitAlien() {
	int result = 0, rowNum, ya;
	
	int y = _s52_shipCannonTopY - _s52_shipCannonPosY;

	if (y == 26) {
		rowNum = 1;
		ya = _s52_shipCannonPosY + 26;
	} else {
		if (y % 52)
			return 0;
		rowNum = y / 52 + 1;
		ya = _s52_shipCannonPosY;
	}

	if (rowNum < 7) {
		int hitAlienNum = scene52_getHitAlienNum(rowNum);
		if (hitAlienNum != -1 && _s52_items[rowNum][hitAlienNum] >= 0) {
			_s52_gameScore = ((_s52_items[rowNum][hitAlienNum] - 24) % 3 + _s52_gameScore + 1) % 1000;
			_s52_items[rowNum][hitAlienNum] = -2;
			playSound(44, 0);
			_gameSys->insertSequence(0x21, 266, 0, 0,
				kSeqNone, 0, _s52_alienLeftX + hitAlienNum * _s52_alienWidth + _s52_alienRowXOfs[rowNum] - 10, ya - _s52_alienHeight);
			result = 1;
		}
	}
	
	return result;
}

int GnapEngine::scene52_getHitAlienNum(int rowNum) {
	
	int result = -1;

	int v3 = _s52_alienLeftX + _s52_alienRowXOfs[rowNum];

	if (_s52_shipCannonPosX >= v3) {
		int v8 = _s52_alienWidth / 2 - 15;
		if (v3 + 5 * _s52_alienWidth - v8 >= _s52_shipCannonPosX) {
			int v4 = v3 + _s52_alienWidth;
			if (_s52_shipCannonPosX >= v4 - v8) {
				int v5 = v4 + _s52_alienWidth;
				if (_s52_shipCannonPosX >= v5 - v8) {
					int v6 = v5 + _s52_alienWidth;
					if (_s52_shipCannonPosX >= v6 - v8) {
						int v7 = v6 + _s52_alienWidth;
						if (_s52_shipCannonPosX >= v7 - v8) {
							if (_s52_shipCannonPosX >= v7 + _s52_alienWidth - v8)
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

int GnapEngine::scene52_alienCannonHitShip(int cannonNum) {
	int result = 0;
	
	if (_s52_aliensCount) {
		result = 0;
	} else {
		int cannonY = _s52_alienCannonPosY[cannonNum] - 13;
		if (_s52_arcadeScreenBottom <= cannonY) {
			if (_s52_shipMidY + _s52_arcadeScreenBottom > cannonY) {
				if (_s52_alienCannonPosX[cannonNum] >= _s52_shipPosX)
					result = _s52_alienCannonPosX[cannonNum] < _s52_shipMidX + _s52_shipPosX;
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

int GnapEngine::scene52_alienCannonHitShield(int cannonNum) {
	int result = 0;
	
	int v3 = _s52_alienCannonPosY[cannonNum] + 39;
	if (_s52_arcadeScreenBottom - 44 > v3)
		return 0;

	if (_s52_arcadeScreenBottom <= v3)
		return 0;

	if (_s52_alienCannonPosX[cannonNum] < _s52_shieldPosX[0])
		return 0;

	if (_s52_alienCannonPosX[cannonNum] > _s52_shieldPosX[2] + 33)
		return 0;

	int shieldNum = -1;
	if (_s52_alienCannonPosX[cannonNum] < _s52_shieldPosX[0] + 33)
		shieldNum = 0;

	if (shieldNum < 0 && _s52_alienCannonPosX[cannonNum] < _s52_shieldPosX[1])
		return 0;

	if (shieldNum < 0 && _s52_alienCannonPosX[cannonNum] < _s52_shieldPosX[1] + 33)
		shieldNum = 1;

	if (shieldNum < 0) {
		if (_s52_alienCannonPosX[cannonNum] < _s52_shieldPosX[2])
			return 0;
		shieldNum = 2;
	}
	
	if (_s52_shieldSpriteIds[shieldNum] == -1) {
		result = 0;
	} else {
		++_s52_shieldSpriteIds[shieldNum];
		if (_s52_shieldSpriteIds[shieldNum] <= 21) {
			_gameSys->drawSpriteToBackground(_s52_shieldPosX[shieldNum], _s52_arcadeScreenBottom - 44, _s52_shieldSpriteIds[shieldNum]);
		} else {
			_gameSys->fillSurface(0, _s52_shieldPosX[shieldNum], _s52_arcadeScreenBottom - 44, 33, 44, 0, 0, 0);
			_s52_shieldSpriteIds[shieldNum] = -1;
		}
		_gameSys->setAnimation(0, 0, cannonNum + 9);
		_gameSys->insertSequence(0x21, shieldNum + 257, 0, 0, kSeqNone, 0, _s52_alienCannonPosX[cannonNum] - 18, _s52_arcadeScreenBottom - 44);
		playSound(0x2C, 0);
		result = 1;
	}

	return result;
}

int GnapEngine::scene52_shipCannonHitShield(int cannonNum) {
	int result = 0;
	
	if (_s52_shipCannonPosX < _s52_shieldPosX[0])
		return 0;

	if (_s52_shipCannonPosX > _s52_shieldPosX[2] + 33)
		return 0;

	int shieldNum = -1;
	if (_s52_shipCannonPosX < _s52_shieldPosX[0] + 33)
		shieldNum = 0;

	if (shieldNum < 0 && _s52_shipCannonPosX < _s52_shieldPosX[1])
		return 0;

	if (shieldNum < 0 && _s52_shipCannonPosX < _s52_shieldPosX[1] + 33)
		shieldNum = 1;

	if (shieldNum < 0) {
		if (_s52_shipCannonPosX < _s52_shieldPosX[2])
			return 0;
		shieldNum = 2;
	}

	if (_s52_shieldSpriteIds[shieldNum] == -1) {
		result = 0;
	} else {
		++_s52_shieldSpriteIds[shieldNum];
		if (_s52_shieldSpriteIds[shieldNum] <= 21) {
			_gameSys->drawSpriteToBackground(_s52_shieldPosX[shieldNum], _s52_arcadeScreenBottom - 44, _s52_shieldSpriteIds[shieldNum]);
		} else {
			_gameSys->fillSurface(0, _s52_shieldPosX[shieldNum], _s52_arcadeScreenBottom - 44, 33, 44, 0, 0, 0);
			_s52_shieldSpriteIds[shieldNum] = -1;
		}
		_gameSys->insertSequence(0x21, shieldNum + 257, 0, 0, kSeqNone, 0, _s52_shipCannonPosX - 18, _s52_arcadeScreenBottom - 44);
		playSound(0x2C, 0);
		result = 1;
	}

	return result;
}

int GnapEngine::scene52_shipCannonHitAlien() {
	int result = 0;
	
	if (_s52_aliensCount) {
		result = 0;
	} else if (scene52_checkAlienRow(0)) {
		result = 0;
	} else {
		int v1 = _s52_alienLeftX + _s52_alienRowXOfs[0];
		if (_s52_shipMidX + _s52_shipPosX >= _s52_alienLeftX + _s52_alienRowXOfs[0]) {
			int v7 = _s52_alienWidth / 2 - 15;
			if (v1 + 5 * _s52_alienWidth - v7 >= _s52_shipPosX) {
				int v2 = v1 + _s52_alienWidth;
				if (_s52_items[0][0] <= -1 || v2 - v7 <= _s52_shipPosX) {
					int v3 = v2 + _s52_alienWidth;
					if (_s52_items[0][1] <= -1 || v3 - v7 <= _s52_shipPosX) {
						int v4 = v3 + _s52_alienWidth;
						if (_s52_items[0][2] <= -1 || v4 - v7 <= _s52_shipPosX) {
							int v5 = v4 + _s52_alienWidth;
							if (_s52_items[0][3] <= -1 || v5 - v7 <= _s52_shipPosX) {
								int v6 = v5 + _s52_alienWidth;
								result = _s52_items[0][4] > -1 && v6 - v7 > _s52_shipPosX;
							} else {
								result = 1;
							}
						} else {
							result = 1;
						}
					} else {
						result = 1;
					}
				} else {
					result = 1;
				}
			} else {
				result = 0;
			}
		} else {
			result = 0;
		}
	}

	return result;
}

void GnapEngine::scene52_shipExplode() {
	if (!_s52_aliensCount) {
		_gameSys->setAnimation(0, 0, 7);
		_gameSys->removeSequence(_s52_ufoSequenceId, 256, true);
		playSound(44, 0);
		_gameSys->insertSequence(0x21, 266, 0, 0, kSeqNone, 0, _s52_shipPosX, _s52_arcadeScreenBottom);
		_s52_aliensCount = 1;
		playSound(0x31, 0);
	}
}

int GnapEngine::scene52_checkAlienRow(int rowNum) {

	int v4 = 0;

	for (int i = 0; i < 5; ++i)
		if (_s52_items[rowNum][i] >= 0)
			return 0;

	for (int j = 0; j < 5; ++j)
		if (_s52_items[rowNum][j] == -2) {
			_gameSys->removeSequence(_s52_alienRowKind[rowNum], j + 256, true);
			_s52_items[rowNum][j] = -1;
			--_s52_alienSpeed;
			v4 = 1;
		}

	if (v4) {
		_gameSys->setAnimation(0, 0, _s52_alienRowAnims[rowNum]);
		--_s52_liveAlienRows;
	}

	if (_s52_liveAlienRows < 0)
		_s52_liveAlienRows = 0;

	return 1;
}

void GnapEngine::scene52_updateAlienRowXOfs() {
	
	int amount = 2 * (3 - _s52_liveAlienRows) + 1;

	if (_s52_alienSpeed == 2)
		amount *= 4;
	else if (_s52_alienSpeed == 1)
		amount *= 10;

	if (_s52_alienDirection) {
		for (int i = 0; i < 7; ++i) {
			_s52_alienRowXOfs[i] -= amount;
			if (_s52_alienRowXOfs[i] <= -100) {
				_s52_alienRowXOfs[i] = -100;
				_s52_alienDirection = 0;
				++_s52_alienRowDownCtr;
			}
		}
	} else {
		for (int j = 0; j < 7; ++j) {
			_s52_alienRowXOfs[j] += amount;
			if (_s52_alienRowXOfs[j] >= 100) {
				_s52_alienRowXOfs[j] = 100;
				_s52_alienDirection = 1;
				++_s52_alienRowDownCtr;
			}
		}
	}

}

void GnapEngine::scene52_initAlienSize() {

	_s52_alienWidth = _gameSys->getSpriteWidthById(0);
	if (_gameSys->getSpriteWidthById(1) > _s52_alienWidth)
		_s52_alienWidth = _gameSys->getSpriteWidthById(1);
	if (_gameSys->getSpriteWidthById(4) > _s52_alienWidth)
		_s52_alienWidth = _gameSys->getSpriteWidthById(4);
	if (_gameSys->getSpriteWidthById(5) > _s52_alienWidth)
		_s52_alienWidth = _gameSys->getSpriteWidthById(5);
	if (_gameSys->getSpriteWidthById(12) > _s52_alienWidth)
		_s52_alienWidth = _gameSys->getSpriteWidthById(12);
	if (_gameSys->getSpriteWidthById(13) > _s52_alienWidth)
		_s52_alienWidth = _gameSys->getSpriteWidthById(13);

	_s52_alienHeight = _gameSys->getSpriteHeightById(0);
	if (_gameSys->getSpriteHeightById(1) > _s52_alienHeight)
		_s52_alienHeight = _gameSys->getSpriteHeightById(1);
	if (_gameSys->getSpriteHeightById(4) > _s52_alienHeight)
		_s52_alienHeight = _gameSys->getSpriteHeightById(4);
	if (_gameSys->getSpriteHeightById(5) > _s52_alienHeight)
		_s52_alienHeight = _gameSys->getSpriteHeightById(5);
	if (_gameSys->getSpriteHeightById(12) > _s52_alienHeight)
		_s52_alienHeight = _gameSys->getSpriteHeightById(12);
	if (_gameSys->getSpriteHeightById(13) > _s52_alienHeight)
		_s52_alienHeight = _gameSys->getSpriteHeightById(13);

	_s52_alienTopY = _s52_shipCannonTopY + 52;
	_s52_alienLeftX = (800 - 5 * _s52_alienWidth) / 2;

}

void GnapEngine::scene52_playSound() {
	if (_s52_soundToggle) {
		playSound(0x2F, 0);
		_s52_soundToggle = false;
	} else {
		playSound(0x2E, 0);
		_s52_soundToggle = true;
	}
}

void GnapEngine::scene52_updateAliens() {
	for (int i = 0; i < 7; ++i)
		scene52_updateAlien(i);
}

void GnapEngine::scene52_updateAlien(int rowNum) {
	if (_s52_alienRowKind[rowNum] >= 0 && !scene52_checkAlienRow(rowNum)) {
		for (int i = 0; i < 5; ++i)
			if (_s52_items[rowNum][i] >= 0)
				_s52_items[rowNum][i] = -2;
		scene52_checkAlienRow(rowNum);
	}
}

void GnapEngine::scene52_loseShip() {
	--_s52_shipsLeft;
	if (_s52_shipsLeft == 2) {
		_gameSys->fillSurface(0, 120, 140, _s52_shipMidX, _s52_shipMidY, 0, 0, 0);
	} else if (_s52_shipsLeft == 1) {
		_gameSys->fillSurface(0, 120, 185, _s52_shipMidX, _s52_shipMidY, 0, 0, 0);
	}
}

void GnapEngine::scene52_initShields() {
	for (int i = 0; i < 3; ++i) {
		_gameSys->drawSpriteToBackground(_s52_shieldPosX[i], _s52_arcadeScreenBottom - 44, 17);
		_s52_shieldSpriteIds[i] = 17;
	}
}

void GnapEngine::scene52_initAnims() {
	for (int i = 0; i < 7; ++i)
		_gameSys->setAnimation(0, 0, i);
	_gameSys->setAnimation(0, 0, 7);
	for (int j = 0; j < 1; ++j)
		_gameSys->setAnimation(0, 0, j + 8);
	for (int k = 0; k < 3; ++k)
		_gameSys->setAnimation(0, 0, k + 9);
}

void GnapEngine::scene52_drawScore(int a1) {
	char str[4];
	sprintf(str, "%03d", a1);
	_gameSys->fillSurface(0, 420, 80, 48, 30, 0, 0, 0);
	_gameSys->drawTextToSurface(0, 420, 80, 255, 255, 255, str);
}

void GnapEngine::scene52_run() {

	_timers[1] = 0;
	
	hideCursor();

	// TODO loadFont("maturasc", "Matura MT Script Capitals", 2000);
	// TODO setFontSize(24);

	_s52_gameScore = 0;
	_gameSys->drawTextToSurface(0, 300, 80, 255, 255, 255, "SCORE");
	_gameSys->drawTextToSurface(0, 468, 80, 255, 255, 255, "0");

	scene52_drawScore(0);

	_s52_shipMidX = 33;
	_s52_shipMidY = _gameSys->getSpriteHeightById(15);
	_s52_shipPosX = (800 - _s52_shipMidX) / 2;
	_s52_arcadeScreenBottom = 496;
	_s52_arcadeScreenRight = 595 - _s52_shipMidX;
	_s52_arcadeScreenLeft = 210;
	_s52_shipsLeft = 3;
	_s52_alienCounter = 0;

	_s52_shieldPosX[0] = 247;
	_s52_shieldPosX[1] = 387;
	_s52_shieldPosX[2] = 525;

	for (int i = 0; i < 3; ++i)
		_s52_shieldSpriteIds[i] = -1;

	_gameSys->drawSpriteToBackground(120, 140, 0xF);
	_gameSys->drawSpriteToBackground(120, 185, 0xF);

	scene52_initShipCannon(_s52_arcadeScreenBottom);
	scene52_initAlienCannons();
	scene52_initAliens();

	_s52_nextUfoSequenceId = 0x22;
	_gameSys->setAnimation(0x22, 256, 7);
	_gameSys->insertSequence(_s52_nextUfoSequenceId, 256, 0, 0, kSeqNone, 0, _s52_shipPosX, _s52_arcadeScreenBottom);

	_s52_ufoSequenceId = _s52_nextUfoSequenceId;

	clearKeyStatus1(Common::KEYCODE_RIGHT);
	clearKeyStatus1(Common::KEYCODE_LEFT);
	clearKeyStatus1(Common::KEYCODE_SPACE);
	clearKeyStatus1(Common::KEYCODE_UP);
	clearKeyStatus1(Common::KEYCODE_ESCAPE);

	_timers[2] = 5;
	_s52_shipFlag = false;

	_timers[0] = 10;
	_s52_alienWave = true;

	while (!_sceneDone) {

		gameUpdateTick();

		while (isKeyStatus2(Common::KEYCODE_RIGHT)) {
			scene52_update();
			if (_gameSys->getAnimationStatus(7) == 2) {
				if (_s52_shipPosX < _s52_arcadeScreenRight) {
					_s52_shipPosX += 15;
					if (_s52_shipPosX > _s52_arcadeScreenRight)
						_s52_shipPosX = _s52_arcadeScreenRight;
					_gameSys->setAnimation(_s52_nextUfoSequenceId, 256, 7);
					_gameSys->insertSequence(_s52_nextUfoSequenceId, 256, _s52_ufoSequenceId, 256, kSeqSyncWait, 0, _s52_shipPosX, _s52_arcadeScreenBottom);
					_s52_ufoSequenceId = _s52_nextUfoSequenceId;
					if (_s52_bottomAlienFlag && scene52_shipCannonHitAlien())
						scene52_shipExplode();
				}
				break;
			}
		}

		while (isKeyStatus2(Common::KEYCODE_LEFT)) {
			scene52_update();
			if (_gameSys->getAnimationStatus(7) == 2) {
				if (_s52_shipPosX > _s52_arcadeScreenLeft) {
					_s52_shipPosX -= 15;
					if (_s52_shipPosX < _s52_arcadeScreenLeft)
						_s52_shipPosX = _s52_arcadeScreenLeft;
					_gameSys->setAnimation(_s52_nextUfoSequenceId, 256, 7);
					_gameSys->insertSequence(_s52_nextUfoSequenceId, 256, _s52_ufoSequenceId, 256, kSeqSyncWait, 0, _s52_shipPosX, _s52_arcadeScreenBottom);
					_s52_ufoSequenceId = _s52_nextUfoSequenceId;
					if (_s52_bottomAlienFlag && scene52_shipCannonHitAlien())
						scene52_shipExplode();
				}
				break;
			}
		}
		
		scene52_update();
		
		if (sceneXX_sub_4466B1()) {
			_s52_alienWave = false;
			_gameSys->waitForUpdate();
			scene52_initAnims();
			clearKeyStatus1(30);
			_sceneDone = true;
		}

	}

	// TODO freeFont();

	_gameSys->waitForUpdate();
	
}

} // End of namespace Gnap
