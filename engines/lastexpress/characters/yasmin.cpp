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

void LogicManager::CONS_Yasmin(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterYasmin,
			_functionsYasmin[getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall]]);

		break;
	case 1:
		CONS_Yasmin_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Yasmin_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Yasmin_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Yasmin_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Yasmin_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::YasminCall(CALL_PARAMS) {
	getCharacter(kCharacterYasmin).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Yasmin_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_DebugWalks);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 1;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterYasmin, kCarGreenSleeping, getCharacterCurrentParams(kCharacterYasmin)[0])) {
			if (getCharacterCurrentParams(kCharacterYasmin)[0] == 10000)
				getCharacterCurrentParams(kCharacterYasmin)[0] = 0;
			else
				getCharacterCurrentParams(kCharacterYasmin)[0] = 10000;
		}

		break;
	case 5:
		playCathExcuseMe();
		break;
	case 6:
		playChrExcuseMe(kCharacterYasmin, kCharacterCath, 0);
		break;
	case 12:
		getCharacter(kCharacterYasmin).characterPosition.position = 0;
		getCharacter(kCharacterYasmin).characterPosition.location = 0;
		getCharacter(kCharacterYasmin).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterYasmin)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_DoCorrOtis);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterYasmin, getCharacterCurrentParams(kCharacterYasmin)[3]);
		getCharacter(kCharacterYasmin).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, _functionsYasmin[getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall]]);
		fedEx(kCharacterYasmin, kCharacterYasmin, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterYasmin, (char *)&getCharacterCurrentParams(kCharacterYasmin)[0]);
		blockAtDoor(kCharacterYasmin, getCharacterCurrentParams(kCharacterYasmin)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_DoDialog);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterYasmin).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, _functionsYasmin[getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall]]);
		fedEx(kCharacterYasmin, kCharacterYasmin, 18, 0);
		break;
	case 12:
		playDialog(kCharacterYasmin, (char *)&getCharacterCurrentParams(kCharacterYasmin)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_DoWait);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 4;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterYasmin)[1] || (getCharacterCurrentParams(kCharacterYasmin)[1] = _gameTime + getCharacterCurrentParams(kCharacterYasmin)[0], _gameTime + getCharacterCurrentParams(kCharacterYasmin)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterYasmin)[1] >= _gameTime)
				return;

			getCharacterCurrentParams(kCharacterYasmin)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterYasmin).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, _functionsYasmin[getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall]]);
		fedEx(kCharacterYasmin, kCharacterYasmin, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_DoWalk);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 5;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterYasmin, getCharacterCurrentParams(kCharacterYasmin)[0], getCharacterCurrentParams(kCharacterYasmin)[1])) {
			getCharacter(kCharacterYasmin).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, _functionsYasmin[getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall]]);
			fedEx(kCharacterYasmin, kCharacterYasmin, 18, 0);
		}

		break;
	case 5:
		playCathExcuseMe();
		break;
	case 6:
		playChrExcuseMe(kCharacterYasmin, kCharacterCath, 0);
		break;
	case 12:
		if (walk(kCharacterYasmin, getCharacterCurrentParams(kCharacterYasmin)[0], getCharacterCurrentParams(kCharacterYasmin)[1])) {
			getCharacter(kCharacterYasmin).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, _functionsYasmin[getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall]]);
			fedEx(kCharacterYasmin, kCharacterYasmin, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_GoEtoG(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_GoEtoG);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 6;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_GoEtoG(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterYasmin).characterPosition.position = 4840;
		getCharacter(kCharacterYasmin).characterPosition.location = 0;
		getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 1;
		YasminCall(&LogicManager::CONS_Yasmin_DoCorrOtis, "615Be", 5, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 2;
			YasminCall(&LogicManager::CONS_Yasmin_DoWalk, 3, 3050, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 3;
			YasminCall(&LogicManager::CONS_Yasmin_DoCorrOtis, "615Ag", 7, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterYasmin).characterPosition.location = 1;
			endGraphics(kCharacterYasmin);
			getCharacter(kCharacterYasmin).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, _functionsYasmin[getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall]]);
			fedEx(kCharacterYasmin, kCharacterYasmin, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_GoGtoE(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_GoGtoE);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 7;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_GoGtoE(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterYasmin).characterPosition.position = 3050;
		getCharacter(kCharacterYasmin).characterPosition.location = 0;
		getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 1;
		YasminCall(&LogicManager::CONS_Yasmin_DoCorrOtis, "615Bg", 7, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 2;
			YasminCall(&LogicManager::CONS_Yasmin_DoWalk, 3, 4840, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 3;
			YasminCall(&LogicManager::CONS_Yasmin_DoCorrOtis, "615Ae", 5, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterYasmin).characterPosition.location = 1;
			endGraphics(kCharacterYasmin);
			getCharacter(kCharacterYasmin).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, _functionsYasmin[getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall]]);
			fedEx(kCharacterYasmin, kCharacterYasmin, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_Birth);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 8;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterYasmin)[0]) {
			getCharacterCurrentParams(kCharacterYasmin)[0] = 1;
			CONS_Yasmin_Part1(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterYasmin).characterPosition.position = 4840;
		getCharacter(kCharacterYasmin).characterPosition.location = 1;
		getCharacter(kCharacterYasmin).characterPosition.car = kCarGreenSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_Part1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_Part1);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 9;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_Part1(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1093500 && !getCharacterCurrentParams(kCharacterYasmin)[0]) {
			getCharacterCurrentParams(kCharacterYasmin)[0] = 1;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 1;
			YasminCall(&LogicManager::CONS_Yasmin_GoEtoG, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 1161000 && !getCharacterCurrentParams(kCharacterYasmin)[1]) {
			getCharacterCurrentParams(kCharacterYasmin)[1] = 1;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 3;
			YasminCall(&LogicManager::CONS_Yasmin_GoGtoE, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 1162800 && !getCharacterCurrentParams(kCharacterYasmin)[2]) {
			getCharacterCurrentParams(kCharacterYasmin)[2] = 1;
			getCharacter(kCharacterYasmin).characterPosition.position = 4070;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 4;
			YasminCall(&LogicManager::CONS_Yasmin_DoDialog, "Har1102", 0, 0, 0);
			break;
		}

		if (_gameTime > 1165500 && !getCharacterCurrentParams(kCharacterYasmin)[3]) {
			getCharacterCurrentParams(kCharacterYasmin)[3] = 1;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 5;
			YasminCall(&LogicManager::CONS_Yasmin_DoDialog, "Har1104", 0, 0, 0);
			break;
		}

		if (_gameTime > 1174500 && !getCharacterCurrentParams(kCharacterYasmin)[4]) {
			getCharacterCurrentParams(kCharacterYasmin)[4] = 1;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 6;
			YasminCall(&LogicManager::CONS_Yasmin_DoDialog, "Har1106", 0, 0, 0);
			break;
		}

		if (_gameTime > 1183500 && !getCharacterCurrentParams(kCharacterYasmin)[5]) {
			getCharacterCurrentParams(kCharacterYasmin)[5] = 1;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 7;
			YasminCall(&LogicManager::CONS_Yasmin_GoEtoG, 0, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterYasmin).characterPosition.position = 2740;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 2;
			YasminCall(&LogicManager::CONS_Yasmin_DoDialog, "Har1102", 0, 0, 0);
			break;
		case 2:
			if (_gameTime > 1161000 && !getCharacterCurrentParams(kCharacterYasmin)[1]) {
				getCharacterCurrentParams(kCharacterYasmin)[1] = 1;
				getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 3;
				YasminCall(&LogicManager::CONS_Yasmin_GoGtoE, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (_gameTime > 1162800 && !getCharacterCurrentParams(kCharacterYasmin)[2]) {
				getCharacterCurrentParams(kCharacterYasmin)[2] = 1;
				getCharacter(kCharacterYasmin).characterPosition.position = 4070;
				getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 4;
				YasminCall(&LogicManager::CONS_Yasmin_DoDialog, "Har1102", 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (_gameTime > 1165500 && !getCharacterCurrentParams(kCharacterYasmin)[3]) {
				getCharacterCurrentParams(kCharacterYasmin)[3] = 1;
				getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 5;
				YasminCall(&LogicManager::CONS_Yasmin_DoDialog, "Har1104", 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (_gameTime > 1174500 && !getCharacterCurrentParams(kCharacterYasmin)[4]) {
				getCharacterCurrentParams(kCharacterYasmin)[4] = 1;
				getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 6;
				YasminCall(&LogicManager::CONS_Yasmin_DoDialog, "Har1106", 0, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (_gameTime > 1183500 && !getCharacterCurrentParams(kCharacterYasmin)[5]) {
				getCharacterCurrentParams(kCharacterYasmin)[5] = 1;
				getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 7;
				YasminCall(&LogicManager::CONS_Yasmin_GoEtoG, 0, 0, 0, 0);
			}

			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}


void LogicManager::CONS_Yasmin_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_Asleep);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 10;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(7, kCharacterCath, 3, 10, 9);
		getCharacter(kCharacterYasmin).characterPosition.position = 3050;
		getCharacter(kCharacterYasmin).characterPosition.location = 1;
		getCharacter(kCharacterYasmin).characterPosition.car = kCarGreenSleeping;
		endGraphics(kCharacterYasmin);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_StartPart2);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 11;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterYasmin);
		getCharacter(kCharacterYasmin).characterPosition.position = 3050;
		getCharacter(kCharacterYasmin).characterPosition.location = 1;
		getCharacter(kCharacterYasmin).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterYasmin).inventoryItem = kItemNone;
		getCharacter(kCharacterYasmin).clothes = 0;
		CONS_Yasmin_Part2(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_Part2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_Part2);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 12;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_Part2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1759500 && !getCharacterCurrentParams(kCharacterYasmin)[0]) {
			getCharacterCurrentParams(kCharacterYasmin)[0] = 1;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 1;
			YasminCall(&LogicManager::CONS_Yasmin_GoGtoE, 0, 0, 0, 0);
		} else if (_gameTime > 1800000 && !getCharacterCurrentParams(kCharacterYasmin)[1]) {
			getCharacterCurrentParams(kCharacterYasmin)[1] = 1;
			getCharacter(kCharacterYasmin).characterPosition.position = 4070;
			send(kCharacterYasmin, kCharacterClerk, 191070912, 4070);
		}

		break;
	case 18:
		if (getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] == 1) {
			if (_gameTime > 1800000 && !getCharacterCurrentParams(kCharacterYasmin)[1]) {
				getCharacterCurrentParams(kCharacterYasmin)[1] = 1;
				getCharacter(kCharacterYasmin).characterPosition.position = 4070;
				send(kCharacterYasmin, kCharacterClerk, 191070912, 4070);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_StartPart3);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 13;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Yasmin_Part3(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterYasmin);
		getCharacter(kCharacterYasmin).characterPosition.position = 3050;
		getCharacter(kCharacterYasmin).characterPosition.location = 1;
		getCharacter(kCharacterYasmin).characterPosition.car = kCarGreenSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_Part3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_Part3);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 14;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_Part3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2062800 && !getCharacterCurrentParams(kCharacterYasmin)[0]) {
			getCharacterCurrentParams(kCharacterYasmin)[0] = 1;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 1;
			YasminCall(&LogicManager::CONS_Yasmin_GoEtoG, 0, 0, 0, 0);
		} else if (_gameTime > 2106000 && !getCharacterCurrentParams(kCharacterYasmin)[1]) {
			getCharacterCurrentParams(kCharacterYasmin)[1] = 1;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 2;
			YasminCall(&LogicManager::CONS_Yasmin_GoGtoE, 0, 0, 0, 0);
		} else if (_gameTime > 2160000 && !getCharacterCurrentParams(kCharacterYasmin)[2]) {
			getCharacterCurrentParams(kCharacterYasmin)[2] = 1;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 3;
			YasminCall(&LogicManager::CONS_Yasmin_GoEtoG, 0, 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] != 1) {
			if (getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] == 2) {
				if (_gameTime > 2160000 && !getCharacterCurrentParams(kCharacterYasmin)[2]) {
					getCharacterCurrentParams(kCharacterYasmin)[2] = 1;
					getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 3;
					YasminCall(&LogicManager::CONS_Yasmin_GoEtoG, 0, 0, 0, 0);
				}
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_StartPart4);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 15;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Yasmin_Part4(0, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterYasmin).characterPosition.position = 3050;
		getCharacter(kCharacterYasmin).characterPosition.location = 1;
		getCharacter(kCharacterYasmin).characterPosition.car = kCarGreenSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_Part4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_Part4);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 16;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_Part4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2457000 && !getCharacterCurrentParams(kCharacterYasmin)[0]) {
			getCharacterCurrentParams(kCharacterYasmin)[0] = 1;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 1;
			YasminCall(&LogicManager::CONS_Yasmin_GoGtoE, 0, 0, 0, 0);
		} else if (_gameTime > 2479500 && !getCharacterCurrentParams(kCharacterYasmin)[1]) {
			getCharacterCurrentParams(kCharacterYasmin)[1] = 1;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 3;
			YasminCall(&LogicManager::CONS_Yasmin_GoEtoG, 0, 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] == 1) {
			getCharacter(kCharacterYasmin).characterPosition.position = 4070;
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 2;
			YasminCall(&LogicManager::CONS_Yasmin_DoDialog, "Har1110", 0, 0, 0);
			return;
		} else if (getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] == 2) {
			if (_gameTime > 2479500 && !getCharacterCurrentParams(kCharacterYasmin)[1]) {
				getCharacterCurrentParams(kCharacterYasmin)[1] = 1;
				getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 3;
				YasminCall(&LogicManager::CONS_Yasmin_GoEtoG, 0, 0, 0, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_Asleep4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_Asleep4);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 17;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_Asleep4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(7, kCharacterCath, 3, 10, 9);
		getCharacter(kCharacterYasmin).characterPosition.position = 3050;
		getCharacter(kCharacterYasmin).characterPosition.location = 1;
		getCharacter(kCharacterYasmin).characterPosition.car = kCarGreenSleeping;
		endGraphics(kCharacterYasmin);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_StartPart5);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 18;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Yasmin_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterYasmin);
		getCharacter(kCharacterYasmin).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterYasmin).characterPosition.position = 3969;
		getCharacter(kCharacterYasmin).characterPosition.location = 1;
		getCharacter(kCharacterYasmin).clothes = 0;
		getCharacter(kCharacterYasmin).inventoryItem = kItemNone;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_Prisoner);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 19;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Yasmin_Free(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_Free(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_Free);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 20;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_Free(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterYasmin)[0] || (getCharacterCurrentParams(kCharacterYasmin)[0] = _gameTime + 2700, _gameTime != -2700)) {
			if (getCharacterCurrentParams(kCharacterYasmin)[0] >= _gameTime)
				return;

			getCharacterCurrentParams(kCharacterYasmin)[0] = 0x7FFFFFFF;
		}

		CONS_Yasmin_Hiding(0, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterYasmin).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterYasmin).characterPosition.position = 2500;
		getCharacter(kCharacterYasmin).characterPosition.location = 0;
		break;
	case 17:
		if (checkLoc(kCharacterCath, kCarGreenSleeping)) {
			CONS_Yasmin_Hiding(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_Hiding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_Hiding);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 21;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_Hiding(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!whoRunningDialog(kCharacterYasmin)) {
			if (getCharacterCurrentParams(kCharacterYasmin)[0] || (getCharacterCurrentParams(kCharacterYasmin)[0] = _realTime + 450, _realTime != -450)) {
				if (getCharacterCurrentParams(kCharacterYasmin)[0] >= _realTime)
					return;

				getCharacterCurrentParams(kCharacterYasmin)[0] = 0x7FFFFFFF;
			}

			playDialog(kCharacterYasmin, "Har5001", -1, 0);
			getCharacterCurrentParams(kCharacterYasmin)[0] = 0;
		}

		break;
	case 12:
		getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 1;
		YasminCall(&LogicManager::CONS_Yasmin_DoWalk, 3, 4840, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] == 1) {
			getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] = 2;
			YasminCall(&LogicManager::CONS_Yasmin_DoCorrOtis, "615BE", 5, 0, 0);
		} else if (getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall + 8] == 2) {
			endGraphics(kCharacterYasmin);
			getCharacter(kCharacterYasmin).characterPosition.location = 1;
			getCharacter(kCharacterYasmin).characterPosition.position = 3050;
			setDoor(7, kCharacterCath, 1, 10, 9);
			playDialog(kCharacterYasmin, "Har5001", -1, 0);
		}
		break;
	case 135800432:
		CONS_Yasmin_Disappear(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Yasmin_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterYasmin).callParams[getCharacter(kCharacterYasmin).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterYasmin, &LogicManager::HAND_Yasmin_Disappear);
	getCharacter(kCharacterYasmin).callbacks[getCharacter(kCharacterYasmin).currentCall] = 22;

	params->clear();

	fedEx(kCharacterYasmin, kCharacterYasmin, 12, 0);
}

void LogicManager::HAND_Yasmin_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsYasmin[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Yasmin_DebugWalks,
	&LogicManager::HAND_Yasmin_DoCorrOtis,
	&LogicManager::HAND_Yasmin_DoDialog,
	&LogicManager::HAND_Yasmin_DoWait,
	&LogicManager::HAND_Yasmin_DoWalk,
	&LogicManager::HAND_Yasmin_GoEtoG,
	&LogicManager::HAND_Yasmin_GoGtoE,
	&LogicManager::HAND_Yasmin_Birth,
	&LogicManager::HAND_Yasmin_Part1,
	&LogicManager::HAND_Yasmin_Asleep,
	&LogicManager::HAND_Yasmin_StartPart2,
	&LogicManager::HAND_Yasmin_Part2,
	&LogicManager::HAND_Yasmin_StartPart3,
	&LogicManager::HAND_Yasmin_Part3,
	&LogicManager::HAND_Yasmin_StartPart4,
	&LogicManager::HAND_Yasmin_Part4,
	&LogicManager::HAND_Yasmin_Asleep4,
	&LogicManager::HAND_Yasmin_StartPart5,
	&LogicManager::HAND_Yasmin_Prisoner,
	&LogicManager::HAND_Yasmin_Free,
	&LogicManager::HAND_Yasmin_Hiding,
	&LogicManager::HAND_Yasmin_Disappear
};

} // End of namespace LastExpress
