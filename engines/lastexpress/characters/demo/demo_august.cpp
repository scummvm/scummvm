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

void LogicManager::CONS_DemoAugust(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterAugust,
			_functionsDemoAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]
		);

		break;
	case 1:
		CONS_DemoAugust_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoAugust_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoAugust_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoAugust_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoAugust_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::DemoAugustCall(CALL_PARAMS) {
	getCharacter(kCharacterAugust).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_DemoAugust_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_DemoAugust_SaveGame);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 1;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_DemoAugust_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsDemoAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			break;
	case 12:
		save(
			kCharacterAugust,
			getCharacterCurrentParams(kCharacterAugust)[0],
			getCharacterCurrentParams(kCharacterAugust)[1]
		);

		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsDemoAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAugust_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_DemoAugust_Birth);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 2;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_DemoAugust_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAugust)[2] && !getCharacterCurrentParams(kCharacterAugust)[0]) {
			getCharacter(kCharacterAugust).inventoryItem = 0x80;
		} else {
			getCharacter(kCharacterAugust).inventoryItem = 0;
		}

		break;
	case 1:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		DemoAugustCall(&LogicManager::CONS_DemoAugust_SaveGame, 2, kEventAugustTalkCigar, 0, 0);
		break;
	case 3:
		releaseView(kCharacterAugust, kCarRestaurant, 50);
		startCycOtis(kCharacterAugust, "105B3");
		break;
	case 12:
		getCharacter(kCharacterAugust).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterAugust).characterPosition.position = 3450;
		getCharacter(kCharacterAugust).characterPosition.location = 1;
		send(kCharacterAugust, kCharacterAbbot, 123712592, 0);
		startCycOtis(kCharacterAugust, "105B3");
		getCharacterCurrentParams(kCharacterAugust)[2] = 1;
		break;
	case 17:
		if (checkCathDir(kCarRestaurant, 60) && !getCharacterCurrentParams(kCharacterAugust)[1] && !getCharacterCurrentParams(kCharacterAugust)[0]) {
			blockView(kCharacterAugust, kCarRestaurant, 50);
			startSeqOtis(kCharacterAugust, "105C3");
		}

		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 1) {
			playNIS(kEventAugustTalkCigar);
			startCycOtis(kCharacterAugust, "105B3");
			cleanNIS();
			getCharacterCurrentParams(kCharacterAugust)[2] = 0;
		}

		break;
	case 122358304:
		startCycOtis(kCharacterAugust, "BLANK");
		getCharacterCurrentParams(kCharacterAugust)[0] = 1;
		getCharacterCurrentParams(kCharacterAugust)[1] = 1;
		break;
	case 136196244:
		getCharacterCurrentParams(kCharacterAugust)[0] = 1;
		getCharacter(kCharacterAugust).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAugust_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_DemoAugust_StartPart2);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 3;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_DemoAugust_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoAugust_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_DemoAugust_StartPart3);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 4;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_DemoAugust_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoAugust_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_DemoAugust_StartPart4);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 5;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_DemoAugust_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoAugust_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_DemoAugust_StartPart5);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 35;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_DemoAugust_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoAugust[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoAugust_SaveGame,
	&LogicManager::HAND_DemoAugust_Birth,
	&LogicManager::HAND_DemoAugust_StartPart2,
	&LogicManager::HAND_DemoAugust_StartPart3,
	&LogicManager::HAND_DemoAugust_StartPart4,
	&LogicManager::HAND_DemoAugust_StartPart5
};

} // End of namespace LastExpress
