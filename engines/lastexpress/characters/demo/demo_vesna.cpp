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

void LogicManager::CONS_DemoVesna(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterVesna,
			_functionsDemoVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]
		);

		break;
	case 1:
		CONS_DemoVesna_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoVesna_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoVesna_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoVesna_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoVesna_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::DemoVesnaCall(CALL_PARAMS) {
	getCharacter(kCharacterVesna).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_DemoVesna_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_DemoVesna_DoCorrOtis);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 1;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_DemoVesna_DoCorrOtis(HAND_PARAMS) {
	if (msg->action == 3) {
		releaseAtDoor(kCharacterVesna, getCharacterCurrentParams(kCharacterVesna)[3]);

		getCharacter(kCharacterVesna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsDemoVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
		fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
	} else if (msg->action == 12) {
		startSeqOtis(kCharacterVesna, (char *)&getCharacterCurrentParams(kCharacterVesna)[0]);
		blockAtDoor(kCharacterVesna, getCharacterCurrentParams(kCharacterVesna)[3]);
	}
}

void LogicManager::CONS_DemoVesna_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_DemoVesna_DoSeqOtis);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_DemoVesna_DoSeqOtis(HAND_PARAMS) {
	if (msg->action == 3) {
		getCharacter(kCharacterVesna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsDemoVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
		fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
	} else if (msg->action == 12) {
		startSeqOtis(kCharacterVesna, (char *)&getCharacterCurrentParams(kCharacterVesna)[0]);
	}
}

void LogicManager::CONS_DemoVesna_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_DemoVesna_DoWalk);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 3;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_DemoVesna_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterVesna, getCharacterCurrentParams(kCharacterVesna)[0], getCharacterCurrentParams(kCharacterVesna)[1])) {
			getCharacter(kCharacterVesna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsDemoVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
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

void LogicManager::CONS_DemoVesna_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_DemoVesna_WaitRCClear);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 4;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_DemoVesna_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterVesna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsDemoVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
			fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoVesna_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_DemoVesna_FinishSeqOtis);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 5;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_DemoVesna_FinishSeqOtis(HAND_PARAMS) {
	if (msg->action == 3) {
		getCharacter(kCharacterVesna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterVesna, _functionsDemoVesna[getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall]]);
		fedEx(kCharacterVesna, kCharacterVesna, 18, 0);
	}
}

void LogicManager::CONS_DemoVesna_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_DemoVesna_Birth);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 6;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_DemoVesna_Birth(HAND_PARAMS) {
	if (msg->action == 12)
		CONS_DemoVesna_InComp(0, 0, 0, 0);
}

void LogicManager::CONS_DemoVesna_InComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_DemoVesna_InComp);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 7;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_DemoVesna_InComp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalAnnaIsInBaggageCar] && getCharacterCurrentParams(kCharacterVesna)[0] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime <= 2259000) {
				if (!cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterVesna)[0]) {
					getCharacterCurrentParams(kCharacterVesna)[0] = _gameTime;
					if (!_gameTime) {
						CONS_DemoVesna_KillAnna(0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterVesna)[0] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterVesna)[0] = 0x7FFFFFFF;
			CONS_DemoVesna_KillAnna(0, 0, 0, 0);
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
	default:
		break;
	}
}

void LogicManager::CONS_DemoVesna_KillAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_DemoVesna_KillAnna);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 8;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_DemoVesna_KillAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 1;
		DemoVesnaCall(&LogicManager::CONS_DemoVesna_DoCorrOtis, "610Bg", 38, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterVesna).characterPosition.location = 0;
			if (getCharacter(kCharacterVesna).characterPosition.position < 2087)
				getCharacter(kCharacterVesna).characterPosition.position = 2088;
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 2;
			DemoVesnaCall(&LogicManager::CONS_DemoVesna_DoWalk, kCarRestaurant, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 3;
			DemoVesnaCall(&LogicManager::CONS_DemoVesna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterVesna).characterPosition.position = 1540;
			getCharacter(kCharacterVesna).characterPosition.location = 0;
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 4;
			DemoVesnaCall(&LogicManager::CONS_DemoVesna_DoSeqOtis, "808US", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterVesna, "808UD");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterVesna);

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 5;
			DemoVesnaCall(&LogicManager::CONS_DemoVesna_FinishSeqOtis, 0, 0, 0, 0);
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
			DemoVesnaCall(&LogicManager::CONS_DemoVesna_DoSeqOtis, "808DD", 0, 0, 0);
			break;
		case 7:
			startSeqOtis(kCharacterVesna, "808DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterVesna);

			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 8;
			DemoVesnaCall(&LogicManager::CONS_DemoVesna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 9;
			DemoVesnaCall(&LogicManager::CONS_DemoVesna_DoWalk, kCarRedSleeping, 3050, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 10;
			DemoVesnaCall(&LogicManager::CONS_DemoVesna_DoCorrOtis, "610Ag", 3050, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterVesna).characterPosition.position = 3050;
			getCharacter(kCharacterVesna).characterPosition.location = 1;
			endGraphics(kCharacterVesna);
			break;
		default:
			break;
		}

		break;
	case 189299008:
		getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall + 8] = 6;
		DemoVesnaCall(&LogicManager::CONS_DemoVesna_WaitRCClear, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoVesna_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_DemoVesna_StartPart2);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 9;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_DemoVesna_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoVesna_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_DemoVesna_StartPart3);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 10;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_DemoVesna_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoVesna_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_DemoVesna_StartPart4);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 11;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_DemoVesna_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoVesna_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterVesna).callParams[getCharacter(kCharacterVesna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterVesna, &LogicManager::HAND_DemoVesna_StartPart5);
	getCharacter(kCharacterVesna).callbacks[getCharacter(kCharacterVesna).currentCall] = 12;

	params->clear();

	fedEx(kCharacterVesna, kCharacterVesna, 12, 0);
}

void LogicManager::HAND_DemoVesna_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoVesna[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoVesna_DoCorrOtis,
	&LogicManager::HAND_DemoVesna_DoSeqOtis,
	&LogicManager::HAND_DemoVesna_DoWalk,
	&LogicManager::HAND_DemoVesna_WaitRCClear,
	&LogicManager::HAND_DemoVesna_FinishSeqOtis,
	&LogicManager::HAND_DemoVesna_Birth,
	&LogicManager::HAND_DemoVesna_InComp,
	&LogicManager::HAND_DemoVesna_KillAnna,
	&LogicManager::HAND_DemoVesna_StartPart2,
	&LogicManager::HAND_DemoVesna_StartPart3,
	&LogicManager::HAND_DemoVesna_StartPart4,
	&LogicManager::HAND_DemoVesna_StartPart5
};


} // End of namespace LastExpress
