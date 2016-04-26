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
#include "gnap/scenes/scene03.h"
namespace Gnap {

enum {
	kHS03Platypus			= 0,
	kHS03Grass				= 1,
	kHS03ExitTruck			= 2,
	kHS03Creek				= 3,
	kHS03TrappedPlatypus	= 4,
	kHS03Device				= 5,
	kHS03WalkAreas1			= 6,
	kHS03WalkAreas2			= 7,
	kHS03PlatypusWalkArea	= 8,
	kHS03WalkAreas3			= 9
};

enum {
	kAS03LeaveScene			= 0,
	kAS03FreePlatypus		= 1,
	kAS03HypnotizePlat		= 2,
	kAS03HypnotizeScaredPlat= 3,
	kAS03FreePlatypusDone	= 4,
	kAS03GrabPlatypus		= 5,
	kAS03GrabCreek			= 6,
	kAS03GrabCreekDone		= 7,
	kAS03GrabScaredPlatypus	= 8
};

Scene03::Scene03(GnapEngine *vm) : Scene(vm) {
	_nextPlatSequenceId = -1;
	_platypusScared = false;
	_platypusHypnotized = false;
	_nextFrogSequenceId = -1;
	_currFrogSequenceId = -1;
}

int Scene03::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	_vm->_gameSys->setAnimation(0, 0, 5);
	return 0x1CC;
}

void Scene03::updateHotspots() {
	_vm->setHotspot(kHS03Platypus, 0, 0, 0, 0, SF_DISABLED | SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS03Grass, 646, 408, 722, 458, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 6);
	_vm->setHotspot(kHS03ExitTruck, 218, 64, 371, 224, SF_EXIT_U_CURSOR | SF_WALKABLE, 4, 4);
	_vm->setHotspot(kHS03Creek, 187, 499, 319, 587, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	_vm->setHotspot(kHS03TrappedPlatypus, 450, 256, 661, 414, SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 2, 5);
	_vm->setHotspot(kHS03WalkAreas1, 0, 500, 300, 600);
	_vm->setHotspot(kHS03WalkAreas2, 300, 447, 800, 600);
	_vm->setHotspot(kHS03PlatypusWalkArea, 235, 0, 800, 600);
	_vm->setHotspot(kHS03WalkAreas3, 0, 0, 800, 354);
	_vm->setDeviceHotspot(kHS03Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFPlatypus))
		_vm->_hotspots[kHS03Platypus]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	if (_vm->isFlag(kGFGrassTaken))
		_vm->_hotspots[kHS03Grass]._flags = SF_WALKABLE | SF_DISABLED;
	if (_vm->isFlag(kGFPlatypus))
		_vm->_hotspots[kHS03TrappedPlatypus]._flags = SF_DISABLED;
	if (_vm->isFlag(kGFPlatypus) || _platypusHypnotized)
		_vm->_hotspots[kHS03PlatypusWalkArea]._flags |= SF_WALKABLE;
	_vm->_hotspotsCount = 10;
}

void Scene03::run() {
	_vm->playSound(0x10925, true);
	_vm->startSoundTimerC(7);
	
	_vm->_gameSys->insertSequence(0x1CA, 251, 0, 0, kSeqLoop, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x1CB, 251, 0, 0, kSeqLoop, 0, 0, 0);
	
	_platypusHypnotized = false;
	
	_vm->initGnapPos(3, 4, kDirBottomRight);
	
	_vm->_gameSys->insertSequence(0x1C6, 253, 0, 0, kSeqNone, 0, 0, 0);
	
	_currFrogSequenceId = 0x1C6;
	_nextFrogSequenceId = -1;
	_vm->_gameSys->setAnimation(0x1C6, 253, 2);
	
	_vm->_timers[6] = _vm->getRandom(20) + 30;
	_vm->_timers[4] = _vm->getRandom(100) + 300;
	_vm->_timers[5] = _vm->getRandom(100) + 200;
	
	if (_vm->isFlag(kGFPlatypus)) {
		_vm->initPlatypusPos(5, 4, kDirNone);
	} else {
		_vm->_timers[1] = _vm->getRandom(40) + 20;
		_vm->_gameSys->setAnimation(0x1C2, 99, 1);
		_vm->_gameSys->insertSequence(0x1C2, 99, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_platypusSequenceId = 0x1C2;
		_vm->_platypusSequenceDatNum = 0;
	}
	
	_vm->_gameSys->insertSequence(0x1C4, 255, 0, 0, kSeqNone, 0, 0, 0);
	
	if (!_vm->isFlag(kGFGrassTaken))
		_vm->_gameSys->insertSequence(0x1B2, 253, 0, 0, kSeqNone, 0, 0, 0);
	
	_vm->queueInsertDeviceIcon();
	
	_vm->endSceneInit();
	
	if (_vm->isFlag(kGFPlatypus))
		_vm->platypusWalkTo(4, 7, -1, 0x107C2, 1);
	_vm->gnapWalkTo(3, 6, -1, 0x107B9, 1);
	
	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		switch (_vm->_sceneClickedHotspot) {
		case kHS03Platypus:
			if (_vm->_gnapActionStatus < 0 && _vm->isFlag(kGFPlatypus)) {
				if (_vm->_grabCursorSpriteIndex == kItemDisguise) {
					_vm->gnapUseDisguiseOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFKeysTaken))
							_vm->playGnapMoan1(_vm->_platX, _vm->_platY);
						else
							_vm->playGnapScratchingHead(_vm->_platX, _vm->_platY);
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

		case kHS03Grass:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFGrassTaken)) {
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 9, 6);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(9, 6);
						break;
					case GRAB_CURSOR:
						_vm->playGnapPullOutDevice(9, 6);
						_vm->playGnapUseDevice(0, 0);
						_vm->_gameSys->insertSequence(0x1B3, 253, 0x1B2, 253, kSeqSyncWait, 0, 0, 0);
						_vm->_gameSys->setAnimation(0x1B3, 253, 5);
						_vm->_hotspots[kHS03Grass]._flags |= SF_WALKABLE | SF_DISABLED;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS03ExitTruck:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_hotspots[kHS03PlatypusWalkArea]._flags |= SF_WALKABLE;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, 0, 0x107AD, 1);
				_vm->_gnapActionStatus = kAS03LeaveScene;
				if (_vm->isFlag(kGFPlatypus))
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, -1, 0x107C2, 1);
				_vm->_hotspots[kHS03PlatypusWalkArea]._flags &= ~SF_WALKABLE;
				if (_vm->_cursorValue == 1)
					_vm->_newSceneNum = 2;
				else
					_vm->_newSceneNum = 33;
			}
			break;

		case kHS03Creek:
			if (_vm->_gnapActionStatus == -1) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(0, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan2(2, 8);
						break;
					case GRAB_CURSOR:
						if (!_vm->isFlag(kGFPlatypus))
							_vm->_hotspots[kHS03PlatypusWalkArea]._flags |= SF_WALKABLE;
						if (_vm->gnapWalkTo(_vm->_hotspotsWalkPos[3].x, _vm->_hotspotsWalkPos[3].y, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[3].x + 1, _vm->_hotspotsWalkPos[3].y + 1) | 0x10000, 1))
							_vm->_gnapActionStatus = kAS03GrabCreek;
						if (!_vm->isFlag(kGFPlatypus))
							_vm->_hotspots[kHS03PlatypusWalkArea]._flags &= ~SF_WALKABLE;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS03TrappedPlatypus:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatypus)) {
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, 8, 4);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(8, 4);
						break;
					case GRAB_CURSOR:
						if (_platypusHypnotized) {
							_vm->gnapWalkTo(7, 6, 0, 0x107B5, 1);
							_vm->_gnapActionStatus = kAS03FreePlatypus;
						} else {
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							if (_platypusScared)
								_vm->_gnapActionStatus = kAS03GrabScaredPlatypus;
							else
								_vm->_gnapActionStatus = kAS03GrabPlatypus;
						}
						break;
					case TALK_CURSOR:
						if (_platypusHypnotized) {
							_vm->playGnapBrainPulsating(8, 4);
						} else {
							_vm->_gnapIdleFacing = kDirBottomRight;
							_vm->gnapWalkTo(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, 0, _vm->getGnapSequenceId(gskIdle, 0, 0) | 0x10000, 1);
							if (_platypusScared)
								_vm->_gnapActionStatus = kAS03HypnotizeScaredPlat;
							else
								_vm->_gnapActionStatus = kAS03HypnotizePlat;
						}
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHS03Device:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS03WalkAreas1:
		case kHS03WalkAreas2:
		case kHS03WalkAreas3:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		case kHS03PlatypusWalkArea:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->isFlag(kGFPlatypus) || _platypusHypnotized) {
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				} else {
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, 0, 0x107B5, 1);
					if (_platypusScared)
						_vm->_gnapActionStatus = kAS03GrabScaredPlatypus;
					else
						_vm->_gnapActionStatus = kAS03GrabPlatypus;
				}
			}
			break;
			
		default:
			if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
			
		}

		updateAnimations();
	
		if (!_vm->isSoundPlaying(0x10925))
			_vm->playSound(0x10925, true);
	
		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0 && _vm->isFlag(kGFPlatypus))
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0)
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[1] && !_platypusScared) {
				_vm->_timers[1] = _vm->getRandom(40) + 20;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && !_vm->isFlag(kGFPlatypus) && !_platypusHypnotized)
					_nextPlatSequenceId = 450;
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(20) + 30;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextFrogSequenceId == -1) {
					if (_vm->getRandom(5) == 1)
						_nextFrogSequenceId = 0x1C6;
					else
						_nextFrogSequenceId = 0x1C7;
				}
			}
			if (!_vm->_timers[4]) {
				// Update bird animation
				_vm->_timers[4] = _vm->getRandom(100) + 300;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0)
					_vm->_gameSys->insertSequence(_vm->getRandom(2) != 0 ? 0x1C8 : 0x1C3, 253, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(100) + 200;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0) {
					_vm->_gameSys->setAnimation(0x1C5, 253, 4);
					_vm->_gameSys->insertSequence(0x1C5, 253, 0, 0, kSeqNone, 0, 0, 0);
				}
			}
			_vm->playSoundC();
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(100) + 200;
			_vm->_timers[4] = _vm->getRandom(100) + 300;
			_vm->_timers[6] = _vm->getRandom(20) + 30;
		}
		
		_vm->gameUpdateTick();
	}
}

void Scene03::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kAS03LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS03FreePlatypus:
			_nextPlatSequenceId = 0x1BC;
			break;
		case kAS03FreePlatypusDone:
			_vm->_gnapActionStatus = -1;
			_vm->_platX = 6;
			_vm->_platY = 6;
			_vm->_platypusFacing = kDirUnk4;
			_vm->_platypusId = 120;
			_vm->_gameSys->insertSequence(0x107CA, _vm->_platypusId, 0x1BC, 99,
				kSeqSyncWait, 0, 75 * _vm->_platX - _vm->_platGridX, 48 * _vm->_platY - _vm->_platGridY);
			_vm->_gameSys->insertSequence(0x1B7, 99, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_platypusSequenceDatNum = 1;
			_vm->_platypusSequenceId = 0x7CA;
			_vm->setFlag(kGFPlatypus);
			_nextPlatSequenceId = -1;
			updateHotspots();
			break;
		case kAS03HypnotizePlat:
			_vm->playGnapBrainPulsating(0, 0);
			_vm->addFullScreenSprite(0x106, 255);
			_vm->_gameSys->setAnimation(0x1C9, 256, 1);
			_vm->_gameSys->insertSequence(0x1C9, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(1) != 2)
				_vm->gameUpdateTick();
			_vm->removeFullScreenSprite();
			_vm->_gameSys->setAnimation(0x1BA, 99, 1);
			_vm->_gameSys->insertSequence(0x1BA, 99, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), 99, kSeqSyncExists, 0, 0, 0);
			_vm->_platypusSequenceDatNum = 0;
			_vm->_platypusSequenceId = 0x1BA;
			_vm->_gnapActionStatus = -1;
			_platypusHypnotized = true;
			updateHotspots();
			break;
		case kAS03HypnotizeScaredPlat:
			_vm->playGnapBrainPulsating(0, 0);
			_vm->_gameSys->insertSequence(0x1BF, 99, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), 99, kSeqSyncExists, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x1BF, 99, 1);
			while (_vm->_gameSys->getAnimationStatus(1) != 2)
				_vm->gameUpdateTick();
			_vm->addFullScreenSprite(0x106, 255);
			_vm->_gameSys->setAnimation(0x1C9, 256, 1);
			_vm->_gameSys->insertSequence(0x1C9, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(1) != 2)
				_vm->gameUpdateTick();
			_vm->removeFullScreenSprite();
			_vm->_gameSys->setAnimation(0x1BA, 99, 1);
			_vm->_gameSys->insertSequence(0x1BA, 99, 447, 99, kSeqSyncWait, 0, 0, 0);
			_vm->_platypusSequenceDatNum = 0;
			_vm->_platypusSequenceId = 0x1BA;
			_vm->_gnapActionStatus = -1;
			_platypusHypnotized = true;
			updateHotspots();
			break;
		case kAS03GrabPlatypus:
			_nextPlatSequenceId = 0x1BD;
			_platypusHypnotized = false;
			break;
		case kAS03GrabScaredPlatypus:
			_nextPlatSequenceId = 0x1C0;
			_platypusHypnotized = false;
			break;
		case kAS03GrabCreek:
			_vm->_gameSys->insertSequence(0x1B4, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x1B4, _vm->_gnapId, 0);
			_vm->_gnapSequenceId = 0x1B4;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapActionStatus = kAS03GrabCreekDone;
			break;
		default:
			_vm->_gnapActionStatus = -1;
			break;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(1) == 2) {
		if (_nextPlatSequenceId == 0x1BD || _nextPlatSequenceId == 0x1C0) {
			_vm->_gameSys->setAnimation(0, 0, 1);
			_platypusScared = true;
			_vm->_gameSys->insertSequence(0x1B5, _vm->_gnapId, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(_nextPlatSequenceId, 99, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), 99, kSeqSyncWait, 0, 0, 0);
			_vm->_gnapSequenceId = 0x1B5;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapIdleFacing = kDirNone;
			_vm->_platypusSequenceId = _nextPlatSequenceId;
			_vm->_platypusSequenceDatNum = 0;
			_vm->_gameSys->setAnimation(_nextPlatSequenceId, 99, 1);
			_nextPlatSequenceId = -1;
			_vm->_gnapActionStatus = -1;
		} else if (_nextPlatSequenceId == 0x1BC) {
			_vm->_gnapX = 3;
			_vm->_gnapY = 6;
			_vm->_gameSys->insertSequence(0x1B6, 120, makeRid(_vm->_gnapSequenceDatNum, _vm->_gnapSequenceId), _vm->_gnapId, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x1BC, 99, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), 99, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->setAnimation(0x1BC, 99, 0);
			_vm->_gnapId = 20 * _vm->_gnapY;
			_vm->_gnapSequenceId = 0x1B6;
			_vm->_gnapSequenceDatNum = 0;
			_vm->_gnapIdleFacing = kDirNone;
			_vm->_gnapActionStatus = kAS03FreePlatypusDone;
			_nextPlatSequenceId = -1;
		} else if (_nextPlatSequenceId == 0x1C2 && !_platypusScared) {
			_vm->_gameSys->setAnimation(0, 0, 1);
			_vm->_gameSys->insertSequence(0x1C2, 99, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), 99, kSeqSyncWait, 0, 0, 0);
			_vm->_platypusSequenceId = 0x1C2;
			_vm->_platypusSequenceDatNum = 0;
			_vm->_gameSys->setAnimation(0x1C2, 99, 1);
			_nextPlatSequenceId = -1;
		} else if (_nextPlatSequenceId == -1 && _platypusScared && !_platypusHypnotized) {
			_vm->_gameSys->setAnimation(0, 0, 1);
			_vm->_gameSys->setAnimation(0x1BE, 99, 1);
			_vm->_gameSys->insertSequence(0x1BE, 99, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), 99, kSeqSyncWait, 0, 0, 0);
			_vm->_platypusSequenceId = 0x1BE;
			_vm->_platypusSequenceDatNum = 0;
			_nextPlatSequenceId = -1;
		}
	}
	
	if (_vm->_gameSys->getAnimationStatus(2) == 2 && _nextFrogSequenceId != -1) {
		_vm->_gameSys->setAnimation(_nextFrogSequenceId, 253, 2);
		_vm->_gameSys->insertSequence(_nextFrogSequenceId, 253, _currFrogSequenceId, 253, kSeqSyncWait, 0, 0, 0);
		_currFrogSequenceId = _nextFrogSequenceId;
		_nextFrogSequenceId = -1;
	}
	
	if (_vm->_gameSys->getAnimationStatus(5) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 5);
		_vm->invAdd(kItemGrass);
		_vm->setGrabCursorSprite(kItemGrass);
		_vm->setFlag(kGFGrassTaken);
		updateHotspots();
	}
}

} // End of namespace Gnap
