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

void LogicManager::CONS_DemoCond2(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterCond2,
			_functionsDemoCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]
		);

		break;
	case 1:
		CONS_DemoCond2_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoCond2_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoCond2_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoCond2_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoCond2_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::DemoCond2Call(CALL_PARAMS) {
	getCharacter(kCharacterCond2).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_DemoCond2_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_DemoCond2_DoSeqOtis);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 1;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_DemoCond2_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsDemoCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterCond2, (char *)&getCharacterCurrentParams(kCharacterCond2)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoCond2_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_DemoCond2_FinishSeqOtis);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 2;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_DemoCond2_FinishSeqOtis(HAND_PARAMS) {
	if (msg->action == 3) {
		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsDemoCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
	}
}

void LogicManager::CONS_DemoCond2_SitDown(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_DemoCond2_SitDown);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 3;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_DemoCond2_SitDown(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterCurrentParams(kCharacterCond2)[0]) {
			startSeqOtis(kCharacterCond2, "697H");
		} else {
			startSeqOtis(kCharacterCond2, "627A");
		}

		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		DemoCond2Call(&LogicManager::CONS_DemoCond2_FinishSeqOtis, 0, 0, 0, 0);

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterCond2, "627B");

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsDemoCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoCond2_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_DemoCond2_Birth);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 4;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_DemoCond2_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterCond2).characterPosition.position = 1500;
		getCharacter(kCharacterCond2).characterPosition.location = 0;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		DemoCond2Call(&LogicManager::CONS_DemoCond2_SitDown, 0, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 2) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			DemoCond2Call(&LogicManager::CONS_DemoCond2_SitDown, 0, 0, 0, 0);
		}

		break;
	case 168253822:
		if (whoOnScreen(kCharacterCond2) && !getCharacterCurrentParams(kCharacterCond2)[0]) {
			getCharacterCurrentParams(kCharacterCond2)[0] = 1;
			playDialog(kCharacterCond2, "JAC1120", -1, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			DemoCond2Call(&LogicManager::CONS_DemoCond2_DoSeqOtis, "697D", 0, 0, 0);
		}

		break;
	case 225932896:
		send(kCharacterCond2, kCharacterFrancois, 205346192, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoCond2_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_DemoCond2_StartPart2);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 5;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_DemoCond2_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoCond2_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_DemoCond2_StartPart3);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 6;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_DemoCond2_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoCond2_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_DemoCond2_StartPart4);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 7;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_DemoCond2_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoCond2_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_DemoCond2_StartPart5);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 8;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_DemoCond2_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoCond2[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoCond2_DoSeqOtis,
	&LogicManager::HAND_DemoCond2_FinishSeqOtis,
	&LogicManager::HAND_DemoCond2_SitDown,
	&LogicManager::HAND_DemoCond2_Birth,
	&LogicManager::HAND_DemoCond2_StartPart2,
	&LogicManager::HAND_DemoCond2_StartPart3,
	&LogicManager::HAND_DemoCond2_StartPart4,
	&LogicManager::HAND_DemoCond2_StartPart5
};

} // End of namespace LastExpress
