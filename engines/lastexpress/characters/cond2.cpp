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

void LogicManager::CONS_Cond2(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterCond2,
			_functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]
		);

		break;
	case 1:
		CONS_Cond2_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Cond2_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Cond2_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Cond2_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Cond2_StartPart5(0, 0, 0, 0);
		break;
	default:
		return;
	}
}

void LogicManager::Cond2Call(CALL_PARAMS) {
	getCharacter(kCharacterCond2).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Cond2_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_DebugWalks);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 1;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterCond2, kCarGreenSleeping, getCharacterCurrentParams(kCharacterCond2)[0])) {
			if (getCharacterCurrentParams(kCharacterCond2)[0] == 10000) {
				getCharacterCurrentParams(kCharacterCond2)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterCond2)[0] = 10000;
			}
		}

		break;
	case 1:
		getCharacter(kCharacterCond2).clothes++;
		if (getCharacter(kCharacterCond2).clothes > 1)
			getCharacter(kCharacterCond2).clothes = 0;

		break;
	case 12:
		getCharacter(kCharacterCond2).characterPosition.position = 0;
		getCharacter(kCharacterCond2).characterPosition.location = 0;
		getCharacter(kCharacterCond2).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterCond2).inventoryItem = 0x80;
		getCharacterCurrentParams(kCharacterCond2)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_DoSeqOtis);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 && nearChar(kCharacterCond2, kCharacterCath, 1000) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterCond2, (char *)&getCharacterCurrentParams(kCharacterCond2)[0]);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_DoCorrOtis);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 && nearChar(kCharacterCond2, kCharacterCath, 1000) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 3:
		releaseAtDoor(kCharacterCond2, getCharacterCurrentParams(kCharacterCond2)[3]);

		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterCond2, (char *)&getCharacterCurrentParams(kCharacterCond2)[0]);
		blockAtDoor(kCharacterCond2, getCharacterCurrentParams(kCharacterCond2)[3]);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_FinishSeqOtis);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 4;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterCond2).direction != 4) {
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		} else if (_globals[kGlobalJacket] == 1 && nearChar(kCharacterCond2, kCharacterCath, 1000) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_DoEnterCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_DoEnterCorrOtis);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;
	params->parameters[5] = param4.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_DoEnterCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 && nearChar(kCharacterCond2, kCharacterCath, 1000) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 3:
		releaseAtDoor(kCharacterCond2, getCharacterCurrentParams(kCharacterCond2)[3]);
		getCharacter(kCharacterCond2).characterPosition.position = getCharacterCurrentParams(kCharacterCond2)[4];

		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterCond2, (char *)&getCharacterCurrentParams(kCharacterCond2)[0]);
		blockAtDoor(kCharacterCond2, getCharacterCurrentParams(kCharacterCond2)[3]);
		getCharacter(kCharacterCond2).characterPosition.location = 1;
		if (inComp(kCharacterCath, kCarRedSleeping, getCharacterCurrentParams(kCharacterCond2)[4]) || inComp(kCharacterCath, kCarRedSleeping, getCharacterCurrentParams(kCharacterCond2)[5])) {
			if (isNight()) {
				playNIS(kEventCathTurningNight);
			} else {
				playNIS(kEventCathTurningDay);
			}

			playDialog(0, "BUMP", -1, 0);
			bumpCathRDoor(getCharacterCurrentParams(kCharacterCond2)[3]);
		}

		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_DoDialog);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 && nearChar(kCharacterCond2, kCharacterCath, 1000) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 2:
		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	case 12:
		playDialog(kCharacterCond2, (char *)&getCharacterCurrentParams(kCharacterCond2)[0], -1, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_DoDialogFullVol(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_DoDialogFullVol);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 7;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_DoDialogFullVol(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 && nearChar(kCharacterCond2, kCharacterCath, 1000) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 2:
		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	case 12:
		playDialog(kCharacterCond2, (char *)&getCharacterCurrentParams(kCharacterCond2)[0], 16, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_SaveGame);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 8;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	case 12:
		save(
			kCharacterCond2,
			getCharacterCurrentParams(kCharacterCond2)[0],
			getCharacterCurrentParams(kCharacterCond2)[1]
		);

		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_DoWalk);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 9;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterCond2)[2] && nearChar(kCharacterCond2, kCharacterCath, 2000)) {
			getCharacter(kCharacterCond2).inventoryItem = 0x80;
		} else {
			getCharacter(kCharacterCond2).inventoryItem = 0;
		}

		if (_globals[kGlobalJacket] != 1 || !nearChar(kCharacterCond2, kCharacterCath, 1000) || inComp(kCharacterCath) || whoOutside(kCharacterCath)) {
			if (walk(kCharacterCond2, getCharacterCurrentParams(kCharacterCond2)[0], getCharacterCurrentParams(kCharacterCond2)[1])) {
				getCharacter(kCharacterCond2).inventoryItem = 0;

				getCharacter(kCharacterCond2).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
				fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			}
		} else {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 1:
		getCharacterCurrentParams(kCharacterCond2)[2] = 0;
		getCharacter(kCharacterCond2).inventoryItem = 0;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
		Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventCoudertAskTylerCompartment, 0, 0);
		break;
	case 5:
		if (getCharacter(kCharacterCond2).clothes == 1) {
			playDialog(0, "ZFX1003", getVolume(kCharacterCond2), 0);
		} else if (!whoRunningDialog(kCharacterCond2)) {
			playDialog(kCharacterCath, "JAC1112", getVolume(kCharacterCond2), 0);
		}

		break;
	case 6:
		if (getCharacter(kCharacterCond2).clothes == 1) {
			playDialog(0, "ZFX1003", getVolume(kCharacterCond2), 0);
		} else {
			playChrExcuseMe(kCharacterCond2, kCharacterCath, 0);
		}

		break;
	case 12:
		if (!_globals[kGlobalFoundCorpse] && !_doneNIS[kEventCoudertAskTylerCompartment])
			getCharacterCurrentParams(kCharacterCond2)[2] = 128;

		if (walk(kCharacterCond2, getCharacterCurrentParams(kCharacterCond2)[0], getCharacterCurrentParams(kCharacterCond2)[1])) {
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);
		} else if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 2) {
			playNIS(kEventCoudertAskTylerCompartment);

			if (getCharacter(kCharacterCond2).direction == 1)
				bumpCathFx(getCharacter(kCharacterCond2).characterPosition.car, getCharacter(kCharacterCond2).characterPosition.position - 750);
			else
				bumpCathRx(getCharacter(kCharacterCond2).characterPosition.car, getCharacter(kCharacterCond2).characterPosition.position + 750);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_DoWait);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 && nearChar(kCharacterCond2, kCharacterCath, 1000) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterCond2)[1] ||
			(getCharacterCurrentParams(kCharacterCond2)[1] = _gameTime + getCharacterCurrentParams(kCharacterCond2)[0], _gameTime + getCharacterCurrentParams(kCharacterCond2)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterCond2)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterCond2)[1] = 0x7FFFFFFF;
		} 

		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_DoWaitReal(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_DoWaitReal);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 11;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_DoWaitReal(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 && nearChar(kCharacterCond2, kCharacterCath, 1000) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterCond2)[1] ||
			(getCharacterCurrentParams(kCharacterCond2)[1] = _realTime + getCharacterCurrentParams(kCharacterCond2)[0], _realTime + getCharacterCurrentParams(kCharacterCond2)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterCond2)[1] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterCond2)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_StandAsideDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_StandAsideDialog);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 12;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_StandAsideDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (!whoRunningDialog(kCharacterCond2)) {
			if (isNight()) {
				if (isFemale(getCharacterCurrentParams(kCharacterCond2)[0])) {
					if (isSingleFemale(getCharacterCurrentParams(kCharacterCond2)[0]))
						playDialog(kCharacterCond2, "JAC1112F", -1, 0);
					else
						playDialog(kCharacterCond2, "JAC1112C", -1, 0);
				} else if (getCharacterCurrentParams(kCharacterCond2)[0] || _globals[kGlobalPhaseOfTheNight] != 2) {
					playDialog(kCharacterCond2, "JAC1112D", -1, 0);
				} else {
					switch (rnd(4)) {
					case 0:
						playDialog(kCharacterCond2, "JAC1013", -1, 0);
						break;
					case 1:
						playDialog(kCharacterCond2, "JAC1013A", -1, 0);
						break;
					case 2:
						playDialog(kCharacterCond2, "JAC1113", -1, 0);
						break;
					case 3:
						playDialog(kCharacterCond2, "JAC1113A", -1, 0);
						break;
					default:
						break;
					}
				}
			} else if (isFemale(getCharacterCurrentParams(kCharacterCond2)[0])) {
				if (isSingleFemale(getCharacterCurrentParams(kCharacterCond2)[0]))
					playDialog(kCharacterCond2, "JAC1112G", -1, 0);
				else
					playDialog(kCharacterCond2, "JAC1112B", -1, 0);
			} else {
				playDialog(kCharacterCond2, "JAC1112E", -1, 0);
			}
		}

		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_Passing(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_Passing);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 13;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_Passing(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 && nearChar(kCharacterCond2, kCharacterCath, 1000) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
			break;
		}

		if (!getCharacterCurrentParams(kCharacterCond2)[1] && !getCharacterCurrentParams(kCharacterCond2)[2]) {
			if (!getCharacterCurrentParams(kCharacterCond2)[3]) {
				getCharacterCurrentParams(kCharacterCond2)[3] = _realTime + 75;
				if (_realTime == -75) {
					getCharacter(kCharacterCond2).inventoryItem = 0;
					getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
					Cond2Call(&LogicManager::CONS_Cond2_SitDownFast, 1, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterCond2)[3] < _realTime) {
				getCharacterCurrentParams(kCharacterCond2)[3] = 0x7FFFFFFF;

				getCharacter(kCharacterCond2).inventoryItem = 0;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
				Cond2Call(&LogicManager::CONS_Cond2_SitDownFast, 1, 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterCond2)[4] ||
			(getCharacterCurrentParams(kCharacterCond2)[4] = _realTime + 225, _realTime != -225)) {
			if (getCharacterCurrentParams(kCharacterCond2)[4] >= _realTime) {
				break;
			}

			getCharacterCurrentParams(kCharacterCond2)[4] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterCond2).inventoryItem = 0;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
		Cond2Call(&LogicManager::CONS_Cond2_SitDownFast, 1, 0, 0, 0);
		break;
	case 1:
		getCharacter(kCharacterCond2).inventoryItem = 0;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
		Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventCoudertAskTylerCompartment, 0, 0);
		break;
	case 11:
		getCharacterCurrentParams(kCharacterCond2)[2]++;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
		Cond2Call(&LogicManager::CONS_Cond2_StandAsideDialog, msg->sender, 0, 0, 0);
		break;
	case 12:
		if (getCharacterCurrentParams(kCharacterCond2)[1])
			getCharacterCurrentParams(kCharacterCond2)[2] = 1;

		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_StandAsideDialog, getCharacterCurrentParams(kCharacterCond2)[1], 0, 0, 0);
		break;
	case 16:
		getCharacterCurrentParams(kCharacterCond2)[2]--;
		if (getCharacterCurrentParams(kCharacterCond2)[1] && getCharacterCurrentParams(kCharacterCond2)[2] == 0) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_SitDownFast, 1, 0, 0, 0);
		}

		break;
	case 17:
		if (!getCharacterCurrentParams(kCharacterCond2)[2]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_SitDownFast, 1, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_GetUpListen, 1, 0, 0, 0);
			break;
		case 2:
			if (_globals[kGlobalChapter] == 1 && !_globals[kGlobalFoundCorpse] && !_doneNIS[kEventCoudertAskTylerCompartment]) {
				getCharacter(kCharacterCond2).inventoryItem = 0x80;
			}

			if (!getCharacterCurrentParams(kCharacterCond2)[0]) {
				startCycOtis(kCharacterCond2, "667H");
			} else {
				startCycOtis(kCharacterCond2, "667I");
			}

			break;
		case 3:
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);

			if (!getCharacterCurrentParams(kCharacterCond2)[1] && !getCharacterCurrentParams(kCharacterCond2)[2]) {
				if (!getCharacterCurrentParams(kCharacterCond2)[3]) {
					getCharacterCurrentParams(kCharacterCond2)[3] = _realTime + 75;
					if (_realTime == -75) {
						getCharacter(kCharacterCond2).inventoryItem = 0;
						getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
						Cond2Call(&LogicManager::CONS_Cond2_SitDownFast, 1, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterCond2)[3] < _realTime) {
					getCharacterCurrentParams(kCharacterCond2)[3] = 0x7FFFFFFF;

					getCharacter(kCharacterCond2).inventoryItem = 0;
					getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
					Cond2Call(&LogicManager::CONS_Cond2_SitDownFast, 1, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterCond2)[4] ||
				(getCharacterCurrentParams(kCharacterCond2)[4] = _realTime + 225, _realTime != -225)) {
				if (getCharacterCurrentParams(kCharacterCond2)[4] >= _realTime) {
					break;
				}

				getCharacterCurrentParams(kCharacterCond2)[4] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_SitDownFast, 1, 0, 0, 0);
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		case 9:
			playNIS(kEventCoudertAskTylerCompartment);
			bumpCath(kCarRedSleeping, 25, 255);
			break;
		default:
			break;
		}

		break;
	case 201439712:
		startCycOtis(kCharacterCond2, "627K");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_Listen(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_Listen);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 14;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_Listen(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 && nearChar(kCharacterCond2, kCharacterCath, 1000) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, 123, 0, 0);
		}

		break;
	case 12:
		if (getCharacterParams(kCharacterCond2, 8)[16]) {
			getCharacterParams(kCharacterCond2, 8)[16] = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 1500, 0, 0);
		} else {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_DoWait, 15, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			send(kCharacterCond2, getCharacterCurrentParams(kCharacterCond2)[0], 202558662, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_GetUpListen, 0, 0, 0, 0);
			break;
		case 2:
			send(kCharacterCond2, getCharacterCurrentParams(kCharacterCond2)[0], 155853632, 0);
			startCycOtis(kCharacterCond2, "627K");
			break;
		case 3:
			send(kCharacterCond2, getCharacterCurrentParams(kCharacterCond2)[0], 202558662, 0);
			send(kCharacterCond2, getCharacterCurrentParams(kCharacterCond2)[0], 155853632, 0);
			startCycOtis(kCharacterCond2, "627K");
			takeItem(kItem5);
			break;
		case 4:
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);
			break;
		case 5:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 125499160:
		if (getCharacterCurrentParams(kCharacterCond2)[0] == kCharacterTrainM) {
			getCharacterParams(kCharacterCond2, 8)[2] = 0;
		} else if (getCharacterCurrentParams(kCharacterCond2)[0] == kCharacterCond1) {
			getCharacterParams(kCharacterCond2, 8)[4] = 0;
		} else if (getCharacterCurrentParams(kCharacterCond2)[0] == kCharacterMadame) {
			getCharacterParams(kCharacterCond2, 8)[3] = 0;
		}

		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
		Cond2Call(&LogicManager::CONS_Cond2_SitDownFast, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_TatianaLockUnlockMyComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_TatianaLockUnlockMyComp);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 15;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_TatianaLockUnlockMyComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacterParams(kCharacterCond2, 8)[7] = 0;
		getCharacterParams(kCharacterCond2, 8)[8] = 0;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 7500, 0, 0);
			break;
		case 2:
			if (getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall].parameters[0]) {
				playDialog(kCharacterCond2, "Tat3163", -1, 0);
			} else if (_globals[kGlobalChapter] == 3 && _gameTime < 1449000) {
				playDialog(kCharacterCond2, "Tat3161A", -1, 0);
			} else {
				playDialog(kCharacterCond2, "Tat3162A", -1, 0);
			}

			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Xb", 33, 0, 0);
			break;
		case 3:
			send(kCharacterCond2, kCharacterTatiana, 69239528, 0);
			getCharacter(kCharacterCond2).characterPosition.position = 7250;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_GetUp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_GetUp);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 16;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_GetUp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterParams(kCharacterCond2, 8)[16]) {
			getCharacterParams(kCharacterCond2, 8)[16] = 0;
			dropItem(kItem5, 1);

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		} else if (getCharacterParams(kCharacterCond2, 8)[1]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_DoSeqOtis, "627C", 0, 0, 0);
		} else {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoSeqOtis, "627F", 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
			if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] <= 2) {
				dropItem(kItem5, 1);
				if (!checkCathDir(kCarRedSleeping, 2))
					getCharacter(kCharacterCond2).characterPosition.position = 2088;

				getCharacter(kCharacterCond2).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
				fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_GetUpListen(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_GetUpListen);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 17;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_GetUpListen(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		takeItem(kItem5);
		if (getCharacterParams(kCharacterCond2, 8)[16]) {
			getCharacterParams(kCharacterCond2, 8)[16] = 0;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		} else if (getCharacterCurrentParams(kCharacterCond2)[0]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_DoSeqOtis, "627H", 0, 0, 0);
		} else if (getCharacterParams(kCharacterCond2, 8)[1]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoSeqOtis, "627C", 0, 0, 0);
		} else {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoSeqOtis, "627F", 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] && getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] <= 3) {
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_SitDown(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_SitDown);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 18;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_SitDown(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterParams(kCharacterCond2, 8)[5] || getCharacterParams(kCharacterCond2, 8)[7] || getCharacterParams(kCharacterCond2, 8)[8] || getCharacterParams(kCharacterCond2, 8)[10] || getCharacterParams(kCharacterCond2, 8)[12] || getCharacterParams(kCharacterCond2, 8)[13] || getCharacterParams(kCharacterCond2, 8)[15] || getCharacterParams(kCharacterCond2, 8)[19] || getCharacterParams(kCharacterCond2, 8)[14] || getCharacterParams(kCharacterCond2, 8)[9] || getCharacterParams(kCharacterCond2, 8)[21]) {
			dropItem(kItem5, 1);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 540, 0, 0);
		} else if (getCharacterParams(kCharacterCond2, 8)[2] || getCharacterParams(kCharacterCond2, 8)[4] || getCharacterParams(kCharacterCond2, 8)[3]) {
			startCycOtis(kCharacterCond2, "627K");
			takeItem(kItem5);
			getCharacterParams(kCharacterCond2, 8)[16] = 1;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		} else {
			if (getCharacterParams(kCharacterCond2, 8)[1])
				startSeqOtis(kCharacterCond2, "627A");
			else
				startSeqOtis(kCharacterCond2, "627D");

			takeItem(kItem5);

			if (checkCathDir(kCarRedSleeping, 68)) {
				if (!whoRunningDialog(kCharacterCond2))
					playDialog(kCharacterCond2, "JAC1111", -1, 0);

				bumpCath(kCarRedSleeping, 25, 255);
			}

			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_FinishSeqOtis, 0, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			endGraphics(kCharacterCond2);
			getCharacterParams(kCharacterCond2, 8)[16] = 1;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoWait, 75, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		case 3:
			if (getCharacterParams(kCharacterCond2, 8)[1]) {
				startCycOtis(kCharacterCond2, "627B");
			} else {
				startCycOtis(kCharacterCond2, "627E");
			}

			getCharacterParams(kCharacterCond2, 8)[0] = 0;
			send(kCharacterCond2, kCharacterCond2, 17, 0);

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_SitDownFast(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_SitDownFast);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 19;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_SitDownFast(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterParams(kCharacterCond2, 8)[5] || getCharacterParams(kCharacterCond2, 8)[7] || getCharacterParams(kCharacterCond2, 8)[8] || getCharacterParams(kCharacterCond2, 8)[10] || getCharacterParams(kCharacterCond2, 8)[12] || getCharacterParams(kCharacterCond2, 8)[13] || getCharacterParams(kCharacterCond2, 8)[15] || getCharacterParams(kCharacterCond2, 8)[19] || getCharacterParams(kCharacterCond2, 8)[14] || getCharacterParams(kCharacterCond2, 8)[9] || getCharacterParams(kCharacterCond2, 8)[21]) {
			dropItem(kItem5, 1);
			getCharacterParams(kCharacterCond2, 8)[16] = 1;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		} else if (getCharacterParams(kCharacterCond2, 8)[2] || getCharacterParams(kCharacterCond2, 8)[4] || getCharacterParams(kCharacterCond2, 8)[3]) {
			takeItem(kItem5);
			getCharacterParams(kCharacterCond2, 8)[16] = 1;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		} else {
			if (getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall].parameters[0]) {
				startSeqOtis(kCharacterCond2, "697H");
			} else if (getCharacterParams(kCharacterCond2, 8)[1]) {
				startSeqOtis(kCharacterCond2, "627A");
			} else {
				startSeqOtis(kCharacterCond2, "627D");
			}

			takeItem(kItem5);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_FinishSeqOtis, 0, 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			if (getCharacterParams(kCharacterCond2, 8)[1]) {
				startCycOtis(kCharacterCond2, "627B");
			} else {
				startCycOtis(kCharacterCond2, "627E");
			}

			getCharacterParams(kCharacterCond2, 8)[0] = 0;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_MakeBed(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_MakeBed);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 20;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_MakeBed(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		bool skip = false; // Horrible way to unroll a goto...

		if (!getCharacterCurrentParams(kCharacterCond2)[10]) {
			getCharacterCurrentParams(kCharacterCond2)[10] = _gameTime + 300;
			if (_gameTime == -300) {
				skip = true;
				playDialog(0, "ZFX1004", getVolume(kCharacterCond2), 0);
			}
		}

		if (!skip && getCharacterCurrentParams(kCharacterCond2)[10] < _gameTime) {
			getCharacterCurrentParams(kCharacterCond2)[10] = 0x7FFFFFFF;
			playDialog(0, "ZFX1004", getVolume(kCharacterCond2), 0);
		}

		if (getCharacterCurrentParams(kCharacterCond2)[11] ||
			(getCharacterCurrentParams(kCharacterCond2)[11] = _gameTime + 900, _gameTime != -900)) {
			if (getCharacterCurrentParams(kCharacterCond2)[11] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterCond2)[11] = 0x7FFFFFFF;
		}

		setModel(getCharacterCurrentParams(kCharacterCond2)[0], 1);

		if (getCharacterCurrentParams(kCharacterCond2)[3] != 2)
			setDoor(getCharacterCurrentParams(kCharacterCond2)[0], getCharacterCurrentParams(kCharacterCond2)[2], getCharacterCurrentParams(kCharacterCond2)[3], getCharacterCurrentParams(kCharacterCond2)[4], getCharacterCurrentParams(kCharacterCond2)[5]);

		if (getCharacterCurrentParams(kCharacterCond2)[1])
			setDoor(getCharacterCurrentParams(kCharacterCond2)[1], getCharacterCurrentParams(kCharacterCond2)[6], getCharacterCurrentParams(kCharacterCond2)[7], getCharacterCurrentParams(kCharacterCond2)[8], getCharacterCurrentParams(kCharacterCond2)[9]);

		getCharacter(kCharacterCond2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
		fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		break;
	}
	case 8:
	case 9:
		setDoor(getCharacterCurrentParams(kCharacterCond2)[0], 4, 1, 0, 0);

		if (getCharacterCurrentParams(kCharacterCond2)[1])
			setDoor(getCharacterCurrentParams(kCharacterCond2)[1], 4, 1, 0, 0);

		if (msg->action == 8) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "LIB012", 0, 0, 0);
		} else {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "LIB013", 0, 0, 0);
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterCond2)[2] = _doors[getCharacterCurrentParams(kCharacterCond2)[0]].who;
		getCharacterCurrentParams(kCharacterCond2)[3] = _doors[getCharacterCurrentParams(kCharacterCond2)[0]].status;
		getCharacterCurrentParams(kCharacterCond2)[4] = _doors[getCharacterCurrentParams(kCharacterCond2)[0]].windowCursor;
		getCharacterCurrentParams(kCharacterCond2)[5] = _doors[getCharacterCurrentParams(kCharacterCond2)[0]].handleCursor;

		if (getCharacterCurrentParams(kCharacterCond2)[1]) {
			getCharacterCurrentParams(kCharacterCond2)[6] = _doors[getCharacterCurrentParams(kCharacterCond2)[1]].who;
			getCharacterCurrentParams(kCharacterCond2)[7] = _doors[getCharacterCurrentParams(kCharacterCond2)[1]].status;
			getCharacterCurrentParams(kCharacterCond2)[8] = _doors[getCharacterCurrentParams(kCharacterCond2)[1]].windowCursor;
			getCharacterCurrentParams(kCharacterCond2)[9] = _doors[getCharacterCurrentParams(kCharacterCond2)[1]].handleCursor;
			setDoor(getCharacterCurrentParams(kCharacterCond2)[1], 4, 1, 10, 9);
		}

		if (getCharacterCurrentParams(kCharacterCond2)[3] != 2)
			setDoor(getCharacterCurrentParams(kCharacterCond2)[0], 4, 1, 10, 9);

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
		case 2:
			if (getCharacterCurrentParams(kCharacterCond2)[0] == 32 ||
				getCharacterCurrentParams(kCharacterCond2)[0] == 34 ||
				getCharacterCurrentParams(kCharacterCond2)[0] == 38 ||
				getCharacterCurrentParams(kCharacterCond2)[0] == 39) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
				Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Jac1001B", 0, 0, 0);
			} else {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
				Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Jac1001A", 0, 0, 0);
			}
			break;
		case 3:
		case 4:
			setDoor(getCharacterCurrentParams(kCharacterCond2)[0], 4, 1, 10, 9);
			if (getCharacterCurrentParams(kCharacterCond2)[1])
				setDoor(getCharacterCurrentParams(kCharacterCond2)[1], 4, 1, 10, 9);

			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_MakeBedIvo(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_MakeBedIvo);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 21;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_MakeBedIvo(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterCond2)[0]) {
			if (getCharacterCurrentParams(kCharacterCond2)[1] ||
				(getCharacterCurrentParams(kCharacterCond2)[1] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterCond2)[1] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterCond2)[1] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Zh", 39, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2740, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Vh", 39, 0, 0);
			break;
		case 2:
			send(kCharacterCond2, kCharacterIvo, 221683008, 0);
			startCycOtis(kCharacterCond2, "627Wh");
			softBlockAtDoor(kCharacterCond2, 39);
			break;
		case 3:
			softReleaseAtDoor(kCharacterCond2, 39);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBed, 39, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "697Ah", 39, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).characterPosition.location = 0;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		case 6:
			send(kCharacterCond2, kCharacterIvo, 122865568, 0);
			break;
		case 7:
			softReleaseAtDoor(kCharacterCond2, 39);
			setDoor(39, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBed, 39, 0, 0, 0);
			break;
		case 8:
			playDialog(kCharacterCond2, "JAC1013A", -1, 0);
			setDoor(39, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "667Uh", 39, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond2).characterPosition.location = 0;
			send(kCharacterCond2, kCharacterIvo, 123852928, 0);

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 88652208:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
		Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "667Th", 39, 0, 0);
		break;
	case 123199584:
		getCharacterCurrentParams(kCharacterCond2)[0] = 1;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
		Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "JAC1012", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_MakeBedMilos(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_MakeBedMilos);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 22;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_MakeBedMilos(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterCond2)[0]) {
			if (getCharacterCurrentParams(kCharacterCond2)[1] ||
				(getCharacterCurrentParams(kCharacterCond2)[1] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterCond2)[1] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterCond2)[1] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Rg", 38, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 3050, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Mg", 38, 0, 0);
			break;
		case 2:
			send(kCharacterCond2, kCharacterMilos, 221683008, 0);
			startCycOtis(kCharacterCond2, "627Ng");
			softBlockAtDoor(kCharacterCond2, 38);
			break;
		case 3:
			softReleaseAtDoor(kCharacterCond2, 38);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBed, 38, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Sg", 38, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).characterPosition.location = 0;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		case 6:
			send(kCharacterCond2, kCharacterMilos, 122865568, 0);
			break;
		case 7:
			softReleaseAtDoor(kCharacterCond2, 38);
			setDoor(38, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBed, 38, 0, 0, 0);
			break;
		case 8:
			playDialog(kCharacterCond2, "JAC1030A", -1, 0);
			setDoor(38, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Ug", 38, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond2).characterPosition.location = 0;
			send(kCharacterCond2, kCharacterMilos, 123852928, 0);

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 88652208:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
		Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Tg", 38, 0, 0);
		break;
	case 123199584:
		getCharacterCurrentParams(kCharacterCond2)[0] = 1;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
		Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "JAC1030", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_TryMakeAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_TryMakeAnna);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 23;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_TryMakeAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 4070, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Vf", 37, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond2, "627Wf");
			softBlockAtDoor(kCharacterCond2, 37);
			send(kCharacterCond2, kCharacterMax, 158007856, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoWait, 150, 0, 0, 0);
			break;
		case 3:
			softReleaseAtDoor(kCharacterCond2, 37);

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_MakeBedAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_MakeBedAnna);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 24;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_MakeBedAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 4070, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Vf", 37, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond2, "627Wf");
			softBlockAtDoor(kCharacterCond2, 37);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoWait, 150, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoEnterCorrOtis, "627Zf", 37, 4070, 4455);
			break;
		case 4:
			softReleaseAtDoor(kCharacterCond2, 37);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBed, 37, 53, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "697Af", 37, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond2).characterPosition.location = 0;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_MakeBedRebecca(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_MakeBedRebecca);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 25;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_MakeBedRebecca(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 4840, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Me", 36, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond2, "627Ne");
			softBlockAtDoor(kCharacterCond2, 36);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoWait, 45, 0, 0, 0);
			break;
		case 3:
			send(kCharacterCond2, kCharacterRebecca, 254915200, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoWait, 450, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoEnterCorrOtis, "627Re", 36, 4840, 4455);
			break;
		case 5:
			softReleaseAtDoor(kCharacterCond2, 36);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBed, 36, 52, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Se", 0x24, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond2).characterPosition.location = 0;
			send(kCharacterCond2, kCharacterRebecca, 123852928, 0);

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_MakeBedMadame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_MakeBedMadame);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 26;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_MakeBedMadame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterCond2)[0]) {
			if (getCharacterCurrentParams(kCharacterCond2)[1] ||
				(getCharacterCurrentParams(kCharacterCond2)[1] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterCond2)[1] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterCond2)[1] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoEnterCorrOtis, "627Zd", 35, 5790, 6130);
		}

		break;
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 5790, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Vd", 0x23, 0, 0);
			break;
		case 2:
			send(kCharacterCond2, kCharacterMadame, 221683008, 0);
			startCycOtis(kCharacterCond2, "627Wd");
			softBlockAtDoor(kCharacterCond2, 35);
			break;
		case 3:
			softReleaseAtDoor(kCharacterCond2, 35);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBed, 35, 51, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "697Ad", 35, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).characterPosition.location = 0;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		case 6:
			send(kCharacterCond2, kCharacterMadame, 122865568, 0);
			break;
		case 7:
			softReleaseAtDoor(kCharacterCond2, 35);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBed, 35, 51, 0, 0);
			break;
		case 8:
			playDialog(kCharacterCond2, "JAC1013", -1, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "697Ad", 35, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond2).characterPosition.location = 0;
			send(kCharacterCond2, kCharacterMadame, 123852928, 0);

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 88652208:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
		Cond2Call(&LogicManager::CONS_Cond2_DoEnterCorrOtis, "627Zd", 35, 5790, 6130);
		break;
	case 123199584:
		getCharacterCurrentParams(kCharacterCond2)[0] = 1;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
		Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "JAC1012", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_MakeBedMonsieur(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_MakeBedMonsieur);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 27;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_MakeBedMonsieur(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterCond2)[0]) {
			if (getCharacterCurrentParams(kCharacterCond2)[1] ||
				(getCharacterCurrentParams(kCharacterCond2)[1] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterCond2)[1] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterCond2)[1] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoEnterCorrOtis, "627Rc", 34, 6470, 6130);
		}

		break;
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 6470, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Mc", 34, 0, 0);
			break;
		case 2:
			send(kCharacterCond2, kCharacterMonsieur, 221683008, 0);
			startCycOtis(kCharacterCond2, "627Nc");
			softBlockAtDoor(kCharacterCond2, 34);
			break;
		case 3:
			softReleaseAtDoor(kCharacterCond2, 34);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBed, 34, 50, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Sc", 34, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).characterPosition.location = 0;
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		case 6:
			send(kCharacterCond2, kCharacterMonsieur, 122865568, 0);
			break;
		case 7:
			softReleaseAtDoor(kCharacterCond2, 34);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBed, 34, 50, 0, 0);
			break;
		case 8:
			playDialog(kCharacterCond2, "JAC1013", -1, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Uc", 34, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond2).characterPosition.location = 0;
			send(kCharacterCond2, kCharacterMonsieur, 123852928, 0);

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 88652208:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
		Cond2Call(&LogicManager::CONS_Cond2_DoEnterCorrOtis, "627Rc", 34, 6470, 6130);
		break;
	case 123199584:
		getCharacterCurrentParams(kCharacterCond2)[0] = 1;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
		Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "JAC1012", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_MakeBedTatiana(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_MakeBedTatiana);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 28;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_MakeBedTatiana(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 7500, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Vb", 33, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond2, "627Wb");
			softBlockAtDoor(kCharacterCond2, 33);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoWait, 150, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoEnterCorrOtis, "627Zb", 33, 7500, 7850);
			break;
		case 4:
			softReleaseAtDoor(kCharacterCond2, 33);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBed, 33, 49, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "697Ab", 33, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond2).characterPosition.location = 0;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_MakeBedVassili(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_MakeBedVassili);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 29;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_MakeBedVassili(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 8200, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Ma", 32, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond2, "627Na");
			softBlockAtDoor(kCharacterCond2, 32);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoWait, 150, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoEnterCorrOtis, "627Ra", 32, 8200, 7850);
			break;
		case 4:
			softReleaseAtDoor(kCharacterCond2, 32);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBed, 32, 48, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Sa", 32, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond2).characterPosition.location = 0;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_CathBuzzing(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_CathBuzzing);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 30;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_CathBuzzing(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		switch (getCharacterCurrentParams(kCharacterCond2)[0]) {
		case 32:
			getCharacterCurrentParams(kCharacterCond2)[1] = 8200;
			getCharacterCurrentParams(kCharacterCond2)[2] = 7850;

			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterCond2)[5], 12, "627Ma");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterCond2)[8], 12, "627Na");
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
			break;
		case 33:
			getCharacterCurrentParams(kCharacterCond2)[1] = 7500;
			getCharacterCurrentParams(kCharacterCond2)[2] = 7850;
			getCharacterCurrentParams(kCharacterCond2)[3] = 1;

			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterCond2)[5], 12, "627Vb");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterCond2)[8], 12, "627Wb");
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
			break;
		case 34:
			getCharacterCurrentParams(kCharacterCond2)[1] = 6470;
			getCharacterCurrentParams(kCharacterCond2)[2] = 6130;

			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterCond2)[5], 12, "627Mc");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterCond2)[8], 12, "627Nc");
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
			break;
		case 35:
			getCharacterCurrentParams(kCharacterCond2)[1] = 5790;
			getCharacterCurrentParams(kCharacterCond2)[2] = 6130;
			getCharacterCurrentParams(kCharacterCond2)[3] = 1;

			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterCond2)[5], 12, "627Vd");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterCond2)[8], 12, "627Wd");
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
			break;
		case 36:
			getCharacterCurrentParams(kCharacterCond2)[1] = 4840;
			getCharacterCurrentParams(kCharacterCond2)[2] = 4455;
			getCharacterCurrentParams(kCharacterCond2)[3] = 1;

			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterCond2)[5], 12, "627Me");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterCond2)[8], 12, "627Ne");
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
			break;
		case 37:
			getCharacterCurrentParams(kCharacterCond2)[1] = 4070;
			getCharacterCurrentParams(kCharacterCond2)[2] = 4455;
			getCharacterCurrentParams(kCharacterCond2)[3] = 1;

			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterCond2)[5], 12, "627Vf");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterCond2)[8], 12, "627Wf");
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
			break;
		default:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, getCharacterCurrentParams(kCharacterCond2)[1], 0, 0);
			break;
		case 2:
			if (inSuite(kCharacterCath, kCarRedSleeping, getCharacterCurrentParams(kCharacterCond2)[2]) ||
				((getCharacterCurrentParams(kCharacterCond2)[0] == 36 || getCharacterCurrentParams(kCharacterCond2)[0] == 37) &&
				cathOutRebeccaWindow())) {
				setDoor(getCharacterCurrentParams(kCharacterCond2)[0], 0, checkDoor(getCharacterCurrentParams(kCharacterCond2)[0]), 0, 0);
				getCharacterCurrentParams(kCharacterCond2)[4] = 1;
			}

			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, (char *)&getCharacterCurrentParams(kCharacterCond2)[5], getCharacterCurrentParams(kCharacterCond2)[0], 0, 0);
			break;
		case 3:
			startCycOtis(kCharacterCond2, (char *)&getCharacterCurrentParams(kCharacterCond2)[8]);
			softBlockAtDoor(kCharacterCond2, getCharacterCurrentParams(kCharacterCond2)[0]);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;

			if (!getCharacterCurrentParams(kCharacterCond2)[3]) {
				Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "JAC3021", 0, 0, 0);
			} else {
				Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "JAC3020", 0, 0, 0);
			}

			break;
		case 4:
			if (getCharacterCurrentParams(kCharacterCond2)[4]) {
				setDoor(getCharacterCurrentParams(kCharacterCond2)[0], 0, checkDoor(getCharacterCurrentParams(kCharacterCond2)[0]), 10, 9);
			}

			softReleaseAtDoor(kCharacterCond2, getCharacterCurrentParams(kCharacterCond2)[0]);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 0x7D0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_CathRattling(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_CathRattling);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 31;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_CathRattling(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
		Cond2Call(&LogicManager::CONS_Cond2_SitDownFast, 1, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoSeqOtis, "627G", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			if (whoRunningDialog(kCharacterCond2)) {
				startCycOtis(kCharacterCond2, "627K");
			} else {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
				Cond2Call(&LogicManager::CONS_Cond2_SitDownFast, 1, 0, 0, 0);
			}
		} else if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 2 ||
				   getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 3) {
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_BathroomTrip(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_BathroomTrip);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 32;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_BathroomTrip(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 9460, 0, 0);
			break;
		case 2:
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoWait, 900, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_DoPending(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_DoPending);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 33;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_DoPending(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterParams(kCharacterCond2, 8)[5] || getCharacterParams(kCharacterCond2, 8)[6] || getCharacterParams(kCharacterCond2, 8)[2] || getCharacterParams(kCharacterCond2, 8)[4] || getCharacterParams(kCharacterCond2, 8)[3] || getCharacterParams(kCharacterCond2, 8)[17] || getCharacterParams(kCharacterCond2, 8)[9] || getCharacterParams(kCharacterCond2, 8)[14]) {
			getCharacterParams(kCharacterCond2, 8)[21] = 1;
			if (getCharacterParams(kCharacterCond2, 8)[2] || getCharacterParams(kCharacterCond2, 8)[4] || getCharacterParams(kCharacterCond2, 8)[3]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
				Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 1500, 0, 0);
			} else {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
				Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 540, 0, 0);
			}
		} else {
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacterParams(kCharacterCond2, 8)[16] = 1;
			if (getCharacterParams(kCharacterCond2, 8)[2]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
				Cond2Call(&LogicManager::CONS_Cond2_Listen, 9, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacterParams(kCharacterCond2, 8)[4]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
				Cond2Call(&LogicManager::CONS_Cond2_Listen, 3, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (getCharacterParams(kCharacterCond2, 8)[3]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
				Cond2Call(&LogicManager::CONS_Cond2_Listen, 21, 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			getCharacterParams(kCharacterCond2, 8)[21] = 0;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		case 5:
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_DoWait, 75, 0, 0, 0);
			break;
		case 6:
			if (getCharacterParams(kCharacterCond2, 8)[5] || getCharacterParams(kCharacterCond2, 8)[6]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
				Cond2Call(&LogicManager::CONS_Cond2_RespondVassili, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 7:
			if (getCharacterParams(kCharacterCond2, 8)[17]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
				Cond2Call(&LogicManager::CONS_Cond2_AfterPolice, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 8:
			if (getCharacterParams(kCharacterCond2, 8)[9]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
				Cond2Call(&LogicManager::CONS_Cond2_ServiceAnna, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 9:
			if (getCharacterParams(kCharacterCond2, 8)[14]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
				Cond2Call(&LogicManager::CONS_Cond2_TakeMaxBack, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 10:
			getCharacterParams(kCharacterCond2, 8)[21] = 0;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_TakeMaxBack(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_TakeMaxBack);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 34;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_TakeMaxBack(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 4070, 0, 0);
			break;
		case 2:
			if (!getCharacterCurrentParams(kCharacterCond2)[0]) {
				playDialog(kCharacterCond2, "Ann3124", -1, 0);
				getCharacterParams(kCharacterCond2, 8)[14] = 0;
				getCharacterParams(kCharacterCond2, 8)[11] = 0;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
				Cond2Call(&LogicManager::CONS_Cond2_ReturnMax, getCharacterCurrentParams(kCharacterCond2)[0], 0, 0, 0);
			} else {
				startCycOtis(kCharacterCond2, "627Vf");
				softBlockAtDoor(kCharacterCond2, 37);
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
				Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "LIB012", 0, 0, 0);
			}

			break;
		case 3:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Jac1001", 0, 0, 0);
			break;
		case 4:
			playDialog(kCharacterCond2, "Ann3125", -1, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "629Bf", 37, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "629Ff", 37, 0, 0);
			break;
		case 6:
			softReleaseAtDoor(kCharacterCond2, 37);
			getCharacterParams(kCharacterCond2, 8)[14] = 0;
			getCharacterParams(kCharacterCond2, 8)[11] = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_ReturnMax, getCharacterCurrentParams(kCharacterCond2)[0], 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_ReturnMax(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_ReturnMax);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 35;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_ReturnMax(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (checkLoc(kCharacterCath, kCarBaggage)) {
			playNIS(kEventCoudertBaggageCar);
			playDialog(0, "BUMP", -1, 0);
			bumpCath(kCarRestaurant, 65, 255);
		}

		if (getCharacterCurrentParams(kCharacterCond2)[1] ||
			(getCharacterCurrentParams(kCharacterCond2)[1] = _gameTime + 2700, _gameTime != -2700)) {
			if (getCharacterCurrentParams(kCharacterCond2)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterCond2)[1] = 0x7FFFFFFF;
		}

		send(kCharacterCond2, kCharacterMax, 135204609, 0);
		getCharacter(kCharacterCond2).clothes = 0;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
		break;
	case 12:
		if (getCharacterCurrentParams(kCharacterCond2)[0])
			send(kCharacterCond2, kCharacterAnna, 156049968, 0);

		send(kCharacterCond2, kCharacterMax, 122358304, 0);
		getCharacter(kCharacterCond2).clothes = 1;
		getCharacter(kCharacterCond2).characterPosition.position = 4370;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 8200, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			if (!whoRunningDialog(kCharacterCond2))
				playDialog(kCharacterCond2, "Ann3124", -1, 0);

			if (getCharacterCurrentParams(kCharacterCond2)[0])
				send(kCharacterCond2, kCharacterAnna, 123733488, 0);

			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 9460, 0, 0);
			break;
		case 2:
			endGraphics(kCharacterCond2);
			break;
		case 3:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_Birth);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 36;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterCond2)[0]) {
			getCharacterCurrentParams(kCharacterCond2)[0] = 1;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
		}

		break;
	case 12:
		autoMessage(kCharacterCond2, 292048641, 7);
		autoMessage(kCharacterCond2, 326348944, 8);
		autoMessage(kCharacterCond2, 171394341, 2);
		autoMessage(kCharacterCond2, 154005632, 4);
		autoMessage(kCharacterCond2, 169557824, 3);
		autoMessage(kCharacterCond2, 226031488, 5);
		autoMessage(kCharacterCond2, 339669520, 6);
		autoMessage(kCharacterCond2, 189750912, 10);
		autoMessage(kCharacterCond2, 185737168, 12);
		autoMessage(kCharacterCond2, 185671840, 13);
		autoMessage(kCharacterCond2, 205033696, 15);
		autoMessage(kCharacterCond2, 157026693, 14);
		autoMessage(kCharacterCond2, 189026624, 11);
		autoMessage(kCharacterCond2, 168254872, 17);
		autoMessage(kCharacterCond2, 201431954, 18);
		autoMessage(kCharacterCond2, 188570113, 19);
		getCharacter(kCharacterCond2).characterPosition.position = 1500;
		getCharacter(kCharacterCond2).characterPosition.location = 0;
		getCharacter(kCharacterCond2).characterPosition.car = kCarRedSleeping;
		getCharacterParams(kCharacterCond2, 8)[1] = 1;
		getCharacterParams(kCharacterCond2, 8)[0] = 0;
		setModel(111, 1);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			CONS_Cond2_Sitting(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_RespondVassili(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_RespondVassili);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 37;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_RespondVassili(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (whoRunningDialog(kCharacterCond2))
			fadeDialog(kCharacterCond2);

		if (getCharacterParams(kCharacterCond2, 8)[6]) {
			getCharacter(kCharacterCond2).characterPosition.position = 8200;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoEnterCorrOtis, "698Ha", 32, 8200, 7850);
		} else {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 5790, 0, 0);
			break;
		case 2:
			send(kCharacterCond2, kCharacterAnna, 238358920, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 8200, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoEnterCorrOtis, "698Ha", 32, 8200, 7850);
			break;
		case 4:
			setDoor(32, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			CONS_Cond2_AtSeizure(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_AtSeizure(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_AtSeizure);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 38;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_AtSeizure(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterCond2).characterPosition.position = 8200;
		getCharacter(kCharacterCond2).characterPosition.location = 1;
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			getCharacterParams(kCharacterCond2, 8)[5] = 0;
			getCharacterParams(kCharacterCond2, 8)[6] = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 2) {
			CONS_Cond2_Sitting(0, 0, 0, 0);
		}

		break;
	case 191477936:
		getCharacter(kCharacterCond2).characterPosition.location = 0;
		getCharacter(kCharacterCond2).characterPosition.position = 4070;
		setDoor(32, kCharacterCath, 0, 10, 9);
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_AfterPolice(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_AfterPolice);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 39;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_AfterPolice(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "LIB070", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 5790, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Vd", 35, 0, 0);
			break;
		case 4:
			startCycOtis(kCharacterCond2, "627Wd");
			softBlockAtDoor(kCharacterCond2, 35);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "MME1151A", 0, 0, 0);
			break;
		case 5:
			softReleaseAtDoor(kCharacterCond2, 35);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Zd", 35, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "MME1151", 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "697Ad", 35, 0, 0);
			break;
		case 8:
			send(kCharacterCond2, kCharacterMadame, 223068211, 0);
			getCharacter(kCharacterCond2).characterPosition.location = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 10:
			send(kCharacterCond2, kCharacterTrainM, 167854368, 0);
			getCharacterParams(kCharacterCond2, 8)[17] = 0;

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_Sitting(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_Sitting);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 40;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_Sitting(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		if (getCharacterParams(kCharacterCond2, 8)[18]) {
			getCharacterParams(kCharacterCond2, 8)[17] = 0;
			getCharacterParams(kCharacterCond2, 8)[2] = 0;
			getCharacterParams(kCharacterCond2, 8)[4] = 0;
			getCharacterParams(kCharacterCond2, 8)[3] = 0;
			getCharacterParams(kCharacterCond2, 8)[16] = 0;
			getCharacterParams(kCharacterCond2, 8)[0] = 1;
			getCharacterParams(kCharacterCond2, 8)[7] = 0;
			getCharacterParams(kCharacterCond2, 8)[8] = 0;
			startCycOtis(kCharacterCond2, "697F");
			getCharacterCurrentParams(kCharacterCond2)[0] = 1;
			getCharacterCurrentParams(kCharacterCond2)[1] = 1;
			getCharacterParams(kCharacterCond2, 8)[18] = 0;
		}

		if (_globals[kGlobalFoundCorpse] || _doneNIS[kEventCoudertAskTylerCompartment]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
		} else {
			getCharacter(kCharacterCond2).inventoryItem = 0x80;
		}

		if (getCharacterParams(kCharacterCond2, 8)[7]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_TatianaLockUnlockMyComp, 1, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[8]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_TatianaLockUnlockMyComp, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[5] || getCharacterParams(kCharacterCond2, 8)[6]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			CONS_Cond2_RespondVassili(0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[2]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_Listen, 9, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[4]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_Listen, 3, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[3]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
			Cond2Call(&LogicManager::CONS_Cond2_Listen, 21, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[17]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_AfterPolice, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[0] && !whoRunningDialog(kCharacterCond2)) {
			if (rnd(2) == 0) {
				playDialog(kCharacterCond2, "JAC1065A", -1, 0);
			} else {
				playDialog(kCharacterCond2, "JAC1065", -1, 0);
			}
		}

		if (_gameTime > 1107000 && !getCharacterCurrentParams(kCharacterCond2)[0] && !_doneNIS[84]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
			Cond2Call(&LogicManager::CONS_Cond2_MakeAllBeds, 0, 0, 0, 0);
			break;
		}

		bool skip = false; // Horrible way to unroll a goto...

		if (_gameTime > 1189800 && !getCharacterParams(kCharacterCond2, 8)[0] && !getCharacterParams(kCharacterCond2, 8)[16]) {
			if (getCharacterCurrentParams(kCharacterCond2)[2] ||
				(getCharacterCurrentParams(kCharacterCond2)[2] = _gameTime + 2700, _gameTime != -2700)) {
				if (getCharacterCurrentParams(kCharacterCond2)[2] >= _gameTime) {
					skip = true;
				}

				if (!skip)
					getCharacterCurrentParams(kCharacterCond2)[2] = 0x7FFFFFFF;
			}

			if (!skip) {
				getCharacterParams(kCharacterCond2, 8)[1] = 0;
				getCharacterParams(kCharacterCond2, 8)[0] = 1;
				startCycOtis(kCharacterCond2, "697F");
				getCharacterCurrentParams(kCharacterCond2)[2] = 0;
			}
		}

		if (getCharacterParams(kCharacterCond2, 8)[1]) {
			if (_gameTime > 1107000 && !getCharacterCurrentParams(kCharacterCond2)[3]) {
				getCharacterCurrentParams(kCharacterCond2)[3] = 1;
				setModel(111, 2);
			}
			if (_gameTime > 1161000 && !getCharacterCurrentParams(kCharacterCond2)[4]) {
				getCharacterCurrentParams(kCharacterCond2)[4] = 1;
				setModel(111, 3);
			}
			if (_gameTime > 1206000 && !getCharacterCurrentParams(kCharacterCond2)[5]) {
				getCharacterCurrentParams(kCharacterCond2)[5] = 1;
				setModel(111, 4);
			}
		}

		break;
	}
	case 1:
		getCharacter(kCharacterCond2).inventoryItem = 0;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 11;
		Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventCoudertAskTylerCompartment, 0, 0);
		break;
	case 11:
		if (!getCharacterParams(kCharacterCond2, 8)[0] && !getCharacterParams(kCharacterCond2, 8)[16]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 13;
			Cond2Call(&LogicManager::CONS_Cond2_Passing, msg->param, msg->sender, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterCond2).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterCond2).characterPosition.position = 1500;
		getCharacter(kCharacterCond2).characterPosition.location = 0;
		takeItem(kItem5);
		break;
	case 17:
		if (!getCharacterParams(kCharacterCond2, 8)[16] &&
			!getCharacterParams(kCharacterCond2, 8)[0] &&
			(checkCathDir(kCarRedSleeping, 1) || checkCathDir(kCarRedSleeping, 23))) {
			if (_globals[kGlobalJacket] == 1) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
				Cond2Call(&LogicManager::CONS_Cond2_SaveGame, 2, kEventCoudertBloodJacket, 0, 0);
			} else {
				if (checkCathDir(kCarRedSleeping, 1))
					getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
				else
					getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;

				Cond2Call(&LogicManager::CONS_Cond2_Passing, 1, 0, 0, 0);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);
			break;
		case 4:
			if (getCharacterParams(kCharacterCond2, 8)[8]) {
				getCharacter(kCharacterCond2).inventoryItem = 0;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
				Cond2Call(&LogicManager::CONS_Cond2_TatianaLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (getCharacterParams(kCharacterCond2, 8)[5] || getCharacterParams(kCharacterCond2, 8)[6]) {
				getCharacter(kCharacterCond2).inventoryItem = 0;
				CONS_Cond2_RespondVassili(0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterCond2, 8)[2]) {
				getCharacter(kCharacterCond2).inventoryItem = 0;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
				Cond2Call(&LogicManager::CONS_Cond2_Listen, 9, 0, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (getCharacterParams(kCharacterCond2, 8)[4]) {
				getCharacter(kCharacterCond2).inventoryItem = 0;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
				Cond2Call(&LogicManager::CONS_Cond2_Listen, 3, 0, 0, 0);
				break;
			}

			// fall through
		case 7:
			if (getCharacterParams(kCharacterCond2, 8)[3]) {
				getCharacter(kCharacterCond2).inventoryItem = 0;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
				Cond2Call(&LogicManager::CONS_Cond2_Listen, 21, 0, 0, 0);
				break;
			}

			// fall through
		case 8:
			if (getCharacterParams(kCharacterCond2, 8)[17]) {
				getCharacter(kCharacterCond2).inventoryItem = 0;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
				Cond2Call(&LogicManager::CONS_Cond2_AfterPolice, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 9:
			if (getCharacterParams(kCharacterCond2, 8)[0] && !whoRunningDialog(kCharacterCond2)) {
				if (rnd(2) == 0) {
					playDialog(kCharacterCond2, "JAC1065A", -1, 0);
				} else {
					playDialog(kCharacterCond2, "JAC1065", -1, 0);
				}
			}

			if (_gameTime > 1107000 && !getCharacterCurrentParams(kCharacterCond2)[0] && !_doneNIS[84]) {
				getCharacter(kCharacterCond2).inventoryItem = 0;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
				Cond2Call(&LogicManager::CONS_Cond2_MakeAllBeds, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 10:
		{
			if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 10) {
				getCharacterCurrentParams(kCharacterCond2)[0] = 1;
			}

			bool skip = false; // Horrible way to unroll a goto...

			if (_gameTime > 1189800 && !getCharacterParams(kCharacterCond2, 8)[0] && !getCharacterParams(kCharacterCond2, 8)[16]) {
				if (getCharacterCurrentParams(kCharacterCond2)[2] ||
					(getCharacterCurrentParams(kCharacterCond2)[2] = _gameTime + 2700, _gameTime != -2700)) {
					if (getCharacterCurrentParams(kCharacterCond2)[2] >= _gameTime) {
						skip = true;
					}

					if (!skip)
						getCharacterCurrentParams(kCharacterCond2)[2] = 0x7FFFFFFF;
				}

				if (!skip) {
					getCharacterParams(kCharacterCond2, 8)[1] = 0;
					getCharacterParams(kCharacterCond2, 8)[0] = 1;
					startCycOtis(kCharacterCond2, "697F");
					getCharacterCurrentParams(kCharacterCond2)[2] = 0;
				}
			}

			if (getCharacterParams(kCharacterCond2, 8)[1]) {
				if (_gameTime > 1107000 && !getCharacterCurrentParams(kCharacterCond2)[3]) {
					getCharacterCurrentParams(kCharacterCond2)[3] = 1;
					setModel(111, 2);
				}
				if (_gameTime > 1161000 && !getCharacterCurrentParams(kCharacterCond2)[4]) {
					getCharacterCurrentParams(kCharacterCond2)[4] = 1;
					setModel(111, 3);
				}
				if (_gameTime > 1206000 && !getCharacterCurrentParams(kCharacterCond2)[5]) {
					getCharacterCurrentParams(kCharacterCond2)[5] = 1;
					setModel(111, 4);
				}
			}

			break;
		}
		case 11:
			playNIS(kEventCoudertAskTylerCompartment);

			if (getCharacterParams(kCharacterCond2, 8)[1])
				startSeqOtis(kCharacterCond2, "627A");
			else
				startSeqOtis(kCharacterCond2, "627D");

			takeItem(kItem5);
			getCharacterParams(kCharacterCond2, 8)[0] = 0;
			bumpCath(kCarRedSleeping, 25, 255);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 12;
			Cond2Call(&LogicManager::CONS_Cond2_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 12:
			if (getCharacterParams(kCharacterCond2, 8)[1])
				startCycOtis(kCharacterCond2, "627B");
			else
				startCycOtis(kCharacterCond2, "627E");

			break;
		case 14:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 15;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 168253822:
		if (!getCharacterParams(kCharacterCond2, 8)[16] && !getCharacterParams(kCharacterCond2, 8)[0]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			playDialog(kCharacterCond2, "JAC1120", -1, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 14;
			Cond2Call(&LogicManager::CONS_Cond2_DoSeqOtis, "697D", 0, 0, 0);
		}

		break;
	case 225932896:
		if (!getCharacterParams(kCharacterCond2, 8)[16] && !getCharacterParams(kCharacterCond2, 8)[0]) {
			send(kCharacterCond2, kCharacterFrancois, 205346192, 0);
		}

		break;
	case 225358684:
		if (!getCharacterParams(kCharacterCond2, 8)[0]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 16;
			Cond2Call(&LogicManager::CONS_Cond2_CathBuzzing, msg->param, 0, 0, 0);
		}

		break;
	case 305159806:
		if (!getCharacterParams(kCharacterCond2, 8)[16] && !getCharacterParams(kCharacterCond2, 8)[0]) {
			getCharacter(kCharacterCond2).inventoryItem = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 17;
			Cond2Call(&LogicManager::CONS_Cond2_CathRattling, msg->param, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_MakeAllBeds(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_MakeAllBeds);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 41;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_MakeAllBeds(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedVassili, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedTatiana, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedMonsieur, 0, 0, 0, 0);
			break;
		case 6:
			send(kCharacterCond2, kCharacterRebecca, 285528346, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedMadame, 0, 0, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedRebecca, 0, 0, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 11;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 11:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 12;
			Cond2Call(&LogicManager::CONS_Cond2_TryMakeAnna, 0, 0, 0, 0);
			break;
		case 12:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 13;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 13:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 14;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedMilos, 0, 0, 0, 0);
			break;
		case 14:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 15;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 15:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 16;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedIvo, 0, 0, 0, 0);
			break;
		case 16:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 17;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 0x7D0, 0, 0);
			break;
		case 17:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 18;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 18:
			send(kCharacterCond2, kCharacterMilos, 208228224, 0);

			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_StartPart2);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 42;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterCond2);
		getCharacter(kCharacterCond2).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterCond2).characterPosition.position = 1500;
		getCharacter(kCharacterCond2).characterPosition.location = 0;
		getCharacter(kCharacterCond2).inventoryItem = 0;
		getCharacterParams(kCharacterCond2, 8)[7] = 0;
		getCharacterParams(kCharacterCond2, 8)[8] = 0;
		getCharacterParams(kCharacterCond2, 8)[2] = 0;
		getCharacterParams(kCharacterCond2, 8)[4] = 0;
		getCharacterParams(kCharacterCond2, 8)[3] = 0;
		getCharacterParams(kCharacterCond2, 8)[5] = 0;
		getCharacterParams(kCharacterCond2, 8)[10] = 0;
		getCharacterParams(kCharacterCond2, 8)[12] = 0;
		getCharacterParams(kCharacterCond2, 8)[13] = 0;
		getCharacterParams(kCharacterCond2, 8)[15] = 0;
		getCharacterParams(kCharacterCond2, 8)[19] = 0;
		getCharacterParams(kCharacterCond2, 8)[14] = 0;
		getCharacterParams(kCharacterCond2, 8)[9] = 0;
		getCharacterParams(kCharacterCond2, 8)[1] = 0;
		setModel(111, 5);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			CONS_Cond2_SittingDay(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_SittingDay(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_SittingDay);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 43;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_SittingDay(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterParams(kCharacterCond2, 8)[7]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_TatianaLockUnlockMyComp, 1, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[8]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_TatianaLockUnlockMyComp, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[2]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_Listen, 9, 0, 0, 0);
		}

		break;
	case 11:
		if (!getCharacterParams(kCharacterCond2, 8)[16]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_Passing, msg->param, msg->sender, 0, 0);
		}

		break;
	case 17:
		if (!getCharacterParams(kCharacterCond2, 8)[16]) {
			if (checkCathDir(kCarRedSleeping, 1)) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
				Cond2Call(&LogicManager::CONS_Cond2_Passing, 1, 0, 0, 0);
			} else if (checkCathDir(kCarRedSleeping, 23)) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
				Cond2Call(&LogicManager::CONS_Cond2_Passing, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			if (getCharacterParams(kCharacterCond2, 8)[8]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
				Cond2Call(&LogicManager::CONS_Cond2_TatianaLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			// INTENTIONAL FALLTHROUGH
		case 2:
			if (getCharacterParams(kCharacterCond2, 8)[2]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
				Cond2Call(&LogicManager::CONS_Cond2_Listen, 9, 0, 0, 0);
			}

			break;
		case 7:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 225358684:
		if (!getCharacterParams(kCharacterCond2, 8)[0]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_CathBuzzing, msg->param, 0, 0, 0);
		}

		break;
	case 226078300:
		if (!getCharacterParams(kCharacterCond2, 8)[16] && !getCharacterParams(kCharacterCond2, 8)[0]) {
			playDialog(kCharacterCond2, "JAC2020", -1, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_DoSeqOtis, "697D", 0, 0, 0);
		}

		break;
	case 305159806:
		if (!getCharacterParams(kCharacterCond2, 8)[16] && !getCharacterParams(kCharacterCond2, 8)[0]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
			Cond2Call(&LogicManager::CONS_Cond2_CathRattling, msg->param, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_StartPart3);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 44;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterCond2);
		getCharacter(kCharacterCond2).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterCond2).characterPosition.position = 1500;
		getCharacter(kCharacterCond2).characterPosition.location = 0;
		getCharacter(kCharacterCond2).clothes = 0;
		getCharacter(kCharacterCond2).inventoryItem = 0;
		getCharacterParams(kCharacterCond2, 8)[7] = 0;
		getCharacterParams(kCharacterCond2, 8)[8] = 0;
		getCharacterParams(kCharacterCond2, 8)[2] = 0;
		getCharacterParams(kCharacterCond2, 8)[4] = 0;
		getCharacterParams(kCharacterCond2, 8)[3] = 0;
		getCharacterParams(kCharacterCond2, 8)[10] = 0;
		getCharacterParams(kCharacterCond2, 8)[12] = 0;
		getCharacterParams(kCharacterCond2, 8)[13] = 0;
		getCharacterParams(kCharacterCond2, 8)[15] = 0;
		getCharacterParams(kCharacterCond2, 8)[19] = 0;
		getCharacterParams(kCharacterCond2, 8)[20] = 0;
		getCharacterParams(kCharacterCond2, 8)[11] = 0;
		getCharacterParams(kCharacterCond2, 8)[14] = 0;
		getCharacterParams(kCharacterCond2, 8)[9] = 0;
		getCharacterParams(kCharacterCond2, 8)[1] = 1;
		setModel(111, 6);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			CONS_Cond2_SittingDay3(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_SittingDay3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_SittingDay3);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 45;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_SittingDay3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterParams(kCharacterCond2, 8)[7]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_TatianaLockUnlockMyComp, 1, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[8]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_TatianaLockUnlockMyComp, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[2]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_Listen, 9, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[4]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_Listen, 3, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[10]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_AnnaTakeMax, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[12]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_AnnaLockMe, 1, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[13]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_AnnaLockMe, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[15]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
			Cond2Call(&LogicManager::CONS_Cond2_AnnaUnlockMe, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[19]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_InviteTatiana, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[11]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
			Cond2Call(&LogicManager::CONS_Cond2_TakeMaxBack, 1, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[14]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 11;
			Cond2Call(&LogicManager::CONS_Cond2_TakeMaxBack, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[3]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 12;
			Cond2Call(&LogicManager::CONS_Cond2_Listen, 21, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[20] && _gameTime > 2056500 && _gameTime < 1417500) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 13;
			Cond2Call(&LogicManager::CONS_Cond2_InviteRebecca, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2088900 && !getCharacterCurrentParams(kCharacterCond2)[0]) {
			getCharacterCurrentParams(kCharacterCond2)[0] = 1;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 14;
			Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2119500 && !getCharacterCurrentParams(kCharacterCond2)[1]) {
			getCharacterCurrentParams(kCharacterCond2)[1] = 1;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 15;
			Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2138400 && !getCharacterCurrentParams(kCharacterCond2)[2]) {
			getCharacterCurrentParams(kCharacterCond2)[2] = 1;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 16;
			Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2147400 && !getCharacterCurrentParams(kCharacterCond2)[3]) {
			getCharacterCurrentParams(kCharacterCond2)[3] = 1;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 17;
			Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2160000 && !getCharacterCurrentParams(kCharacterCond2)[4]) {
			getCharacterCurrentParams(kCharacterCond2)[4] = 1;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 18;
			Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2205000 && !getCharacterCurrentParams(kCharacterCond2)[5]) {
			getCharacterCurrentParams(kCharacterCond2)[5] = 1;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 19;
			Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[1]) {
			if (_gameTime > 2025000 && !getCharacterCurrentParams(kCharacterCond2)[6]) {
				getCharacterCurrentParams(kCharacterCond2)[6] = 1;
				setModel(111, 7);
			}
			if (_gameTime > 2133000 && !getCharacterCurrentParams(kCharacterCond2)[7]) {
				getCharacterCurrentParams(kCharacterCond2)[7] = 1;
				setModel(111, 8);
			}
			if (_gameTime > 2173500 && !getCharacterCurrentParams(kCharacterCond2)[8]) {
				getCharacterCurrentParams(kCharacterCond2)[8] = 1;
				setModel(111, 9);
			}
		}

		break;
	case 11:
		if (!getCharacterParams(kCharacterCond2, 8)[16]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 20;
			Cond2Call(&LogicManager::CONS_Cond2_Passing, msg->param, msg->sender, 0, 0);
		}

		break;
	case 17:
		if (!getCharacterParams(kCharacterCond2, 8)[16]) {
			if (checkCathDir(kCarRedSleeping, 1)) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 21;
				Cond2Call(&LogicManager::CONS_Cond2_Passing, 1, 0, 0, 0);
			} else if (checkCathDir(kCarRedSleeping, 23)) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 22;
				Cond2Call(&LogicManager::CONS_Cond2_Passing, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			if (getCharacterParams(kCharacterCond2, 8)[8]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
				Cond2Call(&LogicManager::CONS_Cond2_TatianaLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacterParams(kCharacterCond2, 8)[2]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
				Cond2Call(&LogicManager::CONS_Cond2_Listen, 9, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (getCharacterParams(kCharacterCond2, 8)[4]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
				Cond2Call(&LogicManager::CONS_Cond2_Listen, 3, 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (getCharacterParams(kCharacterCond2, 8)[10]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
				Cond2Call(&LogicManager::CONS_Cond2_AnnaTakeMax, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (getCharacterParams(kCharacterCond2, 8)[12]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
				Cond2Call(&LogicManager::CONS_Cond2_AnnaLockMe, 1, 0, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (getCharacterParams(kCharacterCond2, 8)[13]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
				Cond2Call(&LogicManager::CONS_Cond2_AnnaLockMe, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 7:
			if (getCharacterParams(kCharacterCond2, 8)[15]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
				Cond2Call(&LogicManager::CONS_Cond2_AnnaUnlockMe, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 8:
			if (getCharacterParams(kCharacterCond2, 8)[19]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
				Cond2Call(&LogicManager::CONS_Cond2_InviteTatiana, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 9:
			if (getCharacterParams(kCharacterCond2, 8)[11]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
				Cond2Call(&LogicManager::CONS_Cond2_TakeMaxBack, 1, 0, 0, 0);
				break;
			}

			// fall through
		case 10:
			if (getCharacterParams(kCharacterCond2, 8)[14]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 11;
				Cond2Call(&LogicManager::CONS_Cond2_TakeMaxBack, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 11:
			if (getCharacterParams(kCharacterCond2, 8)[3]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 12;
				Cond2Call(&LogicManager::CONS_Cond2_Listen, 21, 0, 0, 0);
				break;
			}

			// fall through
		case 12:
			if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 12) {
				send(kCharacterCond2, kCharacterTrainM, 168255788, 0);
			}

			if (getCharacterParams(kCharacterCond2, 8)[20] && _gameTime > 2056500 && _gameTime < 1417500) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 13;
				Cond2Call(&LogicManager::CONS_Cond2_InviteRebecca, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 13:
			if (_gameTime > 2088900 && !getCharacterCurrentParams(kCharacterCond2)[0]) {
				getCharacterCurrentParams(kCharacterCond2)[0] = 1;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 14;
				Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 14:
			if (_gameTime > 2119500 && !getCharacterCurrentParams(kCharacterCond2)[1]) {
				getCharacterCurrentParams(kCharacterCond2)[1] = 1;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 15;
				Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 15:
			if (_gameTime > 2138400 && !getCharacterCurrentParams(kCharacterCond2)[2]) {
				getCharacterCurrentParams(kCharacterCond2)[2] = 1;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 16;
				Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 16:
			if (_gameTime > 2147400 && !getCharacterCurrentParams(kCharacterCond2)[3]) {
				getCharacterCurrentParams(kCharacterCond2)[3] = 1;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 17;
				Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 17:
			if (_gameTime > 2160000 && !getCharacterCurrentParams(kCharacterCond2)[4]) {
				getCharacterCurrentParams(kCharacterCond2)[4] = 1;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 18;
				Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 18:
			if (_gameTime > 2205000 && !getCharacterCurrentParams(kCharacterCond2)[5]) {
				getCharacterCurrentParams(kCharacterCond2)[5] = 1;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 19;
				Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 19:
			if (getCharacterParams(kCharacterCond2, 8)[1]) {
				if (_gameTime > 2025000 && !getCharacterCurrentParams(kCharacterCond2)[6]) {
					getCharacterCurrentParams(kCharacterCond2)[6] = 1;
					setModel(111, 7);
				}
				if (_gameTime > 2133000 && !getCharacterCurrentParams(kCharacterCond2)[7]) {
					getCharacterCurrentParams(kCharacterCond2)[7] = 1;
					setModel(111, 8);
				}
				if (_gameTime > 2173500 && !getCharacterCurrentParams(kCharacterCond2)[8]) {
					getCharacterCurrentParams(kCharacterCond2)[8] = 1;
					setModel(111, 9);
				}
			}

			break;
		case 23:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 24;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 225358684:
		if (!getCharacterParams(kCharacterCond2, 8)[0]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 25;
			Cond2Call(&LogicManager::CONS_Cond2_CathBuzzing, msg->param, 0, 0, 0);
		}

		break;
	case 226078300:
		if (!getCharacterParams(kCharacterCond2, 8)[16] && !getCharacterParams(kCharacterCond2, 8)[0]) {
			playDialog(kCharacterCond2, "JAC2020", -1, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 23;
			Cond2Call(&LogicManager::CONS_Cond2_DoSeqOtis, "697D", 0, 0, 0);
		}

		break;
	case 305159806:
		if (!getCharacterParams(kCharacterCond2, 8)[16] && !getCharacterParams(kCharacterCond2, 8)[0]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 26;
			Cond2Call(&LogicManager::CONS_Cond2_CathRattling, msg->param, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_AnnaTakeMax(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_AnnaTakeMax);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 46;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_AnnaTakeMax(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 4070, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond2, "627Vf");
			softBlockAtDoor(kCharacterCond2, 37);
			send(kCharacterCond2, kCharacterAnna, 253868128, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "LIB012", 0, 0, 0);
			break;
		case 3:
			startCycOtis(kCharacterCond2, "627Wf");
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Ann1016A", 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Ann4150", 0, 0, 0);
			break;
		case 5:
			playDialog(kCharacterCond2, "Ann3121", -1, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "629Bf", 0x25, 0, 0);
			break;
		case 6:
			startCycOtis(kCharacterCond2, "629Cf");
			softBlockAtDoor(kCharacterCond2, 37);

			// fall through
		case 7:
			if (whoRunningDialog(kCharacterCond2)) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
				Cond2Call(&LogicManager::CONS_Cond2_DoWait, 75, 0, 0, 0);
			} else {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
				Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Ann3122", 0, 0, 0);
			}

			break;
		case 8:
			playDialog(kCharacterCond2, "Ann3123", -1, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_DoWaitReal, 75, 0, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "629Ff", 37, 0, 0);
			break;
		case 10:
			softReleaseAtDoor(kCharacterCond2, 37);
			getCharacterParams(kCharacterCond2, 8)[10] = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 11;
			Cond2Call(&LogicManager::CONS_Cond2_ReturnMax, 1, 0, 0, 0);
			break;
		case 11:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_AnnaLockMe(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_AnnaLockMe);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 47;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_AnnaLockMe(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 4070, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Xf", 37, 0, 0);
			break;
		case 3:
			startCycOtis(kCharacterCond2, "627Wf");
			softBlockAtDoor(kCharacterCond2, 37);

			// fall through
		case 4:
			if (whoRunningDialog(kCharacterCond2)) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
				Cond2Call(&LogicManager::CONS_Cond2_DoWait, 225, 0, 0, 0);
			} else {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;

				if (!getCharacterCurrentParams(kCharacterCond2)[0]) {
					Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Ann3147a", 0, 0, 0);
				} else {
					Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Ann3149", 0, 0, 0);
				}
			}

			break;
		case 5:
			softReleaseAtDoor(kCharacterCond2, 37);
			send(kCharacterCond2, kCharacterAnna, 157894320, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
			break;
		case 6:
			getCharacterParams(kCharacterCond2, 8)[12] = 0;
			getCharacterParams(kCharacterCond2, 8)[13] = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_AnnaUnlockMe(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_AnnaUnlockMe);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 48;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_AnnaUnlockMe(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			playDialog(kCharacterCond2, "Ann3148A", -1, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 4070, 0, 0);
			break;
		case 2:
			if (rnd(2) == 0) {
				playDialog(kCharacterCond2, "Ann3148B", -1, 0);
			} else {
				playDialog(kCharacterCond2, "Ann3148", -1, 0);
			}

			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Xf", 37, 0, 0);
			break;
		case 3:
			send(kCharacterCond2, kCharacterAnna, 192063264, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
			break;
		case 4:
			getCharacterParams(kCharacterCond2, 8)[15] = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_InviteTatiana(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_InviteTatiana);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 49;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_InviteTatiana(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 7500, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Vb", 33, 0, 0);
			break;
		case 3:
			if (inComp(kCharacterTatiana, kCarRedSleeping, 7500)) {
				startCycOtis(kCharacterCond2, "627Wb");
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
				Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Jac3006", 0, 0, 0);
			} else {
				startCycOtis(kCharacterCond2, "627Wb");
				softBlockAtDoor(kCharacterCond2, 33);
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
				Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "LIB012", 0, 0, 0);
			}

			break;
		case 4:
			softReleaseAtDoor(kCharacterCond2, 33);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Zb", 33, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Jac3006A", 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "697Ab", 33, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond2).characterPosition.location = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_DoWait, 0x96, 0, 0, 0);
			break;
		case 9:
			softReleaseAtDoor(kCharacterCond2, 33);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
			break;
		case 10:
			send(kCharacterCond2, kCharacterMadame, 242526416, 0);
			getCharacterParams(kCharacterCond2, 8)[19] = 0;
			getCharacterParams(kCharacterCond2, 8)[20] = 1;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 11;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 11:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_InviteRebecca(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_InviteRebecca);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 50;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_InviteRebecca(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 4840, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond2, "627Me");
			softBlockAtDoor(kCharacterCond2, 36);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "LIB012", 0, 0, 0);
			break;
		case 3:
			if (!inComp(kCharacterRebecca, kCarRedSleeping, 4840)) {
				softReleaseAtDoor(kCharacterCond2, 36);
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
				Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
			} else {
				startCycOtis(kCharacterCond2, "627Ne");
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
				Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Jac3005", 0, 0, 0);
			}

			break;
		case 4:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Re", 36, 0, 0);
			break;
		case 5:
			softReleaseAtDoor(kCharacterCond2, 36);
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Jac3005A", 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Se", 0x24, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond2).characterPosition.location = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
			break;
		case 8:
			getCharacterParams(kCharacterCond2, 8)[20] = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_TatianaComeHere(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_TatianaComeHere);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 51;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_TatianaComeHere(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2133000 && !_globals[kGlobalConcertIsHappening]) {
			softReleaseAtDoor(kCharacterCond2, 33);
			setDoor(32, kCharacterCath, 0, 10, 9);
			setDoor(33, kCharacterCath, 1, 10, 9);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
		}

		break;
	case 9:
		if (msg->param.intParam == 33)
			getCharacter(kCharacterCond2).characterPosition.position = 7500;

		playDialog(0, "LIB014", -1, 0);
		playNIS(kEventCoudertGoingOutOfVassiliCompartment);
		walk(kCharacterCond2, kCarRedSleeping, 2000);

		if (msg->param.intParam == 33)
			bumpCathRDoor(33);
		else
			bumpCathRDoor(32);

		softReleaseAtDoor(kCharacterCond2, 33);
		setDoor(32, kCharacterCath, 0, 10, 9);
		setDoor(33, kCharacterCath, 1, 10, 9);
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterCond2).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterCond2).characterPosition.position = 7500;
		getCharacter(kCharacterCond2).characterPosition.location = 0;
		send(kCharacterCond2, kCharacterMax, 135204609, 0);

		if (getCharacterParams(kCharacterCond2, 8)[4]) {
			send(kCharacterCond2, kCharacterCond1, 155853632, 0);
			send(kCharacterCond2, kCharacterCond1, 2, 0);
			getCharacterParams(kCharacterCond2, 8)[4] = 0;
		}

		if (getCharacterParams(kCharacterCond2, 8)[2]) {
			send(kCharacterCond2, kCharacterTrainM, 155853632, 0);
			send(kCharacterCond2, kCharacterTrainM, 2, 0);
			getCharacterParams(kCharacterCond2, 8)[2] = 0;
		}

		startCycOtis(kCharacterCond2, "627Wb");
		softBlockAtDoor(kCharacterCond2, 33);
		send(kCharacterCond2, kCharacterTatiana, 154071333, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 2:
		case 4:
		case 6:
			CONS_Cond2_SittingDay3(0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 168316032:
		setDoor(32, kCharacterCond2, 0, 0, 9);
		setDoor(33, kCharacterCond2, 1, 0, 9);
		break;
	case 235061888:
		softReleaseAtDoor(kCharacterCond2, 33);
		setDoor(32, kCharacterCath, 0, 10, 9);
		setDoor(33, kCharacterCath, 1, 10, 9);
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_StartPart4);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 52;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterCond2);
		getCharacter(kCharacterCond2).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterCond2).characterPosition.position = 1500;
		getCharacter(kCharacterCond2).characterPosition.location = 0;
		getCharacter(kCharacterCond2).clothes = 0;
		getCharacterParams(kCharacterCond2, 8)[18] = 0;
		getCharacterParams(kCharacterCond2, 8)[7] = 0;
		getCharacterParams(kCharacterCond2, 8)[8] = 0;
		getCharacterParams(kCharacterCond2, 8)[2] = 0;
		getCharacterParams(kCharacterCond2, 8)[4] = 0;
		getCharacterParams(kCharacterCond2, 8)[3] = 0;
		getCharacterParams(kCharacterCond2, 8)[5] = 0;
		getCharacterParams(kCharacterCond2, 8)[10] = 0;
		getCharacterParams(kCharacterCond2, 8)[12] = 0;
		getCharacterParams(kCharacterCond2, 8)[13] = 0;
		getCharacterParams(kCharacterCond2, 8)[15] = 0;
		getCharacterParams(kCharacterCond2, 8)[19] = 0;
		getCharacterParams(kCharacterCond2, 8)[14] = 0;
		getCharacterParams(kCharacterCond2, 8)[1] = 1;
		setModel(111, 10);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			getCharacterParams(kCharacterCond2, 8)[9] = 1;
			CONS_Cond2_OnDuty4(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_OnDuty4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_OnDuty4);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 53;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_OnDuty4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterParams(kCharacterCond2, 8)[18]) {
			getCharacterParams(kCharacterCond2, 8)[9] = 0;
			getCharacterParams(kCharacterCond2, 8)[14] = 0;
			getCharacterCurrentParams(kCharacterCond2)[0] = 1;
			setModel(32, 1);
			setModel(33, 1);
			setModel(34, 1);
			setModel(35, 1);
			setModel(36, 1);
			setModel(37, 1);
			setModel(38, 1);
			setModel(39, 1);
			getCharacterParams(kCharacterCond2, 8)[18] = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_WaitBombDefused, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond2, 8)[9]) {
			if (!getCharacterCurrentParams(kCharacterCond2)[1])
				getCharacterCurrentParams(kCharacterCond2)[1] = _gameTime + 4500;

			if (getCharacterCurrentParams(kCharacterCond2)[2] != 0x7FFFFFFF && _gameTime) {
				if (getCharacterCurrentParams(kCharacterCond2)[1] < _gameTime) {
					getCharacterCurrentParams(kCharacterCond2)[2] = 0x7FFFFFFF;
					getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
					Cond2Call(&LogicManager::CONS_Cond2_ServiceAnna, 0, 0, 0, 0);
					break;
				}

				if (!cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterCond2)[2]) {
					getCharacterCurrentParams(kCharacterCond2)[2] = _gameTime;
					if (!_gameTime) {
						getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
						Cond2Call(&LogicManager::CONS_Cond2_ServiceAnna, 0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterCond2)[2] < _gameTime) {
					getCharacterCurrentParams(kCharacterCond2)[2] = 0x7FFFFFFF;

					getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
					Cond2Call(&LogicManager::CONS_Cond2_ServiceAnna, 0, 0, 0, 0);
					break;
				}
			}
		}

		if (getCharacterParams(kCharacterCond2, 8)[14]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_TakeMaxBack, 0, 0, 0, 0);
			break;
		}

		if (!getCharacterCurrentParams(kCharacterCond2)[0]) {
			if (_gameTime > 2394000 && !getCharacterCurrentParams(kCharacterCond2)[3]) {
				getCharacterCurrentParams(kCharacterCond2)[3] = 1;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
				Cond2Call(&LogicManager::CONS_Cond2_MakeAllBeds4, 0, 0, 0, 0);
				break;
			}

			if (_gameTime > 2434500 && !getCharacterCurrentParams(kCharacterCond2)[4]) {
				getCharacterCurrentParams(kCharacterCond2)[4] = 1;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
				Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			if (_gameTime > 2448000 && !getCharacterCurrentParams(kCharacterCond2)[5]) {
				getCharacterCurrentParams(kCharacterCond2)[5] = 1;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
				Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
				break;
			}
		}

		if (_gameTime > 2538000 && !getCharacterParams(kCharacterCond2, 8)[0] && !getCharacterParams(kCharacterCond2, 8)[16]) {
			if (getCharacterCurrentParams(kCharacterCond2)[6] || (getCharacterCurrentParams(kCharacterCond2)[6] = _gameTime + 2700, _gameTime != -2700)) {
				if (getCharacterCurrentParams(kCharacterCond2)[6] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterCond2)[6] = 0x7FFFFFFF;
			}

			getCharacterParams(kCharacterCond2, 8)[1] = 0;
			getCharacterParams(kCharacterCond2, 8)[0] = 1;
			startCycOtis(kCharacterCond2, "697F");
			getCharacterCurrentParams(kCharacterCond2)[6] = 0;
		}

		break;
	case 11:
		if (!getCharacterParams(kCharacterCond2, 8)[16] && !getCharacterParams(kCharacterCond2, 8)[0]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_Passing, msg->param, msg->sender, 0, 0);
		}

		break;
	case 17:
		if (!getCharacterParams(kCharacterCond2, 8)[16] && !getCharacterParams(kCharacterCond2, 8)[0]) {
			if (checkCathDir(kCarRedSleeping, 1)) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
				Cond2Call(&LogicManager::CONS_Cond2_Passing, 1, 0, 0, 0);
			} else if (checkCathDir(kCarRedSleeping, 23)) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
				Cond2Call(&LogicManager::CONS_Cond2_Passing, 0, 0, 0, 0);
			}
		}

		break;
	case 12:
		getCharacter(kCharacterCond2).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterCond2).characterPosition.position = 1500;
		getCharacter(kCharacterCond2).characterPosition.location = 0;
		takeItem(kItem5);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			if (getCharacterParams(kCharacterCond2, 8)[9]) {
				if (!getCharacterCurrentParams(kCharacterCond2)[1])
					getCharacterCurrentParams(kCharacterCond2)[1] = _gameTime + 4500;
				if (getCharacterCurrentParams(kCharacterCond2)[2] != 0x7FFFFFFF && _gameTime) {
					if (getCharacterCurrentParams(kCharacterCond2)[1] < _gameTime) {
						getCharacterCurrentParams(kCharacterCond2)[2] = 0x7FFFFFFF;
						getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
						Cond2Call(&LogicManager::CONS_Cond2_ServiceAnna, 0, 0, 0, 0);
						break;
					}

					if (!cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterCond2)[2]) {
						getCharacterCurrentParams(kCharacterCond2)[2] = _gameTime;
						if (!_gameTime) {
							getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
							Cond2Call(&LogicManager::CONS_Cond2_ServiceAnna, 0, 0, 0, 0);
							break;
						}
					}

					if (getCharacterCurrentParams(kCharacterCond2)[2] < _gameTime) {
						getCharacterCurrentParams(kCharacterCond2)[2] = 0x7FFFFFFF;

						getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
						Cond2Call(&LogicManager::CONS_Cond2_ServiceAnna, 0, 0, 0, 0);
						break;
					}
				}
			}

			// fall through
		case 2:
			if (getCharacterParams(kCharacterCond2, 8)[14]) {
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
				Cond2Call(&LogicManager::CONS_Cond2_TakeMaxBack, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (!getCharacterCurrentParams(kCharacterCond2)[0]) {
				if (_gameTime > 2394000 && !getCharacterCurrentParams(kCharacterCond2)[3]) {
					getCharacterCurrentParams(kCharacterCond2)[3] = 1;
					getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
					Cond2Call(&LogicManager::CONS_Cond2_MakeAllBeds4, 0, 0, 0, 0);
					break;
				}

				if (_gameTime > 2434500 && !getCharacterCurrentParams(kCharacterCond2)[4]) {
					getCharacterCurrentParams(kCharacterCond2)[4] = 1;
					getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
					Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
					break;
				}

				if (_gameTime > 2448000 && !getCharacterCurrentParams(kCharacterCond2)[5]) {
					getCharacterCurrentParams(kCharacterCond2)[5] = 1;
					getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
					Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
					break;
				}
			}

			// fall through
		case 4:
			if (_gameTime > 2434500 && !getCharacterCurrentParams(kCharacterCond2)[4]) {
				getCharacterCurrentParams(kCharacterCond2)[4] = 1;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
				Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (_gameTime > 2448000 && !getCharacterCurrentParams(kCharacterCond2)[5]) {
				getCharacterCurrentParams(kCharacterCond2)[5] = 1;
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
				Cond2Call(&LogicManager::CONS_Cond2_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (_gameTime > 2538000 && !getCharacterParams(kCharacterCond2, 8)[0] && !getCharacterParams(kCharacterCond2, 8)[16]) {
				if (getCharacterCurrentParams(kCharacterCond2)[6] || (getCharacterCurrentParams(kCharacterCond2)[6] = _gameTime + 2700, _gameTime != -2700)) {
					if (getCharacterCurrentParams(kCharacterCond2)[6] >= _gameTime)
						break;

					getCharacterCurrentParams(kCharacterCond2)[6] = 0x7FFFFFFF;
				}

				getCharacterParams(kCharacterCond2, 8)[1] = 0;
				getCharacterParams(kCharacterCond2, 8)[0] = 1;
				startCycOtis(kCharacterCond2, "697F");
				getCharacterCurrentParams(kCharacterCond2)[6] = 0;
			}

			break;
		case 10:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 11;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 226078300:
		if (!getCharacterParams(kCharacterCond2, 8)[16] && !getCharacterParams(kCharacterCond2, 8)[0]) {
			playDialog(kCharacterCond2, "JAC2020", -1, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
			Cond2Call(&LogicManager::CONS_Cond2_DoSeqOtis, "697D", 0, 0, 0);
		}

		break;
	case 305159806:
		if (!getCharacterParams(kCharacterCond2, 8)[16] && !getCharacterParams(kCharacterCond2, 8)[0]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 13;
			Cond2Call(&LogicManager::CONS_Cond2_CathRattling, msg->param, 0, 0, 0);
		}

		break;
	case 225358684:
		if (!getCharacterParams(kCharacterCond2, 8)[0]) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 12;
			Cond2Call(&LogicManager::CONS_Cond2_CathBuzzing, msg->param, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_WaitBombDefused(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_WaitBombDefused);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 54;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_WaitBombDefused(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (whoOnScreen(kCharacterCond2)) {
			getCharacter(kCharacterCond2).characterPosition.location = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 540, 0, 0);
		} else {
			getCharacter(kCharacterCond2).characterPosition.car = kCarLocomotive;
			getCharacter(kCharacterCond2).characterPosition.position = 540;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			endGraphics(kCharacterCond2);
			getCharacter(kCharacterCond2).characterPosition.car = kCarLocomotive;
			break;
		case 2:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 191001984:
		getCharacter(kCharacterCond2).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 1500, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_ServiceAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_ServiceAnna);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 55;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_ServiceAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "LIB070", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 4070, 0, 0);
			break;
		case 3:
			startCycOtis(kCharacterCond2, "627Wf");
			softBlockAtDoor(kCharacterCond2, 37);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Ann4150A", 0, 0, 0);
			break;
		case 4:
			softReleaseAtDoor(kCharacterCond2, 37);
			send(kCharacterCond2, kCharacterAnna, 219971920, 0);
			send(kCharacterCond2, kCharacterHeadWait, 101824388, 0);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 9460, 0, 0);
			break;
		case 5:
			endGraphics(kCharacterCond2);
			send(kCharacterCond2, kCharacterHeadWait, 136059947, 0);
			break;
		case 6:
			getCharacterParams(kCharacterCond2, 8)[9] = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
			fedEx(kCharacterCond2, kCharacterCond2, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 123712592:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_MakeAllBeds4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_MakeAllBeds4);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 56;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_MakeAllBeds4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedIvo, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedMilos, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedAnna, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedRebecca, 0, 0, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 9;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 10;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedMadame, 0, 0, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 11;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 11:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 12;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedMonsieur, 0, 0, 0, 0);
			break;
		case 12:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 13;
			Cond2Call(&LogicManager::CONS_Cond2_DoPending, 0, 0, 0, 0);
			break;
		case 13:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 14;
			Cond2Call(&LogicManager::CONS_Cond2_MakeBedTatiana, 0, 0, 0, 0);
			break;
		case 14:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 15;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
			break;
		case 15:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 16;
			Cond2Call(&LogicManager::CONS_Cond2_SitDown, 0, 0, 0, 0);
			break;
		case 16:
			getCharacter(kCharacterCond2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond2, _functionsCond2[getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall]]);
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

void LogicManager::CONS_Cond2_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_StartPart5);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 57;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Cond2_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterCond2);
		getCharacter(kCharacterCond2).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterCond2).characterPosition.position = 3969;
		getCharacter(kCharacterCond2).characterPosition.location = 1;
		getCharacter(kCharacterCond2).inventoryItem = kItemNone;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_Prisoner);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 58;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Cond2_ReturnSeat(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_ReturnSeat(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_ReturnSeat);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 59;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_ReturnSeat(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterCond2).characterPosition.position = 7500;
		getCharacter(kCharacterCond2).characterPosition.location = 0;
		playDialog(kCharacterCond2, "Jac5010", -1, 0);
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2000, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1) {
			startCycOtis(kCharacterCond2, "627K");
			CONS_Cond2_Waiting5(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_Waiting5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_Waiting5);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 60;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_Waiting5(HAND_PARAMS) {
	switch (msg->action) {
	case 18:
		if (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] == 1)
			CONS_Cond2_MakeRounds(0, 0, 0, 0);

		break;
	case 155991520:
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWait, 225, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_MakeRounds(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_MakeRounds);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 61;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_MakeRounds(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond2).characterPosition.position = 2088;
		getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
		Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 4840, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Me", 36, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond2, "627Ne");
			softBlockAtDoor(kCharacterCond2, 36);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoWait, 75, 0, 0, 0);
			break;
		case 3:
			softReleaseAtDoor(kCharacterCond2, 36);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Re", 36, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			setDoor(36, kCharacterCath, 3, 10, 9);
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Reb5010", 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 6;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Se", 36, 0, 0);
			break;
		case 6:
			send(kCharacterCond2, kCharacterRebecca, 155604840, 0);
			getCharacter(kCharacterCond2).characterPosition.location = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 7;
			Cond2Call(&LogicManager::CONS_Cond2_DoWalk, 4, 2740, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 8;
			Cond2Call(&LogicManager::CONS_Cond2_DoCorrOtis, "627Zh", 39, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterCond2).characterPosition.location = 1;
			endGraphics(kCharacterCond2);
			send(kCharacterCond2, kCharacterHeadWait, 169750080, 0);
			CONS_Cond2_InCompH(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_InCompH(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_InCompH);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 62;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_InCompH(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterCond2)[0]) {
			if (getCharacterCurrentParams(kCharacterCond2)[3] ||
				(getCharacterCurrentParams(kCharacterCond2)[3] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterCond2)[3] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterCond2)[3] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterCond2)[0] = 0;
			getCharacterCurrentParams(kCharacterCond2)[1] = 1;
			setDoor(39, kCharacterCond2, 1, 0, 0);
			getCharacterCurrentParams(kCharacterCond2)[3] = 0;
		} else {
			getCharacterCurrentParams(kCharacterCond2)[3] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterCond2)[0]) {
			setDoor(39, kCharacterCond2, 1, 0, 0);
			getCharacterCurrentParams(kCharacterCond2)[0] = 0;
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 1;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, getCathJustChecking(), 0, 0, 0);
		} else if (msg->action == 8) {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 2;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "LIB012", 0, 0, 0);
		} else {
			getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 3;
			Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "LIB013", 0, 0, 0);
		}

		break;
	case 12:
		setDoor(39, kCharacterCond2, 1, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterCond2)[1] || getCharacterCurrentParams(kCharacterCond2)[0]) {
			getCharacterCurrentParams(kCharacterCond2)[1] = 0;
			getCharacterCurrentParams(kCharacterCond2)[0] = 0;
			setDoor(39, kCharacterCond2, 1, 10, 9);
			getCharacterCurrentParams(kCharacterCond2)[2] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8]) {
		case 1:
			setDoor(39, kCharacterCond2, 1, 10, 9);
			break;
		case 2:
		case 3:
			getCharacterCurrentParams(kCharacterCond2)[2]++;
			if (getCharacterCurrentParams(kCharacterCond2)[2] == 1) {
				setDoor(39, kCharacterCond2, 1, 0, 0);
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 4;
				Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Jac5002", 0, 0, 0);
			} else if (getCharacterCurrentParams(kCharacterCond2)[2] == 2) {
				setDoor(39, kCharacterCond2, 1, 0, 0);
				getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall + 8] = 5;
				Cond2Call(&LogicManager::CONS_Cond2_DoDialog, "Jac5002A", 0, 0, 0);
			}

			break;
		case 4:
			getCharacterCurrentParams(kCharacterCond2)[0] = 1;
			setDoor(39, kCharacterCond2, 1, 14, 0);
			break;
		case 5:
			getCharacterCurrentParams(kCharacterCond2)[1] = 1;
			break;
		default:
			break;
		}

		break;
	case 135800432:
		CONS_Cond2_Disappear(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond2_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond2).callParams[getCharacter(kCharacterCond2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond2, &LogicManager::HAND_Cond2_Disappear);
	getCharacter(kCharacterCond2).callbacks[getCharacter(kCharacterCond2).currentCall] = 63;

	params->clear();

	fedEx(kCharacterCond2, kCharacterCond2, 12, 0);
}

void LogicManager::HAND_Cond2_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsCond2[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Cond2_DebugWalks,
	&LogicManager::HAND_Cond2_DoSeqOtis,
	&LogicManager::HAND_Cond2_DoCorrOtis,
	&LogicManager::HAND_Cond2_FinishSeqOtis,
	&LogicManager::HAND_Cond2_DoEnterCorrOtis,
	&LogicManager::HAND_Cond2_DoDialog,
	&LogicManager::HAND_Cond2_DoDialogFullVol,
	&LogicManager::HAND_Cond2_SaveGame,
	&LogicManager::HAND_Cond2_DoWalk,
	&LogicManager::HAND_Cond2_DoWait,
	&LogicManager::HAND_Cond2_DoWaitReal,
	&LogicManager::HAND_Cond2_StandAsideDialog,
	&LogicManager::HAND_Cond2_Passing,
	&LogicManager::HAND_Cond2_Listen,
	&LogicManager::HAND_Cond2_TatianaLockUnlockMyComp,
	&LogicManager::HAND_Cond2_GetUp,
	&LogicManager::HAND_Cond2_GetUpListen,
	&LogicManager::HAND_Cond2_SitDown,
	&LogicManager::HAND_Cond2_SitDownFast,
	&LogicManager::HAND_Cond2_MakeBed,
	&LogicManager::HAND_Cond2_MakeBedIvo,
	&LogicManager::HAND_Cond2_MakeBedMilos,
	&LogicManager::HAND_Cond2_TryMakeAnna,
	&LogicManager::HAND_Cond2_MakeBedAnna,
	&LogicManager::HAND_Cond2_MakeBedRebecca,
	&LogicManager::HAND_Cond2_MakeBedMadame,
	&LogicManager::HAND_Cond2_MakeBedMonsieur,
	&LogicManager::HAND_Cond2_MakeBedTatiana,
	&LogicManager::HAND_Cond2_MakeBedVassili,
	&LogicManager::HAND_Cond2_CathBuzzing,
	&LogicManager::HAND_Cond2_CathRattling,
	&LogicManager::HAND_Cond2_BathroomTrip,
	&LogicManager::HAND_Cond2_DoPending,
	&LogicManager::HAND_Cond2_TakeMaxBack,
	&LogicManager::HAND_Cond2_ReturnMax,
	&LogicManager::HAND_Cond2_Birth,
	&LogicManager::HAND_Cond2_RespondVassili,
	&LogicManager::HAND_Cond2_AtSeizure,
	&LogicManager::HAND_Cond2_AfterPolice,
	&LogicManager::HAND_Cond2_Sitting,
	&LogicManager::HAND_Cond2_MakeAllBeds,
	&LogicManager::HAND_Cond2_StartPart2,
	&LogicManager::HAND_Cond2_SittingDay,
	&LogicManager::HAND_Cond2_StartPart3,
	&LogicManager::HAND_Cond2_SittingDay3,
	&LogicManager::HAND_Cond2_AnnaTakeMax,
	&LogicManager::HAND_Cond2_AnnaLockMe,
	&LogicManager::HAND_Cond2_AnnaUnlockMe,
	&LogicManager::HAND_Cond2_InviteTatiana,
	&LogicManager::HAND_Cond2_InviteRebecca,
	&LogicManager::HAND_Cond2_TatianaComeHere,
	&LogicManager::HAND_Cond2_StartPart4,
	&LogicManager::HAND_Cond2_OnDuty4,
	&LogicManager::HAND_Cond2_WaitBombDefused,
	&LogicManager::HAND_Cond2_ServiceAnna,
	&LogicManager::HAND_Cond2_MakeAllBeds4,
	&LogicManager::HAND_Cond2_StartPart5,
	&LogicManager::HAND_Cond2_Prisoner,
	&LogicManager::HAND_Cond2_ReturnSeat,
	&LogicManager::HAND_Cond2_Waiting5,
	&LogicManager::HAND_Cond2_MakeRounds,
	&LogicManager::HAND_Cond2_InCompH,
	&LogicManager::HAND_Cond2_Disappear
};

} // End of namespace LastExpress
