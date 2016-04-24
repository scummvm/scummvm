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
#include "gnap/scenes/scene51.h"

namespace Gnap {

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
	_s51_dropLoseCash = false;

	_s51_cashAmount = -1;
	_s51_guySequenceId = -1;
	_s51_guyNextSequenceId = -1;
	_s51_itemsCaughtCtr = -1;
	_s51_dropSpeedTicks = -1;
	_s51_nextDropItemKind = -1;
	_s51_itemInsertX = -1;
	_s51_itemInsertDirection = -1;
	_s51_platypusSequenceId = -1;
	_s51_platypusNextSequenceId = -1;
	_s51_platypusJumpSequenceId = -1;
	_s51_itemsCtr = -1;
	_s51_itemsCtr1 = -1;
	_s51_itemsCtr2 = -1;

	for (int i = 0; i < 4; i++) {
		_s51_digits[i] = 0;
		_s51_digitSequenceIds[4] = -1;
	}

	for (int i = 0; i < 6; i++) {
		_s51_items[i]._currSequenceId = -1;
		_s51_items[i]._droppedSequenceId;
		_s51_items[i]._x = 0;
		_s51_items[i]._y = 0;
		_s51_items[i]._collisionX;
		_s51_items[i]._canCatch = false;
		_s51_items[i]._isCollision = false;
		_s51_items[i]._x2 = 0;
		_s51_items[i]._id = -1;
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
	while (index < 6 && _s51_items[index]._currSequenceId)
		++index;

	if (index == 6)
		return;

	switch (_s51_nextDropItemKind) {
	case 0:
		if (_vm->getRandom(10) != 0 || _s51_itemsCtr2 >= 2) {
			_s51_items[index]._currSequenceId = 0xBD;
		} else {
			--_s51_itemsCtr1;
			_s51_items[index]._currSequenceId = 0xBA;
			++_s51_itemsCtr2;
		}
		break;

	case 1:
		if (_vm->getRandom(8) != 0 || _s51_itemsCtr2 >= 2) {
			if (_vm->getRandom(5) == 0) {
				if (_s51_itemInsertDirection)
					_s51_itemInsertX -= 70;
				else
					_s51_itemInsertX += 70;
			}
			_s51_items[index]._currSequenceId = 0xBD;
		} else {
			--_s51_itemsCtr1;
			_s51_items[index]._currSequenceId = 0xBA;
			++_s51_itemsCtr2;
		}
		break;

	case 2:
		if (_vm->getRandom(6) != 0 || _s51_itemsCtr2 >= 2) {
			_s51_items[index]._currSequenceId = 0xBD;
		} else {
			--_s51_itemsCtr1;
			_s51_items[index]._currSequenceId = 0xBA;
			++_s51_itemsCtr2;
		}
		break;

	case 3:
	case 4:
		if (_s51_itemsCtr == 0)
			_s51_itemsCtr1 = 3;
		_s51_items[index]._currSequenceId = 0xC0;
		break;

	case 5:
	case 6:
		if (_vm->getRandom(5) != 0 || _s51_itemsCtr2 >= 2) {
			if (_vm->getRandom(5) != 0)
				_s51_items[index]._currSequenceId = 0xBD;
			else
				_s51_items[index]._currSequenceId = 0xC0;
		} else {
			--_s51_itemsCtr1;
			_s51_items[index]._currSequenceId = 0xBA;
			++_s51_itemsCtr2;
		}
		break;

	case 7:
		if (_vm->getRandom(5) != 0 || _s51_itemsCtr2 >= 2) {
			if (_vm->getRandom(5) == 0) {
				if (_s51_itemInsertDirection)
					_s51_itemInsertX -= 40;
				else
					_s51_itemInsertX += 40;
			}
			if (_vm->getRandom(9) != 0)
				_s51_items[index]._currSequenceId = 0xBD;
			else
				_s51_items[index]._currSequenceId = 0xC0;
		} else {
			--_s51_itemsCtr1;
			_s51_items[index]._currSequenceId = 0xBA;
			++_s51_itemsCtr2;
		}
		break;

	default:
		if (_vm->getRandom(4) != 0 || _s51_itemsCtr2 >= 2) {
			if (_vm->getRandom(9) != 0)
				_s51_items[index]._currSequenceId = 0xBD;
			else
				_s51_items[index]._currSequenceId = 0xC0;
		} else {
			--_s51_itemsCtr1;
			_s51_items[index]._currSequenceId = 0xBA;
			++_s51_itemsCtr2;
		}
		break;
	}

	if (_s51_itemInsertDirection) {
		_s51_itemInsertX -= 73;
		if (_s51_itemInsertX < 129) {
			_s51_itemInsertX += 146;
			_s51_itemInsertDirection = 0;
		}
	} else {
		_s51_itemInsertX += 73;
		if (_s51_itemInsertX > 685) {
			_s51_itemInsertX -= 146;
			_s51_itemInsertDirection = 1;
		}
	}

	if (_s51_itemInsertX > 685)
		_s51_itemInsertX = 685;

	if (_s51_itemInsertX < 129)
		_s51_itemInsertX = 129;

	if (_s51_items[index]._currSequenceId == 0xBA) {
		_s51_items[index]._x2 = _vm->getRandom(350) + 200;
		_s51_items[index]._x = _s51_items[index]._x2 - 362;
		_s51_items[index]._y = 15;
		_s51_items[index]._id = 249 - index;
	} else {
		_s51_items[index]._collisionX = _s51_itemInsertX;
		_s51_items[index]._x = _s51_items[index]._collisionX - 395;
		if (_s51_items[index]._currSequenceId == 0xC0)
			_s51_items[index]._x -= 65;
		_s51_items[index]._id = index + 250;
		_s51_items[index]._canCatch = true;
	}

	_vm->_gameSys->setAnimation(_s51_items[index]._currSequenceId, _s51_items[index]._id, index + 1);
	_vm->_gameSys->insertSequence(_s51_items[index]._currSequenceId, _s51_items[index]._id, 0, 0,
		kSeqNone, 0, _s51_items[index]._x, _s51_items[index]._y);

	_vm->_timers[0] = _s51_dropSpeedTicks;

	if (_s51_nextDropItemKind >= 3)
		_vm->_timers[0] = 20;

	if (_s51_nextDropItemKind >= 5)
		_vm->_timers[0] = 5;

	if (_s51_nextDropItemKind == 8)
		_vm->_timers[0] = 4;

	++_s51_itemsCtr;
}

void Scene51::updateItemAnimations() {
	for (int i = 0; i < 6; ++i) {
		if (_vm->_gameSys->getAnimationStatus(i + 1) == 2)
			updateItemAnimation(&_s51_items[i], i);
	}
}

int Scene51::checkCollision(int sequenceId) {
	bool jumpingLeft = false, jumpingRight = false;
	int v8, v4;
	int result = 0;

	if (!isJumping(sequenceId))
		return false;

	bool checkFl = false;
	for (int i = 0; i < 6; i++)
		checkFl |= _s51_items[i]._isCollision;
		
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
			if (_s51_items[i]._isCollision) {
				if (jumpingRight && _s51_items[i]._x2 > v8 && _s51_items[i]._x2 < v4) {
					v5 = v8 - 359;
					if (v5 == 0)
						v5 = 1;
					_s51_platypusNextSequenceId = 0xB6;
					break;
				} else if (jumpingLeft && _s51_items[i]._x2 < v4 && _s51_items[i]._x2 > v8) {
					v5 = v8 - 344;
					if (v5 == 0)
						v5 = 1;
					_s51_platypusNextSequenceId = 0xB7;
					break;
				}
			}
		}
		if (v5) {
			_vm->_gameSys->setAnimation(0xBC, _s51_items[i]._id, i + 1);
			_vm->_gameSys->insertSequence(0xBC, _s51_items[i]._id, _s51_items[i]._currSequenceId, _s51_items[i]._id, kSeqSyncWait, 0, _s51_items[i]._x, 15);
			_s51_items[i]._isCollision = false;
			_s51_items[i]._currSequenceId = 0xBC;
			--_s51_itemsCtr2;
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
			if (_s51_dropLoseCash) {
				if (item->_currSequenceId == 0xBD)
					_s51_cashAmount -= 2;
				else
					_s51_cashAmount -= 25;
				if (_s51_cashAmount < 0)
					_s51_cashAmount = 0;
				updateCash(_s51_cashAmount);
			}
			item->_droppedSequenceId = item->_currSequenceId + 1;
			if (item->_currSequenceId != 0xC0) {
				item->_canCatch = false;
				_s51_dropLoseCash = true;
				_s51_itemsCtr = 0;
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
			_vm->playSound(218, false);
			if (incCashAmount(item->_currSequenceId) == 1995) {
				winMinigame();
				_vm->_sceneDone = true;
			} else {
				clearItem(item);
				++_s51_itemsCaughtCtr;
				if (_s51_itemsCaughtCtr == 5)
					--_s51_dropSpeedTicks;
				if (_s51_itemsCaughtCtr == 8)
					--_s51_dropSpeedTicks;
				if (_s51_itemsCaughtCtr == 11)
					--_s51_dropSpeedTicks;
				if (_s51_itemsCaughtCtr == 14)
					--_s51_dropSpeedTicks;
				if (_s51_itemsCaughtCtr >= 15 && _s51_dropSpeedTicks > 4)
					--_s51_dropSpeedTicks;
				if (_s51_itemsCtr1 <= _s51_itemsCaughtCtr) {
					++_s51_nextDropItemKind;
					_s51_dropSpeedTicks = 10;
					_s51_itemsCtr = 0;
					_s51_itemsCtr1 = 20;
					_s51_dropLoseCash = false;
					_s51_itemsCaughtCtr = 0;
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
		if (_s51_items[i]._isCollision) {
			_vm->_gameSys->removeSequence(_s51_items[i]._currSequenceId, _s51_items[i]._id, true);
			_vm->_gameSys->setAnimation(0, 0, i + 1);
			clearItem(&_s51_items[i]);
		}
	}
	_s51_itemsCtr2 = 0;
}

int Scene51::itemIsCaught(Scene51Item *item) {
	if (!item->_canCatch)
		return 0;

	if (isJumpingRight(_s51_platypusJumpSequenceId)) {
		int v4 = getPosRight(_s51_platypusJumpSequenceId) + 97;
		if (item->_collisionX < v4 && v4 - item->_collisionX < 56)
			return 1;
	} else {
		int v2 = getPosLeft(_s51_platypusJumpSequenceId);
		if (item->_collisionX > v2 && item->_collisionX - v2 < 56)
			return 1;
	}

	if (item->_currSequenceId == 0xC1) {
		int v3 = item->_collisionX + 100;
		if (isJumpingRight(_s51_platypusJumpSequenceId)) {
			if (ABS(getPosRight(_s51_platypusJumpSequenceId) + 46 - v3) < 56)
				return 1;
		} else if (ABS(getPosLeft(_s51_platypusJumpSequenceId) + 46 - v3) < 56) {
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
	while (_vm->_gameSys->getAnimationStatus(animationIndex) != 2) {
		// pollMessages();
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

	_s51_platypusSequenceId = 0x76;
	_s51_platypusNextSequenceId = 0x76;

	for (int i = 0; i < 6; ++i)
		clearItem(&_s51_items[i]);

	_s51_items[0]._currSequenceId = 0xBA;
	_s51_items[0]._x2 = 320;
	_s51_items[0]._x = -42;
	_s51_items[0]._y = 15;
	_s51_items[0]._id = 249;
	_s51_items[0]._isCollision = true;

	_vm->_gameSys->insertSequence(_s51_platypusSequenceId, 256, 0, 0, kSeqNone, 0, -179, 0);
	_vm->_gameSys->insertSequence(0xBA, 249, 0, 0, kSeqNone, 0, _s51_items[0]._x, _s51_items[0]._y);
	_vm->_gameSys->setAnimation(0xBA, 249, 1);
	_vm->_gameSys->setAnimation(_s51_platypusSequenceId, 256, 0);

	while (_s51_platypusSequenceId < 0x80) {
		waitForAnim(0);
		++_s51_platypusNextSequenceId;
		_vm->_gameSys->setAnimation(_s51_platypusNextSequenceId, 256, 0);
		_vm->_gameSys->insertSequence(_s51_platypusNextSequenceId, 256, _s51_platypusSequenceId, 256, kSeqSyncWait, 0, -179, 0);
		_s51_platypusSequenceId = _s51_platypusNextSequenceId;
		++soundCtr;
		if (soundCtr % 4 == 0)
			_vm->playSound(214, false);
	}

	_s51_platypusNextSequenceId = 0x75;

	while (_s51_platypusSequenceId != 0x84) {
		waitForAnim(0);
		++_s51_platypusNextSequenceId;
		int oldSequenceId = _s51_platypusNextSequenceId;
		int v0 = checkCollision(_s51_platypusNextSequenceId);
		_vm->_gameSys->setAnimation(_s51_platypusNextSequenceId, 256, 0);
		_vm->_gameSys->insertSequence(_s51_platypusNextSequenceId, 256, _s51_platypusSequenceId, 256, kSeqSyncWait, 0, v0, 0);
		_s51_platypusSequenceId = _s51_platypusNextSequenceId;
		if (v0) {
			_s51_platypusNextSequenceId = oldSequenceId;
		} else {
			++soundCtr;
			if (soundCtr % 4 == 0)
				_vm->playSound(214, false);
		}
	}
	waitForAnim(0);
}

void Scene51::updateGuyAnimation() {
	if (!_vm->_timers[4]) {
		_vm->_timers[4] = _vm->getRandom(20) + 60;

		switch (_vm->getRandom(5)) {
		case 0:
			_s51_guyNextSequenceId = 0xC3;
			break;
		case 1:
			_s51_guyNextSequenceId = 0xC4;
			break;
		case 2:
			_s51_guyNextSequenceId = 0xC5;
			break;
		case 3:
			_s51_guyNextSequenceId = 0xC6;
			break;
		case 4:
			_s51_guyNextSequenceId = 0xC7;
			break;
		}
	
		_vm->_gameSys->insertSequence(_s51_guyNextSequenceId, 39, _s51_guySequenceId, 39, kSeqSyncWait, 0, 0, 0);
		_s51_guySequenceId = _s51_guyNextSequenceId;
		_s51_guyNextSequenceId = -1;
	}
}

int Scene51::incCashAmount(int sequenceId) {
	switch (sequenceId) {
	case 0xBD:
		_s51_cashAmount += 10;
		break;
	case 0xC0:
	case 0xC1:
		_s51_cashAmount += 100;
		break;
	case 0xB6:
	case 0xB7:
		_s51_cashAmount -= 10 * _vm->getRandom(5) + 50;
		if (_s51_cashAmount < 0)
			_s51_cashAmount = 0;
		break;
	}
	if (_s51_cashAmount > 1995)
		_s51_cashAmount = 1995;
	updateCash(_s51_cashAmount);
	return _s51_cashAmount;
}

void Scene51::winMinigame() {
	updateCash(1995);
	_vm->playSound(218, false);
	// TODO delayTicksA(1, 5);
	_vm->_newSceneNum = 48;
	_vm->invRemove(kItemBanana);
}

void Scene51::playCashAppearAnim() {
	_vm->_gameSys->setAnimation(0xC8, 252, 0);
	_vm->_gameSys->insertSequence(0xC8, 252, 0, 0, kSeqNone, 0, -20, -20);
	while (_vm->_gameSys->getAnimationStatus(0) != 2) {
		_vm->gameUpdateTick();
		// checkGameAppStatus();
	}
}

void Scene51::updateCash(int amount) {
	drawDigit(amount / 1000, 0);
	drawDigit(amount / 100 % 10, 1);
	drawDigit(amount / 10 % 10, 2);
	drawDigit(amount % 10, 3);
}

void Scene51::drawDigit(int digit, int position) {
	if (digit != _s51_digits[position]) {
		_vm->_gameSys->insertSequence(kDigitSequenceIds[digit], 253,
			_s51_digitSequenceIds[position], 253,
			kSeqSyncWait, 0, kDigitPositions[position] - 20, -20);
		_s51_digitSequenceIds[position] = kDigitSequenceIds[digit];
		_s51_digits[position] = digit;
	}
}

void Scene51::initCashDisplay() {
	for (int position = 0; position < 4; ++position) {
		_s51_digits[position] = 0;
		_s51_digitSequenceIds[position] = kDigitSequenceIds[0];
		_vm->_gameSys->insertSequence(kDigitSequenceIds[0], 253, 0, 0,
			kSeqNone, 0, kDigitPositions[position] - 20, -20);
	}
	_s51_cashAmount = 0;
}

void Scene51::run() {
	int soundCtr = 0;
	bool isIdle = true;

	_s51_itemsCtr = 0;
	_vm->_newSceneNum = _vm->_prevSceneNum;
	_s51_cashAmount = 0;
	_s51_platypusJumpSequenceId = 0x84;
	_vm->endSceneInit();

	_vm->hideCursor();
	_vm->setGrabCursorSprite(-1);

	_s51_guySequenceId = 0xC3;
	_s51_guyNextSequenceId = -1;

	_vm->_gameSys->insertSequence(0xC3, 39, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_timers[4] = _vm->getRandom(20) + 60;

	playCashAppearAnim();
	initCashDisplay();
	playIntroAnim();

	_s51_platypusNextSequenceId = 0x74;
	_vm->_gameSys->setAnimation(0x74, 256, 0);
	_vm->_gameSys->insertSequence(_s51_platypusNextSequenceId, 256, _s51_platypusSequenceId, 256, kSeqSyncWait, 0, getPosRight(_s51_platypusJumpSequenceId) - 362, 0);
	_s51_platypusSequenceId = _s51_platypusNextSequenceId;

	_s51_itemInsertDirection = 0;
	_s51_itemInsertX = 685;
	_s51_dropSpeedTicks = 10;
	_s51_nextDropItemKind = 0;

	for (int i = 0; i < 6; ++i)
		clearItem(&_s51_items[i]);

	_s51_itemInsertX = _vm->getRandom(556) + 129;
	_vm->_timers[0] = 15;

	_s51_itemsCaughtCtr = 0;
	_s51_dropLoseCash = false;
	_s51_itemsCtr1 = 20;

	_vm->clearKeyStatus1(Common::KEYCODE_RIGHT);
	_vm->clearKeyStatus1(Common::KEYCODE_LEFT);
	_vm->clearKeyStatus1(Common::KEYCODE_UP);
	_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
	_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);

	bool isCollision = false;
	bool startWalk = true;

	while (!_vm->_sceneDone) {
		if (_vm->sceneXX_sub_4466B1())
			_vm->_sceneDone = true;

		_vm->gameUpdateTick();

		updateGuyAnimation();
		dropNextItem();
		updateItemAnimations();

		if (_vm->isKeyStatus2(Common::KEYCODE_UP) || _vm->isKeyStatus2(Common::KEYCODE_SPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_UP);
			_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
			if (isJumpingRight(_s51_platypusJumpSequenceId)) {
				waitForAnim(0);
				_vm->_gameSys->setAnimation(0xB8, 256, 0);
				_vm->_gameSys->insertSequence(0xB8, 256, _s51_platypusSequenceId, 256, kSeqSyncWait, 0, getPosRight(_s51_platypusJumpSequenceId) - 348, 0);
				_s51_platypusSequenceId = 0xB8;
				waitForAnim(0);
				_s51_platypusNextSequenceId += 6;
				if (_s51_platypusNextSequenceId > 0x95)
					_s51_platypusNextSequenceId = 0x95;
				_s51_platypusJumpSequenceId = _s51_platypusNextSequenceId;
			} else {
				waitForAnim(0);
				_vm->_gameSys->setAnimation(0xB9, 256, 0);
				_vm->_gameSys->insertSequence(0xB9, 256, _s51_platypusSequenceId, 256, kSeqSyncWait, 0, getPosLeft(_s51_platypusJumpSequenceId) - 338, 0);
				_s51_platypusSequenceId = 0xB9;
				waitForAnim(0);
				_s51_platypusNextSequenceId += 6;
				if (_s51_platypusNextSequenceId > 0xB5)
					_s51_platypusNextSequenceId = 0xB5;
				_s51_platypusJumpSequenceId = _s51_platypusNextSequenceId;
			}
			isIdle = false;
		}

		while (_vm->isKeyStatus2(Common::KEYCODE_RIGHT) && _s51_platypusNextSequenceId != 0x96) {
			// pollMessages();
			if (_s51_platypusNextSequenceId == 0xB6)
				_s51_platypusNextSequenceId = 0x76;
			updateItemAnimations();
			if (startWalk) {
				_s51_platypusNextSequenceId = 0x86;
				startWalk = false;
			}

			if (_vm->_gameSys->getAnimationStatus(0) == 2) {
				int collisionX = checkCollision(_s51_platypusNextSequenceId);
				if (collisionX)
					incCashAmount(_s51_platypusNextSequenceId);
				_vm->_gameSys->setAnimation(_s51_platypusNextSequenceId, 256, 0);
				_vm->_gameSys->insertSequence(_s51_platypusNextSequenceId, 256, _s51_platypusSequenceId, 256, kSeqSyncWait, 0, collisionX, 0);
				_s51_platypusSequenceId = _s51_platypusNextSequenceId;
				if (collisionX) {
					isCollision = true;
					++_s51_platypusJumpSequenceId;
					_s51_platypusNextSequenceId = _s51_platypusJumpSequenceId;
				} else {
					_s51_platypusJumpSequenceId = _s51_platypusNextSequenceId;
				}
				if (isJumpingRight(_s51_platypusJumpSequenceId)) {
					++_s51_platypusNextSequenceId;
					if (!isCollision) {
						if (_vm->isKeyStatus2(Common::KEYCODE_UP) || _vm->isKeyStatus2(Common::KEYCODE_SPACE)) {
							_vm->clearKeyStatus1(Common::KEYCODE_UP);
							_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
							waitForAnim(0);
							_vm->_gameSys->setAnimation(0xB8, 256, 0);
							_vm->_gameSys->insertSequence(0xB8, 256, _s51_platypusSequenceId, 256, kSeqSyncWait, 0, getPosRight(_s51_platypusJumpSequenceId) - 348, 0);
							_s51_platypusSequenceId = 0xB8;
							waitForAnim(0);
							_s51_platypusNextSequenceId += 6;
							if (_s51_platypusNextSequenceId > 0x95)
								_s51_platypusNextSequenceId = 0x95;
							_s51_platypusJumpSequenceId = _s51_platypusNextSequenceId;
						} else {
							++soundCtr;
							if (soundCtr % 4 == 0)
								_vm->playSound(214, false);
						}
					}
				} else {
					_s51_platypusNextSequenceId = 150 - (_s51_platypusJumpSequenceId - 150);
				}
				isCollision = false;
				isIdle = false;
			}
			_vm->gameUpdateTick();
		}

		while (_vm->isKeyStatus2(Common::KEYCODE_LEFT) && _s51_platypusNextSequenceId != 0xB6) {
			// pollMessages();
			updateItemAnimations();
			if (startWalk) {
				_s51_platypusNextSequenceId = 0xA5;
				startWalk = false;
			}

			if (_vm->_gameSys->getAnimationStatus(0) == 2) {
				int collisionX = checkCollision(_s51_platypusNextSequenceId);
				if (collisionX)
					incCashAmount(_s51_platypusNextSequenceId);
				_vm->_gameSys->setAnimation(_s51_platypusNextSequenceId, 256, 0);
				_vm->_gameSys->insertSequence(_s51_platypusNextSequenceId, 256, _s51_platypusSequenceId, 256, kSeqSyncWait, 0, collisionX, 0);
				_s51_platypusSequenceId = _s51_platypusNextSequenceId;
				if (collisionX) {
					isCollision = true;
					++_s51_platypusJumpSequenceId;
					_s51_platypusNextSequenceId = _s51_platypusJumpSequenceId;
				} else {
					_s51_platypusJumpSequenceId = _s51_platypusNextSequenceId;
				}
				if (isJumpingLeft(_s51_platypusJumpSequenceId)) {
					++_s51_platypusNextSequenceId;
					if (!isCollision) {
						if (_vm->isKeyStatus2(Common::KEYCODE_UP) || _vm->isKeyStatus2(Common::KEYCODE_SPACE)) {
							_vm->clearKeyStatus1(Common::KEYCODE_UP);
							_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
							waitForAnim(0);
							_vm->_gameSys->setAnimation(0xB9, 256, 0);
							_vm->_gameSys->insertSequence(0xB9, 256, _s51_platypusSequenceId, 256, kSeqSyncWait, 0, getPosLeft(_s51_platypusJumpSequenceId) - 338, 0);
							_s51_platypusSequenceId = 0xB9;
							waitForAnim(0);
							_s51_platypusNextSequenceId += 6;
							if (_s51_platypusNextSequenceId > 0xB5)
								_s51_platypusNextSequenceId = 0xB5;
							_s51_platypusJumpSequenceId = _s51_platypusNextSequenceId;
						} else {
							++soundCtr;
							if (soundCtr % 4 == 0)
								_vm->playSound(214, false);
						}
					}
				} else {
					_s51_platypusNextSequenceId = 182 - (_s51_platypusJumpSequenceId - 118);
				}
				isCollision = false;
				isIdle = false;
			}
			_vm->gameUpdateTick();
		}

		if (!isIdle && _vm->_gameSys->getAnimationStatus(0) == 2) {
			if (isJumpingRight(_s51_platypusJumpSequenceId)) {
				_vm->_gameSys->setAnimation(0x74, 256, 0);
				_vm->_gameSys->insertSequence(0x74, 256, _s51_platypusSequenceId, 256, kSeqSyncWait, 0, getPosRight(_s51_platypusJumpSequenceId) - 362, 0);
				_s51_platypusSequenceId = 0x74;
			} else {
				_vm->_gameSys->setAnimation(0x75, 256, 0);
				_vm->_gameSys->insertSequence(0x75, 256, _s51_platypusSequenceId, 256, kSeqSyncWait, 0, getPosLeft(_s51_platypusJumpSequenceId) - 341, 0);
				_s51_platypusSequenceId = 0x75;
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

} // End of namespace Gnap
