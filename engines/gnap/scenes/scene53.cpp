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

int GnapEngine::scene53_init() {
	_gameSys->setAnimation(0, 0, 0);
	_gameSys->setAnimation(0, 0, 1);
	return 0x75;
}

void GnapEngine::scene53_updateHotspots() {
	setHotspot(0, 0, 0, 0, 0, SF_WALKABLE | SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR);
	setHotspot(2, 336, 361, 238, 270, SF_GRAB_CURSOR);
	setHotspot(3, 376, 405, 243, 274, SF_GRAB_CURSOR);
	setHotspot(4, 415, 441, 248, 276, SF_GRAB_CURSOR);
	setHotspot(5, 329, 358, 276, 303, SF_GRAB_CURSOR);
	setHotspot(6, 378, 408, 282, 311, SF_GRAB_CURSOR);
	setHotspot(7, 417, 446, 286, 319, SF_GRAB_CURSOR);
	setHotspot(8, 332, 361, 311, 342, SF_GRAB_CURSOR);
	setHotspot(9, 376, 407, 318, 349, SF_GRAB_CURSOR);
	setHotspot(10, 417, 447, 320, 353, SF_GRAB_CURSOR);
	setHotspot(11, 377, 405, 352, 384, SF_GRAB_CURSOR);
	setHotspot(12, 419, 450, 358, 394, SF_GRAB_CURSOR);
	setHotspot(13, 328, 359, 346, 379, SF_GRAB_CURSOR);
	setHotspot(14, 150, 650, 585, 600, SF_EXIT_D_CURSOR);
	setDeviceHotspot(1, -1, -1, -1, -1);
	_hotspotsCount = 15;
}

int GnapEngine::scene53_pressPhoneNumberButton(int phoneNumber, int buttonNum) {

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
		_gameSys->setAnimation(kGnapHandSequenceIds[buttonNum], 40, 6);
		_gameSys->insertSequence(kGnapHandSequenceIds[buttonNum], 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		_s53_currHandSequenceId = kGnapHandSequenceIds[buttonNum];
	} else {
		_gameSys->setAnimation(kPlatypusHandSequenceIds[buttonNum], 40, 6);
		_gameSys->insertSequence(kPlatypusHandSequenceIds[buttonNum], 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		_s53_currHandSequenceId = kPlatypusHandSequenceIds[buttonNum];
	}

	_gnapActionStatus = 6;
	while (_gameSys->getAnimationStatus(6) != 2) {
		// checkGameAppStatus();
		updateMouseCursor();
		gameUpdateTick();
	}
	_gnapActionStatus = -1;

	if (buttonNum < 11)
		phoneNumber = buttonNum % 10 + 10 * phoneNumber;

	return phoneNumber;
}

int GnapEngine::scene53_getRandomCallIndex() {
	int index, tries = 0;
	if (_s53_callsRndUsed == 0x7FFF)
		_s53_callsRndUsed = 0;
	do {
		index = getRandom(16);
		if (++tries == 300)
			_s53_callsRndUsed = 0;
	} while (_s53_callsRndUsed & (1 << index));
	_s53_callsRndUsed |= (1 << index);
	return index;
}

void GnapEngine::scene53_runRandomCall() {

	static const int kCallSequenceIds[15] = {
		0x60, 0x61, 0x62, 0x63, 0x64,
		0x65, 0x66, 0x67, 0x68, 0x69,
		0x6A, 0x6B, 0x6C, 0x6D, 0x71
	};

	++_s53_callsMadeCtr;

	if (_s53_callsMadeCtr <= 10) {
		int index;

		do {
			index = scene53_getRandomCallIndex();
		} while (!_s53_isGnapPhoning && (index == 0 || index == 3 || index == 4 || index == 11));
		_gameSys->setAnimation(kCallSequenceIds[index], 1, 6);
		_gameSys->insertSequence(kCallSequenceIds[index], 1, 0, 0, kSeqNone, 16, 0, 0);
	} else {
		_gameSys->setAnimation(0x74, 1, 6);
		_gameSys->insertSequence(0x74, 1, 0, 0, kSeqNone, 16, 0, 0);
		_s53_callsMadeCtr = 0;
	}
	
	_gnapActionStatus = 1;
	while (_gameSys->getAnimationStatus(6) != 2) {
		updateMouseCursor();
		// checkGameAppStatus();
		gameUpdateTick();
	}
	_gnapActionStatus = -1;

}

void GnapEngine::scene53_runChitChatLine() {
	bool flag = false;
	int sequenceId = -1;
	
	_gameSys->setAnimation(0x6E, 1, 6);
	_gameSys->insertSequence(0x6E, 1, 0, 0, kSeqNone, 16, 0, 0);

	_gnapActionStatus = 1;
	while (_gameSys->getAnimationStatus(6) != 2) {
		updateMouseCursor();
		// checkGameAppStatus();
		gameUpdateTick();
	}
	_gnapActionStatus = -1;

	if (isFlag(kGFSpringTaken)) {
		_gameSys->insertSequence(0x45, 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		_s53_currHandSequenceId = 0x45;
	} else {
		_gameSys->insertSequence(0x45, 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		_s53_currHandSequenceId = 0x5E;
	}
	
	_hotspots[1]._flags = SF_DISABLED;
	
	while (!flag) {
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		testWalk(0, 0, -1, -1, -1, -1);
		
		_sceneClickedHotspot = getClickedHotspotId();
		
		updateGrabCursorSprite(0, 0);
		
		switch (_sceneClickedHotspot) {
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
			_isLeavingScene = true;
			_sceneDone = true;
			_gnapActionStatus = 0;
			_newSceneNum = 17;
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
			scene53_pressPhoneNumberButton(0, _sceneClickedHotspot - 1);
			break;
		}
	
		if (flag && sequenceId != -1) {
			stopSound(0xA0);
			scene53_pressPhoneNumberButton(0, _sceneClickedHotspot - 1);
			_gnapActionStatus = 1;
			_gameSys->setAnimation(sequenceId, 1, 6);
			_gameSys->insertSequence(sequenceId, 1, 0, 0, kSeqNone, 16, 0, 0);
			_gnapActionStatus = 1;
			while (_gameSys->getAnimationStatus(6) != 2) {
				updateMouseCursor();
				// checkGameAppStatus();
				gameUpdateTick();
			}
			_gnapActionStatus = -1;
			_gameSys->setAnimation(0x72, 1, 6);
			_gameSys->insertSequence(0x72, 1, 0, 0, kSeqNone, 16, 0, 0);
			_gnapActionStatus = 1;
			while (_gameSys->getAnimationStatus(6) != 2) {
				updateMouseCursor();
				// checkGameAppStatus();
				gameUpdateTick();
			}
			_gnapActionStatus = -1;
		}
	
	}
	
	scene53_updateHotspots();
	
	_gnapActionStatus = 1;
	
	if (isFlag(kGFSpringTaken)) {
		_gameSys->setAnimation(0x73, 40, 6);
		_gameSys->insertSequence(0x73, 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
		while (_gameSys->getAnimationStatus(6) != 2) {
			updateMouseCursor();
			// checkGameAppStatus();
			gameUpdateTick();
		}
		_s53_currHandSequenceId = 0x73;
		_gnapActionStatus = -1;
	}

}

void GnapEngine::scene53_run() {
	int phoneNumber = 0;
	int phoneNumberLen = 0;
	
	queueInsertDeviceIcon();
	
	if (isFlag(kGFSpringTaken)) {
		_s53_currHandSequenceId = 0x45;
		_s53_isGnapPhoning = true;
	} else {
		_s53_currHandSequenceId = 0x5E;
		_s53_isGnapPhoning = false;
	}
	
	_gameSys->insertSequence(_s53_currHandSequenceId, 40, 0, 0, kSeqNone, 0, 0, 0);
	
	endSceneInit();
	
	setVerbCursor(GRAB_CURSOR);
	
	playSound(0xA0, 1);
	
	while (!_sceneDone) {
	
		updateMouseCursor();
		updateCursorByHotspot();
	
		testWalk(0, 0, -1, -1, -1, -1);
	
		_sceneClickedHotspot = getClickedHotspotId();
		updateGrabCursorSprite(0, 0);
	
		switch (_sceneClickedHotspot) {
		case 1:
			if (_gnapActionStatus < 0) {
				runMenu();
				scene53_updateHotspots();
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
			stopSound(0xA0);
			++phoneNumberLen;
			phoneNumber = scene53_pressPhoneNumberButton(phoneNumber, _sceneClickedHotspot - 1);
			debug("phoneNumber: %d", phoneNumber);
			if (phoneNumberLen == 7) {
				_gnapActionStatus = 1;
				if (isFlag(kGFSpringTaken)) {
					_gameSys->setAnimation(0x73, 40, 6);
					_gameSys->insertSequence(0x73, 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
					while (_gameSys->getAnimationStatus(6) != 2) {
						updateMouseCursor();
						// checkGameAppStatus();
						gameUpdateTick();
					}
					_s53_currHandSequenceId = 0x73;
					_gnapActionStatus = -1;
				}
				if (phoneNumber == 7284141) {
					scene53_runChitChatLine();
					phoneNumber = 0;
					phoneNumberLen = 0;
					_sceneDone = true;
					_newSceneNum = 17;
				} else if (phoneNumber != 5556789 || isFlag(kGFPictureTaken)) {				
					scene53_runRandomCall();
					phoneNumber = 0;
					phoneNumberLen = 0;
					_sceneDone = true;
					_newSceneNum = 17;
				} else {
					phoneNumber = 0;
					phoneNumberLen = 0;
					_sceneDone = true;
					_newSceneNum = 17;
					if (_s53_isGnapPhoning)
						setFlag(kGFUnk25);
					else
						setFlag(kGFPlatypusTalkingToAssistant);
				}
			}
			break;
		case 12:
		case 13:
			scene53_pressPhoneNumberButton(0, _sceneClickedHotspot - 1);
			break;
		case 14:
			if (_gnapActionStatus < 0) {
				_gnapActionStatus = 1;
				if (isFlag(kGFSpringTaken)) {
					_gameSys->setAnimation(0x73, 40, 6);
					_gameSys->insertSequence(0x73, 40, _s53_currHandSequenceId, 40, kSeqSyncWait, 0, 0, 0);
					while (_gameSys->getAnimationStatus(6) != 2) {
						updateMouseCursor();
						// checkGameAppStatus();
						gameUpdateTick();
					}
					_s53_currHandSequenceId = 0x73;
					_gnapActionStatus = -1;
				}
				_isLeavingScene = true;
				_sceneDone = true;
				_gnapActionStatus = 0;
				_newSceneNum = 17;
			}
			break;
		}
	
		checkGameKeys();
	
		if (isKeyStatus1(8)) {
			clearKeyStatus1(8);
			runMenu();
			scene53_updateHotspots();
		}
		
		gameUpdateTick();
	
	}
	
}

} // End of namespace Gnap
