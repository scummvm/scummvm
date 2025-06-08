/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "lastexpress/lastexpress.h"

namespace LastExpress {

void LogicManager::CONS_DemoMadame(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMadame,
			_functionsDemoMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]
		);

		break;
	case 1:
		CONS_DemoMadame_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoMadame_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoMadame_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoMadame_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoMadame_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::DemoMadameCall(CALL_PARAMS) {
	getCharacter(kCharacterMadame).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_DemoMadame_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_DemoMadame_DoDialog);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 1;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_DemoMadame_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterMadame).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMadame, _functionsDemoMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]);
		fedEx(kCharacterMadame, kCharacterMadame, 18, 0);
		break;
	case 12:
		playDialog(kCharacterMadame, (char *)&getCharacterCurrentParams(kCharacterMadame)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoMadame_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_DemoMadame_Birth);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 2;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_DemoMadame_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMadame)[2]) {
			if (getCharacterCurrentParams(kCharacterMadame)[3] ||
				(getCharacterCurrentParams(kCharacterMadame)[3] = _realTime + 75,
					_realTime != -75)) {

				if (_realTime <= getCharacterCurrentParams(kCharacterMadame)[3])
					return;

				getCharacterCurrentParams(kCharacterMadame)[3] = 0x7FFFFFFF;
			}
			getCharacterCurrentParams(kCharacterMadame)[2] = 0;
			getCharacterCurrentParams(kCharacterMadame)[1] = 1;
			setDoor(33, kCharacterMadame, 1, 0, 0);
			getCharacterCurrentParams(kCharacterMadame)[3] = 0;
		} else {
			getCharacterCurrentParams(kCharacterMadame)[3] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterMadame)[2]) {
			setDoor(33, kCharacterMadame, 1, 0, 0);
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 5;
			DemoMadameCall(&LogicManager::CONS_DemoMadame_DoDialog, getCathSorryDialog(), 0, 0, 0);
		} else {
			getCharacterCurrentParams(kCharacterMadame)[0]++;
			setDoor(33, kCharacterMadame, 1, 0, 0);
			if (msg->action == 9) {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
				DemoMadameCall(&LogicManager::CONS_DemoMadame_DoDialog, "LIB013", 0, 0, 0);
			} else {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
				DemoMadameCall(&LogicManager::CONS_DemoMadame_DoDialog, "LIB012", 0, 0, 0);
			}
		}

		break;
	case 12:
		getCharacter(kCharacterMadame).characterPosition.car = 4;
		getCharacter(kCharacterMadame).characterPosition.location = 1;
		getCharacter(kCharacterMadame).characterPosition.position = 7500;
		setDoor(33, kCharacterMadame, 1, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterMadame)[1] || getCharacterCurrentParams(kCharacterMadame)[2]) {
			setDoor(33, kCharacterMadame, 1, 10, 9);
			getCharacterCurrentParams(kCharacterMadame)[1] = 0;
			getCharacterCurrentParams(kCharacterMadame)[2] = 0;
			getCharacterCurrentParams(kCharacterMadame)[0] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8]) {
		case 1:
		case 2:
			if (getCharacterCurrentParams(kCharacterMadame)[0] <= 1) {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 4;
				DemoMadameCall(&LogicManager::CONS_DemoMadame_DoDialog, "MME1038", 0, 0, 0);
			} else {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 3;
				DemoMadameCall(&LogicManager::CONS_DemoMadame_DoDialog, "MME1038C", 0, 0, 0);
			}

			break;
		case 3:
		case 4:
			setDoor(33, kCharacterMadame, 1, 14, 0);
			getCharacterCurrentParams(kCharacterMadame)[2] = 1;
			break;
		case 5:
			getCharacterCurrentParams(kCharacterMadame)[2] = 0;
			getCharacterCurrentParams(kCharacterMadame)[1] = 1;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoMadame_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_DemoMadame_StartPart2);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 3;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_DemoMadame_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoMadame_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_DemoMadame_StartPart3);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 4;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_DemoMadame_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoMadame_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_DemoMadame_StartPart4);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 5;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_DemoMadame_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoMadame_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_DemoMadame_StartPart5);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 6;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_DemoMadame_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoMadame[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoMadame_DoDialog,
	&LogicManager::HAND_DemoMadame_Birth,
	&LogicManager::HAND_DemoMadame_StartPart2,
	&LogicManager::HAND_DemoMadame_StartPart3,
	&LogicManager::HAND_DemoMadame_StartPart4,
	&LogicManager::HAND_DemoMadame_StartPart5
};

} // End of namespace LastExpress
