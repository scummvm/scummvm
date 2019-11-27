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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"
#include "gnap/scenes/group1.h"

namespace Gnap {

Scene10::Scene10(GnapEngine *vm) : Scene(vm) {
	_nextCookSequenceId = -1;
	_currCookSequenceId = -1;
}

int Scene10::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	gameSys.setAnimation(0, 0, 2);
	return 0x10F;
}

void Scene10::updateHotspots() {
	_vm->setHotspot(kHS10Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS10ExitBar, 0, 75, 85, 455, SF_EXIT_NW_CURSOR);
	_vm->setHotspot(kHS10ExitBackdoor, 75, 590, 500, 599, SF_EXIT_D_CURSOR | SF_WALKABLE);
	_vm->setHotspot(kHS10Cook, 370, 205, 495, 460, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS10Tongs, 250, 290, 350, 337, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS10Box, 510, 275, 565, 330, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS10Oven, 690, 280, 799, 420, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS10WalkArea1, 59, 0, 495, 460);
	_vm->setHotspot(kHS10WalkArea2, 495, 0, 650, 420);
	_vm->setHotspot(kHS10WalkArea3, 651, 0, 725, 400);
	_vm->setHotspot(kHS10WalkArea4, 725, 0, 799, 441);
	_vm->setDeviceHotspot(kHS10Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 12;
}

void Scene10::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_currCookSequenceId = 0x103;

	gameSys.setAnimation(0x103, 100, 2);
	gameSys.insertSequence(0x103, 100, 0, 0, kSeqNone, 0, 0, 0);

	_nextCookSequenceId = 0x106;
	if (!_vm->isFlag(kGFMudTaken))
		gameSys.insertSequence(0x107, 100, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	if (_vm->_prevSceneNum == 9) {
		gnap.initPos(11, 8, kDirBottomLeft);
		plat.initPos(12, 7, kDirIdleRight);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(9, 8), -1, 0x107BA, 1);
		plat.walkTo(Common::Point(9, 7), -1, 0x107D2, 1);
	} else {
		gnap.initPos(-1, 7, kDirBottomRight);
		plat.initPos(-2, 8, kDirIdleLeft);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(1, 7), -1, 0x107B9, 1);
		plat.walkTo(Common::Point(1, 8), -1, 0x107C2, 1);
	}

	_vm->_timers[4] = _vm->getRandom(80) + 150;
	_vm->_timers[5] = _vm->getRandom(100) + 100;

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1091E))
			_vm->playSound(0x1091E, true);

		if (!_vm->isSoundPlaying(0x1091A))
			_vm->playSound(0x1091A, true);

		_vm->updateMouseCursor();

		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS10Platypus:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playImpossible();
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					if (_vm->isFlag(kGFMudTaken))
						gnap.playMoan1(plat._pos);
					else
						gnap.playScratchingHead(plat._pos);
					break;
				case GRAB_CURSOR:
					gnap.kissPlatypus(10);
					break;
				case TALK_CURSOR:
					gnap.playBrainPulsating(plat._pos);
					plat.playSequence(plat.getSequenceId());
					break;
				case PLAT_CURSOR:
					break;
				default:
					break;
				}
			}
			break;

		case kHS10ExitBar:
			_vm->_isLeavingScene = true;
			gnap.actionIdle(0x10C);
			gnap.walkTo(Common::Point(0, 7), 0, 0x107AF, 1);
			gnap._actionStatus = kAS10LeaveScene;
			plat.walkTo(Common::Point(0, 7), -1, 0x107CF, 1);
			_vm->_newSceneNum = 11;
			break;

		case kHS10ExitBackdoor:
			_vm->_isLeavingScene = true;
			gnap.actionIdle(0x10C);
			gnap.walkTo(Common::Point(2, 9), 0, 0x107AE, 1);
			gnap._actionStatus = kAS10LeaveScene;
			plat.walkTo(Common::Point(3, 9), -1, 0x107C7, 1);
			_vm->_newSceneNum = 9;
			break;

		case kHS10Cook:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(4, 8), 6, 0);
				gameSys.setAnimation(makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, 0);
				gnap._actionStatus = kAS10AnnoyCook;
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playScratchingHead(Common::Point(6, 0));
					break;
				case GRAB_CURSOR:
					gnap.playImpossible();
					gnap._idleFacing = kDirBottomRight;
					break;
				case TALK_CURSOR:
					gnap._idleFacing = kDirUpRight;
					gnap.actionIdle(0x10C);
					gnap.walkTo(Common::Point(4, 8), 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
					gnap._actionStatus = kAS10AnnoyCook;
					break;
				case PLAT_CURSOR:
					gnap.actionIdle(0x10C);
					gnap.useDeviceOnPlatypus();
					plat.walkTo(Common::Point(4, 6), -1, -1, 1);
					gnap.walkTo(Common::Point(4, 8), 0, 0x107BB, 1);
					gnap._actionStatus = kAS10AnnoyCook;
					break;
				default:
					break;
				}
			}
			break;

		case kHS10Tongs:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(3, 7), 4, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					if (_vm->isFlag(kGFMudTaken))
						gnap.playMoan2(Common::Point(-1, -1));
					else
						gnap.playScratchingHead(Common::Point(4, 3));
					break;
				case GRAB_CURSOR:
					if (_vm->isFlag(kGFMudTaken)) {
						gnap.playMoan2(Common::Point(-1, -1));
					} else {
						gnap.actionIdle(0x10C);
						gnap.walkTo(Common::Point(4, 8), 0, 0x107BB, 1);
						gnap._actionStatus = kAS10AnnoyCook;
					}
					break;
				case TALK_CURSOR:
					gnap.playImpossible();
					break;
				case PLAT_CURSOR:
					if (_vm->isFlag(kGFMudTaken)) {
						gnap.playMoan2(Common::Point(-1, -1));
					} else {
						gnap.actionIdle(0x10C);
						gnap.useDeviceOnPlatypus();
						plat.walkTo(Common::Point(3, 7), -1, -1, 1);
						gnap.walkTo(Common::Point(4, 8), 0, 0x107BB, 1);
						gnap._actionStatus = kAS10AnnoyCook;
					}
					break;
				default:
					break;
				}
			}
			break;

		case kHS10Box:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(7, 6), 6, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playScratchingHead(Common::Point(7, 3));
					break;
				case GRAB_CURSOR:
					gnap.actionIdle(0x10C);
					gnap.walkTo(Common::Point(4, 8), 0, 0x107BB, 1);
					gnap._actionStatus = kAS10AnnoyCook;
					break;
				case TALK_CURSOR:
					gnap.playImpossible();
					break;
				case PLAT_CURSOR:
					if (_vm->isFlag(kGFMudTaken)) {
						gnap.playMoan2(Common::Point(-1, -1));
					} else {
						_vm->invAdd(kItemTongs);
						_vm->setFlag(kGFMudTaken);
						gnap.actionIdle(0x10C);
						gnap.useDeviceOnPlatypus();
						plat.walkTo(Common::Point(7, 6), 1, 0x107D2, 1);
						plat._actionStatus = kAS10PlatWithBox;
						plat._idleFacing = kDirIdleRight;
						_vm->_largeSprite = gameSys.createSurface(0xC3);
						gnap.playIdle(Common::Point(7, 6));
					}
					break;
				default:
					break;
				}
			}
			break;

		case kHS10Oven:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(9, 6), 10, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playSequence(gnap.getSequenceId(kGSDeflect, Common::Point(10, 5)) | 0x10000);
					break;
				case GRAB_CURSOR:
					gnap.actionIdle(0x10C);
					gnap.walkTo(Common::Point(9, 6), 0, 0x107BB, 1);
					gameSys.insertSequence(0x10E, 120, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
					gnap._sequenceId = 0x10E;
					gnap._id = 120;
					gnap._idleFacing = kDirUpRight;
					gnap._sequenceDatNum = 0;
					gnap._pos = Common::Point(9, 6);
					_vm->_timers[2] = 360;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					gnap.playImpossible();
					break;
				default:
					break;
				}
			}
			break;

		case kHS10WalkArea1:
		case kHS10WalkArea2:
		case kHS10WalkArea3:
		case kHS10WalkArea4:
			gnap.actionIdle(0x10C);
			gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		case kHS10Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		default:
			if (_vm->_mouseClickState._left) {
				gnap.actionIdle(0x10C);
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->_isLeavingScene) {
			plat.updateIdleSequence();
			gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(80) + 150;
				_vm->playSound(0x12B, false);
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(100) + 100;
				int _gnapRandomValue = _vm->getRandom(4);
				if (_gnapRandomValue) {
					int sequenceId;
					if (_gnapRandomValue == 1) {
						sequenceId = 0x8A5;
					} else if (_gnapRandomValue == 2) {
						sequenceId = 0x8A6;
					} else {
						sequenceId = 0x8A7;
					}
					gameSys.insertSequence(sequenceId | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}
}

void Scene10::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS10LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS10AnnoyCook:
			_nextCookSequenceId = 0x105;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		switch (plat._actionStatus) {
		case kAS10PlatWithBox:
			_nextCookSequenceId = 0x109;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2 && _nextCookSequenceId != -1) {

		switch (_nextCookSequenceId) {
		case 0x109:
			plat._pos = Common::Point(4, 8);
			gameSys.insertSequence(0x109, 100, _currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x107C9, 160,
				plat._sequenceId | (plat._sequenceDatNum << 16), plat._id,
				kSeqSyncWait, _vm->getSequenceTotalDuration(0x109) + _vm->getSequenceTotalDuration(0x10A) + _vm->getSequenceTotalDuration(0x10843),
				75 * plat._pos.x - plat._gridX, 48 * plat._pos.y - plat._gridY);
			gameSys.removeSequence(0x107, 100, true);
			_currCookSequenceId = 0x109;
			_nextCookSequenceId = 0x843;
			plat._sequenceId = 0x7C9;
			plat._id = 160;
			plat._idleFacing = kDirIdleLeft;
			plat._sequenceDatNum = 1;
			break;
		case 0x843:
			_vm->hideCursor();
			gameSys.insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
			gameSys.insertSequence(0x10843, 301, _currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_currCookSequenceId = 0x843;
			_nextCookSequenceId = 0x10A;
			break;
		case 0x10A:
			gameSys.insertSequence(_nextCookSequenceId, 100, 0x10843, 301, kSeqSyncWait, 0, 0, 0);
			_currCookSequenceId = _nextCookSequenceId;
			_nextCookSequenceId = 0x104;
			_vm->showCursor();
			gameSys.removeSpriteDrawItem(_vm->_largeSprite, 300);
			_vm->delayTicksCursor(5);
			_vm->deleteSurface(&_vm->_largeSprite);
			_vm->setGrabCursorSprite(kItemTongs);
			if (plat._actionStatus == kAS10PlatWithBox)
				plat._actionStatus = -1;
			if (gnap._pos == Common::Point(4, 8))
				gnap.walkStep();
			break;
		default:
			gameSys.insertSequence(_nextCookSequenceId, 100, _currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
			_currCookSequenceId = _nextCookSequenceId;
			break;
		}

		switch (_currCookSequenceId) {
		case 0x106:
			if (gnap._actionStatus >= 0 || plat._actionStatus >= 0) {
				_nextCookSequenceId = 0x106;
			} else {
				int rnd = _vm->getRandom(7);
				switch (rnd) {
				case 0:
					_nextCookSequenceId = 0x104;
					break;
				case 1:
					_nextCookSequenceId = 0x103;
					break;
				case 2:
					_nextCookSequenceId = 0x106;
					gameSys.insertSequence(0x10D, 1, 0, 0, kSeqNone, 0, 0, 0);
					break;
				default:
					_nextCookSequenceId = 0x106;
					break;
				}
			}
			break;
		case 0x103:
			if (gnap._actionStatus >= 0 || plat._actionStatus >= 0)
				_nextCookSequenceId = 0x106;
			else if (_vm->getRandom(7) == 0)
				_nextCookSequenceId = 0x104;
			else
				_nextCookSequenceId = 0x106;
			break;
		case 0x104:
			if (gnap._actionStatus >= 0 || plat._actionStatus >= 0)
				_nextCookSequenceId = 0x106;
			else if (_vm->getRandom(7) == 0)
				_nextCookSequenceId = 0x103;
			else
				_nextCookSequenceId = 0x106;
			break;
		case 0x105:
			if (gnap._actionStatus >= 0 || plat._actionStatus >= 0)
				_nextCookSequenceId = 0x106;
			else {
				int rnd = _vm->getRandom(7);
				switch (rnd) {
				case 0:
					_nextCookSequenceId = 0x104;
					break;
				case 1:
					_nextCookSequenceId = 0x103;
					break;
				default:
					_nextCookSequenceId = 0x106;
					break;
				}
			}
			_vm->_timers[2] = _vm->getRandom(30) + 20;
			_vm->_timers[3] = 300;
			gameSys.insertSequence(0x10C, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0x10C;
			gnap._idleFacing = kDirUpRight;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = -1;
			plat._actionStatus = -1;
			break;
		default:
			break;
		}
		if (_currCookSequenceId == 0x843) {
			gameSys.setAnimation(_currCookSequenceId | 0x10000, 301, 2);
		} else {
			gameSys.setAnimation(_currCookSequenceId, 100, 2);
		}
	}
}

void Scene10::updateAnimationsCb() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(2) == 2) {
		gameSys.setAnimation(_nextCookSequenceId, 100, 2);
		gameSys.insertSequence(_nextCookSequenceId, 100, _currCookSequenceId, 100, kSeqSyncWait, 0, 0, 0);
		_currCookSequenceId = _nextCookSequenceId;
		_nextCookSequenceId = 0x106;
	}
}

/*****************************************************************************/

Scene11::Scene11(GnapEngine *vm) : Scene(vm) {
	_billardBallCtr = 0;
	_nextHookGuySequenceId = -1;
	_currHookGuySequenceId = -1;
	_nextGoggleGuySequenceId = -1;
	_currGoggleGuySequenceId = -1;
}

int Scene11::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 3);
	gameSys.setAnimation(0, 0, 2);
	if (_vm->_prevSceneNum == 10 || _vm->_prevSceneNum == 13) {
		_vm->playSound(0x108EC, false);
		_vm->playSound(0x10928, false);
	}
	return 0x209;
}

void Scene11::updateHotspots() {
	_vm->setHotspot(kHS11Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS11ExitKitchen, 420, 140, 520, 345, SF_EXIT_U_CURSOR);
	_vm->setHotspot(kHS11ExitToilet, 666, 130, 740, 364, SF_EXIT_R_CURSOR);
	_vm->setHotspot(kHS11ExitLeft, 0, 350, 10, 599, SF_EXIT_L_CURSOR | SF_WALKABLE);
	_vm->setHotspot(kHS11GoggleGuy, 90, 185, 185, 340, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS11HookGuy, 210, 240, 340, 430, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS11Billard, 640, 475, 700, 530, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS11WalkArea1, 0, 0, 365, 453);
	_vm->setHotspot(kHS11WalkArea2, 0, 0, 629, 353);
	_vm->setHotspot(kHS11WalkArea3, 629, 0, 799, 364);
	_vm->setHotspot(kHS11WalkArea4, 735, 0, 799, 397);
	_vm->setHotspot(kHS11WalkArea5, 510, 540, 799, 599);
	_vm->setDeviceHotspot(kHS11Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 13;
}

void Scene11::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	bool flag = true;

	_vm->_timers[7] = 50;
	_vm->_hotspots[kHS11Billard]._flags |= SF_DISABLED;

	_currGoggleGuySequenceId = 0x1F9;
	_currHookGuySequenceId = 0x201;

	switch (_vm->_prevSceneNum) {
	case 13:
		gnap.initPos(8, 5, kDirBottomLeft);
		plat.initPos(9, 6, kDirIdleRight);
		break;
	case 47:
		gnap.initPos(8, 5, kDirBottomLeft);
		plat.initPos(9, 5, kDirIdleRight);
		_currGoggleGuySequenceId = 0x1FA;
		_currHookGuySequenceId = 0x1FF;
		_vm->_timers[7] = 180;
		break;
	case 12:
		gnap.initPos(-1, 9, kDirBottomRight);
		plat.initPos(-2, 8, kDirIdleLeft);
		break;
	default:
		gnap.initPos(6, 6, kDirBottomLeft);
		plat.initPos(6, 5, kDirIdleRight);
		break;
	}

	_vm->queueInsertDeviceIcon();

	gameSys.insertSequence(_currHookGuySequenceId, 120, 0, 0, kSeqNone, 0, 0, 0);

	_nextHookGuySequenceId = -1;

	gameSys.setAnimation(_currHookGuySequenceId, 120, 3);
	gameSys.insertSequence(_currGoggleGuySequenceId, 121, 0, 0, kSeqNone, 0, 0, 0);

	_nextGoggleGuySequenceId = -1;

	gameSys.setAnimation(_currGoggleGuySequenceId, 121, 2);

	_vm->_timers[5] = _vm->getRandom(100) + 75;
	_vm->_timers[4] = _vm->getRandom(40) + 20;
	_vm->_timers[6] = _vm->getRandom(100) + 100;
	_vm->endSceneInit();

	if (_vm->_prevSceneNum == 12) {
		gnap.walkTo(Common::Point(2, 8), -1, 0x107B9, 1);
		plat.walkTo(Common::Point(1, 8), -1, 0x107C2, 1);
	}

	gameSys.insertSequence(0x208, 256, 0, 0, kSeqNone, 40, 0, 0);

	while (!_vm->_sceneDone) {
		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS11Platypus:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playImpossible();
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playMoan1(plat._pos);
					break;
				case GRAB_CURSOR:
					gnap.kissPlatypus(0);
					break;
				case TALK_CURSOR:
					gnap.playBrainPulsating(plat._pos);
					plat.playSequence(plat.getSequenceId());
					break;
				case PLAT_CURSOR:
					break;
				default:
					break;
				}
			}
			break;

		case kHS11ExitKitchen:
			_vm->_isLeavingScene = true;
			gnap.walkTo(Common::Point(6, 5), 0, 0x107BF, 1);
			gnap._actionStatus = kAS11LeaveScene;
			plat.walkTo(Common::Point(6, 6), -1, -1, 1);
			_vm->_newSceneNum = 10;
			break;

		case kHS11ExitToilet:
			_vm->_isLeavingScene = true;
			gnap.walkTo(Common::Point(8, 5), 0, 0x107BF, 1);
			gnap._actionStatus = kAS11LeaveScene;
			plat.walkTo(Common::Point(8, 6), -1, -1, 1);
			_vm->_newSceneNum = 13;
			break;

		case kHS11ExitLeft:
			_vm->_isLeavingScene = true;
			gnap.walkTo(Common::Point(-1, 8), 0, 0x107AF, 1);
			gnap._actionStatus = kAS11LeaveScene;
			plat.walkTo(Common::Point(-1, 9), -1, 0x107CF, 1);
			_vm->_newSceneNum = 12;
			break;

		case kHS11GoggleGuy:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemMagazine) {
					gnap.walkTo(Common::Point(3, 7), 0, 0x107BC, 1);
					gnap._actionStatus = kAS11ShowMagazineToGoggleGuy;
					gnap.playShowItem(_vm->_grabCursorSpriteIndex, 2, 0);
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(Common::Point(3, 7), 2, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(1, 6));
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirUpLeft;
						gnap.walkTo(Common::Point(3, 7), 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS11TalkGoggleGuy;
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS11HookGuy:
			if (gnap._actionStatus < 0) {
				gnap._idleFacing = kDirUpRight;
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.walkTo(Common::Point(5, 6), 0, 0x107BC, 9);
					gnap._actionStatus = kAS11ShowItemToHookGuy;
					gnap.playShowItem(_vm->_grabCursorSpriteIndex, 4, 0);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playSequence(gnap.getSequenceId(kGSDeflect, Common::Point(3, 6)) | 0x10000);
						break;
					case GRAB_CURSOR:
						gnap.walkTo(Common::Point(5, 6), 0, 0x107BC, 1);
						gnap._actionStatus = kAS11GrabHookGuy;
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirBottomLeft;
						gnap.walkTo(Common::Point(5, 6), 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS11TalkHookGuy;
						break;
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS11Billard:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible(Common::Point(9, 8));
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(9, 8));
						break;
					case GRAB_CURSOR:
						gnap.walkTo(Common::Point(9, 8), 0, 0x107BA, 1);
						gnap._actionStatus = kAS11GrabBillardBall;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible(Common::Point(9, 8));
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS11WalkArea1:
		case kHS11WalkArea2:
		case kHS11WalkArea3:
		case kHS11WalkArea4:
		case kHS11WalkArea5:
			if (gnap._actionStatus < 0)
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		case kHS11Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		default:
			if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->_isLeavingScene) {
			if (flag && !_vm->_timers[7]) {
				flag = false;
				gameSys.setAnimation(0x207, 257, 4);
				gameSys.insertSequence(0x207, 257, 0, 0, kSeqNone, 0, 0, 0);
			}
			plat.updateIdleSequence2();
			gnap.updateIdleSequence2();
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(100) + 75;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _nextGoggleGuySequenceId == -1) {
					if (_vm->getRandom(2))
						_nextGoggleGuySequenceId = 0x1F6;
					else
						_nextGoggleGuySequenceId = 0x1F9;
				}
			}
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(40) + 20;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0 && _nextHookGuySequenceId == -1) {
					if (_currHookGuySequenceId == 0x201) {
						switch (_vm->getRandom(7)) {
						case 0:
							_nextHookGuySequenceId = 0x200;
							break;
						case 1:
							_nextHookGuySequenceId = 0x205;
							break;
						case 2:
							_nextHookGuySequenceId = 0x202;
							break;
						default:
							_nextHookGuySequenceId = 0x201;
							break;
						}
					} else {
						_nextHookGuySequenceId = 0x201;
					}
				}
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(100) + 100;
				int _gnapRandomValue = _vm->getRandom(3);
				switch (_gnapRandomValue) {
				case 0:
					gameSys.insertSequence(0x8A5 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					gameSys.insertSequence(0x8A7 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
					gameSys.insertSequence(0x8A6 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				default:
					break;
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(50) + 75;
			_vm->_timers[4] = _vm->getRandom(40) + 20;
		}

		_vm->gameUpdateTick();
	}
}

void Scene11::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		if (gnap._actionStatus != kAS11GrabBillardBall)
			gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS11LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS11ShowMagazineToGoggleGuy:
			_nextGoggleGuySequenceId = 0x1F7;
			break;
		case kAS11TalkGoggleGuy:
			_nextGoggleGuySequenceId = 0x1FB;
			break;
		case kAS11GrabHookGuy:
			_nextHookGuySequenceId = 0x204;
			break;
		case kAS11ShowItemToHookGuy:
			_nextHookGuySequenceId = 0x203;
			break;
		case kAS11TalkHookGuy:
			_nextHookGuySequenceId = 0x206;
			break;
		case kAS11GrabBillardBall:
			if (gameSys.getAnimationStatus(2) == 2 && gameSys.getAnimationStatus(3) == 2) {
				gameSys.setAnimation(0, 0, 0);
				_vm->_timers[2] = _vm->getRandom(30) + 20;
				_vm->_timers[3] = _vm->getRandom(50) + 200;
				gameSys.insertSequence(0x1F4, 255, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gnap._sequenceId = 0x1F4;
				gnap._id = 255;
				gnap._sequenceDatNum = 0;
				gameSys.removeSequence(0x207, 257, true);
				gameSys.removeSequence(0x208, 256, true);
				_nextGoggleGuySequenceId = 0x1F8;
				_vm->_timers[5] = _vm->getRandom(100) + 75;
				gameSys.insertSequence(_nextGoggleGuySequenceId, 121, _currGoggleGuySequenceId, 121, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextGoggleGuySequenceId, 121, 2);
				_currGoggleGuySequenceId = _nextGoggleGuySequenceId;
				_nextGoggleGuySequenceId = -1;
				switch (_billardBallCtr) {
				case 0:
					_nextHookGuySequenceId = 0x1FC;
					break;
				case 1:
					_nextHookGuySequenceId = 0x1FD;
					break;
				default:
					_nextHookGuySequenceId = 0x1FE;
					break;
				}
				++_billardBallCtr;
				gameSys.insertSequence(_nextHookGuySequenceId, 120, _currHookGuySequenceId, 120, kSeqSyncWait, 0, 0, 0);
				gameSys.setAnimation(_nextHookGuySequenceId, 120, 3);
				_currHookGuySequenceId = _nextHookGuySequenceId;
				_nextHookGuySequenceId = -1;
				_vm->_timers[4] = _vm->getRandom(40) + 20;
				gameSys.insertSequence(0x208, 256, 0, 0, kSeqNone, _vm->getSequenceTotalDuration(0x1F4) - 5, 0, 0);
				_vm->_hotspots[kHS11Billard]._flags |= SF_DISABLED;
				gameSys.setAnimation(0x207, 257, 4);
				gameSys.insertSequence(0x207, 257, 0, 0, kSeqNone, _vm->getSequenceTotalDuration(0x1FE), 0, 0);
				gnap._actionStatus = -1;
			}
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2 && _nextGoggleGuySequenceId != -1) {
		_vm->_timers[5] = _vm->getRandom(100) + 75;
		gameSys.insertSequence(_nextGoggleGuySequenceId, 121, _currGoggleGuySequenceId, 121, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextGoggleGuySequenceId, 121, 2);
		_currGoggleGuySequenceId = _nextGoggleGuySequenceId;
		_nextGoggleGuySequenceId = -1;
		if (gnap._actionStatus >= 1 && gnap._actionStatus <= 4)
			gnap._actionStatus = -1;
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		if (_nextHookGuySequenceId == 0x204) {
			gameSys.setAnimation(_nextHookGuySequenceId, 120, 3);
			gameSys.insertSequence(0x204, 120, _currHookGuySequenceId, 120, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x1F5, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			_currHookGuySequenceId = 0x204;
			_nextHookGuySequenceId = -1;
			gnap._sequenceId = 0x1F5;
			gnap._sequenceDatNum = 0;
			_vm->_timers[4] = _vm->getRandom(40) + 20;
			_vm->_timers[2] = _vm->getRandom(20) + 70;
			_vm->_timers[3] = _vm->getRandom(50) + 200;
			if (gnap._actionStatus == kAS11GrabHookGuy)
				gnap._actionStatus = -1;
		} else if (_nextHookGuySequenceId != -1) {
			gameSys.insertSequence(_nextHookGuySequenceId, 120, _currHookGuySequenceId, 120, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextHookGuySequenceId, 120, 3);
			_currHookGuySequenceId = _nextHookGuySequenceId;
			_nextHookGuySequenceId = -1;
			_vm->_timers[4] = _vm->getRandom(40) + 20;
			if (gnap._actionStatus >= 6 && gnap._actionStatus <= 9)
				gnap._actionStatus = -1;
		}
	}

	if (gameSys.getAnimationStatus(4) == 2) {
		gameSys.setAnimation(0, 0, 4);
		_vm->_hotspots[kHS11Billard]._flags &= ~SF_DISABLED;
	}
}

/*****************************************************************************/

Scene12::Scene12(GnapEngine *vm) : Scene(vm) {
	_nextBeardGuySequenceId = -1;
	_currBeardGuySequenceId = -1;
	_nextToothGuySequenceId = -1;
	_currToothGuySequenceId = -1;
	_nextBarkeeperSequenceId = -1;
	_currBarkeeperSequenceId = -1;
}

int Scene12::init() {
	return 0x209;
}

void Scene12::updateHotspots() {
	_vm->setHotspot(kHS12Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS12ExitRight, 790, 360, 799, 599, SF_EXIT_R_CURSOR);
	_vm->setHotspot(kHS12ToothGuy, 80, 180, 160, 380, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS12Barkeeper, 490, 175, 580, 238, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS12BeardGuy, 620, 215, 720, 350, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS12Jukebox, 300, 170, 410, 355, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS12WalkArea1, 0, 0, 260, 460);
	_vm->setHotspot(kHS12WalkArea2, 0, 0, 380, 410);
	_vm->setHotspot(kHS12WalkArea3, 0, 0, 799, 395);
	_vm->setHotspot(kHS12WalkArea4, 585, 0, 799, 455);
	_vm->setDeviceHotspot(kHS12Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 11;
}

void Scene12::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	int v18 = 1;

	_vm->queueInsertDeviceIcon();

	gameSys.insertSequence(0x207, 256, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.insertSequence(0x200, 50, 0, 0, kSeqNone, 0, 0, 0);

	_currToothGuySequenceId = 0x200;
	_nextToothGuySequenceId = -1;

	gameSys.setAnimation(0x200, 50, 2);
	gameSys.insertSequence(0x202, 50, 0, 0, kSeqNone, 0, 0, 0);

	_currBeardGuySequenceId = 0x202;
	_nextBeardGuySequenceId = -1;

	gameSys.setAnimation(0x202, 50, 4);
	gameSys.insertSequence(0x203, 50, 0, 0, kSeqNone, 0, 0, 0);

	_currBarkeeperSequenceId = 0x203;
	_nextBarkeeperSequenceId = -1;

	gameSys.setAnimation(0x203, 50, 3);

	_vm->_timers[4] = 30;
	_vm->_timers[6] = _vm->getRandom(30) + 20;
	_vm->_timers[5] = _vm->getRandom(30) + 20;
	_vm->_timers[7] = _vm->getRandom(100) + 100;

	if (_vm->_prevSceneNum == 15) {
		gnap.initPos(5, 6, kDirBottomRight);
		plat.initPos(3, 7, kDirIdleLeft);
		_vm->endSceneInit();
	} else {
		gnap.initPos(11, 8, kDirBottomLeft);
		plat.initPos(12, 8, kDirIdleRight);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(8, 8), -1, 0x107BA, 1);
		plat.walkTo(Common::Point(9, 8), -1, 0x107D2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();
		_vm->updateGrabCursorSprite(0, 0);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS12Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS12Platypus:
			switch (_vm->_verbCursor) {
			case LOOK_CURSOR:
				gnap.playMoan1(plat._pos);
				break;
			case GRAB_CURSOR:
				gnap.kissPlatypus(0);
				break;
			case TALK_CURSOR:
				gnap.playBrainPulsating(plat._pos);
				plat.playSequence(plat.getSequenceId());
				break;
			case PLAT_CURSOR:
				break;
			default:
				break;
			}
			break;

		case kHS12ExitRight:
			_vm->_isLeavingScene = true;
			gnap.walkTo(Common::Point(10, -1), 0, 0x107AB, 1);
			gnap._actionStatus = kAS12LeaveScene;
			plat.walkTo(Common::Point(10, -1), -1, -1, 1);
			_vm->_newSceneNum = 11;
			break;

		case kHS12ToothGuy:
			if (_vm->_grabCursorSpriteIndex == kItemQuarter) {
				_vm->_largeSprite = gameSys.createSurface(0x141);
				gnap.walkTo(Common::Point(3, 7), 0, 0x107BC, 9);
				gnap._idleFacing = kDirUpLeft;
				gnap._actionStatus = kAS12QuarterToToothGuy;
				gnap.playShowItem(_vm->_grabCursorSpriteIndex, 2, 0);
				_vm->setGrabCursorSprite(-1);
			} else if (_vm->_grabCursorSpriteIndex == kItemQuarterWithHole) {
				gnap.walkTo(Common::Point(3, 7), 0, 0x107BC, 9);
				gnap._idleFacing = kDirUpLeft;
				gnap._actionStatus = kAS12QuarterWithHoleToToothGuy;
				gnap.playShowItem(_vm->_grabCursorSpriteIndex, 2, 0);
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.walkTo(Common::Point(3, 7), 0, 0x107BC, 9);
				gnap._idleFacing = kDirUpLeft;
				gnap._actionStatus = kAS12ShowItemToToothGuy;
				gnap.playShowItem(_vm->_grabCursorSpriteIndex, 2, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playScratchingHead(Common::Point(1, 2));
					break;
				case GRAB_CURSOR:
					gnap.walkTo(Common::Point(3, 7), 0, 0x107BC, 1);
					gnap._actionStatus = kAS12GrabToothGuy;
					break;
				case TALK_CURSOR:
					gnap._idleFacing = kDirUpLeft;
					gnap.walkTo(Common::Point(3, 7), 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
					gnap._actionStatus = kAS12TalkToothGuy;
					break;
				case PLAT_CURSOR:
					gnap.useDeviceOnPlatypus();
					plat.walkTo(Common::Point(3, 7), 1, 0x107D2, 1);
					plat._actionStatus = kAS12PlatWithToothGuy;
					plat._idleFacing = kDirIdleRight;
					gnap.playIdle(Common::Point(2, 7));
					break;
				default:
					break;
				}
			}
			break;

		case kHS12Barkeeper:
			if (_vm->_grabCursorSpriteIndex == kItemQuarter || _vm->_grabCursorSpriteIndex == kItemQuarterWithHole) {
				gnap.walkTo(Common::Point(6, 6), 0, 0x107BB, 9);
				gnap._idleFacing = kDirUpRight;
				gnap._actionStatus = kAS12QuarterWithBarkeeper;
				gnap.playShowItem(_vm->_grabCursorSpriteIndex, 7, 0);
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.walkTo(Common::Point(6, 6), 0, 0x107BB, 9);
				gnap._idleFacing = kDirUpRight;
				gnap._actionStatus = kAS12ShowItemToBarkeeper;
				gnap.playShowItem(_vm->_grabCursorSpriteIndex, 7, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.walkTo(Common::Point(6, 6), 0, 0x107BB, 1);
					gnap._idleFacing = kDirUpRight;
					gnap._actionStatus = kAS12LookBarkeeper;
					break;
				case GRAB_CURSOR:
					gnap.playImpossible();
					break;
				case TALK_CURSOR:
					gnap._idleFacing = kDirUpRight;
					gnap.walkTo(Common::Point(6, 6), 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
					gnap._actionStatus = kAS12TalkBarkeeper;
					break;
				case PLAT_CURSOR:
					gnap.playPullOutDevice(plat._pos);
					gameSys.setAnimation(makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, 0);
					gnap._actionStatus = kAS12PlatWithBarkeeper;
					break;
				default:
					break;
				}
			}
			break;

		case kHS12BeardGuy:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.walkTo(Common::Point(7, 6), 0, 0x107BB, 9);
				gnap._idleFacing = kDirUpRight;
				gnap._actionStatus = kAS12ShowItemToBeardGuy;
				gnap.playShowItem(_vm->_grabCursorSpriteIndex, 8, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.walkTo(Common::Point(7, 6), 0, 0x107BB, 1);
					gnap._idleFacing = kDirUpRight;
					gnap._actionStatus = kAS12LookBeardGuy;
					break;
				case GRAB_CURSOR:
					// NOTE Bug in the original. It has 9 as flags which seems wrong here.
					gnap.walkTo(Common::Point(7, 6), 0, 0x107BB, 1);
					gnap._idleFacing = kDirUpRight;
					gnap._actionStatus = kAS12GrabBeardGuy;
					break;
				case TALK_CURSOR:
					gnap._idleFacing = kDirUpRight;
					gnap.walkTo(Common::Point(7, 6), 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
					gnap._actionStatus = kAS12TalkBeardGuy;
					break;
				case PLAT_CURSOR:
					gnap.useDeviceOnPlatypus();
					plat.walkTo(Common::Point(7, 6), 1, 0x107C2, 1);
					plat._actionStatus = kAS12PlatWithBeardGuy;
					plat._idleFacing = kDirIdleLeft;
					gnap.playIdle(Common::Point(7, 6));
					break;
				default:
					break;
				}
			}
			break;

		case kHS12Jukebox:
			_vm->_newSceneNum = 15;
			_vm->_isLeavingScene = true;
			gnap.walkTo(Common::Point(5, 6), 0, 0x107BC, 1);
			gnap._actionStatus = kAS12LeaveScene;
			break;

		case kHS12WalkArea1:
		case kHS12WalkArea2:
		case kHS12WalkArea3:
		case kHS12WalkArea4:
			if (gnap._actionStatus < 0)
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->_isLeavingScene) {
			plat.updateIdleSequence();
			gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = 15;
				if (_nextToothGuySequenceId == -1) {
					if (v18 == 0 && _currBeardGuySequenceId == 0x202 && _currBarkeeperSequenceId == 0x203 && gnap._actionStatus < 0 && plat._actionStatus < 0) {
						if (_vm->getRandom(2) != 0)
							_nextToothGuySequenceId = 0x1EC;
						else
							_nextToothGuySequenceId = 0x204;
					} else if (_currToothGuySequenceId != 0x200)
						_nextToothGuySequenceId = 0x200;
					v18 = (v18 + 1) % 15;
				}
			}
			if (!_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(30) + 20;
				if (_nextBarkeeperSequenceId == -1 && gnap._actionStatus < 0 && plat._actionStatus < 0) {
					if (v18 == 0 && _currToothGuySequenceId == 0x200 && _currBeardGuySequenceId == 0x202 && gnap._actionStatus < 0 && plat._actionStatus < 0) {
						if (_vm->getRandom(2) != 0)
							_nextBarkeeperSequenceId = 0x208;
						else
							_nextBarkeeperSequenceId = 0x1FB;
					} else
						_nextBarkeeperSequenceId = 0x203;
					v18 = (v18 + 1) % 15;
				}
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(30) + 15;
				if (_nextBeardGuySequenceId == -1 && gnap._actionStatus < 0 && plat._actionStatus < 0) {
					if (v18 == 0 && _currToothGuySequenceId == 0x200 && _currBarkeeperSequenceId == 0x203 && gnap._actionStatus < 0 && plat._actionStatus < 0)
						_nextBeardGuySequenceId = 0x1F2;
					else
						_nextBeardGuySequenceId = 0x202;
					v18 = (v18 + 1) % 15;
				}
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(100) + 100;
				int _gnapRandomValue = _vm->getRandom(3);
				switch (_gnapRandomValue) {
				case 0:
					gameSys.insertSequence(0x8A5 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 1:
					gameSys.insertSequence(0x8A7 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				case 2:
					gameSys.insertSequence(0x8A6 | 0x10000, 179, 0, 0, kSeqNone, 0, 0, 0);
					break;
				default:
					break;
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = 30;
			_vm->_timers[5] = _vm->getRandom(30) + 20;
			_vm->_timers[6] = _vm->getRandom(30) + 20;
		}

		_vm->gameUpdateTick();
	}
}

void Scene12::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS12LeaveScene:
			_vm->_sceneDone = true;
			break;
		case kAS12TalkToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				// Easter egg
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_nextToothGuySequenceId = 0x206;
			} else {
				_nextToothGuySequenceId = 0x1EE;
			}
			break;
		case 3:
			break;
		case kAS12GrabToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_nextToothGuySequenceId = 0x206;
			} else {
				_nextToothGuySequenceId = 0x1EF;
			}
			break;
		case kAS12ShowItemToToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_nextToothGuySequenceId = 0x206;
			} else {
				_nextToothGuySequenceId = 0x1ED;
			}
			break;
		case kAS12QuarterWithHoleToToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_nextToothGuySequenceId = 0x206;
			} else {
				_nextToothGuySequenceId = 0x1EA;
			}
			break;
		case kAS12QuarterToToothGuy:
			if (_vm->isKeyStatus1(Common::KEYCODE_j)) {
				_vm->clearKeyStatus1(Common::KEYCODE_j);
				_nextToothGuySequenceId = 0x206;
			} else {
				_nextToothGuySequenceId = 0x1E9;
			}
			break;
		case kAS12QuarterToToothGuyDone:
			gnap._actionStatus = -1;
			_vm->showCursor();
			gameSys.removeSpriteDrawItem(_vm->_largeSprite, 300);
			_vm->deleteSurface(&_vm->_largeSprite);
			_vm->setGrabCursorSprite(kItemQuarterWithHole);
			break;
		case kAS12TalkBeardGuy:
			_nextBeardGuySequenceId = 0x1F4;
			break;
		case kAS12LookBeardGuy:
			_nextBeardGuySequenceId = 0x1F3;
			break;
		case kAS12GrabBeardGuy:
			_nextBeardGuySequenceId = 0x1F1;
			break;
		case kAS12ShowItemToBeardGuy:
			_nextBeardGuySequenceId = 0x1F0;
			break;
		case kAS12TalkBarkeeper:
			if (_vm->getRandom(2) != 0)
				_nextBarkeeperSequenceId = 0x1FD;
			else
				_nextBarkeeperSequenceId = 0x1FF;
			break;
		case kAS12LookBarkeeper:
			_nextBarkeeperSequenceId = 0x1F8;
			break;
		case 14:
			_nextBarkeeperSequenceId = 0x1F6;
			break;
		case kAS12ShowItemToBarkeeper:
			_nextBarkeeperSequenceId = 0x1F5;
			break;
		case kAS12QuarterWithBarkeeper:
			_nextBarkeeperSequenceId = 0x1FA;
			break;
		case kAS12PlatWithBarkeeper:
			_nextBarkeeperSequenceId = 0x1F9;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		switch (plat._actionStatus) {
		case kAS12PlatWithToothGuy:
			_nextToothGuySequenceId = 0x1EB;
			break;
		case kAS12PlatWithBeardGuy:
			_nextBeardGuySequenceId = 0x1F3;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		if (_currToothGuySequenceId == 0x1E9) {
			gameSys.setAnimation(0, 0, 2);
			_vm->hideCursor();
			gameSys.setAnimation(0x10843, 301, 0);
			gnap._actionStatus = kAS12QuarterToToothGuyDone;
			gameSys.insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
			gameSys.insertSequence(0x10843, 301, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x107B7, gnap._id, 0x10843, 301,
				kSeqSyncWait, 0, 75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
			gnap._sequenceId = 0x7B7;
			gnap._sequenceDatNum = 1;
			_vm->setFlag(kGFTwigTaken);
			_vm->invAdd(kItemQuarterWithHole);
			_vm->invRemove(kItemQuarter);
		}
		if (_nextToothGuySequenceId == 0x1EF) {
			gameSys.setAnimation(_nextToothGuySequenceId, 50, 2);
			gameSys.insertSequence(_nextToothGuySequenceId, 50, _currToothGuySequenceId, 50, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x205, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			_currToothGuySequenceId = _nextToothGuySequenceId;
			_nextToothGuySequenceId = -1;
			gnap._sequenceId = 0x205;
			gnap._sequenceDatNum = 0;
			_vm->_timers[4] = 40;
			_vm->_timers[2] = _vm->getRandom(20) + 70;
			_vm->_timers[3] = _vm->getRandom(50) + 200;
			if (gnap._actionStatus == kAS12GrabToothGuy)
				gnap._actionStatus = -1;
		} else if (_nextToothGuySequenceId != -1) {
			gameSys.insertSequence(_nextToothGuySequenceId, 50, _currToothGuySequenceId, 50, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextToothGuySequenceId, 50, 2);
			_currToothGuySequenceId = _nextToothGuySequenceId;
			_nextToothGuySequenceId = -1;
			_vm->_timers[4] = 50;
			if (gnap._actionStatus >= kAS12TalkToothGuy && gnap._actionStatus <= kAS12QuarterToToothGuy && _currToothGuySequenceId != 0x1E9 &&
				_currToothGuySequenceId != 0x1EC && _currToothGuySequenceId != 0x200)
				gnap._actionStatus = -1;
			if (plat._actionStatus == kAS12PlatWithToothGuy)
				plat._actionStatus = -1;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		if (gnap._actionStatus == kAS12PlatWithBarkeeper && _currBarkeeperSequenceId == 0x1F9) {
			gnap._actionStatus = -1;
			gnap.playIdle(Common::Point(7, 6));
			_vm->_timers[5] = 0;
		}
		if (_nextBarkeeperSequenceId != -1) {
			gameSys.insertSequence(_nextBarkeeperSequenceId, 50, _currBarkeeperSequenceId, 50, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(_nextBarkeeperSequenceId, 50, 3);
			_currBarkeeperSequenceId = _nextBarkeeperSequenceId;
			_nextBarkeeperSequenceId = -1;
			_vm->_timers[5] = _vm->getRandom(30) + 20;
			if (gnap._actionStatus >= kAS12TalkBarkeeper && gnap._actionStatus <= kAS12QuarterWithBarkeeper && _currBarkeeperSequenceId != 0x203 &&
				_currBarkeeperSequenceId != 0x1FB && _currBarkeeperSequenceId != 0x208)
				gnap._actionStatus = -1;
		}
	}

	if (gameSys.getAnimationStatus(4) == 2 && _nextBeardGuySequenceId != -1) {
		gameSys.insertSequence(_nextBeardGuySequenceId, 50, _currBeardGuySequenceId, 50, kSeqSyncWait, 0, 0, 0);
		gameSys.setAnimation(_nextBeardGuySequenceId, 50, 4);
		_currBeardGuySequenceId = _nextBeardGuySequenceId;
		_nextBeardGuySequenceId = -1;
		_vm->_timers[6] = _vm->getRandom(30) + 20;
		if (gnap._actionStatus >= kAS12TalkBeardGuy && gnap._actionStatus <= kAS12ShowItemToBeardGuy && _currBeardGuySequenceId != 0x202 && _currBeardGuySequenceId != 0x1F2)
			gnap._actionStatus = -1;
		if (plat._actionStatus == kAS12PlatWithBeardGuy)
			plat._actionStatus = -1;
	}
}

/*****************************************************************************/

Scene13::Scene13(GnapEngine *vm) : Scene(vm) {
	_backToiletCtr = -1;
}

int Scene13::init() {
	_vm->playSound(0x108EC, false);
	return 0xAC;
}

void Scene13::updateHotspots() {
	_vm->setHotspot(kHS13Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS13ExitBar, 113, 160, 170, 455, SF_EXIT_L_CURSOR);
	_vm->setHotspot(kHS13BackToilet, 385, 195, 478, 367, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS13FrontToilet, 497, 182, 545, 432, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS13Urinal, 680, 265, 760, 445, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS13Scribble, 560, 270, 660, 370, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS13Sink, 310, 520, 560, 599, SF_WALKABLE | SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS13WalkArea1, 268, 270, 325, 385);
	_vm->setHotspot(kHS13WalkArea2, 0, 0, 52, 599);
	_vm->setHotspot(kHS13WalkArea3, 0, 0, 113, 550);
	_vm->setHotspot(kHS13WalkArea4, 0, 0, 226, 438);
	_vm->setHotspot(kHS13WalkArea5, 0, 0, 268, 400);
	_vm->setHotspot(kHS13WalkArea6, 0, 0, 799, 367);
	_vm->setHotspot(kHS13WalkArea7, 478, 0, 799, 401);
	_vm->setHotspot(kHS13WalkArea8, 545, 0, 799, 473);
	_vm->setHotspot(kHS13WalkArea9, 0, 549, 799, 599);
	_vm->setDeviceHotspot(kHS13Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 17;
}

void Scene13::showScribble() {
	GameSys& gameSys = *_vm->_gameSys;

	_vm->hideCursor();
	_vm->_largeSprite = gameSys.createSurface(0x6F);
	gameSys.insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
	while (!_vm->_mouseClickState._left && !_vm->isKeyStatus1(Common::KEYCODE_ESCAPE) &&
		!_vm->isKeyStatus1(Common::KEYCODE_SPACE) && !_vm->isKeyStatus1(Common::KEYCODE_RETURN) && !_vm->_gameDone)
		_vm->gameUpdateTick();
	_vm->_mouseClickState._left = false;
	_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
	_vm->clearKeyStatus1(Common::KEYCODE_RETURN);
	_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
	gameSys.removeSpriteDrawItem(_vm->_largeSprite, 300);
	_vm->deleteSurface(&_vm->_largeSprite);
	_vm->showCursor();
}

void Scene13::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	int currSoundId = 0;

	_vm->queueInsertDeviceIcon();
	gameSys.insertSequence(0xAA, 256, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->_prevSceneNum == 14) {
		gnap.initPos(6, 6, kDirBottomLeft);
		plat.initPos(9, 8, kDirIdleLeft);
	} else {
		gnap.initPos(3, 7, kDirBottomRight);
		plat.initPos(2, 7, kDirIdleLeft);
	}

	_vm->endSceneInit();

	_vm->_timers[4] = _vm->getRandom(20) + 20;
	_vm->_timers[5] = _vm->getRandom(50) + 50;

	while (!_vm->_sceneDone) {
		if (!_vm->isSoundPlaying(0x1091A))
			_vm->playSound(0x1091A, true);

		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS13Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				_vm->_timers[5] = _vm->getRandom(50) + 50;
			}
			break;

		case kHS13Platypus:
			switch (_vm->_verbCursor) {
			case LOOK_CURSOR:
				gnap.playMoan1(plat._pos);
				break;
			case GRAB_CURSOR:
				gnap.kissPlatypus(0);
				break;
			case TALK_CURSOR:
				gnap.playBrainPulsating(plat._pos);
				plat.playSequence(plat.getSequenceId());
				break;
			case PLAT_CURSOR:
				break;
			default:
				break;
			}
			break;

		case kHS13ExitBar:
			_vm->_isLeavingScene = true;
			gnap.walkTo(Common::Point(2, 7), 0, 0x107C0, 1);
			gnap._actionStatus = kAS13LeaveScene;
			plat.walkTo(Common::Point(2, 8), -1, -1, 1);
			if (_vm->isFlag(kGFUnk14) || _vm->isFlag(kGFSpringTaken)) {
				_vm->_newSceneNum = 11;
			} else {
				_vm->setFlag(kGFSpringTaken);
				_vm->_newSceneNum = 47;
			}
			break;

		case kHS13BackToilet:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(5, 5), 6, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
				case TALK_CURSOR:
					if (gnap._pos == Common::Point(5, 5)) {
						_backToiletCtr = MIN(5, _backToiletCtr + 1);
						gameSys.setAnimation(_backToiletCtr + 0xA3, gnap._id, 0);
						gameSys.insertSequence(_backToiletCtr + 0xA3, gnap._id,
							makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
							kSeqScale | kSeqSyncWait, 0, 0, 0);
						gnap._actionStatus = kAS13Wait;
						gnap._sequenceId = _backToiletCtr + 0xA3;
						gnap._idleFacing = kDirUpRight;
						gnap._sequenceDatNum = 0;
					} else {
						gnap.walkTo(Common::Point(5, 5), 0, 0x107BB, 1);
						gnap._actionStatus = kAS13BackToilet;
						gnap._idleFacing = kDirUpRight;
					}
					break;
				case PLAT_CURSOR:
					gnap.playImpossible();
					break;
				default:
					break;
				}
			}
			break;

		case kHS13FrontToilet:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(6, 7), 7, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
				case TALK_CURSOR:
				case PLAT_CURSOR:
					gnap.walkTo(Common::Point(6, 7), 0, 0xA9, 5);
					gnap._actionStatus = kAS13FrontToilet;
					gnap._idleFacing = kDirBottomRight;
					break;
				default:
					break;
				}
			}
			break;

		case kHS13Scribble:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(7, 7), 8, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.walkTo(Common::Point(7, 7), 0, 0x107BB, 1);
					gnap._actionStatus = kAS13LookScribble;
					gnap._idleFacing = kDirUpRight;
					break;
				case GRAB_CURSOR:
					gnap.playScratchingHead();
					break;
				case TALK_CURSOR:
					gnap._idleFacing = kDirUpRight;
					gnap.walkTo(Common::Point(7, 7), -1, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)), 1);
					break;
				case PLAT_CURSOR:
					gnap.playImpossible();
					break;
				default:
					break;
				}
			}
			break;

		case kHS13Urinal:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playShowCurrItem(Common::Point(8, 7), 9, 0);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playSequence(gnap.getSequenceId(kGSDeflect, Common::Point(9, 6)));
					gnap.walkTo(gnap._pos, 0, -1, 1);
					gnap._actionStatus = kAS13Wait;
					break;
				case GRAB_CURSOR:
					gnap.walkTo(Common::Point(8, 7), 0, -1, 1);
					gnap._actionStatus = kAS13GrabUrinal;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					gnap.playImpossible();
					break;
				default:
					break;
				}
			}
			break;

		case kHS13Sink:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gnap.playImpossible();
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					gnap.playSequence(gnap.getSequenceId(kGSDeflect, Common::Point(5, 9)));
					gnap.walkTo(gnap._pos, 0, -1, 1);
					gnap._actionStatus = kAS13Wait;
					break;
				case GRAB_CURSOR:
					gnap.walkTo(Common::Point(4, 8), 0, 0x107B9, 1);
					gnap._actionStatus = kAS13GrabSink;
					break;
				case TALK_CURSOR:
				case PLAT_CURSOR:
					gnap.playImpossible();
					break;
				default:
					break;
				}
			}
			break;

		case kHS13WalkArea2:
		case kHS13WalkArea3:
		case kHS13WalkArea4:
		case kHS13WalkArea5:
		case kHS13WalkArea6:
		case kHS13WalkArea7:
		case kHS13WalkArea8:
		case kHS13WalkArea9:
			gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		case kHS13WalkArea1:
			// Nothing
			break;

		default:
			if (_vm->_mouseClickState._left) {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->_isLeavingScene) {
			plat.updateIdleSequence();
			if (plat._pos.y == 5 || plat._pos.y == 6)
				plat.walkTo(Common::Point(-1, 7), -1, -1, 1);
			if (gnap._actionStatus < 0)
				gnap.updateIdleSequence();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(20) + 20;
				switch (_vm->getRandom(5)) {
				case 0:
					_vm->playSound(0xD2, false);
					break;
				case 1:
					_vm->playSound(0xD3, false);
					break;
				case 2:
					_vm->playSound(0xD4, false);
					break;
				case 3:
					_vm->playSound(0xD5, false);
					break;
				case 4:
					_vm->playSound(0xD6, false);
					break;
				default:
					break;
				}
			}
			if (!_vm->_timers[5]) {
				int newSoundId;
				_vm->_timers[5] = _vm->getRandom(50) + 50;
				switch (_vm->getRandom(7)) {
				case 0:
					newSoundId = 0xD7;
					_vm->_timers[5] = 2 * _vm->getRandom(50) + 100;
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
					_vm->playSound(newSoundId, false);
					currSoundId = newSoundId;
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
			_vm->_timers[4] = _vm->getRandom(20) + 20;
			_vm->_timers[5] = _vm->getRandom(50) + 50;
		}

		_vm->gameUpdateTick();
	}
}

void Scene13::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS13LeaveScene:
			_vm->_sceneDone = true;
			gnap._actionStatus = -1;
			break;
		case kAS13BackToilet:
			_backToiletCtr = MIN(5, _backToiletCtr + 1);
			gameSys.insertSequence(_backToiletCtr + 0xA3, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, 9, 0, 0, 0);
			gnap._sequenceId = _backToiletCtr + 0xA3;
			gnap._sequenceDatNum = 0;
			gnap._actionStatus = -1;
			break;
		case kAS13FrontToilet:
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 14;
			break;
		case kAS13LookScribble:
			gnap._actionStatus = -1;
			showScribble();
			break;
		case kAS13GrabSink:
			gameSys.setAnimation(0xAB, 160, 0);
			gameSys.insertSequence(0xAB, 160, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.removeSequence(0xAA, 256, true);
			gnap._sequenceId = 0xAB;
			gnap._id = 160;
			gnap._idleFacing = kDirBottomRight;
			gnap._sequenceDatNum = 0;
			gnap._pos = Common::Point(4, 8);
			_vm->_timers[2] = 360;
			gnap._actionStatus = kAS13GrabSinkDone;
			break;
		case kAS13GrabSinkDone:
			gameSys.insertSequence(0xAA, 256, 0, 0, kSeqNone, 0, 0, 0);
			gnap._actionStatus = -1;
			break;
		case kAS13Wait:
			gnap._actionStatus = -1;
			break;
		case kAS13GrabUrinal:
			gameSys.setAnimation(0xA2, 120, 0);
			gameSys.insertSequence(0xA2, 120, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0xA2;
			gnap._id = 120;
			gnap._idleFacing = kDirBottomLeft;
			gnap._sequenceDatNum = 0;
			gnap._pos = Common::Point(4, 6);
			_vm->_timers[2] = 360;
			gnap._actionStatus = kAS13Wait;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		_vm->_plat->_actionStatus = -1;
	}
}

/*****************************************************************************/

Scene14::Scene14(GnapEngine *vm) : Scene(vm) {
}

int Scene14::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	return 0x27;
}

void Scene14::updateHotspots() {
	_vm->setHotspot(kHS14Platypus, 0, 0, 0, 0);
	_vm->setHotspot(kHS14Exit, 0, 590, 799, 599, SF_EXIT_D_CURSOR);
	_vm->setHotspot(kHS14Coin, 330, 390, 375, 440, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS14Toilet, 225, 250, 510, 500, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setDeviceHotspot(kHS14Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFNeedleTaken))
		_vm->_hotspots[kHS14Coin]._flags = SF_DISABLED;
	_vm->_hotspotsCount = 5;
}

void Scene14::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	_vm->_largeSprite = nullptr;
	_vm->queueInsertDeviceIcon();

	if (!_vm->isFlag(kGFNeedleTaken))
		gameSys.insertSequence(0x23, 10, 0, 0, kSeqNone, 0, 0, 0);

	_vm->endSceneInit();

	if (!_vm->isFlag(kGFNeedleTaken) && _vm->invHas(kItemTongs))
		_vm->_largeSprite = gameSys.createSurface(1);

	if (!_vm->isFlag(kGFNeedleTaken)) {
		gameSys.insertSequence(0x24, 10, 0x23, 10, kSeqSyncWait, 0, 0, 0);
		gnap._sequenceId = 0x24;
		_vm->_timers[2] = _vm->getRandom(40) + 50;
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS14Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS14Exit:
			_vm->_sceneDone = true;
			_vm->_newSceneNum = 13;
			break;

		case kHS14Coin:
			if (_vm->_grabCursorSpriteIndex == kItemTongs) {
				_vm->invAdd(kItemQuarter);
				_vm->setFlag(kGFNeedleTaken);
				_vm->setGrabCursorSprite(-1);
				_vm->hideCursor();
				gameSys.setAnimation(0x26, 10, 0);
				gameSys.insertSequence(0x26, 10, gnap._sequenceId, 10, kSeqSyncWait, 0, 0, 0);
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
				_vm->playSound(0x108E9, false);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playSound(0x108E9, false);
					break;
				case GRAB_CURSOR:
					gameSys.insertSequence(0x25, 10, gnap._sequenceId, 10, kSeqSyncWait, 0, 0, 0);
					gameSys.insertSequence(0x23, 10, 0x25, 10, kSeqSyncWait, 0, 0, 0);
					gnap._sequenceId = 0x23;
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					gameSys.insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - gnap._gridX, 576 - gnap._gridY);
					break;
				default:
					break;
				}
			}
			break;

		case kHS14Toilet:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gameSys.insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - gnap._gridX, 576 - gnap._gridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
					_vm->playSound(0x108B1, false);
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					gameSys.insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - gnap._gridX, 576 - gnap._gridY);
					break;
				default:
					break;
				}
			}
			break;

		default:
			_vm->_mouseClickState._left = false;
			break;
		}

		updateAnimations();
		_vm->checkGameKeys();

		if (!_vm->isFlag(kGFNeedleTaken) && !_vm->_timers[2]) {
			gameSys.insertSequence(0x24, 10, gnap._sequenceId, 10, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0x24;
			_vm->_timers[2] = _vm->getRandom(40) + 50;
		}

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}

	if (_vm->_largeSprite)
		_vm->deleteSurface(&_vm->_largeSprite);
}

void Scene14::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		gameSys.insertSpriteDrawItem(_vm->_largeSprite, 0, 0, 300);
		gameSys.setAnimation(0x10843, 301, 1);
		gameSys.insertSequence(0x10843, 301, 0x26, 10, kSeqSyncWait, 0, 0, 0);
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		_vm->_sceneDone = true;
		_vm->_newSceneNum = 13;
		_vm->_grabCursorSpriteIndex = kItemQuarter;
	}
}

/*****************************************************************************/

Scene15::Scene15(GnapEngine *vm) : Scene(vm) {
	_nextRecordSequenceId = -1;
	_currRecordSequenceId = -1;
	_nextSlotSequenceId = -1;
	_currSlotSequenceId = -1;
	_nextUpperButtonSequenceId = -1;
	_currUpperButtonSequenceId = -1;
	_nextLowerButtonSequenceId = -1;
	_currLowerButtonSequenceId = -1;
}

int Scene15::init() {
	return 0xDD;
}

void Scene15::updateHotspots() {
	_vm->setHotspot(kHS15Platypus, 0, 0, 0, 0, SF_DISABLED);
	_vm->setHotspot(kHS15Exit, 50, 590, 750, 599, SF_EXIT_D_CURSOR);
	_vm->setHotspot(kHS15Button1, 210, 425, 260, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15Button2, 280, 425, 325, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15Button3, 340, 425, 385, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15Button4, 400, 425, 445, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15Button5, 460, 425, 510, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15Button6, 520, 425, 560, 475, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15ButtonA, 205, 480, 250, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15ButtonB, 270, 480, 320, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15ButtonC, 335, 480, 380, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15ButtonD, 395, 480, 445, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15ButtonE, 460, 480, 505, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15ButtonF, 515, 480, 560, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15CoinSlot, 585, 475, 620, 535, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS15PlayButton, 622, 431, 650, 482, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setDeviceHotspot(kHS15Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 17;
}

void Scene15::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	_currSlotSequenceId = -1;
	_currUpperButtonSequenceId = -1;
	_currLowerButtonSequenceId = -1;
	_nextSlotSequenceId = -1;
	_nextUpperButtonSequenceId = -1;
	_nextLowerButtonSequenceId = -1;
	_currRecordSequenceId = 0xD5;
	_nextRecordSequenceId = -1;

	gameSys.setAnimation(0xD5, 1, 0);
	gameSys.insertSequence(_currRecordSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	_vm->endSceneInit();

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_hotspots[kHS15Platypus].clearRect();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS15Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS15Exit:
			_vm->_newSceneNum = 12;
			_vm->_isLeavingScene = true;
			break;

		case kHS15CoinSlot:
			if (_vm->_grabCursorSpriteIndex == kItemQuarter || _vm->_grabCursorSpriteIndex == kItemQuarterWithHole) {
				_nextSlotSequenceId = 0xDC; // Insert coin
			} else if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
				_nextSlotSequenceId = 0xDB;
			} else if (_vm->_grabCursorSpriteIndex >= 0) {
				gameSys.insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - gnap._gridX, 576 - gnap._gridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
				case GRAB_CURSOR:
					_vm->playSound(0x108E9, false);
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					gameSys.insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - gnap._gridX, 576 - gnap._gridY);
					break;
				default:
					break;
				}
			}
			break;

		case kHS15PlayButton:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gameSys.insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - gnap._gridX, 576 - gnap._gridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					if (_vm->isFlag(kGFGnapControlsToyUFO) || _vm->isFlag(kGFUnk13))
						_vm->playSound(0x108E9, false);
					else
						_nextSlotSequenceId = 0xDA;
					break;
				case GRAB_CURSOR:
					if (_vm->isFlag(kGFGnapControlsToyUFO) || _vm->isFlag(kGFUnk13))
						_nextSlotSequenceId = 0xD9;
					else
						_nextSlotSequenceId = 0xDA;
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					gameSys.insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - gnap._gridX, 576 - gnap._gridY);
					break;
				default:
					break;
				}
			}
			break;

		case kHS15Button1:
		case kHS15Button2:
		case kHS15Button3:
		case kHS15Button4:
		case kHS15Button5:
		case kHS15Button6:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gameSys.insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - gnap._gridX, 576 - gnap._gridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playSound(0x108E9, false);
					break;
				case GRAB_CURSOR:
					_nextUpperButtonSequenceId = _vm->_sceneClickedHotspot + 0xC5;
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					gameSys.insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - gnap._gridX, 576 - gnap._gridY);
					break;
				default:
					break;
				}
			}
			break;

		case kHS15ButtonA:
		case kHS15ButtonB:
		case kHS15ButtonC:
		case kHS15ButtonD:
		case kHS15ButtonE:
		case kHS15ButtonF:
			if (_vm->_grabCursorSpriteIndex >= 0) {
				gameSys.insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - gnap._gridX, 576 - gnap._gridY);
			} else {
				switch (_vm->_verbCursor) {
				case LOOK_CURSOR:
					_vm->playSound(0x108E9, false);
					break;
				case GRAB_CURSOR:
					_nextLowerButtonSequenceId = _vm->_sceneClickedHotspot + 0xC5;
					break;
				case TALK_CURSOR:
					_vm->playSound((_vm->getRandom(5) + 0x8D5) | 0x10000, false);
					break;
				case PLAT_CURSOR:
					gameSys.insertSequence(0x107A8, 1, 0, 0, kSeqNone, 0, 900 - gnap._gridX, 576 - gnap._gridY);
					break;
				default:
					break;
				}
			}
			break;

		default:
			_vm->_mouseClickState._left = false;
			break;

		}

		updateAnimations();
		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}
}

void Scene15::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;

	if (gameSys.getAnimationStatus(0) == 2) {
		if (_vm->_isLeavingScene) {
			_vm->_sceneDone = true;
		} else if (_nextSlotSequenceId != -1) {
			gameSys.setAnimation(_nextSlotSequenceId, 1, 0);
			gameSys.insertSequence(_nextSlotSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
			_currSlotSequenceId = _nextSlotSequenceId;
			_nextSlotSequenceId = -1;
			switch (_currSlotSequenceId) {
			case 0xDC:
				if (_vm->_grabCursorSpriteIndex == kItemQuarter) {
					_vm->invRemove(kItemQuarter);
				} else {
					_vm->invRemove(kItemQuarterWithHole);
					_vm->setFlag(kGFUnk13);
				}
				_vm->setGrabCursorSprite(-1);
				break;
			case 0xDB:
				_vm->setFlag(kGFUnk14);
				_vm->setGrabCursorSprite(-1);
				_nextSlotSequenceId = 0xD8;
				break;
			case 0xD9:
				if (_vm->isFlag(kGFGnapControlsToyUFO)) {
					_vm->clearFlag(kGFGnapControlsToyUFO);
					_vm->invAdd(kItemQuarter);
					_vm->_newGrabCursorSpriteIndex = kItemQuarter;
				} else if (_vm->isFlag(kGFUnk13)) {
					_vm->clearFlag(kGFUnk13);
					_vm->invAdd(kItemQuarterWithHole);
					_vm->_newGrabCursorSpriteIndex = kItemQuarterWithHole;
				}
				_vm->_newSceneNum = 12;
				_vm->_isLeavingScene = true;
				break;
			case 0xD8:
			case 0xDA:
				if (_currUpperButtonSequenceId != -1) {
					gameSys.removeSequence(_currUpperButtonSequenceId, 1, true);
					_currUpperButtonSequenceId = -1;
				}
				if (_currLowerButtonSequenceId != -1) {
					gameSys.removeSequence(_currLowerButtonSequenceId, 1, true);
					_currLowerButtonSequenceId = -1;
				}
				break;
			default:
				break;
			}
		} else if (_nextRecordSequenceId != -1) {
			gameSys.setAnimation(_nextRecordSequenceId, 1, 0);
			gameSys.insertSequence(_nextRecordSequenceId, 1, _currRecordSequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_currRecordSequenceId = _nextRecordSequenceId;
			_nextRecordSequenceId = -1;
			if (_currRecordSequenceId == 0xD3) {
				_vm->invRemove(kItemDiceQuarterHole);
				_vm->_newSceneNum = 16;
				_vm->_isLeavingScene = true;
			}
			gameSys.removeSequence(_currUpperButtonSequenceId, 1, true);
			_currUpperButtonSequenceId = -1;
			gameSys.removeSequence(_currLowerButtonSequenceId, 1, true);
			_currLowerButtonSequenceId = -1;
		} else if (_nextUpperButtonSequenceId != -1) {
			gameSys.setAnimation(_nextUpperButtonSequenceId, 1, 0);
			if (_currUpperButtonSequenceId == -1)
				gameSys.insertSequence(_nextUpperButtonSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
			else
				gameSys.insertSequence(_nextUpperButtonSequenceId, 1, _currUpperButtonSequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_currUpperButtonSequenceId = _nextUpperButtonSequenceId;
			_nextUpperButtonSequenceId = -1;
			if (_currLowerButtonSequenceId != -1 && _vm->isFlag(kGFUnk14)) {
				if (_currUpperButtonSequenceId == 0xCC && _currLowerButtonSequenceId == 0xCE)
					_nextRecordSequenceId = 0xD3;
				else
					_nextRecordSequenceId = 0xD4;
			}
		} else if (_nextLowerButtonSequenceId != -1) {
			gameSys.setAnimation(_nextLowerButtonSequenceId, 1, 0);
			if (_currLowerButtonSequenceId == -1)
				gameSys.insertSequence(_nextLowerButtonSequenceId, 1, 0, 0, kSeqNone, 0, 0, 0);
			else
				gameSys.insertSequence(_nextLowerButtonSequenceId, 1, _currLowerButtonSequenceId, 1, kSeqSyncWait, 0, 0, 0);
			_currLowerButtonSequenceId = _nextLowerButtonSequenceId;
			_nextLowerButtonSequenceId = -1;
			if (_currUpperButtonSequenceId != -1 && _vm->isFlag(kGFUnk14)) {
				if (_currUpperButtonSequenceId == 0xCC && _currLowerButtonSequenceId == 0xCE)
					_nextRecordSequenceId = 0xD3;
				else
					_nextRecordSequenceId = 0xD4;
			}
		}
	}
}

/*****************************************************************************/

Scene17::Scene17(GnapEngine *vm) : Scene(vm) {
	_platTryGetWrenchCtr = 0;
	_wrenchCtr = 2;
	_nextCarWindowSequenceId = -1;
	_nextWrenchSequenceId = -1;
	_canTryGetWrench = true;
	_platPhoneCtr = 0;
	_nextPhoneSequenceId = -1;
	_currPhoneSequenceId = -1;
	_currWrenchSequenceId = -1;
	_currCarWindowSequenceId = -1;
}

int Scene17::init() {
	return 0x263;
}

void Scene17::updateHotspots() {
	// The original is using (1, 0, 0, 0)
	_vm->setHotspot(kHS17Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS17Phone1, 61, 280, 97, 322, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	_vm->setHotspot(kHS17Phone2, 80, 204, 178, 468, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 7);
	_vm->setHotspot(kHS17ExitGrubCity, 196, 207, 280, 304, SF_EXIT_U_CURSOR, 3, 5);
	_vm->setHotspot(kHS17ExitToyStore, 567, 211, 716, 322, SF_EXIT_U_CURSOR, 5, 6);
	_vm->setHotspot(kHS17Wrench, 586, 455, 681, 547, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
	_vm->setHotspot(kHS17WalkArea1, 0, 0, 800, 434);
	_vm->setHotspot(kHS17WalkArea2, 541, 0, 800, 600);
	_vm->setHotspot(kHS17WalkArea3, 0, 204, 173, 468);
	_vm->setDeviceHotspot(kHS17Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFGrassTaken))
		_vm->_hotspots[kHS17Wrench]._flags = SF_NONE;
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_vm->_hotspots[kHS17Device]._flags = SF_DISABLED;
		_vm->_hotspots[kHS17Platypus]._flags = SF_DISABLED;
	}
	_vm->_hotspotsCount = 10;
}

void Scene17::update() {
	_vm->gameUpdateTick();
	_vm->updateMouseCursor();
	_vm->updateGrabCursorSprite(0, 0);
	if (_vm->_mouseClickState._left) {
		_vm->_gnap->walkTo(Common::Point(-1, -1), -1, -1, 1);
		_vm->_mouseClickState._left = false;
	}
}

void Scene17::platHangUpPhone() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	int savedGnapActionStatus = gnap._actionStatus;

	if (plat._actionStatus == kAS17PlatPhoningAssistant) {
		gnap._actionStatus = kAS17PlatHangUpPhone;
		_vm->updateMouseCursor();
		_platPhoneCtr = 0;
		plat._actionStatus = -1;
		gameSys.setAnimation(0x257, 254, 4);
		gameSys.insertSequence(0x257, 254, _currPhoneSequenceId, 254, kSeqSyncExists, 0, 0, 0);
		while (gameSys.getAnimationStatus(4) != 2 && !_vm->_gameDone)
			_vm->gameUpdateTick();
		gameSys.setAnimation(0x25B, plat._id, 1);
		gameSys.insertSequence(0x25B, plat._id, plat._sequenceId | (plat._sequenceDatNum << 16), plat._id, kSeqSyncWait, 0, 0, 0);
		plat._sequenceId = 0x25B;
		plat._sequenceDatNum = 0;
		_currPhoneSequenceId = -1;
		_nextPhoneSequenceId = -1;
		_vm->clearFlag(kGFPlatypusTalkingToAssistant);
		while (gameSys.getAnimationStatus(1) != 2 && !_vm->_gameDone)
			_vm->gameUpdateTick();
		gnap._actionStatus = savedGnapActionStatus;
		_vm->updateMouseCursor();
	}
	updateHotspots();
}

void Scene17::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(8);
	_vm->_sceneWaiting = false;
	_vm->_timers[4] = _vm->getRandom(100) + 200;
	_vm->_timers[3] = 200;
	_vm->_timers[5] = _vm->getRandom(30) + 80;
	_vm->_timers[6] = _vm->getRandom(30) + 200;
	_vm->_timers[7] = _vm->getRandom(100) + 100;

	if (_vm->isFlag(kGFTruckKeysUsed)) {
		gameSys.insertSequence(0x25F, 20, 0, 0, kSeqNone, 0, 0, 0);
	} else {
		if (_vm->_s18GarbageCanPos >= 8) {
			gameSys.insertSequence(0x260, 20, 0, 0, kSeqNone, 0, 97, 1);
		} else if (_vm->_s18GarbageCanPos >= 6) {
			gameSys.insertSequence(0x260, 20, 0, 0, kSeqNone, 0, 68, 2);
		} else if (_vm->_s18GarbageCanPos >= 5) {
			gameSys.insertSequence(0x260, 20, 0, 0, kSeqNone, 0, 23, -1);
		} else if (_vm->_s18GarbageCanPos >= 4) {
			gameSys.insertSequence(0x260, 20, 0, 0, kSeqNone, 0, -11, -5);
		} else {
			gameSys.insertSequence(0x260, 20, 0, 0, kSeqNone, 0, -54, -8);
		}
	}

	if (_vm->isFlag(kGFGroceryStoreHatTaken))
		gameSys.insertSequence(0x262, 1, 0, 0, kSeqNone, 0, 0, 0);

	_vm->queueInsertDeviceIcon();

	if (_vm->isFlag(kGFGrassTaken))
		_currWrenchSequenceId = 0x22D;
	else
		_currWrenchSequenceId = 0x22F;

	_currCarWindowSequenceId = 0x244;

	if (_vm->isFlag(kGFUnk14))
		gameSys.insertSequence(0x261, 1, 0, 0, kSeqNone, 0, 0, 0);

	gameSys.setAnimation(_currWrenchSequenceId, 40, 2);
	gameSys.insertSequence(_currWrenchSequenceId, 40, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFGrassTaken)) {
		gameSys.setAnimation(0, 0, 3);
	} else {
		gameSys.setAnimation(_currCarWindowSequenceId, 40, 3);
		gameSys.insertSequence(_currCarWindowSequenceId, 40, 0, 0, kSeqNone, 0, 0, 0);
	}

	_canTryGetWrench = true;

	if (_vm->isFlag(kGFUnk18))
		gameSys.insertSequence(0x24F, 100, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->_prevSceneNum == 53 || _vm->_prevSceneNum == 18 || _vm->_prevSceneNum == 20 || _vm->_prevSceneNum == 19) {
		if (_vm->_prevSceneNum == 20) {
			gnap.initPos(4, 6, kDirBottomRight);
			plat.initPos(5, 6, kDirIdleLeft);
			_vm->endSceneInit();
			plat.walkTo(Common::Point(5, 9), -1, 0x107C2, 1);
			gnap.walkTo(Common::Point(4, 8), -1, 0x107B9, 1);
		} else if (_vm->isFlag(kGFUnk27)) {
			gnap.initPos(3, 9, kDirUpLeft);
			plat._pos = _vm->_hotspotsWalkPos[2];
			plat._id = 20 * _vm->_hotspotsWalkPos[2].y;
			gameSys.insertSequence(0x25A, 20 * _vm->_hotspotsWalkPos[2].y, 0, 0, kSeqNone, 0, 0, 0);
			gameSys.insertSequence(0x257, 254, 0, 0, kSeqNone, 0, 0, 0);
			plat._sequenceId = 0x25A;
			plat._sequenceDatNum = 0;
			_vm->endSceneInit();
			_vm->clearFlag(kGFSpringTaken);
			_vm->clearFlag(kGFUnk16);
			plat._actionStatus = kAS17PlatPhoningAssistant;
			platHangUpPhone();
			gameSys.setAnimation(0, 0, 4);
			_vm->clearFlag(kGFPlatypusTalkingToAssistant);
			_vm->clearFlag(kGFUnk27);
			updateHotspots();
		} else if (_vm->isFlag(kGFUnk25)) {
			_vm->clearFlag(kGFSpringTaken);
			_vm->clearFlag(kGFUnk16);
			plat.initPos(7, 9, kDirIdleLeft);
			gnap._pos = _vm->_hotspotsWalkPos[2];
			gnap._id = 20 * _vm->_hotspotsWalkPos[2].y;
			gameSys.insertSequence(601, 20 * _vm->_hotspotsWalkPos[2].y, 0, 0, kSeqNone, 0, 0, 0);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 601;
			gnap._actionStatus = kAS17GnapHangUpPhone;
			_vm->clearFlag(kGFUnk25);
			gameSys.insertSequence(0x251, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
			gameSys.setAnimation(0x257, 254, 0);
			gameSys.insertSequence(0x257, 254, 0x251, 254, kSeqSyncWait, 0, 0, 0);
		} else if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
			_vm->clearFlag(kGFSpringTaken);
			_vm->clearFlag(kGFUnk16);
			_vm->_sceneWaiting = true;
			gnap.initPos(3, 9, kDirUpLeft);
			plat._pos = _vm->_hotspotsWalkPos[2];
			plat._id = 20 * _vm->_hotspotsWalkPos[2].y;
			_currPhoneSequenceId = 0x251;
			gameSys.insertSequence(0x25A, 20 * _vm->_hotspotsWalkPos[2].y, 0, 0, kSeqNone, 0, 0, 0);
			gameSys.insertSequence(_currPhoneSequenceId, 254, 0, 0, kSeqNone, 0, 0, 0);
			plat._sequenceId = 0x25A;
			plat._sequenceDatNum = 0;
			_vm->endSceneInit();
			gameSys.setAnimation(_currPhoneSequenceId, 254, 1);
			plat._actionStatus = kAS17PlatPhoningAssistant;
			updateHotspots();
		} else if (_vm->_prevSceneNum == 18) {
			gnap.initPos(6, 6, kDirBottomRight);
			plat.initPos(5, 6, kDirIdleLeft);
			_vm->endSceneInit();
			plat.walkTo(Common::Point(5, 9), -1, 0x107C2, 1);
			gnap.walkTo(Common::Point(4, 8), -1, 0x107B9, 1);
		} else {
			if (_vm->isFlag(kGFSpringTaken)) {
				gnap.initPos(_vm->_hotspotsWalkPos[2].x, _vm->_hotspotsWalkPos[2].y, kDirBottomRight);
				plat.initPos(1, 9, kDirIdleLeft);
				_vm->endSceneInit();
			} else {
				gnap.initPos(3, 7, kDirBottomRight);
				plat.initPos(1, 7, kDirIdleLeft);
				_vm->endSceneInit();
			}
			_vm->clearFlag(kGFSpringTaken);
			_vm->clearFlag(kGFUnk16);
			_vm->endSceneInit();
		}
	} else {
		gnap._pos = Common::Point(3, 6);
		gnap._id = 120;
		gnap._sequenceId = 0x23D;
		gnap._sequenceDatNum = 0;
		gnap._idleFacing = kDirBottomRight;
		gameSys.insertSequence(makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, 0, 0, kSeqNone, 0, 0, 0);
		plat._pos = Common::Point(-1, 8);
		plat._id = 160;
		gameSys.insertSequence(0x241, 160, 0, 0, kSeqNone, 0, 0, 0);
		gameSys.insertSequence(0x107C1, plat._id, 0x241, plat._id,
			kSeqScale | kSeqSyncWait, 0, 75 * plat._pos.x - plat._gridX, 48 * plat._pos.y - plat._gridY);
		gameSys.insertSequence(0x22C, 2, 0, 0, kSeqNone, 0, 0, 0);
		_vm->delayTicksA(2, 9);
		_vm->endSceneInit();
		plat._sequenceId = 0x7C1;
		plat._sequenceDatNum = 1;
		plat._idleFacing = kDirBottomRight;
		plat.walkTo(Common::Point(2, 9), -1, 0x107C2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS17Device:
			if (gnap._actionStatus < 0 || gnap._actionStatus == 3) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS17Platypus:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					if (_vm->isFlag(kGFGrassTaken)) {
						gnap.useJointOnPlatypus();
					} else {
						gnap.useDeviceOnPlatypus();
						plat.walkTo(_vm->_hotspotsWalkPos[6], 1, 0x107C2, 1);
						gnap.walkTo(_vm->_hotspotsWalkPos[6] + Common::Point(1, 0), 0, 0x107BA, 1);
						plat._actionStatus = kAS17GetWrench1;
						gnap._actionStatus = kAS17GetWrench1;
						_vm->_timers[5] = _vm->getRandom(30) + 80;
						_vm->setGrabCursorSprite(-1);
						_vm->invRemove(kItemJoint);
					}
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playScratchingHead(plat._pos);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
					case GRAB_CURSOR:
						gnap.playScratchingHead(plat._pos);
						break;
					case TALK_CURSOR:
						gnap.playBrainPulsating(plat._pos);
						plat.playSequence(plat.getSequenceId());
						break;
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS17Wrench:
			if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFGrassTaken)) {
					gnap.playImpossible();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 8, 7);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
					case GRAB_CURSOR:
						gnap.playScratchingHead(Common::Point(8, 7));
						break;
					case TALK_CURSOR:
						gnap.playImpossible();
						break;
					case PLAT_CURSOR:
						if (_canTryGetWrench) {
							platHangUpPhone();
							gnap.useDeviceOnPlatypus();
							plat.walkTo(_vm->_hotspotsWalkPos[6] + Common::Point(1, 0), 1, 0x107C2, 1);
							plat._actionStatus = kAS17TryGetWrench;
							gnap._actionStatus = kAS17TryGetWrench;
							_vm->_timers[5] = _vm->getRandom(30) + 80;
						} else
							gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS17Phone1:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					gnap.walkTo(_vm->_hotspotsWalkPos[2], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1);
					gnap._actionStatus = kAS17PutCoinIntoPhone;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[2], 1, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(1, 3));
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFUnk18)) {
							platHangUpPhone();
							gnap.walkTo(gnap._pos, 0, gnap.getSequenceId(kGSIdle, _vm->_hotspotsWalkPos[2]) | 0x10000, 1);
							gnap._actionStatus = kAS17GetCoinFromPhone;
						} else
							gnap.playImpossible();
						break;
					case TALK_CURSOR:
						gnap.playImpossible();
						break;
					case PLAT_CURSOR:
						if (_vm->isFlag(kGFUnk18)) {
							platHangUpPhone();
							_vm->_isLeavingScene = true;
							gnap.useDeviceOnPlatypus();
							plat._idleFacing = kDirUpLeft;
							plat.walkTo(_vm->_hotspotsWalkPos[2], 1, 0x107C2, 1);
							_vm->setFlag(kGFUnk16);
							plat._actionStatus = kAS17PlatUsePhone;
							gnap._actionStatus = kAS17PlatUsePhone;
						} else
							gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS17Phone2:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemDiceQuarterHole) {
					gnap.walkTo(_vm->_hotspotsWalkPos[2], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1);
					gnap._actionStatus = kAS17PutCoinIntoPhone;
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[2], 1, 3);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(1, 3));
						break;
					case GRAB_CURSOR:
						if (_vm->isFlag(kGFUnk18)) {
							platHangUpPhone();
							_vm->_isLeavingScene = true;
							gnap._idleFacing = kDirUpLeft;
							gnap.walkTo(_vm->_hotspotsWalkPos[2], 0, gnap.getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, 1);
							gnap._actionStatus = kAS17GnapUsePhone;
							_vm->setFlag(kGFSpringTaken);
						} else
							gnap.playImpossible();
						break;
					case TALK_CURSOR:
						gnap.playImpossible();
						break;
					case PLAT_CURSOR:
						if (_vm->isFlag(kGFUnk18)) {
							platHangUpPhone();
							_vm->_isLeavingScene = true;
							gnap.useDeviceOnPlatypus();
							plat._idleFacing = kDirUpLeft;
							plat.walkTo(_vm->_hotspotsWalkPos[2], 1, 0x107C2, 1);
							_vm->setFlag(kGFUnk16);
							plat._actionStatus = kAS17PlatUsePhone;
							gnap._actionStatus = kAS17PlatUsePhone;
						} else
							gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS17ExitToyStore:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 18;
				gnap._idleFacing = kDirUpRight;
				gnap.walkTo(_vm->_hotspotsWalkPos[5], 0, 0x107BB, 1);
				gnap._actionStatus = kAS17LeaveScene;
				if (plat._actionStatus != kAS17PlatPhoningAssistant)
					plat.walkTo(_vm->_hotspotsWalkPos[5] + Common::Point(-1, 0), -1, 0x107C2, 1);
			}
			break;

		case kHS17ExitGrubCity:
			if (gnap._actionStatus < 0) {
				platHangUpPhone();
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 20;
				gnap._idleFacing = kDirUpLeft;
				gnap.walkTo(_vm->_hotspotsWalkPos[3], 0, 0x107BC, 1);
				gnap._actionStatus = kAS17LeaveScene;
				plat.walkTo(_vm->_hotspotsWalkPos[3] + Common::Point(1, 0), -1, 0x107C2, 1);
			}
			break;

		case kHS17WalkArea1:
		case kHS17WalkArea2:
		case kHS17WalkArea3:
			if (gnap._actionStatus < 0)
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left && gnap._actionStatus < 0) {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = 0;
			}
			break;
		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x10940))
			_vm->playSound(0x10940, true);

		if (!_vm->_isLeavingScene) {
			if (plat._actionStatus < 0)
				plat.updateIdleSequence2();
			gnap.updateIdleSequence2();
			if (!_vm->_timers[4]) {
				_vm->_timers[4] = _vm->getRandom(100) + 200;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0)
					gameSys.insertSequence(0x22B, 21, 0, 0, kSeqNone, 0, 0, 0);
			}
			if (!_vm->_timers[7]) {
				_vm->_timers[7] = _vm->getRandom(100) + 100;
				if (gnap._actionStatus < 0 && plat._actionStatus < 0) {
					switch (_vm->getRandom(3)) {
					case 0:
						gameSys.insertSequence(0x25C, 255, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 1:
						gameSys.insertSequence(0x25D, 255, 0, 0, kSeqNone, 0, 0, 0);
						break;
					case 2:
						gameSys.insertSequence(0x25E, 255, 0, 0, kSeqNone, 0, 0, 0);
						break;
					default:
						break;
					}
				}
			}
			if (plat._actionStatus < 0 && !_vm->_timers[5]) {
				_vm->_timers[5] = _vm->getRandom(30) + 80;
				if (_vm->isFlag(kGFGrassTaken) && _nextWrenchSequenceId == -1) {
					_nextWrenchSequenceId = 0x236;
				} else if (_canTryGetWrench) {
					switch (_vm->getRandom(6)) {
					case 0:
						_nextWrenchSequenceId = 0x231;
						break;
					case 1:
						_nextWrenchSequenceId = 0x232;
						break;
					case 2:
					case 3:
						_nextWrenchSequenceId = 0x23C;
						break;
					case 4:
					case 5:
						_nextWrenchSequenceId = 0x22E;
						break;
					default:
						break;
					}
				} else {
					--_wrenchCtr;
					if (_wrenchCtr) {
						switch (_vm->getRandom(6)) {
						case 0:
							_nextWrenchSequenceId = 0x237;
							break;
						case 1:
							_nextWrenchSequenceId = 0x238;
							break;
						case 2:
							_nextWrenchSequenceId = 0x239;
							break;
						case 3:
							_nextWrenchSequenceId = 0x23A;
							break;
						case 4:
							_nextWrenchSequenceId = 0x23B;
							break;
						case 5:
							_nextWrenchSequenceId = 0x235;
							break;
						default:
							break;
						}
					} else {
						_wrenchCtr = 2;
						_nextWrenchSequenceId = 0x235;
					}
				}
			}
			if (!_vm->_timers[6]) {
				_vm->_timers[6] = _vm->getRandom(30) + 200;
				if (_nextCarWindowSequenceId == -1 && !_vm->isFlag(kGFGrassTaken))
					_nextCarWindowSequenceId = 0x246;
			}
			_vm->playSoundA();
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}
}

void Scene17::updateAnimations() {
	static const int kPlatPhoneSequenceIds[] = {
		0x251, 0x252, 0x253, 0x254, 0x255, 0x256, 0x257
	};

	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS17GetWrench1:
			gnap._actionStatus = kAS17GetWrenchGnapReady;
			break;
		case kAS17GetCoinFromPhone:
			gnap.playPullOutDevice(Common::Point(1, 3));
			gnap.playUseDevice();
			gameSys.setAnimation(0x250, 100, 0);
			gameSys.insertSequence(0x250, 100, 591, 100, kSeqSyncWait, 0, 0, 0);
			_vm->invAdd(kItemDiceQuarterHole);
			_vm->clearFlag(kGFUnk18);
			gnap._actionStatus = kAS17GetCoinFromPhoneDone;
			break;
		case kAS17GetCoinFromPhoneDone:
			_vm->setGrabCursorSprite(kItemDiceQuarterHole);
			gnap._actionStatus = -1;
			break;
		case kAS17PutCoinIntoPhone:
			gameSys.setAnimation(0x24C, gnap._id, 0);
			gameSys.insertSequence(0x24C, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x24C;
			_vm->invRemove(kItemDiceQuarterHole);
			_vm->setGrabCursorSprite(-1);
			_vm->setFlag(kGFUnk18);
			gnap._actionStatus = kAS17PutCoinIntoPhoneDone;
			break;
		case kAS17PutCoinIntoPhoneDone:
			gameSys.insertSequence(0x24F, 100, 0, 0, kSeqNone, 0, 0, 0);
			gnap._actionStatus = -1;
			break;
		case kAS17GnapUsePhone:
			gameSys.setAnimation(0x24D, gnap._id, 0);
			gameSys.insertSequence(0x24D, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._actionStatus = kAS17LeaveScene;
			_vm->_newSceneNum = 53;
			break;
		case kAS17GnapHangUpPhone:
			gameSys.insertSequence(0x258, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x258;
			gnap._actionStatus = -1;
			break;
		case kAS17LeaveScene:
			_vm->_sceneDone = true;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(1) == 2) {
		gameSys.setAnimation(0, 0, 1);
		switch (plat._actionStatus) {
		case kAS17TryGetWrench:
			plat._actionStatus = -1;
			++_platTryGetWrenchCtr;
			if (_platTryGetWrenchCtr % 2 != 0)
				_nextWrenchSequenceId = 0x233;
			else
				_nextWrenchSequenceId = 0x234;
			_canTryGetWrench = false;
			break;
		case kAS17GetWrench1:
			_nextWrenchSequenceId = 0x230;
			break;
		case kAS17GetWrench2:
			_nextCarWindowSequenceId = 0x249;
			break;
		case kAS17GetWrenchDone:
			plat._actionStatus = -1;
			_vm->invAdd(kItemWrench);
			_vm->setGrabCursorSprite(kItemWrench);
			break;
		case kAS17PlatUsePhone:
			gameSys.setAnimation(0x24E, plat._id, 1);
			gameSys.insertSequence(0x24E, plat._id, plat._sequenceId | (plat._sequenceDatNum << 16), plat._id, kSeqSyncWait, 0, 0, 0);
			plat._sequenceDatNum = 0;
			plat._sequenceId = 0x24E;
			plat._actionStatus = kAS17LeaveScene;
			_vm->_newSceneNum = 53;
			break;
		case kAS17PlatPhoningAssistant:
			++_platPhoneCtr;
			if (_platPhoneCtr >= 7) {
				_platPhoneCtr = 0;
				_nextPhoneSequenceId = -1;
				_currPhoneSequenceId = -1;
				gameSys.insertSequence(0x25B, plat._id, 0x25A, plat._id, kSeqSyncWait, 0, 0, 0);
				plat._sequenceDatNum = 0;
				plat._sequenceId = 0x25B;
				plat._actionStatus = -1;
				_vm->clearFlag(kGFPlatypusTalkingToAssistant);
				_vm->_sceneWaiting = false;
				updateHotspots();
			} else {
				_nextPhoneSequenceId = kPlatPhoneSequenceIds[_platPhoneCtr];
				gameSys.setAnimation(_nextPhoneSequenceId, 254, 1);
				gameSys.insertSequence(_nextPhoneSequenceId, 254, _currPhoneSequenceId, 254, kSeqSyncWait, 0, 0, 0);
				gameSys.insertSequence(0x25A, plat._id, 0x25A, plat._id, kSeqSyncWait, 0, 0, 0);
				plat._sequenceDatNum = 0;
				plat._sequenceId = 0x25A;
				_currPhoneSequenceId = _nextPhoneSequenceId;
			}
			break;
		case kAS17LeaveScene:
			_vm->_sceneDone = true;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(2) == 2) {
		switch (_nextWrenchSequenceId) {
		case 0x233:
			gnap._actionStatus = -1;
			gameSys.insertSequence(0x243, plat._id,
				plat._sequenceId | (plat._sequenceDatNum << 16), plat._id,
				kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(_nextWrenchSequenceId, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_currWrenchSequenceId = _nextWrenchSequenceId;
			_nextWrenchSequenceId = -1;
			plat._sequenceId = 0x243;
			plat._sequenceDatNum = 0;
			gameSys.setAnimation(0x243, plat._id, 1);
			break;
		case 0x234:
			gnap._actionStatus = -1;
			gameSys.insertSequence(0x242, plat._id,
				plat._sequenceId | (plat._sequenceDatNum << 16), plat._id,
				kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(_nextWrenchSequenceId, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_currWrenchSequenceId = _nextWrenchSequenceId;
			_nextWrenchSequenceId = -1;
			plat._sequenceId = 0x242;
			plat._sequenceDatNum = 0;
			gameSys.setAnimation(0x242, plat._id, 1);
			break;
		case 0x231:
			if (_vm->getRandom(2) != 0)
				_nextCarWindowSequenceId = 0x245;
			else
				_nextCarWindowSequenceId = 0x248;
			gameSys.setAnimation(0, 0, 2);
			break;
		case 0x232:
			_nextCarWindowSequenceId = 0x247;
			gameSys.setAnimation(0, 0, 2);
			break;
		case 0x22E:
		case 0x235:
			if (_nextWrenchSequenceId == 0x235)
				_vm->_hotspots[kHS17Wrench]._flags &= ~SF_DISABLED;
			else
				_vm->_hotspots[kHS17Wrench]._flags |= SF_DISABLED;
			_canTryGetWrench = !_canTryGetWrench;
			gameSys.setAnimation(_nextWrenchSequenceId, 40, 2);
			gameSys.insertSequence(_nextWrenchSequenceId, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_currWrenchSequenceId = _nextWrenchSequenceId;
			_nextWrenchSequenceId = -1;
			break;
		case 0x230:
			if (gnap._actionStatus == kAS17GetWrenchGnapReady) {
				gameSys.setAnimation(0, 0, 2);
				if (_canTryGetWrench) {
					gameSys.insertSequence(0x22E, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
					_currWrenchSequenceId = 0x22E;
					_canTryGetWrench = false;
				}
				gameSys.setAnimation(0x23F, plat._id, 1);
				gameSys.insertSequence(0x10875, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
				gameSys.insertSequence(0x23F, plat._id,
					plat._sequenceId | (plat._sequenceDatNum << 16), plat._id,
					kSeqSyncWait, 0, 0, 0);
				gnap._sequenceDatNum = 1;
				plat._sequenceDatNum = 0;
				gnap._sequenceId = 0x875;
				plat._sequenceId = 0x23F;
				gnap.walkTo(Common::Point(3, 8), -1, 0x107B9, 1);
				plat._actionStatus = kAS17GetWrench2;
			}
			break;
		default:
			if (_nextWrenchSequenceId != -1) {
				gameSys.setAnimation(_nextWrenchSequenceId, 40, 2);
				gameSys.insertSequence(_nextWrenchSequenceId, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
				_currWrenchSequenceId = _nextWrenchSequenceId;
				_nextWrenchSequenceId = -1;
			}
			break;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		switch (_nextCarWindowSequenceId) {
		case 0x246:
			gameSys.setAnimation(_nextCarWindowSequenceId, 40, 3);
			gameSys.insertSequence(_nextCarWindowSequenceId, 40, _currCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_currCarWindowSequenceId = _nextCarWindowSequenceId;
			_nextCarWindowSequenceId = -1;
			break;
		case 0x245:
		case 0x247:
		case 0x248:
			gameSys.setAnimation(_nextWrenchSequenceId, 40, 2);
			gameSys.insertSequence(_nextWrenchSequenceId, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			while (gameSys.getAnimationStatus(2) != 2)
				update();
			gameSys.setAnimation(_nextCarWindowSequenceId, 40, 3);
			gameSys.insertSequence(_nextCarWindowSequenceId, 40, _currCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			_currCarWindowSequenceId = _nextCarWindowSequenceId;
			_nextCarWindowSequenceId = -1;
			_currWrenchSequenceId = _nextWrenchSequenceId;
			_nextWrenchSequenceId = -1;
			break;
		case 0x249:
			gameSys.setAnimation(0x230, 40, 2);
			gameSys.setAnimation(0x240, plat._id, 1);
			gameSys.insertSequence(0x230, 40, _currWrenchSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(_nextCarWindowSequenceId, 40, _currCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x240, plat._id, plat._sequenceId, plat._id, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x23E, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceId = 0x23E;
			gnap._sequenceDatNum = 0;
			plat._sequenceId = 0x240;
			plat._sequenceDatNum = 0;
			gameSys.setAnimation(0x24A, 40, 3);
			gameSys.insertSequence(0x24A, 40, _nextCarWindowSequenceId, 40, kSeqSyncWait, 0, 0, 0);
			while (gameSys.getAnimationStatus(2) != 2) {
				update();
				if (gameSys.getAnimationStatus(3) == 2) {
					gameSys.setAnimation(0x24A, 40, 3);
					gameSys.insertSequence(0x24A, 40, 586, 40, kSeqSyncWait, 0, 0, 0);
				}
			}
			gameSys.insertSequence(0x22D, 40, 560, 40, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(0x24B, 40, 3);
			gameSys.insertSequence(0x24B, 40, 586, 40, kSeqSyncWait, 0, 0, 0);
			_currCarWindowSequenceId = 0x24B;
			_nextCarWindowSequenceId = -1;
			_currWrenchSequenceId = 0x22D;
			_nextWrenchSequenceId = -1;
			_vm->setFlag(kGFGrassTaken);
			gnap._actionStatus = -1;
			plat._actionStatus = 2;
			updateHotspots();
			_vm->_timers[5] = _vm->getRandom(30) + 80;
			break;
		default:
			break;
		}
	}

}

/*****************************************************************************/

static const int kScene18SequenceIds[] = {
	0x219,  0x21A,  0x21B,  0x21C,  0x21D
};

Scene18::Scene18(GnapEngine *vm) : Scene(vm) {
	_cowboyHatSurface = nullptr;

	_platPhoneCtr = 0;
	_platPhoneIter = 0;
	_nextPhoneSequenceId = -1;
	_currPhoneSequenceId = -1;
}

Scene18::~Scene18() {
	delete _cowboyHatSurface;
}

int Scene18::init() {
	_vm->_gameSys->setAnimation(0, 0, 3);
	return 0x222;
}

void Scene18::updateHotspots() {
	_vm->setHotspot(kHS18Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS18GarbageCan, _vm->_gridMinX + 75 * _vm->_s18GarbageCanPos - 35, _vm->_gridMinY + 230, _vm->_gridMinX + 75 * _vm->_s18GarbageCanPos + 35, _vm->_gridMinY + 318,
		SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, _vm->_s18GarbageCanPos, 7);
	_vm->setHotspot(kHS18ExitToyStore, 460, 238, 592, 442, SF_EXIT_U_CURSOR, 7, 7);
	_vm->setHotspot(kHS18ExitPhoneBooth, 275, 585, 525, 600, SF_EXIT_D_CURSOR | SF_WALKABLE, 5, 10);
	_vm->setHotspot(kHS18ExitGrubCity, 0, 350, 15, 600, SF_EXIT_L_CURSOR, 0, 9);
	_vm->setHotspot(kHS18HydrantTopValve, 100, 345, 182, 410, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 8);
	_vm->setHotspot(kHS18HydrantRightValve, 168, 423, 224, 470, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 7);
	_vm->setHotspot(kHS18CowboyHat, 184, 63, 289, 171, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 4, 7);
	_vm->setHotspot(kHS18WalkArea1, 0, 0, 800, 448);
	_vm->setHotspot(kHS18WalkArea2, 0, 0, 214, 515);
	_vm->setDeviceHotspot(kHS18Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFTruckFilledWithGas)) {
		if (_vm->isFlag(kGFTruckKeysUsed)) {
			_vm->_hotspots[kHS18HydrantTopValve]._flags = SF_DISABLED;
			_vm->_hotspots[kHS18HydrantRightValve]._rect.left = 148;
			_vm->_hotspots[kHS18HydrantRightValve]._rect.top = 403;
			_vm->_hotspots[kHS18GarbageCan]._flags = SF_DISABLED;
			_vm->_hotspotsWalkPos[kHS18GarbageCan] = Common::Point(3, 7);
		} else {
			_vm->_hotspots[kHS18HydrantTopValve]._rect.top = 246;
		}
	} else if (_vm->isFlag(kGFBarnPadlockOpen)) {
		_vm->_hotspots[kHS18HydrantRightValve]._flags = SF_DISABLED;
		_vm->_hotspots[kHS18HydrantTopValve]._rect.left = 105;
		_vm->_hotspots[kHS18HydrantTopValve]._rect.right = 192;
	} else if (_vm->isFlag(kGFTruckKeysUsed)) {
		_vm->_hotspots[kHS18GarbageCan]._rect = Common::Rect(115, 365, 168, 470);
		_vm->_hotspots[kHS18GarbageCan]._flags = SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
		_vm->_hotspotsWalkPos[kHS18GarbageCan] = Common::Point(3, 7);
	}
	if (_vm->isFlag(kGFPlatypusDisguised))
		_vm->_hotspots[kHS18GarbageCan]._flags = SF_DISABLED;
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_vm->_hotspots[kHS18Device]._flags = SF_DISABLED;
		_vm->_hotspots[kHS18HydrantTopValve]._flags = SF_DISABLED;
		_vm->_hotspots[kHS18HydrantRightValve]._flags = SF_DISABLED;
		_vm->_hotspots[kHS18Platypus]._flags = SF_DISABLED;
	}
	if (_vm->isFlag(kGFUnk14)) {
		_vm->_hotspots[kHS18HydrantTopValve]._flags = SF_DISABLED;
		_vm->_hotspots[kHS18CowboyHat]._flags = SF_DISABLED;
	}
	_vm->_hotspotsCount = 11;
}

void Scene18::gnapCarryGarbageCanTo(int gridX) {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	int gnapSeqId, gnapId, gnapDatNum, gnapGridX;
	int destGridX, direction;

	int curGridX = (_vm->_leftClickMouseX - _vm->_gridMinX + 37) / 75;

	if (curGridX >= gnap._pos.x)
		destGridX = curGridX - 1;
	else
		destGridX = curGridX + 1;

	if (gridX < 0)
		gridX = 4;

	if (destGridX <= gridX)
		destGridX = gridX;

	int nextGridX = _vm->_gridMaxX - 1;
	if (nextGridX >= destGridX)
		nextGridX = destGridX;

	if (nextGridX == gnap._pos.x) {
		gnapSeqId = gnap._sequenceId;
		gnapId = gnap._id;
		gnapDatNum = gnap._sequenceDatNum;
		gnapGridX = gnap._pos.x;
		if (gnap._pos.x <= curGridX)
			direction = 1;
		else
			direction = -1;
	} else {
		PlayerPlat& plat = *_vm->_plat;
		if (gnap._pos.y == plat._pos.y) {
			if (nextGridX >= gnap._pos.x) {
				if (nextGridX >= plat._pos.x && gnap._pos.x <= plat._pos.x)
					plat.makeRoom();
			} else if (nextGridX <= plat._pos.x && gnap._pos.x >= plat._pos.x) {
				plat.makeRoom();
			}
		}
		gnapSeqId = gnap._sequenceId;
		gnapId = gnap._id;
		gnapDatNum = gnap._sequenceDatNum;
		gnapGridX = gnap._pos.x;
		int seqId;
		if (nextGridX < gnap._pos.x) {
			direction = -1;
			seqId = 0x204;
		} else {
			direction = 1;
			seqId = 0x203;
		}

		int seqId2 = 20 * gnap._pos.y + 1;
		do {
			if (_vm->isPointBlocked(gnapGridX + direction, gnap._pos.y))
				break;
			seqId2 += direction;
			gameSys.insertSequence(seqId, seqId2,
				gnapSeqId | (gnapDatNum << 16), gnapId,
				kSeqSyncWait, 0, 75 * gnapGridX - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
			gnapSeqId = seqId;
			gnapId = seqId2;
			gnapDatNum = 0;
			gnapGridX += direction;
		} while (nextGridX != gnapGridX);
	}

	if (direction == 1)
		gnap._sequenceId = 0x20A;
	else
		gnap._sequenceId = 0x209;
	gnap._sequenceDatNum = 0;

	if (direction == 1)
		gnap._idleFacing = kDirBottomRight;
	else
		gnap._idleFacing = kDirBottomLeft;

	gnap._id = 20 * gnap._pos.y + 1;

	gameSys.setAnimation(makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, 0);
	gameSys.insertSequence(makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
		gnapSeqId | (gnapDatNum << 16), gnapId,
		kSeqScale | kSeqSyncWait, 0, 75 * gnapGridX - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);

	gnap._pos.x = gnapGridX;
}

void Scene18::putDownGarbageCan(int animationIndex) {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (animationIndex >= 0) {
		while (gameSys.getAnimationStatus(animationIndex) != 2 && !_vm->_gameDone)
			_vm->gameUpdateTick();
	}
	if (gnap._idleFacing != kDirIdleLeft && gnap._idleFacing != kDirBottomRight && gnap._idleFacing != kDirUpRight)
		_vm->_s18GarbageCanPos = gnap._pos.x - 1;
	else
		_vm->_s18GarbageCanPos = gnap._pos.x + 1;
	_vm->clearFlag(kGFPlatypusDisguised);
	updateHotspots();
	if (gnap._idleFacing != kDirIdleLeft && gnap._idleFacing != kDirBottomRight && gnap._idleFacing != kDirUpRight) {
		gameSys.insertSequence(0x107BA, gnap._id,
			makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
			kSeqSyncWait, 0, 75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
		gnap._sequenceId = 0x7BA;
	} else {
		gameSys.insertSequence(0x107B9, gnap._id,
			makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
			kSeqSyncWait, 0, 75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
		gnap._sequenceId = 0x7B9;
	}
	gnap._sequenceDatNum = 1;
	gameSys.insertSequence(0x1FB, 19, 0, 0, kSeqNone, 0, 15 * (5 * _vm->_s18GarbageCanPos - 40), 0);
	gameSys.setAnimation(0x1FA, 19, 4);
	gameSys.insertSequence(0x1FA, 19, 507, 19, kSeqSyncWait, 0, 15 * (5 * _vm->_s18GarbageCanPos - 40), 0);
	while (gameSys.getAnimationStatus(4) != 2 && !_vm->_gameDone)
		_vm->gameUpdateTick();
}

void Scene18::platEndPhoning(bool platFl) {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerPlat& plat = *_vm->_plat;

	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_platPhoneIter = 0;
		_platPhoneCtr = 0;
		plat._actionStatus = -1;
		if (_currPhoneSequenceId != -1) {
			gameSys.setAnimation(0x21E, 254, 3);
			gameSys.insertSequence(0x21E, 254, _currPhoneSequenceId, 254, kSeqSyncExists, 0, 0, 0);
			while (gameSys.getAnimationStatus(3) != 2 && !_vm->_gameDone)
				_vm->gameUpdateTick();
		}
		gameSys.removeSequence(0x21F, 254, true);
		gameSys.setAnimation(0, 0, 3);
		_vm->clearFlag(kGFPlatypusTalkingToAssistant);
		if (platFl) {
			plat._actionStatus = kAS18PlatComesHere;
			_vm->_timers[6] = 50;
			_vm->_sceneWaiting = true;
		}
		_currPhoneSequenceId = -1;
		_nextPhoneSequenceId = -1;
		updateHotspots();
	}
}

void Scene18::closeHydrantValve() {
	PlayerGnap& gnap = *_vm->_gnap;

	gnap._actionStatus = kAS18LeaveScene;
	_vm->updateMouseCursor();
	if (_vm->isFlag(kGFTruckFilledWithGas)) {
		gnap.walkTo(_vm->_hotspotsWalkPos[kHS18HydrantRightValve], 0, 0x107BA, 1);
		if (_vm->isFlag(kGFTruckKeysUsed)) {
			gnap._actionStatus = kAS18CloseRightValveWithGarbageCan;
			waitForGnapAction();
		} else {
			gnap._actionStatus = kAS18CloseRightValveNoGarbageCan;
			waitForGnapAction();
		}
	} else if (_vm->isFlag(kGFBarnPadlockOpen)) {
		gnap.walkTo(_vm->_hotspotsWalkPos[kHS18HydrantTopValve], 0, 0x107BA, 1);
		gnap._actionStatus = kAS18CloseTopValve;
		waitForGnapAction();
	}
}

void Scene18::waitForGnapAction() {
	PlayerGnap& gnap = *_vm->_gnap;

	while (gnap._actionStatus >= 0 && !_vm->_gameDone) {
		updateAnimations();
		_vm->gameUpdateTick();
	}
}

void Scene18::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_cowboyHatSurface = nullptr;

	_vm->playSound(0x10940, true);
	_vm->startSoundTimerA(4);
	_vm->_timers[5] = _vm->getRandom(100) + 100;
	_vm->queueInsertDeviceIcon();
	_vm->clearFlag(kGFPlatypusDisguised);

	if (!_vm->isFlag(kGFUnk14))
		gameSys.insertSequence(0x1F8, 19, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFTruckKeysUsed)) {
		if (_vm->isFlag(kGFTruckFilledWithGas)) {
			gameSys.insertSequence(0x214, 39, 0, 0, kSeqLoop, 0, 0, 0);
			gameSys.insertSequence(0x20D, 39, 0, 0, kSeqLoop, 0, 0, 0);
			_vm->playSound(0x22B, true);
		} else {
			gameSys.insertSequence(0x1F9, 19, 0, 0, kSeqNone, 0, 0, 0);
		}
	} else {
		gameSys.insertSequence(0x1FA, 19, 0, 0, kSeqNone, 0, 15 * (5 * _vm->_s18GarbageCanPos - 40), 0);
		if (_vm->isFlag(kGFTruckFilledWithGas)) {
			gameSys.insertSequence(0x212, 39, 0, 0, kSeqLoop, 0, 0, 0);
			gameSys.insertSequence(0x20D, 39, 0, 0, kSeqLoop, 0, 0, 0);
			_vm->playSound(0x22B, true);
		} else if (_vm->isFlag(kGFBarnPadlockOpen)) {
			gameSys.insertSequence(0x20E, 39, 0, 0, kSeqLoop, 0, 0, 0);
			gameSys.insertSequence(0x217, 39, 0, 0, kSeqLoop, 0, 0, 0);
			_vm->playSound(0x22B, true);
		}
	}

	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		if (_vm->_prevSceneNum == 17)
			gnap.initPos(4, 11, kDirBottomRight);
		else
			gnap.initPos(4, 7, kDirBottomRight);
		_platPhoneCtr = _vm->getRandom(5);
		if (_vm->isFlag(kGFUnk27)) {
			gameSys.insertSequence(0x21E, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
			_currPhoneSequenceId = -1;
			platEndPhoning(true);
			_vm->clearFlag(kGFUnk27);
		} else {
			_currPhoneSequenceId = kScene18SequenceIds[_platPhoneCtr];
			_platPhoneIter = 0;
			gameSys.insertSequence(0x21F, 254, 0, 0, kSeqNone, 0, 0, 0);
			gameSys.insertSequence(_currPhoneSequenceId, 254, 0, 0, kSeqNone, 0, 0, 0);
			_vm->endSceneInit();
		}
		if (_vm->isFlag(kGFUnk27)) {
			platEndPhoning(true);
			_vm->clearFlag(kGFUnk27);
		} else {
			gameSys.setAnimation(_currPhoneSequenceId, 254, 3);
		}
		gnap.walkTo(Common::Point(4, 8), -1, 0x107B9, 1);
	} else {
		if (_vm->isFlag(kGFGnapControlsToyUFO)) {
			_vm->clearFlag(kGFGnapControlsToyUFO);
			_vm->setGrabCursorSprite(kItemCowboyHat);
			_vm->_prevSceneNum = 19;
		}
		if (_vm->_prevSceneNum == 17) {
			gnap.initPos(4, 11, kDirBottomRight);
			plat.initPos(5, 11, kDirIdleLeft);
			_vm->endSceneInit();
			gnap.walkTo(Common::Point(4, 8), -1, 0x107B9, 1);
			plat.walkTo(Common::Point(5, 9), -1, 0x107C2, 1);
		} else if (_vm->_prevSceneNum == 19) {
			gnap.initPos(7, 7, kDirBottomRight);
			plat.initPos(8, 7, kDirIdleLeft);
			_vm->endSceneInit();
			gnap.walkTo(Common::Point(7, 8), -1, 0x107B9, 1);
			plat.walkTo(Common::Point(8, 8), -1, 0x107C2, 1);
		} else {
			gnap.initPos(-1, 10, kDirBottomRight);
			plat.initPos(-1, 10, kDirIdleLeft);
			_vm->endSceneInit();
			gnap.walkTo(Common::Point(3, 7), -1, 0x107B9, 1);
			plat.walkTo(Common::Point(3, 8), -1, 0x107C2, 1);
		}
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 20, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS18Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS18Platypus:
			if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFPlatypusDisguised)) {
					gnapCarryGarbageCanTo(-1);
					putDownGarbageCan(0);
				}
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					gnap.useJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowItem(_vm->_grabCursorSpriteIndex, plat._pos.x, plat._pos.y);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan1(plat._pos);
						break;
					case GRAB_CURSOR:
						gnap.kissPlatypus(0);
						break;
					case TALK_CURSOR:
						gnap.playBrainPulsating(plat._pos);
						plat.playSequence(plat.getSequenceId());
						break;
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS18CowboyHat:
			if (gnap._actionStatus == kAS18StandingOnHydrant) {
				gnap._actionStatus = kAS18GrabCowboyHat;
				_vm->_sceneWaiting = false;
			} else if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFPlatypusDisguised)) {
					gnapCarryGarbageCanTo(-1);
					putDownGarbageCan(0);
				}
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS18CowboyHat], 3, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(3, 2));
						break;
					case GRAB_CURSOR:
						gnap.walkTo(_vm->_hotspotsWalkPos[kHS18CowboyHat], 0, gnap.getSequenceId(kGSPullOutDeviceNonWorking, Common::Point(3, 2)) | 0x10000, 1);
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS18GarbageCan:
			if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFUnk14)) {
					if (_vm->_grabCursorSpriteIndex >= 0)
						gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS18GarbageCan], 1, 5);
					else
						gnap.playImpossible();
				} else {
					if (_vm->isFlag(kGFPlatypusTalkingToAssistant))
						platEndPhoning(true);
					if (_vm->_grabCursorSpriteIndex >= 0) {
						if (!_vm->isFlag(kGFTruckKeysUsed)) {
							Common::Point destPos;
							destPos.x = _vm->_hotspotsWalkPos[kHS18GarbageCan].x - (gnap._pos.x < _vm->_s18GarbageCanPos ? 1 : -1);
							destPos.y = _vm->_hotspotsWalkPos[kHS18GarbageCan].y;
							gnap.playShowCurrItem(destPos, _vm->_hotspotsWalkPos[kHS18GarbageCan].x, _vm->_hotspotsWalkPos[kHS18GarbageCan].y);
						} else
							gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS18GarbageCan], 2, 4);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							if (!_vm->isFlag(kGFTruckKeysUsed))
								gnap.playScratchingHead(Common::Point(_vm->_hotspotsWalkPos[kHS18GarbageCan].x - (gnap._pos.x < _vm->_s18GarbageCanPos ? 1 : -1), _vm->_hotspotsWalkPos[kHS18GarbageCan].y));
							else if (!_vm->isFlag(kGFTruckFilledWithGas))
								gnap.playScratchingHead(Common::Point(2, 4));
							break;
						case GRAB_CURSOR:
							if (!_vm->isFlag(kGFTruckKeysUsed)) {
								gnap.walkTo(_vm->_hotspotsWalkPos[kHS18GarbageCan] + Common::Point((gnap._pos.x < _vm->_s18GarbageCanPos ? 1 : -1), 0),
									-1, -1, 1);
								gnap.walkTo(gnap._pos, 0, gnap.getSequenceId(kGSIdle, Common::Point(_vm->_s18GarbageCanPos, gnap._pos.y)) | 0x10000, 1);
								gnap._actionStatus = kAS18GrabGarbageCanFromStreet;
							} else if (!_vm->isFlag(kGFTruckFilledWithGas)) {
								if (gnap.walkTo(_vm->_hotspotsWalkPos[kHS18GarbageCan], 0, -1, 1))
									gnap._actionStatus = kAS18GrabGarbageCanFromHydrant;
							}
							break;
						case TALK_CURSOR:
						case PLAT_CURSOR:
							gnap.playImpossible();
							break;
						default:
							break;
						}
					}
				}
			}
			break;

		case kHS18HydrantTopValve:
			if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFPlatypusDisguised)) {
					// While carrying garbage can
					if (_vm->_grabCursorSpriteIndex >= 0) {
						gnapCarryGarbageCanTo(-1);
						putDownGarbageCan(0);
						gnap.playShowItem(_vm->_grabCursorSpriteIndex, 0, 0);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							gnapCarryGarbageCanTo(-1);
							putDownGarbageCan(0);
							gnap.playScratchingHead();
							break;
						case GRAB_CURSOR:
							if (_vm->isFlag(kGFTruckFilledWithGas)) {
								gnapCarryGarbageCanTo(2);
								gnap._actionStatus = kAS18PutGarbageCanOnRunningHydrant;
							} else if (!_vm->isFlag(kGFBarnPadlockOpen)) {
								gnapCarryGarbageCanTo(2);
								gnap._actionStatus = kAS18PutGarbageCanOnHydrant;
							} else {
								gnapCarryGarbageCanTo(-1);
								putDownGarbageCan(0);
								gnap.playImpossible();
							}
							break;
						case TALK_CURSOR:
						case PLAT_CURSOR:
							gnapCarryGarbageCanTo(-1);
							putDownGarbageCan(0);
							gnap.playImpossible();
							break;
						default:
							break;
						}
					}
				} else {
					if (_vm->_grabCursorSpriteIndex == kItemWrench) {
						gnap.walkTo(gnap._pos, 0, gnap.getSequenceId(kGSIdle, Common::Point(2, 8)) | 0x10000, 1);
						gnap._actionStatus = kAS18OpenTopValve;
					} else if (_vm->_grabCursorSpriteIndex >= 0) {
						gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS18HydrantTopValve], 1, 5);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							gnap.playScratchingHead(Common::Point(1, 5));
							break;
						case GRAB_CURSOR:
							if (_vm->isFlag(kGFBarnPadlockOpen)) {
								_vm->_hotspots[kHS18WalkArea2]._flags |= SF_WALKABLE;
								gnap.walkTo(_vm->_hotspotsWalkPos[kHS18HydrantTopValve], 0, 0x107BA, 1);
								_vm->_hotspots[kHS18WalkArea2]._flags &= ~SF_WALKABLE;
								gnap._actionStatus = kAS18CloseTopValve;
							} else
								gnap.playImpossible();
							break;
						case TALK_CURSOR:
						case PLAT_CURSOR:
							gnap.playImpossible();
							break;
						default:
							break;
						}
					}
				}
			}
			break;

		case kHS18HydrantRightValve:
			if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFUnk14)) {
					if (_vm->_grabCursorSpriteIndex == -1) {
						gnap.playImpossible();
					} else {
						gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS18HydrantRightValve], 1, 5);
					}
				} else {
					if (_vm->isFlag(kGFPlatypusDisguised)) {
						gnapCarryGarbageCanTo(-1);
						putDownGarbageCan(0);
					}
					if (_vm->_grabCursorSpriteIndex == kItemWrench) {
						gnap.walkTo(gnap._pos, 0, gnap.getSequenceId(kGSIdle, Common::Point(2, 8)) | 0x10000, 1);
						if (_vm->isFlag(kGFTruckKeysUsed))
							gnap._actionStatus = kAS18OpenRightValveWithGarbageCan;
						else
							gnap._actionStatus = kAS18OpenRightValveNoGarbageCan;
					} else if (_vm->_grabCursorSpriteIndex >= 0) {
						gnap.playShowCurrItem(_vm->_hotspotsWalkPos[kHS18HydrantRightValve], 1, 5);
					} else {
						switch (_vm->_verbCursor) {
						case LOOK_CURSOR:
							gnap.playScratchingHead(Common::Point(1, 5));
							break;
						case GRAB_CURSOR:
							if (_vm->isFlag(kGFTruckFilledWithGas)) {
								gnap.walkTo(_vm->_hotspotsWalkPos[kHS18HydrantRightValve], 0, 0x107BA, 1);
								if (_vm->isFlag(kGFTruckKeysUsed))
									gnap._actionStatus = kAS18CloseRightValveWithGarbageCan;
								else
									gnap._actionStatus = kAS18CloseRightValveNoGarbageCan;
							}
							break;
						case TALK_CURSOR:
						case PLAT_CURSOR:
							gnap.playImpossible();
							break;
						default:
							break;
						}
					}
				}
			}
			break;

		case kHS18ExitToyStore:
			if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFPlatypusDisguised)) {
					gnapCarryGarbageCanTo(-1);
					putDownGarbageCan(0);
				}
				if (_vm->isFlag(kGFPictureTaken)) {
					gnap.playImpossible();
				} else {
					_vm->_isLeavingScene = true;
					_vm->_newSceneNum = 19;
					gnap.walkTo(_vm->_hotspotsWalkPos[kHS18ExitToyStore], 0, 0x107C0, 1);
					gnap._actionStatus = kAS18LeaveScene;
					if (!_vm->isFlag(kGFPlatypusTalkingToAssistant))
						plat.walkTo(_vm->_hotspotsWalkPos[kHS18ExitToyStore] + Common::Point(1, 0), -1, 0x107C2, 1);
				}
			}
			break;

		case kHS18ExitPhoneBooth:
			if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFPlatypusDisguised)) {
					gnapCarryGarbageCanTo(-1);
					putDownGarbageCan(0);
				}
				closeHydrantValve();
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 17;
				gnap.walkTo(_vm->_hotspotsWalkPos[kHS18ExitPhoneBooth], 0, 0x107AE, 1);
				gnap._actionStatus = kAS18LeaveScene;
				if (_vm->isFlag(kGFPlatypusTalkingToAssistant))
					_vm->setFlag(kGFUnk27);
				else
					plat.walkTo(_vm->_hotspotsWalkPos[kHS18ExitPhoneBooth] + Common::Point(1, 0), -1, 0x107C2, 1);
			}
			break;

		case kHS18ExitGrubCity:
			if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFPlatypusDisguised)) {
					gnapCarryGarbageCanTo(-1);
					putDownGarbageCan(0);
				}
				closeHydrantValve();
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 20;
				_vm->_hotspots[kHS18WalkArea2]._flags |= SF_WALKABLE;
				gnap.walkTo(_vm->_hotspotsWalkPos[kHS18ExitGrubCity], 0, 0x107B2, 1);
				gnap._actionStatus = kAS18LeaveScene;
				if (_vm->isFlag(kGFPlatypusTalkingToAssistant))
					platEndPhoning(false);
				else
					plat.walkTo(_vm->_hotspotsWalkPos[kHS18ExitGrubCity] + Common::Point(0, -1), -1, 0x107CF, 1);
				_vm->_hotspots[kHS18WalkArea2]._flags &= ~SF_WALKABLE;
			}
			break;

		case kHS18WalkArea1:
		case kHS18WalkArea2:
			if (gnap._actionStatus < 0) {
				if (_vm->isFlag(kGFPlatypusDisguised)) {
					gnapCarryGarbageCanTo(-1);
					putDownGarbageCan(0);
				} else {
					gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				}
				_vm->_mouseClickState._left = false;
			}
			break;

		default:
			if (gnap._actionStatus != kAS18StandingOnHydrant && _vm->_mouseClickState._left) {
				if (_vm->isFlag(kGFPlatypusDisguised)) {
					gnapCarryGarbageCanTo(-1);
					putDownGarbageCan(0);
				} else {
					gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				}
				_vm->_mouseClickState._left = false;
			}
			break;
		}

		updateAnimations();

		if (!_vm->isSoundPlaying(0x10940))
			_vm->playSound(0x10940, true);

		if ((_vm->isFlag(kGFTruckFilledWithGas) || _vm->isFlag(kGFBarnPadlockOpen)) && !_vm->isSoundPlaying(0x22B) &&
			gnap._actionStatus != kAS18OpenRightValveNoGarbageCanDone && gnap._actionStatus != kAS18OpenRightValveNoGarbageCan &&
			gnap._actionStatus != kAS18OpenTopValve && gnap._actionStatus != kAS18OpenTopValveDone &&
			gnap._actionStatus != kAS18OpenRightValveWithGarbageCan && gnap._actionStatus != kAS18OpenRightValveWithGarbageCanDone)
			_vm->playSound(0x22B, true);

		if (!_vm->_isLeavingScene) {
			if (!_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
				if (plat._actionStatus == kAS18PlatComesHere) {
					if (!_vm->_timers[6]) {
						plat._actionStatus = -1;
						_vm->_sceneWaiting = false;
						plat.initPos(-1, 10, kDirIdleLeft);
						plat.walkTo(Common::Point(3, 9), -1, 0x107C2, 1);
						_vm->clearFlag(kGFPlatypusTalkingToAssistant);
					}
				} else {
					_vm->_hotspots[kHS18WalkArea1]._rect.bottom += 48;
					_vm->_hotspots[kHS18WalkArea2]._rect.left += 75;
					plat.updateIdleSequence();
					_vm->_hotspots[kHS18WalkArea2]._rect.left -= 75;
					_vm->_hotspots[kHS18WalkArea1]._rect.bottom -= 48;
				}
				if (!_vm->_timers[5]) {
					_vm->_timers[5] = _vm->getRandom(100) + 100;
					if (gnap._actionStatus < 0) {
						if (_vm->getRandom(2) == 1)
							gameSys.insertSequence(0x220, 255, 0, 0, kSeqNone, 0, 0, 0);
						else
							gameSys.insertSequence(0x221, 255, 0, 0, kSeqNone, 0, 0, 0);
					}
				}
				_vm->playSoundA();
			}
			if (!_vm->isFlag(kGFPlatypusDisguised))
				gnap.updateIdleSequence();
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}

	if (_vm->isFlag(kGFGnapControlsToyUFO))
		_vm->deleteSurface(&_cowboyHatSurface);
}

void Scene18::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS18GrabGarbageCanFromStreet:
			if (gnap._idleFacing != kDirUpRight && gnap._idleFacing != kDirBottomRight) {
				gameSys.insertSequence(0x1FC, gnap._id,
					makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
					kSeqSyncWait, 0, 75 * gnap._pos.x - 675, 0);
				gnap._sequenceDatNum = 0;
				gnap._sequenceId = 0x1FC;
			} else {
				gameSys.insertSequence(0x1FD, gnap._id,
					makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id,
					kSeqSyncWait, 0, 75 * gnap._pos.x - 525, 0);
				gnap._sequenceDatNum = 0;
				gnap._sequenceId = 0x1FD;
			}
			gameSys.removeSequence(0x1FA, 19, true);
			_vm->setFlag(kGFPlatypusDisguised);
			updateHotspots();
			gnap._actionStatus = -1;
			break;
		case kAS18GrabGarbageCanFromHydrant:
			gameSys.insertSequence(0x1FE, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.removeSequence(0x1F9, 19, true);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x1FE;
			_vm->clearFlag(kGFTruckKeysUsed);
			_vm->setFlag(kGFPlatypusDisguised);
			updateHotspots();
			gnap._actionStatus = -1;
			break;
		case kAS18CloseRightValveNoGarbageCan:
			gameSys.insertSequence(0x205, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.removeSequence(0x20D, 39, true);
			gameSys.removeSequence(0x212, 39, true);
			gameSys.removeSequence(0x211, 39, true);
			_vm->stopSound(0x22B);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x205;
			_vm->clearFlag(kGFTruckFilledWithGas);
			_vm->invAdd(kItemWrench);
			_vm->setGrabCursorSprite(kItemWrench);
			updateHotspots();
			gnap._actionStatus = -1;
			break;
		case kAS18OpenTopValve:
			_vm->setFlag(kGFBarnPadlockOpen);
			updateHotspots();
			gnap.playPullOutDevice(Common::Point(2, 7));
			gnap.playUseDevice();
			gameSys.insertSequence(0x20C, 19, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_hotspots[kHS18WalkArea2]._flags |= SF_WALKABLE;
			gnap.walkTo(_vm->_hotspotsWalkPos[kHS18HydrantTopValve], 0, 0x107BB, 1);
			_vm->_hotspots[kHS18WalkArea2]._flags &= ~SF_WALKABLE;
			gnap._actionStatus = kAS18OpenTopValveDone;
			break;
		case kAS18OpenTopValveDone:
			_vm->setGrabCursorSprite(-1);
			gameSys.insertSequence(0x208, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x216, 39, 0, 0, kSeqNone, 21, 0, 0);
			gameSys.removeSequence(0x20C, 19, true);
			gameSys.setAnimation(0x217, 39, 5);
			gameSys.insertSequence(0x217, 39, 0x216, 39, kSeqLoop | kSeqSyncWait, 0, 0, 0);
			while (gameSys.getAnimationStatus(5) != 2 && !_vm->_gameDone)
				_vm->gameUpdateTick();
			_vm->playSound(0x22B, true);
			gameSys.insertSequence(0x20E, 39, 0, 0, kSeqNone, 0, 0, 0);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x208;
			_vm->invRemove(kItemWrench);
			_vm->setGrabCursorSprite(-1);
			gnap._actionStatus = -1;
			break;
		case kAS18CloseTopValve:
			gameSys.insertSequence(0x206, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.removeSequence(0x20E, 39, true);
			gameSys.removeSequence(0x216, 39, true);
			gameSys.removeSequence(0x217, 39, true);
			_vm->stopSound(0x22B);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x206;
			_vm->clearFlag(kGFBarnPadlockOpen);
			_vm->invAdd(kItemWrench);
			_vm->setGrabCursorSprite(kItemWrench);
			updateHotspots();
			gnap._actionStatus = -1;
			break;
		case kAS18GrabCowboyHat:
			gameSys.setAnimation(0x200, gnap._id, 0);
			gameSys.insertSequence(0x200, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x200;
			gnap._actionStatus = kAS18GrabCowboyHatDone;
			break;
		case kAS18GrabCowboyHatDone:
			_vm->hideCursor();
			_vm->setGrabCursorSprite(-1);
			_cowboyHatSurface = _vm->addFullScreenSprite(0x1D2, 255);
			gameSys.setAnimation(0x218, 256, 0);
			gameSys.insertSequence(0x218, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (gameSys.getAnimationStatus(0) != 2 && !_vm->_gameDone)
				_vm->gameUpdateTick();
			_vm->_newSceneNum = 18;
			_vm->invAdd(kItemCowboyHat);
			_vm->invAdd(kItemWrench);
			_vm->setFlag(kGFGnapControlsToyUFO);
			_vm->setFlag(kGFUnk14);
			_vm->clearFlag(kGFTruckFilledWithGas);
			_vm->setFlag(kGFTruckKeysUsed);
			_vm->setFlag(kGFUnk14); // Useless, already set
			updateHotspots();
			gnap._actionStatus = kAS18LeaveScene;
			break;
		case kAS18LeaveScene:
			_vm->_sceneDone = true;
			gnap._actionStatus = -1;
			break;
		case kAS18PutGarbageCanOnRunningHydrant:
			_vm->setFlag(kGFTruckKeysUsed);
			_vm->clearFlag(kGFPlatypusDisguised);
			gameSys.requestRemoveSequence(0x211, 39);
			gameSys.requestRemoveSequence(0x212, 39);
			gameSys.insertSequence(0x210, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			_vm->stopSound(0x22B);
			gameSys.setAnimation(0x210, gnap._id, 0);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x210;
			gnap._actionStatus = kAS18PutGarbageCanOnRunningHydrant2;
			break;
		case kAS18PutGarbageCanOnRunningHydrant2:
			_vm->playSound(0x22B, true);
			gameSys.setAnimation(0x1FF, gnap._id, 0);
			gameSys.insertSequence(0x1FF, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x1FF;
			_vm->_sceneWaiting = true;
			gnap._actionStatus = kAS18StandingOnHydrant;
			break;
		case kAS18StandingOnHydrant:
			gameSys.setAnimation(0x1FF, gnap._id, 0);
			gameSys.insertSequence(0x1FF, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			break;
		case kAS18OpenRightValveNoGarbageCan:
		case kAS18OpenRightValveWithGarbageCan:
			_vm->setFlag(kGFTruckFilledWithGas);
			updateHotspots();
			gnap.playPullOutDevice(Common::Point(2, 7));
			gnap.playUseDevice();
			gameSys.insertSequence(0x20B, 19, 0, 0, kSeqNone, 0, 0, 0);
			_vm->_hotspots[kHS18WalkArea2]._flags |= SF_WALKABLE;
			gnap.walkTo(_vm->_hotspotsWalkPos[kHS18HydrantRightValve], 0, 0x107BA, 1);
			_vm->_hotspots[kHS18WalkArea2]._flags &= ~SF_WALKABLE;
			if (gnap._actionStatus == kAS18OpenRightValveNoGarbageCan)
				gnap._actionStatus = kAS18OpenRightValveNoGarbageCanDone;
			else
				gnap._actionStatus = kAS18OpenRightValveWithGarbageCanDone;
			break;
		case kAS18OpenRightValveWithGarbageCanDone:
			_vm->setGrabCursorSprite(-1);
			gameSys.insertSequence(0x207, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x213, 39, 0, 0, kSeqNone, 21, 0, 0);
			gameSys.requestRemoveSequence(0x1F9, 19);
			gameSys.removeSequence(0x20B, 19, true);
			gameSys.setAnimation(0x213, 39, 5);
			gameSys.insertSequence(0x214, 39, 0x213, 39, kSeqLoop | kSeqSyncWait, 0, 0, 0);
			while (gameSys.getAnimationStatus(5) != 2 && !_vm->_gameDone)
				_vm->gameUpdateTick();
			_vm->playSound(0x22B, true);
			gameSys.insertSequence(0x20D, 39, 0, 0, kSeqNone, 0, 0, 0);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x207;
			_vm->invRemove(kItemWrench);
			gnap._actionStatus = -1;
			break;
		case kAS18OpenRightValveNoGarbageCanDone:
			_vm->setGrabCursorSprite(-1);
			gameSys.insertSequence(0x207, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x211, 39, 0, 0, kSeqNone, 21, 0, 0);
			gameSys.removeSequence(0x20B, 19, true);
			gameSys.setAnimation(0x211, 39, 5);
			gameSys.insertSequence(0x212, 39, 0x211, 39, kSeqLoop | kSeqSyncWait, 0, 0, 0);
			while (gameSys.getAnimationStatus(5) != 2 && !_vm->_gameDone)
				_vm->gameUpdateTick();
			_vm->playSound(0x22B, true);
			gameSys.insertSequence(0x20D, 39, 0, 0, kSeqNone, 0, 0, 0);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x207;
			_vm->invRemove(kItemWrench);
			gnap._actionStatus = -1;
			break;
		case kAS18CloseRightValveWithGarbageCan:
			gameSys.insertSequence(0x205, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.removeSequence(0x20D, 39, true);
			gameSys.insertSequence(0x215, 39, 0x214, 39, kSeqSyncWait, 0, 0, 0);
			_vm->stopSound(0x22B);
			gameSys.setAnimation(0x1F9, 19, 0);
			gameSys.insertSequence(0x1F9, 19, 0x215, 39, kSeqSyncWait, 0, 0, 0);
			_vm->clearFlag(kGFTruckFilledWithGas);
			_vm->invAdd(kItemWrench);
			_vm->setGrabCursorSprite(kItemWrench);
			gameSys.insertSequence(0x107B5, gnap._id, 517, gnap._id, kSeqSyncWait, 0, 75 * gnap._pos.x - gnap._gridX, 48 * gnap._pos.y - gnap._gridY);
			updateHotspots();
			gnap._sequenceDatNum = 1;
			gnap._sequenceId = 0x7B5;
			gnap._actionStatus = kAS18CloseRightValveWithGarbageCanDone;
			break;
		case kAS18CloseRightValveWithGarbageCanDone:
			gnap._actionStatus = -1;
			break;
		case kAS18PutGarbageCanOnHydrant:
			_vm->setFlag(kGFTruckKeysUsed);
			_vm->clearFlag(kGFPlatypusDisguised);
			gameSys.insertSequence(0x20F, gnap._id, makeRid(gnap._sequenceDatNum, gnap._sequenceId), gnap._id, kSeqSyncWait, 0, 0, 0);
			gameSys.setAnimation(0x20F, gnap._id, 0);
			gnap._sequenceDatNum = 0;
			gnap._sequenceId = 0x20F;
			gnap._actionStatus = kAS18PutGarbageCanOnHydrantDone;
			break;
		case kAS18PutGarbageCanOnHydrantDone:
			gameSys.insertSequence(0x1F9, 19, 0x20F, gnap._id, kSeqNone, 0, 0, 0);
			updateHotspots();
			gnap._actionStatus = -1;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(3) == 2) {
		gameSys.setAnimation(0, 0, 3);
		++_platPhoneIter;
		if (_platPhoneIter <= 4) {
			++_platPhoneCtr;
			_nextPhoneSequenceId = kScene18SequenceIds[_platPhoneCtr % 5];
			gameSys.setAnimation(_nextPhoneSequenceId, 254, 3);
			gameSys.insertSequence(_nextPhoneSequenceId, 254, _currPhoneSequenceId, 254, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x21F, 254, 0x21F, 254, kSeqSyncWait, 0, 0, 0);
			_currPhoneSequenceId = _nextPhoneSequenceId;
		} else {
			platEndPhoning(true);
		}
	}
}

/*****************************************************************************/

static const int kS19ShopAssistantSequenceIds[] = {
	0x6F, 0x70, 0x71, 0x72, 0x73
};

Scene19::Scene19(GnapEngine *vm) : Scene(vm) {
	_toyGrabCtr = 0;
	_shopAssistantCtr = 0;
	_currShopAssistantSequenceId = -1;
	_nextShopAssistantSequenceId = -1;

	_pictureSurface = nullptr;
}

Scene19::~Scene19() {
	delete _pictureSurface;
}

int Scene19::init() {
	_vm->playSound(0x79, false);
	return _vm->isFlag(kGFPlatypusTalkingToAssistant) ? 0x77 : 0x76;
}

void Scene19::updateHotspots() {
	_vm->setHotspot(kHS19Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS19ExitOutsideToyStore, 36, 154, 142, 338, SF_EXIT_NW_CURSOR, 4, 6);
	_vm->setHotspot(kHS19Picture, 471, 237, 525, 283, SF_DISABLED, 7, 2);
	_vm->setHotspot(kHS19ShopAssistant, 411, 151, 575, 279, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 5, 7);
	_vm->setHotspot(kHS19Phone, 647, 166, 693, 234, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 0);
	_vm->setHotspot(kHS19Toy1, 181, 11, 319, 149, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 0);
	_vm->setHotspot(kHS19Toy2, 284, 85, 611, 216, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 6, 0);
	_vm->setHotspot(kHS19Toy3, 666, 38, 755, 154, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 0);
	_vm->setHotspot(kHS19Toy4, 154, 206, 285, 327, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 3, 3);
	_vm->setHotspot(kHS19Toy5, 494, 301, 570, 448, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 7, 5);
	_vm->setHotspot(kHS19Toy6, 0, 320, 188, 600, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 1, 6);
	_vm->setHotspot(kHS19Toy7, 597, 434, 800, 600, SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR, 9, 8);
	_vm->setHotspot(kHS19WalkArea1, 0, 0, 170, 600);
	_vm->setHotspot(kHS19WalkArea2, 622, 0, 800, 600);
	_vm->setHotspot(kHS19WalkArea3, 0, 0, 800, 437);
	_vm->setDeviceHotspot(kHS19Device, -1, -1, -1, -1);
	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		_vm->_hotspots[kHS19Toy1]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Toy2]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Toy3]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Toy4]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Toy5]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Toy6]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Toy7]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19ShopAssistant]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Phone]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Platypus]._flags = SF_DISABLED;
		_vm->_hotspots[kHS19Picture]._flags = SF_PLAT_CURSOR | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	}
	_vm->_hotspotsCount = 16;
}

void Scene19::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	PlayerPlat& plat = *_vm->_plat;

	_vm->queueInsertDeviceIcon();
	_toyGrabCtr = 0;
	_pictureSurface = nullptr;

	gameSys.insertSequence(0x74, 254, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.insertSequence(0x75, 254, 0, 0, kSeqNone, 0, 0, 0);

	if (!_vm->isFlag(kGFPictureTaken))
		gameSys.insertSequence(0x69, 19, 0, 0, kSeqNone, 0, 0, 0);

	if (_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
		gnap.initPos(3, 6, kDirBottomRight);
		_currShopAssistantSequenceId = kS19ShopAssistantSequenceIds[_vm->getRandom(5)];
		_nextShopAssistantSequenceId = _currShopAssistantSequenceId;
		gameSys.setAnimation(_currShopAssistantSequenceId, 20, 4);
		gameSys.insertSequence(0x6E, 254, 0, 0, kSeqNone, 0, 0, 0);
		gameSys.insertSequence(_currShopAssistantSequenceId, 20, 0, 0, kSeqNone, 0, 0, 0);
		_shopAssistantCtr = 0;
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(4, 9), -1, 0x107B9, 1);
		updateHotspots();
	} else {
		_currShopAssistantSequenceId = 0x6D;
		_nextShopAssistantSequenceId = -1;
		gameSys.setAnimation(0x6D, 20, 4);
		gameSys.insertSequence(_currShopAssistantSequenceId, 20, 0, 0, kSeqNone, 0, 0, 0);
		_vm->_timers[6] = _vm->getRandom(40) + 50;
		gnap.initPos(3, 6, kDirBottomRight);
		plat.initPos(4, 6, kDirIdleLeft);
		_vm->endSceneInit();
		gnap.walkTo(Common::Point(4, 9), -1, 0x107B9, 1);
		plat.walkTo(Common::Point(5, 9), -1, 0x107C2, 1);
	}

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 5, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS19Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;

		case kHS19Platypus:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex == kItemJoint) {
					gnap.useJointOnPlatypus();
				} else if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible(plat._pos);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan1(plat._pos);
						break;
					case GRAB_CURSOR:
						gnap.kissPlatypus(0);
						break;
					case TALK_CURSOR:
						gnap.playBrainPulsating(plat._pos);
						plat.playSequence(plat.getSequenceId());
						break;
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS19ExitOutsideToyStore:
			if (gnap._actionStatus < 0) {
				_vm->_isLeavingScene = true;
				_vm->_newSceneNum = 18;
				_vm->_hotspots[kHS19WalkArea1]._flags |= SF_WALKABLE;
				gnap.walkTo(_vm->_hotspotsWalkPos[1], 0, 0x107B2, 1);
				gnap._actionStatus = kAS19LeaveScene;
				if (_vm->isFlag(kGFPlatypusTalkingToAssistant))
					_vm->setFlag(kGFUnk27);
				else
					plat.walkTo(_vm->_hotspotsWalkPos[1] + Common::Point(1, 0), -1, 0x107C5, 1);
				_vm->_hotspots[kHS19WalkArea1]._flags &= ~SF_WALKABLE;
			}
			break;

		case kHS19Picture:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 6, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(6, 2));
						break;
					case GRAB_CURSOR:
						if (!_vm->isFlag(kGFPictureTaken)) {
							gnap.walkTo(gnap._pos, 0, gnap.getSequenceId(kGSIdle, _vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot]) | 0x10000, 1);
							gnap._actionStatus = kAS19GrabPicture;
						}
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS19ShopAssistant:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 6, 2);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(6, 2));
						break;
					case TALK_CURSOR:
						gnap._idleFacing = kDirUpRight;
						gnap.walkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 0, gnap.getSequenceId(kGSBrainPulsating, Common::Point(0, 0)) | 0x10000, 1);
						gnap._actionStatus = kAS19TalkShopAssistant;
						break;
					case GRAB_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS19Toy1:
		case kHS19Toy2:
		case kHS19Toy3:
		case kHS19Toy4:
		case kHS19Toy5:
		case kHS19Toy6:
		case kHS19Toy7:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playImpossible(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot]);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playMoan2(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot]);
						break;
					case GRAB_CURSOR:
						gnap.walkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 0, -1, 1);
						gnap.playIdle(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot]);
						gnap._actionStatus = kAS19GrabToy;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS19Phone:
			if (gnap._actionStatus < 0) {
				if (_vm->_grabCursorSpriteIndex >= 0) {
					gnap.playShowCurrItem(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 9, 1);
				} else {
					switch (_vm->_verbCursor) {
					case LOOK_CURSOR:
						gnap.playScratchingHead(Common::Point(9, 1));
						break;
					case GRAB_CURSOR:
						gnap.walkTo(_vm->_hotspotsWalkPos[_vm->_sceneClickedHotspot], 0, -1, 1);
						gnap.playIdle(Common::Point(8, 2));
						gnap._actionStatus = kAS19UsePhone;
						break;
					case TALK_CURSOR:
					case PLAT_CURSOR:
						gnap.playImpossible();
						break;
					default:
						break;
					}
				}
			}
			break;

		case kHS19WalkArea1:
		case kHS19WalkArea2:
		case kHS19WalkArea3:
			if (gnap._actionStatus < 0)
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
			break;

		default:
			if (_vm->_mouseClickState._left) {
				gnap.walkTo(Common::Point(-1, -1), -1, -1, 1);
				_vm->_mouseClickState._left = 0;
			}
		}

		updateAnimations();

		if (!_vm->_isLeavingScene) {
			gnap.updateIdleSequence();
			if (!_vm->isFlag(kGFPlatypusTalkingToAssistant)) {
				plat.updateIdleSequence();
				if (!_vm->_timers[6] && _nextShopAssistantSequenceId == -1) {
					_vm->_timers[6] = _vm->getRandom(40) + 50;
					if (_vm->getRandom(4) != 0) {
						_nextShopAssistantSequenceId = 0x64;
					} else if (_vm->isFlag(kGFPictureTaken)) {
						_nextShopAssistantSequenceId = 0x64;
					} else {
						_nextShopAssistantSequenceId = 0x6C;
					}
				}
			}
		}

		_vm->checkGameKeys();

		if (_vm->isKeyStatus1(Common::KEYCODE_BACKSPACE)) {
			_vm->clearKeyStatus1(Common::KEYCODE_BACKSPACE);
			_vm->runMenu();
			updateHotspots();
		}

		_vm->gameUpdateTick();
	}

	if (_pictureSurface)
		_vm->deleteSurface(&_pictureSurface);
}

void Scene19::updateAnimations() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (gameSys.getAnimationStatus(0) == 2) {
		gameSys.setAnimation(0, 0, 0);
		switch (gnap._actionStatus) {
		case kAS19UsePhone:
			_nextShopAssistantSequenceId = 0x67;
			break;
		case kAS19GrabToy:
			++_toyGrabCtr;
			switch (_toyGrabCtr) {
			case 1:
				_nextShopAssistantSequenceId = 0x62;
				break;
			case 2:
				_nextShopAssistantSequenceId = 0x6B;
				break;
			case 3:
				_nextShopAssistantSequenceId = 0x66;
				break;
			default:
				_nextShopAssistantSequenceId = 0x65;
				break;
			}
			break;
		case kAS19GrabPicture:
			gnap.playPullOutDevice(Common::Point(6, 2));
			gnap.playUseDevice();
			gameSys.setAnimation(0x68, 19, 0);
			gameSys.insertSequence(0x68, 19, 105, 19, kSeqSyncWait, 0, 0, 0);
			_vm->invAdd(kItemPicture);
			_vm->setFlag(kGFPictureTaken);
			updateHotspots();
			gnap._actionStatus = kAS19GrabPictureDone;
			break;
		case kAS19GrabPictureDone:
			_vm->setGrabCursorSprite(-1);
			_vm->hideCursor();
			_pictureSurface = _vm->addFullScreenSprite(0xF, 255);
			gameSys.setAnimation(0x61, 256, 0);
			gameSys.insertSequence(0x61, 256, 0, 0, kSeqNone, 0, 0, 0);
			while (gameSys.getAnimationStatus(0) != 2 && !_vm->_gameDone)
				_vm->gameUpdateTick();

			_vm->setFlag(kGFUnk27);
			_vm->showCursor();
			_vm->_newSceneNum = 17;
			_vm->_isLeavingScene = true;
			_vm->_sceneDone = true;
			_nextShopAssistantSequenceId = -1;
			break;
		case kAS19TalkShopAssistant:
			_nextShopAssistantSequenceId = 0x6D;
			gnap._actionStatus = -1;
			break;
		case kAS19LeaveScene:
			_vm->_sceneDone = true;
			break;
		default:
			break;
		}
	}

	if (gameSys.getAnimationStatus(4) == 2) {
		switch (_nextShopAssistantSequenceId) {
		case 0x6F:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
			_shopAssistantCtr = (_shopAssistantCtr + 1) % 5;
			_nextShopAssistantSequenceId = kS19ShopAssistantSequenceIds[_shopAssistantCtr];
			gameSys.setAnimation(_nextShopAssistantSequenceId, 20, 4);
			gameSys.insertSequence(_nextShopAssistantSequenceId, 20, _currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x6E, 254, 0x6E, 254, kSeqSyncWait, 0, 0, 0);
			_currShopAssistantSequenceId = _nextShopAssistantSequenceId;
			break;
		case 0x62:
		case 0x66:
		case 0x6B:
			gameSys.setAnimation(_nextShopAssistantSequenceId, 20, 4);
			gameSys.insertSequence(_nextShopAssistantSequenceId, 20, _currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_currShopAssistantSequenceId = _nextShopAssistantSequenceId;
			_nextShopAssistantSequenceId = -1;
			_vm->_timers[5] = 10;
			while (_vm->_timers[5] && !_vm->_gameDone)
				_vm->gameUpdateTick();

			gnap.playIdle(Common::Point(6, 2));
			gnap._actionStatus = -1;
			break;
		case 0x67:
			gameSys.setAnimation(_nextShopAssistantSequenceId, 20, 4);
			gameSys.insertSequence(_nextShopAssistantSequenceId, 20, _currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_currShopAssistantSequenceId = _nextShopAssistantSequenceId;
			_nextShopAssistantSequenceId = -1;
			gnap._actionStatus = -1;
			break;
		case 0x65:
			gnap.playIdle(Common::Point(6, 2));
			gameSys.setAnimation(_nextShopAssistantSequenceId, 20, 0);
			gameSys.insertSequence(_nextShopAssistantSequenceId, 20, _currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_currShopAssistantSequenceId = _nextShopAssistantSequenceId;
			_nextShopAssistantSequenceId = -1;
			_vm->_newSceneNum = 18;
			gnap._actionStatus = kAS19LeaveScene;
			break;
		case 0x6D:
			gameSys.setAnimation(_nextShopAssistantSequenceId, 20, 4);
			gameSys.insertSequence(_nextShopAssistantSequenceId, 20, _currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			gameSys.insertSequence(0x69, 19, 0x69, 19, kSeqSyncWait, _vm->getSequenceTotalDuration(_nextShopAssistantSequenceId), 0, 0);
			_currShopAssistantSequenceId = _nextShopAssistantSequenceId;
			_nextShopAssistantSequenceId = -1;
			break;
		case 0x64:
		case 0x6C:
			gameSys.setAnimation(_nextShopAssistantSequenceId, 20, 4);
			gameSys.insertSequence(_nextShopAssistantSequenceId, 20, _currShopAssistantSequenceId, 20, kSeqSyncWait, 0, 0, 0);
			_currShopAssistantSequenceId = _nextShopAssistantSequenceId;
			_nextShopAssistantSequenceId = -1;
			break;
		default:
			break;
		}
	}
}

} // End of namespace Gnap
