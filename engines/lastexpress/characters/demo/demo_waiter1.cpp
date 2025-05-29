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

void LogicManager::CONS_DemoWaiter1(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterWaiter1,
			_functionsDemoWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]
		);

		break;
	case 1:
		CONS_DemoWaiter1_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoWaiter1_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoWaiter1_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoWaiter1_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoWaiter1_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::DemoWaiter1Call(CALL_PARAMS) {
	getCharacter(kCharacterWaiter1).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_DemoWaiter1_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_DemoWaiter1_DoSeqOtis);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 1;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_DemoWaiter1_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterWaiter1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsDemoWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
		fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterWaiter1)[3]) {
			playChrExcuseMe(kCharacterWaiter1, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterWaiter1)[3] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterWaiter1, (char *)&getCharacterCurrentParams(kCharacterWaiter1)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoWaiter1_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_DemoWaiter1_Birth);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 2;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_DemoWaiter1_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		break;
	case 223712416:
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		DemoWaiter1Call(&LogicManager::CONS_DemoWaiter1_RebeccaFeedUs, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoWaiter1_RebeccaFeedUs(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_DemoWaiter1_RebeccaFeedUs);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 3;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_DemoWaiter1_RebeccaFeedUs(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		DemoWaiter1Call(&LogicManager::CONS_DemoWaiter1_DoSeqOtis, "911", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] == 1) {
			endGraphics(kCharacterWaiter1);
			send(kCharacterWaiter1, kCharacterRebecca, 123712592, 0);
		} else if (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] == 2) {
			endGraphics(kCharacterWaiter1);
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
		}

		break;
	case 136702400:
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
		DemoWaiter1Call(&LogicManager::CONS_DemoWaiter1_DoSeqOtis, "913", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoWaiter1_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_DemoWaiter1_StartPart2);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 4;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_DemoWaiter1_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoWaiter1_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_DemoWaiter1_StartPart3);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 5;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_DemoWaiter1_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoWaiter1_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_DemoWaiter1_StartPart4);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 6;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_DemoWaiter1_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoWaiter1_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_DemoWaiter1_StartPart5);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 7;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_DemoWaiter1_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoWaiter1[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoWaiter1_DoSeqOtis,
	&LogicManager::HAND_DemoWaiter1_Birth,
	&LogicManager::HAND_DemoWaiter1_RebeccaFeedUs,
	&LogicManager::HAND_DemoWaiter1_StartPart2,
	&LogicManager::HAND_DemoWaiter1_StartPart3,
	&LogicManager::HAND_DemoWaiter1_StartPart4,
	&LogicManager::HAND_DemoWaiter1_StartPart5
};

} // End of namespace LastExpress
