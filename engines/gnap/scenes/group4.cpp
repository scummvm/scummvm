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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"
#include "gnap/scenes/group4.h"

namespace Gnap {

Scene40::Scene40(GnapEngine *vm) : Scene(vm) {
}

int Scene40::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	return _vm->isFlag(kGFUnk23) ? 0x01 : 0x00;
}

void Scene40::updateHotspots() {
	_vm->setHotspot(kHS40Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_DISABLED | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS40ExitCave, 169, 510, 264, 600, SF_EXIT_D_CURSOR, 0, 8);
	_vm->setHotspot(kHS40ExitToyStand, 238, 297, 328, 376, SF_EXIT_L_CURSOR, 0, 8);
	_vm->setHotspot(kHS40ExitBBQ, 328, 220, 401, 306, SF_EXIT_L_CURSOR, 0, 8);
	_vm->setHotspot(kHS40ExitUfo, 421, 215, 501, 282, SF_EXIT_U_CURSOR, 0, 8);
	_vm->setHotspot(kHS40ExitKissinBooth, 476, 284, 556, 345, SF_EXIT_R_CURSOR, 0, 8);
	_vm->setHotspot(kHS40ExitDancefloor, 317, 455, 445, 600, SF_EXIT_D_CURSOR, 0, 8);
	_vm->setHotspot(kHS40ExitShoe, 455, 346, 549, 417, SF_EXIT_D_CURSOR, 0, 8);
	_vm->setDeviceHotspot(kHS40Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 9;
}

void Scene40::run() {
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->queueInsertDeviceIcon();
	_vm->endSceneInit();

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B))
			_vm->playSound(0x1094B, true);

		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS40Device:
			_vm->runMenu();
			updateHotspots();
			break;

		case kHS40Platypus:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible(plat._pos);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan1(plat._pos);
						break;
					case GRAB_CURSOR:
						gnap.kissPlatypus(0);
						break;
					case TALK_CURSOR:
						gnap.playBrainPulsating(plat._pos);
						plat.playSequence(plat.getSequenceId());
						break;
					case PLAT_CURSOR:
						gnap.playImpossible(plat._pos);
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS40ExitCave:
			if (gnap._actionStatus < 0) {
				_vm->_newSceneNum = 39;
				_vm->_sceneDone = true;
			}
			break;

		case kHS40ExitToyStand:
			if (gnap._actionStatus < 0) {
				_vm->_newSceneNum = 41;
				_vm->_sceneDone = true;
			}
			break;

		case kHS40ExitBBQ:
			if (gnap._actionStatus < 0) {
				_vm->_newSceneNum = 42;
				_vm->_sceneDone = true;
			}
			break;

		case kHS40ExitUfo:
			if (gnap._actionStatus < 0) {
				_vm->_newSceneNum = 43;
				_vm->_sceneDone = true;
			}
			break;

		case kHS40ExitKissinBooth:
			if (gnap._actionStatus < 0) {
				_vm->_newSceneNum = 44;
				_vm->_sceneDone = true;
			}
			break;

		case kHS40ExitDancefloor:
			if (gnap._actionStatus < 0) {
				_vm->_newSceneNum = 45;
				_vm->_sceneDone = true;
			}
			break;

		case kHS40ExitShoe:
			if (gnap._actionStatus < 0) {
				_vm->_newSceneNum = 46;
				_vm->_sceneDone = true;
			}
			break;

		default:
			if (_vm->_mouseClickState._left && gnap._actionStatus < 0)
				_vm->_mouseClickState._left = false;
			break;

		}

		updateAnimations();
		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}
}

void Scene40::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		if (gnap._actionStatus)
			gnap._actionStatus = -1;
		else
			_vm->_sceneDone = true;
	}
}

/*****************************************************************************/

Scene41::Scene41(GnapEngine *vm) : Scene(vm) {
	_currKidSequenceId = -1;
	_nextKidSequenceId = -1;
	_currToyVendorSequenceId = -1;
	_nextToyVendorSequenceId = -1;
}

int Scene41::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	return 0x129;
}

void Scene41::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHS41Platypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHS41UfoExitLeft, 0, 0, 10, 500, SF_EXIT_L_CURSOR | SF_DISABLED);
		_vm->setHotspot(kHS41UfoExitRight, 790, 0, 799, 500, SF_EXIT_R_CURSOR);
		_vm->setHotspot(kHS41UfoWalkArea1, 0, 0, 800, 470, SF_DISABLED);
		_vm->setDeviceHotspot(kHS41UfoDevice, -1, -1, -1, -1);
		_vm->_hotspotsCount = 5;
	} else {
		_vm->setHotspot(kHS41Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS41ExitCave, 150, 590, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHS41Exit, 0, 100, 10, 599, SF_EXIT_L_CURSOR | SF_DISABLED, 0, 8);
		_vm->setHotspot(kHS41ExitBBQ, 790, 100, 799, 599, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 8);
		_vm->setHotspot(kHS41ToyVendor, 320, 150, 430, 310, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS41Kid, 615, 340, 710, 460, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS41ToyUfo, 0, 0, 0, 0, SF_GRAB_CURSOR);
		_vm->setHotspot(kHS41WalkArea1, 0, 0, 800, 470);
		_vm->setDeviceHotspot(kHS41Device, -1, -1, -1, -1);
		_vm->_hotspotsCount = 9;
	}
}

void Scene41::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->queueInsertDeviceIcon();

	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoX = 770;
		if (_vm->_toyUfoY < 0 || _vm->_toyUfoY > 300)
			_vm->_toyUfoY = 150;
		if (!_vm->_timers[9])
			gnap._actionStatus = kAS41GiveBackToyUfo;
	} else {
		if (!_vm->isFlag(kGFUnk16) && !_vm->isFlag(kGFJointTaken) && !_vm->isFlag(kGFUnk18) && !_vm->isFlag(kGFGroceryStoreHatTaken))
			_vm->toyUfoSetStatus(kGFUnk16);
		_vm->_toyUfoX = 600;
		_vm->_toyUfoY = 200;
	}

	_vm->_toyUfoId = 0;
	_vm->_toyUfoActionStatus = -1;
	_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
	_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;

	gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 2);
	gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
	gameSys.insertSequence(0x128, 0, 0, 0, kSeqLoop, 0, 0, 0);

	if (_vm->isFlag(kGFGnapControlsToyUFO))
		_currKidSequenceId = 0x11B;
	else
		_currKidSequenceId = 0x11D;

	_nextKidSequenceId = -1;

	gameSys.setAnimation(_currKidSequenceId, 1, 4);
	gameSys.insertSequence(_currKidSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	_currToyVendorSequenceId = 0x118;
	_nextToyVendorSequenceId = -1;

	gameSys.setAnimation(0x118, 1, 3);
	gameSys.insertSequence(_currToyVendorSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.insertSequence(0x127, 2, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		gnap._sequenceId = 0x120;
		gnap._sequenceDatNum = 0;
		gnap._idleFacing = kDirUpRight;
		gnap._pos = Common::Point(7, 7);
		gnap._id = 140;
		gameSys.insertSequence(0x120, 140, 0, 0, kSeqNone, 0, 0, 0);
		gameSys.setAnimation(makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, 0);
		plat.initPos(8, 10, kDirBottomLeft);
		_vm->endSceneInit();
	} else if (_vm->_prevSceneNum == 45) {
		gnap.initPos(-1, 8, kDirUpRight);
		plat.initPos(-2, 8, kDirUpLeft);
		_vm->endSceneInit();
		plat.walkTo(Common::Point(1, 8), -1, 0x107C2, 1);
		gnap.walkTo(Common::Point(2, 8), -1, 0x107B9, 1);
	} else if (_vm->_prevSceneNum == 42) {
		gnap.initPos(11, 8, kDirUpRight);
		plat.initPos(11, 9, kDirUpLeft);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(8, 8), -1, 0x107BA, 1);
		plat.walkTo(Common::Point(9, 8), -1, 0x107D2, 1);
	} else {
		gnap.initPos(5, 8, kDirBottomRight);
		plat.initPos(6, 8, kDirBottomLeft);
		_vm->endSceneInit();
	}

	_vm->_timers[4] = _vm->getRandom(100) + 100;
	_vm->_timers[5] = _vm->getRandom(30) + 20;

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B))
			_vm->playSound(0x1094B, true);

		if (!_vm->isFlag(kGFGnapControlsToyUFO))
			_vm->_hotspots[kHS41ToyUfo]._rect = Common::Rect(_vm->_toyUfoX - 25, _vm->_toyUfoY - 20, _vm->_toyUfoX + 25, _vm->_toyUfoY + 20);

		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			switch (_vm->_sceneClickedHotspot) {
			case kHS41UfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = kAS41ToyUfoLeaveScene;
					_vm->_newSceneNum = 45;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 2);
				}
				break;

			case kHS41UfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = kAS41ToyUfoLeaveScene;
					_vm->_newSceneNum = 42;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 2);
				}
				break;

			case kHS41UfoDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				break;

			default:
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHS41Device:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				break;

			case kHS41Platypus:
				if (gnap._actionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex >= 0) {
						gnap.playImpossible(plat._pos);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							gnap.playMoan1(plat._pos);
							break;
						case GRAB_CURSOR:
							gnap.kissPlatypus(0);
							break;
						case TALK_CURSOR:
							gnap.playBrainPulsating(plat._pos);
							plat.playSequence(plat.getSequenceId());
							break;
						case PLAT_CURSOR:
							gnap.playImpossible(plat._pos);
							break;
						default:
							break;
						}
					}
				}
				break;

			case kHS41ExitCave:
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[kHS41ExitCave], 0, 0x107AE, 1);
				gnap._actionStatus = kAS41LeaveScene;
				_vm->_newSceneNum = 40;
				break;

			case kHS41Exit:
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS41Exit].x, -1), 0, 0x107AF, 1);
				gnap._actionStatus = kAS41LeaveScene;
				plat.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS41Exit].x, -1), -1, 0x107CF, 1);
				_vm->_newSceneNum = 45;
				break;

			case kHS41ExitBBQ:
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS41ExitBBQ].x, -1), 0, 0x107AB, 1);
				gnap._actionStatus = kAS41LeaveScene;
				plat.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS41ExitBBQ].x, -1), -1, 0x107CD, 1);
				_vm->_newSceneNum = 42;
				break;

			case kHS41ToyVendor:
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					gnap._actionStatus = kAS41UseQuarterWithToyVendor;
					gnap.walkTo(Common::Point(4, 7), 0, 0x107BB, 9);
					gnap.playShowItem(_vm->_grabCursorSpriteIndex, 5, 0);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(Common::Point(4, 7), 5, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan1(Common::Point(5, 0));
						break;
					case GRAB_CURSOR:
						gnap.playImpossible();
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(Common::Point(4, 7), 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS41TalkToyVendor;
						break;
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
				break;

			case kHS41Kid:
				if (_vm->_grabCursorSpriteIndex == kItemChickenBucket) {
					gnap.walkTo(Common::Point(7, 7), 0, 0x107BB, 1);
					gnap._idleFacing = kDirUpRight;
					gnap._actionStatus = kAS41UseChickenBucketWithKid;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(Common::Point(7, 7), 8, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(9, 0));
						break;
					case GRAB_CURSOR:
						gnap.walkTo(Common::Point(7, 7), 0, 0x107BB, 1);
						gnap._idleFacing = kDirUpRight;
						gnap._actionStatus = kAS41GrabKid;
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(Common::Point(7, 7), 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						break;
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
				break;

			case kHS41ToyUfo:
				if (_vm->_grabCursorSpriteIndex == kItemGum) {
					gnap.playPullOutDevice(Common::Point(9, 0));
					gameSys.setAnimation(makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, 0);
					gnap._actionStatus = kAS41UseGumWithToyUfo;
				}
				break;

			case kHS41WalkArea1:
				if (gnap._actionStatus < 0)
					gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				break;

			default:
				break;
			}
		}

		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			if (!_vm->_timers[9] && gnap._actionStatus < 0) {
				gnap._actionStatus = kAS41GiveBackToyUfo;
				if (gnap._sequenceId == 0x121 || gnap._sequenceId == 0x122) {
					gameSys.insertSequence(0x123, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
					gnap._sequenceId = 0x123;
					gnap._sequenceDatNum = 0;
					gameSys.setAnimation(0x123, gnap._id, 0);
				}
			}
		}

		if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				int sequenceId;
				if (_vm->_leftClickMouseX >= 400) {
					if (gnap._sequenceId == 0x11F || gnap._sequenceId == 0x120 || gnap._sequenceId == 0x123 || gnap._sequenceId == 0x126)
						sequenceId = 0x120;
					else if (_vm->_leftClickMouseX - _vm->_toyUfoX >= 400)
						sequenceId = 0x126;
					else
						sequenceId = 0x123;
				} else {
					if (gnap._sequenceId == 0x121 || gnap._sequenceId == 0x125 || gnap._sequenceId == 0x122)
						sequenceId = 0x122;
					else if (_vm->_toyUfoX - _vm->_leftClickMouseX >= 400)
						sequenceId = 0x125;
					else
						sequenceId = 0x121;
				}
				gameSys.insertSequence(sequenceId, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gnap._sequenceId = sequenceId;
				gnap._sequenceDatNum = 0;
				gameSys.setAnimation(sequenceId, gnap._id, 0);
				_vm->_toyUfoActionStatus = kAS41ToyUfoRefresh;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 2);
			} else {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			}
		}

		updateAnimations();

		if (!_vm->_isLeavingScene) {
			if (plat._actionStatus < 0)
				plat.updateIdleSequence();
			if (gnap._actionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _vm->_toyUfoActionStatus == -1 && _nextToyVendorSequenceId == -1) {
					switch (_vm->getRandom(3)) {
					case 0:
						_nextToyVendorSequenceId = 0x113;
						break;
					case 1:
						_nextToyVendorSequenceId = 0x117;
						break;
					case 2:
						_nextToyVendorSequenceId = 0x119;
						break;
					default:
						break;
					}
					if (_nextToyVendorSequenceId == _currToyVendorSequenceId)
						_nextToyVendorSequenceId = -1;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _vm->_toyUfoActionStatus == -1 && _nextKidSequenceId == -1) {
					if (_vm->isFlag(kGFGnapControlsToyUFO))
						_nextKidSequenceId = 0x11B;
					else if (_vm->getRandom(3) != 0)
						_nextKidSequenceId = 0x11D;
					else
						_nextKidSequenceId = 0x11E;
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(100) + 100;
			_vm->_timers[5] = _vm->getRandom(30) + 20;
		}
		_vm->gameUpdateTick();
	}
}

void Scene41::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		switch (gnap._actionStatus) {
		case kAS41LeaveScene:
			gameSys.setAnimation(0, 0, 0);
			_vm->_sceneDone = true;
			gnap._actionStatus = -1;
			break;
		case kAS41UseQuarterWithToyVendor:
			gameSys.setAnimation(0, 0, 0);
			_nextToyVendorSequenceId = 0x114;
			gnap._actionStatus = -1;
			break;
		case kAS41TalkToyVendor:
			gameSys.setAnimation(0, 0, 0);
			_nextToyVendorSequenceId = 0x116;
			gnap._actionStatus = -1;
			break;
		case kAS41UseGumWithToyUfo:
			gameSys.setAnimation(0, 0, 0);
			gnap.playUseDevice(Common::Point(9, 0));
			gnap._actionStatus = -1;
			_vm->setGrabCursorSprite(-1);
			_vm->invRemove(kItemGum);
			_vm->_toyUfoActionStatus = kAS41UfoGumAttached;
			break;
		case kAS41UseChickenBucketWithKid:
			if (gameSys.getAnimationStatus(4) == 2) {
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				_vm->setGrabCursorSprite(-1);
				gameSys.insertSequence(0x11F, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gnap._sequenceId = 0x11F;
				gnap._sequenceDatNum = 0;
				gameSys.setAnimation(0x11F, gnap._id, 0);
				_nextKidSequenceId = 0x11A;
				gameSys.insertSequence(0x11A, 1, _currKidSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextKidSequenceId, 1, 4);
				_currKidSequenceId = _nextKidSequenceId;
				_nextKidSequenceId = 0x11B;
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				gnap._actionStatus = -1;
				_vm->setFlag(kGFGnapControlsToyUFO);
				updateHotspots();
				_vm->_timers[9] = 600;
			}
			break;
		case kAS41GrabKid:
			if (gameSys.getAnimationStatus(3) == 2 && gameSys.getAnimationStatus(4) == 2) {
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				gameSys.insertSequence(0x110, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gnap._sequenceId = 0x110;
				gnap._sequenceDatNum = 0;
				gameSys.setAnimation(0x110, gnap._id, 0);
				_nextToyVendorSequenceId = 0x111;
				gameSys.insertSequence(0x111, 1, _currToyVendorSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextToyVendorSequenceId, 1, 3);
				_currToyVendorSequenceId = _nextToyVendorSequenceId;
				_nextToyVendorSequenceId = -1;
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				_nextKidSequenceId = 0x10F;
				gameSys.insertSequence(0x10F, 1, _currKidSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextKidSequenceId, 1, 4);
				_currKidSequenceId = _nextKidSequenceId;
				_nextKidSequenceId = -1;
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				gnap._actionStatus = -1;
			}
			break;
		case kAS41GiveBackToyUfo:
			if (gameSys.getAnimationStatus(3) == 2 && gameSys.getAnimationStatus(4) == 2) {
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				gameSys.insertSequence(0x124, gnap._id,
					makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
					kSeqSyncWait, 0, 0, 0);
				gnap._sequenceId = 0x124;
				gnap._sequenceDatNum = 0;
				gameSys.setAnimation(0x124, gnap._id, 0);
				_nextToyVendorSequenceId = 0x112;
				gameSys.insertSequence(0x112, 1, _currToyVendorSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextToyVendorSequenceId, 1, 3);
				_currToyVendorSequenceId = _nextToyVendorSequenceId;
				_nextToyVendorSequenceId = -1;
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				_nextKidSequenceId = 0x11C;
				gameSys.insertSequence(0x11C, 1, _currKidSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextKidSequenceId, 1, 4);
				_currKidSequenceId = _nextKidSequenceId;
				_nextKidSequenceId = -1;
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				gnap._actionStatus = -1;
				_vm->clearFlag(kGFGnapControlsToyUFO);
				updateHotspots();
			}
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case kAS41ToyUfoLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS41UfoGumAttached:
			_vm->_toyUfoNextSequenceId = 0x873;
			gameSys.insertSequence(0x10873, _vm->_toyUfoId, _vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 365, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 2);
			_vm->toyUfoSetStatus(kGFJointTaken);
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1, _vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 2);
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}

	if (gameSys.getAnimationStatus(3) == 2 && _nextToyVendorSequenceId != -1) {
		gameSys.insertSequence(_nextToyVendorSequenceId, 1, _currToyVendorSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextToyVendorSequenceId, 1, 3);
		_currToyVendorSequenceId = _nextToyVendorSequenceId;
		_nextToyVendorSequenceId = -1;
		_vm->_timers[4] = _vm->getRandom(100) + 100;
	}

	if (gameSys.getAnimationStatus(4) == 2 && _nextKidSequenceId != -1) {
		gameSys.insertSequence(_nextKidSequenceId, 1, _currKidSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextKidSequenceId, 1, 4);
		_currKidSequenceId = _nextKidSequenceId;
		_nextKidSequenceId = -1;
		_vm->_timers[5] = _vm->getRandom(30) + 20;
		if (_currKidSequenceId == 0x11E) {
			_vm->_toyUfoActionStatus = kAS41ToyUfoRefresh;
			_vm->toyUfoFlyTo(_vm->getRandom(300) + 500, _vm->getRandom(225) + 75, 0, 799, 0, 300, 2);
		}
	}
}

/*****************************************************************************/

Scene42::Scene42(GnapEngine *vm) : Scene(vm) {
	_currBBQVendorSequenceId = -1;
	_nextBBQVendorSequenceId = -1;
}

int Scene42::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	if (_vm->isFlag(kGFPictureTaken) || (_vm->isFlag(kGFUnk18) && _vm->isFlag(kGFUnk23)))
		return 0x153;
	return 0x152;
}

void Scene42::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHS42Platypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHS42UfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHS42UfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		_vm->setHotspot(kHS42UfoHotSauce, 335, 110, 440, 175, SF_DISABLED);
		_vm->setDeviceHotspot(kHS42UfoDevice, -1, 534, -1, 599);
		if ((_vm->isFlag(kGFPictureTaken) || _vm->isFlag(kGFUnk18)) && _vm->isFlag(kGFUnk23) && !_vm->isFlag(kGFUnk24))
			_vm->_hotspots[kHS42UfoHotSauce]._flags = SF_GRAB_CURSOR;
		_vm->_hotspotsCount = 5;
	} else {
		_vm->setHotspot(kHS42Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS42ExitUfoParty, 150, 585, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHS42ExitToyStand, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHS42ExitUfo, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		_vm->setHotspot(kHS42BBQVendor, 410, 200, 520, 365, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 8);
		_vm->setHotspot(kHS42ChickenLeg, 530, 340, 620, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 8, 7);
		_vm->setHotspot(kHS42WalkArea1, 0, 0, 800, 445);
		_vm->setHotspot(kHS42WalkArea2, 240, 0, 550, 495);
		_vm->setDeviceHotspot(kHS42Device, -1, -1, -1, -1);
		_vm->_hotspotsCount = 9;
	}
}

void Scene42::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

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
		gnap.initPos(-1, 8, kDirUpRight);
		plat.initPos(-1, 9, kDirUpLeft);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(2, 8), -1, 0x107B9, 1);
		plat.walkTo(Common::Point(1, 8), -1, 0x107C2, 1);
	} else if (_vm->_prevSceneNum == 43) {
		gnap.initPos(11, 8, kDirUpRight);
		plat.initPos(11, 9, kDirUpLeft);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(8, 8), -1, 0x107BA, 1);
		plat.walkTo(Common::Point(9, 8), -1, 0x107D2, 1);
	} else {
		gnap.initPos(5, 11, kDirUpRight);
		plat.initPos(6, 11, kDirUpLeft);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(5, 8), -1, 0x107BA, 1);
		plat.walkTo(Common::Point(6, 8), -1, 0x107C2, 1);
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
			case kHS42UfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = kAS42ToyUfoLeaveScene;
					_vm->_newSceneNum = 41;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 3);
				}
				break;

			case kHS42UfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = kAS42ToyUfoLeaveScene;
					_vm->_newSceneNum = 43;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 3);
				}
				break;

			case kHS42UfoHotSauce:
				if (_vm->isFlag(kGFJointTaken)) {
					_vm->_toyUfoActionStatus = kAS42ToyUfoPickUpHotSauce;
					_vm->toyUfoFlyTo(384, 77, 0, 799, 0, 300, 3);
					_vm->_timers[9] = 600;
				} else {
					_vm->_toyUfoActionStatus = kAS42ToyUfoRefresh;
					_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
				}
				break;

			case kHS42UfoDevice:
				_vm->runMenu();
				updateHotspots();
				break;

			default:
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHS42Device:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(20) + 30;
				break;

			case kHS42Platypus:
				if (gnap._actionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex >= 0) {
						gnap.playImpossible(plat._pos);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							gnap.playMoan1(plat._pos);
							break;
						case GRAB_CURSOR:
							gnap.kissPlatypus(0);
							break;
						case TALK_CURSOR:
							gnap.playBrainPulsating(plat._pos);
							plat.playSequence(plat.getSequenceId());
							break;
						case PLAT_CURSOR:
							gnap.playImpossible(plat._pos);
							break;
						default:
							break;
						}
					}
				}
				break;

			case kHS42ExitUfoParty:
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(gnap._pos.x, _vm->_hotspotsWalkPos[kHS42ExitUfoParty].y), 0, 0x107AE, 1);
				gnap._actionStatus = kAS42LeaveScene;
				plat.walkTo(Common::Point(plat._pos.x, _vm->_hotspotsWalkPos[kHS42ExitUfoParty].y), -1, 0x107C7, 1);
				_vm->_newSceneNum = 40;
				break;

			case kHS42ExitToyStand:
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS42ExitToyStand].x, gnap._pos.y), 0, 0x107AF, 1);
				gnap._actionStatus = kAS42LeaveScene;
				plat.walkTo(_vm->_hotspotsWalkPos[kHS42ExitToyStand], -1, 0x107CF, 1);
				_vm->_newSceneNum = 41;
				break;

			case kHS42ExitUfo:
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS42ExitUfo].x, gnap._pos.y), 0, 0x107AB, 1);
				gnap._actionStatus = kAS42LeaveScene;
				plat.walkTo(_vm->_hotspotsWalkPos[kHS42ExitUfo], -1, 0x107CD, 1);
				_vm->_newSceneNum = 43;
				break;

			case kHS42BBQVendor:
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					gnap.walkTo(_vm->_hotspotsWalkPos[kHS42BBQVendor], 0, 0x107BB, 1);
					gnap._actionStatus = kAS42UseQuarterWithBBQVendor;
					if (plat._pos.y < 9)
						plat.walkTo(Common::Point(plat._pos.x, 9), -1, -1, 1);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS42BBQVendor], _vm->_hotspotsWalkPos[kHS42BBQVendor].x + 1, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(_vm->_hotspotsWalkPos[kHS42BBQVendor].x - 1, 0));
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[kHS42BBQVendor], 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS42TalkBBQVendor;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
				break;

			case kHS42ChickenLeg:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS42ChickenLeg], _vm->_hotspotsWalkPos[kHS42ChickenLeg].x - 1, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(_vm->_hotspotsWalkPos[kHS42ChickenLeg].x - 1, 0));
						break;
					case GRAB_CURSOR:
						gnap.walkTo(_vm->_hotspotsWalkPos[kHS42ChickenLeg], 0, 0x107BC, 1);
						gnap._actionStatus = kAS42GrabChickenLeg;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
				break;

			case kHS42WalkArea1:
			case kHS42WalkArea2:
				if (gnap._actionStatus < 0)
					gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				break;

			default:
				break;
			}
		}

		if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				_vm->_toyUfoActionStatus = kAS42ToyUfoRefresh;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
			} else {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			}
		}

		updateAnimations();

		_vm->toyUfoCheckTimer();

		if (!_vm->_isLeavingScene) {
			if (plat._actionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				plat.updateIdleSequence();
			if (gnap._actionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 30;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _nextBBQVendorSequenceId == -1) {
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
					default:
						break;
					}
					if (_nextBBQVendorSequenceId == _currBBQVendorSequenceId && _nextBBQVendorSequenceId != 0x14D)
						_nextBBQVendorSequenceId = -1;
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(20) + 30;
		}

		_vm->gameUpdateTick();
	}
}

void Scene42::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		switch (gnap._actionStatus) {
		case kAS42LeaveScene:
			gameSys.setAnimation(0, 0, 0);
			gnap._actionStatus = -1;
			_vm->_sceneDone = true;
			break;
		case kAS42TalkBBQVendor:
			gameSys.setAnimation(0, 0, 0);
			gnap._actionStatus = -1;
			_nextBBQVendorSequenceId = 0x14B;
			break;
		case kAS42UseQuarterWithBBQVendor:
		case kAS42GrabChickenLeg:
			if (gameSys.getAnimationStatus(2) == 2) {
				int sequenceId;
				if (gnap._actionStatus == kAS42UseQuarterWithBBQVendor) {
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
					gameSys.insertSequence(0x107B7, gnap._id,
						makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
						kSeqSyncWait, _vm->getSequenceTotalDuration(_nextBBQVendorSequenceId),
						75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
					gnap._sequenceDatNum = 1;
				} else {
					gameSys.insertSequence(sequenceId, gnap._id,
						makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
						kSeqSyncWait, 0, 0, 0);
					gnap._sequenceDatNum = 0;
				}
				gnap._sequenceId = sequenceId;
				gameSys.setAnimation(sequenceId | (gnap._sequenceDatNum << 16), gnap._id, 0);
				if (gnap._actionStatus == kAS42UseQuarterWithBBQVendor)
					gnap._actionStatus = kAS42UseQuarterWithBBQVendorDone;
				else
					gnap._actionStatus = -1;
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
		case kAS42UseQuarterWithBBQVendorDone:
			gameSys.setAnimation(0, 0, 0);
			_vm->setGrabCursorSprite(kItemChickenBucket);
			gnap._actionStatus = -1;
			break;
		default:
			gameSys.setAnimation(0, 0, 0);
			gnap._actionStatus = -1;
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
		case kAS42ToyUfoLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS42ToyUfoPickUpHotSauce:
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
				while (gameSys.getAnimationStatus(0) != 2 && !_vm->_gameDone)
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

/*****************************************************************************/

Scene43::Scene43(GnapEngine *vm) : Scene(vm) {
	_currTwoHeadedGuySequenceId = -1;
	_nextTwoHeadedGuySequenceId = -1;
}

int Scene43::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	return 0x13F;
}

void Scene43::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHS43Platypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHS43UfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHS43UfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		_vm->setHotspot(kHS43UfoKey, 140, 170, 185, 260, SF_GRAB_CURSOR);
		_vm->setHotspot(kHS43UfoBucket, 475, 290, 545, 365, SF_DISABLED);
		_vm->setDeviceHotspot(kHS43UfoDevice, -1, 534, -1, 599);
		if (_vm->isFlag(kGFGroceryStoreHatTaken))
			_vm->_hotspots[kHS43UfoBucket]._flags = SF_GRAB_CURSOR;
		// NOTE Bug in the original. Key hotspot wasn't disabled.
		if (_vm->isFlag(kGFUnk14))
			_vm->_hotspots[kHS43UfoKey]._flags = SF_DISABLED;
		_vm->_hotspotsCount = 6;
	} else {
		_vm->setHotspot(kHS43Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS43ExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHS43ExitBBQ, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHS43ExitKissinBooth, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		_vm->setHotspot(kHS43TwoHeadedGuy, 470, 240, 700, 470, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS43Key, 140, 170, 185, 260, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS43Ufo, 110, 0, 690, 350, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS43WalkArea1, 0, 0, 800, 445);
		_vm->setHotspot(kHS43WalkArea2, 465, 0, 800, 493);
		_vm->setDeviceHotspot(kHS43Device, -1, -1, -1, -1);
		if (_vm->isFlag(kGFUnk14))
			_vm->_hotspots[kHS43Key]._flags = SF_DISABLED;
		_vm->_hotspotsCount = 10;
	}
}

void Scene43::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->queueInsertDeviceIcon();

	if (!_vm->isFlag(kGFUnk14))
		gameSys.insertSequence(0x1086F, 1, 0, 0, kSeqNone, 0, 0, 0);

	_currTwoHeadedGuySequenceId = 0x13C;
	_nextTwoHeadedGuySequenceId = -1;

	gameSys.setAnimation(0x13C, 1, 2);
	gameSys.insertSequence(_currTwoHeadedGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
		_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
		if (_vm->_prevSceneNum == 42)
			_vm->_toyUfoX = 30;
		else
			_vm->_toyUfoX = 770;
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 3);
		gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		_vm->endSceneInit();
	} else {
		switch (_vm->_prevSceneNum) {
		case 42:
			gnap.initPos(-1, 8, kDirUpRight);
			plat.initPos(-1, 9, kDirUpLeft);
			_vm->endSceneInit();
			gnap.walkTo(Common::Point(2, 8), -1, 0x107B9, 1);
			plat.walkTo(Common::Point(1, 8), -1, 0x107C2, 1);
			break;
		case 44:
			gnap.initPos(11, 8, kDirUpRight);
			plat.initPos(11, 9, kDirUpLeft);
			_vm->endSceneInit();
			gnap.walkTo(Common::Point(8, 8), -1, 0x107BA, 1);
			plat.walkTo(Common::Point(9, 8), -1, 0x107D2, 1);
			break;
		case 54:
			gnap.initPos(4, 7, kDirBottomLeft);
			plat.initPos(11, 8, kDirUpLeft);
			_vm->endSceneInit();
			plat.walkTo(Common::Point(9, 8), -1, 0x107D2, 1);
			break;
		default:
			gnap.initPos(5, 11, kDirUpRight);
			plat.initPos(6, 11, kDirUpLeft);
			_vm->endSceneInit();
			gnap.walkTo(Common::Point(5, 8), -1, 0x107BA, 1);
			plat.walkTo(Common::Point(6, 8), -1, 0x107C2, 1);
			break;
		}
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
			case kHS43UfoDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				break;

			case kHS43UfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 4;
					_vm->_newSceneNum = 42;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 3);
				}
				break;

			case kHS43UfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 4;
					_vm->_newSceneNum = 44;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 3);
				}
				break;

			case kHS43UfoKey:
				if (_vm->isFlag(kGFJointTaken)) {
					_vm->_toyUfoActionStatus = 6;
					_vm->toyUfoFlyTo(163, 145, 0, 799, 0, 300, 3);
				} else {
					_vm->_toyUfoActionStatus = 5;
					_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
				}
				break;

			case kHS43UfoBucket:
				_vm->_toyUfoActionStatus = 7;
				_vm->toyUfoFlyTo(497, 143, 0, 799, 0, 300, 3);
				_vm->_timers[9] = 600;
				break;

			default:
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHS43Device:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				break;

			case kHS43Platypus:
				if (gnap._actionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex >= 0) {
						gnap.playImpossible(plat._pos);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							gnap.playMoan1(plat._pos);
							break;
						case GRAB_CURSOR:
							gnap.kissPlatypus(0);
							break;
						case TALK_CURSOR:
							gnap.playBrainPulsating(plat._pos);
							plat.playSequence(plat.getSequenceId());
							break;
						case PLAT_CURSOR:
							gnap.playImpossible(plat._pos);
							break;
						default:
							break;
						}
					}
				}
				break;

			case kHS43ExitUfoParty:
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[kHS43ExitUfoParty], 0, 0x107AE, 1);
				gnap._actionStatus = 0;
				plat.walkTo(_vm->_hotspotsWalkPos[kHS43ExitUfoParty], -1, 0x107C7, 1);
				_vm->_newSceneNum = 40;
				break;

			case kHS43ExitBBQ:
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS43ExitBBQ].x, gnap._pos.y), 0, 0x107AF, 1);
				gnap._actionStatus = 0;
				plat.walkTo(_vm->_hotspotsWalkPos[kHS43ExitBBQ], -1, 0x107CF, 1);
				_vm->_newSceneNum = 42;
				break;

			case kHS43ExitKissinBooth:
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS43ExitKissinBooth].x, gnap._pos.y), 0, 0x107AB, 1);
				gnap._actionStatus = 0;
				plat.walkTo(_vm->_hotspotsWalkPos[kHS43ExitKissinBooth], -1, 0x107CD, 1);
				_vm->_newSceneNum = 44;
				break;

			case kHS43TwoHeadedGuy:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(Common::Point(6, 8), 7, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(7, 0));
						break;
					case TALK_CURSOR:
							gnap._idleFacing = kDirUpRight;
							gnap.walkTo(Common::Point(5, 8), 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
							gnap._actionStatus = 2;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
				break;

			case kHS43Key:
			case kHS43Ufo:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(Common::Point(3, 7), 2, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead();
						break;
					case GRAB_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(Common::Point(3, 7), 0, 67515, 1);
						gnap._actionStatus = 1;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
				break;

			case kHS43WalkArea1:
			case kHS43WalkArea2:
				if (gnap._actionStatus < 0)
					gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				break;

			default:
				break;
			}
		}

		if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO) && (_vm->_toyUfoActionStatus == 5 || _vm->_toyUfoActionStatus == -1)) {
				_vm->_toyUfoActionStatus = 5;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
			} else {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			}
		}

		updateAnimations();

		_vm->toyUfoCheckTimer();

		if (!_vm->_isLeavingScene) {
			if (plat._actionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				plat.updateIdleSequence();
			if (gnap._actionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				gnap.updateIdleSequence();
			if (!_vm->_timers[4] && (!_vm->isFlag(kGFGnapControlsToyUFO) || !_vm->isFlag(kGFGroceryStoreHatTaken))) {
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _nextTwoHeadedGuySequenceId == -1) {
					switch (_vm->getRandom(5)) {
					case 0:
						_nextTwoHeadedGuySequenceId = 0x13C;
						break;
					case 1:
						_nextTwoHeadedGuySequenceId = 0x134;
						break;
					case 2:
						_nextTwoHeadedGuySequenceId = 0x135;
						break;
					case 3:
						_nextTwoHeadedGuySequenceId = 0x136;
						break;
					case 4:
						_nextTwoHeadedGuySequenceId = 0x13A;
						break;
					default:
						break;
					}
					if (_nextTwoHeadedGuySequenceId == _currTwoHeadedGuySequenceId)
						_nextTwoHeadedGuySequenceId = -1;
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(100) + 100;
		}

		_vm->gameUpdateTick();
	}

	if (_vm->_newSceneNum == 54)
		_vm->clearFlag(kGFGnapControlsToyUFO);
}

void Scene43::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		switch (gnap._actionStatus) {
		case 0:
			gameSys.setAnimation(0, 0, 0);
			_vm->_sceneDone = true;
			break;

		case 1:
			if (gameSys.getAnimationStatus(2) == 2) {
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				gameSys.insertSequence(0x13D, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gnap._sequenceId = 0x13D;
				gnap._sequenceDatNum = 0;
				gameSys.setAnimation(0x13D, gnap._id, 0);
				_nextTwoHeadedGuySequenceId = 0x13B;
				gameSys.insertSequence(0x13B, 1, _currTwoHeadedGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextTwoHeadedGuySequenceId, 1, 2);
				_currTwoHeadedGuySequenceId = _nextTwoHeadedGuySequenceId;
				_nextTwoHeadedGuySequenceId = -1;
				_vm->_timers[4] = _vm->getRandom(100) + 100;
				gnap._actionStatus = -1;
			}
			break;

		default:
			gameSys.setAnimation(0, 0, 0);
			gnap._actionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		if (_currTwoHeadedGuySequenceId == 0x13A) {
			if (_vm->isFlag(kGFGroceryStoreHatTaken)) {
				_nextTwoHeadedGuySequenceId = 0x13E;
				_vm->stopSound(0x108F6);
			} else if (_vm->getRandom(2) != 0) {
				_nextTwoHeadedGuySequenceId = 0x137;
			} else {
				_nextTwoHeadedGuySequenceId = 0x138;
			}
		} else if (_currTwoHeadedGuySequenceId == 0x13E) {
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 54;
		}
		if (_nextTwoHeadedGuySequenceId != -1) {
			gameSys.insertSequence(_nextTwoHeadedGuySequenceId, 1, _currTwoHeadedGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextTwoHeadedGuySequenceId, 1, 2);
			_currTwoHeadedGuySequenceId = _nextTwoHeadedGuySequenceId;
			_nextTwoHeadedGuySequenceId = -1;
			_vm->_timers[4] = _vm->getRandom(100) + 100;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case 4:
			_vm->_sceneDone = true;
			_vm->_toyUfoActionStatus = -1;
			break;
		case 6:
			gameSys.insertSequence(0x10871, _vm->_toyUfoId, _vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, kSeqSyncWait, 0, 0, 0);
			gameSys.removeSequence(0x1086F, 1, true);
			_vm->setFlag(kGFUnk14);
			updateHotspots();
			_vm->toyUfoSetStatus(kGFUnk18);
			_vm->_toyUfoSequenceId = 0x871;
			gameSys.setAnimation(0x10871, _vm->_toyUfoId, 3);
			_vm->_toyUfoActionStatus = -1;
			_vm->_toyUfoX = 96;
			_vm->_toyUfoY = 131;
			break;
		case 7:
			gameSys.insertSequence(0x10874, _vm->_toyUfoId, _vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, kSeqSyncWait, 0, 0, 0);
			_vm->_toyUfoSequenceId = 0x874;
			gameSys.setAnimation(0x10874, _vm->_toyUfoId, 3);
			_vm->_toyUfoActionStatus = 8;
			_vm->setFlag(kGFJointTaken);
			gnap._actionStatus = 3;
			break;
		case 8:
			_nextTwoHeadedGuySequenceId = 0x13A;
			_vm->_toyUfoX = 514;
			_vm->_toyUfoY = 125;
			_vm->toyUfoFlyTo(835, 125, 0, 835, 0, 300, 3);
			_vm->_toyUfoActionStatus = 9;
			break;
		case 9:
			// Nothing
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 3);
			_vm->_toyUfoActionStatus = -1;
			break;
		}
	}
}

/*****************************************************************************/

Scene44::Scene44(GnapEngine *vm) : Scene(vm) {
	_nextSpringGuySequenceId = -1;
	_nextKissingLadySequenceId = -1;
	_currSpringGuySequenceId = -1;
	_currKissingLadySequenceId = -1;
}

int Scene44::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	gameSys.setAnimation(0, 0, 3);
	return 0xFF;
}

void Scene44::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHS44Platypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHS44UfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHS44UfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		_vm->setDeviceHotspot(kHS44UfoDevice, -1, 534, -1, 599);
		_vm->_hotspotsCount = 4;
	} else {
		_vm->setHotspot(kHS44Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS44ExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHS44ExitUfo, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHS44ExitShow, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		_vm->setHotspot(kHS44KissingLady, 300, 160, 400, 315, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
		_vm->setHotspot(kHS44Spring, 580, 310, 635, 375, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
		_vm->setHotspot(kHS44SpringGuy, 610, 375, 690, 515, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
		_vm->setHotspot(kHS44WalkArea1, 0, 0, 800, 445);
		_vm->setHotspot(kHS44WalkArea2, 617, 0, 800, 600);
		_vm->setDeviceHotspot(kHS44Device, -1, -1, -1, -1);
		if (_vm->isFlag(kGFUnk13))
			_vm->_hotspots[kHS44KissingLady]._flags = SF_DISABLED;
		if (_vm->isFlag(kGFSpringTaken))
			_vm->_hotspots[kHS44Spring]._flags = SF_DISABLED;
		_vm->_hotspotsCount = 10;
	}
}

void Scene44::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->queueInsertDeviceIcon();

	gameSys.insertSequence(0xF7, 0, 0, 0, kSeqLoop, 0, 0, 0);
	gameSys.insertSequence(0xFC, 256, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFSpringTaken))
		_currSpringGuySequenceId = 0xF8;
	else
		_currSpringGuySequenceId = 0xF9;

	_nextSpringGuySequenceId = -1;
	gameSys.setAnimation(_currSpringGuySequenceId, 1, 4);
	gameSys.insertSequence(_currSpringGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFUnk13)) {
		if (_vm->_prevSceneNum != 50 || _vm->_sceneSavegameLoaded) {
			_currKissingLadySequenceId = 0xF6;
			_nextKissingLadySequenceId = -1;
		} else {
			_vm->setGrabCursorSprite(kItemGum);
			_currKissingLadySequenceId = 0xF5;
			_nextKissingLadySequenceId = 0xF6;
			gameSys.setAnimation(0xF5, 1, 2);
		}
	} else {
		_currKissingLadySequenceId = 0xEC;
		_nextKissingLadySequenceId = -1;
		gameSys.setAnimation(0xEC, 1, 2);
	}

	gameSys.insertSequence(_currKissingLadySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
		_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
		if (_vm->_prevSceneNum == 43)
			_vm->_toyUfoX = 30;
		else
			_vm->_toyUfoX = 770;
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 3);
		gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		_vm->endSceneInit();
	} else {
		switch (_vm->_prevSceneNum) {
		case 43:
			gnap.initPos(-1, 8, kDirUpRight);
			plat.initPos(-1, 7, kDirUpLeft);
			_vm->endSceneInit();
			gnap.walkTo(Common::Point(2, 8), -1, 0x107B9, 1);
			plat.walkTo(Common::Point(1, 8), -1, 0x107C2, 1);
			break;
		case 46:
			gnap.initPos(11, 8, kDirUpRight);
			plat.initPos(11, 8, kDirUpLeft);
			_vm->endSceneInit();
			gnap.walkTo(Common::Point(6, 8), -1, 0x107BA, 1);
			plat.walkTo(Common::Point(7, 8), -1, 0x107D2, 1);
			break;
		case 50:
			gnap.initPos(4, 8, kDirBottomRight);
			if (_vm->_sceneSavegameLoaded) {
				plat.initPos(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, kDirIdleRight);
			} else if (!_vm->isFlag(kGFUnk13)) {
				_vm->_timers[0] = 50;
				_vm->_timers[1] = 20;
				plat._pos = Common::Point(5, 8);
				plat._sequenceId = 0xFD;
				plat._idleFacing = kDirIdleLeft;
				plat._id = 160;
				plat._sequenceDatNum = 0;
				gameSys.insertSequence(0xFD, 160, 0, 0, kSeqNone, 0, 0, 0);
			}
			_vm->endSceneInit();
			break;
		default:
			gnap.initPos(5, 11, kDirUpRight);
			plat.initPos(6, 11, kDirUpLeft);
			_vm->endSceneInit();
			plat.walkTo(Common::Point(6, 8), -1, 0x107C2, 1);
			gnap.walkTo(Common::Point(5, 8), -1, 0x107BA, 1);
			break;
		}
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
			case kHS44UfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 6;
					_vm->_newSceneNum = 43;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 3);
				}
				break;

			case kHS44UfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 6;
					_vm->_newSceneNum = 46;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 3);
				}
				break;

			case kHS44UfoDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				break;

			default:
				break;
			}
		} else if (_vm->_sceneClickedHotspot <= 9) {
			switch (_vm->_sceneClickedHotspot) {
			case kHS44Device:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				break;

			case kHS44Platypus:
				if (gnap._actionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex >= 0) {
						gnap.playImpossible(plat._pos);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							gnap.playMoan1(plat._pos);
							break;
						case GRAB_CURSOR:
							gnap.kissPlatypus(0);
							break;
						case TALK_CURSOR:
							gnap.playBrainPulsating(plat._pos);
							plat.playSequence(plat.getSequenceId());
							break;
						case PLAT_CURSOR:
							gnap.playImpossible(plat._pos);
							break;
						default:
							break;
						}
					}
				}
				break;

			case kHS44ExitUfoParty:
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[kHS44ExitUfoParty], 0, 0x107AE, 1);
				gnap._actionStatus = 0;
				_vm->_newSceneNum = 40;
				break;

			case kHS44ExitUfo:
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS44ExitUfo].x, gnap._pos.y), 0, 0x107AF, 1);
				gnap._actionStatus = 0;
				plat.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS44ExitUfo].x, plat._pos.y), -1, 0x107CF, 1);
				_vm->_newSceneNum = 43;
				break;

			case kHS44ExitShow:
				_vm->_isLeavingScene = true;
				gnap.walkTo(_vm->_hotspotsWalkPos[kHS44ExitShow], 0, 0x107AB, 1);
				gnap._actionStatus = 0;
				_vm->_newSceneNum = 46;
				break;

			case kHS44KissingLady:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap._actionStatus = 2;
					gnap.walkTo(_vm->_hotspotsWalkPos[kHS44KissingLady], 0, -1, 9);
					gnap.playShowItem(_vm->_grabCursorSpriteIndex, _vm->_hotspotsWalkPos[kHS44KissingLady].x - 1, _vm->_hotspotsWalkPos[kHS44KissingLady].y);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(4, 3));
						break;
					case GRAB_CURSOR:
						gnap.playImpossible();
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirUpLeft;
						gnap.walkTo(_vm->_hotspotsWalkPos[kHS44KissingLady], 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = 1;
						break;
					case PLAT_CURSOR:
						gnap.useDeviceOnPlatypus();
						plat.walkTo(Common::Point(6, 7), 1, 0x107D2, 1);
						if (gnap._pos == Common::Point(7, 7))
							gnap.walkStep();
						gnap.playIdle(Common::Point(5, 7));
						plat._actionStatus = 4;
						break;
					default:
						break;
					}
				}
				break;

			case kHS44Spring:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS44Spring], 8, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(8, 7));
						break;
					case GRAB_CURSOR:
						gnap.playPullOutDevice(Common::Point(8, 0));
						gnap.playUseDevice(Common::Point(8, 0));
						_nextSpringGuySequenceId = 0xFB;
						_vm->invAdd(kItemSpring);
						_vm->setFlag(kGFSpringTaken);
						updateHotspots();
						break;
					case TALK_CURSOR:
						gnap.playImpossible();
						break;
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
				break;

			case kHS44SpringGuy:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS44SpringGuy], 8, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFSpringTaken))
							gnap.playMoan1(Common::Point(8, 7));
						else
							gnap.playScratchingHead(Common::Point(8, 7));
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[kHS44SpringGuy], -1, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
				break;

			case kHS44WalkArea1:
			case kHS44WalkArea2:
				if (gnap._actionStatus < 0)
					gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				break;

			default:
				break;
			}
		}

		if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				_vm->_toyUfoActionStatus = 7;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
			} else {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			}
		}

		updateAnimations();
		_vm->toyUfoCheckTimer();

		if (!_vm->_isLeavingScene) {
			if (plat._actionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO) && _currKissingLadySequenceId != 0xF5)
				plat.updateIdleSequence();
			if (gnap._actionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _nextKissingLadySequenceId == -1) {
					switch (_vm->getRandom(20)) {
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
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(20) + 20;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _nextSpringGuySequenceId == -1) {
					if (_vm->getRandom(5) != 0) {
						if (!_vm->isFlag(kGFSpringTaken))
							_nextSpringGuySequenceId = 0xF9;
					} else {
						if (_vm->isFlag(kGFSpringTaken))
							_nextSpringGuySequenceId = 0xF8;
						else
							_nextSpringGuySequenceId = 0xFA;
					}
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(20) + 20;
		}

		_vm->gameUpdateTick();
	}
}

void Scene44::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	if (gameSys.getAnimationStatus(0) == 2) 	{
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case 0:
			_vm->_sceneDone = true;
			break;
		case 1:
			_nextKissingLadySequenceId = 0xEF;
			break;
		case 2:
			_nextKissingLadySequenceId = 0xF2;
			break;
		default:
			break;
		}
		gnap._actionStatus = -1;
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		switch (plat._actionStatus) {
		case 4:
			if (gameSys.getAnimationStatus(2) == 2) {
				gameSys.insertSequence(0xFE, plat._id, plat._sequenceId | (plat._sequenceDatNum << 16), plat._id, kSeqSyncWait, 0, 0, 0);
				plat._sequenceId = 0xFE;
				plat._sequenceDatNum = 0;
				gameSys.setAnimation(0xFE, plat._id, 1);
				gameSys.removeSequence(_currKissingLadySequenceId, 1, true);
				plat._actionStatus = 5;
			}
			break;
		case 5:
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 50;
			break;
		default:
			plat._actionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		if (_nextKissingLadySequenceId == 0xF6) {
			gameSys.insertSequence(_nextKissingLadySequenceId, 1, _currKissingLadySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			plat.initPos(5, 8, kDirIdleLeft);
			_currKissingLadySequenceId = _nextKissingLadySequenceId;
			_nextKissingLadySequenceId = -1;
			gameSys.setAnimation(0, 0, 2);
		} else if (_nextKissingLadySequenceId != -1) {
			gameSys.insertSequence(_nextKissingLadySequenceId, 1, _currKissingLadySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextKissingLadySequenceId, 1, 2);
			_currKissingLadySequenceId = _nextKissingLadySequenceId;
			_nextKissingLadySequenceId = -1;
			_vm->_timers[4] = _vm->getRandom(20) + 20;
		}
	}

	if (gameSys.getAnimationStatus(4) == 2) {
		if (_currSpringGuySequenceId == 0xFB) {
			_vm->setGrabCursorSprite(kItemSpring);
			_nextSpringGuySequenceId = 0xF8;
		}
		if (_nextSpringGuySequenceId != -1) {
			gameSys.insertSequence(_nextSpringGuySequenceId, 1, _currSpringGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextSpringGuySequenceId, 1, 4);
			_currSpringGuySequenceId = _nextSpringGuySequenceId;
			_nextSpringGuySequenceId = -1;
			_vm->_timers[5] = _vm->getRandom(20) + 20;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case 6:
			_vm->_sceneDone = true;
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 3);
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}
}

/*****************************************************************************/

Scene45::Scene45(GnapEngine *vm) : Scene(vm) {
	_currDancerSequenceId = -1;
}

int Scene45::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	gameSys.setAnimation(0, 0, 3);
	gameSys.setAnimation(0, 0, 4);
	gameSys.setAnimation(0, 0, 5);
	return _vm->isFlag(kGFUnk23) ? 0xA2 : 0xA1;
}

void Scene45::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHS45Platypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHS45UfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHS45UfoExitRight, 794, 0, 799, 599, SF_EXIT_R_CURSOR | SF_DISABLED);
		_vm->setDeviceHotspot(kHS45UfoDevice, -1, 534, -1, 599);
		_vm->_hotspotsCount = 4;
	} else {
		_vm->setHotspot(kHS45Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS45ExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHS45ExitShoe, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHS45ExitRight, 794, 100, 799, 599, SF_EXIT_R_CURSOR | SF_DISABLED, 10, 8);
		_vm->setHotspot(kHS45ExitDiscoBall, 200, 0, 600, 10, SF_DISABLED);
		_vm->setHotspot(kHS45DiscoBall, 370, 10, 470, 125, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 7);
		_vm->setHotspot(kHS45WalkArea1, 0, 0, 800, 472);
		_vm->setDeviceHotspot(kHS45Device, -1, -1, -1, -1);
		if (_vm->isFlag(kGFUnk22)) {
			_vm->_hotspots[kHS45Platypus]._flags = SF_DISABLED;
			_vm->_hotspots[kHS45ExitUfoParty]._flags = SF_DISABLED;
			_vm->_hotspots[kHS45ExitShoe]._flags = SF_DISABLED;
			_vm->_hotspots[kHS45ExitRight]._flags = SF_DISABLED;
			_vm->_hotspots[kHS45ExitDiscoBall]._flags = SF_EXIT_U_CURSOR;
		}
		if (_vm->isFlag(kGFUnk23) || _vm->isFlag(kGFUnk22))
			_vm->_hotspots[kHS45DiscoBall]._flags = SF_DISABLED;
		_vm->_hotspotsCount = 8;
	}
}

void Scene45::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	if (!_vm->isSoundPlaying(0x1094A))
		_vm->playSound(0x1094A, true);

	_vm->queueInsertDeviceIcon();

	gameSys.insertSequence(0x96, 1, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.setAnimation(0x96, 1, 3);
	gameSys.insertSequence(0x99, 1, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.setAnimation(0x99, 1, 4);
	_currDancerSequenceId = 0x8F;
	gameSys.setAnimation(_currDancerSequenceId, 1, 2);
	gameSys.insertSequence(_currDancerSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
		_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
		if (_vm->_prevSceneNum == 46)
			_vm->_toyUfoX = 30;
		else
			_vm->_toyUfoX = 770;
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 5);
		gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		_vm->endSceneInit();
	} else if (_vm->isFlag(kGFUnk22)) {
		gnap._sequenceId = 0x9E;
		gnap._sequenceDatNum = 0;
		gnap._id = 1;
		gameSys.setAnimation(0x9E, 1, 0);
		gnap._actionStatus = 1;
		gameSys.insertSequence(gnap._sequenceId, gnap._id, 0, 0, kSeqNone, 0, 0, 0);
		plat.initPos(4, 8, kDirIdleLeft);
		_vm->endSceneInit();
	} else if (_vm->_prevSceneNum == 46) {
		gnap.initPos(-1, 8, kDirUpRight);
		plat.initPos(-1, 9, kDirUpLeft);
		_vm->endSceneInit();
		plat.walkTo(Common::Point(4, 8), -1, 0x107C2, 1);
		gnap.walkTo(Common::Point(2, 7), -1, 0x107B9, 1);
	} else if (_vm->_prevSceneNum == 41) {
		gnap.initPos(11, 8, kDirUpRight);
		plat.initPos(11, 9, kDirUpLeft);
		_vm->endSceneInit();
		plat.walkTo(Common::Point(4, 8), -1, 0x107D2, 1);
		gnap.walkTo(Common::Point(10, 9), -1, 0x107BA, 1);
	} else {
		gnap.initPos(2, 11, kDirUpRight);
		plat.initPos(6, 11, kDirUpLeft);
		_vm->endSceneInit();
		plat.walkTo(Common::Point(4, 8), -1, 0x107C2, 1);
		gnap.walkTo(Common::Point(2, 7), -1, 0x107B9, 1);
	}

	if (!_vm->isFlag(kGFUnk21) && !_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setFlag(kGFUnk21);
		_vm->setGrabCursorSprite(-1);
		gameSys.setAnimation(0x9D, gnap._id, 0);
		gameSys.insertSequence(0x9D, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
		while (gameSys.getAnimationStatus(0) != 2 && !_vm->_gameDone) {
			_vm->gameUpdateTick();
			if (gameSys.getAnimationStatus(2) == 2) {
				gameSys.setAnimation(0, 0, 2);
				int newSeqId = _vm->getRandom(7) + 0x8F;
				gameSys.insertSequence(newSeqId, 1, _currDancerSequenceId, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(newSeqId, 1, 2);
				_currDancerSequenceId = newSeqId;
			}
			if (gameSys.getAnimationStatus(3) == 2 && gameSys.getAnimationStatus(4) == 2) {
				gameSys.insertSequence(0x96, 1, 0x96, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(0x96, 1, 3);
				gameSys.insertSequence(0x99, 1, 0x99, 1, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(0x99, 1, 4);
			}
		}
		gnap._sequenceId = 0x9D;
		gnap._sequenceDatNum = 0;
		_vm->hideCursor();
		_vm->addFullScreenSprite(0x8A, 255);
		gameSys.setAnimation(0xA0, 256, 0);
		gameSys.insertSequence(0xA0, 256, 0, 0, kSeqNone, 0, 0, 0);
		while (gameSys.getAnimationStatus(0) != 2 && !_vm->_gameDone)
			_vm->gameUpdateTick();
		gameSys.setAnimation(0x107BD, gnap._id, 0);
		gameSys.insertSequence(0x107BD, gnap._id,
			makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
			kSeqSyncWait, 0, 75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
		_vm->removeFullScreenSprite();
		_vm->showCursor();
		gnap._sequenceId = 0x7BD;
		gnap._sequenceDatNum = 1;
	}

	plat.playSequence(0x9A);
	gameSys.setAnimation(plat._sequenceId, plat._id, 1);

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094A))
			_vm->playSound(0x1094A, true);

		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			switch (_vm->_sceneClickedHotspot) {
			case kHS45UfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 2;
					_vm->_newSceneNum = 46;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 5);
				}
				break;

			case kHS45UfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 2;
					_vm->_newSceneNum = 41;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 5);
				}
				break;

			case kHS45UfoDevice:
				_vm->runMenu();
				updateHotspots();
				break;

			default:
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHS45Device:
				_vm->runMenu();
				updateHotspots();
				break;

			case kHS45Platypus:
				if (gnap._actionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex >= 0) {
						gnap.playImpossible(plat._pos);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							gnap.playMoan1(plat._pos);
							break;
						case GRAB_CURSOR:
							gnap.kissPlatypus(0);
							plat.playSequence(0x9A);
							gameSys.setAnimation(plat._sequenceId, plat._id, 1);
							break;
						case TALK_CURSOR:
							gnap.playBrainPulsating(plat._pos);
							plat.playSequence(plat.getSequenceId());
							break;
						case PLAT_CURSOR:
							gnap.playImpossible(plat._pos);
							break;
						default:
							break;
						}
					}
				}
				break;

			case kHS45ExitUfoParty:
				if (gnap._actionStatus < 0) {
					_vm->_isLeavingScene = true;
					gnap.walkTo(Common::Point(gnap._pos.x, _vm->_hotspotsWalkPos[kHS45ExitUfoParty].y), 0, 0x107AE, 1);
					gnap._actionStatus = 0;
					_vm->_newSceneNum = 40;
				}
				break;

			case kHS45ExitShoe:
				if (gnap._actionStatus < 0) {
					_vm->_isLeavingScene = true;
					gnap.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS45ExitShoe].x, gnap._pos.y), 0, 0x107AF, 1);
					gnap._actionStatus = 0;
					plat.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS45ExitShoe].x, plat._pos.y), -1, 0x107CF, 1);
					_vm->_newSceneNum = 46;
				}
				break;

			case kHS45ExitRight:
				if (gnap._actionStatus < 0) {
					_vm->_isLeavingScene = true;
					gnap.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS45ExitRight].x, gnap._pos.y), 0, 0x107AB, 1);
					gnap._actionStatus = 0;
					plat.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS45ExitRight].x, plat._pos.y), -1, 0x107CD, 1);
					_vm->_newSceneNum = 41;
				}
				break;

			case kHS45ExitDiscoBall:
				_vm->clearFlag(kGFUnk22);
				_vm->setFlag(kGFUnk23);
				_vm->_sceneDone = true;
				_vm->_newSceneNum = 54;
				break;

			case kHS45DiscoBall:
				if (gnap._actionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex == kItemSpring) {
						gnap.walkTo(_vm->_hotspotsWalkPos[kHS45DiscoBall], 0, 0x9F, 5);
						gnap._actionStatus = 1;
						_vm->setGrabCursorSprite(-1);
						_vm->invRemove(kItemSpring);
					} else if (_vm->_grabCursorSpriteIndex >= 0) {
						gnap.playShowItem(_vm->_grabCursorSpriteIndex, 5, 0);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							gnap.playScratchingHead(Common::Point(5, 0));
							break;
						case GRAB_CURSOR:
						case TALK_CURSOR:
						case PLAT_CURSOR:
							gnap.playImpossible();
							break;
						default:
							break;
						}
					}
				}
				break;

			case kHS45WalkArea1:
				if (gnap._actionStatus < 0)
					gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				break;

			default:
				break;
			}
		}

		if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				_vm->_toyUfoActionStatus = 3;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 5);
			} else {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			}
		}

		updateAnimations();
		_vm->toyUfoCheckTimer();

		if (!_vm->_isLeavingScene && gnap._actionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
			gnap.updateIdleSequence();

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}

	_vm->_sceneWaiting = false;
}

void Scene45::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case 0:
			_vm->_sceneDone = true;
			break;
		case 1:
			_vm->_sceneWaiting = true;
			_vm->setFlag(kGFUnk22);
			updateHotspots();
			gameSys.insertSequence(0x9E, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0x9E;
			gnap._sequenceDatNum = 0;
			gameSys.setAnimation(0x9E, gnap._id, 0);
			break;
		default:
			gnap._actionStatus = -1;
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		if (_vm->getRandom(2) != 0)
			plat.playSequence(0x9B);
		else
			plat.playSequence(0x9C);
		gameSys.setAnimation(plat._sequenceId, plat._id, 1);
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		gameSys.setAnimation(0, 0, 2);
		int newSeqId = _vm->getRandom(7) + 0x8F;
		gameSys.insertSequence(newSeqId, 1, _currDancerSequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(newSeqId, 1, 2);
		_currDancerSequenceId = newSeqId;
	}

	if (gameSys.getAnimationStatus(3) == 2 && gameSys.getAnimationStatus(4) == 2) {
		gameSys.insertSequence(0x96, 1, 0x96, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(0x96, 1, 3);
		gameSys.insertSequence(0x99, 1, 0x99, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(0x99, 1, 4);
	}

	if (gameSys.getAnimationStatus(5) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case 2:
			_vm->_sceneDone = true;
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 5);
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}
}

/*****************************************************************************/

Scene46::Scene46(GnapEngine *vm) : Scene(vm) {
	_currSackGuySequenceId = -1;
	_nextItchyGuySequenceId = -1;
	_nextSackGuySequenceId = -1;
	_currItchyGuySequenceId = -1;
}

int Scene46::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	gameSys.setAnimation(0, 0, 3);
	gameSys.setAnimation(0, 0, 4);
	return 0x4E;
}

void Scene46::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHS46Platypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHS46UfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHS46UfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		_vm->setDeviceHotspot(kHS46UfoDevice, -1, 534, -1, 599);
		_vm->_hotspotsCount = 4;
	} else {
		_vm->setHotspot(kHS46Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHS46ExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHS46ExitKissinBooth, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHS46ExitDisco, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		_vm->setHotspot(kHS46SackGuy, 180, 370, 235, 490, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 8);
		_vm->setHotspot(kHS46ItchyGuy, 535, 210, 650, 480, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 8);
		_vm->setHotspot(kHS46WalkArea1, 0, 0, 800, 485);
		_vm->setDeviceHotspot(kHS46Device, -1, -1, -1, -1);
		_vm->_hotspotsCount = 8;
	}
}

void Scene46::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->queueInsertDeviceIcon();
	gameSys.insertSequence(0x4D, 0, 0, 0, kSeqLoop, 0, 0, 0);

	_currSackGuySequenceId = 0x4B;
	_nextSackGuySequenceId = -1;
	gameSys.setAnimation(0x4B, 1, 3);
	gameSys.insertSequence(_currSackGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	_currItchyGuySequenceId = 0x47;
	_nextItchyGuySequenceId = -1;
	gameSys.setAnimation(0x47, 1, 4);
	gameSys.insertSequence(_currItchyGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
		_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
		if (_vm->_prevSceneNum == 44)
			_vm->_toyUfoX = 30;
		else
			_vm->_toyUfoX = 770;
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 2);
		gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		_vm->endSceneInit();
	} else if (_vm->_prevSceneNum == 44) {
		gnap.initPos(-1, 8, kDirUpRight);
		plat.initPos(-1, 8, kDirUpLeft);
		_vm->endSceneInit();
		plat.walkTo(Common::Point(1, 8), -1, 0x107C2, 1);
		gnap.walkTo(Common::Point(2, 8), -1, 0x107B9, 1);
	} else if (_vm->_prevSceneNum == 45) {
		gnap.initPos(11, 8, kDirUpRight);
		plat.initPos(12, 8, kDirUpLeft);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(8, 8), -1, 0x107BA, 1);
		plat.walkTo(Common::Point(9, 8), -1, 0x107D2, 1);
	} else {
		gnap.initPos(5, 11, kDirUpRight);
		plat.initPos(6, 11, kDirUpLeft);
		_vm->endSceneInit();
		plat.walkTo(Common::Point(5, 8), -1, 0x107C2, 1);
		gnap.walkTo(Common::Point(6, 8), -1, 0x107BA, 1);
	}

	_vm->_timers[4] = _vm->getRandom(50) + 80;
	_vm->_timers[5] = _vm->getRandom(50) + 80;

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
			case kHS46UfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 3;
					_vm->_newSceneNum = 44;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 2);
				}
				break;

			case kHS46UfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 3;
					_vm->_newSceneNum = 45;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 2);
				}
				break;

			case kHS46UfoDevice:
				_vm->runMenu();
				updateHotspots();
				break;

			default:
				break;
			}
		} else {
			switch (_vm->_sceneClickedHotspot) {
			case kHS46Device:
				_vm->runMenu();
				updateHotspots();
				break;

			case kHS46Platypus:
				if (gnap._actionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex >= 0) {
						gnap.playImpossible(plat._pos);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							gnap.playMoan1(plat._pos);
							break;
						case GRAB_CURSOR:
							gnap.kissPlatypus(0);
							break;
						case TALK_CURSOR:
							gnap.playBrainPulsating(plat._pos);
							plat.playSequence(plat.getSequenceId());
							break;
						case PLAT_CURSOR:
							gnap.playImpossible(plat._pos);
							break;
						default:
							break;
						}
					}
				}
				break;

			case kHS46SackGuy:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS46SackGuy], 2, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan1(Common::Point(_vm->_hotspotsWalkPos[kHS46SackGuy].x + 1, 0));
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirUpLeft;
						gnap.walkTo(_vm->_hotspotsWalkPos[kHS46SackGuy], 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = 2;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
				break;

			case kHS46ItchyGuy:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS46ItchyGuy], 7, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan1(Common::Point(_vm->_hotspotsWalkPos[kHS46ItchyGuy].x - 1, 0));
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[kHS46ItchyGuy], 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = 1;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
				break;

			case kHS46ExitUfoParty:
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(gnap._pos.x, _vm->_hotspotsWalkPos[kHS46ExitUfoParty].y), 0, 0x107AE, 1);
				gnap._actionStatus = 0;
				_vm->_newSceneNum = 40;
				break;

			case kHS46ExitKissinBooth:
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS46ExitKissinBooth].x, gnap._pos.y), 0, 0x107AF, 1);
				gnap._actionStatus = 0;
				plat.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS46ExitKissinBooth].x, plat._pos.y), -1, 0x107CF, 1);
				_vm->_newSceneNum = 44;
				break;

			case kHS46ExitDisco:
				_vm->_isLeavingScene = true;
				gnap.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS46ExitDisco].x, gnap._pos.y), 0, 0x107AB, 1);
				gnap._actionStatus = 0;
				plat.walkTo(Common::Point(_vm->_hotspotsWalkPos[kHS46ExitDisco].x, plat._pos.y), -1, 0x107CD, 1);
				_vm->_newSceneNum = 45;
				break;

			case kHS46WalkArea1:
				if (gnap._actionStatus < 0)
					gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				break;

			default:
				break;
			}
		}

		if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				_vm->_toyUfoActionStatus = 4;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 2);
			} else {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			}
		}

		updateAnimations();
		_vm->toyUfoCheckTimer();

		if (!_vm->_isLeavingScene) {
			if (plat._actionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				plat.updateIdleSequence();
			if (gnap._actionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(50) + 80;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _nextItchyGuySequenceId == -1) {
					if (_vm->getRandom(2) != 0)
						_nextItchyGuySequenceId = 0x49;
					else
						_nextItchyGuySequenceId = 0x48;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(50) + 80;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _nextSackGuySequenceId == -1)
					_nextSackGuySequenceId = 0x4C;
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}
}

void Scene46::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case 0:
			_vm->_sceneDone = true;
			break;
		case 1:
			_nextItchyGuySequenceId = 0x46;
			break;
		case 2:
			_nextSackGuySequenceId = 0x4A;
			break;
		default:
			break;
		}
		gnap._actionStatus = -1;
	}

	if (gameSys.getAnimationStatus(3) == 2 && _nextSackGuySequenceId != -1) {
		gameSys.insertSequence(_nextSackGuySequenceId, 1, _currSackGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextSackGuySequenceId, 1, 3);
		_currSackGuySequenceId = _nextSackGuySequenceId;
		_nextSackGuySequenceId = -1;
		_vm->_timers[5] = _vm->getRandom(50) + 80;
	}

	if (gameSys.getAnimationStatus(4) == 2 && _nextItchyGuySequenceId != -1) {
		gameSys.insertSequence(_nextItchyGuySequenceId, 1, _currItchyGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextItchyGuySequenceId, 1, 4);
		_currItchyGuySequenceId = _nextItchyGuySequenceId;
		_nextItchyGuySequenceId = -1;
		_vm->_timers[4] = _vm->getRandom(50) + 80;
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case 3:
			_vm->_sceneDone = true;
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 2);
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}
}

} // End of namespace Gnap
