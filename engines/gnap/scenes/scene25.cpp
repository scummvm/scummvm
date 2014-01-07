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

int GnapEngine::scene25_init() {
	return 0x62;
}

void GnapEngine::scene25_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSTicketVendor, 416, 94, 574, 324, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 5);
	setHotspot(kHSExitOutsideCircusWorld, 0, 519, 205, 600, SF_EXIT_SW_CURSOR, 5, 10);
	setHotspot(kHSExitInsideCircusWorld, 321, 70, 388, 350, SF_EXIT_NE_CURSOR, 3, 6);
	setHotspot(kHSPosters1, 0, 170, 106, 326, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	setHotspot(kHSPosters2, 146, 192, 254, 306, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	setHotspot(kHSPosters3, 606, 162, 654, 368, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 7);
	setHotspot(kHSPosters4, 708, 114, 754, 490, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
	setHotspot(kHSWalkArea1, 0, 0, 800, 439);
	setHotspot(kHSWalkArea2, 585, 0, 800, 600);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 11;
}

void GnapEngine::scene25_playAnims(int index) {
	
	if (index > 4)
		return;

	hideCursor();
	setGrabCursorSprite(-1);
	switch (index) {
	case 1:
		_largeSprite = _gameSys->createSurface(0x25);
		break;
	case 2:
		_largeSprite = _gameSys->createSurface(0x26);
		break;
	case 3:
		_largeSprite = _gameSys->createSurface(0x27);
		break;
	case 4:
		_largeSprite = _gameSys->createSurface(0x28);
		break;
	}
	_gameSys->insertSpriteDrawItem(_largeSprite, 0, 0, 300);
	delayTicksCursor(5);
	while (!_mouseClickState.left && !isKeyStatus1(Common::KEYCODE_ESCAPE) && !isKeyStatus1(Common::KEYCODE_SPACE) &&
		!isKeyStatus1(29)) {
		gameUpdateTick();
	}
	_mouseClickState.left = false;
	clearKeyStatus1(Common::KEYCODE_ESCAPE);
	clearKeyStatus1(29);
	clearKeyStatus1(Common::KEYCODE_SPACE);
	_gameSys->removeSpriteDrawItem(_largeSprite, 300);
	delayTicksCursor(5);
	deleteSurface(&_largeSprite);
	showCursor();
}

void GnapEngine::scene25_run() {

	playSound(0x10940, 1);
	
	startSoundTimerA(5);
	
	_s25_currTicketVendorSequenceId = 0x52;
	_gameSys->setAnimation(0x52, 39, 3);
	_gameSys->insertSequence(_s25_currTicketVendorSequenceId, 39, 0, 0, kSeqNone, 0, 0, 0);
	
	_s25_nextTicketVendorSequenceId = -1;
	_timers[4] = getRandom(20) + 20;
	
	queueInsertDeviceIcon();
	
	if (_prevSceneNum == 24) {
		initGnapPos(5, 11, 5);
		initBeaverPos(6, 11, 4);
		endSceneInit();
		gnapWalkTo(5, 7, -1, 0x107BA, 1);
		platypusWalkTo(6, 7, -1, 0x107C2, 1);
	} else {
		initGnapPos(5, 6, 1);
		initBeaverPos(6, 6, 0);
		endSceneInit();
		gnapWalkTo(5, 8, -1, 0x107B9, 1);
		platypusWalkTo(6, 8, -1, 0x107C2, 1);
	}
	
	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene25_updateHotspots();
			}
			break;
		
		case kHSPlatypus:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemJoint) {
					gnapUseJointOnPlatypus();
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(0, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(_platX, _platY);
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
		
		case kHSTicketVendor:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemTickets) {
					_gnapActionStatus = kASShowTicketToVendor;
					gnapWalkTo(_hotspotsWalkPos[kHSTicketVendor].x, _hotspotsWalkPos[kHSTicketVendor].y,
						0, getGnapSequenceId(gskIdle, 9, 4) | 0x10000, 1);
					playGnapPullOutDevice(0, 0);
					playGnapUseDevice(0, 0);
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSTicketVendor].x, _hotspotsWalkPos[kHSTicketVendor].y, 6, 1);
					_s25_nextTicketVendorSequenceId = 0x5B;
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(6, 1);
						_s25_nextTicketVendorSequenceId = (getRandom(2) == 1) ? 0x59 : 0x56;
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 7;
						gnapWalkTo(_hotspotsWalkPos[kHSTicketVendor].x, _hotspotsWalkPos[kHSTicketVendor].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTalkTicketVendor;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSExitOutsideCircusWorld:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 24;
				gnapWalkTo(_hotspotsWalkPos[kHSExitOutsideCircusWorld].x, _hotspotsWalkPos[kHSExitOutsideCircusWorld].y, 0, 0x107B4, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitOutsideCircusWorld].x + 1, _hotspotsWalkPos[kHSExitOutsideCircusWorld].y, -1, 0x107C2, 1);
			}
			break;
		
		case kHSExitInsideCircusWorld:
			if (_gnapActionStatus < 0) {
				if (isFlag(2)) {
					_isLeavingScene = true;
					_newSceneNum = 26;
					_hotspots[kHSWalkArea1].flags |= SF_WALKABLE;
					gnapWalkTo(_hotspotsWalkPos[kHSExitInsideCircusWorld].x, _hotspotsWalkPos[kHSExitInsideCircusWorld].y, 0, 0x107B1, 1);
					_gnapActionStatus = kASLeaveScene;
					platypusWalkTo(_hotspotsWalkPos[kHSExitInsideCircusWorld].x + 1, _hotspotsWalkPos[kHSExitInsideCircusWorld].y, -1, 0x107C2, 1);
					_hotspots[kHSWalkArea1].flags &= ~SF_WALKABLE;
				} else {
					_hotspots[kHSWalkArea1].flags |= SF_WALKABLE;
					gnapWalkTo(4, 5, 0, 0x107BB, 1);
					_gnapActionStatus = kASEnterCircusWihoutTicket;
					_hotspots[kHSWalkArea1].flags &= ~SF_WALKABLE;
				}
			}
			break;
		
		case kHSPosters1:
		case kHSPosters2:
		case kHSPosters3:
		case kHSPosters4:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(0, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						gnapWalkTo(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, -1, -1, 1);
						if (_sceneClickedHotspot == 5 || _sceneClickedHotspot == 6)
							_gnapIdleFacing = 5;
						else if (_sceneClickedHotspot == 8)
							_gnapIdleFacing = 1;
						else
							_gnapIdleFacing = 7;
						playGnapIdle(0, 0);
						scene25_playAnims(8 - _sceneClickedHotspot + 1);
						break;
					case GRAB_CURSOR:
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapMoan2(0, 0);
						break;
					}
				}
			}
			break;
		
		case kHSWalkArea1:
		case kHSWalkArea2:
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;
		
		default:
			if (_mouseClickState.left) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState.left = false;
			}
			break;
		
		}
	
		scene25_updateAnimations();
	
		if (!_isLeavingScene) {
			updateBeaverIdleSequence();
			updateGnapIdleSequence();
			if (!_timers[4] && _s25_nextTicketVendorSequenceId == -1 && _gnapActionStatus == -1) {
				_timers[4] = getRandom(20) + 20;
				switch (getRandom(13)) {
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
			playSoundA();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene25_updateHotspots();
		}
		
		gameUpdateTick();
		
	}

}

void GnapEngine::scene25_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASTalkTicketVendor:
			_s25_nextTicketVendorSequenceId = (getRandom(2) == 1) ? 0x57 : 0x5F;
			_gnapActionStatus = -1;
			break;
		case kASEnterCircusWihoutTicket:
			_s25_nextTicketVendorSequenceId = 0x5E;
			_gameSys->setAnimation(0x5E, 39, 0);
			_gameSys->setAnimation(_s25_nextTicketVendorSequenceId, 39, 3);
			_gameSys->insertSequence(_s25_nextTicketVendorSequenceId, 39, _s25_currTicketVendorSequenceId, 39, 32, 0, 0, 0);
			_gameSys->insertSequence(0x60, 2, 0, 0, kSeqNone, 0, 0, 0);
			_s25_currTicketVendorSequenceId = _s25_nextTicketVendorSequenceId;
			_s25_nextTicketVendorSequenceId = -1;
			_hotspots[kHSWalkArea1].flags |= SF_WALKABLE;
			playGnapIdle(0, 0);
			gnapWalkTo(_hotspotsWalkPos[3].x, _hotspotsWalkPos[3].y, -1, 0x107BB, 1);
			_hotspots[kHSWalkArea1].flags &= ~SF_WALKABLE;
			_gnapActionStatus = kASEnterCircusWihoutTicketDone;
			break;
		case kASEnterCircusWihoutTicketDone:
			_gnapActionStatus = -1;
			break;
		case kASShowTicketToVendor:
			setGrabCursorSprite(-1);
			invRemove(kItemTickets);
			setFlag(2);
			_gameSys->setAnimation(0x61, 40, 0);
			_gameSys->insertSequence(0x61, 40, 0, 0, kSeqNone, 0, 0, 0);
			_gnapActionStatus = kASShowTicketToVendorDone;
			break;
		case kASShowTicketToVendorDone:
			_s25_nextTicketVendorSequenceId = 0x53;
			break;
		case kASLeaveScene:
			_sceneDone = true;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(3) == 2) {
		if (_s25_nextTicketVendorSequenceId == 0x53) {
			_gameSys->insertSequence(_s25_nextTicketVendorSequenceId, 39, _s25_currTicketVendorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_s25_currTicketVendorSequenceId = _s25_nextTicketVendorSequenceId;
			_s25_nextTicketVendorSequenceId = -1;
			_gnapActionStatus = -1;
		} else if (_s25_nextTicketVendorSequenceId != -1) {
			_gameSys->setAnimation(_s25_nextTicketVendorSequenceId, 39, 3);
			_gameSys->insertSequence(_s25_nextTicketVendorSequenceId, 39, _s25_currTicketVendorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_s25_currTicketVendorSequenceId = _s25_nextTicketVendorSequenceId;
			_s25_nextTicketVendorSequenceId = -1;
		}
	}

}

} // End of namespace Gnap
