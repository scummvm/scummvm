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

void LogicManager::CONS_DemoFrancois(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterFrancois,
			_functionsDemoFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]
		);

		break;
	case 1:
		CONS_DemoFrancois_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoFrancois_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoFrancois_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoFrancois_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoFrancois_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::DemoFrancoisCall(CALL_PARAMS) {
	getCharacter(kCharacterFrancois).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_DemoFrancois_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_DemoFrancois_DoSeqOtis);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 1;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_DemoFrancois_DoSeqOtis(HAND_PARAMS) {
	if (msg->action == 3) {
		getCharacter(kCharacterFrancois).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsDemoFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
		fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
	} else if (msg->action == 12) {
		startSeqOtis(kCharacterFrancois, (char *)&getCharacterCurrentParams(kCharacterFrancois)[0]);
	}
}

void LogicManager::CONS_DemoFrancois_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_DemoFrancois_SaveGame);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 2;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_DemoFrancois_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterFrancois).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsDemoFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
		fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		break;
	case 12:
		save(
			kCharacterFrancois,
			getCharacterCurrentParams(kCharacterFrancois)[0],
			getCharacterCurrentParams(kCharacterFrancois)[1]
		);

		getCharacter(kCharacterFrancois).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsDemoFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
		fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoFrancois_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_DemoFrancois_Birth);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 3;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_DemoFrancois_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (whoRunningDialog(kCharacterFrancois)) {
			if (whoOnScreen(kCharacterFrancois) && whoFacingCath(kCharacterFrancois)) {
				if (nearChar(kCharacterFrancois, kCharacterCath, 2500) && !_doneNIS[kEventFrancoisWhistleD])
					getCharacter(kCharacterFrancois).inventoryItem = 0x80;
			} else {
				getCharacter(kCharacterFrancois).inventoryItem = 0;
			}

			if (getCharacterCurrentParams(kCharacterFrancois)[4]) {
				getCharacterCurrentParams(kCharacterFrancois)[4]--;
			} else if (walk(kCharacterFrancois, *getCharacterCurrentParams(kCharacterFrancois), getCharacterCurrentParams(kCharacterFrancois)[1])) {
				if (getCharacterCurrentParams(kCharacterFrancois)[1] == 540) {
					getCharacterCurrentParams(kCharacterFrancois)[0] = 4;
					getCharacterCurrentParams(kCharacterFrancois)[1] = 9460;
					getCharacterCurrentParams(kCharacterFrancois)[4] = 120;
				} else {
					getCharacterCurrentParams(kCharacterFrancois)[0] = 3;
					getCharacterCurrentParams(kCharacterFrancois)[1] = 540;
					getCharacterCurrentParams(kCharacterFrancois)[4] = 15;
					getCharacterCurrentParams(kCharacterFrancois)[3] = 0;
					send(kCharacterFrancois, kCharacterCond2, 225932896, 0);
				}
			}

			if (nearX(kCharacterFrancois, 2000, 500) &&
				getCharacter(kCharacterFrancois).direction == 2 &&
				checkLoc(kCharacterFrancois, 4) && getCharacterCurrentParams(kCharacterFrancois)[3]) {
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
				DemoFrancoisCall(&LogicManager::CONS_DemoFrancois_DoSeqOtis, "605A", 0, 0, 0);
			}

			return;
		}

		if (getCharacterCurrentParams(kCharacterFrancois)[5] || (getCharacterCurrentParams(kCharacterFrancois)[5] = getCharacterCurrentParams(kCharacterFrancois)[2] + _realTime,
																 getCharacterCurrentParams(kCharacterFrancois)[5] != 0)) {
			if (_realTime <= getCharacterCurrentParams(kCharacterFrancois)[5]) {
				if (whoOnScreen(kCharacterFrancois) && whoFacingCath(kCharacterFrancois)) {
					if (nearChar(kCharacterFrancois, kCharacterCath, 2500) && !_doneNIS[kEventFrancoisWhistleD])
						getCharacter(kCharacterFrancois).inventoryItem = 0x80;
				} else {
					getCharacter(kCharacterFrancois).inventoryItem = 0;
				}

				if (getCharacterCurrentParams(kCharacterFrancois)[4]) {
					getCharacterCurrentParams(kCharacterFrancois)[4]--;
				} else if (walk(kCharacterFrancois, *getCharacterCurrentParams(kCharacterFrancois), getCharacterCurrentParams(kCharacterFrancois)[1])) {
					if (getCharacterCurrentParams(kCharacterFrancois)[1] == 540) {
						getCharacterCurrentParams(kCharacterFrancois)[0] = 4;
						getCharacterCurrentParams(kCharacterFrancois)[1] = 9460;
						getCharacterCurrentParams(kCharacterFrancois)[4] = 120;
					} else {
						getCharacterCurrentParams(kCharacterFrancois)[0] = 3;
						getCharacterCurrentParams(kCharacterFrancois)[1] = 540;
						getCharacterCurrentParams(kCharacterFrancois)[4] = 15;
						getCharacterCurrentParams(kCharacterFrancois)[3] = 0;
						send(kCharacterFrancois, kCharacterCond2, 225932896, 0);
					}
				}

				if (nearX(kCharacterFrancois, 2000, 500) &&
					getCharacter(kCharacterFrancois).direction == 2 &&
					checkLoc(kCharacterFrancois, 4) && getCharacterCurrentParams(kCharacterFrancois)[3]) {
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
					DemoFrancoisCall(&LogicManager::CONS_DemoFrancois_DoSeqOtis, "605A", 0, 0, 0);
				}

				return;
			}

			getCharacterCurrentParams(kCharacterFrancois)[5] = 0x7FFFFFFF;
		}

		if (rnd(2) == 0) {
			playDialog(kCharacterFrancois, "Fra1002G", -1, 0);
		} else {
			playDialog(kCharacterFrancois, "Fra1002A", -1, 0);
		}

		getCharacterCurrentParams(kCharacterFrancois)[5] = 0;
		getCharacterCurrentParams(kCharacterFrancois)[2] = 15 * rnd(7);

		if (whoOnScreen(kCharacterFrancois) && whoFacingCath(kCharacterFrancois)) {
			if (nearChar(kCharacterFrancois, kCharacterCath, 2500) && !_doneNIS[kEventFrancoisWhistleD])
				getCharacter(kCharacterFrancois).inventoryItem = 0x80;
		} else {
			getCharacter(kCharacterFrancois).inventoryItem = 0;
		}

		if (getCharacterCurrentParams(kCharacterFrancois)[4]) {
			getCharacterCurrentParams(kCharacterFrancois)[4]--;
		} else if (walk(kCharacterFrancois, *getCharacterCurrentParams(kCharacterFrancois), getCharacterCurrentParams(kCharacterFrancois)[1])) {
			if (getCharacterCurrentParams(kCharacterFrancois)[1] == 540) {
				getCharacterCurrentParams(kCharacterFrancois)[0] = 4;
				getCharacterCurrentParams(kCharacterFrancois)[1] = 9460;
				getCharacterCurrentParams(kCharacterFrancois)[4] = 120;
			} else {
				getCharacterCurrentParams(kCharacterFrancois)[0] = 3;
				getCharacterCurrentParams(kCharacterFrancois)[1] = 540;
				getCharacterCurrentParams(kCharacterFrancois)[4] = 15;
				getCharacterCurrentParams(kCharacterFrancois)[3] = 0;
				send(kCharacterFrancois, kCharacterCond2, 225932896, 0);
			}
		}

		if (nearX(kCharacterFrancois, 2000, 500) &&
			getCharacter(kCharacterFrancois).direction == 2 &&
			checkLoc(kCharacterFrancois, 4) && getCharacterCurrentParams(kCharacterFrancois)[3]) {
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
			DemoFrancoisCall(&LogicManager::CONS_DemoFrancois_DoSeqOtis, "605A", 0, 0, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterFrancois).inventoryItem = 0;

		if (whoRunningDialog(kCharacterFrancois))
			fadeDialog(kCharacterFrancois);

		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
		DemoFrancoisCall(&LogicManager::CONS_DemoFrancois_SaveGame, 2, kEventFrancoisWhistle, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterFrancois).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterFrancois).characterPosition.position = 2088;
		getCharacter(kCharacterFrancois).characterPosition.location = 0;
		getCharacter(kCharacterFrancois).clothes = 1;
		getCharacter(kCharacterFrancois).walkStepSize = 100;
		getCharacter(kCharacterFrancois).inventoryItem = 0;
		getCharacterCurrentParams(kCharacterFrancois)[0] = kCarRedSleeping;
		getCharacterCurrentParams(kCharacterFrancois)[1] = 9460;
		walk(kCharacterFrancois, kCarRedSleeping, 9460);
		getCharacterCurrentParams(kCharacterFrancois)[2] = 15 * rnd(7);
		break;
	case 18:
		if (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] == 1) {
			send(kCharacterFrancois, kCharacterCond2, 168253822, 0);
			getCharacterCurrentParams(kCharacterFrancois)[0] = kCarRedSleeping;
			getCharacterCurrentParams(kCharacterFrancois)[1] = 9460;
			getCharacter(kCharacterFrancois).characterPosition.position = 2088;
			walk(kCharacterFrancois, getCharacterCurrentParams(kCharacterFrancois)[0], getCharacterCurrentParams(kCharacterFrancois)[1]);
		} else if (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] == 2) {
			playNIS(kEventFrancoisWhistleD);
			if (getCharacter(kCharacterFrancois).direction == 1) {
				bumpCathFx(kCarRedSleeping, getCharacter(kCharacterFrancois).characterPosition.position - 750);
			} else {
				bumpCathRx(kCarRedSleeping, getCharacter(kCharacterFrancois).characterPosition.position + 750);
			}
		}

		break;
	case 205346192:
		getCharacterCurrentParams(kCharacterFrancois)[3] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoFrancois_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_DemoFrancois_StartPart2);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 4;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_DemoFrancois_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoFrancois_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_DemoFrancois_StartPart3);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 5;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_DemoFrancois_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoFrancois_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_DemoFrancois_StartPart4);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 6;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_DemoFrancois_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoFrancois_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_DemoFrancois_StartPart5);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 7;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_DemoFrancois_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoFrancois[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoFrancois_DoSeqOtis,
	&LogicManager::HAND_DemoFrancois_SaveGame,
	&LogicManager::HAND_DemoFrancois_Birth,
	&LogicManager::HAND_DemoFrancois_StartPart2,
	&LogicManager::HAND_DemoFrancois_StartPart3,
	&LogicManager::HAND_DemoFrancois_StartPart4,
	&LogicManager::HAND_DemoFrancois_StartPart5
};

} // End of namespace LastExpress
