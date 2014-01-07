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

int GnapEngine::scene13_init() {
	playSound(0x108EC, 0);
	return 0xAC;
}

void GnapEngine::scene13_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitBar, 113, 160, 170, 455, SF_EXIT_L_CURSOR);
	setHotspot(kHSBackToilet, 385, 195, 478, 367, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSFrontToilet, 497, 182, 545, 432, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSUrinal, 680, 265, 760, 445, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSScribble, 560, 270, 660, 370, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSSink, 310, 520, 560, 599, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSWalkArea1, 268, 270, 325, 385);
	setHotspot(kHSWalkArea2, 0, 0, 52, 599);
	setHotspot(kHSWalkArea3, 0, 0, 113, 550);
	setHotspot(kHSWalkArea4, 0, 0, 226, 438);
	setHotspot(kHSWalkArea5, 0, 0, 268, 400);
	setHotspot(kHSWalkArea6, 0, 0, 799, 367);
	setHotspot(kHSWalkArea7, 478, 0, 799, 401);
	setHotspot(kHSWalkArea8, 545, 0, 799, 473);
	setHotspot(kHSWalkArea9, 0, 549, 799, 599);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 17;
}

void GnapEngine::scene13_showScribble() {
	hideCursor();
	_largeSprite = _gameSys->createSurface(0x6F);
	_gameSys->insertSpriteDrawItem(_largeSprite, 0, 0, 300);
	while (!_mouseClickState.left && !isKeyStatus1(Common::KEYCODE_ESCAPE) &&
		!isKeyStatus1(Common::KEYCODE_SPACE) && !isKeyStatus1(29))
		gameUpdateTick();
	_mouseClickState.left = false;
	clearKeyStatus1(Common::KEYCODE_ESCAPE);
	clearKeyStatus1(29);
	clearKeyStatus1(Common::KEYCODE_SPACE);
	_gameSys->removeSpriteDrawItem(_largeSprite, 300);
	deleteSurface(&_largeSprite);
	showCursor();
}

void GnapEngine::scene13_run() {
	int currSoundId = 0;

	queueInsertDeviceIcon();
	
	_gameSys->insertSequence(0xAA, 256, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_prevSceneNum == 14) {
		initGnapPos(6, 6, 3);
		initBeaverPos(9, 8, 0);
	} else {
		initGnapPos(3, 7, 1);
		initBeaverPos(2, 7, 0);
	}

	endSceneInit();

	_timers[4] = getRandom(20) + 20;
	_timers[5] = getRandom(50) + 50;

	while (!_sceneDone) {
	
		if (!isSoundPlaying(0x1091A))
			playSound(0x1091A, 1);
	
		testWalk(0, 0, -1, -1, -1, -1);
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene13_updateHotspots();
				_timers[4] = getRandom(20) + 20;
				_timers[5] = getRandom(50) + 50;
			}
			break;

		case kHSPlatypus:
			switch (_verbCursor) {
			case LOOK_CURSOR:
				playGnapMoan1(_platX, _platY);
				break;
			case GRAB_CURSOR:
				gnapKissPlatypus(0);
				break;
			case TALK_CURSOR:
				playGnapBrainPulsating(_platX, _platY);
				playBeaverSequence(getBeaverSequenceId(0, 0, 0));
				break;
			case PLAT_CURSOR:
				break;
			}
			break;

		case kHSExitBar:
			_isLeavingScene = 1;
			gnapWalkTo(2, 7, 0, 0x107C0, 1);
			_gnapActionStatus = kASLeaveScene;
			platypusWalkTo(2, 8, -1, -1, 1);
			if (isFlag(14) || isFlag(15)) {
				_newSceneNum = 11;
			} else {
				setFlag(15);
				_newSceneNum = 47;
			}
			break;

		case kHSBackToilet:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(5, 5, 6, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
				case TALK_CURSOR:
					if (_gnapX == 5 && _gnapY == 5) {
						_s13_backToiletCtr = MIN(5, _s13_backToiletCtr + 1);
						_gameSys->setAnimation(_s13_backToiletCtr + 0xA3, _gnapId, 0);
						_gameSys->insertSequence(_s13_backToiletCtr + 0xA3, _gnapId,
							makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
							9, 0, 0, 0);
						_gnapActionStatus = kASWait;
						_gnapSequenceId = _s13_backToiletCtr + 0xA3;
						_gnapIdleFacing = 7;
						_gnapSequenceDatNum = 0;
					} else {
						gnapWalkTo(5, 5, 0, 0x107BB, 1);
						_gnapActionStatus = kASBackToilet;
						_gnapIdleFacing = 7;
					}
					break;
				case PLAT_CURSOR:
					playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSFrontToilet:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(6, 7, 7, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
				case TALK_CURSOR:
				case PLAT_CURSOR:
					gnapWalkTo(6, 7, 0, 0xA9, 5);
					_gnapActionStatus = kASFrontToilet;
					_gnapIdleFacing = 1;
					break;
				}
			}
			break;

		case kHSScribble:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(7, 7, 8, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					gnapWalkTo(7, 7, 0, 0x107BB, 1);
					_gnapActionStatus = kASLookScribble;
					_gnapIdleFacing = 7;
					break;
				case GRAB_CURSOR:
					playGnapScratchingHead(0, 0);
					break;
				case TALK_CURSOR:
					_gnapIdleFacing = 7;
					gnapWalkTo(7, 7, -1, getGnapSequenceId(gskBrainPulsating, 0, 0), 1);
					break;
				case PLAT_CURSOR:
					playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSUrinal:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(8, 7, 9, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playGnapSequence(getGnapSequenceId(gskDeflect, 9, 6));
					gnapWalkTo(_gnapX, _gnapY, 0, -1, 1);
					_gnapActionStatus = kASWait;
					break;
				case GRAB_CURSOR:
					gnapWalkTo(8, 7, 0, -1, 1);
					_gnapActionStatus = kASGrabUrinal;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSSink:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapImpossible(0, 0);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					playGnapSequence(getGnapSequenceId(gskDeflect, 5, 9));
					gnapWalkTo(_gnapX, _gnapY, 0, -1, 1);
					_gnapActionStatus = kASWait;
					break;
				case GRAB_CURSOR:
					gnapWalkTo(4, 8, 0, 0x107B9, 1);
					_gnapActionStatus = kASGrabSink;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					playGnapImpossible(0, 0);
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
			gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		case kHSWalkArea1:
			// Nothing
			break;
			
		default:
			if (_mouseClickState.left) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState.left = false;
			}
			break;

		}
	
		scene13_updateAnimations();
	
		if (!_isLeavingScene) {
			updateBeaverIdleSequence();
			if (_platY == 5 || _platY == 6)
				platypusWalkTo(-1, 7, -1, -1, 1);
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(20) + 20;
				_gnapRandomValue = getRandom(5);
				switch (_gnapRandomValue) {
				case 0:
					playSound(0xD2, 0);
					break;
				case 1:
					playSound(0xD3, 0);
					break;
				case 2:
					playSound(0xD4, 0);
					break;
				case 3:
					playSound(0xD5, 0);
					break;
				case 4:
					playSound(0xD6, 0);
					break;
				}
			}
			if (!_timers[5]) {
				int newSoundId;
				_timers[5] = getRandom(50) + 50;
				_gnapRandomValue = getRandom(7);
				switch (_gnapRandomValue) {
				case 0:
					newSoundId = 0xD7;
					_timers[5] = 2 * getRandom(50) + 100;
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
					playSound(newSoundId, 0);
					currSoundId = newSoundId;
				}
			}
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene13_updateHotspots();
			_timers[4] = getRandom(20) + 20;
			_timers[5] = getRandom(50) + 50;
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene13_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			_gnapActionStatus = -1;
			break;
		case kASBackToilet:
			_s13_backToiletCtr = MIN(5, _s13_backToiletCtr + 1);
			_gameSys->insertSequence(_s13_backToiletCtr + 0xA3, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, 9, 0, 0, 0);
			_gnapSequenceId = _s13_backToiletCtr + 0xA3;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = -1;
			break;
		case kASFrontToilet:
			_sceneDone = true;
			_newSceneNum = 14;
			break;
		case kASLookScribble:
			_gnapActionStatus = -1;
			scene13_showScribble();
			break;
		case kASGrabSink:
			_gameSys->setAnimation(0xAB, 160, 0);
			_gameSys->insertSequence(0xAB, 160, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->removeSequence(0xAA, 256, 1);
			_gnapSequenceId = 0xAB;
			_gnapId = 160;
			_gnapIdleFacing = 1;
			_gnapSequenceDatNum = 0;
			_gnapX = 4;
			_gnapY = 8;
			_timers[2] = 360;
			_gnapActionStatus = kASGrabSinkDone;
			break;
		case kASGrabSinkDone:
			_gameSys->insertSequence(0xAA, 256, 0, 0, kSeqNone, 0, 0, 0);
			_gnapActionStatus = -1;
			break;
		case kASWait:
			_gnapActionStatus = -1;
			break;
		case kASGrabUrinal:
			_gameSys->setAnimation(0xA2, 120, 0);
			_gameSys->insertSequence(0xA2, 120, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0xA2;
			_gnapId = 120;
			_gnapIdleFacing = 3;
			_gnapSequenceDatNum = 0;
			_gnapX = 4;
			_gnapY = 6;
			_timers[2] = 360;
			_gnapActionStatus = kASWait;
			break;
		}
	}
	
	if (_gameSys->getAnimationStatus(1) == 2) {
		_gameSys->setAnimation(0, 0, 1);
		_beaverActionStatus = -1;
	}

}

} // End of namespace Gnap
