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

int GnapEngine::scene15_init() {
	return 0xDD;
}

void GnapEngine::scene15_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
	setHotspot(kHSExit, 50, 590, 750, 599, SF_EXIT_D_CURSOR);
	setHotspot(kHSButton1, 210, 425, 260, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSButton2, 280, 425, 325, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSButton3, 340, 425, 385, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSButton4, 400, 425, 445, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSButton5, 460, 425, 510, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSButton6, 520, 425, 560, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSButtonA, 205, 480, 250, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSButtonB, 270, 480, 320, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSButtonC, 335, 480, 380, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSButtonD, 395, 480, 445, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSButtonE, 460, 480, 505, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSButtonF, 515, 480, 560, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSCoinSlot, 585, 475, 620, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSPlayButton, 622, 431, 650, 482, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 17;
}

void GnapEngine::scene15_run() {
	
	_s15_currSlotSequenceId = -1;
	_s15_currUpperButtonSequenceId = -1;
	_s15_currLowerButtonSequenceId = -1;
	_s15_nextSlotSequenceId = -1;
	_s15_nextUpperButtonSequenceId = -1;
	_s15_nextLowerButtonSequenceId = -1;
	_s15_currRecordSequenceId = 0xD5;
	_s15_nextRecordSequenceId = -1;

	_gameSys->setAnimation(0xD5, 1, 0);
	_gameSys->insertSequence(_s15_currRecordSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	queueInsertDeviceIcon();
	
	endSceneInit();
	
	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		_hotspots[kHSPlatypus].x1 = 0;
		_hotspots[kHSPlatypus].y1 = 0;
		_hotspots[kHSPlatypus].x2 = 0;
		_hotspots[kHSPlatypus].y2 = 0;
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
		
		if (_sceneClickedHotspot >= 0)
			debug("_sceneClickedHotspot: %d; _verbCursor: %d", _sceneClickedHotspot, _verbCursor);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene15_updateHotspots();
			}
			break;

		case kHSExit:
			_newSceneNum = 12;
			_isLeavingScene = true;
			break;

		case kHSCoinSlot:
			if (_grabCursorSpriteIndex == kItemQuarter || _grabCursorSpriteIndex == kItemQuarterWithHole) {
				_s15_nextSlotSequenceId = 0xDC; // Insert coin
			} else if (_grabCursorSpriteIndex == kItemDiceQuarterHole) {
				_s15_nextSlotSequenceId = 0xDB;
			} else if (_grabCursorSpriteIndex >= 0) {
				_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _gnapGridX, 576 - _gnapGridY);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
					playSound(0x108E9, 0);
					break;
				case TALK_CURSOR:
					playSound((getRandom(5) + 0x8D5) | 0x10000, 0);
					break;
				case PLAT_CURSOR:
					_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _gnapGridX, 576 - _gnapGridY);
					break;
				}
			}
			break;

		case kHSPlayButton:
			if (_grabCursorSpriteIndex >= 0) {
				_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _gnapGridX, 576 - _gnapGridY);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					if (isFlag(12) || isFlag(13))
						playSound(0x108E9, 0);
					else
						_s15_nextSlotSequenceId = 0xDA;
					break;
				case GRAB_CURSOR:
					if (isFlag(12) || isFlag(13))
						_s15_nextSlotSequenceId = 0xD9;
					else
						_s15_nextSlotSequenceId = 0xDA;
					break;
				case TALK_CURSOR:
					playSound((getRandom(5) + 0x8D5) | 0x10000, 0);
					break;
				case PLAT_CURSOR:
					_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _gnapGridX, 576 - _gnapGridY);
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
			if (_grabCursorSpriteIndex >= 0) {
				_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _gnapGridX, 576 - _gnapGridY);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playSound(0x108E9, 0);
					break;
				case GRAB_CURSOR:
					_s15_nextUpperButtonSequenceId = _sceneClickedHotspot + 0xC5;
					break;
				case TALK_CURSOR:
					playSound((getRandom(5) + 0x8D5) | 0x10000, 0);
					break;
				case PLAT_CURSOR:
					_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _gnapGridX, 576 - _gnapGridY);
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
			if (_grabCursorSpriteIndex >= 0) {
				_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _gnapGridX, 576 - _gnapGridY);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playSound(0x108E9, 0);
					break;
				case GRAB_CURSOR:
					_s15_nextLowerButtonSequenceId = _sceneClickedHotspot + 0xC5;
					break;
				case TALK_CURSOR:
					playSound((getRandom(5) + 0x8D5) | 0x10000, 0);
					break;
				case PLAT_CURSOR:
					_gameSys->insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - _gnapGridX, 576 - _gnapGridY);
					break;
				}
			}
			break;

		default:
			_mouseClickState.left = false;
			break;

		}
	
		scene15_updateAnimations();
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene15_updateHotspots();
		}
		
		gameUpdateTick();
		
	}

}

void GnapEngine::scene15_updateAnimations() {
	
	if (_gameSys->getAnimationStatus(0) == 2) {
		if (_isLeavingScene) {
			_sceneDone = true;
		} else if (_s15_nextSlotSequenceId != -1) {
			_gameSys->setAnimation(_s15_nextSlotSequenceId, 1, 0);
			_gameSys->insertSequence(_s15_nextSlotSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
			_s15_currSlotSequenceId = _s15_nextSlotSequenceId;
			_s15_nextSlotSequenceId = -1;
			switch (_s15_currSlotSequenceId) {
			case 0xDC:
				if (_grabCursorSpriteIndex == kItemQuarter) {
					invRemove(kItemQuarter);
				} else {
					invRemove(kItemQuarterWithHole);
					setFlag(13);
				}
				setGrabCursorSprite(-1);
				break;
			case 0xDB:
				setFlag(14);
				setGrabCursorSprite(-1);
				_s15_nextSlotSequenceId = 0xD8;
				break;
			case 0xD9:
				if (isFlag(12)) {
					clearFlag(12);
					invAdd(kItemQuarter);
					_newGrabCursorSpriteIndex = kItemQuarter;
				} else if (isFlag(13)) {
					clearFlag(13);
					invAdd(kItemQuarterWithHole);
					_newGrabCursorSpriteIndex = kItemQuarterWithHole;
				}
				_newSceneNum = 12;
				_isLeavingScene = true;
				break;
			case 0xD8:
			case 0xDA:
				if (_s15_currUpperButtonSequenceId != -1) {
					_gameSys->removeSequence(_s15_currUpperButtonSequenceId, 1, true);
					_s15_currUpperButtonSequenceId = -1;
				}
				if (_s15_currLowerButtonSequenceId != -1) {
					_gameSys->removeSequence(_s15_currLowerButtonSequenceId, 1, true);
					_s15_currLowerButtonSequenceId = -1;
				}
				break;
			}
		} else if (_s15_nextRecordSequenceId != -1) {
			_gameSys->setAnimation(_s15_nextRecordSequenceId, 1, 0);
			_gameSys->insertSequence(_s15_nextRecordSequenceId, 1, _s15_currRecordSequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_s15_currRecordSequenceId = _s15_nextRecordSequenceId;
			_s15_nextRecordSequenceId = -1;
			if (_s15_currRecordSequenceId == 0xD3) {
				invRemove(kItemDiceQuarterHole);
				_newSceneNum = 16;
				_isLeavingScene = true;
			}
			_gameSys->removeSequence(_s15_currUpperButtonSequenceId, 1, true);
			_s15_currUpperButtonSequenceId = -1;
			_gameSys->removeSequence(_s15_currLowerButtonSequenceId, 1, true);
			_s15_currLowerButtonSequenceId = -1;
		} else if (_s15_nextUpperButtonSequenceId != -1) {
			_gameSys->setAnimation(_s15_nextUpperButtonSequenceId, 1, 0);
			if (_s15_currUpperButtonSequenceId == -1)
				_gameSys->insertSequence(_s15_nextUpperButtonSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
			else
				_gameSys->insertSequence(_s15_nextUpperButtonSequenceId, 1, _s15_currUpperButtonSequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_s15_currUpperButtonSequenceId = _s15_nextUpperButtonSequenceId;
			_s15_nextUpperButtonSequenceId = -1;
			if (_s15_currLowerButtonSequenceId != -1 && isFlag(14)) {
				if (_s15_currUpperButtonSequenceId == 0xCC && _s15_currLowerButtonSequenceId == 0xCE)
					_s15_nextRecordSequenceId = 0xD3;
				else
					_s15_nextRecordSequenceId = 0xD4;
			}
		} else if (_s15_nextLowerButtonSequenceId != -1) {
			_gameSys->setAnimation(_s15_nextLowerButtonSequenceId, 1, 0);
			if (_s15_currLowerButtonSequenceId == -1)
				_gameSys->insertSequence(_s15_nextLowerButtonSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
			else
				_gameSys->insertSequence(_s15_nextLowerButtonSequenceId, 1, _s15_currLowerButtonSequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_s15_currLowerButtonSequenceId = _s15_nextLowerButtonSequenceId;
			_s15_nextLowerButtonSequenceId = -1;
			if (_s15_currUpperButtonSequenceId != -1 && isFlag(14)) {
				if (_s15_currUpperButtonSequenceId == 0xCC && _s15_currLowerButtonSequenceId == 0xCE)
					_s15_nextRecordSequenceId = 0xD3;
				else
					_s15_nextRecordSequenceId = 0xD4;
			}
		}
	}
	
}

} // End of namespace Gnap
