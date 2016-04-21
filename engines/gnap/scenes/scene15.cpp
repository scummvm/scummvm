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
#include "gnap/scenes/scene15.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSExit			= 1,
	kHSButton1		= 2,
	kHSButton2		= 3,
	kHSButton3		= 4,
	kHSButton4		= 5,
	kHSButton5		= 6,
	kHSButton6		= 7,
	kHSButtonA		= 8,
	kHSButtonB		= 9,
	kHSButtonC		= 10,
	kHSButtonD		= 11,
	kHSButtonE		= 12,
	kHSButtonF		= 13,
	kHSCoinSlot		= 14,
	kHSPlayButton	= 15,
	kHSDevice		= 16
};

Scene15::Scene15(GnapEngine *vm) : Scene(vm) {
	_s15_nextRecordSequenceId = -1;
	_s15_currRecordSequenceId = -1;
	_s15_nextSlotSequenceId = -1;
	_s15_currSlotSequenceId = -1;
	_s15_nextUpperButtonSequenceId = -1;
	_s15_currUpperButtonSequenceId = -1;
	_s15_nextLowerButtonSequenceId = -1;
	_s15_currLowerButtonSequenceId = -1;
}

int Scene15::init() {
	return 0xDD;
}

void Scene15::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
	_vm->setHotspot(kHSExit, 50, 590, 750, 599, SF_EXIT_D_CURSOR);
	_vm->setHotspot(kHSButton1, 210, 425, 260, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSButton2, 280, 425, 325, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSButton3, 340, 425, 385, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSButton4, 400, 425, 445, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSButton5, 460, 425, 510, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSButton6, 520, 425, 560, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSButtonA, 205, 480, 250, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSButtonB, 270, 480, 320, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSButtonC, 335, 480, 380, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSButtonD, 395, 480, 445, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSButtonE, 460, 480, 505, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSButtonF, 515, 480, 560, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSCoinSlot, 585, 475, 620, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSPlayButton, 622, 431, 650, 482, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_vm->_hotspotsCount = 17;
}

void Scene15::run() {
	_s15_currSlotSequenceId = -1;
	_s15_currUpperButtonSequenceId = -1;
	_s15_currLowerButtonSequenceId = -1;
	_s15_nextSlotSequenceId = -1;
	_s15_nextUpperButtonSequenceId = -1;
	_s15_nextLowerButtonSequenceId = -1;
	_s15_currRecordSequenceId = 0xD5;
	_s15_nextRecordSequenceId = -1;

	_vm->_gameSys->setAnimation(0xD5, 1, 0);
	_vm->_gameSys->insertSequence(_s15_currRecordSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->queueInsertDeviceIcon();
	
	_vm->endSceneInit();
	
	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->_hotspots[kHSPlatypus]._x1 = 0;
		_vm->_hotspots[kHSPlatypus]._y1 = 0;
		_vm->_hotspots[kHSPlatypus]._x2 = 0;
		_vm->_hotspots[kHSPlatypus]._y2 = 0;
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
		
		switch (_vm->_sceneClickedHotspot) {
		case kHSDevice:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHSExit:
			_vm->_newSceneNum = 12;
			_vm->_isLeavingScene = true;
			break;

		case kHSCoinSlot:
			if (_vm->_grabCursorSpriteIndex == kItemQuarter || _vm->_grabCursorSpriteIndex == kItemQuarterWithHole) {
				_s15_nextSlotSequenceId = 0xDC; // Insert coin
			} else if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
				_s15_nextSlotSequenceId = 0xDB;
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

		case kHSPlayButton:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					if (_vm->isFlag(kGFGnapControlsToyUFO) || _vm->isFlag(kGFUnk13))
						_vm->playSound(0x108E9, false);
					else
						_s15_nextSlotSequenceId = 0xDA;
					break;
				case GRAB_CURSOR:
					if (_vm->isFlag(kGFGnapControlsToyUFO) || _vm->isFlag(kGFUnk13))
						_s15_nextSlotSequenceId = 0xD9;
					else
						_s15_nextSlotSequenceId = 0xDA;
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

		case kHSButton1:
		case kHSButton2:
		case kHSButton3:
		case kHSButton4:
		case kHSButton5:
		case kHSButton6:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playSound(0x108E9, false);
					break;
				case GRAB_CURSOR:
					_s15_nextUpperButtonSequenceId = _vm->_sceneClickedHotspot + 0xC5;
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

		case kHSButtonA:
		case kHSButtonB:
		case kHSButtonC:
		case kHSButtonD:
		case kHSButtonE:
		case kHSButtonF:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _vm->_gnapGridX, 576 - _vm->_gnapGridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playSound(0x108E9, false);
					break;
				case GRAB_CURSOR:
					_s15_nextLowerButtonSequenceId = _vm->_sceneClickedHotspot + 0xC5;
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
		} else if (_s15_nextSlotSequenceId != -1) {
			_vm->_gameSys->setAnimation(_s15_nextSlotSequenceId, 1, 0);
			_vm->_gameSys->insertSequence(_s15_nextSlotSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
			_s15_currSlotSequenceId = _s15_nextSlotSequenceId;
			_s15_nextSlotSequenceId = -1;
			switch (_s15_currSlotSequenceId) {
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
				_s15_nextSlotSequenceId = 0xD8;
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
				if (_s15_currUpperButtonSequenceId != -1) {
					_vm->_gameSys->removeSequence(_s15_currUpperButtonSequenceId, 1, true);
					_s15_currUpperButtonSequenceId = -1;
				}
				if (_s15_currLowerButtonSequenceId != -1) {
					_vm->_gameSys->removeSequence(_s15_currLowerButtonSequenceId, 1, true);
					_s15_currLowerButtonSequenceId = -1;
				}
				break;
			}
		} else if (_s15_nextRecordSequenceId != -1) {
			_vm->_gameSys->setAnimation(_s15_nextRecordSequenceId, 1, 0);
			_vm->_gameSys->insertSequence(_s15_nextRecordSequenceId, 1, _s15_currRecordSequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_s15_currRecordSequenceId = _s15_nextRecordSequenceId;
			_s15_nextRecordSequenceId = -1;
			if (_s15_currRecordSequenceId == 0xD3) {
				_vm->invRemove(kItemDiceQuarterHole);
				_vm->_newSceneNum = 16;
				_vm->_isLeavingScene = true;
			}
			_vm->_gameSys->removeSequence(_s15_currUpperButtonSequenceId, 1, true);
			_s15_currUpperButtonSequenceId = -1;
			_vm->_gameSys->removeSequence(_s15_currLowerButtonSequenceId, 1, true);
			_s15_currLowerButtonSequenceId = -1;
		} else if (_s15_nextUpperButtonSequenceId != -1) {
			_vm->_gameSys->setAnimation(_s15_nextUpperButtonSequenceId, 1, 0);
			if (_s15_currUpperButtonSequenceId == -1)
				_vm->_gameSys->insertSequence(_s15_nextUpperButtonSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
			else
				_vm->_gameSys->insertSequence(_s15_nextUpperButtonSequenceId, 1, _s15_currUpperButtonSequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_s15_currUpperButtonSequenceId = _s15_nextUpperButtonSequenceId;
			_s15_nextUpperButtonSequenceId = -1;
			if (_s15_currLowerButtonSequenceId != -1 && _vm->isFlag(kGFUnk14)) {
				if (_s15_currUpperButtonSequenceId == 0xCC && _s15_currLowerButtonSequenceId == 0xCE)
					_s15_nextRecordSequenceId = 0xD3;
				else
					_s15_nextRecordSequenceId = 0xD4;
			}
		} else if (_s15_nextLowerButtonSequenceId != -1) {
			_vm->_gameSys->setAnimation(_s15_nextLowerButtonSequenceId, 1, 0);
			if (_s15_currLowerButtonSequenceId == -1)
				_vm->_gameSys->insertSequence(_s15_nextLowerButtonSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
			else
				_vm->_gameSys->insertSequence(_s15_nextLowerButtonSequenceId, 1, _s15_currLowerButtonSequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_s15_currLowerButtonSequenceId = _s15_nextLowerButtonSequenceId;
			_s15_nextLowerButtonSequenceId = -1;
			if (_s15_currUpperButtonSequenceId != -1 && _vm->isFlag(kGFUnk14)) {
				if (_s15_currUpperButtonSequenceId == 0xCC && _s15_currLowerButtonSequenceId == 0xCE)
					_s15_nextRecordSequenceId = 0xD3;
				else
					_s15_nextRecordSequenceId = 0xD4;
			}
		}
	}
}

} // End of namespace Gnap
