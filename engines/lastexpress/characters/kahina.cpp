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

void LogicManager::CONS_Kahina(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterKahina,
			_functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]
		);

		break;
	case 1:
		CONS_Kahina_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Kahina_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Kahina_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Kahina_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Kahina_StartPart5(0, 0, 0, 0);
		break;
	default:
		return;
	}
}

void LogicManager::KahinaCall(CALL_PARAMS) {
	getCharacter(kCharacterKahina).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Kahina_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_DebugWalks);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 1;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterKahina, kCarGreenSleeping, getCharacterCurrentParams(kCharacterKahina)[0])) {
			if (getCharacterCurrentParams(kCharacterKahina)[0] == 10000) {
				getCharacterCurrentParams(kCharacterKahina)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterKahina)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterKahina).characterPosition.position = 0;
		getCharacter(kCharacterKahina).characterPosition.location = 0;
		getCharacter(kCharacterKahina).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterKahina)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_DoDialog);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterKahina).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
		fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		break;
	case 12:
		playDialog(kCharacterKahina, (char *)&getCharacterCurrentParams(kCharacterKahina)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_SaveGame);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 3;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterKahina).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
		fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		break;
	case 12:
		save(
			kCharacterKahina,
			getCharacterCurrentParams(kCharacterKahina)[0],
			getCharacterCurrentParams(kCharacterKahina)[1]
		);

		getCharacter(kCharacterKahina).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
		fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_DoWait);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 4;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterKahina)[1] || (getCharacterCurrentParams(kCharacterKahina)[1] = _gameTime + getCharacterCurrentParams(kCharacterKahina)[0],
															   _gameTime + getCharacterCurrentParams(kCharacterKahina)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterKahina)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterKahina)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterKahina).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
		fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		break;
	case 137503360:
		getCharacterParams(kCharacterKahina, 8)[1] = 1;

		getCharacter(kCharacterKahina).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
		fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_DoWaitReal(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_DoWaitReal);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 5;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_DoWaitReal(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterKahina)[1] || (getCharacterCurrentParams(kCharacterKahina)[1] = _realTime + getCharacterCurrentParams(kCharacterKahina)[0],
															   _realTime + getCharacterCurrentParams(kCharacterKahina)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterKahina)[1] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterKahina)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterKahina).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
		fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_LookingForCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_LookingForCath);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 6;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_LookingForCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterKahina)[0] >= _gameTime || getCharacterCurrentParams(kCharacterKahina)[1]) {
			if (cathInCorridor(kCarGreenSleeping) || cathInCorridor(kCarRedSleeping)) {
				if (checkLoc(kCharacterCath, kCarGreenSleeping)) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
					KahinaCall(&LogicManager::CONS_Kahina_DoWalkSearchingForCath, 3, 540, 0, 0);
				} else {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
					KahinaCall(&LogicManager::CONS_Kahina_DoWalkSearchingForCath, 4, 9460, 0, 0);
				}
			}
		} else {
			getCharacterCurrentParams(kCharacterKahina)[1] = 1;

			getCharacter(kCharacterKahina).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
			fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		}

		break;
	case 12:
		getCharacterParams(kCharacterKahina, 8)[0] = 0;
		getCharacterParams(kCharacterKahina, 8)[1] = 0;
		getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
		KahinaCall(&LogicManager::CONS_Kahina_DoWalkSearchingForCath, 4, 540, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8]) {
		case 1:
			if (getCharacterParams(kCharacterKahina, 8)[0] || getCharacterParams(kCharacterKahina, 8)[1]) {
				getCharacter(kCharacterKahina).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
				fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
			} else {
				endGraphics(kCharacterKahina);
			}

			break;
		case 2:
		case 3:
			if (getCharacterParams(kCharacterKahina, 8)[0] || getCharacterParams(kCharacterKahina, 8)[1]) {
				getCharacter(kCharacterKahina).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
				fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
			} else {
				endGraphics(kCharacterKahina);
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
				KahinaCall(&LogicManager::CONS_Kahina_DoWait, 450, 0, 0, 0);
			}

			break;
		case 4:
			if (getCharacterParams(kCharacterKahina, 8)[1]) {
				getCharacter(kCharacterKahina).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
				fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
			} else {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 5;
				KahinaCall(&LogicManager::CONS_Kahina_DoWalkSearchingForCath, 4, 540, 0, 0);
			}

			break;
		case 5:
			if (getCharacterParams(kCharacterKahina, 8)[0] || getCharacterParams(kCharacterKahina, 8)[1]) {
				getCharacter(kCharacterKahina).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
				fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
			} else {
				endGraphics(kCharacterKahina);
			}
			break;
		default:
			break;
		}

		break;
	case 137503360:
		getCharacterParams(kCharacterKahina, 8)[1] = 1;

		getCharacter(kCharacterKahina).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
		fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_DoWalkSearchingForCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_DoWalkSearchingForCath);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 7;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_DoWalkSearchingForCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterKahina, getCharacterCurrentParams(kCharacterKahina)[0], getCharacterCurrentParams(kCharacterKahina)[1])) {
			getCharacter(kCharacterKahina).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
			fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
			break;
		}

		if (nearChar(kCharacterKahina, kCharacterCath, 1000) && !onLowPlatform(kCharacterCath) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			if (getCharacter(kCharacterKahina).characterPosition.car == kCarGreenSleeping || getCharacter(kCharacterKahina).characterPosition.car == kCarRedSleeping) {
				getCharacterParams(kCharacterKahina, 8)[0] = 1;

				getCharacter(kCharacterKahina).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
				fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
			}
		}
		break;
	case 12:
		if (walk(kCharacterKahina, getCharacterCurrentParams(kCharacterKahina)[0], getCharacterCurrentParams(kCharacterKahina)[1])) {
			getCharacter(kCharacterKahina).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
			fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		}

		break;
	case 137503360:
		getCharacterParams(kCharacterKahina, 8)[1] = 1;

		getCharacter(kCharacterKahina).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
		fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_DoWalk);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 8;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterKahina, getCharacterCurrentParams(kCharacterKahina)[0], getCharacterCurrentParams(kCharacterKahina)[1])) {
			getCharacter(kCharacterKahina).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
			fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		}

		break;
	case 5:
		if (_doneNIS[kEventKronosConversation] || _doneNIS[kEventKronosConversationFirebird]) {
			if (rnd(2) == 0) {
				playDialog(kCharacterCath, "CAT1019A", -1, 0);
			} else {
				playDialog(kCharacterCath, "CAT1019", -1, 0);
			}
		} else {
			playCathExcuseMe();
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterKahina, kCharacterCath, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_DoCorrOtis);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 9;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterKahina, getCharacterCurrentParams(kCharacterKahina)[3]);

		getCharacter(kCharacterKahina).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
		fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterKahina, (char *)&getCharacterCurrentParams(kCharacterKahina)[0]);
		blockAtDoor(kCharacterKahina, getCharacterCurrentParams(kCharacterKahina)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_Birth);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 10;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterKahina)[0]) {
			getCharacterCurrentParams(kCharacterKahina)[0] = 1;
			CONS_Kahina_InSeclusion(0, 0, 0, 0);
		}

		break;
	case 12:
		setDoor(75, kCharacterCath, 1, 10, 9);
		getCharacter(kCharacterKahina).characterPosition.position = 5000;
		getCharacter(kCharacterKahina).characterPosition.location = 0;
		getCharacter(kCharacterKahina).characterPosition.car = kCarKronos;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_InSeclusion(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_InSeclusion);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 11;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_InSeclusion(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1107000 && !getCharacterCurrentParams(kCharacterKahina)[0] && _globals[kGlobalJacket]) {
			send(kCharacterKahina, kCharacterCond1, 238732837, 0);
			getCharacterCurrentParams(kCharacterKahina)[0] = 1;
		}

		if (_globals[kGlobalKnowAboutKronos])
			CONS_Kahina_AwaitingCath(0, 0, 0, 0);

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_AwaitingCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_AwaitingCath);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 12;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_AwaitingCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1485000 && !getCharacterCurrentParams(kCharacterKahina)[1]) {
			getCharacterCurrentParams(kCharacterKahina)[1] = 1;
			CONS_Kahina_CathDone(0, 0, 0, 0);
		}

		break;
	case 8:
	case 9:
		if (msg->action == 8)
			playDialog(0, "LIB012", -1, 0);

		if (!_doneNIS[kEventKronosGoingToInvitation]) {
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
			KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKronosGoingToInvitation, 0, 0);
			break;
		}

		if (msg->action == 9)
			playDialog(0, "LIB014", -1, 0);

		bumpCath(kCarKronos, 80, 255);
		send(kCharacterKahina, kCharacterKronos, 171849314, 0);
		getCharacterCurrentParams(kCharacterKahina)[0] = 1;
		break;
	case 12:
		setDoor(75, kCharacterKahina, 0, 10, 9);
		break;
	case 18:
		if (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] == 1) {
			playNIS(kEventKronosGoingToInvitation);
			bumpCath(kCarKronos, 80, 255);
			send(kCharacterKahina, kCharacterKronos, 171849314, 0);
			getCharacterCurrentParams(kCharacterKahina)[0] = 1;
		}

		break;
	case 137685712:
		CONS_Kahina_CathDone(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_CathDone(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_CathDone);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 13;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_CathDone(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!_globals[kGlobalCharacterSearchingForCath] && _gameTime < 1201500 && getCharacterCurrentParams(kCharacterKahina)[1] != 0x7FFFFFFF && getCharacterCurrentParams(kCharacterKahina)[0] < _gameTime) {
			if (_gameTime <= 1197000) {
				if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterKahina)[1]) {
					getCharacterCurrentParams(kCharacterKahina)[1] = _gameTime;
					if (!_gameTime) {
						getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
						KahinaCall(&LogicManager::CONS_Kahina_SearchTrain, 0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterKahina)[1] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterKahina)[1] = 0x7FFFFFFF;
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
			KahinaCall(&LogicManager::CONS_Kahina_SearchTrain, 0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterKahina).characterPosition.car = kCarKronos;
		getCharacter(kCharacterKahina).characterPosition.position = 5000;
		getCharacter(kCharacterKahina).characterPosition.location = 0;
		setDoor(75, kCharacterCath, 1, 10, 9);
		getCharacterCurrentParams(kCharacterKahina)[0] = _gameTime + 1800;
		break;
	default:
		break;
	}
	
}

void LogicManager::CONS_Kahina_DoDogBarkOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_DoDogBarkOtis);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 14;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_DoDogBarkOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
	case 4:
		releaseAtDoor(kCharacterKahina, 37);

		getCharacter(kCharacterKahina).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
		fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterKahina, "616Cf");
		blockAtDoor(kCharacterKahina, 37);
		send(kCharacterKahina, kCharacterMax, 158007856, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_SearchTrain(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_SearchTrain);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 15;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_SearchTrain(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterKahina)[1] != 0x7FFFFFFF && _gameTime) {
			if (getCharacterCurrentParams(kCharacterKahina)[0] >= _gameTime) {
				if (!cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterKahina)[1]) {
					getCharacterCurrentParams(kCharacterKahina)[1] = _gameTime;
					if (!_gameTime) {
						getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 9;
						KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 4, 4070, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterKahina)[1] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterKahina)[1] = 0x7FFFFFFF;
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 9;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 4, 4070, 0, 0);
		}

		break;
	case 12:
		_globals[kGlobalCharacterSearchingForCath] = kCharacterKahina;
		getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
		KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 3, 8200, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8]) {
		case 1:
			if (!whoOnScreen(kCharacterKahina)) {
				if (inComp(kCharacterCath, kCarGreenSleeping, 8200) || cathOutHisWindow() || nearChar(kCharacterKahina, kCharacterCath, 2000)) {
					if (_globals[kGlobalCharacterSearchingForCath] == kCharacterKahina)
						_globals[kGlobalCharacterSearchingForCath] = 0;

					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 8;
					KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 3, 9460, 0, 0);
				} else {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 5;
					KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Aa", 1, 0, 0);
				}

				break;
			}

			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 4, 9460, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 1800, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 3, 8200, 0, 0);
			break;
		case 4:
			if (inComp(kCharacterCath, kCarGreenSleeping, 8200) || cathOutHisWindow() || nearChar(kCharacterKahina, kCharacterCath, 2000)) {
				if (_globals[kGlobalCharacterSearchingForCath] == kCharacterKahina)
					_globals[kGlobalCharacterSearchingForCath] = 0;

				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 8;
				KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 3, 9460, 0, 0);
			} else {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 5;
				KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Aa", 1, 0, 0);
			}

			break;
		case 5:
			getCharacter(kCharacterKahina).characterPosition.location = 1;
			endGraphics(kCharacterKahina);
			setDoor(1, kCharacterCath, 0, 0, 0);
			setDoor(17, kCharacterCath, 0, 0, 0);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 6;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 900, 0, 0, 0);
			break;
		case 6:
			setDoor(1, kCharacterCath, 0, 10, 9);
			setDoor(17, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 7;
			KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Ba", 1, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterKahina).characterPosition.location = 0;

			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterKahina)
				_globals[kGlobalCharacterSearchingForCath] = 0;

			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 8;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 3, 9460, 0, 0);
			break;
		case 8:
			endGraphics(kCharacterKahina);
			getCharacterCurrentParams(kCharacterKahina)[0] = _gameTime + 4500;
			break;
		case 9:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 10;
			KahinaCall(&LogicManager::CONS_Kahina_DoDogBarkOtis, 0, 0, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 11;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 4, 6470, 0, 0);
			break;
		case 11:
			if (inSuite(kCharacterCath, kCarRedSleeping, 6130)) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 15;
				KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 4, 9460, 0, 0);
			} else {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 12;
				KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Ac", 34, 0, 0);
			}

			break;
		case 12:
			getCharacter(kCharacterKahina).characterPosition.location = 1;
			endGraphics(kCharacterKahina);
			setDoor(34, kCharacterCath, checkDoor(34), 0, 0);
			setDoor(50, kCharacterCath, checkDoor(50), 0, 0);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 13;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 900, 0, 0, 0);
			break;
		case 13:
			setDoor(34, kCharacterCath, checkDoor(34), 10, 9);
			setDoor(50, kCharacterCath, checkDoor(50), 10, 9);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 14;
			KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Bc", 34, 0, 0);
			break;
		case 14:
			getCharacter(kCharacterKahina).characterPosition.location = 0;
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 15;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 4, 9460, 0, 0);
			break;
		case 15:
			endGraphics(kCharacterKahina);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 16;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 900, 0, 0, 0);
			break;
		case 16:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 17;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 2, 9270, 0, 0);
			break;
		case 17:
			endGraphics(kCharacterKahina);

			getCharacter(kCharacterKahina).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
			fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_StartPart2);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 16;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Kahina_InSeclusionPart2(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterKahina);
		getCharacter(kCharacterKahina).characterPosition.car = kCarKronos;
		getCharacter(kCharacterKahina).characterPosition.position = 6000;
		getCharacter(kCharacterKahina).characterPosition.location = 0;
		getCharacter(kCharacterKahina).inventoryItem = 0;
		getCharacter(kCharacterKahina).clothes = 0;
		setDoor(75, kCharacterKahina, 1, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_InSeclusionPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_InSeclusionPart2);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 17;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_InSeclusionPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterKahina)[0]) {
			bool skip = false;

			if (!getCharacterCurrentParams(kCharacterKahina)[1]) {
				getCharacterCurrentParams(kCharacterKahina)[1] = _gameTime + 9000;
				if (_gameTime == -9000) {
					skip = true;
					getCharacterCurrentParams(kCharacterKahina)[0] = 1;
					getCharacterCurrentParams(kCharacterKahina)[1] = 0;
				}
			}

			if (!skip && getCharacterCurrentParams(kCharacterKahina)[1] < _gameTime) {
				getCharacterCurrentParams(kCharacterKahina)[1] = 0x7FFFFFFF;
				getCharacterCurrentParams(kCharacterKahina)[0] = 1;
				getCharacterCurrentParams(kCharacterKahina)[1] = 0;
			}
		}

		if (!_doneNIS[kEventKahinaAskSpeakFirebird] || _doneNIS[kEventKronosConversationFirebird] || !checkLoc(kCharacterCath, kCarKronos)) {
			if (_gameTime > 1845000 && _doneNIS[kEventKronosConversationFirebird] && inOuterSanctum(kCharacterCath)) {
				setDoor(75, kCharacterCath, 1, 10, 9);
				bumpCath(kCarKronos, 87, 255);
			}

			break;
		}

		if (!getCharacterCurrentParams(kCharacterKahina)[2]) {
			getCharacterCurrentParams(kCharacterKahina)[2] = _gameTime + 900;
			if (_gameTime == -900) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
				KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKronosConversationFirebird, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterKahina)[2] < _gameTime) {
			getCharacterCurrentParams(kCharacterKahina)[2] = 0x7FFFFFFF;
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
			KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKronosConversationFirebird, 0, 0);
			break;
		}

		if (_gameTime > 1845000 && _doneNIS[kEventKronosConversationFirebird] && inOuterSanctum(kCharacterCath)) {
			setDoor(75, kCharacterCath, 1, 10, 9);
			bumpCath(kCarKronos, 87, 255);
		}

		break;
	case 8:
	case 9:
		if (!_doneNIS[kEventKronosConversationFirebird]) {
			if (_doneNIS[kEventKahinaAskSpeakFirebird]) {
				if (whoRunningDialog(kCharacterKahina))
					fadeDialog(kCharacterKahina);

				if (msg->action == 8)
					playDialog(0, "LIB012", -1, 0);

				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
				KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKronosConversationFirebird, 0, 0);
			} else if (_doneNIS[kEventMilosCompartmentVisitAugust] || _doneNIS[kEventTatianaGivePoem] || _doneNIS[kEventTatianaBreakfastGivePoem]) {
				if (msg->action == 8)
					playDialog(0, "LIB012", -1, 0);

				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 7;
				KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKahinaAskSpeakFirebird, 0, 0);
			} else if (getCharacterCurrentParams(kCharacterKahina)[0]) {
				if (msg->action == 8)
					playDialog(0, "LIB012", -1, 0);

				playNIS(kEventKahinaAskSpeak);
				cleanNIS();
				setDoor(75, kCharacterKahina, 1, 0, 0);
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 8;
				KahinaCall(&LogicManager::CONS_Kahina_DoDialog, "KRO3003", 0, 0, 0);
			} else {
				setDoor(75, kCharacterKahina, 1, 0, 0);

				if (msg->action == 8) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 9;
					KahinaCall(&LogicManager::CONS_Kahina_DoDialog, "LIB012", 0, 0, 0);
				} else {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 10;
					KahinaCall(&LogicManager::CONS_Kahina_DoDialog, "LIB013", 0, 0, 0);
				}
			}
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterKahina)[0] = 1;
		setDoor(75, kCharacterKahina, 1, 10, 9);
		break;
	case 18:
		switch (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8]) {
		case 1:
			playNIS(kEventKronosConversationFirebird);
			setDoor(75, kCharacterCath, 0, 10, 9);
			bumpCath(kCarKronos, 80, 1);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 900, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
			KahinaCall(&LogicManager::CONS_Kahina_DoDialog, "KRO3005", 0, 0, 0);
			break;
		case 3:
			if (_gameTime > 1845000 && _doneNIS[kEventKronosConversationFirebird] && inOuterSanctum(kCharacterCath)) {
				setDoor(75, kCharacterCath, 1, 10, 9);
				bumpCath(kCarKronos, 87, 255);
			}

			break;
		case 4:
			playNIS(kEventKronosConversationFirebird);
			setDoor(75, kCharacterCath, 0, 10, 9);
			bumpCath(kCarKronos, 80, 1);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 5;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 900, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 6;
			KahinaCall(&LogicManager::CONS_Kahina_DoDialog, "KRO3005", 0, 0, 0);
			break;
		case 7:
			playNIS(kEventKahinaAskSpeakFirebird);
			bumpCath(kCarKronos, 81, 255);
			playDialog(kCharacterKahina, "KRO3004", -1, 0);
			break;
		case 8:
			setDoor(75, kCharacterKahina, 1, 10, 9);
			getCharacterCurrentParams(kCharacterKahina)[0] = 0;
			break;
		case 9:
		case 10:
			setDoor(75, kCharacterKahina, 1, 10, 9);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_StartPart3);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 18;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Kahina_BeforeConcert(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterKahina);
		getCharacter(kCharacterKahina).characterPosition.car = kCarKronos;
		getCharacter(kCharacterKahina).characterPosition.position = 5000;
		getCharacter(kCharacterKahina).characterPosition.location = 0;
		getCharacter(kCharacterKahina).inventoryItem = 0;
		getCharacter(kCharacterKahina).clothes = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_DoWalk1033(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_DoWalk1033);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 19;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_DoWalk1033(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_doneNIS[kEventAnnaBaggageArgument])
			forceJump(kCharacterKahina, &LogicManager::CONS_Kahina_Finished);

		if (walk(kCharacterKahina, getCharacterCurrentParams(kCharacterKahina)[0], getCharacterCurrentParams(kCharacterKahina)[1])) {
			getCharacter(kCharacterKahina).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
			fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		}

		break;
	case 5:
		if (_doneNIS[kEventKronosConversation] || _doneNIS[kEventKronosConversationFirebird]) {
			if (rnd(2) == 0) {
				playDialog(kCharacterCath, "CAT1019A", -1, 0);
			} else {
				playDialog(kCharacterCath, "CAT1019", -1, 0);
			}
		} else {
			playCathExcuseMe();
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterKahina, kCharacterCath, 0);
		break;
	case 12:
		if (walk(kCharacterKahina, getCharacterCurrentParams(kCharacterKahina)[0], getCharacterCurrentParams(kCharacterKahina)[1])) {
			getCharacter(kCharacterKahina).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
			fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_BeforeConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_BeforeConcert);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 20;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_BeforeConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_doneNIS[kEventKronosVisit])
			setDoor(75, kCharacterCath, 3, 10, 9);

		if (inInnerSanctum(kCharacterCath)) {
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
			KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKahinaPunchSuite4, 0, 0);
			break;
		}

		if (_gameTime > 2079000 && !getCharacterCurrentParams(kCharacterKahina)[1]) {
			getCharacterCurrentParams(kCharacterKahina)[1] = 1;
			if (_doneNIS[kEventKahinaAskSpeakFirebird] && !_doneNIS[kEventKronosConversationFirebird] && checkLoc(kCharacterCath, kCarKronos)) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
				KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKronosConversationFirebird, 0, 0);
			} else {
				if (inOuterSanctum(kCharacterCath))
					bumpCath(kCarKronos, 87, 255);

				CONS_Kahina_Concert(0, 0, 0, 0);
			}

			break;
		}

		if (!getCharacterCurrentParams(kCharacterKahina)[0]) {
			bool skip = false;

			if (!getCharacterCurrentParams(kCharacterKahina)[2]) {
				getCharacterCurrentParams(kCharacterKahina)[2] = _gameTime + 9000;
				if (_gameTime == -9000) {
					skip = true;
					getCharacterCurrentParams(kCharacterKahina)[0] = 1;
					getCharacterCurrentParams(kCharacterKahina)[2] = 0;
				}
			}

			if (!skip && getCharacterCurrentParams(kCharacterKahina)[2] < _gameTime) {
				getCharacterCurrentParams(kCharacterKahina)[2] = 0x7FFFFFFF;
				getCharacterCurrentParams(kCharacterKahina)[0] = 1;
				getCharacterCurrentParams(kCharacterKahina)[2] = 0;
			}
		}

		if (_doneNIS[kEventKahinaAskSpeakFirebird] && !_doneNIS[kEventKronosConversationFirebird] && checkLoc(kCharacterCath, kCarKronos)) {
			if (getCharacterCurrentParams(kCharacterKahina)[3] || (getCharacterCurrentParams(kCharacterKahina)[3] = _gameTime + 900,
																   _gameTime != -900)) {
				if (getCharacterCurrentParams(kCharacterKahina)[3] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterKahina)[3] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
			KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKronosConversationFirebird, 0, 0);
		}

		break;
	case 8:
	case 9:
		if (!_doneNIS[kEventKronosConversationFirebird]) {
			if (_doneNIS[kEventKahinaAskSpeakFirebird]) {
				if (msg->action == 8)
					playDialog(0, "LIB012", -1, 0);

				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 6;
				KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKronosConversationFirebird, 0, 0);
			} else if (_doneNIS[kEventMilosCompartmentVisitAugust] || _doneNIS[kEventTatianaGivePoem] || _doneNIS[kEventTatianaBreakfastGivePoem]) {
				if (msg->action == 8)
					playDialog(0, "LIB012", -1, 0);

				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 9;
				KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKahinaAskSpeakFirebird, 0, 0);
			} else if (getCharacterCurrentParams(kCharacterKahina)[0]) {
				if (msg->action == 8)
					playDialog(0, "LIB012", -1, 0);

				playNIS(kEventKahinaAskSpeak);
				cleanNIS();
				setDoor(75, kCharacterKahina, 1, 0, 0);
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 10;
				KahinaCall(&LogicManager::CONS_Kahina_DoDialog, "KRO3003", 0, 0, 0);
			} else {
				setDoor(75, kCharacterKahina, 1, 0, 0);

				if (msg->action == 8) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 11;
					KahinaCall(&LogicManager::CONS_Kahina_DoDialog, "LIB012", 0, 0, 0);
				} else {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 12;
					KahinaCall(&LogicManager::CONS_Kahina_DoDialog, "LIB013", 0, 0, 0);
				}
			}
		}

		break;
	case 12:
		if (_doneNIS[kEventKronosConversationFirebird]) {
			setDoor(75, kCharacterCath, 3, 10, 9);
		} else {
			setDoor(75, kCharacterKahina, 1, 10, 9);
			getCharacterCurrentParams(kCharacterKahina)[0] = 1;
		}
		return;
	case 18:
		switch (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8]) {
		case 1:
			playNIS(kEventKahinaPunchSuite4);
			endGame(3, 253, 0, true);

			if (_gameTime > 2079000 && !getCharacterCurrentParams(kCharacterKahina)[1]) {
				getCharacterCurrentParams(kCharacterKahina)[1] = 1;
				if (_doneNIS[kEventKahinaAskSpeakFirebird] && !_doneNIS[kEventKronosConversationFirebird] && checkLoc(kCharacterCath, kCarKronos)) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
					KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKronosConversationFirebird, 0, 0);
				} else {
					if (inOuterSanctum(kCharacterCath))
						bumpCath(kCarKronos, 87, 255);

					CONS_Kahina_Concert(0, 0, 0, 0);
				}

				break;
			}

			if (!getCharacterCurrentParams(kCharacterKahina)[0]) {
				bool skip = false;

				if (!getCharacterCurrentParams(kCharacterKahina)[2]) {
					getCharacterCurrentParams(kCharacterKahina)[2] = _gameTime + 9000;
					if (_gameTime == -9000) {
						skip = true;
						getCharacterCurrentParams(kCharacterKahina)[0] = 1;
						getCharacterCurrentParams(kCharacterKahina)[2] = 0;
					}
				}

				if (!skip && getCharacterCurrentParams(kCharacterKahina)[2] < _gameTime) {
					getCharacterCurrentParams(kCharacterKahina)[2] = 0x7FFFFFFF;
					getCharacterCurrentParams(kCharacterKahina)[0] = 1;
					getCharacterCurrentParams(kCharacterKahina)[2] = 0;
				}
			}

			if (_doneNIS[kEventKahinaAskSpeakFirebird] && !_doneNIS[kEventKronosConversationFirebird] && checkLoc(kCharacterCath, kCarKronos)) {
				if (getCharacterCurrentParams(kCharacterKahina)[3] || (getCharacterCurrentParams(kCharacterKahina)[3] = _gameTime + 900,
																	   _gameTime != -900)) {
					if (getCharacterCurrentParams(kCharacterKahina)[3] >= _gameTime)
						break;

					getCharacterCurrentParams(kCharacterKahina)[3] = 0x7FFFFFFF;
				}

				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
				KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKronosConversationFirebird, 0, 0);
			}

			break;
		case 2:
			playNIS(kEventKronosConversationFirebird);
			bumpCath(kCarKronos, 87, 255);

			if (inOuterSanctum(kCharacterCath))
				bumpCath(kCarKronos, 87, 255);

			CONS_Kahina_Concert(0, 0, 0, 0);
			break;
		case 3:
			playNIS(kEventKronosConversationFirebird);
			setDoor(75, kCharacterCath, 0, 10, 9);
			bumpCath(kCarKronos, 80, 1);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 900, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 5;
			KahinaCall(&LogicManager::CONS_Kahina_DoDialog, "KRO3005", 0, 0, 0);
			break;
		case 6:
			playNIS(kEventKronosConversationFirebird);
			setDoor(75, kCharacterCath, 0, 10, 9);
			bumpCath(kCarKronos, 80, 1);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 7;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 900, 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 8;
			KahinaCall(&LogicManager::CONS_Kahina_DoDialog, "KRO3005", 0, 0, 0);
			break;
		case 9:
			playNIS(kEventKahinaAskSpeakFirebird);
			bumpCath(kCarKronos, 81, 255);
			playDialog(kCharacterKahina, "KRO3004", -1, 0);
			break;
		case 10:
			setDoor(75, kCharacterKahina, 1, 10, 9);
			getCharacterCurrentParams(kCharacterKahina)[0] = 0;
			break;
		case 11:
		case 12:
			setDoor(75, kCharacterKahina, 1, 10, 9);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_Concert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_Concert);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 21;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_Concert(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterKahina)[0]) {
			if (!getCharacterCurrentParams(kCharacterKahina)[2])
				getCharacterCurrentParams(kCharacterKahina)[2] = _gameTime + 4500;

			if (getCharacterCurrentParams(kCharacterKahina)[4] != 0x7FFFFFFF && _gameTime) {
				if (getCharacterCurrentParams(kCharacterKahina)[2] < _gameTime) {
					getCharacterCurrentParams(kCharacterKahina)[4] = 0x7FFFFFFF;
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
					KahinaCall(&LogicManager::CONS_Kahina_FindFirebird, 0, 0, 0, 0);
					break;
				}

				if (checkCathDir(kCarKronos, 80) || checkCathDir(kCarKronos, 88) || !getCharacterCurrentParams(kCharacterKahina)[4]) {
					getCharacterCurrentParams(kCharacterKahina)[4] = _gameTime;
					if (!_gameTime) {
						getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
						KahinaCall(&LogicManager::CONS_Kahina_FindFirebird, 0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterKahina)[4] < _gameTime) {
					getCharacterCurrentParams(kCharacterKahina)[4] = 0x7FFFFFFF;
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
					KahinaCall(&LogicManager::CONS_Kahina_FindFirebird, 0, 0, 0, 0);
					break;
				}
			}
		}

		if (!getCharacterCurrentParams(kCharacterKahina)[1]) {
			if (!_globals[kGlobalKahinaKillTimeoutActive] && _gameTime > 2214000) {
				if (_items[kItemFirebird].floating == 3 || _items[kItemFirebird].floating == 7) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
					KahinaCall(&LogicManager::CONS_Kahina_SearchCath, 0, 0, 0, 0);
				} else if (_items[kItemFirebird].floating == 2 || _items[kItemFirebird].floating == 1) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
					KahinaCall(&LogicManager::CONS_Kahina_SearchTatiana, 0, 0, 0, 0);
				}
			}

			break;
		}

		if (!getCharacterCurrentParams(kCharacterKahina)[3])
			getCharacterCurrentParams(kCharacterKahina)[3] = _gameTime + 4500;

		if (getCharacterCurrentParams(kCharacterKahina)[5] == 0x7FFFFFFF || !_gameTime) {
			if (!_globals[kGlobalKahinaKillTimeoutActive] && _gameTime > 2214000) {
				if (_items[kItemFirebird].floating == 3 || _items[kItemFirebird].floating == 7) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
					KahinaCall(&LogicManager::CONS_Kahina_SearchCath, 0, 0, 0, 0);
				} else if (_items[kItemFirebird].floating == 2 || _items[kItemFirebird].floating == 1) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
					KahinaCall(&LogicManager::CONS_Kahina_SearchTatiana, 0, 0, 0, 0);
				}
			}

			break;
		}

		if (getCharacterCurrentParams(kCharacterKahina)[2] >= _gameTime) {
			if (checkCathDir(kCarKronos, 80) || checkCathDir(kCarKronos, 88) || !getCharacterCurrentParams(kCharacterKahina)[5]) {
				getCharacterCurrentParams(kCharacterKahina)[5] = _gameTime;
				if (!_gameTime) {
					playDialog(0, "LIB014", getVolume(kCharacterKahina), 0);
					playDialog(0, "LIB015", getVolume(kCharacterKahina), 15);
					startCycOtis(kCharacterKahina, "202a");
					getCharacterCurrentParams(kCharacterKahina)[1] = 0;
					if (!_globals[kGlobalKahinaKillTimeoutActive] && _gameTime > 2214000) {
						if (_items[kItemFirebird].floating == 3 || _items[kItemFirebird].floating == 7) {
							getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
							KahinaCall(&LogicManager::CONS_Kahina_SearchCath, 0, 0, 0, 0);
						} else if (_items[kItemFirebird].floating == 2 || _items[kItemFirebird].floating == 1) {
							getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
							KahinaCall(&LogicManager::CONS_Kahina_SearchTatiana, 0, 0, 0, 0);
						}
					}

					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterKahina)[5] >= _gameTime) {
				if (!_globals[kGlobalKahinaKillTimeoutActive] && _gameTime > 2214000) {
					if (_items[kItemFirebird].floating == 3 || _items[kItemFirebird].floating == 7) {
						getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
						KahinaCall(&LogicManager::CONS_Kahina_SearchCath, 0, 0, 0, 0);
					} else if (_items[kItemFirebird].floating == 2 || _items[kItemFirebird].floating == 1) {
						getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
						KahinaCall(&LogicManager::CONS_Kahina_SearchTatiana, 0, 0, 0, 0);
					}
				}

				break;
			}
		}

		getCharacterCurrentParams(kCharacterKahina)[5] = 0x7FFFFFFF;
		playDialog(0, "LIB014", getVolume(kCharacterKahina), 0);
		playDialog(0, "LIB015", getVolume(kCharacterKahina), 15);
		startCycOtis(kCharacterKahina, "202a");
		getCharacterCurrentParams(kCharacterKahina)[1] = 0;

		if (!_globals[kGlobalKahinaKillTimeoutActive] && _gameTime > 2214000) {
			if (_items[kItemFirebird].floating == 3 || _items[kItemFirebird].floating == 7) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
				KahinaCall(&LogicManager::CONS_Kahina_SearchCath, 0, 0, 0, 0);
			} else if (_items[kItemFirebird].floating == 2 || _items[kItemFirebird].floating == 1) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
				KahinaCall(&LogicManager::CONS_Kahina_SearchTatiana, 0, 0, 0, 0);
			}
		}

		break;
	case 12:
		getCharacter(kCharacterKahina).characterPosition.car = kCarKronos;
		getCharacter(kCharacterKahina).characterPosition.position = 5000;
		getCharacter(kCharacterKahina).characterPosition.location = 0;
		startCycOtis(kCharacterKahina, "202a");
		break;
	case 18:
		if (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] == 1) {
			getCharacterCurrentParams(kCharacterKahina)[0] = 0;
			getCharacterCurrentParams(kCharacterKahina)[1] = 1;
			break;
		}

		if (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] == 2) {
			getCharacterCurrentParams(kCharacterKahina)[0] = 0;
			getCharacterCurrentParams(kCharacterKahina)[1] = 1;
			if (!getCharacterCurrentParams(kCharacterKahina)[1]) {
				if (!_globals[kGlobalKahinaKillTimeoutActive] && _gameTime > 2214000) {
					if (_items[kItemFirebird].floating == 3 || _items[kItemFirebird].floating == 7) {
						getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
						KahinaCall(&LogicManager::CONS_Kahina_SearchCath, 0, 0, 0, 0);
					} else if (_items[kItemFirebird].floating == 2 || _items[kItemFirebird].floating == 1) {
						getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
						KahinaCall(&LogicManager::CONS_Kahina_SearchTatiana, 0, 0, 0, 0);
					}
				}

				break;
			}

			if (!getCharacterCurrentParams(kCharacterKahina)[3])
				getCharacterCurrentParams(kCharacterKahina)[3] = _gameTime + 4500;

			if (getCharacterCurrentParams(kCharacterKahina)[5] == 0x7FFFFFFF || !_gameTime) {
				if (!_globals[kGlobalKahinaKillTimeoutActive] && _gameTime > 2214000) {
					if (_items[kItemFirebird].floating == 3 || _items[kItemFirebird].floating == 7) {
						getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
						KahinaCall(&LogicManager::CONS_Kahina_SearchCath, 0, 0, 0, 0);
					} else if (_items[kItemFirebird].floating == 2 || _items[kItemFirebird].floating == 1) {
						getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
						KahinaCall(&LogicManager::CONS_Kahina_SearchTatiana, 0, 0, 0, 0);
					}
				}

				break;
			}

			if (getCharacterCurrentParams(kCharacterKahina)[2] >= _gameTime) {
				if (checkCathDir(kCarKronos, 80) || checkCathDir(kCarKronos, 88) || !getCharacterCurrentParams(kCharacterKahina)[5]) {
					getCharacterCurrentParams(kCharacterKahina)[5] = _gameTime;
					if (!_gameTime) {
						playDialog(0, "LIB014", getVolume(kCharacterKahina), 0);
						playDialog(0, "LIB015", getVolume(kCharacterKahina), 15);
						startCycOtis(kCharacterKahina, "202a");
						getCharacterCurrentParams(kCharacterKahina)[1] = 0;
						if (!_globals[kGlobalKahinaKillTimeoutActive] && _gameTime > 2214000) {
							if (_items[kItemFirebird].floating == 3 || _items[kItemFirebird].floating == 7) {
								getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
								KahinaCall(&LogicManager::CONS_Kahina_SearchCath, 0, 0, 0, 0);
							} else if (_items[kItemFirebird].floating == 2 || _items[kItemFirebird].floating == 1) {
								getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
								KahinaCall(&LogicManager::CONS_Kahina_SearchTatiana, 0, 0, 0, 0);
							}
						}

						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterKahina)[5] >= _gameTime) {
					if (!_globals[kGlobalKahinaKillTimeoutActive] && _gameTime > 2214000) {
						if (_items[kItemFirebird].floating == 3 || _items[kItemFirebird].floating == 7) {
							getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
							KahinaCall(&LogicManager::CONS_Kahina_SearchCath, 0, 0, 0, 0);
						} else if (_items[kItemFirebird].floating == 2 || _items[kItemFirebird].floating == 1) {
							getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
							KahinaCall(&LogicManager::CONS_Kahina_SearchTatiana, 0, 0, 0, 0);
						}
					}

					break;
				}
			}

			getCharacterCurrentParams(kCharacterKahina)[5] = 0x7FFFFFFF;
			playDialog(0, "LIB014", getVolume(kCharacterKahina), 0);
			playDialog(0, "LIB015", getVolume(kCharacterKahina), 15);
			startCycOtis(kCharacterKahina, "202a");
			getCharacterCurrentParams(kCharacterKahina)[1] = 0;

			if (!_globals[kGlobalKahinaKillTimeoutActive] && _gameTime > 2214000) {
				if (_items[kItemFirebird].floating == 3 || _items[kItemFirebird].floating == 7) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
					KahinaCall(&LogicManager::CONS_Kahina_SearchCath, 0, 0, 0, 0);
				} else if (_items[kItemFirebird].floating == 2 || _items[kItemFirebird].floating == 1) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
					KahinaCall(&LogicManager::CONS_Kahina_SearchTatiana, 0, 0, 0, 0);
				}
			}

			break;
		}

		break;
	case 92186062:
		if (getCharacterCurrentParams(kCharacterKahina)[0]) {
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
			KahinaCall(&LogicManager::CONS_Kahina_FindFirebird, 0, 0, 0, 0);
		}

		break;
	case 134611040:
		if (_doneNIS[kEventConcertLeaveWithBriefcase])
			CONS_Kahina_SeekCath(0, 0, 0, 0);

		break;
	case 137503360:
		CONS_Kahina_Finished(0, 0, 0, 0);
		break;
	case 237555748:
		getCharacterCurrentParams(kCharacterKahina)[0] = 1;
		break;

	default:
		break;
	}
}

void LogicManager::CONS_Kahina_Finished(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_Finished);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 22;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_Finished(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterKahina)[0]) {
			if (getCharacterParams(kCharacterKahina, 8)[2] || (_items[kItemFirebird].floating == 3) || _items[kItemFirebird].floating == 7) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
				KahinaCall(&LogicManager::CONS_Kahina_SearchCath, 0, 0, 0, 0);
			} else if (_items[kItemFirebird].floating == 2 || _items[kItemFirebird].floating == 1) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
				KahinaCall(&LogicManager::CONS_Kahina_SearchTatiana, 0, 0, 0, 0);
			}
		}

		break;
	case 12:
		getCharacter(kCharacterKahina).characterPosition.car = kCarKronos;
		getCharacter(kCharacterKahina).characterPosition.position = 5000;
		getCharacter(kCharacterKahina).characterPosition.location = 0;
		break;
	case 17:
		if (getCharacter(kCharacterCath).characterPosition.car > kCarGreenSleeping || (getCharacter(kCharacterCath).characterPosition.car == kCarGreenSleeping && getCharacter(kCharacterCath).characterPosition.position > 2740)) {
			getCharacterCurrentParams(kCharacterKahina)[0] = 1;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_FindFirebird(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_FindFirebird);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 23;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_FindFirebird(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		playDialog(0, "LIB014", getVolume(kCharacterKahina), 0);
		playDialog(0, "LIB015", getVolume(kCharacterKahina), 15);
		endGraphics(kCharacterKahina);
		getCharacter(kCharacterKahina).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterKahina).characterPosition.position = 540;
		getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
		KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 4, 4070, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8]) {
		case 1:
			if (inSuite(kCharacterCath, kCarRedSleeping, 4455) || cathOutRebeccaWindow()) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 5;
				KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 4, 9460, 0, 0);
			} else {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
				KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Cf", 37, 0, 0);
			}

			break;
		case 2:
			getCharacter(kCharacterKahina).characterPosition.location = 1;
			endGraphics(kCharacterKahina);

			setDoor(37, kCharacterCath, checkDoor(37), 0, 0);
			setDoor(53, kCharacterCath, checkDoor(53), 0, 0);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 900, 0, 0, 0);
			break;
		case 3:
			setDoor(37, kCharacterCath, checkDoor(37), 10, 9);
			setDoor(53, kCharacterCath, checkDoor(53), 10, 9);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
			KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Df", 37, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterKahina).characterPosition.location = 0;
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 5;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 4, 9460, 0, 0);
			break;
		case 5:
			endGraphics(kCharacterKahina);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 6;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 900, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 7;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 2, 9270, 0, 0);
			break;
		case 7:
			endGraphics(kCharacterKahina);

			getCharacter(kCharacterKahina).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
			fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_SeekCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_SeekCath);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 24;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_SeekCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterKahina)[0]) {
			if (walk(kCharacterKahina, getCharacterCurrentParams(kCharacterKahina)[1], getCharacterCurrentParams(kCharacterKahina)[2])) {
				endGraphics(kCharacterKahina);
				getCharacterCurrentParams(kCharacterKahina)[0] = 0;
			}
		}

		break;
	case 2:
		if (checkLoc(kCharacterCath, kCarKronos)) {
			send(kCharacterKahina, kCharacterKronos, 9, 0);
		} else {
			CONS_Kahina_KillCathAnywhere(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
		KahinaCall(&LogicManager::CONS_Kahina_LookingForCath, 2241000, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] != 1) {
			if (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] == 2) {
				if (getCharacter(kCharacterCath).characterPosition.position >= getCharacter(kCharacterKahina).characterPosition.position) {
					playNIS((kEventKahinaGunBlue - (getCharacter(kCharacterKahina).characterPosition.car == kCarGreenSleeping)));
				} else {
					playNIS(kEventKahinaGun);
				}

				walk(kCharacterKahina, kCarKronos, 9270);
				bumpCathRx(getCharacter(kCharacterKahina).characterPosition.car, getCharacter(kCharacterKahina).characterPosition.position + 750);
				send(kCharacterKahina, kCharacterKronos, 235599361, 0);
				playDialog(kCharacterKahina, "MUS016", 16, 0);
				_globals[kGlobalKahinaKillTimeoutActive] = 1;
				getCharacterCurrentParams(kCharacterKahina)[0] = 1;
				getCharacterCurrentParams(kCharacterKahina)[1] = 2;
				getCharacterCurrentParams(kCharacterKahina)[2] = 9270;
			}

			break;
		}

		if (!getCharacterParams(kCharacterKahina, 8)[1]) {
			if (getCharacterParams(kCharacterKahina, 8)[0]) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
				KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKahinaGunYellow, 0, 0);
			} else {
				CONS_Kahina_KillCathAnywhere(0, 0, 0, 0);
			}

			break;
		}

		endGraphics(kCharacterKahina);

		if (whoRunningDialog(kCharacterKahina))
			fadeDialog(kCharacterKahina);

		_globals[kGlobalKahinaKillTimeoutActive] = 0;
		CONS_Kahina_Finished(0, 0, 0, 0);
		break;
	case 137503360:
		endGraphics(kCharacterKahina);

		if (whoRunningDialog(kCharacterKahina))
			fadeDialog(kCharacterKahina);

		_globals[kGlobalKahinaKillTimeoutActive] = 0;
		CONS_Kahina_Finished(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_SearchCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_SearchCath);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 25;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_SearchCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterKahina)[0] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime <= 2263500) {
				if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterKahina)[0]) {
					getCharacterCurrentParams(kCharacterKahina)[0] = _gameTime;
					if (!_gameTime) {
						getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 12;
						KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Ba", 1, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterKahina)[0] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterKahina)[0] = 0x7FFFFFFF;
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 12;
			KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Ba", 1, 0, 0);
		}

		break;
	case 12:
		if (!_doneNIS[kEventAnnaBaggageArgument]) {
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk1033, 3, 8200, 0, 0);
			break;
		}

		if (_items[kItemFirebird].floating == 3) {
			_globals[kGlobalCanPlayEggSuitcaseMusic] = 1;
		} else {
			if (_items[kItemFirebird].floating != 7) {
				dropItem(kItemBriefcase, 2);
				_globals[kGlobalCanPlayKronosSuitcaseLeftInCompMusic] = 1;
				getCharacterParams(kCharacterKahina, 8)[2] = 0;

				getCharacter(kCharacterKahina).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
				fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
				break;
			}

			_globals[kGlobalCanPlayEggUnderSinkMusic] = 1;
		}

		takeItem(kItemFirebird);
		_items[kItemFirebird].floating = 5;
		send(kCharacterKahina, kCharacterKronos, 138085344, 0);
		dropItem(kItemBriefcase, 2);
		_globals[kGlobalCanPlayKronosSuitcaseLeftInCompMusic] = 1;
		getCharacterParams(kCharacterKahina, 8)[2] = 0;

		getCharacter(kCharacterKahina).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
		fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8]) {
		case 1:
			if (!cathInCorridor(kCarGreenSleeping)) {
				if (inSuite(kCharacterCath, kCarGreenSleeping, 7850) || cathOutHisWindow()) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 6;
					KahinaCall(&LogicManager::CONS_Kahina_DoDialog, "LIB013", 0, 0, 0);
				} else {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 8;
					KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Aa", 1, 0, 0);
				}
			} else {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
				KahinaCall(&LogicManager::CONS_Kahina_DoWalk1033, 3, 9460, 0, 0);
			}

			break;
		case 2:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 1800, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk1033, 3, 8200, 0, 0);
			break;
		case 4:
			if (cathInCorridor(kCarGreenSleeping)) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 5;
				KahinaCall(&LogicManager::CONS_Kahina_DoWalk1033, 2, 9270, 0, 0);
			} else {
				if (inSuite(kCharacterCath, kCarGreenSleeping, 7850) || cathOutHisWindow()) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 6;
					KahinaCall(&LogicManager::CONS_Kahina_DoDialog, "LIB013", 0, 0, 0);
				} else {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 8;
					KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Aa", 1, 0, 0);
				}
			}

			break;
		case 5:
		case 7:
		case 11:
		case 13:
			endGraphics(kCharacterKahina);

			getCharacter(kCharacterKahina).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
			fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
			break;
		case 6:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 7;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk1033, 2, 9270, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterKahina).characterPosition.location = 1;
			endGraphics(kCharacterKahina);
			setDoor(1, kCharacterCath, 0, 0, 0);
			setDoor(17, kCharacterCath, 0, 0, 0);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 9;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 900, 0, 0, 0);
			break;
		case 9:
			setDoor(1, kCharacterCath, 0, 10, 9);
			setDoor(17, kCharacterCath, 0, 10, 9);

			if (_items[kItemFirebird].floating == 3) {
				_globals[kGlobalCanPlayEggSuitcaseMusic] = 1;
				takeItem(kItemFirebird);
				_items[kItemFirebird].floating = 5;
				send(kCharacterKahina, kCharacterKronos, 138085344, 0);
				dropItem(kItemBriefcase, 2);
				_globals[kGlobalRegisteredTimeAtWhichCathGaveFirebirdToKronos] = _gameTime;
				_globals[kGlobalCanPlayKronosSuitcaseLeftInCompMusic] = 1;
				getCharacterParams(kCharacterKahina, 8)[2] = 0;

				if (_items[kItemFirebird].floating != 18) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 10;
					KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Ba", 1, 0, 0);
				}

				break;
			}

			if (_items[kItemFirebird].floating == 7) {
				_globals[kGlobalCanPlayEggUnderSinkMusic] = 1;
				takeItem(kItemFirebird);
				_items[kItemFirebird].floating = 5;
				send(kCharacterKahina, kCharacterKronos, 138085344, 0);
				dropItem(kItemBriefcase, 2);
				_globals[kGlobalRegisteredTimeAtWhichCathGaveFirebirdToKronos] = _gameTime;
				_globals[kGlobalCanPlayKronosSuitcaseLeftInCompMusic] = 1;
				getCharacterParams(kCharacterKahina, 8)[2] = 0;
	
				if (_items[kItemFirebird].floating != 18) {
					getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 10;
					KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Ba", 1, 0, 0);
				}

				break;
			}

			if (getCharacterParams(kCharacterKahina, 8)[2]) {
				dropItem(kItemBriefcase, 2);
				_globals[kGlobalCanPlayKronosSuitcaseLeftInCompMusic] = 1;
				getCharacterParams(kCharacterKahina, 8)[2] = 0;
			}

			if (_items[kItemFirebird].floating != 18) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 10;
				KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Ba", 1, 0, 0);
			}
			break;
		case 10:
			getCharacter(kCharacterKahina).characterPosition.location = 0;
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 11;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 2, 9270, 0, 0);
			break;
		case 12:
			getCharacter(kCharacterKahina).characterPosition.location = 0;
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 13;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 2, 9270, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_SearchTatiana(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_SearchTatiana);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 26;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_SearchTatiana(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (_doneNIS[kEventAnnaBaggageArgument]) {
			takeItem(kItemFirebird);
			_items[kItemFirebird].floating = 5;
			send(kCharacterKahina, kCharacterKronos, 138085344, 0);
			dropItem(kItemBriefcase, 2);
			_globals[kGlobalCanPlayKronosSuitcaseLeftInCompMusic] = 1;

			getCharacter(kCharacterKahina).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
			fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
		} else {
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk1033, 4, 8200, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8]) {
		case 1:
			if (!inSuite(kCharacterCath, kCarRedSleeping, 7850)) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 6;
				KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Aa", 32, 0, 0);
			} else {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 2;
				KahinaCall(&LogicManager::CONS_Kahina_DoWalk1033, 4, 9460, 0, 0);
			}

			break;
		case 2:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 3;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 1800, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 4;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk1033, 4, 8200, 0, 0);
			break;
		case 4:
			if (inSuite(kCharacterCath, kCarRedSleeping, 7850)) {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 5;
				KahinaCall(&LogicManager::CONS_Kahina_DoWalk1033, 2, 9270, 0, 0);
			} else {
				getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 6;
				KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Aa", 32, 0, 0);
			}

			break;
		case 5:
		case 9:
			endGraphics(kCharacterKahina);

			getCharacter(kCharacterKahina).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterKahina, _functionsKahina[getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall]]);
			fedEx(kCharacterKahina, kCharacterKahina, 18, 0);
			break;
		case 6:
			getCharacter(kCharacterKahina).characterPosition.location = 1;
			endGraphics(kCharacterKahina);

			setDoor(32, kCharacterCath, checkDoor(32), 0, 0);
			setDoor(48, kCharacterCath, checkDoor(48), 0, 0);
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 7;
			KahinaCall(&LogicManager::CONS_Kahina_DoWait, 900, 0, 0, 0);
			break;
		case 7:
			setDoor(32, kCharacterCath, checkDoor(32), 10, 9);
			setDoor(48, kCharacterCath, checkDoor(48), 10, 9);

			if (_items[kItemFirebird].floating == 1 || _items[kItemFirebird].floating == 2) {
				takeItem(kItemFirebird);
				_items[kItemFirebird].floating = 5;
				send(kCharacterKahina, kCharacterKronos, 138085344, 0);
				getCharacterParams(kCharacterKahina, 8)[2] = 1;
			}

			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 8;
			KahinaCall(&LogicManager::CONS_Kahina_DoCorrOtis, "616Ba", 32, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterKahina).characterPosition.location = 0;
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 9;
			KahinaCall(&LogicManager::CONS_Kahina_DoWalk, 2, 9270, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_KillCathAnywhere(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_KillCathAnywhere);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 27;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_KillCathAnywhere(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (onLowPlatform(kCharacterCath)) {
			getCharacterCurrentParams(kCharacterKahina)[0] = kEventKahinaPunchCar;
		} else if (cathInCorridor(kCarGreenSleeping)) {
			getCharacterCurrentParams(kCharacterKahina)[0] = kEventKahinaPunchBlue;
		} else if (cathInCorridor(kCarRedSleeping)) {
			getCharacterCurrentParams(kCharacterKahina)[0] = kEventKahinaPunchYellow;
		} else if (inSalon(kCharacterCath)) {
			getCharacterCurrentParams(kCharacterKahina)[0] = kEventKahinaPunchSalon;
		} else if (inDiningRoom(kCharacterCath)) {
			getCharacterCurrentParams(kCharacterKahina)[0] = kEventKahinaPunchRestaurant;
		} else if (inKitchen(kCharacterCath)) {
			getCharacterCurrentParams(kCharacterKahina)[0] = kEventKahinaPunchKitchen;
		} else if (inOffice(kCharacterCath)) {
			getCharacterCurrentParams(kCharacterKahina)[0] = kEventKahinaPunchBaggageCarEntrance;
		} else if (checkLoc(kCharacterCath, kCarBaggage)) {
			getCharacterCurrentParams(kCharacterKahina)[0] = kEventKahinaPunchBaggageCar;
		}

		if (getCharacterCurrentParams(kCharacterKahina)[0]) {
			getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] = 1;
			KahinaCall(&LogicManager::CONS_Kahina_SaveGame, 2, kEventKahinaPunchBaggageCarEntrance, 0, 0);
		}

		break;
	case 12:
		_timeSpeed = 0;
		break;
	case 18:
		if (getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall + 8] == 1) {
			playNIS(getCharacterCurrentParams(kCharacterKahina)[0]);
			endGame(0, 1, 0, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_StartPart4);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 28;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterKahina);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Kahina_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterKahina).callParams[getCharacter(kCharacterKahina).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterKahina, &LogicManager::HAND_Kahina_StartPart5);
	getCharacter(kCharacterKahina).callbacks[getCharacter(kCharacterKahina).currentCall] = 29;

	params->clear();

	fedEx(kCharacterKahina, kCharacterKahina, 12, 0);
}

void LogicManager::HAND_Kahina_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterKahina);
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsKahina[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Kahina_DebugWalks,
	&LogicManager::HAND_Kahina_DoDialog,
	&LogicManager::HAND_Kahina_SaveGame,
	&LogicManager::HAND_Kahina_DoWait,
	&LogicManager::HAND_Kahina_DoWaitReal,
	&LogicManager::HAND_Kahina_LookingForCath,
	&LogicManager::HAND_Kahina_DoWalkSearchingForCath,
	&LogicManager::HAND_Kahina_DoWalk,
	&LogicManager::HAND_Kahina_DoCorrOtis,
	&LogicManager::HAND_Kahina_Birth,
	&LogicManager::HAND_Kahina_InSeclusion,
	&LogicManager::HAND_Kahina_AwaitingCath,
	&LogicManager::HAND_Kahina_CathDone,
	&LogicManager::HAND_Kahina_DoDogBarkOtis,
	&LogicManager::HAND_Kahina_SearchTrain,
	&LogicManager::HAND_Kahina_StartPart2,
	&LogicManager::HAND_Kahina_InSeclusionPart2,
	&LogicManager::HAND_Kahina_StartPart3,
	&LogicManager::HAND_Kahina_DoWalk1033,
	&LogicManager::HAND_Kahina_BeforeConcert,
	&LogicManager::HAND_Kahina_Concert,
	&LogicManager::HAND_Kahina_Finished,
	&LogicManager::HAND_Kahina_FindFirebird,
	&LogicManager::HAND_Kahina_SeekCath,
	&LogicManager::HAND_Kahina_SearchCath,
	&LogicManager::HAND_Kahina_SearchTatiana,
	&LogicManager::HAND_Kahina_KillCathAnywhere,
	&LogicManager::HAND_Kahina_StartPart4,
	&LogicManager::HAND_Kahina_StartPart5
};

} // End of namespace LastExpress
