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

void LogicManager::CONS_Ivo(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterIvo,
			_functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]
		);

		break;
	case 1:
		CONS_Ivo_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Ivo_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Ivo_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Ivo_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Ivo_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::IvoCall(CALL_PARAMS) {
	getCharacter(kCharacterIvo).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Ivo_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_DebugWalks);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 1;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterIvo, kCarGreenSleeping, getCharacterCurrentParams(kCharacterIvo)[0])) {
			if (getCharacterCurrentParams(kCharacterIvo)[0] == 10000) {
				getCharacterCurrentParams(kCharacterIvo)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterIvo)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterIvo).characterPosition.position = 0;
		getCharacter(kCharacterIvo).characterPosition.location = 0;
		getCharacter(kCharacterIvo).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterIvo)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_DoSeqOtis);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterIvo, (char *)&getCharacterCurrentParams(kCharacterIvo)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_DoCorrOtis);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterIvo, getCharacterCurrentParams(kCharacterIvo)[3]);

		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterIvo, (char *)&getCharacterCurrentParams(kCharacterIvo)[0]);
		blockAtDoor(kCharacterIvo, getCharacterCurrentParams(kCharacterIvo)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_DoWait);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 4;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterIvo)[1] || (getCharacterCurrentParams(kCharacterIvo)[1] = _gameTime + getCharacterCurrentParams(kCharacterIvo)[0],
															_gameTime + getCharacterCurrentParams(kCharacterIvo)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterIvo)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterIvo)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_DoWaitReal(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_DoWaitReal);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 5;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_DoWaitReal(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterIvo)[1] || (getCharacterCurrentParams(kCharacterIvo)[1] = _realTime + getCharacterCurrentParams(kCharacterIvo)[0],
															_realTime + getCharacterCurrentParams(kCharacterIvo)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterIvo)[1] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterIvo)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_DoWalk);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 6;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterIvo, getCharacterCurrentParams(kCharacterIvo)[0], getCharacterCurrentParams(kCharacterIvo)[1])) {
			getCharacter(kCharacterIvo).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
			fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		}

		break;
	case 5:
	case 6:
		playDialog(kCharacterCath, "CAT1127A", -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_FinishSeqOtis);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 7;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterIvo).direction != 4) {
			getCharacter(kCharacterIvo).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
			fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_DoDialog);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 8;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	case 12:
		getCharacter(kCharacterIvo).inventoryItem = 0;
		playDialog(kCharacterIvo, (char *)&getCharacterCurrentParams(kCharacterIvo)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_WaitRCClear);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 9;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterIvo).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
			fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_SaveGame);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	case 12:
		save(
			kCharacterIvo,
			getCharacterCurrentParams(kCharacterIvo)[0],
			getCharacterCurrentParams(kCharacterIvo)[1]
		);

		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_GoCompartment(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_GoCompartment);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 11;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_GoCompartment(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (nearChar(kCharacterIvo, kCharacterSalko, 750) || nearX(kCharacterSalko, 2740, 500)) {
			send(kCharacterIvo, kCharacterSalko, 123668192, 0);
			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 4;
			IvoCall(&LogicManager::CONS_Ivo_DoCorrOtis, "613Ah", 39, 0, 0);
		}
		break;
	case 12:
		startSeqOtis(kCharacterIvo, "809DS");

		if (inDiningRoom(kCharacterCath))
			advanceFrame(kCharacterIvo);

		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 1;
		IvoCall(&LogicManager::CONS_Ivo_FinishSeqOtis, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8]) {
		case 1:
			send(kCharacterIvo, kCharacterSalko, 125242096, 0);
			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 2;
			IvoCall(&LogicManager::CONS_Ivo_DoWalk, 4, 2740, 0, 0);
			break;
		case 2:
			if (nearChar(kCharacterIvo, kCharacterSalko, 750) || nearX(kCharacterSalko, 2740, 500)) {
				send(kCharacterIvo, kCharacterSalko, 123668192, 0);
				getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 3;
				IvoCall(&LogicManager::CONS_Ivo_DoCorrOtis, "613Ah", 39, 0, 0);
			} else {
				startCycOtis(kCharacterIvo, "613Hh");
				softBlockAtDoor(kCharacterIvo, 39);
			}

			break;
		case 3:
			getCharacter(kCharacterIvo).characterPosition.position = 2740;
			getCharacter(kCharacterIvo).characterPosition.location = 1;
			endGraphics(kCharacterIvo);

			getCharacter(kCharacterIvo).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
			fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
			break;
		case 4:
			softReleaseAtDoor(kCharacterIvo, 39);
			getCharacter(kCharacterIvo).characterPosition.position = 2740;
			getCharacter(kCharacterIvo).characterPosition.location = 1;
			endGraphics(kCharacterIvo);

			getCharacter(kCharacterIvo).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
			fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_DoSplitOtis023A(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_DoSplitOtis023A);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 12;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_DoSplitOtis023A(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		endGraphics(kCharacterSalko);
		send(kCharacterIvo, kCharacterTableC, 136455232, 0);

		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterIvo, "023A1");
		startSeqOtis(kCharacterSalko, "023A2");
		startSeqOtis(kCharacterTableC, "023A3");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_DoSplitOtis023D(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_DoSplitOtis023D);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 13;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_DoSplitOtis023D(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		send(kCharacterIvo, kCharacterTableC, 103798704, "009E");
		endGraphics(kCharacterSalko);

		getCharacter(kCharacterIvo).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterIvo, _functionsIvo[getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall]]);
		fedEx(kCharacterIvo, kCharacterIvo, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterIvo, "023D1");
		startSeqOtis(kCharacterSalko, "023D2");
		startSeqOtis(kCharacterTableC, "023D3");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_Birth);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 14;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterIvo)[0]) {
			getCharacterCurrentParams(kCharacterIvo)[0] = 1;
			CONS_Ivo_ReturnComp(0, 0, 0, 0);
		}

		break;
	case 12:
		setDoor(39, kCharacterCath, 3, 10, 9);
		setDoor(47, kCharacterCath, 0, 255, 255);
		getCharacter(kCharacterIvo).characterPosition.position = 4691;
		getCharacter(kCharacterIvo).characterPosition.location = 1;
		getCharacter(kCharacterIvo).characterPosition.car = kCarRestaurant;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_ReturnComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_ReturnComp);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 15;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_ReturnComp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterIvo).characterPosition = getCharacter(kCharacterMilos).characterPosition;
		break;
	case 18:
		if (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] == 1) {
			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 2;
			IvoCall(&LogicManager::CONS_Ivo_GoCompartment, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] == 2) {
			send(kCharacterIvo, kCharacterMilos, 135024800, 0);
			CONS_Ivo_InComp(0, 0, 0, 0);
		}

		break;
	case 125242096:
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 1;
		IvoCall(&LogicManager::CONS_Ivo_DoWaitReal, 75, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_InComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_InComp);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 16;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_InComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterIvo).characterPosition.position = 2740;
		getCharacter(kCharacterIvo).characterPosition.location = 1;
		getCharacter(kCharacterIvo).characterPosition.car = kCarRedSleeping;
		setDoor(39, kCharacterCath, 3, 10, 9);
		endGraphics(kCharacterIvo);
		break;
	case 18:
		if (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] == 1) {
			startCycOtis(kCharacterIvo, "613Ch");
			softBlockAtDoor(kCharacterIvo, 39);
			send(kCharacterIvo, kCharacterCond2, 88652208, 0);
		} else if (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] == 2) {
			getCharacter(kCharacterIvo).characterPosition.position = 2740;
			getCharacter(kCharacterIvo).characterPosition.location = 1;
			endGraphics(kCharacterIvo);
			setDoor(39, kCharacterCath, 3, 10, 9);
		}

		break;
	case 122865568:
		getCharacter(kCharacterIvo).characterPosition.location = 0;
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 1;
		IvoCall(&LogicManager::CONS_Ivo_DoCorrOtis, "613Bh", 39, 0, 0);
		break;
	case 123852928:
		softReleaseAtDoor(kCharacterIvo, 39);
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 2;
		IvoCall(&LogicManager::CONS_Ivo_DoCorrOtis, "613Dh", 39, 0, 0);
		break;
	case 221683008:
		send(kCharacterIvo, kCharacterCond2, 123199584, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_Asleep);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 17;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterIvo).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterIvo).characterPosition.position = 2740;
		getCharacter(kCharacterIvo).characterPosition.location = 1;
		endGraphics(kCharacterIvo);
		setDoor(39, kCharacterCath, 3, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_StartPart2);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 18;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1777500 && !getCharacterCurrentParams(kCharacterIvo)[0]) {
			getCharacterCurrentParams(kCharacterIvo)[0] = 1;
			CONS_Ivo_GoBreakfast(0, 0, 0, 0);
		}

		break;
	case 12:
		endGraphics(kCharacterIvo);
		getCharacter(kCharacterIvo).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterIvo).characterPosition.position = 2740;
		getCharacter(kCharacterIvo).characterPosition.location = 1;
		getCharacter(kCharacterIvo).clothes = 0;
		getCharacter(kCharacterIvo).inventoryItem = 0;
		setDoor(39, kCharacterCath, 3, 10, 9);
		setDoor(47, kCharacterCath, 1, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_GoBreakfast(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_GoBreakfast);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 19;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_GoBreakfast(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 1;
		IvoCall(&LogicManager::CONS_Ivo_DoCorrOtis, "613FH", 39, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterIvo).characterPosition.location = 0;

			if (getCharacter(kCharacterIvo).characterPosition.position < 2087)
				getCharacter(kCharacterIvo).characterPosition.position = 2088;

			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 2;
			IvoCall(&LogicManager::CONS_Ivo_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			send(kCharacterIvo, kCharacterSalko, 136184016, 0);
			break;
		case 3:
			getCharacter(kCharacterIvo).characterPosition.position = 1540;
			getCharacter(kCharacterIvo).characterPosition.location = 0;
			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 4;
			IvoCall(&LogicManager::CONS_Ivo_DoSeqOtis, "809US", 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 5;
			IvoCall(&LogicManager::CONS_Ivo_DoSplitOtis023A, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterIvo).characterPosition.location = 1;
			CONS_Ivo_AtBreakfast(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 102675536:
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 3;
		IvoCall(&LogicManager::CONS_Ivo_WaitRCClear, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_AtBreakfast(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_AtBreakfast);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 20;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_AtBreakfast(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1809000 && getCharacterCurrentParams(kCharacterIvo)[0]) {
			if (rcClear()) {
				getCharacter(kCharacterIvo).characterPosition.location = 0;
				getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 2;
				IvoCall(&LogicManager::CONS_Ivo_DoSplitOtis023D, 0, 0, 0, 0);
			}
		}

		break;
	case 12:
		send(kCharacterIvo, kCharacterWaiter2, 189688608, 0);
		startCycOtis(kCharacterIvo, "023B");
		break;
	case 18:
		switch (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8]) {
		case 1:
			send(kCharacterIvo, kCharacterWaiter2, 101106391, 0);
			startCycOtis(kCharacterIvo, "023B");
			getCharacterCurrentParams(kCharacterIvo)[0] = 1;
			break;
		case 2:
			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 3;
			IvoCall(&LogicManager::CONS_Ivo_GoCompartment, 0, 0, 0, 0);
			break;
		case 3:
			send(kCharacterIvo, kCharacterWaiter2, 236237423, 0);
			CONS_Ivo_InComp2(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 123712592:
		startCycOtis(kCharacterIvo, "023C2");
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 1;
		IvoCall(&LogicManager::CONS_Ivo_DoWait, 450, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_InComp2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_InComp2);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 21;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_InComp2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterIvo).characterPosition.position = 2740;
		getCharacter(kCharacterIvo).characterPosition.location = 1;
		getCharacter(kCharacterIvo).characterPosition.car = kCarRedSleeping;
		setDoor(39, kCharacterCath, 3, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_StartPart3);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 22;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Ivo_InComp3(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterIvo);
		getCharacter(kCharacterIvo).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterIvo).characterPosition.position = 2740;
		getCharacter(kCharacterIvo).characterPosition.location = 1;
		getCharacter(kCharacterIvo).clothes = 0;
		getCharacter(kCharacterIvo).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_InComp3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_InComp3);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 23;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_InComp3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(39, kCharacterCath, 3, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_StartPart4);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 24;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Ivo_AtDinner4(0, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterIvo).characterPosition.location = 1;
		getCharacter(kCharacterIvo).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterIvo).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_AtDinner4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_AtDinner4);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 25;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_AtDinner4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2361600 && rcClear()) {
			getCharacter(kCharacterIvo).characterPosition.location = 0;
			CONS_Ivo_ReturnComp4(0, 0, 0, 0);
		}

		break;
	case 12:
		send(kCharacterIvo, kCharacterTableC, 136455232, 0);
		startCycOtis(kCharacterIvo, "023B");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_ReturnComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_ReturnComp4);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 26;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_ReturnComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 1;
		IvoCall(&LogicManager::CONS_Ivo_DoSplitOtis023D, 0, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] == 1) {
			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 2;
			IvoCall(&LogicManager::CONS_Ivo_GoCompartment, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] == 2) {
			CONS_Ivo_InComp4(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_InComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_InComp4);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 27;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_InComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(39, kCharacterCath, 3, 10, 9);
		break;
	case 18:
		switch (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterIvo).characterPosition.location = 0;

			if (getCharacter(kCharacterIvo).characterPosition.position < 2087)
				getCharacter(kCharacterIvo).characterPosition.position = 2088;

			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 2;
			IvoCall(&LogicManager::CONS_Ivo_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			endGraphics(kCharacterIvo);
			CONS_Ivo_Hiding(0, 0, 0, 0);
			break;
		case 3:
			startCycOtis(kCharacterIvo, "613Ch");
			softBlockAtDoor(kCharacterIvo, 39);
			send(kCharacterIvo, kCharacterCond2, 88652208, 0);
			break;
		case 4:
			softReleaseAtDoor(kCharacterIvo, 39);
			getCharacter(kCharacterIvo).characterPosition.position = 2740;
			getCharacter(kCharacterIvo).characterPosition.location = 1;
			endGraphics(kCharacterIvo);
			break;
		default:
			break;
		}

		break;
	case 123852928:
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 4;
		IvoCall(&LogicManager::CONS_Ivo_DoCorrOtis, "613Dh", 39, 0, 0);
		break;
	case 221683008:
		send(kCharacterIvo, kCharacterCond2, 123199584, 0);
		break;
	case 122865568:
		getCharacter(kCharacterIvo).characterPosition.location = 0;
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 3;
		IvoCall(&LogicManager::CONS_Ivo_DoCorrOtis, "613Bh", 39, 0, 0);
		break;
	case 55996766:
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 1;
		IvoCall(&LogicManager::CONS_Ivo_DoCorrOtis, "613FH", 39, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_Hiding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_Hiding);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 28;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_Hiding(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2425500 && !getCharacterCurrentParams(kCharacterIvo)[0]) {
			getCharacterCurrentParams(kCharacterIvo)[0] = 1;
			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 1;
			IvoCall(&LogicManager::CONS_Ivo_DoWalk, 4, 2740, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] == 1) {
			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 2;
			IvoCall(&LogicManager::CONS_Ivo_DoCorrOtis, "613EH", 39, 0, 0);
		} else if (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] == 2) {
			CONS_Ivo_EndPart4(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_EndPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_EndPart4);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 29;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_EndPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterIvo);
		setDoor(39, kCharacterCath, 3, 10, 9);
		getCharacter(kCharacterIvo).characterPosition.location = 1;
		getCharacter(kCharacterIvo).characterPosition.position = 2740;
		getCharacter(kCharacterIvo).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterIvo).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_StartPart5);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 30;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Ivo_GoofingOff(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterIvo);
		getCharacter(kCharacterIvo).characterPosition.location = kCarBaggageRear;
		getCharacter(kCharacterIvo).characterPosition.position = 540;
		getCharacter(kCharacterIvo).characterPosition.car = kCarBaggageRear;
		getCharacter(kCharacterIvo).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_GoofingOff(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_GoofingOff);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 31;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_GoofingOff(HAND_PARAMS) {
	switch (msg->action) {
	case 192637492:
		CONS_Ivo_FightCath(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_FightCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_FightCath);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 32;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_FightCath(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterIvo).characterPosition.location = 0;
		getCharacter(kCharacterIvo).characterPosition.position = 540;
		getCharacter(kCharacterIvo).characterPosition.car = 1;
		getCharacter(kCharacterIvo).inventoryItem = 0;
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 1;
		IvoCall(&LogicManager::CONS_Ivo_SaveGame, 2, kEventCathIvoFight, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] == 1) {
			playDialog(0, "LIB090", -1, 0);
			playNIS(kEventCathIvoFight);
			getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 2;
			IvoCall(&LogicManager::CONS_Ivo_SaveGame, 1, 0, 0, 0);
		} else if (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] == 2) {
			int fightOutcome = playFight(2003);
			getCharacterCurrentParams(kCharacterIvo)[0] = fightOutcome;
			if (fightOutcome) {
				endGame(0, 0, 0, fightOutcome == 1);
			} else {
				bumpCath(kCarBaggageRear, 96, 255);
				CONS_Ivo_KnockedOut(0, 0, 0, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_KnockedOut(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_KnockedOut);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 33;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_KnockedOut(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		_gameTime += 1800;
		getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] = 1;
		IvoCall(&LogicManager::CONS_Ivo_SaveGame, 1, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall + 8] == 1)
			setDoor(94, kCharacterCath, 2, 255, 255);

		break;
	case 135800432:
		CONS_Ivo_Disappear(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Ivo_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterIvo).callParams[getCharacter(kCharacterIvo).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterIvo, &LogicManager::HAND_Ivo_Disappear);
	getCharacter(kCharacterIvo).callbacks[getCharacter(kCharacterIvo).currentCall] = 34;

	params->clear();

	fedEx(kCharacterIvo, kCharacterIvo, 12, 0);
}

void LogicManager::HAND_Ivo_Disappear(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterIvo);
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsIvo[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Ivo_DebugWalks,
	&LogicManager::HAND_Ivo_DoSeqOtis,
	&LogicManager::HAND_Ivo_DoCorrOtis,
	&LogicManager::HAND_Ivo_DoWait,
	&LogicManager::HAND_Ivo_DoWaitReal,
	&LogicManager::HAND_Ivo_DoWalk,
	&LogicManager::HAND_Ivo_FinishSeqOtis,
	&LogicManager::HAND_Ivo_DoDialog,
	&LogicManager::HAND_Ivo_WaitRCClear,
	&LogicManager::HAND_Ivo_SaveGame,
	&LogicManager::HAND_Ivo_GoCompartment,
	&LogicManager::HAND_Ivo_DoSplitOtis023A,
	&LogicManager::HAND_Ivo_DoSplitOtis023D,
	&LogicManager::HAND_Ivo_Birth,
	&LogicManager::HAND_Ivo_ReturnComp,
	&LogicManager::HAND_Ivo_InComp,
	&LogicManager::HAND_Ivo_Asleep,
	&LogicManager::HAND_Ivo_StartPart2,
	&LogicManager::HAND_Ivo_GoBreakfast,
	&LogicManager::HAND_Ivo_AtBreakfast,
	&LogicManager::HAND_Ivo_InComp2,
	&LogicManager::HAND_Ivo_StartPart3,
	&LogicManager::HAND_Ivo_InComp3,
	&LogicManager::HAND_Ivo_StartPart4,
	&LogicManager::HAND_Ivo_AtDinner4,
	&LogicManager::HAND_Ivo_ReturnComp4,
	&LogicManager::HAND_Ivo_InComp4,
	&LogicManager::HAND_Ivo_Hiding,
	&LogicManager::HAND_Ivo_EndPart4,
	&LogicManager::HAND_Ivo_StartPart5,
	&LogicManager::HAND_Ivo_GoofingOff,
	&LogicManager::HAND_Ivo_FightCath,
	&LogicManager::HAND_Ivo_KnockedOut,
	&LogicManager::HAND_Ivo_Disappear
};

} // End of namespace LastExpress
