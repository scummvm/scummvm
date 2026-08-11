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

void LogicManager::CONS_Kronos(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterKronos,
			_functionsKronos[getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall]]
		);

		break;
	case 1:
		CONS_Kronos_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Kronos_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Kronos_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Kronos_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Kronos_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::KronosCall(CALL_PARAMS) {
	getCharacter(kCharacterKronos).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Kronos_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_DebugWalks);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 1;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterKronos, kCarGreenSleeping, getCharacterCurrentParams(kCharacterKronos)[0])) {
			if (getCharacterCurrentParams(kCharacterKronos)[0] == 10000) {
				getCharacterCurrentParams(kCharacterKronos)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterKronos)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterKronos).characterPosition.position = 0;
		getCharacter(kCharacterKronos).characterPosition.location = 0;
		getCharacter(kCharacterKronos).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterKronos)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_SaveGame);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 2;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterKronos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKronos, _functionsKronos[getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall]]);
		fedEx(kCharacterKronos, kCharacterKronos, 18, 0);
		break;
	case 12:
		save(
			kCharacterKronos,
			getCharacterCurrentParams(kCharacterKronos)[0],
			getCharacterCurrentParams(kCharacterKronos)[1]
		);

		getCharacter(kCharacterKronos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKronos, _functionsKronos[getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall]]);
		fedEx(kCharacterKronos, kCharacterKronos, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_DoWalk);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 3;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterKronos, getCharacterCurrentParams(kCharacterKronos)[0], getCharacterCurrentParams(kCharacterKronos)[1])) {
			getCharacter(kCharacterKronos).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterKronos, _functionsKronos[getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall]]);
			fedEx(kCharacterKronos, kCharacterKronos, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_DoDialog);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 4;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterKronos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKronos, _functionsKronos[getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall]]);
		fedEx(kCharacterKronos, kCharacterKronos, 18, 0);
		break;
	case 12:
		playDialog(kCharacterKronos, (char *)&getCharacterCurrentParams(kCharacterKronos)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_DoWait);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 5;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterKronos)[1] || (getCharacterCurrentParams(kCharacterKronos)[1] = _gameTime + getCharacterCurrentParams(kCharacterKronos)[0],
															   _gameTime + getCharacterCurrentParams(kCharacterKronos)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterKronos)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterKronos)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterKronos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKronos, _functionsKronos[getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall]]);
		fedEx(kCharacterKronos, kCharacterKronos, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_DoWaitReal(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_DoWaitReal);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 6;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_DoWaitReal(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterKronos)[1] || (getCharacterCurrentParams(kCharacterKronos)[1] = _realTime + getCharacterCurrentParams(kCharacterKronos)[0],
															   _realTime + getCharacterCurrentParams(kCharacterKronos)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterKronos)[1] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterKronos)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterKronos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKronos, _functionsKronos[getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall]]);
		fedEx(kCharacterKronos, kCharacterKronos, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_Birth);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 7;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterKronos)[0]) {
			getCharacterCurrentParams(kCharacterKronos)[0] = 1;
			CONS_Kronos_AwaitingCath(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterKronos).characterPosition.car = 2;
		getCharacter(kCharacterKronos).characterPosition.position = 6000;
		getCharacter(kCharacterKronos).characterPosition.location = 1;
		setDoor(73, kCharacterCath, 1, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_AwaitingCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_AwaitingCath);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 8;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_AwaitingCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1489500 && !getCharacterCurrentParams(kCharacterKronos)[1]) {
			getCharacterCurrentParams(kCharacterKronos)[1] = 1;
			CONS_Kronos_PlayingAtNight(0, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterKronos)[0] && checkLoc(kCharacterCath, kCarKronos)) {
			if (getCharacterCurrentParams(kCharacterKronos)[2] || (getCharacterCurrentParams(kCharacterKronos)[2] = _realTime + 150,
																   _realTime != -150)) {
				if (getCharacterCurrentParams(kCharacterKronos)[2] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterKronos)[2] = 0x7FFFFFFF;
			}

			CONS_Kronos_GreetCath(0, 0, 0, 0);
		}

		break;
	case 171849314:
		getCharacterCurrentParams(kCharacterKronos)[0] = 1;
		return;
	case 202621266:
		CONS_Kronos_GreetCath(0, 0, 0, 0);
		break;
	}
}

void LogicManager::CONS_Kronos_GreetCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_GreetCath);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 9;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_GreetCath(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 1;
		KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKronosConversation, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] == 1) {
			playNIS(kEventKronosConversation);
			bumpCath(kCarKronos, 87, 255);
			send(kCharacterKronos, kCharacterKahina, 137685712, 0);
			CONS_Kronos_CathDone(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_CathDone(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_CathDone);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 10;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_CathDone(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1489500 && !getCharacterCurrentParams(kCharacterKronos)[0]) {
			getCharacterCurrentParams(kCharacterKronos)[0] = 1;
			CONS_Kronos_PlayingAtNight(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterKronos).characterPosition.car = kCarKronos;
		getCharacter(kCharacterKronos).characterPosition.position = 6000;
		getCharacter(kCharacterKronos).characterPosition.location = 0;
		endGraphics(kCharacterKronos);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_PlayingAtNight(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_PlayingAtNight);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 11;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_PlayingAtNight(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacterCurrentParams(kCharacterKronos)[0]++;
		if ((getCharacterCurrentParams(kCharacterKronos)[0] & 1) == 0) {
			playDialog(kCharacterKronos, "KRO1002", -1, 0);
		} else {
			playDialog(kCharacterKronos, "KRO1001", -1, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterKronos).characterPosition.position = 7000;
		if (!whoRunningDialog(kCharacterKronos))
			playDialog(kCharacterKronos, "KRO1001", -1, 0);

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_StartPart2);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 12;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterKronos);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_StartPart3);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 13;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Kronos_InSeclusion(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterKronos);
		getCharacter(kCharacterKronos).characterPosition.position = 6000;
		getCharacter(kCharacterKronos).characterPosition.location = 0;
		getCharacter(kCharacterKronos).characterPosition.car = kCarKronos;
		getCharacter(kCharacterKronos).inventoryItem = 0;
		getCharacter(kCharacterKronos).clothes = 0;
		setDoor(73, kCharacterCath, 0, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_InSeclusion(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_InSeclusion);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 14;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_InSeclusion(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1993500 && getCharacterCurrentParams(kCharacterKronos)[0] && getCharacterCurrentParams(kCharacterKronos)[1] && getCharacterCurrentParams(kCharacterKronos)[2]) {
			CONS_Kronos_WBWait(0, 0, 0, 0);
		}

		break;
	case 157159392:
		switch (msg->sender) {
		case kCharacterAnna:
			getCharacterCurrentParams(kCharacterKronos)[0] = 1;
			break;
		case kCharacterTatiana:
			getCharacterCurrentParams(kCharacterKronos)[1] = 1;
			break;
		case kCharacterAbbot:
			getCharacterCurrentParams(kCharacterKronos)[2] = 1;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_WBWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_WBWait);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 15;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_WBWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterKronos)[0] && !inSalon(kCharacterMonsieur)) {
			if (!getCharacterCurrentParams(kCharacterKronos)[1]) {
				getCharacterCurrentParams(kCharacterKronos)[1] = _realTime + 75;
				if (_realTime == -75) {
					CONS_Kronos_VisitSalon(0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterKronos)[1] < _realTime) {
				getCharacterCurrentParams(kCharacterKronos)[1] = 0x7FFFFFFF;
				CONS_Kronos_VisitSalon(0, 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterKronos)[2] != 0x7FFFFFFF && _gameTime > 2002500) {
			if (_gameTime <= 2052000) {
				if (!inSalon(kCharacterCath) || inSalon(kCharacterMonsieur) || !getCharacterCurrentParams(kCharacterKronos)[2]) {
					getCharacterCurrentParams(kCharacterKronos)[2] = _gameTime + 900;
					if (_gameTime == -900) {
						if (!inSalon(kCharacterCath)) {
							send(kCharacterKronos, kCharacterAnna, 101169422, 0);
							send(kCharacterKronos, kCharacterTatiana, 101169422, 0);
							send(kCharacterKronos, kCharacterAbbot, 101169422, 0);
							CONS_Kronos_PreConcert(0, 0, 0, 0);
							break;
						}

						CONS_Kronos_VisitSalon(0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterKronos)[2] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterKronos)[2] = 0x7FFFFFFF;
			if (!inSalon(kCharacterCath)) {
				send(kCharacterKronos, kCharacterAnna, 101169422, 0);
				send(kCharacterKronos, kCharacterTatiana, 101169422, 0);
				send(kCharacterKronos, kCharacterAbbot, 101169422, 0);
				CONS_Kronos_PreConcert(0, 0, 0, 0);
				break;
			}

			CONS_Kronos_VisitSalon(0, 0, 0, 0);
		}

		break;
	case 12:
		if (checkCathDir(kCarRestaurant, 60) || checkCathDir(kCarRestaurant, 59) || checkCathDir(kCarRestaurant, 83) || checkCathDir(kCarRestaurant, 81) || checkCathDir(kCarRestaurant, 87)) {
			getCharacterCurrentParams(kCharacterKronos)[0] = 1;
		}

		break;
	case 17:
		if (!getCharacterCurrentParams(kCharacterKronos)[0] || !checkCathDir(kCarRestaurant, 51) || inSalon(kCharacterMonsieur)) {
			getCharacterCurrentParams(kCharacterKronos)[0] = checkCathDir(kCarRestaurant, 60) || checkCathDir(kCarRestaurant, 59) || checkCathDir(kCarRestaurant, 83) || checkCathDir(kCarRestaurant, 81) || checkCathDir(kCarRestaurant, 87);
			break;
		}

		CONS_Kronos_VisitSalon(0, 0, 0, 0);
		break;
	default:
		break;
	}
	
}

void LogicManager::CONS_Kronos_VisitSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_VisitSalon);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 16;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_VisitSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 1;
		KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKronosVisit, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] == 1) {
			playNIS(kEventKronosVisit);
			send(kCharacterKronos, kCharacterAnna, 101169422, 0);
			send(kCharacterKronos, kCharacterTatiana, 101169422, 0);
			send(kCharacterKronos, kCharacterAbbot, 101169422, 0);
			bumpCath(kCarRestaurant, 60, 255);
			CONS_Kronos_ReturnComp(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_ReturnComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_ReturnComp);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 17;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_ReturnComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterKronos).characterPosition.position = 7500;
		getCharacter(kCharacterKronos).characterPosition.location = 0;
		getCharacter(kCharacterKronos).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 1;
		KronosCall(&LogicManager::CONS_Kronos_DoWalk, 2, 9270, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] == 1) {
			CONS_Kronos_PreConcert(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_PreConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_PreConcert);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 18;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_PreConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2079000 && !getCharacterCurrentParams(kCharacterKronos)[1]) {
			getCharacterCurrentParams(kCharacterKronos)[1] = 1;
			setModel(75, 3);
			setDoor(75, kCharacterCath, 3, 10, 9);
			getCharacterCurrentParams(kCharacterKronos)[0] = 1;
		}

		if (_gameTime <= 2106000 || getCharacterCurrentParams(kCharacterKronos)[2]) {
			if (getCharacterCurrentParams(kCharacterKronos)[0] && inInnerSanctum(kCharacterCath)) {
				getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 1;
				KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKahinaPunchSuite4, 0, 0);
			}
		} else {
			getCharacterCurrentParams(kCharacterKronos)[2] = 1;
			CONS_Kronos_StartConcert(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterKronos).characterPosition.position = 6000;
		getCharacter(kCharacterKronos).characterPosition.location = 0;
		getCharacter(kCharacterKronos).characterPosition.car = kCarKronos;
		break;
	case 18:
		if (getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] == 1) {
			playNIS(kEventKahinaPunchSuite4);
			endGame(3, 253, 0, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_StartConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_StartConcert);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 19;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_StartConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime <= 2115000 || getCharacterCurrentParams(kCharacterKronos)[0]) {
			if (inInnerSanctum(kCharacterCath)) {
				getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 1;
				KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKahinaPunchSuite4, 0, 0);
			}
		} else {
			getCharacterCurrentParams(kCharacterKronos)[0] = 1;
			playDialog(kCharacterKronos, "1917.lnk", -1, 0);
			CONS_Kronos_Concert(0, 0, 0, 0);
		}

		break;
	case 12:
		setDoor(75, kCharacterCath, 1, 0, 0);
		break;
	case 17:
		if (checkLoc(kCharacterCath, kCarKronos) && !inInnerSanctum(kCharacterCath) && !cathHasItem(kItemFirebird) && !_doneNIS[kEventConcertStart]) {
			startCycOtis(kCharacterKronos, "201a");
			getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 2;
			KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventConcertStart, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] == 1) {
			playNIS(kEventKahinaPunchSuite4);
			endGame(3, 253, 0, true);
		} else if (getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] == 2) {
			playNIS(kEventConcertStart);
			claimNISLink(kCharacterKronos);
			bumpCath(kCarKronos, 83, 255);
			forceJump(kCharacterRebecca, &LogicManager::CONS_Rebecca_Concert);
			forceJump(kCharacterSophie, &LogicManager::CONS_Sophie_WithRebecca);
			forceJump(kCharacterAugust, &LogicManager::CONS_August_Concert);
			forceJump(kCharacterAnna, &LogicManager::CONS_Anna_Concert);
			forceJump(kCharacterTatiana, &LogicManager::CONS_Tatiana_Concert);
			CONS_Kronos_Concert(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_Concert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_Concert);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 20;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_Concert(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacterCurrentParams(kCharacterKronos)[4] = getSoundTicks(kCharacterKronos);

		if ((getCharacterCurrentParams(kCharacterKronos)[5] < 54) && (_concertPhasesTicks[getCharacterCurrentParams(kCharacterKronos)[5]] < getCharacterCurrentParams(kCharacterKronos)[4])) {
			startCycOtis(kCharacterKronos, _concertPhases[getCharacterCurrentParams(kCharacterKronos)[5]]);
			if (!scumm_stricmp(_concertPhases[getCharacterCurrentParams(kCharacterKronos)[5]], "201e")) {
				if (checkCathDir(kCarKronos, 85) || checkCathDir(kCarKronos, 86))
					bumpCath(kCarKronos, 83, 255);

				blockView(kCharacterKronos, kCarKronos, 85);
				blockView(kCharacterKronos, kCarKronos, 86);
			} else {
				if (!scumm_stricmp(_concertPhases[getCharacterCurrentParams(kCharacterKronos)[5]], "201c")) {
					if (checkCathDir(kCarKronos, 85))
						bumpCath(kCarKronos, 83, 255);
					blockView(kCharacterKronos, kCarKronos, 85);
					releaseView(kCharacterKronos, kCarKronos, 86);
				} else if (scumm_stricmp(_concertPhases[getCharacterCurrentParams(kCharacterKronos)[5]], "201d")) {
					releaseView(kCharacterKronos, kCarKronos, 85);
					releaseView(kCharacterKronos, kCarKronos, 86);
				} else {
					if (checkCathDir(kCarKronos, 86))
						bumpCath(kCarKronos, 83, 255);

					blockView(kCharacterKronos, kCarKronos, 86);
					releaseView(kCharacterKronos, kCarKronos, 85);
				}
			}

			getCharacterCurrentParams(kCharacterKronos)[5]++;
		}

		if (cathHasItem(kItemBriefcase))
			setDoor(76, kCharacterKronos, 0, 0, 9);
		else
			setDoor(76, kCharacterKronos, 0, 0, 0);

		if (!getCharacterCurrentParams(kCharacterKronos)[6]) {
			getCharacterCurrentParams(kCharacterKronos)[6] = _gameTime + 2700;
			getCharacterCurrentParams(kCharacterKronos)[7] = _gameTime + 13500;
		}

		if (getCharacterCurrentParams(kCharacterKronos)[9] != 0x7FFFFFFF && getCharacterCurrentParams(kCharacterKronos)[6] < _gameTime) {
			if (getCharacterCurrentParams(kCharacterKronos)[7] < _gameTime) {
				getCharacterCurrentParams(kCharacterKronos)[9] = 0x7FFFFFFF;
				send(kCharacterKronos, kCharacterKahina, 237555748, 0);
			} else {
				bool skip = false;

				if (!getCharacterCurrentParams(kCharacterKronos)[0] || !getCharacterCurrentParams(kCharacterKronos)[9]) {
					getCharacterCurrentParams(kCharacterKronos)[9] = _gameTime + 450;

					if (_gameTime == -450) {
						skip = true;
						send(kCharacterKronos, kCharacterKahina, 237555748, 0);
					}
				}

				if (!skip && getCharacterCurrentParams(kCharacterKronos)[9] < _gameTime) {
					getCharacterCurrentParams(kCharacterKronos)[9] = 0x7FFFFFFF;
					send(kCharacterKronos, kCharacterKahina, 237555748, 0);
				}
			}
		}

		if (!getCharacterCurrentParams(kCharacterKronos)[0])
			getCharacterCurrentParams(kCharacterKronos)[1] = getCharacterCurrentParams(kCharacterKronos)[2];

		getCharacterCurrentParams(kCharacterKronos)[1] -= _timeSpeed;

		if (_timeSpeed > getCharacterCurrentParams(kCharacterKronos)[1]) {
			send(kCharacterKronos, kCharacterKahina, 92186062, 0);

			getCharacterCurrentParams(kCharacterKronos)[3]++;
			switch (getCharacterCurrentParams(kCharacterKronos)[3]) {
			case 1:
				playNIS(kEventCathWakingUp);
				cleanNIS();
				getCharacterCurrentParams(kCharacterKronos)[2] = 1800;
				break;
			case 2:
				playNIS(kEventCathWakingUp);
				cleanNIS();
				getCharacterCurrentParams(kCharacterKronos)[2] = 3600;
				break;
			case 3:
				playNIS(kEventCathFallingAsleep);
				fadeToBlack();

				while (dialogRunning("1919.LNK")) {
					_engine->getSoundManager()->soundThread();
					_engine->waitForTimer(4);
				}

				playNIS(kEventCathWakingUp);
				cleanNIS();
				getCharacterCurrentParams(kCharacterKronos)[2] = 162000;
				break;
			default:
				break;
			}

			getCharacterCurrentParams(kCharacterKronos)[1] = getCharacterCurrentParams(kCharacterKronos)[2];
		}

		if ((getCharacterCurrentParams(kCharacterKronos)[4] > 23400 || getCharacterCurrentParams(kCharacterKronos)[8]) && inInnerSanctum(kCharacterCath)) {
			getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 1;
			KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKahinaWrongDoor, 0, 0);
		}

		break;
	case 2:
		setDoor(75, kCharacterCath, 3, 10, 9);
		if (getCharacterCurrentParams(kCharacterKronos)[8]) {
			playDialog(0, "BUMP", -1, 0);
			bumpCath(kCarGreenSleeping, 26, 255);
			CONS_Kronos_AfterConcert(0, 0, 0, 0);
		} else if (inOuterSanctum(kCharacterCath)) {
			getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 3;
			KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventConcertEnd, 0, 0);
		} else if (checkLoc(kCharacterCath, kCarKronos)) {
			playDialog(kCharacterKronos, "Kro3001", -1, 0);
			setDoor(75, kCharacterCath, 3, 0, 0);
			getCharacterCurrentParams(kCharacterKronos)[8] = 1;
		} else {
			CONS_Kronos_AfterConcert(0, 0, 0, 0);
		}

		break;
	case 9:
		getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 2;
		KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventConcertLeaveWithBriefcase, 0, 0);
		break;
	case 12:
		_gameTime = 2115000;
		_timeSpeed = 3;

		if (checkCathDir(kCarKronos, 88) || checkCathDir(kCarKronos, 84) || checkCathDir(kCarKronos, 85) || checkCathDir(kCarKronos, 86) || checkCathDir(kCarKronos, 83)) {
			getCharacterCurrentParams(kCharacterKronos)[0] = 1;
		}

		if (cathHasItem(kItemFirebird))
			setDoor(75, kCharacterCath, 0, 0, 0);
		else
			setDoor(75, kCharacterCath, 0, 10, 9);

		setDoor(76, kCharacterKronos, 0, 10, 9);
		_globals[kGlobalConcertIsHappening] = 1;
		startCycOtis(kCharacterKronos, "201a");
		getCharacterCurrentParams(kCharacterKronos)[2] = 2700;
		getCharacterCurrentParams(kCharacterKronos)[1] = 2700;
		break;
	case 17:
		getCharacterCurrentParams(kCharacterKronos)[0] = checkCathDir(kCarKronos, 88) || checkCathDir(kCarKronos, 84) || checkCathDir(kCarKronos, 85) || checkCathDir(kCarKronos, 86) || checkCathDir(kCarKronos, 83);
		if (cathHasItem(kItemFirebird))
			setDoor(75, kCharacterCath, 3, 0, 0);
		else
			setDoor(75, kCharacterCath, 0, 10, 9);

		break;
	case 18:
		switch (getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8]) {
		case 1:
			playNIS(kEventKahinaWrongDoor);

			if (cathHasItem(kItemBriefcase))
				takeCathItem(kItemBriefcase);

			playDialog(0, "BUMP", -1, 0);
			bumpCath(kCarKronos, 81, 255);
			setDoor(75, kCharacterCath, 3, 0, 0);
			playDialog(0, "lib015", -1, 0);
			break;
		case 2:
			getCharacter(kCharacterCath).characterPosition.position = 6000;
			playNIS(kEventConcertLeaveWithBriefcase);
			forceJump(kCharacterKahina, &LogicManager::CONS_Kahina_Concert);
			bumpCath(kCarKronos, 87, 255);
			break;
		case 3:
			playNIS(kEventConcertEnd);
			playDialog(0, "BUMP", -1, 0);
			bumpCath(kCarGreenSleeping, 26, 255);
			CONS_Kronos_AfterConcert(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_AfterConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_AfterConcert);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 21;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_AfterConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inInnerSanctum(kCharacterCath)) {
			getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 1;
			KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKahinaWrongDoor, 0, 0);
		}

		break;
	case 12:
		_globals[kGlobalConcertIsHappening] = 0;
		setDoor(75, kCharacterCath, 3, 0, 0);
		send(kCharacterKronos, kCharacterRebecca, 191668032, 0);

		if (_doneNIS[kEventConcertLeaveWithBriefcase])
			break;

		CONS_Kronos_AwaitingCath3(0, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] == 1) {
			playNIS(kEventKahinaWrongDoor);

			if (cathHasItem(kItemBriefcase))
				takeCathItem(kItemBriefcase);

			playDialog(0, "BUMP", -1, 0);
			bumpCath(kCarKronos, 81, 255);
			playDialog(0, "lib015", -1, 0);
		}

		break;
	case 235599361:
		CONS_Kronos_AwaitingCath3(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_AwaitingCath3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_AwaitingCath3);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 22;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_AwaitingCath3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inInnerSanctum(kCharacterCath)) {
			if (_globals[kGlobalKahinaKillTimeoutActive]) {
				getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 5;
				KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKahinaPunchBaggageCarEntrance, 0, 0);
			} else {
				getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 6;
				KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKahinaWrongDoor, 0, 0);
			}
		}

		break;
	case 8:
	case 9:
		if (msg->action == 8) {
			if (!cathRunningDialog("LIB012"))
				playDialog(0, "LIB012", -1, 0);
		} else if (!cathRunningDialog("LIB013")) {
			playDialog(0, "LIB013", -1, 0);
		}

		if (_doneNIS[kEventConcertLeaveWithBriefcase])
			fedEx(kCharacterKronos, kCharacterKahina, 137503360, 0);
		if (cathHasItem(kItemBriefcase)) {
			getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 1;
			KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKronosReturnBriefcase, 0, 0);
		} else if (cathHasItem(kItemFirebird) && _doneNIS[kEventConcertLeaveWithBriefcase]) {
			getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 2;
			KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKronosBringEggCeiling, 0, 0);
		} else if (cathHasItem(kItemFirebird)) {
			getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 3;
			KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKronosBringEggCeiling, 0, 0);
		} else if (_doneNIS[kEventConcertLeaveWithBriefcase]) {
			getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 4;
			KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKronosBringNothing, 0, 0);
		}

		break;
	case 12:
		setDoor(75, kCharacterKronos, 3, 10, 9);
		break;
	case 18:
		switch (getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8]) {
		case 1:
			playNIS(kEventKronosReturnBriefcase);
			bumpCath(kCarKronos, 87, 255);
			takeCathItem(kItemBriefcase);
			takeCathItem(kItemScarf);
			CONS_Kronos_Finished(0, 0, 0, 0);
			break;
		case 2:
			playNIS(kEventKronosBringEggCeiling);
			bumpCath(kCarKronos, 87, 255);
			takeCathItem(kItemFirebird);
			_items[kItemFirebird].floating = 5;
			CONS_Kronos_Finished(0, 0, 0, 0);
			break;
		case 3:
			takeCathItem(kItemFirebird);
			_items[kItemFirebird].floating = 5;
			playNIS(kEventKronosBringEgg);
			bumpCath(kCarKronos, 87, 255);
			giveCathItem(kItemBriefcase);
			CONS_Kronos_Finished(0, 0, 0, 0);
			break;
		case 4:
			playNIS(kEventKronosBringNothing);
			endGame(0, 1, 0, true);
			break;
		case 5:
			playNIS(kEventKahinaPunchSuite4);
			endGame(0, 1, 0, true);
			break;
		case 6:
			playNIS(kEventKahinaWrongDoor);
			if (cathHasItem(kItemBriefcase))
				takeCathItem(kItemBriefcase);
			playDialog(0, "BUMP", -1, 0);
			bumpCath(kCarKronos, 81, 255);
			playDialog(0, "lib015", -1, 0);
			break;
		default:
			break;
		}

		break;
	case 138085344:
		CONS_Kronos_Finished(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_Finished(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_Finished);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 23;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_Finished(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inInnerSanctum(kCharacterCath)) {
			getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] = 1;
			KronosCall(&LogicManager::CONS_Kronos_SaveGame, 2, kEventKahinaWrongDoor, 0, 0);
		}

		break;
	case 12:
		setDoor(75, kCharacterCath, 3, 10, 9);
		break;
	case 18:
		if (getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall + 8] == 1) {
			playNIS(kEventKahinaWrongDoor);
			if (cathHasItem(kItemBriefcase))
				takeCathItem(kItemBriefcase);
			playDialog(0, "BUMP", -1, 0);
			bumpCath(kCarKronos, 81, 255);
			playDialog(0, "lib015", -1, 0);
		}

		break;
	default:
		break;
	} 
}

void LogicManager::CONS_Kronos_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_StartPart4);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 24;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterKronos);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kronos_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKronos).callParams[getCharacter(kCharacterKronos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKronos, &LogicManager::HAND_Kronos_StartPart5);
	getCharacter(kCharacterKronos).callbacks[getCharacter(kCharacterKronos).currentCall] = 25;

	params->clear();

	fedEx(kCharacterKronos, kCharacterKronos, 12, 0);
}

void LogicManager::HAND_Kronos_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterKronos);
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsKronos[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Kronos_DebugWalks,
	&LogicManager::HAND_Kronos_SaveGame,
	&LogicManager::HAND_Kronos_DoWalk,
	&LogicManager::HAND_Kronos_DoDialog,
	&LogicManager::HAND_Kronos_DoWait,
	&LogicManager::HAND_Kronos_DoWaitReal,
	&LogicManager::HAND_Kronos_Birth,
	&LogicManager::HAND_Kronos_AwaitingCath,
	&LogicManager::HAND_Kronos_GreetCath,
	&LogicManager::HAND_Kronos_CathDone,
	&LogicManager::HAND_Kronos_PlayingAtNight,
	&LogicManager::HAND_Kronos_StartPart2,
	&LogicManager::HAND_Kronos_StartPart3,
	&LogicManager::HAND_Kronos_InSeclusion,
	&LogicManager::HAND_Kronos_WBWait,
	&LogicManager::HAND_Kronos_VisitSalon,
	&LogicManager::HAND_Kronos_ReturnComp,
	&LogicManager::HAND_Kronos_PreConcert,
	&LogicManager::HAND_Kronos_StartConcert,
	&LogicManager::HAND_Kronos_Concert,
	&LogicManager::HAND_Kronos_AfterConcert,
	&LogicManager::HAND_Kronos_AwaitingCath3,
	&LogicManager::HAND_Kronos_Finished,
	&LogicManager::HAND_Kronos_StartPart4,
	&LogicManager::HAND_Kronos_StartPart5
};

} // End of namespace LastExpress
