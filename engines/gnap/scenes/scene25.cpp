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
#include "gnap/scenes/scene25.h"

namespace Gnap {

enum {
	kHSPlatypus					= 0,
	kHSTicketVendor				= 1,
	kHSExitOutsideCircusWorld	= 2,
	kHSExitInsideCircusWorld	= 3,
	kHSDevice					= 4,
	kHSPosters1					= 5,
	kHSPosters2					= 6,
	kHSPosters3					= 7,
	kHSPosters4					= 8,
	kHSWalkArea1				= 9,
	kHSWalkArea2				= 10
};

enum {
	kASTalkTicketVendor				= 0,
	kASEnterCircusWihoutTicket		= 1,
	kASShowTicketToVendor			= 2,
	kASShowTicketToVendorDone		= 3,
	kASEnterCircusWihoutTicketDone	= 4,
	kASLeaveScene					= 5
};

Scene25::Scene25(GnapEngine *vm) : Scene(vm) {
	_s25_currTicketVendorSequenceId = -1;
	_s25_nextTicketVendorSequenceId = -1;
}

int Scene25::init() {
	return 0x62;
}

void Scene25::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSTicketVendor, 416, 94, 574, 324, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 5);
	_vm->setHotspot(kHSExitOutsideCircusWorld, 0, 519, 205, 600, SF_EXIT_SW_CURSOR, 5, 10);
	_vm->setHotspot(kHSExitInsideCircusWorld, 321, 70, 388, 350, SF_EXIT_NE_CURSOR, 3, 6);
	_vm->setHotspot(kHSPosters1, 0, 170, 106, 326, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	_vm->setHotspot(kHSPosters2, 146, 192, 254, 306, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	_vm->setHotspot(kHSPosters3, 606, 162, 654, 368, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 7);
	_vm->setHotspot(kHSPosters4, 708, 114, 754, 490, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 439);
	_vm->setHotspot(kHSWalkArea2, 585, 0, 800, 600);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 11;
}

void Scene25::playAnims(int index) {
	if (index > 4)
		return;

	_vm->hideCursor();
	_vm->setGrabCursorSprite(-1);
	switch (index) {
	case 1:
		_vm->_largeSprite = _vm->_gameSys->createSurface(0x25);
		break;
	case 2:
		_vm->_largeSprite = _vm->_gameSys->createSurface(0x26);
		break;
	case 3:
		_vm->_largeSprite = _vm->_gameSys->createSurface(0x27);
		break;
	case 4:
		_vm->_largeSprite = _vm->_gameSys->createSurface(0x28);
		break;
	}
	_vm->_gameSys->insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
	_vm->delayTicksCursor(5);
	while (!_vm->_mouseClickState._left && !_vm->isKeyStatus1(Common::KEYCODE_ESCAPE) && !_vm->isKeyStatus1(Common::KEYCODE_SPACE) &&
		!_vm->isKeyStatus1(29)) {
		_vm->gameUpdateTick();
	}
	_vm->_mouseClickState._left = false;
	_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
	_vm->clearKeyStatus1(29);
	_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
	_vm->_gameSys->removeSpriteDrawItem(_vm->_largeSprite, 300);
	_vm->delayTicksCursor(5);
	_vm->deleteSurface(&_vm->_largeSprite);
	_vm->showCursor();
}

void Scene25::run() {
	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(5);
	
	_s25_currTicketVendorSequenceId = 0x52;
	_vm->_gameSys->setAnimation(0x52, 39, 3);
	_vm->_gameSys->insertSequence(_s25_currTicketVendorSequenceId, 39, 0, 0, kSeqNone, 0, 0, 0);
	
	_s25_nextTicketVendorSequenceId = -1;
	_vm->_timers[4] = _vm->getRandom(20) + 20;
	
	_vm->queueInsertDeviceIcon();
	
	if (_vm->_prevSceneNum == 24) {
		_vm->initGnapPos(5, 11, kDirUpLeft);
		_vm->initPlatypusPos(6, 11, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(5, 7, -1, 0x107BA, 1);
		_vm->platypusWalkTo(6, 7, -1, 0x107C2, 1);
	} else {
		_vm->initGnapPos(5, 6, kDirBottomRight);
		_vm->initPlatypusPos(6, 6, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(5, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
	}
	
	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
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
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(_vm->_platX, _vm->_platY);
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
		
		case kHSTicketVendor:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemTickets) {
					_vm->_gnapActionStatus = kASShowTicketToVendor;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSTicketVendor].x, _vm->_hotspotsWalkPos[kHSTicketVendor].y,
						0, _vm->getGnapSequenceId(gskIdle, 9, 4) | 0x10000, 1);
					_vm->playGnapPullOutDevice(0, 0);
					_vm->playGnapUseDevice(0, 0);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSTicketVendor].x, _vm->_hotspotsWalkPos[kHSTicketVendor].y, 6, 1);
					_s25_nextTicketVendorSequenceId = 0x5B;
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(6, 1);
						_s25_nextTicketVendorSequenceId = (_vm->getRandom(2) == 1) ? 0x59 : 0x56;
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSTicketVendor].x, _vm->_hotspotsWalkPos[kHSTicketVendor].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkTicketVendor;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSExitOutsideCircusWorld:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 24;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideCircusWorld].x, _vm->_hotspotsWalkPos[kHSExitOutsideCircusWorld].y, 0, 0x107B4, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitOutsideCircusWorld].x + 1, _vm->_hotspotsWalkPos[kHSExitOutsideCircusWorld].y, -1, 0x107C2, 1);
			}
			break;
		
		case kHSExitInsideCircusWorld:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFNeedleTaken)) {
					_vm->_isLeavingScene = true;
					_vm->_newSceneNum = 26;
					_vm->_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitInsideCircusWorld].x, _vm->_hotspotsWalkPos[kHSExitInsideCircusWorld].y, 0, 0x107B1, 1);
					_vm->_gnapActionStatus = kASLeaveScene;
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitInsideCircusWorld].x + 1, _vm->_hotspotsWalkPos[kHSExitInsideCircusWorld].y, -1, 0x107C2, 1);
					_vm->_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
				} else {
					_vm->_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
					_vm->gnapWalkTo(4, 5, 0, 0x107BB, 1);
					_vm->_gnapActionStatus = kASEnterCircusWihoutTicket;
					_vm->_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
				}
			}
			break;
		
		case kHSPosters1:
		case kHSPosters2:
		case kHSPosters3:
		case kHSPosters4:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, -1, -1, 1);
						if (_vm->_sceneClickedHotspot == 5 || _vm->_sceneClickedHotspot == 6)
							_vm->_gnapIdleFacing = kDirUpLeft;
						else if (_vm->_sceneClickedHotspot == 8)
							_vm->_gnapIdleFacing = kDirBottomRight;
						else
							_vm->_gnapIdleFacing = kDirUpRight;
						_vm->playGnapIdle(0, 0);
						playAnims(8 - _vm->_sceneClickedHotspot + 1);
						break;
					case GRAB_CURSOR:
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapMoan2(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSWalkArea1:
		case kHSWalkArea2:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
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
			_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4] && _s25_nextTicketVendorSequenceId == -1 && _vm->_gnapActionStatus == -1) {
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				switch (_vm->getRandom(13)) {
				case 0:
					_s25_nextTicketVendorSequenceId = 0x54;
					break;
				case 1:
					_s25_nextTicketVendorSequenceId = 0x58;
					break;
				case 2:
					_s25_nextTicketVendorSequenceId = 0x55;
					break;
				case 3:
					_s25_nextTicketVendorSequenceId = 0x5A;
					break;
				case 4:
				case 5:
				case 6:
				case 7:
					_s25_nextTicketVendorSequenceId = 0x5B;
					break;
				case 8:
				case 9:
				case 10:
				case 11:
					_s25_nextTicketVendorSequenceId = 0x5C;
					break;
				case 12:
					_s25_nextTicketVendorSequenceId = 0x5D;
					break;
				default:
					_s25_nextTicketVendorSequenceId = 0x52;
					break;
				}
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

void Scene25::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASTalkTicketVendor:
			_s25_nextTicketVendorSequenceId = (_vm->getRandom(2) == 1) ? 0x57 : 0x5F;
			_vm->_gnapActionStatus = -1;
			break;
		case kASEnterCircusWihoutTicket:
			_s25_nextTicketVendorSequenceId = 0x5E;
			_vm->_gameSys->setAnimation(0x5E, 39, 0);
			_vm->_gameSys->setAnimation(_s25_nextTicketVendorSequenceId, 39, 3);
			_vm->_gameSys->insertSequence(_s25_nextTicketVendorSequenceId, 39, _s25_currTicketVendorSequenceId, 39, kSeqSyncExists, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x60, 2, 0, 0, kSeqNone, 0, 0, 0);
			_s25_currTicketVendorSequenceId = _s25_nextTicketVendorSequenceId;
			_s25_nextTicketVendorSequenceId = -1;
			_vm->_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
			_vm->playGnapIdle(0, 0);
			_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, -1, 0x107BB, 1);
			_vm->_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
			_vm->_gnapActionStatus = kASEnterCircusWihoutTicketDone;
			break;
		case kASEnterCircusWihoutTicketDone:
			_vm->_gnapActionStatus = -1;
			break;
		case kASShowTicketToVendor:
			_vm->setGrabCursorSprite(-1);
			_vm->invRemove(kItemTickets);
			_vm->setFlag(kGFNeedleTaken);
			_vm->_gameSys->setAnimation(0x61, 40, 0);
			_vm->_gameSys->insertSequence(0x61, 40, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_gnapActionStatus = kASShowTicketToVendorDone;
			break;
		case kASShowTicketToVendorDone:
			_s25_nextTicketVendorSequenceId = 0x53;
			break;
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		if (_s25_nextTicketVendorSequenceId == 0x53) {
			_vm->_gameSys->insertSequence(_s25_nextTicketVendorSequenceId, 39, _s25_currTicketVendorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_s25_currTicketVendorSequenceId = _s25_nextTicketVendorSequenceId;
			_s25_nextTicketVendorSequenceId = -1;
			_vm->_gnapActionStatus = -1;
		} else if (_s25_nextTicketVendorSequenceId != -1) {
			_vm->_gameSys->setAnimation(_s25_nextTicketVendorSequenceId, 39, 3);
			_vm->_gameSys->insertSequence(_s25_nextTicketVendorSequenceId, 39, _s25_currTicketVendorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_s25_currTicketVendorSequenceId = _s25_nextTicketVendorSequenceId;
			_s25_nextTicketVendorSequenceId = -1;
		}
	}
}

} // End of namespace Gnap
