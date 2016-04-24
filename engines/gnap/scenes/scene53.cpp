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
#include "gnap/scenes/scene53.h"

namespace Gnap {

enum {
	kHSPlatypus		 = 0,
	kHSDevice		 = 1,
	kHSPhoneKey1	 = 2,
	kHSPhoneKey2	 = 3,
	kHSPhoneKey3	 = 4,
	kHSPhoneKey4	 = 5,
	kHSPhoneKey5	 = 6,
	kHSPhoneKey6	 = 7,
	kHSPhoneKey7	 = 8,
	kHSPhoneKey8	 = 9,
	kHSPhoneKey9	 = 10,
	kHSPhoneKey0	 = 11,
	kHSPhoneKeySharp = 12,
	kHSPhoneKeyStar	 = 13,
	kHSPhoneExit	 = 14
};

Scene53::Scene53(GnapEngine *vm) : Scene(vm) {
	_s53_isGnapPhoning = false;
	_s53_currHandSequenceId = -1;
	_s53_callsMadeCtr = 0;
	_s53_callsRndUsed = 0;
}

int Scene53::init() {
	_vm->_gameSys->setAnimation(0, 0, 0);
	_vm->_gameSys->setAnimation(0, 0, 1);
	return 0x75;
}

void Scene53::updateHotspots() {
	_vm->setHotspot(kHSPlatypus, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	_vm->setHotspot(kHSPhoneKey1, 336, 238, 361, 270, SF_GRAB_CURSOR);
	_vm->setHotspot(kHSPhoneKey2, 376, 243, 405, 274, SF_GRAB_CURSOR);
	_vm->setHotspot(kHSPhoneKey3, 415, 248, 441, 276, SF_GRAB_CURSOR);
	_vm->setHotspot(kHSPhoneKey4, 329, 276, 358, 303, SF_GRAB_CURSOR);
	_vm->setHotspot(kHSPhoneKey5, 378, 282, 408, 311, SF_GRAB_CURSOR);
	_vm->setHotspot(kHSPhoneKey6, 417, 286, 446, 319, SF_GRAB_CURSOR);
	_vm->setHotspot(kHSPhoneKey7, 332, 311, 361, 342, SF_GRAB_CURSOR);
	_vm->setHotspot(kHSPhoneKey8, 376, 318, 407, 349, SF_GRAB_CURSOR);
	_vm->setHotspot(kHSPhoneKey9, 417, 320, 447, 353, SF_GRAB_CURSOR);
	_vm->setHotspot(kHSPhoneKey0, 377, 352, 405, 384, SF_GRAB_CURSOR);
	_vm->setHotspot(kHSPhoneKeySharp, 419, 358, 450, 394, SF_GRAB_CURSOR);
	_vm->setHotspot(kHSPhoneKeyStar, 328, 346, 359, 379, SF_GRAB_CURSOR);
	_vm->setHotspot(kHSPhoneExit, 150, 585, 650, 600, SF_EXIT_D_CURSOR);

	_vm->setDeviceHotspot(kHSDevice, -1, -1, -1, -1);
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

	if (_s53_isGnapPhoning) {
		_vm->_gameSys->setAnimation(kGnapHandSequenceIds[buttonNum], 40, 6);
		_vm->_gameSys->insertSequence(kGnapHandSequenceIds[buttonNum], 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		_s53_currHandSequenceId = kGnapHandSequenceIds[buttonNum];
	} else {
		_vm->_gameSys->setAnimation(kPlatypusHandSequenceIds[buttonNum], 40, 6);
		_vm->_gameSys->insertSequence(kPlatypusHandSequenceIds[buttonNum], 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		_s53_currHandSequenceId = kPlatypusHandSequenceIds[buttonNum];
	}

	_vm->_gnapActionStatus = 6;
	while (_vm->_gameSys->getAnimationStatus(6) != 2) {
		// checkGameAppStatus();
		_vm->updateMouseCursor();
		_vm->gameUpdateTick();
	}
	_vm->_gnapActionStatus = -1;

	if (buttonNum < 11)
		phoneNumber = buttonNum % 10 + 10 * phoneNumber;

	return phoneNumber;
}

int Scene53::getRandomCallIndex() {
	int index, tries = 0;
	if (_s53_callsRndUsed == 0x7FFF)
		_s53_callsRndUsed = 0;
	do {
		index = _vm->getRandom(16);
		if (++tries == 300)
			_s53_callsRndUsed = 0;
	} while (_s53_callsRndUsed & (1 << index));
	_s53_callsRndUsed |= (1 << index);
	return index;
}

void Scene53::runRandomCall() {
	static const int kCallSequenceIds[15] = {
		0x60, 0x61, 0x62, 0x63, 0x64,
		0x65, 0x66, 0x67, 0x68, 0x69,
		0x6A, 0x6B, 0x6C, 0x6D, 0x71
	};

	++_s53_callsMadeCtr;

	if (_s53_callsMadeCtr <= 10) {
		int index;

		do {
			index = getRandomCallIndex();
		} while (!_s53_isGnapPhoning && (index == 0 || index == 3 || index == 4 || index == 11));
		_vm->_gameSys->setAnimation(kCallSequenceIds[index], 1, 6);
		_vm->_gameSys->insertSequence(kCallSequenceIds[index], 1, 0, 0, kSeqNone, 16, 0, 0);
	} else {
		_vm->_gameSys->setAnimation(0x74, 1, 6);
		_vm->_gameSys->insertSequence(0x74, 1, 0, 0, kSeqNone, 16, 0, 0);
		_s53_callsMadeCtr = 0;
	}
	
	_vm->_gnapActionStatus = 1;
	while (_vm->_gameSys->getAnimationStatus(6) != 2) {
		_vm->updateMouseCursor();
		// checkGameAppStatus();
		_vm->gameUpdateTick();
	}
	_vm->_gnapActionStatus = -1;
}

void Scene53::runChitChatLine() {
	bool flag = false;
	int sequenceId = -1;
	
	_vm->_gameSys->setAnimation(0x6E, 1, 6);
	_vm->_gameSys->insertSequence(0x6E, 1, 0, 0, kSeqNone, 16, 0, 0);

	_vm->_gnapActionStatus = 1;
	while (_vm->_gameSys->getAnimationStatus(6) != 2) {
		_vm->updateMouseCursor();
		// checkGameAppStatus();
		_vm->gameUpdateTick();
	}
	_vm->_gnapActionStatus = -1;

	if (_vm->isFlag(kGFSpringTaken)) {
		_vm->_gameSys->insertSequence(0x45, 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		_s53_currHandSequenceId = 0x45;
	} else {
		_vm->_gameSys->insertSequence(0x45, 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		_s53_currHandSequenceId = 0x5E;
	}
	
	_vm->_hotspots[kHSDevice]._flags = SF_DISABLED;
	
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
			_vm->_gnapActionStatus = 0;
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
		}
	
		if (flag && sequenceId != -1) {
			_vm->stopSound(0xA0);
			pressPhoneNumberButton(0, _vm->_sceneClickedHotspot - 1);
			_vm->_gnapActionStatus = 1;
			_vm->_gameSys->setAnimation(sequenceId, 1, 6);
			_vm->_gameSys->insertSequence(sequenceId, 1, 0, 0, kSeqNone, 16, 0, 0);
			_vm->_gnapActionStatus = 1;
			while (_vm->_gameSys->getAnimationStatus(6) != 2) {
				_vm->updateMouseCursor();
				// checkGameAppStatus();
				_vm->gameUpdateTick();
			}
			_vm->_gnapActionStatus = -1;
			_vm->_gameSys->setAnimation(0x72, 1, 6);
			_vm->_gameSys->insertSequence(0x72, 1, 0, 0, kSeqNone, 16, 0, 0);
			_vm->_gnapActionStatus = 1;
			while (_vm->_gameSys->getAnimationStatus(6) != 2) {
				_vm->updateMouseCursor();
				// checkGameAppStatus();
				_vm->gameUpdateTick();
			}
			_vm->_gnapActionStatus = -1;
		}
	}
	
	updateHotspots();
	
	_vm->_gnapActionStatus = 1;
	
	if (_vm->isFlag(kGFSpringTaken)) {
		_vm->_gameSys->setAnimation(0x73, 40, 6);
		_vm->_gameSys->insertSequence(0x73, 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		while (_vm->_gameSys->getAnimationStatus(6) != 2) {
			_vm->updateMouseCursor();
			// checkGameAppStatus();
			_vm->gameUpdateTick();
		}
		_s53_currHandSequenceId = 0x73;
		_vm->_gnapActionStatus = -1;
	}
}

void Scene53::run() {
	int phoneNumber = 0;
	int phoneNumberLen = 0;
	
	_vm->queueInsertDeviceIcon();
	
	if (_vm->isFlag(kGFSpringTaken)) {
		_s53_currHandSequenceId = 0x45;
		_s53_isGnapPhoning = true;
	} else {
		_s53_currHandSequenceId = 0x5E;
		_s53_isGnapPhoning = false;
	}
	
	_vm->_gameSys->insertSequence(_s53_currHandSequenceId, 40, 0, 0, kSeqNone, 0, 0, 0);
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
		case 1:
			if (_vm->_gnapActionStatus < 0) {
				_vm->runMenu();
				updateHotspots();
			}
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
			_vm->stopSound(0xA0);
			++phoneNumberLen;
			phoneNumber = pressPhoneNumberButton(phoneNumber, _vm->_sceneClickedHotspot - 1);
			debugC(kDebugBasic, "phoneNumber: %d", phoneNumber);
			if (phoneNumberLen == 7) {
				_vm->_gnapActionStatus = 1;
				if (_vm->isFlag(kGFSpringTaken)) {
					_vm->_gameSys->setAnimation(0x73, 40, 6);
					_vm->_gameSys->insertSequence(0x73, 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
					while (_vm->_gameSys->getAnimationStatus(6) != 2) {
						_vm->updateMouseCursor();
						// checkGameAppStatus();
						_vm->gameUpdateTick();
					}
					_s53_currHandSequenceId = 0x73;
					_vm->_gnapActionStatus = -1;
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
					if (_s53_isGnapPhoning)
						_vm->setFlag(kGFUnk25);
					else
						_vm->setFlag(kGFPlatypusTalkingToAssistant);
				}
			}
			break;
		case 12:
		case 13:
			pressPhoneNumberButton(0, _vm->_sceneClickedHotspot - 1);
			break;
		case 14:
			if (_vm->_gnapActionStatus < 0) {
				_vm->_gnapActionStatus = 1;
				if (_vm->isFlag(kGFSpringTaken)) {
					_vm->_gameSys->setAnimation(0x73, 40, 6);
					_vm->_gameSys->insertSequence(0x73, 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
					while (_vm->_gameSys->getAnimationStatus(6) != 2) {
						_vm->updateMouseCursor();
						// checkGameAppStatus();
						_vm->gameUpdateTick();
					}
					_s53_currHandSequenceId = 0x73;
					_vm->_gnapActionStatus = -1;
				}
				_vm->_isLeavingScene = true;
				_vm->_sceneDone = true;
				_vm->_gnapActionStatus = 0;
				_vm->_newSceneNum = 17;
			}
			break;
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

} // End of namespace Gnap
