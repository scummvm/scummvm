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
#include "gnap/scenes/group5.h"

namespace Gnap {

Scene53::Scene53(GnapEngine *vm) : Scene(vm) {
	_isGnapPhoning = false;
	_currHandSequenceId = -1;
	_callsMadeCtr = 0;
	_callsRndUsed = 0;
}

int Scene53::init() {
	GameSys& gameSys = *_vm->_gameSys;

	gameSys.setAnimation(0, 0, 0);
	gameSys.setAnimation(0, 0, 1);
	return 0x75;
}

void Scene53::updateHotspots() {
	_vm->setHotspot(kHS53Platypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHS53PhoneKey1, 336, 238, 361, 270, SF_GRAB_CURSOR);
	_vm->setHotspot(kHS53PhoneKey2, 376, 243, 405, 274, SF_GRAB_CURSOR);
	_vm->setHotspot(kHS53PhoneKey3, 415, 248, 441, 276, SF_GRAB_CURSOR);
	_vm->setHotspot(kHS53PhoneKey4, 329, 276, 358, 303, SF_GRAB_CURSOR);
	_vm->setHotspot(kHS53PhoneKey5, 378, 282, 408, 311, SF_GRAB_CURSOR);
	_vm->setHotspot(kHS53PhoneKey6, 417, 286, 446, 319, SF_GRAB_CURSOR);
	_vm->setHotspot(kHS53PhoneKey7, 332, 311, 361, 342, SF_GRAB_CURSOR);
	_vm->setHotspot(kHS53PhoneKey8, 376, 318, 407, 349, SF_GRAB_CURSOR);
	_vm->setHotspot(kHS53PhoneKey9, 417, 320, 447, 353, SF_GRAB_CURSOR);
	_vm->setHotspot(kHS53PhoneKey0, 377, 352, 405, 384, SF_GRAB_CURSOR);
	_vm->setHotspot(kHS53PhoneKeySharp, 419, 358, 450, 394, SF_GRAB_CURSOR);
	_vm->setHotspot(kHS53PhoneKeyStar, 328, 346, 359, 379, SF_GRAB_CURSOR);
	_vm->setHotspot(kHS53PhoneExit, 150, 585, 650, 600, SF_EXIT_D_CURSOR);

	_vm->setDeviceHotspot(kHS53Device, -1, -1, -1, -1);
	_vm->_hotspotsCount = 15;
}

int Scene53::pressPhoneNumberButton(int phoneNumber, int buttonNum) {
	static const int kGnapHandSequenceIds[13] = {
		0x00,
		0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B,
		0x4C, 0x4D, 0x4E, 0x50, 0x51, 0x4F
	};

	static const int kPlatypusHandSequenceIds[13] = {
		0x00,
		0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
		0x58, 0x59, 0x5A, 0x5C, 0x5D, 0x5B
	};

	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	if (_isGnapPhoning) {
		gameSys.setAnimation(kGnapHandSequenceIds[buttonNum], 40, 6);
		gameSys.insertSequence(kGnapHandSequenceIds[buttonNum], 40, _currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		_currHandSequenceId = kGnapHandSequenceIds[buttonNum];
	} else {
		gameSys.setAnimation(kPlatypusHandSequenceIds[buttonNum], 40, 6);
		gameSys.insertSequence(kPlatypusHandSequenceIds[buttonNum], 40, _currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		_currHandSequenceId = kPlatypusHandSequenceIds[buttonNum];
	}

	gnap._actionStatus = 6;
	while (gameSys.getAnimationStatus(6) != 2 && !_vm->_gameDone) {
		_vm->updateMouseCursor();
		_vm->gameUpdateTick();
	}
	gnap._actionStatus = -1;

	if (buttonNum < 11)
		phoneNumber = buttonNum % 10 + 10 * phoneNumber;

	return phoneNumber;
}

int Scene53::getRandomCallIndex() {
	int index, tries = 0;
	if (_callsRndUsed == 0x7FFF)
		_callsRndUsed = 0;
	do {
		index = _vm->getRandom(16);
		if (++tries == 300)
			_callsRndUsed = 0;
	} while (_callsRndUsed & (1 << index));
	_callsRndUsed |= (1 << index);
	return index;
}

void Scene53::runRandomCall() {
	static const int kCallSequenceIds[15] = {
		0x60, 0x61, 0x62, 0x63, 0x64,
		0x65, 0x66, 0x67, 0x68, 0x69,
		0x6A, 0x6B, 0x6C, 0x6D, 0x71
	};

	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	++_callsMadeCtr;
	if (_callsMadeCtr <= 10) {
		int index;

		do {
			index = getRandomCallIndex();
		} while (!_isGnapPhoning && (index == 0 || index == 3 || index == 4 || index == 11));
		gameSys.setAnimation(kCallSequenceIds[index], 1, 6);
		gameSys.insertSequence(kCallSequenceIds[index], 1, 0, 0, kSeqNone, 16, 0, 0);
	} else {
		gameSys.setAnimation(0x74, 1, 6);
		gameSys.insertSequence(0x74, 1, 0, 0, kSeqNone, 16, 0, 0);
		_callsMadeCtr = 0;
	}

	gnap._actionStatus = 1;
	while (gameSys.getAnimationStatus(6) != 2 && !_vm->_gameDone) {
		_vm->updateMouseCursor();
		_vm->gameUpdateTick();
	}
	gnap._actionStatus = -1;
}

void Scene53::runChitChatLine() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;
	bool flag = false;
	int sequenceId = -1;

	gameSys.setAnimation(0x6E, 1, 6);
	gameSys.insertSequence(0x6E, 1, 0, 0, kSeqNone, 16, 0, 0);

	gnap._actionStatus = 1;
	while (gameSys.getAnimationStatus(6) != 2 && !_vm->_gameDone) {
		_vm->updateMouseCursor();
		_vm->gameUpdateTick();
	}
	gnap._actionStatus = -1;

	if (_vm->isFlag(kGFSpringTaken)) {
		gameSys.insertSequence(0x45, 40, _currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		_currHandSequenceId = 0x45;
	} else {
		gameSys.insertSequence(0x45, 40, _currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		_currHandSequenceId = 0x5E;
	}

	_vm->_hotspots[kHS53Device]._flags = SF_DISABLED;

	while (!flag) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 0, -1, -1, -1, -1);
		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case 2:
			sequenceId = 0x6F;
			flag = 1;
			break;
		case 3:
			sequenceId = 0x70;
			flag = 1;
			break;
		case 4:
			sequenceId = 0x71;
			flag = 1;
			break;
		case 14:
			sequenceId = -1;
			flag = 1;
			_vm->_isLeavingScene = true;
			_vm->_sceneDone = true;
			gnap._actionStatus = 0;
			_vm->_newSceneNum = 17;
			break;
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
			pressPhoneNumberButton(0, _vm->_sceneClickedHotspot - 1);
			break;
		default:
			break;
		}

		if (flag && sequenceId != -1) {
			_vm->stopSound(0xA0);
			pressPhoneNumberButton(0, _vm->_sceneClickedHotspot - 1);
			gnap._actionStatus = 1;
			gameSys.setAnimation(sequenceId, 1, 6);
			gameSys.insertSequence(sequenceId, 1, 0, 0, kSeqNone, 16, 0, 0);
			gnap._actionStatus = 1;
			while (gameSys.getAnimationStatus(6) != 2 && !_vm->_gameDone) {
				_vm->updateMouseCursor();
				_vm->gameUpdateTick();
			}
			gnap._actionStatus = -1;
			gameSys.setAnimation(0x72, 1, 6);
			gameSys.insertSequence(0x72, 1, 0, 0, kSeqNone, 16, 0, 0);
			gnap._actionStatus = 1;
			while (gameSys.getAnimationStatus(6) != 2 && !_vm->_gameDone) {
				_vm->updateMouseCursor();
				_vm->gameUpdateTick();
			}
			gnap._actionStatus = -1;
		}
	}

	updateHotspots();

	gnap._actionStatus = 1;

	if (_vm->isFlag(kGFSpringTaken)) {
		gameSys.setAnimation(0x73, 40, 6);
		gameSys.insertSequence(0x73, 40, _currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		while (gameSys.getAnimationStatus(6) != 2 && !_vm->_gameDone) {
			_vm->updateMouseCursor();
			_vm->gameUpdateTick();
		}
		_currHandSequenceId = 0x73;
		gnap._actionStatus = -1;
	}
}

void Scene53::run() {
	GameSys& gameSys = *_vm->_gameSys;
	PlayerGnap& gnap = *_vm->_gnap;

	int phoneNumber = 0;
	int phoneNumberLen = 0;

	_vm->queueInsertDeviceIcon();

	if (_vm->isFlag(kGFSpringTaken)) {
		_currHandSequenceId = 0x45;
		_isGnapPhoning = true;
	} else {
		_currHandSequenceId = 0x5E;
		_isGnapPhoning = false;
	}

	gameSys.insertSequence(_currHandSequenceId, 40, 0, 0, kSeqNone, 0, 0, 0);
	_vm->endSceneInit();
	_vm->setVerbCursor(GRAB_CURSOR);
	_vm->playSound(0xA0, true);

	while (!_vm->_sceneDone) {
		_vm->updateMouseCursor();
		_vm->updateCursorByHotspot();

		_vm->testWalk(0, 0, -1, -1, -1, -1);

		_vm->_sceneClickedHotspot = _vm->getClickedHotspotId();
		_vm->updateGrabCursorSprite(0, 0);

		switch (_vm->_sceneClickedHotspot) {
		case kHS53Device:
			if (gnap._actionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;
		case kHS53PhoneKey1:
		case kHS53PhoneKey2:
		case kHS53PhoneKey3:
		case kHS53PhoneKey4:
		case kHS53PhoneKey5:
		case kHS53PhoneKey6:
		case kHS53PhoneKey7:
		case kHS53PhoneKey8:
		case kHS53PhoneKey9:
		case kHS53PhoneKey0:
			_vm->stopSound(0xA0);
			++phoneNumberLen;
			phoneNumber = pressPhoneNumberButton(phoneNumber, _vm->_sceneClickedHotspot - 1);
			debugC(kDebugBasic, "phoneNumber: %d", phoneNumber);
			if (phoneNumberLen == 7) {
				gnap._actionStatus = 1;
				if (_vm->isFlag(kGFSpringTaken)) {
					gameSys.setAnimation(0x73, 40, 6);
					gameSys.insertSequence(0x73, 40, _currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
					while (gameSys.getAnimationStatus(6) != 2 && !_vm->_gameDone) {
						_vm->updateMouseCursor();
						_vm->gameUpdateTick();
					}
					_currHandSequenceId = 0x73;
					gnap._actionStatus = -1;
				}
				if (phoneNumber == 7284141) {
					runChitChatLine();
					phoneNumber = 0;
					phoneNumberLen = 0;
					_vm->_sceneDone = true;
					_vm->_newSceneNum = 17;
				} else if (phoneNumber != 5556789 || _vm->isFlag(kGFPictureTaken)) {
					runRandomCall();
					phoneNumber = 0;
					phoneNumberLen = 0;
					_vm->_sceneDone = true;
					_vm->_newSceneNum = 17;
				} else {
					phoneNumber = 0;
					phoneNumberLen = 0;
					_vm->_sceneDone = true;
					_vm->_newSceneNum = 17;
					if (_isGnapPhoning)
						_vm->setFlag(kGFUnk25);
					else
						_vm->setFlag(kGFPlatypusTalkingToAssistant);
				}
			}
			break;
		case kHS53PhoneKeySharp:
		case kHS53PhoneKeyStar:
			pressPhoneNumberButton(0, _vm->_sceneClickedHotspot - 1);
			break;
		case kHS53PhoneExit:
			if (gnap._actionStatus < 0) {
				gnap._actionStatus = 1;
				if (_vm->isFlag(kGFSpringTaken)) {
					gameSys.setAnimation(0x73, 40, 6);
					gameSys.insertSequence(0x73, 40, _currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
					while (gameSys.getAnimationStatus(6) != 2 && !_vm->_gameDone) {
						_vm->updateMouseCursor();
						_vm->gameUpdateTick();
					}
					_currHandSequenceId = 0x73;
					gnap._actionStatus = -1;
				}
				_vm->_isLeavingScene = true;
				_vm->_sceneDone = true;
				gnap._actionStatus = 0;
				_vm->_newSceneNum = 17;
			}
			break;
		default:
			break;
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

} // End of namespace Gnap
