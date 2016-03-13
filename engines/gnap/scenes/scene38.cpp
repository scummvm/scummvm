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
	kHSPlatypus			= 0,
	kHSExitHouse		= 1,
	kHSExitCave			= 2,
	kHSTrapDoorLid1		= 3,
	kHSTrapDoorLid2		= 4,
	kHSHuntingTrophy	= 5,
	kHSWalkArea1		= 6,
	kHSDevice			= 7,
	kHSWalkArea2		= 8,
	kHSWalkArea3		= 9,
	kHSWalkArea4		= 10,
	kHSWalkArea5		= 11,
	kHSWalkArea6		= 12
};

enum {
	kASLeaveScene				= 0,
	kASExitCave					= 1,
	kASUseHuntingTrophy			= 2,
	kASHoldingHuntingTrophy		= 3,
	kASReleaseHuntingTrophy		= 4,
	kASUseBeaverWithTrapDoor	= 5,
	kASBeaverHoldingTrapDoor	= 6
};

int GnapEngine::scene38_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	return 0xA5;
}

void GnapEngine::scene38_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSExitHouse, 150, 585, 650, 600, SF_EXIT_D_CURSOR, 0, 8);
	setHotspot(kHSExitCave, 430, 440, 655, 470, SF_WALKABLE, 0, 8);
	setHotspot(kHSTrapDoorLid1, 525, 265, 640, 350, SF_DISABLED);
	setHotspot(kHSTrapDoorLid2, 555, 350, 670, 430, SF_DISABLED);
	setHotspot(kHSHuntingTrophy, 170, 85, 250, 190, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 0, 8);
	setHotspot(kHSWalkArea1, 330, 270, 640, 380, SF_DISABLED | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 0, 8);
	setHotspot(kHSWalkArea2, 0, 0, 799, 396);
	setHotspot(kHSWalkArea3, 0, 585, 799, 599, SF_WALKABLE | SF_DISABLED);
	setHotspot(kHSWalkArea4, 0, 0, 97, 445);
	setHotspot(kHSWalkArea5, 770, 0, 799, 445);
	setHotspot(kHSWalkArea6, 393, 0, 698, 445, SF_WALKABLE | SF_DISABLED);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_beaverActionStatus == kASBeaverHoldingTrapDoor)
		_hotspots[kHSPlatypus].flags = SF_WALKABLE | SF_DISABLED;
	if (_beaverActionStatus == kASBeaverHoldingTrapDoor)
		_hotspots[kHSExitCave].flags = SF_EXIT_D_CURSOR;
	else if (_gnapActionStatus == kASHoldingHuntingTrophy)
		_hotspots[kHSExitCave].flags = SF_EXIT_D_CURSOR;
	if (_beaverActionStatus == kASBeaverHoldingTrapDoor)
		_hotspots[kHSTrapDoorLid1].flags = SF_DISABLED;
	else if (_gnapActionStatus == kASHoldingHuntingTrophy)
		_hotspots[kHSTrapDoorLid1].flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_beaverActionStatus == kASBeaverHoldingTrapDoor)
		_hotspots[kHSTrapDoorLid2].flags = SF_DISABLED;
	else if (_gnapActionStatus == kASHoldingHuntingTrophy)
		_hotspots[kHSTrapDoorLid2].flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_beaverActionStatus == kASBeaverHoldingTrapDoor)
		_hotspots[kHSWalkArea6].flags = 0;
	_hotspotsCount = 13;
}

void GnapEngine::scene38_run() {

	queueInsertDeviceIcon();
	_gameSys->insertSequence(0x9B, 0, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_prevSceneNum == 39) {
		initGnapPos(3, 7, 3);
		initBeaverPos(4, 7, 4);
	} else {
		initGnapPos(3, 8, 1);
		initBeaverPos(4, 8, 0);
	}
	endSceneInit();

	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();

		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {

		case kHSDevice:
			runMenu();
			scene38_updateHotspots();
			break;

		case kHSPlatypus:
			if (_gnapActionStatus == kASHoldingHuntingTrophy) {
				_gnapActionStatus = kASReleaseHuntingTrophy;
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
					playGnapImpossible(_platX, _platY);
					break;
				}
			}
			break;

		case kHSExitHouse:
			if (_gnapActionStatus == kASHoldingHuntingTrophy) {
				_gnapActionStatus = kASReleaseHuntingTrophy;
			} else {
				_isLeavingScene = true;
				gnapWalkTo(-1, -1, 0, 0x107AE, 1);
				_gnapActionStatus = kASLeaveScene;
				_newSceneNum = 37;
			}
			break;

		case kHSExitCave:
			if (_gnapActionStatus == kASHoldingHuntingTrophy) {
				_gnapActionStatus = kASReleaseHuntingTrophy;
				if (_beaverActionStatus == kASBeaverHoldingTrapDoor)
					_isLeavingScene = true;
			} else if (_beaverActionStatus == kASBeaverHoldingTrapDoor) {
				_sceneWaiting = false;
				_isLeavingScene = true;
				gnapWalkTo(5, 7, 0, 0x107BB, 1);
				_newSceneNum = 39;
				_gnapActionStatus = kASExitCave;
			}
			break;

		case kHSTrapDoorLid1:
		case kHSTrapDoorLid2:
			if (_gnapActionStatus == kASHoldingHuntingTrophy) {
				if (_verbCursor == PLAT_CURSOR && _beaverActionStatus != kASBeaverHoldingTrapDoor)
					_gnapActionStatus = kASUseBeaverWithTrapDoor;
				else
					_gnapActionStatus = kASReleaseHuntingTrophy;
			}
			break;

		case kHSHuntingTrophy:
			if (_gnapActionStatus != kASHoldingHuntingTrophy) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(3, 6, 2, 0);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(0, 0);
						break;
					case GRAB_CURSOR:
						if (_beaverActionStatus == kASBeaverHoldingTrapDoor)
							playGnapImpossible(0, 0);
						else {
							gnapWalkTo(3, 6, 0, 0x107BB, 1);
							platypusWalkTo(4, 8, -1, -1, 1);
							_gnapActionStatus = kASUseHuntingTrophy;
						}
						break;
					case TALK_CURSOR:
						playGnapBrainPulsating(2, 0);
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSWalkArea1:
			// Nothing
			break;

		case kHSWalkArea2:
		case kHSWalkArea3:
		case kHSWalkArea4:
		case kHSWalkArea5:
		case kHSWalkArea6:
			if (_gnapActionStatus == kASHoldingHuntingTrophy)
				_gnapActionStatus = kASReleaseHuntingTrophy;
			else if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;
			
		default:
			if (_mouseClickState.left) {
				if (_gnapActionStatus == kASHoldingHuntingTrophy)
					_gnapActionStatus = kASReleaseHuntingTrophy;
				else if (_gnapActionStatus < 0)
					gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState.left = false;
			}
			break;

		}

		scene38_updateAnimations();
	
		if (!_isLeavingScene) {
			updateBeaverIdleSequence();
			updateGnapIdleSequence();
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene38_updateHotspots();
		}
		
		gameUpdateTick();
	
	}

}

void GnapEngine::scene38_updateAnimations() {

	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASLeaveScene:
			_sceneDone = true;
			break;
		case kASExitCave:
			_gameSys->removeSequence(_beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId, true);
			_gameSys->insertSequence(0xA3, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0xA3;
			_gnapSequenceDatNum = 0;
			_gameSys->setAnimation(0xA3, _gnapId, 0);
			_gnapActionStatus = kASLeaveScene;
			break;
		case kASUseHuntingTrophy:
			_gameSys->removeSequence(0x9B, 0, true);
			_gameSys->insertSequence(0x9C, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x9C;
			_gnapSequenceDatNum = 0;
			_gameSys->setAnimation(0x9C, _gnapId, 0);
			_gnapActionStatus = kASHoldingHuntingTrophy;
			scene38_updateHotspots();
			break;
		case kASHoldingHuntingTrophy:
			if (_beaverActionStatus != kASBeaverHoldingTrapDoor)
				_sceneWaiting = true;
			if (_gnapSequenceId == 0xA4) {
				_gameSys->insertSequence(0x9D, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
				_gnapSequenceId = 0x9D;
			} else {
				_gameSys->insertSequence(0xA4, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
				_gnapSequenceId = 0xA4;
			}
			_gnapSequenceDatNum = 0;
			_gameSys->setAnimation(_gnapSequenceId, _gnapId, 0);
			break;
		case kASReleaseHuntingTrophy:
			if (_gnapSequenceId == 0x9E) {
				_gameSys->insertSequence(0x9B, 0, 0, 0, kSeqNone, 0, 0, 0);
				_gnapActionStatus = -1;
			} else if (_beaverActionStatus == kASBeaverHoldingTrapDoor) {
				_gameSys->insertSequence(0xA0, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
				_gnapSequenceId = 0xA0;
				_gnapSequenceDatNum = 0;
				_gnapX = 3;
				_gnapY = 6;
				_gnapIdleFacing = 1;
				if (_isLeavingScene) {
					_sceneWaiting = false;
					gnapWalkTo(5, 7, 0, 0x107BB, 1);
					_newSceneNum = 39;
					_gnapActionStatus = kASExitCave;
				} else {
					_gnapActionStatus = -1;
				}
			} else {
				_gameSys->insertSequence(0x9E, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
				_gnapSequenceId = 0x9E;
				_gnapSequenceDatNum = 0;
				_gnapX = 3;
				_gnapY = 6;
				_gnapIdleFacing = 1;
				_gameSys->setAnimation(0x9E, _gnapId, 0);
				_sceneWaiting = false;
				scene38_updateHotspots();
			}
			break;
		case kASUseBeaverWithTrapDoor:
			_sceneWaiting = false;
			_gameSys->insertSequence(0x9F, _gnapId, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gnapSequenceId = 0x9F;
			_gnapSequenceDatNum = 0;
			_gameSys->setAnimation(0x9F, _gnapId, 0);
			_gnapActionStatus = kASHoldingHuntingTrophy;
			if (_beaverFacing)
				playBeaverSequence(0x107D5);
			else
				playBeaverSequence(0x107D4);
			platypusWalkTo(8, 7, -1, 0x107D2, 1);
			_gameSys->insertSequence(0xA1, _gnapId + 1, _beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId, kSeqSyncWait, 0, 0, 0);
			_beaverSequenceId = 0xA1;
			_beaverSequenceDatNum = 0;
			_beaverId = _gnapId + 1;
			_gameSys->setAnimation(0xA1, _gnapId + 1, 1);
			_beaverActionStatus = kASBeaverHoldingTrapDoor;
			scene38_updateHotspots();
			break;
		}
	}

	if (_gameSys->getAnimationStatus(1) == 2) {
		_gameSys->setAnimation(0, 0, 1);
		if (_beaverActionStatus == kASBeaverHoldingTrapDoor) {
			_gameSys->insertSequence(0xA2, _beaverId, _beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId, kSeqSyncWait, 0, 0, 0);
			_beaverSequenceId = 0xA2;
			_beaverSequenceDatNum = 0;
			scene38_updateHotspots();
			_sceneWaiting = true;
		}
	}

}

} // End of namespace Gnap
					