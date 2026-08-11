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

void LogicManager::CONS_Abbot(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterAbbot,
			_functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]
		);

		break;
	case 1:
		CONS_Abbot_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Abbot_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Abbot_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Abbot_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Abbot_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::AbbotCall(CALL_PARAMS) {
	getCharacter(kCharacterAbbot).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Abbot_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_DebugWalks);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 1;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterAbbot, kCarGreenSleeping, getCharacterCurrentParams(kCharacterAbbot)[0])) {
			if (getCharacterCurrentParams(kCharacterAbbot)[0] == 10000) {
				getCharacterCurrentParams(kCharacterAbbot)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterAbbot)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterAbbot).characterPosition.position = 0;
		getCharacter(kCharacterAbbot).characterPosition.location = 0;
		getCharacter(kCharacterAbbot).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterAbbot)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_DoSeqOtis);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);
		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAbbot, (char *)&getCharacterCurrentParams(kCharacterAbbot)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_DoCorrOtis);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[3]);

		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);

		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAbbot, (char *)&getCharacterCurrentParams(kCharacterAbbot)[0]);
		blockAtDoor(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_DoEnterCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_DoEnterCorrOtis);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_DoEnterCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[3]);
		getCharacter(kCharacterAbbot).characterPosition.position = 6470;
		getCharacter(kCharacterAbbot).characterPosition.location = 1;

		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);

		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAbbot, (char *)&getCharacterCurrentParams(kCharacterAbbot)[0]);
		blockAtDoor(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[3]);
		getCharacter(kCharacterAbbot).characterPosition.location = 1;

		if (inComp(kCharacterCath, kCarRedSleeping, 6470) || inComp(kCharacterCath, kCarRedSleeping, 6130)) {
			if (isNight()) {
				playNIS(kEventCathTurningNight);
			} else {
				playNIS(kEventCathTurningDay);
			}
			
			playDialog(kCharacterCath, "BUMP", -1, 0);
			bumpCathFDoor(34);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_FinishSeqOtis);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 5;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 3:
		if (msg->action == 3 || (msg->action == 0 && getCharacter(kCharacterAbbot).direction != 4)) {
			getCharacter(kCharacterAbbot).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);
			fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_DoComplexSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_DoComplexSeqOtis);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);
	strncpy((char *)&params->parameters[3], param2.stringParam, 12);

	params->parameters[6] = param3.intParam;

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_DoComplexSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);

		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAbbot, (char *)&getCharacterCurrentParams(kCharacterAbbot)[0]);
		startSeqOtis(getCharacterCurrentParams(kCharacterAbbot)[6], (char *)&getCharacterCurrentParams(kCharacterAbbot)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_DoWait);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 7;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAbbot)[1]) {
			if (getCharacterCurrentParams(kCharacterAbbot)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterAbbot)[1] = 0x7FFFFFFF;
		} else {
			getCharacterCurrentParams(kCharacterAbbot)[1] = _gameTime + getCharacterCurrentParams(kCharacterAbbot)[0];

			if (_gameTime + getCharacterCurrentParams(kCharacterAbbot)[0] == 0)
				break;

			if (getCharacterCurrentParams(kCharacterAbbot)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterAbbot)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);

		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_DoWaitReal(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_DoWaitReal);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 8;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_DoWaitReal(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAbbot)[1]) {
			if (getCharacterCurrentParams(kCharacterAbbot)[1] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterAbbot)[1] = 0x7FFFFFFF;
		} else {
			getCharacterCurrentParams(kCharacterAbbot)[1] = _realTime + getCharacterCurrentParams(kCharacterAbbot)[0];

			if (_realTime + getCharacterCurrentParams(kCharacterAbbot)[0] == 0)
				break;

			if (getCharacterCurrentParams(kCharacterAbbot)[1] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterAbbot)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);

		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_DoDialog);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 9;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);

		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		break;
	case 12:
		playDialog(kCharacterAbbot, (char *)&getCharacterCurrentParams(kCharacterAbbot)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_SaveGame);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);

		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		break;
	case 12:
		save(
			kCharacterAbbot,
			getCharacterCurrentParams(kCharacterAbbot)[0],
			getCharacterCurrentParams(kCharacterAbbot)[1]
		);

		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);

		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		break;
	default:
		break;
	} 
}

void LogicManager::CONS_Abbot_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_DoWalk);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 11;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[0], getCharacterCurrentParams(kCharacterAbbot)[1])) {
			getCharacter(kCharacterAbbot).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);
			fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		}

		break;
	case 5:
		if (checkCathDir(kCarGreenSleeping, 18) || checkCathDir(kCarRedSleeping, 18)) {
			playChrExcuseMe(kCharacterAbbot, kCharacterCath, 0);
			break;
		}

		if (_doneNIS[kEventAbbotIntroduction]) {
			playDialog(kCharacterCath, "CAT1013", -1, 0);
		} else {
			playCathExcuseMe();
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterAbbot, kCharacterCath, 0);
		break;
	case 12:
		if (walk(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[0], getCharacterCurrentParams(kCharacterAbbot)[1])) {

			getCharacter(kCharacterAbbot).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);
			fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_DoJoinedSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_DoJoinedSeqOtis);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 12;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	strncpy((char *)&params->parameters[5], param4.stringParam, 12);

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_DoJoinedSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		if (!getCharacterCurrentParams(kCharacterAbbot)[8])
			fedEx(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[3], getCharacterCurrentParams(kCharacterAbbot)[4], getCharacterCurrentParams(kCharacterAbbot)[5]);

		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);
		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);

		break;
	case 10:
		if (!getCharacterCurrentParams(kCharacterAbbot)[8]) {
			fedEx(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[3], getCharacterCurrentParams(kCharacterAbbot)[4], getCharacterCurrentParams(kCharacterAbbot)[5]);
			getCharacterCurrentParams(kCharacterAbbot)[8] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterAbbot, (char *)&getCharacterCurrentParams(kCharacterAbbot));
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_DoBlockSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_DoBlockSeqOtis);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 13;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_DoBlockSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseView(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[3], getCharacterCurrentParams(kCharacterAbbot)[4]);
		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);
		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAbbot, (char *)&getCharacterCurrentParams(kCharacterAbbot));
		blockView(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[3], getCharacterCurrentParams(kCharacterAbbot)[4]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_WaitRCClear);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 14;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterAbbot).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);

			fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_Birth);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 15;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		autoMessage(kCharacterAbbot, 203073664, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_StartPart2);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 16;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterAbbot);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_StartPart3);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 17;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Abbot_InKitchen(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAbbot);

		getCharacter(kCharacterAbbot).characterPosition.location = 1;
		getCharacter(kCharacterAbbot).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterAbbot).characterPosition.position = 5900;
		getCharacter(kCharacterAbbot).inventoryItem = 0;
		getCharacter(kCharacterAbbot).clothes = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_InKitchen(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_InKitchen);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 18;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_InKitchen(HAND_PARAMS) {
	switch (msg->action) {
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAbbot).characterPosition.position = 5800;
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoSeqOtis, "804DD", 0, 0, 0);
			break;
		case 2:
			send(kCharacterAbbot, kCharacterCook, 236976550, 0);
			startSeqOtis(kCharacterAbbot, "804DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterAbbot);

			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 4, 6470, 0, 0);
			break;
		case 4:
			setDoor(34, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 5;

			AbbotCall(&LogicManager::CONS_Abbot_DoCorrOtis, "617AC", 34, 0, 0);
			break;
		case 5:
			setDoor(34, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterAbbot).characterPosition.position = 6470;
			getCharacter(kCharacterAbbot).characterPosition.location = 1;

			CONS_Abbot_OpenComp(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 192054567:
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_OpenComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_OpenComp);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 19;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_OpenComp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1953000 && !getCharacterCurrentParams(kCharacterAbbot)[0]) {
			getCharacterCurrentParams(kCharacterAbbot)[0] = 1;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "MrB3010", 0, 0, 0);
		}

		break;
	case 12:
		startCycOtis(kCharacterAbbot, "508A");
		send(kCharacterAbbot, kCharacterMonsieur, 122358304, 0);
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb3010", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoWait, 900, 0, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterAbbot, "508B");
			break;
		case 3:
			send(kCharacterAbbot, kCharacterMonsieur, 122288808, 0);
			CONS_Abbot_AloneComp(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_AloneComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_AloneComp);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 20;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_AloneComp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1966500 && inDiningRoom(kCharacterMonsieur)) {
			CONS_Abbot_GoLunch(0, 0, 0, 0);
		}

		break;
	case 12:
		startCycOtis(kCharacterAbbot, "509A");
		break;
	}
}

void LogicManager::CONS_Abbot_GoLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_GoLunch);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 21;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_GoLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoSeqOtis, "509B", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			setDoor(34, kCharacterCath, 0, 10, 9);
			setDoor(50, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoCorrOtis, "617Mc", 34, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 5, 850, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
			AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAbbot).characterPosition.position = 1540;
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 5;
			AbbotCall(&LogicManager::CONS_Abbot_DoSeqOtis, "804US", 0, 0, 0);
			break;
		case 5:
			startSeqOtis(kCharacterAbbot, "029J");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterAbbot);

			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 6;
			AbbotCall(&LogicManager::CONS_Abbot_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 6:
			startCycOtis(kCharacterAbbot, "029H");
			send(kCharacterAbbot, kCharacterHeadWait, 207769280, 0);
			break;
		case 7:
			CONS_Abbot_EatingLunch(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 122288808:
		send(kCharacterAbbot, kCharacterTableE, 136455232, 0);
		getCharacter(kCharacterAbbot).characterPosition.location = 1;
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 7;
		AbbotCall(&LogicManager::CONS_Abbot_DoSeqOtis, "029B", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_EatingLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_EatingLunch);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 22;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_EatingLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1971000 && !getCharacterCurrentParams(kCharacterAbbot)[0]) {
			getCharacterCurrentParams(kCharacterAbbot)[0] = 1;
			send(kCharacterAbbot, kCharacterWaiter1, 218586752, 0);
		}

		if (_gameTime > 1989000 && rcClear()) {
			getCharacter(kCharacterAbbot).inventoryItem = 0;
			CONS_Abbot_LeaveLunch(0, 0, 0, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterAbbot).inventoryItem = kItemNone;
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_SaveGame, 2, kEventAbbotIntroduction, 0, 0);
		break;
	case 12:
		startCycOtis(kCharacterAbbot, "029E");

		if (!_doneNIS[kEventAbbotIntroduction])
			getCharacter(kCharacterAbbot).inventoryItem = kItemInvalid;

		break;
	case 18:
		if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 1) {
			playNIS(kEventAbbotIntroduction);
			playDialog(0, "LIB036", -1, 0);
			bumpCath(kCarRestaurant, 61, 255);
		}

		break;
	case 122288808:
		startCycOtis(kCharacterAbbot, "029E");
		break;
	case 122358304:
		startCycOtis(kCharacterAbbot, "BLANK");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_LeaveLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_LeaveLunch);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 23;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_LeaveLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAbbot).characterPosition.location = 0;
		blockView(kCharacterAbbot, kCarRestaurant, 67);
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoJoinedSeqOtis, "029F", 37, 103798704, "029G");
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			releaseView(kCharacterAbbot, kCarRestaurant, 67);
			send(kCharacterAbbot, kCharacterWaiter1, 270068760, 0);
			send(kCharacterAbbot, kCharacterAnna, 238936000, 0);
			startSeqOtis(kCharacterAbbot, "804DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterAbbot);

			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 4, 6470, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
			AbbotCall(&LogicManager::CONS_Abbot_DoEnterCorrOtis, "617Cc", 34, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			CONS_Abbot_ClosedComp(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_ClosedComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_ClosedComp);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 24;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_ClosedComp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAbbot)[0] || (getCharacterCurrentParams(kCharacterAbbot)[0] = _gameTime + 900, _gameTime != -900)) {
			if (getCharacterCurrentParams(kCharacterAbbot)[0] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterAbbot)[0] = 0x7FFFFFFF;
		}

		CONS_Abbot_GoSalon1(0, 0, 0, 0);
		break;
	case 8:
	case 9:
		setDoor(34, kCharacterAbbot, 1, 0, 0);
		setDoor(50, kCharacterAbbot, 1, 0, 0);

		if (msg->action == 8) {
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
			AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "LIB012", 0, 0, 0);
		} else {
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "LIB013", 0, 0, 0);
		}

		break;
	case 12:
		endGraphics(kCharacterAbbot);
		setDoor(34, kCharacterAbbot, 0, 10, 9);
		setDoor(50, kCharacterAbbot, 0, 10, 9);
		break;
	case 18:
		if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
			if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] <= 2) {
				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
				AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb3001", 0, 0, 0);
			} else if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 3) {
				setDoor(34, kCharacterAbbot, 1, 10, 9);
				setDoor(50, kCharacterAbbot, 1, 10, 9);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_GoSalon1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_GoSalon1);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 25;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_GoSalon1(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoCorrOtis, "617Dc", 34, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			setDoor(34, kCharacterCath, 0, 10, 9);
			setDoor(50, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAbbot).characterPosition.position = 1540;
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
			AbbotCall(&LogicManager::CONS_Abbot_DoBlockSeqOtis, "115A", 5, 56, 0);
			break;
		case 4:
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			takeItem(kItem3);
			CONS_Abbot_InSalon(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_InSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_InSalon);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 26;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_InSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAbbot)[0]) {
			if ((!getCharacterCurrentParams(kCharacterAbbot)[1] &&
				 (getCharacterCurrentParams(kCharacterAbbot)[1] = _gameTime + 4500, _gameTime == -4500)) ||
				getCharacterCurrentParams(kCharacterAbbot)[1] < _gameTime) {
				if (rcClear())
					CONS_Abbot_GoComp(0, 0, 0, 0);
			}
		}

		break;
	case 12:
		send(kCharacterAbbot, kCharacterKronos, 157159392, 0);
		startCycOtis(kCharacterAbbot, "115B");
		break;
	case 101169422:
		getCharacterCurrentParams(kCharacterAbbot)[0] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_GoComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_GoComp);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 27;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_GoComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoBlockSeqOtis, "115C", 5, 56, 0);
			break;
		case 2:
			dropItem(kItem3, 1);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 4, 6470, 0, 0);
			break;
		case 3:
			setDoor(34, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
			AbbotCall(&LogicManager::CONS_Abbot_DoCorrOtis, "617Ac", 34, 0, 0);
			break;
		case 4:
			setDoor(34, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterAbbot).characterPosition.position = 6470;
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			CONS_Abbot_OpenComp2(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_OpenComp2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_OpenComp2);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 28;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_OpenComp2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2052000 && !getCharacterCurrentParams(kCharacterAbbot)[0]) {
			getCharacterCurrentParams(kCharacterAbbot)[0] = 1;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_GoWander, 0, 0, 0, 0);
		}

		break;
	case 12:
		send(kCharacterAbbot, kCharacterMonsieur, 122358304, 0);
		startCycOtis(kCharacterAbbot, "508A");
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb3013", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 1)
			startCycOtis(kCharacterAbbot, "508B");

		break;
	case 222609266:
		CONS_Abbot_GoSalon2(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_GoWander(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_GoWander);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 29;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_GoWander(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		send(kCharacterAbbot, kCharacterMonsieur, 122288808, 0);
		setDoor(34, kCharacterCath, 1, 255, 255);
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoCorrOtis, "617Bc", 34, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			setDoor(34, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 4, 9460, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_DoWaitReal, 450, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 3, 540, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 5;
			AbbotCall(&LogicManager::CONS_Abbot_DoWait, 225, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 6;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 4, 6470, 0, 0);
			break;
		case 6:
			setDoor(34, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 7;
			AbbotCall(&LogicManager::CONS_Abbot_DoCorrOtis, "617Ac", 34, 0, 0);
			break;
		case 7:
			setDoor(34, kCharacterCath, 2, 255, 255);
			send(kCharacterAbbot, kCharacterMonsieur, 122358304, 0);
			startCycOtis(13, "508B");

			getCharacter(kCharacterAbbot).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);

			fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_GoSalon2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_GoSalon2);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 30;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_GoSalon2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb3030", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			send(kCharacterAbbot, kCharacterMonsieur, 122288808, 0);
			setDoor(34, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoCorrOtis, "617Bc", 34, 0, 0);
			break;
		case 2:
			setDoor(34, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 5, 850, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
			AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAbbot).characterPosition.position = 1540;
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 5;
			AbbotCall(&LogicManager::CONS_Abbot_DoBlockSeqOtis, "115A", 5, 56, 0);
			break;
		case 5:
			takeItem(kItem3);
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			CONS_Abbot_InSalon2(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_InSalon2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_InSalon2);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 31;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_InSalon2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAbbot)[3] != 0x7FFFFFFF && getCharacterCurrentParams(kCharacterAbbot)[1] < _gameTime) {
			if (getCharacterCurrentParams(kCharacterAbbot)[2] < _gameTime) {
				getCharacterCurrentParams(kCharacterAbbot)[3] = 0x7FFFFFFF;
				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
				AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
				break;
			}

			if (!inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterAbbot)[3]) {
				getCharacterCurrentParams(kCharacterAbbot)[3] = _gameTime + 450;
				if (_gameTime == -450) {
					getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
					AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterAbbot)[3] < _gameTime) {
				getCharacterCurrentParams(kCharacterAbbot)[3] = 0x7FFFFFFF;
				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
				AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterAbbot)[0] != 0) {
			if (getCharacterCurrentParams(kCharacterAbbot)[4] || (getCharacterCurrentParams(kCharacterAbbot)[4] = _gameTime + 450, _gameTime != -450)) {
				if (getCharacterCurrentParams(kCharacterAbbot)[4] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterAbbot)[4] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 6;
			AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterAbbot)[1] = _gameTime + 4500;
		getCharacterCurrentParams(kCharacterAbbot)[2] = _gameTime + 18000;
		startCycOtis(kCharacterAbbot, "115B");
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoBlockSeqOtis, "115E", 5, 56, 0);
			break;
		case 2:
			dropItem(kItem3, 1);
			send(kCharacterAbbot, kCharacterAlexei, 122358304, 0);
			playDialog(kCharacterAbbot, "Abb3020", -1, 0);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_DoBlockSeqOtis, "125A", 5, 52, 0);
			break;
		case 3:
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			startCycOtis(kCharacterAbbot, "125B");
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
			AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb3021", 0, 0, 0);
			break;
		case 4:
			playDialog(kCharacterAbbot, "Abb3023", -1, 0);
			blockView(kCharacterAbbot, kCarRestaurant, 52);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 5;
			AbbotCall(&LogicManager::CONS_Abbot_DoComplexSeqOtis, "125C1", "125C2", 12, 0);
			break;
		case 5:
			releaseView(kCharacterAbbot, kCarRestaurant, 52);
			startCycOtis(kCharacterAbbot, "125D");
			send(kCharacterAbbot, kCharacterAlexei, 122288808, 0);
			getCharacterCurrentParams(kCharacterAbbot)[0] = 1;

			if (getCharacterCurrentParams(kCharacterAbbot)[0] != 0) {
				if (getCharacterCurrentParams(kCharacterAbbot)[4] || (getCharacterCurrentParams(kCharacterAbbot)[4] = _gameTime + 450, _gameTime != -450)) {
					if (getCharacterCurrentParams(kCharacterAbbot)[4] >= _gameTime)
						break;

					getCharacterCurrentParams(kCharacterAbbot)[4] = 0x7FFFFFFF;
				}

				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 6;
				AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
			}

			break;
		case 6:
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 7;
			AbbotCall(&LogicManager::CONS_Abbot_DoBlockSeqOtis, "125E", 5, 52, 0);
			break;
		case 7:
			CONS_Abbot_GoComp3(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_GoComp3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_GoComp3);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 32;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_GoComp3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 4, 6470, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 1) {
			setDoor(34, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoCorrOtis, "617Ac", 34, 0, 0);
		} else if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 2) {
			setDoor(34, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterAbbot).characterPosition.position = 6470;
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			send(kCharacterAbbot, kCharacterMonsieur, 122358304, 0);
			CONS_Abbot_OpenComp3(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_OpenComp3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_OpenComp3);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 33;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_OpenComp3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAbbot)[0] != 0x7FFFFFFF && _gameTime > 2115000) {
			if (_gameTime <= 2124000) {
				if (!nearChar(kCharacterAbbot, kCharacterCath, 2000) || !getCharacterCurrentParams(kCharacterAbbot)[0]) {
					getCharacterCurrentParams(kCharacterAbbot)[0] = _gameTime;
					if (!_gameTime) {
						getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
						AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb3014", 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterAbbot)[0] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterAbbot)[0] = 0x7FFFFFFF;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;

			AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb3014", 0, 0, 0);
		}

		break;
	case 12:
		startCycOtis(kCharacterAbbot, "508A");
		break;
	case 18:
		if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 1)
			startCycOtis(kCharacterAbbot, "508B");

		break;
	case 123712592:
		CONS_Abbot_GoSalon3(0, 0, 0, 0);
		break;
	}
}

void LogicManager::CONS_Abbot_GoSalon3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_GoSalon3);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 34;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_GoSalon3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb3031", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			send(kCharacterAbbot, kCharacterMonsieur, 122288808, 0);
			setDoor(34, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoCorrOtis, "617Bc", 34, 0, 0);
			break;
		case 2:
			setDoor(34, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 5, 850, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
			AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAbbot).characterPosition.position = 1540;
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 5;
			AbbotCall(&LogicManager::CONS_Abbot_DoBlockSeqOtis, "115A", 5, 56, 0);
			break;
		case 5:
			takeItem(kItem3);
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			CONS_Abbot_InSalon3(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_InSalon3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_InSalon3);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 35;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_InSalon3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAbbot)[1] == 0x7FFFFFFF || !_gameTime)
			break;

		if (getCharacterCurrentParams(kCharacterAbbot)[0] >= _gameTime) {
			if (!inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterAbbot)[1]) {
				getCharacterCurrentParams(kCharacterAbbot)[1] = _gameTime + 450;
				if (_gameTime == -450) {
					send(kCharacterAbbot, kCharacterAugust, 136196244, 0);
					getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
					AbbotCall(&LogicManager::CONS_Abbot_DoWait, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterAbbot)[1] >= _gameTime)
				break;
		}

		getCharacterCurrentParams(kCharacterAbbot)[1] = 0x7FFFFFFF;
		send(kCharacterAbbot, kCharacterAugust, 136196244, 0);
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoWait, 0, 0, 0, 0);
		break;
	case 12:
		startCycOtis(kCharacterAbbot, "115B");
		getCharacterCurrentParams(kCharacterAbbot)[0] = _gameTime + 9000;
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			playDialog(kCharacterAbbot, "Abb3040", -1, 45);
			blockView(kCharacterAbbot, kCarRestaurant, 57);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_DoJoinedSeqOtis, "121A", 2, 122358304, "BOGUS");
			break;
		case 3:
			releaseView(kCharacterAbbot, kCarRestaurant, 57);
			dropItem(kItem3, 1);
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			CONS_Abbot_WithAugust(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_WithAugust(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_WithAugust);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 36;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_WithAugust(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		switch (getCharacterCurrentParams(kCharacterAbbot)[1]) {
		case 1:
			if (getCharacterCurrentParams(kCharacterAbbot)[2] == 0x7FFFFFFF || !_gameTime)
				break;

			if (getCharacterCurrentParams(kCharacterAbbot)[0] >= _gameTime) {
				if (!inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterAbbot)[2]) {
					getCharacterCurrentParams(kCharacterAbbot)[2] = _gameTime + 675;
					if (_gameTime == -675) {
						playDialog(kCharacterAbbot, "Abb3041", -1, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterAbbot)[2] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterAbbot)[2] = 0x7FFFFFFF;
			playDialog(kCharacterAbbot, "Abb3041", -1, 0);
			break;
		case 2:
			if (getCharacterCurrentParams(kCharacterAbbot)[3] || (getCharacterCurrentParams(kCharacterAbbot)[3] = _gameTime + 900, _gameTime != -900)) {
				if (getCharacterCurrentParams(kCharacterAbbot)[3] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterAbbot)[3] = 0x7FFFFFFF;
			}

			playDialog(kCharacterAbbot, "Abb3042", -1, 0);
			break;
		case 3:
			playDialog(kCharacterAbbot, "Abb3043", -1, 0);
			blockView(kCharacterAbbot, kCarRestaurant, 57);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
			AbbotCall(&LogicManager::CONS_Abbot_DoJoinedSeqOtis, "121D", 2, 122288808, "BOGUS");
			break;
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterAbbot)[1]++;
		break;
	case 12:
		getCharacterCurrentParams(kCharacterAbbot)[0] = _gameTime + 4500;
		startCycOtis(kCharacterAbbot, "121B");
		break;
	case 17:
		if (checkCathDir(kCarRestaurant, 57)) {
			bumpCath(kCarRestaurant, 50, 255);
		}

		break;
	case 18:
		if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 1) {
			releaseView(kCharacterAbbot, kCarRestaurant, 57);
			CONS_Abbot_GoComp4(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_GoComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_GoComp4);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 37;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_GoComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 4, 6470, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 1) {
			setDoor(34, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoCorrOtis, "617Ac", 34, 0, 0);
		} else if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 2) {
			setDoor(34, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterAbbot).characterPosition.position = 6470;
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			send(kCharacterAbbot, kCharacterMonsieur, 122358304, 0);
			CONS_Abbot_InComp4(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_InComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_InComp4);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 38;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_InComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		startCycOtis(kCharacterAbbot, "508A");
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb3014A", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 1) {
			startCycOtis(kCharacterAbbot, "508B");
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_StartPart4);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 39;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Abbot_AtDinner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAbbot);
		getCharacter(kCharacterAbbot).inventoryItem = 0;
		getCharacter(kCharacterAbbot).characterPosition.car = kCarRestaurant;
		getCharacterParams(kCharacterAbbot, 8)[0] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_DoWalkSearchingForCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_DoWalkSearchingForCath);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 40;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_DoWalkSearchingForCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[0], getCharacterCurrentParams(kCharacterAbbot)[1])) {
			getCharacter(kCharacterAbbot).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);
			fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		} else if (!_doneNIS[kEventAbbotInvitationDrink] && nearChar(kCharacterAbbot, kCharacterCath, 1000) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			if (getCharacter(kCharacterAbbot).characterPosition.car == kCarGreenSleeping || getCharacter(kCharacterAbbot).characterPosition.car == kCarRedSleeping) {
				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
				AbbotCall(&LogicManager::CONS_Abbot_SaveGame, 2, kEventAbbotInvitationDrink, 0, 0);
			}
		}

		break;
	case 12:
		if (walk(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[0], getCharacterCurrentParams(kCharacterAbbot)[1])) {
			getCharacter(kCharacterAbbot).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);
			fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 1) {
			playNIS(kEventAbbotInvitationDrink);

			if (getCharacter(kCharacterAbbot).direction == 1) {
				bumpCathFx(getCharacter(kCharacterAbbot).characterPosition.car, getCharacter(kCharacterAbbot).characterPosition.position - 750);
			} else {
				bumpCathRx(getCharacter(kCharacterAbbot).characterPosition.car, getCharacter(kCharacterAbbot).characterPosition.position + 750);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_AtDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_AtDinner);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 41;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_AtDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2358000 && !getCharacterCurrentParams(kCharacterAbbot)[0]) {
			getCharacterCurrentParams(kCharacterAbbot)[0] = 1;
			send(kCharacterAbbot, kCharacterWaiter1, 218128129, 0);
		}

		if (_gameTime > 2389500) {
			if (rcClear())
				CONS_Abbot_LeaveDinner(0, 0, 0, 0);
		}

		break;
	case 12:
		send(kCharacterAbbot, kCharacterTableE, 136455232, 0);
		startCycOtis(kCharacterAbbot, "029E");
		getCharacter(kCharacterAbbot).characterPosition.location = 1;
		break;
	case 122288808:
		startCycOtis(kCharacterAbbot, "029E");
		break;
	case 122358304:
		startCycOtis(kCharacterAbbot, "BLANK");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_LeaveDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_LeaveDinner);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 42;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_LeaveDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAbbot).characterPosition.location = 0;
		blockView(kCharacterAbbot, kCarRestaurant, 67);
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoJoinedSeqOtis, "029F", 37, 103798704, "029G");
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			releaseView(kCharacterAbbot, kCarRestaurant, 67);
			send(kCharacterAbbot, kCharacterWaiter1, 270068760, 0);
			startSeqOtis(kCharacterAbbot, "804DS");
			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterAbbot);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 4, 6470, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
			AbbotCall(&LogicManager::CONS_Abbot_DoEnterCorrOtis, "617Cc", 34, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			endGraphics(kCharacterAbbot);
			CONS_Abbot_InComp(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_InComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_InComp);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 43;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_InComp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAbbot)[0] && getCharacterCurrentParams(kCharacterAbbot)[3] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime > 2452500) {
				getCharacterCurrentParams(kCharacterAbbot)[3] = 0x7FFFFFFF;
				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
				AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb4002", 0, 0, 0);
				break;
			}

			if (!nearChar(kCharacterAbbot, kCharacterCath, 1000) || whoRunningDialog(kCharacterMonsieur) || !getCharacterCurrentParams(kCharacterAbbot)[3]) {
				getCharacterCurrentParams(kCharacterAbbot)[3] = _gameTime;

				if (!_gameTime) {
					getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
					AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb4002", 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterAbbot)[3] < _gameTime) {
				getCharacterCurrentParams(kCharacterAbbot)[3] = 0x7FFFFFFF;
				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
				AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb4002", 0, 0, 0);
				break;
			}
		}

		if (_gameTime > 2466000 && !getCharacterCurrentParams(kCharacterAbbot)[4]) {
			getCharacterCurrentParams(kCharacterAbbot)[4] = 1;
			CONS_Abbot_Asleep4(0, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterAbbot)[2]) {
			if (getCharacterCurrentParams(kCharacterAbbot)[5] || (getCharacterCurrentParams(kCharacterAbbot)[5] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterAbbot)[5] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAbbot)[5] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterAbbot)[2] = 0;
			getCharacterCurrentParams(kCharacterAbbot)[1] = 1;

			setDoor(34, kCharacterAbbot, 1, 0, 0);
			setDoor(50, kCharacterAbbot, 1, 0, 0);

			getCharacterCurrentParams(kCharacterAbbot)[5] = 0;
		} else {
			getCharacterCurrentParams(kCharacterAbbot)[5] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterAbbot)[2]) {
			setDoor(34, kCharacterAbbot, 1, 0, 0);
			setDoor(50, kCharacterAbbot, 1, 0, 0);

			if (msg->param.intParam == 50) {
				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 5;
				AbbotCall(&LogicManager::CONS_Abbot_DoDialog, getCathWCDialog(), 0, 0, 0);
			} else {
				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 6;
				AbbotCall(&LogicManager::CONS_Abbot_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else {
			setDoor(34, kCharacterAbbot, 1, 0, 0);
			setDoor(50, kCharacterAbbot, 1, 0, 0);

			if (msg->action == 8) {
				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
				AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
				AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		setDoor(34, kCharacterAbbot, 1, 10, 9);
		setDoor(50, kCharacterAbbot, 1, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterAbbot)[1] || getCharacterCurrentParams(kCharacterAbbot)[2]) {
			setDoor(34, kCharacterAbbot, 1, 10, 9);
			setDoor(50, kCharacterAbbot, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAbbot)[1] = 0;
			getCharacterCurrentParams(kCharacterAbbot)[2] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			if (_gameTime > 2466000 && !getCharacterCurrentParams(kCharacterAbbot)[4]) {
				getCharacterCurrentParams(kCharacterAbbot)[4] = 1;
				CONS_Abbot_Asleep4(0, 0, 0, 0);
				break;
			}

			if (getCharacterCurrentParams(kCharacterAbbot)[2]) {
				if (getCharacterCurrentParams(kCharacterAbbot)[5] || (getCharacterCurrentParams(kCharacterAbbot)[5] = _realTime + 75, _realTime != -75)) {
					if (getCharacterCurrentParams(kCharacterAbbot)[5] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterAbbot)[5] = 0x7FFFFFFF;
				}

				getCharacterCurrentParams(kCharacterAbbot)[2] = 0;
				getCharacterCurrentParams(kCharacterAbbot)[1] = 1;

				setDoor(34, kCharacterAbbot, 1, 0, 0);
				setDoor(50, kCharacterAbbot, 1, 0, 0);

				getCharacterCurrentParams(kCharacterAbbot)[5] = 0;
			} else {
				getCharacterCurrentParams(kCharacterAbbot)[5] = 0;
			}

			break;
		case 2:
		case 3:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
			AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb3001", 0, 0, 0);
			break;
		case 4:
			setDoor(34, kCharacterAbbot, 1, 14, 0);
			setDoor(50, kCharacterAbbot, 1, 14, 0);
			getCharacterCurrentParams(kCharacterAbbot)[2] = 1;
			break;
		case 5:
		case 6:
			getCharacterCurrentParams(kCharacterAbbot)[2] = 0;
			getCharacterCurrentParams(kCharacterAbbot)[1] = 1;
			break;
		default:
			break;
		}

		break;
	case 101687594:
		getCharacterCurrentParams(kCharacterAbbot)[0] = 1;
		break;
	case 159003408:
		getCharacterCurrentParams(kCharacterAbbot)[0] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_Asleep4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_Asleep4);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 44;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_Asleep4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAbbot).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterAbbot).characterPosition.position = 6470;
		getCharacter(kCharacterAbbot).characterPosition.location = 1;

		endGraphics(kCharacterAbbot);

		setDoor(34, kCharacterCath, 1, 10, 9);
		setDoor(50, kCharacterCath, 1, 10, 9);
		break;
	case 104060776:
		CONS_Abbot_Conferring(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_Conferring(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_Conferring);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 45;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_Conferring(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAbbot).characterPosition.position = 6471;
		getCharacter(kCharacterAbbot).characterPosition.location = 0;
		getCharacter(kCharacterAbbot).characterPosition.car = kCarRedSleeping;

		forceJump(kCharacterTrainM, &LogicManager::CONS_TrainM_Conferring);
		startCycOtis(kCharacterAbbot, "617Ec");
		softBlockAtDoor(kCharacterAbbot, 34);

		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;

		AbbotCall(&LogicManager::CONS_Abbot_DoDialog, "Abb4010", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 1) {
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoCorrOtis, "617Kc", 34, 0, 0);
		} else if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 2) {
			softReleaseAtDoor(kCharacterAbbot, 34);
			send(kCharacterAbbot, kCharacterTrainM, 125233040, 0);

			CONS_Abbot_GoSalon4(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_GoSalon4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_GoSalon4);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 46;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_GoSalon4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAbbot).characterPosition.position = 6471;
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoWalkSearchingForCath, 5, 850, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 1) {
			CONS_Abbot_BeforeBomb(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_BeforeBomb(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_BeforeBomb);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 47;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_BeforeBomb(HAND_PARAMS) {
	switch (msg->action) {
	case 1:
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
		AbbotCall(&LogicManager::CONS_Abbot_SaveGame, 2, kEventAbbotDrinkGiveDetonator, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAbbot).characterPosition.position = 1540;
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoSeqOtis, "126A", 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			startCycOtis(kCharacterAbbot, "126B");
			getCharacter(kCharacterAbbot).inventoryItem = 16;
			break;
		case 3:
			playNIS(kEventAbbotDrinkGiveDetonator);
			endGame(0, 1, 0, true);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_AfterBomb(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_AfterBomb);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 48;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_AfterBomb(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterParams(kCharacterAbbot, 8)[0])
			getCharacter(kCharacterAbbot).inventoryItem = kItemInvalid;

		if (!getCharacterCurrentParams(kCharacterAbbot)[0]) {
			getCharacterCurrentParams(kCharacterAbbot)[0] = _gameTime + 1800;

			if (_gameTime == -1800) {
				getCharacter(kCharacterAbbot).inventoryItem = 0;
				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
				AbbotCall(&LogicManager::CONS_Abbot_DoBlockSeqOtis, "126C", 5, 52, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterAbbot)[0] < _gameTime) {
			getCharacterCurrentParams(kCharacterAbbot)[0] = 0x7FFFFFFF;
			getCharacter(kCharacterAbbot).inventoryItem = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 4;
			AbbotCall(&LogicManager::CONS_Abbot_DoBlockSeqOtis, "126C", 5, 52, 0);
		} else if (_gameTime > 2533500 && !getCharacterCurrentParams(kCharacterAbbot)[1]) {
			getCharacterCurrentParams(kCharacterAbbot)[1] = 1;
			getCharacter(kCharacterAbbot).inventoryItem = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 5;
			AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterAbbot).inventoryItem = 0;
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 9;
		AbbotCall(&LogicManager::CONS_Abbot_SaveGame, 2, kEventAbbotDrinkDefuse, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterAbbot).characterPosition.position = 850;
		getCharacter(kCharacterAbbot).characterPosition.location = 0;
		getCharacter(kCharacterAbbot).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterAbbot).inventoryItem = 0;
		send(kCharacterAbbot, kCharacterTrainM, 125233040, 0);
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 5, 850, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAbbot).characterPosition.position = 1540;
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_DoBlockSeqOtis, "126A", 5, 52, 0);
			break;
		case 3:
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			startCycOtis(13, "126B");
			break;
		case 4:
			if (!_doneNIS[kEventAbbotDrinkDefuse] && getCharacterParams(kCharacterAbbot, 8)[0])
				getCharacter(kCharacterAbbot).inventoryItem = kItemInvalid;

			startCycOtis(kCharacterAbbot, "126B");
			getCharacterCurrentParams(kCharacterAbbot)[0] = 0;

			if (_gameTime > 2533500 && !getCharacterCurrentParams(kCharacterAbbot)[1]) {
				getCharacterCurrentParams(kCharacterAbbot)[1] = 1;
				getCharacter(kCharacterAbbot).inventoryItem = 0;
				getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 5;
				AbbotCall(&LogicManager::CONS_Abbot_WaitRCClear, 0, 0, 0, 0);
			}

			break;
		case 5:
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 6;
			AbbotCall(&LogicManager::CONS_Abbot_DoBlockSeqOtis, "126D", 5, 52, 0);
			break;
		case 6:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 7;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 4, 6470, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 8;
			AbbotCall(&LogicManager::CONS_Abbot_DoEnterCorrOtis, "617Cc", 34, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			endGraphics(kCharacterAbbot);
			CONS_Abbot_Asleep4(0, 0, 0, 0);
			break;
		case 9:
			playNIS(kEventAbbotDrinkDefuse);
			startCycOtis(kCharacterAbbot, "126B");
			send(kCharacterAbbot, kCharacterAnna, 100969180, 0);
			bumpCath(kCarRestaurant, 58, 255);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_CatchCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_CatchCath);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 49;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_CatchCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAbbot)[0] || (getCharacterCurrentParams(kCharacterAbbot)[0] = _realTime + 150, _realTime != -150)) {
			if (getCharacterCurrentParams(kCharacterAbbot)[0] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterAbbot)[0] = 0x7FFFFFFF;
		}

		send(kCharacterAbbot, kCharacterAbbot, 157489665, 0);
		break;
	case 8:
		if (!cathRunningDialog("LIB012"))
			playDialog(0, "LIB012", -1, 0);

		break;
	case 9:
		send(kCharacterAbbot, kCharacterTatiana, 238790488, 0);
		setDoor(2, kCharacterCath, 0, 10, 9);
		setDoor(18, kCharacterCath, 0, 10, 9);
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_SaveGame, 2, kEventAbbotWrongCompartment, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterAbbot).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAbbot).characterPosition.position = 7500;
		getCharacter(kCharacterAbbot).characterPosition.location = 1;
		fedEx(kCharacterAbbot, kCharacterTableE, 103798704, "029G");
		send(kCharacterAbbot, kCharacterWaiter1, 270068760, 0);
		send(kCharacterAbbot, kCharacterMonsieur, 125039808, 0);
		setDoor(2, kCharacterAbbot, 1, 10, 9);
		setDoor(18, kCharacterAbbot, 1, 10, 9);
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			if (getModel(2) == 1) {
				playNIS(kEventAbbotWrongCompartmentBed);
			} else {
				playNIS(kEventAbbotWrongCompartment);
			}

			walk(kCharacterAbbot, kCarRedSleeping, 6470);
			playDialog(0, "LIB015", -1, 0);
			bumpCathFDoor(2);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_DoWalk, 4, 6470, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_Abbot_DoEnterCorrOtis, "617Cc", 34, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			endGraphics(kCharacterAbbot);
			setDoor(34, kCharacterAbbot, 1, 10, 9);
			CONS_Abbot_InComp(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 157489665:
		send(kCharacterAbbot, kCharacterTatiana, 238790488, 0);
		setDoor(2, kCharacterCath, 0, 10, 9);
		setDoor(18, kCharacterCath, 0, 10, 9);
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_Abbot_SaveGame, 2, kEventAbbotWrongCompartment, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_StartPart5);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 50;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Abbot_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAbbot);

		getCharacter(kCharacterAbbot).characterPosition.location = 1;
		getCharacter(kCharacterAbbot).characterPosition.position = 3969;
		getCharacter(kCharacterAbbot).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterAbbot).inventoryItem = 0;
		getCharacter(kCharacterAbbot).clothes = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_Prisoner);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 51;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Abbot_Free(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_Free(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_Free);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 52;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_Free(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterAbbot);
		getCharacter(kCharacterAbbot).characterPosition.car = kCarNone;
		getCharacter(kCharacterAbbot).characterPosition.position = 0;
		getCharacter(kCharacterAbbot).characterPosition.location = 0;
		break;
	case 135600432:
		CONS_Abbot_RunningTrain(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Abbot_RunningTrain(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_Abbot_RunningTrain);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 53;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_Abbot_RunningTrain(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		dropItem(kItem25, 1);
		send(kCharacterAbbot, kCharacterAnna, 158480160, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 1) {
			playNIS(kEventLocomotiveAbbotGetSomeRest);
			cleanNIS();
		} else if (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] == 2) {
			playNIS(kEventLocomotiveAbbotShoveling);
			cleanNIS();
		}

		break;
	case 168646401:
		if (!_doneNIS[kEventLocomotiveAbbotGetSomeRest]) {
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
			AbbotCall(&LogicManager::CONS_Abbot_SaveGame, 2, kEventLocomotiveAbbotGetSomeRest, 0, 0);
		} else if (!_doneNIS[kEventLocomotiveAbbotShoveling]) {
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_Abbot_SaveGame, 2, kEventLocomotiveAbbotShoveling, 0, 0);
		} else {
			playNIS(kEventLocomotiveAbbotShoveling);
			cleanNIS();
		}

		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsAbbot[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Abbot_DebugWalks,
	&LogicManager::HAND_Abbot_DoSeqOtis,
	&LogicManager::HAND_Abbot_DoCorrOtis,
	&LogicManager::HAND_Abbot_DoEnterCorrOtis,
	&LogicManager::HAND_Abbot_FinishSeqOtis,
	&LogicManager::HAND_Abbot_DoComplexSeqOtis,
	&LogicManager::HAND_Abbot_DoWait,
	&LogicManager::HAND_Abbot_DoWaitReal,
	&LogicManager::HAND_Abbot_DoDialog,
	&LogicManager::HAND_Abbot_SaveGame,
	&LogicManager::HAND_Abbot_DoWalk,
	&LogicManager::HAND_Abbot_DoJoinedSeqOtis,
	&LogicManager::HAND_Abbot_DoBlockSeqOtis,
	&LogicManager::HAND_Abbot_WaitRCClear,
	&LogicManager::HAND_Abbot_Birth,
	&LogicManager::HAND_Abbot_StartPart2,
	&LogicManager::HAND_Abbot_StartPart3,
	&LogicManager::HAND_Abbot_InKitchen,
	&LogicManager::HAND_Abbot_OpenComp,
	&LogicManager::HAND_Abbot_AloneComp,
	&LogicManager::HAND_Abbot_GoLunch,
	&LogicManager::HAND_Abbot_EatingLunch,
	&LogicManager::HAND_Abbot_LeaveLunch,
	&LogicManager::HAND_Abbot_ClosedComp,
	&LogicManager::HAND_Abbot_GoSalon1,
	&LogicManager::HAND_Abbot_InSalon,
	&LogicManager::HAND_Abbot_GoComp,
	&LogicManager::HAND_Abbot_OpenComp2,
	&LogicManager::HAND_Abbot_GoWander,
	&LogicManager::HAND_Abbot_GoSalon2,
	&LogicManager::HAND_Abbot_InSalon2,
	&LogicManager::HAND_Abbot_GoComp3,
	&LogicManager::HAND_Abbot_OpenComp3,
	&LogicManager::HAND_Abbot_GoSalon3,
	&LogicManager::HAND_Abbot_InSalon3,
	&LogicManager::HAND_Abbot_WithAugust,
	&LogicManager::HAND_Abbot_GoComp4,
	&LogicManager::HAND_Abbot_InComp4,
	&LogicManager::HAND_Abbot_StartPart4,
	&LogicManager::HAND_Abbot_DoWalkSearchingForCath,
	&LogicManager::HAND_Abbot_AtDinner,
	&LogicManager::HAND_Abbot_LeaveDinner,
	&LogicManager::HAND_Abbot_InComp,
	&LogicManager::HAND_Abbot_Asleep4,
	&LogicManager::HAND_Abbot_Conferring,
	&LogicManager::HAND_Abbot_GoSalon4,
	&LogicManager::HAND_Abbot_BeforeBomb,
	&LogicManager::HAND_Abbot_AfterBomb,
	&LogicManager::HAND_Abbot_CatchCath,
	&LogicManager::HAND_Abbot_StartPart5,
	&LogicManager::HAND_Abbot_Prisoner,
	&LogicManager::HAND_Abbot_Free,
	&LogicManager::HAND_Abbot_RunningTrain
};


} // End of namespace LastExpress
