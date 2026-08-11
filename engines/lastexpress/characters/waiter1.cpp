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

void LogicManager::CONS_Waiter1(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterWaiter1,
			_functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]
		);

		break;
	case 1:
		CONS_Waiter1_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Waiter1_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Waiter1_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Waiter1_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Waiter1_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::Waiter1Call(CALL_PARAMS) {
	getCharacter(kCharacterWaiter1).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Waiter1_DoJoinedSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_DoJoinedSeqOtis);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 1;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	strncpy((char *)&params->parameters[5], param4.stringParam, 12);

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_DoJoinedSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		if (!getCharacterCurrentParams(kCharacterWaiter1)[8])
			fedEx(kCharacterWaiter1, getCharacterCurrentParams(kCharacterWaiter1)[3], getCharacterCurrentParams(kCharacterWaiter1)[4], (char *)&getCharacterCurrentParams(kCharacterWaiter1)[5]);

		getCharacter(kCharacterWaiter1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
		fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterWaiter1)[9]) {
			playChrExcuseMe(kCharacterWaiter1, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterWaiter1)[9] = 1;
		}

		break;
	case 10:
		if (!getCharacterCurrentParams(kCharacterWaiter1)[8]) {
			fedEx(kCharacterWaiter1, getCharacterCurrentParams(kCharacterWaiter1)[3], getCharacterCurrentParams(kCharacterWaiter1)[4], (char *)&getCharacterCurrentParams(kCharacterWaiter1)[5]);
			getCharacterCurrentParams(kCharacterWaiter1)[8] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterWaiter1, (char *)&getCharacterCurrentParams(kCharacterWaiter1)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_DoWait);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterWaiter1)[1] || (getCharacterCurrentParams(kCharacterWaiter1)[1] = _gameTime + getCharacterCurrentParams(kCharacterWaiter1)[0], _gameTime + getCharacterCurrentParams(kCharacterWaiter1)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterWaiter1)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterWaiter1)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterWaiter1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
		fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_DoSeqOtis);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterWaiter1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
		fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterWaiter1)[3]) {
			playChrExcuseMe(kCharacterWaiter1, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterWaiter1)[3] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterWaiter1, (char *)&getCharacterCurrentParams(kCharacterWaiter1)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_DoBlockSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_DoBlockSeqOtis);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_DoBlockSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseView(kCharacterWaiter1, getCharacterCurrentParams(kCharacterWaiter1)[3], getCharacterCurrentParams(kCharacterWaiter1)[4]);

		getCharacter(kCharacterWaiter1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
		fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterWaiter1)[5]) {
			playChrExcuseMe(kCharacterWaiter1, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterWaiter1)[5] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterWaiter1, (char *)&getCharacterCurrentParams(kCharacterWaiter1)[0]);
		blockView(kCharacterWaiter1, getCharacterCurrentParams(kCharacterWaiter1)[3], getCharacterCurrentParams(kCharacterWaiter1)[4]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_FinishSeqOtis);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 5;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterWaiter1).direction != 4) {
			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterWaiter1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
		fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterWaiter1)[0]) {
			playChrExcuseMe(kCharacterWaiter1, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterWaiter1)[0] = 1;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_DoDialog);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 6;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterWaiter1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
		fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		break;
	case 12:
		playDialog(kCharacterWaiter1, (char *)&getCharacterCurrentParams(kCharacterWaiter1)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_RebeccaFeedUs(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_RebeccaFeedUs);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 7;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_RebeccaFeedUs(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacterParams(kCharacterWaiter1, 8)[2] = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "911", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] == 1) {
			endGraphics(kCharacterWaiter1);
			send(kCharacterWaiter1, kCharacterRebecca, 123712592, 0);
		} else if (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] == 2) {
			endGraphics(kCharacterWaiter1);
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		}

		break;
	case 136702400:
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "913", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_RebeccaClearOurTable(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_RebeccaClearOurTable);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 8;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_RebeccaClearOurTable(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "911", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter1, kCharacterTableD, 136455232, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoJoinedSeqOtis, "010L", 36, 103798704, "010M");
			break;
		case 2:
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "913", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter1);
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter1, 8)[9] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_AbbotCheckMe(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_AbbotCheckMe);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 9;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_AbbotCheckMe(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "915", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter1, kCharacterAbbot, 122358304, 0);
			startCycOtis(kCharacterWaiter1, "029D");
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;

			if (_globals[kGlobalChapter] != 3) {
				Waiter1Call(&LogicManager::CONS_Waiter1_DoDialog, "Abb4001", 0, 0, 0);
			} else {
				Waiter1Call(&LogicManager::CONS_Waiter1_DoDialog, "Abb3016", 0, 0, 0);
			}
			
			break;
		case 2:
			send(kCharacterWaiter1, kCharacterAbbot, 122288808, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "917", 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			endGraphics(kCharacterWaiter1);
			getCharacterParams(kCharacterWaiter1, 8)[17] = 0;
			getCharacterParams(kCharacterWaiter1, 8)[13] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_AbbotClearTable(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_AbbotClearTable);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 10;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_AbbotClearTable(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "916", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter1, kCharacterTableE, 136455232, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoJoinedSeqOtis, "014E", 37, 103798704, "014F");
			break;
		case 2:
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "918", 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			endGraphics(kCharacterWaiter1);
			getCharacterParams(kCharacterWaiter1, 8)[18] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_Birth);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 11;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Waiter1_ServingDinner(0, 0, 0, 0);
		break;
	case 12:
		autoMessage(kCharacterWaiter1, 270410280, 0);
		autoMessage(kCharacterWaiter1, 304061224, 1);
		autoMessage(kCharacterWaiter1, 252568704, 10);
		autoMessage(kCharacterWaiter1, 286534136, 11);
		autoMessage(kCharacterWaiter1, 218983616, 12);
		autoMessage(kCharacterWaiter1, 218586752, 13);
		autoMessage(kCharacterWaiter1, 207330561, 14);
		autoMessage(kCharacterWaiter1, 286403504, 16);
		autoMessage(kCharacterWaiter1, 218128129, 17);
		autoMessage(kCharacterWaiter1, 270068760, 18);
		autoMessage(kCharacterWaiter1, 223712416, 2);
		autoMessage(kCharacterWaiter1, 237485916, 5);
		autoMessage(kCharacterWaiter1, 188893625, 8);
		autoMessage(kCharacterWaiter1, 204704037, 6);
		autoMessage(kCharacterWaiter1, 292758554, 7);
		autoMessage(kCharacterWaiter1, 337548856, 9);
		getCharacter(kCharacterWaiter1).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_AnnaOrder(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_AnnaOrder);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 12;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_AnnaOrder(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "907", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] == 1) {
			endGraphics(kCharacterWaiter1);
			send(kCharacterWaiter1, kCharacterAnna, 268773672, 0);
			getCharacterParams(kCharacterWaiter1, 8)[0] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_AugustOrder(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_AugustOrder);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 13;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_AugustOrder(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "911", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] == 1) {
			startCycOtis(kCharacterWaiter1, "010F");
			send(kCharacterWaiter1, kCharacterAugust, 268773672, 0);
			getCharacterParams(kCharacterWaiter1, 8)[1] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_ServeAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_ServeAnna);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 14;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_ServeAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "908", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] == 1) {
			endGraphics(kCharacterWaiter1);
			send(kCharacterWaiter1, kCharacterAnna, 170016384, 0);
			getCharacterParams(kCharacterWaiter1, 8)[3] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_ServeAugust(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_ServeAugust);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 15;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_ServeAugust(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "912", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] == 1) {
			endGraphics(kCharacterWaiter1);
			send(kCharacterWaiter1, kCharacterAugust, 170016384, 0);
			getCharacterParams(kCharacterWaiter1, 8)[4] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_ClearAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_ClearAnna);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 16;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_ClearAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "907", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter1, kCharacterTableA, 136455232, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoJoinedSeqOtis, "001N", 33, 103798704, "001P");
			break;
		case 2:
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "909", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter1);
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter1, 8)[5] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_ClearTatiana(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_ClearTatiana);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 17;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_ClearTatiana(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "915", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			blockView(kCharacterWaiter1, kCarRestaurant, 67);
			send(kCharacterWaiter1, kCharacterTableE, 136455232, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoJoinedSeqOtis, "014E", 37, 103798704, "014F");
			break;
		case 2:
			releaseView(kCharacterWaiter1, kCarRestaurant, 67);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "917", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter1);
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter1, 8)[8] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_ClearAugust1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_ClearAugust1);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 18;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_ClearAugust1(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "911", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter1, kCharacterTableD, 136455232, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoJoinedSeqOtis, "010L", 0x24, 103798704, "010M");
			break;
		case 2:
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "913", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter1);
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter1, 8)[6] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_ClearAugust2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_ClearAugust2);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 19;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_ClearAugust2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "911", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter1, kCharacterTableD, 136455232, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoJoinedSeqOtis, "010L", 36, 103798704, "010M");
			break;
		case 2:
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "913", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter1);
			send(kCharacterWaiter1, kCharacterRebecca, 224253538, 0);
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter1, 8)[7] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_ServingDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_ServingDinner);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 20;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_ServingDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		bool skip1 = false;

		if (getCharacterCurrentParams(kCharacterWaiter1)[1]) {
			if (getCharacterCurrentParams(kCharacterWaiter1)[2] || (getCharacterCurrentParams(kCharacterWaiter1)[2] = _gameTime + 2700, _gameTime != -2700)) {
				if (getCharacterCurrentParams(kCharacterWaiter1)[2] >= _gameTime) {
					skip1 = true;
				}

				if (!skip1) {
					getCharacterCurrentParams(kCharacterWaiter1)[2] = 0x7FFFFFFF;
				}
			}

			if (!skip1) {
				getCharacterParams(kCharacterWaiter1, 8)[3] = 1;
				getCharacterCurrentParams(kCharacterWaiter1)[1] = 0;
			}
		}

		bool skip2 = false;

		if (getCharacterCurrentParams(kCharacterWaiter1)[0]) {
			if (!getCharacterCurrentParams(kCharacterWaiter1)[3]) {

				getCharacterCurrentParams(kCharacterWaiter1)[3] = _gameTime + 4500;
				if (_gameTime == -4500) {
					skip2 = true;
					getCharacterParams(kCharacterWaiter1, 8)[4] = 1;
					getCharacterCurrentParams(kCharacterWaiter1)[0] = 0;
				}
			}

			if (!skip2 && getCharacterCurrentParams(kCharacterWaiter1)[3] < _gameTime) {
				getCharacterCurrentParams(kCharacterWaiter1)[3] = 0x7FFFFFFF;
				getCharacterParams(kCharacterWaiter1, 8)[4] = 1;
				getCharacterCurrentParams(kCharacterWaiter1)[0] = 0;
			}
		}

		if (inKitchen(kCharacterWaiter1) && rcClear()) {
			if (getCharacterParams(kCharacterWaiter1, 8)[0]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
				Waiter1Call(&LogicManager::CONS_Waiter1_AnnaOrder, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter1, 8)[1]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
				Waiter1Call(&LogicManager::CONS_Waiter1_AugustOrder, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter1, 8)[2]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
				Waiter1Call(&LogicManager::CONS_Waiter1_RebeccaFeedUs, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter1, 8)[3]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 4;
				Waiter1Call(&LogicManager::CONS_Waiter1_ServeAnna, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter1, 8)[4]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 5;
				Waiter1Call(&LogicManager::CONS_Waiter1_ServeAugust, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter1, 8)[5]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 6;
				Waiter1Call(&LogicManager::CONS_Waiter1_ClearAnna, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter1, 8)[8]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 7;
				Waiter1Call(&LogicManager::CONS_Waiter1_ClearTatiana, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter1, 8)[6]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 8;
				Waiter1Call(&LogicManager::CONS_Waiter1_ClearAugust1, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter1, 8)[7]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 9;
				Waiter1Call(&LogicManager::CONS_Waiter1_ClearAugust2, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter1, 8)[9]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 10;
				Waiter1Call(&LogicManager::CONS_Waiter1_RebeccaClearOurTable, 0, 0, 0, 0);
			}
		}

		break;
	}
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 10:
			send(kCharacterWaiter1, kCharacterHeadWait, 352703104, 0);
			CONS_Waiter1_AfterDinner(0, 0, 0, 0);
			break;
		case 11:
			endGraphics(kCharacterWaiter1);
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			getCharacterCurrentParams(kCharacterWaiter1)[1] = 1;
			break;
		case 12:
			endGraphics(kCharacterWaiter1);
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			getCharacterCurrentParams(kCharacterWaiter1)[0] = 1;
			break;
		case 13:
		case 14:
			endGraphics(kCharacterWaiter1);
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			break;
		default:
			break;
		}

		break;
	case 136702400:
		if (msg->sender == kCharacterAnna) {
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 13;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "909", 0, 0, 0);
		} else {
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 14;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "913", 0, 0, 0);
		}

		break;
	case 203859488:
		if (msg->sender == kCharacterAnna) {
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 11;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "910", 0, 0, 0);
		} else {
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 12;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "913", 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_AfterDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_AfterDinner);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 21;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_AfterDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
		break;
	case 101632192:
		CONS_Waiter1_LockUp(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_LockUp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_LockUp);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 22;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_LockUp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		endGraphics(kCharacterWaiter1);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_StartPart2);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 23;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Waiter1_InKitchen(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterWaiter1);
		getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter1).inventoryItem = 0;
		getCharacter(kCharacterWaiter1).clothes = 1;
		getCharacterParams(kCharacterWaiter1, 8)[10] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[11] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_InKitchen(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_InKitchen);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 24;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_InKitchen(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inKitchen(kCharacterWaiter1) && rcClear()) {
			if (getCharacterParams(kCharacterWaiter1, 8)[10]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
				Waiter1Call(&LogicManager::CONS_Waiter1_AugustComeHere2, 0, 0, 0, 0);
			} else if (getCharacterParams(kCharacterWaiter1, 8)[11]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
				Waiter1Call(&LogicManager::CONS_Waiter1_AugustClearTable2, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] == 1) {
			if (getCharacterParams(kCharacterWaiter1, 8)[11]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
				Waiter1Call(&LogicManager::CONS_Waiter1_AugustClearTable2, 0, 0, 0, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_AugustComeHere2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_AugustComeHere2);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 25;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_AugustComeHere2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "957", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] == 1) {
			send(kCharacterWaiter1, kCharacterAugust, 123712592, 0);
			startCycOtis(kCharacterWaiter1, "BLANK");
		} else if (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] == 2) {
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			endGraphics(kCharacterWaiter1);
			getCharacterParams(kCharacterWaiter1, 8)[10] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
		}

		break;
	case 219522616:
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "959", 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_AugustClearTable2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_AugustClearTable2);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 26;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_AugustClearTable2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "957", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter1, kCharacterTableA, 136455232, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoJoinedSeqOtis, "016E", 33, 103798704, "016D");
			break;
		case 2:
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "959", 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			endGraphics(kCharacterWaiter1);
			getCharacterParams(kCharacterWaiter1, 8)[11] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_StartPart3);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 27;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Waiter1_Serving3(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterWaiter1);
		getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter1).inventoryItem = 0;
		getCharacter(kCharacterWaiter1).clothes = 1;
		getCharacterParams(kCharacterWaiter1, 8)[12] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[13] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[18] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[19] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[2] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[9] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_Serving3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_Serving3);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 28;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_Serving3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inKitchen(kCharacterWaiter1) && rcClear()) {
			if (getCharacterParams(kCharacterWaiter1, 8)[12]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
				Waiter1Call(&LogicManager::CONS_Waiter1_AnnaComeHere3, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterWaiter1, 8)[13]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
				Waiter1Call(&LogicManager::CONS_Waiter1_AbbotCheckMe, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterWaiter1, 8)[19]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
				Waiter1Call(&LogicManager::CONS_Waiter1_AbbotServeLunch3, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterWaiter1, 8)[18]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 4;
				Waiter1Call(&LogicManager::CONS_Waiter1_AbbotClearTable, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterWaiter1, 8)[2]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 5;
				Waiter1Call(&LogicManager::CONS_Waiter1_RebeccaFeedUs, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterWaiter1, 8)[9]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 6;
				Waiter1Call(&LogicManager::CONS_Waiter1_RebeccaClearOurTable, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			if (getCharacterParams(kCharacterWaiter1, 8)[13]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
				Waiter1Call(&LogicManager::CONS_Waiter1_AbbotCheckMe, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacterParams(kCharacterWaiter1, 8)[19]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
				Waiter1Call(&LogicManager::CONS_Waiter1_AbbotServeLunch3, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (getCharacterParams(kCharacterWaiter1, 8)[18]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 4;
				Waiter1Call(&LogicManager::CONS_Waiter1_AbbotClearTable, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (getCharacterParams(kCharacterWaiter1, 8)[2]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 5;
				Waiter1Call(&LogicManager::CONS_Waiter1_RebeccaFeedUs, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (getCharacterParams(kCharacterWaiter1, 8)[9]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 6;
				Waiter1Call(&LogicManager::CONS_Waiter1_RebeccaClearOurTable, 0, 0, 0, 0);
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

void LogicManager::CONS_Waiter1_AnnaComeHere3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_AnnaComeHere3);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 29;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_AnnaComeHere3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "911", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter1, kCharacterAnna, 122358304, 0);
			startCycOtis(kCharacterWaiter1, "026D");
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoDialog, "Ann3138", 0, 0, 0);
			break;
		case 2:
			send(kCharacterWaiter1, kCharacterAnna, 122288808, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "913", 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterWaiter1);
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			getCharacterParams(kCharacterWaiter1, 8)[12] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_AbbotServeLunch3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_AbbotServeLunch3);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 30;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_AbbotServeLunch3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).characterPosition.position = 5800;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "916", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter1, kCharacterAbbot, 122358304, 0);
			startCycOtis(kCharacterWaiter1, "029D");
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoDialog, "Abb3016a", 0, 0, 0);
			break;
		case 2:
			send(kCharacterWaiter1, kCharacterAbbot, 122288808, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "918", 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			endGraphics(kCharacterWaiter1);
			getCharacterParams(kCharacterWaiter1, 8)[19] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_StartPart4);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 31;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Waiter1_Serving4(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterWaiter1);
		getCharacter(kCharacterWaiter1).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		getCharacter(kCharacterWaiter1).inventoryItem = 0;
		getCharacter(kCharacterWaiter1).clothes = 0;
		getCharacterParams(kCharacterWaiter1, 8)[14] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[15] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[16] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[17] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[18] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[2] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_Serving4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_Serving4);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 32;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_Serving4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterWaiter1)[0]) {
			bool skip = false;

			if (!getCharacterCurrentParams(kCharacterWaiter1)[1]) {
				getCharacterCurrentParams(kCharacterWaiter1)[1] = _gameTime + 3600;
				if (_gameTime == -3600) {
					skip = true;
					getCharacterParams(kCharacterWaiter1, 8)[15] = 1;
					getCharacterCurrentParams(kCharacterWaiter1)[0] = 0;
				}
			}

			if (!skip && getCharacterCurrentParams(kCharacterWaiter1)[1] < _gameTime) {
				getCharacterCurrentParams(kCharacterWaiter1)[1] = 0x7FFFFFFF;
				getCharacterParams(kCharacterWaiter1, 8)[15] = 1;
				getCharacterCurrentParams(kCharacterWaiter1)[0] = 0;
			}
		}

		if (inKitchen(kCharacterWaiter1) && rcClear()) {
			if (getCharacterParams(kCharacterWaiter1, 8)[14]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
				Waiter1Call(&LogicManager::CONS_Waiter1_AugustOrder4, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterWaiter1, 8)[15]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
				Waiter1Call(&LogicManager::CONS_Waiter1_ServeAugust4, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterWaiter1, 8)[16]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
				Waiter1Call(&LogicManager::CONS_Waiter1_AugustClearTable, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterWaiter1, 8)[17]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 4;
				Waiter1Call(&LogicManager::CONS_Waiter1_AbbotCheckMe, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterWaiter1, 8)[18]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 5;
				Waiter1Call(&LogicManager::CONS_Waiter1_AbbotClearTable, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterWaiter1, 8)[2]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 6;
				Waiter1Call(&LogicManager::CONS_Waiter1_RebeccaFeedUs, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			getCharacterCurrentParams(kCharacterWaiter1)[0] = 1;
			if (getCharacterParams(kCharacterWaiter1, 8)[15]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
				Waiter1Call(&LogicManager::CONS_Waiter1_ServeAugust4, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacterParams(kCharacterWaiter1, 8)[16]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
				Waiter1Call(&LogicManager::CONS_Waiter1_AugustClearTable, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (getCharacterParams(kCharacterWaiter1, 8)[17]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 4;
				Waiter1Call(&LogicManager::CONS_Waiter1_AbbotCheckMe, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (getCharacterParams(kCharacterWaiter1, 8)[18]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 5;
				Waiter1Call(&LogicManager::CONS_Waiter1_AbbotClearTable, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (getCharacterParams(kCharacterWaiter1, 8)[2]) {
				getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 6;
				Waiter1Call(&LogicManager::CONS_Waiter1_RebeccaFeedUs, 0, 0, 0, 0);
			}

			break;
		default:
			break;
		}

		break;
	case 201431954:
		getCharacter(kCharacterWaiter1).callParams[8].parameters[14] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[15] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[16] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[18] = 0;
		getCharacterParams(kCharacterWaiter1, 8)[2] = 0;
		getCharacterCurrentParams(kCharacterWaiter1)[0] = 0;
		getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
		getCharacter(kCharacterWaiter1).characterPosition.location = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_AugustOrder4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_AugustOrder4);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 33;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_AugustOrder4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "911", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterWaiter1, "010F3");
			startCycOtis(kCharacterAugust, "010D3");
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoDialog, "AUG4002", 0, 0, 0);
			break;
		case 2:
			send(kCharacterWaiter1, kCharacterAugust, 122288808, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "913", 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			endGraphics(kCharacterWaiter1);
			getCharacterParams(kCharacterWaiter1, 8)[14] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_ServeAugust4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_ServeAugust4);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 34;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_ServeAugust4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "912", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter1, kCharacterAugust, 122358304, 0);
			playDialog(kCharacterWaiter1, "AUG1053", -1, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "010G3", 0, 0, 0);
			break;
		case 2:
			send(kCharacterWaiter1, kCharacterAugust, 201964801, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "914", 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			endGraphics(kCharacterWaiter1);
			getCharacterParams(kCharacterWaiter1, 8)[15] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_AugustClearTable(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_AugustClearTable);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 35;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_AugustClearTable(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 1;
		Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "911", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8]) {
		case 1:
			send(kCharacterWaiter1, kCharacterTableD, 136455232, 0);
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 2;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoJoinedSeqOtis, "010L", 36, 103798704, "010M");
			break;
		case 2:
			getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall + 8] = 3;
			Waiter1Call(&LogicManager::CONS_Waiter1_DoSeqOtis, "914", 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterWaiter1).characterPosition.position = 5900;
			send(kCharacterWaiter1, kCharacterRebecca, 224253538, 0);
			endGraphics(kCharacterWaiter1);
			getCharacterParams(kCharacterWaiter1, 8)[16] = 0;

			getCharacter(kCharacterWaiter1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, _functionsWaiter1[getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall]]);
			fedEx(kCharacterWaiter1, kCharacterWaiter1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_StartPart5);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 36;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Waiter1_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterWaiter1);
		getCharacter(kCharacterWaiter1).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterWaiter1).characterPosition.position = 3969;
		getCharacter(kCharacterWaiter1).characterPosition.location = 1;
		getCharacter(kCharacterWaiter1).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_Prisoner);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 37;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Waiter1_Disappear(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Waiter1_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterWaiter1).callParams[getCharacter(kCharacterWaiter1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterWaiter1, &LogicManager::HAND_Waiter1_Disappear);
	getCharacter(kCharacterWaiter1).callbacks[getCharacter(kCharacterWaiter1).currentCall] = 38;

	params->clear();

	fedEx(kCharacterWaiter1, kCharacterWaiter1, 12, 0);
}

void LogicManager::HAND_Waiter1_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsWaiter1[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Waiter1_DoJoinedSeqOtis,
	&LogicManager::HAND_Waiter1_DoWait,
	&LogicManager::HAND_Waiter1_DoSeqOtis,
	&LogicManager::HAND_Waiter1_DoBlockSeqOtis,
	&LogicManager::HAND_Waiter1_FinishSeqOtis,
	&LogicManager::HAND_Waiter1_DoDialog,
	&LogicManager::HAND_Waiter1_RebeccaFeedUs,
	&LogicManager::HAND_Waiter1_RebeccaClearOurTable,
	&LogicManager::HAND_Waiter1_AbbotCheckMe,
	&LogicManager::HAND_Waiter1_AbbotClearTable,
	&LogicManager::HAND_Waiter1_Birth,
	&LogicManager::HAND_Waiter1_AnnaOrder,
	&LogicManager::HAND_Waiter1_AugustOrder,
	&LogicManager::HAND_Waiter1_ServeAnna,
	&LogicManager::HAND_Waiter1_ServeAugust,
	&LogicManager::HAND_Waiter1_ClearAnna,
	&LogicManager::HAND_Waiter1_ClearTatiana,
	&LogicManager::HAND_Waiter1_ClearAugust1,
	&LogicManager::HAND_Waiter1_ClearAugust2,
	&LogicManager::HAND_Waiter1_ServingDinner,
	&LogicManager::HAND_Waiter1_AfterDinner,
	&LogicManager::HAND_Waiter1_LockUp,
	&LogicManager::HAND_Waiter1_StartPart2,
	&LogicManager::HAND_Waiter1_InKitchen,
	&LogicManager::HAND_Waiter1_AugustComeHere2,
	&LogicManager::HAND_Waiter1_AugustClearTable2,
	&LogicManager::HAND_Waiter1_StartPart3,
	&LogicManager::HAND_Waiter1_Serving3,
	&LogicManager::HAND_Waiter1_AnnaComeHere3,
	&LogicManager::HAND_Waiter1_AbbotServeLunch3,
	&LogicManager::HAND_Waiter1_StartPart4,
	&LogicManager::HAND_Waiter1_Serving4,
	&LogicManager::HAND_Waiter1_AugustOrder4,
	&LogicManager::HAND_Waiter1_ServeAugust4,
	&LogicManager::HAND_Waiter1_AugustClearTable,
	&LogicManager::HAND_Waiter1_StartPart5,
	&LogicManager::HAND_Waiter1_Prisoner,
	&LogicManager::HAND_Waiter1_Disappear
};

} // End of namespace LastExpress
