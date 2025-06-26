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

void LogicManager::CONS_Max(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMax,
			_functionsMax[getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall]]
		);

		break;
	case 1:
		CONS_Max_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Max_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Max_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Max_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Max_StartPart5(0, 0, 0, 0);
		break;
	default:
		return;
	}
}

void LogicManager::MaxCall(CALL_PARAMS) {
	getCharacter(kCharacterMax).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Max_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_DebugWalks);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 1;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterMax, kCarGreenSleeping, getCharacterCurrentParams(kCharacterMax)[0])) {
			if (getCharacterCurrentParams(kCharacterMax)[0] == 10000) {
				getCharacterCurrentParams(kCharacterMax)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterMax)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterMax).characterPosition.position = 0;
		getCharacter(kCharacterMax).characterPosition.location = 0;
		getCharacter(kCharacterMax).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterMax)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_DoDialog);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterMax).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMax, _functionsMax[getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall]]);
		fedEx(kCharacterMax, kCharacterMax, 18, 0);
		break;
	case 12:
		playDialog(kCharacterMax, (char *)&getCharacterCurrentParams(kCharacterMax)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_DoSeqOtis);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterMax).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMax, _functionsMax[getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall]]);
		fedEx(kCharacterMax, kCharacterMax, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMax, (char *)&getCharacterCurrentParams(kCharacterMax)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_DoCorrOtis);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterMax, getCharacterCurrentParams(kCharacterMax)[3]);
		getCharacter(kCharacterMax).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMax, _functionsMax[getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall]]);
		fedEx(kCharacterMax, kCharacterMax, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMax, (char *)&getCharacterCurrentParams(kCharacterMax)[0]);
		blockAtDoor(kCharacterMax, getCharacterCurrentParams(kCharacterMax)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_SaveGame);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 5;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterMax).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMax, _functionsMax[getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall]]);
		fedEx(kCharacterMax, kCharacterMax, 18, 0);
		break;
	case 12:
		save(
			kCharacterMax,
			getCharacterCurrentParams(kCharacterMax)[0],
			getCharacterCurrentParams(kCharacterMax)[1]
		);

		getCharacter(kCharacterMax).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMax, _functionsMax[getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall]]);
		fedEx(kCharacterMax, kCharacterMax, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_WithAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_WithAnna);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 6;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_WithAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMax)[1] || (getCharacterCurrentParams(kCharacterMax)[1] = _gameTime + getCharacterCurrentParams(kCharacterMax)[0], _gameTime + getCharacterCurrentParams(kCharacterMax)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterMax)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterMax)[1] = 0x7FFFFFFF;
		}

		if (!whoRunningDialog(kCharacterMax))
			playDialog(kCharacterMax, "Max1122", -1, 0);

		getCharacterCurrentParams(kCharacterMax)[0] = 225 * (4 * rnd(20) + 40);
		getCharacterCurrentParams(kCharacterMax)[1] = 0;
		break;
	case 12:
		getCharacterCurrentParams(kCharacterMax)[0] = 225 * (4 * rnd(20) + 40);
		break;
	case 18:
		break;
	case 71277948:
		getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] = 1;
		MaxCall(&LogicManager::CONS_Max_GuardingComp, 0, 0, 0, 0);
		break;
	case 158007856:
		if (!whoRunningDialog(kCharacterMax)) {
			playDialog(kCharacterMax, "Max1122", -1, 0);
			getCharacterCurrentParams(kCharacterMax)[0] = 225 * (4 * rnd(20) + 40);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_GuardingComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_GuardingComp);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 7;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_GuardingComp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMax)[1] || (getCharacterCurrentParams(kCharacterMax)[1] = _gameTime + getCharacterCurrentParams(kCharacterMax)[0], _gameTime + getCharacterCurrentParams(kCharacterMax)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterMax)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterMax)[1] = 0x7FFFFFFF;
		}

		if (!whoRunningDialog(kCharacterMax))
			playDialog(kCharacterMax, "Max1122", -1, 0);

		getCharacterCurrentParams(kCharacterMax)[0] = 225 * (4 * rnd(20) + 40);
		getCharacterCurrentParams(kCharacterMax)[1] = 0;
		break;
	case 8:
	case 9:
		setDoor(37, kCharacterMax, 1, 0, 0);
		setDoor(53, kCharacterMax, 1, 0, 0);

		if (whoRunningDialog(kCharacterMax))
			fadeDialog(kCharacterMax);

		if (msg->action == 8) {
			getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] = 1;
			MaxCall(&LogicManager::CONS_Max_DoDialog, "LIB012", 0, 0, 0);
		} else {
			getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] = 2;
			MaxCall(&LogicManager::CONS_Max_DoDialog, "LIB013", 0, 0, 0);
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterMax)[0] = 225 * (4 * rnd(20) + 40);
		getCharacter(kCharacterMax).characterPosition.position = 4070;
		getCharacter(kCharacterMax).characterPosition.location = 1;
		getCharacter(kCharacterMax).characterPosition.car = kCarRedSleeping;
		setDoor(37, kCharacterMax, 1, 10, 9);
		setDoor(53, kCharacterMax, 1, 10, 9);
		break;
	case 17:
		if (!whoRunningDialog(kCharacterMax) && (checkCathDir(kCarRedSleeping, 56) || checkCathDir(kCarRedSleeping, 78))) {
			playDialog(kCharacterMax, "MAX1120", -1, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8]) {
			if (getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] <= 2) {
				getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] = 3;
				MaxCall(&LogicManager::CONS_Max_DoDialog, "MAX1122", 0, 0, 0);
			} else if (getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] == 3) {
				setDoor(37, kCharacterMax, 1, 10, 9);
				setDoor(53, kCharacterMax, 1, 10, 9);
			}
		}

		break;
	case 101687594:
		endGraphics(kCharacterMax);
		getCharacter(kCharacterMax).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMax, _functionsMax[getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall]]);
		fedEx(kCharacterMax, kCharacterMax, 18, 0);
		break;
	case 122358304:
	case 135204609:
		send(kCharacterMax, kCharacterMax, 135204609, 0);
		setDoor(37, kCharacterCath, 0, 10, 9);
		setDoor(53, kCharacterCath, 0, 10, 9);
		getCharacter(kCharacterMax).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMax, _functionsMax[getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall]]);
		fedEx(kCharacterMax, kCharacterMax, 18, 0);
		break;
	case 158007856:
		if (!whoRunningDialog(kCharacterMax)) {
			playDialog(kCharacterMax, "Max1122", -1, 0);
			getCharacterCurrentParams(kCharacterMax)[0] = 225 * (4 * rnd(20) + 40);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_InCageFriendly(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_InCageFriendly);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 8;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_InCageFriendly(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMax)[2] ||
			(getCharacterCurrentParams(kCharacterMax)[2] = _gameTime + getCharacterCurrentParams(kCharacterMax)[1],
				_gameTime + getCharacterCurrentParams(kCharacterMax)[1] != 0)) {
			if (getCharacterCurrentParams(kCharacterMax)[2] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterMax)[2] = 0x7FFFFFFF;
		}

		if (!whoRunningDialog(kCharacterMax))
			playDialog(kCharacterMax, "Max3101", -1, 0);

		getCharacterCurrentParams(kCharacterMax)[1] = 225 * (4 * rnd(4) + 8);
		getCharacterCurrentParams(kCharacterMax)[2] = 0;

		break;
	case 9:
		if (getCharacterCurrentParams(kCharacterMax)[0]) {
			getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] = 1;
			MaxCall(&LogicManager::CONS_Max_SaveGame, 2, kEventCathMaxLickHand, 0, 0);
		} else {
			if (whoRunningDialog(kCharacterMax))
				fadeDialog(kCharacterMax);

			playNIS(kEventCathMaxLickHand);
			cleanNIS();
			getCharacterCurrentParams(kCharacterMax)[0] = 1;
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterMax)[1] = 225 * (4 * rnd(4) + 8);
		setDoor(109, kCharacterMax, 0, 0, 9);
		endGraphics(kCharacterMax);

		getCharacter(kCharacterMax).characterPosition.position = 8000;
		getCharacter(kCharacterMax).characterPosition.location = 1;
		getCharacter(kCharacterMax).characterPosition.car = kCarBaggage;

		if (!whoRunningDialog(kCharacterMax))
			playDialog(kCharacterMax, "Max3101", -1, 0);

		break;
	case 18:
		if (getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] == 1) {
			if (whoRunningDialog(kCharacterMax))
				fadeDialog(kCharacterMax);

			playDialog(0, "LIB026", -1, 0);
			playNIS(kEventCathMaxFree);
			bumpCath(kCarBaggage, 92, 255);
			setDoor(109, kCharacterCath, 0, 0, 9);
			CONS_Max_Escaped(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_Escaped(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_Escaped);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 9;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_Escaped(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMax)[1] != 0x7FFFFFFF && _gameTime) {
			if (getCharacterCurrentParams(kCharacterMax)[0] >= _gameTime) {
				if (!whoOnScreen(kCharacterMax) || !getCharacterCurrentParams(kCharacterMax)[1]) {
					getCharacterCurrentParams(kCharacterMax)[1] = _gameTime;
					if (!_gameTime) {
						if (_globals[kGlobalChapter] == 3) {
							CONS_Max_LetMeIn3(0, 0, 0, 0);
						} else if (_globals[kGlobalChapter] == 4) {
							CONS_Max_LetMeIn4(0, 0, 0, 0);
						}
					}
				}

				if (getCharacterCurrentParams(kCharacterMax)[1] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterMax)[1] = 0x7FFFFFFF;
			if (_globals[kGlobalChapter] == 3) {
				CONS_Max_LetMeIn3(0, 0, 0, 0);
			} else if (_globals[kGlobalChapter] == 4) {
				CONS_Max_LetMeIn4(0, 0, 0, 0);
			}
		}

		break;
	case 12:
		getCharacter(kCharacterMax).characterPosition.position = 4070;
		getCharacter(kCharacterMax).characterPosition.location = 0;
		getCharacter(kCharacterMax).characterPosition.car = kCarRedSleeping;
		startCycOtis(kCharacterMax, "630Af");
		softBlockAtDoor(kCharacterMax, 37);
		getCharacterCurrentParams(kCharacterMax)[0] = _gameTime + 2700;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_Birth);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 10;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterMax)[0]) {
			getCharacterCurrentParams(kCharacterMax)[0] = 1;
			CONS_Max_WithAnna(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterMax).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMax).characterPosition.position = 4070;
		getCharacter(kCharacterMax).characterPosition.location = 1;
		getCharacter(kCharacterMax).clothes = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_StartPart2);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 11;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Max_WithAnna(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMax);
		getCharacter(kCharacterMax).characterPosition.position = 4070;
		getCharacter(kCharacterMax).characterPosition.location = 1;
		getCharacter(kCharacterMax).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMax).clothes = 0;
		getCharacter(kCharacterMax).inventoryItem = kItemNone;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_StartPart3);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 12;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Max_WithAnna3(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMax);
		getCharacter(kCharacterMax).characterPosition.position = 4070;
		getCharacter(kCharacterMax).characterPosition.location = 1;
		getCharacter(kCharacterMax).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMax).clothes = 0;
		getCharacter(kCharacterMax).inventoryItem = kItemNone;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_WithAnna3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_WithAnna3);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 13;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_WithAnna3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMax)[1]) {
			getCharacter(kCharacterMax).characterPosition = getCharacter(kCharacterCond2).characterPosition;
			break;
		}

		if (getCharacterCurrentParams(kCharacterMax)[2] || (getCharacterCurrentParams(kCharacterMax)[2] = _gameTime + getCharacterCurrentParams(kCharacterMax)[0], _gameTime + getCharacterCurrentParams(kCharacterMax)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterMax)[2] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterMax)[2] = 0x7FFFFFFF;
		}

		if (!whoRunningDialog(kCharacterMax))
			playDialog(kCharacterMax, "Max1122", -1, 0);

		getCharacterCurrentParams(kCharacterMax)[0] = 225 * (4 * rnd(20) + 40);
		getCharacterCurrentParams(kCharacterMax)[2] = 0;
		break;
	case 12:
		getCharacterCurrentParams(kCharacterMax)[0] = 225 * (4 * rnd(20) + 40);
		getCharacter(kCharacterMax).characterPosition.position = 4070;
		getCharacter(kCharacterMax).characterPosition.location = 1;
		getCharacter(kCharacterMax).characterPosition.car = kCarRedSleeping;
		break;
	case 71277948:
		getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] = 1;
		MaxCall(&LogicManager::CONS_Max_GuardingComp, 0, 0, 0, 0);
		break;
	case 122358304:
		getCharacterCurrentParams(kCharacterMax)[1] = 1;
		break;
	case 135204609:
		CONS_Max_InCageMad(0, 0, 0, 0);
		break;
	case 158007856:
		if (!getCharacterCurrentParams(kCharacterMax)[1] && !whoRunningDialog(30)) {
			playDialog(kCharacterMax, "Max1122", -1, 0);
			getCharacterCurrentParams(kCharacterMax)[0] = 225 * (4 * rnd(20) + 40);
		}

		break;
	}
}

void LogicManager::CONS_Max_InCageMad(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_InCageMad);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 14;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_InCageMad(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		playDialog(kCharacterMax, "Max1122", -1, 0);
		break;
	case 9:
		if (_doneNIS[kEventCathMaxCage]) {
			if (!_doneNIS[kEventCathMaxFree]) {
				getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] = 2;
				MaxCall(&LogicManager::CONS_Max_SaveGame, 2, kEventCathMaxFree, 0, 0);
			}
		} else {
			getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] = 1;
			MaxCall(&LogicManager::CONS_Max_SaveGame, 2, kEventCathMaxCage, 0, 0);
		}

		break;
	case 12:
		setDoor(109, kCharacterMax, 0, 0, 9);
		getCharacter(kCharacterMax).characterPosition.position = 8000;
		getCharacter(kCharacterMax).characterPosition.location = 1;
		getCharacter(kCharacterMax).characterPosition.car = kCarBaggage;

		if (!whoRunningDialog(kCharacterMax))
			playDialog(kCharacterMax, "Max1122", -1, 0);

		break;
	case 18:
		if (getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] == 1) {
			if (whoRunningDialog(kCharacterMax))
				endDialog(kCharacterMax);

			playNIS(kEventCathMaxCage);
			claimNISLink(kCharacterMax);
			cleanNIS();
		} else if (getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] == 2) {
			if (whoRunningDialog(kCharacterMax))
				fadeDialog(kCharacterMax);

			playDialog(0, "LIB026", -1, 0);
			playNIS(kEventCathMaxFree);
			bumpCath(kCarBaggage, 92, 255);
			setDoor(109, kCharacterCath, 0, 0, 9);
			CONS_Max_Escaped(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_LetMeIn3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_LetMeIn3);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 15;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_LetMeIn3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMax)[1])
			getCharacter(kCharacterMax).characterPosition = getCharacter(kCharacterCond2).characterPosition;

		if (!getCharacterCurrentParams(kCharacterMax)[0]) {
			if (getCharacterCurrentParams(kCharacterMax)[2] || (getCharacterCurrentParams(kCharacterMax)[2] = _gameTime + 900, _gameTime != -900)) {
				if (getCharacterCurrentParams(kCharacterMax)[2] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterMax)[2] = 0x7FFFFFFF;
			}

			send(kCharacterMax, kCharacterCond2, 157026693, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterMax).characterPosition.position = 4070;
		getCharacter(kCharacterMax).characterPosition.location = 0;
		getCharacter(kCharacterMax).characterPosition.car = kCarRedSleeping;

		if (!whoRunningDialog(kCharacterMax))
			playDialog(kCharacterMax, "Max3010", -1, 0);

		getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] = 1;
		MaxCall(&LogicManager::CONS_Max_DoCorrOtis, "630Bf", 37, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall + 8] == 1) {
			startCycOtis(kCharacterMax, "630Af");
			softBlockAtDoor(kCharacterMax, 37);
			send(kCharacterMax, kCharacterAnna, 156622016, 0);
		}

		break;
	case 122358304:
		if (msg->sender == 1) {
			getCharacterCurrentParams(kCharacterMax)[0] = 1;
		} else {
			getCharacterCurrentParams(kCharacterMax)[1] = 1;
		}

		softReleaseAtDoor(kCharacterMax, 37);
		startCycOtis(kCharacterMax, "BLANK");

		break;
	case 135204609:
		softReleaseAtDoor(kCharacterMax, 37);
		CONS_Max_InCageFriendly(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_StartPart4);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 16;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Max_InCageFriendly(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMax);
		getCharacter(kCharacterMax).characterPosition.position = 8000;
		getCharacter(kCharacterMax).characterPosition.location = 1;
		getCharacter(kCharacterMax).characterPosition.car = kCarBaggage;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_LetMeIn4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_LetMeIn4);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 17;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_LetMeIn4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMax)[0])
			getCharacter(kCharacterMax).characterPosition = getCharacter(kCharacterCond2).characterPosition;

		break;
	case 12:
		getCharacter(kCharacterMax).characterPosition.position = 4070;
		getCharacter(kCharacterMax).characterPosition.location = 0;
		getCharacter(kCharacterMax).characterPosition.car = kCarRedSleeping;
		startCycOtis(kCharacterMax, "630Af");
		send(kCharacterMax, kCharacterCond2, 157026693, 0);
		break;
	case 122358304:
		getCharacterCurrentParams(kCharacterMax)[0] = 1;
		softReleaseAtDoor(kCharacterMax, 37);
		startCycOtis(kCharacterMax, "BLANK");
		break;
	case 135204609:
		softReleaseAtDoor(kCharacterMax, 37);
		CONS_Max_InCageFriendly(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Max_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMax).callParams[getCharacter(kCharacterMax).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMax, &LogicManager::HAND_Max_StartPart5);
	getCharacter(kCharacterMax).callbacks[getCharacter(kCharacterMax).currentCall] = 18;

	params->clear();

	fedEx(kCharacterMax, kCharacterMax, 12, 0);
}

void LogicManager::HAND_Max_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterMax);
		getCharacter(kCharacterMax).characterPosition.position = 0;
		getCharacter(kCharacterMax).characterPosition.location = 0;
		getCharacter(kCharacterMax).characterPosition.car = kCarNone;
		setDoor(109, kCharacterCath, 0, 0, 9);
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsMax[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Max_DebugWalks,
	&LogicManager::HAND_Max_DoDialog,
	&LogicManager::HAND_Max_DoSeqOtis,
	&LogicManager::HAND_Max_DoCorrOtis,
	&LogicManager::HAND_Max_SaveGame,
	&LogicManager::HAND_Max_WithAnna,
	&LogicManager::HAND_Max_GuardingComp,
	&LogicManager::HAND_Max_InCageFriendly,
	&LogicManager::HAND_Max_Escaped,
	&LogicManager::HAND_Max_Birth,
	&LogicManager::HAND_Max_StartPart2,
	&LogicManager::HAND_Max_StartPart3,
	&LogicManager::HAND_Max_WithAnna3,
	&LogicManager::HAND_Max_InCageMad,
	&LogicManager::HAND_Max_LetMeIn3,
	&LogicManager::HAND_Max_StartPart4,
	&LogicManager::HAND_Max_LetMeIn4,
	&LogicManager::HAND_Max_StartPart5
};

} // End of namespace LastExpress
