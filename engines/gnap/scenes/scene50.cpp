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

int GnapEngine::scene50_init() {
	return 0xC7;
}

void GnapEngine::scene50_updateHotspots() {
	_hotspotsCount = 0;
}

bool GnapEngine::scene50_tongueWinsRound(int tongueNum) {
	if (tongueNum == 1)
		++_s50_leftTongueRoundsWon;
	else
		++_s50_rightTongueRoundsWon;
	scene50_playWinBadgeAnim(tongueNum);
	bool fightOver = _s50_rightTongueRoundsWon == 2 || _s50_leftTongueRoundsWon == 2;
	scene50_playWinAnim(tongueNum, fightOver);
	return fightOver;
}

void GnapEngine::scene50_playWinAnim(int tongueNum, bool fightOver) {
	if (tongueNum == 1) {
		if (fightOver) {
			_gameSys->insertSequence(0xAD, 140, 0xAC, 140, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xB4, 100, _s50_leftTongueSequenceId, _s50_leftTongueId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xBD, 100, _s50_rightTongueSequenceId, _s50_rightTongueId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xBC, 100, 0xBD, 100, kSeqSyncWait, 0, 0, 0);
			_s50_leftTongueSequenceId = 0xB4;
			_s50_rightTongueSequenceId = 0xBC;
			_s50_rightTongueId = 100;
			_s50_leftTongueId = 100;
			_gameSys->setAnimation(0xB4, 100, 6);
			_gameSys->setAnimation(_s50_rightTongueSequenceId, 100, 5);
			scene50_waitForAnim(6);
			scene50_waitForAnim(5);
			invAdd(kItemGum);
			setFlag(13);
		} else {
			_gameSys->insertSequence(0xB4, 100, _s50_leftTongueSequenceId, _s50_leftTongueId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xBD, 100, _s50_rightTongueSequenceId, _s50_rightTongueId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xBC, 100, 0xBD, 100, kSeqSyncWait, 0, 0, 0);
			_s50_leftTongueSequenceId = 0xB4;
			_s50_rightTongueSequenceId = 0xBC;
			_s50_rightTongueId = 100;
			_s50_leftTongueId = 100;
			_gameSys->setAnimation(0xB4, 100, 6);
			_gameSys->setAnimation(_s50_rightTongueSequenceId, 100, 5);
			scene50_waitForAnim(6);
			scene50_waitForAnim(5);
		}
	} else {
		_gameSys->insertSequence(0xBE, 100, _s50_rightTongueSequenceId, _s50_rightTongueId, kSeqSyncWait, 0, 0, 0);
		_gameSys->setAnimation(0xBE, 100, 5);
		scene50_waitForAnim(5);
		_gameSys->insertSequence(0xBF, 100, 0xBE, 100, kSeqSyncWait, 0, 0, 0);
		_gameSys->insertSequence(0xB5, 100, _s50_leftTongueSequenceId, _s50_leftTongueId, kSeqSyncWait, 0, 0, 0);
		_s50_rightTongueSequenceId = 0xBF;
		_s50_leftTongueSequenceId = 0xB5;
		_s50_rightTongueId = 100;
		_s50_leftTongueId = 100;
		_gameSys->setAnimation(0xB5, 100, 6);
		_gameSys->setAnimation(_s50_rightTongueSequenceId, 100, 5);
		scene50_waitForAnim(6);
		scene50_waitForAnim(5);
	}
	// TODO delayTicksA(1, 7);
}

void GnapEngine::scene50_delayTicks() {
	// TODO delayTicksA(3, 7);
}

void GnapEngine::scene50_initRound() {
	_s50_leftTongueEnergy = 10;
	_s50_rightTongueEnergy = 10;
	_s50_fightDone = false;
	_timers[3] = scene50_getRightTongueActionTicks();
	_timers[4] = 0;
	_timers[6] = 0;
	_gameSys->fillSurface(0, 91, 73, 260, 30, 212, 0, 0);
	_gameSys->fillSurface(0, 450, 73, 260, 30, 212, 0, 0);
	_s50_timeRemaining = 40;
	scene50_drawCountdown(40);
}

bool GnapEngine::scene50_updateCountdown() {
	if (!_timers[5]) {
		--_s50_timeRemaining;
		if (_s50_timeRemaining < 0) {
			return true;
		} else {
			_timers[5] = 15;
			scene50_drawCountdown(_s50_timeRemaining);
		}
	}
	return false;
}

void GnapEngine::scene50_drawCountdown(int value) {
	char str[8];
	sprintf(str, "%02d", value);
	_gameSys->fillSurface(0, 371, 505, 50, 27, 0, 0, 0);
	_gameSys->drawTextToSurface(0, 381, 504, 255, 255, 255, str);
}

void GnapEngine::scene50_playTonguesIdle() {
	_gameSys->insertSequence(0xBA, 100, _s50_leftTongueSequenceId, _s50_leftTongueId, kSeqSyncWait, 0, 0, 0);
	_gameSys->insertSequence(0xC2, 100, _s50_rightTongueSequenceId, _s50_rightTongueId, kSeqSyncWait, 0, 0, 0);
	_s50_leftTongueSequenceId = 0xBA;
	_s50_rightTongueSequenceId = 0xC2;
	_s50_rightTongueNextSequenceId = -1;
	_s50_leftTongueNextSequenceId = -1;
	_s50_leftTongueId = 100;
	_s50_rightTongueId = 100;
	_gameSys->setAnimation(0xC2, 100, 5);
	_gameSys->setAnimation(_s50_leftTongueSequenceId, _s50_leftTongueId, 6);
}

void GnapEngine::scene50_playRoundAnim(int roundNum) {
	int sequenceId;
	
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
	}

	_gameSys->insertSequence(sequenceId, 256, 0, 0, kSeqNone, 0, 0, 0);
	_gameSys->setAnimation(sequenceId, 256, 7);
	scene50_waitForAnim(7);

	_gameSys->insertSequence(0xAB, 256, sequenceId, 256, kSeqSyncWait, 0, 0, 0);
	_gameSys->setAnimation(0xAB, 256, 7);
	scene50_waitForAnim(7);

}

bool GnapEngine::scene50_updateEnergyBars(int newLeftBarPos, int newRightBarPos) {
	
	if (newLeftBarPos != _s50_leftTongueEnergyBarPos) {
		if (newLeftBarPos < 0)
			newLeftBarPos = 0;
		_s50_leftTongueEnergyBarPos = newLeftBarPos;
		_gameSys->fillSurface(0, 26 * newLeftBarPos + 91, 73, 260 - 26 * newLeftBarPos, 30, 0, 0, 0);
	}

	if (newRightBarPos != _s50_rightTongueEnergyBarPos) {
		if (newRightBarPos < 0)
			newRightBarPos = 0;
		_s50_rightTongueEnergyBarPos = newRightBarPos;
		if (newRightBarPos != 10)
			_gameSys->fillSurface(0, 26 * (9 - newRightBarPos) + 450, 73, 26, 30, 0, 0, 0);
	}

	if (newLeftBarPos * newRightBarPos > 0)
		return false;

	_s50_leftTongueEnergyBarPos = 10;
	_s50_rightTongueEnergyBarPos = 10;
	return true;
}

void GnapEngine::scene50_waitForAnim(int animationIndex) {
	while (_gameSys->getAnimationStatus(animationIndex) != 2) {
		gameUpdateTick();
	}
	_gameSys->setAnimation(0, 0, animationIndex);
}

int GnapEngine::scene50_checkInput() {
	int sequenceId = -1;

	if (isKeyStatus1(Common::KEYCODE_RIGHT)) {
		clearKeyStatus1(Common::KEYCODE_RIGHT);
		sequenceId = 0xB6;
	} else if (isKeyStatus1(Common::KEYCODE_LEFT)) {
		clearKeyStatus1(Common::KEYCODE_LEFT);
		sequenceId = 0xB3;
	} else if (isKeyStatus1(Common::KEYCODE_ESCAPE)) {
		clearKeyStatus1(Common::KEYCODE_ESCAPE);
		_s50_fightDone = true;
	}
	
	return sequenceId;
}

int GnapEngine::scene50_getRightTongueAction() {
	int sequenceId = -1;

	if (!_timers[3]) {
		_timers[3] = scene50_getRightTongueActionTicks();
		if (_s50_rightTongueEnergy >= _s50_leftTongueEnergy) {
			switch (getRandom(5)) {
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
			}
		} else {
			switch (getRandom(4)) {
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
			}
		}
	}

	return sequenceId;
}

void GnapEngine::scene50_updateAnimations() {
	
	if (!_timers[4])
		_s50_attackCounter = 0;

	if (_gameSys->getAnimationStatus(5) == 2) {
		if (_s50_rightTongueSequenceId == 0xBE) {
			if (_s50_leftTongueSequenceId != 0xB3 && _s50_leftTongueSequenceId != 0xB8)
				_s50_rightTongueNextSequenceId = 0xBF;
			else
				_s50_rightTongueNextSequenceId = 0xC0;
		}
		if (_s50_rightTongueNextSequenceId == -1)
			_s50_rightTongueNextSequenceId = 0xC2;
		if (_s50_rightTongueNextSequenceId == 0xBF) {
			_s50_leftTongueNextId = scene50_getLeftTongueNextId();
			_s50_rightTongueNextId = scene50_getRightTongueNextId();
			_gameSys->setAnimation(_s50_rightTongueNextSequenceId, _s50_rightTongueNextId, 5);
			_gameSys->setAnimation(0xB9, _s50_leftTongueNextId, 6);
			_gameSys->insertSequence(_s50_rightTongueNextSequenceId, _s50_rightTongueNextId, _s50_rightTongueSequenceId, _s50_rightTongueId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xB9, _s50_leftTongueNextId, _s50_leftTongueSequenceId, _s50_leftTongueId, kSeqSyncExists, 0, 0, 0);
			_s50_rightTongueSequenceId = _s50_rightTongueNextSequenceId;
			_s50_rightTongueNextSequenceId = -1;
			_s50_leftTongueSequenceId = 0xB9;
			_s50_leftTongueNextSequenceId = -1;
			_s50_rightTongueId = _s50_rightTongueNextId;
			_s50_leftTongueId = _s50_leftTongueNextId;
			_s50_leftTongueEnergy -= getRandom(2) + 1;//CHECKME
		} else {
			_s50_rightTongueNextId = scene50_getRightTongueNextId();
			_gameSys->setAnimation(_s50_rightTongueNextSequenceId, _s50_rightTongueNextId, 5);
			_gameSys->insertSequence(_s50_rightTongueNextSequenceId, _s50_rightTongueNextId, _s50_rightTongueSequenceId, _s50_rightTongueId, kSeqSyncWait, 0, 0, 0);
			_s50_rightTongueSequenceId = _s50_rightTongueNextSequenceId;
			_s50_rightTongueNextSequenceId = -1;
			_s50_rightTongueId = _s50_rightTongueNextId;
		}
	}

	if (_gameSys->getAnimationStatus(6) == 2) {
		if (_s50_leftTongueSequenceId == 0xB6) {
			++_s50_attackCounter;
			if (_s50_timesPlayedModifier + 3 <= _s50_attackCounter) {
				_s50_leftTongueNextSequenceId = 0xB8;
			} else {
				_timers[4] = 20;
				//CHECKME
				if (_s50_rightTongueSequenceId != 0xBB && _s50_rightTongueSequenceId != 0xC0 && getRandom(8) != _s50_roundNum)
					_s50_leftTongueNextSequenceId = 0xB7;
				else
					_s50_leftTongueNextSequenceId = 0xB8;
			}
		}
		if (_s50_leftTongueNextSequenceId == 0xB3)
			--_s50_attackCounter;
		if (_s50_leftTongueNextSequenceId == -1)
			_s50_leftTongueNextSequenceId = 0xBA;
		if (_s50_leftTongueNextSequenceId == 0xB7) {
			_s50_leftTongueNextId = scene50_getLeftTongueNextId();
			_s50_rightTongueNextId = scene50_getRightTongueNextId();
			_gameSys->setAnimation(_s50_leftTongueNextSequenceId, _s50_leftTongueNextId, 6);
			_gameSys->setAnimation(0xC1, _s50_rightTongueNextId, 5);
			_gameSys->insertSequence(_s50_leftTongueNextSequenceId, _s50_leftTongueNextId, _s50_leftTongueSequenceId, _s50_leftTongueId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xC1, _s50_rightTongueNextId, _s50_rightTongueSequenceId, _s50_rightTongueId, kSeqSyncExists, 0, 0, 0);
			_s50_leftTongueSequenceId = _s50_leftTongueNextSequenceId;
			_s50_leftTongueNextSequenceId = -1;
			_s50_rightTongueSequenceId = 0xC1;
			_s50_rightTongueNextSequenceId = -1;
			_s50_rightTongueId = _s50_rightTongueNextId;
			_s50_leftTongueId = _s50_leftTongueNextId;
			--_s50_rightTongueEnergy;
		} else if (_s50_leftTongueNextSequenceId != 0xB8 || _s50_rightTongueSequenceId != 0xC2) {
			_s50_leftTongueNextId = scene50_getLeftTongueNextId();
			_gameSys->setAnimation(_s50_leftTongueNextSequenceId, _s50_leftTongueNextId, 6);
			_gameSys->insertSequence(_s50_leftTongueNextSequenceId, _s50_leftTongueNextId, _s50_leftTongueSequenceId, _s50_leftTongueId, kSeqSyncWait, 0, 0, 0);
			_s50_leftTongueSequenceId = _s50_leftTongueNextSequenceId;
			_s50_leftTongueNextSequenceId = -1;
			_s50_leftTongueId = _s50_leftTongueNextId;
		} else {
			_s50_leftTongueNextId = scene50_getLeftTongueNextId();
			_s50_rightTongueNextId = scene50_getRightTongueNextId();
			_gameSys->setAnimation(0xBB, _s50_rightTongueNextId, 5);
			_gameSys->setAnimation(_s50_leftTongueNextSequenceId, _s50_leftTongueNextId, 6);
			_gameSys->insertSequence(_s50_leftTongueNextSequenceId, _s50_leftTongueNextId, _s50_leftTongueSequenceId, _s50_leftTongueId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0xBB, _s50_rightTongueNextId, _s50_rightTongueSequenceId, _s50_rightTongueId, kSeqSyncExists, 0, 0, 0);
			_s50_rightTongueSequenceId = 0xBB;
			_s50_rightTongueId = _s50_rightTongueNextId;
			_s50_rightTongueNextSequenceId = -1;
			_s50_leftTongueSequenceId = _s50_leftTongueNextSequenceId;
			_s50_leftTongueNextSequenceId = -1;
			_s50_leftTongueId = _s50_leftTongueNextId;
		}
	}

}

int GnapEngine::scene50_getRightTongueActionTicks() {
	return 15 - 5 * _s50_roundNum + 1;
}

int GnapEngine::scene50_getLeftTongueNextId() {
	_s50_leftTongueNextIdCtr = (_s50_leftTongueNextIdCtr + 1) % 3;
	return _s50_leftTongueNextIdCtr + 100;
}

int GnapEngine::scene50_getRightTongueNextId() {
	_s50_rightTongueNextIdCtr = (_s50_rightTongueNextIdCtr + 1) % 3;
	return _s50_rightTongueNextIdCtr + 100;
}

void GnapEngine::scene50_playWinBadgeAnim(int tongueNum) {
	int sequenceId;
	
	if (tongueNum == 1) {
		if (_s50_leftTongueRoundsWon == 1)
	  		sequenceId = 0xC3;
		else
	  		sequenceId = 0xC4;
	} else {
		if (_s50_rightTongueRoundsWon == 1)
			sequenceId = 0xC5;
		else
			sequenceId = 0xC6;
	}

	_gameSys->setAnimation(sequenceId, 120, 7);
	_gameSys->insertSequence(sequenceId, 120, 0, 0, kSeqNone, 0, 0, 0);
	scene50_waitForAnim(7);

}

void GnapEngine::scene50_run() {
	
	++_s50_timesPlayed;
	_s50_timesPlayedModifier = _s50_timesPlayed / 4;
	_s50_leftTongueRoundsWon = 0;
	_s50_rightTongueRoundsWon = 0;
	// scene50_initFont();
	_s50_leftTongueSequenceId = 186;
	_s50_rightTongueSequenceId = 194;
	_s50_rightTongueNextSequenceId = -1;
	_s50_leftTongueNextSequenceId = -1;
	_s50_leftTongueId = 100;
	_s50_rightTongueId = 100;

	_gameSys->setAnimation(194, 100, 5);
	_gameSys->setAnimation(_s50_leftTongueSequenceId, _s50_leftTongueId, 6);
	_gameSys->insertSequence(_s50_leftTongueSequenceId, _s50_leftTongueId, 0, 0, kSeqNone, 0, 0, 0);
	_gameSys->insertSequence(_s50_rightTongueSequenceId, _s50_rightTongueId, 0, 0, kSeqNone, 0, 0, 0);
	_gameSys->insertSequence(172, 140, 0, 0, kSeqNone, 0, 0, 0);
	endSceneInit();

	scene50_initRound();

	_s50_roundNum = 1;

	setGrabCursorSprite(-1);
	hideCursor();

	// TODO delayTicksA(1, 7);

	scene50_playRoundAnim(_s50_roundNum);

	_timers[5] = 15;

	while (!_s50_fightDone) {

		/* TODO
		if (sceneXX_sub_4466B1())
			_s50_fightDone = true;
		*/

		int playerSequenceId = scene50_checkInput();
		if (playerSequenceId != -1)
			_s50_leftTongueNextSequenceId = playerSequenceId;

		int rightSequenceId = scene50_getRightTongueAction();
		if (rightSequenceId != -1)
			_s50_rightTongueNextSequenceId = rightSequenceId;

		scene50_updateAnimations();

		if (scene50_updateCountdown() ||
			scene50_updateEnergyBars(_s50_leftTongueEnergy, _s50_rightTongueEnergy)) {
			int v0;
			if (_s50_rightTongueEnergy < _s50_leftTongueEnergy)
				v0 = scene50_tongueWinsRound(1);
			else
				v0 = scene50_tongueWinsRound(2);
			if (v0) {
				scene50_delayTicks();
				_s50_fightDone = true;
			} else {
				++_s50_roundNum;
				scene50_initRound();
				scene50_playTonguesIdle();
				scene50_updateEnergyBars(_s50_leftTongueEnergy, _s50_rightTongueEnergy);
				scene50_playRoundAnim(_s50_roundNum);
				_timers[5] = 15;
			}
		}
		
		gameUpdateTick();

	}

	// scene50_freeFont();
	
	_gameSys->setAnimation(0, 0, 7);
	_gameSys->setAnimation(0, 0, 6);
	_gameSys->setAnimation(0, 0, 5);
	_gameSys->setAnimation(0, 0, 3);

	showCursor();
}

} // End of namespace Gnap
