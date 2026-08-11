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

void LogicManager::CONS_Madame(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMadame,
			_functionsMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]
		);

		break;
	case 1:
		CONS_Madame_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Madame_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Madame_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Madame_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Madame_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::MadameCall(CALL_PARAMS) {
	getCharacter(kCharacterMadame).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Madame_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_DebugWalks);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 1;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterMadame, kCarGreenSleeping, getCharacterCurrentParams(kCharacterMadame)[0])) {
			if (getCharacterCurrentParams(kCharacterMadame)[0] == 10000)
				getCharacterCurrentParams(kCharacterMadame)[0] = 0;
			else
				getCharacterCurrentParams(kCharacterMadame)[0] = 10000;
		}

		break;
	case 12:
		getCharacter(kCharacterMadame).characterPosition.position = 0;
		getCharacter(kCharacterMadame).characterPosition.location = 0;
		getCharacter(kCharacterMadame).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterMadame)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_DoDialog);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterMadame).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMadame, _functionsMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]);
		fedEx(kCharacterMadame, kCharacterMadame, 18, 0);
		break;
	case 12:
		playDialog(kCharacterMadame, (char *)&getCharacterCurrentParams(kCharacterMadame)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_DoSeqOtis);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterMadame).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMadame, _functionsMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]);
		fedEx(kCharacterMadame, kCharacterMadame, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMadame, (char *)&getCharacterCurrentParams(kCharacterMadame)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_DoWait);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 4;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMadame)[1] || (getCharacterCurrentParams(kCharacterMadame)[1] = _gameTime + getCharacterCurrentParams(kCharacterMadame)[0],
															   _gameTime + getCharacterCurrentParams(kCharacterMadame)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterMadame)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterMadame)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterMadame).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMadame, _functionsMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]);
		fedEx(kCharacterMadame, kCharacterMadame, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_DoCorrOtis);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterMadame, getCharacterCurrentParams(kCharacterMadame)[3]);

		getCharacter(kCharacterMadame).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMadame, _functionsMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]);
		fedEx(kCharacterMadame, kCharacterMadame, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMadame, (char *)&getCharacterCurrentParams(kCharacterMadame)[0]);
		blockAtDoor(kCharacterMadame, getCharacterCurrentParams(kCharacterMadame)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_DoEnterCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_DoEnterCorrOtis);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_DoEnterCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterMadame, getCharacterCurrentParams(kCharacterMadame)[3]);
		getCharacter(kCharacterMadame).characterPosition.position = 5790;

		getCharacter(kCharacterMadame).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMadame, _functionsMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]);
		fedEx(kCharacterMadame, kCharacterMadame, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMadame, (char *)&getCharacterCurrentParams(kCharacterMadame)[0]);
		blockAtDoor(kCharacterMadame, getCharacterCurrentParams(kCharacterMadame)[3]);
		getCharacter(kCharacterMadame).characterPosition.location = 1;

		if (inComp(kCharacterCath, kCarRedSleeping, 5790) || inComp(kCharacterCath, kCarRedSleeping, 6130)) {
			if (isNight()) {
				playNIS(kEventCathTurningNight);
			} else {
				playNIS(kEventCathTurningDay);
			}
			
			playDialog(0, "BUMP", -1, 0);
			bumpCathFDoor(35);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_DoWalk);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 7;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterMadame, getCharacterCurrentParams(kCharacterMadame)[0], getCharacterCurrentParams(kCharacterMadame)[1])) {
			getCharacter(kCharacterMadame).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMadame, _functionsMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]);
			fedEx(kCharacterMadame, kCharacterMadame, 18, 0);
		}

		break;
	case 5:
		if (cathHasItem(kItemPassengerList)) {
			playDialog(kCharacterCath, "CAT1021", -1, 0);
		} else {
			playCathExcuseMe();
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterMadame, kCharacterCath, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_ComplainCond2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_ComplainCond2);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 8;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_ComplainCond2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMadame)[4] && getCharacterCurrentParams(kCharacterMadame)[3]) {
			send(kCharacterMadame, kCharacterCond2, 125499160, 0);
			if (!checkCathDir(kCarRedSleeping, 2))
				getCharacter(kCharacterMadame).characterPosition.position = 2088;

			getCharacter(kCharacterMadame).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMadame, _functionsMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]);
			fedEx(kCharacterMadame, kCharacterMadame, 18, 0);
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterMadame)[4] = 1;
		break;
	case 12:
		startCycOtis(kCharacterMadame, "606U");
		send(kCharacterMadame, kCharacterCond2, 169557824, 0);
		break;
	case 155853632:
		getCharacterCurrentParams(kCharacterMadame)[3] = 1;
		break;
	case 202558662:
		startCycOtis(kCharacterMadame, "606L");
		playDialog(kCharacterMadame, (char *)&getCharacterCurrentParams(kCharacterMadame)[0], -1, 0);
		if (whoOnScreen(kCharacterMadame) || nearChar(kCharacterMadame, kCharacterCath, 2000)) {
			if (_globals[kGlobalChapter] == 1) {
				_globals[kGlobalMadameComplainedAboutMax] = 1;
			} else if (_globals[kGlobalChapter] == 3) {
				_globals[kGlobalMadameDemandedMaxInBaggage] = 1;
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_LetsGo(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_LetsGo);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 9;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_LetsGo(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterMadame)[0]) {
			getCharacter(kCharacterMadame).characterPosition = getCharacter(kCharacterMonsieur).characterPosition;
		}

		break;
	case 12:
		setDoor(35, kCharacterCath, 0, 10, 9);
		setDoor(51, kCharacterCath, 0, 10, 9);
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
		MadameCall(&LogicManager::CONS_Madame_DoCorrOtis, "606Rd", 35, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMadame).characterPosition.location = 0;
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
			MadameCall(&LogicManager::CONS_Madame_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			endGraphics(kCharacterMadame);
			send(kCharacterMadame, kCharacterMonsieur, 203520448, 0);
			break;
		case 3:
			if (inComp(kCharacterFrancois, kCarRedSleeping, 5790)) {
				setDoor(35, kCharacterCath, 0, 0, 0);
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 4;
				MadameCall(&LogicManager::CONS_Madame_DoEnterCorrOtis, "606Ad", 35, 0, 0);
			} else {
				getCharacterCurrentParams(kCharacterMadame)[0] = 1;
				startCycOtis(kCharacterMadame, "606Md");
				softBlockAtDoor(kCharacterMadame, 35);
			}

			break;
		case 4:
			setDoor(35, kCharacterCath, 2, 0, 0);
			getCharacter(kCharacterMadame).characterPosition.location = 1;
			endGraphics(kCharacterMadame);

			getCharacter(kCharacterMadame).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMadame, _functionsMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]);
			fedEx(kCharacterMadame, kCharacterMadame, 18, 0);
			break;
		case 5:
			softReleaseAtDoor(kCharacterMadame, 35);
			setDoor(35, kCharacterCath, 2, 0, 0);
			getCharacter(kCharacterMadame).characterPosition.location = 1;
			endGraphics(kCharacterMadame);

			getCharacter(kCharacterMadame).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMadame, _functionsMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]);
			fedEx(kCharacterMadame, kCharacterMadame, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 100901266:
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 3;
		MadameCall(&LogicManager::CONS_Madame_DoWalk, 4, 5790, 0, 0);
		break;
	case 100957716:
		setDoor(35, kCharacterCath, 0, 0, 0);
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 5;
		MadameCall(&LogicManager::CONS_Madame_DoEnterCorrOtis, "606Ad", 35, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_Birth);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 10;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterMadame)[0]) {
			getCharacterCurrentParams(kCharacterMadame)[0] = 1;
			CONS_Madame_OpenComp1(0, 0, 0, 0);
		}

		break;
	case 12:
		setDoor(35, kCharacterCath, 2, 0, 0);
		setDoor(51, kCharacterCath, 0, 10, 9);
		autoMessage(kCharacterMadame, 242526416, 0);
		getCharacter(kCharacterMadame).characterPosition.position = 5790;
		getCharacter(kCharacterMadame).characterPosition.location = 1;
		getCharacter(kCharacterMadame).characterPosition.car = kCarRedSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_FranStory(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_FranStory);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 11;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_FranStory(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMadame)[1] != 0x7FFFFFFF && _gameTime) {
			if (getCharacterCurrentParams(kCharacterMadame)[0] >= _gameTime) {
				if (!nearChar(kCharacterMadame, kCharacterCath, 1000) || !getCharacterCurrentParams(kCharacterMadame)[1]) {
					getCharacterCurrentParams(kCharacterMadame)[1] = _gameTime + 150;
					if (_gameTime == -150) {
						getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
						MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1040", 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterMadame)[1] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterMadame)[1] = 0x7FFFFFFF;
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
			MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1040", 0, 0, 0);
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterMadame)[0] = _gameTime + 1800;
		setDoor(35, kCharacterMadame, 1, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
			MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1040A", 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 3;
			MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1041", 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 4;
			MadameCall(&LogicManager::CONS_Madame_DoWait, 900, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterMadame).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMadame, _functionsMadame[getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall]]);
			fedEx(kCharacterMadame, kCharacterMadame, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_OpenComp1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_OpenComp1);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 12;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_OpenComp1(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacterCurrentParams(kCharacterMadame)[0] = 1;
		startCycOtis(kCharacterMadame, "501");
		break;
	case 17:
		if (checkCathDir(kCarRedSleeping, 44)) {
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
			MadameCall(&LogicManager::CONS_Madame_DoSeqOtis, "502B", 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterMadame, "502A");
			break;
		case 2:
			startCycOtis(kCharacterMadame, "606Qd");
			softBlockAtDoor(kCharacterMadame, 35);
			break;
		case 3:
			getCharacter(kCharacterMadame).characterPosition.location = 1;
			getCharacterCurrentParams(kCharacterMadame)[0] = 1;
			endGraphics(kCharacterMadame);
			CONS_Madame_ClosedComp1(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 102484312:
		startCycOtis(kCharacterMadame, "501");
		getCharacterCurrentParams(kCharacterMadame)[0] = 1;
		break;
	case 134289824:
		startCycOtis(kCharacterMadame, "502A");
		getCharacterCurrentParams(kCharacterMadame)[0] = 0;
		break;
	case 168986720:
		send(kCharacterMadame, kCharacterFrancois, 102752636, 0);
		playDialog(kCharacterMadame, "MME1036", -1, 0);
		softReleaseAtDoor(kCharacterMadame, 35);
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 3;
		MadameCall(&LogicManager::CONS_Madame_DoCorrOtis, "606Fd", 35, 0, 0);
		break;
	case 202221040:
		setDoor(35, kCharacterCath, 0, 255, 255);
		getCharacter(kCharacterMadame).characterPosition.location = 0;
		playDialog(kCharacterMadame, "MME1035A", -1, 0);

		if (whoOnScreen(kCharacterMadame) || nearChar(kCharacterMadame, kCharacterCath, 2000))
			_globals[kGlobalMetMadame] = 1;

		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
		MadameCall(&LogicManager::CONS_Madame_DoCorrOtis, "606Ed", 35, 0, 0);
		break;
	}

}

void LogicManager::CONS_Madame_ClosedComp1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_ClosedComp1);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 13;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_ClosedComp1(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!whoRunningDialog(kCharacterMadame) && getCharacterCurrentParams(kCharacterMadame)[5] != 0x7FFFFFFF && _gameTime) {
			if (getCharacterCurrentParams(kCharacterMadame)[0] < _gameTime) {
				getCharacterCurrentParams(kCharacterMadame)[5] = 0x7FFFFFFF;
				setDoor(35, kCharacterCath, 1, 0, 0);
				setDoor(51, kCharacterCath, 1, 0, 0);

				if (nearChar(kCharacterMadame, kCharacterCath, 2000))
					_globals[kGlobalOverheardMadameAndFrancoisTalkingAboutWhistle] = 1;

				getCharacterCurrentParams(kCharacterMadame)[4] = 1;
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1037", 0, 0, 0);
				break;
			}

			if (!nearChar(kCharacterMadame, kCharacterCath, 2000) || !getCharacterCurrentParams(kCharacterMadame)[5]) {
				getCharacterCurrentParams(kCharacterMadame)[5] = _gameTime;
				if (!_gameTime) {
					setDoor(35, kCharacterCath, 1, 0, 0);
					setDoor(51, kCharacterCath, 1, 0, 0);

					if (nearChar(kCharacterMadame, kCharacterCath, 2000))
						_globals[kGlobalOverheardMadameAndFrancoisTalkingAboutWhistle] = 1;

					getCharacterCurrentParams(kCharacterMadame)[4] = 1;
					getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
					MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1037", 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterMadame)[5] < _gameTime) {
				getCharacterCurrentParams(kCharacterMadame)[5] = 0x7FFFFFFF;
				setDoor(35, kCharacterCath, 1, 0, 0);
				setDoor(51, kCharacterCath, 1, 0, 0);

				if (nearChar(kCharacterMadame, kCharacterCath, 2000))
					_globals[kGlobalOverheardMadameAndFrancoisTalkingAboutWhistle] = 1;

				getCharacterCurrentParams(kCharacterMadame)[4] = 1;
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1037", 0, 0, 0);
				break;
			}
		}

		if (_globals[kGlobalFrancoisHasSeenCorpseThrown] && getCharacterCurrentParams(kCharacterMadame)[6] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime > 1093500) {
				getCharacterCurrentParams(kCharacterMadame)[6] = 0x7FFFFFFF;
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
				MadameCall(&LogicManager::CONS_Madame_FranStory, 0, 0, 0, 0);
				break;
			}

			if (!getCharacterCurrentParams(kCharacterMadame)[4] || !cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterMadame)[6]) {
				getCharacterCurrentParams(kCharacterMadame)[6] = _gameTime;
				if (!_gameTime) {
					getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
					MadameCall(&LogicManager::CONS_Madame_FranStory, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterMadame)[6] < _gameTime) {
				getCharacterCurrentParams(kCharacterMadame)[6] = 0x7FFFFFFF;
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
				MadameCall(&LogicManager::CONS_Madame_FranStory, 0, 0, 0, 0);
				break;
			}
		}

		if (_gameTime > 1094400 && !getCharacterCurrentParams(kCharacterMadame)[7]) {
			getCharacterCurrentParams(kCharacterMadame)[7] = 1;
			CONS_Madame_OpenComp(0, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterMadame)[3]) {
			if (getCharacterCurrentParams(kCharacterMadame)[8] || (getCharacterCurrentParams(kCharacterMadame)[8] = _realTime + 75,
																   _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterMadame)[8] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterMadame)[8] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterMadame)[3] = 0;
			getCharacterCurrentParams(kCharacterMadame)[2] = 1;
			setDoor(35, kCharacterMadame, 1, 0, 0);
			setDoor(51, kCharacterMadame, 1, 0, 0);
			getCharacterCurrentParams(kCharacterMadame)[8] = 0;
		} else {
			getCharacterCurrentParams(kCharacterMadame)[8] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterMadame)[3]) {
			setDoor(35, kCharacterMadame, 1, 0, 0);
			setDoor(51, kCharacterMadame, 1, 0, 0);

			if (cathHasItem(kItemPassengerList)) {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 7;

				if (rnd(2) != 0) {
					MadameCall(&LogicManager::CONS_Madame_DoDialog, getCathSorryDialog(), 0, 0, 0);
				} else {
					MadameCall(&LogicManager::CONS_Madame_DoDialog, "CAT1510", 0, 0, 0);
				}
			} else {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 8;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else {
			getCharacterCurrentParams(kCharacterMadame)[1]++;
			setDoor(35, kCharacterMadame, 1, 0, 0);
			setDoor(51, kCharacterMadame, 1, 0, 0);

			if (msg->action == 9) {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 3;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "LIB013", 0, 0, 0);
			} else {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 4;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "LIB012", 0, 0, 0);
			}
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterMadame)[0] = _gameTime + 900;
		getCharacter(kCharacterMadame).characterPosition.position = 5790;
		setDoor(35, kCharacterMadame, 1, 10, 9);
		setDoor(51, kCharacterMadame, 1, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterMadame)[2] || getCharacterCurrentParams(kCharacterMadame)[3]) {
			setDoor(35, kCharacterMadame, 1, 10, 9);
			setDoor(51, kCharacterMadame, 1, 10, 9);
			getCharacterCurrentParams(kCharacterMadame)[2] = 0;
			getCharacterCurrentParams(kCharacterMadame)[3] = 0;
			getCharacterCurrentParams(kCharacterMadame)[1] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8]) {
		case 1:
			setDoor(35, kCharacterMadame, 1, 10, 9);
			setDoor(51, kCharacterMadame, 1, 10, 9);

			if (_globals[kGlobalFrancoisHasSeenCorpseThrown] && getCharacterCurrentParams(kCharacterMadame)[6] != 0x7FFFFFFF && _gameTime) {
				if (_gameTime > 1093500) {
					getCharacterCurrentParams(kCharacterMadame)[6] = 0x7FFFFFFF;
					getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
					MadameCall(&LogicManager::CONS_Madame_FranStory, 0, 0, 0, 0);
					break;
				}

				if (!getCharacterCurrentParams(kCharacterMadame)[4] || !cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterMadame)[6]) {
					getCharacterCurrentParams(kCharacterMadame)[6] = _gameTime;
					if (!_gameTime) {
						getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
						MadameCall(&LogicManager::CONS_Madame_FranStory, 0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterMadame)[6] < _gameTime) {
					getCharacterCurrentParams(kCharacterMadame)[6] = 0x7FFFFFFF;
					getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
					MadameCall(&LogicManager::CONS_Madame_FranStory, 0, 0, 0, 0);
					break;
				}
			}

			if (_gameTime > 1094400 && !getCharacterCurrentParams(kCharacterMadame)[7]) {
				getCharacterCurrentParams(kCharacterMadame)[7] = 1;
				CONS_Madame_OpenComp(0, 0, 0, 0);
				break;
			}

			if (getCharacterCurrentParams(kCharacterMadame)[3]) {
				if (getCharacterCurrentParams(kCharacterMadame)[8] || (getCharacterCurrentParams(kCharacterMadame)[8] = _realTime + 75,
																	   _realTime != -75)) {
					if (getCharacterCurrentParams(kCharacterMadame)[8] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterMadame)[8] = 0x7FFFFFFF;
				}

				getCharacterCurrentParams(kCharacterMadame)[3] = 0;
				getCharacterCurrentParams(kCharacterMadame)[2] = 1;
				setDoor(35, kCharacterMadame, 1, 0, 0);
				setDoor(51, kCharacterMadame, 1, 0, 0);
				getCharacterCurrentParams(kCharacterMadame)[8] = 0;
			} else {
				getCharacterCurrentParams(kCharacterMadame)[8] = 0;
			}

			break;
		case 2:
			CONS_Madame_OpenComp(0, 0, 0, 0);
			break;
		case 3:
		case 4:
			if (getCharacterCurrentParams(kCharacterMadame)[1] <= 1) {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 6;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1038", 0, 0, 0);
			} else {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 5;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1038C", 0, 0, 0);
			}

			break;
		case 5:
		case 6:
			setDoor(35, kCharacterMadame, 1, 14, 0);
			setDoor(51, kCharacterMadame, 1, 14, 0);
			getCharacterCurrentParams(kCharacterMadame)[3] = 1;
			break;
		case 7:
		case 8:
			getCharacterCurrentParams(kCharacterMadame)[3] = 0;
			getCharacterCurrentParams(kCharacterMadame)[2] = 1;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_OpenComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_OpenComp);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 14;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_OpenComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
		MadameCall(&LogicManager::CONS_Madame_DoCorrOtis, "606Dd", 35, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8]) {
		case 1:
			setDoor(35, kCharacterCath, 2, 0, 0);
			setDoor(51, kCharacterCath, 1, 10, 9);
			startCycOtis(kCharacterMadame, "503");
			break;
		case 2:
			startCycOtis(kCharacterMadame, "503");
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 3;
			MadameCall(&LogicManager::CONS_Madame_DoDialog, "MRB1080", 0, 0, 0);
			break;
		case 3:
			setDoor(35, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 4;
			MadameCall(&LogicManager::CONS_Madame_DoCorrOtis, "606Cd", 35, 0, 0);
			break;
		case 4:
			endGraphics(kCharacterMadame);
			CONS_Madame_InComp(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 101107728:
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
		MadameCall(&LogicManager::CONS_Madame_LetsGo, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_InComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_InComp);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 15;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_InComp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1170000 && !getCharacterCurrentParams(kCharacterMadame)[3]) {
			getCharacterCurrentParams(kCharacterMadame)[3] = 1;
			getCharacter(kCharacterMadame).characterPosition.location = 0;
			setDoor(51, kCharacterCath, 1, 10, 9);
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
			MadameCall(&LogicManager::CONS_Madame_DoCorrOtis, "606Rd", 35, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterMadame)[1]) {
			if (getCharacterCurrentParams(kCharacterMadame)[4] || (getCharacterCurrentParams(kCharacterMadame)[4] = _realTime + 75,
																   _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterMadame)[4] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterMadame)[4] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterMadame)[1] = 0;
			getCharacterCurrentParams(kCharacterMadame)[0] = 1;
			setDoor(35, kCharacterMadame, 1, 0, 0);
			setDoor(51, kCharacterMadame, 1, 0, 0);
			getCharacterCurrentParams(kCharacterMadame)[4] = 0;
		} else {
			getCharacterCurrentParams(kCharacterMadame)[4] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterMadame)[1]) {
			setDoor(35, kCharacterMadame, 1, 0, 0);
			setDoor(51, kCharacterMadame, 1, 0, 0);
			if (cathHasItem(kItemPassengerList)) {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 10;

				if (rnd(2) != 0) {
					MadameCall(&LogicManager::CONS_Madame_DoDialog, getCathSorryDialog(), 0, 0, 0);
				} else {
					MadameCall(&LogicManager::CONS_Madame_DoDialog, "CAT1510", 0, 0, 0);
				}
			} else {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 11;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else {
			getCharacterCurrentParams(kCharacterMadame)[2]++;
			setDoor(35, kCharacterMadame, 1, 0, 0);
			setDoor(51, kCharacterMadame, 1, 0, 0);
			if (msg->action == 9) {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 6;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "LIB013", 0, 0, 0);
			} else {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 7;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "LIB012", 0, 0, 0);
			}
		}

		break;
	case 12:
		setDoor(35, kCharacterMadame, 1, 10, 9);
		setDoor(51, kCharacterMadame, 1, 10, 9);
		getCharacter(kCharacterMadame).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMadame).characterPosition.location = 1;
		getCharacter(kCharacterMadame).characterPosition.position = 5790;
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterMadame)[0] || getCharacterCurrentParams(kCharacterMadame)[1]) {
			setDoor(35, kCharacterMadame, 1, 10, 9);
			setDoor(51, kCharacterMadame, 1, 10, 9);
			getCharacterCurrentParams(kCharacterMadame)[2] = 0;
			getCharacterCurrentParams(kCharacterMadame)[0] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8]) {
		case 1:
			setDoor(35, kCharacterCath, 1, 10, 9);
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
			MadameCall(&LogicManager::CONS_Madame_DoWalk, 4, 2000, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 3;
			MadameCall(&LogicManager::CONS_Madame_ComplainCond2, "MME1101", 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 4;
			MadameCall(&LogicManager::CONS_Madame_DoWalk, 4, 5790, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 5;
			MadameCall(&LogicManager::CONS_Madame_DoEnterCorrOtis, "606Td", 35, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterMadame).characterPosition.location = 1;
			getCharacter(kCharacterMadame).characterPosition.position = 5790;
			endGraphics(kCharacterMadame);
			setDoor(35, kCharacterMadame, 1, 10, 9);
			setDoor(51, kCharacterMadame, 1, 10, 9);

			if (getCharacterCurrentParams(kCharacterMadame)[1]) {
				if (getCharacterCurrentParams(kCharacterMadame)[4] || (getCharacterCurrentParams(kCharacterMadame)[4] = _realTime + 75,
																	   _realTime != -75)) {
					if (getCharacterCurrentParams(kCharacterMadame)[4] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterMadame)[4] = 0x7FFFFFFF;
				}

				getCharacterCurrentParams(kCharacterMadame)[1] = 0;
				getCharacterCurrentParams(kCharacterMadame)[0] = 1;
				setDoor(35, kCharacterMadame, 1, 0, 0);
				setDoor(51, kCharacterMadame, 1, 0, 0);
				getCharacterCurrentParams(kCharacterMadame)[4] = 0;
			} else {
				getCharacterCurrentParams(kCharacterMadame)[4] = 0;
			}

			break;
		case 6:
		case 7:
			if (getCharacterCurrentParams(kCharacterMadame)[2] <= 1) {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 9;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1038", 0, 0, 0);
			} else {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 8;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1038C", 0, 0, 0);
			}

			break;
		case 8:
		case 9:
			setDoor(35, kCharacterMadame, 1, 14, 0);
			setDoor(51, kCharacterMadame, 1, 14, 0);
			getCharacterCurrentParams(kCharacterMadame)[1] = 1;
			break;
		case 10:
		case 11:
			getCharacterCurrentParams(kCharacterMadame)[1] = 0;
			getCharacterCurrentParams(kCharacterMadame)[0] = 1;
			break;
		case 12:
			setDoor(35, kCharacterMadame, 1, 10, 9);
			setDoor(51, kCharacterMadame, 1, 10, 9);
			break;
		default:
			break;
		}

		break;
	case 223068211:
		setDoor(35, kCharacterMadame, 1, 0, 0);
		setDoor(51, kCharacterMadame, 1, 0, 0);
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 12;
		MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1151B", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_Asleep);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 16;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMadame).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMadame).characterPosition.position = 5790;
		getCharacter(kCharacterMadame).characterPosition.location = 1;
		setDoor(35, kCharacterCath, 1, 10, 9);
		setDoor(51, kCharacterCath, 1, 10, 9);
		endGraphics(kCharacterMadame);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_StartPart2);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 17;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Madame_AtBreakfast(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMadame);
		getCharacter(kCharacterMadame).characterPosition.position = 4689;
		getCharacter(kCharacterMadame).characterPosition.location = 1;
		getCharacter(kCharacterMadame).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterMadame).inventoryItem = 0;
		getCharacter(kCharacterMadame).clothes = 0;
		setDoor(35, kCharacterCath, 0, 10, 9);
		setDoor(51, kCharacterCath, 0, 10, 9);
		setDoor(43, kCharacterCath, 0, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_AtBreakfast(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_AtBreakfast);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 18;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_AtBreakfast(HAND_PARAMS) {
	switch (msg->action) {
	case 18:
		if (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] == 1) {
			if (inComp(kCharacterFrancois, kCarRedSleeping, 5790)) {
				setDoor(35, kCharacterCath, 0, 0, 0);
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
				MadameCall(&LogicManager::CONS_Madame_DoEnterCorrOtis, "606Ad", 35, 0, 0);
			} else {
				startCycOtis(kCharacterMadame, "606Md");
				softBlockAtDoor(kCharacterMadame, 35);
			}
		} else if (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] == 2 ||
				   getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] == 3) {
			setDoor(35, kCharacterCath, 2, 0, 0);
			getCharacter(kCharacterMadame).characterPosition.location = 1;
			CONS_Madame_OpenComp2(0, 0, 0, 0);
		}

		break;
	case 100901266:
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
		MadameCall(&LogicManager::CONS_Madame_DoWalk, 4, 5790, 0, 0);
		break;
	case 100957716:
		softReleaseAtDoor(kCharacterMadame, 35);
		setDoor(35, kCharacterCath, 0, 0, 0);
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 3;
		MadameCall(&LogicManager::CONS_Madame_DoEnterCorrOtis, "606Ad", 35, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_OpenComp2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_OpenComp2);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 19;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_OpenComp2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (checkCathDir(kCarRedSleeping, 44) && !getCharacterCurrentParams(kCharacterMadame)[1]) {
			if (getCharacterCurrentParams(kCharacterMadame)[0]) {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
				MadameCall(&LogicManager::CONS_Madame_DoSeqOtis, "502B", 0, 0, 0);
			} else {
				getCharacterCurrentParams(kCharacterMadame)[0] = 1;
			}
		}

		break;
	case 12:
		setDoor(35, kCharacterCath, 2, 0, 0);
		setDoor(51, kCharacterCath, 0, 10, 9);
		getCharacterCurrentParams(kCharacterMadame)[1] = 1;
		startCycOtis(kCharacterMadame, "501");
		break;
	case 18:
		if (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] == 1 && checkCathDir(kCarRedSleeping, 44)) {
			bumpCath(kCarRedSleeping, 11, 255);
		}

		break;
	case 102484312:
		startCycOtis(kCharacterMadame, "501");
		getCharacterCurrentParams(kCharacterMadame)[1] = 1;
		break;
	case 134289824:
		startCycOtis(kCharacterMadame, "502A");
		getCharacterCurrentParams(kCharacterMadame)[1] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_StartPart3);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 20;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Madame_OpenComp3(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMadame);
		getCharacter(kCharacterMadame).characterPosition.position = 5790;
		getCharacter(kCharacterMadame).characterPosition.location = 1;
		getCharacter(kCharacterMadame).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMadame).inventoryItem = 0;
		getCharacter(kCharacterMadame).clothes = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_OpenComp3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_OpenComp3);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 21;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_OpenComp3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterParams(kCharacterMadame, 8)[0] && getCharacterCurrentParams(kCharacterMadame)[1] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime <= 2038500) {
				if (!cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterMadame)[0] ||
					dialogRunning("FRA2012") || dialogRunning("FRA2010") || dialogRunning("FRA2012") ||
					!getCharacterCurrentParams(kCharacterMadame)[1]) {

					getCharacterCurrentParams(kCharacterMadame)[1] = _gameTime;
					if (!_gameTime) {
						send(kCharacterMadame, kCharacterFrancois, 189872836, 0);
						setDoor(35, kCharacterCath, 1, 10, 9);
						getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
						MadameCall(&LogicManager::CONS_Madame_DoCorrOtis, "606Cd", 35, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterMadame)[1] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterMadame)[1] = 0x7FFFFFFF;
			send(kCharacterMadame, kCharacterFrancois, 189872836, 0);
			setDoor(35, kCharacterCath, 1, 10, 9);
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
			MadameCall(&LogicManager::CONS_Madame_DoCorrOtis, "606Cd", 35, 0, 0);
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterMadame)[0] = 1;
		setDoor(35, kCharacterCath, 2, 0, 0);
		setDoor(51, kCharacterCath, 1, 10, 9);
		setDoor(43, kCharacterCath, 0, 255, 255);
		startCycOtis(kCharacterMadame, "501");
		break;
	case 18:
		switch (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
			MadameCall(&LogicManager::CONS_Madame_DoCorrOtis, "606Rd", 35, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterMadame).characterPosition.location = 0;
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 3;
			MadameCall(&LogicManager::CONS_Madame_DoWalk, 4, 2000, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 4;
			MadameCall(&LogicManager::CONS_Madame_ComplainCond2, "MME3001", 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 5;
			MadameCall(&LogicManager::CONS_Madame_DoWalk, 4, 5790, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 6;
			MadameCall(&LogicManager::CONS_Madame_DoEnterCorrOtis, "606Td", 35, 0, 0);
			break;
		case 6:
			endGraphics(kCharacterMadame);
			setDoor(35, kCharacterCath, 1, 10, 9);
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 7;
			MadameCall(&LogicManager::CONS_Madame_DoWait, 150, 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 8;
			MadameCall(&LogicManager::CONS_Madame_DoCorrOtis, "606Dd", 35, 0, 0);
			break;
		case 8:
			setDoor(35, kCharacterCath, 2, 0, 0);
			startCycOtis(kCharacterMadame, "501");
			send(kCharacterMadame, kCharacterFrancois, 190390860, 0);
			break;
		case 9:
			startCycOtis(kCharacterMadame, "501");
			getCharacterCurrentParams(kCharacterMadame)[0] = 1;
			break;
		default:
			break;
		}

		break;
	case 101107728:
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 9;
		MadameCall(&LogicManager::CONS_Madame_LetsGo, 0, 0, 0, 0);
		break;
	case 102484312:
		startCycOtis(kCharacterMadame, "501");
		getCharacterCurrentParams(kCharacterMadame)[0] = 1;
		break;
	case 134289824:
		startCycOtis(kCharacterMadame, "502A");
		getCharacterCurrentParams(kCharacterMadame)[0] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_StartPart4);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 22;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Madame_OpenComp4(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMadame);
		getCharacter(kCharacterMadame).characterPosition.position = 5790;
		getCharacter(kCharacterMadame).characterPosition.location = 1;
		getCharacter(kCharacterMadame).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMadame).inventoryItem = 0;
		getCharacter(kCharacterMadame).clothes = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_OpenComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_OpenComp4);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 23;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_OpenComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMadame)[0]) {
			if (getCharacterCurrentParams(kCharacterMadame)[1] || (getCharacterCurrentParams(kCharacterMadame)[1] = _gameTime + 900,
																   _gameTime != -900)) {
				if (getCharacterCurrentParams(kCharacterMadame)[1] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterMadame)[1] = 0x7FFFFFFF;
			}

			setDoor(35, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
			MadameCall(&LogicManager::CONS_Madame_DoCorrOtis, "606Cd", 35, 0, 0);
		}

		break;
	case 12:
		setDoor(35, kCharacterCath, 2, 0, 0);
		setDoor(51, kCharacterCath, 1, 10, 9);
		startCycOtis(kCharacterMadame, "501");
		break;
	case 18:
		if (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] == 1) {
			endGraphics(kCharacterMadame);
			CONS_Madame_ClosedComp4(0, 0, 0, 0);
		} else if (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] == 2) {
			startCycOtis(kCharacterMadame, "501");
			getCharacterCurrentParams(kCharacterMadame)[0] = 1;
		}

		break;
	case 101107728:
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
		MadameCall(&LogicManager::CONS_Madame_LetsGo, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_ClosedComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_ClosedComp4);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 24;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_ClosedComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2470500 && !getCharacterCurrentParams(kCharacterMadame)[3]) {
			getCharacterCurrentParams(kCharacterMadame)[3] = 1;
			CONS_Madame_Asleep4(0, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterMadame)[1]) {
			if (getCharacterCurrentParams(kCharacterMadame)[4] || (getCharacterCurrentParams(kCharacterMadame)[4] = _realTime + 75,
																   _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterMadame)[4] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterMadame)[4] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterMadame)[1] = 0;
			getCharacterCurrentParams(kCharacterMadame)[0] = 1;
			setDoor(35, kCharacterMadame, 1, 0, 0);
			setDoor(51, kCharacterMadame, 1, 10, 9);
			getCharacterCurrentParams(kCharacterMadame)[4] = 0;
		} else {
			getCharacterCurrentParams(kCharacterMadame)[4] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterMadame)[1]) {
			setDoor(35, kCharacterMadame, 1, 0, 0);
			setDoor(51, kCharacterMadame, 1, 0, 0);
			if (cathHasItem(kItemPassengerList)) {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 5;

				if (rnd(2) != 0) {
					MadameCall(&LogicManager::CONS_Madame_DoDialog, getCathSorryDialog(), 0, 0, 0);
				} else {
					MadameCall(&LogicManager::CONS_Madame_DoDialog, "CAT1510", 0, 0, 0);
				}
			} else {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 6;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else {
			getCharacterCurrentParams(kCharacterMadame)[2]++;
			setDoor(35, kCharacterMadame, 1, 0, 0);
			setDoor(51, kCharacterMadame, 1, 0, 0);
			if (msg->action == 9) {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "LIB013", 0, 0, 0);
			} else {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "LIB012", 0, 0, 0);
			}
		}

		break;
	case 12:
		setDoor(35, kCharacterMadame, 1, 10, 9);
		setDoor(51, kCharacterMadame, 1, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterMadame)[0] || getCharacterCurrentParams(kCharacterMadame)[1]) {
			setDoor(35, kCharacterMadame, 1, 10, 9);
			setDoor(51, kCharacterMadame, 1, 10, 9);
			getCharacterCurrentParams(kCharacterMadame)[0] = 0;
			getCharacterCurrentParams(kCharacterMadame)[1] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8]) {
		case 1:
		case 2:
			if (getCharacterCurrentParams(kCharacterMadame)[2] <= 1) {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 4;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1038", 0, 0, 0);
			} else {
				getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 3;
				MadameCall(&LogicManager::CONS_Madame_DoDialog, "MME1038C", 0, 0, 0);
			}

			break;
		case 3:
		case 4:
			setDoor(35, kCharacterMadame, 1, 14, 0);
			setDoor(51, kCharacterMadame, 1, 14, 0);
			getCharacterCurrentParams(kCharacterMadame)[1] = 1;
			break;
		case 5:
		case 6:
			getCharacterCurrentParams(kCharacterMadame)[1] = 0;
			getCharacterCurrentParams(kCharacterMadame)[0] = 1;
			break;
		case 7:
			send(kCharacterMadame, kCharacterCond2, 123199584, 0);
			break;
		case 8:
			send(kCharacterMadame, kCharacterCond2, 88652208, 0);
			break;
		default:
			break;
		}

		break;
	case 122865568:
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 8;
		MadameCall(&LogicManager::CONS_Madame_DoDialog, "Mme1151A", 0, 0, 0);
		break;
	case 221683008:
		getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 7;
		MadameCall(&LogicManager::CONS_Madame_DoDialog, "Mme1038", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_Asleep4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_Asleep4);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 25;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_Asleep4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterMadame);
		getCharacter(kCharacterMadame).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMadame).characterPosition.position = 5790;
		getCharacter(kCharacterMadame).characterPosition.location = 1;
		setDoor(35, kCharacterCath, 1, 10, 9);
		setDoor(51, kCharacterCath, 1, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_StartPart5);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 26;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Madame_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMadame);
		getCharacter(kCharacterMadame).characterPosition.position = 3969;
		getCharacter(kCharacterMadame).characterPosition.location = 1;
		getCharacter(kCharacterMadame).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterMadame).inventoryItem = 0;
		getCharacter(kCharacterMadame).clothes = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_Prisoner);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 27;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Madame_Hiding(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_Hiding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_Hiding);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 28;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_Hiding(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMadame)[0]) {
			if (getCharacterCurrentParams(kCharacterMadame)[2] || (getCharacterCurrentParams(kCharacterMadame)[2] = _realTime + 75,
																   _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterMadame)[2] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterMadame)[2] = 0x7FFFFFFF;
			}
			getCharacterCurrentParams(kCharacterMadame)[0] = 0;
			getCharacterCurrentParams(kCharacterMadame)[1] = 1;
			setDoor(35, kCharacterMadame, 1, 0, 0);
			setDoor(51, kCharacterMadame, 1, 0, 0);
			getCharacterCurrentParams(kCharacterMadame)[2] = 0;
		} else {
			getCharacterCurrentParams(kCharacterMadame)[2] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterMadame)[0]) {
			setDoor(35, kCharacterMadame, 1, 0, 0);
			setDoor(51, kCharacterMadame, 1, 0, 0);
			getCharacterCurrentParams(kCharacterMadame)[0] = 0;
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 1;

			MadameCall(&LogicManager::CONS_Madame_DoDialog, getCathJustChecking(), 0, 0, 0);
		} else if (msg->action == 8) {
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 2;
			MadameCall(&LogicManager::CONS_Madame_DoDialog, "LIB012", 0, 0, 0);
		} else {
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 3;
			MadameCall(&LogicManager::CONS_Madame_DoDialog, "LIB013", 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterMadame).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMadame).characterPosition.position = 5790;
		getCharacter(kCharacterMadame).characterPosition.location = 1;
		endGraphics(kCharacterMadame);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterMadame)[1] || getCharacterCurrentParams(kCharacterMadame)[0]) {
			getCharacterCurrentParams(kCharacterMadame)[1] = 0;
			getCharacterCurrentParams(kCharacterMadame)[0] = 0;
			setDoor(35, kCharacterMadame, 1, 10, 9);
			setDoor(51, kCharacterMadame, 1, 10, 9);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8]) {
		case 1:
			setDoor(35, kCharacterMadame, 1, 10, 9);
			setDoor(51, kCharacterMadame, 1, 10, 9);
			break;
		case 2:
		case 3:
			setDoor(35, kCharacterMadame, 1, 0, 0);
			setDoor(51, kCharacterMadame, 1, 0, 0);
			getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall + 8] = 4;
			MadameCall(&LogicManager::CONS_Madame_DoDialog, "Mme5001", 0, 0, 0);
			break;
		case 4:
			getCharacterCurrentParams(kCharacterMadame)[0] = 1;
			setDoor(35, kCharacterMadame, 1, 14, 0);
			setDoor(51, kCharacterMadame, 1, 14, 0);
			break;
		default:
			break;
		}

		break;
	case 135800432:
		CONS_Madame_Disappear(0, 0, 0, 0);
		break;
	case 155604840:
		setDoor(35, kCharacterMadame, 1, 10, 9);
		setDoor(51, kCharacterMadame, 1, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Madame_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMadame).callParams[getCharacter(kCharacterMadame).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMadame, &LogicManager::HAND_Madame_Disappear);
	getCharacter(kCharacterMadame).callbacks[getCharacter(kCharacterMadame).currentCall] = 29;

	params->clear();

	fedEx(kCharacterMadame, kCharacterMadame, 12, 0);
}

void LogicManager::HAND_Madame_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsMadame[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Madame_DebugWalks,
	&LogicManager::HAND_Madame_DoDialog,
	&LogicManager::HAND_Madame_DoSeqOtis,
	&LogicManager::HAND_Madame_DoWait,
	&LogicManager::HAND_Madame_DoCorrOtis,
	&LogicManager::HAND_Madame_DoEnterCorrOtis,
	&LogicManager::HAND_Madame_DoWalk,
	&LogicManager::HAND_Madame_ComplainCond2,
	&LogicManager::HAND_Madame_LetsGo,
	&LogicManager::HAND_Madame_Birth,
	&LogicManager::HAND_Madame_FranStory,
	&LogicManager::HAND_Madame_OpenComp1,
	&LogicManager::HAND_Madame_ClosedComp1,
	&LogicManager::HAND_Madame_OpenComp,
	&LogicManager::HAND_Madame_InComp,
	&LogicManager::HAND_Madame_Asleep,
	&LogicManager::HAND_Madame_StartPart2,
	&LogicManager::HAND_Madame_AtBreakfast,
	&LogicManager::HAND_Madame_OpenComp2,
	&LogicManager::HAND_Madame_StartPart3,
	&LogicManager::HAND_Madame_OpenComp3,
	&LogicManager::HAND_Madame_StartPart4,
	&LogicManager::HAND_Madame_OpenComp4,
	&LogicManager::HAND_Madame_ClosedComp4,
	&LogicManager::HAND_Madame_Asleep4,
	&LogicManager::HAND_Madame_StartPart5,
	&LogicManager::HAND_Madame_Prisoner,
	&LogicManager::HAND_Madame_Hiding,
	&LogicManager::HAND_Madame_Disappear
};

} // End of namespace LastExpress
