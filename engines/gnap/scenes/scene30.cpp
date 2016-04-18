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
	kHSPillMachine	= 1,
	kHSDevice		= 2,
	kHSExitCircus	= 3,
	kHSWalkArea1	= 4
};

enum {
	kASLeaveScene				= 0,
	kASUsePillMachine			= 1,
	kASUsePillMachine2			= 2,
	kASLookPillMachine			= 3,
	kASUsePillMachine3			= 4,
	kASUsePillMachine4			= 5
};

int GnapEngine::scene30_init() {
	return isFlag(kGFUnk23) ? 0x10B : 0x10A;
}

void GnapEngine::scene30_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSPillMachine, 598, 342, 658, 426, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 7);
	setHotspot(kHSExitCircus, 100, 590 - _deviceY1, 700, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	setHotspot(kHSWalkArea1, 0, 0, 800, 514);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	_hotspotsCount = 5;
}

void GnapEngine::scene30_run() {
	bool hasTakenPill = false;

	playSound(0x1093B, true);
	startSoundTimerB(6);

	queueInsertDeviceIcon();

	if (isFlag(kGFUnk23))
		_gameSys->insertSequence(0x106, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (!isFlag(kGFUnk13))
		_gameSys->insertSequence(0x107, 1, 0, 0, kSeqNone, 0, 0, 0);
	_timers[5] = getRandom(50) + 180;

	_gameSys->insertSequence(0x101, 40, 0, 0, kSeqNone, 0, 0, 0);
	_timers[4] = getRandom(100) + 300;

	_s30_kidSequenceId = 0x101;
	initGnapPos(7, 12, kDirBottomRight);
	initBeaverPos(6, 12, kDirNone);
	endSceneInit();
	gnapWalkTo(7, 8, -1, 0x107B9, 1);
	platypusWalkTo(6, 8, -1, 0x107C2, 1);

	while (!_sceneDone) {
		updateMouseCursor();
		updateCursorByHotspot();

		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);

		switch (_sceneClickedHotspot) {
		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene30_updateHotspots();
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
						playGnapMoan1(_platX, _platY);
						break;
					case GRAB_CURSOR:
						gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						playGnapBrainPulsating(_platX, _platY);
						playBeaverSequence(getBeaverSequenceId());
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSPillMachine:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemDiceQuarterHole && !isFlag(kGFUnk23)) {
					_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
					gnapWalkTo(_hotspotsWalkPos[kHSPillMachine].x, _hotspotsWalkPos[kHSPillMachine].y, 0, 0x107BC, 1);
					_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
					_gnapActionStatus = kASUsePillMachine;
					hasTakenPill = true;
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSPillMachine].x, _hotspotsWalkPos[kHSPillMachine].y, 8, 5);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						gnapWalkTo(9, 8, 0, 0x107BC, 1);
						_gnapActionStatus = kASLookPillMachine;
						break;
					case GRAB_CURSOR:
						playGnapScratchingHead(8, 5);
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(8, 5);
						break;
					}
				}
			}
			break;

		case kHSExitCircus:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				if (hasTakenPill)
					_newSceneNum = 47;
				else
					_newSceneNum = 26;
				gnapWalkTo(-1, _hotspotsWalkPos[kHSExitCircus].y, 0, 0x107AE, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitCircus].x + 1, _hotspotsWalkPos[kHSExitCircus].y, -1, 0x107C2, 1);
			}
			break;

		case kHSWalkArea1:
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_mouseClickState._left) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;
	
		}

		scene30_updateAnimations();

		if (!isSoundPlaying(0x1093B))
			playSound(0x1093B, true);

		if (!_isLeavingScene) {
			updateBeaverIdleSequence();
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(100) + 300;
				if (_gnapActionStatus < 0) {
					if (getRandom(5) == 1) {
						_gameSys->insertSequence(0xFF, 40, 0, 0, kSeqNone, 0, 0, 0);
						_gameSys->insertSequence(0x100, 40, _s30_kidSequenceId, 40, kSeqSyncWait, 0, 0, 0);
						_s30_kidSequenceId = 0x100;
					} else {
						_gameSys->insertSequence(0xFE, 40, 0, 0, kSeqNone, 0, 0, 0);
					}
				}
			}
			if (!_timers[5]) {
				_timers[5] = getRandom(50) + 180;
				if (_gnapActionStatus < 0) {
					if (!isFlag(kGFUnk23) || hasTakenPill)
						_gameSys->insertSequence(0x109, 20, 0, 0, kSeqNone, 0, 0, 0);
					else
						_gameSys->insertSequence(0x108, 20, 0, 0, kSeqNone, 0, 0, 0);
				}
			}
			playSoundB();
		}
	
		checkGameKeys();
		
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene30_updateHotspots();
		}
		
		gameUpdateTick();
	}
}

void GnapEngine::scene30_updateAnimations() {	
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			break;
		case kASUsePillMachine:
			setGrabCursorSprite(-1);
			_gameSys->setAnimation(0x105, _gnapId, 0);
			_gameSys->insertSequence(0x105, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x105;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = kASUsePillMachine2;
			break;
		case kASUsePillMachine2:
			hideCursor();
			setGrabCursorSprite(-1);
			addFullScreenSprite(0x3F, 255);
			_gameSys->removeSequence(0x105, _gnapId, true);
			_gameSys->setAnimation(0x102, 256, 0);
			_gameSys->insertSequence(0x102, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_gameSys->getAnimationStatus(0) != 2)
				gameUpdateTick();
			_gameSys->setAnimation(0x103, _gnapId, 0);
			_gameSys->insertSequence(0x103, _gnapId, 0, 0, kSeqNone, 0, 0, 0);
			removeFullScreenSprite();
			showCursor();
			_gnapActionStatus = kASUsePillMachine3;
			invAdd(kItemPill);
			setFlag(kGFUnk23);
			break;
		case kASUsePillMachine3:
			_gameSys->setAnimation(0x104, _gnapId, 0);
			_gameSys->insertSequence(0x104, _gnapId, makeRid(_gnapSequenceDatNum, 0x103), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x104;
			_gnapSequenceDatNum = 0;
			_gnapActionStatus = kASUsePillMachine4;
			setGrabCursorSprite(kItemDiceQuarterHole);
			break;
		case kASUsePillMachine4:
			_gameSys->insertSequence(0x106, 1, 0, 0, kSeqNone, 0, 0, 0);
			gnapWalkTo(_hotspotsWalkPos[kHSPillMachine].x, _hotspotsWalkPos[kHSPillMachine].y + 1, -1, 0x107BC, 1);
			_gnapActionStatus = -1;
			break;
		case kASLookPillMachine:
			if (isFlag(kGFUnk23))
				showFullScreenSprite(0xE3);
			else
				showFullScreenSprite(0xE2);
			_gnapActionStatus = -1;
			break;
		}
	}
}

} // End of namespace Gnap
