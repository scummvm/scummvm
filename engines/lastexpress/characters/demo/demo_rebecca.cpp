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

void LogicManager::CONS_DemoRebecca(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterRebecca,
			_functionsDemoRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]
		);

		break;
	case 1:
		CONS_DemoRebecca_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoRebecca_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoRebecca_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoRebecca_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoRebecca_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::DemoRebeccaCall(CALL_PARAMS) {
	getCharacter(kCharacterRebecca).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_DemoRebecca_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_DemoRebecca_DoDialog);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 1;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_DemoRebecca_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterRebecca).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, _functionsDemoRebecca[getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall]]);
		fedEx(kCharacterRebecca, kCharacterRebecca, 18, 0);
		break;
	case 12:
		playDialog(kCharacterRebecca, (char *)&getCharacterCurrentParams(kCharacterRebecca)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoRebecca_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_DemoRebecca_Birth);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 2;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_DemoRebecca_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterRebecca)[0] || getCharacterCurrentParams(kCharacterRebecca)[1] >= 3 || !inDiningRoom(kCharacterCath)) {
			if (_gameTime > 2254500 && !getCharacterCurrentParams(kCharacterRebecca)[3]) {
				getCharacterCurrentParams(kCharacterRebecca)[3] = 1;
				send(kCharacterRebecca, kCharacterWaiter1, 223712416, 0);
				getCharacterCurrentParams(kCharacterRebecca)[0] = 1;
			}

			break;
		}

		if (getCharacterCurrentParams(kCharacterRebecca)[2] || (getCharacterCurrentParams(kCharacterRebecca)[2] = _gameTime + 150, _gameTime != -150)) {
			if (_gameTime <= getCharacterCurrentParams(kCharacterRebecca)[2]) {
				if (_gameTime > 2254500 && !getCharacterCurrentParams(kCharacterRebecca)[3]) {
					getCharacterCurrentParams(kCharacterRebecca)[3] = 1;
					send(kCharacterRebecca, kCharacterWaiter1, 223712416, 0);
					getCharacterCurrentParams(kCharacterRebecca)[0] = 1;
				}

				break;
			}

			getCharacterCurrentParams(kCharacterRebecca)[2] = 0x7FFFFFFF;
		}

		if (getCharacterCurrentParams(kCharacterRebecca)[1] == 1) {
			getCharacterCurrentParams(kCharacterRebecca)[1]++;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 1;
			DemoRebeccaCall(&LogicManager::CONS_DemoRebecca_DoDialog, "REB1013", 0, 0, 0);
		} else if (getCharacterCurrentParams(kCharacterRebecca)[1] == 2) {
			getCharacterCurrentParams(kCharacterRebecca)[1]++;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 2;
			DemoRebeccaCall(&LogicManager::CONS_DemoRebecca_DoDialog, "REB1199A", 0, 0, 0);
		} else if (getCharacterCurrentParams(kCharacterRebecca)[1] == 3) {
			getCharacterCurrentParams(kCharacterRebecca)[1]++;
			getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 3;
			DemoRebeccaCall(&LogicManager::CONS_DemoRebecca_DoDialog, "REB1199C", 0, 0, 0);
		} else {
			getCharacterCurrentParams(kCharacterRebecca)[1]++;
			getCharacterCurrentParams(kCharacterRebecca)[2] = 0;
			if (_gameTime > 2254500 && !getCharacterCurrentParams(kCharacterRebecca)[3]) {
				getCharacterCurrentParams(kCharacterRebecca)[3] = 1;
				send(kCharacterRebecca, kCharacterWaiter1, 223712416, 0);
				getCharacterCurrentParams(kCharacterRebecca)[0] = 1;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterRebecca).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterRebecca).characterPosition.location = 1;
		getCharacter(kCharacterRebecca).characterPosition.position = 4690;
		startCycOtis(kCharacterRebecca, "012D");
		break;
	case 18:
		switch (getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8]) {
		case 1:
		case 2:
		case 3:
			getCharacterCurrentParams(kCharacterRebecca)[2] = 0;
			if (_gameTime > 2254500 && !getCharacterCurrentParams(kCharacterRebecca)[3]) {
				getCharacterCurrentParams(kCharacterRebecca)[3] = 1;
				send(kCharacterRebecca, kCharacterWaiter1, 223712416, 0);
				getCharacterCurrentParams(kCharacterRebecca)[0] = 1;
			}

			break;
		case 4:
			send(kCharacterRebecca, kCharacterWaiter1, 136702400, 0);
			startCycOtis(kCharacterRebecca, "012G");
			getCharacterCurrentParams(kCharacterRebecca)[0] = 0;
			break;
		default:
			break;
		}

		break;
	case 123712592:
		startCycOtis(kCharacterWaiter1, "BLANK");
		startCycOtis(kCharacterRebecca, "012E");
		getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall + 8] = 4;
		DemoRebeccaCall(&LogicManager::CONS_DemoRebecca_DoDialog, "REB1200", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoRebecca_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_DemoRebecca_StartPart2);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 3;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_DemoRebecca_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoRebecca_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_DemoRebecca_StartPart3);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 4;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_DemoRebecca_StartPart3(HAND_PARAMS) {
	// No-op

}

void LogicManager::CONS_DemoRebecca_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_DemoRebecca_StartPart4);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 5;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_DemoRebecca_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoRebecca_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterRebecca).callParams[getCharacter(kCharacterRebecca).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterRebecca, &LogicManager::HAND_DemoRebecca_StartPart5);
	getCharacter(kCharacterRebecca).callbacks[getCharacter(kCharacterRebecca).currentCall] = 6;

	params->clear();

	fedEx(kCharacterRebecca, kCharacterRebecca, 12, 0);
}

void LogicManager::HAND_DemoRebecca_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoRebecca[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoRebecca_DoDialog,
	&LogicManager::HAND_DemoRebecca_Birth,
	&LogicManager::HAND_DemoRebecca_StartPart2,
	&LogicManager::HAND_DemoRebecca_StartPart3,
	&LogicManager::HAND_DemoRebecca_StartPart4,
	&LogicManager::HAND_DemoRebecca_StartPart5
};

} // End of namespace LastExpress
