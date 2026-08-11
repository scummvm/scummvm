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

void LogicManager::CONS_DemoTatiana(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterTatiana,
			_functionsDemoTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]
		);

		break;
	case 1:
		CONS_DemoTatiana_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoTatiana_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoTatiana_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoTatiana_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoTatiana_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoTatiana_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_DemoTatiana_Birth);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 1;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_DemoTatiana_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterTatiana)[1] && !getCharacterCurrentParams(kCharacterTatiana)[3]) {
			getCharacterCurrentParams(kCharacterTatiana)[0] -= _timeSpeed;
			if (getCharacterCurrentParams(kCharacterTatiana)[0] < _timeSpeed) {
				startCycOtis(kCharacterTatiana, (char *)&getCharacterCurrentParams(kCharacterTatiana)[7]);
				playDialog(kCharacterTatiana, (char *)&getCharacterCurrentParams(kCharacterTatiana)[4], -1, 0);
				getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
			}
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterTatiana)[1] = 0;
		getCharacterCurrentParams(kCharacterTatiana)[2]++;
		switch (getCharacterCurrentParams(kCharacterTatiana)[2]) {
		case 1:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 900;
			startCycOtis(kCharacterTatiana, "110A");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[4], 12, "Tat3160B");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[7], 12, "110A");
			break;
		case 2:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 2700;
			startCycOtis(kCharacterTatiana, "110B");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[4], 12, "Tat3160D");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[7], 12, "110D");
			break;
		case 3:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 4500;
			startCycOtis(kCharacterTatiana, "110B");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[4], 12, "Tat3160E");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[7], 12, "110D");
			break;
		default:
			getCharacterCurrentParams(kCharacterTatiana)[3] = 1;
			break;
		}

		break;
	case 12:
		getCharacter(kCharacterTatiana).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterTatiana).characterPosition.location = 1;
		getCharacter(kCharacterTatiana).characterPosition.position = 1750;
		startCycOtis(kCharacterTatiana, "110C");
		playDialog(kCharacterTatiana, "Tat3160A", -1, 0);
		getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoTatiana_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_DemoTatiana_StartPart2);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 2;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_DemoTatiana_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoTatiana_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_DemoTatiana_StartPart3);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 3;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_DemoTatiana_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoTatiana_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_DemoTatiana_StartPart4);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 4;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_DemoTatiana_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoTatiana_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_DemoTatiana_StartPart5);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 5;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_DemoTatiana_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoTatiana[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoTatiana_Birth,
	&LogicManager::HAND_DemoTatiana_StartPart2,
	&LogicManager::HAND_DemoTatiana_StartPart3,
	&LogicManager::HAND_DemoTatiana_StartPart4,
	&LogicManager::HAND_DemoTatiana_StartPart5
};

} // End of namespace LastExpress
