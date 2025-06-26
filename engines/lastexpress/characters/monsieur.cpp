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

void LogicManager::CONS_Monsieur(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMonsieur,
			_functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]
		);

		break;
	case 1:
		CONS_Monsieur_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Monsieur_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Monsieur_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Monsieur_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Monsieur_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::MonsieurCall(CALL_PARAMS) {
	getCharacter(kCharacterMonsieur).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Monsieur_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_DebugWalks);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 1;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterMonsieur, kCarGreenSleeping, getCharacterCurrentParams(kCharacterMonsieur)[0])) {
			if (getCharacterCurrentParams(kCharacterMonsieur)[0] == 10000) {
				getCharacterCurrentParams(kCharacterMonsieur)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterMonsieur)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterMonsieur).characterPosition.position = 0;
		getCharacter(kCharacterMonsieur).characterPosition.location = 0;
		getCharacter(kCharacterMonsieur).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterMonsieur)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_DoDialog);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterMonsieur).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
		fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		break;
	case 12:
		playDialog(kCharacterMonsieur, (char *)&getCharacterCurrentParams(kCharacterMonsieur)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_DoSeqOtis);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterMonsieur).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
		fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMonsieur, (char *)&getCharacterCurrentParams(kCharacterMonsieur)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_DoWait);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 4;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMonsieur)[1] || (getCharacterCurrentParams(kCharacterMonsieur)[1] = _gameTime + getCharacterCurrentParams(kCharacterMonsieur)[0], _gameTime + getCharacterCurrentParams(kCharacterMonsieur)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterMonsieur)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterMonsieur)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterMonsieur).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
		fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_DoBlockSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_DoBlockSeqOtis);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_DoBlockSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseView(kCharacterMonsieur, getCharacterCurrentParams(kCharacterMonsieur)[3], getCharacterCurrentParams(kCharacterMonsieur)[4]);

		getCharacter(kCharacterMonsieur).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
		fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMonsieur, (char *)&getCharacterCurrentParams(kCharacterMonsieur)[0]);
		blockView(kCharacterMonsieur, getCharacterCurrentParams(kCharacterMonsieur)[3], getCharacterCurrentParams(kCharacterMonsieur)[4]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_DoCorrOtis);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterMonsieur, getCharacterCurrentParams(kCharacterMonsieur)[3]);

		getCharacter(kCharacterMonsieur).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
		fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMonsieur, (char *)&getCharacterCurrentParams(kCharacterMonsieur)[0]);
		blockAtDoor(kCharacterMonsieur, getCharacterCurrentParams(kCharacterMonsieur)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_DoEnterCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_DoEnterCorrOtis);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 7;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_DoEnterCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterMonsieur, getCharacterCurrentParams(kCharacterMonsieur)[3]);
		getCharacter(kCharacterMonsieur).characterPosition.position = 6470;

		getCharacter(kCharacterMonsieur).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
		fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMonsieur, (char *)&getCharacterCurrentParams(kCharacterMonsieur)[0]);
		blockAtDoor(kCharacterMonsieur, getCharacterCurrentParams(kCharacterMonsieur)[3]);
		getCharacter(kCharacterMonsieur).characterPosition.location = 1;
		if (inComp(kCharacterCath, kCarRedSleeping, 6470) || inComp(kCharacterCath, kCarRedSleeping, 6130)) {
			playNIS((kEventCathTurningNight - !isNight()));
			playDialog(0, "BUMP", -1, 0);
			bumpCathFDoor(34);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_FinishSeqOtis);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 8;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterMonsieur).direction == 4) {
			getCharacter(kCharacterMonsieur).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
			fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterMonsieur).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
		fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_WaitRCClear);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 9;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterMonsieur).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
			fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_DoWalk);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterMonsieur, getCharacterCurrentParams(kCharacterMonsieur)[0], getCharacterCurrentParams(kCharacterMonsieur)[1])) {
			getCharacter(kCharacterMonsieur).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
			fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		}

		break;
	case 5:
		if (cathHasItem(kItemPassengerList) && _gameTime > 1089000) {
			playDialog(kCharacterCath, "CAT1022", -1, 0);
		} else {
			playCathExcuseMe();
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterMonsieur, kCharacterCath, 0);
		break;
	
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_GoDining(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_GoDining);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 11;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_GoDining(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterParams(kCharacterMonsieur, 8)[0] && getCharacterParams(kCharacterMonsieur, 8)[1]) {
			getCharacterParams(kCharacterMonsieur, 8)[1] = 0;
			getCharacterParams(kCharacterMonsieur, 8)[0] = 0;
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 5;
			MonsieurCall(&LogicManager::CONS_Monsieur_WaitRCClear, 0, 0, 0, 0);
		}

		break;
	case 12:
		if (getCharacterCurrentParams(kCharacterMonsieur)[0]) {
			if (_globals[kGlobalChapter] == 4) {
				setDoor(34, kCharacterCath, 0, 10, 9);
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
				MonsieurCall(&LogicManager::CONS_Monsieur_DoCorrOtis, "607Hc", 34, 0, 0);
			} else {
				setDoor(34, kCharacterCath, 1, 255, 255);
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 2;
				MonsieurCall(&LogicManager::CONS_Monsieur_DoCorrOtis, "607Dc", 34, 0, 0);
			}
		} else {
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 3;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoCorrOtis, "607Bc", 34, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8]) {
		case 1:
		case 2:
		case 3:
			if (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] == 2) {
				setDoor(34, kCharacterCath, 2, 255, 255);
			} else if (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] == 3) {
				setDoor(34, kCharacterCath, 0, 10, 9);
			}

			setDoor(50, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterMonsieur).characterPosition.location = 0;
			send(kCharacterMonsieur, kCharacterFrancois, 101107728, 0);
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 4;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoWalk, 5, 0x352, 0, 0);
			break;
		case 4:
			endGraphics(kCharacterMonsieur);
			break;
		case 5:
			getCharacter(kCharacterMonsieur).characterPosition.position = 1540;
			getCharacter(kCharacterMonsieur).characterPosition.location = 0;
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 6;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoSeqOtis, "812US", 0, 0, 0);
			break;
		case 6:
			if (_globals[kGlobalChapter] == 1) {
				playDialog(kCharacterMonsieur, "MRB1075", -1, 60);
			} else if (_globals[kGlobalChapter] == 3) {
				playDialog(kCharacterMonsieur, "MRB3101", -1, 0);
			}

			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 7;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoOtis5008A, 0, 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterMonsieur).characterPosition.location = 1;

			getCharacter(kCharacterMonsieur).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
			fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_DoOtis5008A(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_DoOtis5008A);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 12;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_DoOtis5008A(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		endGraphics(kCharacterMadame);
		send(kCharacterMonsieur, kCharacterTableC, 136455232, 0);
		getCharacter(kCharacterMonsieur).characterPosition.location = 1;

		getCharacter(kCharacterMonsieur).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
		fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterTableC, "008A3");
		startSeqOtis(kCharacterMadame, "008A2");
		startSeqOtis(kCharacterMonsieur, "008A1");
		if (inSalon(kCharacterCath)) {
			advanceFrame(kCharacterMonsieur);
			getCharacter(kCharacterMadame).elapsedFrames = getCharacter(kCharacterMonsieur).elapsedFrames;
			getCharacter(kCharacterTableC).elapsedFrames = getCharacter(kCharacterMonsieur).elapsedFrames;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_DoOtis5008E(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_DoOtis5008E);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 13;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_DoOtis5008E(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		send(kCharacterMonsieur, kCharacterTableC, 103798704, "008F");
		endGraphics(kCharacterMadame);

		getCharacter(kCharacterMonsieur).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
		fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterTableC, "008E3");
		startSeqOtis(kCharacterMadame, "008E2");
		startSeqOtis(kCharacterMonsieur, "008E1");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_ExitDining(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_ExitDining);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 14;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_ExitDining(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
		MonsieurCall(&LogicManager::CONS_Monsieur_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMonsieur).characterPosition.location = 0;
			playDialog(kCharacterMonsieur, "MRB1079", -1, 0);
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 2;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoOtis5008E, 0, 0, 0, 0);
			break;
		case 2:
			send(kCharacterMonsieur, kCharacterWaiter2, 326144276, 0);
			startSeqOtis(kCharacterMonsieur, "812DS");
			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterMonsieur);
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 3;
			MonsieurCall(&LogicManager::CONS_Monsieur_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterMonsieur).characterPosition.position = 540;
			getCharacter(kCharacterFrancois).characterPosition.position = 540;
			getCharacter(kCharacterMadame).characterPosition.position = getCharacter(kCharacterFrancois).characterPosition.position;
			getCharacter(kCharacterFrancois).characterPosition.location = 0;
			getCharacter(kCharacterMadame).characterPosition.location = getCharacter(kCharacterFrancois).characterPosition.location;
			endGraphics(kCharacterMonsieur);
			send(kCharacterMonsieur, kCharacterMadame, 100901266, 0);
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 4;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoWait, 450, 0, 0, 0);
			break;
		case 4:
			send(kCharacterMonsieur, kCharacterFrancois, 100901266, 0);
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 5;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoWait, 450, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 6;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoWalk, 4, 6470, 0, 0);
			break;
		case 6:
			if (getCharacterCurrentParams(kCharacterMonsieur)[0]) {
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 7;
				MonsieurCall(&LogicManager::CONS_Monsieur_DoEnterCorrOtis, "607Gc", 34, 0, 0);
			} else {
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 8;
				MonsieurCall(&LogicManager::CONS_Monsieur_DoEnterCorrOtis, "607Ac", 34, 0, 0);
			}

			break;
		case 7:
		case 8:
			endGraphics(kCharacterMonsieur);
			getCharacter(kCharacterMonsieur).characterPosition.location = 1;

			getCharacter(kCharacterMonsieur).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
			fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_GoSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_GoSalon);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 15;

	params->clear();

	params->parameters[0] = param1.intParam;

	strncpy((char *)&params->parameters[1], param2.stringParam, 12);

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_GoSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterCurrentParams(kCharacterMonsieur)[0]) {
			setDoor(34, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoCorrOtis, "607Dc", 34, 0, 0);
		} else {
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 2;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoCorrOtis, "607Bc", 34, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8]) {
		case 1:
			setDoor(34, kCharacterCath, 2, 255, 255);
			setDoor(50, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 3;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			setDoor(34, kCharacterCath, 0, 10, 9);
			setDoor(50, kCharacterCath, 0, 10, 9);
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 3;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoWalk, 5, 850, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 4;
			MonsieurCall(&LogicManager::CONS_Monsieur_WaitRCClear, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterMonsieur).characterPosition.position = 1540;
			getCharacter(kCharacterMonsieur).characterPosition.location = 0;
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 5;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoBlockSeqOtis, (char *)&getCharacterCurrentParams(kCharacterMonsieur)[1], 5, 52, 0);
			break;
		case 5:
			getCharacter(kCharacterMonsieur).characterPosition.location = 1;

			getCharacter(kCharacterMonsieur).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
			fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_ReturnSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_ReturnSalon);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 16;

	params->clear();

	params->parameters[0] = param1.intParam;

	strncpy((char *)&params->parameters[1], param2.stringParam, 12);

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_ReturnSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
		MonsieurCall(&LogicManager::CONS_Monsieur_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMonsieur).characterPosition.location = 0;
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 2;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoBlockSeqOtis, (char *)&getCharacterCurrentParams(kCharacterMonsieur)[1], 5, 52, 0);
			break;
		case 2:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 3;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoWalk, 4, 6470, 0, 0);
			break;
		case 3:
			if (getCharacterCurrentParams(kCharacterMonsieur)[0]) {
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 4;
				MonsieurCall(&LogicManager::CONS_Monsieur_DoEnterCorrOtis, "607Gc", 34, 0, 0);
			} else {
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 5;
				MonsieurCall(&LogicManager::CONS_Monsieur_DoEnterCorrOtis, "607Ac", 34, 0, 0);
			}
			break;
		case 4:
		case 5:
			getCharacter(kCharacterMonsieur).characterPosition.location = 1;
			endGraphics(kCharacterMonsieur);

			getCharacter(kCharacterMonsieur).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
			fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_InSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_InSalon);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 17;

	params->clear();

	params->parameters[0] = param1.intParam;

	strncpy((char *)&params->parameters[1], param2.stringParam, 12);

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_InSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMonsieur)[0] < _gameTime && !getCharacterCurrentParams(kCharacterMonsieur)[5]) {
			getCharacterCurrentParams(kCharacterMonsieur)[5] = 1;

			getCharacter(kCharacterMonsieur).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
			fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		} else {
			if (getCharacterCurrentParams(kCharacterMonsieur)[4]) {
				if (getCharacterCurrentParams(kCharacterMonsieur)[6] || (getCharacterCurrentParams(kCharacterMonsieur)[6] = _realTime + 90, _realTime != -90)) {
					if (getCharacterCurrentParams(kCharacterMonsieur)[6] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterMonsieur)[6] = 0x7FFFFFFF;
				}

				bumpCath(kCarRestaurant, 51, 255);
			} else {
				getCharacterCurrentParams(kCharacterMonsieur)[6] = 0;
			}
		}

		break;
	case 12:
		startCycOtis(kCharacterMonsieur, (char *)&getCharacterCurrentParams(kCharacterMonsieur)[1]);
		break;
	case 18:
		getCharacterCurrentParams(kCharacterMonsieur)[4] = checkCathDir(kCarRestaurant, 52);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_CompLogic(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_CompLogic);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 18;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_CompLogic(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMonsieur)[0] < _gameTime && !getCharacterCurrentParams(kCharacterMonsieur)[3]) {
			getCharacterCurrentParams(kCharacterMonsieur)[3] = 1;
			setDoor(34, kCharacterCath, 0, 10, 9);
			setDoor(50, kCharacterCath, 0, 10, 9);

			getCharacter(kCharacterMonsieur).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
			fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
		} else {
			if (getCharacterCurrentParams(kCharacterMonsieur)[1]) {
				if (getCharacterCurrentParams(kCharacterMonsieur)[4] || (getCharacterCurrentParams(kCharacterMonsieur)[4] = _realTime + 75, _realTime != -75)) {
					if (getCharacterCurrentParams(kCharacterMonsieur)[4] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterMonsieur)[4] = 0x7FFFFFFF;
				}

				getCharacterCurrentParams(kCharacterMonsieur)[1] = 0;
				getCharacterCurrentParams(kCharacterMonsieur)[2] = 1;
				setDoor(34, kCharacterMonsieur, 1, 0, 0);
				setDoor(50, kCharacterMonsieur, 1, 0, 0);
				getCharacterCurrentParams(kCharacterMonsieur)[4] = 0;
			} else {
				getCharacterCurrentParams(kCharacterMonsieur)[4] = 0;
			}
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterMonsieur)[1]) {
			setDoor(34, kCharacterMonsieur, 1, 0, 0);
			setDoor(50, kCharacterMonsieur, 1, 0, 0);

			if (msg->param.intParam == 50) {
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 4;
				MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, getCathWCDialog(), 0, 0, 0);
			} else if (cathHasItem(kItemPassengerList)) {
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 5;

				if (rnd(2) != 0) {
					MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, getCathSorryDialog(), 0, 0, 0);
				} else {
					MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "CAT1511", 0, 0, 0);
				}
			} else {
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 6;
				MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else {
			setDoor(34, kCharacterMonsieur, 1, 0, 0);
			setDoor(50, kCharacterMonsieur, 1, 0, 0);

			if (msg->action == 8) {
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
				MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 2;
				MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		setDoor(34, kCharacterMonsieur, 1, 10, 9);
		setDoor(50, kCharacterMonsieur, 1, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterMonsieur)[2] || getCharacterCurrentParams(kCharacterMonsieur)[1]) {
			setDoor(34, kCharacterMonsieur, 1, 10, 9);
			setDoor(50, kCharacterMonsieur, 1, 10, 9);
			getCharacterCurrentParams(kCharacterMonsieur)[2] = 0;
			getCharacterCurrentParams(kCharacterMonsieur)[1] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8]) {
		case 1:
		case 2:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 3;
			if (rnd(2) == 0) {
				MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB1001A", 0, 0, 0);
			} else {
				MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB1001", 0, 0, 0);
			}

			break;
		case 3:
			setDoor(34, kCharacterMonsieur, 1, 14, 0);
			setDoor(50, kCharacterMonsieur, 1, 14, 0);
			getCharacterCurrentParams(kCharacterMonsieur)[1] = 1;
			break;
		case 4:
		case 5:
		case 6:
			getCharacterCurrentParams(kCharacterMonsieur)[1] = 0;
			getCharacterCurrentParams(kCharacterMonsieur)[2] = 1;
			break;
		case 7:
			send(kCharacterMonsieur, kCharacterCond2, 123199584, 0);
			break;
		default:
			break;
		}

		break;
	case 122865568:
		send(kCharacterMonsieur, kCharacterCond2, 88652208, 0);
		break;
	case 221683008:
		getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 7;
		MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "Mrb1001", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_Birth);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 19;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterMonsieur)[0]) {
			getCharacterCurrentParams(kCharacterMonsieur)[0] = 1;
			CONS_Monsieur_InPart1(0, 0, 0, 0);
		}

		break;
	case 12:
		autoMessage(kCharacterMonsieur, 203520448, 0);
		autoMessage(kCharacterMonsieur, 237889408, 1);
		setDoor(34, kCharacterCath, 0, 10, 9);
		setDoor(50, kCharacterCath, 0, 10, 9);
		setDoor(42, kCharacterCath, 0, 255, 255);
		getCharacter(kCharacterMonsieur).characterPosition.position = 1750;
		getCharacter(kCharacterMonsieur).characterPosition.location = 1;
		getCharacter(kCharacterMonsieur).characterPosition.car = kCarRestaurant;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_DoDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_DoDinner);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 20;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_DoDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMonsieur)[0]) {
			if (!getCharacterCurrentParams(kCharacterMonsieur)[1]) {
				if (!getCharacterCurrentParams(kCharacterMonsieur)[2]) {
					getCharacterCurrentParams(kCharacterMonsieur)[2] = _gameTime + 4500;
					if (_gameTime == -4500) {
						getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 3;
						MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB1078A", 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterMonsieur)[2] < _gameTime) {
					getCharacterCurrentParams(kCharacterMonsieur)[2] = 0x7FFFFFFF;
					getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 3;
					MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB1078A", 0, 0, 0);
					break;
				}
			}

			if (_gameTime > 1138500 && !getCharacterCurrentParams(kCharacterMonsieur)[3]) {
				getCharacterCurrentParams(kCharacterMonsieur)[3] = 1;
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 4;
				MonsieurCall(&LogicManager::CONS_Monsieur_ExitDining, 0, 0, 0, 0);
			}
		}

		break;
	case 12:
		getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
		MonsieurCall(&LogicManager::CONS_Monsieur_GoDining, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterMonsieur, "008B");
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 2;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB1076", 0, 0, 0);
			break;
		case 2:
			send(kCharacterMonsieur, kCharacterWaiter2, 256200848, 0);
			break;
		case 3:
			if (_gameTime > 1138500 && !getCharacterCurrentParams(kCharacterMonsieur)[3]) {
				getCharacterCurrentParams(kCharacterMonsieur)[3] = 1;
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 4;
				MonsieurCall(&LogicManager::CONS_Monsieur_ExitDining, 0, 0, 0, 0);
			}

			break;
		case 4:
			send(kCharacterMonsieur, kCharacterCook, 224849280, 0);

			getCharacter(kCharacterMonsieur).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, _functionsMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]);
			fedEx(kCharacterMonsieur, kCharacterMonsieur, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 134466544:
		getCharacterCurrentParams(kCharacterMonsieur)[1] = 0;
		break;
	case 135854206:
		getCharacterCurrentParams(kCharacterMonsieur)[1] = 1;
		break;
	case 168717392:
		getCharacterCurrentParams(kCharacterMonsieur)[0] = 1;
		startCycOtis(kCharacterMonsieur, "008D");
		if (!getCharacterCurrentParams(kCharacterMonsieur)[1]) {
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 5;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB1078", 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_InPart1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_InPart1);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 21;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_InPart1(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
		MonsieurCall(&LogicManager::CONS_Monsieur_InSalon, 1071000, "101A", 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 2;
			MonsieurCall(&LogicManager::CONS_Monsieur_ReturnSalon, 0, "101B", 0, 0);
			break;
		case 2:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 3;
			MonsieurCall(&LogicManager::CONS_Monsieur_CompLogic, 1102500, 0, 0, 0);
			break;
		case 3:
			setDoor(34, kCharacterCath, 1, 0, 0);
			setDoor(50, kCharacterCath, 1, 0, 0);

			if (checkCathDir(kCarRedSleeping, 54) || checkCathDir(kCarRedSleeping, 44))
				bumpCath(kCarRedSleeping, 10, 255);

			blockView(kCharacterMonsieur, kCarRedSleeping, 54);
			blockView(kCharacterMonsieur, kCarRedSleeping, 44);
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 4;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB1074", 0, 0, 0);
			break;
		case 4:
			releaseView(kCharacterMonsieur, kCarRedSleeping, 54);
			releaseView(kCharacterMonsieur, kCarRedSleeping, 44);
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 5;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoDinner, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 6;
			MonsieurCall(&LogicManager::CONS_Monsieur_CompLogic, 1170000, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 7;
			MonsieurCall(&LogicManager::CONS_Monsieur_GoSalon, 0, "102A", 0, 0);
			break;
		case 7:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 8;
			MonsieurCall(&LogicManager::CONS_Monsieur_InSalon, 1183500, "102B", 0, 0);
			break;
		case 8:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 9;
			MonsieurCall(&LogicManager::CONS_Monsieur_ReturnSalon, 0, "102C", 0, 0);
			break;
		case 9:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 10;
			MonsieurCall(&LogicManager::CONS_Monsieur_CompLogic, 1215000, 0, 0, 0);
			break;
		case 10:
			CONS_Monsieur_Asleep(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_Asleep);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 22;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMonsieur).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMonsieur).characterPosition.position = 6470;
		getCharacter(kCharacterMonsieur).characterPosition.location = 1;
		setDoor(34, kCharacterCath, 1, 10, 9);
		setDoor(50, kCharacterCath, 1, 10, 9);
		endGraphics(kCharacterMonsieur);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_StartPart2);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 23;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Monsieur_AtBreakfast(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMonsieur);
		getCharacter(kCharacterMonsieur).characterPosition.position = 4689;
		getCharacter(kCharacterMonsieur).characterPosition.location = 1;
		getCharacter(kCharacterMonsieur).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterMonsieur).inventoryItem = 0;
		setDoor(34, kCharacterCath, 0, 10, 9);
		setDoor(50, kCharacterCath, 0, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_AtBreakfast(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_AtBreakfast);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 24;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_AtBreakfast(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1759500 && !getCharacterCurrentParams(kCharacterMonsieur)[1]) {
			getCharacterCurrentParams(kCharacterMonsieur)[1] = 1;
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
			MonsieurCall(&LogicManager::CONS_Monsieur_ExitDining, 1, 0, 0, 0);
		}

		break;
	case 12:
		startCycOtis(kCharacterMonsieur, "008D");
		break;
	case 17:
		if (inDiningRoom(kCharacterCath) && !getCharacterCurrentParams(kCharacterMonsieur)[0]) {
			playDialog(kCharacterMonsieur, "MRB2001", -1, 0);
			getCharacterCurrentParams(kCharacterMonsieur)[0] = 1;
		}

		break;
	case 18:
		if (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] == 1)
			CONS_Monsieur_InComp2(0, 0, 0, 0);

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_InComp2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_InComp2);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 25;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_InComp2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(34, kCharacterCath, 2, 255, 255);
		startCycOtis(kCharacterMonsieur, "510");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_StartPart3);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 26;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Monsieur_OpenComp3(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMonsieur);
		getCharacter(kCharacterMonsieur).characterPosition.location = 1;
		getCharacter(kCharacterMonsieur).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMonsieur).inventoryItem = 0;
		getCharacter(kCharacterMonsieur).clothes = 0;
		setDoor(34, kCharacterCath, 2, 255, 255);
		setDoor(50, kCharacterCath, 1, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_OpenComp3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_OpenComp3);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 27;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_OpenComp3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(34, kCharacterCath, 2, 255, 255);
		startCycOtis(kCharacterMonsieur, "510");
		break;
	case 122288808:
		CONS_Monsieur_GoLunch(0, 0, 0, 0);
		break;
	case 122358304:
		startCycOtis(kCharacterMonsieur, "BLANK");
		break;
	}
}

void LogicManager::CONS_Monsieur_GoLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_GoLunch);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 28;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_GoLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
		MonsieurCall(&LogicManager::CONS_Monsieur_GoDining, 1, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] == 1) {
			CONS_Monsieur_AtLunch(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_AtLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_AtLunch);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 29;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_AtLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMonsieur)[1] || (getCharacterCurrentParams(kCharacterMonsieur)[1] = _gameTime + 450, _gameTime != -450)) {
			if (getCharacterCurrentParams(kCharacterMonsieur)[1] >= _gameTime) {
				if (!getCharacterCurrentParams(kCharacterMonsieur)[0])
					break;

				if (inDiningRoom(kCharacterAnna) && inDiningRoom(kCharacterAugust) && !whoRunningDialog(kCharacterMonsieur) && getCharacterCurrentParams(kCharacterMonsieur)[2] != 0x7FFFFFFF && _gameTime) {
					if (_gameTime > 1998000) {
						getCharacterCurrentParams(kCharacterMonsieur)[2] = 0x7FFFFFFF;
						getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
						MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB3102", 0, 0, 0);
						break;
					}

					if (!inDiningRoom(kCharacterCath) || !getCharacterCurrentParams(kCharacterMonsieur)[2]) {
						getCharacterCurrentParams(kCharacterMonsieur)[2] = _gameTime + 450;
						if (_gameTime == -450) {
							getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
							MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB3102", 0, 0, 0);
							break;
						}
					}

					if (getCharacterCurrentParams(kCharacterMonsieur)[2] < _gameTime) {
						getCharacterCurrentParams(kCharacterMonsieur)[2] = 0x7FFFFFFF;
						getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
						MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB3102", 0, 0, 0);
						break;
					}
				}

				if (_gameTime > 2002500 && !getCharacterCurrentParams(kCharacterMonsieur)[3]) {
					getCharacterCurrentParams(kCharacterMonsieur)[3] = 1;
					getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 2;
					MonsieurCall(&LogicManager::CONS_Monsieur_ExitDining, 1, 0, 0, 0);
				}

				break;
			}

			getCharacterCurrentParams(kCharacterMonsieur)[1] = 0x7FFFFFFF;
		}

		send(kCharacterMonsieur, kCharacterWaiter2, 256200848, 0);

		if (!getCharacterCurrentParams(kCharacterMonsieur)[0])
			break;

		if (inDiningRoom(kCharacterAnna) && inDiningRoom(kCharacterAugust) && !whoRunningDialog(kCharacterMonsieur) && getCharacterCurrentParams(kCharacterMonsieur)[2] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime > 1998000) {
				getCharacterCurrentParams(kCharacterMonsieur)[2] = 0x7FFFFFFF;
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
				MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB3102", 0, 0, 0);
				break;
			}

			if (!inDiningRoom(kCharacterCath) || !getCharacterCurrentParams(kCharacterMonsieur)[2]) {
				getCharacterCurrentParams(kCharacterMonsieur)[2] = _gameTime + 450;
				if (_gameTime == -450) {
					getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
					MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB3102", 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterMonsieur)[2] < _gameTime) {
				getCharacterCurrentParams(kCharacterMonsieur)[2] = 0x7FFFFFFF;
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
				MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "MRB3102", 0, 0, 0);
				break;
			}
		}

		if (_gameTime > 2002500 && !getCharacterCurrentParams(kCharacterMonsieur)[3]) {
			getCharacterCurrentParams(kCharacterMonsieur)[3] = 1;
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 2;
			MonsieurCall(&LogicManager::CONS_Monsieur_ExitDining, 1, 0, 0, 0);
		}

		break;
	case 12:
		startCycOtis(kCharacterMonsieur, "008B");
		break;
	case 18:

		if (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] != 1) {
			if (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] == 2)
				CONS_Monsieur_AfterLunchOpen(0, 0, 0, 0);
		} else if (_gameTime > 2002500 && !getCharacterCurrentParams(kCharacterMonsieur)[3]) {
			getCharacterCurrentParams(kCharacterMonsieur)[3] = 1;
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 2;
			MonsieurCall(&LogicManager::CONS_Monsieur_ExitDining, 1, 0, 0, 0);
		}

		break;
	case 122288808:
		startCycOtis(kCharacterMonsieur, "008D");
		getCharacterCurrentParams(kCharacterMonsieur)[0] = 1;
		break;
	case 122358304:
		startCycOtis(kCharacterMonsieur, "BLANK");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_AfterLunchOpen(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_AfterLunchOpen);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 30;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_AfterLunchOpen(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(34, kCharacterCath, 2, 255, 255);
		startCycOtis(kCharacterMonsieur, "510");
		break;
	case 122288808:
		startCycOtis(kCharacterMonsieur, "510");
		break;
	case 122358304:
		startCycOtis(kCharacterMonsieur, "BLANK");
		break;
	}
}

void LogicManager::CONS_Monsieur_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_StartPart4);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 31;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Monsieur_OpenComp4(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMonsieur);
		getCharacter(kCharacterMonsieur).characterPosition.position = 6470;
		getCharacter(kCharacterMonsieur).characterPosition.location = 1;
		getCharacter(kCharacterMonsieur).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMonsieur).inventoryItem = 0;
		getCharacter(kCharacterMonsieur).clothes = 0;
		setDoor(34, kCharacterCath, 2, 255, 255);
		setDoor(50, kCharacterCath, 1, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_OpenComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_OpenComp4);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 32;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_OpenComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2367000 && !getCharacterCurrentParams(kCharacterMonsieur)[0]) {
			getCharacterCurrentParams(kCharacterMonsieur)[0] = 1;
			CONS_Monsieur_Dinner4(0, 0, 0, 0);
		}

		break;
	case 12:
		setDoor(34, kCarNone, 2, 255, 255);
		startCycOtis(kCharacterMonsieur, "510");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_Dinner4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_Dinner4);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 33;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_Dinner4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMonsieur)[0] && _gameTime > 2389500 && !getCharacterCurrentParams(kCharacterMonsieur)[1]) {
			getCharacterCurrentParams(kCharacterMonsieur)[1] = 1;
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 3;
			MonsieurCall(&LogicManager::CONS_Monsieur_ExitDining, 0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
		MonsieurCall(&LogicManager::CONS_Monsieur_GoDining, 1, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterMonsieur, "008B");
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 2;
			MonsieurCall(&LogicManager::CONS_Monsieur_DoWait, 450, 0, 0, 0);
			break;
		case 2:
			send(kCharacterMonsieur, kCharacterWaiter2, 256200848, 0);
			break;
		case 3:
			CONS_Monsieur_ReturnComp4(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 122288808:
		getCharacterCurrentParams(kCharacterMonsieur)[0] = 1;
		startCycOtis(kCharacterMonsieur, "008D");
		break;
	case 122358304:
		startCycOtis(kCharacterMonsieur, "BLANK");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_ReturnComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_ReturnComp4);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 34;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_ReturnComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime <= 2470500 || getCharacterCurrentParams(kCharacterMonsieur)[0]) {
			if (_gameTime > 2457000) {
				if (_doneNIS[kEventAugustDrink]) {
					send(kCharacterMonsieur, kCharacterAbbot, 159003408, 0);
					getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 1;
					MonsieurCall(&LogicManager::CONS_Monsieur_GoSalon, 0, "102A", 0, 0);
				}
			}
		} else {
			getCharacterCurrentParams(kCharacterMonsieur)[0] = 1;
			CONS_Monsieur_Asleep4(0, 0, 0, 0);
		}

		break;
	case 12:
		send(kCharacterMonsieur, kCharacterAbbot, 101687594, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 2;
			MonsieurCall(&LogicManager::CONS_Monsieur_InSalon, 2479500, "102B", 0, 0);
			break;
		case 2:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 3;
			MonsieurCall(&LogicManager::CONS_Monsieur_ReturnSalon, 0, "102C", 0, 0);
			break;
		case 3:
		case 7:
			CONS_Monsieur_Asleep4(0, 0, 0, 0);
			break;
		case 4:
		case 8:
			if (_gameTime >= 2470500) {
				CONS_Monsieur_Asleep4(0, 0, 0, 0);
			} else if (_doneNIS[kEventAugustDrink]) {
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 5;
				MonsieurCall(&LogicManager::CONS_Monsieur_GoSalon, 0, "102A", 0, 0);
			} else {
				getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 8;
				MonsieurCall(&LogicManager::CONS_Monsieur_CompLogic, _gameTime + 900, 0, 0, 0);
			}

			break;
		case 5:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 6;
			MonsieurCall(&LogicManager::CONS_Monsieur_InSalon, 2479500, "102B", 0, 0);
			break;
		case 6:
			getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 7;
			MonsieurCall(&LogicManager::CONS_Monsieur_ReturnSalon, 0, "102C", 0, 0);
			break;
		case 9:
			send(kCharacterMonsieur, kCharacterCond2, 123199584, 0);
			break;
		default:
			break;
		}

		break;
	case 122865568:
		send(kCharacterMonsieur, kCharacterCond2, 88652208, 0);
		break;
	case 125039808:
		getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 4;
		MonsieurCall(&LogicManager::CONS_Monsieur_CompLogic, 2457000, 0, 0, 0);
		break;
	case 221683008:
		getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall + 8] = 9;
		MonsieurCall(&LogicManager::CONS_Monsieur_DoDialog, "Mrb1001", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_Asleep4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_Asleep4);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 35;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_Asleep4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMonsieur).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMonsieur).characterPosition.position = 6470;
		getCharacter(kCharacterMonsieur).characterPosition.location = 1;
		endGraphics(kCharacterMonsieur);
		setDoor(34, kCharacterCath, 1, 10, 9);
		setDoor(50, kCharacterCath, 1, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_StartPart5);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 36;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Monsieur_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMonsieur);
		getCharacter(kCharacterMonsieur).characterPosition.position = 3969;
		getCharacter(kCharacterMonsieur).characterPosition.location = 1;
		getCharacter(kCharacterMonsieur).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterMonsieur).inventoryItem = 0;
		getCharacter(kCharacterMonsieur).clothes = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_Prisoner);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 37;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Monsieur_Hiding(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_Hiding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_Hiding);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 38;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_Hiding(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMonsieur).characterPosition.position = 5790;
		getCharacter(kCharacterMonsieur).characterPosition.location = 1;
		getCharacter(kCharacterMonsieur).characterPosition.car = kCarRedSleeping;
		break;
	case 135800432:
		CONS_Monsieur_Disappear(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Monsieur_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_Monsieur_Disappear);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 39;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_Monsieur_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsMonsieur[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Monsieur_DebugWalks,
	&LogicManager::HAND_Monsieur_DoDialog,
	&LogicManager::HAND_Monsieur_DoSeqOtis,
	&LogicManager::HAND_Monsieur_DoWait,
	&LogicManager::HAND_Monsieur_DoBlockSeqOtis,
	&LogicManager::HAND_Monsieur_DoCorrOtis,
	&LogicManager::HAND_Monsieur_DoEnterCorrOtis,
	&LogicManager::HAND_Monsieur_FinishSeqOtis,
	&LogicManager::HAND_Monsieur_WaitRCClear,
	&LogicManager::HAND_Monsieur_DoWalk,
	&LogicManager::HAND_Monsieur_GoDining,
	&LogicManager::HAND_Monsieur_DoOtis5008A,
	&LogicManager::HAND_Monsieur_DoOtis5008E,
	&LogicManager::HAND_Monsieur_ExitDining,
	&LogicManager::HAND_Monsieur_GoSalon,
	&LogicManager::HAND_Monsieur_ReturnSalon,
	&LogicManager::HAND_Monsieur_InSalon,
	&LogicManager::HAND_Monsieur_CompLogic,
	&LogicManager::HAND_Monsieur_Birth,
	&LogicManager::HAND_Monsieur_DoDinner,
	&LogicManager::HAND_Monsieur_InPart1,
	&LogicManager::HAND_Monsieur_Asleep,
	&LogicManager::HAND_Monsieur_StartPart2,
	&LogicManager::HAND_Monsieur_AtBreakfast,
	&LogicManager::HAND_Monsieur_InComp2,
	&LogicManager::HAND_Monsieur_StartPart3,
	&LogicManager::HAND_Monsieur_OpenComp3,
	&LogicManager::HAND_Monsieur_GoLunch,
	&LogicManager::HAND_Monsieur_AtLunch,
	&LogicManager::HAND_Monsieur_AfterLunchOpen,
	&LogicManager::HAND_Monsieur_StartPart4,
	&LogicManager::HAND_Monsieur_OpenComp4,
	&LogicManager::HAND_Monsieur_Dinner4,
	&LogicManager::HAND_Monsieur_ReturnComp4,
	&LogicManager::HAND_Monsieur_Asleep4,
	&LogicManager::HAND_Monsieur_StartPart5,
	&LogicManager::HAND_Monsieur_Prisoner,
	&LogicManager::HAND_Monsieur_Hiding,
	&LogicManager::HAND_Monsieur_Disappear
};

} // End of namespace LastExpress
