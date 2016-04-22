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
#include "gnap/scenes/scene18.h"

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

Scene18::Scene18(GnapEngine *vm) : Scene(vm) {
	_s18_cowboyHatSurface = nullptr;

	_s18_platPhoneCtr = 0;
	_s18_platPhoneIter = 0;
	_s18_nextPhoneSequenceId = -1;
	_s18_currPhoneSequenceId = -1;
}

Scene18::~Scene18() {
	delete _s18_cowboyHatSurface;
}

int Scene18::init() {
	_vm->_gameSys->setAnimation(0, 0, 3);
	return 0x222;
}

void Scene18::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSGarbageCan, _vm->_gridMinX + 75 * _vm->_s18_garbageCanPos - 35, _vm->_gridMinY + 230, _vm->_gridMinX + 75 * _vm->_s18_garbageCanPos + 35, _vm->_gridMinY + 318,
		SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, _vm->_s18_garbageCanPos, 7);
	_vm->setHotspot(kHSExitToyStore, 460, 238, 592, 442, SF_EXIT_U_CURSOR, 7, 7);
	_vm->setHotspot(kHSExitPhoneBooth, 275, 585, 525, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 10);
	_vm->setHotspot(kHSExitGrubCity, 0, 350, 15, 600, SF_EXIT_L_CURSOR, 0, 9);
	_vm->setHotspot(kHSHydrantTopValve, 100, 345, 182, 410, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 8);
	_vm->setHotspot(kHSHydrantRightValve, 168, 423, 224, 470, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	_vm->setHotspot(kHSCowboyHat, 184, 63, 289, 171, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 448);
	_vm->setHotspot(kHSWalkArea2, 0, 0, 214, 515);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->isFlag(kGFTruckFilledWithGas)) {
		if (_vm->isFlag(kGFTruckKeysUsed)) {
			_vm->_hotspots[kHSHydrantTopValve]._flags = SF_DISABLED;
			_vm->_hotspots[kHSHydrantRightValve]._x1 = 148;
			_vm->_hotspots[kHSHydrantRightValve]._y1 = 403;
			_vm->_hotspots[kHSGarbageCan]._flags = SF_DISABLED;
			_vm->_hotspotsWalkPos[kHSGarbageCan].x = 3;
			_vm->_hotspotsWalkPos[kHSGarbageCan].y = 7;
		} else {
			_vm->_hotspots[kHSHydrantTopValve]._y1 = 246;
		}
	} else if (_vm->isFlag(kGFBarnPadlockOpen)) {
		_vm->_hotspots[kHSHydrantRightValve]._flags = SF_DISABLED;
		_vm->_hotspots[kHSHydrantTopValve]._x1 = 105;
		_vm->_hotspots[kHSHydrantTopValve]._x2 = 192;
	} else if (_vm->isFlag(kGFTruckKeysUsed)) {
		_vm->_hotspots[kHSGarbageCan]._x1 = 115;
		_vm->_hotspots[kHSGarbageCan]._y1 = 365;
		_vm->_hotspots[kHSGarbageCan]._x2 = 168;
		_vm->_hotspots[kHSGarbageCan]._y2 = 470;
		_vm->_hotspots[kHSGarbageCan]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
		_vm->_hotspotsWalkPos[kHSGarbageCan].x = 3;
		_vm->_hotspotsWalkPos[kHSGarbageCan].y = 7;
	}
	if (_vm->isFlag(kGFPlatyPussDisguised))
		_vm->_hotspots[kHSGarbageCan]._flags = SF_DISABLED;
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_vm->_hotspots[kHSDevice]._flags = SF_DISABLED;
		_vm->_hotspots[kHSHydrantTopValve]._flags = SF_DISABLED;
		_vm->_hotspots[kHSHydrantRightValve]._flags = SF_DISABLED;
		_vm->_hotspots[kHSPlatypus]._flags = SF_DISABLED;
	}
	if (_vm->isFlag(kGFUnk14)) {
		_vm->_hotspots[kHSHydrantTopValve]._flags = SF_DISABLED;
		_vm->_hotspots[kHSCowboyHat]._flags = SF_DISABLED;
	}
	_vm->_hotspotsCount = 11;
}

void Scene18::gnapCarryGarbageCanTo(int x, int y, int animationIndex, int argC, int a5) {
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
		clippedX = (_vm->_leftClickMouseX - _vm->_gridMinX + 37) / 75;
	if (clippedX >= _vm->_gnapX)
		v10 = clippedX - 1;
	else
		v10 = clippedX + 1;

	if (a5 < 0)
		a5 = 4;
	v5 = v10;
	if (v10 <= a5)
		v5 = a5;
	v11 = v5;
	v12 = _vm->_gridMaxX - 1;
	if (_vm->_gridMaxX - 1 >= v11)
		v12 = v11;
	
	if (v12 == _vm->_gnapX) {
		gnapSeqId = _vm->_gnapSequenceId;
		gnapId = _vm->_gnapId;
		gnapDatNum = _vm->_gnapSequenceDatNum;
		gnapGridX = _vm->_gnapX;
		if (_vm->_gnapX <= clippedX)
			direction = 1;
		else
			direction = -1;
	} else {
		if (_vm->_gnapY == _vm->_platY) {
			if (v12 >= _vm->_gnapX) {
				if (v12 >= _vm->_platX && _vm->_gnapX <= _vm->_platX)
					_vm->platypusMakeRoom();
			} else if (v12 <= _vm->_platX && _vm->_gnapX >= _vm->_platX) {
				_vm->platypusMakeRoom();
			}
		}
		gnapSeqId = _vm->_gnapSequenceId;
		gnapId = _vm->_gnapId;
		gnapDatNum = _vm->_gnapSequenceDatNum;
		gnapGridX = _vm->_gnapX;
		int seqId = 0;
		if (v12 < _vm->_gnapX) {
			direction = -1;
			seqId = 1;
		} else
			direction = 1;
		int a2 = 20 * _vm->_gnapY + 1;
		do {
			if (_vm->isPointBlocked(gnapGridX + direction, _vm->_gnapY))
				break;
			a2 += direction;
			_vm->_gameSys->insertSequence(kSequenceIds[seqId], a2,
				gnapSeqId | (gnapDatNum << 16), gnapId,
				kSeqSyncWait, 0, 75 * gnapGridX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
			gnapSeqId = kSequenceIds[seqId];
			gnapId = a2;
			gnapDatNum = 0;
			gnapGridX += direction;
		} while (v12 != gnapGridX);
	}
	
	if (argC >= 0) {
		_vm->_gnapSequenceId = ridToEntryIndex(argC);
		_vm->_gnapSequenceDatNum = ridToDatIndex(argC);
	} else {
		if (direction == 1)
			_vm->_gnapSequenceId = 0x20A;
		else
			_vm->_gnapSequenceId = 0x209;
		_vm->_gnapSequenceDatNum = 0;
	}
	
	if (direction == 1)
		_vm->_gnapIdleFacing = kDirBottomRight;
	else
		_vm->_gnapIdleFacing = kDirBottomLeft;
	
	_vm->_gnapId = 20 * _vm->_gnapY + 1;
	
	if (animationIndex >= 0)
		_vm->_gameSys->setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, animationIndex);
	
	_vm->_gameSys->insertSequence(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
		gnapSeqId | (gnapDatNum << 16), gnapId,
		kSeqScale | kSeqSyncWait, 0, 75 * gnapGridX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);

	_vm->_gnapX = gnapGridX;

}

void Scene18::putDownGarbageCan(int animationIndex) {
	if (animationIndex >= 0) {
		while (_vm->_gameSys->getAnimationStatus(animationIndex) != 2)
			_vm->gameUpdateTick();
	}
	if (_vm->_gnapIdleFacing != kDirNone && _vm->_gnapIdleFacing != kDirBottomRight && _vm->_gnapIdleFacing != kDirUpRight)
		_vm->_s18_garbageCanPos = _vm->_gnapX - 1;
	else
		_vm->_s18_garbageCanPos = _vm->_gnapX + 1;
	_vm->clearFlag(kGFPlatyPussDisguised);
	updateHotspots();
	if (_vm->_gnapIdleFacing != kDirNone && _vm->_gnapIdleFacing != kDirBottomRight && _vm->_gnapIdleFacing != kDirUpRight) {
		_vm->_gameSys->insertSequence(0x107BA, _vm->_gnapId,
			makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
			kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
		_vm->_gnapSequenceId = 0x7BA;
	} else {
		_vm->_gameSys->insertSequence(0x107B9, _vm->_gnapId,
			makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
			kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
		_vm->_gnapSequenceId = 0x7B9;
	}
	_vm->_gnapSequenceDatNum = 1;
	_vm->_gameSys->insertSequence(0x1FB, 19, 0, 0, kSeqNone, 0, 15 * (5 * _vm->_s18_garbageCanPos - 40), 0);
	_vm->_gameSys->setAnimation(0x1FA, 19, 4);
	_vm->_gameSys->insertSequence(0x1FA, 19, 507, 19, kSeqSyncWait, 0, 15 * (5 * _vm->_s18_garbageCanPos - 40), 0);
	while (_vm->_gameSys->getAnimationStatus(4) != 2)
		_vm->gameUpdateTick();
}

void Scene18::platEndPhoning(bool platFl) {
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_s18_platPhoneIter = 0;
		_s18_platPhoneCtr = 0;
		_vm->_platypusActionStatus = -1;
		if (_s18_currPhoneSequenceId != -1) {
			_vm->_gameSys->setAnimation(0x21E, 254, 3);
			_vm->_gameSys->insertSequence(0x21E, 254, _s18_currPhoneSequenceId, 254, kSeqSyncExists, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(3) != 2)
				_vm->gameUpdateTick();
		}
		_vm->_gameSys->removeSequence(0x21F, 254, true);
		_vm->_gameSys->setAnimation(0, 0, 3);
		_vm->clearFlag(kGFPlatypusTalkingToAssistant);
		if (platFl) {
			_vm->_platypusActionStatus = kASPlatComesHere;
			_vm->_timers[6] = 50;
			_vm->_sceneWaiting = true;
		}
		_s18_currPhoneSequenceId = -1;
		_s18_nextPhoneSequenceId = -1;
		updateHotspots();
	}
}

void Scene18::closeHydrantValve() {
	_vm->_gnapActionStatus = kASLeaveScene;
	_vm->updateMouseCursor();
	if (_vm->isFlag(kGFTruckFilledWithGas)) {
		_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSHydrantRightValve].x, _vm->_hotspotsWalkPos[kHSHydrantRightValve].y, 0, 0x107BA, 1);
		if (_vm->isFlag(kGFTruckKeysUsed)) {
			_vm->_gnapActionStatus = kASCloseRightValveWithGarbageCan;
			waitForGnapAction();
		} else {
			_vm->_gnapActionStatus = kASCloseRightValveNoGarbageCan;
			waitForGnapAction();
		}
	} else if (_vm->isFlag(kGFBarnPadlockOpen)) {
		_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSHydrantTopValve].x, _vm->_hotspotsWalkPos[kHSHydrantTopValve].y, 0, 0x107BA, 1);
		_vm->_gnapActionStatus = kASCloseTopValve;
		waitForGnapAction();
	}
}

void Scene18::waitForGnapAction() {
	while (_vm->_gnapActionStatus >= 0) {
		updateAnimations();
		_vm->gameUpdateTick();
	}
}

void Scene18::run() {
	_s18_cowboyHatSurface = nullptr;

	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(4);
	
	_vm->_timers[5] = _vm->getRandom(100) + 100;
	
	_vm->queueInsertDeviceIcon();
	
	_vm->clearFlag(kGFPlatyPussDisguised);
	
	if (!_vm->isFlag(kGFUnk14))
		_vm->_gameSys->insertSequence(0x1F8, 19, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFTruckKeysUsed)) {
		if (_vm->isFlag(kGFTruckFilledWithGas)) {
			_vm->_gameSys->insertSequence(0x214, 39, 0, 0, kSeqLoop, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x20D, 39, 0, 0, kSeqLoop, 0, 0, 0);
			_vm->playSound(0x22B, true);
		} else {
			_vm->_gameSys->insertSequence(0x1F9, 19, 0, 0, kSeqNone, 0, 0, 0);
		}
	} else {
		_vm->_gameSys->insertSequence(0x1FA, 19, 0, 0, kSeqNone, 0, 15 * (5 * _vm->_s18_garbageCanPos - 40), 0);
		if (_vm->isFlag(kGFTruckFilledWithGas)) {
			_vm->_gameSys->insertSequence(0x212, 39, 0, 0, kSeqLoop, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x20D, 39, 0, 0, kSeqLoop, 0, 0, 0);
			_vm->playSound(0x22B, true);
		} else if (_vm->isFlag(kGFBarnPadlockOpen)) {
			_vm->_gameSys->insertSequence(0x20E, 39, 0, 0, kSeqLoop, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x217, 39, 0, 0, kSeqLoop, 0, 0, 0);
			_vm->playSound(0x22B, true);
		}
	}
	
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		if (_vm->_prevSceneNum == 17)
			_vm->initGnapPos(4, 11, kDirBottomRight);
		else
			_vm->initGnapPos(4, 7, kDirBottomRight);
		_s18_platPhoneCtr = _vm->getRandom(5);
		if (_vm->isFlag(kGFUnk27)) {
			_vm->_gameSys->insertSequence(0x21E, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
			_s18_currPhoneSequenceId = -1;
			platEndPhoning(true);
			_vm->clearFlag(kGFUnk27);
		} else {
			_s18_currPhoneSequenceId = kScene18SequenceIds[_s18_platPhoneCtr];
			_s18_platPhoneIter = 0;
			_vm->_gameSys->insertSequence(0x21F, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gameSys->insertSequence(_s18_currPhoneSequenceId, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
		}
		if (_vm->isFlag(kGFUnk27)) {
			platEndPhoning(true);
			_vm->clearFlag(kGFUnk27);
		} else {
			_vm->_gameSys->setAnimation(_s18_currPhoneSequenceId, 254, 3);
		}
		_vm->gnapWalkTo(4, 8, -1, 0x107B9, 1);
	} else {
		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			_vm->clearFlag(kGFGnapControlsToyUFO);
			_vm->setGrabCursorSprite(kItemCowboyHat);
			_vm->_prevSceneNum = 19;
		}
		if (_vm->_prevSceneNum == 17) {
			_vm->initGnapPos(4, 11, kDirBottomRight);
			_vm->initPlatypusPos(5, 11, kDirNone);
			_vm->endSceneInit();
			_vm->gnapWalkTo(4, 8, -1, 0x107B9, 1);
			_vm->platypusWalkTo(5, 9, -1, 0x107C2, 1);
		} else if (_vm->_prevSceneNum == 19) {
			_vm->initGnapPos(7, 7, kDirBottomRight);
			_vm->initPlatypusPos(8, 7, kDirNone);
			_vm->endSceneInit();
			_vm->gnapWalkTo(7, 8, -1, 0x107B9, 1);
			_vm->platypusWalkTo(8, 8, -1, 0x107C2, 1);
		} else {
			_vm->initGnapPos(-1, 10, kDirBottomRight);
			_vm->initPlatypusPos(-1, 10, kDirNone);
			_vm->endSceneInit();
			_vm->gnapWalkTo(3, 7, -1, 0x107B9, 1);
			_vm->platypusWalkTo(3, 8, -1, 0x107C2, 1);
		}
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 20, -1, -1, -1, -1);
		
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
		
		switch (_vm->_sceneClickedHotspot) {
		case kHSDevice:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatyPussDisguised)) {
					gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					putDownGarbageCan(0);
				}
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, _vm->_platX, _vm->_platY);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan1(_vm->_platX, _vm->_platY);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_platX, _vm->_platY);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSCowboyHat:
			if (_vm->_gnapActionStatus == kASStandingOnHydrant) {
				_vm->_gnapActionStatus = kASGrabCowboyHat;
				_vm->_sceneWaiting = 0;
			} else if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatyPussDisguised)) {
					gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					putDownGarbageCan(0);
				}
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSCowboyHat].x, _vm->_hotspotsWalkPos[kHSCowboyHat].y, 3, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(3, 2);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSCowboyHat].x, _vm->_hotspotsWalkPos[kHSCowboyHat].y, 0, _vm->getGnapSequenceId(gskPullOutDeviceNonWorking, 3, 2) | 0x10000, 1);
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSGarbageCan:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFUnk14)) {
					if (_vm->_grabCursorSpriteIndex >= 0)
						_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSGarbageCan].x, _vm->_hotspotsWalkPos[kHSGarbageCan].y, 1, 5);
					else
						_vm->playGnapImpossible(0, 0);
				} else {
					if (_vm->isFlag(kGFPlatypusTalkingToAssistant))
						platEndPhoning(true);
					if (_vm->_grabCursorSpriteIndex >= 0) {
						if (!_vm->isFlag(kGFTruckKeysUsed))
							_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSGarbageCan].x - (_vm->_gnapX < _vm->_s18_garbageCanPos ? 1 : -1),
								_vm->_hotspotsWalkPos[kHSGarbageCan].y, _vm->_hotspotsWalkPos[kHSGarbageCan].x, _vm->_hotspotsWalkPos[kHSGarbageCan].y);
						else
							_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSGarbageCan].x, _vm->_hotspotsWalkPos[kHSGarbageCan].y, 2, 4);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							if (!_vm->isFlag(kGFTruckKeysUsed))
								_vm->playGnapScratchingHead(_vm->_hotspotsWalkPos[kHSGarbageCan].x - (_vm->_gnapX < _vm->_s18_garbageCanPos ? 1 : -1), _vm->_hotspotsWalkPos[kHSGarbageCan].y);
							else if (!_vm->isFlag(kGFTruckFilledWithGas))
								_vm->playGnapScratchingHead(2, 4);
							break;
						case GRAB_CURSOR:
							if (!_vm->isFlag(kGFTruckKeysUsed)) {
								_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSGarbageCan].x - (_vm->_gnapX < _vm->_s18_garbageCanPos ? 1 : -1), _vm->_hotspotsWalkPos[kHSGarbageCan].y,
									-1, -1, 1);
								_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, _vm->_s18_garbageCanPos, _vm->_gnapY) | 0x10000, 1);
								_vm->_gnapActionStatus = kASGrabGarbageCanFromStreet;
							} else if (!_vm->isFlag(kGFTruckFilledWithGas)) {
								if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSGarbageCan].x, _vm->_hotspotsWalkPos[kHSGarbageCan].y, 0, -1, 1))
									_vm->_gnapActionStatus = kASGrabGarbageCanFromHydrant;
							}
							break;
						case TALK_CURSOR:
						case PLAT_CURSOR:
							_vm->playGnapImpossible(0, 0);
							break;
						}
					}
				}
			}
			break;

		case kHSHydrantTopValve:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatyPussDisguised)) {
					// While carrying garbage can
					if (_vm->_grabCursorSpriteIndex >= 0) {
						gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
						putDownGarbageCan(0);
						_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 0, 0);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
							putDownGarbageCan(0);
							_vm->playGnapScratchingHead(0, 0);
							break;
						case GRAB_CURSOR:
							if (_vm->isFlag(kGFTruckFilledWithGas)) {
								gnapCarryGarbageCanTo(-1, -1, 0, -1, 2);
								_vm->_gnapActionStatus = kASPutGarbageCanOnRunningHydrant;
							} else if (!_vm->isFlag(kGFBarnPadlockOpen)) {
								gnapCarryGarbageCanTo(-1, -1, 0, -1, 2);
								_vm->_gnapActionStatus = kASPutGarbageCanOnHydrant;
							} else {
								gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
								putDownGarbageCan(0);
								_vm->playGnapImpossible(0, 0);
							}
							break;
						case TALK_CURSOR:
						case PLAT_CURSOR:
							gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
							putDownGarbageCan(0);
							_vm->playGnapImpossible(0, 0);
							break;
						}
					}
				} else {
					if (_vm->_grabCursorSpriteIndex == kItemWrench) {
						_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, 2, 8) | 0x10000, 1);
						_vm->_gnapActionStatus = kASOpenTopValve;
					} else if (_vm->_grabCursorSpriteIndex >= 0) {
						_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSHydrantTopValve].x, _vm->_hotspotsWalkPos[kHSHydrantTopValve].y, 1, 5);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							_vm->playGnapScratchingHead(1, 5);
							break;
						case GRAB_CURSOR:
							if (_vm->isFlag(kGFBarnPadlockOpen)) {
								_vm->_hotspots[kHSWalkArea2]._flags |= SF_WALKABLE;
								_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSHydrantTopValve].x, _vm->_hotspotsWalkPos[kHSHydrantTopValve].y, 0, 0x107BA, 1);
								_vm->_hotspots[kHSWalkArea2]._flags &= ~SF_WALKABLE;
								_vm->_gnapActionStatus = kASCloseTopValve;
							} else
								_vm->playGnapImpossible(0, 0);
							break;
						case TALK_CURSOR:
						case PLAT_CURSOR:
							_vm->playGnapImpossible(0, 0);
							break;
						}
					}
				}
			}
			break;

		case kHSHydrantRightValve:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFUnk14)) {
					if (_vm->_grabCursorSpriteIndex == -1) {
						_vm->playGnapImpossible(0, 0);
					} else {
						_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSHydrantRightValve].x, _vm->_hotspotsWalkPos[kHSHydrantRightValve].y, 1, 5);
					}
				} else {
					if (_vm->isFlag(kGFPlatyPussDisguised)) {
						gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
						putDownGarbageCan(0);
					}
					if (_vm->_grabCursorSpriteIndex == kItemWrench) {
						_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, 2, 8) | 0x10000, 1);
						if (_vm->isFlag(kGFTruckKeysUsed))
							_vm->_gnapActionStatus = kASOpenRightValveWithGarbageCan;
						else
							_vm->_gnapActionStatus = kASOpenRightValveNoGarbageCan;
					} else if (_vm->_grabCursorSpriteIndex >= 0) {
						_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSHydrantRightValve].x, _vm->_hotspotsWalkPos[kHSHydrantRightValve].y, 1, 5);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							_vm->playGnapScratchingHead(1, 5);
							break;
						case GRAB_CURSOR:
							if (_vm->isFlag(kGFTruckFilledWithGas)) {
								_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSHydrantRightValve].x, _vm->_hotspotsWalkPos[kHSHydrantRightValve].y, 0, 0x107BA, 1);
								if (_vm->isFlag(kGFTruckKeysUsed))
									_vm->_gnapActionStatus = kASCloseRightValveWithGarbageCan;
								else
									_vm->_gnapActionStatus = kASCloseRightValveNoGarbageCan;
							}
							break;
						case TALK_CURSOR:
						case PLAT_CURSOR:
							_vm->playGnapImpossible(0, 0);
							break;
						}
					}
				}
			}
			break;

		case kHSExitToyStore:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatyPussDisguised)) {
					gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					putDownGarbageCan(0);
				}
				if (_vm->isFlag(kGFPictureTaken)) {
					_vm->playGnapImpossible(0, 0);
				} else {
					_vm->_isLeavingScene = true;
					_vm->_newSceneNum = 19;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitToyStore].x, _vm->_hotspotsWalkPos[kHSExitToyStore].y, 0, 0x107C0, 1);
					_vm->_gnapActionStatus = kASLeaveScene;
					if (!_vm->isFlag(kGFPlatypusTalkingToAssistant))
						_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitToyStore].x + 1, _vm->_hotspotsWalkPos[kHSExitToyStore].y, -1, 0x107C2, 1);
				}
			}
			break;

		case kHSExitPhoneBooth:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatyPussDisguised)) {
					gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					putDownGarbageCan(0);
				}
				closeHydrantValve();
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 17;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitPhoneBooth].x, _vm->_hotspotsWalkPos[kHSExitPhoneBooth].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				if (_vm->isFlag(kGFPlatypusTalkingToAssistant))
					_vm->setFlag(kGFUnk27);
				else
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitPhoneBooth].x + 1, _vm->_hotspotsWalkPos[kHSExitPhoneBooth].y, -1, 0x107C2, 1);
			}
			break;

		case kHSExitGrubCity:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatyPussDisguised)) {
					gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					putDownGarbageCan(0);
				}
				closeHydrantValve();
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 20;
				_vm->_hotspots[kHSWalkArea2]._flags |= SF_WALKABLE;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitGrubCity].x, _vm->_hotspotsWalkPos[kHSExitGrubCity].y, 0, 0x107B2, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				if (_vm->isFlag(kGFPlatypusTalkingToAssistant))
					platEndPhoning(false);
				else
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitGrubCity].x, _vm->_hotspotsWalkPos[kHSExitGrubCity].y - 1, -1, 0x107CF, 1);
				_vm->_hotspots[kHSWalkArea2]._flags &= ~SF_WALKABLE;
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatyPussDisguised)) {
					gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					putDownGarbageCan(0);
				} else {
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				}
				_vm->_mouseClickState._left = false;
			}
			break;

		default:
			if (_vm->_gnapActionStatus != kASStandingOnHydrant && _vm->_mouseClickState._left) {
				if (_vm->isFlag(kGFPlatyPussDisguised)) {
					gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					putDownGarbageCan(0);
				} else {
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				}
				_vm->_mouseClickState._left = false;
			}
			break;
		}
	
		updateAnimations();
	
		if (!_vm->isSoundPlaying(0x10940))
			_vm->playSound(0x10940, true);
	
		if ((_vm->isFlag(kGFTruckFilledWithGas) || _vm->isFlag(kGFBarnPadlockOpen)) && !_vm->isSoundPlaying(0x22B) &&
			_vm->_gnapActionStatus != kASOpenRightValveNoGarbageCanDone && _vm->_gnapActionStatus != kASOpenRightValveNoGarbageCan &&
			_vm->_gnapActionStatus != kASOpenTopValve && _vm->_gnapActionStatus != kASOpenTopValveDone &&
			_vm->_gnapActionStatus != kASOpenRightValveWithGarbageCan && _vm->_gnapActionStatus != kASOpenRightValveWithGarbageCanDone)
			_vm->playSound(0x22B, true);
	
		if (!_vm->_isLeavingScene) {
			if (!_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
				if (_vm->_platypusActionStatus == kASPlatComesHere) {
					if (!_vm->_timers[6]) {
						_vm->_platypusActionStatus = -1;
						_vm->_sceneWaiting = false;
						_vm->initPlatypusPos(-1, 10, kDirNone);
						_vm->platypusWalkTo(3, 9, -1, 0x107C2, 1);
						_vm->clearFlag(kGFPlatypusTalkingToAssistant);
					}
				} else {
					_vm->_hotspots[kHSWalkArea1]._y2 += 48;
					_vm->_hotspots[kHSWalkArea2]._x1 += 75;
					_vm->updatePlatypusIdleSequence();
					_vm->_hotspots[kHSWalkArea2]._x1 -= 75;
					_vm->_hotspots[kHSWalkArea1]._y2 -= 48;
				}
				if (!_vm->_timers[5]) {
					_vm->_timers[5] = _vm->getRandom(100) + 100;
					if (_vm->_gnapActionStatus < 0) {
						if (_vm->getRandom(2) == 1)
							_vm->_gameSys->insertSequence(0x220, 255, 0, 0, kSeqNone, 0, 0, 0);
						else
							_vm->_gameSys->insertSequence(0x221, 255, 0, 0, kSeqNone, 0, 0, 0);
					}
				}
				_vm->playSoundA();
			}
			if (!_vm->isFlag(kGFPlatyPussDisguised))
				_vm->updateGnapIdleSequence();
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
		
		_vm->gameUpdateTick();
	}

	if (_vm->isFlag(kGFGnapControlsToyUFO))
		_vm->deleteSurface(&_s18_cowboyHatSurface);
}

void Scene18::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASGrabGarbageCanFromStreet:
			if (_vm->_gnapIdleFacing != kDirUpRight && _vm->_gnapIdleFacing != kDirBottomRight) {
				_vm->_gameSys->insertSequence(0x1FC, _vm->_gnapId,
					makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
					kSeqSyncWait, 0, 75 * _vm->_gnapX - 675, 0);
				_vm->_gnapSequenceDatNum = 0;
				_vm->_gnapSequenceId = 0x1FC;
			} else {
				_vm->_gameSys->insertSequence(0x1FD, _vm->_gnapId,
					makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
					kSeqSyncWait, 0, 75 * _vm->_gnapX - 525, 0);
				_vm->_gnapSequenceDatNum = 0;
				_vm->_gnapSequenceId = 0x1FD;
			}
			_vm->_gameSys->removeSequence(0x1FA, 19, true);
			_vm->setFlag(kGFPlatyPussDisguised);
			updateHotspots();
			_vm->_gnapActionStatus = -1;
			break;
		case kASGrabGarbageCanFromHydrant:
			_vm->_gameSys->insertSequence(0x1FE, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->removeSequence(0x1F9, 19, true);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x1FE;
			_vm->clearFlag(kGFTruckKeysUsed);
			_vm->setFlag(kGFPlatyPussDisguised);
			updateHotspots();
			_vm->_gnapActionStatus = -1;
			break;
		case kASCloseRightValveNoGarbageCan:
			_vm->_gameSys->insertSequence(0x205, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->removeSequence(0x20D, 39, true);
			_vm->_gameSys->removeSequence(0x212, 39, true);
			_vm->_gameSys->removeSequence(0x211, 39, true);
			_vm->stopSound(0x22B);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x205;
			_vm->clearFlag(kGFTruckFilledWithGas);
			_vm->invAdd(kItemWrench);
			_vm->setGrabCursorSprite(kItemWrench);
			updateHotspots();
			_vm->_gnapActionStatus = -1;
			break;
		case kASOpenTopValve:
			_vm->setFlag(kGFBarnPadlockOpen);
			updateHotspots();
			_vm->playGnapPullOutDevice(2, 7);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->insertSequence(0x20C, 19, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_hotspots[kHSWalkArea2]._flags |= SF_WALKABLE;
			_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSHydrantTopValve].x, _vm->_hotspotsWalkPos[kHSHydrantTopValve].y, 0, 0x107BB, 1);
			_vm->_hotspots[kHSWalkArea2]._flags &= ~SF_WALKABLE;
			_vm->_gnapActionStatus = kASOpenTopValveDone;
			break;
		case kASOpenTopValveDone:
			_vm->setGrabCursorSprite(-1);
			_vm->_gameSys->insertSequence(0x208, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x216, 39, 0, 0, kSeqNone, 21, 0, 0);
			_vm->_gameSys->removeSequence(0x20C, 19, true);
			_vm->_gameSys->setAnimation(0x217, 39, 5);
			_vm->_gameSys->insertSequence(0x217, 39, 0x216, 39, kSeqLoop | kSeqSyncWait, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(5) != 2)
				_vm->gameUpdateTick();
			_vm->playSound(0x22B, true);
			_vm->_gameSys->insertSequence(0x20E, 39, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x208;
			_vm->invRemove(kItemWrench);
			_vm->setGrabCursorSprite(-1);
			_vm->_gnapActionStatus = -1;
			break;
		case kASCloseTopValve:
			_vm->_gameSys->insertSequence(0x206, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->removeSequence(0x20E, 39, true);
			_vm->_gameSys->removeSequence(0x216, 39, true);
			_vm->_gameSys->removeSequence(0x217, 39, true);
			_vm->stopSound(0x22B);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x206;
			_vm->clearFlag(kGFBarnPadlockOpen);
			_vm->invAdd(kItemWrench);
			_vm->setGrabCursorSprite(kItemWrench);
			updateHotspots();
			_vm->_gnapActionStatus = -1;
			break;
		case kASGrabCowboyHat:
			_vm->_gameSys->setAnimation(0x200, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x200, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x200;
			_vm->_gnapActionStatus = kASGrabCowboyHatDone;
			break;
		case kASGrabCowboyHatDone:
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			_s18_cowboyHatSurface = _vm->addFullScreenSprite(0x1D2, 255);
			_vm->_gameSys->setAnimation(0x218, 256, 0);
			_vm->_gameSys->insertSequence(0x218, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(0) != 2)
				_vm->gameUpdateTick();
			_vm->_newSceneNum = 18;
			_vm->invAdd(kItemCowboyHat);
			_vm->invAdd(kItemWrench);
			_vm->setFlag(kGFGnapControlsToyUFO);
			_vm->setFlag(kGFUnk14);
			_vm->clearFlag(kGFTruckFilledWithGas);
			_vm->setFlag(kGFTruckKeysUsed);
			_vm->setFlag(kGFUnk14); // CHECKME - Set 2 times?
			updateHotspots();
			_vm->_gnapActionStatus = kASLeaveScene;
			break;
		case kASLeaveScene:
			_vm->_sceneDone = true;
			_vm->_gnapActionStatus = -1;
			break;
		case kASPutGarbageCanOnRunningHydrant:
			_vm->setFlag(kGFTruckKeysUsed);
			_vm->clearFlag(kGFPlatyPussDisguised);
			_vm->_gameSys->requestRemoveSequence(0x211, 39);
			_vm->_gameSys->requestRemoveSequence(0x212, 39);
			_vm->_gameSys->insertSequence(0x210, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->stopSound(0x22B);
			_vm->_gameSys->setAnimation(0x210, _vm->_gnapId, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x210;
			_vm->_gnapActionStatus = kASPutGarbageCanOnRunningHydrant2;
			break;
		case kASPutGarbageCanOnRunningHydrant2:
			_vm->playSound(0x22B, true);
			_vm->_gameSys->setAnimation(0x1FF, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x1FF, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x1FF;
			_vm->_sceneWaiting = true;
			_vm->_gnapActionStatus = kASStandingOnHydrant;
			break;
		case kASStandingOnHydrant:
			_vm->_gameSys->setAnimation(0x1FF, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x1FF, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			break;
		case kASOpenRightValveNoGarbageCan:
		case kASOpenRightValveWithGarbageCan:
			_vm->setFlag(kGFTruckFilledWithGas);
			updateHotspots();
			_vm->playGnapPullOutDevice(2, 7);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->insertSequence(0x20B, 19, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_hotspots[kHSWalkArea2]._flags |= SF_WALKABLE;
			_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSHydrantRightValve].x, _vm->_hotspotsWalkPos[kHSHydrantRightValve].y, 0, 0x107BA, 1);
			_vm->_hotspots[kHSWalkArea2]._flags &= ~SF_WALKABLE;
			if (_vm->_gnapActionStatus == kASOpenRightValveNoGarbageCan)
				_vm->_gnapActionStatus = kASOpenRightValveNoGarbageCanDone;
			else
				_vm->_gnapActionStatus = kASOpenRightValveWithGarbageCanDone;
			break;
		case kASOpenRightValveWithGarbageCanDone:
			_vm->setGrabCursorSprite(-1);
			_vm->_gameSys->insertSequence(0x207, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x213, 39, 0, 0, kSeqNone, 21, 0, 0);
			_vm->_gameSys->requestRemoveSequence(0x1F9, 19);
			_vm->_gameSys->removeSequence(0x20B, 19, true);
			_vm->_gameSys->setAnimation(0x213, 39, 5);
			_vm->_gameSys->insertSequence(0x214, 39, 0x213, 39, kSeqLoop | kSeqSyncWait, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(5) != 2)
				_vm->gameUpdateTick();
			_vm->playSound(555, true);
			_vm->_gameSys->insertSequence(0x20D, 39, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x207;
			_vm->invRemove(kItemWrench);
			_vm->_gnapActionStatus = -1;
			break;
		case kASOpenRightValveNoGarbageCanDone:
			_vm->setGrabCursorSprite(-1);
			_vm->_gameSys->insertSequence(0x207, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x211, 39, 0, 0, kSeqNone, 21, 0, 0);
			_vm->_gameSys->removeSequence(0x20B, 19, true);
			_vm->_gameSys->setAnimation(0x211, 39, 5);
			_vm->_gameSys->insertSequence(0x212, 39, 0x211, 39, kSeqLoop | kSeqSyncWait, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(5) != 2)
				_vm->gameUpdateTick();
			_vm->playSound(0x22B, true);
			_vm->_gameSys->insertSequence(0x20D, 39, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x207;
			_vm->invRemove(kItemWrench);
			_vm->_gnapActionStatus = -1;
			break;
		case kASCloseRightValveWithGarbageCan:
			_vm->_gameSys->insertSequence(0x205, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->removeSequence(0x20D, 39, true);
			_vm->_gameSys->insertSequence(0x215, 39, 0x214, 39, kSeqSyncWait, 0, 0, 0);
			_vm->stopSound(0x22B);
			_vm->_gameSys->setAnimation(0x1F9, 19, 0);
			_vm->_gameSys->insertSequence(0x1F9, 19, 0x215, 39, kSeqSyncWait, 0, 0, 0);
			_vm->clearFlag(kGFTruckFilledWithGas);
			_vm->invAdd(kItemWrench);
			_vm->setGrabCursorSprite(kItemWrench);
			_vm->_gameSys->insertSequence(0x107B5, _vm->_gnapId, 517, _vm->_gnapId, kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
			updateHotspots();
			_vm->_gnapSequenceDatNum = 1;
			_vm->_gnapSequenceId = 0x7B5;
			_vm->_gnapActionStatus = kASCloseRightValveWithGarbageCanDone;
			break;
		case kASCloseRightValveWithGarbageCanDone:
			_vm->_gnapActionStatus = -1;
			break;
		case kASPutGarbageCanOnHydrant:
			_vm->setFlag(kGFTruckKeysUsed);
			_vm->clearFlag(kGFPlatyPussDisguised);
			_vm->_gameSys->insertSequence(0x20F, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x20F, _vm->_gnapId, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x20F;
			_vm->_gnapActionStatus = kASPutGarbageCanOnHydrantDone;
			break;
		case kASPutGarbageCanOnHydrantDone:
			_vm->_gameSys->insertSequence(0x1F9, 19, 0x20F, _vm->_gnapId, kSeqNone, 0, 0, 0);
			updateHotspots();
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 3);
		++_s18_platPhoneIter;
		if (_s18_platPhoneIter <= 4) {
			++_s18_platPhoneCtr;
			_s18_nextPhoneSequenceId = kScene18SequenceIds[_s18_platPhoneCtr % 5];
			_vm->_gameSys->setAnimation(_s18_nextPhoneSequenceId, 254, 3);
			_vm->_gameSys->insertSequence(_s18_nextPhoneSequenceId, 254, _s18_currPhoneSequenceId, 254, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x21F, 254, 0x21F, 254, kSeqSyncWait, 0, 0, 0);
			_s18_currPhoneSequenceId = _s18_nextPhoneSequenceId;
		} else {
			platEndPhoning(true);
		}
	}
}

} // End of namespace Gnap
