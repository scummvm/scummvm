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
#include "gnap/scenes/group1.h"

namespace Gnap {

Scene10::Scene10(GnapEngine *vm) : Scene(vm) {
	_nextCookSequenceId = -1;
	_currCookSequenceId = -1;
}

int Scene10::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	_vm->_gameSys->setAnimation(0, 0, 2);
	return 0x10F;
}

void Scene10::updateHotspots() {
	_vm->setHotspot(kHS10Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS10ExitBar, 0, 75, 85, 455, SF_EXIT_NW_CURSOR);
	_vm->setHotspot(kHS10ExitBackdoor, 75, 590, 500, 599, SF_EXIT_D_CURSOR | SF_WALKABLE);
	_vm->setHotspot(kHS10Cook, 370, 205, 495, 460, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS10Tongs, 250, 290, 350, 337, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS10Box, 510, 275, 565, 330, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS10Oven, 690, 280, 799, 420, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS10WalkArea1, 59, 0, 495, 460);
	_vm->setHotspot(kHS10WalkArea2, 495, 0, 650, 420);
	_vm->setHotspot(kHS10WalkArea3, 651, 0, 725, 400);
	_vm->setHotspot(kHS10WalkArea4, 725, 0, 799, 441);
	_vm->setDeviceHotspot(kHS10Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 12;
}

void Scene10::run() {
	_currCookSequenceId = 0x103;
	
	_vm->_gameSys->setAnimation(0x103, 100, 2);
	_vm->_gameSys->insertSequence(0x103, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	_nextCookSequenceId = 0x106;
	if (!_vm->isFlag(kGFMudTaken))
		_vm->_gameSys->insertSequence(0x107, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->queueInsertDeviceIcon();
	
	if (_vm->_prevSceneNum == 9) {
		_vm->initGnapPos(11, 8, kDirBottomLeft);
		_vm->initPlatypusPos(12, 7, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(9, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(9, 7, -1, 0x107D2, 1);
	} else {
		_vm->initGnapPos(-1, 7, kDirBottomRight);
		_vm->initPlatypusPos(-2, 8, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(1, 7, -1, 0x107B9, 1);
		_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
	}

	_vm->_timers[4] = _vm->getRandom(80) + 150;
	_vm->_timers[5] = _vm->getRandom(100) + 100;

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1091E))
			_vm->playSound(0x1091E, true);
	
		if (!_vm->isSoundPlaying(0x1091A))
			_vm->playSound(0x1091A, true);
	
		_vm->updateMouseCursor();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->updateCursorByHotspot();
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHS10Platypus:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapImpossible(0, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					if (_vm->isFlag(kGFMudTaken))
						_vm->playGnapMoan1(_vm->_platX, _vm->_platY);
					else
						_vm->playGnapScratchingHead(_vm->_platX, _vm->_platY);
					break;
				case GRAB_CURSOR:
					_vm->gnapKissPlatypus(10);
					break;
				case TALK_CURSOR:
					_vm->playGnapBrainPulsating(_vm->_platX, _vm->_platY);
					_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
					break;
				case PLAT_CURSOR:
					break;
				}
			}
			break;

		case kHS10ExitBar:
			_vm->_isLeavingScene = true;
			_vm->gnapActionIdle(0x10C);
			_vm->gnapWalkTo(0, 7, 0, 0x107AF, 1);
			_vm->_gnapActionStatus = kAS10LeaveScene;
			_vm->platypusWalkTo(0, 7, -1, 0x107CF, 1);
			_vm->_newSceneNum = 11;
			break;

		case kHS10ExitBackdoor:
			_vm->_isLeavingScene = true;
			_vm->gnapActionIdle(0x10C);
			_vm->gnapWalkTo(2, 9, 0, 0x107AE, 1);
			_vm->_gnapActionStatus = kAS10LeaveScene;
			_vm->platypusWalkTo(3, 9, -1, 0x107C7, 1);
			_vm->_newSceneNum = 9;
			break;

		case kHS10Cook:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(4, 8, 6, 0);
				_vm->_gameSys->setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0);
				_vm->_gnapActionStatus = kAS10AnnoyCook;
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(6, 0);
					break;
				case GRAB_CURSOR:
					_vm->playGnapImpossible(0, 0);
					_vm->_gnapIdleFacing = kDirBottomRight;
					break;
				case TALK_CURSOR:
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->gnapActionIdle(0x10C);
					_vm->gnapWalkTo(4, 8, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kAS10AnnoyCook;
					break;
				case PLAT_CURSOR:
					_vm->gnapActionIdle(0x10C);
					_vm->gnapUseDeviceOnPlatypuss();
					_vm->platypusWalkTo(4, 6, -1, -1, 1);
					_vm->gnapWalkTo(4, 8, 0, 0x107BB, 1);
					_vm->_gnapActionStatus = kAS10AnnoyCook;
					break;
				}
			}
			break;

		case kHS10Tongs:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(3, 7, 4, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					if (_vm->isFlag(kGFMudTaken))
						_vm->playGnapMoan2(-1, -1);
					else
						_vm->playGnapScratchingHead(4, 3);
					break;
				case GRAB_CURSOR:
					if (_vm->isFlag(kGFMudTaken))
						_vm->playGnapMoan2(-1, -1);
					else {
						_vm->gnapActionIdle(0x10C);
						_vm->gnapWalkTo(4, 8, 0, 0x107BB, 1);
						_vm->_gnapActionStatus = kAS10AnnoyCook;
					}
					break;
				case TALK_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				case PLAT_CURSOR:
					if (_vm->isFlag(kGFMudTaken))
						_vm->playGnapMoan2(-1, -1);
					else {
						_vm->gnapActionIdle(0x10C);
						_vm->gnapUseDeviceOnPlatypuss();
						_vm->platypusWalkTo(3, 7, -1, -1, 1);
						_vm->gnapWalkTo(4, 8, 0, 0x107BB, 1);
						_vm->_gnapActionStatus = kAS10AnnoyCook;
					}
					break;
				}
			}
			break;

		case kHS10Box:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(7, 6, 6, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(7, 3);
					break;
				case GRAB_CURSOR:
					_vm->gnapActionIdle(0x10C);
					_vm->gnapWalkTo(4, 8, 0, 0x107BB, 1);
					_vm->_gnapActionStatus = kAS10AnnoyCook;
					break;
				case TALK_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				case PLAT_CURSOR:
					if (_vm->isFlag(kGFMudTaken))
						_vm->playGnapMoan2(-1, -1);
					else {
						_vm->invAdd(kItemTongs);
						_vm->setFlag(kGFMudTaken);
						_vm->gnapActionIdle(0x10C);
						_vm->gnapUseDeviceOnPlatypuss();
						_vm->platypusWalkTo(7, 6, 1, 0x107D2, 1);
						_vm->_platypusActionStatus = kAS10PlatWithBox;
						_vm->_platypusFacing = kDirUnk4;
						_vm->_largeSprite = _vm->_gameSys->createSurface(0xC3);
						_vm->playGnapIdle(7, 6);
					}
					break;
				}
			}
			break;

		case kHS10Oven:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(9, 6, 10, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapSequence(_vm->getGnapSequenceId(gskDeflect, 10, 5) | 0x10000);
					break;
				case GRAB_CURSOR:
					_vm->gnapActionIdle(0x10C);
					_vm->gnapWalkTo(9, 6, 0, 0x107BB, 1);
					_vm->_gameSys->insertSequence(0x10E, 120, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
					_vm->_gnapSequenceId = 0x10E;
					_vm->_gnapId = 120;
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->_gnapSequenceDatNum = 0;
					_vm->_gnapX = 9;
					_vm->_gnapY = 6;
					_vm->_timers[2] = 360;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHS10WalkArea1:
		case kHS10WalkArea2:
		case kHS10WalkArea3:
		case kHS10WalkArea4:
			_vm->gnapActionIdle(0x10C);
			_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		case kHS10Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;
			
		default:
			if (_vm->_mouseClickState._left) {
				_vm->gnapActionIdle(0x10C);
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}
	
		updateAnimations();
	
		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(80) + 150;
				_vm->playSound(0x12B, false);
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(100) + 100;
				int _gnapRandomValue = _vm->getRandom(4);
				if (_gnapRandomValue) {
					int sequenceId;
					if (_gnapRandomValue == 1) {
						sequenceId = 0x8A5;
					} else if (_gnapRandomValue == 2) {
						sequenceId = 0x8A6;
					} else {
						sequenceId = 0x8A7;
					}
					_vm->_gameSys->insertSequence(sequenceId | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene10::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS10LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS10AnnoyCook:
			_nextCookSequenceId = 0x105;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		switch (_vm->_platypusActionStatus) {
		case kAS10PlatWithBox:
			_nextCookSequenceId = 0x109;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2 && _nextCookSequenceId != -1) {
		
		switch (_nextCookSequenceId) {
		case 0x109:
			_vm->_platX = 4;
			_vm->_platY = 8;
			_vm->_gameSys->insertSequence(0x109, 100, _currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x107C9, 160,
				_vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId,
				kSeqSyncWait, _vm->getSequenceTotalDuration(0x109) + _vm->getSequenceTotalDuration(0x10A) + _vm->getSequenceTotalDuration(0x10843),
				75 * _vm->_platX - _vm->_platGridX, 48 * _vm->_platY - _vm->_platGridY);
			_vm->_gameSys->removeSequence(0x107, 100, true);
			_currCookSequenceId = 0x109;
			_nextCookSequenceId = 0x843;
			_vm->_platypusSequenceId = 0x7C9;
			_vm->_platypusId = 160;
			_vm->_platypusFacing = kDirNone;
			_vm->_platypusSequenceDatNum = 1;
			break;
		case 0x843:
			_vm->hideCursor();
			_vm->_gameSys->insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
			_vm->_gameSys->insertSequence(0x10843, 301, _currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_currCookSequenceId = 0x843;
			_nextCookSequenceId = 0x10A;
			break;
		case 0x10A:
			_vm->_gameSys->insertSequence(_nextCookSequenceId, 100, 0x10843, 301, kSeqSyncWait, 0, 0, 0);
			_currCookSequenceId = _nextCookSequenceId;
			_nextCookSequenceId = 0x104;
			_vm->showCursor();
			_vm->_gameSys->removeSpriteDrawItem(_vm->_largeSprite, 300);
			_vm->delayTicksCursor(5);
			_vm->deleteSurface(&_vm->_largeSprite);
			_vm->setGrabCursorSprite(kItemTongs);
			if (_vm->_platypusActionStatus == kAS10PlatWithBox)
				_vm->_platypusActionStatus = -1;
			if (_vm->_gnapX == 4 && _vm->_gnapY == 8)
				_vm->gnapWalkStep();
			break;
		default:
			_vm->_gameSys->insertSequence(_nextCookSequenceId, 100, _currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_currCookSequenceId = _nextCookSequenceId;
			break;
		}
	
		switch (_currCookSequenceId) {
		case 0x106: {
			// TODO: Refactor into a if + a switch
			int rnd = _vm->getRandom(7);
			if (_vm->_gnapActionStatus >= 0 || _vm->_platypusActionStatus >= 0)
				_nextCookSequenceId = 0x106;
			else if (rnd == 0)
				_nextCookSequenceId = 0x104;
			else if (rnd == 1)
				_nextCookSequenceId = 0x103;
			else if (rnd == 2) {
				_nextCookSequenceId = 0x106;
				_vm->_gameSys->insertSequence(0x10D, 1, 0, 0, kSeqNone, 0, 0, 0);
			} else
				_nextCookSequenceId = 0x106;
			}
			break;
		case 0x103:
			if (_vm->_gnapActionStatus >= 0 || _vm->_platypusActionStatus >= 0)
				_nextCookSequenceId = 0x106;
			else if (_vm->getRandom(7) == 0)
				_nextCookSequenceId = 0x104;
			else
				_nextCookSequenceId = 0x106;
			break;
		case 0x104:
			if (_vm->_gnapActionStatus >= 0 || _vm->_platypusActionStatus >= 0)
				_nextCookSequenceId = 0x106;
			else if (_vm->getRandom(7) == 0)
				_nextCookSequenceId = 0x103;
			else
				_nextCookSequenceId = 0x106;
			break;
		case 0x105: {
			// TODO: Refactor into a if + a switch
			int rnd = _vm->getRandom(7);
			if (_vm->_gnapActionStatus >= 0 || _vm->_platypusActionStatus >= 0)
				_nextCookSequenceId = 0x106;
			else if (rnd == 0)
				_nextCookSequenceId = 0x104;
			else if (rnd == 1)
				_nextCookSequenceId = 0x103;
			else
				_nextCookSequenceId = 0x106;
			_vm->_timers[2] = _vm->getRandom(30) + 20;
			_vm->_timers[3] = 300;
			_vm->_gameSys->insertSequence(0x10C, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x10C;
			_vm->_gnapIdleFacing = kDirUpRight;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = -1;
			_vm->_platypusActionStatus = -1;
			}
			break;
		}
		if (_currCookSequenceId == 0x843)
			_vm->_gameSys->setAnimation(_currCookSequenceId | 0x10000, 301, 2);
		else
			_vm->_gameSys->setAnimation(_currCookSequenceId, 100, 2);
	}
}

void Scene10::updateAnimationsCb() {
	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		_vm->_gameSys->setAnimation(_nextCookSequenceId, 100, 2);
		_vm->_gameSys->insertSequence(_nextCookSequenceId, 100, _currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
		_currCookSequenceId = _nextCookSequenceId;
		_nextCookSequenceId = 0x106;
	}
}

/*****************************************************************************/

Scene11::Scene11(GnapEngine *vm) : Scene(vm) {
	_billardBallCtr = 0;
	_nextHookGuySequenceId = -1;
	_currHookGuySequenceId = -1;
	_nextGoggleGuySequenceId = -1;
	_currGoggleGuySequenceId = -1;
}

int Scene11::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 3);
	_vm->_gameSys->setAnimation(0, 0, 2);
	if (_vm->_prevSceneNum == 10 || _vm->_prevSceneNum == 13) {
		_vm->playSound(0x108EC, false);
		_vm->playSound(0x10928, false);
	}
	return 0x209;
}

void Scene11::updateHotspots() {
	_vm->setHotspot(kHS11Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS11ExitKitchen, 420, 140, 520, 345, SF_EXIT_U_CURSOR);
	_vm->setHotspot(kHS11ExitToilet, 666, 130, 740, 364, SF_EXIT_R_CURSOR);
	_vm->setHotspot(kHS11ExitLeft, 0, 350, 10, 599, SF_EXIT_L_CURSOR | SF_WALKABLE);
	_vm->setHotspot(kHS11GoggleGuy, 90, 185, 185, 340, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS11HookGuy, 210, 240, 340, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS11Billard, 640, 475, 700, 530, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS11WalkArea1, 0, 0, 365, 453);
	_vm->setHotspot(kHS11WalkArea2, 0, 0, 629, 353);
	_vm->setHotspot(kHS11WalkArea3, 629, 0, 799, 364);
	_vm->setHotspot(kHS11WalkArea4, 735, 0, 799, 397);
	_vm->setHotspot(kHS11WalkArea5, 510, 540, 799, 599);
	_vm->setDeviceHotspot(kHS11Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 13;
}

void Scene11::run() {
	bool flag = true;
	
	_vm->_timers[7] = 50;
	_vm->_hotspots[kHS11Billard]._flags |= SF_DISABLED;
	
	_currGoggleGuySequenceId = 0x1F9;
	_currHookGuySequenceId = 0x201;
	
	switch (_vm->_prevSceneNum) {
	case 13:
		_vm->initGnapPos(8, 5, kDirBottomLeft);
		_vm->initPlatypusPos(9, 6, kDirUnk4);
		break;
	case 47:
		_vm->initGnapPos(8, 5, kDirBottomLeft);
		_vm->initPlatypusPos(9, 5, kDirUnk4);
		_currGoggleGuySequenceId = 0x1FA;
		_currHookGuySequenceId = 0x1FF;
		_vm->_timers[7] = 180;
		break;
	case 12:
		_vm->initGnapPos(-1, 9, kDirBottomRight);
		_vm->initPlatypusPos(-2, 8, kDirNone);
		break;
	default:
		_vm->initGnapPos(6, 6, kDirBottomLeft);
		_vm->initPlatypusPos(6, 5, kDirUnk4);
		break;
	}
	
	_vm->queueInsertDeviceIcon();
	
	_vm->_gameSys->insertSequence(_currHookGuySequenceId, 120, 0, 0, kSeqNone, 0, 0, 0);
	
	_nextHookGuySequenceId = -1;
	
	_vm->_gameSys->setAnimation(_currHookGuySequenceId, 120, 3);
	_vm->_gameSys->insertSequence(_currGoggleGuySequenceId, 121, 0, 0, kSeqNone, 0, 0, 0);
	
	_nextGoggleGuySequenceId = -1;
	
	_vm->_gameSys->setAnimation(_currGoggleGuySequenceId, 121, 2);
	
	_vm->_timers[5] = _vm->getRandom(100) + 75;
	_vm->_timers[4] = _vm->getRandom(40) + 20;
	_vm->_timers[6] = _vm->getRandom(100) + 100;
	_vm->endSceneInit();
	
	if (_vm->_prevSceneNum == 12) {
		_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
	}

	_vm->_gameSys->insertSequence(0x208, 256, 0, 0, kSeqNone, 40, 0, 0);
	
	while (!_vm->_sceneDone) {
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHS11Platypus:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapImpossible(0, 0);
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
					break;
				}
			}
			break;

		case kHS11ExitKitchen:
			_vm->_isLeavingScene = true;
			_vm->gnapWalkTo(6, 5, 0, 0x107BF, 1);
			_vm->_gnapActionStatus = kAS11LeaveScene;
			_vm->platypusWalkTo(6, 6, -1, -1, 1);
			_vm->_newSceneNum = 10;
			break;

		case kHS11ExitToilet:
			_vm->_isLeavingScene = true;
			_vm->gnapWalkTo(8, 5, 0, 0x107BF, 1);
			_vm->_gnapActionStatus = kAS11LeaveScene;
			_vm->platypusWalkTo(8, 6, -1, -1, 1);
			_vm->_newSceneNum = 13;
			break;

		case kHS11ExitLeft:
			_vm->_isLeavingScene = true;
			_vm->gnapWalkTo(-1, 8, 0, 0x107AF, 1);
			_vm->_gnapActionStatus = kAS11LeaveScene;
			_vm->platypusWalkTo(-1, 9, -1, 0x107CF, 1);
			_vm->_newSceneNum = 12;
			break;

		case kHS11GoggleGuy:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemMagazine) {
					_vm->gnapWalkTo(3, 7, 0, 0x107BC, 1);
					_vm->_gnapActionStatus = kAS11ShowMagazineToGoggleGuy;
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 2, 0);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(3, 7, 2, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(1, 6);
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->gnapWalkTo(3, 7, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kAS11TalkGoggleGuy;
						break;
					}
				}
			}
			break;

		case kHS11HookGuy:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_gnapIdleFacing = kDirUpRight;
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->gnapWalkTo(5, 6, 0, 0x107BC, 9);
					_vm->_gnapActionStatus = kAS11ShowItemToHookGuy;
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 4, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapSequence(_vm->getGnapSequenceId(gskDeflect, 3, 6) | 0x10000);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(5, 6, 0, 0x107BC, 1);
						_vm->_gnapActionStatus = kAS11GrabHookGuy;
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirBottomLeft;
						_vm->gnapWalkTo(5, 6, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kAS11TalkHookGuy;
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS11Billard:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(9, 8);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(9, 8);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(9, 8, 0, 0x107BA, 1);
						_vm->_gnapActionStatus = kAS11GrabBillardBall;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(9, 8);
						break;
					}
				}
			}
			break;

		case kHS11WalkArea1:
		case kHS11WalkArea2:
		case kHS11WalkArea3:
		case kHS11WalkArea4:
		case kHS11WalkArea5:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		case kHS11Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;
			
		default:
			if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
			
		}

		updateAnimations();
	
		if (!_vm->_isLeavingScene) {
			if (flag && !_vm->_timers[7]) {
				flag = false;
				_vm->_gameSys->setAnimation(0x207, 257, 4);
				_vm->_gameSys->insertSequence(0x207, 257, 0, 0, kSeqNone, 0, 0, 0);
			}
			_vm->platypusSub426234();
			_vm->updateGnapIdleSequence2();
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(100) + 75;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextGoggleGuySequenceId == -1) {
					if (_vm->getRandom(2))
						_nextGoggleGuySequenceId = 0x1F6;
					else
						_nextGoggleGuySequenceId = 0x1F9;
				}
			}
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(40) + 20;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextHookGuySequenceId == -1) {
					if (_currHookGuySequenceId == 0x201) {
						switch (_vm->getRandom(7)) {
						case 0:
							_nextHookGuySequenceId = 0x200;
							break;
						case 1:
							_nextHookGuySequenceId = 0x205;
							break;
						case 2:
							_nextHookGuySequenceId = 0x202;
							break;
						default:
							_nextHookGuySequenceId = 0x201;
							break;
						}
					} else {
						_nextHookGuySequenceId = 0x201;
					}
				}
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(100) + 100;
				int _gnapRandomValue = _vm->getRandom(3);
				switch (_gnapRandomValue) {
				case 0:
					_vm->_gameSys->insertSequence(0x8A5 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					_vm->_gameSys->insertSequence(0x8A7 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
					_vm->_gameSys->insertSequence(0x8A6 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(50) + 75;
			_vm->_timers[4] = _vm->getRandom(40) + 20;
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene11::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		if (_vm->_gnapActionStatus != kAS11GrabBillardBall)
			_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS11LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS11ShowMagazineToGoggleGuy:
			_nextGoggleGuySequenceId = 0x1F7;
			break;
		case kAS11TalkGoggleGuy:
			_nextGoggleGuySequenceId = 0x1FB;
			break;
		case kAS11GrabHookGuy:
			_nextHookGuySequenceId = 0x204;
			break;
		case kAS11ShowItemToHookGuy:
			_nextHookGuySequenceId = 0x203;
			break;
		case kAS11TalkHookGuy:
			_nextHookGuySequenceId = 0x206;
			break;
		case kAS11GrabBillardBall:
			if (_vm->_gameSys->getAnimationStatus(2) == 2 && _vm->_gameSys->getAnimationStatus(3) == 2) {
				_vm->_gameSys->setAnimation(0, 0, 0);
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				_vm->_gameSys->insertSequence(0x1F4, 255, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceId = 0x1F4;
				_vm->_gnapId = 255;
				_vm->_gnapSequenceDatNum = 0;
				_vm->_gameSys->removeSequence(0x207, 257, true);
				_vm->_gameSys->removeSequence(0x208, 256, true);
				_nextGoggleGuySequenceId = 0x1F8;
				_vm->_timers[5] = _vm->getRandom(100) + 75;
				_vm->_gameSys->insertSequence(_nextGoggleGuySequenceId, 121, _currGoggleGuySequenceId, 121, kSeqSyncWait, 0, 0, 0);
				_vm->_gameSys->setAnimation(_nextGoggleGuySequenceId, 121, 2);
				_currGoggleGuySequenceId = _nextGoggleGuySequenceId;
				_nextGoggleGuySequenceId = -1;
				switch (_billardBallCtr) {
				case 0:
					_nextHookGuySequenceId = 0x1FC;
					break;
				case 1:
					_nextHookGuySequenceId = 0x1FD;
					break;
				default:
					_nextHookGuySequenceId = 0x1FE;
					break;
				}
				++_billardBallCtr;
				_vm->_gameSys->insertSequence(_nextHookGuySequenceId, 120, _currHookGuySequenceId, 120, kSeqSyncWait, 0, 0, 0);
				_vm->_gameSys->setAnimation(_nextHookGuySequenceId, 120, 3);
				_currHookGuySequenceId = _nextHookGuySequenceId;
				_nextHookGuySequenceId = -1;
				_vm->_timers[4] = _vm->getRandom(40) + 20;
				_vm->_gameSys->insertSequence(0x208, 256, 0, 0, kSeqNone, _vm->getSequenceTotalDuration(0x1F4) - 5, 0, 0);
				_vm->_hotspots[kHS11Billard]._flags |= SF_DISABLED;
				_vm->_gameSys->setAnimation(0x207, 257, 4);
				_vm->_gameSys->insertSequence(0x207, 257, 0, 0, kSeqNone, _vm->getSequenceTotalDuration(0x1FE), 0, 0);
				_vm->_gnapActionStatus = -1;
			}
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2 && _nextGoggleGuySequenceId != -1) {
		_vm->_timers[5] = _vm->getRandom(100) + 75;
		_vm->_gameSys->insertSequence(_nextGoggleGuySequenceId, 121, _currGoggleGuySequenceId, 121, kSeqSyncWait, 0, 0, 0);
		_vm->_gameSys->setAnimation(_nextGoggleGuySequenceId, 121, 2);
		_currGoggleGuySequenceId = _nextGoggleGuySequenceId;
		_nextGoggleGuySequenceId = -1;
		if (_vm->_gnapActionStatus >= 1 && _vm->_gnapActionStatus <= 4)
			_vm->_gnapActionStatus = -1;
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		if (_nextHookGuySequenceId == 0x204) {
			_vm->_gameSys->setAnimation(_nextHookGuySequenceId, 120, 3);
			_vm->_gameSys->insertSequence(0x204, 120, _currHookGuySequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x1F5, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_currHookGuySequenceId = 0x204;
			_nextHookGuySequenceId = -1;
			_vm->_gnapSequenceId = 0x1F5;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_timers[4] = _vm->getRandom(40) + 20;
			_vm->_timers[2] = _vm->getRandom(20) + 70;
			_vm->_timers[3] = _vm->getRandom(50) + 200;
			if (_vm->_gnapActionStatus == kAS11GrabHookGuy)
				_vm->_gnapActionStatus = -1;
		} else if (_nextHookGuySequenceId != -1) {
			_vm->_gameSys->insertSequence(_nextHookGuySequenceId, 120, _currHookGuySequenceId, 120, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_nextHookGuySequenceId, 120, 3);
			_currHookGuySequenceId = _nextHookGuySequenceId;
			_nextHookGuySequenceId = -1;
			_vm->_timers[4] = _vm->getRandom(40) + 20;
			if (_vm->_gnapActionStatus >= 6 && _vm->_gnapActionStatus <= 9)
				_vm->_gnapActionStatus = -1;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(4) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 4);
		_vm->_hotspots[kHS11Billard]._flags &= ~SF_DISABLED;
	}
}

/*****************************************************************************/

Scene12::Scene12(GnapEngine *vm) : Scene(vm) {
	_nextBeardGuySequenceId = -1;
	_currBeardGuySequenceId = -1;
	_nextToothGuySequenceId = -1;
	_currToothGuySequenceId = -1;
	_nextBarkeeperSequenceId = -1;
	_currBarkeeperSequenceId = -1;
}

int Scene12::init() {
	return 0x209;
}

void Scene12::updateHotspots() {
	_vm->setHotspot(kHS12Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS12ExitRight, 790, 360, 799, 599, SF_EXIT_R_CURSOR);
	_vm->setHotspot(kHS12ToothGuy, 80, 180, 160, 380, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS12Barkeeper, 490, 175, 580, 238, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS12BeardGuy, 620, 215, 720, 350, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS12Jukebox, 300, 170, 410, 355, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS12WalkArea1, 0, 0, 260, 460);
	_vm->setHotspot(kHS12WalkArea2, 0, 0, 380, 410);
	_vm->setHotspot(kHS12WalkArea3, 0, 0, 799, 395);
	_vm->setHotspot(kHS12WalkArea4, 585, 0, 799, 455);
	_vm->setDeviceHotspot(kHS12Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 11;
}

void Scene12::run() {
	int v18 = 1;

	_vm->queueInsertDeviceIcon();

	_vm->_gameSys->insertSequence(0x207, 256, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x200, 50, 0, 0, kSeqNone, 0, 0, 0);
	
	_currToothGuySequenceId = 0x200;
	_nextToothGuySequenceId = -1;
	
	_vm->_gameSys->setAnimation(0x200, 50, 2);
	_vm->_gameSys->insertSequence(0x202, 50, 0, 0, kSeqNone, 0, 0, 0);
	
	_currBeardGuySequenceId = 0x202;
	_nextBeardGuySequenceId = -1;
	
	_vm->_gameSys->setAnimation(0x202, 50, 4);
	_vm->_gameSys->insertSequence(0x203, 50, 0, 0, kSeqNone, 0, 0, 0);
	
	_currBarkeeperSequenceId = 0x203;
	_nextBarkeeperSequenceId = -1;
	
	_vm->_gameSys->setAnimation(0x203, 50, 3);
	
	_vm->_timers[4] = 30;
	_vm->_timers[6] = _vm->getRandom(30) + 20;
	_vm->_timers[5] = _vm->getRandom(30) + 20;
	_vm->_timers[7] = _vm->getRandom(100) + 100;
	
	if (_vm->_prevSceneNum == 15) {
		_vm->initGnapPos(5, 6, kDirBottomRight);
		_vm->initPlatypusPos(3, 7, kDirNone);
		_vm->endSceneInit();
	} else {
		_vm->initGnapPos(11, 8, kDirBottomLeft);
		_vm->initPlatypusPos(12, 8, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(9, 8, -1, 0x107D2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
		_vm->updateGrabCursorSprite(0, 0);
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHS12Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS12Platypus:
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
				break;
			}
			break;

		case kHS12ExitRight:
			_vm->_isLeavingScene = true;
			_vm->gnapWalkTo(10, -1, 0, 0x107AB, 1);
			_vm->_gnapActionStatus = kAS12LeaveScene;
			_vm->platypusWalkTo(10, -1, -1, -1, 1);
			_vm->_newSceneNum = 11;
			break;

		case kHS12ToothGuy:
			if (_vm->_grabCursorSpriteIndex == kItemQuarter) {
				_vm->_largeSprite = _vm->_gameSys->createSurface(0x141);
				_vm->gnapWalkTo(3, 7, 0, 0x107BC, 9);
				_vm->_gnapIdleFacing = kDirUpLeft;
				_vm->_gnapActionStatus = kAS12QuarterToToothGuy;
				_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 2, 0);
				_vm->setGrabCursorSprite(-1);
			} else if (_vm->_grabCursorSpriteIndex == kItemQuarterWithHole) {
				_vm->gnapWalkTo(3, 7, 0, 0x107BC, 9);
				_vm->_gnapIdleFacing = kDirUpLeft;
				_vm->_gnapActionStatus = kAS12QuarterWithHoleToToothGuy;
				_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 2, 0);
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->gnapWalkTo(3, 7, 0, 0x107BC, 9);
				_vm->_gnapIdleFacing = kDirUpLeft;
				_vm->_gnapActionStatus = kAS12ShowItemToToothGuy;
				_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 2, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapScratchingHead(1, 2);
					break;
				case GRAB_CURSOR:
					_vm->gnapWalkTo(3, 7, 0, 0x107BC, 1);
					_vm->_gnapActionStatus = kAS12GrabToothGuy;
					break;
				case TALK_CURSOR:
					_vm->_gnapIdleFacing = kDirUpLeft;
					_vm->gnapWalkTo(3, 7, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kAS12TalkToothGuy;
					break;
				case PLAT_CURSOR:
					_vm->gnapUseDeviceOnPlatypuss();
					_vm->platypusWalkTo(3, 7, 1, 0x107D2, 1);
					_vm->_platypusActionStatus = kAS12PlatWithToothGuy;
					_vm->_platypusFacing = kDirUnk4;
					_vm->playGnapIdle(2, 7);
					break;
				}
			}
			break;

		case kHS12Barkeeper:
			if (_vm->_grabCursorSpriteIndex == kItemQuarter || _vm->_grabCursorSpriteIndex == kItemQuarterWithHole) {
				_vm->gnapWalkTo(6, 6, 0, 0x107BB, 9);
				_vm->_gnapIdleFacing = kDirUpRight;
				_vm->_gnapActionStatus = kAS12QuarterWithBarkeeper;
				_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 7, 0);
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->gnapWalkTo(6, 6, 0, 0x107BB, 9);
				_vm->_gnapIdleFacing = kDirUpRight;
				_vm->_gnapActionStatus = kAS12ShowItemToBarkeeper;
				_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 7, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->gnapWalkTo(6, 6, 0, 0x107BB, 1);
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->_gnapActionStatus = kAS12LookBarkeeper;
					break;
				case GRAB_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				case TALK_CURSOR:
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->gnapWalkTo(6, 6, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kAS12TalkBarkeeper;
					break;
				case PLAT_CURSOR:
					_vm->playGnapPullOutDevice(_vm->_platX, _vm->_platY);
					_vm->_gameSys->setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0);
					_vm->_gnapActionStatus = kAS12PlatWithBarkeeper;
					break;
				}
			}
			break;

		case kHS12BeardGuy:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->gnapWalkTo(7, 6, 0, 0x107BB, 9);
				_vm->_gnapIdleFacing = kDirUpRight;
				_vm->_gnapActionStatus = kAS12ShowItemToBeardGuy;
				_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 8, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->gnapWalkTo(7, 6, 0, 0x107BB, 1);
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->_gnapActionStatus = kAS12LookBeardGuy;
					break;
				case GRAB_CURSOR:
					// NOTE Bug in the original. It has 9 as flags which seems wrong here.
					_vm->gnapWalkTo(7, 6, 0, 0x107BB, 1);
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->_gnapActionStatus = kAS12GrabBeardGuy;
					break;
				case TALK_CURSOR:
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->gnapWalkTo(7, 6, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kAS12TalkBeardGuy;
					break;
				case PLAT_CURSOR:
					_vm->gnapUseDeviceOnPlatypuss();
					_vm->platypusWalkTo(7, 6, 1, 0x107C2, 1);
					_vm->_platypusActionStatus = kAS12PlatWithBeardGuy;
					_vm->_platypusFacing = kDirNone;
					_vm->playGnapIdle(7, 6);
					break;
				}
			}
			break;

		case kHS12Jukebox:
			_vm->_newSceneNum = 15;
			_vm->_isLeavingScene = true;
			_vm->gnapWalkTo(5, 6, 0, 0x107BC, 1);
			_vm->_gnapActionStatus = kAS12LeaveScene;
			break;

		case kHS12WalkArea1:
		case kHS12WalkArea2:
		case kHS12WalkArea3:
		case kHS12WalkArea4:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;

		}
	
		updateAnimations();
	
		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = 15;
				if (_nextToothGuySequenceId == -1) {
					if (v18 == 0 && _currBeardGuySequenceId == 0x202 && _currBarkeeperSequenceId == 0x203 && _vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0) {
						if (_vm->getRandom(2) != 0)
							_nextToothGuySequenceId = 0x1EC;
						else
							_nextToothGuySequenceId = 0x204;
					} else if (_currToothGuySequenceId != 0x200)
						_nextToothGuySequenceId = 0x200;
					v18 = (v18 + 1) % 15;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				if (_nextBarkeeperSequenceId == -1 && _vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0) {
					if (v18 == 0 && _currToothGuySequenceId == 0x200 && _currBeardGuySequenceId == 0x202 && _vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0) {
						if (_vm->getRandom(2) != 0)
							_nextBarkeeperSequenceId = 0x208;
						else
							_nextBarkeeperSequenceId = 0x1FB;
					} else
						_nextBarkeeperSequenceId = 0x203;
					v18 = (v18 + 1) % 15;
				}
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(30) + 15;
				if (_nextBeardGuySequenceId == -1 && _vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0) {
					if (v18 == 0 && _currToothGuySequenceId == 0x200 && _currBarkeeperSequenceId == 0x203 && _vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0)
						_nextBeardGuySequenceId = 0x1F2;
					else
						_nextBeardGuySequenceId = 0x202;
					v18 = (v18 + 1) % 15;
				}
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(100) + 100;
				int _gnapRandomValue = _vm->getRandom(3);
				switch (_gnapRandomValue) {
				case 0:
					_vm->_gameSys->insertSequence(0x8A5 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					_vm->_gameSys->insertSequence(0x8A7 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
					_vm->_gameSys->insertSequence(0x8A6 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = 30;
			_vm->_timers[5] = _vm->getRandom(30) + 20;
			_vm->_timers[6] = _vm->getRandom(30) + 20;
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene12::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS12LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS12TalkToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				// Easter egg
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_nextToothGuySequenceId = 0x206;
			} else {
				_nextToothGuySequenceId = 0x1EE;
			}
			break;
		case 3:
			break;
		case kAS12GrabToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_nextToothGuySequenceId = 0x206;
			} else {
				_nextToothGuySequenceId = 0x1EF;
			}
			break;
		case kAS12ShowItemToToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_nextToothGuySequenceId = 0x206;
			} else {
				_nextToothGuySequenceId = 0x1ED;
			}
			break;
		case kAS12QuarterWithHoleToToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_nextToothGuySequenceId = 0x206;
			} else {
				_nextToothGuySequenceId = 0x1EA;
			}
			break;
		case kAS12QuarterToToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_nextToothGuySequenceId = 0x206;
			} else {
				_nextToothGuySequenceId = 0x1E9;
			}
			break;
		case kAS12QuarterToToothGuyDone:
			_vm->_gnapActionStatus = -1;
			_vm->showCursor();
			_vm->_gameSys->removeSpriteDrawItem(_vm->_largeSprite, 300);
			_vm->deleteSurface(&_vm->_largeSprite);
			_vm->setGrabCursorSprite(kItemQuarterWithHole);
			break;
		case kAS12TalkBeardGuy:
			_nextBeardGuySequenceId = 0x1F4;
			break;
		case kAS12LookBeardGuy:
			_nextBeardGuySequenceId = 0x1F3;
			break;
		case kAS12GrabBeardGuy:
			_nextBeardGuySequenceId = 0x1F1;
			break;
		case kAS12ShowItemToBeardGuy:
			_nextBeardGuySequenceId = 0x1F0;
			break;
		case kAS12TalkBarkeeper:
			if (_vm->getRandom(2) != 0)
				_nextBarkeeperSequenceId = 0x1FD;
			else
				_nextBarkeeperSequenceId = 0x1FF;
			break;
		case kAS12LookBarkeeper:
			_nextBarkeeperSequenceId = 0x1F8;
			break;
		case 14:
			_nextBarkeeperSequenceId = 0x1F6;
			break;
		case kAS12ShowItemToBarkeeper:
			_nextBarkeeperSequenceId = 0x1F5;
			break;
		case kAS12QuarterWithBarkeeper:
			_nextBarkeeperSequenceId = 0x1FA;
			break;
		case kAS12PlatWithBarkeeper:
			_nextBarkeeperSequenceId = 0x1F9;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		switch (_vm->_platypusActionStatus) {
		case kAS12PlatWithToothGuy:
			_nextToothGuySequenceId = 0x1EB;
			break;
		case kAS12PlatWithBeardGuy:
			_nextBeardGuySequenceId = 0x1F3;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		if (_currToothGuySequenceId == 0x1E9) {
			_vm->_gameSys->setAnimation(0, 0, 2);
			_vm->hideCursor();
			_vm->_gameSys->setAnimation(0x10843, 301, 0);
			_vm->_gnapActionStatus = kAS12QuarterToToothGuyDone;
			_vm->_gameSys->insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
			_vm->_gameSys->insertSequence(0x10843, 301, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x107B7, _vm->_gnapId, 0x10843, 301,
				kSeqSyncWait, 0, 75 * _vm->_gnapX - _vm->_gnapGridX, 48 * _vm->_gnapY - _vm->_gnapGridY);
			_vm->_gnapSequenceId = 0x7B7;
			_vm->_gnapSequenceDatNum = 1;
			_vm->setFlag(kGFTwigTaken);
			_vm->invAdd(kItemQuarterWithHole);
			_vm->invRemove(kItemQuarter);
		}
		if (_nextToothGuySequenceId == 0x1EF) {
			_vm->_gameSys->setAnimation(_nextToothGuySequenceId, 50, 2);
			_vm->_gameSys->insertSequence(_nextToothGuySequenceId, 50, _currToothGuySequenceId, 50, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x205, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_currToothGuySequenceId = _nextToothGuySequenceId;
			_nextToothGuySequenceId = -1;
			_vm->_gnapSequenceId = 0x205;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_timers[4] = 40;
			_vm->_timers[2] = _vm->getRandom(20) + 70;
			_vm->_timers[3] = _vm->getRandom(50) + 200;
			if (_vm->_gnapActionStatus == kAS12GrabToothGuy)
				_vm->_gnapActionStatus = -1;
		} else if (_nextToothGuySequenceId != -1) {
			_vm->_gameSys->insertSequence(_nextToothGuySequenceId, 50, _currToothGuySequenceId, 50, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_nextToothGuySequenceId, 50, 2);
			_currToothGuySequenceId = _nextToothGuySequenceId;
			_nextToothGuySequenceId = -1;
			_vm->_timers[4] = 50;
			if (_vm->_gnapActionStatus >= kAS12TalkToothGuy && _vm->_gnapActionStatus <= kAS12QuarterToToothGuy && _currToothGuySequenceId != 0x1E9 &&
				_currToothGuySequenceId != 0x1EC && _currToothGuySequenceId != 0x200)
				_vm->_gnapActionStatus = -1;
			if (_vm->_platypusActionStatus == kAS12PlatWithToothGuy)
				_vm->_platypusActionStatus = -1;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		if (_vm->_gnapActionStatus == kAS12PlatWithBarkeeper && _currBarkeeperSequenceId == 0x1F9) {
			_vm->_gnapActionStatus = -1;
			_vm->playGnapIdle(7, 6);
			_vm->_timers[5] = 0;
		}
		if (_nextBarkeeperSequenceId != -1) {
			_vm->_gameSys->insertSequence(_nextBarkeeperSequenceId, 50, _currBarkeeperSequenceId, 50, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_nextBarkeeperSequenceId, 50, 3);
			_currBarkeeperSequenceId = _nextBarkeeperSequenceId;
			_nextBarkeeperSequenceId = -1;
			_vm->_timers[5] = _vm->getRandom(30) + 20;
			if (_vm->_gnapActionStatus >= kAS12TalkBarkeeper && _vm->_gnapActionStatus <= kAS12QuarterWithBarkeeper && _currBarkeeperSequenceId != 0x203 &&
				_currBarkeeperSequenceId != 0x1FB && _currBarkeeperSequenceId != 0x208)
				_vm->_gnapActionStatus = -1;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(4) == 2 && _nextBeardGuySequenceId != -1) {
		_vm->_gameSys->insertSequence(_nextBeardGuySequenceId, 50, _currBeardGuySequenceId, 50, kSeqSyncWait, 0, 0, 0);
		_vm->_gameSys->setAnimation(_nextBeardGuySequenceId, 50, 4);
		_currBeardGuySequenceId = _nextBeardGuySequenceId;
		_nextBeardGuySequenceId = -1;
		_vm->_timers[6] = _vm->getRandom(30) + 20;
		if (_vm->_gnapActionStatus >= kAS12TalkBeardGuy && _vm->_gnapActionStatus <= kAS12ShowItemToBeardGuy && _currBeardGuySequenceId != 0x202 && _currBeardGuySequenceId != 0x1F2)
			_vm->_gnapActionStatus = -1;
		if (_vm->_platypusActionStatus == kAS12PlatWithBeardGuy)
			_vm->_platypusActionStatus = -1;
	}
}

/*****************************************************************************/

Scene13::Scene13(GnapEngine *vm) : Scene(vm) {
	_backToiletCtr = -1;
}

int Scene13::init() {
	_vm->playSound(0x108EC, false);
	return 0xAC;
}

void Scene13::updateHotspots() {
	_vm->setHotspot(kHS13Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS13ExitBar, 113, 160, 170, 455, SF_EXIT_L_CURSOR);
	_vm->setHotspot(kHS13BackToilet, 385, 195, 478, 367, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS13FrontToilet, 497, 182, 545, 432, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS13Urinal, 680, 265, 760, 445, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS13Scribble, 560, 270, 660, 370, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS13Sink, 310, 520, 560, 599, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS13WalkArea1, 268, 270, 325, 385);
	_vm->setHotspot(kHS13WalkArea2, 0, 0, 52, 599);
	_vm->setHotspot(kHS13WalkArea3, 0, 0, 113, 550);
	_vm->setHotspot(kHS13WalkArea4, 0, 0, 226, 438);
	_vm->setHotspot(kHS13WalkArea5, 0, 0, 268, 400);
	_vm->setHotspot(kHS13WalkArea6, 0, 0, 799, 367);
	_vm->setHotspot(kHS13WalkArea7, 478, 0, 799, 401);
	_vm->setHotspot(kHS13WalkArea8, 545, 0, 799, 473);
	_vm->setHotspot(kHS13WalkArea9, 0, 549, 799, 599);
	_vm->setDeviceHotspot(kHS13Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 17;
}

void Scene13::showScribble() {
	_vm->hideCursor();
	_vm->_largeSprite = _vm->_gameSys->createSurface(0x6F);
	_vm->_gameSys->insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
	while (!_vm->_mouseClickState._left && !_vm->isKeyStatus1(Common::KEYCODE_ESCAPE) &&
		!_vm->isKeyStatus1(Common::KEYCODE_SPACE) && !_vm->isKeyStatus1(29))
		_vm->gameUpdateTick();
	_vm->_mouseClickState._left = false;
	_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
	_vm->clearKeyStatus1(29);
	_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
	_vm->_gameSys->removeSpriteDrawItem(_vm->_largeSprite, 300);
	_vm->deleteSurface(&_vm->_largeSprite);
	_vm->showCursor();
}

void Scene13::run() {
	int currSoundId = 0;

	_vm->queueInsertDeviceIcon();
	
	_vm->_gameSys->insertSequence(0xAA, 256, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_vm->_prevSceneNum == 14) {
		_vm->initGnapPos(6, 6, kDirBottomLeft);
		_vm->initPlatypusPos(9, 8, kDirNone);
	} else {
		_vm->initGnapPos(3, 7, kDirBottomRight);
		_vm->initPlatypusPos(2, 7, kDirNone);
	}

	_vm->endSceneInit();

	_vm->_timers[4] = _vm->getRandom(20) + 20;
	_vm->_timers[5] = _vm->getRandom(50) + 50;

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1091A))
			_vm->playSound(0x1091A, true);
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHS13Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				_vm->_timers[5] = _vm->getRandom(50) + 50;
			}
			break;

		case kHS13Platypus:
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
				break;
			}
			break;

		case kHS13ExitBar:
			_vm->_isLeavingScene = true;
			_vm->gnapWalkTo(2, 7, 0, 0x107C0, 1);
			_vm->_gnapActionStatus = kAS13LeaveScene;
			_vm->platypusWalkTo(2, 8, -1, -1, 1);
			if (_vm->isFlag(kGFUnk14) || _vm->isFlag(kGFSpringTaken)) {
				_vm->_newSceneNum = 11;
			} else {
				_vm->setFlag(kGFSpringTaken);
				_vm->_newSceneNum = 47;
			}
			break;

		case kHS13BackToilet:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(5, 5, 6, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
				case TALK_CURSOR:
					if (_vm->_gnapX == 5 && _vm->_gnapY == 5) {
						_backToiletCtr = MIN(5, _backToiletCtr + 1);
						_vm->_gameSys->setAnimation(_backToiletCtr + 0xA3, _vm->_gnapId, 0);
						_vm->_gameSys->insertSequence(_backToiletCtr + 0xA3, _vm->_gnapId,
							makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
							kSeqScale | kSeqSyncWait, 0, 0, 0);
						_vm->_gnapActionStatus = kAS13Wait;
						_vm->_gnapSequenceId = _backToiletCtr + 0xA3;
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->_gnapSequenceDatNum = 0;
					} else {
						_vm->gnapWalkTo(5, 5, 0, 0x107BB, 1);
						_vm->_gnapActionStatus = kAS13BackToilet;
						_vm->_gnapIdleFacing = kDirUpRight;
					}
					break;
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHS13FrontToilet:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(6, 7, 7, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->gnapWalkTo(6, 7, 0, 0xA9, 5);
					_vm->_gnapActionStatus = kAS13FrontToilet;
					_vm->_gnapIdleFacing = kDirBottomRight;
					break;
				}
			}
			break;

		case kHS13Scribble:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(7, 7, 8, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->gnapWalkTo(7, 7, 0, 0x107BB, 1);
					_vm->_gnapActionStatus = kAS13LookScribble;
					_vm->_gnapIdleFacing = kDirUpRight;
					break;
				case GRAB_CURSOR:
					_vm->playGnapScratchingHead(0, 0);
					break;
				case TALK_CURSOR:
					_vm->_gnapIdleFacing = kDirUpRight;
					_vm->gnapWalkTo(7, 7, -1, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0), 1);
					break;
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHS13Urinal:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(8, 7, 9, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapSequence(_vm->getGnapSequenceId(gskDeflect, 9, 6));
					_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, -1, 1);
					_vm->_gnapActionStatus = kAS13Wait;
					break;
				case GRAB_CURSOR:
					_vm->gnapWalkTo(8, 7, 0, -1, 1);
					_vm->_gnapActionStatus = kAS13GrabUrinal;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHS13Sink:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapImpossible(0, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapSequence(_vm->getGnapSequenceId(gskDeflect, 5, 9));
					_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, -1, 1);
					_vm->_gnapActionStatus = kAS13Wait;
					break;
				case GRAB_CURSOR:
					_vm->gnapWalkTo(4, 8, 0, 0x107B9, 1);
					_vm->_gnapActionStatus = kAS13GrabSink;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHS13WalkArea2:
		case kHS13WalkArea3:
		case kHS13WalkArea4:
		case kHS13WalkArea5:
		case kHS13WalkArea6:
		case kHS13WalkArea7:
		case kHS13WalkArea8:
		case kHS13WalkArea9:
			_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		case kHS13WalkArea1:
			// Nothing
			break;
			
		default:
			if (_vm->_mouseClickState._left) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}
	
		updateAnimations();
	
		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			if (_vm->_platY == 5 || _vm->_platY == 6)
				_vm->platypusWalkTo(-1, 7, -1, -1, 1);
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				switch (_vm->getRandom(5)) {
				case 0:
					_vm->playSound(0xD2, false);
					break;
				case 1:
					_vm->playSound(0xD3, false);
					break;
				case 2:
					_vm->playSound(0xD4, false);
					break;
				case 3:
					_vm->playSound(0xD5, false);
					break;
				case 4:
					_vm->playSound(0xD6, false);
					break;
				}
			}
			if (!_vm->_timers[5]) {
				int newSoundId;
				_vm->_timers[5] = _vm->getRandom(50) + 50;
				switch (_vm->getRandom(7)) {
				case 0:
					newSoundId = 0xD7;
					_vm->_timers[5] = 2 * _vm->getRandom(50) + 100;
					break;
				case 1:
				case 2:
					newSoundId = 0xCF;
					break;
				case 3:
				case 4:
					newSoundId = 0xD0;
					break;
				default:
					newSoundId = 0xD1;
					break;
				}
				if (newSoundId != currSoundId) {
					_vm->playSound(newSoundId, false);
					currSoundId = newSoundId;
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(20) + 20;
			_vm->_timers[5] = _vm->getRandom(50) + 50;
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene13::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS13LeaveScene:
			_vm->_sceneDone = true;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS13BackToilet:
			_backToiletCtr = MIN(5, _backToiletCtr + 1);
			_vm->_gameSys->insertSequence(_backToiletCtr + 0xA3, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 9, 0, 0, 0);
			_vm->_gnapSequenceId = _backToiletCtr + 0xA3;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS13FrontToilet:
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 14;
			break;
		case kAS13LookScribble:
			_vm->_gnapActionStatus = -1;
			showScribble();
			break;
		case kAS13GrabSink:
			_vm->_gameSys->setAnimation(0xAB, 160, 0);
			_vm->_gameSys->insertSequence(0xAB, 160, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->removeSequence(0xAA, 256, true);
			_vm->_gnapSequenceId = 0xAB;
			_vm->_gnapId = 160;
			_vm->_gnapIdleFacing = kDirBottomRight;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapX = 4;
			_vm->_gnapY = 8;
			_vm->_timers[2] = 360;
			_vm->_gnapActionStatus = kAS13GrabSinkDone;
			break;
		case kAS13GrabSinkDone:
			_vm->_gameSys->insertSequence(0xAA, 256, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapActionStatus = -1;
			break;
		case kAS13Wait:
			_vm->_gnapActionStatus = -1;
			break;
		case kAS13GrabUrinal:
			_vm->_gameSys->setAnimation(0xA2, 120, 0);
			_vm->_gameSys->insertSequence(0xA2, 120, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0xA2;
			_vm->_gnapId = 120;
			_vm->_gnapIdleFacing = kDirBottomLeft;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapX = 4;
			_vm->_gnapY = 6;
			_vm->_timers[2] = 360;
			_vm->_gnapActionStatus = kAS13Wait;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		_vm->_platypusActionStatus = -1;
	}
}

/*****************************************************************************/

Scene14::Scene14(GnapEngine *vm) : Scene(vm) {
}

int Scene14::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	return 0x27;
}

void Scene14::updateHotspots() {
	_vm->setHotspot(kHS14Platypus, 0, 0, 0, 0);
	_vm->setHotspot(kHS14Exit, 0, 590, 799, 599, SF_EXIT_D_CURSOR);
	_vm->setHotspot(kHS14Coin, 330, 390, 375, 440, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS14Toilet, 225, 250, 510, 500, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setDeviceHotspot(kHS14Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFNeedleTaken))
		_vm->_hotspots[kHS14Coin]._flags = SF_DISABLED;
	_vm->_hotspotsCount = 5;
}

void Scene14::run() {
	_vm->_largeSprite = nullptr;

	_vm->queueInsertDeviceIcon();
	
	if (!_vm->isFlag(kGFNeedleTaken))
		_vm->_gameSys->insertSequence(0x23, 10, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->endSceneInit();
	
	if (!_vm->isFlag(kGFNeedleTaken) && _vm->invHas(kItemTongs))
		_vm->_largeSprite = _vm->_gameSys->createSurface(1);

	if (!_vm->isFlag(kGFNeedleTaken)) {
		_vm->_gameSys->insertSequence(0x24, 10, 0x23, 10, kSeqSyncWait, 0, 0, 0);
		_vm->_gnapSequenceId = 0x24;
		_vm->_timers[2] = _vm->getRandom(40) + 50;
	}
	
	while (!_vm->_sceneDone) {	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHS14Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS14Exit:
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 13;
			break;

		case kHS14Coin:
			if (_vm->_grabCursorSpriteIndex == kItemTongs) {
				_vm->invAdd(kItemQuarter);
				_vm->setFlag(kGFNeedleTaken);
				_vm->setGrabCursorSprite(-1);
				_vm->hideCursor();
				_vm->_gameSys->setAnimation(0x26, 10, 0);
				_vm->_gameSys->insertSequence(0x26, 10, _vm->_gnapSequenceId, 10, kSeqSyncWait, 0, 0, 0);
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playSound(0x108E9, false);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playSound(0x108E9, false);
					break;
				case GRAB_CURSOR:
					_vm->_gameSys->insertSequence(0x25, 10, _vm->_gnapSequenceId, 10, kSeqSyncWait, 0, 0, 0);
					_vm->_gameSys->insertSequence(0x23, 10, 0x25, 10, kSeqSyncWait, 0, 0, 0);
					_vm->_gnapSequenceId = 0x23;
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
					break;
				}
			}
			break;

		case kHS14Toilet:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
					_vm->playSound(0x108B1, false);
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
					break;
				}
			}
			break;

		default:
			_vm->_mouseClickState._left = false;
			break;
		}
	
		updateAnimations();
		_vm->checkGameKeys();
	
		if (!_vm->isFlag(kGFNeedleTaken) && !_vm->_timers[2]) {
			_vm->_gameSys->insertSequence(0x24, 10, _vm->_gnapSequenceId, 10, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x24;
			_vm->_timers[2] = _vm->getRandom(40) + 50;
		}
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
		
		_vm->gameUpdateTick();
	}
	
	if (_vm->_largeSprite)
		_vm->deleteSurface(&_vm->_largeSprite);
}

void Scene14::updateAnimations() {	
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		_vm->_gameSys->insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
		_vm->_gameSys->setAnimation(0x10843, 301, 1);
		_vm->_gameSys->insertSequence(0x10843, 301, 0x26, 10, kSeqSyncWait, 0, 0, 0);
	}
	
	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		_vm->_sceneDone = true;
		_vm->_newSceneNum = 13;
		_vm->_grabCursorSpriteIndex = kItemQuarter;
	}
}

/*****************************************************************************/

Scene15::Scene15(GnapEngine *vm) : Scene(vm) {
	_nextRecordSequenceId = -1;
	_currRecordSequenceId = -1;
	_nextSlotSequenceId = -1;
	_currSlotSequenceId = -1;
	_nextUpperButtonSequenceId = -1;
	_currUpperButtonSequenceId = -1;
	_nextLowerButtonSequenceId = -1;
	_currLowerButtonSequenceId = -1;
}

int Scene15::init() {
	return 0xDD;
}

void Scene15::updateHotspots() {
	_vm->setHotspot(kHS15Platypus, 0, 0, 0, 0, SF_DISABLED);
	_vm->setHotspot(kHS15Exit, 50, 590, 750, 599, SF_EXIT_D_CURSOR);
	_vm->setHotspot(kHS15Button1, 210, 425, 260, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15Button2, 280, 425, 325, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15Button3, 340, 425, 385, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15Button4, 400, 425, 445, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15Button5, 460, 425, 510, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15Button6, 520, 425, 560, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15ButtonA, 205, 480, 250, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15ButtonB, 270, 480, 320, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15ButtonC, 335, 480, 380, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15ButtonD, 395, 480, 445, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15ButtonE, 460, 480, 505, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15ButtonF, 515, 480, 560, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15CoinSlot, 585, 475, 620, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15PlayButton, 622, 431, 650, 482, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setDeviceHotspot(kHS15Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 17;
}

void Scene15::run() {
	_currSlotSequenceId = -1;
	_currUpperButtonSequenceId = -1;
	_currLowerButtonSequenceId = -1;
	_nextSlotSequenceId = -1;
	_nextUpperButtonSequenceId = -1;
	_nextLowerButtonSequenceId = -1;
	_currRecordSequenceId = 0xD5;
	_nextRecordSequenceId = -1;

	_vm->_gameSys->setAnimation(0xD5, 1, 0);
	_vm->_gameSys->insertSequence(_currRecordSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->queueInsertDeviceIcon();
	
	_vm->endSceneInit();
	
	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->_hotspots[kHS15Platypus]._x1 = 0;
		_vm->_hotspots[kHS15Platypus]._y1 = 0;
		_vm->_hotspots[kHS15Platypus]._x2 = 0;
		_vm->_hotspots[kHS15Platypus]._y2 = 0;
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
		
		switch (_vm->_sceneClickedHotspot) {
		case kHS15Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS15Exit:
			_vm->_newSceneNum = 12;
			_vm->_isLeavingScene = true;
			break;

		case kHS15CoinSlot:
			if (_vm->_grabCursorSpriteIndex == kItemQuarter || _vm->_grabCursorSpriteIndex == kItemQuarterWithHole) {
				_nextSlotSequenceId = 0xDC; // Insert coin
			} else if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
				_nextSlotSequenceId = 0xDB;
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
					_vm->playSound(0x108E9, false);
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
					break;
				}
			}
			break;

		case kHS15PlayButton:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					if (_vm->isFlag(kGFGnapControlsToyUFO) || _vm->isFlag(kGFUnk13))
						_vm->playSound(0x108E9, false);
					else
						_nextSlotSequenceId = 0xDA;
					break;
				case GRAB_CURSOR:
					if (_vm->isFlag(kGFGnapControlsToyUFO) || _vm->isFlag(kGFUnk13))
						_nextSlotSequenceId = 0xD9;
					else
						_nextSlotSequenceId = 0xDA;
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
					break;
				}
			}
			break;

		case kHS15Button1:
		case kHS15Button2:
		case kHS15Button3:
		case kHS15Button4:
		case kHS15Button5:
		case kHS15Button6:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playSound(0x108E9, false);
					break;
				case GRAB_CURSOR:
					_nextUpperButtonSequenceId = _vm->_sceneClickedHotspot + 0xC5;
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
					break;
				}
			}
			break;

		case kHS15ButtonA:
		case kHS15ButtonB:
		case kHS15ButtonC:
		case kHS15ButtonD:
		case kHS15ButtonE:
		case kHS15ButtonF:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playSound(0x108E9, false);
					break;
				case GRAB_CURSOR:
					_nextLowerButtonSequenceId = _vm->_sceneClickedHotspot + 0xC5;
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
					break;
				}
			}
			break;

		default:
			_vm->_mouseClickState._left = false;
			break;

		}
	
		updateAnimations();
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene15::updateAnimations() {	
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		if (_vm->_isLeavingScene) {
			_vm->_sceneDone = true;
		} else if (_nextSlotSequenceId != -1) {
			_vm->_gameSys->setAnimation(_nextSlotSequenceId, 1, 0);
			_vm->_gameSys->insertSequence(_nextSlotSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
			_currSlotSequenceId = _nextSlotSequenceId;
			_nextSlotSequenceId = -1;
			switch (_currSlotSequenceId) {
			case 0xDC:
				if (_vm->_grabCursorSpriteIndex == kItemQuarter) {
					_vm->invRemove(kItemQuarter);
				} else {
					_vm->invRemove(kItemQuarterWithHole);
					_vm->setFlag(kGFUnk13);
				}
				_vm->setGrabCursorSprite(-1);
				break;
			case 0xDB:
				_vm->setFlag(kGFUnk14);
				_vm->setGrabCursorSprite(-1);
				_nextSlotSequenceId = 0xD8;
				break;
			case 0xD9:
				if (_vm->isFlag(kGFGnapControlsToyUFO)) {
					_vm->clearFlag(kGFGnapControlsToyUFO);
					_vm->invAdd(kItemQuarter);
					_vm->_newGrabCursorSpriteIndex = kItemQuarter;
				} else if (_vm->isFlag(kGFUnk13)) {
					_vm->clearFlag(kGFUnk13);
					_vm->invAdd(kItemQuarterWithHole);
					_vm->_newGrabCursorSpriteIndex = kItemQuarterWithHole;
				}
				_vm->_newSceneNum = 12;
				_vm->_isLeavingScene = true;
				break;
			case 0xD8:
			case 0xDA:
				if (_currUpperButtonSequenceId != -1) {
					_vm->_gameSys->removeSequence(_currUpperButtonSequenceId, 1, true);
					_currUpperButtonSequenceId = -1;
				}
				if (_currLowerButtonSequenceId != -1) {
					_vm->_gameSys->removeSequence(_currLowerButtonSequenceId, 1, true);
					_currLowerButtonSequenceId = -1;
				}
				break;
			}
		} else if (_nextRecordSequenceId != -1) {
			_vm->_gameSys->setAnimation(_nextRecordSequenceId, 1, 0);
			_vm->_gameSys->insertSequence(_nextRecordSequenceId, 1, _currRecordSequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_currRecordSequenceId = _nextRecordSequenceId;
			_nextRecordSequenceId = -1;
			if (_currRecordSequenceId == 0xD3) {
				_vm->invRemove(kItemDiceQuarterHole);
				_vm->_newSceneNum = 16;
				_vm->_isLeavingScene = true;
			}
			_vm->_gameSys->removeSequence(_currUpperButtonSequenceId, 1, true);
			_currUpperButtonSequenceId = -1;
			_vm->_gameSys->removeSequence(_currLowerButtonSequenceId, 1, true);
			_currLowerButtonSequenceId = -1;
		} else if (_nextUpperButtonSequenceId != -1) {
			_vm->_gameSys->setAnimation(_nextUpperButtonSequenceId, 1, 0);
			if (_currUpperButtonSequenceId == -1)
				_vm->_gameSys->insertSequence(_nextUpperButtonSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
			else
				_vm->_gameSys->insertSequence(_nextUpperButtonSequenceId, 1, _currUpperButtonSequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_currUpperButtonSequenceId = _nextUpperButtonSequenceId;
			_nextUpperButtonSequenceId = -1;
			if (_currLowerButtonSequenceId != -1 && _vm->isFlag(kGFUnk14)) {
				if (_currUpperButtonSequenceId == 0xCC && _currLowerButtonSequenceId == 0xCE)
					_nextRecordSequenceId = 0xD3;
				else
					_nextRecordSequenceId = 0xD4;
			}
		} else if (_nextLowerButtonSequenceId != -1) {
			_vm->_gameSys->setAnimation(_nextLowerButtonSequenceId, 1, 0);
			if (_currLowerButtonSequenceId == -1)
				_vm->_gameSys->insertSequence(_nextLowerButtonSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
			else
				_vm->_gameSys->insertSequence(_nextLowerButtonSequenceId, 1, _currLowerButtonSequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_currLowerButtonSequenceId = _nextLowerButtonSequenceId;
			_nextLowerButtonSequenceId = -1;
			if (_currUpperButtonSequenceId != -1 && _vm->isFlag(kGFUnk14)) {
				if (_currUpperButtonSequenceId == 0xCC && _currLowerButtonSequenceId == 0xCE)
					_nextRecordSequenceId = 0xD3;
				else
					_nextRecordSequenceId = 0xD4;
			}
		}
	}
}

/*****************************************************************************/

Scene17::Scene17(GnapEngine *vm) : Scene(vm) {
	_platTryGetWrenchCtr = 0;
	_wrenchCtr = 2;
	_nextCarWindowSequenceId = -1;
	_nextWrenchSequenceId = -1;
	_canTryGetWrench = true;
	_platPhoneCtr = 0;
	_nextPhoneSequenceId = -1;
	_currPhoneSequenceId = -1;
}

int Scene17::init() {
	return 0x263;
}

void Scene17::updateHotspots() {
	_vm->setHotspot(kHS17Platypus, 1, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS17Phone1, 61, 280, 97, 322, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	_vm->setHotspot(kHS17Phone2, 80, 204, 178, 468, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	_vm->setHotspot(kHS17ExitGrubCity, 196, 207, 280, 304, SF_EXIT_U_CURSOR, 3, 5);
	_vm->setHotspot(kHS17ExitToyStore, 567, 211, 716, 322, SF_EXIT_U_CURSOR, 5, 6);
	_vm->setHotspot(kHS17Wrench, 586, 455, 681, 547, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
	_vm->setHotspot(kHS17WalkArea1, 0, 0, 800, 434);
	_vm->setHotspot(kHS17WalkArea2, 541, 0, 800, 600);
	_vm->setHotspot(kHS17WalkArea3, 0, 204, 173, 468);
	_vm->setDeviceHotspot(kHS17Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFGrassTaken))
		_vm->_hotspots[kHS17Wrench]._flags = SF_NONE;
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_vm->_hotspots[kHS17Device]._flags = SF_DISABLED;
		_vm->_hotspots[kHS17Platypus]._flags = SF_DISABLED;
	}
	_vm->_hotspotsCount = 10;
}

void Scene17::update() {
	_vm->gameUpdateTick();
	_vm->updateMouseCursor();
	_vm->updateGrabCursorSprite(0, 0);
	if (_vm->_mouseClickState._left) {
		_vm->gnapWalkTo(-1, -1, -1, -1, 1);
		_vm->_mouseClickState._left = false;
	}
}

void Scene17::platHangUpPhone() {
	int savedGnapActionStatus = _vm->_gnapActionStatus;

	if (_vm->_platypusActionStatus == kAS17PlatPhoningAssistant) {
		_vm->_gnapActionStatus = kAS17PlatHangUpPhone;
		_vm->updateMouseCursor();
		_platPhoneCtr = 0;
		_vm->_platypusActionStatus = -1;
		_vm->_gameSys->setAnimation(0x257, 254, 4);
		_vm->_gameSys->insertSequence(0x257, 254, _currPhoneSequenceId, 254, kSeqSyncExists, 0, 0, 0);
		while (_vm->_gameSys->getAnimationStatus(4) != 2)
			_vm->gameUpdateTick();
		_vm->_gameSys->setAnimation(0x25B, _vm->_platypusId, 1);
		_vm->_gameSys->insertSequence(0x25B, _vm->_platypusId, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
		_vm->_platypusSequenceId = 0x25B;
		_vm->_platypusSequenceDatNum = 0;
		_currPhoneSequenceId = -1;
		_nextPhoneSequenceId = -1;
		_vm->clearFlag(kGFPlatypusTalkingToAssistant);
		while (_vm->_gameSys->getAnimationStatus(1) != 2)
			_vm->gameUpdateTick();
		_vm->_gnapActionStatus = savedGnapActionStatus;
		_vm->updateMouseCursor();
	}
	updateHotspots();
}

void Scene17::run() {
	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(8);
	_vm->_sceneWaiting = false;
	_vm->_timers[4] = _vm->getRandom(100) + 200;
	_vm->_timers[3] = 200;
	_vm->_timers[5] = _vm->getRandom(30) + 80;
	_vm->_timers[6] = _vm->getRandom(30) + 200;
	_vm->_timers[7] = _vm->getRandom(100) + 100;

	if (_vm->isFlag(kGFTruckKeysUsed)) {
		_vm->_gameSys->insertSequence(0x25F, 20, 0, 0, kSeqNone, 0, 0, 0);
	} else {
		if (_vm->_s18GarbageCanPos >= 8) {
			_vm->_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, 97, 1);
		} else if (_vm->_s18GarbageCanPos >= 6) {
			_vm->_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, 68, 2);
		} else if (_vm->_s18GarbageCanPos >= 5) {
			_vm->_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, 23, -1);
		} else if (_vm->_s18GarbageCanPos >= 4) {
			_vm->_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, -11, -5);
		} else {
			_vm->_gameSys->insertSequence(0x260, 20, 0, 0, kSeqNone, 0, -54, -8);
		}
	}

	if (_vm->isFlag(kGFGroceryStoreHatTaken))
		_vm->_gameSys->insertSequence(0x262, 1, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	if (_vm->isFlag(kGFGrassTaken))
		_currWrenchSequenceId = 0x22D;
	else
		_currWrenchSequenceId = 0x22F;

	_currCarWindowSequenceId = 0x244;

	if (_vm->isFlag(kGFUnk14))
		_vm->_gameSys->insertSequence(0x261, 1, 0, 0, kSeqNone, 0, 0, 0);

	_vm->_gameSys->setAnimation(_currWrenchSequenceId, 40, 2);
	_vm->_gameSys->insertSequence(_currWrenchSequenceId, 40, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFGrassTaken)) {
		_vm->_gameSys->setAnimation(0, 0, 3);
	} else {
		_vm->_gameSys->setAnimation(_currCarWindowSequenceId, 40, 3);
		_vm->_gameSys->insertSequence(_currCarWindowSequenceId, 40, 0, 0, kSeqNone, 0, 0, 0);
	}

	_canTryGetWrench = true;

	if (_vm->isFlag(kGFUnk18))
		_vm->_gameSys->insertSequence(0x24F, 100, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->_prevSceneNum == 53 || _vm->_prevSceneNum == 18 || _vm->_prevSceneNum == 20 || _vm->_prevSceneNum == 19) {
		if (_vm->_prevSceneNum == 20) {
			_vm->initGnapPos(4, 6, kDirBottomRight);
			_vm->initPlatypusPos(5, 6, kDirNone);
			_vm->endSceneInit();
			_vm->platypusWalkTo(5, 9, -1, 0x107C2, 1);
			_vm->gnapWalkTo(4, 8, -1, 0x107B9, 1);
		} else if (_vm->isFlag(kGFUnk27)) {
			_vm->initGnapPos(3, 9, kDirUpLeft);
			_vm->_platX = _vm->_hotspotsWalkPos[2].x;
			_vm->_platY = _vm->_hotspotsWalkPos[2].y;
			_vm->_platypusId = 20 * _vm->_hotspotsWalkPos[2].y;
			_vm->_gameSys->insertSequence(0x25A, 20 * _vm->_hotspotsWalkPos[2].y, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x257, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_platypusSequenceId = 0x25A;
			_vm->_platypusSequenceDatNum = 0;
			_vm->endSceneInit();
			_vm->clearFlag(kGFSpringTaken);
			_vm->clearFlag(kGFUnk16);
			_vm->_platypusActionStatus = kAS17PlatPhoningAssistant;
			platHangUpPhone();
			_vm->_gameSys->setAnimation(0, 0, 4);
			_vm->clearFlag(kGFPlatypusTalkingToAssistant);
			_vm->clearFlag(kGFUnk27);
			updateHotspots();
		} else if (_vm->isFlag(kGFUnk25)) {
			_vm->clearFlag(kGFSpringTaken);
			_vm->clearFlag(kGFUnk16);
			_vm->initPlatypusPos(7, 9, kDirNone);
			_vm->_gnapX = _vm->_hotspotsWalkPos[2].x;
			_vm->_gnapY = _vm->_hotspotsWalkPos[2].y;
			_vm->_gnapId = 20 * _vm->_hotspotsWalkPos[2].y;
			_vm->_gameSys->insertSequence(601, 20 * _vm->_hotspotsWalkPos[2].y, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 601;
			_vm->_gnapActionStatus = kAS17GnapHangUpPhone;
			_vm->clearFlag(kGFUnk25);
			_vm->_gameSys->insertSequence(0x251, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
			_vm->_gameSys->setAnimation(0x257, 254, 0);
			_vm->_gameSys->insertSequence(0x257, 254, 0x251, 254, kSeqSyncWait, 0, 0, 0);
		} else if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
			_vm->clearFlag(kGFSpringTaken);
			_vm->clearFlag(kGFUnk16);
			_vm->_sceneWaiting = true;
			_vm->initGnapPos(3, 9, kDirUpLeft);
			_vm->_platX = _vm->_hotspotsWalkPos[2].x;
			_vm->_platY = _vm->_hotspotsWalkPos[2].y;
			_vm->_platypusId = 20 * _vm->_hotspotsWalkPos[2].y;
			_currPhoneSequenceId = 0x251;
			_vm->_gameSys->insertSequence(0x25A, 20 * _vm->_hotspotsWalkPos[2].y, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gameSys->insertSequence(_currPhoneSequenceId, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_platypusSequenceId = 0x25A;
			_vm->_platypusSequenceDatNum = 0;
			_vm->endSceneInit();
			_vm->_gameSys->setAnimation(_currPhoneSequenceId, 254, 1);
			_vm->_platypusActionStatus = kAS17PlatPhoningAssistant;
			updateHotspots();
		} else if (_vm->_prevSceneNum == 18) {
			_vm->initGnapPos(6, 6, kDirBottomRight);
			_vm->initPlatypusPos(5, 6, kDirNone);
			_vm->endSceneInit();
			_vm->platypusWalkTo(5, 9, -1, 0x107C2, 1);
			_vm->gnapWalkTo(4, 8, -1, 0x107B9, 1);
		} else {
			if (_vm->isFlag(kGFSpringTaken)) {
				_vm->initGnapPos(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, kDirBottomRight);
				_vm->initPlatypusPos(1, 9, kDirNone);
				_vm->endSceneInit();
			} else {
				_vm->initGnapPos(3, 7, kDirBottomRight);
				_vm->initPlatypusPos(1, 7, kDirNone);
				_vm->endSceneInit();
			}
			_vm->clearFlag(kGFSpringTaken);
			_vm->clearFlag(kGFUnk16);
			_vm->endSceneInit();
		}
	} else {
		_vm->_gnapX = 3;
		_vm->_gnapY = 6;
		_vm->_gnapId = 120;
		_vm->_gnapSequenceId = 0x23D;
		_vm->_gnapSequenceDatNum = 0;
		_vm->_gnapIdleFacing = kDirBottomRight;
		_vm->_gameSys->insertSequence(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_platX = -1;
		_vm->_platY = 8;
		_vm->_platypusId = 160;
		_vm->_gameSys->insertSequence(0x241, 160, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_gameSys->insertSequence(0x107C1, _vm->_platypusId, 0x241, _vm->_platypusId,
			kSeqScale | kSeqSyncWait, 0, 75 * _vm->_platX - _vm->_platGridX, 48 * _vm->_platY - _vm->_platGridY);
		_vm->_gameSys->insertSequence(0x22C, 2, 0, 0, kSeqNone, 0, 0, 0);
		// TODO delayTicksA(2, 9);
		_vm->endSceneInit();
		_vm->_platypusSequenceId = 0x7C1;
		_vm->_platypusSequenceDatNum = 1;
		_vm->_platypusFacing = kDirBottomRight;
		_vm->platypusWalkTo(2, 9, -1, 0x107C2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHS17Device:
			if (_vm->_gnapActionStatus < 0 || _vm->_gnapActionStatus == 3) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS17Platypus:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					if (_vm->isFlag(kGFGrassTaken)) {
						_vm->gnapUseJointOnPlatypus();
					} else {
						_vm->gnapUseDeviceOnPlatypuss();
						_vm->platypusWalkTo(_vm->_hotspotsWalkPos[6].x, _vm->_hotspotsWalkPos[6].y, 1, 0x107C2, 1);
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[6].x + 1, _vm->_hotspotsWalkPos[6].y, 0, 0x107BA, 1);
						_vm->_platypusActionStatus = kAS17GetWrench1;
						_vm->_gnapActionStatus = kAS17GetWrench1;
						_vm->_timers[5] = _vm->getRandom(30) + 80;
						_vm->setGrabCursorSprite(-1);
						_vm->invRemove(kItemJoint);
					}
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapScratchingHead(_vm->_platX, _vm->_platY);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
					case GRAB_CURSOR:
						_vm->playGnapScratchingHead(_vm->_platX, _vm->_platY);
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

		case kHS17Wrench:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFGrassTaken)) {
					_vm->playGnapImpossible(0, 0);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 8, 7);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
					case GRAB_CURSOR:
						_vm->playGnapScratchingHead(8, 7);
						break;
					case TALK_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (_canTryGetWrench) {
							platHangUpPhone();
							_vm->gnapUseDeviceOnPlatypuss();
							_vm->platypusWalkTo(_vm->_hotspotsWalkPos[6].x + 1, _vm->_hotspotsWalkPos[6].y, 1, 0x107C2, 1);
							_vm->_platypusActionStatus = kAS17TryGetWrench;
							_vm->_gnapActionStatus = kAS17TryGetWrench;
							_vm->_timers[5] = _vm->getRandom(30) + 80;
						} else
							_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
	
		case kHS17Phone1:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kAS17PutCoinIntoPhone;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 1, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(1, 3);
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFUnk18)) {
							platHangUpPhone();
							_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y) | 0x10000, 1);
							_vm->_gnapActionStatus = kAS17GetCoinFromPhone;
						} else
							_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (_vm->isFlag(kGFUnk18)) {
							platHangUpPhone();
							_vm->_isLeavingScene = true;
							_vm->gnapUseDeviceOnPlatypuss();
							_vm->_platypusFacing = kDirUpLeft;
							_vm->platypusWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 1, 0x107C2, 1);
							_vm->setFlag(kGFUnk16);
							_vm->_platypusActionStatus = kAS17PlatUsePhone;
							_vm->_gnapActionStatus = kAS17PlatUsePhone;
						} else
							_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS17Phone2:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
					_vm->_gnapActionStatus = kAS17PutCoinIntoPhone;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 1, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(1, 3);
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFUnk18)) {
							platHangUpPhone();
							_vm->_isLeavingScene = true;
							_vm->_gnapIdleFacing = kDirUpLeft;
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							_vm->_gnapActionStatus = kAS17GnapUsePhone;
							_vm->setFlag(kGFSpringTaken);
						} else
							_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						if (_vm->isFlag(kGFUnk18)) {
							platHangUpPhone();
							_vm->_isLeavingScene = true;
							_vm->gnapUseDeviceOnPlatypuss();
							_vm->_platypusFacing = kDirUpLeft;
							_vm->platypusWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 1, 0x107C2, 1);
							_vm->setFlag(kGFUnk16);
							_vm->_platypusActionStatus = kAS17PlatUsePhone;
							_vm->_gnapActionStatus = kAS17PlatUsePhone;
						} else
							_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
	
		case kHS17ExitToyStore:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 18;
				_vm->_gnapIdleFacing = kDirUpRight;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[5].x, _vm->_hotspotsWalkPos[5].y, 0, 0x107BB, 1);
				_vm->_gnapActionStatus = kAS17LeaveScene;
				if (_vm->_platypusActionStatus != kAS17PlatPhoningAssistant)
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[5].x - 1, _vm->_hotspotsWalkPos[5].y, -1, 0x107C2, 1);
			}
			break;
	
		case kHS17ExitGrubCity:
			if (_vm->_gnapActionStatus < 0) {
				platHangUpPhone();
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 20;
				_vm->_gnapIdleFacing = kDirUpLeft;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, 0x107BC, 1);
				_vm->_gnapActionStatus = kAS17LeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[3].x + 1, _vm->_hotspotsWalkPos[3].y, -1, 0x107C2, 1);
			}
			break;
	
		case kHS17WalkArea1:
		case kHS17WalkArea2:
		case kHS17WalkArea3:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;
	
		default:
			if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = 0;
			}
			break;
		}
	
		updateAnimations();
		
		if (!_vm->isSoundPlaying(0x10940))
			_vm->playSound(0x10940, true);
	
		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0)
				_vm->platypusSub426234();
			_vm->updateGnapIdleSequence2();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(100) + 200;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0)
					_vm->_gameSys->insertSequence(0x22B, 21, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(100) + 100;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0) {
					switch (_vm->getRandom(3)) {
					case 0:
						_vm->_gameSys->insertSequence(0x25C, 255, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 1:
						_vm->_gameSys->insertSequence(0x25D, 255, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 2:
						_vm->_gameSys->insertSequence(0x25E, 255, 0, 0, kSeqNone, 0, 0, 0);
						break;
					}
				}
			}
			if (_vm->_platypusActionStatus < 0 && !_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(30) + 80;
				if (_vm->isFlag(kGFGrassTaken) && _nextWrenchSequenceId == -1) {
					_nextWrenchSequenceId = 0x236;
				} else if (_canTryGetWrench) {
					switch (_vm->getRandom(6)) {
					case 0:
						_nextWrenchSequenceId = 0x231;
						break;
					case 1:
						_nextWrenchSequenceId = 0x232;
						break;
					case 2:
					case 3:
						_nextWrenchSequenceId = 0x23C;
						break;
					case 4:
					case 5:
						_nextWrenchSequenceId = 0x22E;
						break;
					}
				} else {
					--_wrenchCtr;
					if (_wrenchCtr) {
						switch (_vm->getRandom(6)) {
						case 0:
							_nextWrenchSequenceId = 0x237;
							break;
						case 1:
							_nextWrenchSequenceId = 0x238;
							break;
						case 2:
							_nextWrenchSequenceId = 0x239;
							break;
						case 3:
							_nextWrenchSequenceId = 0x23A;
							break;
						case 4:
							_nextWrenchSequenceId = 0x23B;
							break;
						case 5:
							_nextWrenchSequenceId = 0x235;
							break;
						}
					} else {
						_wrenchCtr = 2;
						_nextWrenchSequenceId = 0x235;
					}
				}
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(30) + 200;
				if (_nextCarWindowSequenceId == -1 && !_vm->isFlag(kGFGrassTaken))
					_nextCarWindowSequenceId = 0x246;
			}
			_vm->playSoundA();
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene17::updateAnimations() {
	static const int kPlatPhoneSequenceIds[] = {
		0x251, 0x252, 0x253, 0x254, 0x255, 0x256, 0x257
	};

	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS17GetWrench1:
			_vm->_gnapActionStatus = kAS17GetWrenchGnapReady;
			break;
		case kAS17GetCoinFromPhone:
			_vm->playGnapPullOutDevice(1, 3);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->setAnimation(0x250, 100, 0);
			_vm->_gameSys->insertSequence(0x250, 100, 591, 100, kSeqSyncWait, 0, 0, 0);
			_vm->invAdd(kItemDiceQuarterHole);
			_vm->clearFlag(kGFUnk18);
			_vm->_gnapActionStatus = kAS17GetCoinFromPhoneDone;
			break;
		case kAS17GetCoinFromPhoneDone:
			_vm->setGrabCursorSprite(kItemDiceQuarterHole);
			_vm->_gnapActionStatus = -1;
			break;
		case kAS17PutCoinIntoPhone:
			_vm->_gameSys->setAnimation(0x24C, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x24C, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x24C;
			_vm->invRemove(kItemDiceQuarterHole);
			_vm->setGrabCursorSprite(-1);
			_vm->setFlag(kGFUnk18);
			_vm->_gnapActionStatus = kAS17PutCoinIntoPhoneDone;
			break;
		case kAS17PutCoinIntoPhoneDone:
			_vm->_gameSys->insertSequence(0x24F, 100, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapActionStatus = -1;
			break;
		case kAS17GnapUsePhone:
			_vm->_gameSys->setAnimation(0x24D, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x24D, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapActionStatus = kAS17LeaveScene;
			_vm->_newSceneNum = 53;
			break;
		case kAS17GnapHangUpPhone:
			_vm->_gameSys->insertSequence(0x258, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x258;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS17LeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		switch (_vm->_platypusActionStatus) {
		case kAS17TryGetWrench:
			_vm->_platypusActionStatus = -1;
			++_platTryGetWrenchCtr;
			if (_platTryGetWrenchCtr % 2 != 0)
				_nextWrenchSequenceId = 0x233;
			else
				_nextWrenchSequenceId = 0x234;
			_canTryGetWrench = false;
			break;
		case kAS17GetWrench1:
			_nextWrenchSequenceId = 0x230;
			break;
		case kAS17GetWrench2:
			_nextCarWindowSequenceId = 0x249;
			break;
		case kAS17GetWrenchDone:
			_vm->_platypusActionStatus = -1;
			_vm->invAdd(kItemWrench);
			_vm->setGrabCursorSprite(kItemWrench);
			break;
		case kAS17PlatUsePhone:
			_vm->_gameSys->setAnimation(0x24E, _vm->_platypusId, 1);
			_vm->_gameSys->insertSequence(0x24E, _vm->_platypusId, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
			_vm->_platypusSequenceDatNum = 0;
			_vm->_platypusSequenceId = 0x24E;
			_vm->_platypusActionStatus = kAS17LeaveScene;
			_vm->_newSceneNum = 53;
			break;
		case kAS17PlatPhoningAssistant:
			++_platPhoneCtr;
			if (_platPhoneCtr >= 7) {
				_platPhoneCtr = 0;
				_nextPhoneSequenceId = -1;
				_currPhoneSequenceId = -1;
				_vm->_gameSys->insertSequence(0x25B, _vm->_platypusId, 0x25A, _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
				_vm->_platypusSequenceDatNum = 0;
				_vm->_platypusSequenceId = 0x25B;
				_vm->_platypusActionStatus = -1;
				_vm->clearFlag(kGFPlatypusTalkingToAssistant);
				_vm->_sceneWaiting = false;
				updateHotspots();
			} else {
				_nextPhoneSequenceId = kPlatPhoneSequenceIds[_platPhoneCtr];
				_vm->_gameSys->setAnimation(_nextPhoneSequenceId, 254, 1);
				_vm->_gameSys->insertSequence(_nextPhoneSequenceId, 254, _currPhoneSequenceId, 254, kSeqSyncWait, 0, 0, 0);
				_vm->_gameSys->insertSequence(0x25A, _vm->_platypusId, 0x25A, _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
				_vm->_platypusSequenceDatNum = 0;
				_vm->_platypusSequenceId = 0x25A;
				_currPhoneSequenceId = _nextPhoneSequenceId;
			}
			break;
		case kAS17LeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		switch (_nextWrenchSequenceId) {
		case 0x233:
			_vm->_gnapActionStatus = -1;
			_vm->_gameSys->insertSequence(0x243, _vm->_platypusId,
				_vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId,
				kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(_nextWrenchSequenceId, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_currWrenchSequenceId = _nextWrenchSequenceId;
			_nextWrenchSequenceId = -1;
			_vm->_platypusSequenceId = 0x243;
			_vm->_platypusSequenceDatNum = 0;
			_vm->_gameSys->setAnimation(0x243, _vm->_platypusId, 1);
			break;
		case 0x234:
			_vm->_gnapActionStatus = -1;
			_vm->_gameSys->insertSequence(0x242, _vm->_platypusId,
				_vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId,
				kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(_nextWrenchSequenceId, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_currWrenchSequenceId = _nextWrenchSequenceId;
			_nextWrenchSequenceId = -1;
			_vm->_platypusSequenceId = 0x242;
			_vm->_platypusSequenceDatNum = 0;
			_vm->_gameSys->setAnimation(0x242, _vm->_platypusId, 1);
			break;
		case 0x231:
			if (_vm->getRandom(2) != 0)
				_nextCarWindowSequenceId = 0x245;
			else
				_nextCarWindowSequenceId = 0x248;
			_vm->_gameSys->setAnimation(0, 0, 2);
			break;
		case 0x232:
			_nextCarWindowSequenceId = 0x247;
			_vm->_gameSys->setAnimation(0, 0, 2);
			break;
		case 0x22E:
		case 0x235:
			if (_nextWrenchSequenceId == 0x235)
				_vm->_hotspots[kHS17Wrench]._flags &= ~SF_DISABLED;
			else
				_vm->_hotspots[kHS17Wrench]._flags |= SF_DISABLED;
			_canTryGetWrench = !_canTryGetWrench;
			_vm->_gameSys->setAnimation(_nextWrenchSequenceId, 40, 2);
			_vm->_gameSys->insertSequence(_nextWrenchSequenceId, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_currWrenchSequenceId = _nextWrenchSequenceId;
			_nextWrenchSequenceId = -1;
			break;
		case 0x230:
			if (_vm->_gnapActionStatus == kAS17GetWrenchGnapReady) {
				_vm->_gameSys->setAnimation(0, 0, 2);
				if (_canTryGetWrench) {
					_vm->_gameSys->insertSequence(0x22E, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
					_currWrenchSequenceId = 0x22E;
					_canTryGetWrench = false;
				}
				_vm->_gameSys->setAnimation(0x23F, _vm->_platypusId, 1);
				_vm->_gameSys->insertSequence(0x10875, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
				_vm->_gameSys->insertSequence(0x23F, _vm->_platypusId,
					_vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId,
					kSeqSyncWait, 0, 0, 0);
				_vm->_gnapSequenceDatNum = 1;
				_vm->_platypusSequenceDatNum = 0;
				_vm->_gnapSequenceId = 0x875;
				_vm->_platypusSequenceId = 0x23F;
				_vm->gnapWalkTo(3, 8, -1, 0x107B9, 1);
				_vm->_platypusActionStatus = kAS17GetWrench2;
			}
			break;
		default:
			if (_nextWrenchSequenceId != -1) {
				_vm->_gameSys->setAnimation(_nextWrenchSequenceId, 40, 2);
				_vm->_gameSys->insertSequence(_nextWrenchSequenceId, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
				_currWrenchSequenceId = _nextWrenchSequenceId;
				_nextWrenchSequenceId = -1;
			}
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		switch (_nextCarWindowSequenceId) {
		case 0x246:
			_vm->_gameSys->setAnimation(_nextCarWindowSequenceId, 40, 3);
			_vm->_gameSys->insertSequence(_nextCarWindowSequenceId, 40, _currCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_currCarWindowSequenceId = _nextCarWindowSequenceId;
			_nextCarWindowSequenceId = -1;
			break;
		case 0x245:
		case 0x247:
		case 0x248:
			_vm->_gameSys->setAnimation(_nextWrenchSequenceId, 40, 2);
			_vm->_gameSys->insertSequence(_nextWrenchSequenceId, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(2) != 2)
				update();
			_vm->_gameSys->setAnimation(_nextCarWindowSequenceId, 40, 3);
			_vm->_gameSys->insertSequence(_nextCarWindowSequenceId, 40, _currCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_currCarWindowSequenceId = _nextCarWindowSequenceId;
			_nextCarWindowSequenceId = -1;
			_currWrenchSequenceId = _nextWrenchSequenceId;
			_nextWrenchSequenceId = -1;
			break;
		case 0x249:
			_vm->_gameSys->setAnimation(0x230, 40, 2);
			_vm->_gameSys->setAnimation(0x240, _vm->_platypusId, 1);
			_vm->_gameSys->insertSequence(0x230, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(_nextCarWindowSequenceId, 40, _currCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x240, _vm->_platypusId, _vm->_platypusSequenceId, _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x23E, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x23E;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_platypusSequenceId = 0x240;
			_vm->_platypusSequenceDatNum = 0;
			_vm->_gameSys->setAnimation(0x24A, 40, 3);
			_vm->_gameSys->insertSequence(0x24A, 40, _nextCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(2) != 2) {
				update();
				if (_vm->_gameSys->getAnimationStatus(3) == 2) {
					_vm->_gameSys->setAnimation(0x24A, 40, 3);
					_vm->_gameSys->insertSequence(0x24A, 40, 586, 40, kSeqSyncWait, 0, 0, 0);
				}
			}
			_vm->_gameSys->insertSequence(0x22D, 40, 560, 40, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x24B, 40, 3);
			_vm->_gameSys->insertSequence(0x24B, 40, 586, 40, kSeqSyncWait, 0, 0, 0);
			_currCarWindowSequenceId = 0x24B;
			_nextCarWindowSequenceId = -1;
			_currWrenchSequenceId = 0x22D;
			_nextWrenchSequenceId = -1;
			_vm->setFlag(kGFGrassTaken);
			_vm->_gnapActionStatus = -1;
			_vm->_platypusActionStatus = 2;
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(30) + 80;
			break;
		}
	}

}

/*****************************************************************************/

static const int kScene18SequenceIds[] = {
	0x219,  0x21A,  0x21B,  0x21C,  0x21D
};

Scene18::Scene18(GnapEngine *vm) : Scene(vm) {
	_cowboyHatSurface = nullptr;

	_platPhoneCtr = 0;
	_platPhoneIter = 0;
	_nextPhoneSequenceId = -1;
	_currPhoneSequenceId = -1;
}

Scene18::~Scene18() {
	delete _cowboyHatSurface;
}

int Scene18::init() {
	_vm->_gameSys->setAnimation(0, 0, 3);
	return 0x222;
}

void Scene18::updateHotspots() {
	_vm->setHotspot(kHS18Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS18GarbageCan, _vm->_gridMinX + 75 * _vm->_s18GarbageCanPos - 35, _vm->_gridMinY + 230, _vm->_gridMinX + 75 * _vm->_s18GarbageCanPos + 35, _vm->_gridMinY + 318,
		SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, _vm->_s18GarbageCanPos, 7);
	_vm->setHotspot(kHS18ExitToyStore, 460, 238, 592, 442, SF_EXIT_U_CURSOR, 7, 7);
	_vm->setHotspot(kHS18ExitPhoneBooth, 275, 585, 525, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 10);
	_vm->setHotspot(kHS18ExitGrubCity, 0, 350, 15, 600, SF_EXIT_L_CURSOR, 0, 9);
	_vm->setHotspot(kHS18HydrantTopValve, 100, 345, 182, 410, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 8);
	_vm->setHotspot(kHS18HydrantRightValve, 168, 423, 224, 470, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	_vm->setHotspot(kHS18CowboyHat, 184, 63, 289, 171, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	_vm->setHotspot(kHS18WalkArea1, 0, 0, 800, 448);
	_vm->setHotspot(kHS18WalkArea2, 0, 0, 214, 515);
	_vm->setDeviceHotspot(kHS18Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFTruckFilledWithGas)) {
		if (_vm->isFlag(kGFTruckKeysUsed)) {
			_vm->_hotspots[kHS18HydrantTopValve]._flags = SF_DISABLED;
			_vm->_hotspots[kHS18HydrantRightValve]._x1 = 148;
			_vm->_hotspots[kHS18HydrantRightValve]._y1 = 403;
			_vm->_hotspots[kHS18GarbageCan]._flags = SF_DISABLED;
			_vm->_hotspotsWalkPos[kHS18GarbageCan].x = 3;
			_vm->_hotspotsWalkPos[kHS18GarbageCan].y = 7;
		} else {
			_vm->_hotspots[kHS18HydrantTopValve]._y1 = 246;
		}
	} else if (_vm->isFlag(kGFBarnPadlockOpen)) {
		_vm->_hotspots[kHS18HydrantRightValve]._flags = SF_DISABLED;
		_vm->_hotspots[kHS18HydrantTopValve]._x1 = 105;
		_vm->_hotspots[kHS18HydrantTopValve]._x2 = 192;
	} else if (_vm->isFlag(kGFTruckKeysUsed)) {
		_vm->_hotspots[kHS18GarbageCan]._x1 = 115;
		_vm->_hotspots[kHS18GarbageCan]._y1 = 365;
		_vm->_hotspots[kHS18GarbageCan]._x2 = 168;
		_vm->_hotspots[kHS18GarbageCan]._y2 = 470;
		_vm->_hotspots[kHS18GarbageCan]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
		_vm->_hotspotsWalkPos[kHS18GarbageCan].x = 3;
		_vm->_hotspotsWalkPos[kHS18GarbageCan].y = 7;
	}
	if (_vm->isFlag(kGFPlatyPussDisguised))
		_vm->_hotspots[kHS18GarbageCan]._flags = SF_DISABLED;
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_vm->_hotspots[kHS18Device]._flags = SF_DISABLED;
		_vm->_hotspots[kHS18HydrantTopValve]._flags = SF_DISABLED;
		_vm->_hotspots[kHS18HydrantRightValve]._flags = SF_DISABLED;
		_vm->_hotspots[kHS18Platypus]._flags = SF_DISABLED;
	}
	if (_vm->isFlag(kGFUnk14)) {
		_vm->_hotspots[kHS18HydrantTopValve]._flags = SF_DISABLED;
		_vm->_hotspots[kHS18CowboyHat]._flags = SF_DISABLED;
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
		_vm->_s18GarbageCanPos = _vm->_gnapX - 1;
	else
		_vm->_s18GarbageCanPos = _vm->_gnapX + 1;
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
	_vm->_gameSys->insertSequence(0x1FB, 19, 0, 0, kSeqNone, 0, 15 * (5 * _vm->_s18GarbageCanPos - 40), 0);
	_vm->_gameSys->setAnimation(0x1FA, 19, 4);
	_vm->_gameSys->insertSequence(0x1FA, 19, 507, 19, kSeqSyncWait, 0, 15 * (5 * _vm->_s18GarbageCanPos - 40), 0);
	while (_vm->_gameSys->getAnimationStatus(4) != 2)
		_vm->gameUpdateTick();
}

void Scene18::platEndPhoning(bool platFl) {
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_platPhoneIter = 0;
		_platPhoneCtr = 0;
		_vm->_platypusActionStatus = -1;
		if (_currPhoneSequenceId != -1) {
			_vm->_gameSys->setAnimation(0x21E, 254, 3);
			_vm->_gameSys->insertSequence(0x21E, 254, _currPhoneSequenceId, 254, kSeqSyncExists, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(3) != 2)
				_vm->gameUpdateTick();
		}
		_vm->_gameSys->removeSequence(0x21F, 254, true);
		_vm->_gameSys->setAnimation(0, 0, 3);
		_vm->clearFlag(kGFPlatypusTalkingToAssistant);
		if (platFl) {
			_vm->_platypusActionStatus = kAS18PlatComesHere;
			_vm->_timers[6] = 50;
			_vm->_sceneWaiting = true;
		}
		_currPhoneSequenceId = -1;
		_nextPhoneSequenceId = -1;
		updateHotspots();
	}
}

void Scene18::closeHydrantValve() {
	_vm->_gnapActionStatus = kAS18LeaveScene;
	_vm->updateMouseCursor();
	if (_vm->isFlag(kGFTruckFilledWithGas)) {
		_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS18HydrantRightValve].x, _vm->_hotspotsWalkPos[kHS18HydrantRightValve].y, 0, 0x107BA, 1);
		if (_vm->isFlag(kGFTruckKeysUsed)) {
			_vm->_gnapActionStatus = kAS18CloseRightValveWithGarbageCan;
			waitForGnapAction();
		} else {
			_vm->_gnapActionStatus = kAS18CloseRightValveNoGarbageCan;
			waitForGnapAction();
		}
	} else if (_vm->isFlag(kGFBarnPadlockOpen)) {
		_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS18HydrantTopValve].x, _vm->_hotspotsWalkPos[kHS18HydrantTopValve].y, 0, 0x107BA, 1);
		_vm->_gnapActionStatus = kAS18CloseTopValve;
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
	_cowboyHatSurface = nullptr;

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
		_vm->_gameSys->insertSequence(0x1FA, 19, 0, 0, kSeqNone, 0, 15 * (5 * _vm->_s18GarbageCanPos - 40), 0);
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
		_platPhoneCtr = _vm->getRandom(5);
		if (_vm->isFlag(kGFUnk27)) {
			_vm->_gameSys->insertSequence(0x21E, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
			_currPhoneSequenceId = -1;
			platEndPhoning(true);
			_vm->clearFlag(kGFUnk27);
		} else {
			_currPhoneSequenceId = kScene18SequenceIds[_platPhoneCtr];
			_platPhoneIter = 0;
			_vm->_gameSys->insertSequence(0x21F, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gameSys->insertSequence(_currPhoneSequenceId, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
		}
		if (_vm->isFlag(kGFUnk27)) {
			platEndPhoning(true);
			_vm->clearFlag(kGFUnk27);
		} else {
			_vm->_gameSys->setAnimation(_currPhoneSequenceId, 254, 3);
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
		case kHS18Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS18Platypus:
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

		case kHS18CowboyHat:
			if (_vm->_gnapActionStatus == kAS18StandingOnHydrant) {
				_vm->_gnapActionStatus = kAS18GrabCowboyHat;
				_vm->_sceneWaiting = false;
			} else if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatyPussDisguised)) {
					gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					putDownGarbageCan(0);
				}
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS18CowboyHat].x, _vm->_hotspotsWalkPos[kHS18CowboyHat].y, 3, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(3, 2);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS18CowboyHat].x, _vm->_hotspotsWalkPos[kHS18CowboyHat].y, 0, _vm->getGnapSequenceId(gskPullOutDeviceNonWorking, 3, 2) | 0x10000, 1);
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS18GarbageCan:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFUnk14)) {
					if (_vm->_grabCursorSpriteIndex >= 0)
						_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS18GarbageCan].x, _vm->_hotspotsWalkPos[kHS18GarbageCan].y, 1, 5);
					else
						_vm->playGnapImpossible(0, 0);
				} else {
					if (_vm->isFlag(kGFPlatypusTalkingToAssistant))
						platEndPhoning(true);
					if (_vm->_grabCursorSpriteIndex >= 0) {
						if (!_vm->isFlag(kGFTruckKeysUsed))
							_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS18GarbageCan].x - (_vm->_gnapX < _vm->_s18GarbageCanPos ? 1 : -1),
								_vm->_hotspotsWalkPos[kHS18GarbageCan].y, _vm->_hotspotsWalkPos[kHS18GarbageCan].x, _vm->_hotspotsWalkPos[kHS18GarbageCan].y);
						else
							_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS18GarbageCan].x, _vm->_hotspotsWalkPos[kHS18GarbageCan].y, 2, 4);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							if (!_vm->isFlag(kGFTruckKeysUsed))
								_vm->playGnapScratchingHead(_vm->_hotspotsWalkPos[kHS18GarbageCan].x - (_vm->_gnapX < _vm->_s18GarbageCanPos ? 1 : -1), _vm->_hotspotsWalkPos[kHS18GarbageCan].y);
							else if (!_vm->isFlag(kGFTruckFilledWithGas))
								_vm->playGnapScratchingHead(2, 4);
							break;
						case GRAB_CURSOR:
							if (!_vm->isFlag(kGFTruckKeysUsed)) {
								_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS18GarbageCan].x - (_vm->_gnapX < _vm->_s18GarbageCanPos ? 1 : -1), _vm->_hotspotsWalkPos[kHS18GarbageCan].y,
									-1, -1, 1);
								_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, _vm->_s18GarbageCanPos, _vm->_gnapY) | 0x10000, 1);
								_vm->_gnapActionStatus = kAS18GrabGarbageCanFromStreet;
							} else if (!_vm->isFlag(kGFTruckFilledWithGas)) {
								if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS18GarbageCan].x, _vm->_hotspotsWalkPos[kHS18GarbageCan].y, 0, -1, 1))
									_vm->_gnapActionStatus = kAS18GrabGarbageCanFromHydrant;
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

		case kHS18HydrantTopValve:
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
								_vm->_gnapActionStatus = kAS18PutGarbageCanOnRunningHydrant;
							} else if (!_vm->isFlag(kGFBarnPadlockOpen)) {
								gnapCarryGarbageCanTo(-1, -1, 0, -1, 2);
								_vm->_gnapActionStatus = kAS18PutGarbageCanOnHydrant;
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
						_vm->_gnapActionStatus = kAS18OpenTopValve;
					} else if (_vm->_grabCursorSpriteIndex >= 0) {
						_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS18HydrantTopValve].x, _vm->_hotspotsWalkPos[kHS18HydrantTopValve].y, 1, 5);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							_vm->playGnapScratchingHead(1, 5);
							break;
						case GRAB_CURSOR:
							if (_vm->isFlag(kGFBarnPadlockOpen)) {
								_vm->_hotspots[kHS18WalkArea2]._flags |= SF_WALKABLE;
								_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS18HydrantTopValve].x, _vm->_hotspotsWalkPos[kHS18HydrantTopValve].y, 0, 0x107BA, 1);
								_vm->_hotspots[kHS18WalkArea2]._flags &= ~SF_WALKABLE;
								_vm->_gnapActionStatus = kAS18CloseTopValve;
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

		case kHS18HydrantRightValve:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFUnk14)) {
					if (_vm->_grabCursorSpriteIndex == -1) {
						_vm->playGnapImpossible(0, 0);
					} else {
						_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS18HydrantRightValve].x, _vm->_hotspotsWalkPos[kHS18HydrantRightValve].y, 1, 5);
					}
				} else {
					if (_vm->isFlag(kGFPlatyPussDisguised)) {
						gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
						putDownGarbageCan(0);
					}
					if (_vm->_grabCursorSpriteIndex == kItemWrench) {
						_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, 2, 8) | 0x10000, 1);
						if (_vm->isFlag(kGFTruckKeysUsed))
							_vm->_gnapActionStatus = kAS18OpenRightValveWithGarbageCan;
						else
							_vm->_gnapActionStatus = kAS18OpenRightValveNoGarbageCan;
					} else if (_vm->_grabCursorSpriteIndex >= 0) {
						_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHS18HydrantRightValve].x, _vm->_hotspotsWalkPos[kHS18HydrantRightValve].y, 1, 5);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							_vm->playGnapScratchingHead(1, 5);
							break;
						case GRAB_CURSOR:
							if (_vm->isFlag(kGFTruckFilledWithGas)) {
								_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS18HydrantRightValve].x, _vm->_hotspotsWalkPos[kHS18HydrantRightValve].y, 0, 0x107BA, 1);
								if (_vm->isFlag(kGFTruckKeysUsed))
									_vm->_gnapActionStatus = kAS18CloseRightValveWithGarbageCan;
								else
									_vm->_gnapActionStatus = kAS18CloseRightValveNoGarbageCan;
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

		case kHS18ExitToyStore:
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
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS18ExitToyStore].x, _vm->_hotspotsWalkPos[kHS18ExitToyStore].y, 0, 0x107C0, 1);
					_vm->_gnapActionStatus = kAS18LeaveScene;
					if (!_vm->isFlag(kGFPlatypusTalkingToAssistant))
						_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS18ExitToyStore].x + 1, _vm->_hotspotsWalkPos[kHS18ExitToyStore].y, -1, 0x107C2, 1);
				}
			}
			break;

		case kHS18ExitPhoneBooth:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatyPussDisguised)) {
					gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					putDownGarbageCan(0);
				}
				closeHydrantValve();
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 17;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS18ExitPhoneBooth].x, _vm->_hotspotsWalkPos[kHS18ExitPhoneBooth].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kAS18LeaveScene;
				if (_vm->isFlag(kGFPlatypusTalkingToAssistant))
					_vm->setFlag(kGFUnk27);
				else
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS18ExitPhoneBooth].x + 1, _vm->_hotspotsWalkPos[kHS18ExitPhoneBooth].y, -1, 0x107C2, 1);
			}
			break;

		case kHS18ExitGrubCity:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatyPussDisguised)) {
					gnapCarryGarbageCanTo(-1, -1, 0, -1, -1);
					putDownGarbageCan(0);
				}
				closeHydrantValve();
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 20;
				_vm->_hotspots[kHS18WalkArea2]._flags |= SF_WALKABLE;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS18ExitGrubCity].x, _vm->_hotspotsWalkPos[kHS18ExitGrubCity].y, 0, 0x107B2, 1);
				_vm->_gnapActionStatus = kAS18LeaveScene;
				if (_vm->isFlag(kGFPlatypusTalkingToAssistant))
					platEndPhoning(false);
				else
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHS18ExitGrubCity].x, _vm->_hotspotsWalkPos[kHS18ExitGrubCity].y - 1, -1, 0x107CF, 1);
				_vm->_hotspots[kHS18WalkArea2]._flags &= ~SF_WALKABLE;
			}
			break;

		case kHS18WalkArea1:
		case kHS18WalkArea2:
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
			if (_vm->_gnapActionStatus != kAS18StandingOnHydrant && _vm->_mouseClickState._left) {
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
			_vm->_gnapActionStatus != kAS18OpenRightValveNoGarbageCanDone && _vm->_gnapActionStatus != kAS18OpenRightValveNoGarbageCan &&
			_vm->_gnapActionStatus != kAS18OpenTopValve && _vm->_gnapActionStatus != kAS18OpenTopValveDone &&
			_vm->_gnapActionStatus != kAS18OpenRightValveWithGarbageCan && _vm->_gnapActionStatus != kAS18OpenRightValveWithGarbageCanDone)
			_vm->playSound(0x22B, true);
	
		if (!_vm->_isLeavingScene) {
			if (!_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
				if (_vm->_platypusActionStatus == kAS18PlatComesHere) {
					if (!_vm->_timers[6]) {
						_vm->_platypusActionStatus = -1;
						_vm->_sceneWaiting = false;
						_vm->initPlatypusPos(-1, 10, kDirNone);
						_vm->platypusWalkTo(3, 9, -1, 0x107C2, 1);
						_vm->clearFlag(kGFPlatypusTalkingToAssistant);
					}
				} else {
					_vm->_hotspots[kHS18WalkArea1]._y2 += 48;
					_vm->_hotspots[kHS18WalkArea2]._x1 += 75;
					_vm->updatePlatypusIdleSequence();
					_vm->_hotspots[kHS18WalkArea2]._x1 -= 75;
					_vm->_hotspots[kHS18WalkArea1]._y2 -= 48;
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
		_vm->deleteSurface(&_cowboyHatSurface);
}

void Scene18::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS18GrabGarbageCanFromStreet:
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
		case kAS18GrabGarbageCanFromHydrant:
			_vm->_gameSys->insertSequence(0x1FE, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->removeSequence(0x1F9, 19, true);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x1FE;
			_vm->clearFlag(kGFTruckKeysUsed);
			_vm->setFlag(kGFPlatyPussDisguised);
			updateHotspots();
			_vm->_gnapActionStatus = -1;
			break;
		case kAS18CloseRightValveNoGarbageCan:
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
		case kAS18OpenTopValve:
			_vm->setFlag(kGFBarnPadlockOpen);
			updateHotspots();
			_vm->playGnapPullOutDevice(2, 7);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->insertSequence(0x20C, 19, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_hotspots[kHS18WalkArea2]._flags |= SF_WALKABLE;
			_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS18HydrantTopValve].x, _vm->_hotspotsWalkPos[kHS18HydrantTopValve].y, 0, 0x107BB, 1);
			_vm->_hotspots[kHS18WalkArea2]._flags &= ~SF_WALKABLE;
			_vm->_gnapActionStatus = kAS18OpenTopValveDone;
			break;
		case kAS18OpenTopValveDone:
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
		case kAS18CloseTopValve:
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
		case kAS18GrabCowboyHat:
			_vm->_gameSys->setAnimation(0x200, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x200, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x200;
			_vm->_gnapActionStatus = kAS18GrabCowboyHatDone;
			break;
		case kAS18GrabCowboyHatDone:
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			_cowboyHatSurface = _vm->addFullScreenSprite(0x1D2, 255);
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
			_vm->_gnapActionStatus = kAS18LeaveScene;
			break;
		case kAS18LeaveScene:
			_vm->_sceneDone = true;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS18PutGarbageCanOnRunningHydrant:
			_vm->setFlag(kGFTruckKeysUsed);
			_vm->clearFlag(kGFPlatyPussDisguised);
			_vm->_gameSys->requestRemoveSequence(0x211, 39);
			_vm->_gameSys->requestRemoveSequence(0x212, 39);
			_vm->_gameSys->insertSequence(0x210, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->stopSound(0x22B);
			_vm->_gameSys->setAnimation(0x210, _vm->_gnapId, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x210;
			_vm->_gnapActionStatus = kAS18PutGarbageCanOnRunningHydrant2;
			break;
		case kAS18PutGarbageCanOnRunningHydrant2:
			_vm->playSound(0x22B, true);
			_vm->_gameSys->setAnimation(0x1FF, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x1FF, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x1FF;
			_vm->_sceneWaiting = true;
			_vm->_gnapActionStatus = kAS18StandingOnHydrant;
			break;
		case kAS18StandingOnHydrant:
			_vm->_gameSys->setAnimation(0x1FF, _vm->_gnapId, 0);
			_vm->_gameSys->insertSequence(0x1FF, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			break;
		case kAS18OpenRightValveNoGarbageCan:
		case kAS18OpenRightValveWithGarbageCan:
			_vm->setFlag(kGFTruckFilledWithGas);
			updateHotspots();
			_vm->playGnapPullOutDevice(2, 7);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->insertSequence(0x20B, 19, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_hotspots[kHS18WalkArea2]._flags |= SF_WALKABLE;
			_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHS18HydrantRightValve].x, _vm->_hotspotsWalkPos[kHS18HydrantRightValve].y, 0, 0x107BA, 1);
			_vm->_hotspots[kHS18WalkArea2]._flags &= ~SF_WALKABLE;
			if (_vm->_gnapActionStatus == kAS18OpenRightValveNoGarbageCan)
				_vm->_gnapActionStatus = kAS18OpenRightValveNoGarbageCanDone;
			else
				_vm->_gnapActionStatus = kAS18OpenRightValveWithGarbageCanDone;
			break;
		case kAS18OpenRightValveWithGarbageCanDone:
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
		case kAS18OpenRightValveNoGarbageCanDone:
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
		case kAS18CloseRightValveWithGarbageCan:
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
			_vm->_gnapActionStatus = kAS18CloseRightValveWithGarbageCanDone;
			break;
		case kAS18CloseRightValveWithGarbageCanDone:
			_vm->_gnapActionStatus = -1;
			break;
		case kAS18PutGarbageCanOnHydrant:
			_vm->setFlag(kGFTruckKeysUsed);
			_vm->clearFlag(kGFPlatyPussDisguised);
			_vm->_gameSys->insertSequence(0x20F, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x20F, _vm->_gnapId, 0);
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapSequenceId = 0x20F;
			_vm->_gnapActionStatus = kAS18PutGarbageCanOnHydrantDone;
			break;
		case kAS18PutGarbageCanOnHydrantDone:
			_vm->_gameSys->insertSequence(0x1F9, 19, 0x20F, _vm->_gnapId, kSeqNone, 0, 0, 0);
			updateHotspots();
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 3);
		++_platPhoneIter;
		if (_platPhoneIter <= 4) {
			++_platPhoneCtr;
			_nextPhoneSequenceId = kScene18SequenceIds[_platPhoneCtr % 5];
			_vm->_gameSys->setAnimation(_nextPhoneSequenceId, 254, 3);
			_vm->_gameSys->insertSequence(_nextPhoneSequenceId, 254, _currPhoneSequenceId, 254, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x21F, 254, 0x21F, 254, kSeqSyncWait, 0, 0, 0);
			_currPhoneSequenceId = _nextPhoneSequenceId;
		} else {
			platEndPhoning(true);
		}
	}
}

/*****************************************************************************/

static const int kS19ShopAssistantSequenceIds[] = {
	0x6F, 0x70, 0x71, 0x72, 0x73
};

Scene19::Scene19(GnapEngine *vm) : Scene(vm) {
	_toyGrabCtr = 0;
	_pictureSurface = 0;
	_shopAssistantCtr = 0;
	_pictureSurface = nullptr;
}

Scene19::~Scene19() {
	delete _pictureSurface;
}

int Scene19::init() {
	_vm->playSound(0x79, false);
	return _vm->isFlag(kGFPlatypusTalkingToAssistant) ? 0x77 : 0x76;
}

void Scene19::updateHotspots() {
	_vm->setHotspot(kHS19Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS19ExitOutsideToyStore, 36, 154, 142, 338, SF_EXIT_NW_CURSOR, 4, 6);
	_vm->setHotspot(kHS19Picture, 471, 237, 525, 283, SF_DISABLED, 7, 2);
	_vm->setHotspot(kHS19ShopAssistant, 411, 151, 575, 279, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
	_vm->setHotspot(kHS19Phone, 647, 166, 693, 234, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 0);
	_vm->setHotspot(kHS19Toy1, 181, 11, 319, 149, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 0);
	_vm->setHotspot(kHS19Toy2, 284, 85, 611, 216, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 0);
	_vm->setHotspot(kHS19Toy3, 666, 38, 755, 154, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 0);
	_vm->setHotspot(kHS19Toy4, 154, 206, 285, 327, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 3);
	_vm->setHotspot(kHS19Toy5, 494, 301, 570, 448, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 5);
	_vm->setHotspot(kHS19Toy6, 0, 320, 188, 600, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 6);
	_vm->setHotspot(kHS19Toy7, 597, 434, 800, 600, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 8);
	_vm->setHotspot(kHS19WalkArea1, 0, 0, 170, 600);
	_vm->setHotspot(kHS19WalkArea2, 622, 0, 800, 600);
	_vm->setHotspot(kHS19WalkArea3, 0, 0, 800, 437);
	_vm->setDeviceHotspot(kHS19Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_vm->_hotspots[kHS19Toy1]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Toy2]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Toy3]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Toy4]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Toy5]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Toy6]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Toy7]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19ShopAssistant]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Phone]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Platypus]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Picture]._flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	}
	_vm->_hotspotsCount = 16;
}

void Scene19::run() {
	_vm->queueInsertDeviceIcon();

	_toyGrabCtr = 0;
	_pictureSurface = 0;

	_vm->_gameSys->insertSequence(0x74, 254, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x75, 254, 0, 0, kSeqNone, 0, 0, 0);

	if (!_vm->isFlag(kGFPictureTaken))
		_vm->_gameSys->insertSequence(0x69, 19, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_vm->initGnapPos(3, 6, kDirBottomRight);
		_currShopAssistantSequenceId = kS19ShopAssistantSequenceIds[_vm->getRandom(5)];
		_nextShopAssistantSequenceId = _currShopAssistantSequenceId;
		_vm->_gameSys->setAnimation(_currShopAssistantSequenceId, 20, 4);
		_vm->_gameSys->insertSequence(0x6E, 254, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_gameSys->insertSequence(_currShopAssistantSequenceId, 20, 0, 0, kSeqNone, 0, 0, 0);
		_shopAssistantCtr = 0;
		_vm->endSceneInit();
		_vm->gnapWalkTo(4, 9, -1, 0x107B9, 1);
		updateHotspots();
	} else {
		_currShopAssistantSequenceId = 0x6D;
		_nextShopAssistantSequenceId = -1;
		_vm->_gameSys->setAnimation(0x6D, 20, 4);
		_vm->_gameSys->insertSequence(_currShopAssistantSequenceId, 20, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_timers[6] = _vm->getRandom(40) + 50;
		_vm->initGnapPos(3, 6, kDirBottomRight);
		_vm->initPlatypusPos(4, 6, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(4, 9, -1, 0x107B9, 1);
		_vm->platypusWalkTo(5, 9, -1, 0x107C2, 1);
	}
	
	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 5, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS19Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS19Platypus:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
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
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS19ExitOutsideToyStore:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 18;
				_vm->_hotspots[kHS19WalkArea1]._flags |= SF_WALKABLE;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[1].x, _vm->_hotspotsWalkPos[1].y, 0, 0x107B2, 1);
				_vm->_gnapActionStatus = kAS19LeaveScene;
				if (_vm->isFlag(kGFPlatypusTalkingToAssistant))
					_vm->setFlag(kGFUnk27);
				else
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[1].x + 1, _vm->_hotspotsWalkPos[1].y, -1, 0x107C5, 1);
				_vm->_hotspots[kHS19WalkArea1]._flags &= ~SF_WALKABLE;
			}
			break;

		case kHS19Picture:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 6, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(6, 2);
						break;
					case GRAB_CURSOR:
						if (!_vm->isFlag(kGFPictureTaken)) {
							_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y) | 0x10000, 1);
							_vm->_gnapActionStatus = kAS19GrabPicture;
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS19ShopAssistant:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 6, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(6, 2);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kAS19TalkShopAssistant;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS19Toy1:
		case kHS19Toy2:
		case kHS19Toy3:
		case kHS19Toy4:
		case kHS19Toy5:
		case kHS19Toy6:
		case kHS19Toy7:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan2(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 0, -1, 1);
						_vm->playGnapIdle(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y);
						_vm->_gnapActionStatus = kAS19GrabToy;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS19Phone:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 9, 1);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(9, 1);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 0, -1, 1);
						_vm->playGnapIdle(8, 2);
						_vm->_gnapActionStatus = kAS19UsePhone;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS19WalkArea1:
		case kHS19WalkArea2:
		case kHS19WalkArea3:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = 0;
			}
		}

		updateAnimations();
	
		if (!_vm->_isLeavingScene) {
			_vm->updateGnapIdleSequence();
			if (!_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
				_vm->updatePlatypusIdleSequence();
				if (!_vm->_timers[6] && _nextShopAssistantSequenceId == -1) {
					_vm->_timers[6] = _vm->getRandom(40) + 50;
					if (_vm->getRandom(4) != 0) {
						_nextShopAssistantSequenceId = 0x64;
					} else if (_vm->isFlag(kGFPictureTaken)) {
						_nextShopAssistantSequenceId = 0x64;
					} else {
						_nextShopAssistantSequenceId = 0x6C;
					}
				}
			}
		}

		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
		
		_vm->gameUpdateTick();
	}

	if (_pictureSurface)
		_vm->deleteSurface(&_pictureSurface);
}

void Scene19::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS19UsePhone:
			_nextShopAssistantSequenceId = 0x67;
			break;
		case kAS19GrabToy:
			++_toyGrabCtr;
			switch (_toyGrabCtr) {
			case 1:
				_nextShopAssistantSequenceId = 0x62;
				break;
			case 2:
				_nextShopAssistantSequenceId = 0x6B;
				break;
			case 3:
				_nextShopAssistantSequenceId = 0x66;
				break;
			default:
				_nextShopAssistantSequenceId = 0x65;
				break;
			}
			break;
		case kAS19GrabPicture:
			_vm->playGnapPullOutDevice(6, 2);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->setAnimation(0x68, 19, 0);
			_vm->_gameSys->insertSequence(0x68, 19, 105, 19, kSeqSyncWait, 0, 0, 0);
			_vm->invAdd(kItemPicture);
			_vm->setFlag(kGFPictureTaken);
			updateHotspots();
			_vm->_gnapActionStatus = kAS19GrabPictureDone;
			break;
		case kAS19GrabPictureDone:
			_vm->setGrabCursorSprite(-1);
			_vm->hideCursor();
			_pictureSurface = _vm->addFullScreenSprite(0xF, 255);
			_vm->_gameSys->setAnimation(0x61, 256, 0);
			_vm->_gameSys->insertSequence(0x61, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(0) != 2) {
				// checkGameAppStatus();
				_vm->gameUpdateTick();
			}
			_vm->setFlag(kGFUnk27);
			_vm->showCursor();
			_vm->_newSceneNum = 17;
			_vm->_isLeavingScene = true;
			_vm->_sceneDone = true;
			_nextShopAssistantSequenceId = -1;
			break;
		case kAS19TalkShopAssistant:
			_nextShopAssistantSequenceId = 0x6D;
			_vm->_gnapActionStatus = -1;
			break;
		case kAS19LeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(4) == 2) {
		switch (_nextShopAssistantSequenceId) {
		case 0x6F:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
			_shopAssistantCtr = (_shopAssistantCtr + 1) % 5;
			_nextShopAssistantSequenceId = kS19ShopAssistantSequenceIds[_shopAssistantCtr];
			_vm->_gameSys->setAnimation(_nextShopAssistantSequenceId, 20, 4);
			_vm->_gameSys->insertSequence(_nextShopAssistantSequenceId, 20, _currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x6E, 254, 0x6E, 254, kSeqSyncWait, 0, 0, 0);
			_currShopAssistantSequenceId = _nextShopAssistantSequenceId;
			break;
		case 0x62:
		case 0x66:
		case 0x6B:
			_vm->_gameSys->setAnimation(_nextShopAssistantSequenceId, 20, 4);
			_vm->_gameSys->insertSequence(_nextShopAssistantSequenceId, 20, _currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_currShopAssistantSequenceId = _nextShopAssistantSequenceId;
			_nextShopAssistantSequenceId = -1;
			_vm->_timers[5] = 10;
			while (_vm->_timers[5]) {
				_vm->gameUpdateTick();
			}
			_vm->playGnapIdle(6, 2);
			_vm->_gnapActionStatus = -1;
			break;
		case 0x67:
			_vm->_gameSys->setAnimation(_nextShopAssistantSequenceId, 20, 4);
			_vm->_gameSys->insertSequence(_nextShopAssistantSequenceId, 20, _currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_currShopAssistantSequenceId = _nextShopAssistantSequenceId;
			_nextShopAssistantSequenceId = -1;
			_vm->_gnapActionStatus = -1;
			break;
		case 0x65:
			_vm->playGnapIdle(6, 2);
			_vm->_gameSys->setAnimation(_nextShopAssistantSequenceId, 20, 0);
			_vm->_gameSys->insertSequence(_nextShopAssistantSequenceId, 20, _currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_currShopAssistantSequenceId = _nextShopAssistantSequenceId;
			_nextShopAssistantSequenceId = -1;
			_vm->_newSceneNum = 18;
			_vm->_gnapActionStatus = kAS19LeaveScene;
			break;
		case 0x6D:
			_vm->_gameSys->setAnimation(_nextShopAssistantSequenceId, 20, 4);
			_vm->_gameSys->insertSequence(_nextShopAssistantSequenceId, 20, _currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x69, 19, 0x69, 19, kSeqSyncWait, _vm->getSequenceTotalDuration(_nextShopAssistantSequenceId), 0, 0);
			_currShopAssistantSequenceId = _nextShopAssistantSequenceId;
			_nextShopAssistantSequenceId = -1;
			break;
		case 0x64:
		case 0x6C:
			_vm->_gameSys->setAnimation(_nextShopAssistantSequenceId, 20, 4);
			_vm->_gameSys->insertSequence(_nextShopAssistantSequenceId, 20, _currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_currShopAssistantSequenceId = _nextShopAssistantSequenceId;
			_nextShopAssistantSequenceId = -1;
			break;
		}
	}
}


} // End of namespace Gnap
