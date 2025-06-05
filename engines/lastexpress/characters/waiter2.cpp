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

void LogicManager::CONS_Waiter2(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterWaiter2,
			_functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]
		);

		break;
	case 1:
		CONS_Waiter2_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Waiter2_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Waiter2_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Waiter2_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Waiter2_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::Waiter2Call(CALL_PARAMS) {
	getCharacter(kCharacterWaiter2).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Waiter2_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_DoWait);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 1;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterWaiter2)[1] ||
			(getCharacterCurrentParams(kCharacterWaiter2)[1] = _gameTime + getCharacterCurrentParams(kCharacterWaiter2)[0], _gameTime + getCharacterCurrentParams(kCharacterWaiter2)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterWaiter2)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterWaiter2)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterWaiter2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
		fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_DoSeqOtis);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterWaiter2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
		fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterWaiter2)[3]) {
			playChrExcuseMe(kCharacterWaiter2, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterWaiter2)[3] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterWaiter2, (char *)&getCharacterCurrentParams(kCharacterWaiter2)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_DoBlockSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_DoBlockSeqOtis);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_DoBlockSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseView(kCharacterWaiter2, getCharacterCurrentParams(kCharacterWaiter2)[3], getCharacterCurrentParams(kCharacterWaiter2)[4]);

		getCharacter(kCharacterWaiter2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
		fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterWaiter2)[5]) {
			playChrExcuseMe(kCharacterWaiter2, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterWaiter2)[5] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterWaiter2, (char *)&getCharacterCurrentParams(kCharacterWaiter2)[0]);
		blockView(kCharacterWaiter2, getCharacterCurrentParams(kCharacterWaiter2)[3], getCharacterCurrentParams(kCharacterWaiter2)[4]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_FinishSeqOtis);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 4;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterWaiter2).direction != 4) {
			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterWaiter2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
		fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterWaiter2)[0]) {
			playChrExcuseMe(kCharacterWaiter2, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterWaiter2)[0] = 1;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_DoJoinedSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_DoJoinedSeqOtis);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	strncpy((char *)&params->parameters[5], param4.stringParam, 12);

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_DoJoinedSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		if (!getCharacterCurrentParams(kCharacterWaiter2)[8])
			fedEx(kCharacterWaiter2, getCharacterCurrentParams(kCharacterWaiter2)[3], getCharacterCurrentParams(kCharacterWaiter2)[4], (char *)&getCharacterCurrentParams(kCharacterWaiter2)[5]);

		getCharacter(kCharacterWaiter2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
		fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterWaiter2)[9]) {
			playChrExcuseMe(kCharacterWaiter2, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterWaiter2)[9] = 1;
		}

		break;
	case 10:
		if (!getCharacterCurrentParams(kCharacterWaiter2)[8]) {
			fedEx(kCharacterWaiter2, getCharacterCurrentParams(kCharacterWaiter2)[3], getCharacterCurrentParams(kCharacterWaiter2)[4], (char *)&getCharacterCurrentParams(kCharacterWaiter2)[5]);
			getCharacterCurrentParams(kCharacterWaiter2)[8] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterWaiter2, (char *)&getCharacterCurrentParams(kCharacterWaiter2)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_DoDialog);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterWaiter2).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
		fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
		break;
	case 12:
		playDialog(kCharacterWaiter2, (char *)&getCharacterCurrentParams(kCharacterWaiter2)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_MonsieurServeUs(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_MonsieurServeUs);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 7;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_MonsieurServeUs(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "924", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter2, kCharacterMonsieur, 122358304, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "008C", 0, 0, 0);
			break;
		case 2:
			send(kCharacterWaiter2, kCharacterMonsieur, 122288808, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "926", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter2, 8)[9] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_Birth);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 8;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Waiter2_ServingDinner(0, 0, 0, 0);
		break;
	case 12:
		autoMessage(kCharacterWaiter2, 302203328, 5);
		autoMessage(kCharacterWaiter2, 189688608, 6);
		autoMessage(kCharacterWaiter2, 236237423, 7);
		autoMessage(kCharacterWaiter2, 219377792, 8);
		autoMessage(kCharacterWaiter2, 291721418, 10);
		autoMessage(kCharacterWaiter2, 258136010, 11);
		autoMessage(kCharacterWaiter2, 302996448, 2);
		autoMessage(kCharacterWaiter2, 223002560, 0);
		autoMessage(kCharacterWaiter2, 256200848, 9);
		autoMessage(kCharacterWaiter2, 269485588, 3);
		autoMessage(kCharacterWaiter2, 326144276, 4);
		getCharacter(kCharacterWaiter2).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_MilosOrder(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_MilosOrder);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 9;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_MilosOrder(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "924", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterMilos, "BLANK");
			startCycOtis(kCharacterWaiter2, "009B");
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoDialog, "WAT1001", 0, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterMilos, "009A");
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "926", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter2, 8)[0] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_MonsieurOrder(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_MonsieurOrder);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 10;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_MonsieurOrder(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "924", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterMonsieur, "BLANK");
			startCycOtis(kCharacterWaiter2, "008C");
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoDialog, "MRB1077", 0, 0, 0);
			break;
		case 2:
			send(kCharacterWaiter2, kCharacterMonsieur, 168717392, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "926", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter2, 8)[9] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_ClearAlexei(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_ClearAlexei);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 11;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_ClearAlexei(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "919", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8]) {
		case 1:
			blockView(kCharacterWaiter2, kCarRestaurant, 63);
			send(kCharacterWaiter2, kCharacterTableB, 136455232, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoJoinedSeqOtis, "005H", 0x22, 0x62FD7B0, "005J");
			break;
		case 2:
			releaseView(kCharacterWaiter2, kCarRestaurant, 63);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "921", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter2, 8)[2] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_ClearMilos(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_ClearMilos);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 12;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_ClearMilos(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "923", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter2, kCharacterTableC, 136455232, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoJoinedSeqOtis, "009F", 35, 103798704, "009G");
			break;
		case 2:
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "926", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter2, 8)[3] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_ClearMonsieur(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_ClearMonsieur);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 13;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_ClearMonsieur(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "923", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter2, kCharacterTableC, 136455232, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoJoinedSeqOtis, "009F", 35, 103798704, "009G");
			break;
		case 2:
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "926", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter2, 8)[4] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_ServingDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_ServingDinner);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 14;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_ServingDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inKitchen(kCharacterWaiter2) && rcClear()) {
			if (getCharacterParams(kCharacterWaiter2, 8)[0]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
				Waiter2Call(&LogicManager::CONS_Waiter2_MilosOrder, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter2, 8)[9]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
				Waiter2Call(&LogicManager::CONS_Waiter2_MonsieurOrder, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter2, 8)[2]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
				Waiter2Call(&LogicManager::CONS_Waiter2_ClearAlexei, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter2, 8)[3]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 4;
				Waiter2Call(&LogicManager::CONS_Waiter2_ClearMilos, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter2, 8)[4]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 5;
				Waiter2Call(&LogicManager::CONS_Waiter2_ClearMonsieur, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] == 5) {
			send(kCharacterWaiter2, kCharacterHeadWait, 352768896, 0);
			CONS_Waiter2_AfterDinner(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_AfterDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_AfterDinner);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 15;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_AfterDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
		break;
	case 101632192:
		CONS_Waiter2_LockUp(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_LockUp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_LockUp);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 16;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_LockUp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		endGraphics(kCharacterWaiter2);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_StartPart2);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 17;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Waiter2_InKitchen(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterWaiter2);
		getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter2).clothes = 1;
		getCharacter(kCharacterWaiter2).inventoryItem = 0;
		getCharacterParams(kCharacterWaiter2, 8)[5] = 0;
		getCharacterParams(kCharacterWaiter2, 8)[6] = 0;
		getCharacterParams(kCharacterWaiter2, 8)[7] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_InKitchen(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_InKitchen);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 18;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_InKitchen(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inKitchen(kCharacterWaiter2) && rcClear()) {
			if (getCharacterParams(kCharacterWaiter2, 8)[5]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
				Waiter2Call(&LogicManager::CONS_Waiter2_TatianaClearTableB, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter2, 8)[6]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
				Waiter2Call(&LogicManager::CONS_Waiter2_IvoComeHere, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter2, 8)[7] || getCharacterParams(kCharacterWaiter2, 8)[4]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
				Waiter2Call(&LogicManager::CONS_Waiter2_IvoClearTableC, 0, 0, 0, 0);
			}

			break;
		}

		break;
	case 18:
		if (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] == 1) {
			if (getCharacterParams(kCharacterWaiter2, 8)[6]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
				Waiter2Call(&LogicManager::CONS_Waiter2_IvoComeHere, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter2, 8)[7] || getCharacterParams(kCharacterWaiter2, 8)[4]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
				Waiter2Call(&LogicManager::CONS_Waiter2_IvoClearTableC, 0, 0, 0, 0);
			}

			break;
		}

		if (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] != 2) {
			if (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] == 4) {
				endGraphics(kCharacterWaiter2);
				getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			}

			break;
		}

		if (getCharacterParams(kCharacterWaiter2, 8)[7] || getCharacterParams(kCharacterWaiter2, 8)[4]) {
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_IvoClearTableC, 0, 0, 0, 0);
		}

		break;
	case 101106391:
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 4;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "975", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_TatianaClearTableB(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_TatianaClearTableB);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 19;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_TatianaClearTableB(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "969", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8]) {
		case 1:
			blockView(kCharacterWaiter2, kCarRestaurant, 63);
			send(kCharacterWaiter2, kCharacterTableB, 136455232, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoJoinedSeqOtis, "005H2", 34, 103798704, "018A");
			break;
		case 2:
			releaseView(kCharacterWaiter2, kCarRestaurant, 63);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "971", 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			endGraphics(kCharacterWaiter2);
			getCharacterParams(kCharacterWaiter2, 8)[5] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_IvoComeHere(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_IvoComeHere);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 20;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_IvoComeHere(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "973", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] == 1) {
			send(kCharacterWaiter2, kCharacterIvo, 123712592, 0);
			startCycOtis(kCharacterWaiter2, "BLANK");
			getCharacterParams(kCharacterWaiter2, 8)[6] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_IvoClearTableC(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_IvoClearTableC);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 21;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_IvoClearTableC(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "974", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter2, kCharacterTableC, 136455232, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoJoinedSeqOtis, "009F2", 35, 103798704, "009G");
			break;
		case 2:
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "976", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter2, 8)[7] = 0;
			getCharacterParams(kCharacterWaiter2, 8)[4] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_StartPart3);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 22;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Waiter2_Serving3(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterWaiter2);
		getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter2).clothes = 1;
		getCharacter(kCharacterWaiter2).inventoryItem = 0;
		getCharacterParams(kCharacterWaiter2, 8)[8] = 0;
		getCharacterParams(kCharacterWaiter2, 8)[9] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_Serving3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_Serving3);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 23;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_Serving3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inKitchen(kCharacterWaiter2) && rcClear()) {
			if (getCharacterParams(kCharacterWaiter2, 8)[8]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
				Waiter2Call(&LogicManager::CONS_Waiter2_AnnaBringTea3, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter2, 8)[9]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
				Waiter2Call(&LogicManager::CONS_Waiter2_MonsieurServeUs, 0, 0, 0, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_AnnaBringTea3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_AnnaBringTea3);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 24;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_AnnaBringTea3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "816DD", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8]) {
		case 1:
			startSeqOtis(kCharacterWaiter2, "927");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterWaiter2);

			playDialog(kCharacterWaiter2, "Ann3143A", -1, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
			Waiter2Call(&LogicManager::CONS_Waiter2_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 2:
			send(kCharacterWaiter2, kCharacterAnna, 122358304, 0);
			playDialog(kCharacterWaiter2, "Ann3144", -1, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoBlockSeqOtis, "112C", 5, 57, 0);
			break;
		case 3:
			send(kCharacterWaiter2, kCharacterAnna, 122288808, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 4;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "928", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterWaiter2, "816UD");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterWaiter2);

			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 5;
			Waiter2Call(&LogicManager::CONS_Waiter2_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
			endGraphics(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter2, 8)[8] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_StartPart4);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 25;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Waiter2_Serving4(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterWaiter2);
		getCharacter(kCharacterWaiter2).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).inventoryItem = 0;
		endGraphics(kCharacterWaiter2);
		getCharacterParams(kCharacterWaiter2, 8)[10] = 0;
		getCharacterParams(kCharacterWaiter2, 8)[12] = 0;
		getCharacterParams(kCharacterWaiter2, 8)[11] = 0;
		getCharacterParams(kCharacterWaiter2, 8)[9] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_Serving4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_Serving4);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 26;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_Serving4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterWaiter2)[0]) {
			bool skip = false;

			if (!getCharacterCurrentParams(kCharacterWaiter2)[1]) {
				getCharacterCurrentParams(kCharacterWaiter2)[1] = _gameTime + 900;
				if (_gameTime == -900) {
					skip = true;
					getCharacterParams(kCharacterWaiter2, 8)[12] = 1;
					getCharacterCurrentParams(kCharacterWaiter2)[0] = 0;
				}
			}

			if (!skip && getCharacterCurrentParams(kCharacterWaiter2)[1] < _gameTime) {
				getCharacterCurrentParams(kCharacterWaiter2)[1] = 0x7FFFFFFF;
				getCharacterParams(kCharacterWaiter2, 8)[12] = 1;
				getCharacterCurrentParams(kCharacterWaiter2)[0] = 0;
			}
		}

		if (inKitchen(kCharacterWaiter2) && rcClear()) {
			if (getCharacterParams(kCharacterWaiter2, 8)[10]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
				Waiter2Call(&LogicManager::CONS_Waiter2_AugustNeedsADrink, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter2, 8)[12]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
				Waiter2Call(&LogicManager::CONS_Waiter2_ServeAugustADrink, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter2, 8)[11]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
				Waiter2Call(&LogicManager::CONS_Waiter2_AnnaNeedsADrink, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter2, 8)[9]) {
				getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 4;
				Waiter2Call(&LogicManager::CONS_Waiter2_MonsieurServeUs, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] == 1)
			getCharacterCurrentParams(kCharacterWaiter2)[0] = 1;

		break;
	case 201431954:
		getCharacter(kCharacterWaiter2).callParams[8].parameters[10] = 0;
		getCharacterParams(kCharacterWaiter2, 8)[12] = 0;
		getCharacterParams(kCharacterWaiter2, 8)[11] = 0;
		getCharacterParams(kCharacterWaiter2, 8)[9] = 0;
		getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_AugustNeedsADrink(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_AugustNeedsADrink);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 27;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_AugustNeedsADrink(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "816DD", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8]) {
		case 1:
			startSeqOtis(kCharacterWaiter2, "929");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterWaiter2);

			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
			Waiter2Call(&LogicManager::CONS_Waiter2_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 2:
			send(kCharacterWaiter2, kCharacterAugust, 122358304, 0);
			playDialog(7, "Aug4003", -1, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoBlockSeqOtis, "122D", 5, 57, 0);
			break;
		case 3:
			send(kCharacterWaiter2, kCharacterAugust, 134486752, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 4;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "930", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterWaiter2, "816UD");
			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 5;
			Waiter2Call(&LogicManager::CONS_Waiter2_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
			endGraphics(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter2, 8)[10] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_ServeAugustADrink(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_ServeAugustADrink);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 28;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_ServeAugustADrink(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "816DD", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8]) {
		case 1:
			startSeqOtis(kCharacterWaiter2, "931");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterWaiter2);

			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
			Waiter2Call(&LogicManager::CONS_Waiter2_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 2:
			send(kCharacterWaiter2, kCharacterAugust, 122358304, 0);
			playDialog(kCharacterWaiter2, "Aug4004", -1, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoBlockSeqOtis, "122E", 5, 57, 0);
			break;
		case 3:
			send(kCharacterWaiter2, kCharacterAugust, 125826561, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 4;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "930", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterWaiter2, "816UD");
			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 5;
			Waiter2Call(&LogicManager::CONS_Waiter2_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
			endGraphics(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter2, 8)[12] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_AnnaNeedsADrink(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_AnnaNeedsADrink);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 29;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_AnnaNeedsADrink(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter2).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter2).characterPosition.location = 0;
		getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 1;
		Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "816DD", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8]) {
		case 1:
			startSeqOtis(kCharacterWaiter2, "932");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterWaiter2);

			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 2;
			Waiter2Call(&LogicManager::CONS_Waiter2_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 2:
			send(kCharacterWaiter2, kCharacterAnna, 122358304, 0);
			playDialog(kCharacterWaiter2, "Ann4151", -1, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 3;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoBlockSeqOtis, "127D", 5, 56, 0);
			break;
		case 3:
			send(kCharacterWaiter2, kCharacterAnna, 122288808, 0);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 4;
			Waiter2Call(&LogicManager::CONS_Waiter2_DoSeqOtis, "930", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterWaiter2, "816UD");
			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall + 8] = 5;
			Waiter2Call(&LogicManager::CONS_Waiter2_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
			endGraphics(kCharacterWaiter2);
			getCharacter(kCharacterWaiter2).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter2, 8)[11] = 0;

			getCharacter(kCharacterWaiter2).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, _functionsWaiter2[getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall]]);
			fedEx(kCharacterWaiter2, kCharacterWaiter2, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_StartPart5);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 30;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Waiter2_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterWaiter2);
		getCharacter(kCharacterWaiter2).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter2).characterPosition.position = 3969;
		getCharacter(kCharacterWaiter2).characterPosition.location = 1;
		getCharacter(kCharacterWaiter2).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_Prisoner);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 31;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Waiter2_Disappear(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter2_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter2).callParams[getCharacter(kCharacterWaiter2).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter2, &LogicManager::HAND_Waiter2_Disappear);
	getCharacter(kCharacterWaiter2).callbacks[getCharacter(kCharacterWaiter2).currentCall] = 32;

	params->clear();

	fedEx(kCharacterWaiter2, kCharacterWaiter2, 12, 0);
}

void LogicManager::HAND_Waiter2_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsWaiter2[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Waiter2_DoWait,
	&LogicManager::HAND_Waiter2_DoSeqOtis,
	&LogicManager::HAND_Waiter2_DoBlockSeqOtis,
	&LogicManager::HAND_Waiter2_FinishSeqOtis,
	&LogicManager::HAND_Waiter2_DoJoinedSeqOtis,
	&LogicManager::HAND_Waiter2_DoDialog,
	&LogicManager::HAND_Waiter2_MonsieurServeUs,
	&LogicManager::HAND_Waiter2_Birth,
	&LogicManager::HAND_Waiter2_MilosOrder,
	&LogicManager::HAND_Waiter2_MonsieurOrder,
	&LogicManager::HAND_Waiter2_ClearAlexei,
	&LogicManager::HAND_Waiter2_ClearMilos,
	&LogicManager::HAND_Waiter2_ClearMonsieur,
	&LogicManager::HAND_Waiter2_ServingDinner,
	&LogicManager::HAND_Waiter2_AfterDinner,
	&LogicManager::HAND_Waiter2_LockUp,
	&LogicManager::HAND_Waiter2_StartPart2,
	&LogicManager::HAND_Waiter2_InKitchen,
	&LogicManager::HAND_Waiter2_TatianaClearTableB,
	&LogicManager::HAND_Waiter2_IvoComeHere,
	&LogicManager::HAND_Waiter2_IvoClearTableC,
	&LogicManager::HAND_Waiter2_StartPart3,
	&LogicManager::HAND_Waiter2_Serving3,
	&LogicManager::HAND_Waiter2_AnnaBringTea3,
	&LogicManager::HAND_Waiter2_StartPart4,
	&LogicManager::HAND_Waiter2_Serving4,
	&LogicManager::HAND_Waiter2_AugustNeedsADrink,
	&LogicManager::HAND_Waiter2_ServeAugustADrink,
	&LogicManager::HAND_Waiter2_AnnaNeedsADrink,
	&LogicManager::HAND_Waiter2_StartPart5,
	&LogicManager::HAND_Waiter2_Prisoner,
	&LogicManager::HAND_Waiter2_Disappear
};

} // End of namespace LastExpress
