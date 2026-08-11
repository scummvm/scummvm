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

void LogicManager::CONS_Police(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterPolice,
			_functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]
		);

		break;
	case 1:
		CONS_Police_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Police_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Police_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Police_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Police_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::PoliceCall(CALL_PARAMS) {
	getCharacter(kCharacterPolice).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Police_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_DebugWalks);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 1;

	params->clear();

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterPolice, kCarGreenSleeping, getCharacterCurrentParams(kCharacterPolice)[0])) {
			if (getCharacterCurrentParams(kCharacterPolice)[0] == 10000) {
				getCharacterCurrentParams(kCharacterPolice)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterPolice)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterPolice).characterPosition.position = 0;
		getCharacter(kCharacterPolice).characterPosition.location = 0;
		getCharacter(kCharacterPolice).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterPolice)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_Birth);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 2;

	params->clear();

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterPolice)[0]) {
			getCharacterCurrentParams(kCharacterPolice)[0] = 1;
			CONS_Police_ReadyToBoard(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterPolice).characterPosition.car = kCarNone;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_DoSeqOtis);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 17:
		if (!getCharacterParams(kCharacterPolice, 8)[0] && whoOnScreen(kCharacterPolice)) {
			playDialog(0, "MUS007", -1, 0);
			getCharacterParams(kCharacterPolice, 8)[0] = 1;
		}

		if (nearChar(kCharacterPolice, kCharacterCath, 1000) && !getCharacter(kCharacterCath).characterPosition.location) {
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 1;
			PoliceCall(&LogicManager::CONS_Police_SaveGame, 2, kEventGendarmesArrestation, 0, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterPolice).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
		fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterPolice, (char *)&getCharacterCurrentParams(kCharacterPolice)[0]);
		break;
	case 18:
		if (getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] == 1) {
			playNIS(kEventGendarmesArrestation);
			endGame(0, 1, 57, 1);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_DoDialog);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 17:
		if (!getCharacterParams(kCharacterPolice, 8)[0] && whoOnScreen(kCharacterPolice)) {
			playDialog(0, "MUS007", -1, 0);
			getCharacterParams(kCharacterPolice, 8)[0] = 1;
		}

		if (nearChar(kCharacterPolice, kCharacterCath, 1000) && !getCharacter(kCharacterCath).characterPosition.location) {
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 1;
			PoliceCall(&LogicManager::CONS_Police_SaveGame, 2, kEventGendarmesArrestation, 0, 0);
		}

		break;
	case 2:
		getCharacter(kCharacterPolice).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
		fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
		break;
	case 12:
		playDialog(kCharacterPolice, (char *)&getCharacterCurrentParams(kCharacterPolice)[0], -1, 0);
		break;
	case 18:
		if (getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] == 1) {
			playNIS(kEventGendarmesArrestation);
			endGame(0, 1, 57, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_DoDialogFullVol(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_DoDialogFullVol);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_DoDialogFullVol(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 17:
		if (!getCharacterParams(kCharacterPolice, 8)[0] && whoOnScreen(kCharacterPolice)) {
			playDialog(0, "MUS007", -1, 0);
			getCharacterParams(kCharacterPolice, 8)[0] = 1;
		}

		if (nearChar(kCharacterPolice, kCharacterCath, 1000) && !getCharacter(kCharacterCath).characterPosition.location) {
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 1;
			PoliceCall(&LogicManager::CONS_Police_SaveGame, 2, kEventGendarmesArrestation, 0, 0);
		}

		break;
	case 2:
		getCharacter(kCharacterPolice).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
		fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
		break;
	case 12:
		playDialog(kCharacterPolice, (char *)&getCharacterCurrentParams(kCharacterPolice)[0], 16, 0);
		break;
	case 18:
		if (getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] == 1) {
			playNIS(kEventGendarmesArrestation);
			endGame(0, 1, 57, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_DoWait);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 6;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterPolice)[0] < _gameTime && !getCharacterCurrentParams(kCharacterPolice)[1]) {
			getCharacterCurrentParams(kCharacterPolice)[1] = 1;

			getCharacter(kCharacterPolice).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
			fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
			break;
		}

		// fall through
	case 17:
		if (!getCharacterParams(kCharacterPolice, 8)[0] && whoOnScreen(kCharacterPolice)) {
			playDialog(0, "MUS007", -1, 0);
			getCharacterParams(kCharacterPolice, 8)[0] = 1;
		}

		if (nearChar(kCharacterPolice, kCharacterCath, 1000) && !getCharacter(kCharacterCath).characterPosition.location) {
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 1;
			PoliceCall(&LogicManager::CONS_Police_SaveGame, 2, kEventGendarmesArrestation, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] == 1) {
			playNIS(kEventGendarmesArrestation);
			endGame(0, 1, 57, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_SaveGame);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 7;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterPolice).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
		fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
		break;
	case 12:
		save(
			kCharacterPolice,
			getCharacterCurrentParams(kCharacterPolice)[0],
			getCharacterCurrentParams(kCharacterPolice)[1]
		);

		getCharacter(kCharacterPolice).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
		fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_DoWalk);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 8;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!walk(kCharacterPolice, getCharacterCurrentParams(kCharacterPolice)[0], getCharacterCurrentParams(kCharacterPolice)[1])) {
			if (!getCharacterParams(kCharacterPolice, 8)[0] && whoOnScreen(29)) {
				playDialog(0, "MUS007", -1, 0);
				getCharacterParams(kCharacterPolice, 8)[0] = 1;
			}

			if (nearChar(kCharacterPolice, kCharacterCath, 0x6D6) && !getCharacter(kCharacterCath).characterPosition.location && (!checkCathDir(kCarRedSleeping, 22) || nearChar(kCharacterPolice, kCharacterCath, 0xFA))) {
				getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 1;
				PoliceCall(&LogicManager::CONS_Police_SaveGame, 2, kEventGendarmesArrestation, 0, 0);
			}
		} else {
			getCharacter(kCharacterPolice).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
			fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
		}

		break;
	case 12:
		if (walk(kCharacterPolice, getCharacterCurrentParams(kCharacterPolice)[0], getCharacterCurrentParams(kCharacterPolice)[1])) {
			getCharacter(kCharacterPolice).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
			fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
		}
		break;
	case 17:
		if (!getCharacterParams(kCharacterPolice, 8)[0] && whoOnScreen(29)) {
			playDialog(0, "MUS007", -1, 0);
			getCharacterParams(kCharacterPolice, 8)[0] = 1;
		}

		if (nearChar(kCharacterPolice, kCharacterCath, 0x6D6) && !getCharacter(kCharacterCath).characterPosition.location && (!checkCathDir(kCarRedSleeping, 22) || nearChar(kCharacterPolice, kCharacterCath, 0xFA))) {
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 1;
			PoliceCall(&LogicManager::CONS_Police_SaveGame, 2, kEventGendarmesArrestation, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] == 1) {
			playNIS(kEventGendarmesArrestation);
			endGame(0, 1, 57, 1);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_DoComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_DoComp);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 9;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;
	strncpy((char *)&params->parameters[2], param3.stringParam, 12);
	strncpy((char *)&params->parameters[5], param4.stringParam, 12);

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_DoComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterCurrentParams(kCharacterPolice)[1] == 5790) {
			getCharacterCurrentParams(kCharacterPolice)[20] = 4;
			getCharacterCurrentParams(kCharacterPolice)[22] = 6130;
		} else if (getCharacterCurrentParams(kCharacterPolice)[1] >= 5790) {
			if (getCharacterCurrentParams(kCharacterPolice)[1] == 7500) {
				getCharacterCurrentParams(kCharacterPolice)[20] = 2;
				getCharacterCurrentParams(kCharacterPolice)[22] = 7850;
			} else if (getCharacterCurrentParams(kCharacterPolice)[1] >= 7500) {
				if (getCharacterCurrentParams(kCharacterPolice)[1] == 8200) {
					getCharacterCurrentParams(kCharacterPolice)[20] = 1;
					getCharacterCurrentParams(kCharacterPolice)[22] = 7850;
					getCharacterCurrentParams(kCharacterPolice)[21] = 1;
				}
			} else if (getCharacterCurrentParams(kCharacterPolice)[1] == 6470) {
				getCharacterCurrentParams(kCharacterPolice)[20] = 3;
				getCharacterCurrentParams(kCharacterPolice)[22] = 6130;
				getCharacterCurrentParams(kCharacterPolice)[21] = 1;
			}
		} else if (getCharacterCurrentParams(kCharacterPolice)[1] == 4070) {
			getCharacterCurrentParams(kCharacterPolice)[20] = 6;
			getCharacterCurrentParams(kCharacterPolice)[22] = 4455;
		} else if (getCharacterCurrentParams(kCharacterPolice)[1] >= 4070) {
			if (getCharacterCurrentParams(kCharacterPolice)[1] == 4840) {
				getCharacterCurrentParams(kCharacterPolice)[20] = 5;
				getCharacterCurrentParams(kCharacterPolice)[22] = 4455;
				getCharacterCurrentParams(kCharacterPolice)[21] = 1;
			}
		} else if (getCharacterCurrentParams(kCharacterPolice)[1] == 3050) {
			getCharacterCurrentParams(kCharacterPolice)[20] = 7;
			getCharacterCurrentParams(kCharacterPolice)[21] = 1;
		} else if (getCharacterCurrentParams(kCharacterPolice)[1] == 2740) {
			getCharacterCurrentParams(kCharacterPolice)[20] = 8;
		}

		if (getCharacterCurrentParams(kCharacterPolice)[0] == kCarRedSleeping)
			getCharacterCurrentParams(kCharacterPolice)[20] += 31;

		if (getCharacterCurrentParams(kCharacterPolice)[21]) {
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterPolice)[8], 12, "632A");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterPolice)[11], 12, "632B");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterPolice)[14], 12, "632C");
		} else {
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterPolice)[8], 12, "632D");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterPolice)[11], 12, "632E");
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterPolice)[14], 12, "632F");
		}

		Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterPolice)[8], 12, (char *)&getCharacterCurrentParams(kCharacterPolice)[2]);
		Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterPolice)[11], 12, (char *)&getCharacterCurrentParams(kCharacterPolice)[2]);
		Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterPolice)[14], 12, (char *)&getCharacterCurrentParams(kCharacterPolice)[2]);

		if ((inComp(kCharacterCath, getCharacterCurrentParams(kCharacterPolice)[0], getCharacterCurrentParams(kCharacterPolice)[1]) ||
			 inComp(kCharacterCath, getCharacterCurrentParams(kCharacterPolice)[0], getCharacterCurrentParams(kCharacterPolice)[22]) ||
			 (getCharacterCurrentParams(kCharacterPolice)[0] == kCarGreenSleeping && getCharacterCurrentParams(kCharacterPolice)[1] == 8200 && cathOutHisWindow())) &&
			!inComp(kCharacterCath, kCarRedSleeping, 7850)) {
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 1;
			PoliceCall(&LogicManager::CONS_Police_TrappedCath, getCharacterCurrentParams(kCharacterPolice)[0], getCharacterCurrentParams(kCharacterPolice)[1], getCharacterCurrentParams(kCharacterPolice)[20], 0);
		} else {
			startCycOtis(kCharacterPolice, (char *)&getCharacterCurrentParams(kCharacterPolice)[8]);
			softBlockAtDoor(kCharacterPolice, getCharacterCurrentParams(kCharacterPolice)[20]);
			if (getCharacterCurrentParams(kCharacterPolice)[21]) {
				getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 2;
				PoliceCall(&LogicManager::CONS_Police_DoDialog, "POL1044A", 0, 0, 0);
			} else {
				getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 3;
				PoliceCall(&LogicManager::CONS_Police_DoDialog, "POL1044B", 0, 0, 0);
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] < 4) {
			if (getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] != 1) {
				if (getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8]) {
					startCycOtis(kCharacterPolice, (char *)&getCharacterCurrentParams(kCharacterPolice)[11]);
					if (!emptyComp(getCharacterCurrentParams(kCharacterPolice)[0], getCharacterCurrentParams(kCharacterPolice)[1]) &&
						strcmp((char *)&getCharacterCurrentParams(kCharacterPolice)[5], "NODIALOG")) {
						Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterPolice)[17], 12, "POL1045");
						Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterPolice)[17], 12, (char *)&getCharacterCurrentParams(kCharacterPolice)[5]);
						getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 5;
						PoliceCall(&LogicManager::CONS_Police_DoDialog, (char *)&getCharacterCurrentParams(kCharacterPolice)[17], 0, 0, 0);
					} else {
						getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 4;
						PoliceCall(&LogicManager::CONS_Police_DoWait, 150, 0, 0, 0);
					}
				}

				break;
			}

			getCharacter(kCharacterPolice).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
			fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
			break;
		}

		if (getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] == 6) {
			getCharacter(kCharacterPolice).characterPosition.location = 0;
			softReleaseAtDoor(kCharacterPolice, getCharacterCurrentParams(kCharacterPolice)[20]);
			getCharacter(kCharacterPolice).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
			fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
			break;
		}

		if (getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] < 6) {
			if (!emptyComp(getCharacterCurrentParams(kCharacterPolice)[0], getCharacterCurrentParams(kCharacterPolice)[1]) &&
				strcmp((char *)&getCharacterCurrentParams(kCharacterPolice)[5], "NODIALOG")) {
				Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterPolice)[17], 12, "POL1043");
				Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterPolice)[17], 12, (char *)&getCharacterCurrentParams(kCharacterPolice)[5]);
				playDialog(kCharacterPolice, (char *)&getCharacterCurrentParams(kCharacterPolice)[17], -1, 30);
			}

			getCharacter(kCharacterPolice).characterPosition.location = 1;
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 6;
			PoliceCall(&LogicManager::CONS_Police_DoSeqOtis, (char *)&getCharacterCurrentParams(kCharacterPolice)[14], 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_TrappedCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_TrappedCath);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;
	params->parameters[2] = param3.intParam;

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_TrappedCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		if ((!getCharacterCurrentParams(kCharacterPolice)[4] &&
				(getCharacterCurrentParams(kCharacterPolice)[4] = _realTime + 75, _realTime == -75)) ||
			getCharacterCurrentParams(kCharacterPolice)[4] < _realTime) {
			if (!cathOutHisWindow() && checkDoor(getCharacterCurrentParams(kCharacterPolice)[2]) != 1) {
				getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 2;
				PoliceCall(&LogicManager::CONS_Police_SaveGame, 2, kEventGendarmesArrestation, 0, 0);
				break;
			}
		}

		bool skip = false; // Horrible way to unroll a goto...

		if (!getCharacterCurrentParams(kCharacterPolice)[5]) {
			getCharacterCurrentParams(kCharacterPolice)[5] = _realTime + 150;
			if (_realTime == -150) {
				skip = true;
				playDialog(kCharacterPolice, "POL1046A", 16, 0);
			}
		}

		if (!skip && getCharacterCurrentParams(kCharacterPolice)[5] < _realTime) {
			getCharacterCurrentParams(kCharacterPolice)[5] = 0x7FFFFFFF;
			playDialog(kCharacterPolice, "POL1046A", 16, 0);
		}

		if (getCharacterCurrentParams(kCharacterPolice)[6] || (getCharacterCurrentParams(kCharacterPolice)[6] = _realTime + 300, _realTime != -300)) {
			if (getCharacterCurrentParams(kCharacterPolice)[6] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterPolice)[6] = 0x7FFFFFFF;
		}

		if (!getCharacterCurrentParams(kCharacterPolice)[3] && cathOutHisWindow()) {
			setDoor(getCharacterCurrentParams(kCharacterPolice)[2], kCharacterCath, 0, 10, 9);

			getCharacter(kCharacterPolice).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
			fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
		} else {
			if (cathOutHisWindow())
				bumpCath(kCarGreenSleeping, 49, 255);
			playDialog(kCharacterPolice, "LIB017", 16, 0);
			if (_globals[kGlobalJacket] == 1) {
				getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 3;
				PoliceCall(&LogicManager::CONS_Police_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
			} else {
				getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 4;
				PoliceCall(&LogicManager::CONS_Police_SaveGame, 2, kEventGendarmesArrestation, 0, 0);
			}
		}

		break;
	}
	case 8:
		setDoor(getCharacterCurrentParams(kCharacterPolice)[2], kCharacterPolice, checkDoor(getCharacterCurrentParams(kCharacterPolice)[2]), 0, 0);
		getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 5;
		PoliceCall(&LogicManager::CONS_Police_DoDialogFullVol, "POL1046B", 0, 0, 0);
		break;
	case 9:
		getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 6;
		PoliceCall(&LogicManager::CONS_Police_SaveGame, 2, kEventGendarmesArrestation, 0, 0);
		break;
	case 12:
		setDoor(getCharacterCurrentParams(kCharacterPolice)[2], kCharacterPolice, checkDoor(getCharacterCurrentParams(kCharacterPolice)[2]), 0, 0);
		getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 1;
		PoliceCall(&LogicManager::CONS_Police_DoDialogFullVol, "POL1046", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8]) {
		case 1:
			setDoor(getCharacterCurrentParams(kCharacterPolice)[2], kCharacterPolice, checkDoor(getCharacterCurrentParams(kCharacterPolice)[2]), 14, 0);
			break;
		case 2:
		case 6:
			playDialog(kCharacterPolice, "LIB014", 16, 0);
			playNIS(kEventGendarmesArrestation);
			endGame(0, 1, 57, true);
			break;
		case 3:
			playNIS((kEventCoudertBloodJacket - (getCharacterCurrentParams(kCharacterPolice)[0] == 3)));
			endGame(0, 1, 55, true);
			setDoor(getCharacterCurrentParams(kCharacterPolice)[2], kCharacterCath, 0, 10, 9);

			getCharacter(kCharacterPolice).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
			fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
			break;
		case 4:
			playNIS(kEventGendarmesArrestation);
			endGame(0, 1, 57, true);
			setDoor(getCharacterCurrentParams(kCharacterPolice)[2], kCharacterCath, 0, 10, 9);

			getCharacter(kCharacterPolice).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterPolice, _functionsPolice[getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall]]);
			fedEx(kCharacterPolice, kCharacterPolice, 18, 0);
			break;
		case 5:
			setDoor(getCharacterCurrentParams(kCharacterPolice)[2], kCharacterPolice, checkDoor(getCharacterCurrentParams(kCharacterPolice)[2]), 0, 9);
			getCharacterCurrentParams(kCharacterPolice)[3] = 1;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_ReadyToBoard(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_ReadyToBoard);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 11;

	params->clear();

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_ReadyToBoard(HAND_PARAMS) {
	switch (msg->action) {
	case 169499649:
		send(kCharacterPolice, kCharacterCond1, 190082817, 0);
		CONS_Police_SearchTrain(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_SearchTrain(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_SearchTrain);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 12;

	params->clear();

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_SearchTrain(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterPolice).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterPolice).characterPosition.location = 0;
		getCharacter(kCharacterPolice).characterPosition.position = 540;
		_globals[kGlobalCharacterSearchingForCath] = kCharacterPolice;
		getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 1;
		PoliceCall(&LogicManager::CONS_Police_DoWalk, 3, 5540, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 2;
			PoliceCall(&LogicManager::CONS_Police_DoComp, 3, 5790, "d", "A");
			break;
		case 2:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 3;
			PoliceCall(&LogicManager::CONS_Police_DoWalk, 3, 6220, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 4;
			PoliceCall(&LogicManager::CONS_Police_DoComp, 3, 6470, "c", "B");
			break;
		case 4:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 5;
			PoliceCall(&LogicManager::CONS_Police_DoWalk, 3, 7250, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 6;
			PoliceCall(&LogicManager::CONS_Police_DoComp, 3, 7500, "b", "C");
			break;
		case 6:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 7;
			PoliceCall(&LogicManager::CONS_Police_DoWalk, 3, 7950, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 8;
			PoliceCall(&LogicManager::CONS_Police_DoComp, 3, 8200, "a", "NODIALOG");
			break;
		case 8:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 9;
			PoliceCall(&LogicManager::CONS_Police_DoWalk, 3, 9460, 0, 0);
			break;
		case 9:
			if (getCharacter(kCharacterCath).characterPosition.car == 3) {
				_globals[kGlobalCharacterSearchingForCath] = 0;
				endGraphics(kCharacterPolice);
				send(kCharacterPolice, kCharacterTrainM, 168710784, 0);
				CONS_Police_SearchDone(0, 0, 0, 0);
			} else {
				getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 10;
				PoliceCall(&LogicManager::CONS_Police_DoWalk, 4, 2490, 0, 0);
			}

			break;
		case 10:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 11;
			PoliceCall(&LogicManager::CONS_Police_DoComp, 4, 2740, "h", "NODIALOG");
			break;
		case 11:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 12;
			PoliceCall(&LogicManager::CONS_Police_DoWalk, 4, 3820, 0, 0);
			break;
		case 12:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 13;
			PoliceCall(&LogicManager::CONS_Police_DoComp, 4, 4070, "f", "E");
			break;
		case 13:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 14;
			PoliceCall(&LogicManager::CONS_Police_DoWalk, 4, 4590, 0, 0);
			break;
		case 14:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 15;
			PoliceCall(&LogicManager::CONS_Police_DoComp, 4, 4840, "e", "F");
			break;
		case 15:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 16;
			PoliceCall(&LogicManager::CONS_Police_DoWalk, 4, 5540, 0, 0);
			break;
		case 16:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 17;
			PoliceCall(&LogicManager::CONS_Police_DoComp, 4, 5790, "d", "G");
			break;
		case 17:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 18;
			PoliceCall(&LogicManager::CONS_Police_DoWalk, 4, 6220, 0, 0);
			break;
		case 18:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 19;
			PoliceCall(&LogicManager::CONS_Police_DoComp, 4, 6470, "c", "H");
			break;
		case 19:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 20;
			PoliceCall(&LogicManager::CONS_Police_DoWalk, 4, 7250, 0, 0);
			break;
		case 20:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 21;
			PoliceCall(&LogicManager::CONS_Police_DoComp, 4, 7500, "b", "J");
			break;
		case 21:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 22;
			PoliceCall(&LogicManager::CONS_Police_DoWalk, 4, 7950, 0, 0);
			break;
		case 22:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 23;
			PoliceCall(&LogicManager::CONS_Police_DoComp, 4, 8200, "a", "NODIALOG");
			break;
		case 23:
			getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall + 8] = 24;
			PoliceCall(&LogicManager::CONS_Police_DoWalk, 4, 9460, 0, 0);
			break;
		case 24:
			_globals[kGlobalCharacterSearchingForCath] = 0;
			endGraphics(kCharacterPolice);
			send(kCharacterPolice, kCharacterTrainM, 168710784, 0);
			CONS_Police_SearchDone(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_SearchDone(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_SearchDone);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 13;

	params->clear();

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_SearchDone(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterPolice).characterPosition.car = kCarNone;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_StartPart2);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 14;

	params->clear();

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterPolice);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_StartPart3);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 15;

	params->clear();

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterPolice);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_StartPart4);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 16;

	params->clear();

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterPolice);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Police_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterPolice).callParams[getCharacter(kCharacterPolice).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterPolice, &LogicManager::HAND_Police_StartPart5);
	getCharacter(kCharacterPolice).callbacks[getCharacter(kCharacterPolice).currentCall] = 17;

	params->clear();

	fedEx(kCharacterPolice, kCharacterPolice, 12, 0);
}

void LogicManager::HAND_Police_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterPolice);
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsPolice[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Police_DebugWalks,
	&LogicManager::HAND_Police_Birth,
	&LogicManager::HAND_Police_DoSeqOtis,
	&LogicManager::HAND_Police_DoDialog,
	&LogicManager::HAND_Police_DoDialogFullVol,
	&LogicManager::HAND_Police_DoWait,
	&LogicManager::HAND_Police_SaveGame,
	&LogicManager::HAND_Police_DoWalk,
	&LogicManager::HAND_Police_DoComp,
	&LogicManager::HAND_Police_TrappedCath,
	&LogicManager::HAND_Police_ReadyToBoard,
	&LogicManager::HAND_Police_SearchTrain,
	&LogicManager::HAND_Police_SearchDone,
	&LogicManager::HAND_Police_StartPart2,
	&LogicManager::HAND_Police_StartPart3,
	&LogicManager::HAND_Police_StartPart4,
	&LogicManager::HAND_Police_StartPart5
};

} // End of namespace LastExpress
