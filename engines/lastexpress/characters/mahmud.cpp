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

void LogicManager::CONS_Mahmud(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMahmud,
			_functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);

		break;
	case 1:
		CONS_Mahmud_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Mahmud_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Mahmud_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Mahmud_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Mahmud_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::MahmudCall(CALL_PARAMS) {
	getCharacter(kCharacterMahmud).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Mahmud_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_DebugWalks);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 1;

	params->clear();

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterMahmud, kCarGreenSleeping, getCharacterCurrentParams(kCharacterMahmud)[0])) {
			if (getCharacterCurrentParams(kCharacterMahmud)[0] == 10000) {
				getCharacterCurrentParams(kCharacterMahmud)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterMahmud)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterMahmud).characterPosition.position = 0;
		getCharacter(kCharacterMahmud).characterPosition.location = 0;
		getCharacter(kCharacterMahmud).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterMahmud)[0] = 10000;

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_DoSeqOtis);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterMahmud).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
		fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMahmud, (char *)&getCharacterCurrentParams(kCharacterMahmud));
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_DoCorrOtis);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterMahmud, getCharacterCurrentParams(kCharacterMahmud)[3]);
		getCharacter(kCharacterMahmud).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
		fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMahmud, (char *)&getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall]);
		blockAtDoor(kCharacterMahmud, getCharacterCurrentParams(kCharacterMahmud)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_DoBumpCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_DoBumpCorrOtis);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;
	params->parameters[5] = param4.intParam;

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_DoBumpCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMahmud)[6] ||
			(getCharacterCurrentParams(kCharacterMahmud)[6] = _realTime + getCharacterCurrentParams(kCharacterMahmud)[4], _realTime + getCharacterCurrentParams(kCharacterMahmud)[4] != 0)) {
			if (getCharacterCurrentParams(kCharacterMahmud)[6] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterMahmud)[6] = 0x7FFFFFFF;
		}

		if (!cathFacingUp())
			bumpCathFDoor(getCharacterCurrentParams(kCharacterMahmud)[5]);

		break;
	case 3:
		releaseAtDoor(kCharacterMahmud, getCharacterCurrentParams(kCharacterMahmud)[3]);
		getCharacter(kCharacterMahmud).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
		fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMahmud, (char *)&getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall]);
		blockAtDoor(kCharacterMahmud, getCharacterCurrentParams(kCharacterMahmud)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_DoDialog);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterMahmud).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
		fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
		break;
	case 12:
		playDialog(kCharacterMahmud, (char *)&getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_DoCondDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_DoCondDialog);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_DoCondDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterMahmud).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
		fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
		break;
	case 12:
		playDialog(kCharacterMahmud, (char *)&getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall], getVolume(kCharacterCond1), 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_DoWait);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 7;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMahmud)[1] ||
			(getCharacterCurrentParams(kCharacterMahmud)[1] = _gameTime + getCharacterCurrentParams(kCharacterMahmud)[0], _gameTime + getCharacterCurrentParams(kCharacterMahmud)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterMahmud)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterMahmud)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterMahmud).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
		fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_SaveGame);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 8;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterMahmud).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
		fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
		break;
	case 12:
		save(
			kCharacterMahmud,
			getCharacterCurrentParams(kCharacterMahmud)[0],
			getCharacterCurrentParams(kCharacterMahmud)[1]
		);

		getCharacter(kCharacterMahmud).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
		fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_DoWalk);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 9;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterMahmud, getCharacterCurrentParams(kCharacterMahmud)[0], getCharacterCurrentParams(kCharacterMahmud)[1])) {
			getCharacter(kCharacterMahmud).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
			fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
		}

		break;
	case 5:
		if (cathHasItem(kItemPassengerList)) {
			switch (rnd(2)) {
			case 0:
				playDialog(kCharacterCath, "CAT1025A", -1, 0);
				break;
			case 1:
				playDialog(kCharacterCath, "CAT1025", -1, 0);
			default:
				break;
			}
		} else {
			playCathExcuseMe();
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterMahmud, kCharacterCath, 0);
		break;
	case 12:
		if (walk(kCharacterMahmud, getCharacterCurrentParams(kCharacterMahmud)[0], getCharacterCurrentParams(kCharacterMahmud)[1])) {
			getCharacter(kCharacterMahmud).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
			fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_CathKnockingHarem(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_CathKnockingHarem);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_CathKnockingHarem(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMahmud)[5] || (getCharacterCurrentParams(kCharacterMahmud)[5] = _gameTime + 13500, _gameTime != -13500)) {
			if (getCharacterCurrentParams(kCharacterMahmud)[5] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterMahmud)[5] = 0x7FFFFFFF;
		}

		setDoor(5, kCharacterClerk, 3, 10, 9);
		setDoor(6, kCharacterClerk, 3, 10, 9);
		setDoor(7, kCharacterClerk, 3, 10, 9);
		setDoor(8, kCharacterClerk, 3, 10, 9);
		getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 2;
		MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Ed", 4, 0, 0);
		break;
	case 2:
	case 17:
		if (!whoRunningDialog(kCharacterMahmud)) {
			if (getCharacter(kCharacterCath).characterPosition.position < 1500 ||
				getCharacter(kCharacterCath).characterPosition.position >= 5790 ||
				(getCharacter(kCharacterCath).characterPosition.position > 4455 && getCharacterCurrentParams(kCharacterMahmud)[4] != 5)) {
				setDoor(5, kCharacterClerk, 3, 10, 9);
				setDoor(6, kCharacterClerk, 3, 10, 9);
				setDoor(7, kCharacterClerk, 3, 10, 9);
				setDoor(8, kCharacterClerk, 3, 10, 9);
				getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 3;
				MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Ed", 4, 0, 0);
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

		getCharacterCurrentParams(kCharacterMahmud)[4] = msg->param.intParam;
		if (!whoRunningDialog(kCharacterMahmud)) {
			getCharacterCurrentParams(kCharacterMahmud)[2]++;
			switch (getCharacterCurrentParams(kCharacterMahmud)[2]) {
			case 1:
				if (!getCharacterCurrentParams(kCharacterMahmud)[1]) {
					playDialog(kCharacterMahmud, "MAH1173A", -1, 0);
				} else {
					playDialog(kCharacterMahmud, "MAH1170E", -1, 0);
				}
				
				break;
			case 2:
				playDialog(kCharacterMahmud, "MAH1173B", -1, 0);
				break;
			case 3:
				playDialog(kCharacterMahmud, "MAH1174", -1, 0);
				break;
			default:
				getCharacterCurrentParams(kCharacterMahmud)[3] = 1;
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterMahmud)[3]) {
			if (_gameTime >= 2418300) {
				getCharacterCurrentParams(kCharacterMahmud)[2] = 0;
			} else {
				playDialog(kCharacterClerk, "LIB050", 16, 0);
				if (_globals[kGlobalChapter] == 1)
					endGame(0, 0, 57, true);
				else
					endGame(0, 0, 63, true);
			}
		} else {
			bumpCathTowardsCond(msg->param.intParam, 0, 0);
			switch (checkCathDir()) {
			case 55:
				bumpCathFDoor(5);
				break;
			case 56:
				bumpCathFDoor(6);
				break;
			case 57:
				bumpCathFDoor(7);
				break;
			case 58:
				bumpCathFDoor(8);
				break;
			default:
				break;
			}
		}

		break;
	case 12:
		if (!getCharacterCurrentParams(kCharacterMahmud)[1]) {
			playDialog(kCharacterMahmud, "MAH1173", -1, 45);
		} else {
			playDialog(kCharacterMahmud, "MAH1170A", -1, 45);
		}
		
		_globals[kGlobalMetMahmud] = 1;
		getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 1;
		MahmudCall(&LogicManager::CONS_Mahmud_DoBumpCorrOtis, "614Dd", 4, 30, *getCharacterCurrentParams(kCharacterMahmud));
		break;
	case 18:
		if (getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] == 1) {
			setDoor(5, kCharacterMahmud, 3, 10, 9);
			setDoor(6, kCharacterMahmud, 3, 10, 9);
			setDoor(7, kCharacterMahmud, 3, 10, 9);
			setDoor(8, kCharacterMahmud, 3, 10, 9);
			getCharacter(kCharacterMahmud).characterPosition.location = 0;
			startCycOtis(kCharacterMahmud, "614Md");
			softBlockAtDoor(kCharacterMahmud, 4);
		} else if (getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] == 2 || getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] == 3) {
			softReleaseAtDoor(kCharacterMahmud, 4);
			getCharacter(kCharacterMahmud).characterPosition.location = 1;
			endGraphics(kCharacterMahmud);
			getCharacter(kCharacterMahmud).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
			fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_CondKnocking(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_CondKnocking);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 11;

	params->clear();

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_CondKnocking(HAND_PARAMS) {
	switch (msg->action) {
	case 8:
	case 9:
		if (msg->action == 8)
			playDialog(0, "LIB012", -1, 0);
		else
			playDialog(0, "LIB013", -1, 0);

		if (!whoRunningDialog(kCharacterMahmud)) {
			getCharacterCurrentParams(kCharacterMahmud)[0]++;
			if (getCharacterCurrentParams(kCharacterMahmud)[0] == 1) {
				playDialog(kCharacterMahmud, "MAH1170E", -1, 0);
			} else if (getCharacterCurrentParams(kCharacterMahmud)[0] == 2) {
				playDialog(kCharacterMahmud, "MAH1173B", -1, 0);
			} else {
				playDialog(kCharacterMahmud, "MAH1174", -1, 0);
			}
		}

		switch (checkCathDir()) {
		case 55:
			bumpCathFDoor(5);
			break;
		case 56:
			bumpCathFDoor(6);
			break;
		case 57:
			bumpCathFDoor(7);
			break;
		case 58:
			bumpCathFDoor(8);
			break;
		default:
			break;
		}

		break;
	case 12:
		send(kCharacterMahmud, kCharacterCond1, 102227384, 0);
		getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 1;
		MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Ad", 4, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMahmud).characterPosition.location = 0;
			setDoor(4, kCharacterCath, 3, 10, 9);
			startCycOtis(kCharacterMahmud, "614Kd");
			softBlockAtDoor(kCharacterMahmud, 4);
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 2;
			MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, "MAH1170A", 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 3;
			MahmudCall(&LogicManager::CONS_Mahmud_DoCondDialog, "MAH1170B", 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 4;
			MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, "MAH1170C", 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 5;
			MahmudCall(&LogicManager::CONS_Mahmud_DoCondDialog, "MAH1170D", 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 6;
			MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, "MAH1170E", 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 7;
			MahmudCall(&LogicManager::CONS_Mahmud_DoCondDialog, "MAH1170F", 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 8;
			MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Ld", 4, 0, 0);
			break;
		case 8:
			send(kCharacterMahmud, kCharacterCond1, 156567128, 0);
			startCycOtis(kCharacterMahmud, "614Bd");
			softBlockAtDoor(kCharacterMahmud, 4);
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 9;
			MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, "MAH1170G", 0, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 10;
			MahmudCall(&LogicManager::CONS_Mahmud_DoCondDialog, "MAH1170H", 0, 0, 0);
			break;
		case 10:
			setDoor(5, kCharacterMahmud, 3, 10, 9);
			setDoor(6, kCharacterMahmud, 3, 10, 9);
			setDoor(7, kCharacterMahmud, 3, 10, 9);
			setDoor(8, kCharacterMahmud, 3, 10, 9);
			break;
		case 11:
			softReleaseAtDoor(kCharacterMahmud, 4);
			getCharacter(kCharacterMahmud).characterPosition.location = 1;
			endGraphics(kCharacterMahmud);
			setDoor(4, kCharacterMahmud, 3, 10, 9);

			getCharacter(kCharacterMahmud).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
			fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 123852928:
		if (whoRunningDialog(kCharacterMahmud))
			fadeDialog(kCharacterMahmud);

		setDoor(5, kCharacterClerk, 3, 10, 9);
		setDoor(6, kCharacterClerk, 3, 10, 9);
		setDoor(7, kCharacterClerk, 3, 10, 9);
		setDoor(8, kCharacterClerk, 3, 10, 9);
		getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 11;
		MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Cd", 4, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_CheckF(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_CheckF);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 12;

	params->clear();

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_CheckF(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 1;
		MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Gd", 4, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMahmud).characterPosition.location = 0;
			setDoor(4, kCharacterCath, 3, 10, 9);
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 2;
			MahmudCall(&LogicManager::CONS_Mahmud_DoWalk, 3, 4070, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 3;
			MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Ff", 6, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterMahmud).characterPosition.location = 1;
			endGraphics(kCharacterMahmud);
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 4;
			MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, "Har1105", 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 5;
			MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Gf", 6, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterMahmud).characterPosition.location = 0;
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 6;
			MahmudCall(&LogicManager::CONS_Mahmud_DoWalk, 3, 5790, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 7;
			MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Fd", 4, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterMahmud).characterPosition.location = 1;
			endGraphics(kCharacterMahmud);

			getCharacter(kCharacterMahmud).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
			fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_CheckH(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_CheckH);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 13;

	params->clear();

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_CheckH(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 1;
		MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Gd", 4, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMahmud).characterPosition.location = 0;
			setDoor(4, kCharacterCath, 3, 10, 9);
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 2;
			MahmudCall(&LogicManager::CONS_Mahmud_DoWalk, 3, 2740, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 3;
			MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Fh", 8, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterMahmud).characterPosition.location = 1;
			endGraphics(kCharacterMahmud);
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 4;
			MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, "Har1107", 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 5;
			MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Gh", 8, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterMahmud).characterPosition.location = 0;
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 6;
			MahmudCall(&LogicManager::CONS_Mahmud_DoWalk, 3, 5790, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 7;
			MahmudCall(&LogicManager::CONS_Mahmud_DoCorrOtis, "614Fd", 4, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterMahmud).characterPosition.location = 1;
			endGraphics(kCharacterMahmud);
			getCharacter(kCharacterMahmud).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, _functionsMahmud[getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall]]);
			fedEx(kCharacterMahmud, kCharacterMahmud, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_Vigilant(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_Vigilant);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 14;

	params->clear();

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_Vigilant(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterParams(kCharacterMahmud, 8)[0]) {
			getCharacterCurrentParams(kCharacterMahmud)[1] = 1;
			send(kCharacterMahmud, kCharacterCond1, 204379649, 0);
			getCharacterParams(kCharacterMahmud, 8)[0] = 0;
		}

		if (!getCharacterCurrentParams(kCharacterMahmud)[1] && _globals[kGlobalChapter] == 1) {
			if (_gameTime > 1098000 && !getCharacterCurrentParams(kCharacterMahmud)[5]) {
				getCharacterCurrentParams(kCharacterMahmud)[5] = 1;
				getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 1;
				MahmudCall(&LogicManager::CONS_Mahmud_CheckH, 0, 0, 0, 0);
				break;
			}

			if (!dialogRunning("HAR1104") && _gameTime > 1167300 && !getCharacterCurrentParams(kCharacterMahmud)[6]) {
				getCharacterCurrentParams(kCharacterMahmud)[6] = 1;
				getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 2;
				MahmudCall(&LogicManager::CONS_Mahmud_CheckF, 0, 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterMahmud)[4]) {
			if (getCharacterCurrentParams(kCharacterMahmud)[7] || (getCharacterCurrentParams(kCharacterMahmud)[7] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterMahmud)[7] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterMahmud)[7] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterMahmud)[4] = 0;
			getCharacterCurrentParams(kCharacterMahmud)[3] = 1;
			setDoor(4, kCharacterMahmud, 3, 0, 0);
			getCharacterCurrentParams(kCharacterMahmud)[7] = 0;
		} else {
			getCharacterCurrentParams(kCharacterMahmud)[7] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterMahmud)[4]) {
			setDoor(4, kCharacterMahmud, 3, 0, 0);
			if (_globals[kGlobalJacket] == 1 || _doneNIS[kEventMahmudWrongDoor] || _doneNIS[kEventMahmudWrongDoorOriginalJacket] || _doneNIS[kEventMahmudWrongDoorDay]) {
				if (cathHasItem(kItemPassengerList)) {
					getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 6;

					switch (rnd(2)) {
					case 0:
						MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, "CAT1501", 0, 0, 0);
						break;
					case 1:
					default:
						MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, getCathSorryDialog(), 0, 0, 0);
						break;
					}

				} else {
					getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 7;
					MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, getCathSorryDialog(), 0, 0, 0);
				}
			} else if (msg->action == 8) {
				getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 8;
				MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 9;
				MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, "LIB013", 0, 0, 0);
			}
		} else {
			setDoor(4, kCharacterMahmud, 1, 0, 0);
			if (msg->action == 8) {
				getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 3;
				MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 4;
				MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		getCharacter(kCharacterMahmud).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterMahmud).characterPosition.position = 5790;
		getCharacter(kCharacterMahmud).characterPosition.location = 1;
		endGraphics(kCharacterMahmud);
		getCharacterCurrentParams(kCharacterMahmud)[2] = 1;
		setDoor(4, kCharacterMahmud, 3, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterMahmud)[3] || getCharacterCurrentParams(kCharacterMahmud)[4]) {
			setDoor(4, kCharacterMahmud, 3, 10, 9);
			getCharacterCurrentParams(kCharacterMahmud)[3] = 0;
			getCharacterCurrentParams(kCharacterMahmud)[4] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8]) {
		case 1:
			setDoor(4, kCharacterMahmud, 3, 10, 9);
			getCharacterCurrentParams(kCharacterMahmud)[3] = 0;
			getCharacterCurrentParams(kCharacterMahmud)[4] = 0;

			if (!dialogRunning("HAR1104") && _gameTime > 1167300 && !getCharacterCurrentParams(kCharacterMahmud)[6]) {
				getCharacterCurrentParams(kCharacterMahmud)[6] = 1;
				getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 2;
				MahmudCall(&LogicManager::CONS_Mahmud_CheckF, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] == 2) {
				setDoor(4, kCharacterMahmud, 3, 10, 9);
				getCharacterCurrentParams(kCharacterMahmud)[3] = 0;
				getCharacterCurrentParams(kCharacterMahmud)[4] = 0;
			}

			if (getCharacterCurrentParams(kCharacterMahmud)[4]) {
				if (getCharacterCurrentParams(kCharacterMahmud)[7] || (getCharacterCurrentParams(kCharacterMahmud)[7] = _realTime + 75, _realTime != -75)) {
					if (getCharacterCurrentParams(kCharacterMahmud)[7] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterMahmud)[7] = 0x7FFFFFFF;
				}

				getCharacterCurrentParams(kCharacterMahmud)[4] = 0;
				getCharacterCurrentParams(kCharacterMahmud)[3] = 1;
				setDoor(4, kCharacterMahmud, 3, 0, 0);
				getCharacterCurrentParams(kCharacterMahmud)[7] = 0;
			} else {
				getCharacterCurrentParams(kCharacterMahmud)[7] = 0;
			}

			break;
		case 3:
		case 4:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 5;
			MahmudCall(&LogicManager::CONS_Mahmud_DoDialog, "MAH1175", 0, 0, 0);
			break;
		case 5:
			if (_globals[kGlobalJacket] == 1 ||
				_doneNIS[kEventMahmudWrongDoor] ||
				_doneNIS[kEventMahmudWrongDoorOriginalJacket] ||
				_doneNIS[kEventMahmudWrongDoorDay]) {
				setDoor(4, kCharacterMahmud, 1, 14, 0);
			} else {
				setDoor(4, kCharacterMahmud, 1, 10, 9);
			}

			getCharacterCurrentParams(kCharacterMahmud)[4] = 1;
			break;
		case 6:
		case 7:
			getCharacterCurrentParams(kCharacterMahmud)[3] = 1;
			break;
		case 8:
		case 9:
			getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 10;
			MahmudCall(&LogicManager::CONS_Mahmud_SaveGame, 2, kEventMahmudWrongDoor, 0, 0);
			break;
		case 10:
			if (_globals[kGlobalJacket] == 2) {
				if (isNight()) {
					playNIS(kEventMahmudWrongDoor);
				} else {
					playNIS(kEventMahmudWrongDoorDay);
				}
			} else {
				playNIS(kEventMahmudWrongDoorOriginalJacket);
			}

			playDialog(0, "LIB015", -1, 0);
			cleanNIS();

			getCharacterCurrentParams(kCharacterMahmud)[3] = 1;
			break;
		case 11:
			setDoor(4, kCharacterMahmud, 3, 10, 9);
			getCharacterCurrentParams(kCharacterMahmud)[3] = 0;
			getCharacterCurrentParams(kCharacterMahmud)[4] = 0;
			break;
		case 12:
			setDoor(4, kCharacterMahmud, 3, 10, 9);
			getCharacterCurrentParams(kCharacterMahmud)[3] = 0;
			getCharacterCurrentParams(kCharacterMahmud)[4] = 0;
			getCharacterCurrentParams(kCharacterMahmud)[1] = 0;
			break;
		default:
			break;
		}

		break;
	case 225563840:
		getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 12;
		MahmudCall(&LogicManager::CONS_Mahmud_CondKnocking, 0, 0, 0, 0);
		break;
	case 290410610:
		getCharacterCurrentParams(kCharacterMahmud)[2] = getCharacterCurrentParams(kCharacterMahmud)[2] == 0;
		getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall + 8] = 11;
		MahmudCall(&LogicManager::CONS_Mahmud_CathKnockingHarem, msg->param, getCharacterCurrentParams(kCharacterMahmud)[2], 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_Birth);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 15;

	params->clear();

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterMahmud)[0]) {
			getCharacterCurrentParams(kCharacterMahmud)[0] = 1;
			CONS_Mahmud_Vigilant(0, 0, 0, 0);
		}

		break;
	case 12:
		autoMessage(kCharacterMahmud, 170483072, 0);
		getCharacter(kCharacterMahmud).characterPosition.position = 540;
		getCharacter(kCharacterMahmud).characterPosition.location = 0;
		getCharacter(kCharacterMahmud).characterPosition.car = kCarGreenSleeping;
		setDoor(4, kCharacterCath, 3, 10, 9);
		setDoor(20, kCharacterCath, 3, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_Asleep);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 16;

	params->clear();

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMahmud).characterPosition.position = 5790;
		getCharacter(kCharacterMahmud).characterPosition.location = 1;
		getCharacter(kCharacterMahmud).characterPosition.car = kCarGreenSleeping;
		setDoor(4, kCharacterCath, 3, 10, 9);
		endGraphics(kCharacterMahmud);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_StartPart2);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 17;

	params->clear();

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Mahmud_Vigilant(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMahmud);
		getCharacter(kCharacterMahmud).characterPosition.position = 5790;
		getCharacter(kCharacterMahmud).characterPosition.location = 1;
		getCharacter(kCharacterMahmud).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterMahmud).clothes = 0;
		getCharacter(kCharacterMahmud).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_StartPart3);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 18;

	params->clear();

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Mahmud_Vigilant(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMahmud);
		getCharacter(kCharacterMahmud).characterPosition.position = 5790;
		getCharacter(kCharacterMahmud).characterPosition.location = 1;
		getCharacter(kCharacterMahmud).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterMahmud).clothes = 0;
		getCharacter(kCharacterMahmud).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_StartPart4);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 19;

	params->clear();

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Mahmud_Vigilant(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMahmud);
		getCharacter(kCharacterMahmud).characterPosition.position = 2740;
		getCharacter(kCharacterMahmud).characterPosition.location = 1;
		getCharacter(kCharacterMahmud).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterMahmud).clothes = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mahmud_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMahmud).callParams[getCharacter(kCharacterMahmud).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMahmud, &LogicManager::HAND_Mahmud_StartPart5);
	getCharacter(kCharacterMahmud).callbacks[getCharacter(kCharacterMahmud).currentCall] = 20;

	params->clear();

	fedEx(kCharacterMahmud, kCharacterMahmud, 12, 0);
}

void LogicManager::HAND_Mahmud_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterMahmud);
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsMahmud[])(HAND_PARAMS) = {
	nullptr,	
	&LogicManager::HAND_Mahmud_DebugWalks,
	&LogicManager::HAND_Mahmud_DoSeqOtis,
	&LogicManager::HAND_Mahmud_DoCorrOtis,
	&LogicManager::HAND_Mahmud_DoBumpCorrOtis,
	&LogicManager::HAND_Mahmud_DoDialog,
	&LogicManager::HAND_Mahmud_DoCondDialog,
	&LogicManager::HAND_Mahmud_DoWait,
	&LogicManager::HAND_Mahmud_SaveGame,
	&LogicManager::HAND_Mahmud_DoWalk,
	&LogicManager::HAND_Mahmud_CathKnockingHarem,
	&LogicManager::HAND_Mahmud_CondKnocking,
	&LogicManager::HAND_Mahmud_CheckF,
	&LogicManager::HAND_Mahmud_CheckH,
	&LogicManager::HAND_Mahmud_Vigilant,
	&LogicManager::HAND_Mahmud_Birth,
	&LogicManager::HAND_Mahmud_Asleep,
	&LogicManager::HAND_Mahmud_StartPart2,
	&LogicManager::HAND_Mahmud_StartPart3,
	&LogicManager::HAND_Mahmud_StartPart4,
	&LogicManager::HAND_Mahmud_StartPart5
};

} // End of namespace LastExpress
