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

void LogicManager::CONS_August(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterAugust,
			_functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]
		);

		break;
	case 1:
		CONS_August_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_August_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_August_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_August_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_August_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::AugustCall(CALL_PARAMS) {
	getCharacter(kCharacterAugust).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_August_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DebugWalks);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 1;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterAugust, kCarGreenSleeping, getCharacterCurrentParams(kCharacterAugust)[0])) {
			if (getCharacterCurrentParams(kCharacterAugust)[0] == 10000) {
				getCharacterCurrentParams(kCharacterAugust)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterAugust)[0] = 10000;
			}
		}

		break;
	case 1:
		getCharacter(kCharacterAugust).clothes++;
		if (getCharacter(kCharacterAugust).clothes > 2)
			getCharacter(kCharacterAugust).clothes = 0;

		break;
	case 12:
		getCharacter(kCharacterAugust).characterPosition.position = 0;
		getCharacter(kCharacterAugust).characterPosition.location = 0;
		getCharacter(kCharacterAugust).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAugust).inventoryItem = kItemInvalid;
		getCharacterCurrentParams(kCharacterAugust)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoWait);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 2;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAugust)[1] ||
			(getCharacterCurrentParams(kCharacterAugust)[1] = _gameTime + getCharacterCurrentParams(kCharacterAugust)[0], _gameTime + getCharacterCurrentParams(kCharacterAugust)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterAugust)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterAugust)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoSeqOtis);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoBlockSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoBlockSeqOtis);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoBlockSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseView(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[3], getCharacterCurrentParams(kCharacterAugust)[4]);

		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[0]);
		blockView(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[3], getCharacterCurrentParams(kCharacterAugust)[4]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoCorrOtis);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[3]);

		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[0]);
		blockAtDoor(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoEnterCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoEnterCorrOtis);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoEnterCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[3]);
		getCharacter(kCharacterAugust).characterPosition.position = 6470;
		getCharacter(kCharacterAugust).characterPosition.location = 1;

		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[0]);
		blockAtDoor(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[3]);
		getCharacter(kCharacterAugust).characterPosition.location = 1;
		if (inComp(kCharacterCath, kCarGreenSleeping, 6470) || inComp(kCharacterCath, kCarGreenSleeping, 6130)) {
			if (isNight()) {
				playNIS(kEventCathTurningNight);
			} else {
				playNIS(kEventCathTurningDay);
			}
			
			playDialog(0, "BUMP", -1, 0);
			bumpCathFDoor(3);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoBriefCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoBriefCorrOtis);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 7;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoBriefCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
	case 4:
		releaseAtDoor(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[3]);

		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[0]);
		blockAtDoor(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_FinishSeqOtis);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 8;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterAugust).direction != 4) {
			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoJoinedSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoJoinedSeqOtis);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 9;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	strncpy((char *)&params->parameters[5], param4.stringParam, 12);


	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoJoinedSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		if (!getCharacterCurrentParams(kCharacterAugust)[8])
			fedEx(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[3], getCharacterCurrentParams(kCharacterAugust)[4], (char *)&getCharacterCurrentParams(kCharacterAugust)[5]);

		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	case 10:
		if (!getCharacterCurrentParams(kCharacterAugust)[8]) {
			fedEx(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[3], getCharacterCurrentParams(kCharacterAugust)[4], (char *)&getCharacterCurrentParams(kCharacterAugust)[5]);
			getCharacterCurrentParams(kCharacterAugust)[8] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_FinishJoinedSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_FinishJoinedSeqOtis);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	strncpy((char *)&params->parameters[2], param3.stringParam, 12);

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_FinishJoinedSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		if (!getCharacterCurrentParams(kCharacterAugust)[5])
			fedEx(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[0], getCharacterCurrentParams(kCharacterAugust)[1], (char *)&getCharacterCurrentParams(kCharacterAugust)[2]);

		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	case 10:
		if (!getCharacterCurrentParams(kCharacterAugust)[5]) {
			fedEx(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[0], getCharacterCurrentParams(kCharacterAugust)[1], (char *)&getCharacterCurrentParams(kCharacterAugust)[2]);
			getCharacterCurrentParams(kCharacterAugust)[5] = 1;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoComplexSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoComplexSeqOtis);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 11;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);
	strncpy((char *)&params->parameters[3], param2.stringParam, 12);

	params->parameters[6] = param3.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoComplexSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[0]);
		startSeqOtis(getCharacterCurrentParams(kCharacterAugust)[6], (char *)&getCharacterCurrentParams(kCharacterAugust)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoDialog);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 12;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	case 12:
		playDialog(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoDialogFullVol(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoDialogFullVol);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 13;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoDialogFullVol(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	case 12:
		playDialog(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[0], 16, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_WaitRCClear);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 14;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_SaveGame);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 15;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	case 12:
		save(
			kCharacterAugust,
			getCharacterCurrentParams(kCharacterAugust)[0],
			getCharacterCurrentParams(kCharacterAugust)[1]
		);

		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoWalk);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 16;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[0], getCharacterCurrentParams(kCharacterAugust)[1])) {
			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		}

		break;
	case 5:
		if (_globals[kGlobalMetAugust]) {
			if (rnd(2) == 0) {
				playDialog(kCharacterCath, "CAT1002A", -1, 0);
			} else {
				playDialog(kCharacterCath, "CAT1002", -1, 0);
			}
		} else {
			playCathExcuseMe();
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterAugust, kCharacterCath, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_LookingForCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_LookingForCath);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 17;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_LookingForCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAugust)[0] >= _gameTime || getCharacterCurrentParams(kCharacterAugust)[1]) {
			if (cathInCorridor(kCarGreenSleeping) || cathInCorridor(kCarRedSleeping)) {
				if (checkLoc(kCharacterCath, kCarGreenSleeping)) {
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
					AugustCall(&LogicManager::CONS_August_DoWalkSearchingForCath, 3, 540, 0, 0);
				} else {
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
					AugustCall(&LogicManager::CONS_August_DoWalkSearchingForCath, 4, 9460, 0, 0);
				}
			}
		} else {
			getCharacterCurrentParams(kCharacterAugust)[1] = 1;

			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		}

		break;
	case 12:
		getCharacterParams(kCharacterAugust, 8)[0] = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_DoWalkSearchingForCath, 4, 540, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			if (!getCharacterParams(kCharacterAugust, 8)[0]) {
				endGraphics(kCharacterAugust);
			} else {
				getCharacter(kCharacterAugust).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
				fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			}

			break;
		case 2:
		case 3:
			if (getCharacterParams(kCharacterAugust, 8)[0]) {
				getCharacter(kCharacterAugust).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
				fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			} else {
				endGraphics(kCharacterAugust);
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
				AugustCall(&LogicManager::CONS_August_DoWait, 450, 0, 0, 0);
			}

			break;
		case 4:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			AugustCall(&LogicManager::CONS_August_DoWalkSearchingForCath, 4, 540, 0, 0);
			break;
		case 5:
			if (getCharacterParams(kCharacterAugust, 8)[0]) {
				getCharacter(kCharacterAugust).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
				fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			} else {
				endGraphics(kCharacterAugust);
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

void LogicManager::CONS_August_DoWalkSearchingForCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoWalkSearchingForCath);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 18;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoWalkSearchingForCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[0], getCharacterCurrentParams(kCharacterAugust)[1])) {
			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		} else if (nearChar(kCharacterAugust, kCharacterCath, 1000) && !onLowPlatform(kCharacterCath) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			if (getCharacter(kCharacterAugust).characterPosition.car == kCarGreenSleeping || getCharacter(kCharacterAugust).characterPosition.car == kCarRedSleeping) {
				getCharacterParams(kCharacterAugust, 8)[0] = 1;

				getCharacter(kCharacterAugust).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
				fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			}
		}

		break;
	case 12:
		if (walk(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[0], getCharacterCurrentParams(kCharacterAugust)[1])) {
			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_EnterComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_EnterComp);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 19;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_EnterComp(HAND_PARAMS) {
	switch (msg->action) {
	case 1:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		playDialog(kCharacterCath, "CAT1002", -1, 0);
		playDialog(kCharacterAugust, "AUG3101", -1, 15);
		break;
	case 12:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		switch (_globals[kGlobalChapter]) {
		case 1:
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[2], 12, "626");
			break;
		case 2:
		case 3:
			if (getCharacter(kCharacterAugust).clothes == 2) {
				Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[2], 12, "696");
			} else {
				Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[2], 12, "666");
			}

			break;
		case 4:
		case 5:
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[2], 12, "696");
			break;
		default:
			break;
		}

		send(kCharacterAugust, kCharacterCond1, 303343617, 0);
		Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[5], 12, (char *)&getCharacterCurrentParams(kCharacterAugust)[2]);
		Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterAugust)[5], 12, "Pc");
		startCycOtis(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[5]);
		softBlockAtDoor(kCharacterAugust, 3);
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_DoDialog, "AUG2096", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[5], 12, (char *)&getCharacterCurrentParams(kCharacterAugust)[2]);
			Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterAugust)[5], 12, "Qc");
			startCycOtis(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[5]);
			if (getCharacterCurrentParams(kCharacterAugust)[1])
				getCharacter(kCharacterAugust).inventoryItem = 147;

			break;
		case 2:
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[5], 12, (char *)&getCharacterCurrentParams(kCharacterAugust)[2]);
			if (getCharacterCurrentParams(kCharacterAugust)[0]) {
				Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterAugust)[5], 12, "Fc");
			} else {
				Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterAugust)[5], 12, "Dc");
			}

			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_DoEnterCorrOtis, (char *)&getCharacterCurrentParams(kCharacterAugust)[5], 3, 0, 0);
			break;
		case 3:
			softReleaseAtDoor(kCharacterAugust, 3);
			getCharacter(kCharacterAugust).characterPosition.location = 1;
			endGraphics(kCharacterAugust);

			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 69239528:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
		AugustCall(&LogicManager::CONS_August_DoWait, 75, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_ExitComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_ExitComp);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 20;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_ExitComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		switch (_globals[kGlobalChapter]) {
		case 1:
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[1], 12, "626");
			break;
		case 2:
		case 3:
			if (getCharacter(kCharacterAugust).clothes == 2) {
				Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[1], 12, "696");
			} else {
				Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[1], 12, "666");
			}

			break;
		case 4:
		case 5:
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[1], 12, "696");
			break;
		default:
			break;
		}

		Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[4], 12, (char *)&getCharacterCurrentParams(kCharacterAugust)[1]);
		if (getCharacterCurrentParams(kCharacterAugust)[0]) {
			Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterAugust)[4], 12, "Gc");
			setDoor(3, kCharacterCath, 1, 255, 255);
		} else {
			Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterAugust)[4], 12, "Ec");
		}

		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_DoCorrOtis, (char *)&getCharacterCurrentParams(kCharacterAugust)[4], 3, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 1) {
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[4], 12, (char *)&getCharacterCurrentParams(kCharacterAugust)[1]);
			Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterAugust)[4], 12, "Pc");
			startCycOtis(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[4]);
			softBlockAtDoor(kCharacterAugust, 3);
			if (_globals[kGlobalChapter] == 3 && _gameTime < 1998000) {
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
				AugustCall(&LogicManager::CONS_August_DoDialog, "AUG2094", 0, 0, 0);
			} else {
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
				AugustCall(&LogicManager::CONS_August_DoDialog, "AUG2095", 0, 0, 0);
			}
		} else if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] >= 2 && getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] <= 3) {
			send(kCharacterAugust, kCharacterCond1, 269436673, 0);
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterAugust)[4], 12, (char *)&getCharacterCurrentParams(kCharacterAugust)[1]);
			Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterAugust)[4], 12, "Qc");
			startCycOtis(kCharacterAugust, (char *)&getCharacterCurrentParams(kCharacterAugust)[4]);
		}

		break;
	case 69239528:
		setDoor(3, kCharacterCath, 1, 10, 9);
		softReleaseAtDoor(kCharacterAugust, 3);

		getCharacter(kCharacterAugust).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
		fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_CompLogic(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_CompLogic);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 21;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_CompLogic(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterAugust)[3] && getCharacterCurrentParams(kCharacterAugust)[0] < _gameTime && !getCharacterCurrentParams(kCharacterAugust)[6]) {
			getCharacterCurrentParams(kCharacterAugust)[6] = 1;
			setDoor(3, kCharacterCath, 0, 10, 9);

			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterAugust)[1]) {
			if (getCharacterCurrentParams(kCharacterAugust)[7] ||
				(getCharacterCurrentParams(kCharacterAugust)[7] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterAugust)[7] >= _realTime) {
					if (_globals[kGlobalChapter] == 1) {
						if (getCharacterCurrentParams(kCharacterAugust)[5] != 0) {
							if (getCharacterCurrentParams(kCharacterAugust)[8] ||
								(getCharacterCurrentParams(kCharacterAugust)[8] = _gameTime + 6300, _gameTime != -6300)) {
								if (getCharacterCurrentParams(kCharacterAugust)[8] >= _gameTime) {

									if (!getCharacterCurrentParams(kCharacterAugust)[3] && !_globals[kGlobalMetAugust] && !getCharacterCurrentParams(kCharacterAugust)[5] && getCharacterCurrentParams(kCharacterAugust)[0] - 4500 > _gameTime && !_globals[kGlobalCharacterSearchingForCath]) {
										_globals[kGlobalCharacterSearchingForCath] = kCharacterAugust;
										getCharacter(kCharacterAugust).characterPosition.location = 0;
										getCharacter(kCharacterAugust).characterPosition.position = 8200;
										getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
										AugustCall(&LogicManager::CONS_August_ExitComp, 0, 0, 0, 0);
									}

									break;
								}

								getCharacterCurrentParams(kCharacterAugust)[8] = 0x7FFFFFFF;
							}

							getCharacterCurrentParams(kCharacterAugust)[5] = 0;
							getCharacterCurrentParams(kCharacterAugust)[8] = 0;
						}

						if (!getCharacterCurrentParams(kCharacterAugust)[3] && !_globals[kGlobalMetAugust] && !getCharacterCurrentParams(kCharacterAugust)[5] && getCharacterCurrentParams(kCharacterAugust)[0] - 4500 > _gameTime && !_globals[kGlobalCharacterSearchingForCath]) {
							_globals[kGlobalCharacterSearchingForCath] = kCharacterAugust;
							getCharacter(kCharacterAugust).characterPosition.location = 0;
							getCharacter(kCharacterAugust).characterPosition.position = 8200;
							getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
							AugustCall(&LogicManager::CONS_August_ExitComp, 0, 0, 0, 0);
						}
					}

					break;
				}

				getCharacterCurrentParams(kCharacterAugust)[7] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterAugust)[1] = 0;
			getCharacterCurrentParams(kCharacterAugust)[2] = 1;

			if (_globals[kGlobalMetAugust] || _globals[kGlobalJacket] != 2) {
				setDoor(3, kCharacterAugust, 1, 0, 0);
			} else {
				setDoor(3, kCharacterAugust, 1, 0, 9);
			}
		}

		getCharacterCurrentParams(kCharacterAugust)[7] = 0;

		if (_globals[kGlobalChapter] == 1) {
			if (getCharacterCurrentParams(kCharacterAugust)[5] != 0) {
				if (getCharacterCurrentParams(kCharacterAugust)[8] ||
					(getCharacterCurrentParams(kCharacterAugust)[8] = _gameTime + 6300, _gameTime != -6300)) {
					if (getCharacterCurrentParams(kCharacterAugust)[8] >= _gameTime) {

						if (!getCharacterCurrentParams(kCharacterAugust)[3] && !_globals[kGlobalMetAugust] && !getCharacterCurrentParams(kCharacterAugust)[5] && getCharacterCurrentParams(kCharacterAugust)[0] - 4500 > _gameTime && !_globals[kGlobalCharacterSearchingForCath]) {
							_globals[kGlobalCharacterSearchingForCath] = kCharacterAugust;
							getCharacter(kCharacterAugust).characterPosition.location = 0;
							getCharacter(kCharacterAugust).characterPosition.position = 8200;
							getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
							AugustCall(&LogicManager::CONS_August_ExitComp, 0, 0, 0, 0);
						}

						break;
					}

					getCharacterCurrentParams(kCharacterAugust)[8] = 0x7FFFFFFF;
				}

				getCharacterCurrentParams(kCharacterAugust)[5] = 0;
				getCharacterCurrentParams(kCharacterAugust)[8] = 0;
			}

			if (!getCharacterCurrentParams(kCharacterAugust)[3] && !_globals[kGlobalMetAugust] && !getCharacterCurrentParams(kCharacterAugust)[5] && getCharacterCurrentParams(kCharacterAugust)[0] - 4500 > _gameTime && !_globals[kGlobalCharacterSearchingForCath]) {
				_globals[kGlobalCharacterSearchingForCath] = kCharacterAugust;
				getCharacter(kCharacterAugust).characterPosition.location = 0;
				getCharacter(kCharacterAugust).characterPosition.position = 8200;
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
				AugustCall(&LogicManager::CONS_August_ExitComp, 0, 0, 0, 0);
			}
		}

		break;
	case 8:
	case 9:
		if (msg->action == 9 && _globals[kGlobalChapter] == 1 && !_globals[kGlobalMetAugust] && _globals[kGlobalJacket] == 2) {
			setDoor(9, kCharacterCath, 0, 255, 255);
			getCharacter(kCharacterCath).characterPosition.location = 1;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 6;
			AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventMeetAugustHisCompartment, 0, 0);
		} else if (getCharacterCurrentParams(kCharacterAugust)[1]) {
			setDoor(3, kCharacterAugust, 1, 0, 0);
			if (cathHasItem(kItemPassengerList)) {
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 12;
				if (rnd(2)) {
					AugustCall(&LogicManager::CONS_August_DoDialog, getCathSorryDialog(), 0, 0, 0);
				} else {
					if (rnd(2) == 0) {
						AugustCall(&LogicManager::CONS_August_DoDialog, "CAT1502A", 0, 0, 0);
					} else {
						AugustCall(&LogicManager::CONS_August_DoDialog, "CAT1502", 0, 0, 0);
					}
				}
			} else {
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 13;
				AugustCall(&LogicManager::CONS_August_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else {
			setDoor(3, kCharacterAugust, 1, 0, 0);
			if (msg->action == 8) {
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 7;
				AugustCall(&LogicManager::CONS_August_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 8;
				AugustCall(&LogicManager::CONS_August_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		setDoor(3, kCharacterAugust, 1, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterAugust)[2] || getCharacterCurrentParams(kCharacterAugust)[1]) {
			setDoor(3, kCharacterAugust, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAugust)[2] = 0;
			getCharacterCurrentParams(kCharacterAugust)[1] = 0;
			getCharacterCurrentParams(kCharacterAugust)[4] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoWalk, 3, 8200, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_KnockTyler, getCharacterCurrentParams(kCharacterAugust)[0] - 2700, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_DoWalk, 3, 6470, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			AugustCall(&LogicManager::CONS_August_EnterComp, 0, 0, 0, 0);
			break;
		case 5:
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterAugust)
				_globals[kGlobalCharacterSearchingForCath] = 0;

			getCharacterCurrentParams(kCharacterAugust)[5] = 1;
			getCharacterCurrentParams(kCharacterAugust)[1] = 0;
			getCharacterCurrentParams(kCharacterAugust)[2] = 0;
			setDoor(3, kCharacterAugust, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAugust)[4] = 0;
			getCharacterCurrentParams(kCharacterAugust)[8] = 0;
			break;
		case 6:
			if (getModel(3) == 1)
				playNIS(kEventMeetAugustHisCompartmentBed);
			else
				playNIS(kEventMeetAugustHisCompartment);

			_globals[kGlobalMetAugust] = 1;
			setDoor(3, 2, 1, 0, 0);
			getCharacterCurrentParams(kCharacterAugust)[1] = 0;
			getCharacterCurrentParams(kCharacterAugust)[2] = 1;
			bumpCathFDoor(3);
			break;
		case 7:
		case 8:
			getCharacterCurrentParams(kCharacterAugust)[4]++;
			switch (getCharacterCurrentParams(kCharacterAugust)[4]) {
			case 1:
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 9;
				if (rnd(2) == 0) {
					AugustCall(&LogicManager::CONS_August_DoDialog, "AUG1128B", 0, 0, 0);
				} else {
					AugustCall(&LogicManager::CONS_August_DoDialog, "AUG1128A", 0, 0, 0);
				}

				break;
			case 2:
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 10;
				if (!_globals[kGlobalMetAugust]) {
					AugustCall(&LogicManager::CONS_August_DoDialog, "AUG1128G", 0, 0, 0);
				} else {
					AugustCall(&LogicManager::CONS_August_DoDialog, "AUG1128E", 0, 0, 0);
				}

				break;
			case 3:
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 11;
				if (!_globals[kGlobalMetAugust]) {
					AugustCall(&LogicManager::CONS_August_DoDialog, "AUG1128H", 0, 0, 0);
				} else {
					AugustCall(&LogicManager::CONS_August_DoDialog, "AUG1128F", 0, 0, 0);
				}

				break;
			default:
				if (_globals[kGlobalMetAugust] || _globals[kGlobalJacket] != 2) {
					setDoor(3, kCharacterAugust, 1, 14, 0);
				} else {
					setDoor(3, kCharacterAugust, 1, 14, 9);
				}

				getCharacterCurrentParams(kCharacterAugust)[1] = 1;
				break;
			}

			break;
		case 9:
		case 10:
		case 11:
			if (_globals[kGlobalMetAugust] || _globals[kGlobalJacket] != 2) {
				setDoor(3, kCharacterAugust, 1, 14, 0);
			} else {
				setDoor(3, kCharacterAugust, 1, 14, 9);
			}

			getCharacterCurrentParams(kCharacterAugust)[1] = 1;
			break;
		case 12:
		case 13:
			if (_globals[kGlobalMetAugust] || _globals[kGlobalJacket] != 2) {
				setDoor(3, kCharacterAugust, 1, 0, 0);
			} else {
				setDoor(3, kCharacterAugust, 1, 0, 9);
			}

			getCharacterCurrentParams(kCharacterAugust)[1] = 0;
			getCharacterCurrentParams(kCharacterAugust)[2] = 1;
			break;
		case 14:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 15;
			AugustCall(&LogicManager::CONS_August_DoWait, 75, 0, 0, 0);
			break;
		case 15:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 16;
			AugustCall(&LogicManager::CONS_August_DoDialog, "AUG1128I", 0, 0, 0);
			break;
		case 16:
			send(kCharacterAugust, kCharacterCond1, 100906246, 0);
			break;
		case 17:
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			send(kCharacterAugust, kCharacterCond1, 156567128, 0);
			startCycOtis(kCharacterAugust, "626Lc");
			softBlockAtDoor(kCharacterAugust, 3);
			break;
		case 18:
			softReleaseAtDoor(kCharacterAugust, 3);
			getCharacter(kCharacterAugust).characterPosition.location = 6470;
			endGraphics(kCharacterAugust);
			setDoor(3, kCharacterAugust, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAugust)[3] = 0;
			break;
		default:
			break;
		}

		break;
	case 124697504:
		playDialog(kCharacterAugust, "CON1023A", -1, 0);
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 18;
		AugustCall(&LogicManager::CONS_August_DoCorrOtis, "626Mc", 3, 0, 0);
		break;
	case 192849856:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 17;
		AugustCall(&LogicManager::CONS_August_DoCorrOtis, "626Kc", 3, 0, 0);
		break;
	case 221617184:
		getCharacterCurrentParams(kCharacterAugust)[3] = 1;
		send(kCharacterAugust, kCharacterCond1, 102675536, 0);
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 14;
		AugustCall(&LogicManager::CONS_August_DoDialog, "CON1023", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Birth);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 22;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterAugust)[0]) {
			getCharacterCurrentParams(kCharacterAugust)[0] = 1;
			CONS_August_WaitTyler(0, 0, 0, 0);
		}

		break;
	case 12:
		setDoor(3, kCharacterCath, 1, 10, 9);
		setDoor(11, kCharacterCath, 0, 255, 255);
		getCharacter(kCharacterAugust).characterPosition.position = 4691;
		getCharacter(kCharacterAugust).characterPosition.location = 1;
		getCharacter(kCharacterAugust).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterAugust).clothes = 0;
		_globals[kGlobalMetAugust] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_KnockTyler(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_KnockTyler);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 23;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_KnockTyler(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalCharacterSearchingForCath] == kCharacterPolice || _globals[kGlobalCharacterSearchingForCath] == kCharacterCond1) {
			if (getCharacterCurrentParams(kCharacterAugust)[2]) {
				getCharacter(kCharacterAugust).characterPosition.location = 0;
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
				AugustCall(&LogicManager::CONS_August_DoCorrOtis, "626Ea", 1, 0, 0);
			} else {
				softReleaseAtDoor(kCharacterAugust, 1);
				setDoor(1, kCharacterCath, 0, 10, 9);

				getCharacter(kCharacterAugust).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
				fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			}

			break;
		}

		if (!getCharacterCurrentParams(kCharacterAugust)[1]) {
			if ((!getCharacterCurrentParams(kCharacterAugust)[10] &&
				(getCharacterCurrentParams(kCharacterAugust)[10] = _realTime + 75, _realTime == -75)) ||
				getCharacterCurrentParams(kCharacterAugust)[10] < _realTime) {
				if (!getCharacterCurrentParams(kCharacterAugust)[4]) {
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 8;
					AugustCall(&LogicManager::CONS_August_DoDialog, "AUG1002B", 0, 0, 0);
					break;
				}

				if (!getCharacterCurrentParams(kCharacterAugust)[11]) {
					getCharacterCurrentParams(kCharacterAugust)[11] = _realTime + 75;
					if (_realTime == -75) {
						softReleaseAtDoor(kCharacterAugust, 1);
						if (_globals[kGlobalCorpseMovedFromFloor]) {
							getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 9;
							AugustCall(&LogicManager::CONS_August_DoCorrOtis, "626Da", 1, 0, 0);
						} else if (checkLoc(kCharacterCath, kCarGreenSleeping)) {
							getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 10;
							AugustCall(&LogicManager::CONS_August_DoBriefCorrOtis, "626Da", 1, 0, 0);
						} else {
							bumpCath(kCarNone, 1, 255);
							setDoor(9, kCharacterCath, 0, 255, 255);
							getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 11;
							AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustFindCorpse, 0, 0);
						}

						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterAugust)[11] < _realTime) {
					getCharacterCurrentParams(kCharacterAugust)[11] = 0x7FFFFFFF;
					softReleaseAtDoor(kCharacterAugust, 1);
					if (_globals[kGlobalCorpseMovedFromFloor]) {
						getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 9;
						AugustCall(&LogicManager::CONS_August_DoCorrOtis, "626Da", 1, 0, 0);
					} else if (checkLoc(kCharacterCath, kCarGreenSleeping)) {
						getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 10;
						AugustCall(&LogicManager::CONS_August_DoBriefCorrOtis, "626Da", 1, 0, 0);
					} else {
						bumpCath(kCarNone, 1, 255);
						setDoor(9, kCharacterCath, 0, 255, 255);
						getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 11;
						AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustFindCorpse, 0, 0);
					}

					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterAugust)[2] && getCharacterCurrentParams(kCharacterAugust)[0] < _gameTime && !getCharacterCurrentParams(kCharacterAugust)[12]) {
				getCharacterCurrentParams(kCharacterAugust)[12] = 1;
				setDoor(1, kCharacterCath, 0, 10, 9);
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 12;
				AugustCall(&LogicManager::CONS_August_DoCorrOtis, "626Ea", 1, 0, 0);
			}

			break;
		}

		if ((!getCharacterCurrentParams(kCharacterAugust)[8] && (getCharacterCurrentParams(kCharacterAugust)[8] = _realTime + 45, _realTime == -45)) || getCharacterCurrentParams(kCharacterAugust)[8] < _realTime) {
			if (checkDoor(1) == 1) {
				if (getCharacterCurrentParams(kCharacterAugust)[9] || (getCharacterCurrentParams(kCharacterAugust)[9] = _realTime + 75, _realTime != -75)) {
					if (getCharacterCurrentParams(kCharacterAugust)[9] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterAugust)[9] = 0x7FFFFFFF;
				}

				setDoor(1, kCharacterAugust, checkDoor(1), 0, 0);

				getCharacterCurrentParams(kCharacterAugust)[5]++;
				switch (getCharacterCurrentParams(kCharacterAugust)[5]) {
				case 1:
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
					AugustCall(&LogicManager::CONS_August_DoDialog, "LIB013", 0, 0, 0);
					break;
				case 2:
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 7;
					AugustCall(&LogicManager::CONS_August_DoDialog, "LIB012", 0, 0, 0);
					break;
				case 3:
					getCharacterCurrentParams(kCharacterAugust)[7]++;
					if (getCharacterCurrentParams(kCharacterAugust)[7] >= 3) {
						setDoor(1, kCharacterCath, checkDoor(1), 10, 9);

						getCharacter(kCharacterAugust).currentCall--;
						_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
						fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
						break;
					}

					getCharacterCurrentParams(kCharacterAugust)[5] = 0;
					break;
				}

				setDoor(1, kCharacterAugust, checkDoor(1), getCharacterCurrentParams(kCharacterAugust)[3] == 0 ? 14 : 0, 9);
				getCharacterCurrentParams(kCharacterAugust)[9] = 0;
			} else if (!_globals[kGlobalCorpseMovedFromFloor] || _globals[kGlobalJacket] == 1) {
				setDoor(9, 0, 0, 255, 255);
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
				AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustFindCorpse, 0, 0);
			} else {
				getCharacterCurrentParams(kCharacterAugust)[6] = 8;
				if (getModel(1) != 1)
					getCharacterCurrentParams(kCharacterAugust)[6] = 7;
				setDoor(9, kCharacterCath, 0, 255, 255);
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
				AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventMeetAugustTylerCompartment, 0, 0);
			}
		}

		break;
	case 8:
		if (getCharacterCurrentParams(kCharacterAugust)[2]) {
			setDoor(1, kCharacterAugust, 0, 0, 0);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 15;
			AugustCall(&LogicManager::CONS_August_DoDialog, "LIB012", 0, 0, 0);
		} else if (!getCharacterCurrentParams(kCharacterAugust)[3]) {
			setDoor(1, kCharacterAugust, checkDoor(1), 0, 0);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 17;
			AugustCall(&LogicManager::CONS_August_DoDialogFullVol, "AUG1002A", 0, 0, 0);
		}

		break;
	case 9:
		if (!_globals[kGlobalCorpseMovedFromFloor] || _globals[kGlobalJacket] == 1) {
			setDoor(9, kCharacterCath, 0, 255, 255);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 13;
			AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustFindCorpse, 0, 0);
		} else {
			if (getCharacterCurrentParams(kCharacterAugust)[2]) {
				getCharacter(kCharacterCath).characterPosition.location = 1;
				if (getModel(1) == 1)
					getCharacterCurrentParams(kCharacterAugust)[6] = 10;
				else
					getCharacterCurrentParams(kCharacterAugust)[6] = 9;
			} else {
				getCharacterCurrentParams(kCharacterAugust)[6] = 8;
				if (getModel(1) != 1)
					getCharacterCurrentParams(kCharacterAugust)[6] = 7;
			}
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 14;
			AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventMeetAugustTylerCompartment, 0, 0);
		}

		break;
	case 12:
		if (inComp(kCharacterCath, kCarGreenSleeping, 8200) || inComp(kCharacterCath, kCarGreenSleeping, 7850) || cathOutHisWindow()) {
			setDoor(1, kCharacterAugust, checkDoor(1), 0, 0);
			if (cathOutHisWindow())
				bumpCath(kCarGreenSleeping, 49, 255);

			playDialog(0, "LIB012", -1, 0);

			setDoor(1, kCharacterAugust, checkDoor(1), 14, 9);
			getCharacterCurrentParams(kCharacterAugust)[1] = 1;
		} else {
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
			AugustCall(&LogicManager::CONS_August_DoCorrOtis, "626Aa", 1, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterAugust, "626Ba");
			softBlockAtDoor(kCharacterAugust, 1);
			break;
		case 2:
			setDoor(1, kCharacterCath, 0, 10, 9);

			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			break;
		case 3:
			playDialog(0, "LIB014", -1, 0);
			playNIS(kEventAugustFindCorpse);

			if (_doneNIS[kEventDinerAugustOriginalJacket]) {
				endGame(3, 4, _globals[kGlobalFoundCorpse] == 0 ? 56 : 50, true);
			} else if (_globals[kGlobalCorpseMovedFromFloor]) {
				endGame(0, 1, 55, true);
			} else {
				endGame(0, 1, _globals[kGlobalFoundCorpse] == 0 ? 56 : 50, true);
			}

			break;
		case 4:
			setDoor(1, kCharacterCath, 0, 10, 9);
			playDialog(0, "LIB014", -1, 0);
			endGraphics(kCharacterAugust);
			getCharacter(kCharacterAugust).characterPosition.location = 1;
			playNIS(getCharacterCurrentParams(kCharacterAugust)[6]);
			playDialog(0, "LIB015", -1, 0);
			_globals[kGlobalMetAugust] = 1;
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			bumpCathTylerComp();

			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			break;
		case 5:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 6;
			AugustCall(&LogicManager::CONS_August_DoDialogFullVol, "AUG1002B", 0, 0, 0);
			break;
		case 6:
		case 7:
			setDoor(1, kCharacterAugust, checkDoor(1), getCharacterCurrentParams(kCharacterAugust)[3] == 0 ? 14 : 0, 9);
			getCharacterCurrentParams(kCharacterAugust)[9] = 0;
			break;
		case 8:
			getCharacterCurrentParams(kCharacterAugust)[4] = 1;
			if (!getCharacterCurrentParams(kCharacterAugust)[11]) {
				getCharacterCurrentParams(kCharacterAugust)[11] = _realTime + 75;
				if (_realTime == -75) {
					softReleaseAtDoor(kCharacterAugust, 1);
					if (_globals[kGlobalCorpseMovedFromFloor]) {
						getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 9;
						AugustCall(&LogicManager::CONS_August_DoCorrOtis, "626Da", 1, 0, 0);
					} else if (checkLoc(kCharacterCath, kCarGreenSleeping)) {
						getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 10;
						AugustCall(&LogicManager::CONS_August_DoBriefCorrOtis, "626Da", 1, 0, 0);
					} else {
						bumpCath(kCarNone, 1, 255);
						setDoor(9, kCharacterCath, 0, 255, 255);
						getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 11;
						AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustFindCorpse, 0, 0);
					}

					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterAugust)[11] < _realTime) {
				getCharacterCurrentParams(kCharacterAugust)[11] = 0x7FFFFFFF;
				softReleaseAtDoor(kCharacterAugust, 1);
				if (_globals[kGlobalCorpseMovedFromFloor]) {
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 9;
					AugustCall(&LogicManager::CONS_August_DoCorrOtis, "626Da", 1, 0, 0);
				} else if (checkLoc(kCharacterCath, kCarGreenSleeping)) {
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 10;
					AugustCall(&LogicManager::CONS_August_DoBriefCorrOtis, "626Da", 1, 0, 0);
				} else {
					bumpCath(kCarNone, 1, 255);
					setDoor(9, kCharacterCath, 0, 255, 255);
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 11;
					AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustFindCorpse, 0, 0);
				}

				break;
			}

			if (getCharacterCurrentParams(kCharacterAugust)[2] && getCharacterCurrentParams(kCharacterAugust)[0] < _gameTime && !getCharacterCurrentParams(kCharacterAugust)[12]) {
				getCharacterCurrentParams(kCharacterAugust)[12] = 1;
				setDoor(1, kCharacterCath, 0, 10, 9);
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 12;
				AugustCall(&LogicManager::CONS_August_DoCorrOtis, "626Ea", 1, 0, 0);
			}

			break;
		case 9:
			getCharacterCurrentParams(kCharacterAugust)[2] = 1;
			endGraphics(kCharacterAugust);
			getCharacter(kCharacterAugust).characterPosition.location = 1;
			setDoor(1, kCharacterAugust, 0, 10, 9);
			if (getCharacterCurrentParams(kCharacterAugust)[2] && getCharacterCurrentParams(kCharacterAugust)[0] < _gameTime && !getCharacterCurrentParams(kCharacterAugust)[12]) {
				getCharacterCurrentParams(kCharacterAugust)[12] = 1;
				setDoor(1, kCharacterCath, 0, 10, 9);
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 12;
				AugustCall(&LogicManager::CONS_August_DoCorrOtis, "626Ea", 1, 0, 0);
			}

			break;
		case 10:
			setDoor(9, kCharacterCath, 0, 255, 255);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 11;
			AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustFindCorpse, 0, 0);
			break;
		case 11:
			playNIS(kEventAugustFindCorpse);
			if (_doneNIS[kEventDinerAugustOriginalJacket]) {
				endGame(3, 4, _globals[kGlobalFoundCorpse] == 0 ? 56 : 50, true);
			} else {
				endGame(0, 1, _globals[kGlobalFoundCorpse] == 0 ? 56 : 50, true);
			}

			break;
		case 12:
			getCharacter(kCharacterAugust).characterPosition.location = 0;

			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			break;
		case 13:
			if (checkDoor(1) != 1) {
				playDialog(0, "LIB014", -1, 0);
			} else {
				playDialog(0, "LIB032", -1, 0);
			}

			playNIS(kEventAugustFindCorpse);

			if (_doneNIS[kEventDinerAugustOriginalJacket]) {
				endGame(3, 4, _globals[kGlobalFoundCorpse] == 0 ? 56 : 50, true);
			} else if (_globals[kGlobalCorpseMovedFromFloor]) {
				endGame(0, 1, 55, true);
			} else {
				endGame(0, 1, _globals[kGlobalFoundCorpse] == 0 ? 56 : 50, true);
			}

			break;
		case 14:
			if (!getCharacterCurrentParams(kCharacterAugust)[2]) {
				if (checkDoor(1) != 1) {
					playDialog(0, "LIB014", -1, 0);
				} else {
					playDialog(0, "LIB032", -1, 0);
				}
			}

			setDoor(1, kCharacterCath, 0, 10, 9);
			setDoor(9, kCharacterCath, 0, 255, 255);
			playNIS(getCharacterCurrentParams(kCharacterAugust)[6]);
			_globals[kGlobalMetAugust] = 1;
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			bumpCathTylerComp();

			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			break;
		case 15:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 16;
			AugustCall(&LogicManager::CONS_August_DoDialog, "AUG1128A", 0, 0, 0);
			break;
		case 16:
			setDoor(1, kCharacterAugust, 0, 10, 9);
			break;
		case 17:
			getCharacterCurrentParams(kCharacterAugust)[3] = 1;
			setDoor(1, kCharacterAugust, checkDoor(1), 0, 9);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoNIS1006(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoNIS1006);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 24;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoNIS1006(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventDinerAugust, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 1) {
			if (inDiningRoom(kCharacterAlexei))
				playNIS(kEventDinerAugustAlexeiBackground);
			else
				playNIS(kEventDinerAugust);

			_globals[kGlobalMetAugust] = 1;
			bumpCath(kCarRestaurant, 61, 255);

			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_WaitTyler(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_WaitTyler);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 25;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_WaitTyler(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterAugust)[0] && _globals[kGlobalFoundCorpse]) {
			send(kCharacterAugust, kCharacterHeadWait, 239072064, 0);
			getCharacterCurrentParams(kCharacterAugust)[0] = 1;
		}

		if (_gameTime > 1080000 && !getCharacterCurrentParams(kCharacterAugust)[2]) {
			getCharacterCurrentParams(kCharacterAugust)[2] = 1;
			if (!getCharacterCurrentParams(kCharacterAugust)[0]) {
				send(kCharacterAugust, kCharacterHeadWait, 239072064, 0);
				getCharacterCurrentParams(kCharacterAugust)[0] = 1;
			}
		}

		if (_gameTime > 1093500 && rcClear()) {
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).inventoryItem = 0;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
			AugustCall(&LogicManager::CONS_August_DoJoinedSeqOtis, "010J", 36, 103798704, "010K");
		}

		break;
	case 1:
		getCharacterCurrentParams(kCharacterAugust)[1] = 0;
		getCharacter(kCharacterAugust).inventoryItem = 0;
		send(kCharacterAugust, kCharacterHeadWait, 191604416, 0);
		if (_globals[kGlobalJacket] == 2) {
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_DoNIS1006, 0, 0, 0, 0);
		} else {
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventDinerAugustOriginalJacket, 0, 0);
		}

		break;
	case 12:
		send(kCharacterAugust, kCharacterTableD, 136455232, 0);
		startCycOtis(kCharacterAugust, "010B");

		if (!_globals[kGlobalMetAugust])
			getCharacterCurrentParams(kCharacterAugust)[1] = 128;

		getCharacter(kCharacterAugust).inventoryItem = getCharacterCurrentParams(kCharacterAugust)[1];
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			send(kCharacterAugust, kCharacterWaiter1, 204704037, 0);
			startSeqOtis(kCharacterAugust, "803DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterAugust);

			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 2:
			CONS_August_SeekTyler(0, 0, 0, 0);
			break;
		case 3:
			CONS_August_OrderDinner(0, 0, 0, 0);
			break;
		case 4:
			send(kCharacterAugust, kCharacterAlexei, 225182640, 0);
			playNIS(kEventDinerAugustOriginalJacket);
			setDoor(1, kCharacterCath, 3, 0, 0);
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			send(kCharacterAugust, kCharacterTableD, 103798704, "010K");
			startSeqOtis(kCharacterAugust, "010P");
			bumpCath(kCarRestaurant, 65, 255);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			AugustCall(&LogicManager::CONS_August_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
			send(kCharacterAugust, kCharacterWaiter1, 204704037, 0);
			startSeqOtis(kCharacterAugust, "803DS");
			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterAugust);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 6;
			AugustCall(&LogicManager::CONS_August_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 6:
			_globals[kGlobalCharacterSearchingForCath] = kCharacterAugust;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 7;
			AugustCall(&LogicManager::CONS_August_DoWalk, 3, 8200, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 8;
			AugustCall(&LogicManager::CONS_August_KnockTyler, 0, 0, 0, 0);
			break;
		case 8:
			endGame(0, 0, 0, 1);
			break;
		default:
			break;
		}

		break;
	case 168046720:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		break;
	case 168627977:
		getCharacter(kCharacterAugust).inventoryItem = getCharacterCurrentParams(kCharacterAugust)[1];
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_SeekTyler(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_SeekTyler);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 26;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_SeekTyler(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (_globals[kGlobalMetAugust] || _globals[kGlobalCharacterSearchingForCath]) {
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			AugustCall(&LogicManager::CONS_August_DoWalk, 3, 6470, 0, 0);
		} else {
			_globals[kGlobalCharacterSearchingForCath] = kCharacterAugust;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
			AugustCall(&LogicManager::CONS_August_DoWalk, 3, 8200, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_KnockTyler, _gameTime + 13500, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_DoWalk, 3, 6470, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_EnterComp, 0, 0, 0, 0);
			break;
		case 4:
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterAugust)
				_globals[kGlobalCharacterSearchingForCath] = 0;

			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 7;
			AugustCall(&LogicManager::CONS_August_CompLogic, _gameTime + 900, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 6;
			AugustCall(&LogicManager::CONS_August_EnterComp, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 7;
			AugustCall(&LogicManager::CONS_August_CompLogic, _gameTime + 900, 0, 0, 0);
			break;
		case 7:
			CONS_August_GotoDinner(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_GotoDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_GotoDinner);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 27;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_GotoDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_ExitComp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAugust).characterPosition.position = 1540;
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_DoSeqOtis, "803US", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterAugust, "010A");
			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterAugust);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			AugustCall(&LogicManager::CONS_August_FinishJoinedSeqOtis, 36, 136455232, "BOGUS", 0);
			break;
		case 5:
			getCharacter(kCharacterAugust).characterPosition.location = 1;
			CONS_August_OrderDinner(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_OrderDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_OrderDinner);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 28;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_OrderDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 1:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacterCurrentParams(kCharacterAugust)[0] = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
		AugustCall(&LogicManager::CONS_August_DoNIS1006, 0, 0, 0, 0);
		break;
	case 12:
		if (!_globals[kGlobalMetAugust] && _globals[kGlobalJacket] == 2)
			getCharacterCurrentParams(kCharacterAugust)[0] = 128;

		startCycOtis(kCharacterAugust, "010B");
		send(kCharacterAugust, kCharacterWaiter1, 304061224, 0);
		getCharacter(kCharacterAugust).inventoryItem = getCharacterCurrentParams(kCharacterAugust)[0];
		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 1) {
			send(kCharacterAugust, kCharacterWaiter1, 203859488, 0);
			getCharacter(kCharacterAugust).inventoryItem = getCharacterCurrentParams(kCharacterAugust)[0];
			startCycOtis(kCharacterAugust, "010B");
		} else if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 2) {
			send(kCharacterAugust, kCharacterWaiter1, 136702400, 0);
			startCycOtis(kCharacterAugust, "010B");
			CONS_August_EatingDinner(0, 0, 0, 0);
		}

		break;
	case 168046720:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		break;
	case 168627977:
		getCharacter(kCharacterAugust).inventoryItem = getCharacterCurrentParams(kCharacterAugust)[0];
		break;
	case 170016384:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		startCycOtis(kCharacterWaiter1, "BLANK");
		startCycOtis(kCharacterAugust, "010G");
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
		AugustCall(&LogicManager::CONS_August_DoDialog, "AUG1053", 0, 0, 0);
		break;
	case 268773672:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		startCycOtis(kCharacterAugust, "010D");
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_DoDialog, "AUG1052", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_EatingDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_EatingDinner);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 29;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_EatingDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalAnnaIsEating] && !getCharacterCurrentParams(kCharacterAugust)[1] && getCharacterCurrentParams(kCharacterAugust)[2] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime <= 1134000) {
				if (!inDiningRoom(kCharacterCath) || dialogRunning("MRB1076") || dialogRunning("MRB1078") || dialogRunning("MRB1078A") || !getCharacterCurrentParams(kCharacterAugust)[2]) {
					getCharacterCurrentParams(kCharacterAugust)[2] = _gameTime + 225;
					if (_gameTime == -225) {
						getCharacter(kCharacterAugust).inventoryItem = 0;
						_globals[kGlobalAnnaIsEating] = 0;
						CONS_August_GreetAnna(0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterAugust)[2] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterAugust)[2] = 0x7FFFFFFF;
			getCharacter(kCharacterAugust).inventoryItem = 0;
			_globals[kGlobalAnnaIsEating] = 0;
			CONS_August_GreetAnna(0, 0, 0, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacterCurrentParams(kCharacterAugust)[0] = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_DoNIS1006, 0, 0, 0, 0);
		break;
	case 12:
		if (!_globals[kGlobalMetAugust] && _globals[kGlobalJacket] == 2)
			getCharacterCurrentParams(kCharacterAugust)[0] = 128;

		getCharacter(kCharacterAugust).inventoryItem = getCharacterCurrentParams(kCharacterAugust)[0];
		startCycOtis(kCharacterAugust, "010H");
		break;
	case 168046720:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		break;
	case 168627977:
		getCharacter(kCharacterAugust).inventoryItem = getCharacterCurrentParams(kCharacterAugust)[0];
		break;
	case 189426612:
		getCharacterCurrentParams(kCharacterAugust)[1] = 1;
		break;
	case 235257824:
		getCharacterCurrentParams(kCharacterAugust)[1] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_GreetAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_GreetAnna);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 30;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_GreetAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAugust)[2] || (getCharacterCurrentParams(kCharacterAugust)[2] = _realTime + 75, _realTime != -75)) {
			if (getCharacterCurrentParams(kCharacterAugust)[2] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterAugust)[2] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterAugust).inventoryItem = 0x80;
		break;
	case 1:
		getCharacterCurrentParams(kCharacterAugust)[0] = 1;
		getCharacter(kCharacterAugust).inventoryItem = 0;
		bumpCath(kCarRestaurant, 62, 255);
		blockView(kCharacterAugust, kCarRestaurant, 61);
		blockView(kCharacterAugust, kCarRestaurant, 64);
		break;
	case 2:
		if (getCharacterCurrentParams(kCharacterAugust)[0]) {
			getCharacter(kCharacterAugust).inventoryItem = 0;
			releaseView(kCharacterAugust, kCarRestaurant, 61);
			releaseView(kCharacterAugust, kCarRestaurant, 64);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustPresentAnna, 0, 0);
		} else if (getCharacterCurrentParams(kCharacterAugust)[1]) {
			getCharacterCurrentParams(kCharacterAugust)[1] = 0;

			if (_globals[kGlobalMetAugust])
				getCharacter(kCharacterAugust).inventoryItem = 0;

			playDialog(kCharacterAugust, "AUG1003A", -1, 0);
		} else {
			getCharacter(kCharacterAugust).inventoryItem = 0;
			send(kCharacterAugust, kCharacterAnna, 201437056, 0);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 8;
			AugustCall(&LogicManager::CONS_August_DoSeqOtis, "010P", 0, 0, 0);
		}

		break;
	case 12:
		send(kCharacterAugust, kCharacterMonsieur, 135854206, 0);
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_DoWait, 450, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_WaitRCClear, 0, 0, 0, 0);
			break;
		case 2:
			send(kCharacterAugust, kCharacterAnna, 259136835, 0);
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_DoJoinedSeqOtis, "010N", 36, 103798704, "010K");
			break;
		case 3:
			send(kCharacterAugust, kCharacterWaiter1, 292758554, 0);
			send(kCharacterAugust, kCharacterAnna, 122358304, 0);
			startCycOtis(kCharacterAugust, "001K");
			playDialog(kCharacterAugust, "AUG1003", -1, 0);

			if (inDiningRoom(kCharacterCath))
				_globals[kGlobalOverheardAugustInterruptingAnnaAtDinner] = 1;

			getCharacterCurrentParams(kCharacterAugust)[1] = 1;
			break;
		case 4:
			if (_globals[kGlobalMetAugust]) {
				playNIS(kEventAugustPresentAnna);
				send(kCharacterAugust, kCharacterAnna, 201437056, 0);
				startSeqOtis(kCharacterAugust, "803GS");
				bumpCath(kCarRestaurant, 55, 255);
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			} else {
				playNIS(kEventAugustPresentAnnaFirstIntroduction);
				send(kCharacterAugust, kCharacterAnna, 201437056, 0);
				startSeqOtis(kCharacterAugust, "010P");
				bumpCath(kCarRestaurant, 65, 255);
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 6;
			}

			AugustCall(&LogicManager::CONS_August_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
		case 7:
		case 9:
			send(kCharacterAugust, kCharacterMonsieur, 134466544, 0);
			CONS_August_ReturnFromDinner(0, 0, 0, 0);
			break;
		case 6:
			startSeqOtis(kCharacterAugust, "803DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterAugust);

			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 7;
			AugustCall(&LogicManager::CONS_August_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 8:
			startSeqOtis(kCharacterAugust, "803DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterAugust);

			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 9;
			AugustCall(&LogicManager::CONS_August_FinishSeqOtis, 0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_ReturnFromDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_ReturnFromDinner);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 31;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_ReturnFromDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_DoWalk, 3, 6470, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_EnterComp, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_CompLogic, 1161000, 0, 0, 0);
			break;
		case 3:
		case 4:
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterPolice) {
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
				AugustCall(&LogicManager::CONS_August_CompLogic, _gameTime + 900, 0, 0, 0);
			} else {
				CONS_August_GoSalon(0, 0, 0, 0);
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

void LogicManager::CONS_August_GoSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_GoSalon);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 32;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_GoSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAugust)[5] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime > 1179000) {
				getCharacterCurrentParams(kCharacterAugust)[5] = 0x7FFFFFFF;
				send(kCharacterAugust, kCharacterAnna, 123712592, 0);
			} else {
				bool skip = false; // Horrible way to unroll a goto...

				if (!inSalon(kCharacterAnna) || inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterAugust)[5]) {
					getCharacterCurrentParams(kCharacterAugust)[5] = _gameTime;
					if (!_gameTime) {
						skip = true;
						send(kCharacterAugust, kCharacterAnna, 123712592, 0);
					}
				}

				if (!skip && getCharacterCurrentParams(kCharacterAugust)[5] < _gameTime) {
					getCharacterCurrentParams(kCharacterAugust)[5] = 0x7FFFFFFF;
					send(kCharacterAugust, kCharacterAnna, 123712592, 0);
				}
			}
		}

		if (getCharacterCurrentParams(kCharacterAugust)[0] && rcClear()) {
			if (!getCharacterCurrentParams(kCharacterAugust)[3]) {
				getCharacterCurrentParams(kCharacterAugust)[3] = _gameTime + 1800;
				getCharacterCurrentParams(kCharacterAugust)[4] = _gameTime + 9000;
			}

			if (getCharacterCurrentParams(kCharacterAugust)[6] != 0x7FFFFFFF && getCharacterCurrentParams(kCharacterAugust)[3] < _gameTime) {
				if (getCharacterCurrentParams(kCharacterAugust)[4] < _gameTime) {
					getCharacterCurrentParams(kCharacterAugust)[6] = 0x7FFFFFFF;
					getCharacter(kCharacterAugust).characterPosition.location = 0;
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
					AugustCall(&LogicManager::CONS_August_DoBlockSeqOtis, "109D", 5, 0x38, 0);
					break;
				}

				if (inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterAugust)[6]) {
					getCharacterCurrentParams(kCharacterAugust)[6] = _gameTime;
					if (!_gameTime) {
						getCharacter(kCharacterAugust).characterPosition.location = 0;
						getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
						AugustCall(&LogicManager::CONS_August_DoBlockSeqOtis, "109D", 5, 0x38, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterAugust)[6] < _gameTime) {
					getCharacterCurrentParams(kCharacterAugust)[6] = 0x7FFFFFFF;
					getCharacter(kCharacterAugust).characterPosition.location = 0;
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
					AugustCall(&LogicManager::CONS_August_DoBlockSeqOtis, "109D", 5, 0x38, 0);
					break;
				}
			}
		}

		if (getCharacterCurrentParams(kCharacterAugust)[2]) {
			if (getCharacterCurrentParams(kCharacterAugust)[7] || (getCharacterCurrentParams(kCharacterAugust)[7] = _realTime + 90, _realTime != -90)) {
				if (getCharacterCurrentParams(kCharacterAugust)[7] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAugust)[7] = 0x7FFFFFFF;
			}

			bumpCath(kCarRestaurant, 55, 255);
		} else {
			getCharacterCurrentParams(kCharacterAugust)[7] = 0;
		}

		break;
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_ExitComp, 0, 0, 0, 0);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterAugust)[1]) {
			if (checkCathDir(kCarRestaurant, 57)) {
				bumpCath(kCarRestaurant, 50, 255);
				getCharacterCurrentParams(kCharacterAugust)[2] = 1;
			} else if (!checkCathDir(kCarRestaurant, 50)) {
				getCharacterCurrentParams(kCharacterAugust)[2] = 0;
			}
		} else {
			getCharacterCurrentParams(kCharacterAugust)[2] = (checkCathDir(kCarRestaurant, 56) && getCharacterCurrentParams(kCharacterAugust)[0]) ? 1 : 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAugust).characterPosition.position = 1540;
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_DoBlockSeqOtis, "105A", 5, 57, 0);
			break;
		case 4:
			getCharacter(kCharacterAugust).characterPosition.location = 1;
			startCycOtis(kCharacterAugust, "105B");
			getCharacterCurrentParams(kCharacterAugust)[1] = 1;
			break;
		case 5:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 6;
			AugustCall(&LogicManager::CONS_August_DoWalk, 3, 6470, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 7;
			AugustCall(&LogicManager::CONS_August_EnterComp, 0, 0, 0, 0);
			break;
		case 7:
			CONS_August_BackFromSalon(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 122358304:
		getCharacterCurrentParams(kCharacterAugust)[1] = 0;
		startCycOtis(kCharacterAugust, "BLANK");
		break;
	case 159332865:
		startCycOtis(kCharacterAugust, "106E");
		getCharacterCurrentParams(kCharacterAugust)[0] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_BackFromSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_BackFromSalon);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 33;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_BackFromSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (_globals[kGlobalMetAugust]) {
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
			AugustCall(&LogicManager::CONS_August_CompLogic, _gameTime + 9000, 0, 0, 0);
		} else {
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_CompLogic, 1404000, 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
			if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] <= 2)
				CONS_August_Asleep(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Asleep);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 34;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!whoRunningDialog(kCharacterAugust) && _globals[kGlobalPhaseOfTheNight] != 4) {
			playDialog(kCharacterAugust, "AUG1057", -1, 0);
		}

		break;
	case 12:
		setDoor(3, kCharacterCath, 1, 10, 9);
		getCharacter(kCharacterAugust).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAugust).characterPosition.position = 6470;
		getCharacter(kCharacterAugust).characterPosition.location = 1;
		endGraphics(kCharacterAugust);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_StartPart2);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 35;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_August_AtBreakfast(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAugust);
		getCharacter(kCharacterAugust).characterPosition.position = 3970;
		getCharacter(kCharacterAugust).characterPosition.location = 1;
		getCharacter(kCharacterAugust).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterAugust).clothes = 1;
		getCharacter(kCharacterAugust).inventoryItem = 0;
		setDoor(3, kCharacterCath, 1, 10, 9);
		setDoor(11, kCharacterCath, 0, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_AtBreakfast(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_AtBreakfast);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 36;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_AtBreakfast(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1755000 && !getCharacterCurrentParams(kCharacterAugust)[1]) {
			getCharacterCurrentParams(kCharacterAugust)[1] = 1;
			send(kCharacterAugust, kCharacterWaiter1, 252568704, 0);
		}

		if (_gameTime > 1773000 && getCharacterCurrentParams(kCharacterAugust)[0] && rcClear()) {
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).inventoryItem = 0;
			blockView(kCharacterAugust, kCarRestaurant, 62);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoJoinedSeqOtis, "016C", 33, 103798704, "016D");
		}

		break;
	case 1:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustGoodMorning, 0, 0);
		break;
	case 12:
		if (!_doneNIS[kEventAugustGoodMorning])
			getCharacter(kCharacterAugust).inventoryItem = 0x80;

		send(kCharacterAugust, kCharacterTableA, 136455232, 0);
		startCycOtis(kCharacterAugust, "016B");
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			playNIS(kEventAugustGoodMorning);
			bumpCath(kCarRestaurant, 61, 255);
			break;
		case 2:
			releaseView(kCharacterAugust, kCarRestaurant, 62);
			startSeqOtis(kCharacterAugust, "803ES");
			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterAugust);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 3:
			send(kCharacterAugust, kCharacterWaiter1, 286534136, 0);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_DoWalk, 3, 6470, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			AugustCall(&LogicManager::CONS_August_EnterComp, 1, 0, 0, 0);
			break;
		case 5:
			CONS_August_OpenComp(0, 0, 0, 0);
			break;
		case 6:
			if (!_doneNIS[kEventAugustGoodMorning])
				getCharacter(kCharacterAugust).inventoryItem = 0x80;
			send(kCharacterAugust, kCharacterWaiter1, 219522616, 0);
			startCycOtis(kCharacterAugust, "016B");
			getCharacterCurrentParams(kCharacterAugust)[0] = 1;
			break;
		default:
			break;
		}

		break;
	case 123712592:
		startCycOtis(kCharacterAugust, "016A");
		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 6;
		AugustCall(&LogicManager::CONS_August_DoDialog, "AUG2113", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_OpenComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_OpenComp);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 37;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_OpenComp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1791000 && !getCharacterCurrentParams(kCharacterAugust)[1]) {
			getCharacterCurrentParams(kCharacterAugust)[1] = 1;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			AugustCall(&LogicManager::CONS_August_ExitComp, 1, 0, 0, 0);
		}

		break;
	case 12:
		setDoor(3, kCharacterCath, 2, 0, 0);
		startCycOtis(kCharacterAugust, "506A2");
		break;
	case 17:
		if (_gameTime > 1786500 && checkCathDir(kCarGreenSleeping, 43)) {
			if (getCharacterCurrentParams(kCharacterAugust)[0]) {
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
				AugustCall(&LogicManager::CONS_August_DoSeqOtis, "506C2", 0, 0, 0);
			} else {
				getCharacterCurrentParams(kCharacterAugust)[0] = 1;
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
				AugustCall(&LogicManager::CONS_August_DoSeqOtis, "506B2", 0, 0, 0);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			bumpCath(kCarGreenSleeping, 16, 255);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_ExitComp, 1, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_DoWalk, 5, 850, 0, 0);
			break;
		case 4:
		case 6:
			CONS_August_InSalon(0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 6;
			AugustCall(&LogicManager::CONS_August_DoWalk, 5, 850, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_InSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_InSalon);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 38;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_InSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1801800 && !getCharacterCurrentParams(kCharacterAugust)[0]) {
			getCharacterCurrentParams(kCharacterAugust)[0] = 1;
			send(kCharacterAugust, kCharacterRebecca, 155980128, 0);
		}

		if (_gameTime > 1820700 && !getCharacterCurrentParams(kCharacterAugust)[1]) {
			getCharacterCurrentParams(kCharacterAugust)[1] = 1;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_WaitRCClear, 0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).characterPosition.position = 1540;
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoBlockSeqOtis, "109A", 5, 56, 0);
			break;
		case 2:
			takeItem(kItem3);
			getCharacter(kCharacterAugust).characterPosition.location = 1;
			startCycOtis(kCharacterAugust, "109B");
			break;
		case 3:
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_DoBlockSeqOtis, "109D2", 5, 56, 0);
			break;
		case 4:
			dropItem(kItem3, 1);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			AugustCall(&LogicManager::CONS_August_LookingForCath, 1849500, 0, 0, 0);
			break;
		case 5:
			CONS_August_EndPart2(0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 7;
			AugustCall(&LogicManager::CONS_August_DoDialog, "AUG2114", 0, 0, 0);
			break;
		case 7:
			startCycOtis(kCharacterAugust, "108C");
			blockView(kCharacterAugust, kCarRestaurant, 56);
			blockView(kCharacterAugust, kCarRestaurant, 57);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 8;
			AugustCall(&LogicManager::CONS_August_DoDialog, "AUG2114A", 0, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 9;
			AugustCall(&LogicManager::CONS_August_DoDialog, "AUG2115", 0, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 10;
			AugustCall(&LogicManager::CONS_August_DoComplexSeqOtis, "108D1", "108D2", 23, 0);
			break;
		case 10:
			startCycOtis(kCharacterAugust, "109B");
			releaseView(kCharacterAugust, kCarRestaurant, 56);
			releaseView(kCharacterAugust, kCarRestaurant, 57);
			send(kCharacterAugust, kCharacterRebecca, 125496184, 0);
			break;
		default:
			break;
		}

		break;
	case 169358379:
		send(kCharacterAugust, kCharacterRebecca, 155465152, 0);
		startCycOtis(kCharacterAugust, "108A");
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 6;
		AugustCall(&LogicManager::CONS_August_DoWait, 900, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_EndPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_EndPart2);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 39;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_EndPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (!getCharacterParams(kCharacterAugust, 8)[0])
			playDialog(0, "BUMP", -1, 0);

		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustArrivalInMunich, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 1) {
			playNIS(kEventAugustArrivalInMunich);
			send(kCharacterAugust, kCharacterMaster, 139122728, 0);
			endGraphics(kCharacterAugust);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_StartPart3);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 40;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_August_GoLunch(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAugust);
		getCharacter(kCharacterAugust).characterPosition.position = 6470;
		getCharacter(kCharacterAugust).characterPosition.location = 0;
		getCharacter(kCharacterAugust).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacter(kCharacterAugust).clothes = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoWalkP3A(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoWalkP3A);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 41;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoWalkP3A(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAugust)[2] && nearChar(kCharacterAugust, kCharacterCath, 2000)) {
			getCharacter(kCharacterAugust).inventoryItem = 0x80;
		} else {
			getCharacter(kCharacterAugust).inventoryItem = 0;
		}

		if (walk(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[0], getCharacterCurrentParams(kCharacterAugust)[1])) {
			getCharacter(kCharacterAugust).inventoryItem = 0;

			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		} else if (!_doneNIS[kEventAugustMerchandise] && nearChar(kCharacterAugust, kCharacterCath, 1000) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			if (getCharacter(kCharacterAugust).characterPosition.car == kCarGreenSleeping || getCharacter(kCharacterAugust).characterPosition.car == kCarRedSleeping) {
				playNIS(kEventAugustMerchandise);
				if (getCharacter(kCharacterAugust).direction == 1)
					bumpCathFx(getCharacter(kCharacterAugust).characterPosition.car, getCharacter(kCharacterAugust).characterPosition.position - 750);
				else
					bumpCathRx(getCharacter(kCharacterAugust).characterPosition.car, getCharacter(kCharacterAugust).characterPosition.position + 750);

				break;
			}
		}

		break;
	case 1:
		getCharacterCurrentParams(kCharacterAugust)[2] = 0;
		getCharacter(kCharacterAugust).inventoryItem = 0;
		playNIS(((getCharacter(kCharacterAugust).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position) + 22));

		if (getCharacter(kCharacterAugust).direction == 1)
			bumpCathFx(getCharacter(kCharacterAugust).characterPosition.car, getCharacter(kCharacterAugust).characterPosition.position - 750);
		else
			bumpCathRx(getCharacter(kCharacterAugust).characterPosition.car, getCharacter(kCharacterAugust).characterPosition.position + 750);

		break;
	case 5:
		if (_globals[kGlobalMetAugust]) {
			if (rnd(2) == 0) {
				playDialog(kCharacterCath, "CAT1002A", -1, 0);
			} else {
				playDialog(kCharacterCath, "CAT1002", -1, 0);
			}
		} else {
			playCathExcuseMe();
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterAugust, kCharacterCath, 0);
		break;
	case 12:
		if (walk(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[0], getCharacterCurrentParams(kCharacterAugust)[1])) {
			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		} else if (_doneNIS[kEventAugustMerchandise] && !_doneNIS[kEventAugustTalkGold] && !_doneNIS[kEventAugustTalkGoldDay]) {
			getCharacterCurrentParams(kCharacterAugust)[2] = 128;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_DoWalkP3B(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_DoWalkP3B);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 42;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;
	params->parameters[2] = param3.intParam;

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_DoWalkP3B(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAugust)[3] && nearChar(kCharacterAugust, kCharacterCath, 2000)) {
			getCharacter(kCharacterAugust).inventoryItem = 0x80;
		} else {
			getCharacter(kCharacterAugust).inventoryItem = 0;
		}

		if (walk(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[0], getCharacterCurrentParams(kCharacterAugust)[1])) {
			getCharacter(kCharacterAugust).inventoryItem = 0;

			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		}

		break;
	case 1:
		getCharacterCurrentParams(kCharacterAugust)[3] = 0;
		getCharacter(kCharacterAugust).inventoryItem = 0;
		playDialog(0, "CAT1002", -1, 0);

		if (_doneNIS[kEventAugustBringBriefcase]) {
			playDialog(kCharacterAugust, "AUG3103", -1, 15);
		} else {
			playDialog(kCharacterAugust, "AUG3100", -1, 15);
		}

		break;
	case 6:
		if (!whoRunningDialog(kCharacterAugust))
			playChrExcuseMe(kCharacterAugust, kCharacterCath, 0);

		break;
	case 12:
		if (walk(kCharacterAugust, getCharacterCurrentParams(kCharacterAugust)[0], getCharacterCurrentParams(kCharacterAugust)[1])) {
			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
		} else if (getCharacterCurrentParams(kCharacterAugust)[2]) {
			getCharacterCurrentParams(kCharacterAugust)[3] = 128;
			if (!_doneNIS[kEventAugustBringBriefcase])
				getCharacterCurrentParams(kCharacterAugust)[3] = 147;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_GoLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_GoLunch);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 43;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_GoLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1953000 && !getCharacterCurrentParams(kCharacterAugust)[1]) {
			getCharacterCurrentParams(kCharacterAugust)[1] = 1;
			send(kCharacterAugust, kCharacterAnna, 291662081, 0);
		}

		if (getCharacterCurrentParams(kCharacterAugust)[0])
			getCharacter(kCharacterAugust).characterPosition = getCharacter(kCharacterAnna).characterPosition;

		if (_gameTime > 2016000 && !getCharacterCurrentParams(kCharacterAugust)[0] && rcClear()) {
			getCharacter(kCharacterAugust).inventoryItem = 0;
			CONS_August_ReturnLunch(0, 0, 0, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 6;
		AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustLunch, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_ExitComp, 1, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoWalkP3A, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAugust).characterPosition.position = 1540;
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_DoSeqOtis, "803VS", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterAugust, "010A2");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterAugust);

			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			AugustCall(&LogicManager::CONS_August_FinishJoinedSeqOtis, 36, 136455232, "BOGUS", 0);
			break;
		case 5:
			getCharacter(kCharacterAugust).characterPosition.location = 1;
			startCycOtis(kCharacterAugust, "010B2");
			if (!_doneNIS[kEventAugustLunch])
				getCharacter(kCharacterAugust).inventoryItem = 0x80;

			break;
		case 6:
			playNIS(kEventAugustLunch);
			cleanNIS();
			break;
		default:
			break;
		}

		break;
	case 122288808:
		getCharacterCurrentParams(kCharacterAugust)[0] = 0;
		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacter(kCharacterAugust).characterPosition.location = 1;
		startCycOtis(kCharacterAugust, "112G");
		break;
	case 122358304:
		getCharacterCurrentParams(kCharacterAugust)[0] = 1;
		getCharacter(kCharacterAugust).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_ReturnLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_ReturnLunch);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 44;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_ReturnLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAugust).characterPosition.location = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_DoBlockSeqOtis, "112H", 5, 57, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			if (_doneNIS[kEventAugustMerchandise]) {
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
				AugustCall(&LogicManager::CONS_August_DoWalkP3A, 3, 6470, 0, 0);
			} else {
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
				AugustCall(&LogicManager::CONS_August_LookingForCath, 2043000, 0, 0, 0);
			}

			break;
		case 2:
			if (!getCharacterParams(kCharacterAugust, 8)[0]) {
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
				AugustCall(&LogicManager::CONS_August_DoWalkP3A, 3, 6470, 0, 0);
			} else {
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
				AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustMerchandise, 0, 0);
			}

			break;
		case 3:
			playNIS(kEventAugustMerchandise);
			if (getCharacter(kCharacterAugust).characterPosition.car == kCarGreenSleeping && nearX(kCharacterAugust, 6470, 500))
				getCharacter(kCharacterAugust).characterPosition.position = 5970;

			walk(kCharacterAugust, kCarGreenSleeping, 6470);

			if (getCharacter(kCharacterAugust).direction == 1)
				bumpCathFx(getCharacter(kCharacterAugust).characterPosition.car, getCharacter(kCharacterAugust).characterPosition.position - 750);
			else
				bumpCathRx(getCharacter(kCharacterAugust).characterPosition.car, getCharacter(kCharacterAugust).characterPosition.position + 750);

			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_DoWalkP3A, 3, 6470, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			AugustCall(&LogicManager::CONS_August_EnterComp, 1, 0, 0, 0);
			break;
		case 5:
			CONS_August_AfterLunch(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_AfterLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_AfterLunch);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 45;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_AfterLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2061000 && !getCharacterCurrentParams(kCharacterAugust)[0]) {
			getCharacterCurrentParams(kCharacterAugust)[0] = 1;
			getCharacter(kCharacterAugust).inventoryItem = 0;
			CONS_August_Reading(0, 0, 0, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		playDialog(kCharacterCath, "CAT1002", -1, 0);
		playDialog(kCharacterAugust, "AUG3102", -1, 15);
		break;
	case 12:
		setDoor(3, kCharacterCath, 2, 0, 0);
		startCycOtis(kCharacterAugust, "506A2");
		getCharacter(kCharacterAugust).inventoryItem = 0x92;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Reading(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Reading);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 46;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Reading(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2088000 && !getCharacterCurrentParams(kCharacterAugust)[0]) {
			getCharacterCurrentParams(kCharacterAugust)[0] = 1;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
			AugustCall(&LogicManager::CONS_August_BathroomTrip, 0, 0, 0, 0);
		}

		break;
	case 17:
		if (checkCathDir(kCarGreenSleeping, 43)) {
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoSeqOtis, "507B2", 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 1) {
			CONS_August_Dressing(0, 0, 0, 0);
		} else if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 2) {
			if (checkCathDir(kCarGreenSleeping, 43))
				bumpCath(kCarGreenSleeping, 34, 255);

			endGraphics(kCharacterAugust);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_BathroomTrip(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_BathroomTrip);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 47;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_BathroomTrip(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_ExitComp, 1, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoWalkP3A, 3, 9460, 0, 0);
			break;
		case 2:
			endGraphics(kCharacterAugust);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_DoWait, 2700, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_DoWalkP3A, 3, 6470, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			AugustCall(&LogicManager::CONS_August_EnterComp, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterAugust).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAugust, _functionsAugust[getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall]]);
			fedEx(kCharacterAugust, kCharacterAugust, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Dressing(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Dressing);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 48;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Dressing(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2101500 && !getCharacterCurrentParams(kCharacterAugust)[0]) {
			getCharacterCurrentParams(kCharacterAugust)[0] = 1;
			CONS_August_GoConcert(0, 0, 0, 0);
		}

		break;
	case 8:
	case 9:
		if (!_doneNIS[kEventAugustTalkCompartmentDoorBlueRedingote] && !_doneNIS[kEventAugustTalkCompartmentDoor] && !_doneNIS[kEventAugustBringEgg] && !_doneNIS[kEventAugustBringBriefcase]) {
			if (msg->action == 8)
				playDialog(0, "LIB012", -1, 0);

			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
			AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustTalkCompartmentDoor, 0, 0);
		}

		break;
	case 12:
		setDoor(3, kCharacterAugust, 1, 10, 9);
		getCharacter(kCharacterAugust).clothes = 2;
		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 1) {
			playNIS(kEventAugustTalkCompartmentDoor);
			cleanNIS();
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_CompLogic, 2101500, 0, 0, 0);
		} else if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 2) {
			CONS_August_GoConcert(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_GoConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_GoConcert);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 49;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_GoConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_ExitComp, 0, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 1) {
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoWalk, 2, 9270, 0, 0);
		} else if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 2) {
			CONS_August_Concert(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Concert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Concert);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 50;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Concert(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(3, kCharacterCath, 1, 10, 9);
		endGraphics(kCharacterAugust);
		getCharacter(kCharacterAugust).characterPosition.car = kCarKronos;
		getCharacter(kCharacterAugust).characterPosition.position = 6000;
		getCharacter(kCharacterAugust).characterPosition.location = 1;
		break;
	case 191668032:
		CONS_August_StalkAnna(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_StalkAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_StalkAnna);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 51;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_StalkAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAugust).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAugust).characterPosition.position = 850;
		getCharacter(kCharacterAugust).characterPosition.location = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_DoWalkP3B, 3, 5790, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			send(kCharacterAugust, kCharacterTatiana, 191668032, 0);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoWalkP3B, 4, 540, 1, 0);
			break;
		case 2:
			endGraphics(kCharacterAugust);
			break;
		case 3:
			startCycOtis(kCharacterAugust, "BLANK");
			send(kCharacterAugust, kCharacterAnna, 123712592, 0);
			break;
		default:
			break;
		}

		break;
	case 122288808:
		CONS_August_AfterConcert(0, 0, 0, 0);
		break;
	case 122358304:
		startCycOtis(kCharacterAugust, "BLANK");
		break;
	case 169032608:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
		AugustCall(&LogicManager::CONS_August_DoWalkP3B, 4, 3820, 1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_AfterConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_AfterConcert);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 52;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_AfterConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 8:
	case 9:
		if (cathHasItem(kItemBriefcase)) {
			getCharacter(kCharacterCath).characterPosition.location = 1;
			if (msg->action == 8)
				playDialog(0, "LIB012", -1, 0);

			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustBringBriefcase, 0, 0);
		} else if (!cathHasItem(kItemFirebird) || _doneNIS[kEventAugustBringEgg]) {
			if (_doneNIS[kEventAugustTalkCompartmentDoorBlueRedingote] || _doneNIS[kEventAugustBringEgg] || _doneNIS[kEventAugustBringBriefcase]) {
				setDoor(3, kCharacterAugust, 1, 0, 0);

				if (msg->action == 8) {
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 6;
					AugustCall(&LogicManager::CONS_August_DoDialog, "LIB012", 0, 0, 0);
				} else {
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 7;
					AugustCall(&LogicManager::CONS_August_DoDialog, "LIB013", 0, 0, 0);
				}
			} else {
				if (msg->action == 8)
					playDialog(0, "LIB012", -1, 0);

				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
				AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustBringEgg, 0, 0);
			}
		} else {
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustBringEgg, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_DoWalkP3B, 3, 6470, 1, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_EnterComp, 0, 1, 0, 0);
			break;
		case 2:
			setDoor(3, kCharacterAugust, 0, 10, 9);
			send(kCharacterAugust, kCharacterKahina, 134611040, 0);
			break;
		case 3:
			playNIS(kEventAugustBringBriefcase);
			playDialog(0, "LIB015", -1, 0);
			forceJump(kCharacterSalko, &LogicManager::CONS_Salko_TellMilos);
			bumpCath(kCarGreenSleeping, 13, 255);
			CONS_August_Satisfied(0, 0, 0, 0);
			break;
		case 4:
			playNIS(kEventAugustBringEgg);
			cleanNIS();
			break;
		case 5:
			playNIS(kEventAugustTalkCompartmentDoorBlueRedingote);
			cleanNIS();
			break;
		case 6:
		case 7:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 8;
			AugustCall(&LogicManager::CONS_August_DoDialog, "AUG1128F", 0, 0, 0);
			break;
		case 8:
			setDoor(3, kCharacterAugust, 1, 10, 9);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Satisfied(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Satisfied);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 53;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Satisfied(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_DoWait, 2700, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_ExitComp, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_DoWalk, 5, 850, 0, 0);
			break;
		case 3:
			CONS_August_InSalon3(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_InSalon3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_InSalon3);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 54;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_InSalon3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterAugust)[3] || getCharacterCurrentParams(kCharacterAugust)[1] || _globals[kGlobalKahinaKillTimeoutActive]) {
			getCharacter(kCharacterAugust).inventoryItem = 0;
		} else {
			getCharacter(kCharacterAugust).inventoryItem = 0x80;
		}

		if (!getCharacterCurrentParams(kCharacterAugust)[1]) {
			if (getCharacterCurrentParams(kCharacterAugust)[0]) {
				if (getCharacterCurrentParams(kCharacterAugust)[4] || (getCharacterCurrentParams(kCharacterAugust)[4] = _gameTime + getCharacterCurrentParams(kCharacterAugust)[0], _gameTime + getCharacterCurrentParams(kCharacterAugust)[0] != 0)) {
					if (getCharacterCurrentParams(kCharacterAugust)[4] >= _gameTime)
						break;

					getCharacterCurrentParams(kCharacterAugust)[4] = 0x7FFFFFFF;
				}

				getCharacter(kCharacterAugust).inventoryItem = 0;
				CONS_August_LeaveSalon(0, 0, 0, 0);
			}
		}

		break;
	case 1:
		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
		AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustTalkCigar, 0, 0);
		break;
	case 3:
		releaseView(kCharacterAugust, kCarRestaurant, 57);
		startCycOtis(kCharacterAugust, "105B3");
		break;
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_WaitRCClear, 0, 0, 0, 0);
		break;
	case 17:
		if (!checkCathDir(kCarRestaurant, 60) || getCharacterCurrentParams(kCharacterAugust)[2]) {
			if (!getCharacterCurrentParams(kCharacterAugust)[1] && checkCathDir(kCarRestaurant, 57))
				bumpCath(kCarRestaurant, 50, 255);
		} else if (!getCharacterCurrentParams(kCharacterAugust)[1]) {
			blockView(kCharacterAugust, kCarRestaurant, 57);
			startSeqOtis(kCharacterAugust, "105C3");
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).characterPosition.position = 1540;
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoBlockSeqOtis, "105A3", 5, 57, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).characterPosition.location = 1;
			send(kCharacterAugust, kCharacterAbbot, 123712592, 0);
			startCycOtis(kCharacterAugust, "105B3");
			getCharacterCurrentParams(kCharacterAugust)[3] = 1;
			break;
		case 3:
			playNIS(kEventAugustTalkCigar);
			if (getCharacterCurrentParams(kCharacterAugust)[2])
				startCycOtis(kCharacterAugust, "122B");
			else
				startCycOtis(kCharacterAugust, "105B3");
			cleanNIS();
			getCharacterCurrentParams(kCharacterAugust)[0] = 9000;
			getCharacterCurrentParams(kCharacterAugust)[3] = 0;
			break;
		default:
			break;
		}

		break;
	case 122288808:
		startCycOtis(kCharacterAugust, "122B");
		getCharacterCurrentParams(kCharacterAugust)[1] = 0;
		if (_doneNIS[kEventAugustTalkCigar])
			getCharacterCurrentParams(kCharacterAugust)[0] = 9000;

		break;
	case 122358304:
		startCycOtis(kCharacterAugust, "BLANK");
		getCharacterCurrentParams(kCharacterAugust)[1] = 1;
		getCharacterCurrentParams(kCharacterAugust)[2] = 1;
		break;
	case 136196244:
		getCharacterCurrentParams(kCharacterAugust)[1] = 1;
		getCharacter(kCharacterAugust).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_LeaveSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_LeaveSalon);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 55;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_LeaveSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoBlockSeqOtis, "105D3", 5, 57, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_DoWalk, 3, 6470, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_EnterComp, 1, 0, 0, 0);
			break;
		case 4:
			CONS_August_BeforeVienna(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_BeforeVienna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_BeforeVienna);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 56;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_BeforeVienna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(3, kCharacterCath, 2, 0, 0);
		startCycOtis(kCharacterAugust, "507A3");
		break;
	case 17:
		if (!getCharacterCurrentParams(kCharacterAugust)[0] && checkCathDir(kCarGreenSleeping, 43)) {
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
			AugustCall(&LogicManager::CONS_August_DoSeqOtis, "507B3", 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 1) {
			getCharacterCurrentParams(kCharacterAugust)[0] = 1;
			startCycOtis(kCharacterAugust, "507A3");
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_StartPart4);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 57;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_August_GoDinner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAugust);
		getCharacter(kCharacterAugust).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAugust).characterPosition.position = 6470;
		getCharacter(kCharacterAugust).characterPosition.location = 1;
		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacter(kCharacterAugust).clothes = 2;
		setDoor(3, kCharacterCath, 2, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_GoDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_GoDinner);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 58;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_GoDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_ExitComp, 1, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAugust).characterPosition.position = 1540;
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_DoSeqOtis, "803WS", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterAugust, "010A3");
			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterAugust);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
			AugustCall(&LogicManager::CONS_August_FinishJoinedSeqOtis, 36, 136455232, "Bogus", 0);
			break;
		case 5:
			getCharacter(kCharacterAugust).characterPosition.location = 1;
			CONS_August_WaitingAnna(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_WaitingAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_WaitingAnna);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 59;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_WaitingAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		startCycOtis(kCharacterAugust, "010B3");
		send(kCharacterAugust, kCharacterHeadWait, 190605184, 0);
		break;
	case 122358304:
		startCycOtis(kCharacterAugust, "BLANK");
		break;
	case 123793792:
		CONS_August_Dinner(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Dinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Dinner);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 60;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Dinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		bool skip = false; // Horrible way to unroll a goto...

		if (!getCharacterCurrentParams(kCharacterAugust)[1]) {
			getCharacterCurrentParams(kCharacterAugust)[1] = _gameTime + 450;
			if (_gameTime == -450) {
				skip = true;
				send(kCharacterAugust, kCharacterWaiter1, 207330561, 0);
			}
		}

		if (!skip && getCharacterCurrentParams(kCharacterAugust)[1] < _gameTime) {
			getCharacterCurrentParams(kCharacterAugust)[1] = 0x7FFFFFFF;
			send(kCharacterAugust, kCharacterWaiter1, 207330561, 0);
		}

		if (getCharacterCurrentParams(kCharacterAugust)[0] != 0) {
			if (getCharacterCurrentParams(kCharacterAugust)[2] || (getCharacterCurrentParams(kCharacterAugust)[2] = _gameTime + 9000, _gameTime != -9000)) {
				if (getCharacterCurrentParams(kCharacterAugust)[2] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterAugust)[2] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
			AugustCall(&LogicManager::CONS_August_WaitRCClear, 0, 0, 0, 0);
		}

		break;
	}
	case 12:
	case 122288808:
		startCycOtis(kCharacterAugust, "010B3");
		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 1) {
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoJoinedSeqOtis, "010J3", 36, 103798704, "010M");
		} else if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 2) {
			send(kCharacterAugust, kCharacterWaiter1, 286403504, 0);
			CONS_August_ReturnComp4(0, 0, 0, 0);
		}

		break;
	case 122358304:
		startCycOtis(kCharacterAugust, "BLANK");
		break;
	case 201964801:
		startCycOtis(kCharacterAugust, "010H3");
		getCharacterCurrentParams(kCharacterAugust)[0] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_ReturnComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_ReturnComp4);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 61;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_ReturnComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAugust).characterPosition.location = 0;
		startSeqOtis(kCharacterAugust, "803FS");

		if (inDiningRoom(kCharacterCath))
			advanceFrame(kCharacterAugust);

		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_FinishSeqOtis, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoWalk, 3, 6470, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_EnterComp, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_CompLogic, _gameTime + 4500, 0, 0, 0);
			break;
		case 4:
			CONS_August_GoSalon4(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_GoSalon4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_GoSalon4);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 62;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_GoSalon4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAugust)[0] || (getCharacterCurrentParams(kCharacterAugust)[0] = _gameTime + 900, _gameTime != -900)) {
			if (getCharacterCurrentParams(kCharacterAugust)[0] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterAugust)[0] = 0x7FFFFFFF;
		}

		playDialog(kCharacterAugust, "Aug4003A", -1, 0);
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
		AugustCall(&LogicManager::CONS_August_DoBlockSeqOtis, "122C", 5, 57, 0);
		break;
	case 12:
		getCharacter(kCharacterAugust).characterPosition.location = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_DoCorrOtis, "696Ec", 3, 0, 0);
		break;
	case 17:
		if (checkCathDir(kCarRestaurant, 57))
			bumpCath(kCarRestaurant, 50, 255);

		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			setDoor(3, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAugust).characterPosition.position = 1540;
			getCharacter(kCharacterAugust).characterPosition.location = 0;
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
			AugustCall(&LogicManager::CONS_August_DoBlockSeqOtis, "122A", 5, 57, 0);
			break;
		case 4:
			getCharacter(kCharacterAugust).characterPosition.location = 1;
			startCycOtis(kCharacterAugust, "122B");
			break;
		case 5:
			startCycOtis(kCharacterAugust, "122B");
			send(kCharacterAugust, kCharacterWaiter2, 291721418, 0);
			break;
		default:
			break;
		}

		break;
	case 122358304:
		startCycOtis(kCharacterAugust, "BLANK");
		break;
	case 125826561:
		CONS_August_Drinking(0, 0, 0, 0);
		break;
	case 134486752:
		startCycOtis(kCharacterAugust, "122B");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Drinking(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Drinking);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 63;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Drinking(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		bool skip = false; // Horrible way to unroll a goto...

		if (!getCharacterCurrentParams(kCharacterAugust)[2]) {
			getCharacterCurrentParams(kCharacterAugust)[2] = _gameTime + 1800;
			if (_gameTime == -1800) {
				skip = true;
				getCharacter(kCharacterAugust).inventoryItem = 0x80;
			}
		}

		if (!skip && getCharacterCurrentParams(kCharacterAugust)[2] < _gameTime) {
			getCharacterCurrentParams(kCharacterAugust)[2] = 0x7FFFFFFF;
			getCharacter(kCharacterAugust).inventoryItem = 0x80;
		}

		if (_gameTime > 2488500 && !getCharacterCurrentParams(kCharacterAugust)[3]) {
			getCharacterCurrentParams(kCharacterAugust)[3] = 1;
			getCharacter(kCharacterAugust).inventoryItem = 0;
			CONS_August_Drunk(0, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterAugust)[4] ||
			(getCharacterCurrentParams(kCharacterAugust)[4] = _realTime + getCharacterCurrentParams(kCharacterAugust)[0], _realTime + getCharacterCurrentParams(kCharacterAugust)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterAugust)[4] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterAugust)[4] = 0x7FFFFFFF;
		}

		bool tmp = getCharacterCurrentParams(kCharacterAugust)[1] == 0;
		getCharacterCurrentParams(kCharacterAugust)[1] = tmp;

		if (tmp) {
			startCycOtis(kCharacterAugust, "122H");
		} else {
			startCycOtis(kCharacterAugust, "122F");
		}

		getCharacterCurrentParams(kCharacterAugust)[0] = 5 * (3 * rnd(20) + 15);
		getCharacterCurrentParams(kCharacterAugust)[4] = 0;
		break;
	}
	case 1:
		if (inSalon(kCharacterAlexei))
			forceJump(kCharacterAlexei, &LogicManager::CONS_Alexei_GoToPlatform);

		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustDrink, 0, 0);
		break;
	case 12:
		getCharacterCurrentParams(kCharacterAugust)[0] = 5 * (3 * rnd(20) + 15);
		startCycOtis(kCharacterAugust, "122F");
		break;
	case 17:
		if (checkCathDir(kCarRestaurant, 57))
			bumpCath(kCarRestaurant, 50, 255);

		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 1) {
			playNIS(kEventAugustDrink);
			bumpCath(kCarRestaurant, 55, 255);
			CONS_August_Drunk(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Drunk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Drunk);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 64;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Drunk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if ((!getCharacterCurrentParams(kCharacterAugust)[0] && (getCharacterCurrentParams(kCharacterAugust)[0] = _gameTime + 1800, _gameTime == -1800)) || getCharacterCurrentParams(kCharacterAugust)[0] < _gameTime) {
			if (_gameTime > 2430000) {
				if (rcClear()) {
					getCharacter(kCharacterAugust).characterPosition.location = 0;
					getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
					AugustCall(&LogicManager::CONS_August_DoBlockSeqOtis, "122J", 5, 57, 0);
				}
			}
		}

		break;
	case 12:
		startCycOtis(kCharacterAugust, "122H");
		break;
	case 17:
		if (checkCathDir(kCarRestaurant, 57))
			bumpCath(kCarRestaurant, 50, 255);

		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoWalk, 3, 6470, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_DoEnterCorrOtis, "696Dc", 3, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterAugust);
			CONS_August_Asleep4(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Asleep4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Asleep4);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 65;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Asleep4(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
	case 12:
		if (getCharacter(kCharacterAugust).characterPosition.car = kCarGreenSleeping,
			getCharacter(kCharacterAugust).characterPosition.position = 6470,
			getCharacter(kCharacterAugust).characterPosition.location = 1,
			endGraphics(kCharacterAugust),
			setDoor(3, kCharacterCath, 1, 10, 9),
			!whoRunningDialog(kCharacterAugust)) {
			playDialog(kCharacterAugust, "AUG1057", -1, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_StartPart5);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 66;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_August_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAugust);
		getCharacter(kCharacterAugust).characterPosition.position = 3969;
		getCharacter(kCharacterAugust).characterPosition.location = 1;
		getCharacter(kCharacterAugust).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterAugust).inventoryItem = 0;
		getCharacter(kCharacterAugust).clothes = 2;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Prisoner);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 67;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_August_Hiding(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Hiding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Hiding);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 68;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Hiding(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAugust)[0]) {
			if (getCharacterCurrentParams(kCharacterAugust)[3] ||
				(getCharacterCurrentParams(kCharacterAugust)[3] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterAugust)[3] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAugust)[3] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterAugust)[0] = 0;
			getCharacterCurrentParams(kCharacterAugust)[1] = 1;
			setDoor(3, kCharacterAugust, 1, 0, 0);
			getCharacterCurrentParams(kCharacterAugust)[3] = 0;
		} else {
			getCharacterCurrentParams(kCharacterAugust)[3] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterAugust)[0]) {
			setDoor(3, kCharacterAugust, 1, 0, 0);
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
			AugustCall(&LogicManager::CONS_August_DoDialog, getCathJustChecking(), 0, 0, 0);
		} else if (msg->action == 8) {
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 2;
			AugustCall(&LogicManager::CONS_August_DoDialog, "LIB012", 0, 0, 0);
		} else {
			getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 3;
			AugustCall(&LogicManager::CONS_August_DoDialog, "LIB013", 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterAugust).characterPosition.position = 6470;
		getCharacter(kCharacterAugust).characterPosition.location = 1;
		getCharacter(kCharacterAugust).characterPosition.car = 3;
		setDoor(3, 2, 1, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterAugust)[1] || getCharacterCurrentParams(kCharacterAugust)[0]) {
			getCharacterCurrentParams(kCharacterAugust)[1] = 0;
			getCharacterCurrentParams(kCharacterAugust)[0] = 0;
			setDoor(3, kCharacterAugust, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAugust)[2] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8]) {
		case 1:
			getCharacterCurrentParams(kCharacterAugust)[0] = 0;
			setDoor(3, kCharacterAugust, 1, 10, 9);
			break;
		case 2:
		case 3:
			getCharacterCurrentParams(kCharacterAugust)[2]++;
			switch (getCharacterCurrentParams(kCharacterAugust)[2]) {
			case 1:
				setDoor(3, kCharacterAugust, 1, 0, 0);
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 4;
				AugustCall(&LogicManager::CONS_August_DoDialog, "Aug5002", 0, 0, 0);
				break;
			case 2:
				setDoor(3, kCharacterAugust, 1, 0, 0);
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 5;
				AugustCall(&LogicManager::CONS_August_DoDialog, "Aug5002A", 0, 0, 0);
				break;
			case 3:
				setDoor(3, kCharacterAugust, 1, 0, 0);
				getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 6;
				AugustCall(&LogicManager::CONS_August_DoDialog, "Aug5002B", 0, 0, 0);
				break;
			default:
				break;
			}

			break;
		case 4:
			getCharacterCurrentParams(kCharacterAugust)[0] = 1;
			setDoor(3, kCharacterAugust, 1, 14, 0);
			break;
		case 5:
			setDoor(3, kCharacterAugust, 1, 10, 9);
			break;
		case 6:
			getCharacterCurrentParams(kCharacterAugust)[1] = 1;
			break;
		default:
			break;
		}

		break;
	case 203078272:
		send(kCharacterAugust, kCharacterTatiana, 203078272, 0);
		CONS_August_CutLoose(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_CutLoose(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_CutLoose);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 69;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_CutLoose(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		sendAll(kCharacterAugust, 135800432, 0);
		CONS_August_Disappear(0, 0, 0, 0);
		break;
	case 12:
		_engine->getSoundManager()->endAmbient();
		if (dialogRunning("ARRIVE"))
			endDialog("ARRIVE");
		getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] = 1;
		AugustCall(&LogicManager::CONS_August_SaveGame, 2, kEventAugustUnhookCarsBetrayal, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall + 8] == 1) {
			if (_globals[kGlobalReadLetterInAugustSuitcase])
				playNIS(kEventAugustUnhookCarsBetrayal);
			else
				playNIS(kEventAugustUnhookCars);

			endGraphics(kCharacterAugust);
			_engine->getSoundManager()->startAmbient();
			playDialog(0, "MUS050", -1, 0);
			bumpCath(kCarRestaurant, 85, 1);
			sendAll(kCharacterAugust, 70549068, 0);
			forceJump(kCharacterTrainM, &LogicManager::CONS_TrainM_Disappear);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_August_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAugust).callParams[getCharacter(kCharacterAugust).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAugust, &LogicManager::HAND_August_Disappear);
	getCharacter(kCharacterAugust).callbacks[getCharacter(kCharacterAugust).currentCall] = 70;

	params->clear();

	fedEx(kCharacterAugust, kCharacterAugust, 12, 0);
}

void LogicManager::HAND_August_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsAugust[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_August_DebugWalks,
	&LogicManager::HAND_August_DoWait,
	&LogicManager::HAND_August_DoSeqOtis,
	&LogicManager::HAND_August_DoBlockSeqOtis,
	&LogicManager::HAND_August_DoCorrOtis,
	&LogicManager::HAND_August_DoEnterCorrOtis,
	&LogicManager::HAND_August_DoBriefCorrOtis,
	&LogicManager::HAND_August_FinishSeqOtis,
	&LogicManager::HAND_August_DoJoinedSeqOtis,
	&LogicManager::HAND_August_FinishJoinedSeqOtis,
	&LogicManager::HAND_August_DoComplexSeqOtis,
	&LogicManager::HAND_August_DoDialog,
	&LogicManager::HAND_August_DoDialogFullVol,
	&LogicManager::HAND_August_WaitRCClear,
	&LogicManager::HAND_August_SaveGame,
	&LogicManager::HAND_August_DoWalk,
	&LogicManager::HAND_August_LookingForCath,
	&LogicManager::HAND_August_DoWalkSearchingForCath,
	&LogicManager::HAND_August_EnterComp,
	&LogicManager::HAND_August_ExitComp,
	&LogicManager::HAND_August_CompLogic,
	&LogicManager::HAND_August_Birth,
	&LogicManager::HAND_August_KnockTyler,
	&LogicManager::HAND_August_DoNIS1006,
	&LogicManager::HAND_August_WaitTyler,
	&LogicManager::HAND_August_SeekTyler,
	&LogicManager::HAND_August_GotoDinner,
	&LogicManager::HAND_August_OrderDinner,
	&LogicManager::HAND_August_EatingDinner,
	&LogicManager::HAND_August_GreetAnna,
	&LogicManager::HAND_August_ReturnFromDinner,
	&LogicManager::HAND_August_GoSalon,
	&LogicManager::HAND_August_BackFromSalon,
	&LogicManager::HAND_August_Asleep,
	&LogicManager::HAND_August_StartPart2,
	&LogicManager::HAND_August_AtBreakfast,
	&LogicManager::HAND_August_OpenComp,
	&LogicManager::HAND_August_InSalon,
	&LogicManager::HAND_August_EndPart2,
	&LogicManager::HAND_August_StartPart3,
	&LogicManager::HAND_August_DoWalkP3A,
	&LogicManager::HAND_August_DoWalkP3B,
	&LogicManager::HAND_August_GoLunch,
	&LogicManager::HAND_August_ReturnLunch,
	&LogicManager::HAND_August_AfterLunch,
	&LogicManager::HAND_August_Reading,
	&LogicManager::HAND_August_BathroomTrip,
	&LogicManager::HAND_August_Dressing,
	&LogicManager::HAND_August_GoConcert,
	&LogicManager::HAND_August_Concert,
	&LogicManager::HAND_August_StalkAnna,
	&LogicManager::HAND_August_AfterConcert,
	&LogicManager::HAND_August_Satisfied,
	&LogicManager::HAND_August_InSalon3,
	&LogicManager::HAND_August_LeaveSalon,
	&LogicManager::HAND_August_BeforeVienna,
	&LogicManager::HAND_August_StartPart4,
	&LogicManager::HAND_August_GoDinner,
	&LogicManager::HAND_August_WaitingAnna,
	&LogicManager::HAND_August_Dinner,
	&LogicManager::HAND_August_ReturnComp4,
	&LogicManager::HAND_August_GoSalon4,
	&LogicManager::HAND_August_Drinking,
	&LogicManager::HAND_August_Drunk,
	&LogicManager::HAND_August_Asleep4,
	&LogicManager::HAND_August_StartPart5,
	&LogicManager::HAND_August_Prisoner,
	&LogicManager::HAND_August_Hiding,
	&LogicManager::HAND_August_CutLoose,
	&LogicManager::HAND_August_Disappear
};

} // End of namespace LastExpress
