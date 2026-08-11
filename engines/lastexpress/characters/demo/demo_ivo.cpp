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

void LogicManager::CONS_DemoIvo(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterIvo,
			_functionsDemoIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]
		);

		break;
	case 1:
		CONS_DemoIvo_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoIvo_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoIvo_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoIvo_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoIvo_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::DemoIvoCall(CALL_PARAMS) {
	getCharacter(kCharacterIvo).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_DemoIvo_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_DemoIvo_WaitRCClear);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 1;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_DemoIvo_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterIvo).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsDemoIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
			fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoIvo_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_DemoIvo_DoSeqOtis);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_DemoIvo_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsDemoIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterIvo, (char *)&getCharacterCurrentParams(kCharacterIvo)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoIvo_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_DemoIvo_FinishSeqOtis);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 3;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_DemoIvo_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsDemoIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoIvo_DoSplitOtis023A(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_DemoIvo_DoSplitOtis023A);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 4;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_DemoIvo_DoSplitOtis023A(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		endGraphics(kCharacterTableD);
		send(kCharacterIvo, kCharacterTableC, 136455232, 0);

		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsDemoIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterIvo, "023A1");
		startSeqOtis(kCharacterTableD, "023A2");
		startSeqOtis(kCharacterTableC, "023A3");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoIvo_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_DemoIvo_Birth);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 5;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_DemoIvo_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterIvo).characterPosition.position = 1539;
		getCharacter(kCharacterIvo).characterPosition.location = 0;
		getCharacter(kCharacterIvo).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 1;
		DemoIvoCall(&LogicManager::CONS_DemoIvo_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 2;
			DemoIvoCall(&LogicManager::CONS_DemoIvo_DoSeqOtis, "809US", 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 3;
			DemoIvoCall(&LogicManager::CONS_DemoIvo_DoSplitOtis023A, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterIvo).characterPosition.location = 1;
			startCycOtis(kCharacterIvo, "023B");
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoIvo_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_DemoIvo_StartPart2);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 6;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_DemoIvo_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoIvo_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_DemoIvo_StartPart3);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 7;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_DemoIvo_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoIvo_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_DemoIvo_StartPart4);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 8;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_DemoIvo_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoIvo_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_DemoIvo_StartPart5);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 9;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_DemoIvo_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoIvo[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoIvo_WaitRCClear,
	&LogicManager::HAND_DemoIvo_DoSeqOtis,
	&LogicManager::HAND_DemoIvo_FinishSeqOtis,
	&LogicManager::HAND_DemoIvo_DoSplitOtis023A,
	&LogicManager::HAND_DemoIvo_Birth,
	&LogicManager::HAND_DemoIvo_StartPart2,
	&LogicManager::HAND_DemoIvo_StartPart3,
	&LogicManager::HAND_DemoIvo_StartPart4,
	&LogicManager::HAND_DemoIvo_StartPart5
};

} // End of namespace LastExpress
