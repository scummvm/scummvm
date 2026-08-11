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

void LogicManager::CONS_DemoTableE(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterTableE,
			_functionsDemoTableE[getCharacter(kCharacterTableE).callbacks[getCharacter(kCharacterTableE).currentCall]]
		);

		break;
	case 1:
		CONS_DemoTableE_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoTableE_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoTableE_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoTableE_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoTableE_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoTableE_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableE).callParams[getCharacter(kCharacterTableE).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableE, &LogicManager::HAND_DemoTableE_Birth);
	getCharacter(kCharacterTableE).callbacks[getCharacter(kCharacterTableE).currentCall] = 1;

	params->clear();

	fedEx(kCharacterTableE, kCharacterTableE, 12, 0);
}

void LogicManager::HAND_DemoTableE_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTableE).characterPosition.position = 5420;
		getCharacter(kCharacterTableE).characterPosition.location = 1;
		getCharacter(kCharacterTableE).characterPosition.car = kCarRestaurant;
		startCycOtis(kCharacterTableE, "014F");
		break;
	case 103798704:
		if (msg->param.stringParam) {
			startCycOtis(kCharacterTableE, msg->param.stringParam);
		} else {
			startCycOtis(kCharacterTableE, "014F");
		}

		break;
	case 136455232:
		startCycOtis(kCharacterTableE, "BLANK");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoTableE_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableE).callParams[getCharacter(kCharacterTableE).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableE, &LogicManager::HAND_DemoTableE_StartPart2);
	getCharacter(kCharacterTableE).callbacks[getCharacter(kCharacterTableE).currentCall] = 2;

	params->clear();

	fedEx(kCharacterTableE, kCharacterTableE, 12, 0);
}

void LogicManager::HAND_DemoTableE_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoTableE_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableE).callParams[getCharacter(kCharacterTableE).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableE, &LogicManager::HAND_DemoTableE_StartPart3);
	getCharacter(kCharacterTableE).callbacks[getCharacter(kCharacterTableE).currentCall] = 3;

	params->clear();

	fedEx(kCharacterTableE, kCharacterTableE, 12, 0);
}

void LogicManager::HAND_DemoTableE_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoTableE_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableE).callParams[getCharacter(kCharacterTableE).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableE, &LogicManager::HAND_DemoTableE_StartPart4);
	getCharacter(kCharacterTableE).callbacks[getCharacter(kCharacterTableE).currentCall] = 4;

	params->clear();

	fedEx(kCharacterTableE, kCharacterTableE, 12, 0);
}

void LogicManager::HAND_DemoTableE_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoTableE_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableE).callParams[getCharacter(kCharacterTableE).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableE, &LogicManager::HAND_DemoTableE_StartPart5);
	getCharacter(kCharacterTableE).callbacks[getCharacter(kCharacterTableE).currentCall] = 5;

	params->clear();

	fedEx(kCharacterTableE, kCharacterTableE, 12, 0);
}

void LogicManager::HAND_DemoTableE_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoTableE[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoTableE_Birth,
	&LogicManager::HAND_DemoTableE_StartPart2,
	&LogicManager::HAND_DemoTableE_StartPart3,
	&LogicManager::HAND_DemoTableE_StartPart4,
	&LogicManager::HAND_DemoTableE_StartPart5,
};

} // End of namespace LastExpress
