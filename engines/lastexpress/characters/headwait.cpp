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

void LogicManager::CONS_HeadWait(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterHeadWait,
			_functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]
		);

		break;
	case 1:
		CONS_HeadWait_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_HeadWait_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_HeadWait_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_HeadWait_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_HeadWait_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::HeadWaitCall(CALL_PARAMS) {
	getCharacter(kCharacterHeadWait).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_HeadWait_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_DoSeqOtis);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 1;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterHeadWait).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
		fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterHeadWait)[3]) {
			playChrExcuseMe(kCharacterHeadWait, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterHeadWait)[3] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterHeadWait, (char *)&getCharacterCurrentParams(kCharacterHeadWait)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_WaitRCClear);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 2;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterHeadWait).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
			fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_FinishSeqOtis);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 3;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterHeadWait).direction != 4) {
			getCharacter(kCharacterHeadWait).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
			fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterHeadWait).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
		fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterHeadWait)[0]) {
			playChrExcuseMe(kCharacterHeadWait, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterHeadWait)[0] = 1;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_DoWait);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 4;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterHeadWait)[1] || (getCharacterCurrentParams(kCharacterHeadWait)[1] = _gameTime + getCharacterCurrentParams(kCharacterHeadWait)[0], _gameTime + getCharacterCurrentParams(kCharacterHeadWait)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterHeadWait)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterHeadWait)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterHeadWait).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
		fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_DoBlockSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_DoBlockSeqOtis);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_DoBlockSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseView(kCharacterHeadWait, getCharacterCurrentParams(kCharacterHeadWait)[3], getCharacterCurrentParams(kCharacterHeadWait)[4]);

		getCharacter(kCharacterHeadWait).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
		fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterHeadWait)[5]) {
			playChrExcuseMe(kCharacterHeadWait, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterHeadWait)[5] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterHeadWait, (char *)&getCharacterCurrentParams(kCharacterHeadWait)[0]);
		blockView(kCharacterHeadWait, getCharacterCurrentParams(kCharacterHeadWait)[3], getCharacterCurrentParams(kCharacterHeadWait)[4]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_DoDialog);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterHeadWait).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
		fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		break;
	case 12:
		playDialog(kCharacterHeadWait, (char *)&getCharacterCurrentParams(kCharacterHeadWait)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_DoComplexSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_DoComplexSeqOtis);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 7;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_DoComplexSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterHeadWait).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
		fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterHeadWait, (char *)&getCharacterCurrentParams(kCharacterHeadWait)[0]);
		startSeqOtis(getCharacterCurrentParams(kCharacterHeadWait)[6], (char *)&getCharacterCurrentParams(kCharacterHeadWait)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_RebeccaHereWeAre(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_RebeccaHereWeAre);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 8;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_RebeccaHereWeAre(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHeadWait).characterPosition.position = 850;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
		HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "901", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8]) {
		case 1:
			switch (_globals[kGlobalChapter]) {
			case 1:
				playDialog(kCharacterHeadWait, "REB1198", -1, 30);
				break;
			case 3:
				playDialog(kCharacterHeadWait, "REB3001", -1, 30);
				break;
			case 4:
				playDialog(kCharacterHeadWait, "REB4001", -1, 30);
				break;
			default:
				break;
			}

			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoOtis5012C, 0, 0, 0, 0);
			break;
		case 2:
			send(kCharacterHeadWait, kCharacterRebecca, 157370960, 0);
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 3;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "905", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterHeadWait);
			getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
			getCharacterParams(kCharacterHeadWait, 8)[3] = 0;

			getCharacter(kCharacterHeadWait).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
			fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_DoOtis5012C(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_DoOtis5012C);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 9;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_DoOtis5012C(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterHeadWait).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
		fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterHeadWait, "012C1");
		startSeqOtis(kCharacterRebecca, "012C2");
		startSeqOtis(kCharacterTableD, "012C3");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_DoSeatOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_DoSeatOtis);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 10;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_DoSeatOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterHeadWait)[0]) {
			if (!whoRunningDialog(kCharacterHeadWait))
				releaseView(kCharacterHeadWait, kCarRestaurant, 64);
		}

		break;
	case 3:
		if (!getCharacterCurrentParams(kCharacterHeadWait)[1]) {
			getCharacterCurrentParams(kCharacterHeadWait)[1] = 1;
			playDialog(kCharacterHeadWait, "HED1001A", -1, 0);
			playDialog(0, "LIB004", -1, 0);
			bumpCath(kCarRestaurant, 69, 255);
		}

		getCharacter(kCharacterHeadWait).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
		fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		break;
	case 4:
		if (!getCharacterCurrentParams(kCharacterHeadWait)[0]) {
			getCharacterCurrentParams(kCharacterHeadWait)[0] = 1;
			playDialog(kCharacterHeadWait, "HED1001", -1, 0);
		}

		break;
	case 12:
		blockView(kCharacterHeadWait, kCarRestaurant, 64);
		startSeqOtis(kCharacterHeadWait, "035A");
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterHeadWait)[0] && checkCathDir(kCarRestaurant, 64)) {
			playDialog(kCharacterHeadWait, "HED1001A", -1, 0);
			playDialog(0, "LIB004", -1, 0);
			bumpCath(kCarRestaurant, 69, 255);

			getCharacter(kCharacterHeadWait).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
			fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_SeatCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_SeatCath);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 11;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_SeatCath(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHeadWait).characterPosition.position = 5800;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		send(kCharacterHeadWait, kCharacterAugust, 168046720, 0);
		send(kCharacterHeadWait, kCharacterAnna, 168046720, 0);
		send(kCharacterHeadWait, kCharacterAlexei, 168046720, 0);
		blockView(kCharacterHeadWait, kCarRestaurant, 55);
		getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
		HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeatOtis, 0, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] == 1) {
			send(kCharacterHeadWait, kCharacterAugust, 168627977, 0);
			send(kCharacterHeadWait, kCharacterAnna, 168627977, 0);
			send(kCharacterHeadWait, kCharacterAlexei, 168627977, 0);
			releaseView(kCharacterHeadWait, kCarRestaurant, 55);
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "905", 0, 0, 0);
		} else if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] == 2) {
			endGraphics(kCharacterHeadWait);
			getCharacter(kCharacterHeadWait).characterPosition.position = 5900;

			getCharacter(kCharacterHeadWait).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
			fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_Birth);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 12;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_HeadWait_ServingDinner(0, 0, 0, 0);
		break;
	case 12:
		autoMessage(kCharacterHeadWait, 239072064, 0);
		autoMessage(kCharacterHeadWait, 257489762, 2);
		autoMessage(kCharacterHeadWait, 207769280, 6);
		autoMessage(kCharacterHeadWait, 101824388, 7);
		autoMessage(kCharacterHeadWait, 136059947, 8);
		autoMessage(kCharacterHeadWait, 223262556, 1);
		autoMessage(kCharacterHeadWait, 269479296, 3);
		autoMessage(kCharacterHeadWait, 352703104, 4);
		autoMessage(kCharacterHeadWait, 352768896, 5);
		autoMessage(kCharacterHeadWait, 191604416, 10);
		autoMessage(kCharacterHeadWait, 190605184, 11);
		getCharacter(kCharacterHeadWait).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_GreetAugust(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_GreetAugust);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 13;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_GreetAugust(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHeadWait).characterPosition.position = 5800;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
		HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "902", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] == 1) {
			if (!getCharacterParams(kCharacterHeadWait, 8)[10]) {
				startCycOtis(kCharacterHeadWait, "010E");
				startCycOtis(kCharacterAugust, "BLANK");
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
				HeadWaitCall(&LogicManager::CONS_HeadWait_DoDialog, "AUG1001", 0, 0, 0);
				break;
			}
		} else {
			if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] != 2) {
				if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] == 3) {
					getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
					endGraphics(kCharacterHeadWait);
					send(kCharacterHeadWait, kCharacterTrainM, 191337656, 0);
					getCharacterParams(kCharacterHeadWait, 8)[0] = 0;

					getCharacter(kCharacterHeadWait).currentCall--;
					_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
					fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
				}

				break;
			}

			startCycOtis(kCharacterAugust, "010B");
		}

		getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 3;
		HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "905", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_DoOtis5001C(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_DoOtis5001C);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 14;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_DoOtis5001C(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseView(kCharacterHeadWait, kCarRestaurant, 62);
		getCharacter(kCharacterHeadWait).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
		fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterTableA, "001C3");
		startSeqOtis(kCharacterAnna, "001C2");
		startSeqOtis(kCharacterHeadWait, "001C1");
		blockView(kCharacterHeadWait, kCarRestaurant, 62);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_GreetAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_GreetAnna);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 15;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_GreetAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHeadWait).characterPosition.position = 5800;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
		HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "901", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8]) {
		case 1:
			playDialog(kCharacterHeadWait, "ANN1047", -1, 0);
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoOtis5001C, 0, 0, 0, 0);
			break;
		case 2:
			send(kCharacterHeadWait, kCharacterAnna, 157370960, 0);
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 3;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "904", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterHeadWait);
			getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
			getCharacterParams(kCharacterHeadWait, 8)[1] = 0;

			getCharacter(kCharacterHeadWait).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
			fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_GreetTatiana(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_GreetTatiana);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 16;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_GreetTatiana(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHeadWait).characterPosition.position = 5800;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
		HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "903", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8]) {
		case 1:
			send(kCharacterHeadWait, kCharacterTatiana, 122358304, 0);
			startCycOtis(kCharacterHeadWait, "014B");
			blockView(kCharacterHeadWait, kCarRestaurant, 67);

			if (dialogRunning("TAT1069A")) {
				fadeDialog("TAT1069A");
			} else if (dialogRunning("TAT1069B")) {
				fadeDialog("TAT1069B");
			}

			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoDialog, "TAT1066", 0, 0, 0);
			break;
		case 2:
			releaseView(kCharacterHeadWait, kCarRestaurant, 67);
			send(kCharacterHeadWait, kCharacterTatiana, 122288808, 0);
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 3;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "906", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterHeadWait);
			getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
			getCharacterParams(kCharacterHeadWait, 8)[2] = 0;

			getCharacter(kCharacterHeadWait).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
			fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_ServingDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_ServingDinner);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 17;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_ServingDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterHeadWait)[1]) {
			if (checkCathDir(kCarRestaurant, 69) || checkCathDir(kCarRestaurant, 70) || checkCathDir(kCarRestaurant, 71)) {
				getCharacterCurrentParams(kCharacterHeadWait)[1] = 1;
			}

			if (!getCharacterCurrentParams(kCharacterHeadWait)[1] && checkCathDir(kCarRestaurant, 61))
				getCharacterCurrentParams(kCharacterHeadWait)[0] = 1;
		}

		if (inKitchen(kCharacterHeadWait)) {
			if (getCharacterParams(kCharacterHeadWait, 8)[4] && getCharacterParams(kCharacterHeadWait, 8)[5]) {
				CONS_HeadWait_AfterDinner(0, 0, 0, 0);
				break;
			}

			if (rcClear()) {
				if (getCharacterCurrentParams(kCharacterHeadWait)[0] && !getCharacterCurrentParams(kCharacterHeadWait)[1] && checkCathDir(kCarRestaurant, 61)) {
					getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
					HeadWaitCall(&LogicManager::CONS_HeadWait_SeatCath, 0, 0, 0, 0);
					break;
				}

				if (getCharacterParams(kCharacterHeadWait, 8)[0] && !getCharacterParams(kCharacterHeadWait, 8)[10]) {
					getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
					HeadWaitCall(&LogicManager::CONS_HeadWait_GreetAugust, 0, 0, 0, 0);
					break;
				}

				if (getCharacterParams(kCharacterHeadWait, 8)[2]) {
					getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 3;
					HeadWaitCall(&LogicManager::CONS_HeadWait_GreetTatiana, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterParams(kCharacterHeadWait, 8)[1]) {
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 4;
				HeadWaitCall(&LogicManager::CONS_HeadWait_GreetAnna, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterHeadWait, 8)[3]) {
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 5;
				HeadWaitCall(&LogicManager::CONS_HeadWait_RebeccaHereWeAre, 0, 0, 0, 0);
				break;
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8]) {
		case 1:
			getCharacterCurrentParams(kCharacterHeadWait)[1] = 1;
			getCharacterCurrentParams(kCharacterHeadWait)[0] = 0;

			if (getCharacterParams(kCharacterHeadWait, 8)[0] && !getCharacterParams(kCharacterHeadWait, 8)[10]) {
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
				HeadWaitCall(&LogicManager::CONS_HeadWait_GreetAugust, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacterParams(kCharacterHeadWait, 8)[2]) {
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 3;
				HeadWaitCall(&LogicManager::CONS_HeadWait_GreetTatiana, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (getCharacterParams(kCharacterHeadWait, 8)[1]) {
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 4;
				HeadWaitCall(&LogicManager::CONS_HeadWait_GreetAnna, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (getCharacterParams(kCharacterHeadWait, 8)[3]) {
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 5;
				HeadWaitCall(&LogicManager::CONS_HeadWait_RebeccaHereWeAre, 0, 0, 0, 0);
			}

			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_AfterDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_AfterDinner);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 18;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_AfterDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1242000 && !getCharacterCurrentParams(kCharacterHeadWait)[0]) {
			getCharacterCurrentParams(kCharacterHeadWait)[0] = 1;
			send(kCharacterHeadWait, kCharacterWaiter1, 101632192, 0);
			send(kCharacterHeadWait, kCharacterWaiter2, 101632192, 0);
			send(kCharacterHeadWait, kCharacterCook, 101632192, 0);
			send(kCharacterHeadWait, kCharacterTrainM, 101632192, 0);
			CONS_HeadWait_LockUp(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_LockUp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_LockUp);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 19;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_LockUp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterHeadWait)[0] && getCharacter(kCharacterCath).characterPosition.position < 3650) {
			setDoor(65, kCharacterCath, 1, 10, 9);
			send(kCharacterHeadWait, kCharacterTableA, 103798704, "001P");
			send(kCharacterHeadWait, kCharacterTableB, 103798704, "005J");
			send(kCharacterHeadWait, kCharacterTableC, 103798704, "009G");
			send(kCharacterHeadWait, kCharacterTableD, 103798704, "010M");
			send(kCharacterHeadWait, kCharacterTableE, 103798704, "014F");
			send(kCharacterHeadWait, kCharacterTableF, 103798704, "024D");
			getCharacterCurrentParams(kCharacterHeadWait)[0] = 1;
		}

		break;
	case 12:
		getCharacter(kCharacterHeadWait).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		endGraphics(kCharacterHeadWait);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_StartPart2);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 20;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterHeadWait);
		getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		getCharacter(kCharacterHeadWait).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterHeadWait).inventoryItem = 0;
		getCharacter(kCharacterHeadWait).clothes = 1;
		setDoor(65, kCharacterCath, 0, 0, 1);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_StartPart3);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 21;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_HeadWait_InPart3(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterHeadWait);
		getCharacter(kCharacterHeadWait).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		getCharacter(kCharacterHeadWait).inventoryItem = 0;
		getCharacterParams(kCharacterHeadWait, 8)[6] = 0;
		getCharacterParams(kCharacterHeadWait, 8)[3] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_InPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_InPart3);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 22;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_InPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inKitchen(kCharacterHeadWait)) {
			if (getCharacterParams(kCharacterHeadWait, 8)[6]) {
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
				HeadWaitCall(&LogicManager::CONS_HeadWait_AbbotSeatMe3, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterHeadWait, 8)[3]) {
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
				HeadWaitCall(&LogicManager::CONS_HeadWait_RebeccaHereWeAre, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] == 1) {
			if (getCharacterParams(kCharacterHeadWait, 8)[3]) {
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
				HeadWaitCall(&LogicManager::CONS_HeadWait_RebeccaHereWeAre, 0, 0, 0, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_AbbotSeatMe3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_AbbotSeatMe3);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 23;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_AbbotSeatMe3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHeadWait).characterPosition.position = 5800;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		blockView(kCharacterHeadWait, kCarRestaurant, 67);
		getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
		HeadWaitCall(&LogicManager::CONS_HeadWait_DoOtis5029A, 0, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] == 1) {
			releaseView(kCharacterHeadWait, kCarRestaurant, 67);
			send(kCharacterHeadWait, kCharacterAbbot, 122288808, 0);
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "906", 0, 0, 0);
		} else if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] == 2) {
			getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
			getCharacterParams(kCharacterHeadWait, 8)[6] = 0;
			endGraphics(kCharacterHeadWait);

			getCharacter(kCharacterHeadWait).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
			fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_DoOtis5029A(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_DoOtis5029A);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 24;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_DoOtis5029A(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		if (!getCharacterCurrentParams(kCharacterHeadWait)[0]) {
			playDialog(kCharacterHeadWait, "ABB3015A", -1, 0);
			getCharacterCurrentParams(kCharacterHeadWait)[0] = 1;
		}

		break;
	case 3:
		getCharacter(kCharacterHeadWait).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
		fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		break;
	case 10:
		send(kCharacterHeadWait, kCharacterTableE, 136455232, 0);
		break;
	case 12:
		playDialog(kCharacterHeadWait, "ABB3015", -1, 105);
		startSeqOtis(kCharacterHeadWait, "029A1");
		startSeqOtis(kCharacterAbbot, "029A2");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_StartPart4);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 25;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_HeadWait_InPart4(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterHeadWait);
		getCharacter(kCharacterHeadWait).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		getCharacter(kCharacterHeadWait).inventoryItem = 0;
		getCharacterParams(kCharacterHeadWait, 8)[7] = 0;
		getCharacterParams(kCharacterHeadWait, 8)[8] = 0;
		getCharacterParams(kCharacterHeadWait, 8)[3] = 0;
		getCharacterParams(kCharacterHeadWait, 8)[9] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_InPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_InPart4);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 26;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_InPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2511000 && !getCharacterCurrentParams(kCharacterHeadWait)[3]) {
			getCharacterCurrentParams(kCharacterHeadWait)[3] = 1;
			getCharacterCurrentParams(kCharacterHeadWait)[1] = 1;
		}

		if (inKitchen(kCharacterHeadWait)) {
			if (rcClear()) {
				if (getCharacterParams(kCharacterHeadWait, 8)[7]) {
					getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
					HeadWaitCall(&LogicManager::CONS_HeadWait_MeetCond2, 0, 0, 0, 0);
					break;
				}

				if (getCharacterParams(kCharacterHeadWait, 8)[9] && getCharacterParams(kCharacterHeadWait, 8)[11]) {
					if (!getCharacterCurrentParams(kCharacterHeadWait)[2])
						getCharacterCurrentParams(kCharacterHeadWait)[2] = _gameTime + 9000;

					if (getCharacterCurrentParams(kCharacterHeadWait)[4] != 0x7FFFFFFF && _gameTime) {
						if (getCharacterCurrentParams(kCharacterHeadWait)[2] < _gameTime) {
							getCharacterCurrentParams(kCharacterHeadWait)[4] = 0x7FFFFFFF;
							getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
							HeadWaitCall(&LogicManager::CONS_HeadWait_TellAug, 0, 0, 0, 0);
							break;
						}

						if (!inDiningRoom(kCharacterCath) || !getCharacterCurrentParams(kCharacterHeadWait)[4]) {
							getCharacterCurrentParams(kCharacterHeadWait)[4] = _gameTime;
							if (!_gameTime) {
								getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
								HeadWaitCall(&LogicManager::CONS_HeadWait_TellAug, 0, 0, 0, 0);
								break;
							}
						}

						if (getCharacterCurrentParams(kCharacterHeadWait)[4] < _gameTime) {
							getCharacterCurrentParams(kCharacterHeadWait)[4] = 0x7FFFFFFF;
							getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
							HeadWaitCall(&LogicManager::CONS_HeadWait_TellAug, 0, 0, 0, 0);
							break;
						}
					}
				}

				if (getCharacterCurrentParams(kCharacterHeadWait)[0] && !getCharacterCurrentParams(kCharacterHeadWait)[1] && checkCathDir(kCarRestaurant, 61)) {
					getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 3;
					HeadWaitCall(&LogicManager::CONS_HeadWait_SeatCath, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterParams(kCharacterHeadWait, 8)[3]) {
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 4;
				HeadWaitCall(&LogicManager::CONS_HeadWait_RebeccaHereWeAre, 0, 0, 0, 0);
			}

			break;
		}

		break;
	case 12:
		if (checkCathDir(kCarRestaurant, 69) || checkCathDir(kCarRestaurant, 70) || checkCathDir(kCarRestaurant, 71)) {
			getCharacterCurrentParams(kCharacterHeadWait)[1] = 1;
		}

		break;
	case 17:
		if (!getCharacterCurrentParams(kCharacterHeadWait)[1]) {
			if (checkCathDir(kCarRestaurant, 69) || checkCathDir(kCarRestaurant, 70) || checkCathDir(kCarRestaurant, 71)) {
				getCharacterCurrentParams(kCharacterHeadWait)[1] = 1;
			}

			if (!getCharacterCurrentParams(kCharacterHeadWait)[1] && checkCathDir(kCarRestaurant, 61))
				getCharacterCurrentParams(kCharacterHeadWait)[0] = 1;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8]) {
		case 1:
			if (getCharacterParams(kCharacterHeadWait, 8)[9] && getCharacterParams(kCharacterHeadWait, 8)[11]) {
				if (!getCharacterCurrentParams(kCharacterHeadWait)[2])
					getCharacterCurrentParams(kCharacterHeadWait)[2] = _gameTime + 9000;

				if (getCharacterCurrentParams(kCharacterHeadWait)[4] != 0x7FFFFFFF && _gameTime) {
					if (getCharacterCurrentParams(kCharacterHeadWait)[2] < _gameTime) {
						getCharacterCurrentParams(kCharacterHeadWait)[4] = 0x7FFFFFFF;
						getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
						HeadWaitCall(&LogicManager::CONS_HeadWait_TellAug, 0, 0, 0, 0);
						break;
					}

					if (!inDiningRoom(kCharacterCath) || !getCharacterCurrentParams(kCharacterHeadWait)[4]) {
						getCharacterCurrentParams(kCharacterHeadWait)[4] = _gameTime;
						if (!_gameTime) {
							getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
							HeadWaitCall(&LogicManager::CONS_HeadWait_TellAug, 0, 0, 0, 0);
							break;
						}
					}

					if (getCharacterCurrentParams(kCharacterHeadWait)[4] < _gameTime) {
						getCharacterCurrentParams(kCharacterHeadWait)[4] = 0x7FFFFFFF;
						getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
						HeadWaitCall(&LogicManager::CONS_HeadWait_TellAug, 0, 0, 0, 0);
						break;
					}
				}
			}

			// fall through
		case 2:
			if (getCharacterCurrentParams(kCharacterHeadWait)[0] && !getCharacterCurrentParams(kCharacterHeadWait)[1] && checkCathDir(kCarRestaurant, 61)) {
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 3;
				HeadWaitCall(&LogicManager::CONS_HeadWait_SeatCath, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] == 3) {
				getCharacterCurrentParams(kCharacterHeadWait)[1] = 1;
				getCharacterCurrentParams(kCharacterHeadWait)[0] = 0;
			}

			if (getCharacterParams(kCharacterHeadWait, 8)[3]) {
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 4;
				HeadWaitCall(&LogicManager::CONS_HeadWait_RebeccaHereWeAre, 0, 0, 0, 0);
			}

			break;
		default:
			break;
		}

		break;
	case 201431954:
		getCharacterParams(kCharacterHeadWait, 8)[7] = 0;
		getCharacterParams(kCharacterHeadWait, 8)[3] = 0;
		send(kCharacterHeadWait, kCharacterTableA, 103798704, "001P");
		send(kCharacterHeadWait, kCharacterTableB, 103798704, "005J");
		send(kCharacterHeadWait, kCharacterTableC, 103798704, "009G");
		send(kCharacterHeadWait, kCharacterTableD, 103798704, "010M");
		send(kCharacterHeadWait, kCharacterTableE, 103798704, "014F");
		send(kCharacterHeadWait, kCharacterTableF, 103798704, "024D");
		getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_MeetCond2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_MeetCond2);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 27;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_MeetCond2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterParams(kCharacterHeadWait, 8)[8]) {
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoWait, 450, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
		HeadWaitCall(&LogicManager::CONS_HeadWait_RSWalkDowntrain, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8]) {
		case 1:
			endGraphics(kCharacterHeadWait);
			break;
		case 2:
			send(kCharacterHeadWait, kCharacterCond2, 123712592, 0);
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 3;
			HeadWaitCall(&LogicManager::CONS_HeadWait_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 4;
			HeadWaitCall(&LogicManager::CONS_HeadWait_RSWalkUptrain, 0, 0, 0, 0);
			break;
		case 4:
			endGraphics(kCharacterHeadWait);
			getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
			getCharacterParams(kCharacterHeadWait, 8)[7] = 0;
			getCharacterParams(kCharacterHeadWait, 8)[8] = 0;
			getCharacterParams(kCharacterHeadWait, 8)[9] = 1;

			getCharacter(kCharacterHeadWait).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
			fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_TellAug(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_TellAug);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 28;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_TellAug(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHeadWait).characterPosition.position = 5800;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
		HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "902", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8]) {
		case 1:
			send(kCharacterHeadWait, kCharacterAugust, 122358304, 0);
			startCycOtis(kCharacterHeadWait, "010E2");
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoDialog, "Aug4001", 0, 0, 0);
			break;
		case 2:
			send(kCharacterHeadWait, kCharacterAugust, 123793792, 0);
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 3;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "905", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterHeadWait);
			getCharacter(kCharacterHeadWait).characterPosition.position = 5900;
			getCharacterParams(kCharacterHeadWait, 8)[9] = 0;

			getCharacter(kCharacterHeadWait).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
			fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_RSWalkDowntrain(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_RSWalkDowntrain);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 29;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_RSWalkDowntrain(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHeadWait).characterPosition.position = 1540;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
		HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "817DD", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] == 1) {
			startSeqOtis(kCharacterHeadWait, "817DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterHeadWait);

			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
			HeadWaitCall(&LogicManager::CONS_HeadWait_FinishSeqOtis, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] == 2) {
			getCharacter(kCharacterHeadWait).characterPosition.position = 850;

			getCharacter(kCharacterHeadWait).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
			fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_RSWalkUptrain(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_RSWalkUptrain);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 30;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_RSWalkUptrain(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHeadWait).characterPosition.position = 9270;
		getCharacter(kCharacterHeadWait).characterPosition.location = 0;
		getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
		HeadWaitCall(&LogicManager::CONS_HeadWait_DoSeqOtis, "817US", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] == 1) {
			startSeqOtis(kCharacterHeadWait, "817UD");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterHeadWait);

			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
			HeadWaitCall(&LogicManager::CONS_HeadWait_FinishSeqOtis, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] == 2) {
			getCharacter(kCharacterHeadWait).characterPosition.position = 5900;

			getCharacter(kCharacterHeadWait).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, _functionsHeadWait[getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall]]);
			fedEx(kCharacterHeadWait, kCharacterHeadWait, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_StartPart5);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 31;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_HeadWait_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterHeadWait);
		getCharacter(kCharacterHeadWait).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterHeadWait).characterPosition.position = 3969;
		getCharacter(kCharacterHeadWait).characterPosition.location = 1;
		getCharacter(kCharacterHeadWait).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_Prisoner);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 32;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_HeadWait_Hiding(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_Hiding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_Hiding);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 33;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_Hiding(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterHeadWait)[3]) {
			if (!getCharacterCurrentParams(kCharacterHeadWait)[4]) {
				getCharacterCurrentParams(kCharacterHeadWait)[4] = _gameTime + 4500;
				if (_gameTime == -4500) {
					setDoor(38, kCharacterHeadWait, 1, 0, 0);
					getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
					HeadWaitCall(&LogicManager::CONS_HeadWait_DoDialog, "Wat5010", 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterHeadWait)[4] < _gameTime) {
				getCharacterCurrentParams(kCharacterHeadWait)[4] = 0x7FFFFFFF;
				setDoor(38, kCharacterHeadWait, 1, 0, 0);
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 1;
				HeadWaitCall(&LogicManager::CONS_HeadWait_DoDialog, "Wat5010", 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterHeadWait)[0]) {
			if (getCharacterCurrentParams(kCharacterHeadWait)[5] || (getCharacterCurrentParams(kCharacterHeadWait)[5] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterHeadWait)[5] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterHeadWait)[5] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterHeadWait)[0] = 0;
			getCharacterCurrentParams(kCharacterHeadWait)[1] = 1;
			setDoor(38, kCharacterHeadWait, 1, 0, 0);
			getCharacterCurrentParams(kCharacterHeadWait)[5] = 0;
		} else {
			getCharacterCurrentParams(kCharacterHeadWait)[5] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterHeadWait)[0]) {
			setDoor(38, kCharacterHeadWait, 1, 0, 0);
			getCharacterCurrentParams(kCharacterHeadWait)[0] = 0;
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 2;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoDialog, getCathJustChecking(), 0, 0, 0);
		} else if (msg->action == 8) {
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 3;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoDialog, "LIB012", 0, 0, 0);
		} else {
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 4;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoDialog, "LIB013", 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterHeadWait).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterHeadWait).characterPosition.position = 3050;
		getCharacter(kCharacterHeadWait).characterPosition.location = 1;
		setDoor(38, kCharacterHeadWait, 1, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterHeadWait)[1] || getCharacterCurrentParams(kCharacterHeadWait)[0]) {
			getCharacterCurrentParams(kCharacterHeadWait)[1] = 0;
			getCharacterCurrentParams(kCharacterHeadWait)[0] = 0;
			setDoor(38, kCharacterHeadWait, 1, 10, 9);
			getCharacterCurrentParams(kCharacterHeadWait)[2] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8]) {
		case 1:
			setDoor(38, kCharacterHeadWait, 1, 10, 9);

			if (getCharacterCurrentParams(kCharacterHeadWait)[0]) {
				if (getCharacterCurrentParams(kCharacterHeadWait)[5] || (getCharacterCurrentParams(kCharacterHeadWait)[5] = _realTime + 75, _realTime != -75)) {
					if (getCharacterCurrentParams(kCharacterHeadWait)[5] >= _realTime)
						break;

					getCharacterCurrentParams(kCharacterHeadWait)[5] = 0x7FFFFFFF;
				}

				getCharacterCurrentParams(kCharacterHeadWait)[0] = 0;
				getCharacterCurrentParams(kCharacterHeadWait)[1] = 1;
				setDoor(38, kCharacterHeadWait, 1, 0, 0);
				getCharacterCurrentParams(kCharacterHeadWait)[5] = 0;
			} else {
				getCharacterCurrentParams(kCharacterHeadWait)[5] = 0;
			}

			break;
		case 2:
			setDoor(38, kCharacterHeadWait, 1, 10, 9);
			break;
		case 3:
		case 4:
			getCharacterCurrentParams(kCharacterHeadWait)[2]++;
			if (getCharacterCurrentParams(kCharacterHeadWait)[2] == 1) {
				setDoor(38, kCharacterHeadWait, 1, 0, 0);
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 5;
				HeadWaitCall(&LogicManager::CONS_HeadWait_DoDialog, "Wat5001", 0, 0, 0);
			} else if (getCharacterCurrentParams(kCharacterHeadWait)[2] == 2) {
				setDoor(38, kCharacterHeadWait, 1, 0, 0);
				getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 6;
				HeadWaitCall(&LogicManager::CONS_HeadWait_DoDialog, "Wat5001A", 0, 0, 0);
			}

			break;
		case 5:
			getCharacterCurrentParams(kCharacterHeadWait)[0] = 1;
			setDoor(38, kCharacterHeadWait, 1, 14, 0);
			break;
		case 6:
			getCharacterCurrentParams(kCharacterHeadWait)[1] = 1;
			break;
		case 7:
			getCharacterCurrentParams(kCharacterHeadWait)[3] = 1;
			break;
		default:
			break;
		}

		break;
	case 135800432:
		CONS_HeadWait_Disappear(0, 0, 0, 0);
		break;
	case 169750080:
		if (whoRunningDialog(kCharacterHeadWait)) {
			getCharacterCurrentParams(kCharacterHeadWait)[3] = 1;
		} else {
			getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall + 8] = 7;
			HeadWaitCall(&LogicManager::CONS_HeadWait_DoDialog, "Wat5002", 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_HeadWait_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHeadWait).callParams[getCharacter(kCharacterHeadWait).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHeadWait, &LogicManager::HAND_HeadWait_Disappear);
	getCharacter(kCharacterHeadWait).callbacks[getCharacter(kCharacterHeadWait).currentCall] = 34;

	params->clear();

	fedEx(kCharacterHeadWait, kCharacterHeadWait, 12, 0);
}

void LogicManager::HAND_HeadWait_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsHeadWait[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_HeadWait_DoSeqOtis,
	&LogicManager::HAND_HeadWait_WaitRCClear,
	&LogicManager::HAND_HeadWait_FinishSeqOtis,
	&LogicManager::HAND_HeadWait_DoWait,
	&LogicManager::HAND_HeadWait_DoBlockSeqOtis,
	&LogicManager::HAND_HeadWait_DoDialog,
	&LogicManager::HAND_HeadWait_DoComplexSeqOtis,
	&LogicManager::HAND_HeadWait_RebeccaHereWeAre,
	&LogicManager::HAND_HeadWait_DoOtis5012C,
	&LogicManager::HAND_HeadWait_DoSeatOtis,
	&LogicManager::HAND_HeadWait_SeatCath,
	&LogicManager::HAND_HeadWait_Birth,
	&LogicManager::HAND_HeadWait_GreetAugust,
	&LogicManager::HAND_HeadWait_DoOtis5001C,
	&LogicManager::HAND_HeadWait_GreetAnna,
	&LogicManager::HAND_HeadWait_GreetTatiana,
	&LogicManager::HAND_HeadWait_ServingDinner,
	&LogicManager::HAND_HeadWait_AfterDinner,
	&LogicManager::HAND_HeadWait_LockUp,
	&LogicManager::HAND_HeadWait_StartPart2,
	&LogicManager::HAND_HeadWait_StartPart3,
	&LogicManager::HAND_HeadWait_InPart3,
	&LogicManager::HAND_HeadWait_AbbotSeatMe3,
	&LogicManager::HAND_HeadWait_DoOtis5029A,
	&LogicManager::HAND_HeadWait_StartPart4,
	&LogicManager::HAND_HeadWait_InPart4,
	&LogicManager::HAND_HeadWait_MeetCond2,
	&LogicManager::HAND_HeadWait_TellAug,
	&LogicManager::HAND_HeadWait_RSWalkDowntrain,
	&LogicManager::HAND_HeadWait_RSWalkUptrain,
	&LogicManager::HAND_HeadWait_StartPart5,
	&LogicManager::HAND_HeadWait_Prisoner,
	&LogicManager::HAND_HeadWait_Hiding,
	&LogicManager::HAND_HeadWait_Disappear
};

} // End of namespace LastExpress
