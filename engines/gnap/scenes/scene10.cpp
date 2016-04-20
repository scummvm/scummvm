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
#include "gnap/scenes/scene10.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSExitBar		= 1,
	kHSExitBackdoor	= 2,
	kHSCook			= 3,
	kHSTongs		= 4,
	kHSBox			= 5,
	kHSOven			= 6,
	kHSWalkArea1	= 7,
	kHSDevice		= 8,
	kHSWalkArea2	= 9,
	kHSWalkArea3	= 10,
	kHSWalkArea4	= 11
};

enum {
	kASLeaveScene		= 0,
	kASAnnoyCook		= 1,
	kASPlatWithBox		= 4
};

Scene10::Scene10(GnapEngine *vm) : Scene(vm) {
	_s10_nextCookSequenceId = -1;
	_s10_currCookSequenceId = -1;
}

int Scene10::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	_vm->_gameSys->setAnimation(0, 0, 2);
	return 0x10F;
}

void Scene10::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitBar, 0, 75, 85, 455, SF_EXIT_NW_CURSOR);
	_vm->setHotspot(kHSExitBackdoor, 75, 590, 500, 599, SF_EXIT_D_CURSOR | SF_WALKABLE);
	_vm->setHotspot(kHSCook, 370, 205, 495, 460, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSTongs, 250, 290, 350, 337, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSBox, 510, 275, 565, 330, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSOven, 690, 280, 799, 420, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSWalkArea1, 59, 0, 495, 460);
	_vm->setHotspot(kHSWalkArea2, 495, 0, 650, 420);
	_vm->setHotspot(kHSWalkArea3, 651, 0, 725, 400);
	_vm->setHotspot(kHSWalkArea4, 725, 0, 799, 441);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 12;
}

void Scene10::run() {
	_s10_currCookSequenceId = 0x103;
	
	_vm->_gameSys->setAnimation(0x103, 100, 2);
	_vm->_gameSys->insertSequence(0x103, 100, 0, 0, kSeqNone, 0, 0, 0);
	
	_s10_nextCookSequenceId = 0x106;
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
		case kHSPlatypus:
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

		case kHSExitBar:
			_vm->_isLeavingScene = true;
			_vm->gnapActionIdle(0x10C);
			_vm->gnapWalkTo(0, 7, 0, 0x107AF, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			_vm->platypusWalkTo(0, 7, -1, 0x107CF, 1);
			_vm->_newSceneNum = 11;
			break;

		case kHSExitBackdoor:
			_vm->_isLeavingScene = true;
			_vm->gnapActionIdle(0x10C);
			_vm->gnapWalkTo(2, 9, 0, 0x107AE, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			_vm->platypusWalkTo(3, 9, -1, 0x107C7, 1);
			_vm->_newSceneNum = 9;
			break;

		case kHSCook:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(4, 8, 6, 0);
				_vm->_gameSys->setAnimation(makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 0);
				_vm->_gnapActionStatus = kASAnnoyCook;
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
					_vm->_gnapActionStatus = kASAnnoyCook;
					break;
				case PLAT_CURSOR:
					_vm->gnapActionIdle(0x10C);
					_vm->gnapUseDeviceOnPlatypuss();
					_vm->platypusWalkTo(4, 6, -1, -1, 1);
					_vm->gnapWalkTo(4, 8, 0, 0x107BB, 1);
					_vm->_gnapActionStatus = kASAnnoyCook;
					break;
				}
			}
			break;

		case kHSTongs:
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
						_vm->_gnapActionStatus = kASAnnoyCook;
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
						_vm->_gnapActionStatus = kASAnnoyCook;
					}
					break;
				}
			}
			break;

		case kHSBox:
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
					_vm->_gnapActionStatus = kASAnnoyCook;
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
						_vm->_platypusActionStatus = kASPlatWithBox;
						_vm->_platypusFacing = kDirUnk4;
						_vm->_largeSprite = _vm->_gameSys->createSurface(0xC3);
						_vm->playGnapIdle(7, 6);
					}
					break;
				}
			}
			break;

		case kHSOven:
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

		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
		case kHSWalkArea4:
			_vm->gnapActionIdle(0x10C);
			_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		case kHSDevice:
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
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		case kASAnnoyCook:
			_s10_nextCookSequenceId = 0x105;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		switch (_vm->_platypusActionStatus) {
		case kASPlatWithBox:
			_s10_nextCookSequenceId = 0x109;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2 && _s10_nextCookSequenceId != -1) {
		
		switch (_s10_nextCookSequenceId) {
		case 0x109:
			_vm->_platX = 4;
			_vm->_platY = 8;
			_vm->_gameSys->insertSequence(0x109, 100, _s10_currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x107C9, 160,
				_vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId,
				kSeqSyncWait, _vm->getSequenceTotalDuration(0x109) + _vm->getSequenceTotalDuration(0x10A) + _vm->getSequenceTotalDuration(0x10843),
				75 * _vm->_platX - _vm->_platGridX, 48 * _vm->_platY - _vm->_platGridY);
			_vm->_gameSys->removeSequence(0x107, 100, true);
			_s10_currCookSequenceId = 0x109;
			_s10_nextCookSequenceId = 0x843;
			_vm->_platypusSequenceId = 0x7C9;
			_vm->_platypusId = 160;
			_vm->_platypusFacing = kDirNone;
			_vm->_platypusSequenceDatNum = 1;
			break;
		case 0x843:
			_vm->hideCursor();
			_vm->_gameSys->insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
			_vm->_gameSys->insertSequence(0x10843, 301, _s10_currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_s10_currCookSequenceId = 0x843;
			_s10_nextCookSequenceId = 0x10A;
			break;
		case 0x10A:
			_vm->_gameSys->insertSequence(_s10_nextCookSequenceId, 100, 0x10843, 301, kSeqSyncWait, 0, 0, 0);
			_s10_currCookSequenceId = _s10_nextCookSequenceId;
			_s10_nextCookSequenceId = 0x104;
			_vm->showCursor();
			_vm->_gameSys->removeSpriteDrawItem(_vm->_largeSprite, 300);
			_vm->delayTicksCursor(5);
			_vm->deleteSurface(&_vm->_largeSprite);
			_vm->setGrabCursorSprite(kItemTongs);
			if (_vm->_platypusActionStatus == kASPlatWithBox)
				_vm->_platypusActionStatus = -1;
			if (_vm->_gnapX == 4 && _vm->_gnapY == 8)
				_vm->gnapWalkStep();
			break;
		default:
			_vm->_gameSys->insertSequence(_s10_nextCookSequenceId, 100, _s10_currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_s10_currCookSequenceId = _s10_nextCookSequenceId;
			break;
		}
	
		switch (_s10_currCookSequenceId) {
		case 0x106: {
			// TODO: Refactor into a if + a switch
			int rnd = _vm->getRandom(7);
			if (_vm->_gnapActionStatus >= 0 || _vm->_platypusActionStatus >= 0)
				_s10_nextCookSequenceId = 0x106;
			else if (rnd == 0)
				_s10_nextCookSequenceId = 0x104;
			else if (rnd == 1)
				_s10_nextCookSequenceId = 0x103;
			else if (rnd == 2) {
				_s10_nextCookSequenceId = 0x106;
				_vm->_gameSys->insertSequence(0x10D, 1, 0, 0, kSeqNone, 0, 0, 0);
			} else
				_s10_nextCookSequenceId = 0x106;
			}
			break;
		case 0x103:
			if (_vm->_gnapActionStatus >= 0 || _vm->_platypusActionStatus >= 0)
				_s10_nextCookSequenceId = 0x106;
			else if (_vm->getRandom(7) == 0)
				_s10_nextCookSequenceId = 0x104;
			else
				_s10_nextCookSequenceId = 0x106;
			break;
		case 0x104:
			if (_vm->_gnapActionStatus >= 0 || _vm->_platypusActionStatus >= 0)
				_s10_nextCookSequenceId = 0x106;
			else if (_vm->getRandom(7) == 0)
				_s10_nextCookSequenceId = 0x103;
			else
				_s10_nextCookSequenceId = 0x106;
			break;
		case 0x105: {
			// TODO: Refactor into a if + a switch
			int rnd = _vm->getRandom(7);
			if (_vm->_gnapActionStatus >= 0 || _vm->_platypusActionStatus >= 0)
				_s10_nextCookSequenceId = 0x106;
			else if (rnd == 0)
				_s10_nextCookSequenceId = 0x104;
			else if (rnd == 1)
				_s10_nextCookSequenceId = 0x103;
			else
				_s10_nextCookSequenceId = 0x106;
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
		if (_s10_currCookSequenceId == 0x843)
			_vm->_gameSys->setAnimation(_s10_currCookSequenceId | 0x10000, 301, 2);
		else
			_vm->_gameSys->setAnimation(_s10_currCookSequenceId, 100, 2);
	}
}

void Scene10::updateAnimationsCb() {
	if (_vm->_gameSys->getAnimationStatus(2) == 2) {
		_vm->_gameSys->setAnimation(_s10_nextCookSequenceId, 100, 2);
		_vm->_gameSys->insertSequence(_s10_nextCookSequenceId, 100, _s10_currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
		_s10_currCookSequenceId = _s10_nextCookSequenceId;
		_s10_nextCookSequenceId = 0x106;
	}
}

} // End of namespace Gnap
