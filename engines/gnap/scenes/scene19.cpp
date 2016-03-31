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

static const int kS19ShopAssistantSequenceIds[] = {
	0x6F, 0x70, 0x71, 0x72, 0x73
};

enum {
	kHSPlatypus				= 0,
	kHSExitOutsideToyStore	= 1,
	kHSDevice				= 2,
	kHSPicture				= 3,
	kHSShopAssistant		= 4,
	kHSToy1					= 5,
	kHSToy2					= 6,
	kHSToy3					= 7,
	kHSPhone				= 8,
	kHSToy4					= 9,
	kHSToy5					= 10,
	kHSToy6					= 11,
	kHSToy7					= 12,
	kHSWalkArea1			= 13,
	kHSWalkArea2			= 14,
	kHSWalkArea3			= 15
};

enum {
	kASUsePhone					= 0,
	kASGrabToy					= 1,
	kASGrabPicture				= 2,
	kASGrabPictureDone			= 3,
	kASTalkShopAssistant		= 4,
	kASLeaveScene				= 5
};

int GnapEngine::scene19_init() {
	playSound(0x79, 0);
	return isFlag(26) ? 0x77 : 0x76;
}

void GnapEngine::scene19_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitOutsideToyStore, 36, 154, 142, 338, SF_EXIT_NW_CURSOR, 4, 6);
	setHotspot(kHSPicture, 471, 237, 525, 283, SF_DISABLED, 7, 2);
	setHotspot(kHSShopAssistant, 411, 151, 575, 279, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
	setHotspot(kHSPhone, 647, 166, 693, 234, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 0);
	setHotspot(kHSToy1, 181, 11, 319, 149, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 0);
	setHotspot(kHSToy2, 284, 85, 611, 216, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 0);
	setHotspot(kHSToy3, 666, 38, 755, 154, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 0);
	setHotspot(kHSToy4, 154, 206, 285, 327, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 3);
	setHotspot(kHSToy5, 494, 301, 570, 448, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 5);
	setHotspot(kHSToy6, 0, 320, 188, 600, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 6);
	setHotspot(kHSToy7, 597, 434, 800, 600, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 8);
	setHotspot(kHSWalkArea1, 0, 0, 170, 600);
	setHotspot(kHSWalkArea2, 622, 0, 800, 600);
	setHotspot(kHSWalkArea3, 0, 0, 800, 437);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (isFlag(26)) {
		_hotspots[kHSToy1]._flags = SF_DISABLED;
		_hotspots[kHSToy2]._flags = SF_DISABLED;
		_hotspots[kHSToy3]._flags = SF_DISABLED;
		_hotspots[kHSToy4]._flags = SF_DISABLED;
		_hotspots[kHSToy5]._flags = SF_DISABLED;
		_hotspots[kHSToy6]._flags = SF_DISABLED;
		_hotspots[kHSToy7]._flags = SF_DISABLED;
		_hotspots[kHSShopAssistant]._flags = SF_DISABLED;
		_hotspots[kHSPhone]._flags = SF_DISABLED;
		_hotspots[kHSPlatypus]._flags = SF_DISABLED;
		_hotspots[kHSPicture]._flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	}
	_hotspotsCount = 16;
}

void GnapEngine::scene19_run() {

	queueInsertDeviceIcon();

	_s19_toyGrabCtr = 0;
	_s19_pictureSurface = 0;

	_gameSys->insertSequence(0x74, 254, 0, 0, kSeqNone, 0, 0, 0);
	_gameSys->insertSequence(0x75, 254, 0, 0, kSeqNone, 0, 0, 0);

	if (!isFlag(20))
		_gameSys->insertSequence(0x69, 19, 0, 0, kSeqNone, 0, 0, 0);

	if (isFlag(26)) {
		initGnapPos(3, 6, kDirBottomRight);
		_s19_currShopAssistantSequenceId = kS19ShopAssistantSequenceIds[getRandom(5)];
		_s19_nextShopAssistantSequenceId = _s19_currShopAssistantSequenceId;
		_gameSys->setAnimation(_s19_currShopAssistantSequenceId, 20, 4);
		_gameSys->insertSequence(0x6E, 254, 0, 0, kSeqNone, 0, 0, 0);
		_gameSys->insertSequence(_s19_currShopAssistantSequenceId, 20, 0, 0, kSeqNone, 0, 0, 0);
		_s19_shopAssistantCtr = 0;
		endSceneInit();
		gnapWalkTo(4, 9, -1, 0x107B9, 1);
		scene19_updateHotspots();
	} else {
		_s19_currShopAssistantSequenceId = 0x6D;
		_s19_nextShopAssistantSequenceId = -1;
		_gameSys->setAnimation(0x6D, 20, 4);
		_gameSys->insertSequence(_s19_currShopAssistantSequenceId, 20, 0, 0, kSeqNone, 0, 0, 0);
		_timers[6] = getRandom(40) + 50;
		initGnapPos(3, 6, kDirBottomRight);
		initBeaverPos(4, 6, 0);
		endSceneInit();
		gnapWalkTo(4, 9, -1, 0x107B9, 1);
		platypusWalkTo(5, 9, -1, 0x107C2, 1);
	}
	
	while (!_sceneDone) {

		updateMouseCursor();
		updateCursorByHotspot();

		testWalk(0, 5, -1, -1, -1, -1);

		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);

		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene19_updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemJoint) {
					gnapUseJointOnPlatypus();
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(_platX, _platY);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan1(_platX, _platY);
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

		case kHSExitOutsideToyStore:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 18;
				_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
				gnapWalkTo(_hotspotsWalkPos[1].x, _hotspotsWalkPos[1].y, 0, 0x107B2, 1);
				_gnapActionStatus = kASLeaveScene;
				if (isFlag(26))
					setFlag(27);
				else
					platypusWalkTo(_hotspotsWalkPos[1].x + 1, _hotspotsWalkPos[1].y, -1, 0x107C5, 1);
				_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
			}
			break;

		case kHSPicture:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 6, 2);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(6, 2);
						break;
					case GRAB_CURSOR:
						if (!isFlag(20)) {
							gnapWalkTo(_gnapX, _gnapY, 0, getGnapSequenceId(gskIdle, _hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y) | 0x10000, 1);
							_gnapActionStatus = kASGrabPicture;
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSShopAssistant:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 6, 2);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(6, 2);
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = kDirUpRight;
						gnapWalkTo(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTalkShopAssistant;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSToy1:
		case kHSToy2:
		case kHSToy3:
		case kHSToy4:
		case kHSToy5:
		case kHSToy6:
		case kHSToy7:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapImpossible(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapMoan2(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y);
						break;
					case GRAB_CURSOR:
						gnapWalkTo(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 0, -1, 1);
						playGnapIdle(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y);
						_gnapActionStatus = kASGrabToy;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSPhone:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 9, 1);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(9, 1);
						break;
					case GRAB_CURSOR:
						gnapWalkTo(_hotspotsWalkPos[_sceneClickedHotspot].x, _hotspotsWalkPos[_sceneClickedHotspot].y, 0, -1, 1);
						playGnapIdle(8, 2);
						_gnapActionStatus = kASUsePhone;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_mouseClickState._left) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = 0;
			}
		}

		scene19_updateAnimations();
	
		if (!_isLeavingScene) {
			updateGnapIdleSequence();
			if (!isFlag(26)) {
				updateBeaverIdleSequence();
				if (!_timers[6] && _s19_nextShopAssistantSequenceId == -1) {
					_timers[6] = getRandom(40) + 50;
					if (getRandom(4) != 0) {
						_s19_nextShopAssistantSequenceId = 0x64;
					} else if (isFlag(20)) {
						_s19_nextShopAssistantSequenceId = 0x64;
					} else {
						_s19_nextShopAssistantSequenceId = 0x6C;
					}
				}
			}
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene19_updateHotspots();
		}
		
		gameUpdateTick();

	}

	if (_s19_pictureSurface)
		deleteSurface(&_s19_pictureSurface);

}

void GnapEngine::scene19_updateAnimations() {
	
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASUsePhone:
			_s19_nextShopAssistantSequenceId = 0x67;
			break;
		case kASGrabToy:
			++_s19_toyGrabCtr;
			switch (_s19_toyGrabCtr) {
			case 1:
				_s19_nextShopAssistantSequenceId = 0x62;
				break;
			case 2:
				_s19_nextShopAssistantSequenceId = 0x6B;
				break;
			case 3:
				_s19_nextShopAssistantSequenceId = 0x66;
				break;
			default:
				_s19_nextShopAssistantSequenceId = 0x65;
				break;
			}
			break;
		case kASGrabPicture:
			playGnapPullOutDevice(6, 2);
			playGnapUseDevice(0, 0);
			_gameSys->setAnimation(0x68, 19, 0);
			_gameSys->insertSequence(0x68, 19, 105, 19, kSeqSyncWait, 0, 0, 0);
			invAdd(kItemPicture);
			setFlag(20);
			scene19_updateHotspots();
			_gnapActionStatus = kASGrabPictureDone;
			break;
		case kASGrabPictureDone:
			setGrabCursorSprite(-1);
			hideCursor();
			_s19_pictureSurface = addFullScreenSprite(0xF, 255);
			_gameSys->setAnimation(0x61, 256, 0);
			_gameSys->insertSequence(0x61, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_gameSys->getAnimationStatus(0) != 2) {
				// checkGameAppStatus();
				gameUpdateTick();
			}
			setFlag(27);
			showCursor();
			_newSceneNum = 17;
			_isLeavingScene = true;
			_sceneDone = true;
			_s19_nextShopAssistantSequenceId = -1;
			break;
		case kASTalkShopAssistant:
			_s19_nextShopAssistantSequenceId = 0x6D;
			_gnapActionStatus = -1;
			break;
		case kASLeaveScene:
			_sceneDone = true;
			break;
		}
	}

	if (_gameSys->getAnimationStatus(4) == 2) {
		switch (_s19_nextShopAssistantSequenceId) {
		case 0x6F:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
			_s19_shopAssistantCtr = (_s19_shopAssistantCtr + 1) % 5;
			_s19_nextShopAssistantSequenceId = kS19ShopAssistantSequenceIds[_s19_shopAssistantCtr];
			_gameSys->setAnimation(_s19_nextShopAssistantSequenceId, 20, 4);
			_gameSys->insertSequence(_s19_nextShopAssistantSequenceId, 20, _s19_currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x6E, 254, 0x6E, 254, kSeqSyncWait, 0, 0, 0);
			_s19_currShopAssistantSequenceId = _s19_nextShopAssistantSequenceId;
			break;
		case 0x62:
		case 0x66:
		case 0x6B:
			_gameSys->setAnimation(_s19_nextShopAssistantSequenceId, 20, 4);
			_gameSys->insertSequence(_s19_nextShopAssistantSequenceId, 20, _s19_currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s19_currShopAssistantSequenceId = _s19_nextShopAssistantSequenceId;
			_s19_nextShopAssistantSequenceId = -1;
			_timers[5] = 10;
			while (_timers[5]) {
				gameUpdateTick();
			}
			playGnapIdle(6, 2);
			_gnapActionStatus = -1;
			break;
		case 0x67:
			_gameSys->setAnimation(_s19_nextShopAssistantSequenceId, 20, 4);
			_gameSys->insertSequence(_s19_nextShopAssistantSequenceId, 20, _s19_currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s19_currShopAssistantSequenceId = _s19_nextShopAssistantSequenceId;
			_s19_nextShopAssistantSequenceId = -1;
			_gnapActionStatus = -1;
			break;
		case 0x65:
			playGnapIdle(6, 2);
			_gameSys->setAnimation(_s19_nextShopAssistantSequenceId, 20, 0);
			_gameSys->insertSequence(_s19_nextShopAssistantSequenceId, 20, _s19_currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s19_currShopAssistantSequenceId = _s19_nextShopAssistantSequenceId;
			_s19_nextShopAssistantSequenceId = -1;
			_newSceneNum = 18;
			_gnapActionStatus = kASLeaveScene;
			break;
		case 0x6D:
			_gameSys->setAnimation(_s19_nextShopAssistantSequenceId, 20, 4);
			_gameSys->insertSequence(_s19_nextShopAssistantSequenceId, 20, _s19_currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x69, 19, 0x69, 19, kSeqSyncWait, getSequenceTotalDuration(_s19_nextShopAssistantSequenceId), 0, 0);
			_s19_currShopAssistantSequenceId = _s19_nextShopAssistantSequenceId;
			_s19_nextShopAssistantSequenceId = -1;
			break;
		case 0x64:
		case 0x6C:
			_gameSys->setAnimation(_s19_nextShopAssistantSequenceId, 20, 4);
			_gameSys->insertSequence(_s19_nextShopAssistantSequenceId, 20, _s19_currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s19_currShopAssistantSequenceId = _s19_nextShopAssistantSequenceId;
			_s19_nextShopAssistantSequenceId = -1;
			break;
		}
	}

}

} // End of namespace Gnap
