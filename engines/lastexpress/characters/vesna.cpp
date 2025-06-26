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

void LogicManager::CONS_Vesna(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterVesna,
			_functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]
		);

		break;
	case 1:
		CONS_Vesna_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Vesna_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Vesna_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Vesna_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Vesna_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::VesnaCall(CALL_PARAMS) {
	getCharacter(kCharacterVesna).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Vesna_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_DebugWalks);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 1;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterVesna, kCarGreenSleeping, getCharacterCurrentParams(kCharacterVesna)[0])) {
			if (getCharacterCurrentParams(kCharacterVesna)[0] == 10000) {
				getCharacterCurrentParams(kCharacterVesna)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterVesna)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterVesna).characterPosition.position = 0;
		getCharacter(kCharacterVesna).characterPosition.location = 0;
		getCharacter(kCharacterVesna).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterVesna)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_DoDialog);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterVesna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
		fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		break;
	case 12:
		playDialog(kCharacterVesna, (char *)&getCharacterCurrentParams(kCharacterVesna)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_DoCorrOtis);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterVesna, getCharacterCurrentParams(kCharacterVesna)[3]);

		getCharacter(kCharacterVesna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
		fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterVesna, (char *)&getCharacterCurrentParams(kCharacterVesna)[0]);
		blockAtDoor(kCharacterVesna, getCharacterCurrentParams(kCharacterVesna)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_DoSeqOtis);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterVesna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
		fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterVesna, (char *)&getCharacterCurrentParams(kCharacterVesna)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_DoWalk);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 5;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterVesna, getCharacterCurrentParams(kCharacterVesna)[0], getCharacterCurrentParams(kCharacterVesna)[1])) {
			getCharacter(kCharacterVesna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
			fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		}

		break;
	case 5:
		if (rnd(2) == 0) {
			playDialog(kCharacterCath, "CAT1015A", -1, 0);
		} else {
			playDialog(kCharacterCath, "CAT1015", -1, 0);
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterVesna, kCharacterCath, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_DoWait);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 6;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterVesna)[1] || (getCharacterCurrentParams(kCharacterVesna)[1] = _gameTime + getCharacterCurrentParams(kCharacterVesna)[0],
															  _gameTime + getCharacterCurrentParams(kCharacterVesna)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterVesna)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterVesna)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterVesna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
		fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_DoWalkBehind(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_DoWalkBehind);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 7;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_DoWalkBehind(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacterCurrentParams(kCharacterVesna)[2] = 0;

		if (nearChar(kCharacterVesna, kCharacterMilos, 500) ||
			(getCharacter(kCharacterVesna).direction == 1 &&
				((getCharacter(kCharacterVesna).characterPosition.car > getCharacter(kCharacterMilos).characterPosition.car) ||
				 (getCharacter(kCharacterVesna).characterPosition.car == getCharacter(kCharacterMilos).characterPosition.car && getCharacter(kCharacterVesna).characterPosition.position > getCharacter(kCharacterMilos).characterPosition.position))) ||
			((getCharacter(kCharacterVesna).direction == 2 && getCharacter(kCharacterVesna).characterPosition.car < getCharacter(kCharacterMilos).characterPosition.car) ||
															 (getCharacter(kCharacterVesna).characterPosition.car == getCharacter(kCharacterMilos).characterPosition.car && getCharacter(kCharacterVesna).characterPosition.position < getCharacter(kCharacterMilos).characterPosition.position))) {

			getCharacter(kCharacterVesna).waitedTicksUntilCycleRestart = 0;
			getCharacterCurrentParams(kCharacterVesna)[2] = 1;
		}

		if (!getCharacterCurrentParams(kCharacterVesna)[2]) {
			walk(kCharacterVesna, getCharacterCurrentParams(kCharacterVesna)[0], getCharacterCurrentParams(kCharacterVesna)[1]);
		}

		break;
	case 12:
		walk(kCharacterVesna, getCharacterCurrentParams(kCharacterVesna)[0], getCharacterCurrentParams(kCharacterVesna)[1]);
		break;
	case 123668192:
		getCharacter(kCharacterVesna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
		fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_WaitRCClear);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 8;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterVesna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
			fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_FinishSeqOtis);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 9;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterVesna).direction != 4) {
			getCharacter(kCharacterVesna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
			fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterVesna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
		fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_SaveGame);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterVesna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
		fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		break;
	case 12:
		save(
			kCharacterVesna,
			getCharacterCurrentParams(kCharacterVesna)[0],
			getCharacterCurrentParams(kCharacterVesna)[1]
		);

		getCharacter(kCharacterVesna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
		fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_HomeAlone(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_HomeAlone);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 11;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_HomeAlone(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterVesna)[2]) {
			if (getCharacterCurrentParams(kCharacterVesna)[6] || (getCharacterCurrentParams(kCharacterVesna)[6] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterVesna)[6] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterVesna)[6] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterVesna)[2] = 0;
			getCharacterCurrentParams(kCharacterVesna)[1] = 1;
			setDoor(38, kCharacterVesna, 1, 0, 0);
			getCharacterCurrentParams(kCharacterVesna)[6] = 0;
		} else {
			getCharacterCurrentParams(kCharacterVesna)[6] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterVesna)[2]) {
			setDoor(38, kCharacterVesna, 3, 0, 0);
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 4;
			VesnaCall(&LogicManager::CONS_Vesna_DoDialog, getCathSorryDialog(), 0, 0, 0);
			break;
		}

		getCharacterCurrentParams(kCharacterVesna)[0]++;
		if (getCharacterCurrentParams(kCharacterVesna)[0] == 1) {
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterVesna)[3], 12, "VES1015A");
		} else {
			if (getCharacterCurrentParams(kCharacterVesna)[0] != 2) {
				Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterVesna)[3], 12, "VES1015C");
				getCharacterCurrentParams(kCharacterVesna)[0] = 0;
			} else {
				Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterVesna)[3], 12, "VES1015B");
			}
		}

		setDoor(38, kCharacterVesna, 3, 0, 0);
		if (msg->action == 9) {
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
			VesnaCall(&LogicManager::CONS_Vesna_DoDialog, "LIB013", 0, 0, 0);
		} else {
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 2;
			VesnaCall(&LogicManager::CONS_Vesna_DoDialog, "LIB012", 0, 0, 0);
		}

		break;
	case 12:
		setDoor(38, kCharacterVesna, 3, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterVesna)[1] || getCharacterCurrentParams(kCharacterVesna)[2]) {
			setDoor(38, kCharacterVesna, 1, 10, 9);
			getCharacterCurrentParams(kCharacterVesna)[1] = 0;
			getCharacterCurrentParams(kCharacterVesna)[2] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8]) {
		case 1:
		case 2:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 3;
			VesnaCall(&LogicManager::CONS_Vesna_DoDialog, (char *)&getCharacterCurrentParams(kCharacterVesna)[3], 0, 0, 0);
			break;
		case 3:
			setDoor(38, kCharacterVesna, 3, 14, 0);
			getCharacterCurrentParams(kCharacterVesna)[2] = 1;
			break;
		case 4:
			getCharacterCurrentParams(kCharacterVesna)[2] = 0;
			getCharacterCurrentParams(kCharacterVesna)[1] = 1;
			break;
		default:
			break;
		}

		break;
	case 55996766:
	case 101687594:
		getCharacter(kCharacterVesna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
		fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_Birth);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 12;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterVesna)[0]) {
			getCharacterCurrentParams(kCharacterVesna)[0] = 1;
			CONS_Vesna_WithMilos(0, 0, 0, 0);
		}

		break;
	case 12:
		autoMessage(kCharacterVesna, 124190740, 0);
		getCharacter(kCharacterVesna).characterPosition.position = 4689;
		getCharacter(kCharacterVesna).characterPosition.location = 1;
		getCharacter(kCharacterVesna).characterPosition.car = kCarRestaurant;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_WithMilos(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_WithMilos);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 13;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_WithMilos(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterVesna).characterPosition = getCharacter(kCharacterMilos).characterPosition;
		break;
	case 18:
		if (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] == 1) {
			endGraphics(kCharacterVesna);
			CONS_Vesna_HomeTogether(0, 0, 0, 0);
		}

		break;
	case 204832737:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
		VesnaCall(&LogicManager::CONS_Vesna_DoWalkBehind, 4, 3050, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_HomeTogether(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_HomeTogether);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 14;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_HomeTogether(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterVesna).characterPosition.position = 3050;
		getCharacter(kCharacterVesna).characterPosition.location = 1;
		getCharacter(kCharacterVesna).characterPosition.car = kCarRedSleeping;
		break;
	case 190412928:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
		VesnaCall(&LogicManager::CONS_Vesna_HomeAlone, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_Asleep);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 15;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterVesna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterVesna).characterPosition.position = 3050;
		getCharacter(kCharacterVesna).characterPosition.location = 1;
		endGraphics(kCharacterVesna);
		setDoor(38, kCharacterCath, 3, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_StartPart2);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 16;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Vesna_InPart2(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterVesna);
		getCharacter(kCharacterVesna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterVesna).characterPosition.position = 3050;
		getCharacter(kCharacterVesna).characterPosition.location = 1;
		getCharacter(kCharacterVesna).clothes = 0;
		getCharacter(kCharacterVesna).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_InPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_InPart2);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 17;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_InPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 18:
		break;
	case 135024800:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 2;
		VesnaCall(&LogicManager::CONS_Vesna_CheckTrain, 0, 0, 0, 0);
		break;
	case 137165825:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
		VesnaCall(&LogicManager::CONS_Vesna_HomeAlone, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_CheckTrain(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_CheckTrain);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 18;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_CheckTrain(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
		VesnaCall(&LogicManager::CONS_Vesna_DoCorrOtis, "610BG", 38, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterVesna).characterPosition.location = 0;

			if (getCharacter(kCharacterVesna).characterPosition.position < 2087)
				getCharacter(kCharacterVesna).characterPosition.position = 2088;

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 2;
			VesnaCall(&LogicManager::CONS_Vesna_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 3;
			VesnaCall(&LogicManager::CONS_Vesna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterVesna).characterPosition.position = 1540;
			getCharacter(kCharacterVesna).characterPosition.location = 0;
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 4;
			VesnaCall(&LogicManager::CONS_Vesna_DoSeqOtis, "808US", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterVesna, "808UD");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterVesna);

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 5;
			VesnaCall(&LogicManager::CONS_Vesna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterVesna).characterPosition.location = 1;
			endGraphics(kCharacterVesna);
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 6;
			VesnaCall(&LogicManager::CONS_Vesna_DoWait, 4500, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 7;
			VesnaCall(&LogicManager::CONS_Vesna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterVesna).characterPosition.position = 5800;
			getCharacter(kCharacterVesna).characterPosition.location = 0;
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 8;
			VesnaCall(&LogicManager::CONS_Vesna_DoSeqOtis, "808DD", 0, 0, 0);
			break;
		case 8:
			startSeqOtis(kCharacterVesna, "808DS");
			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterVesna);
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 9;
			VesnaCall(&LogicManager::CONS_Vesna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 10;
			VesnaCall(&LogicManager::CONS_Vesna_DoWalk, 4, 3050, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 11;
			VesnaCall(&LogicManager::CONS_Vesna_DoCorrOtis, "610AG", 38, 0, 0);
			break;
		case 11:
			getCharacter(kCharacterVesna).characterPosition.location = 1;
			endGraphics(kCharacterVesna);

			getCharacter(kCharacterVesna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
			fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_StartPart3);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 19;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Vesna_InComp(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterVesna);
		getCharacter(kCharacterVesna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterVesna).characterPosition.position = 3050;
		getCharacter(kCharacterVesna).characterPosition.location = 1;
		getCharacter(kCharacterVesna).clothes = 0;
		getCharacter(kCharacterVesna).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_InComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_InComp);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 20;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_InComp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalAnnaIsInBaggageCar] && getCharacterCurrentParams(kCharacterVesna)[6] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime > 2250000) {
				getCharacterCurrentParams(kCharacterVesna)[6] = 0x7FFFFFFF;
				CONS_Vesna_KillAnna(0, 0, 0, 0);
				break;
			}

			if (!cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterVesna)[6]) {
				getCharacterCurrentParams(kCharacterVesna)[6] = _gameTime;
				if (!_gameTime) {
					CONS_Vesna_KillAnna(0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterVesna)[6] < _gameTime) {
				getCharacterCurrentParams(kCharacterVesna)[6] = 0x7FFFFFFF;
				CONS_Vesna_KillAnna(0, 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterVesna)[1]) {
			if (getCharacterCurrentParams(kCharacterVesna)[7] || (getCharacterCurrentParams(kCharacterVesna)[7] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterVesna)[7] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterVesna)[7] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterVesna)[1] = 0;
			getCharacterCurrentParams(kCharacterVesna)[0] = 1;
			setDoor(38, kCharacterVesna, 1, 0, 0);
			getCharacterCurrentParams(kCharacterVesna)[7] = 0;
		} else {
			getCharacterCurrentParams(kCharacterVesna)[7] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterVesna)[1]) {
			setDoor(38, kCharacterVesna, 3, 0, 0);
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 4;
			VesnaCall(&LogicManager::CONS_Vesna_DoDialog, getCathSorryDialog(), 0, 0, 0);
			break;
		}

		getCharacterCurrentParams(kCharacterVesna)[2]++;
		if (getCharacterCurrentParams(kCharacterVesna)[2] == 1) {
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterVesna)[3], 12, "VES1015A");
		} else {
			if (getCharacterCurrentParams(kCharacterVesna)[2] != 2) {
				Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterVesna)[3], 12, "VES1015C");
				getCharacterCurrentParams(kCharacterVesna)[2] = 0;
			} else {
				Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterVesna)[3], 12, "VES1015B");
			}
		}

		setDoor(38, kCharacterVesna, 3, 0, 0);
		if (msg->action == 9) {
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
			VesnaCall(&LogicManager::CONS_Vesna_DoDialog, "LIB013", 0, 0, 0);
		} else {
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 2;
			VesnaCall(&LogicManager::CONS_Vesna_DoDialog, "LIB012", 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterVesna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterVesna).characterPosition.position = 3050;
		getCharacter(kCharacterVesna).characterPosition.location = 1;
		getCharacter(kCharacterVesna).clothes = 0;
		getCharacter(kCharacterVesna).inventoryItem = 0;
		endGraphics(kCharacterVesna);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterVesna)[0] || getCharacterCurrentParams(kCharacterVesna)[1]) {
			setDoor(38, kCharacterVesna, 1, 10, 9);
			getCharacterCurrentParams(kCharacterVesna)[0] = 0;
			getCharacterCurrentParams(kCharacterVesna)[1] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8]) {
		case 1:
		case 2:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 3;
			VesnaCall(&LogicManager::CONS_Vesna_DoDialog, (char *)&getCharacterCurrentParams(kCharacterVesna)[3], 0, 0, 0);
			break;
		case 3:
			setDoor(38, kCharacterVesna, 3, 14, 0);
			getCharacterCurrentParams(kCharacterVesna)[1] = 1;
			break;
		case 4:
			getCharacterCurrentParams(kCharacterVesna)[1] = 0;
			getCharacterCurrentParams(kCharacterVesna)[0] = 1;
			break;
		default:
			break;
		}

		break;
	case 137165825:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 5;
		VesnaCall(&LogicManager::CONS_Vesna_HomeAlone, 0, 0, 0, 0);
		break;
	case 155913424:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 6;
		VesnaCall(&LogicManager::CONS_Vesna_TakeAWalk, 0, 0, 0, 0);
		break;
	case 203663744:
		setDoor(38, kCharacterVesna, 3, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_TakeAWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_TakeAWalk);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 21;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_TakeAWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
		VesnaCall(&LogicManager::CONS_Vesna_DoCorrOtis, "610Bg", 38, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterVesna).characterPosition.location = 0;

			if (getCharacter(kCharacterVesna).characterPosition.position < 2087)
				getCharacter(kCharacterVesna).characterPosition.position = 2088;

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 2;
			VesnaCall(&LogicManager::CONS_Vesna_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 3;
			VesnaCall(&LogicManager::CONS_Vesna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterVesna).characterPosition.position = 1540;
			getCharacter(kCharacterVesna).characterPosition.location = 0;
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 4;
			VesnaCall(&LogicManager::CONS_Vesna_DoSeqOtis, "808US", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterVesna, "808UD");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterVesna);

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 5;
			VesnaCall(&LogicManager::CONS_Vesna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
			endGraphics(kCharacterVesna);
			getCharacter(kCharacterVesna).characterPosition.position = 5900;
			getCharacter(kCharacterVesna).characterPosition.location = 1;
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 6;
			VesnaCall(&LogicManager::CONS_Vesna_DoWait, 4500, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 7;
			VesnaCall(&LogicManager::CONS_Vesna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterVesna).characterPosition.position = 5800;
			getCharacter(kCharacterVesna).characterPosition.location = 0;
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 8;
			VesnaCall(&LogicManager::CONS_Vesna_DoSeqOtis, "808DD", 0, 0, 0);
			break;
		case 8:
			startSeqOtis(kCharacterVesna, "808DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterVesna);

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 9;
			VesnaCall(&LogicManager::CONS_Vesna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 10;
			VesnaCall(&LogicManager::CONS_Vesna_DoWalk, 4, 3050, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 11;
			VesnaCall(&LogicManager::CONS_Vesna_DoCorrOtis, "610Ag", 3050, 0, 0);
			break;
		case 11:
			getCharacter(kCharacterVesna).characterPosition.position = 3050;
			getCharacter(kCharacterVesna).characterPosition.location = 1;
			endGraphics(kCharacterVesna);

			getCharacter(kCharacterVesna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
			fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_KillAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_KillAnna);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 22;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_KillAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		send(kCharacterVesna, kCharacterMilos, 259125998, 0);
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
		VesnaCall(&LogicManager::CONS_Vesna_DoCorrOtis, "610Bg", 38, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterVesna).characterPosition.location = 0;

			if (getCharacter(kCharacterVesna).characterPosition.position < 2087)
				getCharacter(kCharacterVesna).characterPosition.position = 2088;

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 2;
			VesnaCall(&LogicManager::CONS_Vesna_DoWalk, kCarRestaurant, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 3;
			VesnaCall(&LogicManager::CONS_Vesna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterVesna).characterPosition.position = 1540;
			getCharacter(kCharacterVesna).characterPosition.location = 0;
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 4;
			VesnaCall(&LogicManager::CONS_Vesna_DoSeqOtis, "808US", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterVesna, "808UD");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterVesna);

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 5;
			VesnaCall(&LogicManager::CONS_Vesna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
			endGraphics(kCharacterVesna);
			getCharacter(kCharacterVesna).characterPosition.car = kCarBaggage;
			send(kCharacterVesna, kCharacterAnna, 235856512, 0);
			break;
		case 6:
			getCharacter(kCharacterVesna).characterPosition.car = kCarRestaurant;
			getCharacter(kCharacterVesna).characterPosition.position = 5800;
			getCharacter(kCharacterVesna).characterPosition.location = 0;
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 7;
			VesnaCall(&LogicManager::CONS_Vesna_DoSeqOtis, "808DD", 0, 0, 0);
			break;
		case 7:
			startSeqOtis(kCharacterVesna, "808DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterVesna);

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 8;
			VesnaCall(&LogicManager::CONS_Vesna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 9;
			VesnaCall(&LogicManager::CONS_Vesna_DoWalk, kCarRedSleeping, 3050, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 10;
			VesnaCall(&LogicManager::CONS_Vesna_DoCorrOtis, "610Ag", 3050, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterVesna).characterPosition.position = 3050;
			getCharacter(kCharacterVesna).characterPosition.location = 1;
			endGraphics(kCharacterVesna);
			CONS_Vesna_KilledAnna(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 189299008:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 6;
		VesnaCall(&LogicManager::CONS_Vesna_WaitRCClear, 0, 0, 0, 0);
		break;
	}
}

void LogicManager::CONS_Vesna_KilledAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_KilledAnna);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 23;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_KilledAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 8:
	case 9:
		setDoor(38, kCharacterVesna, 3, 0, 0);
		if (msg->action == 8) {
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
			VesnaCall(&LogicManager::CONS_Vesna_DoDialog, "LIB012", 0, 0, 0);
		} else {
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 2;
			VesnaCall(&LogicManager::CONS_Vesna_DoDialog, "LIB013", 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterVesna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterVesna).characterPosition.position = 3050;
		getCharacter(kCharacterVesna).characterPosition.location = 1;
		getCharacter(kCharacterVesna).clothes = 0;
		getCharacter(kCharacterVesna).inventoryItem = 0;
		break;
	case 18:
		if (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8]) {
			if (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] <= 2) {
				getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 3;
				VesnaCall(&LogicManager::CONS_Vesna_DoDialog, "VES1015A", 0, 0, 0);
			} else if (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] == 3) {
				setDoor(38, kCharacterVesna, 3, 10, 9);
			}
		}

		break;
	case 203663744:
		setDoor(38, kCharacterVesna, 3, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_StartPart4);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 24;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
		VesnaCall(&LogicManager::CONS_Vesna_HomeAlone, 0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterVesna);
		getCharacter(kCharacterVesna).characterPosition.position = 3050;
		getCharacter(kCharacterVesna).characterPosition.location = 1;
		getCharacter(kCharacterVesna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterVesna).inventoryItem = 0;
		setDoor(38, kCharacterVesna, 3, 10, 9);
		break;
	case 18:
		if (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] == 1) {
			CONS_Vesna_Exit(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_Exit(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_Exit);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 25;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_Exit(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2428200 && !getCharacterCurrentParams(kCharacterVesna)[0]) {
			getCharacterCurrentParams(kCharacterVesna)[0] = 1;
			CONS_Vesna_Done(0, 0, 0, 0);
		}

		break;
	case 12:
		send(kCharacterVesna, kCharacterMilos, 135600432, 0);
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
		VesnaCall(&LogicManager::CONS_Vesna_DoCorrOtis, "610BG", 38, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterVesna).characterPosition.location = 0;

			if (getCharacter(kCharacterVesna).characterPosition.position < 2087)
				getCharacter(kCharacterVesna).characterPosition.position = 2088;

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 2;
			VesnaCall(&LogicManager::CONS_Vesna_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 3;
			VesnaCall(&LogicManager::CONS_Vesna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterVesna).characterPosition.position = 1540;
			getCharacter(kCharacterVesna).characterPosition.location = 0;
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 4;
			VesnaCall(&LogicManager::CONS_Vesna_DoSeqOtis, "808US", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterVesna, "808UD");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterVesna);

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 5;
			VesnaCall(&LogicManager::CONS_Vesna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
			endGraphics(kCharacterVesna);
			getCharacter(kCharacterVesna).characterPosition.position = 5900;
			getCharacter(kCharacterVesna).characterPosition.location = 1;
			endGraphics(kCharacterVesna);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_Done(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_Done);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 26;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_Done(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
		VesnaCall(&LogicManager::CONS_Vesna_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterVesna).characterPosition.car = kCarRestaurant;
			getCharacter(kCharacterVesna).characterPosition.position = 5800;
			getCharacter(kCharacterVesna).characterPosition.location = 0;
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 2;
			VesnaCall(&LogicManager::CONS_Vesna_DoSeqOtis, "808DD", 0, 0, 0);
			break;
		case 2:
			startSeqOtis(kCharacterVesna, "808DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterVesna);

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 3;
			VesnaCall(&LogicManager::CONS_Vesna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 4;
			VesnaCall(&LogicManager::CONS_Vesna_DoWalk, 4, 3050, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 5;
			VesnaCall(&LogicManager::CONS_Vesna_DoCorrOtis, "610AG", 38, 0, 0);
			break;
		case 5:
			CONS_Vesna_EndPart4(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_EndPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_EndPart4);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 27;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_EndPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterVesna);
		setDoor(38, kCharacterCath, 3, 10, 9);
		getCharacter(kCharacterVesna).characterPosition.location = 1;
		getCharacter(kCharacterVesna).characterPosition.position = 3050;
		getCharacter(kCharacterVesna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterVesna).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_StartPart5);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 28;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Vesna_Guarding(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterVesna);
		getCharacter(kCharacterVesna).characterPosition.location = 1;
		getCharacter(kCharacterVesna).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterVesna).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_Guarding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_Guarding);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 29;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_Guarding(HAND_PARAMS) {
	switch (msg->action) {
	case 9:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
		VesnaCall(&LogicManager::CONS_Vesna_SaveGame, 2, kEventCathVesnaRestaurantKilled, 0, 0);
		break;
	case 12:
		setDoor(64, kCharacterVesna, 0, 0, 1);
		break;
	case 18:
		if (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] == 1) {
			playNIS(kEventCathVesnaRestaurantKilled);
			endGame(0, 1, 0, true);
		}

		break;
	case 134427424:
		setDoor(64, kCharacterCath, 0, 0, 1);
		CONS_Vesna_Climbing(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_Climbing(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_Climbing);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 30;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_Climbing(HAND_PARAMS) {
	int fightOutcome;

	switch (msg->action) {
	case 0:
	{
		bool skip = false;

		if (!getCharacterCurrentParams(kCharacterVesna)[0]) {
			if (getCharacterCurrentParams(kCharacterVesna)[2] || (getCharacterCurrentParams(kCharacterVesna)[2] = _realTime + 120, _realTime != -120)) {
				if (getCharacterCurrentParams(kCharacterVesna)[2] >= _realTime) {
					skip = true;
				}

				if (!skip) {
					getCharacterCurrentParams(kCharacterVesna)[2] = 0x7FFFFFFF;
				}
			}

			if (!skip) {
				playDialog(kCharacterVesna, "Ves5001", 16, 0);
				getCharacterCurrentParams(kCharacterVesna)[0] = 1;
			}
		}

		if (getCharacterCurrentParams(kCharacterVesna)[3] || (getCharacterCurrentParams(kCharacterVesna)[3] = _realTime + 180, _realTime != -180)) {
			if (getCharacterCurrentParams(kCharacterVesna)[3] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterVesna)[3] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
		VesnaCall(&LogicManager::CONS_Vesna_SaveGame, 2, kEventCathVesnaTrainTopKilled, 0, 0);
		break;
	}
	case 18:
		switch (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8]) {
		case 1:
		case 2:
			playNIS(kEventCathVesnaTrainTopKilled);
			endGame(0, 1, 0, true);
			break;
		case 3:
			playNIS(kEventCathVesnaTrainTopFight);
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 4;
			VesnaCall(&LogicManager::CONS_Vesna_SaveGame, 1, 0, 0, 0);
			break;
		case 4:
			fightOutcome = playFight(2005);
			getCharacterCurrentParams(kCharacterVesna)[1] = fightOutcome;

			if (fightOutcome) {
				endGame(0, 0, 0, fightOutcome == 1);
			} else {
				playDialog(0, "TUNNEL", -1, 0);
				fadeToBlack();
				_gameTime += 1800;
				getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 5;
				VesnaCall(&LogicManager::CONS_Vesna_SaveGame, 2, kEventCathVesnaTrainTopWin, 0, 0);
			}

			break;
		case 5:
			playNIS(kEventCathVesnaTrainTopWin);
			bumpCath(kCarRestaurant, 11, 255);
			CONS_Vesna_Disappear(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 167992577:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 3;
		VesnaCall(&LogicManager::CONS_Vesna_SaveGame, 2, kEventCathVesnaTrainTopFight, 0, 0);
		break;
	case 202884544:
		if (getCharacterCurrentParams(kCharacterVesna)[0]) {
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 2;
			VesnaCall(&LogicManager::CONS_Vesna_SaveGame, 2, kEventCathVesnaTrainTopKilled, 0, 0);
		} else {
			playDialog(kCharacterVesna, "Ves5001", 16, 0);
			getCharacterCurrentParams(kCharacterVesna)[0] = 1;
		}
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Vesna_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_Vesna_Disappear);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 31;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_Vesna_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsVesna[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Vesna_DebugWalks,
	&LogicManager::HAND_Vesna_DoDialog,
	&LogicManager::HAND_Vesna_DoCorrOtis,
	&LogicManager::HAND_Vesna_DoSeqOtis,
	&LogicManager::HAND_Vesna_DoWalk,
	&LogicManager::HAND_Vesna_DoWait,
	&LogicManager::HAND_Vesna_DoWalkBehind,
	&LogicManager::HAND_Vesna_WaitRCClear,
	&LogicManager::HAND_Vesna_FinishSeqOtis,
	&LogicManager::HAND_Vesna_SaveGame,
	&LogicManager::HAND_Vesna_HomeAlone,
	&LogicManager::HAND_Vesna_Birth,
	&LogicManager::HAND_Vesna_WithMilos,
	&LogicManager::HAND_Vesna_HomeTogether,
	&LogicManager::HAND_Vesna_Asleep,
	&LogicManager::HAND_Vesna_StartPart2,
	&LogicManager::HAND_Vesna_InPart2,
	&LogicManager::HAND_Vesna_CheckTrain,
	&LogicManager::HAND_Vesna_StartPart3,
	&LogicManager::HAND_Vesna_InComp,
	&LogicManager::HAND_Vesna_TakeAWalk,
	&LogicManager::HAND_Vesna_KillAnna,
	&LogicManager::HAND_Vesna_KilledAnna,
	&LogicManager::HAND_Vesna_StartPart4,
	&LogicManager::HAND_Vesna_Exit,
	&LogicManager::HAND_Vesna_Done,
	&LogicManager::HAND_Vesna_EndPart4,
	&LogicManager::HAND_Vesna_StartPart5,
	&LogicManager::HAND_Vesna_Guarding,
	&LogicManager::HAND_Vesna_Climbing,
	&LogicManager::HAND_Vesna_Disappear
};

} // End of namespace LastExpress
