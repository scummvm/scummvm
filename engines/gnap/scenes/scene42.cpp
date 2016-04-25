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
#include "gnap/scenes/scene42.h"

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

Scene42::Scene42(GnapEngine *vm) : Scene(vm) {
	_currBBQVendorSequenceId = -1;
	_nextBBQVendorSequenceId = -1;
}

int Scene42::init() {
	GameSys gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	if (_vm->isFlag(kGFPictureTaken) || (_vm->isFlag(kGFUnk18) && _vm->isFlag(kGFUnk23)))
		return 0x153;
	return 0x152;
}

void Scene42::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHSUfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHSUfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		_vm->setHotspot(kHSUfoHotSauce, 335, 110, 440, 175, SF_DISABLED);
		_vm->setDeviceHotspot(kHSUfoDevice, -1, 534, -1, 599);
		if ((_vm->isFlag(kGFPictureTaken) || _vm->isFlag(kGFUnk18)) && _vm->isFlag(kGFUnk23) && !_vm->isFlag(kGFUnk24))
			_vm->_hotspots[kHSUfoHotSauce]._flags = SF_GRAB_CURSOR;
		_vm->_hotspotsCount = 5;
	} else {
		_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHSExitUfoParty, 150, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHSExitToyStand, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHSExitUfo, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		_vm->setHotspot(kHSBBQVendor, 410, 200, 520, 365, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 8);
		_vm->setHotspot(kHSChickenLeg, 530, 340, 620, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 8, 7);
		_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 445);
		_vm->setHotspot(kHSWalkArea2, 240, 0, 550, 495);
		_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
		_vm->_hotspotsCount = 9;
	}
}

void Scene42::run() {
	GameSys gameSys = *_vm->_gameSys;

	_vm->queueInsertDeviceIcon();

	_currBBQVendorSequenceId = 0x14A;
	_nextBBQVendorSequenceId = -1;

	gameSys.setAnimation(0x14A, 1, 2);
	gameSys.insertSequence(_currBBQVendorSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		if (_vm->_prevSceneNum == 43 && _vm->isFlag(kGFUnk18)) {
			_vm->_toyUfoSequenceId = 0x872;
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_toyUfoX = 317;
			_vm->_toyUfoY = 61;
			_vm->toyUfoSetStatus(kGFJointTaken);
			_vm->setFlag(kGFPictureTaken);
			_vm->_timers[9] = 600;
		} else {
			_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
			_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
			if (_vm->_prevSceneNum == 41)
				_vm->_toyUfoX = 30;
			else
				_vm->_toyUfoX = 770;
			gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		}
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 3);
		_vm->endSceneInit();
		if (_vm->_toyUfoSequenceId == 0x872)
			_vm->setGrabCursorSprite(-1);
	} else if (_vm->_prevSceneNum == 41) {
		_vm->initGnapPos(-1, 8, kDirUpRight);
		_vm->initPlatypusPos(-1, 9, kDirUpLeft);
		_vm->endSceneInit();
		_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
	} else if (_vm->_prevSceneNum == 43) {
		_vm->initGnapPos(11, 8, kDirUpRight);
		_vm->initPlatypusPos(11, 9, kDirUpLeft);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(9, 8, -1, 0x107D2, 1);
	} else {
		_vm->initGnapPos(5, 11, kDirUpRight);
		_vm->initPlatypusPos(6, 11, kDirUpLeft);
		_vm->endSceneInit();
		_vm->gnapWalkTo(5, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
	}

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B))
			_vm->playSound(0x1094B, true);

		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			switch (_vm->_sceneClickedHotspot) {
			case kHSUfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = kASToyUfoLeaveScene;
					_vm->_newSceneNum = 41;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 3);
				}
				break;

			case kHSUfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = kASToyUfoLeaveScene;
					_vm->_newSceneNum = 43;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 3);
				}
				break;

			case kHSUfoHotSauce:
				if (_vm->isFlag(kGFJointTaken)) {
					_vm->_toyUfoActionStatus = kASToyUfoPickUpHotSauce;
					_vm->toyUfoFlyTo(384, 77, 0, 799, 0, 300, 3);
					_vm->_timers[9] = 600;
				} else {
					_vm->_toyUfoActionStatus = kASToyUfoRefresh;
					_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
				}
				break;

			case kHSUfoDevice:
				_vm->runMenu();
				updateHotspots();
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHSDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(20) + 30;
				break;

			case kHSPlatypus:
				if (_vm->_gnapActionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex >= 0) {
						_vm->playGnapImpossible(_vm->_platX, _vm->_platY);
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
							_vm->playGnapImpossible(_vm->_platX, _vm->_platY);
							break;
						}
					}
				}
				break;

			case kHSExitUfoParty:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_gnapX, _vm->_hotspotsWalkPos[kHSExitUfoParty].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_platX, _vm->_hotspotsWalkPos[kHSExitUfoParty].y, -1, 0x107C7, 1);
				_vm->_newSceneNum = 40;
				break;

			case kHSExitToyStand:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitToyStand].x, _vm->_gnapY, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitToyStand].x, _vm->_hotspotsWalkPos[kHSExitToyStand].y, -1, 0x107CF, 1);
				_vm->_newSceneNum = 41;
				break;

			case kHSExitUfo:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitUfo].x, _vm->_gnapY, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitUfo].x, _vm->_hotspotsWalkPos[kHSExitUfo].y, -1, 0x107CD, 1);
				_vm->_newSceneNum = 43;
				break;

			case kHSBBQVendor:
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSBBQVendor].x, _vm->_hotspotsWalkPos[kHSBBQVendor].y, 0, 0x107BB, 1);
					_vm->_gnapActionStatus = kASUseQuarterWithBBQVendor;
					if (_vm->_platY < 9)
						_vm->platypusWalkTo(_vm->_platX, 9, -1, -1, 1);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSBBQVendor].x, _vm->_hotspotsWalkPos[kHSBBQVendor].y, _vm->_hotspotsWalkPos[kHSBBQVendor].x + 1, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_hotspotsWalkPos[kHSBBQVendor].x - 1, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSBBQVendor].x, _vm->_hotspotsWalkPos[kHSBBQVendor].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkBBQVendor;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHSChickenLeg:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSChickenLeg].x, _vm->_hotspotsWalkPos[kHSChickenLeg].y, _vm->_hotspotsWalkPos[kHSChickenLeg].x - 1, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_hotspotsWalkPos[kHSChickenLeg].x - 1, 0);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSChickenLeg].x, _vm->_hotspotsWalkPos[kHSChickenLeg].y, 0, 0x107BC, 1);
						_vm->_gnapActionStatus = kASGrabChickenLeg;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;

			case kHSWalkArea1:
			case kHSWalkArea2:
				if (_vm->_gnapActionStatus < 0)
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				break;

			}
		}

		if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				_vm->_toyUfoActionStatus = kASToyUfoRefresh;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
			} else {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}

		updateAnimations();

		_vm->toyUfoCheckTimer();

		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 30;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextBBQVendorSequenceId == -1) {
					switch (_vm->getRandom(8)) {
					case 0:
						_nextBBQVendorSequenceId = 0x14C;
						break;
					case 1:
					case 2:
						_nextBBQVendorSequenceId = 0x149;
						break;
					case 3:
					case 4:
					case 5:
					case 6:
						_nextBBQVendorSequenceId = 0x14D;
						break;
					case 7:
						_nextBBQVendorSequenceId = 0x14A;
						break;
					}
					if (_nextBBQVendorSequenceId == _currBBQVendorSequenceId && _nextBBQVendorSequenceId != 0x14D)
						_nextBBQVendorSequenceId = -1;
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(20) + 30;
		}

		_vm->gameUpdateTick();
	}
}

void Scene42::updateAnimations() {
	GameSys gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		switch (_vm->_gnapActionStatus) {
		case kASLeaveScene:
			gameSys.setAnimation(0, 0, 0);
			_vm->_gnapActionStatus = -1;
			_vm->_sceneDone = true;
			break;
		case kASTalkBBQVendor:
			gameSys.setAnimation(0, 0, 0);
			_vm->_gnapActionStatus = -1;
			_nextBBQVendorSequenceId = 0x14B;
			break;
		case kASUseQuarterWithBBQVendor:
		case kASGrabChickenLeg:
			if (gameSys.getAnimationStatus(2) == 2) {
				int sequenceId;
				if (_vm->_gnapActionStatus == kASUseQuarterWithBBQVendor) {
					_vm->invRemove(kItemDiceQuarterHole);
					_vm->invAdd(kItemChickenBucket);
					_vm->setGrabCursorSprite(-1);
					sequenceId = 0x150;
					_nextBBQVendorSequenceId = 0x148;
				} else if (_vm->isFlag(kGFUnk27)) {
					if (_vm->isFlag(kGFUnk28)) {
						sequenceId = 0x7B7;
						_nextBBQVendorSequenceId = 0x145;
					} else {
						_vm->setFlag(kGFUnk28);
						sequenceId = 0x14F;
						_nextBBQVendorSequenceId = 0x147;
					}
				} else {
					_vm->setFlag(kGFUnk27);
					sequenceId = 0x14E;
					_nextBBQVendorSequenceId = 0x146;
				}
				if (sequenceId == 0x7B7) {
					gameSys.insertSequence(0x107B7, _vm->_gnapId,
						makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
						kSeqSyncWait, _vm->getSequenceTotalDuration(_nextBBQVendorSequenceId),
						75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
					_vm->_gnapSequenceDatNum = 1;
				} else {
					gameSys.insertSequence(sequenceId, _vm->_gnapId,
						makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
						kSeqSyncWait, 0, 0, 0);
					_vm->_gnapSequenceDatNum = 0;
				}
				_vm->_gnapSequenceId = sequenceId;
				gameSys.setAnimation(sequenceId | (_vm->_gnapSequenceDatNum << 16), _vm->_gnapId, 0);
				if (_vm->_gnapActionStatus == kASUseQuarterWithBBQVendor)
					_vm->_gnapActionStatus = kASUseQuarterWithBBQVendorDone;
				else
					_vm->_gnapActionStatus = -1;
				gameSys.insertSequence(_nextBBQVendorSequenceId, 1, _currBBQVendorSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextBBQVendorSequenceId, 1, 2);
				_currBBQVendorSequenceId = _nextBBQVendorSequenceId;
				if (_nextBBQVendorSequenceId == 0x145)
					_nextBBQVendorSequenceId = 0x14A;
				else
					_nextBBQVendorSequenceId = -1;
				_vm->_timers[4] = _vm->getRandom(20) + 30;
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
			}
			break;
		case kASUseQuarterWithBBQVendorDone:
			gameSys.setAnimation(0, 0, 0);
			_vm->setGrabCursorSprite(kItemChickenBucket);
			_vm->_gnapActionStatus = -1;
			break;
		default:
			gameSys.setAnimation(0, 0, 0);
			_vm->_gnapActionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2 && _nextBBQVendorSequenceId != -1) {
		gameSys.insertSequence(_nextBBQVendorSequenceId, 1, _currBBQVendorSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextBBQVendorSequenceId, 1, 2);
		_currBBQVendorSequenceId = _nextBBQVendorSequenceId;
		_nextBBQVendorSequenceId = -1;
		_vm->_timers[4] = _vm->getRandom(20) + 30;
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case kASToyUfoLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kASToyUfoPickUpHotSauce:
			gameSys.insertSequence(0x10870, _vm->_toyUfoId, _vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, kSeqSyncWait, 0, 0, 0);
			_vm->setFlag(kGFUnk24);
			updateHotspots();
			_vm->toyUfoSetStatus(kGFGroceryStoreHatTaken);
			_vm->_toyUfoSequenceId = 0x870;
			gameSys.setAnimation(0x10870, _vm->_toyUfoId, 3);
			_vm->_toyUfoActionStatus = -1;
			_vm->_toyUfoX = 0x181;
			_vm->_toyUfoY = 53;
			break;
		default:
			if (_vm->_toyUfoSequenceId == 0x872) {
				_vm->hideCursor();
				_vm->addFullScreenSprite(0x13E, 255);
				gameSys.setAnimation(0x151, 256, 0);
				gameSys.insertSequence(0x151, 256, 0, 0, kSeqNone, 0, 0, 0);
				while (gameSys.getAnimationStatus(0) != 2)
					_vm->gameUpdateTick();
				_vm->removeFullScreenSprite();
				_vm->showCursor();
			}
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, (_vm->_toyUfoId + 1) % 10, 3);
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, (_vm->_toyUfoId + 1) % 10,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			_vm->_toyUfoId = (_vm->_toyUfoId + 1) % 10;
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}
}

} // End of namespace Gnap
