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
	kHSHorn				= 1,
	kHSClown			= 2,
	kHSExitOutsideClown	= 3,
	kHSUnknown			= 4,
	kHSDevice			= 5,
	kHSWalkArea1		= 6,
	kHSWalkArea2		= 7
};

int GnapEngine::scene28_init() {
	return 0x125;
}

void GnapEngine::scene28_updateHotspots() {
	setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(kHSHorn, 148, 352, 215, 383, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	setHotspot(kHSClown, 130, 250, 285, 413, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 5);
	setHotspot(kHSExitOutsideClown, 660, 190, 799, 400, SF_EXIT_R_CURSOR, 9, 6);
	setHotspot(kHSUnknown, 582, 421, 643, 478, SF_WALKABLE | SF_DISABLED, 9, 7);
	setHotspot(kHSWalkArea1, 0, 0, 799, 523);
	setHotspot(kHSWalkArea2, 0, 0, 0, 0, 7, SF_DISABLED);
	setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (invHas(kItemHorn))
		_hotspots[kHSHorn].flags = SF_DISABLED;
	if (isFlag(22))
		_hotspots[kHSUnknown].flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	_hotspotsCount = 8;
}

void GnapEngine::scene28_run() {

	playSound(0x1093C, 1);
	_s28_dword_47EA60 = -1;
	queueInsertDeviceIcon();
	_gameSys->insertSequence(0x124, 255, 0, 0, kSeqNone, 0, 0, 0);

	if (isFlag(22))
		_gameSys->insertSequence(0x112, 99, 0, 0, kSeqNone, 0, 0, 0);

	if (isFlag(1)) {
		if (isFlag(21)) {
			_gameSys->setAnimation(0x11C, 39, 3);
			_gameSys->insertSequence(0x11C, 39, 0, 0, kSeqNone, 0, 0, 0);
			if (!invHas(kItemHorn))
				_gameSys->insertSequence(0x118, 59, 0, 0, kSeqNone, 0, 0, 0);
			_s28_dword_47EA5C = 0x11C;
		} else {
			_s28_dword_47EA5C = 0x11B;
			_gameSys->setAnimation(0x11B, 39, 3);
			_gameSys->insertSequence(_s28_dword_47EA5C, 39, 0, 0, kSeqNone, 0, 0, 0);
			_timers[4] = getRandom(20) + 80;
		}
		initGnapPos(8, 8, 3);
		initBeaverPos(9, 8, 4);
		endSceneInit();
	} else {
		_gameSys->insertSequence(0x11B, 39, 0, 0, kSeqNone, 0, 0, 0);
		initGnapPos(8, 8, 3);
		initBeaverPos(9, 8, 4);
		endSceneInit();
		playSequences(0xF7, 0x121, 0x122, 0x123);
		_s28_dword_47EA5C = 0x115;
		setFlag(1);
		_gameSys->setAnimation(0x115, 39, 3);
		_gameSys->insertSequence(_s28_dword_47EA5C, 39, 283, 39, kSeqSyncWait, 0, 0, 0);
		_s28_dword_47EA60 = -1;
		_timers[4] = getRandom(20) + 80;
		_gnapActionStatus = 9;
		while (_gameSys->getAnimationStatus(3) != 2) {
			gameUpdateTick();
			updateMouseCursor();
		}
		_gnapActionStatus = -1;
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
				scene28_updateHotspots();
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

		case kHSHorn:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(2, 8, 3, 4);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(2, 4);
						break;
					case GRAB_CURSOR:
						if (isFlag(21)) {
							if (!invHas(kItemHorn)) {
								gnapWalkTo(_gnapX, _gnapY, 0, getGnapSequenceId(gskIdle, _hotspotsWalkPos[kHSHorn].x, _hotspotsWalkPos[kHSHorn].y) | 0x10000, 1);
								_gnapActionStatus = 4;
							}
						} else {
							_gnapIdleFacing = 5;
							gnapWalkTo(2, 8, 0, 0x107BB, 1);
							_hotspots[kHSWalkArea1].flags |= SF_WALKABLE;
							gnapWalkTo(_hotspotsWalkPos[kHSHorn].x, _hotspotsWalkPos[kHSHorn].y, 0, 0x107BB, 1);
							_hotspots[kHSWalkArea1].flags &= ~SF_WALKABLE;
							_gnapActionStatus = 2;
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

		case kHSClown:
			if (_gnapActionStatus < 0) {
				if (isFlag(21)) {
					if (_verbCursor == LOOK_CURSOR)
						playGnapScratchingHead(5, 2);
					else
						playGnapImpossible(0, 0);
				} else if (_grabCursorSpriteIndex == kItemBucketWithBeer) {
					_gnapIdleFacing = 5;
					gnapWalkTo(_hotspotsWalkPos[kHSClown].x, _hotspotsWalkPos[kHSClown].y, 0, 0x107BC, 1);
					playGnapPullOutDevice(0, 0);
					playGnapUseDevice(0, 0);
					_gnapActionStatus = 0;
				} else if (_grabCursorSpriteIndex == kItemBucketWithPill) {
					_gnapIdleFacing = 5;
					gnapWalkTo(_hotspotsWalkPos[kHSClown].x, _hotspotsWalkPos[kHSClown].y, 0, 0x107BC, 1);
					playGnapPullOutDevice(0, 0);
					playGnapUseDevice(0, 0);
					_gnapActionStatus = 1;
				} else if (_grabCursorSpriteIndex >= 0) {
					playGnapShowCurrItem(_hotspotsWalkPos[kHSClown].x, _hotspotsWalkPos[kHSClown].y, 2, 4);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(5, 2);
						break;
					case TALK_CURSOR:
						_gnapIdleFacing = 5;
						gnapWalkTo(5, 8, 0, getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_gnapActionStatus = 8;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitOutsideClown:
			if (_gnapActionStatus < 0) {
				_isLeavingScene = true;
				_newSceneNum = 27;
				_hotspots[kHSWalkArea1].flags |= SF_WALKABLE;
				gnapWalkTo(_hotspotsWalkPos[kHSExitOutsideClown].x, _hotspotsWalkPos[kHSExitOutsideClown].y, 0, 0x107BF, 1);
				_gnapActionStatus = 10;
				_hotspots[kHSWalkArea1].flags &= ~SF_WALKABLE;
				platypusWalkTo(_hotspotsWalkPos[kHSExitOutsideClown].x - 1, _hotspotsWalkPos[kHSExitOutsideClown].y, -1, 0x107C2, 1);
			}
			break;

		case kHSUnknown:
			if (_gnapActionStatus < 0) {
				if (_grabCursorSpriteIndex >= 0) {
					playGnapShowItem(_grabCursorSpriteIndex, 8, 6);
				} else if (isFlag(21)) {
					playGnapImpossible(8, 6);
				} else {
					switch (_verbCursor) {
					case LOOK_CURSOR:
						playGnapScratchingHead(8, 6);
						break;
					case GRAB_CURSOR:
						gnapWalkTo(_gnapX, _gnapY, 0, getGnapSequenceId(gskIdle, _hotspotsWalkPos[kHSUnknown].x, _hotspotsWalkPos[kHSUnknown].y) | 0x10000, 1);
						_gnapActionStatus = 3;
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
			if (_gnapActionStatus < 0)
				gnapWalkTo(-1, -1, -1, -1, 1);
			break;
			
		default:
			if (_mouseClickState.left && _gnapActionStatus < 0) {
				gnapWalkTo(-1, -1, -1, -1, 1);
				_mouseClickState.left = false;
			}
			break;
			
		}

		scene28_updateAnimations();

		if (!isSoundPlaying(0x1093C))
			playSound(0x1093C, 1);

		if (!_isLeavingScene) {
			updateBeaverIdleSequence();
			updateGnapIdleSequence();
			if (!_timers[4]) {
				_timers[4] = getRandom(20) + 80;
				if (_gnapActionStatus < 0 && !isFlag(21))
					_s28_dword_47EA60 = 0x114;
			}
		}

		checkGameKeys();

		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene28_updateHotspots();
		}
		
		gameUpdateTick();

	}
	
}

void GnapEngine::scene28_updateAnimations() {
	
	if (_gameSys->getAnimationStatus(0) == 2) {
		_gameSys->setAnimation(0, 0, 0);
		switch (_gnapActionStatus) {
		case 0:
			setFlag(22);
			_s28_dword_47EA60 = 275;
			invRemove(kItemBucketWithBeer);
			scene28_updateHotspots();
			break;
		case 1:
			_s28_dword_47EA60 = 0x116;
			invRemove(kItemBucketWithPill);
			setFlag(22);
			setFlag(21);
			scene28_updateHotspots();
			break;
		case 2:
			_s28_dword_47EA60 = 0x119;
			break;
		case 3:
			playGnapPullOutDevice(0, 0);
			playGnapUseDevice(0, 0);
			_gameSys->setAnimation(0x111, 99, 0);
			_gameSys->insertSequence(0x111, 99, 274, 99, kSeqSyncWait, 0, 0, 0);
			_gnapActionStatus = 6;
			break;
		case 4:
			playGnapPullOutDevice(0, 0);
			playGnapUseDevice(0, 0);
			_gameSys->setAnimation(0x117, 59, 0);
			_gameSys->insertSequence(0x117, 59, 280, 59, kSeqSyncWait, 0, 0, 0);
			_gnapActionStatus = 5;
			break;
		case 5:
			hideCursor();
			setGrabCursorSprite(-1);
			addFullScreenSprite(0xF6, 255);
			_gameSys->setAnimation(0x120, 256, 0);
			_gameSys->insertSequence(0x120, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_gameSys->getAnimationStatus(0) != 2)
				gameUpdateTick();
			removeFullScreenSprite();
			showCursor();
			setGrabCursorSprite(kItemHorn);
			invAdd(kItemHorn);
			scene28_updateHotspots();
			_gnapActionStatus = -1;
			break;
		case 6:
			setGrabCursorSprite(kItemEmptyBucket);
			clearFlag(22);;
			scene28_updateHotspots();
			invAdd(kItemEmptyBucket);
			_gnapActionStatus = -1;
			break;
		case 7:
			_gameSys->insertSequence(0x107B5, _gnapId, 281, 39, kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
			_gnapSequenceId = 0x7B5;
			_gnapSequenceDatNum = 1;
			_gameSys->insertSequence(0x11B, 39, 0, 0, kSeqNone, 0, 0, 0);
			_s28_dword_47EA5C = 0x11B;
			_s28_dword_47EA60 = -1;
			_gnapActionStatus = -1;
			gnapWalkTo(2, 8, -1, 0x107BB, 1);
			break;
		case 8:
			_s28_dword_474938 = (_s28_dword_474938 + 1) % 2;
			if (_s28_dword_474938 == 0)
				_s28_dword_47EA60 = 0x11D;
			else if (_s28_dword_474938 == 1)
				_s28_dword_47EA60 = 0x11E;
			else if (_s28_dword_474938 == 2)
				_s28_dword_47EA60 = 0x11F; // NOTE CHECKME Never set, bug in the original?
			break;
		case 9:
			_gnapActionStatus = -1;
			break;
		case 10:
			_sceneDone = true;
			break;
		}
	}

	if (_gameSys->getAnimationStatus(3) == 2) {
		switch (_s28_dword_47EA60) {
		case 0x113:
			setGrabCursorSprite(-1);
			_gameSys->setAnimation(_s28_dword_47EA60, 39, 0);
			_gameSys->insertSequence(0x112, 99, 0, 0, kSeqNone, getSequenceTotalDuration(_s28_dword_47EA60), 0, 0);
			_gameSys->insertSequence(_s28_dword_47EA60, 39, _s28_dword_47EA5C, 39, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x11B, 39, _s28_dword_47EA60, 39, kSeqSyncWait, 0, 0, 0);
			_s28_dword_47EA5C = 0x11B;
			_s28_dword_47EA60 = -1;
			_gnapActionStatus = 9;
			break;
		case 0x116:
			setGrabCursorSprite(-1);
			_gameSys->setAnimation(_s28_dword_47EA60, 39, 0);
			_gameSys->insertSequence(0x112, 99, 0, 0, kSeqNone, getSequenceTotalDuration(_s28_dword_47EA60), 0, 0);
			_gameSys->insertSequence(_s28_dword_47EA60, 39, _s28_dword_47EA5C, 39, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x11C, 39, _s28_dword_47EA60, 39, kSeqSyncWait, 0, 0, 0);
			_gameSys->insertSequence(0x118, 59, 0, 0, kSeqNone, getSequenceTotalDuration(_s28_dword_47EA60), 0, 0);
			_s28_dword_47EA5C = _s28_dword_47EA60;
			_s28_dword_47EA60 = -1;
			_gnapActionStatus = 9;
			break;
		case 0x11D:
		case 0x11E:
		case 0x11F:
			_gnapActionStatus = -1;
			break;
		case 0x119:
			_gameSys->insertSequence(_s28_dword_47EA60, 39, makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s28_dword_47EA60, 39, 0);
			_gameSys->removeSequence(_s28_dword_47EA5C, 39, 1);
			_gnapActionStatus = 7;
			_gnapSequenceId = _s28_dword_47EA60;
			_gnapSequenceDatNum = 0;
			_s28_dword_47EA60 = -1;
			break;
		}
		if (_s28_dword_47EA60 != -1) {
			_gameSys->insertSequence(_s28_dword_47EA60, 39, _s28_dword_47EA5C, 39, kSeqSyncWait, 0, 0, 0);
			_gameSys->setAnimation(_s28_dword_47EA60, 39, 3);
			_s28_dword_47EA5C = _s28_dword_47EA60;
			_s28_dword_47EA60 = -1;
		}
	}
	
}

} // End of namespace Gnap
