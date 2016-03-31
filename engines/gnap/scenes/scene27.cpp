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
	kHSJanitor		= 1,
	kHSDevice			= 2,
	kHSBucket			= 3,
	kHSExitCircus		= 4,
	kHSExitArcade		= 5,
	kHSExitBeerStand	= 6,
	kHSExitClown		= 7,
	kHSWalkArea1		= 8
};

enum {
	kASTalkJanitor				= 0,
	kASGrabBucket				= 1,
	kASGrabBucketDone			= 2,
	kASShowPictureToJanitor		= 3,
	kASTryEnterClownTent		= 4,
	kASTryEnterClownTentDone	= 5,
	kASEnterClownTent			= 6,
	kASLeaveScene				= 7
};

int GnapEngine::scene27_init() {
	return 0xD5;
}

void GnapEngine::scene27_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSJanitor, 488, 204, 664, 450, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 8, 8);
	setHotspot(kHSBucket, 129, 406, 186, 453, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 6);
	setHotspot(kHSExitCircus, 200, 585, 700, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	setHotspot(kHSExitArcade, 0, 0, 15, 600, SF_EXIT_L_CURSOR, 0, 6);
	setHotspot(kHSExitBeerStand, 785, 0, 800, 600, SF_EXIT_R_CURSOR, 11, 7);
	setHotspot(kHSExitClown, 340, 240, 460, 420, SF_EXIT_U_CURSOR, 6, 8);
	setHotspot(kHSWalkArea1, 0, 0, 800, 507);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (isFlag(13))
		_hotspots[kHSBucket]._flags = SF_DISABLED;
	_hotspotsCount = 9;
}

void GnapEngine::scene27_run() {
	
	playSound(0x1093B, 1);
	startSoundTimerB(4);
	_timers[7] = getRandom(100) + 300;
	queueInsertDeviceIcon();

	if (!isFlag(13))
		_gameSys->insertSequence(0xD3, 39, 0, 0, kSeqNone, 0, 0, 0);

	if (_debugLevel == 4)
		startIdleTimer(6);

	_gameSys->insertSequence(0xCB, 39, 0, 0, kSeqNone, 0, 0, 0);
	
	_s27_currJanitorSequenceId = 0xCB;
	_s27_nextJanitorSequenceId = -1;
	
	_gameSys->setAnimation(0xCB, 39, 3);

	_timers[5] = getRandom(20) + 60;

	switch (_prevSceneNum) {
	case 26:
		initGnapPos(7, 12, kDirBottomRight);
		initBeaverPos(6, 12, 0);
		endSceneInit();
		gnapWalkTo(7, 8, -1, 0x107B9, 1);
		platypusWalkTo(6, 8, -1, 0x107C2, 1);
		break;
	case 29:
		initGnapPos(-1, 8, kDirBottomRight);
		initBeaverPos(-1, 9, 0);
		endSceneInit();
		gnapWalkTo(3, 8, -1, 0x107B9, 1);
		platypusWalkTo(3, 9, -1, 0x107C2, 1);
		break;
	case 31:
		initGnapPos(12, 8, kDirBottomLeft);
		initBeaverPos(12, 9, 4);
		endSceneInit();
		gnapWalkTo(8, 8, -1, 0x107BA, 1);
		platypusWalkTo(8, 9, -1, 0x107C2, 1);
		break;
	default:
		initGnapPos(6, 8, kDirBottomRight);
		initBeaverPos(5, 9, 0);
		endSceneInit();
		break;
	}

	while (!_sceneDone) {

		updateMouseCursor();
		updateCursorByHotspot();
		_sceneClickedHotspot = -1;
		if (_gnapActionStatus < 0)
			_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);

		switch (_sceneClickedHotspot) {

		case kHSDevice:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene27_updateHotspots();
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
						playBeaverSequence(getBeaverSequenceId(0, 0, 0) | 0x10000);
						break;
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSJanitor:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex == kItemPicture) {
					_gnapIdleFacing = kDirUpLeft;
					if (gnapWalkTo(_hotspotsWalkPos[kHSJanitor].x, _hotspotsWalkPos[kHSJanitor].y, 0, 0x107BC, 1))
						_gnapActionStatus = kASShowPictureToJanitor;
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSJanitor].x, _hotspotsWalkPos[kHSJanitor].y, 7, 3);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(6, 3);
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = kDirUpLeft;
						gnapWalkTo(_hotspotsWalkPos[kHSJanitor].x, _hotspotsWalkPos[kHSJanitor].y, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = kASTalkJanitor;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSBucket:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowItem(_grabCursorSpriteIndex, 3, 3);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(3, 3);
						break;
					case GRAB_CURSOR:
						_gnapIdleFacing = kDirUpLeft;
						gnapWalkTo(_gnapX, _gnapY, 0, getGnapSequenceId(gskIdle, _hotspotsWalkPos[kHSBucket].x, _hotspotsWalkPos[kHSBucket].y) | 0x10000, 1);
						_gnapActionStatus = kASGrabBucket;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitCircus:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 26;
				gnapWalkTo(-1, _hotspotsWalkPos[kHSExitCircus].y, 0, 0x107AE, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitCircus].x + 1, _hotspotsWalkPos[kHSExitCircus].y, -1, 0x107C7, 1);
			}
			break;

		case kHSExitArcade:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 29;
				gnapWalkTo(-1, _hotspotsWalkPos[kHSExitArcade].y, 0, 0x107AF, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitArcade].x, _hotspotsWalkPos[kHSExitArcade].y + 1, -1, 0x107CF, 1);
			}
			break;

		case kHSExitBeerStand:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = 1;
				_newSceneNum = 31;
				gnapWalkTo(-1, _hotspotsWalkPos[kHSExitBeerStand].y, 0, 0x107AB, 1);
				_gnapActionStatus = kASLeaveScene;
				platypusWalkTo(_hotspotsWalkPos[kHSExitBeerStand].x, _hotspotsWalkPos[kHSExitBeerStand].y + 1, -1, 0x107CD, 1);
			}
			break;

		case kHSExitClown:
			if (_gnapActionStatus < 0) {
				if (isFlag(0)) {
					_isLeavingScene = true;
					_newSceneNum = 28;
					gnapWalkTo(_hotspotsWalkPos[kHSExitClown].x, _hotspotsWalkPos[kHSExitClown].y, 0, 0x107AD, 1);
					_gnapActionStatus = kASLeaveScene;
					platypusWalkTo(_hotspotsWalkPos[kHSExitClown].x + 1, _hotspotsWalkPos[kHSExitClown].y, -1, 0x107C4, 1);
				} else {
					_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
					gnapWalkTo(_hotspotsWalkPos[kHSExitClown].x, 7, 0, 0x107BC, 1);
					_hotspots[kHSWalkArea1]._flags &= SF_WALKABLE;
					_gnapActionStatus = kASTryEnterClownTent;
				}
			}
			break;

		case kHSWalkArea1:
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
	
		scene27_updateAnimations();

		if (!isSoundPlaying(0x1093B))
			playSound(0x1093B, 1);

		if (!_isLeavingScene) {
			updateBeaverIdleSequence();
			if (_gnapActionStatus < 0)
				updateGnapIdleSequence();
			if (_debugLevel == 4)
				updateIdleTimer();
			if (!_timers[5]) {
				_timers[5] = getRandom(20) + 60;
				if (_gnapActionStatus < 0) {
					if (getRandom(3) != 0)
						_s27_nextJanitorSequenceId = 0xCB;
					else
						_s27_nextJanitorSequenceId = 0xCF;
				}
			}
			if (!_timers[7]) {
				_timers[7] = getRandom(100) + 300;
				if (_gnapActionStatus < 0)
					_gameSys->insertSequence(0xD4, 120, 0, 0, kSeqNone, 0, 0, 0);
			}
			playSoundB();
		}

		checkGameKeys();

		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene27_updateHotspots();
		}
		
		gameUpdateTick();

	}
	
}

void GnapEngine::scene27_updateAnimations() {
	
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case kASTalkJanitor:
			switch (getRandom(3)) {
			case 0:
				_s27_nextJanitorSequenceId = 0xCC;
				break;
			case 1:
				_s27_nextJanitorSequenceId = 0xCD;
				break;
			case 2:
				_s27_nextJanitorSequenceId = 0xCE;
				break;
			}
			break;
		case kASGrabBucket:
			playGnapPullOutDevice(0, 0);
			playGnapUseDevice(0, 0);
			_hotspots[kHSBucket]._flags = SF_DISABLED;
			invAdd(kItemEmptyBucket);
			setFlag(13);
			_gameSys->setAnimation(0xD2, 39, 0);
			_gameSys->insertSequence(0xD2, 39, 211, 39, kSeqSyncWait, 0, 0, 0);
			_gnapActionStatus = kASGrabBucketDone;
			break;
		case kASGrabBucketDone:
			setGrabCursorSprite(kItemEmptyBucket);
			_gnapActionStatus = -1;
			break;
		case kASShowPictureToJanitor:
			_s27_nextJanitorSequenceId = 0xD0;
			break;
		case kASTryEnterClownTent:
			_s27_nextJanitorSequenceId = 0xD1;
			_gameSys->insertSequence(0xD1, 39, _s27_currJanitorSequenceId, 39, kSeqSyncExists, 0, 0, 0);
			_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 3);
			_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 0);
			_s27_currJanitorSequenceId = _s27_nextJanitorSequenceId;
			_s27_nextJanitorSequenceId = -1;
			_gnapActionStatus = kASTryEnterClownTentDone;
			break;
		case kASTryEnterClownTentDone:
			_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
			gnapWalkTo(_hotspotsWalkPos[7].x, 9, -1, 0x107BC, 1);
			_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
			_gnapActionStatus = -1;
			break;
		case kASEnterClownTent:
			gnapWalkTo(_gnapX, _gnapY, 0, 0x107B2, 1);
			_gnapActionStatus = kASLeaveScene;
			break;
		case kASLeaveScene:
			_sceneDone = true;
			break;
		}
	}

	if (_gameSys->getAnimationStatus(3) == 2) {
		switch (_s27_nextJanitorSequenceId) {
		case -1:
			_s27_nextJanitorSequenceId = 0xCB;
			_gameSys->insertSequence(0xCB, 39, _s27_currJanitorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 3);
			_s27_currJanitorSequenceId = _s27_nextJanitorSequenceId;
			_s27_nextJanitorSequenceId = -1;
			break;
		case 0xCC:
		case 0xCD:
		case 0xCE:
			_gnapActionStatus = -1;
			_gameSys->insertSequence(_s27_nextJanitorSequenceId, 39, _s27_currJanitorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 3);
			_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 0);
			_s27_currJanitorSequenceId = _s27_nextJanitorSequenceId;
			_s27_nextJanitorSequenceId = -1;
			break;
		case 0xD0:
			// Show picture to janitor
			playGnapPullOutDevice(0, 0);
			playGnapUseDevice(0, 0);
			_gameSys->insertSequence(_s27_nextJanitorSequenceId, 39, _s27_currJanitorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 0);
			_gnapActionStatus = kASEnterClownTent;
			_s27_currJanitorSequenceId = _s27_nextJanitorSequenceId;
			_s27_nextJanitorSequenceId = -1;
			setFlag(0);
			setGrabCursorSprite(-1);
			invRemove(kItemPicture);
			_newSceneNum = 28;
			break;
		default:
			_gameSys->insertSequence(_s27_nextJanitorSequenceId, 39, _s27_currJanitorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 3);
			_s27_currJanitorSequenceId = _s27_nextJanitorSequenceId;
			_s27_nextJanitorSequenceId = -1;
			break;
		}
	}
  
}

} // End of namespace Gnap
