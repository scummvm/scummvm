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
	kHSExitToyStand	= 2,
	kHSExitUfo		= 3,
	kHSBBQVendor	= 4,
	kHSChickenLeg	= 5,
	kHSDevice		= 6,
	kHSWalkArea1	= 7,
	kHSWalkArea2	= 8
};

enum {
	kHSUfoExitLeft	= 1,
	kHSUfoExitRight	= 2,
	kHSUfoHotSauce	= 3,
	kHSUfoDevice	= 4
};

enum {
	kASLeaveScene					= 0,
	kASTalkBBQVendor				= 1,
	kASUseQuarterWithBBQVendor		= 2,
	kASUseQuarterWithBBQVendorDone	= 3,
	kASGrabChickenLeg				= 4,
	kASToyUfoLeaveScene				= 5,
	kASToyUfoRefresh				= 6,
	kASToyUfoPickUpHotSauce			= 7
};

int GnapEngine::scene42_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	_gameSys->setAnimation(0, 0, 2);
	if (isFlag(20) || (isFlag(18) && isFlag(23)))
		return 0x153;
	return 0x152;
}

void GnapEngine::scene42_updateHotspots() {
	if (isFlag(12)) {
		setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
		setHotspot(kHSUfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		setHotspot(kHSUfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		setHotspot(kHSUfoHotSauce, 335, 110, 440, 175, SF_DISABLED);
		setDeviceHotspot(kHSUfoDevice, -1, 534, -1, 599);
		if ((isFlag(20) || isFlag(18)) && isFlag(23) && !isFlag(24))
			_hotspots[kHSUfoHotSauce].flags = SF_GRAB_CURSOR;
		_hotspotsCount = 5;
	} else {
		setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		setHotspot(kHSExitUfoParty, 150, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		setHotspot(kHSExitToyStand, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		setHotspot(kHSExitUfo, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		setHotspot(kHSBBQVendor, 410, 200, 520, 365, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 8);
		setHotspot(kHSChickenLeg, 530, 340, 620, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 8, 7);
		setHotspot(kHSWalkArea1, 0, 0, 800, 445);
		setHotspot(kHSWalkArea2, 240, 0, 550, 495);
		setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
		_hotspotsCount = 9;
	}
}

void GnapEngine::scene42_run() {

	queueInsertDeviceIcon();

	_s42_currBBQVendorSequenceId = 0x14A;
	_s42_nextBBQVendorSequenceId = -1;

	_gameSys->setAnimation(0x14A, 1, 2);
	_gameSys->insertSequence(_s42_currBBQVendorSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (isFlag(12)) {
		_toyUfoId = 0;
		_toyUfoActionStatus = -1;
		if (_prevSceneNum == 43 && isFlag(18)) {
			_toyUfoSequenceId = 0x872;
			_toyUfoNextSequenceId = toyUfoGetSequenceId();
			_gameSys->insertSequence(_toyUfoSequenceId | 0x10000, _toyUfoId, 0, 0, kSeqNone, 0, 0, 0);
			_toyUfoX = 317;
			_toyUfoY = 61;
			toyUfoSetStatus(17);
			setFlag(20);
			_timers[9] = 600;
		} else {
			_toyUfoSequenceId = toyUfoGetSequenceId();
			_toyUfoNextSequenceId = _toyUfoSequenceId;
			if (_prevSceneNum == 41)
				_toyUfoX = 30;
			else
				_toyUfoX = 770;
			_gameSys->insertSequence(_toyUfoSequenceId | 0x10000, _toyUfoId, 0, 0, kSeqNone, 0, _toyUfoX - 274, _toyUfoY - 128);
		}
		_gameSys->setAnimation(_toyUfoSequenceId | 0x10000, _toyUfoId, 3);
		endSceneInit();
		if (_toyUfoSequenceId == 0x872)
			setGrabCursorSprite(-1);
	} else if (_prevSceneNum == 41) {
		initGnapPos(-1, 8, 7);
		initBeaverPos(-1, 9, 5);
		endSceneInit();
		gnapWalkTo(2, 8, -1, 0x107B9, 1);
		platypusWalkTo(1, 8, -1, 0x107C2, 1);
	} else if (_prevSceneNum == 43) {
		initGnapPos(11, 8, 7);
		initBeaverPos(11, 9, 5);
		endSceneInit();
		gnapWalkTo(8, 8, -1, 0x107BA, 1);
		platypusWalkTo(9, 8, -1, 0x107D2, 1);
	} else {
		initGnapPos(5, 11, 7);
		initBeaverPos(6, 11, 5);
		endSceneInit();
		gnapWalkTo(5, 8, -1, 0x107BA, 1);
		platypusWalkTo(6, 8, -1, 0x107C2, 1);
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
					_isLeavingScene = true;
					_toyUfoActionStatus = kASToyUfoLeaveScene;
					_newSceneNum = 41;
					toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 3);
				}
				break;

			case kHSUfoExitRight:
				if (_toyUfoActionStatus < 0) {
					_isLeavingScene = true;
					_toyUfoActionStatus = kASToyUfoLeaveScene;
					_newSceneNum = 43;
					toyUfoFlyTo(835, -1, 0, 835, 0, 300, 3);
				}
				break;

			case kHSUfoHotSauce:
				if (isFlag(17)) {
					_toyUfoActionStatus = kASToyUfoPickUpHotSauce;
					toyUfoFlyTo(384, 77, 0, 799, 0, 300, 3);
					_timers[9] = 600;
				} else {
					_toyUfoActionStatus = kASToyUfoRefresh;
					toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
				}
				break;

			case kHSUfoDevice:
				runMenu();
				scene42_updateHotspots();
				break;

			}

		} else {

			switch (_sceneClickedHotspot) {

			case kHSDevice:
				runMenu();
				scene42_updateHotspots();
				_timers[4] = getRandom(20) + 30;
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
				_isLeavingScene = true;
				gnapWalkTo(_gnapX, _hotspotsWalkPos[kHSExitUfoParty].y, 0, 0x107AE, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_platX, _hotspotsWalkPos[kHSExitUfoParty].y, -1, 0x107C7, 1);
				_newSceneNum = 40;
				break;

			case kHSExitToyStand:
				_isLeavingScene = true;
				gnapWalkTo(_hotspotsWalkPos[kHSExitToyStand].x, _gnapY, 0, 0x107AF, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitToyStand].x, _hotspotsWalkPos[kHSExitToyStand].y, -1, 0x107CF, 1);
				_newSceneNum = 41;
				break;

			case kHSExitUfo:
				_isLeavingScene = true;
				gnapWalkTo(_hotspotsWalkPos[kHSExitUfo].x, _gnapY, 0, 0x107AB, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitUfo].x, _hotspotsWalkPos[kHSExitUfo].y, -1, 0x107CD, 1);
				_newSceneNum = 43;
				break;

			case kHSBBQVendor:
				if (_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					gnapWalkTo(_hotspotsWalkPos[kHSBBQVendor].x, _hotspotsWalkPos[kHSBBQVendor].y, 0, 0x107BB, 1);
					_gnapActionStatus = kASUseQuarterWithBBQVendor;
					if (_platY < 9)
						platypusWalkTo(_platX, 9, -1, -1, 1);
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSBBQVendor].x, _hotspotsWalkPos[kHSBBQVendor].y, _hotspotsWalkPos[kHSBBQVendor].x + 1, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(_hotspotsWalkPos[kHSBBQVendor].x - 1, 0);
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 7;
						gnapWalkTo(_hotspotsWalkPos[kHSBBQVendor].x, _hotspotsWalkPos[kHSBBQVendor].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTalkBBQVendor;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
				break;
			
			case kHSChickenLeg:
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSChickenLeg].x, _hotspotsWalkPos[kHSChickenLeg].y, _hotspotsWalkPos[kHSChickenLeg].x - 1, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(_hotspotsWalkPos[kHSChickenLeg].x - 1, 0);
						break;
					case GRAB_CURSOR:
						gnapWalkTo(_hotspotsWalkPos[kHSChickenLeg].x, _hotspotsWalkPos[kHSChickenLeg].y, 0, 0x107BC, 1);
						_gnapActionStatus = kASGrabChickenLeg;
						break;
					case TALK_CURSOR:
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
	
		if (_mouseClickState.left && _gnapActionStatus < 0) {
			_mouseClickState.left = false;
			if (isFlag(12)) {
				_toyUfoActionStatus = kASToyUfoRefresh;
				toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
			} else {
				gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}
	
		scene42_updateAnimations();
	
		toyUfoCheckTimer();
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0 && !isFlag(12))
				updateBeaverIdleSequence();
			if (_gnapActionStatus < 0 && !isFlag(12))
				updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(20) + 30;
				if (_gnapActionStatus < 0 && _beaverActionStatus < 0 && _s42_nextBBQVendorSequenceId == -1) {
					_gnapRandomValue = getRandom(8);
					switch (_gnapRandomValue) {
					case 0:
						_s42_nextBBQVendorSequenceId = 0x14C;
						break;
					case 1:
					case 2:
						_s42_nextBBQVendorSequenceId = 0x149;
						break;
					case 3:
					case 4:
					case 5:
					case 6:
						_s42_nextBBQVendorSequenceId = 0x14D;
						break;
					case 7:
						_s42_nextBBQVendorSequenceId = 0x14A;
						break;
					}
					if (_s42_nextBBQVendorSequenceId == _s42_currBBQVendorSequenceId && _s42_nextBBQVendorSequenceId != 0x14D)
						_s42_nextBBQVendorSequenceId = -1;
				}
			}
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene42_updateHotspots();
			_timers[4] = getRandom(20) + 30;
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene42_updateAnimations() {
	int sequenceId;

	if (_gameSys->getAnimationStatus(0) == 2) {
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_gameSys->setAnimation(0, 0, 0);
			_gnapActionStatus = -1;
			_sceneDone = true;
			break;
		case kASTalkBBQVendor:
			_gameSys->setAnimation(0, 0, 0);
			_gnapActionStatus = -1;
			_s42_nextBBQVendorSequenceId = 0x14B;
			break;
		case kASUseQuarterWithBBQVendor:
		case kASGrabChickenLeg:
			if (_gameSys->getAnimationStatus(2) == 2) {
				if (_gnapActionStatus == kASUseQuarterWithBBQVendor) {
					invRemove(kItemDiceQuarterHole);
					invAdd(kItemChickenBucket);
					setGrabCursorSprite(-1);
					sequenceId = 0x150;
					_s42_nextBBQVendorSequenceId = 0x148;
				} else if (isFlag(27)) {
					if (isFlag(28)) {
						sequenceId = 0x7B7;
						_s42_nextBBQVendorSequenceId = 0x145;
					} else {
						setFlag(28);
						sequenceId = 0x14F;
						_s42_nextBBQVendorSequenceId = 0x147;
					}
				} else {
					setFlag(27);
					sequenceId = 0x14E;
					_s42_nextBBQVendorSequenceId = 0x146;
				}
				if (sequenceId == 0x7B7) {
					_gameSys->insertSequence(0x107B7, _gnapId,
						makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
						kSeqSyncWait, getSequenceTotalDuration(_s42_nextBBQVendorSequenceId),
						75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
					_gnapSequenceDatNum = 1;
				} else {
					_gameSys->insertSequence(sequenceId, _gnapId,
						makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
						kSeqSyncWait, 0, 0, 0);
					_gnapSequenceDatNum = 0;
				}
				_gnapSequenceId = sequenceId;
				_gameSys->setAnimation(sequenceId | (_gnapSequenceDatNum << 16), _gnapId, 0);
				if (_gnapActionStatus == kASUseQuarterWithBBQVendor)
					_gnapActionStatus = kASUseQuarterWithBBQVendorDone;
				else
					_gnapActionStatus = -1;
				_gameSys->insertSequence(_s42_nextBBQVendorSequenceId, 1, _s42_currBBQVendorSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				_gameSys->setAnimation(_s42_nextBBQVendorSequenceId, 1, 2);
				_s42_currBBQVendorSequenceId = _s42_nextBBQVendorSequenceId;
				if (_s42_nextBBQVendorSequenceId == 0x145)
					_s42_nextBBQVendorSequenceId = 0x14A;
				else
					_s42_nextBBQVendorSequenceId = -1;
				_timers[4] = getRandom(20) + 30;
				_timers[2] = getRandom(30) + 20;
				_timers[3] = getRandom(50) + 200;
			}
			break;
		case kASUseQuarterWithBBQVendorDone:
			_gameSys->setAnimation(0, 0, 0);
			setGrabCursorSprite(kItemChickenBucket);
			_gnapActionStatus = -1;
			break;
		default:
			_gameSys->setAnimation(0, 0, 0);
			_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(2) == 2 && _s42_nextBBQVendorSequenceId != -1) {
		_gameSys->insertSequence(_s42_nextBBQVendorSequenceId, 1, _s42_currBBQVendorSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		_gameSys->setAnimation(_s42_nextBBQVendorSequenceId, 1, 2);
		_s42_currBBQVendorSequenceId = _s42_nextBBQVendorSequenceId;
		_s42_nextBBQVendorSequenceId = -1;
		_timers[4] = getRandom(20) + 30;
	}
	
	if (_gameSys->getAnimationStatus(3) == 2) {
		switch (_toyUfoActionStatus) {
		case kASToyUfoLeaveScene:
			_sceneDone = true;
			break;
		case kASToyUfoPickUpHotSauce:
			_gameSys->insertSequence(0x10870, _toyUfoId, _toyUfoSequenceId | 0x10000, _toyUfoId, kSeqSyncWait, 0, 0, 0);
			setFlag(24);
			scene42_updateHotspots();
			toyUfoSetStatus(19);
			_toyUfoSequenceId = 0x870;
			_gameSys->setAnimation(0x10870, _toyUfoId, 3);
			_toyUfoActionStatus = -1;
			_toyUfoX = 0x181;
			_toyUfoY = 53;
			break;
		default:
			if (_toyUfoSequenceId == 0x872) {
				hideCursor();
				addFullScreenSprite(0x13E, 255);
				_gameSys->setAnimation(0x151, 256, 0);
				_gameSys->insertSequence(0x151, 256, 0, 0, kSeqNone, 0, 0, 0);
				while (_gameSys->getAnimationStatus(0) != 2)
					gameUpdateTick();
				removeFullScreenSprite();
				showCursor();
			}
			_toyUfoNextSequenceId = toyUfoGetSequenceId();
			_gameSys->setAnimation(_toyUfoNextSequenceId | 0x10000, (_toyUfoId + 1) % 10, 3);
			_gameSys->insertSequence(_toyUfoNextSequenceId | 0x10000, (_toyUfoId + 1) % 10,
				_toyUfoSequenceId | 0x10000, _toyUfoId,
				kSeqSyncWait, 0, _toyUfoX - 274, _toyUfoY - 128);
			_toyUfoSequenceId = _toyUfoNextSequenceId;
			_toyUfoId = (_toyUfoId + 1) % 10;
			break;
		}
		_toyUfoActionStatus = -1;
	}

}

} // End of namespace Gnap
