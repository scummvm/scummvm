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

void LogicManager::CONS_Tatiana(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterTatiana,
			_functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]
		);

		break;
	case 1:
		CONS_Tatiana_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Tatiana_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Tatiana_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Tatiana_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Tatiana_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::TatianaCall(CALL_PARAMS) {
	getCharacter(kCharacterTatiana).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Tatiana_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_DebugWalks);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 1;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterTatiana, kCarGreenSleeping, getCharacterCurrentParams(kCharacterTatiana)[0])) {
			if (getCharacterCurrentParams(kCharacterTatiana)[0] == 10000) {
				getCharacterCurrentParams(kCharacterTatiana)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterTatiana)[0] = 10000;
			}
		}

		break;
	case 1:
		getCharacter(kCharacterTatiana).clothes++;
		if (getCharacter(kCharacterTatiana).clothes > 3)
			getCharacter(kCharacterTatiana).clothes = 0;

		break;
	case 12:
		getCharacter(kCharacterTatiana).characterPosition.position = 0;
		getCharacter(kCharacterTatiana).characterPosition.location = 0;
		getCharacter(kCharacterTatiana).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterTatiana).inventoryItem = 0x80;
		getCharacterCurrentParams(kCharacterTatiana)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_DoDialog);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	case 12:
		playDialog(kCharacterTatiana, (char *)&getCharacterCurrentParams(kCharacterTatiana)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_DoSeqOtis);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterTatiana, (char *)&getCharacterCurrentParams(kCharacterTatiana)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_DoBlockSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_DoBlockSeqOtis);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_DoBlockSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseView(kCharacterTatiana, getCharacterCurrentParams(kCharacterTatiana)[3], getCharacterCurrentParams(kCharacterTatiana)[4]);

		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterTatiana, (char *)&getCharacterCurrentParams(kCharacterTatiana)[0]);
		blockView(kCharacterTatiana, getCharacterCurrentParams(kCharacterTatiana)[3], getCharacterCurrentParams(kCharacterTatiana)[4]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_DoCorrOtis);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterTatiana, getCharacterCurrentParams(kCharacterTatiana)[3]);

		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterTatiana, (char *)&getCharacterCurrentParams(kCharacterTatiana)[0]);
		blockAtDoor(kCharacterTatiana, getCharacterCurrentParams(kCharacterTatiana)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_DoEnterCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_DoEnterCorrOtis);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_DoEnterCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterTatiana, getCharacterCurrentParams(kCharacterTatiana)[3]);
		getCharacter(kCharacterTatiana).characterPosition.position = 7500;

		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterTatiana, (char *)&getCharacterCurrentParams(kCharacterTatiana)[0]);
		blockAtDoor(kCharacterTatiana, getCharacterCurrentParams(kCharacterTatiana)[3]);
		getCharacter(kCharacterTatiana).characterPosition.location = 1;

		if (inComp(kCharacterCath, kCarRedSleeping, 7500) || inComp(kCharacterCath, kCarRedSleeping, 7850)) {
			playNIS((kEventCathTurningNight - !isNight()));
			playDialog(0, "BUMP", -1, 0);
			bumpCathRDoor(33);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_DoJoinedSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_DoJoinedSeqOtis);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 7;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	strncpy((char *)&params->parameters[5], param4.stringParam, 12);

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_DoJoinedSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		if (!getCharacterCurrentParams(kCharacterTatiana)[8])
			fedEx(kCharacterTatiana, getCharacterCurrentParams(kCharacterTatiana)[3], getCharacterCurrentParams(kCharacterTatiana)[4], (char *)&getCharacterCurrentParams(kCharacterTatiana)[5]);

		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	case 10:
		if (!getCharacterCurrentParams(kCharacterTatiana)[8]) {
			fedEx(kCharacterTatiana, getCharacterCurrentParams(kCharacterTatiana)[3], getCharacterCurrentParams(kCharacterTatiana)[4], (char *)&getCharacterCurrentParams(kCharacterTatiana)[5]);
			getCharacterCurrentParams(kCharacterTatiana)[8] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterTatiana, (char *)&getCharacterCurrentParams(kCharacterTatiana)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_FinishSeqOtis);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 8;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterTatiana).direction != 4) {
			getCharacter(kCharacterTatiana).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
			fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_DoWaitRealTime(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_DoWaitRealTime);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 9;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_DoWaitRealTime(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterTatiana)[1] || (getCharacterCurrentParams(kCharacterTatiana)[1] = _realTime + getCharacterCurrentParams(kCharacterTatiana)[0],
																_realTime + getCharacterCurrentParams(kCharacterTatiana)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterTatiana)[1] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterTatiana)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_DoWait);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterTatiana)[1] || (getCharacterCurrentParams(kCharacterTatiana)[1] = _gameTime + getCharacterCurrentParams(kCharacterTatiana)[0],
																_gameTime + getCharacterCurrentParams(kCharacterTatiana)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterTatiana)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterTatiana)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_WaitRCClear);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 11;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterTatiana).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
			fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_SaveGame);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 12;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	case 12:
		save(
			kCharacterTatiana,
			getCharacterCurrentParams(kCharacterTatiana)[0],
			getCharacterCurrentParams(kCharacterTatiana)[1]
		);

		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_DoWalk);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 13;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterTatiana, getCharacterCurrentParams(kCharacterTatiana)[0], getCharacterCurrentParams(kCharacterTatiana)[1])) {
			getCharacter(kCharacterTatiana).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
			fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		}

		break;
	case 5:
		if (_doneNIS[kEventTatianaAskMatchSpeakRussian] || _doneNIS[kEventTatianaAskMatch] || _doneNIS[kEventVassiliSeizure]) {
			if (rnd(2) == 0) {
				playDialog(kCharacterCath, "CAT1010A", -1, 0);
			} else {
				playDialog(kCharacterCath, "CAT1010", -1, 0);
			}
		} else {
			playCathExcuseMe();
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterTatiana, kCharacterCath, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_EnterComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_EnterComp);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 14;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_EnterComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		send(kCharacterTatiana, kCharacterCond2, 326348944, 0);

		if (_globals[kGlobalChapter] != 1) {
			startCycOtis(kCharacterTatiana, "673Fb");
		} else {
			startCycOtis(kCharacterTatiana, "603Fb");
		}
		
		softBlockAtDoor(kCharacterTatiana, 33);
		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] &&
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] <= 2) {
			softReleaseAtDoor(kCharacterTatiana, 33);
			getCharacter(kCharacterTatiana).characterPosition.location = 1;
			endGraphics(kCharacterTatiana);

			getCharacter(kCharacterTatiana).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
			fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		}

		break;
	case 69239528:
		if (_globals[kGlobalChapter] == 1) {
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
			TatianaCall(&LogicManager::CONS_Tatiana_DoEnterCorrOtis, "603Db", 33, 0, 0);
		} else {
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_DoEnterCorrOtis, "673Db", 33, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_ExitComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_ExitComp);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 15;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_ExitComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (_globals[kGlobalChapter] == 1) {
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
			TatianaCall(&LogicManager::CONS_Tatiana_DoCorrOtis, "603Bb", 33, 0, 0);
		} else {
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_DoCorrOtis, "673Bb", 33, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] &&
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] <= 2) {
			getCharacter(kCharacterTatiana).characterPosition.location = 0;
			send(kCharacterTatiana, kCharacterCond2, 292048641, 0);

			if (_globals[kGlobalChapter] != 1) {
				startCycOtis(kCharacterTatiana, "673Fb");
			} else {
				startCycOtis(kCharacterTatiana, "603Fb");
			}
			
			softBlockAtDoor(kCharacterTatiana, 33);
		}

		break;
	case 69239528:
		softReleaseAtDoor(kCharacterTatiana, 33);
		setDoor(33, kCharacterCath, 1, 10, 9);

		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_CompLogic(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_CompLogic);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 16;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_CompLogic(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterTatiana)[0] < _gameTime && !getCharacterCurrentParams(kCharacterTatiana)[3]) {
			getCharacterCurrentParams(kCharacterTatiana)[3] = 1;
			setDoor(33, kCharacterCath, 0, 10, 9);
			setDoor(49, kCharacterCath, 0, 10, 9);

			getCharacter(kCharacterTatiana).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
			fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterTatiana)[1]) {
			if (getCharacterCurrentParams(kCharacterTatiana)[4] || (getCharacterCurrentParams(kCharacterTatiana)[4] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterTatiana)[4] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterTatiana)[4] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterTatiana)[1] = 0;
			getCharacterCurrentParams(kCharacterTatiana)[2] = 1;
			setDoor(33, kCharacterTatiana, 1, 0, 0);
			setDoor(49, kCharacterTatiana, 1, 0, 0);
			getCharacterCurrentParams(kCharacterTatiana)[4] = 0;
		} else {
			getCharacterCurrentParams(kCharacterTatiana)[4] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterTatiana)[1]) {
			setDoor(33, kCharacterTatiana, 1, 0, 0);
			setDoor(49, kCharacterTatiana, 1, 0, 0);

			if (msg->param.intParam == 49) {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 4;
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, getCathWCDialog(), 0, 0, 0);
			} else if (cathHasItem(kItemPassengerList)) {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 5;

				if (rnd(2) != 0) {
					TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, getCathSorryDialog(), 0, 0, 0);
				} else {
					TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "CAT1512", 0, 0, 0);
				}
			} else {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 6;
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else {
			setDoor(33, kCharacterTatiana, 1, 0, 0);
			setDoor(49, kCharacterTatiana, 1, 0, 0);
			if (msg->action == 8) {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		setDoor(33, kCharacterTatiana, 1, 10, 9);
		setDoor(49, kCharacterTatiana, 1, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterTatiana)[2] || getCharacterCurrentParams(kCharacterTatiana)[1]) {
			setDoor(33, kCharacterTatiana, 1, 10, 9);
			setDoor(49, kCharacterTatiana, 1, 10, 9);
			getCharacterCurrentParams(kCharacterTatiana)[2] = 0;
			getCharacterCurrentParams(kCharacterTatiana)[1] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8]) {
		case 1:
		case 2:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
			if (rnd(2) == 0) {
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "TAT1133B", 0, 0, 0);
			} else {
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "TAT1133A", 0, 0, 0);
			}

			break;
		case 3:
			setDoor(33, kCharacterTatiana, 1, 14, 0);
			setDoor(49, kCharacterTatiana, 1, 14, 0);
			getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
			break;
		case 4:
		case 5:
		case 6:
			getCharacterCurrentParams(kCharacterTatiana)[1] = 0;
			getCharacterCurrentParams(kCharacterTatiana)[2] = 1;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_Birth);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 17;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterTatiana)[0]) {
			getCharacterCurrentParams(kCharacterTatiana)[0] = 1;
			CONS_Tatiana_AtDinner(0, 0, 0, 0);
		}

		break;
	case 12:
		autoMessage(kCharacterTatiana, 191198209, 0);
		getCharacter(kCharacterTatiana).characterPosition.position = 5419;
		getCharacter(kCharacterTatiana).characterPosition.location = 1;
		getCharacter(kCharacterTatiana).characterPosition.car = kCarRestaurant;
		setDoor(33, kCharacterCath, 1, 10, 9);
		setDoor(49, kCharacterCath, 1, 10, 9);
		setDoor(41, kCharacterCath, 0, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_DoSpecialSalonWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_DoSpecialSalonWalk);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 18;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_DoSpecialSalonWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterTatiana)[0]) {
			if (_gameTime > 1143000 && !getCharacterCurrentParams(kCharacterTatiana)[1]) {
				getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
				startSeqOtis(kCharacterTatiana, "806DS");
				getCharacterCurrentParams(kCharacterTatiana)[0] = 1;
			}

			if (!getCharacterCurrentParams(kCharacterTatiana)[0]) {
				bool skip = false;

				if (!getCharacterCurrentParams(kCharacterTatiana)[2]) {
					getCharacterCurrentParams(kCharacterTatiana)[2] = _gameTime + 4500;
					if (_gameTime == -4500) {
						skip = true;
						startSeqOtis(kCharacterTatiana, "806DS");
						getCharacterCurrentParams(kCharacterTatiana)[0] = 1;
					}
				}

				if (!skip && getCharacterCurrentParams(kCharacterTatiana)[2] < _gameTime) {
					getCharacterCurrentParams(kCharacterTatiana)[2] = 0x7FFFFFFF;
					startSeqOtis(kCharacterTatiana, "806DS");
					getCharacterCurrentParams(kCharacterTatiana)[0] = 1;
				}
			}
		}

		if (getCharacter(kCharacterTatiana).characterPosition.position <= 2330) {
			send(kCharacterTatiana, kCharacterAlexei, 157159392, 0);
			endGraphics(kCharacterTatiana);

			getCharacter(kCharacterTatiana).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
			fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		}

		break;
	case 3:
		send(kCharacterTatiana, kCharacterAlexei, 188784532, 0);

		getCharacter(kCharacterTatiana).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
		fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		break;
	case 12:
		if (inSalon(kCharacterCath)) {
			startSeqOtis(kCharacterTatiana, "806DS");
			getCharacterCurrentParams(kCharacterTatiana)[0] = 1;
		} else {
			endGraphics(kCharacterTatiana);
		}

		break;
	case 17:
		if (!getCharacterCurrentParams(kCharacterTatiana)[0] && inSalon(kCharacterCath)) {
			startSeqOtis(kCharacterTatiana, "806DS");
			advanceFrame(kCharacterTatiana);
			getCharacterCurrentParams(kCharacterTatiana)[0] = 1;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_AtDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_AtDinner);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 19;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_AtDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		if (!whoRunningDialog(kCharacterTatiana) && getCharacterCurrentParams(kCharacterTatiana)[3] && getCharacterCurrentParams(kCharacterTatiana)[2] < 2 && !dialogRunning("TAT1066")) {
			bool skip1 = false;

			if (getCharacterCurrentParams(kCharacterTatiana)[4] || (getCharacterCurrentParams(kCharacterTatiana)[4] = _realTime + 450, _realTime != -450)) {
				if (getCharacterCurrentParams(kCharacterTatiana)[4] >= _realTime) {
					skip1 = true;
				}

				if (!skip1) {
					getCharacterCurrentParams(kCharacterTatiana)[4] = 0x7FFFFFFF;
				}
			}

			if (!skip1) {
				if (!getCharacterCurrentParams(kCharacterTatiana)[2]) {
					playDialog(kCharacterTatiana, "TAT1069A", -1, 0);
				} else {
					playDialog(kCharacterTatiana, "TAT1069B", -1, 0);
				}

				_globals[kGlobalMetTatianaAndVassili] = 1;
				getCharacterCurrentParams(kCharacterTatiana)[2]++;
				getCharacterCurrentParams(kCharacterTatiana)[4] = 0;
			}

			bool skip2 = false;

			if (checkCathDir(kCarRestaurant, 71)) {
				if (getCharacterCurrentParams(kCharacterTatiana)[5] || (getCharacterCurrentParams(kCharacterTatiana)[5] = _realTime + 75, _realTime != -75)) {
					if (getCharacterCurrentParams(kCharacterTatiana)[5] >= _realTime) {
						skip2 = true;
					}

					if (!skip2) {
						getCharacterCurrentParams(kCharacterTatiana)[5] = 0x7FFFFFFF;
					}
				}

				if (!skip2) {
					if (!getCharacterCurrentParams(kCharacterTatiana)[2]) {
						playDialog(kCharacterTatiana, "TAT1069A", -1, 0);
					} else {
						playDialog(kCharacterTatiana, "TAT1069B", -1, 0);
					}

					_globals[kGlobalMetTatianaAndVassili] = 1;
					getCharacterCurrentParams(kCharacterTatiana)[2]++;
					getCharacterCurrentParams(kCharacterTatiana)[5] = 0;
				}
			}
		}

		if (_gameTime > 1084500 && !getCharacterCurrentParams(kCharacterTatiana)[6]) {
			getCharacterCurrentParams(kCharacterTatiana)[6] = 1;
			send(kCharacterTatiana, kCharacterHeadWait, 257489762, 0);
		}

		if (getCharacterCurrentParams(kCharacterTatiana)[0]) {
			if (getCharacterCurrentParams(kCharacterTatiana)[7] || (getCharacterCurrentParams(kCharacterTatiana)[7] = _realTime + 90, _realTime != -90)) {
				if (getCharacterCurrentParams(kCharacterTatiana)[7] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterTatiana)[7] = 0x7FFFFFFF;
			}

			bumpCath(kCarRestaurant, 65, 255);
		} else {
			getCharacterCurrentParams(kCharacterTatiana)[7] = 0;
		}

		break;
	}
	case 12:
		send(kCharacterTatiana, kCharacterTableE, 136455232, 0);
		startCycOtis(kCharacterTatiana, "014A");
		break;
	case 17:
		getCharacterCurrentParams(kCharacterTatiana)[0] = 1;

		if (!checkCathDir(kCarRestaurant, 67))
			getCharacterCurrentParams(kCharacterTatiana)[0] = 0;

		getCharacterCurrentParams(kCharacterTatiana)[3] = (checkCathDir(kCarRestaurant, 69) || checkCathDir(kCarRestaurant, 70) || checkCathDir(kCarRestaurant, 71)) ? 1 : 0;
		break;
	case 122358304:
		startCycOtis(kCharacterTatiana, "BLANK");
		break;
	case 124973510:
		CONS_Tatiana_ExitDining(0, 0, 0, 0);
		break;
	case 122288808:
		startCycOtis(kCharacterTatiana, "014A");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_ExitDining(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_ExitDining);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 20;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_ExitDining(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTatiana).characterPosition.location = 0;
			send(kCharacterTatiana, kCharacterAugust, 223183000, 0);
			blockView(kCharacterTatiana, kCarRestaurant, 67);
			playDialog(kCharacterTatiana, "TAT1070", -1, 0);
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_DoJoinedSeqOtis, "014C", 37, 103798704, "014D");
			break;
		case 2:
			releaseView(kCharacterTatiana, kCarRestaurant, 67);
			send(kCharacterTatiana, kCharacterWaiter1, 188893625, 0);
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
			TatianaCall(&LogicManager::CONS_Tatiana_DoSpecialSalonWalk, 0, 0, 0, 0);
			break;
		case 3:
			send(kCharacterTatiana, kCharacterAugust, 268620864, 0);
			CONS_Tatiana_ReturnToComp(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_ReturnToComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_ReturnToComp);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 21;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_ReturnToComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).clothes = 1;
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 4, 8513, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTatiana).clothes = 0;
			playDialog(kCharacterTatiana, "TAT1071", -1, 0);
			startSeqOtis(kCharacterTatiana, "604Aa");
			blockAtDoor(kCharacterTatiana, 32);
			getCharacter(kCharacterTatiana).characterPosition.location = 1;

			if (inSuite(kCharacterCath, kCarRedSleeping, 7850)) {
				playNIS((kEventCathTurningNight - !isNight()));
				playDialog(0, "BUMP", -1, 0);
				bumpCathFDoor(32);
			}

			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 2:
			releaseAtDoor(kCharacterTatiana, 32);
			getCharacter(kCharacterTatiana).characterPosition.location = 1;
			endGraphics(kCharacterTatiana);
			send(kCharacterTatiana, kCharacterAlexei, 135854208, 0);
			setDoor(32, kCharacterCath, 1, 0, 0);
			setDoor(33, kCharacterCath, 1, 0, 0);

			// fall through
		case 3:
			if (whoRunningDialog(kCharacterTatiana)) {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
				TatianaCall(&LogicManager::CONS_Tatiana_DoWait, 75, 0, 0, 0);
			} else {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 4;
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "TAT1071A", 0, 0, 0);
			}

			break;
		case 4:
			getCharacter(kCharacterTatiana).characterPosition.position = 7500;
			send(kCharacterTatiana, kCharacterVassili, 168459827, 0);
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 5;
			TatianaCall(&LogicManager::CONS_Tatiana_CompLogic, 1156500, 0, 0, 0);
			break;
		case 5:
		case 6:
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterPolice) {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 6;
				TatianaCall(&LogicManager::CONS_Tatiana_CompLogic, _gameTime + 900, 0, 0, 0);
			} else {
				setDoor(49, kCharacterCath, 1, 10, 9);
				CONS_Tatiana_GetSomeAir(0, 0, 0, 0);
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

void LogicManager::CONS_Tatiana_GetSomeAir(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_GetSomeAir);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 22;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_GetSomeAir(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterTatiana)[0] == 0x7FFFFFFF || _gameTime <= 1179000) {
			if (!_doneNIS[kEventTatianaAskMatchSpeakRussian] && !_doneNIS[kEventTatianaAskMatch] && cathHasItem(kItemMatchBox) && onLowPlatform(kCharacterCath)) {
				setDoor(25, kCharacterTatiana, 1, 0, 1);
				setDoor(23, kCharacterTatiana, 1, 0, 1);
			}

			break;
		}

		if (_gameTime > 1233000) {
			getCharacterCurrentParams(kCharacterTatiana)[0] = 0x7FFFFFFF;
			setDoor(25, kCharacterCath, 0, 255, 255);
			setDoor(23, kCharacterCath, 0, 255, 255);
			releaseView(kCharacterTatiana, kCarGreenSleeping, 70);
			releaseView(kCharacterTatiana, kCarGreenSleeping, 71);

			if (onLowPlatform(kCharacterCath)) {
				playChrExcuseMe(kCharacterTatiana, kCharacterCath, 0);
				if (checkCathDir(kCarGreenSleeping, 62))
					bumpCath(kCarGreenSleeping, 72, 255);
			}

			getCharacter(kCharacterTatiana).inventoryItem = 0;
			CONS_Tatiana_ReturnToCompAgain(0, 0, 0, 0);
			break;
		}

		if ((!_doneNIS[kEventTatianaAskMatchSpeakRussian] && !_doneNIS[kEventTatianaAskMatch]) || onLowPlatform(kCharacterCath) || !getCharacterCurrentParams(kCharacterTatiana)[0]) {
			getCharacterCurrentParams(kCharacterTatiana)[0] = _gameTime;
			if (!_gameTime) {
				setDoor(25, kCharacterCath, 0, 255, 255);
				setDoor(23, kCharacterCath, 0, 255, 255);
				releaseView(kCharacterTatiana, kCarGreenSleeping, 70);
				releaseView(kCharacterTatiana, kCarGreenSleeping, 71);

				if (onLowPlatform(kCharacterCath)) {
					playChrExcuseMe(kCharacterTatiana, kCharacterCath, 0);
					if (checkCathDir(kCarGreenSleeping, 62))
						bumpCath(kCarGreenSleeping, 72, 255);
				}

				getCharacter(kCharacterTatiana).inventoryItem = 0;
				CONS_Tatiana_ReturnToCompAgain(0, 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterTatiana)[0] >= _gameTime) {
			if (!_doneNIS[kEventTatianaAskMatchSpeakRussian] && !_doneNIS[kEventTatianaAskMatch] && cathHasItem(kItemMatchBox) && onLowPlatform(kCharacterCath)) {
				setDoor(25, kCharacterTatiana, 1, 0, 1);
				setDoor(23, kCharacterTatiana, 1, 0, 1);
			}
		} else {
			getCharacterCurrentParams(kCharacterTatiana)[0] = 0x7FFFFFFF;
			setDoor(25, kCharacterCath, 0, 255, 255);
			setDoor(23, kCharacterCath, 0, 255, 255);
			releaseView(kCharacterTatiana, kCarGreenSleeping, 70);
			releaseView(kCharacterTatiana, kCarGreenSleeping, 71);

			if (onLowPlatform(kCharacterCath)) {
				playChrExcuseMe(kCharacterTatiana, kCharacterCath, 0);
				if (checkCathDir(kCarGreenSleeping, 62))
					bumpCath(kCarGreenSleeping, 72, 255);
			}

			getCharacter(kCharacterTatiana).inventoryItem = 0;
			CONS_Tatiana_ReturnToCompAgain(0, 0, 0, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterTatiana).inventoryItem = 0;
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 4;
		TatianaCall(&LogicManager::CONS_Tatiana_SaveGame, 2, kEventTatianaGivePoem, 0, 0);
		break;
	case 9:
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
		TatianaCall(&LogicManager::CONS_Tatiana_SaveGame, 2, kEventTatianaAskMatchSpeakRussian, 0, 0);
		break;
	case 12:
		send(kCharacterTatiana, kCharacterVassili, 122732000, 0);
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_ExitComp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 3, 540, 0, 0);
			break;
		case 2:
			if (onLowPlatform(kCharacterCath)) {
				playChrExcuseMe(kCharacterTatiana, kCharacterCath, 0);
				if (checkCathDir(kCarGreenSleeping, 62))
					bumpCath(kCarGreenSleeping, 72, 255);
			}

			startCycOtis(kCharacterTatiana, "306B");
			blockView(kCharacterTatiana, kCarGreenSleeping, 70);
			blockView(kCharacterTatiana, kCarGreenSleeping, 71);
			break;
		case 3:
			playNIS(((_doneNIS[kEventAlexeiSalonVassili] == 0) + kEventTatianaAskMatchSpeakRussian));
			bumpCath(kCarGreenSleeping, 62, 255);
			getCharacter(kCharacterTatiana).inventoryItem = 10;
			setDoor(25, kCharacterCath, 0, 255, 255);
			setDoor(23, kCharacterCath, 0, 255, 255);
			break;
		case 4:
			playNIS(kEventTatianaGivePoem);
			takeCathItem(kItemParchemin);
			cleanNIS();
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_ReturnToCompAgain(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_ReturnToCompAgain);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 23;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_ReturnToCompAgain(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 4, 7500, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 1) {
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_EnterComp, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 2) {
			CONS_Tatiana_Asleep(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_Asleep);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 24;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterTatiana).characterPosition.position = 7500;
		getCharacter(kCharacterTatiana).characterPosition.location = 1;
		setDoor(25, kCharacterCath, 0, 255, 255);
		setDoor(23, kCharacterCath, 0, 255, 255);
		releaseView(kCharacterTatiana, kCarGreenSleeping, 70);
		releaseView(kCharacterTatiana, kCarGreenSleeping, 71);
		endGraphics(kCharacterTatiana);
		setDoor(33, kCharacterCath, 1, 10, 9);
		setDoor(49, kCharacterCath, 1, 10, 9);
		setDoor(41, kCharacterCath, 0, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_StartPart2);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 25;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Tatiana_EatingBreakfast(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterTatiana);
		getCharacter(kCharacterTatiana).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterTatiana).characterPosition.position = 5420;
		getCharacter(kCharacterTatiana).characterPosition.location = 1;
		getCharacter(kCharacterTatiana).clothes = 2;
		setDoor(33, kCharacterCath, 1, 10, 9);
		setDoor(49, kCharacterCath, 1, 10, 9);
		setDoor(41, kCharacterCath, 0, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_EatingBreakfast(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_EatingBreakfast);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 26;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_EatingBreakfast(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1800000 && getCharacterCurrentParams(kCharacterTatiana)[0] && rcClear()) {
			getCharacter(kCharacterTatiana).inventoryItem = 0;
			CONS_Tatiana_JoinAlexei(0, 0, 0, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterTatiana).inventoryItem = 0;
		CONS_Tatiana_BreakfastClick(0, 0, 0, 0);
		break;
	case 12:
		startCycOtis(kCharacterTatiana, "024A");
		send(kCharacterTatiana, kCharacterTableF, 136455232, 0);
		getCharacter(kCharacterTatiana).inventoryItem = 0x80;
		break;
	case 17:
		if (checkCathDir(kCarRestaurant, 64) || checkCathDir(kCarRestaurant, 65)) {
			getCharacter(kCharacterTatiana).inventoryItem = 0;
			CONS_Tatiana_BreakfastClick(0, 0, 0, 0);
		}

		break;
	case 290869168:
		getCharacterCurrentParams(kCharacterTatiana)[0] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_BreakfastClick(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_BreakfastClick);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 27;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_BreakfastClick(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (_doneNIS[kEventTatianaGivePoem]) {
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
			TatianaCall(&LogicManager::CONS_Tatiana_SaveGame, 2, kEventTatianaBreakfastAlexei, 0, 0);
		} else {
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_SaveGame, 2, kEventTatianaBreakfast, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 1) {
			forceJump(kCharacterAlexei, &LogicManager::CONS_Alexei_AtBreakfast);
			playNIS(kEventTatianaBreakfastAlexei);
			giveCathItem(kItemParchemin);
			dropItem(kItem11, 1);
			CONS_Tatiana_JoinAlexei(0, 0, 0, 0);
		} else if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 2) {
			forceJump(kCharacterAlexei, &LogicManager::CONS_Alexei_AtBreakfast);
			playNIS(kEventTatianaBreakfast);

			if (cathHasItem(kItemParchemin)) {
				playNIS(kEventTatianaBreakfastGivePoem);
				takeCathItem(kItemParchemin);
			} else {
				playNIS(kEventTatianaAlexei);
			}

			CONS_Tatiana_JoinAlexei(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_JoinAlexei(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_JoinAlexei);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 28;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_JoinAlexei(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).inventoryItem = 0;
		getCharacter(kCharacterTatiana).characterPosition.location = 0;
		send(kCharacterTatiana, kCharacterTableF, 103798704, "024D");
		if (_doneNIS[kEventTatianaBreakfastAlexei] || _doneNIS[kEventTatianaBreakfast]) {
			send(kCharacterTatiana, kCharacterAlexei, 236053296, 69);
		} else {
			send(kCharacterTatiana, kCharacterAlexei, 236053296, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 1)
			CONS_Tatiana_LeaveBreakfast(0, 0, 0, 0);

		break;
	case 123857088:
		startCycOtis(kCharacterTatiana, "018G");
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_DoWait, 1800, 0, 0, 0);
		break;
	case 156444784:
		getCharacter(kCharacterTatiana).characterPosition.location = 1;
		startCycOtis(kCharacterTatiana, "BLANK");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_LeaveBreakfast(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_LeaveBreakfast);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 29;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_LeaveBreakfast(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTatiana).characterPosition.location = 0;
			blockView(kCharacterTatiana, kCarRestaurant, 63);
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_DoJoinedSeqOtis, "018H", 34, 103798704, "018A");
			break;
		case 2:
			releaseView(kCharacterTatiana, kCarRestaurant, 63);
			send(kCharacterTatiana, kCharacterWaiter2, 302203328, 0);
			startSeqOtis(kCharacterTatiana, "805DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterTatiana);

			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
			TatianaCall(&LogicManager::CONS_Tatiana_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 3:
			CONS_Tatiana_ReturnComp(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_ReturnComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_ReturnComp);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 30;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_ReturnComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 4, 7500, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 1) {
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_EnterComp, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 2) {
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
			TatianaCall(&LogicManager::CONS_Tatiana_CompLogic, 15803100, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_StartPart3);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 31;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Tatiana_PlayChess(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterTatiana);
		getCharacter(kCharacterTatiana).characterPosition.position = 1750;
		getCharacter(kCharacterTatiana).characterPosition.location = 1;
		getCharacter(kCharacterTatiana).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterTatiana).inventoryItem = 0;
		getCharacter(kCharacterTatiana).clothes = 2;
		setDoor(33, kCharacterCath, 1, 10, 9);
		setDoor(49, kCharacterCath, 1, 10, 9);
		_items[kItemFirebird].floating = 2;

		if (_doneNIS[kEventTatianaBreakfastGivePoem] || (_doneNIS[kEventTatianaGivePoem] && !_doneNIS[kEventTatianaBreakfastAlexei])) {
			_items[kItemParchemin].floating = 2;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_PlayChess(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_PlayChess);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 32;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_PlayChess(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterTatiana)[1] && !getCharacterCurrentParams(kCharacterTatiana)[4]) {
			getCharacterCurrentParams(kCharacterTatiana)[0] -= _timeSpeed;
			if (_timeSpeed > getCharacterCurrentParams(kCharacterTatiana)[0]) {
				startCycOtis(kCharacterTatiana, (char *)&getCharacterCurrentParams(kCharacterTatiana)[8]);
				playDialog(kCharacterTatiana, (char *)&getCharacterCurrentParams(kCharacterTatiana)[5], -1, 0);

				if (getCharacterCurrentParams(kCharacterTatiana)[2] == 4 && inSalon(kCharacterCath))
					_globals[kGlobalOverheardTatianaAndAlexeiPlayingChess] = 1;

				getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
			}
		}

		if (getCharacterCurrentParams(kCharacterTatiana)[3] && getCharacterCurrentParams(kCharacterTatiana)[4]) {
			if ((!getCharacterCurrentParams(kCharacterTatiana)[11] && (getCharacterCurrentParams(kCharacterTatiana)[11] = _gameTime + 6300, _gameTime == -6300)) || getCharacterCurrentParams(kCharacterTatiana)[11] < _gameTime) {
				if (rcClear()) {
					getCharacter(kCharacterTatiana).characterPosition.location = 0;
					getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
					TatianaCall(&LogicManager::CONS_Tatiana_DoBlockSeqOtis, "110E", 5, 52, 0);
				}
			}
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterTatiana)[1] = 0;
		getCharacterCurrentParams(kCharacterTatiana)[2]++;

		switch (getCharacterCurrentParams(kCharacterTatiana)[2]) {
		case 1:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 900;
			startCycOtis(kCharacterTatiana, "110A");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat3160B");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[8], 12, "110A");
			break;
		case 2:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 9000;
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat3160C");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[8], 12, "110C");
			break;
		case 3:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 13500;
			startCycOtis(kCharacterTatiana, "110B");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat3160D");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[8], 12, "110D");
			break;
		case 4:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 9000;
			startCycOtis(kCharacterTatiana, "110B");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat3160E");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[8], 12, "110D");
			break;
		case 5:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 4500;
			startCycOtis(kCharacterTatiana, "110B");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat3160G");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[8], 12, "110D");
			break;
		case 6:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 4500;
			startCycOtis(kCharacterTatiana, "110B");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat3160F");
			break;
		default:
			getCharacterCurrentParams(kCharacterTatiana)[4] = 1;
			break;
		}

		break;
	case 12:
		send(kCharacterTatiana, kCharacterAlexei, 122358304, 0);
		send(kCharacterTatiana, kCharacterKronos, 157159392, 0);
		startCycOtis(kCharacterTatiana, "110C");
		playDialog(kCharacterTatiana, "Tat3160A", -1, 0);
		getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 1) {
			send(kCharacterTatiana, kCharacterAlexei, 122288808, 0);
			CONS_Tatiana_ReturnComp3(0, 0, 0, 0);
		}

		break;
	case 101169422:
		getCharacterCurrentParams(kCharacterTatiana)[3] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_ReturnComp3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_ReturnComp3);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 33;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_ReturnComp3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterTatiana);
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_DoWait, 75, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 4, 7500, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
			TatianaCall(&LogicManager::CONS_Tatiana_EnterComp, 0, 0, 0, 0);
			break;
		case 3:
			CONS_Tatiana_BeforeConcert(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_BeforeConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_BeforeConcert);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 34;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_BeforeConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_CompLogic, 2097000, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8]) {
		case 1:
			_items[kItemFirebird].floating = 1;

			if (inSuite(kCharacterCath, kCarRedSleeping, 7850))
				bumpCathRDoor(33);

			setDoor(33, kCharacterCath, 1, 10, 9);
			setDoor(49, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_ExitComp, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
			TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 2, 9270, 0, 0);
			break;
		case 3:
			CONS_Tatiana_Concert(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_Concert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_Concert);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 35;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_Concert(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterTatiana)[0] && cathHasItem(kItemFirebird) && inSuite(kCharacterCath, kCarRedSleeping, 7850) && (_gameTime < 2133000 || _globals[16])) {
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
			TatianaCall(&LogicManager::CONS_Tatiana_TrapCath, 0, 0, 0, 0);
			break;
		}

		if (_gameTime <= 2133000)
			break;

		if (getCharacter(kCharacterAugust).characterPosition.car < kCarRedSleeping && (getCharacter(kCharacterAugust).characterPosition.car != kCarGreenSleeping || getCharacter(kCharacterAugust).characterPosition.position <= 5790))
			break;

		CONS_Tatiana_LeaveConcert(0, 0, 0, 0);
		break;
	case 12:
		setDoor(33, kCharacterCath, 1, 10, 9);
		setDoor(49, kCharacterCath, 0, 10, 9);
		endGraphics(kCharacterTatiana);
		getCharacter(kCharacterTatiana).characterPosition.car = kCarKronos;
		getCharacter(kCharacterTatiana).characterPosition.position = 6000;
		getCharacter(kCharacterTatiana).characterPosition.location = 1;
		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] != 1)
			break;

		getCharacterCurrentParams(kCharacterTatiana)[0] = 1;

		if (_gameTime <= 2133000)
			break;

		if (getCharacter(kCharacterAugust).characterPosition.car < kCarRedSleeping && (getCharacter(kCharacterAugust).characterPosition.car != kCarGreenSleeping || getCharacter(kCharacterAugust).characterPosition.position <= 5790))
			break;

		CONS_Tatiana_LeaveConcert(0, 0, 0, 0);
		break;
	case 191668032:
		CONS_Tatiana_LeaveConcert(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_LeaveConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_LeaveConcert);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 36;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_LeaveConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterTatiana).characterPosition.position = 850;
		getCharacter(kCharacterTatiana).characterPosition.location = 0;
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 4, 7500, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 1) {
			if (!inSuite(kCharacterCath, kCarRedSleeping, 7850) || inComp(kCharacterCath, kCarRedSleeping, 8200)) {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
				TatianaCall(&LogicManager::CONS_Tatiana_EnterComp, 0, 0, 0, 0);
				break;
			}

			if (cathHasItem(kItemFirebird)) {
				playNIS(kEventTatianaCompartmentStealEgg);
				takeCathItem(kItemFirebird);
				_items[kItemFirebird].floating = 2;
			} else {
				playNIS(kEventTatianaCompartment);
			}

			bumpCathRDoor(33);
		} else if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 2) {
			CONS_Tatiana_AfterConcert(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_AfterConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_AfterConcert);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 37;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_AfterConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_items[kItemFirebird].floating != 1 && _items[kItemFirebird].floating != 2) {
			if (!getCharacterCurrentParams(kCharacterTatiana)[2])
				getCharacterCurrentParams(kCharacterTatiana)[2] = _gameTime + 900;

			if (getCharacterCurrentParams(kCharacterTatiana)[3] != 0x7FFFFFFF && getCharacterCurrentParams(kCharacterTatiana)[2] < _gameTime) {
				if (_gameTime > 2227500) {
					getCharacterCurrentParams(kCharacterTatiana)[3] = 0x7FFFFFFF;
					_globals[kGlobalTatianaFoundOutEggStolen] = 1;

					if (inComp(kCharacterAnna, kCarRedSleeping, 4070)) {
						CONS_Tatiana_CryAnna(0, 0, 0, 0);
						break;
					}
				} else {
					bool skip = false;
					if (!cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterTatiana)[3]) {
						getCharacterCurrentParams(kCharacterTatiana)[3] = _gameTime + 450;
						if (_gameTime == -450) {
							skip = true;

							_globals[kGlobalTatianaFoundOutEggStolen] = 1;
							if (inComp(kCharacterAnna, kCarRedSleeping, 4070)) {
								CONS_Tatiana_CryAnna(0, 0, 0, 0);
								break;
							}
						}
					}

					if (!skip && getCharacterCurrentParams(kCharacterTatiana)[3] < _gameTime) {
						getCharacterCurrentParams(kCharacterTatiana)[3] = 0x7FFFFFFF;
						_globals[kGlobalTatianaFoundOutEggStolen] = 1;
						if (inComp(kCharacterAnna, kCarRedSleeping, 4070)) {
							CONS_Tatiana_CryAnna(0, 0, 0, 0);
							break;
						}
					}
				}
			}
		}

		if (getCharacterCurrentParams(kCharacterTatiana)[0]) {
			if (getCharacterCurrentParams(kCharacterTatiana)[4] || (getCharacterCurrentParams(kCharacterTatiana)[4] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterTatiana)[4] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterTatiana)[4] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterTatiana)[0] = 0;
			getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
			setDoor(33, kCharacterTatiana, 1, 0, 0);
			setDoor(49, kCharacterTatiana, 1, 0, 0);
			getCharacterCurrentParams(kCharacterTatiana)[4] = 0;
		} else {
			getCharacterCurrentParams(kCharacterTatiana)[4] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterTatiana)[0]) {
			setDoor(33, kCharacterTatiana, 1, 0, 0);
			setDoor(49, kCharacterTatiana, 1, 0, 0);

			if (msg->param.intParam == 49) {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 4;
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, getCathWCDialog(), 0, 0, 0);
			} else if (cathHasItem(kItemPassengerList)) {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 5;

				if (rnd(2) != 0) {
					TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, getCathSorryDialog(), 0, 0, 0);
				} else {
					TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "CAT1512", 0, 0, 0);
				}
			} else {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 6;
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else if (msg->param.intParam == 49) {
			if (cathHasItem(kItemFirebird)) {
				playNIS(kEventTatianaCompartmentStealEgg);
				takeCathItem(kItemFirebird);
				_items[kItemFirebird].floating = 2;
			} else {
				playNIS(kEventTatianaCompartment);
			}

			bumpCathRDoor(33);
		} else {
			setDoor(33, kCharacterTatiana, 1, 0, 0);
			setDoor(49, kCharacterTatiana, 1, 0, 0);

			if (msg->action == 8) {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		setDoor(33, kCharacterTatiana, 1, 10, 9);
		setDoor(49, kCharacterTatiana, 1, 0, 9);
		getCharacter(kCharacterTatiana).characterPosition.location = 1;
		endGraphics(kCharacterTatiana);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterTatiana)[1] || getCharacterCurrentParams(kCharacterTatiana)[0]) {
			getCharacterCurrentParams(kCharacterTatiana)[1] = 0;
			getCharacterCurrentParams(kCharacterTatiana)[0] = 0;
			setDoor(33, kCharacterTatiana, 1, 10, 9);
			setDoor(49, kCharacterTatiana, 1, 0, 9);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8]) {
		case 1:
		case 2:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
			if (rnd(2) == 0) {
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "TAT1133B", 0, 0, 0);
			} else {
				TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "TAT1133A", 0, 0, 0);
			}

			break;
		case 3:
			setDoor(33, kCharacterTatiana, 1, 14, 0);
			setDoor(49, kCharacterTatiana, 1, 14, 0);
			getCharacterCurrentParams(kCharacterTatiana)[0] = 1;
			break;
		case 4:
		case 5:
		case 6:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 0;
			getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_CryAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_CryAnna);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 38;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_CryAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterTatiana)[0] || (getCharacterCurrentParams(kCharacterTatiana)[0] = _gameTime + 450, _gameTime != -450)) {
			if (getCharacterCurrentParams(kCharacterTatiana)[0] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterTatiana)[0] = 0x7FFFFFFF;
		}

		softReleaseAtDoor(kCharacterTatiana, 37);
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 4;
		TatianaCall(&LogicManager::CONS_Tatiana_WalkSniffle, 4, 7500, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterTatiana).clothes = 3;
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_DoCorrOtis, "673Jb", 33, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTatiana).characterPosition.location = 0;
			setDoor(33, kCharacterCath, 0, 10, 9);
			setDoor(49, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_WalkSniffle, 4, 4070, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterTatiana, "673Gf");
			softBlockAtDoor(kCharacterTatiana, 37);
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
			TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "Tat3164", 0, 0, 0);
			break;
		case 3:
			send(kCharacterTatiana, kCharacterAnna, 236241630, 0);
			break;
		case 4:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 5;
			TatianaCall(&LogicManager::CONS_Tatiana_DoEnterCorrOtis, "673Db", 0x21, 0, 0);
			break;
		case 5:
		case 11:
			getCharacter(kCharacterTatiana).characterPosition.location = 1;
			endGraphics(kCharacterTatiana);
			CONS_Tatiana_Upset(0, 0, 0, 0);
			break;
		case 6:
			softReleaseAtDoor(kCharacterTatiana, 37);
			endGraphics(kCharacterTatiana);
			getCharacter(kCharacterTatiana).characterPosition.location = 1;
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 7;
			TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "ANN3011", 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 8;
			TatianaCall(&LogicManager::CONS_Tatiana_DoWait, 900, 0, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 9;
			TatianaCall(&LogicManager::CONS_Tatiana_DoCorrOtis, "673Jf", 37, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterTatiana).characterPosition.location = 0;
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 10;
			TatianaCall(&LogicManager::CONS_Tatiana_WalkSniffle, 4, 7500, 0, 0);
			break;
		case 10:
			send(kCharacterTatiana, kCharacterAnna, 236517970, 0);
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 11;
			TatianaCall(&LogicManager::CONS_Tatiana_DoEnterCorrOtis, "673Db", 33, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 100906246:
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 6;
		TatianaCall(&LogicManager::CONS_Tatiana_DoCorrOtis, "673Df", 37, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_Upset(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_Upset);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 39;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_Upset(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterTatiana)[0] && nearChar(kCharacterTatiana, kCharacterCath, 1000)) {
			playDialog(kCharacterTatiana, "Tat3164", -1, 0);
			getCharacterCurrentParams(kCharacterTatiana)[0] = 1;
		}

		break;
	case 12:
		setDoor(33, kCharacterCath, 1, 10, 9);
		setDoor(49, kCharacterCath, 1, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_DoConcertWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_DoConcertWalk);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 40;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_DoConcertWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (checkLoc(kCharacterCath, kCarKronos) || getCharacter(kCharacterTatiana).characterPosition.car != getCharacter(kCharacterCath).characterPosition.car || walk(kCharacterTatiana, kCarKronos, 9270)) {
			getCharacter(kCharacterTatiana).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
			fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		}

		break;
	case 6:
		if (_doneNIS[kEventTatianaAskMatchSpeakRussian] || _doneNIS[kEventTatianaAskMatch] || _doneNIS[kEventVassiliSeizure]) {
			if (rnd(2) == 0) {
				playDialog(kCharacterCath, "CAT1010A", -1, 0);
			} else {
				playDialog(kCharacterCath, "CAT1010", -1, 0);
			}
		} else {
			playCathExcuseMe();
		}

		break;
	case 12:
		if (walk(kCharacterTatiana, kCarKronos, 9270)) {
			getCharacter(kCharacterTatiana).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
			fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_TrapCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_TrapCath);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 41;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_TrapCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterTatiana)[0]) {
			if (inSuite(kCharacterCath, kCarRedSleeping, 7850) && !_doneNIS[kEventVassiliCompartmentStealEgg] && (_gameTime <= 2133000 || _globals[kGlobalConcertIsHappening])) {
				if (inComp(kCharacterCath, kCarRedSleeping, 7500)) {
					send(kCharacterTatiana, kCharacterCond2, 235061888, 0);
					endGraphics(kCharacterTatiana);
					softReleaseAtDoor(kCharacterTatiana, 33);
					getCharacter(kCharacterTatiana).characterPosition.location = 1;

					if (cathHasItem(kItemFirebird)) {
						playNIS(kEventTatianaCompartmentStealEgg);
						takeCathItem(kItemFirebird);
						_items[kItemFirebird].floating = 2;
					} else {
						playNIS(kEventTatianaCompartment);
					}

					bumpCathRDoor(33);
					getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 4;
					TatianaCall(&LogicManager::CONS_Tatiana_DoWait, 150, 0, 0, 0);
				}
			} else {
				softReleaseAtDoor(kCharacterTatiana, 33);
				if (_gameTime < 2133000 || _globals[16]) {
					getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
					TatianaCall(&LogicManager::CONS_Tatiana_DoConcertWalk, 0, 0, 0, 0);
				} else {
					endGraphics(kCharacterTatiana);

					getCharacter(kCharacterTatiana).currentCall--;
					_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
					fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
				}
			}
		}

		break;
	case 12:
		getCharacter(kCharacterTatiana).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterTatiana).characterPosition.position = 7500;
		getCharacter(kCharacterTatiana).characterPosition.location = 0;
		forceJump(kCharacterCond2, &LogicManager::CONS_Cond2_TatianaComeHere);
		startCycOtis(kCharacterTatiana, "673Fb");
		softBlockAtDoor(kCharacterTatiana, 33);
		break;
	case 18:
		switch (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_DoDialog, "Tat3161B", 0, 0, 0);
			break;
		case 2:
			send(kCharacterTatiana, kCharacterCond2, 168316032, 0);
			getCharacterCurrentParams(kCharacterTatiana)[0] = 1;
			break;
		case 3:
		case 6:
			endGraphics(kCharacterTatiana);

			getCharacter(kCharacterTatiana).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
			fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
			break;
		case 4:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 5;
			TatianaCall(&LogicManager::CONS_Tatiana_ExitComp, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 6;
			TatianaCall(&LogicManager::CONS_Tatiana_DoConcertWalk, 0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 154071333:
		setDoor(33, kCharacterCath, 1, 0, 0);
		setDoor(32, kCharacterCath, 1, 0, 0);
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_SaveGame, 1, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_WalkSniffle(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_WalkSniffle);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 42;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_WalkSniffle(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterTatiana, getCharacterCurrentParams(kCharacterTatiana)[0], getCharacterCurrentParams(kCharacterTatiana)[1])) {
			getCharacter(kCharacterTatiana).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, _functionsTatiana[getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall]]);
			fedEx(kCharacterTatiana, kCharacterTatiana, 18, 0);
		}

		break;
	case 5:
	case 6:
		playDialog(kCharacterCath, "Tat3124", getVolume(kCharacterTatiana), 0);
		break;	
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_StartPart4);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 43;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Tatiana_InComp(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterTatiana);
		getCharacter(kCharacterTatiana).characterPosition.position = 7500;
		getCharacter(kCharacterTatiana).characterPosition.location = 1;
		getCharacter(kCharacterTatiana).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterTatiana).inventoryItem = 0;
		getCharacter(kCharacterTatiana).clothes = 2;
		getCharacter(kCharacterTatiana).callParams[8].parameters[0] = 0;
		setDoor(33, kCharacterCath, 1, 10, 9);
		setDoor(49, kCharacterCath, 1, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_InComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_InComp);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 44;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_InComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_CompLogic, 2362500, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 1) {
			CONS_Tatiana_MeetAlexei(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_MeetAlexei(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_MeetAlexei);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 45;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_MeetAlexei(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_DoCorrOtis, "673Bb", 33, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 1) {
			setDoor(33, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterTatiana).characterPosition.location = 0;
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 3, 540, 0, 0);
		} else if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 2) {
			if (onLowPlatform(kCharacterCath)) {
				playChrExcuseMe(kCharacterTatiana, kCharacterCath, 0);

				if (checkCathDir(kCarGreenSleeping, 62))
					bumpCath(kCarGreenSleeping, 72, 255);
			}

			send(kCharacterTatiana, kCharacterAlexei, 123712592, 0);
			CONS_Tatiana_WithAlexei(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_WithAlexei(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_WithAlexei);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 46;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_WithAlexei(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterTatiana)[1] && !getCharacterCurrentParams(kCharacterTatiana)[2]) {
			getCharacterCurrentParams(kCharacterTatiana)[0] -= _timeSpeed;
			if (_timeSpeed > getCharacterCurrentParams(kCharacterTatiana)[0]) {
				playDialog(kCharacterTatiana, (char *)&getCharacterCurrentParams(kCharacterTatiana)[5], -1, 0);
				if (getCharacterCurrentParams(kCharacterTatiana)[3] == 4 && nearChar(kCharacterTatiana, kCharacterCath, 2000)) {
					_globals[kGlobalOverheardAlexeiTellingTatianaAboutWantingToKillVassili] = 1;
				} else if (getCharacterCurrentParams(kCharacterTatiana)[3] == 7 && nearChar(kCharacterTatiana, kCharacterCath, 2000)) {
					_globals[kGlobalOverheardAlexeiTellingTatianaAboutBomb] = 1;
				}

				getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
			}
		}

		if (getCharacterCurrentParams(kCharacterTatiana)[8] == 0x7FFFFFFF || _gameTime <= 2394000)
			break;

		if (_gameTime > 2398500) {
			getCharacterCurrentParams(kCharacterTatiana)[8] = 0x7FFFFFFF;
		} else {
			if ((!onLowPlatform(kCharacterCath) && getCharacterCurrentParams(kCharacterTatiana)[8]) ||
				(getCharacterCurrentParams(kCharacterTatiana)[8] = _gameTime, _gameTime)) {
				if (getCharacterCurrentParams(kCharacterTatiana)[8] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterTatiana)[8] = 0x7FFFFFFF;
			}
		}

		if (onLowPlatform(kCharacterCath)) {
			playChrExcuseMe(kCharacterTatiana, kCharacterCath, 0);
			if (checkCathDir(kCarGreenSleeping, 62))
				bumpCath(kCarGreenSleeping, 72, 255);
		}

		send(kCharacterTatiana, kCharacterAlexei, 123536024, 0);
		CONS_Tatiana_Thinking(0, 0, 0, 0);
		break;
	case 2:
		getCharacterCurrentParams(kCharacterTatiana)[1] = 0;
		getCharacterCurrentParams(kCharacterTatiana)[3]++;
		switch (getCharacterCurrentParams(kCharacterTatiana)[3]) {
		case 1:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 900;
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat4165F");
			break;
		case 2:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 900;
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat4165B");
			break;
		case 3:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 1800;
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat4165G");
			break;
		case 4:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 900;
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat4165H");
			break;
		case 5:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 2700;
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat4165C");
			break;
		case 6:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 900;
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat4165D");
			break;
		case 7:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 900;
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat4165E");
			break;
		default:
			getCharacterCurrentParams(kCharacterTatiana)[0] = 162000;
			break;
		}

		break;
	case 12:
		startCycOtis(kCharacterTatiana, "306E");
		getCharacterCurrentParams(kCharacterTatiana)[0] = 450;
		Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterTatiana)[5], 12, "Tat4165A");
		break;
	case 17:
		if (onLowPlatform(kCharacterCath)) {
			getCharacterCurrentParams(kCharacterTatiana)[2] = 1;
			if (getCharacterCurrentParams(kCharacterTatiana)[1]) {
				endDialog(kCharacterTatiana);
				fedEx(kCharacterTatiana, kCharacterTatiana, 2, 0);
			}
		} else {
			getCharacterCurrentParams(kCharacterTatiana)[2] = 0;
			getCharacterCurrentParams(kCharacterTatiana)[4] = 0;
		}

		if (checkCathDir(kCarGreenSleeping, 62) && !getCharacterCurrentParams(kCharacterTatiana)[4]) {
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
			TatianaCall(&LogicManager::CONS_Tatiana_DoSeqOtis, "306D", 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 1) {
			startCycOtis(kCharacterTatiana, "306E");
			getCharacterCurrentParams(kCharacterTatiana)[4] = 1;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_Thinking(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_Thinking);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 47;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_Thinking(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 4, 7500, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_DoEnterCorrOtis, "673Db", 33, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterTatiana).characterPosition.location = 1;
			endGraphics(kCharacterTatiana);
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
			TatianaCall(&LogicManager::CONS_Tatiana_CompLogic, 2407500, 0, 0, 0);
			break;
		case 3:
		case 4:
			if (getCharacter(kCharacterTatiana).callParams[8].parameters[0] && getModel(1) == 1) {
				CONS_Tatiana_SeekCath(0, 0, 0, 0);
			} else {
				getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 4;
				TatianaCall(&LogicManager::CONS_Tatiana_CompLogic, 900, 0, 0, 0);
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

void LogicManager::CONS_Tatiana_SeekCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_SeekCath);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 48;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_SeekCath(HAND_PARAMS) {
	int rndNum;
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterTatiana)[0]) {
			if (!_doneNIS[kEventTatianaTylerCompartment] && inComp(kCharacterCath, kCarGreenSleeping, 8200)) {
				getCharacterCurrentParams(kCharacterTatiana)[0] = 1;
				_globals[kGlobalTatianaScheduledToVisitCath] = 1;
				setDoor(1, kCharacterTatiana, checkDoor(1), 0, 9);
			}

			if (!getCharacterCurrentParams(kCharacterTatiana)[0]) {
				if (_doneNIS[kEventTatianaTylerCompartment] || _gameTime > 2475000) {
					// Dead-code, byproduct of unrolling a goto...
					// 
					// if (getCharacterCurrentParams(kCharacterTatiana)[0]) {
					// 	setDoor(1, kCharacterCath, checkDoor(1), 10, 9);
					// }

					_globals[kGlobalTatianaScheduledToVisitCath] = 0;
					softReleaseAtDoor(kCharacterTatiana, 2);
					getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
					TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 4, 7500, 0, 0);
				}

				break;
			}
		}

		if (!inSuite(kCharacterCath, kCarGreenSleeping, 7850)) {
			setDoor(1, kCharacterCath, checkDoor(1), 10, 9);
			getCharacterCurrentParams(kCharacterTatiana)[0] = 0;
		}

		if (getCharacterCurrentParams(kCharacterTatiana)[0] && !whoRunningDialog(kCharacterTatiana)) {
			rndNum = 5 * (3 * rnd(5) + 30);
			if (getCharacterCurrentParams(kCharacterTatiana)[1] || (getCharacterCurrentParams(kCharacterTatiana)[1] = _realTime + rndNum,
																	_realTime + rndNum != 0)) {
				if (getCharacterCurrentParams(kCharacterTatiana)[1] >= _realTime) {
					if (_doneNIS[kEventTatianaTylerCompartment] || _gameTime > 2475000) {
						if (getCharacterCurrentParams(kCharacterTatiana)[0]) {
							setDoor(1, kCharacterCath, checkDoor(1), 10, 9);
						}
						_globals[kGlobalTatianaScheduledToVisitCath] = 0;
						softReleaseAtDoor(kCharacterTatiana, 2);
						getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
						TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 4, 7500, 0, 0);
					}

					break;
				}

				getCharacterCurrentParams(kCharacterTatiana)[1] = 0x7FFFFFFF;
			}

			playDialog(kCharacterTatiana, "LIB012", 16, 0);
			getCharacterCurrentParams(kCharacterTatiana)[1] = 0;
		}

		if (_doneNIS[kEventTatianaTylerCompartment] || _gameTime > 2475000) {
			if (getCharacterCurrentParams(kCharacterTatiana)[0]) {
				setDoor(1, kCharacterCath, checkDoor(1), 10, 9);
			}
			_globals[kGlobalTatianaScheduledToVisitCath] = 0;
			softReleaseAtDoor(kCharacterTatiana, 2);
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 3;
			TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 4, 7500, 0, 0);
		}

		break;
	case 9:
		getCharacterCurrentParams(kCharacterTatiana)[0] = 0;
		setDoor(1, kCharacterCath, 0, 10, 9);
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 5;
		TatianaCall(&LogicManager::CONS_Tatiana_SaveGame, 2, kEventTatianaTylerCompartment, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_DoCorrOtis, "673Bb", 33, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8]) {
		case 1:
			setDoor(33, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterTatiana).characterPosition.location = 0;
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 2;
			TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 3, 7500, 0, 0);
			break;
		case 2:
		case 7:
			startCycOtis(kCharacterTatiana, "673Fb");
			softBlockAtDoor(kCharacterTatiana, 2);
			break;
		case 3:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 4;
			TatianaCall(&LogicManager::CONS_Tatiana_DoEnterCorrOtis, "673Db", 33, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterTatiana).characterPosition.location = 1;
			endGraphics(kCharacterTatiana);
			CONS_Tatiana_Asleep4(0, 0, 0, 0);
			break;
		case 5:
			setDoor(9, kCharacterCath, 0, 255, 255);
			playNIS(kEventTatianaTylerCompartment);
			playDialog(0, "LIB015", -1, 0);
			bumpCathTylerComp();
			break;
		case 6:
			getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 7;
			TatianaCall(&LogicManager::CONS_Tatiana_DoWalk, 3, 7500, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 238790488:
		getCharacterCurrentParams(kCharacterTatiana)[0] = 0;
		setDoor(1, kCharacterCath, checkDoor(1), 10, 9);
		softReleaseAtDoor(kCharacterTatiana, 2);
		endGraphics(kCharacterTatiana);
		getCharacter(kCharacterTatiana).characterPosition.car = 3;
		getCharacter(kCharacterTatiana).characterPosition.position = 9460;
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 6;
		TatianaCall(&LogicManager::CONS_Tatiana_DoWait, 1800, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_Asleep4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_Asleep4);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 49;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_Asleep4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTatiana).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterTatiana).characterPosition.position = 7500;
		getCharacter(kCharacterTatiana).characterPosition.location = 1;
		setDoor(33, kCharacterCath, 1, 10, 9);
		setDoor(49, kCharacterCath, 1, 10, 9);
		break;
	case 169360385:
		CONS_Tatiana_AlexeiDead(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_AlexeiDead(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_AlexeiDead);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 50;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_AlexeiDead(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2520000 && !getCharacterCurrentParams(kCharacterTatiana)[0]) {
			getCharacterCurrentParams(kCharacterTatiana)[0] = 1;
			CONS_Tatiana_Calm(0, 0, 0, 0);
		}

		break;
	case 2:
		playDialog(kCharacterTatiana, "Tat4166", -1, 0);
		break;
	case 8:
		if (!cathRunningDialog("LIB012"))
			playDialog(0, "LIB012", -1, 0);

		break;
	case 9:
		playDialog(0, "LIB014", -1, 0);
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_SaveGame, 2, kEventVassiliDeadAlexei, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterTatiana).characterPosition.position = 8200;
		getCharacter(kCharacterTatiana).characterPosition.location = 1;
		getCharacter(kCharacterTatiana).characterPosition.car = kCarRedSleeping;
		setDoor(33, kCharacterCath, 0, 10, 9);
		setDoor(49, kCharacterCath, 0, 10, 9);
		setDoor(48, kCharacterTatiana, 0, 10, 9);
		setDoor(32, kCharacterTatiana, 0, 10, 9);

		if (!whoRunningDialog(kCharacterTatiana))
			playDialog(kCharacterTatiana, "Tat4166", -1, 0);

		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 1) {
			if (dialogRunning("MUS013"))
				fadeDialog("MUS013");

			playNIS(kEventVassiliDeadAlexei);
			send(kCharacterTatiana, kCharacterAbbot, 104060776, 0);
			bumpCath(kCarRedSleeping, 38, 255);
			CONS_Tatiana_Calm(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_Calm(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_Calm);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 51;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_Calm(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(32, kCharacterCath, 1, 0, 0);
		setDoor(48, kCharacterCath, 1, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_StartPart5);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 52;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Tatiana_Trapped(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterTatiana);
		getCharacter(kCharacterTatiana).characterPosition.location = 1;
		getCharacter(kCharacterTatiana).characterPosition.position = 3969;
		getCharacter(kCharacterTatiana).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterTatiana).inventoryItem = 0;
		getCharacter(kCharacterTatiana).clothes = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_Trapped(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_Trapped);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 53;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_Trapped(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Tatiana_Autistic(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_Autistic(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_Autistic);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 54;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_Autistic(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterTatiana)[1]) {
			switch (getCharacterCurrentParams(kCharacterTatiana)[0]) {
			case 0:
				playDialog(kCharacterTatiana, "Tat5167A", -1, 0);
				getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
				break;
			case 1:
				playDialog(kCharacterTatiana, "Tat5167B", -1, 0);
				getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
				break;
			case 2:
				playDialog(kCharacterTatiana, "Tat5167C", -1, 0);
				getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
				break;
			case 3:
				playDialog(kCharacterTatiana, "Tat5167D", -1, 0);
				getCharacterCurrentParams(kCharacterTatiana)[1] = 1;
				break;
			default:
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterTatiana)[0] > 3) {
			if (getCharacterCurrentParams(kCharacterTatiana)[2] || (getCharacterCurrentParams(kCharacterTatiana)[2] = _realTime + 225, _realTime != -225)) {
				if (getCharacterCurrentParams(kCharacterTatiana)[2] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterTatiana)[2] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterTatiana)[0] = 0;
			getCharacterCurrentParams(kCharacterTatiana)[2] = 0;
		}

		break;
	case 1:
		getCharacter(kCharacterTatiana).inventoryItem = 0;
		getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] = 1;
		TatianaCall(&LogicManager::CONS_Tatiana_SaveGame, 2, kEventTatianaVassiliTalk, 0, 0);
		break;
	case 2:
		getCharacterCurrentParams(kCharacterTatiana)[0]++;
		getCharacterCurrentParams(kCharacterTatiana)[1] = 0;
		break;
	case 12:
		startCycOtis(kCharacterTatiana, "033A");
		getCharacter(kCharacterTatiana).inventoryItem = 0x80;
		break;
	case 18:
		if (getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall + 8] == 1) {
			if (dialogRunning("MUS050"))
				fadeDialog("MUS050");

			if (whoRunningDialog(kCharacterTatiana))
				fadeDialog(kCharacterTatiana);

			if (isNight()) {
				playNIS(kEventTatianaVassiliTalkNight);
			} else {
				playNIS(kEventTatianaVassiliTalk);
			}
			
			cleanNIS();
			getCharacterCurrentParams(kCharacterTatiana)[0] = 4;
			getCharacterCurrentParams(kCharacterTatiana)[1] = 0;
			getCharacterCurrentParams(kCharacterTatiana)[2] = 0;
		}

		break;
	case 203078272:
		startCycOtis(kCharacterTatiana, "033E");
		break;
	case 236060709:
		getCharacter(kCharacterTatiana).inventoryItem = 0;
		CONS_Tatiana_Asleep5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Tatiana_Asleep5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTatiana).callParams[getCharacter(kCharacterTatiana).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTatiana, &LogicManager::HAND_Tatiana_Asleep5);
	getCharacter(kCharacterTatiana).callbacks[getCharacter(kCharacterTatiana).currentCall] = 55;

	params->clear();

	fedEx(kCharacterTatiana, kCharacterTatiana, 12, 0);
}

void LogicManager::HAND_Tatiana_Asleep5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterTatiana);
		if (checkCathDir(kCarRestaurant, 72))
			bumpCath(kCarRestaurant, 86, 255);

		break;
	case 17:
		if (checkCathDir(kCarRestaurant, 72))
			bumpCath(kCarRestaurant, 86, 255);

		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsTatiana[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Tatiana_DebugWalks,
	&LogicManager::HAND_Tatiana_DoDialog,
	&LogicManager::HAND_Tatiana_DoSeqOtis,
	&LogicManager::HAND_Tatiana_DoBlockSeqOtis,
	&LogicManager::HAND_Tatiana_DoCorrOtis,
	&LogicManager::HAND_Tatiana_DoEnterCorrOtis,
	&LogicManager::HAND_Tatiana_DoJoinedSeqOtis,
	&LogicManager::HAND_Tatiana_FinishSeqOtis,
	&LogicManager::HAND_Tatiana_DoWaitRealTime,
	&LogicManager::HAND_Tatiana_DoWait,
	&LogicManager::HAND_Tatiana_WaitRCClear,
	&LogicManager::HAND_Tatiana_SaveGame,
	&LogicManager::HAND_Tatiana_DoWalk,
	&LogicManager::HAND_Tatiana_EnterComp,
	&LogicManager::HAND_Tatiana_ExitComp,
	&LogicManager::HAND_Tatiana_CompLogic,
	&LogicManager::HAND_Tatiana_Birth,
	&LogicManager::HAND_Tatiana_DoSpecialSalonWalk,
	&LogicManager::HAND_Tatiana_AtDinner,
	&LogicManager::HAND_Tatiana_ExitDining,
	&LogicManager::HAND_Tatiana_ReturnToComp,
	&LogicManager::HAND_Tatiana_GetSomeAir,
	&LogicManager::HAND_Tatiana_ReturnToCompAgain,
	&LogicManager::HAND_Tatiana_Asleep,
	&LogicManager::HAND_Tatiana_StartPart2,
	&LogicManager::HAND_Tatiana_EatingBreakfast,
	&LogicManager::HAND_Tatiana_BreakfastClick,
	&LogicManager::HAND_Tatiana_JoinAlexei,
	&LogicManager::HAND_Tatiana_LeaveBreakfast,
	&LogicManager::HAND_Tatiana_ReturnComp,
	&LogicManager::HAND_Tatiana_StartPart3,
	&LogicManager::HAND_Tatiana_PlayChess,
	&LogicManager::HAND_Tatiana_ReturnComp3,
	&LogicManager::HAND_Tatiana_BeforeConcert,
	&LogicManager::HAND_Tatiana_Concert,
	&LogicManager::HAND_Tatiana_LeaveConcert,
	&LogicManager::HAND_Tatiana_AfterConcert,
	&LogicManager::HAND_Tatiana_CryAnna,
	&LogicManager::HAND_Tatiana_Upset,
	&LogicManager::HAND_Tatiana_DoConcertWalk,
	&LogicManager::HAND_Tatiana_TrapCath,
	&LogicManager::HAND_Tatiana_WalkSniffle,
	&LogicManager::HAND_Tatiana_StartPart4,
	&LogicManager::HAND_Tatiana_InComp,
	&LogicManager::HAND_Tatiana_MeetAlexei,
	&LogicManager::HAND_Tatiana_WithAlexei,
	&LogicManager::HAND_Tatiana_Thinking,
	&LogicManager::HAND_Tatiana_SeekCath,
	&LogicManager::HAND_Tatiana_Asleep4,
	&LogicManager::HAND_Tatiana_AlexeiDead,
	&LogicManager::HAND_Tatiana_Calm,
	&LogicManager::HAND_Tatiana_StartPart5,
	&LogicManager::HAND_Tatiana_Trapped,
	&LogicManager::HAND_Tatiana_Autistic,
	&LogicManager::HAND_Tatiana_Asleep5
};

} // End of namespace LastExpress
