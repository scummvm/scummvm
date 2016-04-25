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
#include "gnap/scenes/scene44.h"

namespace Gnap {

enum {
	kHSPlatypus		= 0,
	kHSExitUfoParty	= 1,
	kHSExitUfo		= 2,
	kHSExitShow		= 3,
	kHSKissingLady	= 4,
	kHSSpring		= 5,
	kHSSpringGuy	= 6,
	kHSDevice		= 7,
	kHSWalkArea1	= 8,
	kHSWalkArea2	= 9
};

enum {
	kHSUfoExitLeft	= 1,
	kHSUfoExitRight	= 2,
	kHSUfoDevice	= 3
};

Scene44::Scene44(GnapEngine *vm) : Scene(vm) {
	_nextSpringGuySequenceId = -1;
	_nextKissingLadySequenceId = -1;
	_currSpringGuySequenceId = -1;
	_currKissingLadySequenceId = -1;
}

int Scene44::init() {
	GameSys gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	gameSys.setAnimation(0, 0, 3);
	return 0xFF;
}

void Scene44::updateHotspots() {
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_DISABLED);
		_vm->setHotspot(kHSUfoExitLeft, 0, 0, 10, 599, SF_EXIT_L_CURSOR);
		_vm->setHotspot(kHSUfoExitRight, 790, 0, 799, 599, SF_EXIT_R_CURSOR);
		_vm->setDeviceHotspot(kHSUfoDevice, -1, 534, -1, 599);
		_vm->_hotspotsCount = 4;
	} else {
		_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
		_vm->setHotspot(kHSExitUfoParty, 150, 580, 650, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 9);
		_vm->setHotspot(kHSExitUfo, 0, 100, 10, 599, SF_EXIT_L_CURSOR, 0, 8);
		_vm->setHotspot(kHSExitShow, 790, 100, 799, 599, SF_EXIT_R_CURSOR, 10, 8);
		_vm->setHotspot(kHSKissingLady, 300, 160, 400, 315, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
		_vm->setHotspot(kHSSpring, 580, 310, 635, 375, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
		_vm->setHotspot(kHSSpringGuy, 610, 375, 690, 515, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 8);
		_vm->setHotspot(kHSWalkArea1, 0, 0, 800, 445);
		_vm->setHotspot(kHSWalkArea2, 617, 0, 800, 600);
		_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
		if (_vm->isFlag(kGFUnk13))
			_vm->_hotspots[kHSKissingLady]._flags = SF_DISABLED;
		if (_vm->isFlag(kGFSpringTaken))
			_vm->_hotspots[kHSSpring]._flags = SF_DISABLED;
		_vm->_hotspotsCount = 10;
	}
}

void Scene44::run() {
	GameSys gameSys = *_vm->_gameSys;

	_vm->queueInsertDeviceIcon();

	gameSys.insertSequence(0xF7, 0, 0, 0, kSeqLoop, 0, 0, 0);
	gameSys.insertSequence(0xFC, 256, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFSpringTaken))
		_currSpringGuySequenceId = 0xF8;
	else
		_currSpringGuySequenceId = 0xF9;
	
	_nextSpringGuySequenceId = -1;
	gameSys.setAnimation(_currSpringGuySequenceId, 1, 4);
	gameSys.insertSequence(_currSpringGuySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFUnk13)) {
		if (_vm->_prevSceneNum != 50 || _vm->_sceneSavegameLoaded) {
			_currKissingLadySequenceId = 0xF6;
			_nextKissingLadySequenceId = -1;
		} else {
			_vm->setGrabCursorSprite(kItemGum);
			_currKissingLadySequenceId = 0xF5;
			_nextKissingLadySequenceId = 0xF6;
			gameSys.setAnimation(0xF5, 1, 2);
		}
	} else {
		_currKissingLadySequenceId = 0xEC;
		_nextKissingLadySequenceId = -1;
		gameSys.setAnimation(0xEC, 1, 2);
	}
	
	gameSys.insertSequence(_currKissingLadySequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
	
	if (_vm->isFlag(kGFGnapControlsToyUFO)) {
		_vm->_toyUfoId = 0;
		_vm->_toyUfoActionStatus = -1;
		_vm->_toyUfoSequenceId = _vm->toyUfoGetSequenceId();
		_vm->_toyUfoNextSequenceId = _vm->_toyUfoSequenceId;
		if (_vm->_prevSceneNum == 43)
			_vm->_toyUfoX = 30;
		else
			_vm->_toyUfoX = 770;
		gameSys.setAnimation(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 3);
		gameSys.insertSequence(_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId, 0, 0, kSeqNone, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
		_vm->endSceneInit();
	} else {
		switch (_vm->_prevSceneNum) {
		case 43:
			_vm->initGnapPos(-1, 8, kDirUpRight);
			_vm->initPlatypusPos(-1, 7, kDirUpLeft);
			_vm->endSceneInit();
			_vm->gnapWalkTo(2, 8, -1, 0x107B9, 1);
			_vm->platypusWalkTo(1, 8, -1, 0x107C2, 1);
			break;
		case 46:
			_vm->initGnapPos(11, 8, kDirUpRight);
			_vm->initPlatypusPos(11, 8, kDirUpLeft);
			_vm->endSceneInit();
			_vm->gnapWalkTo(6, 8, -1, 0x107BA, 1);
			_vm->platypusWalkTo(7, 8, -1, 0x107D2, 1);
			break;
		case 50:
			_vm->initGnapPos(4, 8, kDirBottomRight);
			if (_vm->_sceneSavegameLoaded) {
				_vm->initPlatypusPos(_vm->_hotspotsWalkPos[4].x, _vm->_hotspotsWalkPos[4].y, kDirUnk4);
			} else if (!_vm->isFlag(kGFUnk13)) {
				_vm->_timers[0] = 50;
				_vm->_timers[1] = 20;
				_vm->_platX = 5;
				_vm->_platY = 8;
				_vm->_platypusSequenceId = 0xFD;
				_vm->_platypusFacing = kDirNone;
				_vm->_platypusId = 160;
				_vm->_platypusSequenceDatNum = 0;
				gameSys.insertSequence(0xFD, 160, 0, 0, kSeqNone, 0, 0, 0);
			}
			_vm->endSceneInit();
			break;
		default:
			_vm->initGnapPos(5, 11, kDirUpRight);
			_vm->initPlatypusPos(6, 11, kDirUpLeft);
			_vm->endSceneInit();
			_vm->platypusWalkTo(6, 8, -1, 0x107C2, 1);
			_vm->gnapWalkTo(5, 8, -1, 0x107BA, 1);
			break;
		}
	}
	
	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1094B))
			_vm->playSound(0x1094B, true);
	
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
	
		_vm->testWalk(0, 0, -1, -1, -1, -1);
	
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);
	
		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			switch (_vm->_sceneClickedHotspot) {
			case kHSUfoExitLeft:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 6;
					_vm->_newSceneNum = 43;
					_vm->toyUfoFlyTo(-35, -1, -35, 799, 0, 300, 3);
				}
				break;

			case kHSUfoExitRight:
				if (_vm->_toyUfoActionStatus < 0) {
					_vm->_isLeavingScene = true;
					_vm->_toyUfoActionStatus = 6;
					_vm->_newSceneNum = 46;
					_vm->toyUfoFlyTo(835, -1, 0, 835, 0, 300, 3);
				}
				break;

			case kHSUfoDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				break;
			}
		} else if (_vm->_sceneClickedHotspot <= 9) {
			switch (_vm->_sceneClickedHotspot) {
			case kHSDevice:
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				break;
			
			case kHSPlatypus:
				if (_vm->_gnapActionStatus < 0) {
					if (_vm->_grabCursorSpriteIndex >= 0) {
						_vm->playGnapImpossible(_vm->_platX, _vm->_platY);
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
							_vm->playGnapImpossible(_vm->_platX, _vm->_platY);
							break;
						}
					}
				}
				break;

			case kHSExitUfoParty:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitUfoParty].x, _vm->_hotspotsWalkPos[kHSExitUfoParty].y, 0, 0x107AE, 1);
				_vm->_gnapActionStatus = 0;
				_vm->_newSceneNum = 40;
				break;
			
			case kHSExitUfo:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitUfo].x, _vm->_gnapY, 0, 0x107AF, 1);
				_vm->_gnapActionStatus = 0;
				_vm->platypusWalkTo(_vm->_hotspotsWalkPos[kHSExitUfo].x, _vm->_platY, -1, 0x107CF, 1);
				_vm->_newSceneNum = 43;
				break;
			
			case kHSExitShow:
				_vm->_isLeavingScene = true;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSExitShow].x, _vm->_hotspotsWalkPos[kHSExitShow].y, 0, 0x107AB, 1);
				_vm->_gnapActionStatus = 0;
				_vm->_newSceneNum = 46;
				break;
			
			case kHSKissingLady:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->_gnapActionStatus = 2;
					_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSKissingLady].x, _vm->_hotspotsWalkPos[kHSKissingLady].y, 0, -1, 9);
					_vm->playGnapShowItem(_vm->_grabCursorSpriteIndex, _vm->_hotspotsWalkPos[kHSKissingLady].x - 1, _vm->_hotspotsWalkPos[kHSKissingLady].y);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(4, 3);
						break;
					case GRAB_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpLeft;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSKissingLady].x, _vm->_hotspotsWalkPos[kHSKissingLady].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = 1;
						break;
					case PLAT_CURSOR:
						_vm->gnapUseDeviceOnPlatypuss();
						_vm->platypusWalkTo(6, 7, 1, 0x107D2, 1);
						if (_vm->_gnapX == 7 && _vm->_gnapY == 7)
							_vm->gnapWalkStep();
						_vm->playGnapIdle(5, 7);
						_vm->_platypusActionStatus = 4;
						break;
					}
				}
				break;
			
			case kHSSpring:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSSpring].x, _vm->_hotspotsWalkPos[kHSSpring].y, 8, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(8, 7);
						break;
					case GRAB_CURSOR:
						_vm->playGnapPullOutDevice(8, 0);
						_vm->playGnapUseDevice(8, 0);
						_nextSpringGuySequenceId = 0xFB;
						_vm->invAdd(kItemSpring);
						_vm->setFlag(kGFSpringTaken);
						updateHotspots();
						break;
					case TALK_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;
			
			case kHSSpringGuy:
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[kHSSpringGuy].x, _vm->_hotspotsWalkPos[kHSSpringGuy].y, 8, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						if (_vm->isFlag(kGFSpringTaken))
							_vm->playGnapMoan1(8, 7);
						else
							_vm->playGnapScratchingHead(8, 7);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[kHSSpringGuy].x, _vm->_hotspotsWalkPos[kHSSpringGuy].y, -1, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
				break;
			
			case kHSWalkArea1:
			case kHSWalkArea2:
				if (_vm->_gnapActionStatus < 0)
					_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				break;
			
			}
		}
	
		if (_vm->_mouseClickState._left && _vm->_gnapActionStatus < 0) {
			_vm->_mouseClickState._left = false;
			if (_vm->isFlag(kGFGnapControlsToyUFO)) {
				_vm->_toyUfoActionStatus = 7;
				_vm->toyUfoFlyTo(-1, -1, 0, 799, 0, 300, 3);
			} else {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			}
		}
	
		updateAnimations();
		_vm->toyUfoCheckTimer();
	
		if (!_vm->_isLeavingScene) {
			if (_vm->_platypusActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO) && _currKissingLadySequenceId != 0xF5)
				_vm->updatePlatypusIdleSequence();
			if (_vm->_gnapActionStatus < 0 && !_vm->isFlag(kGFGnapControlsToyUFO))
				_vm->updateGnapIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextKissingLadySequenceId == -1) {
					switch (_vm->getRandom(20)) {
					case 0:
						_nextKissingLadySequenceId = 0xED;
						break;
					case 1:
						_nextKissingLadySequenceId = 0xEE;
						break;
					case 2:
						_nextKissingLadySequenceId = 0xF0;
						break;
					case 3:
						_nextKissingLadySequenceId = 0xF3;
						break;
					case 4:
						_nextKissingLadySequenceId = 0xF4;
						break;
					default:
						_nextKissingLadySequenceId = 0xEC;
						break;
					}
					if (_nextKissingLadySequenceId != 0xEC && _nextKissingLadySequenceId == _currKissingLadySequenceId)
						_nextKissingLadySequenceId = -1;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(20) + 20;
				if (_vm->_gnapActionStatus < 0 && _vm->_platypusActionStatus < 0 && _nextSpringGuySequenceId == -1) {
					if (_vm->getRandom(5) != 0) {
						if (!_vm->isFlag(kGFSpringTaken))
							_nextSpringGuySequenceId = 0xF9;
					} else {
						if (_vm->isFlag(kGFSpringTaken))
							_nextSpringGuySequenceId = 0xF8;
						else
							_nextSpringGuySequenceId = 0xFA;
					}
				}
			}
		}
	
		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(20) + 20;
		}
		
		_vm->gameUpdateTick();	
	}
}

void Scene44::updateAnimations() {
	GameSys gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) 	{
		gameSys.setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case 0:
			_vm->_sceneDone = true;
			break;
		case 1:
			_nextKissingLadySequenceId = 0xEF;
			break;
		case 2:
			_nextKissingLadySequenceId = 0xF2;
			break;
		}
		_vm->_gnapActionStatus = -1;
	}
	
	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		switch (_vm->_platypusActionStatus) {
		case 4:
			if (gameSys.getAnimationStatus(2) == 2) {
				gameSys.insertSequence(0xFE, _vm->_platypusId, _vm->_platypusSequenceId | (_vm->_platypusSequenceDatNum << 16), _vm->_platypusId, kSeqSyncWait, 0, 0, 0);
				_vm->_platypusSequenceId = 0xFE;
				_vm->_platypusSequenceDatNum = 0;
				gameSys.setAnimation(0xFE, _vm->_platypusId, 1);
				gameSys.removeSequence(_currKissingLadySequenceId, 1, true);
				_vm->_platypusActionStatus = 5;
			}
			break;
		case 5:
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 50;
			break;
		default:
			_vm->_platypusActionStatus = -1;
			break;
		}
	}
	
	if (gameSys.getAnimationStatus(2) == 2) {
		if (_nextKissingLadySequenceId == 0xF6) {
			gameSys.insertSequence(_nextKissingLadySequenceId, 1, _currKissingLadySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_vm->initPlatypusPos(5, 8, kDirNone);
			_currKissingLadySequenceId = _nextKissingLadySequenceId;
			_nextKissingLadySequenceId = -1;
			gameSys.setAnimation(0, 0, 2);
		} else if (_nextKissingLadySequenceId != -1) {
			gameSys.insertSequence(_nextKissingLadySequenceId, 1, _currKissingLadySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextKissingLadySequenceId, 1, 2);
			_currKissingLadySequenceId = _nextKissingLadySequenceId;
			_nextKissingLadySequenceId = -1;
			_vm->_timers[4] = _vm->getRandom(20) + 20;
		}
	}
	
	if (gameSys.getAnimationStatus(4) == 2) {
		if (_currSpringGuySequenceId == 0xFB) {
			_vm->setGrabCursorSprite(kItemSpring);
			_nextSpringGuySequenceId = 0xF8;
		}
		if (_nextSpringGuySequenceId != -1) {
			gameSys.insertSequence(_nextSpringGuySequenceId, 1, _currSpringGuySequenceId, 1, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextSpringGuySequenceId, 1, 4);
			_currSpringGuySequenceId = _nextSpringGuySequenceId;
			_nextSpringGuySequenceId = -1;
			_vm->_timers[5] = _vm->getRandom(20) + 20;
		}
	}
	
	if (gameSys.getAnimationStatus(3) == 2) {
		switch (_vm->_toyUfoActionStatus) {
		case 6:
			_vm->_sceneDone = true;
			break;
		default:
			_vm->_toyUfoNextSequenceId = _vm->toyUfoGetSequenceId();
			gameSys.insertSequence(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId + 1,
				_vm->_toyUfoSequenceId | 0x10000, _vm->_toyUfoId,
				kSeqSyncWait, 0, _vm->_toyUfoX - 274, _vm->_toyUfoY - 128);
			_vm->_toyUfoSequenceId = _vm->_toyUfoNextSequenceId;
			++_vm->_toyUfoId;
			gameSys.setAnimation(_vm->_toyUfoNextSequenceId | 0x10000, _vm->_toyUfoId, 3);
			break;
		}
		_vm->_toyUfoActionStatus = -1;
	}
}

} // End of namespace Gnap
