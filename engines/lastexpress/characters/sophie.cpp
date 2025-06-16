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

void LogicManager::CONS_Sophie(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterSophie,
			_functionsSophie[getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall]]
		);

		break;
	case 1:
		CONS_Sophie_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Sophie_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Sophie_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Sophie_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Sophie_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::SophieCall(CALL_PARAMS) {
	getCharacter(kCharacterSophie).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Sophie_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSophie).callParams[getCharacter(kCharacterSophie).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSophie, &LogicManager::HAND_Sophie_DebugWalks);
	getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall] = 1;

	params->clear();

	fedEx(kCharacterSophie, kCharacterSophie, 12, 0);
}

void LogicManager::HAND_Sophie_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterSophie, kCarGreenSleeping, getCharacterCurrentParams(kCharacterSophie)[0])) {
			if (getCharacterCurrentParams(kCharacterSophie)[0] == 10000) {
				getCharacterCurrentParams(kCharacterSophie)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterSophie)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterSophie).characterPosition.position = 0;
		getCharacter(kCharacterSophie).characterPosition.location = 0;
		getCharacter(kCharacterSophie).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterSophie)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Sophie_DoWalkBehind(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSophie).callParams[getCharacter(kCharacterSophie).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSophie, &LogicManager::HAND_Sophie_DoWalkBehind);
	getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall] = 2;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterSophie, kCharacterSophie, 12, 0);
}

void LogicManager::HAND_Sophie_DoWalkBehind(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacterCurrentParams(kCharacterSophie)[2] = 0;
		if (nearChar(kCharacterSophie, kCharacterRebecca, 500) ||
			((getCharacter(kCharacterSophie).direction == 1) && ((getCharacter(kCharacterSophie).characterPosition.car > getCharacter(kCharacterRebecca).characterPosition.car) ||
																 (getCharacter(kCharacterSophie).characterPosition.car == getCharacter(kCharacterRebecca).characterPosition.car && getCharacter(kCharacterSophie).characterPosition.position > getCharacter(kCharacterRebecca).characterPosition.position))) ||
			(getCharacter(kCharacterSophie).direction == 2 && ((getCharacter(kCharacterSophie).characterPosition.car < getCharacter(kCharacterRebecca).characterPosition.car) ||
															   (getCharacter(kCharacterSophie).characterPosition.car == getCharacter(kCharacterRebecca).characterPosition.car && getCharacter(kCharacterSophie).characterPosition.position < getCharacter(kCharacterRebecca).characterPosition.position)))) {

			getCharacter(kCharacterSophie).waitedTicksUntilCycleRestart = 0;
			getCharacterCurrentParams(kCharacterSophie)[2] = 1;
		}

		if (!getCharacterCurrentParams(kCharacterSophie)[2]) {
			walk(kCharacterSophie, getCharacterCurrentParams(kCharacterSophie)[0], getCharacterCurrentParams(kCharacterSophie)[1]);
		}

		break;
	case 5:
		playCathExcuseMe();
		break;
	case 6:
		playChrExcuseMe(kCharacterSophie, kCharacterCath, 0);
		break;
	case 12:
		walk(kCharacterSophie, getCharacterCurrentParams(kCharacterSophie)[0], getCharacterCurrentParams(kCharacterSophie)[1]);
		break;
	case 123668192:
		getCharacter(kCharacterSophie).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterSophie, _functionsSophie[getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall]]);
		fedEx(kCharacterSophie, kCharacterSophie, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Sophie_WithRebecca(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSophie).callParams[getCharacter(kCharacterSophie).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSophie, &LogicManager::HAND_Sophie_WithRebecca);
	getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall] = 3;

	params->clear();

	fedEx(kCharacterSophie, kCharacterSophie, 12, 0);
}

void LogicManager::HAND_Sophie_WithRebecca(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterSophie).characterPosition = getCharacter(kCharacterRebecca).characterPosition;
		break;
	case 18:
		switch (getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall + 8]) {
		case 1:
		case 3:
			endGraphics(kCharacterSophie);
			break;
		case 2:
		case 4:
			startCycOtis(kCharacterSophie, "BLANK");
			break;
		default:
			break;
		}

		break;
	case 259921280:
		getCharacter(kCharacterSophie).characterPosition.car = getCharacter(kCharacterRebecca).characterPosition.car;
		getCharacter(kCharacterSophie).characterPosition.location = getCharacter(kCharacterRebecca).characterPosition.location;
		getCharacter(kCharacterSophie).characterPosition.position = getCharacter(kCharacterRebecca).characterPosition.position + 100;
		getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall + 8] = 3;
		SophieCall(&LogicManager::CONS_Sophie_DoWalkBehind, 2, 9460, 0, 0);
		break;
	case 292775040:
		getCharacter(kCharacterSophie).characterPosition.car = 2;
		getCharacter(kCharacterSophie).characterPosition.location = 0;
		getCharacter(kCharacterSophie).characterPosition.position = 9270;
		getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall + 8] = 4;
		SophieCall(&LogicManager::CONS_Sophie_DoWalkBehind, 4, 4840, 0, 0);
		break;
	case 136654208:
		getCharacter(kCharacterSophie).characterPosition.car = getCharacter(kCharacterRebecca).characterPosition.car;
		getCharacter(kCharacterSophie).characterPosition.location = getCharacter(kCharacterRebecca).characterPosition.location;
		getCharacter(kCharacterSophie).characterPosition.position = getCharacter(kCharacterRebecca).characterPosition.position + 100;
		getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall + 8] = 2;
		SophieCall(&LogicManager::CONS_Sophie_DoWalkBehind, 4, 4840, 0, 0);
		break;
	case 125242096:
		getCharacter(kCharacterSophie).characterPosition.car = getCharacter(kCharacterRebecca).characterPosition.car;
		getCharacter(kCharacterSophie).characterPosition.location = getCharacter(kCharacterRebecca).characterPosition.location;
		getCharacter(kCharacterSophie).characterPosition.position = getCharacter(kCharacterRebecca).characterPosition.position - 100;
		getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall + 8] = 1;
		SophieCall(&LogicManager::CONS_Sophie_DoWalkBehind, 5, 850, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Sophie_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSophie).callParams[getCharacter(kCharacterSophie).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSophie, &LogicManager::HAND_Sophie_Birth);
	getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall] = 4;

	params->clear();

	fedEx(kCharacterSophie, kCharacterSophie, 12, 0);
}

void LogicManager::HAND_Sophie_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterSophie)[0]) {
			getCharacterCurrentParams(kCharacterSophie)[0] = 1;
			CONS_Sophie_WithRebecca(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterSophie).characterPosition.position = 4840;
		getCharacter(kCharacterSophie).characterPosition.location = 1;
		getCharacter(kCharacterSophie).characterPosition.car = kCarRedSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Sophie_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSophie).callParams[getCharacter(kCharacterSophie).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSophie, &LogicManager::HAND_Sophie_Asleep);
	getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall] = 5;

	params->clear();

	fedEx(kCharacterSophie, kCharacterSophie, 12, 0);
}

void LogicManager::HAND_Sophie_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterSophie).characterPosition.position = 4840;
		getCharacter(kCharacterSophie).characterPosition.location = 1;
		getCharacter(kCharacterSophie).characterPosition.car = kCarRedSleeping;
		endGraphics(kCharacterSophie);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Sophie_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSophie).callParams[getCharacter(kCharacterSophie).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSophie, &LogicManager::HAND_Sophie_StartPart2);
	getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall] = 6;

	params->clear();

	fedEx(kCharacterSophie, kCharacterSophie, 12, 0);
}

void LogicManager::HAND_Sophie_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Sophie_WithRebecca(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterSophie);
		getCharacter(kCharacterSophie).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterSophie).characterPosition.position = 4840;
		getCharacter(kCharacterSophie).characterPosition.location = 1;
		getCharacter(kCharacterSophie).clothes = 0;
		getCharacter(kCharacterSophie).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Sophie_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSophie).callParams[getCharacter(kCharacterSophie).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSophie, &LogicManager::HAND_Sophie_StartPart3);
	getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall] = 7;

	params->clear();

	fedEx(kCharacterSophie, kCharacterSophie, 12, 0);
}

void LogicManager::HAND_Sophie_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Sophie_WithRebecca(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterSophie);
		getCharacter(kCharacterSophie).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterSophie).characterPosition.position = 4840;
		getCharacter(kCharacterSophie).characterPosition.location = 1;
		getCharacter(kCharacterSophie).clothes = 0;
		getCharacter(kCharacterSophie).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Sophie_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSophie).callParams[getCharacter(kCharacterSophie).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSophie, &LogicManager::HAND_Sophie_StartPart4);
	getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall] = 8;

	params->clear();

	fedEx(kCharacterSophie, kCharacterSophie, 12, 0);
}

void LogicManager::HAND_Sophie_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Sophie_WithRebecca(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterSophie);
		getCharacter(kCharacterSophie).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterSophie).characterPosition.position = 4840;
		getCharacter(kCharacterSophie).characterPosition.location = 1;
		getCharacter(kCharacterSophie).clothes = 0;
		getCharacter(kCharacterSophie).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Sophie_Asleep4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSophie).callParams[getCharacter(kCharacterSophie).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSophie, &LogicManager::HAND_Sophie_Asleep4);
	getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall] = 9;

	params->clear();

	fedEx(kCharacterSophie, kCharacterSophie, 12, 0);
}

void LogicManager::HAND_Sophie_Asleep4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterSophie).characterPosition.position = 4840;
		getCharacter(kCharacterSophie).characterPosition.location = 1;
		getCharacter(kCharacterSophie).characterPosition.car = kCarRedSleeping;
		endGraphics(kCharacterSophie);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Sophie_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSophie).callParams[getCharacter(kCharacterSophie).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSophie, &LogicManager::HAND_Sophie_StartPart5);
	getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall] = 10;

	params->clear();

	fedEx(kCharacterSophie, kCharacterSophie, 12, 0);
}

void LogicManager::HAND_Sophie_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Sophie_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterSophie);
		getCharacter(kCharacterSophie).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterSophie).characterPosition.position = 3969;
		getCharacter(kCharacterSophie).characterPosition.location = 1;
		getCharacter(kCharacterSophie).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Sophie_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSophie).callParams[getCharacter(kCharacterSophie).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSophie, &LogicManager::HAND_Sophie_Prisoner);
	getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall] = 11;

	params->clear();

	fedEx(kCharacterSophie, kCharacterSophie, 12, 0);
}

void LogicManager::HAND_Sophie_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Sophie_Free(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Sophie_Free(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterSophie).callParams[getCharacter(kCharacterSophie).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterSophie, &LogicManager::HAND_Sophie_Free);
	getCharacter(kCharacterSophie).callbacks[getCharacter(kCharacterSophie).currentCall] = 12;

	params->clear();

	fedEx(kCharacterSophie, kCharacterSophie, 12, 0);
}

void LogicManager::HAND_Sophie_Free(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsSophie[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Sophie_DebugWalks,
	&LogicManager::HAND_Sophie_DoWalkBehind,
	&LogicManager::HAND_Sophie_WithRebecca,
	&LogicManager::HAND_Sophie_Birth,
	&LogicManager::HAND_Sophie_Asleep,
	&LogicManager::HAND_Sophie_StartPart2,
	&LogicManager::HAND_Sophie_StartPart3,
	&LogicManager::HAND_Sophie_StartPart4,
	&LogicManager::HAND_Sophie_Asleep4,
	&LogicManager::HAND_Sophie_StartPart5,
	&LogicManager::HAND_Sophie_Prisoner,
	&LogicManager::HAND_Sophie_Free
};

} // End of namespace LastExpress
