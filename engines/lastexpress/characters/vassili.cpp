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

void LogicManager::CONS_Vassili(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterVassili,
			_functionsVassili[getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall]]
		);

		break;
	case 1:
		CONS_Vassili_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Vassili_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Vassili_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Vassili_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Vassili_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::VassiliCall(CALL_PARAMS) {
	getCharacter(kCharacterVassili).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Vassili_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_DebugWalks);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 1;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterVassili, kCarGreenSleeping, getCharacterCurrentParams(kCharacterVassili)[0])) {
			if (getCharacterCurrentParams(kCharacterVassili)[0] == 10000) {
				getCharacterCurrentParams(kCharacterVassili)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterVassili)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterVassili).characterPosition.position = 0;
		getCharacter(kCharacterVassili).characterPosition.location = 0;
		getCharacter(kCharacterVassili).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterVassili)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_DoSeqOtis);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterVassili).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVassili, _functionsVassili[getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall]]);
		fedEx(kCharacterVassili, kCharacterVassili, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterVassili, (char *)&getCharacterCurrentParams(kCharacterVassili)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_SaveGame);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 3;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterVassili).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVassili, _functionsVassili[getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall]]);

		fedEx(kCharacterVassili, kCharacterVassili, 18, 0);
		break;
	case 12:
		save(
			kCharacterVassili,
			getCharacterCurrentParams(kCharacterVassili)[0],
			getCharacterCurrentParams(kCharacterVassili)[1]
		);

		getCharacter(kCharacterVassili).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVassili, _functionsVassili[getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall]]);

		fedEx(kCharacterVassili, kCharacterVassili, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_Birth);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 4;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterVassili)[0]) {
			getCharacterCurrentParams(kCharacterVassili)[0] = 1;
			CONS_Vassili_WithTatiana(0, 0, 0, 0);
		}

		break;
	case 12:
		setDoor(32, kCharacterCath, 0, 10, 9);
		setDoor(40, kCharacterCath, 0, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_WithTatiana(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_WithTatiana);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 5;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_WithTatiana(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterVassili)[0]) {
			getCharacter(kCharacterVassili).characterPosition = getCharacter(kCharacterTatiana).characterPosition;
		} else if ((!getCharacterCurrentParams(kCharacterVassili)[2] &&
			(getCharacterCurrentParams(kCharacterVassili)[2] = _gameTime + 450, _gameTime == -450)) || getCharacterCurrentParams(kCharacterVassili)[2] < _gameTime) {
			if (!getCharacterCurrentParams(kCharacterVassili)[1] && getModel(32) == 1) {
				getCharacterCurrentParams(kCharacterVassili)[1] = 1;
				startCycOtis(kCharacterVassili, "303A");
				setDoor(32, kCharacterCath, 0, 10, 9);
			}
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterVassili)[0] = 1;
		break;
	case 122732000:
		CONS_Vassili_InBed(0, 0, 0, 0);
		break;
	case 168459827:
		getCharacterCurrentParams(kCharacterVassili)[0] = 0;
		setDoor(32, kCharacterCath, 1, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_InBed(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_InBed);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 6;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_InBed(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inComp(kCharacterCath, kCarRedSleeping, 8200)) {
			if (getCharacterCurrentParams(kCharacterVassili)[2] ||
				((getCharacterCurrentParams(kCharacterVassili)[2] = _realTime + getCharacterCurrentParams(kCharacterVassili)[0]),
				 _realTime + getCharacterCurrentParams(kCharacterVassili)[0] != 0)) {

				if (getCharacterCurrentParams(kCharacterVassili)[2] < _realTime) {
					getCharacterCurrentParams(kCharacterVassili)[2] = 0x7FFFFFFF;
					getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] = 1;
					VassiliCall(&LogicManager::CONS_Vassili_DoSeqOtis, "303B", 0, 0, 0);
					break;
				}
			} else {
				getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] = 1;
				VassiliCall(&LogicManager::CONS_Vassili_DoSeqOtis, "303B", 0, 0, 0);
				break;
			}
		} else {
			getCharacterCurrentParams(kCharacterVassili)[2] = 0;
			if (getCharacterCurrentParams(kCharacterVassili)[1])
				startCycOtis(kCharacterVassili, "303A");
		}

		if (getCharacterCurrentParams(kCharacterVassili)[3] != 0x7FFFFFFF && _gameTime > 1489500) {
			if (_gameTime <= 1503000) {
				if (inComp(kCharacterCath, kCarRedSleeping, 8200) || !getCharacterCurrentParams(kCharacterVassili)[3]) {
					getCharacterCurrentParams(kCharacterVassili)[3] = _gameTime;
					if (!_gameTime) {
						CONS_Vassili_InBed2(0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterVassili)[3] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterVassili)[3] = 0x7FFFFFFF;
			CONS_Vassili_InBed2(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterVassili).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterVassili).characterPosition.location = 1;
		getCharacter(kCharacterVassili).characterPosition.position = 8200;
		setDoor(32, kCharacterCath, 0, 10, 9);
		getCharacterCurrentParams(kCharacterVassili)[0] = 5 * (3 * rnd(25) + 15);
		startCycOtis(kCharacterVassili, "303A");
		break;
	case 18:
		if (getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] == 1) {
			startCycOtis(kCharacterVassili, "303C");
			getCharacterCurrentParams(kCharacterVassili)[1] = 1;
			getCharacterCurrentParams(kCharacterVassili)[0] = 5 * (3 * rnd(25) + 15);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_InBed2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_InBed2);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 7;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_InBed2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterVassili)[0] == 0x7FFFFFFF || _gameTime <= 1503000)
			break;

		if (_gameTime <= 1512000) {
			if (!cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterVassili)[0]) {
				getCharacterCurrentParams(kCharacterVassili)[0] = _gameTime + 150;
				if (_gameTime == -150) {
					CONS_Vassili_HaveSeizureNow(0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterVassili)[0] >= _gameTime)
				break;
		}

		getCharacterCurrentParams(kCharacterVassili)[0] = 0x7FFFFFFF;
		CONS_Vassili_HaveSeizureNow(0, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterVassili).characterPosition.car = 4;
		getCharacter(kCharacterVassili).characterPosition.location = 1;
		getCharacter(kCharacterVassili).characterPosition.position = 8200;
		endGraphics(kCharacterVassili);

		if (inComp(kCharacterCath, kCarRedSleeping, 8200))
			bumpCathRDoor(32);

		setDoor(32, kCharacterCath, 1, 10, 9);
		break;
	case 339669520:
		CONS_Vassili_HavingSeizure(0, 0, 0, 0);
		break;
	}
}

void LogicManager::CONS_Vassili_HaveSeizureNow(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_HaveSeizureNow);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 8;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_HaveSeizureNow(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		CONS_Vassili_HavingSeizure(0, 0, 0, 0);
		break;
	case 12:
		if (!checkLoc(kCharacterCath, kCarRedSleeping)) {
			playDialog(0, "BUMP", -1, 0);

			if (getCharacter(kCharacterCath).characterPosition.car <= 4) {
				bumpCath(kCarRedSleeping, 1, 255);
			} else {
				bumpCath(kCarRedSleeping, 40, 255);
			}
		}

		send(kCharacterVassili, kCharacterAnna, 226031488, 0);
		send(kCharacterVassili, kCharacterTrainM, 226031488, 0);
		send(kCharacterVassili, kCharacterCond2, 226031488, 0);
		playDialog(kCharacterVassili, "VAS1027", 16, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_HavingSeizure(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_HavingSeizure);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 9;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_HavingSeizure(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		if (!nearChar(kCharacterVassili, kCharacterCath, 2500))
			playDialog(0, "BUMP", -1, 0);

		CONS_Vassili_CathArrives(0, 0, 0, 0);
		break;
	case 12:
		if ((checkDoor(32) != 2 ||
			!checkCathDir(kCarRedSleeping, 17)) &&
			!checkCathDir(kCarRedSleeping, 18) &&
			!checkCathDir(kCarRedSleeping, 37) &&
			!checkCathDir(kCarRedSleeping, 38) &&
			!checkCathDir(kCarRedSleeping, 41)) {

			playDialog(kCharacterVassili, "VAS1028", 16, 0);
		} else {
			CONS_Vassili_CathArrives(0, 0, 0, 0);
		}

		break;
	case 17:
		if ((checkDoor(32) == 2 &&
			checkCathDir(kCarRedSleeping, 17)) ||
			checkCathDir(kCarRedSleeping, 18) ||
			checkCathDir(kCarRedSleeping, 37) ||
			checkCathDir(kCarRedSleeping, 38) ||
			checkCathDir(kCarRedSleeping, 41)) {

			fadeDialog(kCharacterVassili);
			CONS_Vassili_CathArrives(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_CathArrives(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_CathArrives);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 10;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_CathArrives(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (_globals[kGlobalCorpseMovedFromFloor]) {
			if (_globals[kGlobalCorpseHasBeenThrown]) {
				if (_globals[kGlobalJacket] == 1) {
					playNIS(kEventMertensBloodJacket);
					endGame(0, 0, 0, true);
				} else {
					forceJump(kCharacterAnna, &LogicManager::CONS_Anna_AtSeizure);
					forceJump(kCharacterCond2, &LogicManager::CONS_Cond2_AtSeizure);
					getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] = 1;
					VassiliCall(&LogicManager::CONS_Vassili_SaveGame, 2, kEventVassiliSeizure, 0, 0);
				}
			} else {
				playNIS(kEventMertensCorpseBed);
				endGame(0, 0, 0, true);
			}
		} else {
			playNIS(kEventMertensCorpseFloor);
			endGame(0, 0, 0, true);
		}

		break;
	case 18:
		if (getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] == 1) {
			getCharacter(kCharacterCath).characterPosition.location = 1;
			playNIS(kEventVassiliSeizure);
			setDoor(32, kCharacterCath, 0, 10, 9);
			setDoor(1, kCharacterCath, 0, 10, 9);
			_globals[kGlobalPhaseOfTheNight] = 2;
			send(kCharacterVassili, kCharacterAnna, 191477936, 0);
			send(kCharacterVassili, kCharacterTrainM, 191477936, 0);
			send(kCharacterVassili, kCharacterCond2, 191477936, 0);
			bumpCathRDoor(32);
			CONS_Vassili_AsleepAgain(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_AsleepAgain(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_AsleepAgain);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 11;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_AsleepAgain(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		startCycOtis(kCharacterVassili, "303A");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_StartPart2);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 12;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Vassili_InPart2(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterVassili);
		getCharacter(kCharacterVassili).characterPosition.position = 8200;
		getCharacter(kCharacterVassili).characterPosition.location = 1;
		getCharacter(kCharacterVassili).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterVassili).inventoryItem = kItemNone;
		getCharacter(kCharacterVassili).clothes = 0;
		setDoor(32, kCharacterCath, 0, 10, 9);
		setModel(32, 1);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_InPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_InPart2);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 13;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_InPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inComp(kCharacterCath, kCarRedSleeping, 8200)) {
			if (getCharacterCurrentParams(kCharacterVassili)[2] ||
				(getCharacterCurrentParams(kCharacterVassili)[2] = _realTime + getCharacterCurrentParams(kCharacterVassili)[0],
				 _realTime + getCharacterCurrentParams(kCharacterVassili)[0] != 0)) {
				if (getCharacterCurrentParams(kCharacterVassili)[2] >= _realTime)
					return;
				getCharacterCurrentParams(kCharacterVassili)[2] = 0x7FFFFFFF;
			}
			getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] = 1;
			VassiliCall(&LogicManager::CONS_Vassili_DoSeqOtis, "303B", 0, 0, 0);
		} else {
			getCharacterCurrentParams(kCharacterVassili)[2] = 0;
			if (getCharacterCurrentParams(kCharacterVassili)[1])
				startCycOtis(kCharacterVassili, "303A");
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterVassili)[0] = 5 * (3 * rnd(25) + 15);
		startCycOtis(kCharacterVassili, "303A");
		break;
	case 18:
		if (getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] == 1) {
			startCycOtis(kCharacterVassili, "303C");
			getCharacterCurrentParams(kCharacterVassili)[1] = 1;
			getCharacterCurrentParams(kCharacterVassili)[0] = 5 * (3 * rnd(25) + 15);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_StartPart3);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 14;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Vassili_Asleep(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterVassili);
		getCharacter(kCharacterVassili).characterPosition.position = 8200;
		getCharacter(kCharacterVassili).characterPosition.location = 1;
		getCharacter(kCharacterVassili).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterVassili).inventoryItem = kItemNone;
		getCharacter(kCharacterVassili).clothes = 0;
		setDoor(32, kCharacterCath, 0, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_Asleep);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 15;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inComp(kCharacterCath, kCarRedSleeping, 8200)) {
			if (getCharacterCurrentParams(kCharacterVassili)[2] ||
				(getCharacterCurrentParams(kCharacterVassili)[2] = _realTime + getCharacterCurrentParams(kCharacterVassili)[0],
				 _realTime + getCharacterCurrentParams(kCharacterVassili)[0] != 0)) {

				if (getCharacterCurrentParams(kCharacterVassili)[2] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterVassili)[2] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] = 1;
			VassiliCall(&LogicManager::CONS_Vassili_DoSeqOtis, "303B", 0, 0, 0);
		} else {
			getCharacterCurrentParams(kCharacterVassili)[2] = 0;
			if (getCharacterCurrentParams(kCharacterVassili)[1])
				startCycOtis(kCharacterVassili, "303A");
		}

		break;
	case 9:
		getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] = 2;
		VassiliCall(&LogicManager::CONS_Vassili_SaveGame, 2, kEventVassiliCompartmentStealEgg, 0, 0);
		break;
	case 12:
		getCharacterCurrentParams(kCharacterVassili)[0] = 5 * (3 * rnd(25) + 15);
		startCycOtis(kCharacterVassili, "303A");
		break;
	case 17:
		if (inComp(kCharacterCath, kCarRedSleeping, 7850) && cathHasItem(kItemFirebird) && !_doneNIS[kEventVassiliCompartmentStealEgg]) {
			setDoor(48, kCharacterVassili, 0, 0, 9);
		} else {
			setDoor(48, kCharacterCath, 0, 0, 9);
		}

		break;
	case 18:
		if (getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] == 1) {
			startCycOtis(kCharacterVassili, "303C");
			getCharacterCurrentParams(kCharacterVassili)[1] = 1;
			getCharacterCurrentParams(kCharacterVassili)[0] = 5 * (3 * rnd(25) + 15);
		} else if (getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] == 2) {
			playNIS(kEventVassiliCompartmentStealEgg);
			bumpCath(kCarRedSleeping, 67, 255);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_StartPart4);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 16;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Vassili_InPart4(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterVassili);
		getCharacter(kCharacterVassili).characterPosition.position = 8200;
		getCharacter(kCharacterVassili).characterPosition.location = 1;
		getCharacter(kCharacterVassili).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterVassili).inventoryItem = kItemNone;
		getCharacter(kCharacterVassili).clothes = 0;
		setDoor(32, kCharacterCath, 0, 10, 9);
		setModel(32, 1);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_InPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_InPart4);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 17;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_InPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inComp(kCharacterCath, kCarRedSleeping, 8200)) {
			if (getCharacterCurrentParams(kCharacterVassili)[2] ||
				(getCharacterCurrentParams(kCharacterVassili)[2] = _realTime + getCharacterCurrentParams(kCharacterVassili)[0],
				 _realTime + getCharacterCurrentParams(kCharacterVassili)[0] != 0)) {
				if (getCharacterCurrentParams(kCharacterVassili)[2] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterVassili)[2] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] = 1;
			VassiliCall(&LogicManager::CONS_Vassili_DoSeqOtis, "303B", 0, 0, 0);
		} else {
			getCharacterCurrentParams(kCharacterVassili)[2] = 0;
			if (getCharacterCurrentParams(kCharacterVassili)[1])
				startCycOtis(kCharacterVassili, "303A");
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterVassili)[0] = 5 * (3 * rnd(25) + 15);
		startCycOtis(kCharacterVassili, "303A");
		break;
	case 18:
		if (getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall + 8] == 1) {
			startCycOtis(kCharacterVassili, "303C");
			getCharacterCurrentParams(kCharacterVassili)[1] = 1;
			getCharacterCurrentParams(kCharacterVassili)[0] = 5 * (3 * rnd(25) + 15);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vassili_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVassili).callParams[getCharacter(kCharacterVassili).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVassili, &LogicManager::HAND_Vassili_StartPart5);
	getCharacter(kCharacterVassili).callbacks[getCharacter(kCharacterVassili).currentCall] = 18;

	params->clear();

	fedEx(kCharacterVassili, kCharacterVassili, 12, 0);
}

void LogicManager::HAND_Vassili_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterVassili);
		getCharacter(kCharacterVassili).characterPosition.location = 1;
		getCharacter(kCharacterVassili).characterPosition.position = 3969;
		getCharacter(kCharacterVassili).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterVassili).inventoryItem = kItemNone;
		getCharacter(kCharacterVassili).clothes = 0;
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsVassili[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Vassili_DebugWalks,
	&LogicManager::HAND_Vassili_DoSeqOtis,
	&LogicManager::HAND_Vassili_SaveGame,
	&LogicManager::HAND_Vassili_Birth,
	&LogicManager::HAND_Vassili_WithTatiana,
	&LogicManager::HAND_Vassili_InBed,
	&LogicManager::HAND_Vassili_InBed2,
	&LogicManager::HAND_Vassili_HaveSeizureNow,
	&LogicManager::HAND_Vassili_HavingSeizure,
	&LogicManager::HAND_Vassili_CathArrives,
	&LogicManager::HAND_Vassili_AsleepAgain,
	&LogicManager::HAND_Vassili_StartPart2,
	&LogicManager::HAND_Vassili_InPart2,
	&LogicManager::HAND_Vassili_StartPart3,
	&LogicManager::HAND_Vassili_Asleep,
	&LogicManager::HAND_Vassili_StartPart4,
	&LogicManager::HAND_Vassili_InPart4,
	&LogicManager::HAND_Vassili_StartPart5
};

} // End of namespace LastExpress
