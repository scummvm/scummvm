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
#include "lastexpress/game/logic.h"

namespace LastExpress {

void LogicManager::CONS_Alexei(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterAlexei,
			_functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]
		);

		break;
	case 1:
		CONS_Alexei_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Alexei_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Alexei_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Alexei_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Alexei_StartPart5(0, 0, 0, 0);
		break;
	default:
		return;
	}
}

void LogicManager::AlexeiCall(CALL_PARAMS) {
	getCharacter(kCharacterAlexei).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Alexei_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_DebugWalks);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 1;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterAlexei, kCarGreenSleeping, getCharacterCurrentParams(kCharacterAlexei)[0])) {
			if (getCharacterCurrentParams(kCharacterAlexei)[0] == 10000)
				getCharacterCurrentParams(kCharacterAlexei)[0] = 0;
			else
				getCharacterCurrentParams(kCharacterAlexei)[0] = 10000;
		}

		break;
	case 12:
		getCharacter(kCharacterAlexei).characterPosition.position = 0;
		getCharacter(kCharacterAlexei).characterPosition.location = 0;
		getCharacter(kCharacterAlexei).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterAlexei)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_DoDialog);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterAlexei).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
		fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		break;
	case 12:
		playDialog(kCharacterAlexei, (char *)&getCharacterCurrentParams(kCharacterAlexei)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_DoWait);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 3;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAlexei)[1] ||
			(getCharacterCurrentParams(kCharacterAlexei)[1] = _gameTime + getCharacterCurrentParams(kCharacterAlexei)[0], _gameTime + getCharacterCurrentParams(kCharacterAlexei)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterAlexei)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterAlexei)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterAlexei).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
		fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_DoSeqOtis);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterAlexei).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
		fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAlexei, (char *)&getCharacterCurrentParams(kCharacterAlexei)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_DoBlockSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_DoBlockSeqOtis);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_DoBlockSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseView(kCharacterAlexei, getCharacterCurrentParams(kCharacterAlexei)[3], getCharacterCurrentParams(kCharacterAlexei)[4]);

		getCharacter(kCharacterAlexei).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
		fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAlexei, (char *)&getCharacterCurrentParams(kCharacterAlexei)[0]);
		blockView(kCharacterAlexei, getCharacterCurrentParams(kCharacterAlexei)[3], getCharacterCurrentParams(kCharacterAlexei)[4]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_DoCorrOtis);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterAlexei, getCharacterCurrentParams(kCharacterAlexei)[3]);

		getCharacter(kCharacterAlexei).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
		fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAlexei, (char *)&getCharacterCurrentParams(kCharacterAlexei)[0]);
		blockAtDoor(kCharacterAlexei, getCharacterCurrentParams(kCharacterAlexei)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_FinishSeqOtis);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 7;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterAlexei).direction != 4) {
			getCharacter(kCharacterAlexei).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
			fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterAlexei).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
		fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_DoJoinedSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_DoJoinedSeqOtis);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 8;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	strncpy((char *)&params->parameters[5], param4.stringParam, 12);

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_DoJoinedSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		if (!getCharacterCurrentParams(kCharacterAlexei)[8])
			fedEx(kCharacterAlexei, getCharacterCurrentParams(kCharacterAlexei)[3], getCharacterCurrentParams(kCharacterAlexei)[4], (char *)&getCharacterCurrentParams(kCharacterAlexei)[5]);

		getCharacter(kCharacterAlexei).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
		fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		break;
	case 10:
		if (!getCharacterCurrentParams(kCharacterAlexei)[8]) {
			fedEx(kCharacterAlexei, getCharacterCurrentParams(kCharacterAlexei)[3], getCharacterCurrentParams(kCharacterAlexei)[4], (char *)&getCharacterCurrentParams(kCharacterAlexei)[5]);
			getCharacterCurrentParams(kCharacterAlexei)[8] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterAlexei, (char *)&getCharacterCurrentParams(kCharacterAlexei)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_SaveGame);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 9;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterAlexei).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
		fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		break;
	case 12:
		save(
			kCharacterAlexei,
			getCharacterCurrentParams(kCharacterAlexei)[0],
			getCharacterCurrentParams(kCharacterAlexei)[1]
		);

		getCharacter(kCharacterAlexei).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
		fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_DoWalk);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterAlexei, getCharacterCurrentParams(kCharacterAlexei)[0], getCharacterCurrentParams(kCharacterAlexei)[1])) {
			getCharacter(kCharacterAlexei).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
			fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		}

		break;
	case 5:
		if (checkCathDir(kCarGreenSleeping, 18) || checkCathDir(kCarRedSleeping, 18)) {
			playChrExcuseMe(kCharacterAlexei, kCharacterCath, 0);
			break;
		}

		if (_doneNIS[kEventAlexeiSalonVassili] || (_doneNIS[kEventTatianaAskMatchSpeakRussian] && cathHasItem(kItemPassengerList))) {
			if (rnd(2) == 0) {
				playDialog(kCharacterCath, "CAT1012A", -1, 0);
			} else {
				playDialog(kCharacterCath, "CAT1012", -1, 0);
			}			
		} else {
			playCathExcuseMe();
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterAlexei, kCharacterCath, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_DoComplexSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_DoComplexSeqOtis);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 11;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_DoComplexSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterAlexei).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
		fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAlexei, (char *)&getCharacterCurrentParams(kCharacterAlexei)[0]);
		startSeqOtis(getCharacterCurrentParams(kCharacterAlexei)[6], (char *)&getCharacterCurrentParams(kCharacterAlexei)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_WaitRCClear);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 12;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterAlexei).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
			fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_EnterComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_EnterComp);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 13;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_EnterComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 3, 7500, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1) {
			send(kCharacterAlexei, kCharacterCond1, 302614416, 0);
			startCycOtis(kCharacterAlexei, "602DB");
			softBlockAtDoor(kCharacterAlexei, 2);
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			if (inComp(kCharacterCath, kCarGreenSleeping, 7500)) {
				playNIS((kEventCathTurningNight - !isNight()));
				playDialog(0, "BUMP", -1, 0);
				bumpCathFDoor(2);
			}
		} else if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 2) {
			softReleaseAtDoor(kCharacterAlexei, 2);
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			getCharacter(kCharacterAlexei).characterPosition.position = 7500;
			endGraphics(kCharacterAlexei);

			getCharacter(kCharacterAlexei).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
			fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		}

		break;
	case 135664192:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
		AlexeiCall(&LogicManager::CONS_Alexei_DoCorrOtis, "602Eb", 2, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_ExitComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_ExitComp);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 14;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_ExitComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_DoCorrOtis, "602Fb", 2, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1) {
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			send(kCharacterAlexei, kCharacterCond1, 302614416, 0);
			startCycOtis(kCharacterAlexei, "602DB");
			softBlockAtDoor(kCharacterAlexei, 2);
		}

		break;
	case 135664192:
		setDoor(2, kCharacterCath, 1, 10, 9);
		softReleaseAtDoor(kCharacterAlexei, 2);

		getCharacter(kCharacterAlexei).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
		fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_PacingAtWindow(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_PacingAtWindow);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 15;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_PacingAtWindow(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if ((!getCharacterCurrentParams(kCharacterAlexei)[1] && (getCharacterCurrentParams(kCharacterAlexei)[1] = _gameTime + getCharacterCurrentParams(kCharacterAlexei)[0], _gameTime + getCharacterCurrentParams(kCharacterAlexei)[0] == 0)) ||
			getCharacterCurrentParams(kCharacterAlexei)[1] < _gameTime) {
			if (rcClear()) {
				getCharacter(kCharacterAlexei).characterPosition.location = 0;
				getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
				AlexeiCall(&LogicManager::CONS_Alexei_DoBlockSeqOtis, "103D", 5, 52, 0);
			}
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterAlexei)[0] = 5 * (3 * rnd(60) + 90);
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_DoBlockSeqOtis, "103C", 5, 52, 0);
			break;
		case 2:
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			startCycOtis(kCharacterAlexei, "103E");
			break;
		case 3:
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			startCycOtis(kCharacterAlexei, "103B");

			getCharacter(kCharacterAlexei).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
			fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_CompLogic(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_CompLogic);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 16;

	params->clear();

	params->parameters[0] = param1.intParam;

	strncpy((char *)&params->parameters[1], param2.stringParam, 12);

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_CompLogic(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterAlexei)[6] && getCharacterCurrentParams(kCharacterAlexei)[0] < _gameTime && !getCharacterCurrentParams(kCharacterAlexei)[7]) {
			getCharacterCurrentParams(kCharacterAlexei)[7] = 1;
			setDoor(2, kCharacterCath, 1, 10, 9);
			setDoor(18, kCharacterCath, 1, 10, 9);

			getCharacter(kCharacterAlexei).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
			fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
		} else {
			if (getCharacterCurrentParams(kCharacterAlexei)[4]) {
				if (getCharacterCurrentParams(kCharacterAlexei)[8] || (getCharacterCurrentParams(kCharacterAlexei)[8] = _realTime + 75, _realTime != -75)) {
					if (getCharacterCurrentParams(kCharacterAlexei)[8] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterAlexei)[8] = 0x7FFFFFFF;
				}

				getCharacterCurrentParams(kCharacterAlexei)[4] = 0;
				getCharacterCurrentParams(kCharacterAlexei)[5] = 1;
				setDoor(2, kCharacterAlexei, 1, 0, 0);
				setDoor(18, kCharacterAlexei, 1, 0, 0);
				getCharacterCurrentParams(kCharacterAlexei)[8] = 0;
			} else {
				getCharacterCurrentParams(kCharacterAlexei)[8] = 0;
			}
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterAlexei)[4]) {
			setDoor(2, kCharacterAlexei, 1, 0, 0);
			setDoor(18, kCharacterAlexei, 1, 0, 0);

			if (msg->param.intParam == 18) {
				getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 4;
				AlexeiCall(&LogicManager::CONS_Alexei_DoDialog, getCathWCDialog(), 0, 0, 0);
			} else if (cathHasItem(kItemPassengerList)) {
				getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 5;

				if (rnd(2) != 0) {
					AlexeiCall(&LogicManager::CONS_Alexei_DoDialog, getCathSorryDialog(), 0, 0, 0);
				} else {
					AlexeiCall(&LogicManager::CONS_Alexei_DoDialog, "CAT1503", 0, 0, 0);
				}
			} else {
				getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 6;
				AlexeiCall(&LogicManager::CONS_Alexei_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else {
			setDoor(2, kCharacterAlexei, 1, 0, 0);
			setDoor(18, kCharacterAlexei, 1, 0, 0);

			if (msg->action == 8) {
				getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
				AlexeiCall(&LogicManager::CONS_Alexei_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
				AlexeiCall(&LogicManager::CONS_Alexei_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		startCycOtis(kCharacterAlexei, (char *)&getCharacterCurrentParams(kCharacterAlexei)[1]);
		setDoor(2, kCharacterAlexei, 1, 10, 9);
		setDoor(18, kCharacterAlexei, 1, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterAlexei)[5] || getCharacterCurrentParams(kCharacterAlexei)[4]) {
			setDoor(2, kCharacterAlexei, 1, 10, 9);
			setDoor(18, kCharacterAlexei, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAlexei)[5] = 0;
			getCharacterCurrentParams(kCharacterAlexei)[4] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
		case 2:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_DoDialog, "ALX1134A", 0, 0, 0);
			break;
		case 3:
			setDoor(2, kCharacterAlexei, 1, 14, 0);
			setDoor(18, kCharacterAlexei, 1, 14, 0);
			getCharacterCurrentParams(kCharacterAlexei)[4] = 1;
			break;
		case 4:
		case 5:
		case 6:
			getCharacterCurrentParams(kCharacterAlexei)[4] = 0;
			getCharacterCurrentParams(kCharacterAlexei)[5] = 1;
			break;
		case 7:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 8;
			AlexeiCall(&LogicManager::CONS_Alexei_DoWait, 0x12C, 0, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 9;
			AlexeiCall(&LogicManager::CONS_Alexei_DoCorrOtis, "602Gb", 2, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			send(kCharacterAlexei, kCharacterCond1, 156567128, 0);
			startCycOtis(kCharacterAlexei, "602Hb");
			softBlockAtDoor(kCharacterAlexei, 2);
			break;
		case 10:
			softReleaseAtDoor(kCharacterAlexei, 2);
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			getCharacter(kCharacterAlexei).characterPosition.position = 7500;
			startCycOtis(kCharacterAlexei, (char *)&getCharacterCurrentParams(kCharacterAlexei)[1]);
			setDoor(2, kCharacterAlexei, 1, 10, 9);
			setDoor(18, kCharacterAlexei, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAlexei)[6] = 0;
			break;
		default:
			break;
		}

		break;
	case 124697504:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 10;
		AlexeiCall(&LogicManager::CONS_Alexei_DoCorrOtis, "602Ib", 2, 0, 0);
		break;
	case 221617184:
		getCharacterCurrentParams(kCharacterAlexei)[6] = 1;
		send(kCharacterAlexei, kCharacterCond1, 100906246, 0);
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 7;
		AlexeiCall(&LogicManager::CONS_Alexei_DoDialog, "CON1024", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_Birth);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 17;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterAlexei)[0]) {
			getCharacterCurrentParams(kCharacterAlexei)[0] = 1;
			CONS_Alexei_AtDinner(0, 0, 0, 0);
		}

		break;
	case 12:
		setDoor(2, kCharacterCath, 1, 10, 9);
		setDoor(10, kCharacterCath, 0, 255, 255);
		setDoor(18, kCharacterCath, 1, 10, 9);
		getCharacter(kCharacterAlexei).characterPosition.position = 3969;
		getCharacter(kCharacterAlexei).characterPosition.location = 1;
		getCharacter(kCharacterAlexei).characterPosition.car = kCarRestaurant;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_AtDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_AtDinner);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 18;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_AtDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1089000 && rcClear()) {
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			getCharacterCurrentParams(kCharacterAlexei)[1] = 0;
			getCharacter(kCharacterAlexei).inventoryItem = 0;
			blockView(kCharacterAlexei, kCarRestaurant, 63);
			dropItem(kItem17, 1);
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
			AlexeiCall(&LogicManager::CONS_Alexei_DoJoinedSeqOtis, "005D", 34, 103798704, "005E");
		} else {
			if (getCharacterCurrentParams(kCharacterAlexei)[0]) {
				if (getCharacterCurrentParams(kCharacterAlexei)[2] || (getCharacterCurrentParams(kCharacterAlexei)[2] = _realTime + 90, _realTime != -90)) {
					if (getCharacterCurrentParams(kCharacterAlexei)[2] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterAlexei)[2] = 0x7FFFFFFF;
				}

				bumpCath(kCarRestaurant, 61, 255);
			} else {
				getCharacterCurrentParams(kCharacterAlexei)[2] = 0;
			}
		}

		break;
	case 1:
		getCharacterCurrentParams(kCharacterAlexei)[1] = 0;
		getCharacter(kCharacterAlexei).inventoryItem = 0;
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
		AlexeiCall(&LogicManager::CONS_Alexei_SaveGame, 2, kEventAlexeiDiner, 0, 0);
		break;
	case 12:
		send(kCharacterAlexei, kCharacterTableB, 136455232, 0);
		startCycOtis(kCharacterAlexei, "005B");
		getCharacterCurrentParams(kCharacterAlexei)[1] = 128;
		getCharacter(kCharacterAlexei).inventoryItem = 0x80;
		break;
	case 17:
		getCharacterCurrentParams(kCharacterAlexei)[0] = checkCathDir(kCarRestaurant, 63);
		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1) {
			releaseView(kCharacterAlexei, kCarRestaurant, 63);
			CONS_Alexei_ReturnComp(0, 0, 0, 0);
		} else if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 2) {
			if (_globals[kGlobalJacket] == 2)
				playNIS(kEventAlexeiDiner);
			else
				playNIS(kEventAlexeiDinerOriginalJacket);

			send(kCharacterAlexei, kCharacterTableB, 103798704, "005E");
			getCharacter(kCharacterAlexei).characterPosition.position = 3650;
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			endGraphics(kCharacterAlexei);
			_items[kItem17].floating = 1;
			bumpCath(kCarRestaurant, 63, 255);
			CONS_Alexei_ReturnComp(0, 0, 0, 0);
		}

		break;
	case 168046720:
		getCharacter(kCharacterAlexei).inventoryItem = 0;
		break;
	case 168627977:
		getCharacter(kCharacterAlexei).inventoryItem = getCharacterCurrentParams(kCharacterAlexei)[1];
		break;
	case 225182640:
		getCharacter(kCharacterAlexei).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_ReturnComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_ReturnComp);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 19;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_ReturnComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_DoSeqOtis, "811DS", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 3, 9460, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 5, 850, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 4;
			AlexeiCall(&LogicManager::CONS_Alexei_WaitRCClear, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAlexei).characterPosition.position = 1540;
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 5;
			AlexeiCall(&LogicManager::CONS_Alexei_DoSeqOtis, "811US", 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 6;
			AlexeiCall(&LogicManager::CONS_Alexei_DoSeqOtis, "933", 0, 0, 0);
			break;
		case 6:
			blockView(kCharacterAlexei, kCarRestaurant, 63);
			takeItem(kItem17);
			send(kCharacterAlexei, kCharacterTableB, 136455232, 0);
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 7;
			AlexeiCall(&LogicManager::CONS_Alexei_DoJoinedSeqOtis, "005F", 34, 103798704, "005G");
			break;
		case 7:
			releaseView(kCharacterAlexei, kCarRestaurant, 63);
			send(kCharacterAlexei, kCharacterWaiter2, 302996448, 0);
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 8;
			AlexeiCall(&LogicManager::CONS_Alexei_DoSeqOtis, "934", 0, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 9;
			AlexeiCall(&LogicManager::CONS_Alexei_DoSeqOtis, "811DS", 0, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 10;
			AlexeiCall(&LogicManager::CONS_Alexei_EnterComp, 0, 0, 0, 0);
			break;
		case 10:
			if (checkCathDir(kCarGreenSleeping, 61))
				bumpCath(kCarGreenSleeping, 49, 255);

			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 11;
			AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 1098000, "411", 0, 0);
			break;
		case 11:
			CONS_Alexei_GoSalon(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_GoSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_GoSalon);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 20;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_GoSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_ExitComp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAlexei).characterPosition.position = 1540;
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 4;
			AlexeiCall(&LogicManager::CONS_Alexei_DoBlockSeqOtis, "103A", 5, 52, 0);
			break;
		case 4:
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			CONS_Alexei_Sitting(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_Sitting(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_Sitting);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 21;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_Sitting(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if ((!getCharacterCurrentParams(kCharacterAlexei)[1] && (getCharacterCurrentParams(kCharacterAlexei)[1] = _gameTime + getCharacterCurrentParams(kCharacterAlexei)[0], _gameTime + getCharacterCurrentParams(kCharacterAlexei)[0] == 0)) || getCharacterCurrentParams(kCharacterAlexei)[1] < _gameTime) {
			if (rcClear()) {
				getCharacter(kCharacterAlexei).characterPosition.location = 0;
				getCharacter(kCharacterAlexei).inventoryItem = 0;
				getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
				AlexeiCall(&LogicManager::CONS_Alexei_DoBlockSeqOtis, "103C", 5, 52, 0);
			}
		}

		break;
	case 1:
		getCharacter(kCharacterAlexei).inventoryItem = 0;
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
		AlexeiCall(&LogicManager::CONS_Alexei_SaveGame, 2, kEventAlexeiSalonPoem, 0, 0);
		break;
	case 12:
		startCycOtis(kCharacterAlexei, "103B");
		getCharacterCurrentParams(kCharacterAlexei)[0] = 225 * (4 * rnd(3) + 4);

		if (!_doneNIS[kEventAlexeiSalonPoem])
			getCharacter(kCharacterAlexei).inventoryItem = 10;

		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			CONS_Alexei_StandingAtWindow(0, 0, 0, 0);
			break;
		case 2:
			playNIS(kEventAlexeiSalonPoem);
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			startSeqOtis(kCharacterAlexei, "103D");
			bumpCath(kCarRestaurant, 55, 255);
			blockView(kCharacterAlexei, kCarRestaurant, 52);
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 3:
			startCycOtis(kCharacterAlexei, "103B");
			releaseView(kCharacterAlexei, kCarRestaurant, 52);
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_StandingAtWindow(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_StandingAtWindow);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 22;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_StandingAtWindow(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if ((!getCharacterCurrentParams(kCharacterAlexei)[1] && (getCharacterCurrentParams(kCharacterAlexei)[1] = _gameTime + getCharacterCurrentParams(kCharacterAlexei)[0], _gameTime + getCharacterCurrentParams(kCharacterAlexei)[0] == 0)) ||
			getCharacterCurrentParams(kCharacterAlexei)[1] < _gameTime) {
			if (rcClear()) {
				getCharacter(kCharacterAlexei).characterPosition.location = 0;
				getCharacter(kCharacterAlexei).inventoryItem = 0;
				getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
				AlexeiCall(&LogicManager::CONS_Alexei_DoBlockSeqOtis, "103D", 5, 52, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterAlexei)[2] != 0x7FFFFFFF && _gameTime > 1111500) {
			if (_gameTime > 1138500) {
				getCharacterCurrentParams(kCharacterAlexei)[2] = 0x7FFFFFFF;
				getCharacter(kCharacterAlexei).inventoryItem = 0;
				CONS_Alexei_WaitingForTat(0, 0, 0, 0);
				break;
			}

			if (((inSalon(kCharacterCath) || inDiningRoom(kCharacterCath)) && getCharacterCurrentParams(kCharacterAlexei)[2]) || (getCharacterCurrentParams(kCharacterAlexei)[2] = _gameTime, _gameTime)) {
				if (getCharacterCurrentParams(kCharacterAlexei)[2] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterAlexei)[2] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterAlexei).inventoryItem = 0;
			CONS_Alexei_WaitingForTat(0, 0, 0, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
		AlexeiCall(&LogicManager::CONS_Alexei_SaveGame, 2, kEventAlexeiSalonPoem, 0, 0);
		break;
	case 12:
		getCharacterCurrentParams(kCharacterAlexei)[0] = 225 * (4 * rnd(4) + 8);
		startCycOtis(kCharacterAlexei, "103E");
		if (!_doneNIS[kEventAlexeiSalonPoem])
			getCharacter(kCharacterAlexei).inventoryItem = 10;

		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			CONS_Alexei_Sitting(0, 0, 0, 0);
			break;
		case 2:
			playNIS(kEventAlexeiSalonPoem);
			getCharacter(kCharacterAlexei).inventoryItem = 0;
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			startSeqOtis(kCharacterAlexei, "103D");
			blockView(kCharacterAlexei, kCarRestaurant, 52);
			bumpCath(kCarRestaurant, 55, 255);
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 3:
			releaseView(kCharacterAlexei, kCarRestaurant, 52);
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			CONS_Alexei_Sitting(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_WaitingForTat(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_WaitingForTat);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 23;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_WaitingForTat(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!inDiningRoom(kCharacterTatiana) || _doneNIS[kEventAlexeiSalonPoem]) {
			getCharacter(kCharacterAlexei).inventoryItem = 0;
		} else {
			getCharacter(kCharacterAlexei).inventoryItem = 10;
		}

		break;
	case 1:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
		AlexeiCall(&LogicManager::CONS_Alexei_SaveGame, 2, kEventAlexeiSalonPoem, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterAlexei).characterPosition.location = 1;
		send(kCharacterAlexei, kCharacterTatiana, 124973510, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1) {
			playNIS(kEventAlexeiSalonVassili);
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			startCycOtis(kCharacterAlexei, "103F");
			cleanNIS();
			CONS_Alexei_Upset(0, 0, 0, 0);
		} else {
			if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 2) {
				playNIS(kEventAlexeiSalonPoem);
				getCharacter(kCharacterAlexei).inventoryItem = 0;
				bumpCath(kCarRestaurant, 55, 255);
			}
		}

		break;
	case 157159392:
		if (inSalon(kCharacterCath)) {
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
			AlexeiCall(&LogicManager::CONS_Alexei_SaveGame, 2, kEventAlexeiSalonVassili, 0, 0);
		}

		break;
	case 188784532:
		CONS_Alexei_Upset(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_Upset(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_Upset);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 24;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_Upset(HAND_PARAMS) {
	switch (msg->action) {
	case 1:
		getCharacter(kCharacterAlexei).inventoryItem = 0;
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_SaveGame, 2, kEventAlexeiSalonCath, 0, 0);
		break;
	case 12:
		if (_doneNIS[kEventAlexeiSalonVassili])
			getCharacter(kCharacterAlexei).inventoryItem = 0x80;

		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1) {
			playNIS(kEventAlexeiSalonCath);
			getCharacter(kCharacterAlexei).characterPosition.car = kCarRedSleeping;
			getCharacter(kCharacterAlexei).characterPosition.position = 9460;
			endGraphics(kCharacterAlexei);
			bumpCath(kCarRestaurant, 55, 255);
			CONS_Alexei_ReturnCompNight(0, 0, 0, 0);
		} else if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 2) {
			CONS_Alexei_ReturnCompNight(0, 0, 0, 0);
		}

		break;
	case 135854208:
		getCharacter(kCharacterAlexei).inventoryItem = 0;
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
		AlexeiCall(&LogicManager::CONS_Alexei_DoSeqOtis, "103G", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_ReturnCompNight(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_ReturnCompNight);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 25;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_ReturnCompNight(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_EnterComp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			if (checkCathDir(kCarGreenSleeping, 61))
				bumpCath(kCarGreenSleeping, 49, 255);

			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 1179000, "411", 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 1323000, "412", 0, 0);
			break;
		case 3:
			CONS_Alexei_Asleep(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_Asleep);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 26;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1512000 && !getCharacterCurrentParams(kCharacterAlexei)[0]) {
			getCharacterCurrentParams(kCharacterAlexei)[0] = 1;
			CONS_Alexei_WakeUp(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterAlexei).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAlexei).characterPosition.position = 7500;
		getCharacter(kCharacterAlexei).characterPosition.location = 1;
		setDoor(2, kCharacterCath, 1, 10, 9);
		setDoor(18, kCharacterCath, 1, 10, 9);
		setDoor(10, kCharacterCath, 1, 255, 255);

		if (checkCathDir(kCarGreenSleeping, 61))
			bumpCath(kCarGreenSleeping, 66, 255);

		endGraphics(kCharacterAlexei);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_WakeUp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_WakeUp);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 27;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_WakeUp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(10, kCharacterCath, 0, 255, 255);

		if (checkCathDir(kCarGreenSleeping, 66))
			bumpCath(kCarGreenSleeping, 49, 255);

		startCycOtis(kCharacterAlexei, "412");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_StartPart2);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 28;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Alexei_InComp2(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAlexei);
		getCharacter(kCharacterAlexei).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAlexei).characterPosition.position = 7500;
		getCharacter(kCharacterAlexei).characterPosition.location = 1;
		getCharacter(kCharacterAlexei).inventoryItem = 0;
		getCharacter(kCharacterAlexei).clothes = 0;
		setDoor(2, kCharacterAlexei, 1, 10, 9);
		setDoor(18, kCharacterAlexei, 1, 10, 9);
		setDoor(10, kCharacterCath, 0, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_InComp2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_InComp2);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 29;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_InComp2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 1791000, "411", 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_ExitComp, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 5, 850, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 4;
			AlexeiCall(&LogicManager::CONS_Alexei_WaitRCClear, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAlexei).characterPosition.position = 1540;
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 5;
			AlexeiCall(&LogicManager::CONS_Alexei_DoSeqOtis, "811US", 0, 0, 0);
			break;
		case 5:
			blockView(kCharacterAlexei, kCarRestaurant, 63);
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 6;
			AlexeiCall(&LogicManager::CONS_Alexei_DoJoinedSeqOtis, "018B", 34, 136455232, "BOGUS");
			break;
		case 6:
			releaseView(kCharacterAlexei, kCarRestaurant, 63);
			send(kCharacterAlexei, kCharacterTatiana, 290869168, 0);
			CONS_Alexei_AtBreakfast(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_AtBreakfast(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_AtBreakfast);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 30;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_AtBreakfast(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(2, kCharacterCath, 1, 10, 9);
		getCharacter(kCharacterAlexei).characterPosition.car = 5;
		getCharacter(kCharacterAlexei).characterPosition.location = 1;
		startCycOtis(kCharacterAlexei, "018C");
		send(kCharacterAlexei, kCharacterTableB, 136455232, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			releaseView(kCharacterAlexei, kCarRestaurant, 63);
			send(kCharacterAlexei, kCharacterTatiana, 156444784, 0);
			startCycOtis(kCharacterAlexei, "018E");

			if (inDiningRoom(kCharacterCath))
				_globals[kGlobalOverheardTatianaAndAlexeiAtBreakfast] = 1;

			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_DoDialog, "TAT2116", 0, 0, 0);
			break;
		case 2:
			playDialog(kCharacterAlexei, "TAT2116A", -1, 0);
			blockView(kCharacterAlexei, kCarRestaurant, 63);
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_DoJoinedSeqOtis, "018F", 10, 123857088, "BOGUS");
			break;
		case 3:
			releaseView(kCharacterAlexei, kCarRestaurant, 63);
			CONS_Alexei_ReturnCompPart2(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 236053296:
		startSeqOtis(kCharacterAlexei, "018D1");
		startSeqOtis(kCharacterTatiana, "018D2");
		blockView(kCharacterAlexei, kCarRestaurant, 63);

		if (msg->param.intParam)
			bumpCath(kCarRestaurant, msg->param.intParam, 255);

		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_FinishSeqOtis, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_ReturnCompPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_ReturnCompPart2);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 31;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_ReturnCompPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		startSeqOtis(kCharacterAlexei, "811DS");

		if (inDiningRoom(kCharacterCath))
			advanceFrame(kCharacterAlexei);

		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_FinishSeqOtis, 0, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1) {
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_EnterComp, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 2) {
			if (checkCathDir(kCarGreenSleeping, 61))
				bumpCath(kCarGreenSleeping, 49, 255);

			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 15803100, "411", 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_StartPart3);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 32;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Alexei_PlayingChess(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAlexei);
		getCharacter(kCharacterAlexei).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterAlexei).characterPosition.location = 1;
		getCharacter(kCharacterAlexei).inventoryItem = 0;
		getCharacter(kCharacterAlexei).clothes = 0;
		setDoor(2, kCharacterCath, 1, 10, 9);
		setDoor(18, kCharacterCath, 1, 10, 9);
		setDoor(10, kCharacterCath, 0, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_PlayingChess(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_PlayingChess);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 33;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_PlayingChess(HAND_PARAMS) {
	switch (msg->action) {
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1)
			CONS_Alexei_InPart3(0, 0, 0, 0);

		break;
	case 122288808:
		getCharacter(kCharacterAlexei).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterAlexei).characterPosition.location = 0;
		getCharacter(kCharacterAlexei).characterPosition.position = 9270;
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_EnterComp, 0, 0, 0, 0);
		break;
	case 122358304:
		startCycOtis(kCharacterAlexei, "BLANK");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_InPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_InPart3);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 34;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_InPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (checkCathDir(kCarGreenSleeping, 61))
			bumpCath(kCarGreenSleeping, 49, 255);

		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 2083500, "411", 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_ExitComp, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 5, 850, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 4;
			AlexeiCall(&LogicManager::CONS_Alexei_WaitRCClear, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAlexei).characterPosition.position = 1540;
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 5;
			AlexeiCall(&LogicManager::CONS_Alexei_DoBlockSeqOtis, "103A", 5, 52, 0);
			break;
		case 5:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 6;
			AlexeiCall(&LogicManager::CONS_Alexei_Pacing3, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 7;
			AlexeiCall(&LogicManager::CONS_Alexei_EnterComp, 0, 0, 0, 0);
			break;
		case 7:
			setDoor(10, kCharacterCath, 1, 255, 255);

			if (checkCathDir(kCarGreenSleeping, 61))
				bumpCath(kCarGreenSleeping, 66, 255);

			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 8;
			AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 2124000, "NONE", 0, 0);
			break;
		case 8:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 9;
			AlexeiCall(&LogicManager::CONS_Alexei_ExitComp, 0, 0, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 10;
			AlexeiCall(&LogicManager::CONS_Alexei_GoSalon3, 0, 0, 0, 0);
			break;
		case 10:
			setDoor(10, kCharacterCath, 0, 255, 255);

			if (checkCathDir(kCarGreenSleeping, 66))
				bumpCath(kCarGreenSleeping, 49, 255);

			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 11;
			AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 16451100, "411", 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_Pacing3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_Pacing3);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 35;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_Pacing3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inSalon(kCharacterCath)) {
			if ((!getCharacterCurrentParams(kCharacterAlexei)[1] && (getCharacterCurrentParams(kCharacterAlexei)[1] = _gameTime + 2700, _gameTime == -2700)) || getCharacterCurrentParams(kCharacterAlexei)[1] < _gameTime) {
				getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
				AlexeiCall(&LogicManager::CONS_Alexei_WaitRCClear, 0, 0, 0, 0);
				break;
			}
		} else {
			getCharacterCurrentParams(kCharacterAlexei)[1] = 0;
		}
		if ((!getCharacterCurrentParams(kCharacterAlexei)[2] && (getCharacterCurrentParams(kCharacterAlexei)[2] = _gameTime + getCharacterCurrentParams(kCharacterAlexei)[0], _gameTime + getCharacterCurrentParams(kCharacterAlexei)[0] == 0)) || getCharacterCurrentParams(kCharacterAlexei)[2] < _gameTime) {
			if (rcClear()) {
				getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
				AlexeiCall(&LogicManager::CONS_Alexei_PacingAtWindow, 0, 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterAlexei)[3] || (getCharacterCurrentParams(kCharacterAlexei)[3] = _gameTime + 9000, _gameTime != -9000)) {
			if (getCharacterCurrentParams(kCharacterAlexei)[3] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterAlexei)[3] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 4;
		AlexeiCall(&LogicManager::CONS_Alexei_WaitRCClear, 0, 0, 0, 0);
		break;
	case 12:
		getCharacterCurrentParams(kCharacterAlexei)[0] = 15 * rnd(120);
		startCycOtis(kCharacterAlexei, "103B");
		getCharacter(kCharacterAlexei).characterPosition.location = 1;
		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_DoBlockSeqOtis, "124C", 5, 52, 0);
			break;
		case 2:
		case 5:
			getCharacter(kCharacterAlexei).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
			fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
			break;
		case 3:
			getCharacterCurrentParams(kCharacterAlexei)[0] = 15 * rnd(120);
			getCharacterCurrentParams(kCharacterAlexei)[2] = 0;

			if (getCharacterCurrentParams(kCharacterAlexei)[3] || (getCharacterCurrentParams(kCharacterAlexei)[3] = _gameTime + 9000, _gameTime != -9000)) {
				if (getCharacterCurrentParams(kCharacterAlexei)[3] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterAlexei)[3] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 4;
			AlexeiCall(&LogicManager::CONS_Alexei_WaitRCClear, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 5;
			AlexeiCall(&LogicManager::CONS_Alexei_DoBlockSeqOtis, "124C", 5, 52, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_GoSalon3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_GoSalon3);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 36;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_GoSalon3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterAlexei)[2] && !getCharacterCurrentParams(kCharacterAlexei)[1]) {
			if (getCharacterCurrentParams(kCharacterAlexei)[3] || (getCharacterCurrentParams(kCharacterAlexei)[3] = _realTime + getCharacterCurrentParams(kCharacterAlexei)[0],
																   _realTime + getCharacterCurrentParams(kCharacterAlexei)[0] != 0)) {
				if (getCharacterCurrentParams(kCharacterAlexei)[3] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAlexei)[3] = 0x7FFFFFFF;
			}

			startSeqOtis(kCharacterAlexei, "124B");
			getCharacterCurrentParams(kCharacterAlexei)[1] = 1;
			getCharacterCurrentParams(kCharacterAlexei)[3] = 0;
		}

		break;
	case 3:
		if (getCharacterCurrentParams(kCharacterAlexei)[1]) {
			startCycOtis(kCharacterAlexei, "124A");
			getCharacterCurrentParams(kCharacterAlexei)[0] = 5 * (3 * rnd(15) + 15);
			getCharacterCurrentParams(kCharacterAlexei)[1] = 0;
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterAlexei)[0] = 5 * (3 * rnd(15) + 15);
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 5, 850, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_WaitRCClear, 0, 0, 0, 0);
			break;
		case 2:
			send(kCharacterAlexei, kCharacterAbbot, 222609266, 0);
			getCharacter(kCharacterAlexei).characterPosition.position = 1540;
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_DoBlockSeqOtis, "103A", 5, 52, 0);
			break;
		case 3:
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			startCycOtis(kCharacterAlexei, "124A");
			break;
		case 4:
			getCharacter(kCharacterAlexei).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, _functionsAlexei[getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall]]);
			fedEx(kCharacterAlexei, kCharacterAlexei, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 122288808:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 4;
		AlexeiCall(&LogicManager::CONS_Alexei_EnterComp, 0, 0, 0, 0);
		break;
	case 122358304:
		startCycOtis(kCharacterAlexei, "BLANK");
		getCharacterCurrentParams(kCharacterAlexei)[2] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_StartPart4);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 37;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Alexei_InComp4(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAlexei);
		getCharacter(kCharacterAlexei).characterPosition.position = 7500;
		getCharacter(kCharacterAlexei).characterPosition.location = 1;
		getCharacter(kCharacterAlexei).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAlexei).inventoryItem = 0;
		setDoor(2, kCharacterCath, 1, 10, 9);
		setDoor(18, kCharacterCath, 1, 10, 9);
		setDoor(10, kCharacterCath, 0, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_InComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_InComp4);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 38;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_InComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 2354400, "411", 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1) {
			CONS_Alexei_MeetTat(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_MeetTat(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_MeetTat);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 39;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_MeetTat(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAlexei)[1]) {
			if (!getCharacterCurrentParams(kCharacterAlexei)[3]) {
				getCharacterCurrentParams(kCharacterAlexei)[2] = _gameTime + 4500;
				getCharacterCurrentParams(kCharacterAlexei)[3] = _gameTime + 9000;
			}
			if (getCharacterCurrentParams(kCharacterAlexei)[4] != 0x7FFFFFFF && getCharacterCurrentParams(kCharacterAlexei)[2] < _gameTime) {
				if (getCharacterCurrentParams(kCharacterAlexei)[3] >= _gameTime) {
					if (onLowPlatform(kCharacterCath) || !getCharacterCurrentParams(kCharacterAlexei)[4]) {
						getCharacterCurrentParams(kCharacterAlexei)[4] = _gameTime;
						if (!_gameTime) {
							releaseView(kCharacterAlexei, kCarGreenSleeping, 70);
							releaseView(kCharacterAlexei, kCarGreenSleeping, 71);
							if (onLowPlatform(kCharacterCath)) {
								playChrExcuseMe(kCharacterAlexei, kCharacterCath, 0);
								if (checkCathDir(kCarGreenSleeping, 62))
									bumpCath(kCarGreenSleeping, 72, 255);
							}

							CONS_Alexei_LeavePlat(0, 0, 0, 0);
						}
					}

					if (getCharacterCurrentParams(kCharacterAlexei)[4] >= _gameTime)
						break;
				}

				getCharacterCurrentParams(kCharacterAlexei)[4] = 0x7FFFFFFF;
				releaseView(kCharacterAlexei, kCarGreenSleeping, 70);
				releaseView(kCharacterAlexei, kCarGreenSleeping, 71);
				if (onLowPlatform(kCharacterCath)) {
					playChrExcuseMe(kCharacterAlexei, kCharacterCath, 0);
					if (checkCathDir(kCarGreenSleeping, 62))
						bumpCath(kCarGreenSleeping, 72, 255);
				}

				CONS_Alexei_LeavePlat(0, 0, 0, 0);
			}
		}

		break;
	case 3:
		if (!getCharacterCurrentParams(kCharacterAlexei)[0] && !getCharacterCurrentParams(kCharacterAlexei)[1])
			startCycOtis(kCharacterAlexei, "306F");

		break;
	case 12:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_DoCorrOtis, "602FB", 2, 0, 0);
		break;
	case 17:
		if (checkCathDir(kCarGreenSleeping, 62)) {
			if (getCharacterCurrentParams(kCharacterAlexei)[0]) {
				if (!getCharacterCurrentParams(kCharacterAlexei)[1])
					break;
			} else if (!getCharacterCurrentParams(kCharacterAlexei)[1]) {
				startSeqOtis(kCharacterAlexei, "306A");
				break;
			}

			CONS_Alexei_LeavePlat(0, 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1) {
			setDoor(2, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 3, 540, 0, 0);
		} else if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 2) {
			endGraphics(kCharacterAlexei);
			if (onLowPlatform(kCharacterCath)) {
				playChrExcuseMe(kCharacterAlexei, kCharacterCath, 0);

				if (checkCathDir(kCarGreenSleeping, 62))
					bumpCath(kCarGreenSleeping, 72, 255);
			}

			blockView(kCharacterAlexei, kCarGreenSleeping, 70);
			blockView(kCharacterAlexei, kCarGreenSleeping, 71);
		}
		break;
	case 123536024:
		getCharacterCurrentParams(kCharacterAlexei)[1] = 1;
		break;
	case 123712592:
		endGraphics(kCharacterAlexei);
		getCharacterCurrentParams(kCharacterAlexei)[0] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_LeavePlat(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_LeavePlat);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 40;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_LeavePlat(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 3, 7500, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1) {
			startSeqOtis(kCharacterAlexei, "602Eb");
			blockAtDoor(kCharacterAlexei, 2);
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			if (inComp(kCharacterCath, kCarGreenSleeping, 7500)) {
				if (isNight()) {
					playNIS(kEventCathTurningNight);
				} else {
					playNIS(kEventCathTurningDay);
				}

				playDialog(0, "BUMP", -1, 0);
				bumpCathRDoor(2);
			}
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_FinishSeqOtis, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 2) {
			releaseAtDoor(kCharacterAlexei, 2);
			getCharacter(kCharacterAlexei).characterPosition.position = 7500;
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			endGraphics(kCharacterAlexei);
			CONS_Alexei_InCompAgain(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_InCompAgain(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_InCompAgain);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 41;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_InCompAgain(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (checkCathDir(kCarGreenSleeping, 66))
			bumpCath(kCarGreenSleeping, 49, 255);
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 2403000, "411", 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1) {
			CONS_Alexei_GoSalon4(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_GoSalon4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_GoSalon4);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 42;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_GoSalon4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_ExitComp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			send(kCharacterAlexei, kCharacterTatiana, 191198209, 0);
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_DoBlockSeqOtis, "103A", 5, 52, 0);
			break;
		case 3:
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			CONS_Alexei_Pacing(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_Pacing(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_Pacing);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 43;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_Pacing(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime < 1806300) {
			if ((!getCharacterCurrentParams(kCharacterAlexei)[1] && (getCharacterCurrentParams(kCharacterAlexei)[1] = _gameTime + getCharacterCurrentParams(kCharacterAlexei)[0], _gameTime + getCharacterCurrentParams(kCharacterAlexei)[0] == 0)) || getCharacterCurrentParams(kCharacterAlexei)[1] < _gameTime) {
				if (rcClear()) {
					getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
					AlexeiCall(&LogicManager::CONS_Alexei_PacingAtWindow, 0, 0, 0, 0);
					break;
				}
			}
		}

		if (_gameTime > 2457000 && !getCharacterCurrentParams(kCharacterAlexei)[2]) {
			getCharacterCurrentParams(kCharacterAlexei)[2] = 1;
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_WaitRCClear, 0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterAlexei)[0] = 5 * (3 * rnd(120) + 180);
		startCycOtis(kCharacterAlexei, "103B");
		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] != 1) {
			if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 2) {
				getCharacter(kCharacterAlexei).characterPosition.location = 0;
				getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
				AlexeiCall(&LogicManager::CONS_Alexei_DoBlockSeqOtis, "124C", 5, 52, 0);
			} else if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 3) {
				CONS_Alexei_GoToPlatform(0, 0, 0, 0);
			}

			break;
		}

		getCharacterCurrentParams(kCharacterAlexei)[0] = 5 * (3 * rnd(120) + 180);
		getCharacterCurrentParams(kCharacterAlexei)[1] = 0;
		if (_gameTime > 2457000 && !getCharacterCurrentParams(kCharacterAlexei)[2]) {
			getCharacterCurrentParams(kCharacterAlexei)[2] = 1;
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_WaitRCClear, 0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_GoToPlatform(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_GoToPlatform);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 44;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_GoToPlatform(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2475000 && !getCharacterCurrentParams(kCharacterAlexei)[0]) {
			getCharacterCurrentParams(kCharacterAlexei)[0] = 1;
			releaseView(kCharacterAlexei, kCarGreenSleeping, 70);
			releaseView(kCharacterAlexei, kCarGreenSleeping, 71);

			if (onLowPlatform(kCharacterCath)) {
				playChrExcuseMe(kCharacterAlexei, kCharacterCath, 0);
				if (checkCathDir(kCarGreenSleeping, 62))
					bumpCath(kCarGreenSleeping, 72, 255);
			}

			CONS_Alexei_ReturnComp4(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterAlexei).characterPosition.car = 4;
		getCharacter(kCharacterAlexei).characterPosition.location = 0;
		getCharacter(kCharacterAlexei).characterPosition.position = 9460;
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 3, 540, 0, 0);
		break;
	case 17:
		if (checkCathDir(kCarGreenSleeping, 62)) {
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_DoSeqOtis, "306A", 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1) {
			endGraphics(kCharacterAlexei);

			if (onLowPlatform(kCharacterCath)) {
				playChrExcuseMe(kCharacterAlexei, kCharacterCath, 0);

				if (checkCathDir(kCarGreenSleeping, 62))
					bumpCath(kCarGreenSleeping, 72, 255);
			}

			blockView(kCharacterAlexei, kCarGreenSleeping, 70);
			blockView(kCharacterAlexei, kCarGreenSleeping, 71);
		} else if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 2) {
			startCycOtis(kCharacterAlexei, "306F");
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_ReturnComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_ReturnComp4);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 45;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_ReturnComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_EnterComp, 0, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] == 1) {
			if (checkCathDir(kCarGreenSleeping, 66))
				bumpCath(kCarGreenSleeping, 49, 255);
			if (cathHasItem(kItemBomb)) {
				CONS_Alexei_BombPlanB(0, 0, 0, 0);
			} else {
				getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
				AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 15803100, "412", 0, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_BombPlanB(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_BombPlanB);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 46;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_BombPlanB(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAlexei)[0] != 0x7FFFFFFF && _gameTime != 0) {
			if (_gameTime <= 2493000) {
				if (inSalon(kCharacterCath) || inSalon(kCharacterAugust) || !getCharacterCurrentParams(kCharacterAlexei)[0]) {
					getCharacterCurrentParams(kCharacterAlexei)[0] = _gameTime;
					if (!_gameTime) {
						takeItem(kItem22);
						if (inSalon(kCharacterCath)) {
							playChrExcuseMe(kCharacterAlexei, kCharacterCath, 0);
							bumpCath(kCarRestaurant, checkCathDir(), 255);
						}

						getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 4;
						AlexeiCall(&LogicManager::CONS_Alexei_EnterComp, 0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterAlexei)[0] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterAlexei)[0] = 0x7FFFFFFF;

			takeItem(kItem22);
			if (inSalon(kCharacterCath)) {
				playChrExcuseMe(kCharacterAlexei, kCharacterCath, 0);
				bumpCath(kCarRestaurant, checkCathDir(), 255);
			}

			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 4;
			AlexeiCall(&LogicManager::CONS_Alexei_EnterComp, 0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 1;
		AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 2488500, "411", 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 2;
			AlexeiCall(&LogicManager::CONS_Alexei_ExitComp, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 3;
			AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 4, 9460, 0, 0);
			break;
		case 4:
			if (checkCathDir(kCarGreenSleeping, 66))
				bumpCath(kCarGreenSleeping, 49, 255);

			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 5;
			AlexeiCall(&LogicManager::CONS_Alexei_CompLogic, 2507400, "412", 0, 0);
			break;
		case 5:
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 6;
			AlexeiCall(&LogicManager::CONS_Alexei_DoCorrOtis, "602Fb", 2, 0, 0);
			break;
		case 6:
			setDoor(2, 0, 0, 10, 9);
			getCharacter(kCharacterAlexei).characterPosition.location = 0;
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 7;
			AlexeiCall(&LogicManager::CONS_Alexei_DoWalk, 4, 7500, 0, 0);
			break;
		case 7:
			startSeqOtis(kCharacterAlexei, "602Eb");
			blockAtDoor(kCharacterAlexei, 33);
			getCharacter(kCharacterAlexei).characterPosition.location = 1;

			if (inSuite(kCharacterCath, kCarRedSleeping, 7850)) {
				playNIS((158 - !isNight()));
				playDialog(0, "BUMP", -1, 0);
				bumpCathRDoor(33);
			}

			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 8;
			AlexeiCall(&LogicManager::CONS_Alexei_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 8:
			releaseAtDoor(kCharacterAlexei, 33);
			endGraphics(kCharacterAlexei);
			getCharacter(kCharacterAlexei).characterPosition.position = 8200;
			getCharacter(kCharacterAlexei).characterPosition.location = 1;
			setDoor(32, kCharacterCath, 1, 0, 0);
			setDoor(48, kCharacterCath, 1, 0, 0);
			getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall + 8] = 9;
			AlexeiCall(&LogicManager::CONS_Alexei_DoDialog, "TAT4167", 0, 0, 0);
			break;
		case 9:
			send(kCharacterAlexei, kCharacterMaster, 156435676, 0);
			CONS_Alexei_Dead(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_Dead(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_Dead);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 47;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_Dead(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterAlexei);
		getCharacter(kCharacterAlexei).characterPosition.car = kCarLocomotive;
		getCharacter(kCharacterAlexei).characterPosition.position = 0;
		getCharacter(kCharacterAlexei).characterPosition.location = 0;
		setDoor(2, kCharacterCath, 0, 10, 9);
		setDoor(18, kCharacterCath, 0, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alexei_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlexei).callParams[getCharacter(kCharacterAlexei).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlexei, &LogicManager::HAND_Alexei_StartPart5);
	getCharacter(kCharacterAlexei).callbacks[getCharacter(kCharacterAlexei).currentCall] = 48;

	params->clear();

	fedEx(kCharacterAlexei, kCharacterAlexei, 12, 0);
}

void LogicManager::HAND_Alexei_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterAlexei);
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsAlexei[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Alexei_DebugWalks,
	&LogicManager::HAND_Alexei_DoDialog,
	&LogicManager::HAND_Alexei_DoWait,
	&LogicManager::HAND_Alexei_DoSeqOtis,
	&LogicManager::HAND_Alexei_DoBlockSeqOtis,
	&LogicManager::HAND_Alexei_DoCorrOtis,
	&LogicManager::HAND_Alexei_FinishSeqOtis,
	&LogicManager::HAND_Alexei_DoJoinedSeqOtis,
	&LogicManager::HAND_Alexei_SaveGame,
	&LogicManager::HAND_Alexei_DoWalk,
	&LogicManager::HAND_Alexei_DoComplexSeqOtis,
	&LogicManager::HAND_Alexei_WaitRCClear,
	&LogicManager::HAND_Alexei_EnterComp,
	&LogicManager::HAND_Alexei_ExitComp,
	&LogicManager::HAND_Alexei_PacingAtWindow,
	&LogicManager::HAND_Alexei_CompLogic,
	&LogicManager::HAND_Alexei_Birth,
	&LogicManager::HAND_Alexei_AtDinner,
	&LogicManager::HAND_Alexei_ReturnComp,
	&LogicManager::HAND_Alexei_GoSalon,
	&LogicManager::HAND_Alexei_Sitting,
	&LogicManager::HAND_Alexei_StandingAtWindow,
	&LogicManager::HAND_Alexei_WaitingForTat,
	&LogicManager::HAND_Alexei_Upset,
	&LogicManager::HAND_Alexei_ReturnCompNight,
	&LogicManager::HAND_Alexei_Asleep,
	&LogicManager::HAND_Alexei_WakeUp,
	&LogicManager::HAND_Alexei_StartPart2,
	&LogicManager::HAND_Alexei_InComp2,
	&LogicManager::HAND_Alexei_AtBreakfast,
	&LogicManager::HAND_Alexei_ReturnCompPart2,
	&LogicManager::HAND_Alexei_StartPart3,
	&LogicManager::HAND_Alexei_PlayingChess,
	&LogicManager::HAND_Alexei_InPart3,
	&LogicManager::HAND_Alexei_Pacing3,
	&LogicManager::HAND_Alexei_GoSalon3,
	&LogicManager::HAND_Alexei_StartPart4,
	&LogicManager::HAND_Alexei_InComp4,
	&LogicManager::HAND_Alexei_MeetTat,
	&LogicManager::HAND_Alexei_LeavePlat,
	&LogicManager::HAND_Alexei_InCompAgain,
	&LogicManager::HAND_Alexei_GoSalon4,
	&LogicManager::HAND_Alexei_Pacing,
	&LogicManager::HAND_Alexei_GoToPlatform,
	&LogicManager::HAND_Alexei_ReturnComp4,
	&LogicManager::HAND_Alexei_BombPlanB,
	&LogicManager::HAND_Alexei_Dead,
	&LogicManager::HAND_Alexei_StartPart5
};


} // End of namespace LastExpress
