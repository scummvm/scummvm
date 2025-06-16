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

void LogicManager::CONS_TableA(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterTableA,
			_functionsTableA[getCharacter(kCharacterTableA).callbacks[getCharacter(kCharacterTableA).currentCall]]
		);

		break;
	case 1:
		CONS_TableA_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_TableA_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_TableA_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_TableA_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_TableA_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableA_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableA).callParams[getCharacter(kCharacterTableA).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableA, &LogicManager::HAND_TableA_Birth);
	getCharacter(kCharacterTableA).callbacks[getCharacter(kCharacterTableA).currentCall] = 1;

	params->clear();

	fedEx(kCharacterTableA, kCharacterTableA, 12, 0);
}

void LogicManager::HAND_TableA_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_TableA_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableA_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableA).callParams[getCharacter(kCharacterTableA).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableA, &LogicManager::HAND_TableA_StartPart2);
	getCharacter(kCharacterTableA).callbacks[getCharacter(kCharacterTableA).currentCall] = 2;

	params->clear();

	fedEx(kCharacterTableA, kCharacterTableA, 12, 0);
}

void LogicManager::HAND_TableA_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_TableA_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableA_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableA).callParams[getCharacter(kCharacterTableA).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableA, &LogicManager::HAND_TableA_StartPart3);
	getCharacter(kCharacterTableA).callbacks[getCharacter(kCharacterTableA).currentCall] = 3;

	params->clear();

	fedEx(kCharacterTableA, kCharacterTableA, 12, 0);
}

void LogicManager::HAND_TableA_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_TableA_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableA_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableA).callParams[getCharacter(kCharacterTableA).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableA, &LogicManager::HAND_TableA_StartPart4);
	getCharacter(kCharacterTableA).callbacks[getCharacter(kCharacterTableA).currentCall] = 4;

	params->clear();

	fedEx(kCharacterTableA, kCharacterTableA, 12, 0);
}

void LogicManager::HAND_TableA_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_TableA_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableA_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableA).callParams[getCharacter(kCharacterTableA).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableA, &LogicManager::HAND_TableA_StartPart5);
	getCharacter(kCharacterTableA).callbacks[getCharacter(kCharacterTableA).currentCall] = 5;

	params->clear();

	fedEx(kCharacterTableA, kCharacterTableA, 12, 0);
}

void LogicManager::HAND_TableA_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_TableA_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableA_Idling(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableA).callParams[getCharacter(kCharacterTableA).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableA, &LogicManager::HAND_TableA_Idling);
	getCharacter(kCharacterTableA).callbacks[getCharacter(kCharacterTableA).currentCall] = 6;

	params->clear();

	fedEx(kCharacterTableA, kCharacterTableA, 12, 0);
}

void LogicManager::HAND_TableA_Idling(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTableA).characterPosition.position = 3970;
		getCharacter(kCharacterTableA).characterPosition.location = 1;
		getCharacter(kCharacterTableA).characterPosition.car = kCarRestaurant;
		startCycOtis(kCharacterTableA, "001P");
		break;
	case 103798704:
		if (msg->param.stringParam) {
			startCycOtis(kCharacterTableA, msg->param.stringParam);
		} else {
			startCycOtis(kCharacterTableA, "001P");
		}

		break;
	case 136455232:
		startCycOtis(kCharacterTableA, "BLANK");
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsTableA[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_TableA_Birth,
	&LogicManager::HAND_TableA_StartPart2,
	&LogicManager::HAND_TableA_StartPart3,
	&LogicManager::HAND_TableA_StartPart4,
	&LogicManager::HAND_TableA_StartPart5,
	&LogicManager::HAND_TableA_Idling
};

} // End of namespace LastExpress
