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

void LogicManager::CONS_TableD(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterTableD,
			_functionsTableD[getCharacter(kCharacterTableD).callbacks[getCharacter(kCharacterTableD).currentCall]]
		);

		break;
	case 1:
		CONS_TableD_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_TableD_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_TableD_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_TableD_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_TableD_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableD_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableD).callParams[getCharacter(kCharacterTableD).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableD, &LogicManager::HAND_TableD_Birth);
	getCharacter(kCharacterTableD).callbacks[getCharacter(kCharacterTableD).currentCall] = 1;

	params->clear();

	fedEx(kCharacterTableD, kCharacterTableD, 12, 0);
}

void LogicManager::HAND_TableD_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_TableD_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableD_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableD).callParams[getCharacter(kCharacterTableD).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableD, &LogicManager::HAND_TableD_StartPart2);
	getCharacter(kCharacterTableD).callbacks[getCharacter(kCharacterTableD).currentCall] = 2;

	params->clear();

	fedEx(kCharacterTableD, kCharacterTableD, 12, 0);
}

void LogicManager::HAND_TableD_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_TableD_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableD_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableD).callParams[getCharacter(kCharacterTableD).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableD, &LogicManager::HAND_TableD_StartPart3);
	getCharacter(kCharacterTableD).callbacks[getCharacter(kCharacterTableD).currentCall] = 3;

	params->clear();

	fedEx(kCharacterTableD, kCharacterTableD, 12, 0);
}

void LogicManager::HAND_TableD_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_TableD_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableD_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableD).callParams[getCharacter(kCharacterTableD).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableD, &LogicManager::HAND_TableD_StartPart4);
	getCharacter(kCharacterTableD).callbacks[getCharacter(kCharacterTableD).currentCall] = 4;

	params->clear();

	fedEx(kCharacterTableD, kCharacterTableD, 12, 0);
}

void LogicManager::HAND_TableD_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_TableD_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableD_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableD).callParams[getCharacter(kCharacterTableD).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableD, &LogicManager::HAND_TableD_StartPart5);
	getCharacter(kCharacterTableD).callbacks[getCharacter(kCharacterTableD).currentCall] = 5;

	params->clear();

	fedEx(kCharacterTableD, kCharacterTableD, 12, 0);
}

void LogicManager::HAND_TableD_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_TableD_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableD_Idling(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableD).callParams[getCharacter(kCharacterTableD).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableD, &LogicManager::HAND_TableD_Idling);
	getCharacter(kCharacterTableD).callbacks[getCharacter(kCharacterTableD).currentCall] = 6;

	params->clear();

	fedEx(kCharacterTableD, kCharacterTableD, 12, 0);
}

void LogicManager::HAND_TableD_Idling(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTableD).characterPosition.position = 4690;
		getCharacter(kCharacterTableD).characterPosition.location = 1;
		getCharacter(kCharacterTableD).characterPosition.car = kCarRestaurant;
		startCycOtis(kCharacterTableD, "010M");
		break;
	case 103798704:
		if (msg->param.stringParam) {
			startCycOtis(kCharacterTableD, msg->param.stringParam);
		} else {
			startCycOtis(kCharacterTableD, "010M");
		}

		break;
	case 136455232:
		startCycOtis(kCharacterTableD, "BLANK");
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsTableD[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_TableD_Birth,
	&LogicManager::HAND_TableD_StartPart2,
	&LogicManager::HAND_TableD_StartPart3,
	&LogicManager::HAND_TableD_StartPart4,
	&LogicManager::HAND_TableD_StartPart5,
	&LogicManager::HAND_TableD_Idling
};

} // End of namespace LastExpress
