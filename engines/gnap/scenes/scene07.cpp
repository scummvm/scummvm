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
	kHSExitHouse	= 1,
	kHSDice			= 2,
	kHSDevice		= 3,
	kHSWalkArea1	= 4,
	kHSWalkArea2	= 5,
	kHSWalkArea3	= 6
};

enum {
	kASWait			= 0,
	kASLeaveScene	= 1
};

int GnapEngine::scene07_init() {
	return 0x92;
}

void GnapEngine::scene07_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitHouse, 700, 125, 799, 290, SF_EXIT_NE_CURSOR);
	setHotspot(kHSDice, 200, 290, 270, 360, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSWalkArea1, 0, 0, 325, 445);
	setHotspot(kHSWalkArea2, 325, 0, 799, 445, _isLeavingScene ? SF_WALKABLE : 0);
	setHotspot(kHSWalkArea3, 160, 0, 325, 495);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (isFlag(0))
		_hotspots[kHSDice]._flags = SF_DISABLED;
	_hotspotsCount = 7;
}

void GnapEngine::scene07_run() {
	
	queueInsertDeviceIcon();
	_gameSys->insertSequence(0x8C, 1, 0, 0, kSeqLoop, 0, 0, 0);
	_gameSys->insertSequence(0x90, 1, 0, 0, kSeqLoop, 0, 0, 0);

	invRemove(kItemGas);
	invRemove(kItemNeedle);
	
	if (!isFlag(0))
		_gameSys->insertSequence(0x8D, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (_prevSceneNum == 8) {
		initGnapPos(7, 7, kDirBottomLeft);
		initBeaverPos(9, 7, kDirUnk4);
		endSceneInit();
	} else {
		_gnapX = 6;
		_gnapY = 7;
		_gnapId = 140;
		_gnapSequenceId = 0x8F;
		_gnapSequenceDatNum = 0;
		_gnapIdleFacing = kDirBottomRight;
		_gameSys->insertSequence(0x8F, 140, 0, 0, kSeqNone, 0, 0, 0);
		_gameSys->setAnimation(makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, 0);
		_gnapActionStatus = kASWait;
		_platX = 3;
		_platY = 8;
		_beaverId = 160;
		_beaverSequenceId = 0x91;
		_beaverSequenceDatNum = 0;
		_beaverFacing = kDirNone;
		_gameSys->insertSequence(0x91, 160, 0, 0, kSeqNone, 0, 0, 0);
		endSceneInit();
	}

	_timers[3] = 600;
	_timers[4] = getRandom(40) + 50;

	while (!_sceneDone) {

		if (!isSoundPlaying(0x10919))
			playSound(0x10919, 1);

		if (testWalk(0, 1, 8, 7, 6, 7))
			scene07_updateHotspots();

		updateMouseCursor();
		updateCursorByHotspot();

		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);

		switch (_sceneClickedHotspot) {

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
				playBeaverSequence(getBeaverSequenceId(0, 0, 0) | 0x10000);
				break;
			case PLAT_CURSOR:
				break;
			}
			break;

		case kHSExitHouse:
			_isLeavingScene = true;
			if (_gnapX > 8)
				gnapWalkTo(_gnapX, 7, 0, 0x107AD, 1);
			else
				gnapWalkTo(8, 7, 0, 0x107AD, 1);
			_gnapActionStatus = kASLeaveScene;
			break;

		case kHSDice:
			if (_grabCursorSpriteIndex >= 0) {
				playGnapShowCurrItem(4, 8, 3, 3);
			} else {
				switch (_verbCursor) {
				case LOOK_CURSOR:
					break;
				case GRAB_CURSOR:
					setFlag(0);
					invAdd(kItemDice);
					scene07_updateHotspots();
					playGnapPullOutDevice(3, 3);
					_gameSys->setAnimation(0x8E, 1, 2);
					_gameSys->insertSequence(0x8E, 1, 141, 1, kSeqSyncWait, 0, 0, 0);
					_gameSys->insertSequence(getGnapSequenceId(gskUseDevice, 0, 0) | 0x10000, _gnapId,
						makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
						kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
					_gnapSequenceId = getGnapSequenceId(gskUseDevice, 0, 0);
					_gnapSequenceDatNum = 1;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					playGnapImpossible(0, 0);
					break;
				}
			}
			break;

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene07_updateHotspots();
				_timers[4] = getRandom(40) + 50;
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
			gnapWalkTo(-1, -1, -1, -1, 1);
			break;
			
		case kHSWalkArea3:
			// Nothing
			break;

		default:
			if (_mouseClickState._left) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState._left = false;
			}
			break;
			
		}

		scene07_updateAnimations();

		if (!_isLeavingScene) {
			updateGnapIdleSequence();
			if (_beaverActionStatus < 0 && _gnapActionStatus < 0) {
				if (_timers[0]) {
					if (!_timers[1]) {
						_timers[1] = getRandom(20) + 30;
						int _gnapRandomValue = getRandom(20);
						// TODO Cleanup
						if (_beaverFacing != 0) {
							if (_gnapRandomValue != 0 || _beaverSequenceId != 0x7CA) {
								if (_gnapRandomValue != 1 || _beaverSequenceId != 0x7CA) {
									if (_platY == 9)
										playBeaverSequence(0x107CA);
								} else {
									playBeaverSequence(0x10845);
								}
							} else {
								playBeaverSequence(0x107CC);
							}
						} else if (_gnapRandomValue != 0 || _beaverSequenceId != 0x7C9) {
							if (_gnapRandomValue != 1 || _beaverSequenceId != 0x7C9) {
								if (_platY == 9)
									playBeaverSequence(0x107C9);
							} else {
								playBeaverSequence(0x10844);
							}
						} else {
							playBeaverSequence(0x107CB);
						}
						_gameSys->setAnimation(_beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId, 1);
					}
				} else {
					_timers[0] = getRandom(75) + 75;
					beaverMakeRoom();
				}
			} else {
				_timers[0] = 100;
				_timers[1] = 35;
			}
			sceneXX_playRandomSound(4);
		}

		checkGameKeys();

		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene07_updateHotspots();
			_timers[4] = getRandom(40) + 50;
		}
		
		gameUpdateTick();
		
	}

}

void GnapEngine::scene07_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_newSceneNum = 8;
			_sceneDone = true;
			break;
		}
		_gnapActionStatus = -1;
	}

	if (_gameSys->getAnimationStatus(2) == 2) {
		_gameSys->setAnimation(0, 0, 2);
		setGrabCursorSprite(kItemDice);
	}
	
}

} // End of namespace Gnap
