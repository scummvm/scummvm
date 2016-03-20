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

enum {
	kHSPlatypus		= 0,
	kHSExitUfoParty	= 1,
	kHSExitUfo		= 2,
	kHSExitShow		= 3,
	kHSKissingLady	= 4,
	kHSSpring		= 5,
	kHSSpringGuy	= 6,
	kHSDevice		= 7,
	kHSWalkArea1	= 8,
	kHSWalkArea2	= 9
};

enum {
	kHSUfoExitLeft	= 1,
	kHSUfoExitRight	= 2,
	kHSUfoDevice	= 3
};

int GnapEngine::scene44_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	_gameSys->setAnimation(0, 0, 2);
	_gameSys->setAnimation(0, 0, 3);
	return 0xFF;
}

void GnapEngine::scene44_updateHotspots() {
	if (isFlag(12)) {
		setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
		setHotspot(kHSUfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		setHotspot(kHSUfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		setDeviceHotspot(kHSUfoDevice, -1, 534, -1, 599);
		_hotspotsCount = 4;
	} else {
		setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		setHotspot(kHSExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		setHotspot(kHSExitUfo, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		setHotspot(kHSExitShow, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		setHotspot(kHSKissingLady, 300, 160, 400, 315, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
		setHotspot(kHSSpring, 580, 310, 635, 375, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
		setHotspot(kHSSpringGuy, 610, 375, 690, 515, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
		setHotspot(kHSWalkArea1, 0, 0, 800, 445);
		setHotspot(kHSWalkArea2, 617, 0, 800, 600);
		setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
		if (isFlag(13))
			_hotspots[kHSKissingLady]._flags = SF_DISABLED;
		if (isFlag(15))
			_hotspots[kHSSpring]._flags = SF_DISABLED;
		_hotspotsCount = 10;
	}
}

void GnapEngine::scene44_run() {
	queueInsertDeviceIcon();

	_gameSys->insertSequence(0xF7, 0, 0, 0, kSeqLoop, 0, 0, 0);
	_gameSys->insertSequence(0xFC, 256, 0, 0, kSeqNone, 0, 0, 0);

	if (isFlag(15))
		_currSpringGuySequenceId = 0xF8;
	else
		_currSpringGuySequenceId = 0xF9;
	
	_nextSpringGuySequenceId = -1;
	_gameSys->setAnimation(_currSpringGuySequenceId, 1, 4);
	_gameSys->insertSequence(_currSpringGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (isFlag(13)) {
		if (_prevSceneNum != 50 || _sceneSavegameLoaded) {
			_currKissingLadySequenceId = 0xF6;
			_nextKissingLadySequenceId = -1;
		} else {
			setGrabCursorSprite(kItemGum);
			_currKissingLadySequenceId = 0xF5;
			_nextKissingLadySequenceId = 0xF6;
			_gameSys->setAnimation(0xF5, 1, 2);
		}
	} else {
		_currKissingLadySequenceId = 0xEC;
		_nextKissingLadySequenceId = -1;
		_gameSys->setAnimation(0xEC, 1, 2);
	}
	
	_gameSys->insertSequence(_currKissingLadySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (isFlag(12)) {
		_toyUfoId = 0;
		_toyUfoActionStatus = -1;
		_toyUfoSequenceId = toyUfoGetSequenceId();
		_toyUfoNextSequenceId = _toyUfoSequenceId;
		if (_prevSceneNum == 43)
			_toyUfoX = 30;
		else
			_toyUfoX = 770;
		_gameSys->setAnimation(_toyUfoSequenceId | 0x10000, _toyUfoId, 3);
		_gameSys->insertSequence(_toyUfoSequenceId | 0x10000, _toyUfoId, 0, 0, kSeqNone, 0, _toyUfoX - 274, _toyUfoY - 128);
		endSceneInit();
	} else {
		switch (_prevSceneNum) {
		case 43:
			initGnapPos(-1, 8, 7);
			initBeaverPos(-1, 7, 5);
			endSceneInit();
			gnapWalkTo(2, 8, -1, 0x107B9, 1);
			platypusWalkTo(1, 8, -1, 0x107C2, 1);
			break;
		case 46:
			initGnapPos(11, 8, 7);
			initBeaverPos(11, 8, 5);
			endSceneInit();
			gnapWalkTo(6, 8, -1, 0x107BA, 1);
			platypusWalkTo(7, 8, -1, 0x107D2, 1);
			break;
		case 50:
			initGnapPos(4, 8, 1);
			if (_sceneSavegameLoaded) {
				initBeaverPos(_hotspotsWalkPos[4].x, _hotspotsWalkPos[4].y, 4);
			} else if (!isFlag(13)) {
				_timers[0] = 50;
				_timers[1] = 20;
				_platX = 5;
				_platY = 8;
				_beaverSequenceId = 0xFD;
				_beaverFacing = 0;
				_beaverId = 160;
				_beaverSequenceDatNum = 0;
				_gameSys->insertSequence(0xFD, 160, 0, 0, kSeqNone, 0, 0, 0);
			}
			endSceneInit();
			break;
		default:
			initGnapPos(5, 11, 7);
			initBeaverPos(6, 11, 5);
			endSceneInit();
			platypusWalkTo(6, 8, -1, 0x107C2, 1);
			gnapWalkTo(5, 8, -1, 0x107BA, 1);
			break;
		}
	}
	
	while (!_sceneDone) {
		if (!isSoundPlaying(0x1094B))
			playSound(0x1094B, 1);
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		testWalk(0, 0, -1, -1, -1, -1);
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		if (isFlag(12)) {

			switch (_sceneClickedHotspot) {

			case kHSUfoExitLeft:
				if (_toyUfoActionStatus < 0) {
					_isLeavingScene = 1;
					_toyUfoActionStatus = 6;
					_newSceneNum = 43;
					toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 3);
				}
				break;

			case kHSUfoExitRight:
				if (_toyUfoActionStatus < 0) {
					_isLeavingScene = 1;
					_toyUfoActionStatus = 6;
					_newSceneNum = 46;
					toyUfoFlyTo(835, -1, 0, 835, 0, 300, 3);
				}
				break;

			case kHSUfoDevice:
				runMenu();
				scene44_updateHotspots();
				_timers[4] = getRandom(20) + 20;
				break;

			}

		} else if (_sceneClickedHotspot <= 9) {

			switch (_sceneClickedHotspot) {

			case kHSDevice:
				runMenu();
				scene44_updateHotspots();
				_timers[4] = getRandom(20) + 20;
				break;
			
			case kHSPlatypus:
				if (_gnapActionStatus < 0) {
					if (_grabCursorSpriteIndex >= 0) {
						playGnapImpossible(_platX, _platY);
					} else {
						switch (_verbCursor) {
						case LOOK_CURSOR:
							playGnapMoan1(_platX, _platY);
							break;
						case GRAB_CURSOR:
							gnapKissPlatypus(0);
							break;
						case TALK_CURSOR:
							playGnapBrainPulsating(_platX, _platY);
							playBeaverSequence(getBeaverSequenceId(0, 0, 0) | 0x10000);
							break;
						case PLAT_CURSOR:
							playGnapImpossible(_platX, _platY);
							break;
						}
					}
				}
				break;

			case kHSExitUfoParty:
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[kHSExitUfoParty].x, _hotspotsWalkPos[kHSExitUfoParty].y, 0, 0x107AE, 1);
				_gnapActionStatus = 0;
				_newSceneNum = 40;
				break;
			
			case kHSExitUfo:
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[kHSExitUfo].x, _gnapY, 0, 0x107AF, 1);
				_gnapActionStatus = 0;
				platypusWalkTo(_hotspotsWalkPos[kHSExitUfo].x, _platY, -1, 0x107CF, 1);
				_newSceneNum = 43;
				break;
			
			case kHSExitShow:
				_isLeavingScene = 1;
				gnapWalkTo(_hotspotsWalkPos[kHSExitShow].x, _hotspotsWalkPos[kHSExitShow].y, 0, 0x107AB, 1);
				_gnapActionStatus = 0;
				_newSceneNum = 46;
				break;
			
			case kHSKissingLady:
				if (_grabCursorSpriteIndex >= 0) {
					_gnapActionStatus = 2;
					gnapWalkTo(_hotspotsWalkPos[kHSKissingLady].x, _hotspotsWalkPos[kHSKissingLady].y, 0, -1, 9);
					playGnapShowItem(_grabCursorSpriteIndex, _hotspotsWalkPos[kHSKissingLady].x - 1, _hotspotsWalkPos[kHSKissingLady].y);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(4, 3);
						break;
					case GRAB_CURSOR:
						playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 5;
						gnapWalkTo(_hotspotsWalkPos[kHSKissingLady].x, _hotspotsWalkPos[kHSKissingLady].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = 1;
						break;
					case PLAT_CURSOR:
						gnapUseDeviceOnBeaver();
						platypusWalkTo(6, 7, 1, 0x107D2, 1);
						if (_gnapX == 7 && _gnapY == 7)
							gnapWalkStep();
						playGnapIdle(5, 7);
						_beaverActionStatus = 4;
						break;
					}
				}
				break;
			
			case kHSSpring:
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSSpring].x, _hotspotsWalkPos[kHSSpring].y, 8, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(8, 7);
						break;
					case GRAB_CURSOR:
						playGnapPullOutDevice(8, 0);
						playGnapUseDevice(8, 0);
						_nextSpringGuySequenceId = 0xFB;
						invAdd(kItemSpring);
						setFlag(15);
						scene44_updateHotspots();
						break;
					case TALK_CURSOR:
						playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
				break;
			
			case kHSSpringGuy:
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSSpringGuy].x, _hotspotsWalkPos[kHSSpringGuy].y, 8, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						if (isFlag(15))
							playGnapMoan1(8, 7);
						else
							playGnapScratchingHead(8, 7);
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 7;
						gnapWalkTo(_hotspotsWalkPos[kHSSpringGuy].x, _hotspotsWalkPos[kHSSpringGuy].y, -1, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
				break;
			
			case kHSWalkArea1:
			case kHSWalkArea2:
				if (_gnapActionStatus < 0)
					gnapWalkTo(-1, -1, -1, -1, 1);
				break;
			
			}

		}
	
		if (_mouseClickState._left && _gnapActionStatus < 0) {
			_mouseClickState._left = false;
			if (isFlag(12)) {
				_toyUfoActionStatus = 7;
				toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
			} else {
				gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}
	
		scene44_updateAnimations();
	
		toyUfoCheckTimer();
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0 && !isFlag(12) && _currKissingLadySequenceId != 0xF5)
				updateBeaverIdleSequence();
			if (_gnapActionStatus < 0 && !isFlag(12))
				updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(20) + 20;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0 && _nextKissingLadySequenceId == -1) {
					_gnapRandomValue = getRandom(20);
					switch (_gnapRandomValue) {
					case 0:
						_nextKissingLadySequenceId = 0xED;
						break;
					case 1:
						_nextKissingLadySequenceId = 0xEE;
						break;
					case 2:
						_nextKissingLadySequenceId = 0xF0;
						break;
					case 3:
						_nextKissingLadySequenceId = 0xF3;
						break;
					case 4:
						_nextKissingLadySequenceId = 0xF4;
						break;
					default:
						_nextKissingLadySequenceId = 0xEC;
						break;
					}
					if (_nextKissingLadySequenceId != 0xEC && _nextKissingLadySequenceId == _currKissingLadySequenceId)
						_nextKissingLadySequenceId = -1;
				}
			}
			if (!_timers[5]) {
				_timers[5] = getRandom(20) + 20;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0 && _nextSpringGuySequenceId == -1) {
					_gnapRandomValue = getRandom(5);
					if (_gnapRandomValue != 0) {
						if (!isFlag(15))
							_nextSpringGuySequenceId = 0xF9;
					} else {
						if (isFlag(15))
							_nextSpringGuySequenceId = 0xF8;
						else
							_nextSpringGuySequenceId = 0xFA;
					}
				}
			}
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene44_updateHotspots();
			_timers[4] = getRandom(20) + 20;
		}
		
		gameUpdateTick();	
	}
}

void GnapEngine::scene44_updateAnimations() {
	if (_gameSys->getAnimationStatus(0) == 2) 	{
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case 0:
			_sceneDone = true;
			break;
		case 1:
			_nextKissingLadySequenceId = 0xEF;
			break;
		case 2:
			_nextKissingLadySequenceId = 0xF2;
			break;
		}
		_gnapActionStatus = -1;
	}
	
	if (_gameSys->getAnimationStatus(1) == 2) {
		_gameSys->setAnimation(0, 0, 1);
		switch (_beaverActionStatus) {
		case 4:
			if (_gameSys->getAnimationStatus(2) == 2) {
				_gameSys->insertSequence(0xFE, _beaverId, _beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId, kSeqSyncWait, 0, 0, 0);
				_beaverSequenceId = 0xFE;
				_beaverSequenceDatNum = 0;
				_gameSys->setAnimation(0xFE, _beaverId, 1);
				_gameSys->removeSequence(_currKissingLadySequenceId, 1, true);
				_beaverActionStatus = 5;
			}
			break;
		case 5:
			_sceneDone = true;
			_newSceneNum = 50;
			break;
		default:
			_beaverActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(2) == 2) {
		if (_nextKissingLadySequenceId == 0xF6) {
			_gameSys->insertSequence(_nextKissingLadySequenceId, 1, _currKissingLadySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			initBeaverPos(5, 8, 0);
			_currKissingLadySequenceId = _nextKissingLadySequenceId;
			_nextKissingLadySequenceId = -1;
			_gameSys->setAnimation(0, 0, 2);
		} else if (_nextKissingLadySequenceId != -1) {
			_gameSys->insertSequence(_nextKissingLadySequenceId, 1, _currKissingLadySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_nextKissingLadySequenceId, 1, 2);
			_currKissingLadySequenceId = _nextKissingLadySequenceId;
			_nextKissingLadySequenceId = -1;
			_timers[4] = getRandom(20) + 20;
		}
	}
	
	if (_gameSys->getAnimationStatus(4) == 2) {
		if (_currSpringGuySequenceId == 0xFB) {
			setGrabCursorSprite(kItemSpring);
			_nextSpringGuySequenceId = 0xF8;
		}
		if (_nextSpringGuySequenceId != -1) {
			_gameSys->insertSequence(_nextSpringGuySequenceId, 1, _currSpringGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_nextSpringGuySequenceId, 1, 4);
			_currSpringGuySequenceId = _nextSpringGuySequenceId;
			_nextSpringGuySequenceId = -1;
			_timers[5] = getRandom(20) + 20;
		}
	}
	
	if (_gameSys->getAnimationStatus(3) == 2) {
		switch (_toyUfoActionStatus) {
		case 6:
			_sceneDone = true;
			break;
		default:
			_toyUfoNextSequenceId = toyUfoGetSequenceId();
			_gameSys->insertSequence(_toyUfoNextSequenceId | 0x10000, _toyUfoId + 1,
				_toyUfoSequenceId | 0x10000, _toyUfoId,
				kSeqSyncWait, 0, _toyUfoX - 274, _toyUfoY - 128);
			_toyUfoSequenceId = _toyUfoNextSequenceId;
			++_toyUfoId;
			_gameSys->setAnimation(_toyUfoNextSequenceId | 0x10000, _toyUfoId, 3);
			break;
		}
		_toyUfoActionStatus = -1;
	}
}

} // End of namespace Gnap
