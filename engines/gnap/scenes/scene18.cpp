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

static const int kScene18SequenceIds[] = {
	0x219,  0x21A,  0x21B,  0x21C,  0x21D
};

enum {
	kHSPlatypus				= 0,
	kHSGarbageCan			= 1,
	kHSDevice				= 2,
	kHSExitToyStore			= 3,
	kHSExitPhoneBooth		= 4,
	kHSExitGrubCity			= 5,
	kHSHydrantTopValve		= 6,
	kHSHydrantRightValve	= 7,
	kHSCowboyHat			= 8,
	kHSWalkArea1			= 9,
	kHSWalkArea2			= 10
};

enum {
	kASOpenRightValveNoGarbageCanDone		= 0,
	kASOpenRightValveNoGarbageCan			= 1,
	kASCloseRightValveNoGarbageCan			= 2,
	kASOpenTopValveDone						= 3,
	kASOpenTopValve							= 4,
	kASCloseTopValve						= 5,
	kASGrabGarbageCanFromStreet				= 6,
	kASGrabCowboyHat						= 7,
	kASGrabGarbageCanFromHydrant			= 8,
	kASPutGarbageCanOnRunningHydrant		= 9,
	kASPutGarbageCanOnRunningHydrant2		= 10,
	kASGrabCowboyHatDone					= 11,
	kASStandingOnHydrant					= 12,
	kASOpenRightValveWithGarbageCan			= 13,
	kASOpenRightValveWithGarbageCanDone		= 14,
	kASCloseRightValveWithGarbageCan		= 15,
	kASPutGarbageCanOnHydrant				= 16,
	kASPutGarbageCanOnHydrantDone			= 17,
	kASPlatComesHere						= 18,
	kASCloseRightValveWithGarbageCanDone	= 19,
	kASLeaveScene							= 20
};

int GnapEngine::scene18_init() {
	_gameSys->setAnimation(0, 0, 3);
	return 0x222;
}

void GnapEngine::scene18_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSGarbageCan, _gridMinX + 75 * _s18_garbageCanPos - 35, _gridMinY + 230, _gridMinX + 75 * _s18_garbageCanPos + 35, _gridMinY + 318,
		SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, _s18_garbageCanPos, 7);
	setHotspot(kHSExitToyStore, 460, 238, 592, 442, SF_EXIT_U_CURSOR, 7, 7);
	setHotspot(kHSExitPhoneBooth, 275, 585, 525, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 10);
	setHotspot(kHSExitGrubCity, 0, 350, 15, 600, SF_EXIT_L_CURSOR, 0, 9);
	setHotspot(kHSHydrantTopValve, 100, 345, 182, 410, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 8);
	setHotspot(kHSHydrantRightValve, 168, 423, 224, 470, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	setHotspot(kHSCowboyHat, 184, 63, 289, 171, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	setHotspot(kHSWalkArea1, 0, 0, 800, 448);
	setHotspot(kHSWalkArea2, 0, 0, 214, 515);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (isFlag(kGFTruckFilledWithGas)) {
		if (isFlag(kGFTruckKeysUsed)) {
			_hotspots[kHSHydrantTopValve]._flags = SF_DISABLED;
			_hotspots[kHSHydrantRightValve]._x1 = 148;
			_hotspots[kHSHydrantRightValve]._y1 = 403;
			_hotspots[kHSGarbageCan]._flags = SF_DISABLED;
			_hotspotsWalkPos[kHSGarbageCan].x = 3;
			_hotspotsWalkPos[kHSGarbageCan].y = 7;
		} else {
			_hotspots[kHSHydrantTopValve]._y1 = 246;
		}
	} else if (isFlag(kGFBarnPadlockOpen)) {
		_hotspots[kHSHydrantRightValve]._flags = SF_DISABLED;
		_hotspots[kHSHydrantTopValve]._x1 = 105;
		_hotspots[kHSHydrantTopValve]._x2 = 192;
	} else if (isFlag(kGFTruckKeysUsed)) {
		_hotspots[kHSGarbageCan]._x1 = 115;
		_hotspots[kHSGarbageCan]._y1 = 365;
		_hotspots[kHSGarbageCan]._x2 = 168;
		_hotspots[kHSGarbageCan]._y2 = 470;
		_hotspots[kHSGarbageCan]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
		_hotspotsWalkPos[kHSGarbageCan].x = 3;
		_hotspotsWalkPos[kHSGarbageCan].y = 7;
	}
	if (isFlag(kGFPlatyPussDisguised))
		_hotspots[kHSGarbageCan]._flags = SF_DISABLED;
	if (isFlag(kGFPlatypusTalkingToAssistant)) {
		_hotspots[kHSDevice]._flags = SF_DISABLED;
		_hotspots[kHSHydrantTopValve]._flags = SF_DISABLED;
		_hotspots[kHSHydrantRightValve]._flags = SF_DISABLED;
		_hotspots[kHSPlatypus]._flags = SF_DISABLED;
	}
	if (isFlag(kGFUnk14)) {
		_hotspots[kHSHydrantTopValve]._flags = SF_DISABLED;
		_hotspots[kHSCowboyHat]._flags = SF_DISABLED;
	}
	_hotspotsCount = 11;
}

void GnapEngine::scene18_gnapCarryGarbageCanTo(int x, int y, int animationIndex, int argC, int a5) {
	// CHECKME: (x, y) is always set to (-1, -1)

	// TODO Cleanup	
	static const int kSequenceIds[] = {
		0x203, 0x204
	};
	
	int gnapSeqId, gnapId, gnapDatNum, gnapGridX;
	int clippedX, v12, v5, v10, v11, direction;

	if (x >= 0)
		clippedX = x;
	else
		clippedX = (_leftClickMouseX - _gridMinX + 37) / 75;
	if (clippedX >= _gnapX)
		v10 = clippedX - 1;
	else
		v10 = clippedX + 1;

	if (a5 < 0)
		a5 = 4;
	v5 = v10;
	if (v10 <= a5)
		v5 = a5;
	v11 = v5;
	v12 = _gridMaxX - 1;
	if (_gridMaxX - 1 >= v11)
		v12 = v11;
	
	if (v12 == _gnapX) {
		gnapSeqId = _gnapSequenceId;
		gnapId = _gnapId;
		gnapDatNum = _gnapSequenceDatNum;
		gnapGridX = _gnapX;
		if (_gnapX <= clippedX)
			direction = 1;
		else
			direction = -1;
	} else {
		if (_gnapY == _platY) {
			if (v12 >= _gnapX) {
				if (v12 >= _platX && _gnapX <= _platX)
					beaverMakeRoom();
			} else if (v12 <= _platX && _gnapX >= _platX) {
				beaverMakeRoom();
			}
		}
		gnapSeqId = _gnapSequenceId;
		gnapId = _gnapId;
		gnapDatNum = _gnapSequenceDatNum;
		gnapGridX = _gnapX;
		int seqId = 0;
		if (v12 < _gnapX) {
			direction = -1;
			seqId = 1;
		} else
			direction = 1;
		int a2 = 20 * _gnapY + 1;
		do {
			if (isPointBlocked(gnapGridX + direction, _gnapY))
				break;
			a2 += direction;
			_gameSys->insertSequence(kSequenceIds[seqId], a2,
				gnapSeqId | (gnapDatNum << 16), gnapId,
				kSeqSyncWait, 0, 75 * gnapGridX - _gnapGridX, 48 * _gnapY - _gnapGridY);
			gnapSeqId = kSequenceIds[seqId];
			gnapId = a2;
			gnapDatNum = 0;
			gnapGridX += direction;
		} while (v12 != gnapGridX);
	}
	
	if (argC >= 0) {
		_gnapSequenceId = ridToEntryIndex(argC);
		_gnapSequenceDatNum = ridToDatIndex(argC);
	} else {
		if (direction == 1)
			_gnapSequenceId = 0x20A;
		else
			_gnapSequenceId = 0x209;
		_gnapSequenceDatNum = 0;
	}
	
	if (direction == 1)
		_gnapIdleFacing = kDirBottomRight;
	else
		_gnapIdleFacing = kDirBottomLeft;
	
	_gnapId = 20 * _gnapY + 1;
	
	if (animationIndex >= 0)
		_gameSys->setAnimation(makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, animationIndex);
	
	_gameSys->insertSequence(makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
		gnapSeqId | (gnapDatNum << 16), gnapId,
		kSeqScale | kSeqSyncWait, 0, 75 * gnapGridX - _gnapGridX, 48 * _gnapY - _gnapGridY);

	_gnapX = gnapGridX;

}

void GnapEngine::scene18_putDownGarbageCan(int animationIndex) {
	if (animationIndex >= 0) {
		while (_gameSys->getAnimationStatus(animationIndex) != 2)
			gameUpdateTick();
	}
	if (_gnapIdleFacing != kDirNone && _gnapIdleFacing != kDirBottomRight && _gnapIdleFacing != kDirUpRight)
		_s18_garbageCanPos = _gnapX - 1;
	else
		_s18_garbageCanPos = _gnapX + 1;
	clearFlag(kGFPlatyPussDisguised);
	scene18_updateHotspots();
	if (_gnapIdleFacing != kDirNone && _gnapIdleFacing != kDirBottomRight && _gnapIdleFacing != kDirUpRight) {
		_gameSys->insertSequence(0x107BA, _gnapId,
			makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
			kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
		_gnapSequenceId = 0x7BA;
	} else {
		_gameSys->insertSequence(0x107B9, _gnapId,
			makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
			kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
		_gnapSequenceId = 0x7B9;
	}
	_gnapSequenceDatNum = 1;
	_gameSys->insertSequence(0x1FB, 19, 0, 0, kSeqNone, 0, 15 * (5 * _s18_garbageCanPos - 40), 0);
	_gameSys->setAnimation(0x1FA, 19, 4);
	_gameSys->insertSequence(0x1FA, 19, 507, 19, kSeqSyncWait, 0, 15 * (5 * _s18_garbageCanPos - 40), 0);
	while (_gameSys->getAnimationStatus(4) != 2)
		gameUpdateTick();
}

void GnapEngine::scene18_platEndPhoning(bool platFl) {
	if (isFlag(kGFPlatypusTalkingToAssistant)) {
		_s18_platPhoneIter = 0;
		_s18_platPhoneCtr = 0;
		_beaverActionStatus = -1;
		if (_s18_currPhoneSequenceId != -1) {
			_gameSys->setAnimation(0x21E, 254, 3);
			_gameSys->insertSequence(0x21E, 254, _s18_currPhoneSequenceId, 254, kSeqSyncExists, 0, 0, 0);
			while (_gameSys->getAnimationStatus(3) != 2)
				gameUpdateTick();
		}
		_gameSys->removeSequence(0x21F, 254, true);
		_gameSys->setAnimation(0, 0, 3);
		clearFlag(kGFPlatypusTalkingToAssistant);
		if (platFl) {
			_beaverActionStatus = kASPlatComesHere;
			_timers[6] = 50;
			_sceneWaiting = true;
		}
		_s18_currPhoneSequenceId = -1;
		_s18_nextPhoneSequenceId = -1;
		scene18_updateHotspots();
	}
}

void GnapEngine::scene18_closeHydrantValve() {
	_gnapActionStatus = kASLeaveScene;
	updateMouseCursor();
	if (isFlag(kGFTruckFilledWithGas)) {
		gnapWalkTo(_hotspotsWalkPos[kHSHydrantRightValve].x, _hotspotsWalkPos[kHSHydrantRightValve].y, 0, 0x107BA, 1);
		if (isFlag(kGFTruckKeysUsed)) {
			_gnapActionStatus = kASCloseRightValveWithGarbageCan;
			scene18_waitForGnapAction();
		} else {
			_gnapActionStatus = kASCloseRightValveNoGarbageCan;
			scene18_waitForGnapAction();
		}
	} else if (isFlag(kGFBarnPadlockOpen)) {
		gnapWalkTo(_hotspotsWalkPos[kHSHydrantTopValve].x, _hotspotsWalkPos[kHSHydrantTopValve].y, 0, 0x107BA, 1);
		_gnapActionStatus = kASCloseTopValve;
		scene18_waitForGnapAction();
	}
}

void GnapEngine::scene18_waitForGnapAction() {
	while (_gnapActionStatus >= 0) {
		scene18_updateAnimations();
		gameUpdateTick();
	}
}

void GnapEngine::scene18_run() {
	_s18_cowboyHatSurface = nullptr;

	playSound(0x10940, true);
	startSoundTimerA(4);
	
	_timers[5] = getRandom(100) + 100;
	
	queueInsertDeviceIcon();
	
	clearFlag(kGFPlatyPussDisguised);
	
	if (!isFlag(kGFUnk14))
		_gameSys->insertSequence(0x1F8, 19, 0, 0, kSeqNone, 0, 0, 0);

	if (isFlag(kGFTruckKeysUsed)) {
		if (isFlag(kGFTruckFilledWithGas)) {
			_gameSys->insertSequence(0x214, 39, 0, 0, kSeqLoop, 0, 0, 0);
			_gameSys->insertSequence(0x20D, 39, 0, 0, kSeqLoop, 0, 0, 0);
			playSound(0x22B, true);
		} else {
			_gameSys->insertSequence(0x1F9, 19, 0, 0, kSeqNone, 0, 0, 0);
		}
	} else {
		_gameSys->insertSequence(0x1FA, 19, 0, 0, kSeqNone, 0, 15 * (5 * _s18_garbageCanPos - 40), 0);
		if (isFlag(kGFTruckFilledWithGas)) {
			_gameSys->insertSequence(0x212, 39, 0, 0, kSeqLoop, 0, 0, 0);
			_gameSys->insertSequence(0x20D, 39, 0, 0, kSeqLoop, 0, 0, 0);
			playSound(0x22B, true);
		} else if (isFlag(kGFBarnPadlockOpen)) {
			_gameSys->insertSequence(0x20E, 39, 0, 0, kSeqLoop, 0, 0, 0);
			_gameSys->insertSequence(0x217, 39, 0, 0, kSeqLoop, 0, 0, 0);
			playSound(0x22B, true);
		}
	}
	
	if (isFlag(kGFPlatypusTalkingToAssistant)) {
		if (_prevSceneNum == 17)
			initGnapPos(4, 11, kDirBottomRight);
		else
			initGnapPos(4, 7, kDirBottomRight);
		_s18_platPhoneCtr = getRandom(5);
		if (isFlag(kGFUnk27)) {
			_gameSys->insertSequence(0x21E, 254, 0, 0, kSeqNone, 0, 0, 0);
			endSceneInit();
			_s18_currPhoneSequenceId = -1;
			scene18_platEndPhoning(true);
			clearFlag(kGFUnk27);
		} else {
			_s18_currPhoneSequenceId = kScene18SequenceIds[_s18_platPhoneCtr];
			_s18_platPhoneIter = 0;
			_gameSys->insertSequence(0x21F, 254, 0, 0, kSeqNone, 0, 0, 0);
			_gameSys->insertSequence(_s18_currPhoneSequenceId, 254, 0, 0, kSeqNone, 0, 0, 0);
			endSceneInit();
		}
		if (isFlag(kGFUnk27)) {
			scene18_platEndPhoning(true);
			clearFlag(kGFUnk27);
		} else {
			_gameSys->setAnimation(_s18_currPhoneSequenceId, 254, 3);
		}
		gnapWalkTo(4, 8, -1, 0x107B9, 1);
	} else {
		if (isFlag(kGFGnapControlsToyUFO)) {
			clearFlag(kGFGnapControlsToyUFO);
			setGrabCursorSprite(kItemCowboyHat);
			_prevSceneNum = 19;
		}
		if (_prevSceneNum == 17) {
			initGnapPos(4, 11, kDirBottomRight);
			initBeaverPos(5, 11, kDirNone);
			endSceneInit();
			gnapWalkTo(4, 8, -1, 0x107B9, 1);
			platypusWalkTo(5, 9, -1, 0x107C2, 1);
		} else if (_prevSceneNum == 19) {
			initGnapPos(7, 7, kDirBottomRight);
			initBeaverPos(8, 7, kDirNone);
			endSceneInit();
			gnapWalkTo(7, 8, -1, 0x107B9, 1);
			platypusWalkTo(8, 8, -1, 0x107C2, 1);
		} else {
			initGnapPos(-1, 10, kDirBottomRight);
			initBeaverPos(-1, 10, kDirNone);
			endSceneInit();
			gnapWalkTo(3, 7, -1, 0x107B9, 1);
			platypusWalkTo(3, 8, -1, 0x107C2, 1);
		}
	}

	while (!_sceneDone) {
		updateMouseCursor();
		updateCursorByHotspot();
	
		testWalk(0, 20, -1, -1, -1, -1);
		
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
		
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene18_updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_gnapActionStatus < 0) {
				if (isFlag(kGFPlatyPussDisguised)) {
					scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					scene18_putDownGarbageCan(0);
				}
				if (_grabCursorSpriteIndex == kItemJoint) {
					gnapUseJointOnPlatypus();
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowItem(_grabCursorSpriteIndex, _platX, _platY);
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
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSCowboyHat:
			if (_gnapActionStatus == kASStandingOnHydrant) {
				_gnapActionStatus = kASGrabCowboyHat;
				_sceneWaiting = 0;
			} else if (_gnapActionStatus < 0) {
				if (isFlag(kGFPlatyPussDisguised)) {
					scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					scene18_putDownGarbageCan(0);
				}
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSCowboyHat].x, _hotspotsWalkPos[kHSCowboyHat].y, 3, 2);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(3, 2);
						break;
					case GRAB_CURSOR:
						gnapWalkTo(_hotspotsWalkPos[kHSCowboyHat].x, _hotspotsWalkPos[kHSCowboyHat].y, 0, getGnapSequenceId(gskPullOutDeviceNonWorking, 3, 2) | 0x10000, 1);
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSGarbageCan:
			if (_gnapActionStatus < 0) {
				if (isFlag(kGFUnk14)) {
					if (_grabCursorSpriteIndex >= 0)
						playGnapShowCurrItem(_hotspotsWalkPos[kHSGarbageCan].x, _hotspotsWalkPos[kHSGarbageCan].y, 1, 5);
					else
						playGnapImpossible(0, 0);
				} else {
					if (isFlag(kGFPlatypusTalkingToAssistant))
						scene18_platEndPhoning(true);
					if (_grabCursorSpriteIndex >= 0) {
						if (!isFlag(kGFTruckKeysUsed))
							playGnapShowCurrItem(_hotspotsWalkPos[kHSGarbageCan].x - (_gnapX < _s18_garbageCanPos ? 1 : -1),
								_hotspotsWalkPos[kHSGarbageCan].y, _hotspotsWalkPos[kHSGarbageCan].x, _hotspotsWalkPos[kHSGarbageCan].y);
						else
							playGnapShowCurrItem(_hotspotsWalkPos[kHSGarbageCan].x, _hotspotsWalkPos[kHSGarbageCan].y, 2, 4);
					} else {
						switch (_verbCursor) {
						case LOOK_CURSOR:
							if (!isFlag(kGFTruckKeysUsed))
								playGnapScratchingHead(_hotspotsWalkPos[kHSGarbageCan].x - (_gnapX < _s18_garbageCanPos ? 1 : -1), _hotspotsWalkPos[kHSGarbageCan].y);
							else if (!isFlag(kGFTruckFilledWithGas))
								playGnapScratchingHead(2, 4);
							break;
						case GRAB_CURSOR:
							if (!isFlag(kGFTruckKeysUsed)) {
								gnapWalkTo(_hotspotsWalkPos[kHSGarbageCan].x - (_gnapX < _s18_garbageCanPos ? 1 : -1), _hotspotsWalkPos[kHSGarbageCan].y,
									-1, -1, 1);
								gnapWalkTo(_gnapX, _gnapY, 0, getGnapSequenceId(gskIdle, _s18_garbageCanPos, _gnapY) | 0x10000, 1);
								_gnapActionStatus = kASGrabGarbageCanFromStreet;
							} else if (!isFlag(kGFTruckFilledWithGas)) {
								if (gnapWalkTo(_hotspotsWalkPos[kHSGarbageCan].x, _hotspotsWalkPos[kHSGarbageCan].y, 0, -1, 1))
									_gnapActionStatus = kASGrabGarbageCanFromHydrant;
							}
							break;
						case TALK_CURSOR:
						case PLAT_CURSOR:
							playGnapImpossible(0, 0);
							break;
						}
					}
				}
			}
			break;

		case kHSHydrantTopValve:
			if (_gnapActionStatus < 0) {
				if (isFlag(kGFPlatyPussDisguised)) {
					// While carrying garbage can
					if (_grabCursorSpriteIndex >= 0) {
						scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
						scene18_putDownGarbageCan(0);
						playGnapShowItem(_grabCursorSpriteIndex, 0, 0);
					} else {
						switch (_verbCursor) {
						case LOOK_CURSOR:
							scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
							scene18_putDownGarbageCan(0);
							playGnapScratchingHead(0, 0);
							break;
						case GRAB_CURSOR:
							if (isFlag(kGFTruckFilledWithGas)) {
								scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, 2);
								_gnapActionStatus = kASPutGarbageCanOnRunningHydrant;
							} else if (!isFlag(kGFBarnPadlockOpen)) {
								scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, 2);
								_gnapActionStatus = kASPutGarbageCanOnHydrant;
							} else {
								scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
								scene18_putDownGarbageCan(0);
								playGnapImpossible(0, 0);
							}
							break;
						case TALK_CURSOR:
						case PLAT_CURSOR:
							scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
							scene18_putDownGarbageCan(0);
							playGnapImpossible(0, 0);
							break;
						}
					}
				} else {
					if (_grabCursorSpriteIndex == kItemWrench) {
						gnapWalkTo(_gnapX, _gnapY, 0, getGnapSequenceId(gskIdle, 2, 8) | 0x10000, 1);
						_gnapActionStatus = kASOpenTopValve;
					} else if (_grabCursorSpriteIndex >= 0) {
						playGnapShowCurrItem(_hotspotsWalkPos[kHSHydrantTopValve].x, _hotspotsWalkPos[kHSHydrantTopValve].y, 1, 5);
					} else {
						switch (_verbCursor) {
						case LOOK_CURSOR:
							playGnapScratchingHead(1, 5);
							break;
						case GRAB_CURSOR:
							if (isFlag(kGFBarnPadlockOpen)) {
								_hotspots[kHSWalkArea2]._flags |= SF_WALKABLE;
								gnapWalkTo(_hotspotsWalkPos[kHSHydrantTopValve].x, _hotspotsWalkPos[kHSHydrantTopValve].y, 0, 0x107BA, 1);
								_hotspots[kHSWalkArea2]._flags &= ~SF_WALKABLE;
								_gnapActionStatus = kASCloseTopValve;
							} else
								playGnapImpossible(0, 0);
							break;
						case TALK_CURSOR:
						case PLAT_CURSOR:
							playGnapImpossible(0, 0);
							break;
						}
					}
				}
			}
			break;

		case kHSHydrantRightValve:
			if (_gnapActionStatus < 0) {
				if (isFlag(kGFUnk14)) {
					if (_grabCursorSpriteIndex == -1) {
						playGnapImpossible(0, 0);
					} else {
						playGnapShowCurrItem(_hotspotsWalkPos[kHSHydrantRightValve].x, _hotspotsWalkPos[kHSHydrantRightValve].y, 1, 5);
					}
				} else {
					if (isFlag(kGFPlatyPussDisguised)) {
						scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
						scene18_putDownGarbageCan(0);
					}
					if (_grabCursorSpriteIndex == kItemWrench) {
						gnapWalkTo(_gnapX, _gnapY, 0, getGnapSequenceId(gskIdle, 2, 8) | 0x10000, 1);
						if (isFlag(kGFTruckKeysUsed))
							_gnapActionStatus = kASOpenRightValveWithGarbageCan;
						else
							_gnapActionStatus = kASOpenRightValveNoGarbageCan;
					} else if (_grabCursorSpriteIndex >= 0) {
						playGnapShowCurrItem(_hotspotsWalkPos[kHSHydrantRightValve].x, _hotspotsWalkPos[kHSHydrantRightValve].y, 1, 5);
					} else {
						switch (_verbCursor) {
						case LOOK_CURSOR:
							playGnapScratchingHead(1, 5);
							break;
						case GRAB_CURSOR:
							if (isFlag(kGFTruckFilledWithGas)) {
								gnapWalkTo(_hotspotsWalkPos[kHSHydrantRightValve].x, _hotspotsWalkPos[kHSHydrantRightValve].y, 0, 0x107BA, 1);
								if (isFlag(kGFTruckKeysUsed))
									_gnapActionStatus = kASCloseRightValveWithGarbageCan;
								else
									_gnapActionStatus = kASCloseRightValveNoGarbageCan;
							}
							break;
						case TALK_CURSOR:
						case PLAT_CURSOR:
							playGnapImpossible(0, 0);
							break;
						}
					}
				}
			}
			break;

		case kHSExitToyStore:
			if (_gnapActionStatus < 0) {
				if (isFlag(kGFPlatyPussDisguised)) {
					scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					scene18_putDownGarbageCan(0);
				}
				if (isFlag(kGFPictureTaken)) {
					playGnapImpossible(0, 0);
				} else {
					_isLeavingScene = 1;
					_newSceneNum = 19;
					gnapWalkTo(_hotspotsWalkPos[kHSExitToyStore].x, _hotspotsWalkPos[kHSExitToyStore].y, 0, 0x107C0, 1);
					_gnapActionStatus = kASLeaveScene;
					if (!isFlag(kGFPlatypusTalkingToAssistant))
						platypusWalkTo(_hotspotsWalkPos[kHSExitToyStore].x + 1, _hotspotsWalkPos[kHSExitToyStore].y, -1, 0x107C2, 1);
				}
			}
			break;

		case kHSExitPhoneBooth:
			if (_gnapActionStatus < 0) {
				if (isFlag(kGFPlatyPussDisguised)) {
					scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					scene18_putDownGarbageCan(0);
				}
				scene18_closeHydrantValve();
				_isLeavingScene = 1;
				_newSceneNum = 17;
				gnapWalkTo(_hotspotsWalkPos[kHSExitPhoneBooth].x, _hotspotsWalkPos[kHSExitPhoneBooth].y, 0, 0x107AE, 1);
				_gnapActionStatus = kASLeaveScene;
				if (isFlag(kGFPlatypusTalkingToAssistant))
					setFlag(kGFUnk27);
				else
					platypusWalkTo(_hotspotsWalkPos[kHSExitPhoneBooth].x + 1, _hotspotsWalkPos[kHSExitPhoneBooth].y, -1, 0x107C2, 1);
			}
			break;

		case kHSExitGrubCity:
			if (_gnapActionStatus < 0) {
				if (isFlag(kGFPlatyPussDisguised)) {
					scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					scene18_putDownGarbageCan(0);
				}
				scene18_closeHydrantValve();
				_isLeavingScene = 1;
				_newSceneNum = 20;
				_hotspots[kHSWalkArea2]._flags |= SF_WALKABLE;
				gnapWalkTo(_hotspotsWalkPos[kHSExitGrubCity].x, _hotspotsWalkPos[kHSExitGrubCity].y, 0, 0x107B2, 1);
				_gnapActionStatus = kASLeaveScene;
				if (isFlag(kGFPlatypusTalkingToAssistant))
					scene18_platEndPhoning(false);
				else
					platypusWalkTo(_hotspotsWalkPos[kHSExitGrubCity].x, _hotspotsWalkPos[kHSExitGrubCity].y - 1, -1, 0x107CF, 1);
				_hotspots[kHSWalkArea2]._flags &= ~SF_WALKABLE;
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
			if (_gnapActionStatus < 0) {
				if (isFlag(kGFPlatyPussDisguised)) {
					scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					scene18_putDownGarbageCan(0);
				} else {
					gnapWalkTo(-1, -1, -1, -1, 1);
				}
				_mouseClickState._left = false;
			}
			break;

		default:
			if (_gnapActionStatus != kASStandingOnHydrant && _mouseClickState._left) {
				if (isFlag(kGFPlatyPussDisguised)) {
					scene18_gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					scene18_putDownGarbageCan(0);
				} else {
					gnapWalkTo(-1, -1, -1, -1, 1);
				}
				_mouseClickState._left = false;
			}
			break;

		}
	
		scene18_updateAnimations();
	
		if (!isSoundPlaying(0x10940))
			playSound(0x10940, true);
	
		if ((isFlag(kGFTruckFilledWithGas) || isFlag(kGFBarnPadlockOpen)) && !isSoundPlaying(0x22B) &&
			_gnapActionStatus != kASOpenRightValveNoGarbageCanDone && _gnapActionStatus != kASOpenRightValveNoGarbageCan &&
			_gnapActionStatus != kASOpenTopValve && _gnapActionStatus != kASOpenTopValveDone &&
			_gnapActionStatus != kASOpenRightValveWithGarbageCan && _gnapActionStatus != kASOpenRightValveWithGarbageCanDone)
			playSound(0x22B, true);
	
		if (!_isLeavingScene) {
			if (!isFlag(kGFPlatypusTalkingToAssistant)) {
				if (_beaverActionStatus == kASPlatComesHere) {
					if (!_timers[6]) {
						_beaverActionStatus = -1;
						_sceneWaiting = false;
						initBeaverPos(-1, 10, kDirNone);
						platypusWalkTo(3, 9, -1, 0x107C2, 1);
						clearFlag(kGFPlatypusTalkingToAssistant);
					}
				} else {
					_hotspots[kHSWalkArea1]._y2 += 48;
					_hotspots[kHSWalkArea2]._x1 += 75;
					updateBeaverIdleSequence();
					_hotspots[kHSWalkArea2]._x1 -= 75;
					_hotspots[kHSWalkArea1]._y2 -= 48;
				}
				if (!_timers[5]) {
					_timers[5] = getRandom(100) + 100;
					if (_gnapActionStatus < 0) {
						if (getRandom(2) == 1)
							_gameSys->insertSequence(0x220, 255, 0, 0, kSeqNone, 0, 0, 0);
						else
							_gameSys->insertSequence(0x221, 255, 0, 0, kSeqNone, 0, 0, 0);
					}
				}
				playSoundA();
			}
			if (!isFlag(kGFPlatyPussDisguised))
				updateGnapIdleSequence();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene18_updateHotspots();
		}
		
		gameUpdateTick();
	
	}

	if (isFlag(kGFGnapControlsToyUFO))
		deleteSurface(&_s18_cowboyHatSurface);
}

void GnapEngine::scene18_updateAnimations() {
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASGrabGarbageCanFromStreet:
			if (_gnapIdleFacing != kDirUpRight && _gnapIdleFacing != kDirBottomRight) {
				_gameSys->insertSequence(0x1FC, _gnapId,
					makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
					kSeqSyncWait, 0, 75 * _gnapX - 675, 0);
				_gnapSequenceDatNum = 0;
				_gnapSequenceId = 0x1FC;
			} else {
				_gameSys->insertSequence(0x1FD, _gnapId,
					makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
					kSeqSyncWait, 0, 75 * _gnapX - 525, 0);
				_gnapSequenceDatNum = 0;
				_gnapSequenceId = 0x1FD;
			}
			_gameSys->removeSequence(0x1FA, 19, true);
			setFlag(kGFPlatyPussDisguised);
			scene18_updateHotspots();
			_gnapActionStatus = -1;
			break;
		case kASGrabGarbageCanFromHydrant:
			_gameSys->insertSequence(0x1FE, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->removeSequence(0x1F9, 19, true);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x1FE;
			clearFlag(kGFTruckKeysUsed);
			setFlag(kGFPlatyPussDisguised);
			scene18_updateHotspots();
			_gnapActionStatus = -1;
			break;
		case kASCloseRightValveNoGarbageCan:
			_gameSys->insertSequence(0x205, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->removeSequence(0x20D, 39, true);
			_gameSys->removeSequence(0x212, 39, true);
			_gameSys->removeSequence(0x211, 39, true);
			stopSound(0x22B);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x205;
			clearFlag(kGFTruckFilledWithGas);
			invAdd(kItemWrench);
			setGrabCursorSprite(kItemWrench);
			scene18_updateHotspots();
			_gnapActionStatus = -1;
			break;
		case kASOpenTopValve:
			setFlag(kGFBarnPadlockOpen);
			scene18_updateHotspots();
			playGnapPullOutDevice(2, 7);
			playGnapUseDevice(0, 0);
			_gameSys->insertSequence(0x20C, 19, 0, 0, kSeqNone, 0, 0, 0);
			_hotspots[kHSWalkArea2]._flags |= SF_WALKABLE;
			gnapWalkTo(_hotspotsWalkPos[kHSHydrantTopValve].x, _hotspotsWalkPos[kHSHydrantTopValve].y, 0, 0x107BB, 1);
			_hotspots[kHSWalkArea2]._flags &= ~SF_WALKABLE;
			_gnapActionStatus = kASOpenTopValveDone;
			break;
		case kASOpenTopValveDone:
			setGrabCursorSprite(-1);
			_gameSys->insertSequence(0x208, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x216, 39, 0, 0, kSeqNone, 21, 0, 0);
			_gameSys->removeSequence(0x20C, 19, true);
			_gameSys->setAnimation(0x217, 39, 5);
			_gameSys->insertSequence(0x217, 39, 0x216, 39, kSeqLoop | kSeqSyncWait, 0, 0, 0);
			while (_gameSys->getAnimationStatus(5) != 2)
				gameUpdateTick();
			playSound(0x22B, true);
			_gameSys->insertSequence(0x20E, 39, 0, 0, kSeqNone, 0, 0, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x208;
			invRemove(kItemWrench);
			setGrabCursorSprite(-1);
			_gnapActionStatus = -1;
			break;
		case kASCloseTopValve:
			_gameSys->insertSequence(0x206, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->removeSequence(0x20E, 39, true);
			_gameSys->removeSequence(0x216, 39, true);
			_gameSys->removeSequence(0x217, 39, true);
			stopSound(0x22B);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x206;
			clearFlag(kGFBarnPadlockOpen);
			invAdd(kItemWrench);
			setGrabCursorSprite(kItemWrench);
			scene18_updateHotspots();
			_gnapActionStatus = -1;
			break;
		case kASGrabCowboyHat:
			_gameSys->setAnimation(0x200, _gnapId, 0);
			_gameSys->insertSequence(0x200, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x200;
			_gnapActionStatus = kASGrabCowboyHatDone;
			break;
		case kASGrabCowboyHatDone:
			hideCursor();
			setGrabCursorSprite(-1);
			_s18_cowboyHatSurface = addFullScreenSprite(0x1D2, 255);
			_gameSys->setAnimation(0x218, 256, 0);
			_gameSys->insertSequence(0x218, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_gameSys->getAnimationStatus(0) != 2)
				gameUpdateTick();
			_newSceneNum = 18;
			invAdd(kItemCowboyHat);
			invAdd(kItemWrench);
			setFlag(kGFGnapControlsToyUFO);
			setFlag(kGFUnk14);
			clearFlag(kGFTruckFilledWithGas);
			setFlag(kGFTruckKeysUsed);
			setFlag(kGFUnk14); // CHECKME - Set 2 times?
			scene18_updateHotspots();
			_gnapActionStatus = kASLeaveScene;
			break;
		case kASLeaveScene:
			_sceneDone = true;
			_gnapActionStatus = -1;
			break;
		case kASPutGarbageCanOnRunningHydrant:
			setFlag(kGFTruckKeysUsed);
			clearFlag(kGFPlatyPussDisguised);
			_gameSys->requestRemoveSequence(0x211, 39);
			_gameSys->requestRemoveSequence(0x212, 39);
			_gameSys->insertSequence(0x210, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			stopSound(0x22B);
			_gameSys->setAnimation(0x210, _gnapId, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x210;
			_gnapActionStatus = kASPutGarbageCanOnRunningHydrant2;
			break;
		case kASPutGarbageCanOnRunningHydrant2:
			playSound(0x22B, true);
			_gameSys->setAnimation(0x1FF, _gnapId, 0);
			_gameSys->insertSequence(0x1FF, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x1FF;
			_sceneWaiting = true;
			_gnapActionStatus = kASStandingOnHydrant;
			break;
		case kASStandingOnHydrant:
			_gameSys->setAnimation(0x1FF, _gnapId, 0);
			_gameSys->insertSequence(0x1FF, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			break;
		case kASOpenRightValveNoGarbageCan:
		case kASOpenRightValveWithGarbageCan:
			setFlag(kGFTruckFilledWithGas);
			scene18_updateHotspots();
			playGnapPullOutDevice(2, 7);
			playGnapUseDevice(0, 0);
			_gameSys->insertSequence(0x20B, 19, 0, 0, kSeqNone, 0, 0, 0);
			_hotspots[kHSWalkArea2]._flags |= SF_WALKABLE;
			gnapWalkTo(_hotspotsWalkPos[kHSHydrantRightValve].x, _hotspotsWalkPos[kHSHydrantRightValve].y, 0, 0x107BA, 1);
			_hotspots[kHSWalkArea2]._flags &= ~SF_WALKABLE;
			if (_gnapActionStatus == kASOpenRightValveNoGarbageCan)
				_gnapActionStatus = kASOpenRightValveNoGarbageCanDone;
			else
				_gnapActionStatus = kASOpenRightValveWithGarbageCanDone;
			break;
		case kASOpenRightValveWithGarbageCanDone:
			setGrabCursorSprite(-1);
			_gameSys->insertSequence(0x207, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x213, 39, 0, 0, kSeqNone, 21, 0, 0);
			_gameSys->requestRemoveSequence(0x1F9, 19);
			_gameSys->removeSequence(0x20B, 19, true);
			_gameSys->setAnimation(0x213, 39, 5);
			_gameSys->insertSequence(0x214, 39, 0x213, 39, kSeqLoop | kSeqSyncWait, 0, 0, 0);
			while (_gameSys->getAnimationStatus(5) != 2)
				gameUpdateTick();
			playSound(555, true);
			_gameSys->insertSequence(0x20D, 39, 0, 0, kSeqNone, 0, 0, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x207;
			invRemove(kItemWrench);
			_gnapActionStatus = -1;
			break;
		case kASOpenRightValveNoGarbageCanDone:
			setGrabCursorSprite(-1);
			_gameSys->insertSequence(0x207, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x211, 39, 0, 0, kSeqNone, 21, 0, 0);
			_gameSys->removeSequence(0x20B, 19, true);
			_gameSys->setAnimation(0x211, 39, 5);
			_gameSys->insertSequence(0x212, 39, 0x211, 39, kSeqLoop | kSeqSyncWait, 0, 0, 0);
			while (_gameSys->getAnimationStatus(5) != 2)
				gameUpdateTick();
			playSound(0x22B, true);
			_gameSys->insertSequence(0x20D, 39, 0, 0, kSeqNone, 0, 0, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x207;
			invRemove(kItemWrench);
			_gnapActionStatus = -1;
			break;
		case kASCloseRightValveWithGarbageCan:
			_gameSys->insertSequence(0x205, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->removeSequence(0x20D, 39, true);
			_gameSys->insertSequence(0x215, 39, 0x214, 39, kSeqSyncWait, 0, 0, 0);
			stopSound(0x22B);
			_gameSys->setAnimation(0x1F9, 19, 0);
			_gameSys->insertSequence(0x1F9, 19, 0x215, 39, kSeqSyncWait, 0, 0, 0);
			clearFlag(kGFTruckFilledWithGas);
			invAdd(kItemWrench);
			setGrabCursorSprite(kItemWrench);
			_gameSys->insertSequence(0x107B5, _gnapId, 517, _gnapId, kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
			scene18_updateHotspots();
			_gnapSequenceDatNum = 1;
			_gnapSequenceId = 0x7B5;
			_gnapActionStatus = kASCloseRightValveWithGarbageCanDone;
			break;
		case kASCloseRightValveWithGarbageCanDone:
			_gnapActionStatus = -1;
			break;
		case kASPutGarbageCanOnHydrant:
			setFlag(kGFTruckKeysUsed);
			clearFlag(kGFPlatyPussDisguised);
			_gameSys->insertSequence(0x20F, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(0x20F, _gnapId, 0);
			_gnapSequenceDatNum = 0;
			_gnapSequenceId = 0x20F;
			_gnapActionStatus = kASPutGarbageCanOnHydrantDone;
			break;
		case kASPutGarbageCanOnHydrantDone:
			_gameSys->insertSequence(0x1F9, 19, 0x20F, _gnapId, kSeqNone, 0, 0, 0);
			scene18_updateHotspots();
			_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(3) == 2) {
		_gameSys->setAnimation(0, 0, 3);
		++_s18_platPhoneIter;
		if (_s18_platPhoneIter <= 4) {
			++_s18_platPhoneCtr;
			_s18_nextPhoneSequenceId = kScene18SequenceIds[_s18_platPhoneCtr % 5];
			_gameSys->setAnimation(_s18_nextPhoneSequenceId, 254, 3);
			_gameSys->insertSequence(_s18_nextPhoneSequenceId, 254, _s18_currPhoneSequenceId, 254, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x21F, 254, 0x21F, 254, kSeqSyncWait, 0, 0, 0);
			_s18_currPhoneSequenceId = _s18_nextPhoneSequenceId;
		} else {
			scene18_platEndPhoning(true);
		}
	}
}

} // End of namespace Gnap
