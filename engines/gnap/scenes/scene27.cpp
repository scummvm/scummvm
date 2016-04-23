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
#include "gnap/scenes/scene27.h"

namespace Gnap {

enum {
	kHSPlatypus			= 0,
	kHSJanitor			= 1,
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

Scene27::Scene27(GnapEngine *vm) : Scene(vm) {
	_s27_nextJanitorSequenceId = -1;
	_s27_currJanitorSequenceId = -1;
}

int Scene27::init() {
	return 0xD5;
}

void Scene27::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSJanitor, 488, 204, 664, 450, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 8, 8);
	_vm->setHotspot(kHSBucket, 129, 406, 186, 453, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 6);
	_vm->setHotspot(kHSExitCircus, 200, 585, 700, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
	_vm->setHotspot(kHSExitArcade, 0, 0, 15, 600, SF_EXIT_L_CURSOR, 0, 6);
	_vm->setHotspot(kHSExitBeerStand, 785, 0, 800, 600, SF_EXIT_R_CURSOR, 11, 7);
	_vm->setHotspot(kHSExitClown, 340, 240, 460, 420, SF_EXIT_U_CURSOR, 6, 8);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 507);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->isFlag(kGFUnk13))
		_vm->_hotspots[kHSBucket]._flags = SF_DISABLED;
	_vm->_hotspotsCount = 9;
}

void Scene27::run() {
	_vm->playSound(0x1093B, true);
	_vm->startSoundTimerB(4);
	_vm->_timers[7] = _vm->getRandom(100) + 300;
	_vm->queueInsertDeviceIcon();

	if (!_vm->isFlag(kGFUnk13))
		_vm->_gameSys->insertSequence(0xD3, 39, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->_debugLevel == 4)
		_vm->startIdleTimer(6);

	_vm->_gameSys->insertSequence(0xCB, 39, 0, 0, kSeqNone, 0, 0, 0);
	
	_s27_currJanitorSequenceId = 0xCB;
	_s27_nextJanitorSequenceId = -1;
	
	_vm->_gameSys->setAnimation(0xCB, 39, 3);
	_vm->_timers[5] = _vm->getRandom(20) + 60;

	switch (_vm->_prevSceneNum) {
	case 26:
		_vm->initGnapPos(7, 12, kDirBottomRight);
		_vm->initPlatypusPos(6, 12, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(7, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
		break;
	case 29:
		_vm->initGnapPos(-1, 8, kDirBottomRight);
		_vm->initPlatypusPos(-1, 9, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(3, 8, -1, 0x107B9, 1);
		_vm->platypusWalkTo(3, 9, -1, 0x107C2, 1);
		break;
	case 31:
		_vm->initGnapPos(12, 8, kDirBottomLeft);
		_vm->initPlatypusPos(12, 9, kDirUnk4);
		_vm->endSceneInit();
		_vm->gnapWalkTo(8, 8, -1, 0x107BA, 1);
		_vm->platypusWalkTo(8, 9, -1, 0x107C2, 1);
		break;
	default:
		_vm->initGnapPos(6, 8, kDirBottomRight);
		_vm->initPlatypusPos(5, 9, kDirNone);
		_vm->endSceneInit();
		break;
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
		_vm->_sceneClickedHotspot = -1;
		if (_vm->_gnapActionStatus < 0)
			_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHSDevice:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHSPlatypus:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					_vm->gnapUseJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan1(_vm->_platX, _vm->_platY);
						break;
					case GRAB_CURSOR:
						_vm->gnapKissPlatypus(0);
						break;
					case TALK_CURSOR:
						_vm->playGnapBrainPulsating(_vm->_platX, _vm->_platY);
						_vm->playPlatypusSequence(_vm->getPlatypusSequenceId());
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSJanitor:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemPicture) {
					_vm->_gnapIdleFacing = kDirUpLeft;
					if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSJanitor].x, _vm->_hotspotsWalkPos[kHSJanitor].y, 0, 0x107BC, 1))
						_vm->_gnapActionStatus = kASShowPictureToJanitor;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSJanitor].x, _vm->_hotspotsWalkPos[kHSJanitor].y, 7, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(6, 3);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSJanitor].x, _vm->_hotspotsWalkPos[kHSJanitor].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkJanitor;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSBucket:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 3, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(3, 3);
						break;
					case GRAB_CURSOR:
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[kHSBucket].x, _vm->_hotspotsWalkPos[kHSBucket].y) | 0x10000, 1);
						_vm->_gnapActionStatus = kASGrabBucket;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitCircus:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 26;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHSExitCircus].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitCircus].x + 1, _vm->_hotspotsWalkPos[kHSExitCircus].y, -1, 0x107C7, 1);
			}
			break;

		case kHSExitArcade:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 29;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHSExitArcade].y, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitArcade].x, _vm->_hotspotsWalkPos[kHSExitArcade].y + 1, -1, 0x107CF, 1);
			}
			break;

		case kHSExitBeerStand:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 31;
				_vm->gnapWalkTo(-1, _vm->_hotspotsWalkPos[kHSExitBeerStand].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitBeerStand].x, _vm->_hotspotsWalkPos[kHSExitBeerStand].y + 1, -1, 0x107CD, 1);
			}
			break;

		case kHSExitClown:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatypus)) {
					_vm->_isLeavingScene = true;
					_vm->_newSceneNum = 28;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitClown].x, _vm->_hotspotsWalkPos[kHSExitClown].y, 0, 0x107AD, 1);
					_vm->_gnapActionStatus = kASLeaveScene;
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitClown].x + 1, _vm->_hotspotsWalkPos[kHSExitClown].y, -1, 0x107C4, 1);
				} else {
					_vm->_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitClown].x, 7, 0, 0x107BC, 1);
					_vm->_hotspots[kHSWalkArea1]._flags &= SF_WALKABLE;
					_vm->_gnapActionStatus = kASTryEnterClownTent;
				}
			}
			break;

		case kHSWalkArea1:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;
			
		default:
			if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		
		}
	
		updateAnimations();

		if (!_vm->isSoundPlaying(0x1093B))
			_vm->playSound(0x1093B, true);

		if (!_vm->_isLeavingScene) {
			_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (_vm->_debugLevel == 4)
				_vm->updateIdleTimer();
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(20) + 60;
				if (_vm->_gnapActionStatus < 0) {
					if (_vm->getRandom(3) != 0)
						_s27_nextJanitorSequenceId = 0xCB;
					else
						_s27_nextJanitorSequenceId = 0xCF;
				}
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(100) + 300;
				if (_vm->_gnapActionStatus < 0)
					_vm->_gameSys->insertSequence(0xD4, 120, 0, 0, kSeqNone, 0, 0, 0);
			}
			_vm->playSoundB();
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}
}

void Scene27::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASTalkJanitor:
			switch (_vm->getRandom(3)) {
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
			_vm->playGnapPullOutDevice(0, 0);
			_vm->playGnapUseDevice(0, 0);
			_vm->_hotspots[kHSBucket]._flags = SF_DISABLED;
			_vm->invAdd(kItemEmptyBucket);
			_vm->setFlag(kGFUnk13);
			_vm->_gameSys->setAnimation(0xD2, 39, 0);
			_vm->_gameSys->insertSequence(0xD2, 39, 211, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapActionStatus = kASGrabBucketDone;
			break;
		case kASGrabBucketDone:
			_vm->setGrabCursorSprite(kItemEmptyBucket);
			_vm->_gnapActionStatus = -1;
			break;
		case kASShowPictureToJanitor:
			_s27_nextJanitorSequenceId = 0xD0;
			break;
		case kASTryEnterClownTent:
			_s27_nextJanitorSequenceId = 0xD1;
			_vm->_gameSys->insertSequence(0xD1, 39, _s27_currJanitorSequenceId, 39, kSeqSyncExists, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 3);
			_vm->_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 0);
			_s27_currJanitorSequenceId = _s27_nextJanitorSequenceId;
			_s27_nextJanitorSequenceId = -1;
			_vm->_gnapActionStatus = kASTryEnterClownTentDone;
			break;
		case kASTryEnterClownTentDone:
			_vm->_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
			_vm->gnapWalkTo(_vm->_hotspotsWalkPos[7].x, 9, -1, 0x107BC, 1);
			_vm->_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
			_vm->_gnapActionStatus = -1;
			break;
		case kASEnterClownTent:
			_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, 0x107B2, 1);
			_vm->_gnapActionStatus = kASLeaveScene;
			break;
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(3) == 2) {
		switch (_s27_nextJanitorSequenceId) {
		case -1:
			_s27_nextJanitorSequenceId = 0xCB;
			_vm->_gameSys->insertSequence(0xCB, 39, _s27_currJanitorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 3);
			_s27_currJanitorSequenceId = _s27_nextJanitorSequenceId;
			_s27_nextJanitorSequenceId = -1;
			break;
		case 0xCC:
		case 0xCD:
		case 0xCE:
			_vm->_gnapActionStatus = -1;
			_vm->_gameSys->insertSequence(_s27_nextJanitorSequenceId, 39, _s27_currJanitorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 3);
			_vm->_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 0);
			_s27_currJanitorSequenceId = _s27_nextJanitorSequenceId;
			_s27_nextJanitorSequenceId = -1;
			break;
		case 0xD0:
			// Show picture to janitor
			_vm->playGnapPullOutDevice(0, 0);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->insertSequence(_s27_nextJanitorSequenceId, 39, _s27_currJanitorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 0);
			_vm->_gnapActionStatus = kASEnterClownTent;
			_s27_currJanitorSequenceId = _s27_nextJanitorSequenceId;
			_s27_nextJanitorSequenceId = -1;
			_vm->setFlag(kGFPlatypus);
			_vm->setGrabCursorSprite(-1);
			_vm->invRemove(kItemPicture);
			_vm->_newSceneNum = 28;
			break;
		default:
			_vm->_gameSys->insertSequence(_s27_nextJanitorSequenceId, 39, _s27_currJanitorSequenceId, 39, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(_s27_nextJanitorSequenceId, 39, 3);
			_s27_currJanitorSequenceId = _s27_nextJanitorSequenceId;
			_s27_nextJanitorSequenceId = -1;
			break;
		}
	}
}

} // End of namespace Gnap
