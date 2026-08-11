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

void LogicManager::CONS_Rebecca(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterRebecca,
			_functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]
		);

		break;
	case 1:
		CONS_Rebecca_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Rebecca_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Rebecca_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Rebecca_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Rebecca_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::RebeccaCall(CALL_PARAMS) {
	getCharacter(kCharacterRebecca).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Rebecca_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_DebugWalks);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 1;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterRebecca, kCarGreenSleeping, getCharacterCurrentParams(kCharacterRebecca)[0])) {
			if (getCharacterCurrentParams(kCharacterRebecca)[0] == 10000)
				getCharacterCurrentParams(kCharacterRebecca)[0] = 0;
			else
				getCharacterCurrentParams(kCharacterRebecca)[0] = 10000;
		}

		break;
	case 12:
		getCharacter(kCharacterRebecca).characterPosition.position = 0;
		getCharacter(kCharacterRebecca).characterPosition.location = 0;
		getCharacter(kCharacterRebecca).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterRebecca)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_DoWait);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 2;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterRebecca)[1] || (getCharacterCurrentParams(kCharacterRebecca)[1] = _gameTime + getCharacterCurrentParams(kCharacterRebecca)[0], _gameTime + getCharacterCurrentParams(kCharacterRebecca)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterRebecca)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterRebecca)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_DoDialog);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		break;
	case 12:
		playDialog(kCharacterRebecca, (char *)&getCharacterCurrentParams(kCharacterRebecca)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_DoCondDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_DoCondDialog);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_DoCondDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		break;
	case 12:
		playDialog(kCharacterRebecca, (char *)&getCharacterCurrentParams(kCharacterRebecca)[0], getVolume(kCharacterCond2), 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_DoJoinedSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_DoJoinedSeqOtis);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	strncpy((char *)&params->parameters[5], param4.stringParam, 12);

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_DoJoinedSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		if (!getCharacterCurrentParams(kCharacterRebecca)[8])
			fedEx(kCharacterRebecca, getCharacterCurrentParams(kCharacterRebecca)[3], getCharacterCurrentParams(kCharacterRebecca)[4], (char *)&getCharacterCurrentParams(kCharacterRebecca)[5]);

		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		break;
	case 10:
		if (!getCharacterCurrentParams(kCharacterRebecca)[8]) {
			fedEx(kCharacterRebecca, getCharacterCurrentParams(kCharacterRebecca)[3], getCharacterCurrentParams(kCharacterRebecca)[4], (char *)&getCharacterCurrentParams(kCharacterRebecca)[5]);
			getCharacterCurrentParams(kCharacterRebecca)[8] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterRebecca, (char *)&getCharacterCurrentParams(kCharacterRebecca)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_DoSeqOtis);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterRebecca, (char *)&getCharacterCurrentParams(kCharacterRebecca)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_DoCorrOtis);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 7;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterRebecca, getCharacterCurrentParams(kCharacterRebecca)[3]);

		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterRebecca, (char *)&getCharacterCurrentParams(kCharacterRebecca)[0]);
		blockAtDoor(kCharacterRebecca, getCharacterCurrentParams(kCharacterRebecca)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_DoEnterCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_DoEnterCorrOtis);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 8;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_DoEnterCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterRebecca, getCharacterCurrentParams(kCharacterRebecca)[3]);
		getCharacter(kCharacterRebecca).characterPosition.position = 4840;

		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterRebecca, (char *)&getCharacterCurrentParams(kCharacterRebecca)[0]);
		blockAtDoor(kCharacterRebecca, getCharacterCurrentParams(kCharacterRebecca)[3]);
		getCharacter(kCharacterRebecca).characterPosition.location = 1;

		if (inComp(kCharacterCath, kCarRedSleeping, 4840) || inComp(kCharacterCath, kCarRedSleeping, 4455)) {
			if (isNight()) {
				playNIS(kEventCathTurningNight);
			} else {
				playNIS(kEventCathTurningDay);
			}

			playDialog(0, "BUMP", -1, 0);
			bumpCathFDoor(36);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_DoBriefCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_DoBriefCorrOtis);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 9;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_DoBriefCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
	case 4:
		releaseAtDoor(kCharacterRebecca, getCharacterCurrentParams(kCharacterRebecca)[3]);
		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterRebecca, (char *)&getCharacterCurrentParams(kCharacterRebecca)[0]);
		blockAtDoor(kCharacterRebecca, getCharacterCurrentParams(kCharacterRebecca)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_FinishSeqOtis);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 10;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterRebecca).direction != 4) {
			getCharacter(kCharacterRebecca).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
			fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_WaitRCClear);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 11;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterRebecca).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
			fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_DoWalk);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 12;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterRebecca, getCharacterCurrentParams(kCharacterRebecca)[0], getCharacterCurrentParams(kCharacterRebecca)[1])) {
			getCharacter(kCharacterRebecca).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
			fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		}

		break;
	case 5:
		playCathExcuseMe();
		break;
	case 6:
		playChrExcuseMe(kCharacterRebecca, kCharacterCath, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_DoBlockSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_DoBlockSeqOtis);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 13;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_DoBlockSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseView(kCharacterRebecca, getCharacterCurrentParams(kCharacterRebecca)[3], getCharacterCurrentParams(kCharacterRebecca)[4]);
		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterRebecca, (char *)&getCharacterCurrentParams(kCharacterRebecca)[0]);
		blockView(kCharacterRebecca, getCharacterCurrentParams(kCharacterRebecca)[3], getCharacterCurrentParams(kCharacterRebecca)[4]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_DoComplexSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_DoComplexSeqOtis);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 14;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);
	strncpy((char *)&params->parameters[3], param2.stringParam, 12);

	params->parameters[6] = param3.intParam;

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_DoComplexSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterRebecca, (char *)&getCharacterCurrentParams(kCharacterRebecca)[0]);
		startSeqOtis(getCharacterCurrentParams(kCharacterRebecca)[6], (char *)&getCharacterCurrentParams(kCharacterRebecca)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_EnterComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_EnterComp);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 15;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_EnterComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (cathOutRebeccaWindow())
			bumpCath(kCarRedSleeping, 49, 255);

		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoEnterCorrOtis, "624Ae", 36, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1) {
			setDoor(44, kCharacterCath, 0, 255, 255);
			getCharacter(kCharacterRebecca).characterPosition.location = 1;
			endGraphics(kCharacterRebecca);

			getCharacter(kCharacterRebecca).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
			fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_GotoDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_GotoDinner);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 16;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_GotoDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterRebecca)[1] && nearChar(kCharacterRebecca, kCharacterSophie, 750) && !whoOnScreen(kCharacterSophie)) {
			send(kCharacterRebecca, kCharacterSophie, 123668192, 0);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
			RebeccaCall(&LogicManager::CONS_Rebecca_WaitRCClear, 0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		if (!getCharacterCurrentParams(kCharacterRebecca)[0]) {
			RebeccaCall(&LogicManager::CONS_Rebecca_DoCorrOtis, "623Ee", 36, 0, 0);
		} else {
			RebeccaCall(&LogicManager::CONS_Rebecca_DoCorrOtis, "624Be", 36, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
			setDoor(36, kCharacterCath, 0, 10, 9);
			setDoor(52, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterRebecca).characterPosition.location = 0;
			send(kCharacterRebecca, kCharacterSophie, 125242096, 0);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			endGraphics(kCharacterRebecca);
			break;
		case 3:
			getCharacter(kCharacterRebecca).characterPosition.position = 1540;
			getCharacter(kCharacterRebecca).characterPosition.location = 0;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoSeqOtis, "810US", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterRebecca, "012B");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterRebecca);

			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
			RebeccaCall(&LogicManager::CONS_Rebecca_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
			startCycOtis(kCharacterRebecca, "012A");

			if (_globals[kGlobalChapter] == 3)
				playDialog(kCharacterRebecca, "REB3000", -1, 0);

			send(kCharacterRebecca, kCharacterHeadWait, 269479296, 0);
			getCharacterCurrentParams(kCharacterRebecca)[1] = 1;
			break;
		default:
			break;
		}

		break;
	case 157370960:
		send(kCharacterRebecca, kCharacterTableD, 136455232, 0);
		getCharacter(kCharacterRebecca).characterPosition.location = 1;

		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_GotoSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_GotoSalon);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 17;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_GotoSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (nearChar(kCharacterRebecca, kCharacterSophie, 750) && !whoOnScreen(kCharacterSophie)) {
			send(kCharacterRebecca, kCharacterSophie, 123668192, 0);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoWait, 0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoCorrOtis, "624Be", 36, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
			setDoor(36, kCharacterCath, 0, 10, 9);
			setDoor(52, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterRebecca).characterPosition.location = 0;
			send(kCharacterRebecca, kCharacterSophie, 125242096, 0);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			endGraphics(kCharacterRebecca);
			break;
		case 3:
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
			RebeccaCall(&LogicManager::CONS_Rebecca_WaitRCClear, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterRebecca).characterPosition.position = 1540;
			getCharacter(kCharacterRebecca).characterPosition.location = 0;

			if (_globals[kGlobalChapter] == 3)
				playDialog(kCharacterRebecca, "Reb3005", -1, 75);

			if (getCharacterCurrentParams(kCharacterRebecca)[0]) {
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "118A", 5, 52, 0);
			} else {
				blockView(kCharacterRebecca, kCarRestaurant, 57);
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 6;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoComplexSeqOtis, "107A1", "107A2", 24, 0);
			}

			break;
		case 5:
			getCharacter(kCharacterRebecca).characterPosition.location = 1;

			getCharacter(kCharacterRebecca).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
			fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
			break;
		case 6:
			releaseView(kCharacterRebecca, kCarRestaurant, 57);
			endGraphics(kCharacterSophie);
			getCharacter(kCharacterRebecca).characterPosition.location = 1;

			getCharacter(kCharacterRebecca).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
			fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_ReturnFromSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_ReturnFromSalon);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 18;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_ReturnFromSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (nearChar(kCharacterRebecca, kCharacterSophie, 750) || nearX(kCharacterSophie, 4840, 500)) {
			send(kCharacterRebecca, kCharacterSophie, 123668192, 0);
			softReleaseAtDoor(kCharacterRebecca, 36);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
			RebeccaCall(&LogicManager::CONS_Rebecca_EnterComp, 0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterRebecca).characterPosition.car = 4;
		getCharacter(kCharacterRebecca).characterPosition.position = 9270;
		getCharacter(kCharacterRebecca).characterPosition.location = 0;
		send(kCharacterRebecca, kCharacterSophie, 136654208, 0);
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoWalk, 4, 4840, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1) {
			if (nearChar(kCharacterRebecca, kCharacterSophie, 750) || nearX(kCharacterSophie, 4840, 500)) {
				send(kCharacterRebecca, kCharacterSophie, 123668192, 0);
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
				RebeccaCall(&LogicManager::CONS_Rebecca_EnterComp, 0, 0, 0, 0);
			} else {
				startCycOtis(kCharacterRebecca, "623Ge");
				softBlockAtDoor(kCharacterRebecca, 36);
			}
		} else if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 2 ||
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 3) {
			getCharacter(kCharacterRebecca).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
			fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_ReturnFromDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_ReturnFromDinner);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 19;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_ReturnFromDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (nearChar(kCharacterRebecca, kCharacterSophie, 750) || nearX(kCharacterSophie, 4840, 500)) {
			send(kCharacterRebecca, kCharacterSophie, 123668192, 0);
			softReleaseAtDoor(kCharacterRebecca, 36);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 6;
			RebeccaCall(&LogicManager::CONS_Rebecca_EnterComp, 0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterRebecca).characterPosition.location = 0;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoJoinedSeqOtis, "012H", 36, 103798704, "010M");
			break;
		case 2:
			send(kCharacterRebecca, kCharacterWaiter1, 337548856, 0);
			startSeqOtis(kCharacterRebecca, "810DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterRebecca);

			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
			RebeccaCall(&LogicManager::CONS_Rebecca_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterRebecca).characterPosition.car = 4;
			getCharacter(kCharacterRebecca).characterPosition.position = 9270;
			getCharacter(kCharacterRebecca).characterPosition.location = 0;
			send(kCharacterRebecca, kCharacterSophie, 136654208, 0);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoWalk, 4, 4840, 0, 0);
			break;
		case 4:
			if (nearChar(kCharacterRebecca, kCharacterSophie, 750) || nearX(kCharacterSophie, 4840, 500)) {
				send(kCharacterRebecca, kCharacterSophie, 123668192, 0);
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
				RebeccaCall(&LogicManager::CONS_Rebecca_EnterComp, 0, 0, 0, 0);
			} else {
				startCycOtis(kCharacterRebecca, "623Ge");
				softBlockAtDoor(kCharacterRebecca, 36);
			}

			break;
		case 5:
		case 6:
			getCharacter(kCharacterRebecca).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
			fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_CompLogic(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_CompLogic);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 20;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_CompLogic(HAND_PARAMS) {
	switch (msg->action) {
	case 0: {
		if (getCharacterCurrentParams(kCharacterRebecca)[0] < _gameTime && !getCharacterCurrentParams(kCharacterRebecca)[4]) {
			getCharacterCurrentParams(kCharacterRebecca)[4] = 1;
			setDoor(36, kCharacterCath, 0, 10, 9);
			setDoor(52, kCharacterCath, 0, 10, 9);

			getCharacter(kCharacterRebecca).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
			fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
			break;
		}

		bool skip = false;

		if (getCharacterCurrentParams(kCharacterRebecca)[1]) {
			if (getCharacterCurrentParams(kCharacterRebecca)[5] || (getCharacterCurrentParams(kCharacterRebecca)[5] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterRebecca)[5] >= _realTime) {
					skip = true;
				}

				if (!skip) {
					getCharacterCurrentParams(kCharacterRebecca)[5] = 0x7FFFFFFF;
				}
			}

			if (!skip) {
				getCharacterCurrentParams(kCharacterRebecca)[1] = 0;
				getCharacterCurrentParams(kCharacterRebecca)[2] = 1;
				setDoor(36, kCharacterRebecca, 1, 0, 0);
				setDoor(52, kCharacterRebecca, 1, 0, 0);
			}
		}

		if (!skip) {
			getCharacterCurrentParams(kCharacterRebecca)[5] = 0;
		}

		if (_globals[kGlobalChapter] == 1 && !getCharacterParams(kCharacterRebecca, 8)[2]) {
			if (getCharacterCurrentParams(kCharacterRebecca)[6] != 0x7FFFFFFF && _gameTime > 1174500) {
				if (_gameTime > 1183500) {
					getCharacterCurrentParams(kCharacterRebecca)[6] = 0x7FFFFFFF;
					getCharacterParams(kCharacterRebecca, 8)[2] = 1;
					setDoor(36, kCharacterRebecca, 1, 0, 0);
					setDoor(52, kCharacterRebecca, 1, 0, 0);
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1205", 0, 0, 0);
					break;
				}

				if (!nearChar(kCharacterRebecca, kCharacterCath, 2000) || dialogRunning("CON1210") || !getCharacterCurrentParams(kCharacterRebecca)[6]) {
					getCharacterCurrentParams(kCharacterRebecca)[6] = _gameTime;
					if (!_gameTime) {
						getCharacterParams(kCharacterRebecca, 8)[2] = 1;
						setDoor(36, kCharacterRebecca, 1, 0, 0);
						setDoor(52, kCharacterRebecca, 1, 0, 0);
						getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
						RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1205", 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterRebecca)[6] < _gameTime) {
					getCharacterCurrentParams(kCharacterRebecca)[6] = 0x7FFFFFFF;
					getCharacterParams(kCharacterRebecca, 8)[2] = 1;
					setDoor(36, kCharacterRebecca, 1, 0, 0);
					setDoor(52, kCharacterRebecca, 1, 0, 0);
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1205", 0, 0, 0);
					break;
				}
			}
		} else if (_globals[kGlobalChapter] == 3 && !getCharacterParams(kCharacterRebecca, 8)[3] && getCharacterCurrentParams(kCharacterRebecca)[7] != 0x7FFFFFFF && _gameTime > 2097000) {
			if (_gameTime > 2106000) {
				getCharacterCurrentParams(kCharacterRebecca)[7] = 0x7FFFFFFF;
				getCharacterParams(kCharacterRebecca, 8)[3] = 1;
				setDoor(36, kCharacterRebecca, 1, 0, 0);
				setDoor(52, kCharacterRebecca, 1, 0, 0);
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB3010", 0, 0, 0);
				break;
			}

			if (!nearChar(kCharacterRebecca, kCharacterCath, 1000) || !getCharacterCurrentParams(kCharacterRebecca)[7]) {
				getCharacterCurrentParams(kCharacterRebecca)[7] = _gameTime;
				if (!_gameTime) {
					getCharacterParams(kCharacterRebecca, 8)[3] = 1;
					setDoor(36, kCharacterRebecca, 1, 0, 0);
					setDoor(52, kCharacterRebecca, 1, 0, 0);
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB3010", 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterRebecca)[7] < _gameTime) {
				getCharacterCurrentParams(kCharacterRebecca)[7] = 0x7FFFFFFF;
				getCharacterParams(kCharacterRebecca, 8)[3] = 1;
				setDoor(36, kCharacterRebecca, 1, 0, 0);
				setDoor(52, kCharacterRebecca, 1, 0, 0);
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB3010", 0, 0, 0);
				break;
			}
		}

		if (getCharacterParams(kCharacterRebecca, 8)[1]) {
			if (nearChar(kCharacterRebecca, kCharacterCath, 1000)) {
				setDoor(36, kCharacterRebecca, 1, 0, 0);
				setDoor(52, kCharacterRebecca, 1, 0, 0);
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1040", 0, 0, 0);
			}
		}

		break;
	}
	case 8:
	case 9:
		if (!getCharacterCurrentParams(kCharacterRebecca)[1]) {
			setDoor(36, kCharacterRebecca, 1, 0, 0);
			setDoor(52, kCharacterRebecca, 1, 0, 0);

			if (msg->action == 8) {
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "LIB013", 0, 0, 0);
			}

			break;
		}

		setDoor(36, kCharacterRebecca, 1, 0, 0);
		setDoor(52, kCharacterRebecca, 1, 0, 0);

		if (msg->param.intParam == 52) {
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 8;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, getCathWCDialog(), 0, 0, 0);
			break;
		}

		if (!cathHasItem(kItemPassengerList)) {
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 11;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, getCathSorryDialog(), 0, 0, 0);
			break;
		}

		if (rnd(2)) {
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 9;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, getCathSorryDialog(), 0, 0, 0);
			break;
		}

		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 10;

		if (getCharacterCurrentParams(kCharacterRebecca)[3]) {
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "CAT1509", 0, 0, 0);
		} else {
			if (rnd(2) == 0) {
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "CAT1508A", 0, 0, 0);
			} else {
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "CAT1508", 0, 0, 0);
			}
		}

		break;
	case 12:
		setDoor(36, kCharacterRebecca, 1, 10, 9);
		setDoor(52, kCharacterRebecca, 1, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterRebecca)[2] || getCharacterCurrentParams(kCharacterRebecca)[1]) {
			setDoor(36, kCharacterRebecca, 1, 10, 9);
			setDoor(52, kCharacterRebecca, 1, 10, 9);
			getCharacterCurrentParams(kCharacterRebecca)[2] = 0;
			getCharacterCurrentParams(kCharacterRebecca)[1] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
			setDoor(36, kCharacterRebecca, 1, 10, 9);
			setDoor(52, kCharacterRebecca, 1, 10, 9);
			getCharacterParams(kCharacterRebecca, 8)[1] = 0;
			if (getCharacterParams(kCharacterRebecca, 8)[1]) {
				if (nearChar(kCharacterRebecca, kCharacterCath, 1000)) {
					setDoor(36, kCharacterRebecca, 1, 0, 0);
					setDoor(52, kCharacterRebecca, 1, 0, 0);
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1040", 0, 0, 0);
				}
			}

			break;
		case 2:
			setDoor(36, kCharacterRebecca, 1, 10, 9);
			setDoor(52, kCharacterRebecca, 1, 10, 9);
			if (getCharacterParams(kCharacterRebecca, 8)[1]) {
				if (nearChar(kCharacterRebecca, kCharacterCath, 1000)) {
					setDoor(36, kCharacterRebecca, 1, 0, 0);
					setDoor(52, kCharacterRebecca, 1, 0, 0);
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1040", 0, 0, 0);
				}
			}

			break;
		case 3:
			setDoor(36, kCharacterRebecca, 1, 10, 9);
			setDoor(52, kCharacterRebecca, 1, 10, 9);
			getCharacterParams(kCharacterRebecca, 8)[1] = 0;
			break;
		case 4:
		case 5:
			if (rnd(2)) {
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 6;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1039", 0, 0, 0);
				break;
			}

			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 7;

			if (rnd(2) == 0) {
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "SOP1039A", 0, 0, 0);
			} else {
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "SOP1039", 0, 0, 0);
			}

			break;
		case 6:
			getCharacterCurrentParams(kCharacterRebecca)[3] = 0;
			setDoor(36, kCharacterRebecca, 1, 14, 0);
			setDoor(52, kCharacterRebecca, 1, 14, 0);
			getCharacterCurrentParams(kCharacterRebecca)[1] = 1;
			break;
		case 7:
			getCharacterCurrentParams(kCharacterRebecca)[3] = 1;
			setDoor(36, kCharacterRebecca, 1, 14, 0);
			setDoor(52, kCharacterRebecca, 1, 14, 0);
			getCharacterCurrentParams(kCharacterRebecca)[1] = 1;
			break;
		case 8:
		case 9:
		case 10:
		case 11:
			getCharacterCurrentParams(kCharacterRebecca)[1] = 0;
			getCharacterCurrentParams(kCharacterRebecca)[2] = 1;
			break;
		case 12:
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 13;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoCondDialog, "JAC1012B", 0, 0, 0);
			break;
		case 13:
			setDoor(36, kCharacterRebecca, 1, 10, 9);
			setDoor(52, kCharacterRebecca, 1, 10, 9);
			break;
		default:
			break;
		}

		break;
	case 254915200:
		setDoor(36, kCharacterRebecca, 1, 0, 0);
		setDoor(52, kCharacterRebecca, 1, 0, 0);
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 12;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1039A", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_Birth);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 21;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterRebecca)[0]) {
			getCharacterCurrentParams(kCharacterRebecca)[0] = 1;
			CONS_Rebecca_InSalon1A(0, 0, 0, 0);
		}

		break;
	case 12:
		autoMessage(kCharacterRebecca, 224253538, 0);
		setDoor(36, kCharacterCath, 0, 10, 9);
		setDoor(52, kCharacterCath, 0, 10, 9);
		setDoor(44, kCharacterCath, 0, 255, 255);
		setModel(110, 1);
		getCharacter(kCharacterRebecca).characterPosition.position = 2830;
		getCharacter(kCharacterRebecca).characterPosition.location = 1;
		getCharacter(kCharacterRebecca).characterPosition.car = kCarRestaurant;
		getCharacterParams(kCharacterRebecca, 8)[1] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_InSalon1A(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_InSalon1A);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 22;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_InSalon1A(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		if (_gameTime > 1084500 && !getCharacterCurrentParams(kCharacterRebecca)[2]) {
			getCharacterCurrentParams(kCharacterRebecca)[2] = 1;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1015", 0, 0, 0);
			break;
		}

		bool skip = false;

		if (getCharacterCurrentParams(kCharacterRebecca)[3] == 0x7FFFFFFF || !_gameTime) {
			skip = true;
		}

		if (!skip) {
			if (_gameTime > 1080000) {
				getCharacterCurrentParams(kCharacterRebecca)[3] = 0x7FFFFFFF;
				if (inSalon(kCharacterCath))
					_globals[kGlobalMetSophieAndRebecca] = 1;

				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1012", 0, 0, 0);
				break;
			}

			if (!inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterRebecca)[3]) {
				getCharacterCurrentParams(kCharacterRebecca)[3] = _gameTime + 150;
				if (_gameTime == -150) {
					if (inSalon(kCharacterCath))
						_globals[kGlobalMetSophieAndRebecca] = 1;

					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1012", 0, 0, 0);
					break;
				}
			}
		}

		if (skip || getCharacterCurrentParams(kCharacterRebecca)[3] >= _gameTime) {
			if (getCharacterCurrentParams(kCharacterRebecca)[0]) {
				if ((!getCharacterCurrentParams(kCharacterRebecca)[4] && (getCharacterCurrentParams(kCharacterRebecca)[4] = _gameTime + 900, _gameTime == -900)) || getCharacterCurrentParams(kCharacterRebecca)[4] < _gameTime) {
					if (inSalon(kCharacterCath)) {
						getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
						RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1013", 0, 0, 0);
						break;
					}
				}
			}

			if (getCharacterCurrentParams(kCharacterRebecca)[1]) {
				if (getCharacterCurrentParams(kCharacterRebecca)[5] || (getCharacterCurrentParams(kCharacterRebecca)[5] = _realTime + 90, _realTime != -90)) {
					if (getCharacterCurrentParams(kCharacterRebecca)[5] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterRebecca)[5] = 0x7FFFFFFF;
				}

				bumpCath(kCarRestaurant, 55, 255);
			} else {
				getCharacterCurrentParams(kCharacterRebecca)[5] = 0;
			}
		} else {
			getCharacterCurrentParams(kCharacterRebecca)[3] = 0x7FFFFFFF;

			if (inSalon(kCharacterCath))
				_globals[kGlobalMetSophieAndRebecca] = 1;

			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1012", 0, 0, 0);
		}

		break;
	}
	case 12:
		startCycOtis(kCharacterRebecca, "107B");
		break;
	case 17:
		getCharacterCurrentParams(kCharacterRebecca)[1] = checkCathDir(kCarRestaurant, 57);
		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "107C", 5, 57, 0);
			break;
		case 2:
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
			RebeccaCall(&LogicManager::CONS_Rebecca_ReturnFromSalon, 0, 0, 0, 0);
			break;
		case 3:
			CONS_Rebecca_InComp1(0, 0, 0, 0);
			break;
		case 4:
			getCharacterCurrentParams(kCharacterRebecca)[0] = 1;
			if (getCharacterCurrentParams(kCharacterRebecca)[0]) {
				if ((!getCharacterCurrentParams(kCharacterRebecca)[4] && (getCharacterCurrentParams(kCharacterRebecca)[4] = _gameTime + 900, _gameTime == -900)) || getCharacterCurrentParams(kCharacterRebecca)[4] < _gameTime) {
					if (inSalon(kCharacterCath)) {
						getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
						RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1013", 0, 0, 0);
						break;
					}
				}
			}

			if (getCharacterCurrentParams(kCharacterRebecca)[1]) {
				if (getCharacterCurrentParams(kCharacterRebecca)[5] || (getCharacterCurrentParams(kCharacterRebecca)[5] = _realTime + 90, _realTime != -90)) {
					if (getCharacterCurrentParams(kCharacterRebecca)[5] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterRebecca)[5] = 0x7FFFFFFF;
				}

				bumpCath(kCarRestaurant, 55, 255);
			} else {
				getCharacterCurrentParams(kCharacterRebecca)[5] = 0;
			}

			break;
		case 5:
			_globals[kGlobalOverheardSophieTalkingAboutCath] = 1;
			getCharacterCurrentParams(kCharacterRebecca)[0] = 0;

			if (getCharacterCurrentParams(kCharacterRebecca)[1]) {
				if (getCharacterCurrentParams(kCharacterRebecca)[5] || (getCharacterCurrentParams(kCharacterRebecca)[5] = _realTime + 90, _realTime != -90)) {
					if (getCharacterCurrentParams(kCharacterRebecca)[5] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterRebecca)[5] = 0x7FFFFFFF;
				}

				bumpCath(kCarRestaurant, 55, 255);
			} else {
				getCharacterCurrentParams(kCharacterRebecca)[5] = 0;
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

void LogicManager::CONS_Rebecca_InComp1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_InComp1);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 23;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_InComp1(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1111500 && !getCharacterCurrentParams(kCharacterRebecca)[1]) {
			getCharacterCurrentParams(kCharacterRebecca)[1] = 1;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoCorrOtis, "623De", 36, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoWait, 900, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoCorrOtis, "623Ce", 36, 0, 0);
			break;
		case 2:
			setDoor(36, kCharacterCath, 2, 0, 0);
			startCycOtis(kCharacterRebecca, "504");
			break;
		case 3:
			endGraphics(kCharacterRebecca);
			getCharacter(kCharacterRebecca).characterPosition.position = 4840;
			getCharacter(kCharacterRebecca).characterPosition.location = 1;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
			RebeccaCall(&LogicManager::CONS_Rebecca_CompLogic, 1120500, 0, 0, 0);
			break;
		case 4:
		case 5:
			if (getCharacterParams(kCharacterRebecca, 8)[0]) {
				CONS_Rebecca_GoDinner(0, 0, 0, 0);
			} else {
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
				RebeccaCall(&LogicManager::CONS_Rebecca_CompLogic, _gameTime + 900, 0, 0, 0);
			}

			break;
		case 6:
			endGraphics(kCharacterRebecca);
			getCharacter(kCharacterRebecca).characterPosition.position = 4840;
			getCharacter(kCharacterRebecca).characterPosition.location = 1;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 7;
			RebeccaCall(&LogicManager::CONS_Rebecca_CompLogic, 1120500, 0, 0, 0);
			break;
		case 7:
		case 8:
			if (getCharacterParams(kCharacterRebecca, 8)[0]) {
				CONS_Rebecca_GoDinner(0, 0, 0, 0);
			} else {
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 8;
				RebeccaCall(&LogicManager::CONS_Rebecca_CompLogic, _gameTime + 900, 0, 0, 0);
			}

			break;
		default:
			break;
		}

		break;
	case 285528346:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 6;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoCorrOtis, "623De", 36, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_GoDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_GoDinner);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 24;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_GoDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1134000 && !getCharacterCurrentParams(kCharacterRebecca)[1]) {
			getCharacterCurrentParams(kCharacterRebecca)[1] = 1;
			send(kCharacterRebecca, kCharacterWaiter1, 223712416, 0);
		}

		if (getCharacterCurrentParams(kCharacterRebecca)[0]) {
			if (_gameTime > 1165500 && !getCharacterCurrentParams(kCharacterRebecca)[2]) {
				getCharacterCurrentParams(kCharacterRebecca)[2] = 1;
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 6;
				RebeccaCall(&LogicManager::CONS_Rebecca_ReturnFromDinner, 0, 0, 0, 0);
				break;
			}

			if (getCharacterCurrentParams(kCharacterRebecca)[3] != 0x7FFFFFFF && _gameTime) {
				if (_gameTime <= 1161000) {
					if (!inDiningRoom(kCharacterCath) || !getCharacterCurrentParams(kCharacterRebecca)[3]) {
						getCharacterCurrentParams(kCharacterRebecca)[3] = _gameTime + 150;
						if (_gameTime == -150) {
							getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 7;
							RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1200A", 0, 0, 0);
							break;
						}
					}

					if (getCharacterCurrentParams(kCharacterRebecca)[3] >= _gameTime)
						break;
				}

				getCharacterCurrentParams(kCharacterRebecca)[3] = 0x7FFFFFFF;
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 7;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1200A", 0, 0, 0);
			}
		}

		break;
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_GotoDinner, 1, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterRebecca, "012D");
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1199", 0, 0, 0);
			break;
		case 2:
			if (!inDiningRoom(kCharacterCath)) {
				if (!inDiningRoom(kCharacterAnna)) {
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1199C", 0, 0, 0);
				} else {
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1199B", 0, 0, 0);
				}
			} else {
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1199A", 0, 0, 0);
			}

			break;
		case 3:
			_globals[kGlobalKnowAboutRebeccaAndSophieRelationship] = 1;

			if (!inDiningRoom(kCharacterAnna)) {
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1199C", 0, 0, 0);
			} else {
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1199B", 0, 0, 0);
			}

			break;
		case 4:
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1199C", 0, 0, 0);
			break;
		case 6:
			CONS_Rebecca_AfterDinner(0, 0, 0, 0);
			break;
		case 8:
			send(kCharacterRebecca, kCharacterWaiter1, 136702400, 0);
			startCycOtis(kCharacterRebecca, "012G");
			getCharacterCurrentParams(kCharacterRebecca)[0] = 1;
			break;
		default:
			break;
		}

		break;
	case 123712592:
		startCycOtis(kCharacterWaiter1, "BLANK");
		startCycOtis(kCharacterRebecca, "012E");
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 8;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "REB1200", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_AfterDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_AfterDinner);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 25;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_AfterDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_CompLogic, 1184400, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1) {
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_GotoSalon, 1, 0, 0, 0);
		} else if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 2) {
			CONS_Rebecca_InSalon1B(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_InSalon1B(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_InSalon1B);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 26;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_InSalon1B(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1224000 && !getCharacterCurrentParams(kCharacterRebecca)[1]) {
			getCharacterCurrentParams(kCharacterRebecca)[1] = 1;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "118H", 5, 52, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterRebecca)[0]) {
			if (getCharacterCurrentParams(kCharacterRebecca)[2] || (getCharacterCurrentParams(kCharacterRebecca)[2] = _realTime + 90, _realTime != -90)) {
				if (getCharacterCurrentParams(kCharacterRebecca)[2] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterRebecca)[2] = 0x7FFFFFFF;
			}

			bumpCath(kCarRestaurant, 51, 255);
		}

		break;
	case 12:
		startCycOtis(kCharacterRebecca, "118D");
		break;
	case 17:
		getCharacterCurrentParams(kCharacterRebecca)[0] = 1;

		if (!checkCathDir(kCarRestaurant, 52))
			getCharacterCurrentParams(kCharacterRebecca)[0] = 0;

		getCharacterCurrentParams(kCharacterRebecca)[2] = 0;
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1) {
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_ReturnFromSalon, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 2) {
			CONS_Rebecca_Asleep(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_Asleep);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 27;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterRebecca).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterRebecca).characterPosition.position = 4840;
		getCharacter(kCharacterRebecca).characterPosition.location = 1;
		setDoor(36, kCharacterCath, 1, 10, 9);
		setDoor(52, kCharacterCath, 1, 10, 9);
		endGraphics(kCharacterRebecca);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_StartPart2);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 28;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Rebecca_InComp2(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterRebecca);
		getCharacter(kCharacterRebecca).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterRebecca).characterPosition.position = 4840;
		getCharacter(kCharacterRebecca).characterPosition.location = 1;
		getCharacter(kCharacterRebecca).clothes = 0;
		getCharacter(kCharacterRebecca).inventoryItem = 0;
		setDoor(36, kCharacterCath, 1, 10, 9);
		setDoor(52, kCharacterCath, 1, 10, 9);
		setModel(110, 2);
		getCharacterParams(kCharacterRebecca, 8)[1] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_InComp2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_InComp2);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 29;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_InComp2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_CompLogic, 1764000, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1) {
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_GotoSalon, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 2) {
			CONS_Rebecca_InSalon2(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_InSalon2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_InSalon2);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 30;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_InSalon2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterRebecca)[0] && getCharacterCurrentParams(kCharacterRebecca)[3] != 0x7FFFFFFF && _gameTime) {
			bool skip = false;

			if (_gameTime > 15803100) {
				getCharacterCurrentParams(kCharacterRebecca)[3] = 0x7FFFFFFF;
				playDialog(kCharacterRebecca, "Reb2001", -1, 0);
				_globals[kGlobalKnowAboutRebeccaDiary] = 1;
				getCharacterCurrentParams(kCharacterRebecca)[1] = 1;
				skip = true;
			}

			if (!skip) {
				if (!inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterRebecca)[3]) {
					getCharacterCurrentParams(kCharacterRebecca)[3] = _gameTime + 450;
					if (_gameTime == -450) {
						skip = true;
						playDialog(kCharacterRebecca, "Reb2001", -1, 0);
						_globals[kGlobalKnowAboutRebeccaDiary] = 1;
						getCharacterCurrentParams(kCharacterRebecca)[1] = 1;
					}
				}

				if (!skip && getCharacterCurrentParams(kCharacterRebecca)[3] < _gameTime) {
					getCharacterCurrentParams(kCharacterRebecca)[3] = 0x7FFFFFFF;
					playDialog(kCharacterRebecca, "Reb2001", -1, 0);
					_globals[kGlobalKnowAboutRebeccaDiary] = 1;
					getCharacterCurrentParams(kCharacterRebecca)[1] = 1;
				}
			}
		}

		if (getCharacterCurrentParams(kCharacterRebecca)[2] && !getCharacterCurrentParams(kCharacterRebecca)[1] && getCharacterCurrentParams(kCharacterRebecca)[4] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime <= 10881000) {
				if (!inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterRebecca)[4]) {
					getCharacterCurrentParams(kCharacterRebecca)[4] = _gameTime + 450;
					if (_gameTime == -450) {
						send(kCharacterRebecca, kCharacterAugust, 169358379, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterRebecca)[4] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterRebecca)[4] = 0x7FFFFFFF;
			send(kCharacterRebecca, kCharacterAugust, 169358379, 0);
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterRebecca)[1] = 0;
		break;
	case 12:
		startCycOtis(kCharacterRebecca, "107B");
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1) {
			CONS_Rebecca_OpenComp2(0, 0, 0, 0);
		}

		break;
	case 125496184:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_ReturnFromSalon, 0, 0, 0, 0);
		break;
	case 155465152:
		startCycOtis(kCharacterRebecca, "BLANK");
		break;
	case 155980128:
		getCharacterCurrentParams(kCharacterRebecca)[2] = 1;
		getCharacterCurrentParams(kCharacterRebecca)[0] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_OpenComp2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_OpenComp2);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 31;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_OpenComp2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoWait, 900, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1) {
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoCorrOtis, "623CE", 36, 0, 0);
		} else if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 2) {
			setDoor(36, kCharacterCath, 2, 0, 0);
			startCycOtis(kCharacterRebecca, "504");
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_StartPart3);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 32;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Rebecca_InComp3a(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterRebecca);
		getCharacter(kCharacterRebecca).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterRebecca).characterPosition.position = 4840;
		getCharacter(kCharacterRebecca).characterPosition.location = 1;
		getCharacter(kCharacterRebecca).clothes = 0;
		getCharacter(kCharacterRebecca).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_InComp3a(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_InComp3a);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 33;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_InComp3a(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_CompLogic, 2016000, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1) {
			CONS_Rebecca_GoLunch(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_GoLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_GoLunch);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 34;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_GoLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterRebecca)[1] == 0x7FFFFFFF || !_gameTime) {
			if (getCharacterCurrentParams(kCharacterRebecca)[0] && _gameTime > 2052000 && !getCharacterCurrentParams(kCharacterRebecca)[2]) {
				getCharacterCurrentParams(kCharacterRebecca)[2] = 1;
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
				RebeccaCall(&LogicManager::CONS_Rebecca_ReturnFromDinner, 0, 0, 0, 0);
			}

			break;
		}

		if (_gameTime <= 1386000) {
			if (!inDiningRoom(kCharacterCath) || !getCharacterCurrentParams(kCharacterRebecca)[1]) {
				getCharacterCurrentParams(kCharacterRebecca)[1] = _gameTime;
				if (!_gameTime) {
					send(kCharacterRebecca, kCharacterWaiter1, 223712416, 0);

					if (getCharacterCurrentParams(kCharacterRebecca)[0] && _gameTime > 2052000 && !getCharacterCurrentParams(kCharacterRebecca)[2]) {
						getCharacterCurrentParams(kCharacterRebecca)[2] = 1;
						getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
						RebeccaCall(&LogicManager::CONS_Rebecca_ReturnFromDinner, 0, 0, 0, 0);
					}

					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterRebecca)[1] >= _gameTime) {
				if (getCharacterCurrentParams(kCharacterRebecca)[0] && _gameTime > 2052000 && !getCharacterCurrentParams(kCharacterRebecca)[2]) {
					getCharacterCurrentParams(kCharacterRebecca)[2] = 1;
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
					RebeccaCall(&LogicManager::CONS_Rebecca_ReturnFromDinner, 0, 0, 0, 0);
				}

				break;
			}
		}

		getCharacterCurrentParams(kCharacterRebecca)[1] = 0x7FFFFFFF;
		send(kCharacterRebecca, kCharacterWaiter1, 223712416, 0);

		if (getCharacterCurrentParams(kCharacterRebecca)[0] && _gameTime > 2052000 && !getCharacterCurrentParams(kCharacterRebecca)[2]) {
			getCharacterCurrentParams(kCharacterRebecca)[2] = 1;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
			RebeccaCall(&LogicManager::CONS_Rebecca_ReturnFromDinner, 0, 0, 0, 0);
		}

		break;
	case 2:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "Reb3004", 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterRebecca).characterPosition.location = 0;
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_GotoDinner, 1, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterRebecca, "012D");
			getCharacter(kCharacterRebecca).characterPosition.location = 1;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "Reb3002", 0, 0, 0);
			break;
		case 3:
			CONS_Rebecca_InComp3b(0, 0, 0, 0);
			break;
		case 4:
			send(kCharacterRebecca, kCharacterWaiter1, 136702400, 0);
			startCycOtis(kCharacterRebecca, "012G");
			getCharacterCurrentParams(kCharacterRebecca)[0] = 1;
			break;
		default:
			break;
		}

		break;
	case 123712592:
		startCycOtis(kCharacterWaiter1, "BLANK");
		playDialog(kCharacterRebecca, "Reb3003", -1, 0);
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoSeqOtis, "012E", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_InComp3b(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_InComp3b);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 35;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_InComp3b(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_CompLogic, 2070000, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1) {
			CONS_Rebecca_GoSalon(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_GoSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_GoSalon);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 36;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_GoSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterRebecca)[1])
			getCharacterCurrentParams(kCharacterRebecca)[1] = _gameTime + 1800;

		if (getCharacterCurrentParams(kCharacterRebecca)[3] != 0x7FFFFFFF && getCharacterCurrentParams(kCharacterRebecca)[1] < _gameTime) {
			if (_gameTime > 2083500) {
				getCharacterCurrentParams(kCharacterRebecca)[3] = 0x7FFFFFFF;
				playDialog(kCharacterRebecca, "Reb3007", -1, 0);
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "118E", 5, 52, 0);
				break;
			}

			if (!inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterRebecca)[3]) {
				getCharacterCurrentParams(kCharacterRebecca)[3] = _gameTime + 300;
				if (_gameTime == -300) {
					playDialog(kCharacterRebecca, "Reb3007", -1, 0);
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "118E", 5, 52, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterRebecca)[3] < _gameTime) {
				getCharacterCurrentParams(kCharacterRebecca)[3] = 0x7FFFFFFF;
				playDialog(kCharacterRebecca, "Reb3007", -1, 0);
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "118E", 5, 52, 0);
				break;
			}
		}

		if (!getCharacterCurrentParams(kCharacterRebecca)[0]) {
			if (_gameTime > 2097000 && !getCharacterCurrentParams(kCharacterRebecca)[5]) {
				getCharacterCurrentParams(kCharacterRebecca)[5] = 1;
				getCharacter(kCharacterRebecca).inventoryItem = 0;
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "118H", 5, 52, 0);
			}

			break;
		}

		if (!getCharacterCurrentParams(kCharacterRebecca)[2])
			getCharacterCurrentParams(kCharacterRebecca)[2] = _gameTime + 9000;

		if (getCharacterCurrentParams(kCharacterRebecca)[4] == 0x7FFFFFFF || getCharacterCurrentParams(kCharacterRebecca)[2] >= _gameTime) {
			if (_gameTime > 2097000 && !getCharacterCurrentParams(kCharacterRebecca)[5]) {
				getCharacterCurrentParams(kCharacterRebecca)[5] = 1;
				getCharacter(kCharacterRebecca).inventoryItem = 0;
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "118H", 5, 52, 0);
			}

			break;
		}

		if (_gameTime > 2092500) {
			getCharacterCurrentParams(kCharacterRebecca)[4] = 0x7FFFFFFF;
			getCharacter(kCharacterRebecca).inventoryItem = 0;
			playDialog(kCharacterRebecca, "Reb3008", -1, 60);
			blockView(kCharacterRebecca, kCarRestaurant, 52);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoComplexSeqOtis, "118G1", "118G2", 24, 0);
			break;
		}

		if (!inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterRebecca)[4]) {
			getCharacterCurrentParams(kCharacterRebecca)[4] = _gameTime + 300;

			if (_gameTime == -300) {
				getCharacter(kCharacterRebecca).inventoryItem = 0;
				playDialog(kCharacterRebecca, "Reb3008", -1, 60);
				blockView(kCharacterRebecca, kCarRestaurant, 52);
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoComplexSeqOtis, "118G1", "118G2", 24, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterRebecca)[4] >= _gameTime) {
			if (_gameTime > 2097000 && !getCharacterCurrentParams(kCharacterRebecca)[5]) {
				getCharacterCurrentParams(kCharacterRebecca)[5] = 1;
				getCharacter(kCharacterRebecca).inventoryItem = 0;
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "118H", 5, 52, 0);
			}
		} else {
			getCharacterCurrentParams(kCharacterRebecca)[4] = 0x7FFFFFFF;
			getCharacter(kCharacterRebecca).inventoryItem = 0;
			playDialog(kCharacterRebecca, "Reb3008", -1, 60);
			blockView(kCharacterRebecca, kCarRestaurant, 52);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoComplexSeqOtis, "118G1", "118G2", 24, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterRebecca).inventoryItem = 0;
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 6;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "SOP3008", 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_GotoSalon, 1, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterRebecca, "118D");
			break;
		case 2:
			getCharacterCurrentParams(kCharacterRebecca)[0] = 1;
			getCharacter(kCharacterRebecca).inventoryItem = 0x80;
			startCycOtis(kCharacterRebecca, "118F");

			if (!getCharacterCurrentParams(kCharacterRebecca)[0]) {
				if (_gameTime > 2097000 && !getCharacterCurrentParams(kCharacterRebecca)[5]) {
					getCharacterCurrentParams(kCharacterRebecca)[5] = 1;
					getCharacter(kCharacterRebecca).inventoryItem = 0;
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "118H", 5, 52, 0);
				}

				break;
			}

			if (!getCharacterCurrentParams(kCharacterRebecca)[2])
				getCharacterCurrentParams(kCharacterRebecca)[2] = _gameTime + 9000;

			if (getCharacterCurrentParams(kCharacterRebecca)[4] == 0x7FFFFFFF || getCharacterCurrentParams(kCharacterRebecca)[2] >= _gameTime) {
				if (_gameTime > 2097000 && !getCharacterCurrentParams(kCharacterRebecca)[5]) {
					getCharacterCurrentParams(kCharacterRebecca)[5] = 1;
					getCharacter(kCharacterRebecca).inventoryItem = 0;
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "118H", 5, 52, 0);
				}

				break;
			}

			if (_gameTime > 2092500) {
				getCharacterCurrentParams(kCharacterRebecca)[4] = 0x7FFFFFFF;
				getCharacter(kCharacterRebecca).inventoryItem = 0;
				playDialog(kCharacterRebecca, "Reb3008", -1, 60);
				blockView(kCharacterRebecca, kCarRestaurant, 52);
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoComplexSeqOtis, "118G1", "118G2", 24, 0);
				break;
			}

			if (!inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterRebecca)[4]) {
				getCharacterCurrentParams(kCharacterRebecca)[4] = _gameTime + 300;

				if (_gameTime == -300) {
					getCharacter(kCharacterRebecca).inventoryItem = 0;
					playDialog(kCharacterRebecca, "Reb3008", -1, 60);
					blockView(kCharacterRebecca, kCarRestaurant, 52);
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoComplexSeqOtis, "118G1", "118G2", 24, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterRebecca)[4] >= _gameTime) {
				if (_gameTime > 2097000 && !getCharacterCurrentParams(kCharacterRebecca)[5]) {
					getCharacterCurrentParams(kCharacterRebecca)[5] = 1;
					getCharacter(kCharacterRebecca).inventoryItem = 0;
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "118H", 5, 52, 0);
				}
			} else {
				getCharacterCurrentParams(kCharacterRebecca)[4] = 0x7FFFFFFF;
				getCharacter(kCharacterRebecca).inventoryItem = 0;
				playDialog(kCharacterRebecca, "Reb3008", -1, 60);
				blockView(kCharacterRebecca, kCarRestaurant, 52);
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoComplexSeqOtis, "118G1", "118G2", 24, 0);
			}

			break;
		case 3:
			endGraphics(kCharacterSophie);
			releaseView(kCharacterRebecca, kCarRestaurant, 52);
			startCycOtis(kCharacterRebecca, "118D");

			if (_gameTime > 2097000 && !getCharacterCurrentParams(kCharacterRebecca)[5]) {
				getCharacterCurrentParams(kCharacterRebecca)[5] = 1;
				getCharacter(kCharacterRebecca).inventoryItem = 0;
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoBlockSeqOtis, "118H", 5, 52, 0);
			}

			break;
		case 4:
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
			RebeccaCall(&LogicManager::CONS_Rebecca_ReturnFromSalon, 0, 0, 0, 0);
			break;
		case 5:
			CONS_Rebecca_ClosedComp(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_ClosedComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_ClosedComp);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 37;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_ClosedComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_CompLogic, 2110500, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1) {
			CONS_Rebecca_GoConcert(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_GoConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_GoConcert);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 38;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_GoConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterRebecca).characterPosition.location = 0;
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoBriefCorrOtis, "624Be", 36, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1) {
			setDoor(36, kCharacterCath, 0, 10, 9);
			send(kCharacterRebecca, kCharacterSophie, 259921280, 0);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoWalk, 2, 9270, 0, 0);
		} else if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 2) {
			send(kCharacterRebecca, kCharacterSophie, 123668192, 0);
			CONS_Rebecca_Concert(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_Concert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_Concert);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 39;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_Concert(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterRebecca);
		setDoor(36, kCharacterCath, 0, 10, 9);
		setDoor(52, kCharacterCath, 0, 10, 9);
		getCharacter(kCharacterRebecca).characterPosition.car = kCarKronos;
		getCharacter(kCharacterRebecca).characterPosition.position = 6000;
		getCharacter(kCharacterRebecca).characterPosition.location = 1;
		break;
	case 191668032:
		CONS_Rebecca_LeaveConcert(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_LeaveConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_LeaveConcert);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 40;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_LeaveConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterRebecca).characterPosition.position = 9270;
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoWalk, 3, 2740, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
			send(kCharacterRebecca, kCharacterSophie, 292775040, 0);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoWalk, 4, 2740, 0, 0);
			break;
		case 2:
			send(kCharacterRebecca, kCharacterAnna, 191668032, 0);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoWalk, 4, 4840, 0, 0);
			break;
		case 3:
			send(kCharacterRebecca, kCharacterSophie, 123668192, 0);
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
			RebeccaCall(&LogicManager::CONS_Rebecca_EnterComp, 0, 0, 0, 0);
			break;
		case 4:
			CONS_Rebecca_SiestaTime(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_SiestaTime(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_SiestaTime);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 41;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_SiestaTime(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacterParams(kCharacterRebecca, 8)[1] = 1;
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_CompLogic, 15803100, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_StartPart4);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 42;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Rebecca_InComp4(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterRebecca);
		getCharacter(kCharacterRebecca).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterRebecca).characterPosition.position = 4840;
		getCharacter(kCharacterRebecca).characterPosition.location = 1;
		getCharacter(kCharacterRebecca).clothes = 0;
		getCharacter(kCharacterRebecca).inventoryItem = 0;
		setModel(110, 3);
		getCharacterParams(kCharacterRebecca, 8)[0] = 0;
		getCharacterParams(kCharacterRebecca, 8)[1] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_InComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_InComp4);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 43;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_InComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_CompLogic, 2385000, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1 || getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 2) {
			if (getCharacterParams(kCharacterRebecca, 8)[0]) {
				CONS_Rebecca_GoDinner4(0, 0, 0, 0);
			} else {
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
				RebeccaCall(&LogicManager::CONS_Rebecca_CompLogic, _gameTime + 900, 0, 0, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_GoDinner4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_GoDinner4);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 44;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_GoDinner4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterRebecca)[2] != 0x7FFFFFFF && _gameTime) {
			bool skip = false;

			if (_gameTime > 2412000) {
				getCharacterCurrentParams(kCharacterRebecca)[2] = 0x7FFFFFFF;
				send(kCharacterRebecca, kCharacterWaiter1, 223712416, 0);
				skip = true;
			}

			if (!skip) {
				if (!inDiningRoom(kCharacterCath) || !getCharacterCurrentParams(kCharacterRebecca)[2]) {
					getCharacterCurrentParams(kCharacterRebecca)[2] = _gameTime;
					if (!_gameTime) {
						skip = true;
						send(kCharacterRebecca, kCharacterWaiter1, 223712416, 0);
					}
				}

				if (!skip && getCharacterCurrentParams(kCharacterRebecca)[2] < _gameTime) {
					getCharacterCurrentParams(kCharacterRebecca)[2] = 0x7FFFFFFF;
					send(kCharacterRebecca, kCharacterWaiter1, 223712416, 0);
				}
			}
		}

		if (getCharacterCurrentParams(kCharacterRebecca)[0] && getCharacterCurrentParams(kCharacterRebecca)[3] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime > 2430000) {
				getCharacterCurrentParams(kCharacterRebecca)[3] = 0x7FFFFFFF;
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "Reb4004", 0, 0, 0);
				break;
			}

			if (!inDiningRoom(kCharacterCath) || !getCharacterCurrentParams(kCharacterRebecca)[3]) {
				getCharacterCurrentParams(kCharacterRebecca)[3] = _gameTime + 150;
				if (_gameTime == -150) {
					getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
					RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "Reb4004", 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterRebecca)[3] < _gameTime) {
				getCharacterCurrentParams(kCharacterRebecca)[3] = 0x7FFFFFFF;
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "Reb4004", 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterRebecca)[1] && _gameTime > 2443500 && !getCharacterCurrentParams(kCharacterRebecca)[4]) {
			getCharacterCurrentParams(kCharacterRebecca)[4] = 1;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
			RebeccaCall(&LogicManager::CONS_Rebecca_ReturnFromDinner, 0, 0, 0, 0);
		}

		break;
	case 2:
		if (inDiningRoom(kCharacterCath)) {
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "Reb4004", 0, 0, 0);
		} else {
			getCharacterCurrentParams(kCharacterRebecca)[0] = 1;
		}

		break;
	case 12:
		getCharacter(kCharacterRebecca).characterPosition.location = 0;
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_GotoDinner, 1, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterRebecca, "012D");
			getCharacter(kCharacterRebecca).characterPosition.location = 1;
			break;
		case 2:
			if (getCharacterCurrentParams(kCharacterRebecca)[1] && _gameTime > 2443500 && !getCharacterCurrentParams(kCharacterRebecca)[4]) {
				getCharacterCurrentParams(kCharacterRebecca)[4] = 1;
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
				RebeccaCall(&LogicManager::CONS_Rebecca_ReturnFromDinner, 0, 0, 0, 0);
			}

			break;
		case 3:
			CONS_Rebecca_Asleep4(0, 0, 0, 0);
			break;
		case 4:
			send(kCharacterRebecca, kCharacterWaiter1, 136702400, 0);
			startCycOtis(kCharacterRebecca, "012G");
			getCharacterCurrentParams(kCharacterRebecca)[1] = 1;
			break;
		default:
			break;
		}

		break;
	case 123712592:
		startCycOtis(kCharacterWaiter1, "BLANK");
		playDialog(kCharacterRebecca, "Reb4003", -1, 0);
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoSeqOtis, "012E", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_Asleep4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_Asleep4);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 45;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_Asleep4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterRebecca).characterPosition.car = 4;
		getCharacter(kCharacterRebecca).characterPosition.position = 4840;
		getCharacter(kCharacterRebecca).characterPosition.location = 1;
		setDoor(36, kCharacterCath, 1, 10, 9);
		setDoor(52, kCharacterCath, 1, 10, 9);
		endGraphics(kCharacterRebecca);
		break;
	case 18:
		if (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] == 1)
			getCharacterCurrentParams(kCharacterRebecca)[0] = 1;

		break;
	case 205034665:
		if (!getCharacterCurrentParams(kCharacterRebecca)[0] && _gameTime < 2511000) {
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "Reb6969", 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_StartPart5);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 46;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Rebecca_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterRebecca);
		getCharacter(kCharacterRebecca).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterRebecca).characterPosition.position = 3969;
		getCharacter(kCharacterRebecca).characterPosition.location = 1;
		getCharacter(kCharacterRebecca).inventoryItem = 0;
		setModel(110, 4);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_Prisoner);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 47;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Rebecca_Hiding(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_Hiding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_Hiding);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 48;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_Hiding(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterRebecca)[0]) {
			if (getCharacterCurrentParams(kCharacterRebecca)[2] || (getCharacterCurrentParams(kCharacterRebecca)[2] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterRebecca)[2] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterRebecca)[2] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterRebecca)[0] = 0;
			getCharacterCurrentParams(kCharacterRebecca)[1] = 1;
			setDoor(36, kCharacterRebecca, 1, 0, 0);
			getCharacterCurrentParams(kCharacterRebecca)[2] = 0;
		} else {
			getCharacterCurrentParams(kCharacterRebecca)[2] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterRebecca)[0]) {
			setDoor(36, kCharacterRebecca, 1, 0, 0);
			getCharacterCurrentParams(kCharacterRebecca)[1] = 1;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, getCathJustChecking(), 0, 0, 0);
		} else {
			setDoor(36, kCharacterRebecca, 1, 0, 0);
			if (msg->action == 8) {
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
				RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		getCharacter(kCharacterRebecca).characterPosition.car = 4;
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
		RebeccaCall(&LogicManager::CONS_Rebecca_DoCorrOtis, "624AE", 36, 0, 0);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterRebecca)[1] || getCharacterCurrentParams(kCharacterRebecca)[0]) {
			getCharacterCurrentParams(kCharacterRebecca)[1] = 0;
			getCharacterCurrentParams(kCharacterRebecca)[0] = 0;
			setDoor(36, kCharacterRebecca, 1, 10, 9);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
			endGraphics(kCharacterRebecca);
			getCharacter(kCharacterRebecca).characterPosition.location = 1;
			getCharacter(kCharacterRebecca).characterPosition.position = 4840;
			setDoor(36, kCharacterRebecca, 1, 10, 9);
			break;
		case 3:
		case 4:
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 5;
			RebeccaCall(&LogicManager::CONS_Rebecca_DoDialog, "Reb5001", 0, 0, 0);
			break;
		case 5:
			getCharacterCurrentParams(kCharacterRebecca)[0] = 1;
			setDoor(36, kCharacterRebecca, 1, 14, 0);
			break;
		default:
			break;
		}

		break;
	case 135800432:
		CONS_Rebecca_Disappear(0, 0, 0, 0);
		break;
	case 155604840:
		setDoor(36, kCharacterRebecca, 1, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Rebecca_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_Rebecca_Disappear);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 49;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_Rebecca_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsRebecca[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Rebecca_DebugWalks,
	&LogicManager::HAND_Rebecca_DoWait,
	&LogicManager::HAND_Rebecca_DoDialog,
	&LogicManager::HAND_Rebecca_DoCondDialog,
	&LogicManager::HAND_Rebecca_DoJoinedSeqOtis,
	&LogicManager::HAND_Rebecca_DoSeqOtis,
	&LogicManager::HAND_Rebecca_DoCorrOtis,
	&LogicManager::HAND_Rebecca_DoEnterCorrOtis,
	&LogicManager::HAND_Rebecca_DoBriefCorrOtis,
	&LogicManager::HAND_Rebecca_FinishSeqOtis,
	&LogicManager::HAND_Rebecca_WaitRCClear,
	&LogicManager::HAND_Rebecca_DoWalk,
	&LogicManager::HAND_Rebecca_DoBlockSeqOtis,
	&LogicManager::HAND_Rebecca_DoComplexSeqOtis,
	&LogicManager::HAND_Rebecca_EnterComp,
	&LogicManager::HAND_Rebecca_GotoDinner,
	&LogicManager::HAND_Rebecca_GotoSalon,
	&LogicManager::HAND_Rebecca_ReturnFromSalon,
	&LogicManager::HAND_Rebecca_ReturnFromDinner,
	&LogicManager::HAND_Rebecca_CompLogic,
	&LogicManager::HAND_Rebecca_Birth,
	&LogicManager::HAND_Rebecca_InSalon1A,
	&LogicManager::HAND_Rebecca_InComp1,
	&LogicManager::HAND_Rebecca_GoDinner,
	&LogicManager::HAND_Rebecca_AfterDinner,
	&LogicManager::HAND_Rebecca_InSalon1B,
	&LogicManager::HAND_Rebecca_Asleep,
	&LogicManager::HAND_Rebecca_StartPart2,
	&LogicManager::HAND_Rebecca_InComp2,
	&LogicManager::HAND_Rebecca_InSalon2,
	&LogicManager::HAND_Rebecca_OpenComp2,
	&LogicManager::HAND_Rebecca_StartPart3,
	&LogicManager::HAND_Rebecca_InComp3a,
	&LogicManager::HAND_Rebecca_GoLunch,
	&LogicManager::HAND_Rebecca_InComp3b,
	&LogicManager::HAND_Rebecca_GoSalon,
	&LogicManager::HAND_Rebecca_ClosedComp,
	&LogicManager::HAND_Rebecca_GoConcert,
	&LogicManager::HAND_Rebecca_Concert,
	&LogicManager::HAND_Rebecca_LeaveConcert,
	&LogicManager::HAND_Rebecca_SiestaTime,
	&LogicManager::HAND_Rebecca_StartPart4,
	&LogicManager::HAND_Rebecca_InComp4,
	&LogicManager::HAND_Rebecca_GoDinner4,
	&LogicManager::HAND_Rebecca_Asleep4,
	&LogicManager::HAND_Rebecca_StartPart5,
	&LogicManager::HAND_Rebecca_Prisoner,
	&LogicManager::HAND_Rebecca_Hiding,
	&LogicManager::HAND_Rebecca_Disappear
};

} // End of namespace LastExpress
