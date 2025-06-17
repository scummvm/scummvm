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

void LogicManager::CONS_Salko(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterSalko,
			_functionsSalko[getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall]]
		);

		break;
	case 1:
		CONS_Salko_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Salko_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Salko_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Salko_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Salko_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::SalkoCall(CALL_PARAMS) {
	getCharacter(kCharacterSalko).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Salko_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_DebugWalks);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 1;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterSalko, kCarGreenSleeping, getCharacterCurrentParams(kCharacterSalko)[0])) {
			if (getCharacterCurrentParams(kCharacterSalko)[0] == 10000) {
				getCharacterCurrentParams(kCharacterSalko)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterSalko)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterSalko).characterPosition.position = 0;
		getCharacter(kCharacterSalko).characterPosition.location = 0;
		getCharacter(kCharacterSalko).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterSalko)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_DoCorrOtis);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterSalko, getCharacterCurrentParams(kCharacterSalko)[3]);

		getCharacter(kCharacterSalko).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterSalko, _functionsSalko[getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall]]);
		fedEx(kCharacterSalko, kCharacterSalko, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterSalko, (char *)&getCharacterCurrentParams(kCharacterSalko)[0]);
		blockAtDoor(kCharacterSalko, getCharacterCurrentParams(kCharacterSalko)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_DoSeqOtis);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterSalko).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterSalko, _functionsSalko[getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall]]);
		fedEx(kCharacterSalko, kCharacterSalko, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterSalko, (char *)&getCharacterCurrentParams(kCharacterSalko)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_DoWalk);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 4;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterSalko, getCharacterCurrentParams(kCharacterSalko)[0], getCharacterCurrentParams(kCharacterSalko)[1])) {
			getCharacter(kCharacterSalko).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterSalko, _functionsSalko[getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall]]);
			fedEx(kCharacterSalko, kCharacterSalko, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_DoWait);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 5;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterSalko)[1] || (getCharacterCurrentParams(kCharacterSalko)[1] = _gameTime + getCharacterCurrentParams(kCharacterSalko)[0], _gameTime + getCharacterCurrentParams(kCharacterSalko)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterSalko)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterSalko)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterSalko).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterSalko, _functionsSalko[getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall]]);
		fedEx(kCharacterSalko, kCharacterSalko, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_SaveGame);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 6;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterSalko).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterSalko, _functionsSalko[getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall]]);
		fedEx(kCharacterSalko, kCharacterSalko, 18, 0);
		break;
	case 12:
		save(
			kCharacterSalko,
			getCharacterCurrentParams(kCharacterSalko)[0],
			getCharacterCurrentParams(kCharacterSalko)[1]
		);

		getCharacter(kCharacterSalko).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterSalko, _functionsSalko[getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall]]);
		fedEx(kCharacterSalko, kCharacterSalko, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_DoWalkBehind(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_DoWalkBehind);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 7;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_DoWalkBehind(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacterCurrentParams(kCharacterSalko)[2] = 0;
		if (nearChar(kCharacterSalko, kCharacterIvo, 500) ||
			((getCharacter(kCharacterSalko).direction == 1) &&
				((getCharacter(kCharacterSalko).characterPosition.car > getCharacter(kCharacterIvo).characterPosition.car) ||
				 (getCharacter(kCharacterSalko).characterPosition.car == getCharacter(kCharacterIvo).characterPosition.car &&
					 getCharacter(kCharacterSalko).characterPosition.position > getCharacter(kCharacterIvo).characterPosition.position))) ||
			(getCharacter(kCharacterSalko).direction == 2 &&
				((getCharacter(kCharacterSalko).characterPosition.car < getCharacter(kCharacterIvo).characterPosition.car) ||
				 (getCharacter(kCharacterSalko).characterPosition.car == getCharacter(kCharacterIvo).characterPosition.car &&
					 getCharacter(kCharacterSalko).characterPosition.position < getCharacter(kCharacterIvo).characterPosition.position)))) {

			getCharacter(kCharacterSalko).waitedTicksUntilCycleRestart = 0;
			getCharacterCurrentParams(kCharacterSalko)[2] = 1;
		}

		if (!getCharacterCurrentParams(kCharacterSalko)[2]) {
			walk(kCharacterSalko, getCharacterCurrentParams(kCharacterSalko)[0], getCharacterCurrentParams(kCharacterSalko)[1]);
		}

		break;
	case 5:
	case 6:
		playDialog(0, "ZFX1002", getVolume(kCharacterSalko), 0);
		playDialog(kCharacterCath, "CAT1127A", -1, 0);
		break;
	case 12:
		walk(kCharacterSalko, getCharacterCurrentParams(kCharacterSalko)[0], getCharacterCurrentParams(kCharacterSalko)[1]);
		break;
	case 123668192:
		getCharacter(kCharacterSalko).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterSalko, _functionsSalko[getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall]]);
		fedEx(kCharacterSalko, kCharacterSalko, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_HomeTogether(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_HomeTogether);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 8;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_HomeTogether(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterSalko).characterPosition.position = 2740;
		getCharacter(kCharacterSalko).characterPosition.location = 1;
		getCharacter(kCharacterSalko).characterPosition.car = kCarRedSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_Birth);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 9;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterSalko)[0]) {
			getCharacterCurrentParams(kCharacterSalko)[0] = 1;
			CONS_Salko_WithIvo(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterSalko).characterPosition.position = 4691;
		getCharacter(kCharacterSalko).characterPosition.location = 0;
		getCharacter(kCharacterSalko).characterPosition.car = kCarRestaurant;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_WithIvo(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_WithIvo);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 10;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_WithIvo(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterSalko).characterPosition = getCharacter(kCharacterIvo).characterPosition;
		break;
	case 18:
		if (getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] == 1) {
			startCycOtis(kCharacterSalko, "BLANK");
			CONS_Salko_HomeTogether(0, 0, 0, 0);
		}

		break;
	case 125242096:
		getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 1;
		SalkoCall(&LogicManager::CONS_Salko_DoWalkBehind, 4, 2740, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_Asleep);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 11;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterSalko).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterSalko).characterPosition.position = 2740;
		getCharacter(kCharacterSalko).characterPosition.location = 1;
		endGraphics(kCharacterSalko);
		setDoor(39, kCharacterCath, 3, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_StartPart2);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 12;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterSalko);
		getCharacter(kCharacterSalko).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterSalko).characterPosition.position = 2740;
		getCharacter(kCharacterSalko).characterPosition.location = 1;
		getCharacter(kCharacterSalko).clothes = 0;
		getCharacter(kCharacterSalko).inventoryItem = 0;
		break;
	case 136184016:
		getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 1;
		SalkoCall(&LogicManager::CONS_Salko_JoinIvo, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_JoinIvo(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_JoinIvo);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 13;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_JoinIvo(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 1;
		SalkoCall(&LogicManager::CONS_Salko_DoCorrOtis, "612DH", 39, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 2;
			SalkoCall(&LogicManager::CONS_Salko_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			send(kCharacterSalko, kCharacterIvo, 102675536, 0);
			endGraphics(kCharacterSalko);
			break;
		case 3:
			startCycOtis(kCharacterSalko, "BLANK");
			getCharacter(kCharacterSalko).characterPosition.location = 1;
			CONS_Salko_HomeTogether(0, 0, 0, 0);
			break;
		}

		break;
	case 125242096:
		getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 3;
		SalkoCall(&LogicManager::CONS_Salko_DoWalkBehind, 4, 2740, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_StartPart3);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 14;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Salko_InComp(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterSalko);
		getCharacter(kCharacterSalko).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterSalko).characterPosition.position = 2740;
		getCharacter(kCharacterSalko).characterPosition.location = 1;
		getCharacter(kCharacterSalko).clothes = 0;
		getCharacter(kCharacterSalko).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_InComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_InComp);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 15;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_InComp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime < 2200500) {
			if (getCharacterCurrentParams(kCharacterSalko)[0] || (getCharacterCurrentParams(kCharacterSalko)[0] = _gameTime + 81000, _gameTime != -81000)) {
				if (getCharacterCurrentParams(kCharacterSalko)[0] >= _gameTime)
					break;
				getCharacterCurrentParams(kCharacterSalko)[0] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 1;
			SalkoCall(&LogicManager::CONS_Salko_EavesdropAnna, 0, 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] == 1)
			getCharacterCurrentParams(kCharacterSalko)[0] = 0;

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_EavesdropAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_EavesdropAnna);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 16;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_EavesdropAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (whoOnScreen(kCharacterSalko) && nearChar(kCharacterSalko, kCharacterCath, 5000)) {
			send(kCharacterSalko, kCharacterMax, 158007856, 0);
			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 3;
			SalkoCall(&LogicManager::CONS_Salko_DoWait, 75, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterSalko)[0] || (getCharacterCurrentParams(kCharacterSalko)[0] = _gameTime + 4500, _gameTime != -4500)) {
			if (getCharacterCurrentParams(kCharacterSalko)[0] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterSalko)[0] = 0x7FFFFFFF;
		}

		send(kCharacterSalko, kCharacterSalko, 101169464, 0);
		break;
	case 12:
		getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 1;
		SalkoCall(&LogicManager::CONS_Salko_DoCorrOtis, "612DH", 0x27, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterSalko).characterPosition.location = 0;

			if (getCharacter(kCharacterSalko).characterPosition.position < 2087)
				getCharacter(kCharacterSalko).characterPosition.position = 2088;

			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 2;
			SalkoCall(&LogicManager::CONS_Salko_DoWalk, 4, 4070, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterSalko, "612AF");
			softBlockAtDoor(kCharacterSalko, 37);
			break;
		case 3:
			send(kCharacterSalko, kCharacterSalko, 101169464, 0);
			if (getCharacterCurrentParams(kCharacterSalko)[0] || (getCharacterCurrentParams(kCharacterSalko)[0] = _gameTime + 4500, _gameTime != -4500)) {
				if (getCharacterCurrentParams(kCharacterSalko)[0] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterSalko)[0] = 0x7FFFFFFF;
			}

			send(kCharacterSalko, kCharacterSalko, 101169464, 0);
			break;
		case 4:
			softReleaseAtDoor(kCharacterSalko, 37);
			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 5;
			SalkoCall(&LogicManager::CONS_Salko_DoWalk, 4, 9460, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 6;
			SalkoCall(&LogicManager::CONS_Salko_DoWait, 4500, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 7;
			SalkoCall(&LogicManager::CONS_Salko_DoWalk, 4, 2740, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 8;
			SalkoCall(&LogicManager::CONS_Salko_DoCorrOtis, "612Ch", 39, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterSalko).characterPosition.location = 1;
			getCharacter(kCharacterSalko).characterPosition.position = 2740;
			endGraphics(kCharacterSalko);

			getCharacter(kCharacterSalko).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterSalko, _functionsSalko[getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall]]);
			fedEx(kCharacterSalko, kCharacterSalko, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 101169464:
		getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 4;
		SalkoCall(&LogicManager::CONS_Salko_DoCorrOtis, "612Bf", 37, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_TellMilos(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_TellMilos);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 17;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_TellMilos(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterSalko).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterSalko).characterPosition.position = 6470;
		getCharacter(kCharacterSalko).characterPosition.location = 0;
		getCharacter(kCharacterSalko).clothes = 0;
		getCharacter(kCharacterSalko).inventoryItem = 0;
		getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 1;
		SalkoCall(&LogicManager::CONS_Salko_DoWalk, 4, 2740, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] == 1) {
			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 2;
			SalkoCall(&LogicManager::CONS_Salko_DoCorrOtis, "612Ch", 39, 0, 0);
		} else if (getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] == 2) {
			endGraphics(kCharacterSalko);
			getCharacter(kCharacterSalko).characterPosition.position = 2740;
			getCharacter(kCharacterSalko).characterPosition.location = 1;
			send(kCharacterSalko, kCharacterMilos, 157691176, 0);
			CONS_Salko_InComp(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_StartPart4);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 18;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Salko_WithIvo4(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterSalko);
		getCharacter(kCharacterSalko).characterPosition.position = 5420;
		getCharacter(kCharacterSalko).characterPosition.location = 1;
		getCharacter(kCharacterSalko).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterSalko).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_WithIvo4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_WithIvo4);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 19;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_WithIvo4(HAND_PARAMS) {
	switch (msg->action) {
	case 18:
		if (getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] == 1) {
			startCycOtis(kCharacterSalko, "BLANK");
			getCharacter(kCharacterSalko).characterPosition.location = 1;
			CONS_Salko_InComp4(0, 0, 0, 0);
		}

		break;
	case 125242096:
		getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 1;
		SalkoCall(&LogicManager::CONS_Salko_DoWalkBehind, 4, 2740, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_InComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_InComp4);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 20;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_InComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 18:
		if (getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] == 1) {
			getCharacter(kCharacterSalko).characterPosition.location = 0;

			if (getCharacter(kCharacterSalko).characterPosition.position < 2087)
				getCharacter(kCharacterSalko).characterPosition.position = 2088;

			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 2;
			SalkoCall(&LogicManager::CONS_Salko_DoWalk, 5, 850, 0, 0);
		} else if (getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] == 2) {
			endGraphics(kCharacterSalko);
			CONS_Salko_Hiding(0, 0, 0, 0);
		}

		break;
	case 55996766:
		getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 1;
		SalkoCall(&LogicManager::CONS_Salko_DoCorrOtis, "612Dh", 39, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_Hiding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_Hiding);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 21;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_Hiding(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2422800 && !getCharacterCurrentParams(kCharacterSalko)[0]) {
			getCharacterCurrentParams(kCharacterSalko)[0] = 1;
			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 1;
			SalkoCall(&LogicManager::CONS_Salko_DoWalk, 4, 2740, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] == 1) {
			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 2;
			SalkoCall(&LogicManager::CONS_Salko_DoCorrOtis, "612Ch", 39, 0, 0);
		} else if (getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] == 2) {
			CONS_Salko_EndPart4(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_EndPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_EndPart4);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 22;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_EndPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterSalko);
		setDoor(39, kCharacterCath, 3, 10, 9);
		getCharacter(kCharacterSalko).characterPosition.location = 1;
		getCharacter(kCharacterSalko).characterPosition.position = 2740;
		getCharacter(kCharacterSalko).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterSalko).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_StartPart5);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 23;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Salko_Guarding(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterSalko);
		getCharacter(kCharacterSalko).characterPosition.location = 1;
		getCharacter(kCharacterSalko).characterPosition.position = 9460;
		getCharacter(kCharacterSalko).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterSalko).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_Guarding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_Guarding);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 24;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_Guarding(HAND_PARAMS) {
	switch (msg->action) {
	case 18:
		switch (getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8]) {
		case 1:
			if (dialogRunning("MUS050"))
				fadeDialog("MUS050");
			playNIS(kEventCathSalkoTrainTopFight);
			getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 2;
			SalkoCall(&LogicManager::CONS_Salko_SaveGame, 1, 0, 0, 0);
			break;
		case 2:
		{
			int fightOutcome = playFight(2004);
			getCharacterCurrentParams(kCharacterSalko)[0] = fightOutcome;
			if (fightOutcome) {
				endGame(0, 0, 0, fightOutcome == 1);
			} else {
				_gameTime += 1800;
				getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 3;
				SalkoCall(&LogicManager::CONS_Salko_SaveGame, 2, 176, 0, 0);
			}

			break;
		}
		case 3:
			playNIS(kEventCathSalkoTrainTopWin);
			send(kCharacterSalko, kCharacterVesna, 134427424, 0);
			bumpCath(kCarRestaurant, 10, 255);
			CONS_Salko_Disappear(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 167992577:
		getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall + 8] = 1;
		SalkoCall(&LogicManager::CONS_Salko_SaveGame, 2, 172, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Salko_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSalko).callParams[getCharacter(kCharacterSalko).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSalko, &LogicManager::HAND_Salko_Disappear);
	getCharacter(kCharacterSalko).callbacks[getCharacter(kCharacterSalko).currentCall] = 25;

	params->clear();

	fedEx(kCharacterSalko, kCharacterSalko, 12, 0);
}

void LogicManager::HAND_Salko_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsSalko[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Salko_DebugWalks,
	&LogicManager::HAND_Salko_DoCorrOtis,
	&LogicManager::HAND_Salko_DoSeqOtis,
	&LogicManager::HAND_Salko_DoWalk,
	&LogicManager::HAND_Salko_DoWait,
	&LogicManager::HAND_Salko_SaveGame,
	&LogicManager::HAND_Salko_DoWalkBehind,
	&LogicManager::HAND_Salko_HomeTogether,
	&LogicManager::HAND_Salko_Birth,
	&LogicManager::HAND_Salko_WithIvo,
	&LogicManager::HAND_Salko_Asleep,
	&LogicManager::HAND_Salko_StartPart2,
	&LogicManager::HAND_Salko_JoinIvo,
	&LogicManager::HAND_Salko_StartPart3,
	&LogicManager::HAND_Salko_InComp,
	&LogicManager::HAND_Salko_EavesdropAnna,
	&LogicManager::HAND_Salko_TellMilos,
	&LogicManager::HAND_Salko_StartPart4,
	&LogicManager::HAND_Salko_WithIvo4,
	&LogicManager::HAND_Salko_InComp4,
	&LogicManager::HAND_Salko_Hiding,
	&LogicManager::HAND_Salko_EndPart4,
	&LogicManager::HAND_Salko_StartPart5,
	&LogicManager::HAND_Salko_Guarding,
	&LogicManager::HAND_Salko_Disappear
};

} // End of namespace LastExpress
