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

void LogicManager::CONS_Cook(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterCook,
			_functionsCook[getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall]]
		);

		break;
	case 1:
		CONS_Cook_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Cook_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Cook_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Cook_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Cook_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CookCall(CALL_PARAMS) {
	getCharacter(kCharacterCook).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Cook_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_DoSeqOtis);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 1;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterCook).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCook, _functionsCook[getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall]]);
		fedEx(kCharacterCook, kCharacterCook, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterCook, (char *)&getCharacterCurrentParams(kCharacterCook)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_DoDialog);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterCook).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCook, _functionsCook[getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall]]);
		fedEx(kCharacterCook, kCharacterCook, 18, 0);
		break;
	case 12:
		playDialog(kCharacterCook, (char *)&getCharacterCurrentParams(kCharacterCook)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_UptrainVersion(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_UptrainVersion);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 3;

	params->clear();

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_UptrainVersion(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		startCycOtis(kCharacterCook, "308A");
		blockView(kCharacterCook, kCarRestaurant, 75);
		blockView(kCharacterCook, kCarRestaurant, 78);
		if (_globals[kGlobalChapter] == 1) {
			getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 1;
			CookCall(&LogicManager::CONS_Cook_DoDialog, "KIT1010", 0, 0, 0);
		} else if (_globals[kGlobalChapter] == 3) {
			getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 2;
			CookCall(&LogicManager::CONS_Cook_DoDialog, "KIT1012", 0, 0, 0);
		} else {
			playDialog(kCharacterCook, "KIT1011", -1, 0);
			getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 3;
			CookCall(&LogicManager::CONS_Cook_DoSeqOtis, "308B", 0, 0, 0);
		}

		break;
	case 17:
		if (!inKitchen(kCharacterCath)) {
			endGraphics(kCharacterCook);
			getCharacter(kCharacterCook).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCook, _functionsCook[getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall]]);
			fedEx(kCharacterCook, kCharacterCook, 18, 0);
		} else {
			if (checkCathDir(kCarRestaurant, 76)) {
				startCycOtis(kCharacterCook, "308D");
				if (!whoRunningDialog(kCharacterCook)) {
					if (getCharacterCurrentParams(kCharacterCook)[0]) {
						if (!whoOnScreen(kCharacterCook) && !whoRunningDialog(kCharacterCook)) {
							playDialog(kCharacterCook, "LIB015", -1, 0);
							endGraphics(kCharacterCook);
							getCharacter(kCharacterCook).currentCall--;
							_engine->getMessageManager()->setMessageHandle(kCharacterCook, _functionsCook[getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall]]);
							fedEx(kCharacterCook, kCharacterCook, 18, 0);
						}

						break;
					}

					playDialog(kCharacterCook, "KIT1011A", -1, 0);
					getCharacterCurrentParams(kCharacterCook)[0] = 1;
				}
			}

			if (getCharacterCurrentParams(kCharacterCook)[0]) {
				if (!whoOnScreen(kCharacterCook) && !whoRunningDialog(kCharacterCook)) {
					playDialog(kCharacterCook, "LIB015", -1, 0);
					endGraphics(kCharacterCook);
					getCharacter(kCharacterCook).currentCall--;
					_engine->getMessageManager()->setMessageHandle(kCharacterCook, _functionsCook[getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall]]);
					fedEx(kCharacterCook, kCharacterCook, 18, 0);
				}
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8]) {
			if (getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] > 2) {
				if (getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] == 3) {
					startCycOtis(kCharacterCook, "308C");
					releaseView(kCharacterCook, kCarRestaurant, 75);
					releaseView(kCharacterCook, kCarRestaurant, 78);
				}
			} else {
				playDialog(kCharacterCook, "KIT1011", -1, 0);
				getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 3;
				CookCall(&LogicManager::CONS_Cook_DoSeqOtis, "308B", 0, 0, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_DowntrainVersion(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_DowntrainVersion);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 4;

	params->clear();

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_DowntrainVersion(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		startCycOtis(kCharacterCook, "308A");
		blockView(kCharacterCook, kCarRestaurant, 75);
		blockView(kCharacterCook, kCarRestaurant, 78);

		if (_globals[kGlobalChapter] == 1) {
			getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 1;
			CookCall(&LogicManager::CONS_Cook_DoDialog, "KIT1010", 0, 0, 0);
		} else if (_globals[kGlobalChapter] == 3) {
			getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 2;
			CookCall(&LogicManager::CONS_Cook_DoDialog, "KIT1012", 0, 0, 0);
		} else {
			playDialog(kCharacterCook, "KIT1011", -1, 0);
			getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 3;
			CookCall(&LogicManager::CONS_Cook_DoSeqOtis, "308B", 0, 0, 0);
		}

		break;
	case 17:
		if (!inKitchen(kCharacterCath)) {
			endGraphics(kCharacterCook);
			getCharacter(kCharacterCook).currentCall--;
			_engine->getMessageManager()->setMessageHandle(8, _functionsCook[getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall]]);
			fedEx(kCharacterCook, kCharacterCook, 18, 0);
		} else {
			if (checkCathDir(kCarRestaurant, 80)) {
				startCycOtis(kCharacterCook, "308D");
				if (!whoRunningDialog(kCharacterCook)) {
					if (getCharacterCurrentParams(kCharacterCook)[0]) {
						if (!whoOnScreen(kCharacterCook) && !whoRunningDialog(kCharacterCook)) {
							playDialog(kCharacterCook, "LIB015", -1, 0);
							endGraphics(kCharacterCook);
							getCharacter(kCharacterCook).currentCall--;
							_engine->getMessageManager()->setMessageHandle(kCharacterCook, _functionsCook[getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall]]);
							fedEx(kCharacterCook, kCharacterCook, 18, 0);
						}

						break;
					}

					playDialog(kCharacterCook, "KIT1011A", -1, 0);
					getCharacterCurrentParams(kCharacterCook)[0] = 1;
				}
			}

			if (getCharacterCurrentParams(kCharacterCook)[0]) {
				if (!whoOnScreen(kCharacterCook) && !whoRunningDialog(kCharacterCook)) {
					playDialog(kCharacterCook, "LIB015", -1, 0);
					endGraphics(kCharacterCook);
					getCharacter(kCharacterCook).currentCall--;
					_engine->getMessageManager()->setMessageHandle(kCharacterCook, _functionsCook[getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall]]);
					fedEx(kCharacterCook, kCharacterCook, 18, 0);
				}
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8]) {
			if (getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] > 2) {
				if (getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] == 3) {
					startCycOtis(kCharacterCook, "308C");
					releaseView(kCharacterCook, kCarRestaurant, 75);
					releaseView(kCharacterCook, kCarRestaurant, 78);
				}
			} else {
				playDialog(kCharacterCook, "KIT1011", -1, 0);
				getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 3;
				CookCall(&LogicManager::CONS_Cook_DoSeqOtis, "308B", 0, 0, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_Birth);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 5;

	params->clear();

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterCook)[0]) {
			getCharacterCurrentParams(kCharacterCook)[0] = 1;
			CONS_Cook_InKitchenDinner(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterCook).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterCook).characterPosition.position = 5900;
		getCharacter(kCharacterCook).characterPosition.location = 0;
		_globals[kGlobalUnknownDebugFlag] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_InKitchenDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_InKitchenDinner);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 6;

	params->clear();

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_InKitchenDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterCook)[3] || (getCharacterCurrentParams(kCharacterCook)[3] = _gameTime + getCharacterCurrentParams(kCharacterCook)[1], _gameTime + getCharacterCurrentParams(kCharacterCook)[1] != 0)) {
			if (getCharacterCurrentParams(kCharacterCook)[3] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterCook)[3] = 0x7FFFFFFF;
		}

		playDialog(0, "LIB122", getVolume(kCharacterCook), 0);
		getCharacterCurrentParams(kCharacterCook)[1] = 225 * (4 * rnd(30) + 120);
		getCharacterCurrentParams(kCharacterCook)[3] = 0;
		break;
	case 12:
		getCharacterCurrentParams(kCharacterCook)[1] = 225 * (4 * rnd(30) + 120);
		getCharacterCurrentParams(kCharacterCook)[0] = 1;
		break;
	case 17:
		if (inKitchen(kCharacterCath)) {
			if (getCharacterCurrentParams(kCharacterCook)[0]) {
				if (checkCathDir(kCarRedSleeping, 73)) {
					getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 1;
					CookCall(&LogicManager::CONS_Cook_UptrainVersion, 0, 0, 0, 0);
				}
			} else if (getCharacterCurrentParams(kCharacterCook)[2]) {
				getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 2;
				CookCall(&LogicManager::CONS_Cook_DoDialog, "ZFX1011", 0, 0, 0);
			} else {
				getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 3;
				CookCall(&LogicManager::CONS_Cook_DoDialog, "ZFX1012", 0, 0, 0);
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] == 1) {
			getCharacterCurrentParams(kCharacterCook)[0] = 0;
		} else if (getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] == 2 || getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] == 3) {
			getCharacterCurrentParams(kCharacterCook)[2] = (getCharacterCurrentParams(kCharacterCook)[2] == 0) ? 1 : 0;
		}

		break;
	case 101632192:
		CONS_Cook_LockUp(0, 0, 0, 0);
		break;
	case 224849280:
		_globals[kGlobalUnknownDebugFlag] = 1;
		getCharacterCurrentParams(kCharacterCook)[0] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_LockUp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_LockUp);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 7;

	params->clear();

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_LockUp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 1;
		CookCall(&LogicManager::CONS_Cook_DoDialog, "WAT1200", 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterCook).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterCook).characterPosition.position = 3650;
		getCharacter(kCharacterCook).characterPosition.location = 0;
		endGraphics(kCharacterCook);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_StartPart2);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 8;

	params->clear();

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Cook_InKitchenBreakfast(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterCook);
		getCharacter(kCharacterCook).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterCook).characterPosition.position = 5900;
		getCharacter(kCharacterCook).characterPosition.location = 0;
		getCharacter(kCharacterCook).inventoryItem = kItemNone;
		_globals[kGlobalUnknownDebugFlag] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_InKitchenBreakfast(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_InKitchenBreakfast);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 9;

	params->clear();

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_InKitchenBreakfast(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterCook)[2] || (getCharacterCurrentParams(kCharacterCook)[2] = _gameTime + getCharacterCurrentParams(kCharacterCook)[0], _gameTime + getCharacterCurrentParams(kCharacterCook)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterCook)[2] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterCook)[2] = 0x7FFFFFFF;
		}

		playDialog(0, "LIB122", getVolume(kCharacterCook), 0);
		getCharacterCurrentParams(kCharacterCook)[0] = 225 * (4 * rnd(30) + 120);
		getCharacterCurrentParams(kCharacterCook)[2] = 0;
		break;
	case 12:
		getCharacterCurrentParams(kCharacterCook)[0] = 225 * (4 * rnd(30) + 120);
		break;
	case 17:
		if (inKitchen(kCharacterCath)) {
			if (getCharacterCurrentParams(kCharacterCook)[1]) {
				getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 1;
				CookCall(&LogicManager::CONS_Cook_DoDialog, "ZFX1011", 0, 0, 0);
			} else {
				getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 2;
				CookCall(&LogicManager::CONS_Cook_DoDialog, "ZFX1012", 0, 0, 0);
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] && getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] <= 2)
			getCharacterCurrentParams(kCharacterCook)[1] = (getCharacterCurrentParams(kCharacterCook)[1] == 0) ? 1 : 0;

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_StartPart3);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 10;

	params->clear();

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Cook_InKitchenLunch(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterCook);
		getCharacter(kCharacterCook).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterCook).characterPosition.position = 5900;
		_globals[kGlobalUnknownDebugFlag] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_InKitchenLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_InKitchenLunch);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 11;

	params->clear();

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_InKitchenLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterCook)[3] || (getCharacterCurrentParams(kCharacterCook)[3] = _gameTime + getCharacterCurrentParams(kCharacterCook)[1], _gameTime + getCharacterCurrentParams(kCharacterCook)[1] != 0)) {
			if (getCharacterCurrentParams(kCharacterCook)[3] >= _gameTime) {
				if (_gameTime > 2079000 && !getCharacterCurrentParams(kCharacterCook)[4]) {
					getCharacterCurrentParams(kCharacterCook)[4] = 1;
					getCharacterCurrentParams(kCharacterCook)[0] = 0;
				}

				break;
			}

			getCharacterCurrentParams(kCharacterCook)[3] = 0x7FFFFFFF;
		}

		playDialog(0, "LIB122", getVolume(kCharacterCook), 0);
		getCharacterCurrentParams(kCharacterCook)[1] = 225 * (4 * rnd(30) + 120);
		getCharacterCurrentParams(kCharacterCook)[3] = 0;

		if (_gameTime > 2079000 && !getCharacterCurrentParams(kCharacterCook)[4]) {
			getCharacterCurrentParams(kCharacterCook)[4] = 1;
			getCharacterCurrentParams(kCharacterCook)[0] = 0;
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterCook)[1] = 225 * (4 * rnd(30) + 120);
		getCharacterCurrentParams(kCharacterCook)[0] = 1;
		break;
	case 17:
		if (inKitchen(kCharacterCath)) {
			if (getCharacterCurrentParams(kCharacterCook)[0]) {
				if (checkCathDir(kCarRestaurant, 80)) {
					getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 1;
					CookCall(&LogicManager::CONS_Cook_DowntrainVersion, 0, 0, 0, 0);
				}
			} else if (getCharacterCurrentParams(kCharacterCook)[2]) {
				getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 2;
				CookCall(&LogicManager::CONS_Cook_DoDialog, "ZFX1011", 0, 0, 0);
			} else {
				getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 3;
				CookCall(&LogicManager::CONS_Cook_DoDialog, "ZFX1012", 0, 0, 0);
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] == 1) {
			getCharacterCurrentParams(kCharacterCook)[0] = 0;
		} else if (getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] == 2 || getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] == 3) {
			getCharacterCurrentParams(kCharacterCook)[2] = (getCharacterCurrentParams(kCharacterCook)[2] == 0) ? 1 : 0;
		}

		break;
	case 236976550:
		_globals[kGlobalUnknownDebugFlag] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_StartPart4);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 12;

	params->clear();

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Cook_InKitchenDinner2(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterCook);
		getCharacter(kCharacterCook).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterCook).characterPosition.position = 5900;
		getCharacter(kCharacterCook).characterPosition.location = 0;
		getCharacter(kCharacterCook).inventoryItem = kItemNone;
		_globals[kGlobalUnknownDebugFlag] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_InKitchenDinner2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_InKitchenDinner2);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 13;

	params->clear();

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_InKitchenDinner2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterCook)[2] || (getCharacterCurrentParams(kCharacterCook)[2] = _gameTime + getCharacterCurrentParams(kCharacterCook)[0], _gameTime + getCharacterCurrentParams(kCharacterCook)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterCook)[2] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterCook)[2] = 0x7FFFFFFF;
		}

		playDialog(0, "LIB122", getVolume(kCharacterCook), 0);
		getCharacterCurrentParams(kCharacterCook)[0] = 225 * (4 * rnd(30) + 120);
		getCharacterCurrentParams(kCharacterCook)[2] = 0;
		break;
	case 12:
		getCharacterCurrentParams(kCharacterCook)[0] = 225 * (4 * rnd(30) + 120);
		break;
	case 17:
		if (inKitchen(kCharacterCath)) {
			if (getCharacterCurrentParams(kCharacterCook)[1]) {
				getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 1;
				CookCall(&LogicManager::CONS_Cook_DoDialog, "ZFX1011", 0, 0, 0);
			} else {
				getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] = 2;
				CookCall(&LogicManager::CONS_Cook_DoDialog, "ZFX1012", 0, 0, 0);
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] && getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall + 8] <= 2)
			getCharacterCurrentParams(kCharacterCook)[1] = (getCharacterCurrentParams(kCharacterCook)[1] == 0) ? 1 : 0;

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cook_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCook).callParams[getCharacter(kCharacterCook).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCook, &LogicManager::HAND_Cook_StartPart5);
	getCharacter(kCharacterCook).callbacks[getCharacter(kCharacterCook).currentCall] = 14;

	params->clear();

	fedEx(kCharacterCook, kCharacterCook, 12, 0);
}

void LogicManager::HAND_Cook_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterCook);
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsCook[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Cook_DoSeqOtis,
	&LogicManager::HAND_Cook_DoDialog,
	&LogicManager::HAND_Cook_UptrainVersion,
	&LogicManager::HAND_Cook_DowntrainVersion,
	&LogicManager::HAND_Cook_Birth,
	&LogicManager::HAND_Cook_InKitchenDinner,
	&LogicManager::HAND_Cook_LockUp,
	&LogicManager::HAND_Cook_StartPart2,
	&LogicManager::HAND_Cook_InKitchenBreakfast,
	&LogicManager::HAND_Cook_StartPart3,
	&LogicManager::HAND_Cook_InKitchenLunch,
	&LogicManager::HAND_Cook_StartPart4,
	&LogicManager::HAND_Cook_InKitchenDinner2,
	&LogicManager::HAND_Cook_StartPart5
};

} // End of namespace LastExpress
