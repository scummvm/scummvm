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
	kHSChicken		= 1,
	kHSDevice		= 2,
	kHSExitHouse	= 3,
	kHSExitBarn		= 4,
	kHSExitCreek	= 5,
	kHSExitPigpen	= 6,
	kHSWalkArea1	= 7,
	kHSWalkArea2	= 8
};

enum {
	kASLeaveScene			= 0,
	kASTalkChicken			= 1,
	kASUseChicken			= 2,
	kASUseChickenDone		= 3
};

int GnapEngine::scene33_init() {
	return isFlag(26) ? 0x84 : 0x85;
}

void GnapEngine::scene33_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSChicken, 606, 455, 702, 568, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
	setHotspot(kHSExitHouse, 480, 120, 556, 240, SF_EXIT_U_CURSOR, 7, 3);
	setHotspot(kHSExitBarn, 610, 75, 800, 164, SF_EXIT_U_CURSOR, 10, 3);
	setHotspot(kHSExitCreek, 780, 336, 800, 556, SF_EXIT_R_CURSOR | SF_WALKABLE, 10, 8);
	setHotspot(kHSExitPigpen, 0, 300, 20, 600, SF_EXIT_L_CURSOR | SF_WALKABLE, 0, 8);
	setHotspot(kHSWalkArea1, 120, 0, 514, 458);
	setHotspot(kHSWalkArea2, 0, 0, 800, 452);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 9;
}

void GnapEngine::scene33_run() {
	
	playSound(0x1091C, 1);
	startSoundTimerC(6);
	queueInsertDeviceIcon();

	_s33_currChickenSequenceId = 0x7E;
	_gameSys->setAnimation(0x7E, 179, 2);
	_gameSys->insertSequence(_s33_currChickenSequenceId, 179, 0, 0, kSeqNone, 0, 0, 0);
	_s33_nextChickenSequenceId = -1;
	_timers[5] = getRandom(20) + 30;
	_timers[4] = getRandom(100) + 300;

	switch (_prevSceneNum) {
	case 34:
		initGnapPos(11, 7, 3);
		initBeaverPos(12, 7, 4);
		endSceneInit();
		gnapWalkTo(8, 7, -1, 0x107BA, 1);
		platypusWalkTo(9, 7, -1, 0x107D2, 1);
		break;
	case 37:
		initGnapPos(7, 7, 1);
		initBeaverPos(8, 7, 0);
		endSceneInit();
		break;
	case 32:
		initGnapPos(-1, 6, 1);
		initBeaverPos(-1, 7, 0);
		endSceneInit();
		platypusWalkTo(2, 7, -1, 0x107C2, 1);
		gnapWalkTo(2, 8, -1, 0x107B9, 1);
		break;
	default:
		initGnapPos(3, 7, 1);
		initBeaverPos(2, 7, 0);
		endSceneInit();
		break;
	}

	while (!_sceneDone) {

		updateMouseCursor();
		updateCursorByHotspot();
		testWalk(0, 0, 7, 6, 8, 6);
		
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene33_updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
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
						playGnapImpossible(_platX, _platY);
						break;
					}
				}
			}
			break;

		case kHSChicken:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(7, 9, 9, 8);
				} else {
					switch (_verbCursor) {
					case GRAB_CURSOR:
						_gnapIdleFacing = 1;
						if (gnapWalkTo(_hotspotsWalkPos[kHSChicken].x, _hotspotsWalkPos[kHSChicken].y, 0, getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1))
							_gnapActionStatus = kASUseChicken;
						else
							_gnapActionStatus = -1;
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 1;
						gnapWalkTo(_hotspotsWalkPos[kHSChicken].x, _hotspotsWalkPos[kHSChicken].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTalkChicken;
						break;
					case LOOK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;
	
		case kHSExitHouse:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_gnapActionStatus = kASLeaveScene;
				_newSceneNum = 37;
				if (_gnapX > 6)
					gnapWalkTo(_gnapX, _gnapY, 0, 0x107AD, 1);
				else
					gnapWalkTo(6, 7, 0, 0x107B1, 1);
			}
			break;
	
		case kHSExitBarn:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_gnapActionStatus = kASLeaveScene;
				_newSceneNum = 35;
				if (_gnapX > 7)
					gnapWalkTo(_gnapX, _gnapY, 0, 0x107AD, 1);
				else
					gnapWalkTo(7, 7, 0, 0x107B1, 1);
			}
			break;

		case kHSExitCreek:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				gnapWalkTo(_hotspotsWalkPos[kHSExitCreek].x, _hotspotsWalkPos[kHSExitCreek].y, 0, 0x107AB, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitCreek].x, _hotspotsWalkPos[kHSExitCreek].y, -1, 0x107CD, 1);
				_newSceneNum = 34;
			}
			break;
	
		case kHSExitPigpen:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				gnapWalkTo(_hotspotsWalkPos[kHSExitPigpen].x, _hotspotsWalkPos[kHSExitPigpen].y, 0, 0x107AF, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitPigpen].x, _hotspotsWalkPos[kHSExitPigpen].y, -1, 0x107CF, 1);
				_newSceneNum = 32;
			}
			break;
	
		case kHSWalkArea1:
		case kHSWalkArea2:
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;
			
		default:
			if (_mouseClickState._left && _gnapActionStatus < 0) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;

		}
	
		scene33_updateAnimations();
	
		if (!isSoundPlaying(0x1091C))
			playSound(0x1091C, 1);
	
		if (!_isLeavingScene) {
			if (_beaverActionStatus < 0)
				updateBeaverIdleSequence();
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(100) + 300;
				if (getRandom(2) != 0)
					_gameSys->insertSequence(0x83, 256, 0, 0, kSeqNone, 0, 0, 0);
				else
					_gameSys->insertSequence(0x82, 256, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_timers[5] && _s33_nextChickenSequenceId == -1 && _gnapActionStatus != kASTalkChicken && _gnapActionStatus != kASUseChicken) {
				if (getRandom(6) != 0) {
					_s33_nextChickenSequenceId = 0x7E;
					_timers[5] = getRandom(20) + 30;
				} else {
					_s33_nextChickenSequenceId = 0x80;
					_timers[5] = getRandom(20) + 50;
				}
			}
			playSoundC();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene33_updateHotspots();
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene33_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			break;
		case kASTalkChicken:
			_s33_nextChickenSequenceId = 0x7F;
			break;
		case kASUseChicken:
			_s33_nextChickenSequenceId = 0x81;
			_timers[2] = 100;
			break;
		case kASUseChickenDone:
			_gameSys->insertSequence(0x107B5, _gnapId, 0x81, 179, kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
			_gnapSequenceId = 0x7B5;
			_gnapSequenceDatNum = 1;
			_s33_currChickenSequenceId = 0x7E;
			_gameSys->setAnimation(0x7E, 179, 2);
			_gameSys->insertSequence(_s33_currChickenSequenceId, 179, 0, 0, kSeqNone, 0, 0, 0);
			_gnapActionStatus = -1;
			_timers[5] = 30;
			break;
		default:
			_gnapActionStatus = -1;
			break;
		}
	}

	if (_gameSys->getAnimationStatus(2) == 2) {
		if (_s33_nextChickenSequenceId == 0x81) {
			_gameSys->setAnimation(_s33_nextChickenSequenceId, 179, 0);
			_gameSys->insertSequence(_s33_nextChickenSequenceId, 179, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->removeSequence(_s33_currChickenSequenceId, 179, true);
			_s33_nextChickenSequenceId = -1;
			_s33_currChickenSequenceId = -1;
			_gnapActionStatus = kASUseChickenDone;
			_timers[5] = 500;
		} else if (_s33_nextChickenSequenceId == 0x7F) {
			_gameSys->setAnimation(_s33_nextChickenSequenceId, 179, 2);
			_gameSys->insertSequence(_s33_nextChickenSequenceId, 179, _s33_currChickenSequenceId, 179, kSeqSyncWait, 0, 0, 0);
			_s33_currChickenSequenceId = _s33_nextChickenSequenceId;
			_s33_nextChickenSequenceId = -1;
			_gnapActionStatus = -1;
		} else if (_s33_nextChickenSequenceId != -1) {
			_gameSys->setAnimation(_s33_nextChickenSequenceId, 179, 2);
			_gameSys->insertSequence(_s33_nextChickenSequenceId, 179, _s33_currChickenSequenceId, 179, kSeqSyncWait, 0, 0, 0);
			_s33_currChickenSequenceId = _s33_nextChickenSequenceId;
			_s33_nextChickenSequenceId = -1;
		}
	}

}

} // End of namespace Gnap
					