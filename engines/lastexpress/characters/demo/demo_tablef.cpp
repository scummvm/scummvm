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

void LogicManager::CONS_DemoTableF(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterTableF,
			_functionsDemoTableF[getCharacter(kCharacterTableF).callbacks[getCharacter(kCharacterTableF).currentCall]]
		);

		break;
	case 1:
		CONS_DemoTableF_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoTableF_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoTableF_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoTableF_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoTableF_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoTableF_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableF).callParams[getCharacter(kCharacterTableF).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableF, &LogicManager::HAND_DemoTableF_Birth);
	getCharacter(kCharacterTableF).callbacks[getCharacter(kCharacterTableF).currentCall] = 1;

	params->clear();

	fedEx(kCharacterTableF, kCharacterTableF, 12, 0);
}

void LogicManager::HAND_DemoTableF_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTableF).characterPosition.position = 5420;
		getCharacter(kCharacterTableF).characterPosition.location = 1;
		getCharacter(kCharacterTableF).characterPosition.car = kCarRestaurant;
		startCycOtis(kCharacterTableF, "024D");
		break;
	case 103798704:
		if (msg->param.stringParam) {
			startCycOtis(kCharacterTableF, msg->param.stringParam);
		} else {
			startCycOtis(kCharacterTableF, "024D");
		}

		break;
	case 136455232:
		startCycOtis(kCharacterTableF, "BLANK");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoTableF_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableF).callParams[getCharacter(kCharacterTableF).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableF, &LogicManager::HAND_DemoTableF_StartPart2);
	getCharacter(kCharacterTableF).callbacks[getCharacter(kCharacterTableF).currentCall] = 2;

	params->clear();

	fedEx(kCharacterTableF, kCharacterTableF, 12, 0);
}

void LogicManager::HAND_DemoTableF_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoTableF_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableF).callParams[getCharacter(kCharacterTableF).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableF, &LogicManager::HAND_DemoTableF_StartPart3);
	getCharacter(kCharacterTableF).callbacks[getCharacter(kCharacterTableF).currentCall] = 3;

	params->clear();

	fedEx(kCharacterTableF, kCharacterTableF, 12, 0);
}

void LogicManager::HAND_DemoTableF_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoTableF_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableF).callParams[getCharacter(kCharacterTableF).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableF, &LogicManager::HAND_DemoTableF_StartPart4);
	getCharacter(kCharacterTableF).callbacks[getCharacter(kCharacterTableF).currentCall] = 4;

	params->clear();

	fedEx(kCharacterTableF, kCharacterTableF, 12, 0);
}

void LogicManager::HAND_DemoTableF_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoTableF_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableF).callParams[getCharacter(kCharacterTableF).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableF, &LogicManager::HAND_DemoTableF_StartPart5);
	getCharacter(kCharacterTableF).callbacks[getCharacter(kCharacterTableF).currentCall] = 5;

	params->clear();

	fedEx(kCharacterTableF, kCharacterTableF, 12, 0);
}

void LogicManager::HAND_DemoTableF_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoTableF[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoTableF_Birth,
	&LogicManager::HAND_DemoTableF_StartPart2,
	&LogicManager::HAND_DemoTableF_StartPart3,
	&LogicManager::HAND_DemoTableF_StartPart4,
	&LogicManager::HAND_DemoTableF_StartPart5,
};

} // End of namespace LastExpress
