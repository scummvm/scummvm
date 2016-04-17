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
	kHSPlatypus			= 0,
	kHSExitUfoParty		= 1,
	kHSExitShoe			= 2,
	kHSExitRight		= 3,
	kHSExitDiscoBall	= 4,
	kHSDiscoBall		= 5,
	kHSDevice			= 6,
	kHSWalkArea1		= 7
};

enum {
	kHSUfoExitLeft	= 1,
	kHSUfoExitRight	= 2,
	kHSUfoDevice	= 3
};

int GnapEngine::scene45_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	_gameSys->setAnimation(0, 0, 2);
	_gameSys->setAnimation(0, 0, 3);
	_gameSys->setAnimation(0, 0, 4);
	_gameSys->setAnimation(0, 0, 5);
	return isFlag(kGFUnk23) ? 0xA2 : 0xA1;
}

void GnapEngine::scene45_updateHotspots() {
	if (isFlag(kGFGnapControlsToyUFO)) {
		setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
		setHotspot(kHSUfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		setHotspot(kHSUfoExitRight, 794, 0, 799, 599, SF_EXIT_R_CURSOR | SF_DISABLED);
		setDeviceHotspot(kHSUfoDevice, -1, 534, -1, 599);
		_hotspotsCount = 4;
	} else {
		setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		setHotspot(kHSExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		setHotspot(kHSExitShoe, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		setHotspot(kHSExitRight, 794, 100, 799, 599, SF_EXIT_R_CURSOR | SF_DISABLED, 10, 8);
		setHotspot(kHSExitDiscoBall, 200, 0, 600, 10, SF_DISABLED);
		setHotspot(kHSDiscoBall, 370, 10, 470, 125, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
		setHotspot(kHSWalkArea1, 0, 0, 800, 472);
		setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
		if (isFlag(kGFUnk22)) {
			_hotspots[kHSPlatypus]._flags = SF_DISABLED;
			_hotspots[kHSExitUfoParty]._flags = SF_DISABLED;
			_hotspots[kHSExitShoe]._flags = SF_DISABLED;
			_hotspots[kHSExitRight]._flags = SF_DISABLED;
			_hotspots[kHSExitDiscoBall]._flags = SF_EXIT_U_CURSOR;
		}
		if (isFlag(kGFUnk23) || isFlag(kGFUnk22))
			_hotspots[kHSDiscoBall]._flags = SF_DISABLED;
		_hotspotsCount = 8;
	}
}

void GnapEngine::scene45_run() {
	if (!isSoundPlaying(0x1094A))
		playSound(0x1094A, true);
	
	queueInsertDeviceIcon();

	_gameSys->insertSequence(0x96, 1, 0, 0, kSeqNone, 0, 0, 0);
	_gameSys->setAnimation(0x96, 1, 3);
	_gameSys->insertSequence(0x99, 1, 0, 0, kSeqNone, 0, 0, 0);
	_gameSys->setAnimation(0x99, 1, 4);
	_s45_currDancerSequenceId = 0x8F;
	_gameSys->setAnimation(_s45_currDancerSequenceId, 1, 2);
	_gameSys->insertSequence(_s45_currDancerSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (isFlag(kGFGnapControlsToyUFO)) {
		_toyUfoId = 0;
		_toyUfoActionStatus = -1;
		_toyUfoSequenceId = toyUfoGetSequenceId();
		_toyUfoNextSequenceId = _toyUfoSequenceId;
		if (_prevSceneNum == 46)
			_toyUfoX = 30;
		else
			_toyUfoX = 770;
		_gameSys->setAnimation(_toyUfoSequenceId | 0x10000, _toyUfoId, 5);
		_gameSys->insertSequence(_toyUfoSequenceId | 0x10000, _toyUfoId, 0, 0, kSeqNone, 0, _toyUfoX - 274, _toyUfoY - 128);
		endSceneInit();
	} else if (isFlag(kGFUnk22)) {
		_gnapSequenceId = 0x9E;
		_gnapSequenceDatNum = 0;
		_gnapId = 1;
		_gameSys->setAnimation(0x9E, 1, 0);
		_gnapActionStatus = 1;
		_gameSys->insertSequence(_gnapSequenceId, _gnapId, 0, 0, kSeqNone, 0, 0, 0);
		initBeaverPos(4, 8, kDirNone);
		endSceneInit();
	} else if (_prevSceneNum == 46) {
		initGnapPos(-1, 8, kDirUpRight);
		initBeaverPos(-1, 9, kDirUpLeft);
		endSceneInit();
		platypusWalkTo(4, 8, -1, 0x107C2, 1);
		gnapWalkTo(2, 7, -1, 0x107B9, 1);
	} else if (_prevSceneNum == 41) {
		initGnapPos(11, 8, kDirUpRight);
		initBeaverPos(11, 9, kDirUpLeft);
		endSceneInit();
		platypusWalkTo(4, 8, -1, 0x107D2, 1);
		gnapWalkTo(10, 9, -1, 0x107BA, 1);
	} else {
		initGnapPos(2, 11, kDirUpRight);
		initBeaverPos(6, 11, kDirUpLeft);
		endSceneInit();
		platypusWalkTo(4, 8, -1, 0x107C2, 1);
		gnapWalkTo(2, 7, -1, 0x107B9, 1);
	}
	
	if (!isFlag(kGFUnk21) && !isFlag(kGFGnapControlsToyUFO)) {
		setFlag(kGFUnk21);
		setGrabCursorSprite(-1);
		_gameSys->setAnimation(0x9D, _gnapId, 0);
		_gameSys->insertSequence(0x9D, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
		while (_gameSys->getAnimationStatus(0) != 2) {
			gameUpdateTick();
			if (_gameSys->getAnimationStatus(2) == 2) {
				_gameSys->setAnimation(0, 0, 2);
				int newSeqId = getRandom(7) + 0x8F;
				_gameSys->insertSequence(newSeqId, 1, _s45_currDancerSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				_gameSys->setAnimation(newSeqId, 1, 2);
				_s45_currDancerSequenceId = newSeqId;
			}
			if (_gameSys->getAnimationStatus(3) == 2 && _gameSys->getAnimationStatus(4) == 2) {
				_gameSys->insertSequence(0x96, 1, 0x96, 1, kSeqSyncWait, 0, 0, 0);
				_gameSys->setAnimation(0x96, 1, 3);
				_gameSys->insertSequence(0x99, 1, 0x99, 1, kSeqSyncWait, 0, 0, 0);
				_gameSys->setAnimation(0x99, 1, 4);
			}
		}
		_gnapSequenceId = 0x9D;
		_gnapSequenceDatNum = 0;
		hideCursor();
		addFullScreenSprite(0x8A, 255);
		_gameSys->setAnimation(0xA0, 256, 0);
		_gameSys->insertSequence(0xA0, 256, 0, 0, kSeqNone, 0, 0, 0);
		while (_gameSys->getAnimationStatus(0) != 2)
			gameUpdateTick();
		_gameSys->setAnimation(0x107BD, _gnapId, 0);
		_gameSys->insertSequence(0x107BD, _gnapId,
			makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
			kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
		removeFullScreenSprite();
		showCursor();
		_gnapSequenceId = 0x7BD;
		_gnapSequenceDatNum = 1;
	}
	
	playBeaverSequence(0x9A);
	_gameSys->setAnimation(_beaverSequenceId, _beaverId, 1);
	
	while (!_sceneDone) {
		if (!isSoundPlaying(0x1094A))
			playSound(0x1094A, true);
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		testWalk(0, 0, -1, -1, -1, -1);
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		if (isFlag(kGFGnapControlsToyUFO)) {

			switch (_sceneClickedHotspot) {

			case kHSUfoExitLeft:
				if (_toyUfoActionStatus < 0) {
					_isLeavingScene = 1;
					_toyUfoActionStatus = 2;
					_newSceneNum = 46;
					toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 5);
				}
				break;

			case kHSUfoExitRight:
				if (_toyUfoActionStatus < 0) {
					_isLeavingScene = 1;
					_toyUfoActionStatus = 2;
					_newSceneNum = 41;
					toyUfoFlyTo(835, -1, 0, 835, 0, 300, 5);
				}
				break;

			case kHSUfoDevice:
				runMenu();
				scene45_updateHotspots();
				break;

			}

		} else {

			switch (_sceneClickedHotspot) {

			case kHSDevice:
				runMenu();
				scene45_updateHotspots();
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
							playBeaverSequence(0x9A);
							_gameSys->setAnimation(_beaverSequenceId, _beaverId, 1);
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
				if (_gnapActionStatus < 0) {
					_isLeavingScene = 1;
					gnapWalkTo(_gnapX, _hotspotsWalkPos[kHSExitUfoParty].y, 0, 0x107AE, 1);
					_gnapActionStatus = 0;
					_newSceneNum = 40;
				}
				break;
			
			case kHSExitShoe:
				if (_gnapActionStatus < 0) {
					_isLeavingScene = 1;
					gnapWalkTo(_hotspotsWalkPos[kHSExitShoe].x, _gnapY, 0, 0x107AF, 1);
					_gnapActionStatus = 0;
					platypusWalkTo(_hotspotsWalkPos[kHSExitShoe].x, _platY, -1, 0x107CF, 1);
					_newSceneNum = 46;
				}
				break;
			
			case kHSExitRight:
				if (_gnapActionStatus < 0) {
					_isLeavingScene = 1;
					gnapWalkTo(_hotspotsWalkPos[kHSExitRight].x, _gnapY, 0, 0x107AB, 1);
					_gnapActionStatus = 0;
					platypusWalkTo(_hotspotsWalkPos[kHSExitRight].x, _platY, -1, 0x107CD, 1);
					_newSceneNum = 41;
				}
				break;

			case kHSExitDiscoBall:
				clearFlag(kGFUnk22);
				setFlag(kGFUnk23);
				_sceneDone = true;
				_newSceneNum = 54;
				break;

			case kHSDiscoBall:
				if (_gnapActionStatus < 0) {
					if (_grabCursorSpriteIndex == kItemSpring) {
						gnapWalkTo(_hotspotsWalkPos[kHSDiscoBall].x, _hotspotsWalkPos[kHSDiscoBall].y, 0, 0x9F, 5);
						_gnapActionStatus = 1;
						setGrabCursorSprite(-1);
						invRemove(kItemSpring);
					} else if (_grabCursorSpriteIndex >= 0) {
						playGnapShowItem(_grabCursorSpriteIndex, 5, 0);
					} else {
						switch (_verbCursor) {
						case LOOK_CURSOR:
							playGnapScratchingHead(5, 0);
							break;
						case GRAB_CURSOR:
						case TALK_CURSOR:
						case PLAT_CURSOR:
							playGnapImpossible(0, 0);
							break;
						}
					}
				}
				break;
			
			case kHSWalkArea1:
				if (_gnapActionStatus < 0)
					gnapWalkTo(-1, -1, -1, -1, 1);
				break;
			
			}
		
		}

		if (_mouseClickState._left && _gnapActionStatus < 0) {
			_mouseClickState._left = false;
			if (isFlag(kGFGnapControlsToyUFO)) {
				_toyUfoActionStatus = 3;
				toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 5);
			} else {
				gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}
	
		scene45_updateAnimations();
		toyUfoCheckTimer();
	
		if (!_isLeavingScene && _gnapActionStatus < 0 && !isFlag(kGFGnapControlsToyUFO))
			updateGnapIdleSequence();
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene45_updateHotspots();
		}
	
		gameUpdateTick();

	}

	_sceneWaiting = 0;

}

void GnapEngine::scene45_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case 0:
			_sceneDone = true;
			break;
		case 1:
			_sceneWaiting = 1;
			setFlag(kGFUnk22);
			scene45_updateHotspots();
			_gameSys->insertSequence(0x9E, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x9E;
			_gnapSequenceDatNum = 0;
			_gameSys->setAnimation(0x9E, _gnapId, 0);
			break;
		default:
			_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(1) == 2) {
		_gameSys->setAnimation(0, 0, 1);
		if (getRandom(2) != 0)
			playBeaverSequence(0x9B);
		else
			playBeaverSequence(0x9C);
		_gameSys->setAnimation(_beaverSequenceId, _beaverId, 1);
	}
	
	if (_gameSys->getAnimationStatus(2) == 2) {
		_gameSys->setAnimation(0, 0, 2);
		int newSeqId = getRandom(7) + 0x8F;
		_gameSys->insertSequence(newSeqId, 1, _s45_currDancerSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		_gameSys->setAnimation(newSeqId, 1, 2);
		_s45_currDancerSequenceId = newSeqId;
	}
	
	if (_gameSys->getAnimationStatus(3) == 2 && _gameSys->getAnimationStatus(4) == 2) {
		_gameSys->insertSequence(0x96, 1, 0x96, 1, kSeqSyncWait, 0, 0, 0);
		_gameSys->setAnimation(0x96, 1, 3);
		_gameSys->insertSequence(0x99, 1, 0x99, 1, kSeqSyncWait, 0, 0, 0);
		_gameSys->setAnimation(0x99, 1, 4);
	}
	
	if (_gameSys->getAnimationStatus(5) == 2) {
		switch (_toyUfoActionStatus) {
		case 2:
			_sceneDone = true;
			break;
		default:
			_toyUfoNextSequenceId = toyUfoGetSequenceId();
			_gameSys->insertSequence(_toyUfoNextSequenceId | 0x10000, _toyUfoId + 1,
				_toyUfoSequenceId | 0x10000, _toyUfoId,
				kSeqSyncWait, 0, _toyUfoX - 274, _toyUfoY - 128);
			_toyUfoSequenceId = _toyUfoNextSequenceId;
			++_toyUfoId;
			_gameSys->setAnimation(_toyUfoNextSequenceId | 0x10000, _toyUfoId, 5);
			break;
		}
		_toyUfoActionStatus = -1;
	}

}

} // End of namespace Gnap
