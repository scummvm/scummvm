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
void LogicManager::CONS_DemoAbbot(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterAbbot,
			_functionsDemoAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]
		);

		break;
	case 1:
		CONS_DemoAbbot_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoAbbot_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoAbbot_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoAbbot_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoAbbot_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::DemoAbbotCall(CALL_PARAMS) {
	getCharacter(kCharacterAbbot).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_DemoAbbot_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_DemoAbbot_DoWait);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 1;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_DemoAbbot_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAbbot)[1]) {
			if (getCharacterCurrentParams(kCharacterAbbot)[1] >= _gameTime)
				return;

			getCharacterCurrentParams(kCharacterAbbot)[1] = 0x7FFFFFFF;
		} else {
			getCharacterCurrentParams(kCharacterAbbot)[1] = _gameTime + getCharacterCurrentParams(kCharacterAbbot)[0];

			if (_gameTime + getCharacterCurrentParams(kCharacterAbbot)[0] == 0)
				return;

			if (_gameTime <= getCharacterCurrentParams(kCharacterAbbot)[1])
				return;

			getCharacterCurrentParams(kCharacterAbbot)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsDemoAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);

		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAbbot_DoJoinedSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_DemoAbbot_DoJoinedSeqOtis);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	strncpy((char *)&params->parameters[5], param4.stringParam, 12);

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_DemoAbbot_DoJoinedSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		if (!getCharacterCurrentParams(kCharacterAbbot)[8])
			fedEx(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[3], getCharacterCurrentParams(kCharacterAbbot)[4], getCharacterCurrentParams(kCharacterAbbot)[5]);

		getCharacter(kCharacterAbbot).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsDemoAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);
		fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);

		break;
	case 10:
		if (!getCharacterCurrentParams(kCharacterAbbot)[8]) {
			fedEx(kCharacterAbbot, getCharacterCurrentParams(kCharacterAbbot)[3], getCharacterCurrentParams(kCharacterAbbot)[4], getCharacterCurrentParams(kCharacterAbbot)[5]);
			getCharacterCurrentParams(kCharacterAbbot)[8] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterAbbot, (char *)&getCharacterCurrentParams(kCharacterAbbot));
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAbbot_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_DemoAbbot_WaitRCClear);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 3;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_DemoAbbot_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterAbbot).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, _functionsDemoAbbot[getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall]]);

			fedEx(kCharacterAbbot, kCharacterAbbot, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAbbot_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_DemoAbbot_Birth);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 4;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_DemoAbbot_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAbbot)[0] == 0x7FFFFFFF || _gameTime <= 2248200)
			return;

		if (_gameTime <= 2259000) {
			if (!inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterAbbot)[0]) {
				getCharacterCurrentParams(kCharacterAbbot)[0] = _gameTime + 225;
				if (_gameTime == -225) {
					send(kCharacterAbbot, kCharacterAugust, 136196244, 0);
					getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
					AbbotCall(&LogicManager::CONS_DemoAbbot_DoWait, 0, 0, 0, 0);
					return;
				}
			}

			if (getCharacterCurrentParams(kCharacterAbbot)[0] >= _gameTime)
				return;
		}

		getCharacterCurrentParams(kCharacterAbbot)[0] = 0x7FFFFFFF;
		send(kCharacterAbbot, kCharacterAugust, 136196244, 0);
		getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 1;
		AbbotCall(&LogicManager::CONS_DemoAbbot_DoWait, 0, 0, 0, 0);

		break;
	case 12:
		getCharacter(kCharacterAbbot).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterAbbot).characterPosition.position = 3450;
		getCharacter(kCharacterAbbot).characterPosition.location = 1;
		startCycOtis(kCharacterAbbot, "115B");
		break;
	case 18:
		switch (getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 2;
			AbbotCall(&LogicManager::CONS_DemoAbbot_WaitRCClear, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAbbot).characterPosition.location = 0;
			playDialog(kCharacterAbbot, "Abb3demo", -1, 45);
			blockView(kCharacterAbbot, kCarRestaurant, 50);
			getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall + 8] = 3;
			AbbotCall(&LogicManager::CONS_DemoAbbot_DoJoinedSeqOtis, "121A", 2, 122358304, "BOGUS");
			break;
		case 3:
			releaseView(kCharacterAbbot, kCarRestaurant, 50);
			getCharacter(kCharacterAbbot).characterPosition.location = 1;
			startCycOtis(kCharacterAbbot, "121B");
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAbbot_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_DemoAbbot_StartPart2);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 5;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_DemoAbbot_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoAbbot_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_DemoAbbot_StartPart3);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 6;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_DemoAbbot_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoAbbot_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_DemoAbbot_StartPart4);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 7;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_DemoAbbot_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoAbbot_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAbbot).callParams[getCharacter(kCharacterAbbot).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAbbot, &LogicManager::HAND_DemoAbbot_StartPart5);
	getCharacter(kCharacterAbbot).callbacks[getCharacter(kCharacterAbbot).currentCall] = 8;

	params->clear();

	fedEx(kCharacterAbbot, kCharacterAbbot, 12, 0);
}

void LogicManager::HAND_DemoAbbot_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoAbbot[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoAbbot_DoWait,
	&LogicManager::HAND_DemoAbbot_DoJoinedSeqOtis,
	&LogicManager::HAND_DemoAbbot_WaitRCClear,
	&LogicManager::HAND_DemoAbbot_Birth,
	&LogicManager::HAND_DemoAbbot_StartPart2,
	&LogicManager::HAND_DemoAbbot_StartPart3,
	&LogicManager::HAND_DemoAbbot_StartPart4,
	&LogicManager::HAND_DemoAbbot_StartPart5
};

} // End of namespace LastExpress
