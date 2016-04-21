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
#include "gnap/scenes/scene13.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSExitBar		= 1,
	kHSWalkArea1	= 2,
	kHSBackToilet	= 3,
	kHSFrontToilet	= 4,
	kHSUrinal		= 5,
	kHSScribble		= 6,
	kHSSink			= 7,
	kHSWalkArea2	= 8,
	kHSDevice		= 9,
	kHSWalkArea3	= 10,
	kHSWalkArea4	= 11,
	kHSWalkArea5	= 12,
	kHSWalkArea6	= 13,
	kHSWalkArea7	= 14,
	kHSWalkArea8	= 15,
	kHSWalkArea9	= 16
};

enum {
	kASLeaveScene		= 0,
	kASBackToilet		= 1,
	kASFrontToilet		= 2,
	kASLookScribble		= 6,
	kASGrabSink			= 7,
	kASGrabSinkDone		= 8,
	kASWait				= 12,
	kASGrabUrinal		= 13
};

Scene13::Scene13(GnapEngine *vm) : Scene(vm) {
	_s13_backToiletCtr = -1;
}

int Scene13::init() {
	_vm->playSound(0x108EC, false);
	return 0xAC;
}

void Scene13::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitBar, 113, 160, 170, 455, SF_EXIT_L_CURSOR);
	_vm->setHotspot(kHSBackToilet, 385, 195, 478, 367, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSFrontToilet, 497, 182, 545, 432, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSUrinal, 680, 265, 760, 445, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSScribble, 560, 270, 660, 370, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSSink, 310, 520, 560, 599, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSWalkArea1, 268, 270, 325, 385);
	_vm->setHotspot(kHSWalkArea2, 0, 0, 52, 599);
	_vm->setHotspot(kHSWalkArea3, 0, 0, 113, 550);
	_vm->setHotspot(kHSWalkArea4, 0, 0, 226, 438);
	_vm->setHotspot(kHSWalkArea5, 0, 0, 268, 400);
	_vm->setHotspot(kHSWalkArea6, 0, 0, 799, 367);
	_vm->setHotspot(kHSWalkArea7, 478, 0, 799, 401);
	_vm->setHotspot(kHSWalkArea8, 545, 0, 799, 473);
	_vm->setHotspot(kHSWalkArea9, 0, 549, 799, 599);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
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
		case kHSDevice:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				_vm->_timers[5] = _vm->getRandom(50) + 50;
			}
			break;

		case kHSPlatypus:
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

		case kHSExitBar:
			_vm->_isLeavingScene = true;
			_vm->gnapWalkTo(2, 7, 0, 0x107C0, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			_vm->platypusWalkTo(2, 8, -1, -1, 1);
			if (_vm->isFlag(kGFUnk14) || _vm->isFlag(kGFSpringTaken)) {
				_vm->_newSceneNum = 11;
			} else {
				_vm->setFlag(kGFSpringTaken);
				_vm->_newSceneNum = 47;
			}
			break;

		case kHSBackToilet:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(5, 5, 6, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
				case TALK_CURSOR:
					if (_vm->_gnapX == 5 && _vm->_gnapY == 5) {
						_s13_backToiletCtr = MIN(5, _s13_backToiletCtr + 1);
						_vm->_gameSys->setAnimation(_s13_backToiletCtr + 0xA3, _vm->_gnapId, 0);
						_vm->_gameSys->insertSequence(_s13_backToiletCtr + 0xA3, _vm->_gnapId,
							makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId,
							kSeqScale | kSeqSyncWait, 0, 0, 0);
						_vm->_gnapActionStatus = kASWait;
						_vm->_gnapSequenceId = _s13_backToiletCtr + 0xA3;
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->_gnapSequenceDatNum = 0;
					} else {
						_vm->gnapWalkTo(5, 5, 0, 0x107BB, 1);
						_vm->_gnapActionStatus = kASBackToilet;
						_vm->_gnapIdleFacing = kDirUpRight;
					}
					break;
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSFrontToilet:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(6, 7, 7, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->gnapWalkTo(6, 7, 0, 0xA9, 5);
					_vm->_gnapActionStatus = kASFrontToilet;
					_vm->_gnapIdleFacing = kDirBottomRight;
					break;
				}
			}
			break;

		case kHSScribble:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(7, 7, 8, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->gnapWalkTo(7, 7, 0, 0x107BB, 1);
					_vm->_gnapActionStatus = kASLookScribble;
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

		case kHSUrinal:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapShowCurrItem(8, 7, 9, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapSequence(_vm->getGnapSequenceId(gskDeflect, 9, 6));
					_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, -1, 1);
					_vm->_gnapActionStatus = kASWait;
					break;
				case GRAB_CURSOR:
					_vm->gnapWalkTo(8, 7, 0, -1, 1);
					_vm->_gnapActionStatus = kASGrabUrinal;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSSink:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playGnapImpossible(0, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playGnapSequence(_vm->getGnapSequenceId(gskDeflect, 5, 9));
					_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, -1, 1);
					_vm->_gnapActionStatus = kASWait;
					break;
				case GRAB_CURSOR:
					_vm->gnapWalkTo(4, 8, 0, 0x107B9, 1);
					_vm->_gnapActionStatus = kASGrabSink;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					_vm->playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSWalkArea2:
		case kHSWalkArea3:
		case kHSWalkArea4:
		case kHSWalkArea5:
		case kHSWalkArea6:
		case kHSWalkArea7:
		case kHSWalkArea8:
		case kHSWalkArea9:
			_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		case kHSWalkArea1:
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
		case kASLeaveScene:
			_vm->_sceneDone = true;
			_vm->_gnapActionStatus = -1;
			break;
		case kASBackToilet:
			_s13_backToiletCtr = MIN(5, _s13_backToiletCtr + 1);
			_vm->_gameSys->insertSequence(_s13_backToiletCtr + 0xA3, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, 9, 0, 0, 0);
			_vm->_gnapSequenceId = _s13_backToiletCtr + 0xA3;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = -1;
			break;
		case kASFrontToilet:
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 14;
			break;
		case kASLookScribble:
			_vm->_gnapActionStatus = -1;
			showScribble();
			break;
		case kASGrabSink:
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
			_vm->_gnapActionStatus = kASGrabSinkDone;
			break;
		case kASGrabSinkDone:
			_vm->_gameSys->insertSequence(0xAA, 256, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapActionStatus = -1;
			break;
		case kASWait:
			_vm->_gnapActionStatus = -1;
			break;
		case kASGrabUrinal:
			_vm->_gameSys->setAnimation(0xA2, 120, 0);
			_vm->_gameSys->insertSequence(0xA2, 120, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0xA2;
			_vm->_gnapId = 120;
			_vm->_gnapIdleFacing = kDirBottomLeft;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapX = 4;
			_vm->_gnapY = 6;
			_vm->_timers[2] = 360;
			_vm->_gnapActionStatus = kASWait;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 1);
		_vm->_platypusActionStatus = -1;
	}
}

} // End of namespace Gnap
