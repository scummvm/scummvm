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
#include "gnap/scenes/scene19.h"

namespace Gnap {

static const int kS19ShopAssistantSequenceIds[] = {
	0x6F, 0x70, 0x71, 0x72, 0x73
};

enum {
	kHSPlatypus				= 0,
	kHSExitOutsideToyStore	= 1,
	kHSDevice				= 2,
	kHSPicture				= 3,
	kHSShopAssistant		= 4,
	kHSToy1					= 5,
	kHSToy2					= 6,
	kHSToy3					= 7,
	kHSPhone				= 8,
	kHSToy4					= 9,
	kHSToy5					= 10,
	kHSToy6					= 11,
	kHSToy7					= 12,
	kHSWalkArea1			= 13,
	kHSWalkArea2			= 14,
	kHSWalkArea3			= 15
};

enum {
	kASUsePhone					= 0,
	kASGrabToy					= 1,
	kASGrabPicture				= 2,
	kASGrabPictureDone			= 3,
	kASTalkShopAssistant		= 4,
	kASLeaveScene				= 5
};

Scene19::Scene19(GnapEngine *vm) : Scene(vm) {
	_s19_toyGrabCtr = 0;
	_s19_pictureSurface = 0;
	_s19_shopAssistantCtr = 0;
	_s19_pictureSurface = nullptr;
}

Scene19::~Scene19() {
	delete _s19_pictureSurface;
}

int Scene19::init() {
	_vm->playSound(0x79, false);
	return _vm->isFlag(kGFPlatypusTalkingToAssistant) ? 0x77 : 0x76;
}

void Scene19::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSExitOutsideToyStore, 36, 154, 142, 338, SF_EXIT_NW_CURSOR, 4, 6);
	_vm->setHotspot(kHSPicture, 471, 237, 525, 283, SF_DISABLED, 7, 2);
	_vm->setHotspot(kHSShopAssistant, 411, 151, 575, 279, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
	_vm->setHotspot(kHSPhone, 647, 166, 693, 234, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 0);
	_vm->setHotspot(kHSToy1, 181, 11, 319, 149, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 0);
	_vm->setHotspot(kHSToy2, 284, 85, 611, 216, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 0);
	_vm->setHotspot(kHSToy3, 666, 38, 755, 154, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 0);
	_vm->setHotspot(kHSToy4, 154, 206, 285, 327, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 3);
	_vm->setHotspot(kHSToy5, 494, 301, 570, 448, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 5);
	_vm->setHotspot(kHSToy6, 0, 320, 188, 600, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 6);
	_vm->setHotspot(kHSToy7, 597, 434, 800, 600, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 8);
	_vm->setHotspot(kHSWalkArea1, 0, 0, 170, 600);
	_vm->setHotspot(kHSWalkArea2, 622, 0, 800, 600);
	_vm->setHotspot(kHSWalkArea3, 0, 0, 800, 437);
	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_vm->_hotspots[kHSToy1]._flags = SF_DISABLED;
		_vm->_hotspots[kHSToy2]._flags = SF_DISABLED;
		_vm->_hotspots[kHSToy3]._flags = SF_DISABLED;
		_vm->_hotspots[kHSToy4]._flags = SF_DISABLED;
		_vm->_hotspots[kHSToy5]._flags = SF_DISABLED;
		_vm->_hotspots[kHSToy6]._flags = SF_DISABLED;
		_vm->_hotspots[kHSToy7]._flags = SF_DISABLED;
		_vm->_hotspots[kHSShopAssistant]._flags = SF_DISABLED;
		_vm->_hotspots[kHSPhone]._flags = SF_DISABLED;
		_vm->_hotspots[kHSPlatypus]._flags = SF_DISABLED;
		_vm->_hotspots[kHSPicture]._flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	}
	_vm->_hotspotsCount = 16;
}

void Scene19::run() {
	_vm->queueInsertDeviceIcon();

	_s19_toyGrabCtr = 0;
	_s19_pictureSurface = 0;

	_vm->_gameSys->insertSequence(0x74, 254, 0, 0, kSeqNone, 0, 0, 0);
	_vm->_gameSys->insertSequence(0x75, 254, 0, 0, kSeqNone, 0, 0, 0);

	if (!_vm->isFlag(kGFPictureTaken))
		_vm->_gameSys->insertSequence(0x69, 19, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_vm->initGnapPos(3, 6, kDirBottomRight);
		_s19_currShopAssistantSequenceId = kS19ShopAssistantSequenceIds[_vm->getRandom(5)];
		_s19_nextShopAssistantSequenceId = _s19_currShopAssistantSequenceId;
		_vm->_gameSys->setAnimation(_s19_currShopAssistantSequenceId, 20, 4);
		_vm->_gameSys->insertSequence(0x6E, 254, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_gameSys->insertSequence(_s19_currShopAssistantSequenceId, 20, 0, 0, kSeqNone, 0, 0, 0);
		_s19_shopAssistantCtr = 0;
		_vm->endSceneInit();
		_vm->gnapWalkTo(4, 9, -1, 0x107B9, 1);
		updateHotspots();
	} else {
		_s19_currShopAssistantSequenceId = 0x6D;
		_s19_nextShopAssistantSequenceId = -1;
		_vm->_gameSys->setAnimation(0x6D, 20, 4);
		_vm->_gameSys->insertSequence(_s19_currShopAssistantSequenceId, 20, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_timers[6] = _vm->getRandom(40) + 50;
		_vm->initGnapPos(3, 6, kDirBottomRight);
		_vm->initPlatypusPos(4, 6, kDirNone);
		_vm->endSceneInit();
		_vm->gnapWalkTo(4, 9, -1, 0x107B9, 1);
		_vm->platypusWalkTo(5, 9, -1, 0x107C2, 1);
	}
	
	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 5, -1, -1, -1, -1);

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
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSExitOutsideToyStore:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 18;
				_vm->_hotspots[kHSWalkArea1]._flags |= SF_WALKABLE;
				_vm->gnapWalkTo(_vm->_hotspotsWalkPos[1].x, _vm->_hotspotsWalkPos[1].y, 0, 0x107B2, 1);
				_vm->_gnapActionStatus = kASLeaveScene;
				if (_vm->isFlag(kGFPlatypusTalkingToAssistant))
					_vm->setFlag(kGFUnk27);
				else
					_vm->platypusWalkTo(_vm->_hotspotsWalkPos[1].x + 1, _vm->_hotspotsWalkPos[1].y, -1, 0x107C5, 1);
				_vm->_hotspots[kHSWalkArea1]._flags &= ~SF_WALKABLE;
			}
			break;

		case kHSPicture:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 6, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(6, 2);
						break;
					case GRAB_CURSOR:
						if (!_vm->isFlag(kGFPictureTaken)) {
							_vm->gnapWalkTo(_vm->_gnapX, _vm->_gnapY, 0, _vm->getGnapSequenceId(gskIdle, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y) | 0x10000, 1);
							_vm->_gnapActionStatus = kASGrabPicture;
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSShopAssistant:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 6, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(6, 2);
						break;
					case TALK_CURSOR:
						_vm->_gnapIdleFacing = kDirUpRight;
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 0, _vm->getGnapSequenceId(gskBrainPulsating, 0, 0) | 0x10000, 1);
						_vm->_gnapActionStatus = kASTalkShopAssistant;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSToy1:
		case kHSToy2:
		case kHSToy3:
		case kHSToy4:
		case kHSToy5:
		case kHSToy6:
		case kHSToy7:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapImpossible(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapMoan2(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 0, -1, 1);
						_vm->playGnapIdle(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y);
						_vm->_gnapActionStatus = kASGrabToy;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSPhone:
			if (_vm->_gnapActionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					_vm->playGnapShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 9, 1);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						_vm->playGnapScratchingHead(9, 1);
						break;
					case GRAB_CURSOR:
						_vm->gnapWalkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].x, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot].y, 0, -1, 1);
						_vm->playGnapIdle(8, 2);
						_vm->_gnapActionStatus = kASUsePhone;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						_vm->playGnapImpossible(0, 0);
						break;
					}
				}
			}
			break;

		case kHSWalkArea1:
		case kHSWalkArea2:
		case kHSWalkArea3:
			if (_vm->_gnapActionStatus < 0)
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left) {
				_vm->gnapWalkTo(-1, -1, -1, -1, 1);
				_vm->_mouseClickState._left = 0;
			}
		}

		updateAnimations();
	
		if (!_vm->_isLeavingScene) {
			_vm->updateGnapIdleSequence();
			if (!_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
				_vm->updatePlatypusIdleSequence();
				if (!_vm->_timers[6] && _s19_nextShopAssistantSequenceId == -1) {
					_vm->_timers[6] = _vm->getRandom(40) + 50;
					if (_vm->getRandom(4) != 0) {
						_s19_nextShopAssistantSequenceId = 0x64;
					} else if (_vm->isFlag(kGFPictureTaken)) {
						_s19_nextShopAssistantSequenceId = 0x64;
					} else {
						_s19_nextShopAssistantSequenceId = 0x6C;
					}
				}
			}
		}

		_vm->checkGameKeys();
	
		if (_vm->isKeyStatus1(8)) {
			_vm->clearKeyStatus1(8);
			_vm->runMenu();
			updateHotspots();
		}
		
		_vm->gameUpdateTick();
	}

	if (_s19_pictureSurface)
		_vm->deleteSurface(&_s19_pictureSurface);
}

void Scene19::updateAnimations() {
	if (_vm->_gameSys->getAnimationStatus(0) == 2) {
		_vm->_gameSys->setAnimation(0, 0, 0);
		switch (_vm->_gnapActionStatus) {
		case kASUsePhone:
			_s19_nextShopAssistantSequenceId = 0x67;
			break;
		case kASGrabToy:
			++_s19_toyGrabCtr;
			switch (_s19_toyGrabCtr) {
			case 1:
				_s19_nextShopAssistantSequenceId = 0x62;
				break;
			case 2:
				_s19_nextShopAssistantSequenceId = 0x6B;
				break;
			case 3:
				_s19_nextShopAssistantSequenceId = 0x66;
				break;
			default:
				_s19_nextShopAssistantSequenceId = 0x65;
				break;
			}
			break;
		case kASGrabPicture:
			_vm->playGnapPullOutDevice(6, 2);
			_vm->playGnapUseDevice(0, 0);
			_vm->_gameSys->setAnimation(0x68, 19, 0);
			_vm->_gameSys->insertSequence(0x68, 19, 105, 19, kSeqSyncWait, 0, 0, 0);
			_vm->invAdd(kItemPicture);
			_vm->setFlag(kGFPictureTaken);
			updateHotspots();
			_vm->_gnapActionStatus = kASGrabPictureDone;
			break;
		case kASGrabPictureDone:
			_vm->setGrabCursorSprite(-1);
			_vm->hideCursor();
			_s19_pictureSurface = _vm->addFullScreenSprite(0xF, 255);
			_vm->_gameSys->setAnimation(0x61, 256, 0);
			_vm->_gameSys->insertSequence(0x61, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (_vm->_gameSys->getAnimationStatus(0) != 2) {
				// checkGameAppStatus();
				_vm->gameUpdateTick();
			}
			_vm->setFlag(kGFUnk27);
			_vm->showCursor();
			_vm->_newSceneNum = 17;
			_vm->_isLeavingScene = true;
			_vm->_sceneDone = true;
			_s19_nextShopAssistantSequenceId = -1;
			break;
		case kASTalkShopAssistant:
			_s19_nextShopAssistantSequenceId = 0x6D;
			_vm->_gnapActionStatus = -1;
			break;
		case kASLeaveScene:
			_vm->_sceneDone = true;
			break;
		}
	}

	if (_vm->_gameSys->getAnimationStatus(4) == 2) {
		switch (_s19_nextShopAssistantSequenceId) {
		case 0x6F:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
			_s19_shopAssistantCtr = (_s19_shopAssistantCtr + 1) % 5;
			_s19_nextShopAssistantSequenceId = kS19ShopAssistantSequenceIds[_s19_shopAssistantCtr];
			_vm->_gameSys->setAnimation(_s19_nextShopAssistantSequenceId, 20, 4);
			_vm->_gameSys->insertSequence(_s19_nextShopAssistantSequenceId, 20, _s19_currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x6E, 254, 0x6E, 254, kSeqSyncWait, 0, 0, 0);
			_s19_currShopAssistantSequenceId = _s19_nextShopAssistantSequenceId;
			break;
		case 0x62:
		case 0x66:
		case 0x6B:
			_vm->_gameSys->setAnimation(_s19_nextShopAssistantSequenceId, 20, 4);
			_vm->_gameSys->insertSequence(_s19_nextShopAssistantSequenceId, 20, _s19_currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s19_currShopAssistantSequenceId = _s19_nextShopAssistantSequenceId;
			_s19_nextShopAssistantSequenceId = -1;
			_vm->_timers[5] = 10;
			while (_vm->_timers[5]) {
				_vm->gameUpdateTick();
			}
			_vm->playGnapIdle(6, 2);
			_vm->_gnapActionStatus = -1;
			break;
		case 0x67:
			_vm->_gameSys->setAnimation(_s19_nextShopAssistantSequenceId, 20, 4);
			_vm->_gameSys->insertSequence(_s19_nextShopAssistantSequenceId, 20, _s19_currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s19_currShopAssistantSequenceId = _s19_nextShopAssistantSequenceId;
			_s19_nextShopAssistantSequenceId = -1;
			_vm->_gnapActionStatus = -1;
			break;
		case 0x65:
			_vm->playGnapIdle(6, 2);
			_vm->_gameSys->setAnimation(_s19_nextShopAssistantSequenceId, 20, 0);
			_vm->_gameSys->insertSequence(_s19_nextShopAssistantSequenceId, 20, _s19_currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s19_currShopAssistantSequenceId = _s19_nextShopAssistantSequenceId;
			_s19_nextShopAssistantSequenceId = -1;
			_vm->_newSceneNum = 18;
			_vm->_gnapActionStatus = kASLeaveScene;
			break;
		case 0x6D:
			_vm->_gameSys->setAnimation(_s19_nextShopAssistantSequenceId, 20, 4);
			_vm->_gameSys->insertSequence(_s19_nextShopAssistantSequenceId, 20, _s19_currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_vm->_gameSys->insertSequence(0x69, 19, 0x69, 19, kSeqSyncWait, _vm->getSequenceTotalDuration(_s19_nextShopAssistantSequenceId), 0, 0);
			_s19_currShopAssistantSequenceId = _s19_nextShopAssistantSequenceId;
			_s19_nextShopAssistantSequenceId = -1;
			break;
		case 0x64:
		case 0x6C:
			_vm->_gameSys->setAnimation(_s19_nextShopAssistantSequenceId, 20, 4);
			_vm->_gameSys->insertSequence(_s19_nextShopAssistantSequenceId, 20, _s19_currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_s19_currShopAssistantSequenceId = _s19_nextShopAssistantSequenceId;
			_s19_nextShopAssistantSequenceId = -1;
			break;
		}
	}
}

} // End of namespace Gnap
