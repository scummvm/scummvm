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

void LogicManager::CONS_Cond1(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterCond1,
			_functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]
		);

		break;
	case 1:
		CONS_Cond1_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Cond1_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Cond1_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Cond1_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Cond1_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::Cond1Call(CALL_PARAMS) {
	getCharacter(kCharacterCond1).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Cond1_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_DebugWalks);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 1;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterCond1, kCarGreenSleeping, getCharacterCurrentParams(kCharacterCond1)[0])) {
			if (getCharacterCurrentParams(kCharacterCond1)[0] == 10000) {
				getCharacterCurrentParams(kCharacterCond1)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterCond1)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterCond1).characterPosition.position = 0;
		getCharacter(kCharacterCond1).characterPosition.location = 0;
		getCharacter(kCharacterCond1).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterCond1)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_DoSeqOtis);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 &&
			nearChar(kCharacterCond1, kCharacterCath, 1000) &&
			!inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterCond1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(3, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
		fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterCond1, (char *)&getCharacterCurrentParams(kCharacterCond1)[0]);
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_DoCorrOtis);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 &&
			nearChar(kCharacterCond1, kCharacterCath, 1000) &&
			!inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 3:
		releaseAtDoor(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[3]);
		getCharacter(kCharacterCond1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
		fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterCond1, (char *)&getCharacterCurrentParams(kCharacterCond1)[0]);
		blockAtDoor(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[3]);
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_DoBriefCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_DoBriefCorrOtis);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_DoBriefCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 &&
			nearChar(kCharacterCond1, kCharacterCath, 1000) &&
			!inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 3:
	case 4:
		releaseAtDoor(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[3]);
		getCharacter(kCharacterCond1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
		fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterCond1, (char *)&getCharacterCurrentParams(kCharacterCond1)[0]);
		blockAtDoor(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[3]);
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_DoEnterCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_DoEnterCorrOtis);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;
	params->parameters[5] = param4.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_DoEnterCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 &&
			nearChar(kCharacterCond1, kCharacterCath, 1000) &&
			!inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 3:
		releaseAtDoor(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[3]);
		getCharacter(kCharacterCond1).characterPosition.position = getCharacterCurrentParams(kCharacterCond1)[4];
		getCharacter(kCharacterCond1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(3, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
		fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterCond1, (char *)&getCharacterCurrentParams(kCharacterCond1)[0]);
		blockAtDoor(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[3]);
		getCharacter(kCharacterCond1).characterPosition.position = getCharacterCurrentParams(kCharacterCond1)[4];

		if (inComp(kCharacterCath, kCarGreenSleeping, getCharacterCurrentParams(kCharacterCond1)[4]) ||
			inComp(kCharacterCath, kCarGreenSleeping, getCharacterCurrentParams(kCharacterCond1)[5])) {

			if (isNight()) {
				playNIS(kEventCathTurningNight);
			} else {
				playNIS(kEventCathTurningDay);
			}

			playDialog(0, "BUMP", -1, 0);
			bumpCathRDoor(getCharacterCurrentParams(kCharacterCond1)[3]);
		}

		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_FinishSeqOtis);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 6;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterCond1).direction == 4) {
			if (_globals[kGlobalJacket] == 1 &&
				nearChar(kCharacterCond1, kCharacterCath, 1000) &&
				!inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {

				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
				Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
			}
		} else {
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterCond1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
		fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_DoDialog);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 7;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 &&
			nearChar(kCharacterCond1, kCharacterCath, 1000) &&
			!inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 2:
		getCharacter(kCharacterCond1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
		fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		break;
	case 12:
		playDialog(kCharacterCond1, (char *)&getCharacterCurrentParams(kCharacterCond1)[0], -1, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_DoDialogFullVol(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_DoDialogFullVol);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 8;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_DoDialogFullVol(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 &&
			nearChar(kCharacterCond1, kCharacterCath, 1000) &&
			!inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 2:
		getCharacter(kCharacterCond1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
		fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		break;
	case 12:
		playDialog(kCharacterCond1, (char *)&getCharacterCurrentParams(kCharacterCond1)[0], 16, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_SaveGame);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 9;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterCond1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
		fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		break;
	case 12:
		save(
			kCharacterCond1,
			getCharacterCurrentParams(kCharacterCond1)[0],
			getCharacterCurrentParams(kCharacterCond1)[1]
		);

		getCharacter(kCharacterCond1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(3, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
		fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_DoWalk);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterCond1)[2] && nearChar(kCharacterCond1, kCharacterCath, 2000)) {
			getCharacter(kCharacterCond1).inventoryItem |= kItemInvalid; // Is this "unpickable" instead?
		} else {
			getCharacter(kCharacterCond1).inventoryItem &= ~kItemInvalid;
		}

		if (!nearChar(kCharacterCond1, kCharacterCath, 1000) ||
			inComp(kCharacterCath) || whoOutside(kCharacterCath)) {
			if (walk(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[0], getCharacterCurrentParams(kCharacterCond1)[1])) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
				fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			}

			break;
		}

		if (_globals[kGlobalJacket] == 1) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		} else if ((getCharacterParams(kCharacterCond1, 8)[5] || getCharacterParams(kCharacterCond1, 8)[6]) && !_doneNIS[kEventKronosConversation] && _globals[kGlobalJacket] == 2) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensKronosInvitation, 0, 0);
		} else if (getCharacterParams(kCharacterCond1, 8)[9] && _globals[kGlobalJacket] == 2 && !_globals[kGlobalMetAugust]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensAugustWaiting, 0, 0);
		} else {
			if (!getCharacterParams(kCharacterCond1, 8)[19] || _gameTime >= 2133000) {
				if (walk(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[0], getCharacterCurrentParams(kCharacterCond1)[1])) {
					getCharacter(kCharacterCond1).inventoryItem = kItemNone;
					getCharacter(kCharacterCond1).currentCall--;
					_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
					fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
				}

				break;
			}

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensKronosConcertInvitation, 0, 0);
		}

		break;
	case 1:
		getCharacterCurrentParams(kCharacterCond1)[2] = 0;
		getCharacter(kCharacterCond1).inventoryItem = kItemNone;

		if (_globals[kGlobalFoundCorpse] || _doneNIS[kEventMertensAskTylerCompartment] || _doneNIS[kEventMertensAskTylerCompartmentD]) {
			if (getCharacterParams(kCharacterCond1, 8)[3] && _globals[kGlobalJacket] == 2 && !_doneNIS[kEventMertensDontMakeBed] && !_globals[kGlobalCorpseHasBeenThrown]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
				Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensDontMakeBed, 0, 0);
			}
		} else {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensAskTylerCompartment, 0, 0);
		}

		break;
	case 5:
		playDialog(kCharacterCond1, "CON1110B", -1, 0);
		break;
	case 6:
		playChrExcuseMe(kCharacterCond1, kCharacterCath, 0);
		break;
	case 12:
		if ((!_globals[kGlobalFoundCorpse] && !_doneNIS[kEventMertensAskTylerCompartment] && !_doneNIS[kEventMertensAskTylerCompartmentD]) || (getCharacterParams(kCharacterCond1, 8)[3] && _globals[kGlobalJacket] == 2 && !_doneNIS[kEventMertensDontMakeBed] && !_globals[kGlobalCorpseHasBeenThrown])) {
			getCharacterCurrentParams(kCharacterCond1)[2] = 1;
		}

		if (walk(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[0], getCharacterCurrentParams(kCharacterCond1)[1])) {
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
			return;
		case 2:
			if (getCharacter(kCharacterCond1).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position) {
				playNIS(kEventMertensKronosInvitation);
			} else {
				playNIS(kEventMertensKronosInvitationClosedWindows);
			}

			_globals[kGlobalKnowAboutKronos] = 1;
			getCharacterParams(kCharacterCond1, 8)[6] = 0;
			getCharacterParams(kCharacterCond1, 8)[5] = 0;

			if (getCharacterCurrentParams(kCharacterCond1)[0] == 3) {
				if (getCharacterCurrentParams(kCharacterCond1)[1] == 8200 || getCharacterCurrentParams(kCharacterCond1)[1] == 9510) {
					getCharacter(kCharacterCond1).inventoryItem = kItemNone;
					if (getCharacter(kCharacterCond1).characterPosition.car == kCarGreenSleeping && nearX(kCharacterCond1, 2000, 500))
						getCharacter(kCharacterCond1).characterPosition.position = 2500;

					walk(kCharacterCond1, kCharacterCond1, 2000);
					bumpCathRx(getCharacter(kCharacterCond1).characterPosition.car, getCharacter(kCharacterCond1).characterPosition.position + 750);
					getCharacter(kCharacterCond1).currentCall--;
					_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
					fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
					break;
				}
			}

			if (getCharacter(kCharacterCond1).direction == 1)
				bumpCathFx(getCharacter(kCharacterCond1).characterPosition.car, getCharacter(kCharacterCond1).characterPosition.position - 750);
			else
				bumpCathRx(getCharacter(kCharacterCond1).characterPosition.car, getCharacter(kCharacterCond1).characterPosition.position + 750);

			break;
		case 3:
			playNIS(kEventMertensAugustWaiting);
			_globals[kGlobalKnowAboutAugust] = 1;
			getCharacterParams(kCharacterCond1, 8)[9] = 0;

			if (getCharacterCurrentParams(kCharacterCond1)[0] == 3 && getCharacterCurrentParams(kCharacterCond1)[1] == 8200) {
				if (getCharacter(kCharacterCond1).characterPosition.car == kCarGreenSleeping && nearX(kCharacterCond1, 2000, 500))
					getCharacter(kCharacterCond1).characterPosition.position = 2500;

				walk(kCharacterCond1, kCharacterCond1, 2000);
				bumpCathRx(getCharacter(kCharacterCond1).characterPosition.car, getCharacter(kCharacterCond1).characterPosition.position + 750);
				getCharacter(kCharacterCond1).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
				fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
				break;
			}

			if (getCharacter(kCharacterCond1).direction == 1)
				bumpCathFx(getCharacter(kCharacterCond1).characterPosition.car, getCharacter(kCharacterCond1).characterPosition.position - 750);
			else
				bumpCathRx(getCharacter(kCharacterCond1).characterPosition.car, getCharacter(kCharacterCond1).characterPosition.position + 750);

			break;
		case 4:
			playNIS(kEventMertensKronosConcertInvitation);
			getCharacterParams(kCharacterCond1, 8)[19] = 0;

			if (getCharacter(kCharacterCond1).direction == 1)
				bumpCathFx(getCharacter(kCharacterCond1).characterPosition.car, getCharacter(kCharacterCond1).characterPosition.position - 750);
			else
				bumpCathRx(getCharacter(kCharacterCond1).characterPosition.car, getCharacter(kCharacterCond1).characterPosition.position + 750);

			break;
		case 5:
			if (getCharacter(kCharacterCond1).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position)
				playNIS(kEventMertensAskTylerCompartmentD);
			else
				playNIS(kEventMertensAskTylerCompartment);

			if (getCharacter(kCharacterCond1).direction == 1)
				bumpCathFx(getCharacter(kCharacterCond1).characterPosition.car, getCharacter(kCharacterCond1).characterPosition.position - 750);
			else
				bumpCathRx(getCharacter(kCharacterCond1).characterPosition.car, getCharacter(kCharacterCond1).characterPosition.position + 750);

			return;
		case 6:
			playNIS(kEventMertensDontMakeBed);

			if (getCharacter(kCharacterCond1).direction == 1)
				bumpCathFx(getCharacter(kCharacterCond1).characterPosition.car, getCharacter(kCharacterCond1).characterPosition.position - 750);
			else
				bumpCathRx(getCharacter(kCharacterCond1).characterPosition.car, getCharacter(kCharacterCond1).characterPosition.position + 750);

			getCharacterParams(kCharacterCond1, 8)[3] = 0;
			return;
		default:
			return;
		}

		if (walk(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[0], getCharacterCurrentParams(kCharacterCond1)[1])) {
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_DoWait);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 11;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 &&
			nearChar(kCharacterCond1, kCharacterCath, 1000) &&
			!inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterCond1)[1] ||
			(getCharacterCurrentParams(kCharacterCond1)[1] = _gameTime + getCharacterCurrentParams(kCharacterCond1)[0], _gameTime + getCharacterCurrentParams(kCharacterCond1)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterCond1)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterCond1)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterCond1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
		fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_StandAsideDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_StandAsideDialog);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 12;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_StandAsideDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (!whoRunningDialog(kCharacterCond1)) {
			if (isNight()) {
				if (!isFemale(getCharacterCurrentParams(kCharacterCond1)[0])) {
					if (getCharacterCurrentParams(kCharacterCond1)[0] || _globals[kGlobalPhaseOfTheNight] != 2) {
						playDialog(kCharacterCond1, "CON1112F", -1, 0);
					} else {
						switch (rnd(3)) {
						case 0:
							playDialog(kCharacterCond1, "CON1061", -1, 0);
							break;
						case 1:
							playDialog(kCharacterCond1, "CON1110G", -1, 0);
							break;
						case 2:
						default:
							playDialog(kCharacterCond1, "CON1110H", -1, 0);
							break;
						}
					}
				} else {
					switch (rnd(2)) {
					case 0:
						playDialog(kCharacterCond1, "CON1112A", -1, 0);
						break;
					case 1:
					default:
						playDialog(kCharacterCond1, "CON1112", -1, 0);
						break;
					}
				}
			} else {
				if (!isFemale(getCharacterCurrentParams(kCharacterCond1)[0])) {
					playDialog(kCharacterCond1, "CON1112G", -1, 0);
				} else {
					switch (rnd(2)) {
					case 0:
						playDialog(kCharacterCond1, "CON1112C", -1, 0);
						break;
					case 1:
					default:
						playDialog(kCharacterCond1, "CON1112B", -1, 0);
						break;
					}
				}
			}
		}

		getCharacter(kCharacterCond1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
		fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_Passing(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_Passing);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 13;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_Passing(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 &&
			nearChar(kCharacterCond1, kCharacterCath, 1000) &&
			!inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
			break;
		}
		if (!getCharacterCurrentParams(kCharacterCond1)[1] && !getCharacterCurrentParams(kCharacterCond1)[2]) {
			if (!getCharacterCurrentParams(kCharacterCond1)[3]) {
				getCharacterCurrentParams(kCharacterCond1)[3] = _realTime + 75;
				if (_realTime + 75 == 0) {
					getCharacter(kCharacterCond1).inventoryItem = kItemNone;
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
					Cond1Call(&LogicManager::CONS_Cond1_SitDownFast, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterCond1)[3] < _realTime) {
				getCharacterCurrentParams(kCharacterCond1)[3] = 0x7FFFFFFF;
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
				Cond1Call(&LogicManager::CONS_Cond1_SitDownFast, 0, 0, 0, 0);
				break;
			}
		}

		if (!getCharacterCurrentParams(kCharacterCond1)[4]) {
			getCharacterCurrentParams(kCharacterCond1)[4] = _realTime + 225;
			if (_realTime + 225 == 0) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
				Cond1Call(&LogicManager::CONS_Cond1_SitDownFast, 0, 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterCond1)[4] >= _realTime) {
			getCharacter(kCharacterCond1).inventoryItem = _globals[kGlobalChapter] == 1 && !getCharacterParams(kCharacterCond1, 8)[16] && !_globals[kGlobalFoundCorpse] && !_doneNIS[kEventMertensAskTylerCompartment] && !_doneNIS[kEventMertensAskTylerCompartmentD];
		} else {
			getCharacterCurrentParams(kCharacterCond1)[4] = 0x7FFFFFFF;
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
			Cond1Call(&LogicManager::CONS_Cond1_SitDownFast, 0, 0, 0, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterCond1).inventoryItem = kItemNone;
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
		Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensAskTylerCompartmentD, 0, 0);
		break;
	case 11:
		getCharacterCurrentParams(kCharacterCond1)[2]++;
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 11;
		Cond1Call(&LogicManager::CONS_Cond1_StandAsideDialog, msg->sender, 0, 0, 0);
		break;
	case 12:
		if (getCharacterCurrentParams(kCharacterCond1)[1])
			getCharacterCurrentParams(kCharacterCond1)[2] = 1;

		if (whoRunningDialog(kCharacterCond1)) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_GetUpListen, 0, 0, 0, 0);
			break;
		}

		if (_globals[kGlobalChapter] == 3 && !getCharacterCurrentParams(kCharacterCond1)[0] && _gameTime < 2173500 && (_gameTime > 2106000 || (getCharacterCurrentParams(kCharacterCond1)[1] && _gameTime > 2079000))) {
			if (getCharacterCurrentParams(kCharacterCond1)[1] == 2) {
				playDialog(kCharacterCond1, "CON3052", -1, 0);
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_GetUpListen, 0, 0, 0, 0);
				break;
			}

			if (isFemale(getCharacterCurrentParams(kCharacterCond1)[1])) {
				playDialog(kCharacterCond1, "CON3051", -1, 0);
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_GetUpListen, 0, 0, 0, 0);
				break;
			}
			if (!getCharacterCurrentParams(kCharacterCond1)[1]) {
				if (_globals[kGlobalConcertIsHappening])
					playDialog(kCharacterCond1, "CON3054", -1, 0);
				else
					playDialog(kCharacterCond1, "CON3053", -1, 0);

				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_GetUpListen, 0, 0, 0, 0);
				break;
			}
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_StandAsideDialog, getCharacterCurrentParams(kCharacterCond1)[1], 0, 0, 0);
		} else {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_StandAsideDialog, getCharacterCurrentParams(kCharacterCond1)[1], 0, 0, 0);
		}

		break;
	case 16:
		getCharacterCurrentParams(kCharacterCond1)[2]--;

		if (getCharacterCurrentParams(kCharacterCond1)[1] && !getCharacterCurrentParams(kCharacterCond1)[2]) {
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 10;
			Cond1Call(&LogicManager::CONS_Cond1_SitDownFast, 0, 0, 0, 0);
		}
		return;
	case 17:
		if (checkCathDir(kCarGreenSleeping, 23) && getCharacterParams(kCharacterCond1, 8)[6] && !_doneNIS[kEventKronosConversation]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensKronosInvitation, 0, 0);
		} else {
			if (!getCharacterCurrentParams(kCharacterCond1)[2]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
				Cond1Call(&LogicManager::CONS_Cond1_SitDownFast, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
		case 2:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_GetUpListen, 0, 0, 0, 0);
			break;
		case 3:
			if (!getCharacterCurrentParams(kCharacterCond1)[0]) {
				startCycOtis(kCharacterCond1, "601H");
			} else {
				startCycOtis(kCharacterCond1, "601I");
			}

			break;
		case 4:
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
			break;
		case 5:
		case 6:
		case 9:
		case 10:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 7:
			playNIS(kEventMertensAskTylerCompartmentD);
			bumpCath(kCarGreenSleeping, 25, 255);
			break;
		case 8:
			playNIS(kEventMertensKronosInvitation);
			getCharacterParams(kCharacterCond1, 8)[6] = 0;
			getCharacterParams(kCharacterCond1, 8)[5] = 0;
			cleanNIS();

			if (!getCharacterCurrentParams(kCharacterCond1)[2]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
				Cond1Call(&LogicManager::CONS_Cond1_SitDownFast, 0, 0, 0, 0);
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

void LogicManager::CONS_Cond1_Listen(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_Listen);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 14;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_Listen(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalJacket] == 1 &&
			nearChar(kCharacterCond1, kCharacterCath, 1000) &&
			!inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterCond1).inventoryItem = kItemNone;
		if (getCharacterParams(kCharacterCond1, 8)[16]) {
			getCharacterParams(kCharacterCond1, 8)[16] = 0;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 1500, 0, 0);
		} else {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_DoWait, 15, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			send(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[0], 202558662, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_GetUpListen, 0, 0, 0, 0);
			break;
		case 2:
			send(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[0], 155853632, 0);
			startCycOtis(kCharacterCond1, "601K");
			break;
		case 3:
			send(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[0], 202558662, 0);
			send(kCharacterCond1, getCharacterCurrentParams(kCharacterCond1)[0], 155853632, 0);
			startCycOtis(kCharacterCond1, "601K");
			takeItem(kItem7);
			break;
		case 4:
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
			break;
		case 5:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 125499160:
		if (getCharacterCurrentParams(kCharacterCond1)[0] == 9)
			getCharacterParams(kCharacterCond1, 8)[7] = 0;

		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
		Cond1Call(&LogicManager::CONS_Cond1_SitDownFast, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_AlexeiLockUnlockMyComp);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 15;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_AlexeiLockUnlockMyComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacterParams(kCharacterCond1, 8)[11] = 0;
		getCharacterParams(kCharacterCond1, 8)[12] = 0;
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 4070, 0, 0);
			break;
		case 2:
			if (getCharacterCurrentParams(kCharacterCond1)[0]) {
				playDialog(kCharacterCond1, "CON1059A", -1, 0);
			} else {
				playDialog(kCharacterCond1, "CON1059", -1, 0);
			}

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 7500, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Xb", 2, 0, 0);
			break;
		case 4:
			send(kCharacterCond1, kCharacterAlexei, 135664192, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_AugustLockUnlockMyComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_AugustLockUnlockMyComp);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 16;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_AugustLockUnlockMyComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacterParams(kCharacterCond1, 8)[14] = 0;
		getCharacterParams(kCharacterCond1, 8)[13] = 0;
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 4070, 0, 0);
			break;
		case 2:
			switch (rnd(4)) {
			case 0:
				playDialog(kCharacterCond1, "AUG2095A", -1, 0);
				break;
			case 1:
				playDialog(kCharacterCond1, "AUG2096A", -1, 0);
				break;
			case 2:
				playDialog(kCharacterCond1, "AUG2094B", -1, 0);
				break;
			case 3:
				playDialog(kCharacterCond1, "AUG2094C", -1, 0);
				break;
			default:
				break;
			}

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 6470, 0, 0);
			break;
		case 3:
			if (getCharacterCurrentParams(kCharacterCond1)[0]) {
				playDialog(kCharacterCond1, "AUG2097", -1, 0);
			} else {
				playDialog(kCharacterCond1, "AUG2098", -1, 0);
			}

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Xc", 3, 0, 0);
			break;
		case 4:
			send(kCharacterCond1, kCharacterAugust, 69239528, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_SitDown(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_SitDown);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 17;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_SitDown(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterParams(kCharacterCond1, 8)[5]  ||
			getCharacterParams(kCharacterCond1, 8)[9]  ||
			getCharacterParams(kCharacterCond1, 8)[13] ||
			getCharacterParams(kCharacterCond1, 8)[14] ||
			getCharacterParams(kCharacterCond1, 8)[11] ||
			getCharacterParams(kCharacterCond1, 8)[12] ||
			getCharacterParams(kCharacterCond1, 8)[8]  ||
			getCharacterParams(kCharacterCond1, 8)[10] ||
			getCharacterParams(kCharacterCond1, 8)[15]) {

			dropItem(kItem7, 1);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 540, 0, 0);
		} else if (getCharacterParams(kCharacterCond1, 8)[7]) {
			startCycOtis(kCharacterCond1, "601K");
			takeItem(kItem7);
			getCharacterParams(kCharacterCond1, 8)[16] = 1;

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		} else {
			if (!cathHasItem(kItemPassengerList) || getCharacterParams(kCharacterCond1, 8)[1]) {
				startSeqOtis(kCharacterCond1, "601A");
			} else {
				getCharacterParams(kCharacterCond1, 8)[1] = 1;
				playDialog(kCharacterCond1, "CON1058", -1, 75);
				startSeqOtis(kCharacterCond1, "601D");
			}

			takeItem(kItem7);

			if (checkCathDir(kCarGreenSleeping, 68)) {
				playDialog(kCharacterCath, "CON1110", -1, 0);
				bumpCath(kCarGreenSleeping, 25, 255);
			}

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_FinishSeqOtis, 0, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			endGraphics(kCharacterCond1);
			getCharacterParams(kCharacterCond1, 8)[16] = 1;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoWait, 75, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 3:
			if (!getCharacterParams(kCharacterCond1, 8)[2] && !cathHasItem(kItemPassengerList)) {
				if (getCharacterParams(kCharacterCond1, 8)[1]) {
					send(kCharacterCond1, kCharacterTrainM, 158617345, 0);
					getCharacterParams(kCharacterCond1, 8)[2] = 1;
				}
			}

			startCycOtis(kCharacterCond1, "601B");
			getCharacterParams(kCharacterCond1, 8)[0] = 0;
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			send(kCharacterCond1, kCharacterCond1, 17, 0);

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_SitDownFast(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_SitDownFast);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 18;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_SitDownFast(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterParams(kCharacterCond1, 8)[5]  ||
			getCharacterParams(kCharacterCond1, 8)[9]  ||
			getCharacterParams(kCharacterCond1, 8)[13] ||
			getCharacterParams(kCharacterCond1, 8)[14] ||
			getCharacterParams(kCharacterCond1, 8)[11] ||
			getCharacterParams(kCharacterCond1, 8)[12] ||
			getCharacterParams(kCharacterCond1, 8)[8]  ||
			getCharacterParams(kCharacterCond1, 8)[10] ||
			getCharacterParams(kCharacterCond1, 8)[15]) {

			dropItem(kItem7, 1);

			getCharacterParams(kCharacterCond1, 8)[16] = 1;

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		} else if (getCharacterParams(kCharacterCond1, 8)[7]) {
			takeItem(kItem7);
			getCharacterParams(kCharacterCond1, 8)[16] = 1;

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		} else {
			if (!cathHasItem(kItemPassengerList) || getCharacterParams(kCharacterCond1, 8)[1]) {
				startSeqOtis(kCharacterCond1, "601A");
			} else {
				getCharacterParams(kCharacterCond1, 8)[1] = 1;
				playDialog(kCharacterCond1, "CON1058", -1, 75);
				startSeqOtis(kCharacterCond1, "601D");
			}

			takeItem(kItem7);

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_FinishSeqOtis, 0, 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			if (!getCharacterParams(kCharacterCond1, 8)[2] && !cathHasItem(kItemPassengerList)) {
				if (getCharacterParams(kCharacterCond1, 8)[1]) {
					send(kCharacterCond1, kCharacterTrainM, 158617345, 0);
					getCharacterParams(kCharacterCond1, 8)[2] = 1;
				}
			}

			startCycOtis(kCharacterCond1, "601B");
			getCharacterParams(kCharacterCond1, 8)[0] = 0;
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_GetUp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_GetUp);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 19;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_GetUp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterParams(kCharacterCond1, 8)[16]) {
			dropItem(kItem7, 1);
			getCharacterParams(kCharacterCond1, 8)[16] = 0;

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		} else {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_DoSeqOtis, "601C", 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			dropItem(kItem7, 1);
			if (!checkCathDir(kCarGreenSleeping, 2))
				getCharacter(kCharacterCond1).characterPosition.position = 2088;

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		}

		break;
	}
}

void LogicManager::CONS_Cond1_GetUpListen(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_GetUpListen);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 20;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_GetUpListen(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		takeItem(kItem7);

		if (getCharacterParams(kCharacterCond1, 8)[16]) {
			getCharacterParams(kCharacterCond1, 8)[16] = 0;
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		} else {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_DoSeqOtis, "601C", 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_MakeBed(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_MakeBed);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 21;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_MakeBed(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		bool skip = false; // Horrible way to unroll a goto...

		if (!getCharacterCurrentParams(kCharacterCond1)[11]) {
			getCharacterCurrentParams(kCharacterCond1)[11] = _gameTime + 300;
			if (_gameTime + 300 == 0) {
				skip = true;
				playDialog(0, "ZFX1004", getVolume(kCharacterCond1), 0);
			}
		}

		if (!skip && getCharacterCurrentParams(kCharacterCond1)[11] < _gameTime) {
			getCharacterCurrentParams(kCharacterCond1)[11] = 0x7FFFFFFF;
			playDialog(0, "ZFX1004", getVolume(kCharacterCond1), 0);
		}

		if (getCharacterCurrentParams(kCharacterCond1)[12] || (getCharacterCurrentParams(kCharacterCond1)[12] = _gameTime + 900, _gameTime != -900)) {
			if (getCharacterCurrentParams(kCharacterCond1)[12] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterCond1)[12] = 0x7FFFFFFF;
		}

		setModel(getCharacterCurrentParams(kCharacterCond1)[0], 1);

		if (getCharacterCurrentParams(kCharacterCond1)[4] != 2)
			setDoor(getCharacterCurrentParams(kCharacterCond1)[0], getCharacterCurrentParams(kCharacterCond1)[3], getCharacterCurrentParams(kCharacterCond1)[4], getCharacterCurrentParams(kCharacterCond1)[5], getCharacterCurrentParams(kCharacterCond1)[6]);

		if (getCharacterCurrentParams(kCharacterCond1)[1])
			setDoor(getCharacterCurrentParams(kCharacterCond1)[1], getCharacterCurrentParams(kCharacterCond1)[7], getCharacterCurrentParams(kCharacterCond1)[8], getCharacterCurrentParams(kCharacterCond1)[9], getCharacterCurrentParams(kCharacterCond1)[10]);

		getCharacter(kCharacterCond1).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
		fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		break;
	}
	case 8:
	case 9:
		setDoor(getCharacterCurrentParams(kCharacterCond1)[0], kCharacterCond1, 1, 0, 0);

		if (getCharacterCurrentParams(kCharacterCond1)[1])
			setDoor(getCharacterCurrentParams(kCharacterCond1)[1], kCharacterCond1, 1, 0, 0);

		if (msg->action == 8) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "LIB012", 0, 0, 0);
		} else {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "LIB013", 0, 0, 0);
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterCond1)[2] = 1;
		getCharacterCurrentParams(kCharacterCond1)[3] = _doors[getCharacterCurrentParams(kCharacterCond1)[0]].who;
		getCharacterCurrentParams(kCharacterCond1)[4] = _doors[getCharacterCurrentParams(kCharacterCond1)[0]].status;
		getCharacterCurrentParams(kCharacterCond1)[5] = _doors[getCharacterCurrentParams(kCharacterCond1)[0]].windowCursor;
		getCharacterCurrentParams(kCharacterCond1)[6] = _doors[getCharacterCurrentParams(kCharacterCond1)[0]].handleCursor;

		if (getCharacterCurrentParams(kCharacterCond1)[1]) {
			getCharacterCurrentParams(kCharacterCond1)[7] = _doors[getCharacterCurrentParams(kCharacterCond1)[1]].who;
			getCharacterCurrentParams(kCharacterCond1)[8] = _doors[getCharacterCurrentParams(kCharacterCond1)[1]].status;
			getCharacterCurrentParams(kCharacterCond1)[9] = _doors[getCharacterCurrentParams(kCharacterCond1)[1]].windowCursor;
			getCharacterCurrentParams(kCharacterCond1)[10] = _doors[getCharacterCurrentParams(kCharacterCond1)[1]].handleCursor;
			setDoor(getCharacterCurrentParams(kCharacterCond1)[1], kCharacterCond1, 1, 10, 9);
		}

		if (getCharacterCurrentParams(kCharacterCond1)[4] != 2)
			setDoor(getCharacterCurrentParams(kCharacterCond1)[0], kCharacterCond1, 1, 10, 9);

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
		case 2:
			if (getCharacterCurrentParams(kCharacterCond1)[2]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "Con1017", 0, 0, 0);
			} else {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
				Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "Con1017A", 0, 0, 0);
			}
			break;
		case 3:
		case 4:
			getCharacterCurrentParams(kCharacterCond1)[2] = 0;
			setDoor(getCharacterCurrentParams(kCharacterCond1)[0], kCharacterCond1, 1, 10, 9);
			if (getCharacterCurrentParams(kCharacterCond1)[1])
				setDoor(getCharacterCurrentParams(kCharacterCond1)[1], kCharacterCond1, 1, 10, 9);

			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_MakeBedMahmud(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_MakeBedMahmud);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 22;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_MakeBedMahmud(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2740, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Mh", 8, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond1, "601Nh");
			softBlockAtDoor(kCharacterCond1, 8);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoWait, 150, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Mh", 8, 0, 0);
			break;
		case 4:
			startCycOtis(kCharacterCond1, "601Nh");
			softBlockAtDoor(kCharacterCond1, 8);
			send(kCharacterCond1, kCharacterMahmud, 225563840, 0);
			break;
		case 5:
			if (!whoRunningDialog(kCharacterCond1))
				playDialog(kCharacterCond1, "MAH1170I", -1, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Zd", 4, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond1).characterPosition.location = 1;
			endGraphics(kCharacterCond1);
			if (!whoRunningDialog(kCharacterCond1))
				playDialog(kCharacterCond1, "MAH1172", -1, 225);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBed, 4, 20, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "671Ad", 4, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterCond1).characterPosition.location = 0;
			send(kCharacterCond1, kCharacterMahmud, 123852928, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 540, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 102227384:
		startCycOtis(kCharacterCond1, "671Dh");
		break;
	case 156567128:
		softReleaseAtDoor(kCharacterCond1, 8);
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
		Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 5790, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_MakeBedMahmud2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_MakeBedMahmud2);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 23;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_MakeBedMahmud2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 5790, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Vd", 4, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond1, "601Wd");
			softBlockAtDoor(kCharacterCond1, 4);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoWait, 150, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Zd", 4, 0, 0);
			break;
		case 4:
			softReleaseAtDoor(kCharacterCond1, 4);
			getCharacter(kCharacterCond1).characterPosition.location = 1;
			endGraphics(kCharacterCond1);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBed, 4, 20, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "671Ad", 4, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond1).characterPosition.location = 0;

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_MakeBedAugust(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_MakeBedAugust);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 24;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_MakeBedAugust(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterCond1)[0]) {
			if (getCharacterCurrentParams(kCharacterCond1)[1] || (getCharacterCurrentParams(kCharacterCond1)[1] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterCond1)[1] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterCond1)[1] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoEnterCorrOtis, "601Rc", 3, 6470, 6130);
		}

		break;
	case 12:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 6470, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Mc", 3, 0, 0);
			break;
		case 2:
			send(kCharacterCond1, kCharacterAugust, 221617184, 0);
			startCycOtis(kCharacterCond1, "601Nc");
			softBlockAtDoor(kCharacterCond1, 3);
			break;
		case 3:
			softReleaseAtDoor(kCharacterCond1, 3);
			getCharacter(kCharacterCond1).characterPosition.location = 1;
			endGraphics(kCharacterCond1);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBed, 3, 19, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Sc", 3, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond1).characterPosition.location = 0;
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 6:
			softReleaseAtDoor(kCharacterCond1, 3);
			setDoor(3, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterCond1).characterPosition.location = 1;
			endGraphics(kCharacterCond1);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBed, 3, 19, 0, 0);
			break;
		case 7:
			setDoor(3, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Uc", 3, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterCond1).characterPosition.location = 0;
			send(kCharacterCond1, kCharacterAugust, 124697504, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 540, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 100906246:
		send(kCharacterCond1, kCharacterAugust, 192849856, 0);
		startCycOtis(kCharacterCond1, "601Qc");
		break;
	case 102675536:
		getCharacterCurrentParams(kCharacterCond1)[0] = 1;
		break;
	case 156567128:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
		Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Tc", 3, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_MakeBedAlexei(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_MakeBedAlexei);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 25;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_MakeBedAlexei(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterCond1)[0]) {
			if (getCharacterCurrentParams(kCharacterCond1)[1] ||
				(getCharacterCurrentParams(kCharacterCond1)[1] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterCond1)[1] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterCond1)[1] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoEnterCorrOtis, "601Zb", 2, 7500, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 7500, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Vb", 2, 0, 0);
			break;
		case 2:
			send(kCharacterCond1, kCharacterAlexei, 221617184, 0);
			startCycOtis(kCharacterCond1, "601Wb");
			softBlockAtDoor(kCharacterCond1, 2);
			break;
		case 3:
			softReleaseAtDoor(kCharacterCond1, 2);
			getCharacter(kCharacterCond1).characterPosition.location = 1;
			endGraphics(kCharacterCond1);

			if (_globals[kGlobalChapter] == 1 &&
				getCharacterParams(kCharacterCond1, 8)[3] &&
				_globals[kGlobalCharacterSearchingForCath] != kCharacterPolice) {
				_globals[kGlobalCharacterSearchingForCath] = kCharacterCond1;
			}

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBed, 2, 18, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "671Ab", 2, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond1).characterPosition.location = 0;
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 6:
			softReleaseAtDoor(kCharacterCond1, 2);
			setDoor(2, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterCond1).characterPosition.location = 1;
			endGraphics(kCharacterCond1);

			if (_globals[kGlobalChapter] == 1 &&
				getCharacterParams(kCharacterCond1, 8)[3] &&
				_globals[kGlobalCharacterSearchingForCath] != kCharacterPolice) {
				_globals[kGlobalCharacterSearchingForCath] = kCharacterCond1;
			}

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBed, 2, 0x12, 0, 0);
			break;
		case 7:
			playDialog(kCharacterCond1, "Con1024A", -1, 0);
			setDoor(2, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "641Ub", 2, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterCond1).characterPosition.location = 0;
			send(kCharacterCond1, kCharacterAlexei, 124697504, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 9460, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 100906246:
		getCharacterCurrentParams(kCharacterCond1)[0] = 1;
		break;
	case 156567128:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
		Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "641Tb", 2, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_SubMakeBed(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_SubMakeBed);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 26;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_SubMakeBed(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (!_globals[kGlobalCorpseHasBeenThrown] &&
			getCharacterCurrentParams(kCharacterCond1)[0] &&
			_globals[kGlobalChapter] == 1 && _globals[kGlobalJacket] == 2) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 1, 0, 0, 0);
		} else {
			getCharacter(kCharacterCond1).characterPosition.location = 1;
			endGraphics(kCharacterCond1);
			setDoor(1, kCharacterCath, checkDoor(1), 0, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoDialogFullVol, "ZNU1001", 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			setDoor(1, kCharacterCath, checkDoor(1), 0, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoDialogFullVol, "CON1062", 0, 0, 0);
			break;
		case 2:
			setDoor(1, kCharacterCath, checkDoor(1), 10, 9);

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 3:
			if (_globals[kGlobalJacket] == 1) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
				Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
			} else if (_globals[kGlobalCorpseMovedFromFloor]) {
				blockAtDoor(kCharacterCond1, 1);
				startSeqOtis(kCharacterCond1, "601Ra");
				bumpCath(kCarGreenSleeping, 16, 255);
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
				Cond1Call(&LogicManager::CONS_Cond1_FinishSeqOtis, 0, 0, 0, 0);
			} else {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
				Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensCorpseFloor, 0, 0);
			}

			break;
		case 4:
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
			break;
		case 5:
			playNIS(kEventMertensCorpseFloor);
			endGame(0, 1, _globals[kGlobalFoundCorpse] == 0 ? 56 : 50, true);
			break;
		case 6:
			releaseAtDoor(kCharacterCond1, 1);
			getCharacter(kCharacterCond1).characterPosition.location = 1;
			endGraphics(kCharacterCond1);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBed, 1, 17, 0, 0);
			break;
		case 7:
			if (_globals[kGlobalCorpseHasBeenThrown] || _globals[kGlobalChapter] != 1) {
				if (nearChar(kCharacterCond1, kCharacterCath, 1000) && !whoOutside(kCharacterCath))
					playDialog(kCharacterCond1, "CON1061", -1, 0);

				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
				Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Sa", 1, 0, 0);
			} else {
				if (!checkLoc(kCharacterCath, 3))
					bumpCath(kCarNone, 1, 255);

				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
				Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensCorpseBed, 0, 0);
			}

			break;
		case 8:
			playNIS(kEventMertensCorpseBed);
			endGame(0, 1, 57, true);
			break;
		case 9:
			getCharacter(kCharacterCond1).characterPosition.location = 0;
			setDoor(1, kCharacterCath, 0, 10, 9);

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_EnterTyler(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_EnterTyler);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 27;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_EnterTyler(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 10:
		if (msg->action == 0) {
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterPolice) {
				getCharacter(kCharacterCond1).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
				fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
				break;
			}

			if (!getCharacterCurrentParams(kCharacterCond1)[1]) {
				getCharacterCurrentParams(kCharacterCond1)[1] = _realTime + 150;
				if (_realTime == -150) {
					setDoor(1, kCharacterCath, checkDoor(1), 0, 0);
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 10;
					Cond1Call(&LogicManager::CONS_Cond1_DoDialogFullVol, "CON1018A", 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterCond1)[1] < _realTime) {
				getCharacterCurrentParams(kCharacterCond1)[1] = 0x7FFFFFFF;
				setDoor(1, kCharacterCath, checkDoor(1), 0, 0);
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 10;
				Cond1Call(&LogicManager::CONS_Cond1_DoDialogFullVol, "CON1018A", 0, 0, 0);
				break;
			}
		} else if (msg->action == 10) {
			setDoor(1, kCharacterCond1, checkDoor(1), 14, 9);
		}

		if (!getCharacterCurrentParams(kCharacterCond1)[2]) {
			getCharacterCurrentParams(kCharacterCond1)[2] = _realTime + 300;
			if (_realTime == -300) {
				if (checkDoor(1) == 1) {
					setDoor(1, kCharacterCath, checkDoor(1), 0, 0);
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 11;
					Cond1Call(&LogicManager::CONS_Cond1_DoDialogFullVol, "CON1018B", 0, 0, 0);
				} else {
					playDialog(0, "LIB014", -1, 0);
					if (_globals[kGlobalCorpseMovedFromFloor]) {
						if (_globals[kGlobalJacket] == 1) {
							getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 13;
							Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
						} else if (getCharacterCurrentParams(kCharacterCond1)[0]) {
							setDoor(1, kCharacterCath, 0, 10, 9);

							if (getCharacterCurrentParams(kCharacterCond1)[0] == 1) {
								getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 15;
								Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensAugustWaitingCompartment, 0, 0);
							} else {
								if (getCharacterCurrentParams(kCharacterCond1)[0] != 2) {
									if (getCharacterCurrentParams(kCharacterCond1)[0] != 3) {
										playDialog(0, "LIB015", -1, 0);
										bumpCathTylerComp();

										getCharacter(kCharacterCond1).currentCall--;
										_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
										fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
										break;
									}

									if (isNight()) {
										playNIS(kEventMertensPushCallNight);
									} else {
										playNIS(kEventMertensPushCall);
									}

									playDialog(0, "LIB015", -1, 0);
									bumpCathTylerComp();

									getCharacter(kCharacterCond1).currentCall--;
									_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
									fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
									break;
								}

								getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 16;
								Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensKronosInvitationCompartment, 0, 0);
							}
						} else {
							getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 14;
							Cond1Call(&LogicManager::CONS_Cond1_SubMakeBed, 0, 0, 0, 0);
						}
					} else {
						if (!checkLoc(kCharacterCath, 3))
							bumpCath(kCarNone, 1, 255);

						getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 12;
						Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensCorpseFloor, 0, 0);
					}
				}
			}
		}

		if (getCharacterCurrentParams(kCharacterCond1)[2] >= _realTime) {
			if (getCharacterCurrentParams(kCharacterCond1)[3] || (getCharacterCurrentParams(kCharacterCond1)[3] = _realTime + 375, _realTime != -375)) {
				if (getCharacterCurrentParams(kCharacterCond1)[3] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterCond1)[3] = 0x7FFFFFFF;
			}

			playDialog(0, "LIB033", -1, 0);

			if (_globals[kGlobalCorpseMovedFromFloor]) {
				if (_globals[kGlobalJacket] == 1) {
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 18;
					Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
				} else if (getCharacterCurrentParams(kCharacterCond1)[0]) {
					setDoor(1, kCharacterCath, 0, 10, 9);

					if (getCharacterCurrentParams(kCharacterCond1)[0] == 1) {
						getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 20;
						Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensAugustWaitingCompartment, 0, 0);
					} else {
						if (getCharacterCurrentParams(kCharacterCond1)[0] != 2) {
							if (getCharacterCurrentParams(kCharacterCond1)[0] != 3) {
								playDialog(0, "LIB015", -1, 0);
								bumpCathTylerComp();

								getCharacter(kCharacterCond1).currentCall--;
								_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
								fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
								break;
							}

							if (isNight()) {
								playNIS(kEventMertensPushCallNight);
							} else {
								playNIS(kEventMertensPushCall);
							}

							playDialog(0, "LIB015", -1, 0);
							bumpCathTylerComp();

							getCharacter(kCharacterCond1).currentCall--;
							_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
							fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
							break;
						}

						getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 21;
						Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensKronosInvitationCompartment, 0, 0);
					}
				} else {
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 19;
					Cond1Call(&LogicManager::CONS_Cond1_SubMakeBed, 0, 0, 0, 0);
				}
			} else {
				if (!checkLoc(kCharacterCath, 3))
					bumpCath(kCarNone, 1, 255);

				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 17;
				Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensCorpseFloor, 0, 0);
			}
		} else {
			getCharacterCurrentParams(kCharacterCond1)[2] = 0x7FFFFFFF;

			if (checkDoor(1) == 1) {
				setDoor(1, kCharacterCath, checkDoor(1), 0, 0);
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 11;
				Cond1Call(&LogicManager::CONS_Cond1_DoDialogFullVol, "CON1018B", 0, 0, 0);
			} else {
				playDialog(0, "LIB014", -1, 0);
				if (_globals[kGlobalCorpseMovedFromFloor]) {
					if (_globals[kGlobalJacket] == 1) {
						getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 13;
						Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
					} else if (getCharacterCurrentParams(kCharacterCond1)[0]) {
						setDoor(1, kCharacterCath, 0, 10, 9);

						if (getCharacterCurrentParams(kCharacterCond1)[0] == 1) {
							getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 15;
							Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensAugustWaitingCompartment, 0, 0);
						} else {
							if (getCharacterCurrentParams(kCharacterCond1)[0] != 2) {
								if (getCharacterCurrentParams(kCharacterCond1)[0] != 3) {
									playDialog(0, "LIB015", -1, 0);
									bumpCathTylerComp();

									getCharacter(kCharacterCond1).currentCall--;
									_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
									fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
									break;
								}

								if (isNight()) {
									playNIS(kEventMertensPushCallNight);
								} else {
									playNIS(kEventMertensPushCall);
								}

								playDialog(0, "LIB015", -1, 0);
								bumpCathTylerComp();

								getCharacter(kCharacterCond1).currentCall--;
								_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
								fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
								break;
							}

							getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 16;
							Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensKronosInvitationCompartment, 0, 0);
						}
					} else {
						getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 14;
						Cond1Call(&LogicManager::CONS_Cond1_SubMakeBed, 0, 0, 0, 0);
					}
				} else {
					if (!checkLoc(kCharacterCath, 3))
						bumpCath(kCarNone, 1, 255);

					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 12;
					Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensCorpseFloor, 0, 0);
				}
			}
		}

		break;
	case 8:
		if (getCharacterCurrentParams(kCharacterCond1)[0]) {
			setDoor(1, kCharacterCath, checkDoor(1), 0, 0);

			if (getCharacterCurrentParams(kCharacterCond1)[0] == 1) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 23;
				Cond1Call(&LogicManager::CONS_Cond1_DoDialogFullVol, "CON1018D", 0, 0, 0);
			} else if (getCharacterCurrentParams(kCharacterCond1)[0] == 2) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 24;
				Cond1Call(&LogicManager::CONS_Cond1_DoDialogFullVol, "CON1018E", 0, 0, 0);
			} else if (getCharacterCurrentParams(kCharacterCond1)[0] == 3) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 25;
				Cond1Call(&LogicManager::CONS_Cond1_DoDialogFullVol, "CON1025", 0, 0, 0);
			} else {
				setDoor(1, kCharacterCath, checkDoor(1), 10, 9);

				getCharacter(kCharacterCond1).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
				fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			}
		} else {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 22;
			Cond1Call(&LogicManager::CONS_Cond1_SubMakeBed, 1, 0, 0, 0);
		}

		break;
	case 9:
		if (checkDoor(1) != 1) {
			playDialog(0, "LIB014", -1, 0);
		} else {
			playDialog(0, "LIB032", -1, 0);
		}

		if (_globals[kGlobalCorpseMovedFromFloor]) {
			if (_globals[kGlobalJacket] == 1) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 27;
				Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
			} else if (getCharacterCurrentParams(kCharacterCond1)[0]) {
				setDoor(1, kCharacterCath, 0, 10, 9);

				if (getCharacterCurrentParams(kCharacterCond1)[0] == 1) {
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 29;
					Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensAugustWaitingCompartment, 0, 0);
				} else if (getCharacterCurrentParams(kCharacterCond1)[0] == 2) {
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 30;
					Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensKronosInvitationCompartment, 0, 0);
				} else {
					if (getCharacterCurrentParams(kCharacterCond1)[0] == 3) {
						if (isNight()) {
							playNIS(kEventMertensPushCallNight);
						} else {
							playNIS(kEventMertensPushCall);
						}
					}

					playDialog(0, "LIB015", -1, 0);
					bumpCathTylerComp();

					getCharacter(kCharacterCond1).currentCall--;
					_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
					fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
				}
			} else {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 28;
				Cond1Call(&LogicManager::CONS_Cond1_SubMakeBed, 0, 0, 0, 0);
			}
		} else {
			if (!checkLoc(kCharacterCath, 3))
				bumpCath(kCarNone, 1, 255);

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 26;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensCorpseFloor, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterCond1).inventoryItem = kItemNone;
		if (inComp(kCharacterCath, kCarGreenSleeping, 8200) ||
			inComp(kCharacterCath, kCarGreenSleeping, 7850) ||
			cathOutHisWindow()) {

			setDoor(1, kCharacterCath, checkDoor(1), 0, 0);
			if (cathOutHisWindow())
				bumpCath(kCarGreenSleeping, 49, 255);
			if (getCharacterCurrentParams(kCharacterCond1)[0]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
				Cond1Call(&LogicManager::CONS_Cond1_DoDialogFullVol, "CON1018", 0, 0, 0);
			} else {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
				Cond1Call(&LogicManager::CONS_Cond1_DoDialogFullVol, "CON1060", 0, 0, 0);
			}
		} else {
			playDialog(kCharacterCond1, "CON1019", -1, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Ma", 1, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			if (_globals[kGlobalCorpseMovedFromFloor]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
				Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Ra", 1, 0, 0);
			} else if (checkLoc(kCharacterCath, 3)) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
				Cond1Call(&LogicManager::CONS_Cond1_DoBriefCorrOtis, "601Ra", 1, 0, 0);
			} else {
				bumpCath(kCarNone, 1, 255);
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensCorpseFloor, 0, 0);
			}

			break;
		case 2:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensCorpseFloor, 0, 0);
			break;
		case 3:
		case 12:
		case 17:
		case 26:
			playNIS(kEventMertensCorpseFloor);
			endGame(0, 1, _globals[kGlobalFoundCorpse] == 0 ? 56 : 50, true);
			return;
		case 4:
			setDoor(1, kCharacterCath, 1, 0, 0);
			getCharacter(kCharacterCond1).characterPosition.location = 1;
			endGraphics(kCharacterCond1);

			if (getCharacterCurrentParams(kCharacterCond1)[0]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
				Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Sa", 1, 0, 0);
				break;
			}

			if (_globals[kGlobalCorpseHasBeenThrown] || _globals[kGlobalChapter] != 1) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
				Cond1Call(&LogicManager::CONS_Cond1_MakeBed, 1, 17, 0, 0);
			} else {
				if (!checkLoc(kCharacterCath, 3))
					bumpCath(kCarNone, 1, 255);

				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
				Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensCorpseBed, 0, 0);
			}

			break;
		case 5:
			playNIS(kEventMertensCorpseBed);
			endGame(0, 1, 57, true);
			break;
		case 6:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Sa", 1, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond1).characterPosition.location = 0;
			setDoor(1, kCharacterCath, 0, 10, 9);

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 8:
		case 9:
			setDoor(1, kCharacterCond1, checkDoor(1), 14, 9);
			break;
		case 11:
		case 13:
			if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 11) {
				setDoor(1, kCharacterCond1, checkDoor(1), 14, 9);
			} else if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 13) {
				playNIS(kEventMertensBloodJacket);
				endGame(0, 1, 55, 1);
			}

			if (getCharacterCurrentParams(kCharacterCond1)[3] || (getCharacterCurrentParams(kCharacterCond1)[3] = _realTime + 375, _realTime != -375)) {
				if (getCharacterCurrentParams(kCharacterCond1)[3] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterCond1)[3] = 0x7FFFFFFF;
			}

			playDialog(0, "LIB033", -1, 0);

			if (_globals[kGlobalCorpseMovedFromFloor]) {
				if (_globals[kGlobalJacket] == 1) {
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 18;
					Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
				} else if (getCharacterCurrentParams(kCharacterCond1)[0]) {
					setDoor(1, kCharacterCath, 0, 10, 9);

					if (getCharacterCurrentParams(kCharacterCond1)[0] == 1) {
						getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 20;
						Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensAugustWaitingCompartment, 0, 0);
					} else {
						if (getCharacterCurrentParams(kCharacterCond1)[0] != 2) {
							if (getCharacterCurrentParams(kCharacterCond1)[0] != 3) {
								playDialog(0, "LIB015", -1, 0);
								bumpCathTylerComp();

								getCharacter(kCharacterCond1).currentCall--;
								_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
								fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
								break;
							}

							if (isNight()) {
								playNIS(kEventMertensPushCallNight);
							} else {
								playNIS(kEventMertensPushCall);
							}

							playDialog(0, "LIB015", -1, 0);
							bumpCathTylerComp();

							getCharacter(kCharacterCond1).currentCall--;
							_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
							fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
							break;
						}

						getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 21;
						Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensKronosInvitationCompartment, 0, 0);
					}
				} else {
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 19;
					Cond1Call(&LogicManager::CONS_Cond1_SubMakeBed, 0, 0, 0, 0);
				}
			} else {
				if (!checkLoc(kCharacterCath, 3))
					bumpCath(kCarNone, 1, 255);

				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 17;
				Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensCorpseFloor, 0, 0);
			}

			break;
		case 14:
		case 19:
		case 22:
		case 28:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 15:
			playNIS(kEventMertensAugustWaitingCompartment);
			_globals[kGlobalKnowAboutAugust] = 1;
			playDialog(0, "LIB015", -1, 0);
			bumpCathTylerComp();

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 16:
			playNIS(kEventMertensKronosInvitationCompartment);
			_globals[kGlobalKnowAboutKronos] = 1;
			playDialog(0, "LIB015", -1, 0);
			bumpCathTylerComp();

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 18:
		case 27:
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);
			break;
		case 20:
			playNIS(kEventMertensAugustWaitingCompartment);
			_globals[kGlobalKnowAboutAugust] = 1;
			playDialog(0, "LIB015", -1, 0);
			bumpCathTylerComp();

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 21:
			playNIS(kEventMertensKronosInvitationCompartment);
			_globals[kGlobalKnowAboutKronos] = 1;
			playDialog(0, "LIB015", -1, 0);
			bumpCathTylerComp();

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 23:
			_globals[kGlobalKnowAboutAugust] = 1;
			setDoor(1, kCharacterCath, checkDoor(1), 10, 9);

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 24:
			_globals[kGlobalKnowAboutKronos] = 1;
			setDoor(1, kCharacterCath, checkDoor(1), 10, 9);

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 25:
			setDoor(1, kCharacterCath, checkDoor(1), 10, 9);

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 29:
			playNIS(kEventMertensAugustWaitingCompartment);
			_globals[kGlobalKnowAboutAugust] = 1;
			playDialog(0, "LIB015", -1, 0);
			bumpCathTylerComp();

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 30:
			playNIS(kEventMertensKronosInvitationCompartment);
			_globals[kGlobalKnowAboutKronos] = 1;
			playDialog(0, "LIB015", -1, 0);
			bumpCathTylerComp();

			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_VisitCond2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_VisitCond2);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 28;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_VisitCond2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterCond1)[3] && getCharacterCurrentParams(kCharacterCond1)[4]) {
			send(kCharacterCond1, kCharacterCond2, 125499160, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterCond1)[3] = 1;
		break;
	case 12:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 4, 1500, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond1, "601O");
			send(kCharacterCond1, kCharacterCond2, 154005632, 0);
			break;
		case 3:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 155853632:
		getCharacterCurrentParams(kCharacterCond1)[4] = 1;
		break;
	case 202558662:
		startCycOtis(kCharacterCond1, "601L");
		playDialog(kCharacterCond1, (char *)&getCharacterCurrentParams(kCharacterCond1)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_VisitCond2Double(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_VisitCond2Double);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 29;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);
	strncpy((char *)&params->parameters[3], param2.stringParam, 12);

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_VisitCond2Double(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterCond1)[6] > 1 && getCharacterCurrentParams(kCharacterCond1)[7]) {
			send(kCharacterCond1, kCharacterCond2, 125499160, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterCond1)[6]++;
		if (getCharacterCurrentParams(kCharacterCond1)[6] == 1)
			playDialog(kCharacterCond1, (char *)&getCharacterCurrentParams(kCharacterCond1)[3], -1, 0);

		break;
	case 12:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 4, 1500, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond1, "601O");
			send(kCharacterCond1, kCharacterCond2, 154005632, 0);
			break;
		case 3:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 155853632:
		getCharacterCurrentParams(kCharacterCond1)[7] = 1;
		break;
	case 202558662:
		startCycOtis(kCharacterCond1, "601L");
		playDialog(kCharacterCond1, (char *)&getCharacterCurrentParams(kCharacterCond1)[0], -1, 0);
		break;
	}
}

void LogicManager::CONS_Cond1_CathBuzzing(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_CathBuzzing);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 30;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_CathBuzzing(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterCurrentParams(kCharacterCond1)[0] == 1) {
			getCharacterCurrentParams(kCharacterCond1)[1] = 8200;
		} else if (getCharacterCurrentParams(kCharacterCond1)[0] == 2) {
			getCharacterCurrentParams(kCharacterCond1)[1] = 7500;
		} else {
			if (getCharacterCurrentParams(kCharacterCond1)[0] != 3) {
				getCharacter(kCharacterCond1).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
				fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
				break;
			}

			getCharacterCurrentParams(kCharacterCond1)[1] = 6470;
		}

		if (getCharacterCurrentParams(kCharacterCond1)[0] == 1) {
			if (_globals[kGlobalCharacterSearchingForCath]) {
				getCharacter(kCharacterCond1).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
				fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
				break;
			}

			_globals[kGlobalCharacterSearchingForCath] = kCharacterCond1;
		}

		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, getCharacterCurrentParams(kCharacterCond1)[1], 0, 0);
			return;
		case 2:
			if (getCharacterCurrentParams(kCharacterCond1)[0] == 1) {
				if (_globals[kGlobalChapter] == 4)
					send(kCharacterCond1, kCharacterTatiana, 238790488, 0);
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_EnterTyler, 3, 0, 0, 0);
			} else if (getCharacterCurrentParams(kCharacterCond1)[0] == 2) {
				if (inComp(kCharacterCath, kCarGreenSleeping, 7500)) {
					setDoor(2, kCharacterCath, checkDoor(2), 0, 0);
					getCharacterCurrentParams(kCharacterCond1)[2] = 1;
				}
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
				Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Vb", 2, 0, 0);
			} else if (getCharacterCurrentParams(kCharacterCond1)[0] == 3) {
				if (inComp(kCharacterCath, kCarGreenSleeping, 6470)) {
					setDoor(3, kCharacterCath, checkDoor(3), 0, 0);
					getCharacterCurrentParams(kCharacterCond1)[2] = 1;
				}

				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
				Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601Mc", 3, 0, 0);
			} else {
				if (_globals[kGlobalCharacterSearchingForCath] == kCharacterCond1)
					_globals[kGlobalCharacterSearchingForCath] = 0;

				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
				Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			}

			break;
		case 3:
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterCond1)
				_globals[kGlobalCharacterSearchingForCath] = 0;

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			break;
		case 4:
			startCycOtis(kCharacterCond1, "601Wb");
			softBlockAtDoor(kCharacterCond1, 2);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "CON3020", 0, 0, 0);
			return;
		case 5:
			if (getCharacterCurrentParams(kCharacterCond1)[2]) {
				setDoor(2, kCharacterCath, checkDoor(2), 10, 9);
			}

			softReleaseAtDoor(kCharacterCond1, 2);
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterCond1)
				_globals[kGlobalCharacterSearchingForCath] = 0;

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			break;
		case 6:
			startCycOtis(kCharacterCond1, "601Nc");
			softBlockAtDoor(kCharacterCond1, 3);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
			Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "CON3020", 0, 0, 0);
			break;
		case 7:
			if (getCharacterCurrentParams(kCharacterCond1)[2]) {
				setDoor(3, kCharacterCath, checkDoor(3), 10, 9);
			}

			softReleaseAtDoor(kCharacterCond1, 3);
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterCond1)
				_globals[kGlobalCharacterSearchingForCath] = 0;

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_CathRattling(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_CathRattling);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 31;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_CathRattling(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
		Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_DoSeqOtis, "601G", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			if (whoRunningDialog(kCharacterCond1)) {
				startCycOtis(kCharacterCond1, "601J");
			} else {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
				Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			}
		} else if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 2 ||
				   getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 3) {
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_BathroomTrip(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_BathroomTrip);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 32;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_BathroomTrip(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 9510, 0, 0);
			break;
		case 2:
			if (getCharacter(kCharacterCond1).characterPosition.position < 9460) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
				Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			} else {
				endGraphics(kCharacterCond1);
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_DoWait, 900, 0, 0, 0);
			}

			break;
		case 3:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_DoPending(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_DoPending);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 33;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_DoPending(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterParams(kCharacterCond1, 8)[7]  ||
			getCharacterParams(kCharacterCond1, 8)[13] ||
			getCharacterParams(kCharacterCond1, 8)[14] ||
			getCharacterParams(kCharacterCond1, 8)[12] ||
			getCharacterParams(kCharacterCond1, 8)[11] ||
			getCharacterParams(kCharacterCond1, 8)[9]  ||
			getCharacterParams(kCharacterCond1, 8)[5]  ||
			getCharacterParams(kCharacterCond1, 8)[10] ||
			getCharacterParams(kCharacterCond1, 8)[8]  ||
			getCharacterParams(kCharacterCond1, 8)[17]) {

			getCharacterParams(kCharacterCond1, 8)[15] = 1;

			if (getCharacterParams(kCharacterCond1, 8)[7]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
				Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 1500, 0, 0);
			} else {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 540, 0, 0);
			}
		} else {
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacterParams(kCharacterCond1, 8)[16] = 1;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_Listen, 9, 0, 0, 0);
			break;
		case 2:
			getCharacterParams(kCharacterCond1, 8)[15] = 0;
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 3:
			endGraphics(kCharacterCond1);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_DoWait, 75, 0, 0, 0);
			break;
		case 4:
			if (getCharacterParams(kCharacterCond1, 8)[13]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
				Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 1, 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (getCharacterParams(kCharacterCond1, 8)[14]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
				Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (getCharacterParams(kCharacterCond1, 8)[12]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
				Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 1, 0, 0, 0);
				break;
			}

			// fall through
		case 7:
			if (getCharacterParams(kCharacterCond1, 8)[11]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
				Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 8:
			if (getCharacterParams(kCharacterCond1, 8)[9]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
				Cond1Call(&LogicManager::CONS_Cond1_DeliverAugustMessage, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 9:
			if (getCharacterParams(kCharacterCond1, 8)[5]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 10;
				Cond1Call(&LogicManager::CONS_Cond1_DeliverKronosMessage, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 10:
			if (getCharacterParams(kCharacterCond1, 8)[10]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 11;
				Cond1Call(&LogicManager::CONS_Cond1_VisitKahina, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 11:
			if (getCharacterParams(kCharacterCond1, 8)[8]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 12;
				Cond1Call(&LogicManager::CONS_Cond1_VisitCond2, "CON1200", 0, 0, 0);
				break;
			}

			// fall through
		case 12:
			if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 12) {
				send(kCharacterCond1, kCharacterCond2, 168254872, 0);
				getCharacterParams(kCharacterCond1, 8)[8] = 0;
			}

			if (getCharacterParams(kCharacterCond1, 8)[17]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 13;
				Cond1Call(&LogicManager::CONS_Cond1_VisitCond2Special, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 13:
			if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 13) {
				getCharacterParams(kCharacterCond1, 8)[17] = 0;
			}

			getCharacterParams(kCharacterCond1, 8)[15] = 0;
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_Birth);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 34;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterCond1)[0]) {
			getCharacterCurrentParams(kCharacterCond1)[0] = 1;
			CONS_Cond1_StartPart1(0, 0, 0, 0);
		}

		break;
	case 12:
		autoMessage(kCharacterCond1, 171394341, 7);
		autoMessage(kCharacterCond1, 169633856, 9);
		autoMessage(kCharacterCond1, 238732837, 10);
		autoMessage(kCharacterCond1, 269624833, 12);
		autoMessage(kCharacterCond1, 302614416, 11);
		autoMessage(kCharacterCond1, 190082817, 8);
		autoMessage(kCharacterCond1, 269436673, 13);
		autoMessage(kCharacterCond1, 303343617, 14);
		autoMessage(kCharacterCond1, 224122407, 17);
		autoMessage(kCharacterCond1, 201431954, 18);
		autoMessage(kCharacterCond1, 188635520, 19);
		autoMessage(kCharacterCond1, 204379649, 4);
		getCharacterParams(kCharacterCond1, 8)[0] = 0;
		getCharacter(kCharacterCond1).characterPosition.position = 9460;
		getCharacter(kCharacterCond1).characterPosition.location = 0;
		getCharacter(kCharacterCond1).characterPosition.car = kCarGreenSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_DeliverAugustMessage(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_DeliverAugustMessage);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 35;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_DeliverAugustMessage(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (_globals[kGlobalCharacterSearchingForCath] == kCharacterPolice) {
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		} else {
			_globals[kGlobalCharacterSearchingForCath] = kCharacterCond1;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_GetUp, 0, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 8200, 0, 0);
			break;
		case 2:
			if (!getCharacterParams(kCharacterCond1, 8)[9] ||
				_globals[kGlobalMetAugust]) {
				getCharacterParams(kCharacterCond1, 8)[9] = 0;

				if (_globals[kGlobalCharacterSearchingForCath] == kCharacterCond1)
					_globals[kGlobalCharacterSearchingForCath] = 0;

				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			} else {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
				Cond1Call(&LogicManager::CONS_Cond1_EnterTyler, 1, 0, 0, 0);
			}

			break;
		case 3:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		case 4:
		case 7:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 5:
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterCond1)
				_globals[kGlobalCharacterSearchingForCath] = 0;

			if (_globals[kGlobalKnowAboutAugust])
				getCharacterParams(kCharacterCond1, 8)[9] = 0;

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			break;
		case 6:
			getCharacterParams(kCharacterCond1, 8)[9] = 0;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_DeliverKronosMessage(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_DeliverKronosMessage);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 36;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_DeliverKronosMessage(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (_globals[kGlobalCharacterSearchingForCath] == kCharacterPolice) {
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		} else {
			_globals[kGlobalCharacterSearchingForCath] = kCharacterCond1;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_GetUp, 0, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 8200, 0, 0);
			return;
		case 2:
			if (!getCharacterParams(kCharacterCond1, 8)[5]) {
				getCharacterParams(kCharacterCond1, 8)[5] = 0;
				getCharacterParams(kCharacterCond1, 8)[6] = 0;

				if (_globals[kGlobalCharacterSearchingForCath] == kCharacterCond1)
					_globals[kGlobalCharacterSearchingForCath] = 0;

				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
				Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
				break;
			}

			if (cathInCorridor(kCarGreenSleeping) &&
				getCharacter(kCharacterCond1).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 9460, 0, 0);
			} else {
				if (getCharacterParams(kCharacterCond1, 8)[5]) {
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
					Cond1Call(&LogicManager::CONS_Cond1_EnterTyler, 2, 0, 0, 0);
				} else {
					getCharacterParams(kCharacterCond1, 8)[5] = 0;
					getCharacterParams(kCharacterCond1, 8)[6] = 0;

					if (_globals[kGlobalCharacterSearchingForCath] == kCharacterCond1)
						_globals[kGlobalCharacterSearchingForCath] = 0;

					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
					Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
				}
			}

			break;
		case 3:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 8200, 0, 0);
			break;
		case 4:
			if (getCharacterParams(kCharacterCond1, 8)[5]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
				Cond1Call(&LogicManager::CONS_Cond1_EnterTyler, 2, 0, 0, 0);
			} else {
				getCharacterParams(kCharacterCond1, 8)[5] = 0;
				getCharacterParams(kCharacterCond1, 8)[6] = 0;
				if (_globals[kGlobalCharacterSearchingForCath] == kCharacterCond1)
					_globals[kGlobalCharacterSearchingForCath] = 0;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
				Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			}

			break;
		case 5:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		case 6:
		case 9:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		case 7:
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterCond1)
				_globals[kGlobalCharacterSearchingForCath] = 0;

			if (!_globals[kGlobalKnowAboutKronos])
				getCharacterParams(kCharacterCond1, 8)[6] = 1;

			getCharacterParams(kCharacterCond1, 8)[5] = 0;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_VisitCond2Special(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_VisitCond2Special);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 37;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_VisitCond2Special(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterCond1)[0] >= 2 && getCharacterCurrentParams(kCharacterCond1)[1]) {
			send(kCharacterCond1, kCharacterCond2, 125499160, 0);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterCond1)[0]++;
		if (getCharacterCurrentParams(kCharacterCond1)[0] == 1) {
			if (nearChar(kCharacterCond1, kCharacterCath, 2000))
				playDialog(kCharacterCond1, "CON1152", -1, 0);
			else
				playDialog(kCharacterCond1, "CON1151", -1, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 4, 1500, 0, 0);
			break;
		case 2:
			startCycOtis(kCharacterCond1, "601O");
			send(kCharacterCond1, kCharacterCond2, 154005632, 0);
			break;
		case 3:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 155853632:
		getCharacterCurrentParams(kCharacterCond1)[1] = 1;
		break;
	case 202558662:
		startCycOtis(kCharacterCond1, "601L");
		playDialog(kCharacterCond1, "CON1150", -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_MakeBedTyler(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_MakeBedTyler);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 38;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_MakeBedTyler(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterParams(kCharacterCond1, 8)[3] && _globals[kGlobalCharacterSearchingForCath] != kCharacterPolice) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 8200, 0, 0);
		} else {
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			if (getCharacterParams(kCharacterCond1, 8)[3]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
				Cond1Call(&LogicManager::CONS_Cond1_EnterTyler, 0, 0, 0, 0);
			} else {
				getCharacter(kCharacterCond1).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
				fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			}

			break;
		}

		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 2) {
			getCharacterParams(kCharacterCond1, 8)[3] = 0;
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_MakeAllBeds(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_MakeAllBeds);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 39;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_MakeAllBeds(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacterParams(kCharacterCond1, 8)[3] = 1;
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBedMahmud, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoPending, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBedAugust, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_DoPending, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBedAlexei, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
			Cond1Call(&LogicManager::CONS_Cond1_DoPending, 0, 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBedTyler, 0, 0, 0, 0);
			break;
		case 8:
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterCond1)
				_globals[kGlobalCharacterSearchingForCath] = 0;

			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 10;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_VisitKahina(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_VisitKahina);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 40;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_VisitKahina(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacterParams(kCharacterCond1, 8)[10] = 0;
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 2, 9460, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoWait, 1800, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 1500, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		case 5:
			getCharacterParams(kCharacterCond1, 8)[5] = 1;
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_StartPart1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_StartPart1);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 41;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_StartPart1(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 2) {
			CONS_Cond1_Sitting(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_Sitting(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_Sitting);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 42;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_Sitting(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		if (getCharacterParams(kCharacterCond1, 8)[18]) {
			getCharacterParams(kCharacterCond1, 8)[7] = 0;
			getCharacterParams(kCharacterCond1, 8)[13] = 0;
			getCharacterParams(kCharacterCond1, 8)[14] = 0;
			getCharacterParams(kCharacterCond1, 8)[11] = 0;
			getCharacterParams(kCharacterCond1, 8)[12] = 0;
			getCharacterParams(kCharacterCond1, 8)[9] = 0;
			getCharacterParams(kCharacterCond1, 8)[5] = 0;
			getCharacterParams(kCharacterCond1, 8)[6] = 0;
			getCharacterParams(kCharacterCond1, 8)[10] = 0;
			getCharacterParams(kCharacterCond1, 8)[8] = 0;
			getCharacterParams(kCharacterCond1, 8)[17] = 0;
			getCharacterParams(kCharacterCond1, 8)[16] = 0;
			getCharacterParams(kCharacterCond1, 8)[0] = 1;
			getCharacterParams(kCharacterCond1, 8)[16] = 0;
			getCharacterCurrentParams(kCharacterCond1)[0] = 1;
			getCharacterCurrentParams(kCharacterCond1)[1] = 1;
			startCycOtis(kCharacterCond1, "601E");
			getCharacterParams(kCharacterCond1, 8)[18] = 0;
		}

		if (getCharacterParams(kCharacterCond1, 8)[16] ||
			_globals[kGlobalFoundCorpse] ||
			_doneNIS[kEventMertensAskTylerCompartment] ||
			_doneNIS[kEventMertensAskTylerCompartmentD]) {
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
		} else {
			getCharacter(kCharacterCond1).inventoryItem = 0x80;
		}

		if (!getCharacterCurrentParams(kCharacterCond1)[1]) {
			if (_gameTime > 1125000 && !getCharacterCurrentParams(kCharacterCond1)[2]) {
				getCharacterCurrentParams(kCharacterCond1)[2] = 1;
				send(kCharacterCond1, kCharacterMahmud, 170483072, 0);
			}

			if (getCharacterCurrentParams(kCharacterCond1)[3] != 0x7FFFFFFF && _gameTime > 1170900) {
				if (_gameTime > 1188000) {
					getCharacterCurrentParams(kCharacterCond1)[3] = 0x7FFFFFFF;
					getCharacter(kCharacterCond1).inventoryItem = kItemNone;
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
					Cond1Call(&LogicManager::CONS_Cond1_VisitCond2Double, "CON1210", "CON1210A", 0, 0);
					break;
				}

				if ((!cathInCorridor(kCarGreenSleeping) &&
						!cathInCorridor(kCarRedSleeping)) ||
					dialogRunning("REB1205") ||
					!inComp(kCharacterMadame, kCarRedSleeping, 5790) ||
					!getCharacterCurrentParams(kCharacterCond1)[3]) {

					getCharacterCurrentParams(kCharacterCond1)[3] = _gameTime;
					if (!_gameTime) {
						getCharacter(kCharacterCond1).inventoryItem = kItemNone;
						getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
						Cond1Call(&LogicManager::CONS_Cond1_VisitCond2Double, "CON1210", "CON1210A", 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterCond1)[3] < _gameTime) {
					getCharacterCurrentParams(kCharacterCond1)[3] = 0x7FFFFFFF;
					getCharacter(kCharacterCond1).inventoryItem = kItemNone;
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
					Cond1Call(&LogicManager::CONS_Cond1_VisitCond2Double, "CON1210", "CON1210A", 0, 0);
					break;
				}
			}
		}

		bool skip = false;
		if (_gameTime > 1215000 && !getCharacterParams(kCharacterCond1, 8)[0] && !getCharacterParams(kCharacterCond1, 8)[16]) {
			if (!getCharacterCurrentParams(kCharacterCond1)[4]) {
				getCharacterCurrentParams(kCharacterCond1)[4] = _gameTime + 2700;
				if (_gameTime == -2700) {
					skip = true;
					startCycOtis(kCharacterCond1, "601E");
					getCharacterParams(kCharacterCond1, 8)[0] = 1;
					getCharacterCurrentParams(kCharacterCond1)[4] = 0;
				}
			}

			if (!skip && getCharacterCurrentParams(kCharacterCond1)[4] < _gameTime) {
				getCharacterCurrentParams(kCharacterCond1)[4] = 0x7FFFFFFF;
				startCycOtis(kCharacterCond1, "601E");
				getCharacterParams(kCharacterCond1, 8)[0] = 1;
				getCharacterCurrentParams(kCharacterCond1)[4] = 0;
			}
		}

		if (getCharacterParams(kCharacterCond1, 8)[7]) {
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
			Cond1Call(&LogicManager::CONS_Cond1_Listen, 9, 0, 0, 0);
			break;
		}

		if (_globals[kGlobalCharacterSearchingForCath] != kCharacterPolice) {
			if (getCharacterParams(kCharacterCond1, 8)[13]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 10;
				Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 1, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterCond1, 8)[14]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 11;
				Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterCond1, 8)[12]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 12;
				Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 1, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterCond1, 8)[11]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 13;
				Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}
		}

		if (getCharacterParams(kCharacterCond1, 8)[9]) {
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 14;
			Cond1Call(&LogicManager::CONS_Cond1_DeliverAugustMessage, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[5]) {
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 15;
			Cond1Call(&LogicManager::CONS_Cond1_DeliverKronosMessage, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[10]) {
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 16;
			Cond1Call(&LogicManager::CONS_Cond1_VisitKahina, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[8]) {
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacterParams(kCharacterCond1, 8)[8] = 0;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 17;
			Cond1Call(&LogicManager::CONS_Cond1_VisitCond2, "CON1200", 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[17]) {
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacterParams(kCharacterCond1, 8)[17] = 0;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 18;
			Cond1Call(&LogicManager::CONS_Cond1_VisitCond2Special, 0, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterCond1)[0] || !getCharacterParams(kCharacterCond1, 8)[4]) {
			if (getCharacterParams(kCharacterCond1, 8)[0] &&
				!whoRunningDialog(kCharacterCond1) && _globals[kGlobalPhaseOfTheNight] != 4) {
				playDialog(kCharacterCond1, "CON1505", -1, 0);
			}

			break;
		}

		getCharacter(kCharacterCond1).inventoryItem = kItemNone;
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 19;
		Cond1Call(&LogicManager::CONS_Cond1_MakeAllBeds, 0, 0, 0, 0);

		break;
	}
	case 1:
		getCharacter(kCharacterCond1).inventoryItem = kItemNone;
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 21;
		Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensAskTylerCompartmentD, 0, 0);
		break;
	case 11:
		if (!getCharacterParams(kCharacterCond1, 8)[0] && !getCharacterParams(kCharacterCond1, 8)[16]) {
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 20;
			Cond1Call(&LogicManager::CONS_Cond1_Passing, msg->param, msg->sender, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterCond1).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterCond1).characterPosition.position = 1500;
		getCharacter(kCharacterCond1).characterPosition.location = 0;
		takeItem(kItem7);
		break;
	case 17:
		if (!getCharacterParams(kCharacterCond1, 8)[16]) {
			if (checkCathDir(kCarGreenSleeping, 23) && getCharacterParams(kCharacterCond1, 8)[6] && !_doneNIS[kEventKronosConversation]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
				Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensKronosInvitation, 0, 0);
			} else if (!checkCathDir(kCarGreenSleeping, 23) || _globals[kGlobalKnowAboutKronos] || _doneNIS[kEventMertensLastCar] || _doneNIS[kEventMertensLastCarOriginalJacket]) {
				if ((!checkCathDir(kCarGreenSleeping, 1) && !checkCathDir(kCarGreenSleeping, 23)) || getCharacterParams(kCharacterCond1, 8)[0] || getCharacterParams(kCharacterCond1, 8)[16]) {
					if (cathInCorridor(kCarGreenSleeping) && getCharacter(kCharacterCond1).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position && (_globals[1] == 1 || getCharacterParams(kCharacterCond1, 8)[6])) {
						getCharacter(kCharacterCond1).inventoryItem = kItemNone;
						getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
						Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
					}
				} else {
					getCharacter(kCharacterCond1).inventoryItem = kItemNone;
					if (checkCathDir(kCarGreenSleeping, 1)) {
						getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
						Cond1Call(&LogicManager::CONS_Cond1_Passing, 1, 0, 0, 0);
					} else {
						getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
						Cond1Call(&LogicManager::CONS_Cond1_Passing, 0, 0, 0, 0);
					}
				}
			} else {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_SaveGame, 2, kEventMertensLastCar, 0, 0);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			playNIS(kEventMertensKronosInvitation);
			_globals[kGlobalKnowAboutKronos] = 1;
			getCharacterParams(kCharacterCond1, 8)[5] = 0;
			getCharacterParams(kCharacterCond1, 8)[6] = 0;
			startSeqOtis(kCharacterCond1, "601A");
			takeItem(kItem7);
			getCharacterParams(kCharacterCond1, 8)[0] = 0;
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 2:
		case 4:
			startCycOtis(kCharacterCond1, "601B");
			if ((!checkCathDir(kCarGreenSleeping, 1) && !checkCathDir(kCarGreenSleeping, 23)) || getCharacterParams(kCharacterCond1, 8)[0] || getCharacterParams(kCharacterCond1, 8)[16]) {
				if (cathInCorridor(kCarGreenSleeping) && getCharacter(kCharacterCond1).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position && (_globals[1] == 1 || getCharacterParams(kCharacterCond1, 8)[6])) {
					getCharacter(kCharacterCond1).inventoryItem = kItemNone;
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
					Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
				}
			} else {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				if (checkCathDir(kCarGreenSleeping, 1)) {
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
					Cond1Call(&LogicManager::CONS_Cond1_Passing, 1, 0, 0, 0);
				} else {
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
					Cond1Call(&LogicManager::CONS_Cond1_Passing, 0, 0, 0, 0);
				}
			}

			break;
		case 3:
			playNIS((kEventMertensLastCarOriginalJacket - (_globals[kGlobalJacket] == 2)));
			startSeqOtis(kCharacterCond1, "601A");
			bumpCath(kCarGreenSleeping, 6, 255);
			takeItem(kItem7);
			getCharacterParams(kCharacterCond1, 8)[0] = 0;
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
		case 6:
			if (cathInCorridor(kCarGreenSleeping) && getCharacter(kCharacterCond1).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position && (_globals[1] == 1 || getCharacterParams(kCharacterCond1, 8)[6])) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
				Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
			}

			break;
		case 8:
		{
			bool skip = false;
			if (_gameTime > 1215000 && !getCharacterParams(kCharacterCond1, 8)[0] && !getCharacterParams(kCharacterCond1, 8)[16]) {
				if (!getCharacterCurrentParams(kCharacterCond1)[4]) {
					getCharacterCurrentParams(kCharacterCond1)[4] = _gameTime + 2700;
					if (_gameTime == -2700) {
						skip = true;
						startCycOtis(kCharacterCond1, "601E");
						getCharacterParams(kCharacterCond1, 8)[0] = 1;
						getCharacterCurrentParams(kCharacterCond1)[4] = 0;
					}
				}

				if (!skip && getCharacterCurrentParams(kCharacterCond1)[4] < _gameTime) {
					getCharacterCurrentParams(kCharacterCond1)[4] = 0x7FFFFFFF;
					startCycOtis(kCharacterCond1, "601E");
					getCharacterParams(kCharacterCond1, 8)[0] = 1;
					getCharacterCurrentParams(kCharacterCond1)[4] = 0;
				}
			}

			if (getCharacterParams(kCharacterCond1, 8)[7]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
				Cond1Call(&LogicManager::CONS_Cond1_Listen, 9, 0, 0, 0);
				break;
			}
		}

		// fall through
		case 9:
			if (_globals[kGlobalCharacterSearchingForCath] != kCharacterPolice) {
				if (getCharacterParams(kCharacterCond1, 8)[13]) {
					getCharacter(kCharacterCond1).inventoryItem = kItemNone;
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 10;
					Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 1, 0, 0, 0);
					break;
				}

				if (getCharacterParams(kCharacterCond1, 8)[14]) {
					getCharacter(kCharacterCond1).inventoryItem = kItemNone;
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 11;
					Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 0, 0, 0, 0);
					break;
				}

				if (getCharacterParams(kCharacterCond1, 8)[12]) {
					getCharacter(kCharacterCond1).inventoryItem = kItemNone;
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 12;
					Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 1, 0, 0, 0);
					break;
				}

				if (getCharacterParams(kCharacterCond1, 8)[11]) {
					getCharacter(kCharacterCond1).inventoryItem = kItemNone;
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 13;
					Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterParams(kCharacterCond1, 8)[9]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 14;
				Cond1Call(&LogicManager::CONS_Cond1_DeliverAugustMessage, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterCond1, 8)[5]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 15;
				Cond1Call(&LogicManager::CONS_Cond1_DeliverKronosMessage, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterCond1, 8)[10]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 16;
				Cond1Call(&LogicManager::CONS_Cond1_VisitKahina, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterCond1, 8)[8]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacterParams(kCharacterCond1, 8)[8] = 0;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 17;
				Cond1Call(&LogicManager::CONS_Cond1_VisitCond2, "CON1200", 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterCond1, 8)[17]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacterParams(kCharacterCond1, 8)[17] = 0;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 18;
				Cond1Call(&LogicManager::CONS_Cond1_VisitCond2Special, 0, 0, 0, 0);
				break;
			}

			if (getCharacterCurrentParams(kCharacterCond1)[0] || !getCharacterParams(kCharacterCond1, 8)[4]) {
				if (getCharacterParams(kCharacterCond1, 8)[0] &&
					!whoRunningDialog(kCharacterCond1) && _globals[kGlobalPhaseOfTheNight] != 4) {
					playDialog(kCharacterCond1, "CON1505", -1, 0);
				}

				break;
			}

			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 19;
			Cond1Call(&LogicManager::CONS_Cond1_MakeAllBeds, 0, 0, 0, 0);

			break;
		case 10:
			if (getCharacterParams(kCharacterCond1, 8)[14]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 11;
				Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 11:
			if (getCharacterParams(kCharacterCond1, 8)[12]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 12;
				Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 1, 0, 0, 0);
				break;
			}

			// fall through
		case 12:
			if (getCharacterParams(kCharacterCond1, 8)[11]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 13;
				Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 13:
			if (getCharacterParams(kCharacterCond1, 8)[9]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 14;
				Cond1Call(&LogicManager::CONS_Cond1_DeliverAugustMessage, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 14:
			if (getCharacterParams(kCharacterCond1, 8)[5]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 15;
				Cond1Call(&LogicManager::CONS_Cond1_DeliverKronosMessage, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 15:
			if (getCharacterParams(kCharacterCond1, 8)[10]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 16;
				Cond1Call(&LogicManager::CONS_Cond1_VisitKahina, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 16:
			if (getCharacterParams(kCharacterCond1, 8)[8]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacterParams(kCharacterCond1, 8)[8] = 0;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 17;
				Cond1Call(&LogicManager::CONS_Cond1_VisitCond2, "CON1200", 0, 0, 0);
				break;
			}

			// fall through
		case 17:
			if (getCharacterParams(kCharacterCond1, 8)[17]) {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacterParams(kCharacterCond1, 8)[17] = 0;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 18;
				Cond1Call(&LogicManager::CONS_Cond1_VisitCond2Special, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 18:
			if (getCharacterCurrentParams(kCharacterCond1)[0] || !getCharacterParams(kCharacterCond1, 8)[4]) {
				if (getCharacterParams(kCharacterCond1, 8)[0] &&
					!whoRunningDialog(kCharacterCond1) && _globals[kGlobalPhaseOfTheNight] != 4) {
					playDialog(kCharacterCond1, "CON1505", -1, 0);
				}
			} else {
				getCharacter(kCharacterCond1).inventoryItem = kItemNone;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 19;
				Cond1Call(&LogicManager::CONS_Cond1_MakeAllBeds, 0, 0, 0, 0);
			}

			break;
		case 19:
			getCharacterCurrentParams(kCharacterCond1)[0] = 1;
			if (getCharacterParams(kCharacterCond1, 8)[0] &&
				!whoRunningDialog(kCharacterCond1) && _globals[kGlobalPhaseOfTheNight] != 4) {
				playDialog(kCharacterCond1, "CON1505", -1, 0);
			}

			break;
		case 21:
			playNIS(kEventMertensAskTylerCompartmentD);
			startSeqOtis(kCharacterCond1, "601A");
			_items[kItem7].floating = 0;
			bumpCath(kCarGreenSleeping, 25, 255);
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 22;
			Cond1Call(&LogicManager::CONS_Cond1_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 22:
			startCycOtis(kCharacterCond1, "601B");
			break;
		default:
			break;
		}

		break;
	case 225932896:
		if (!getCharacterParams(kCharacterCond1, 8)[16] && !getCharacterParams(kCharacterCond1, 8)[0]) {
			send(kCharacterCond1, kCharacterFrancois, 205346192, 0);
		}

		break;
	case 225358684:
		if (!getCharacterParams(kCharacterCond1, 8)[0]) {
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 23;
			Cond1Call(&LogicManager::CONS_Cond1_CathBuzzing, msg->param, 0, 0, 0);
		}

		break;
	case 305159806:
		if (!getCharacterParams(kCharacterCond1, 8)[16] && !getCharacterParams(kCharacterCond1, 8)[0]) {
			getCharacter(kCharacterCond1).inventoryItem = kItemNone;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 24;
			Cond1Call(&LogicManager::CONS_Cond1_CathRattling, msg->param, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_StartPart2);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 43;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterCond1);
		getCharacter(kCharacterCond1).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterCond1).characterPosition.position = 1500;
		getCharacter(kCharacterCond1).characterPosition.location = 0;
		getCharacter(kCharacterCond1).inventoryItem = kItemNone;
		getCharacterParams(kCharacterCond1, 8)[13] = 0;
		getCharacterParams(kCharacterCond1, 8)[14] = 0;
		getCharacterParams(kCharacterCond1, 8)[12] = 0;
		getCharacterParams(kCharacterCond1, 8)[11] = 0;
		getCharacterParams(kCharacterCond1, 8)[5] = 0;
		getCharacterParams(kCharacterCond1, 8)[9] = 0;
		getCharacterParams(kCharacterCond1, 8)[7] = 0;
		getCharacterParams(kCharacterCond1, 8)[8] = 0;
		getCharacterParams(kCharacterCond1, 8)[10] = 0;
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			CONS_Cond1_SittingDay(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_SittingDay(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_SittingDay);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 44;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_SittingDay(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterParams(kCharacterCond1, 8)[13]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 1, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[14]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[12]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 1, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[11]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 0, 0, 0, 0);
		}

		break;
	case 11:
		if (!getCharacterParams(kCharacterCond1, 8)[16]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_Passing, msg->param, msg->sender, 0, 0);
		}

		break;
	case 17:
		if (!getCharacterParams(kCharacterCond1, 8)[16]) {
			if (checkCathDir(kCarGreenSleeping, 1)) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
				Cond1Call(&LogicManager::CONS_Cond1_Passing, 1, 0, 0, 0);
			} else if (checkCathDir(kCarGreenSleeping, 23)) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
				Cond1Call(&LogicManager::CONS_Cond1_Passing, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			if (getCharacterParams(kCharacterCond1, 8)[14]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
				Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacterParams(kCharacterCond1, 8)[12]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 1, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (getCharacterParams(kCharacterCond1, 8)[11]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
				Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 0, 0, 0, 0);
			}

			break;
		default:
			break;
		}

		break;
	case 225358684:
		if (!getCharacterParams(kCharacterCond1, 8)[0]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
			Cond1Call(&LogicManager::CONS_Cond1_CathBuzzing, msg->param, 0, 0, 0);
		}

		break;
	case 225932896:
		if (!getCharacterParams(kCharacterCond1, 8)[16] && !getCharacterParams(kCharacterCond1, 8)[0]) {
			send(kCharacterCond1, kCharacterFrancois, 205346192, 0);
		}

		break;
	case 226078300:
		if (!getCharacterParams(kCharacterCond1, 8)[16] && !getCharacterParams(kCharacterCond1, 8)[0]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
			Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "CON2020", 0, 0, 0);
		}

		break;
	case 305159806:
		if (!getCharacterParams(kCharacterCond1, 8)[16] && !getCharacterParams(kCharacterCond1, 8)[0]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 10;
			Cond1Call(&LogicManager::CONS_Cond1_CathRattling, msg->param, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_StartPart3);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 45;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterCond1).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterCond1).characterPosition.position = 1500;
		getCharacter(kCharacterCond1).characterPosition.location = 0;
		getCharacter(kCharacterCond1).inventoryItem = kItemNone;
		getCharacterParams(kCharacterCond1, 8)[13] = 0;
		getCharacterParams(kCharacterCond1, 8)[14] = 0;
		getCharacterParams(kCharacterCond1, 8)[12] = 0;
		getCharacterParams(kCharacterCond1, 8)[11] = 0;
		getCharacterParams(kCharacterCond1, 8)[5] = 0;
		getCharacterParams(kCharacterCond1, 8)[9] = 0;
		getCharacterParams(kCharacterCond1, 8)[7] = 0;
		getCharacterParams(kCharacterCond1, 8)[8] = 0;
		getCharacterParams(kCharacterCond1, 8)[10] = 0;
		getCharacterParams(kCharacterCond1, 8)[19] = 0;
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			CONS_Cond1_OnDuty(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_OnDuty(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_OnDuty);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 46;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_OnDuty(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterParams(kCharacterCond1, 8)[13]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 1, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[14]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[12]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 1, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[11]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[7]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_Listen, 9, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[19] && (_doneNIS[kEventKronosVisit] || _gameTime > 2052000) && _gameTime < 2133000 && cathInCorridor(kCarGreenSleeping)) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
			Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 1971000 && !getCharacterCurrentParams(kCharacterCond1)[0]) {
			getCharacterCurrentParams(kCharacterCond1)[0] = 1;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
			Cond1Call(&LogicManager::CONS_Cond1_VisitCond2, "CON3012", 0, 0, 0);
			break;
		}

		if (_gameTime > 2117700 && !getCharacterCurrentParams(kCharacterCond1)[1]) {
			getCharacterCurrentParams(kCharacterCond1)[1] = 1;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
			Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2124000 && !getCharacterCurrentParams(kCharacterCond1)[2]) {
			getCharacterCurrentParams(kCharacterCond1)[2] = 1;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
			Cond1Call(&LogicManager::CONS_Cond1_VisitCond2, "CON2010", 0, 0, 0);
			break;
		}

		if (_gameTime > 2146500 && !getCharacterCurrentParams(kCharacterCond1)[3]) {
			getCharacterCurrentParams(kCharacterCond1)[3] = 1;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 10;
			Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2169000 && !getCharacterCurrentParams(kCharacterCond1)[4]) {
			getCharacterCurrentParams(kCharacterCond1)[4] = 1;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 11;
			Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
			break;
		}

		break;
	case 11:
		if (!getCharacterParams(kCharacterCond1, 8)[16]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 12;
			Cond1Call(&LogicManager::CONS_Cond1_Passing, msg->param, msg->sender, 0, 0);
		}

		break;
	case 17:
		if (!getCharacterParams(kCharacterCond1, 8)[16]) {
			if (checkCathDir(kCarGreenSleeping, 1)) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 13;
				Cond1Call(&LogicManager::CONS_Cond1_Passing, 1, 0, 0, 0);
			} else if (checkCathDir(kCarGreenSleeping, 23)) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 14;
				Cond1Call(&LogicManager::CONS_Cond1_Passing, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			if (getCharacterParams(kCharacterCond1, 8)[14]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
				Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacterParams(kCharacterCond1, 8)[12]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 1, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (getCharacterParams(kCharacterCond1, 8)[11]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
				Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (getCharacterParams(kCharacterCond1, 8)[7]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
				Cond1Call(&LogicManager::CONS_Cond1_Listen, 9, 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (getCharacterParams(kCharacterCond1, 8)[19] && (_doneNIS[kEventKronosVisit] || _gameTime > 2052000) && _gameTime < 2133000 && cathInCorridor(kCarGreenSleeping)) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
				Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (_gameTime > 1971000 && !getCharacterCurrentParams(kCharacterCond1)[0]) {
				getCharacterCurrentParams(kCharacterCond1)[0] = 1;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
				Cond1Call(&LogicManager::CONS_Cond1_VisitCond2, "CON3012", 0, 0, 0);
				break;
			}

			// fall through
		case 7:
			if (_gameTime > 2117700 && !getCharacterCurrentParams(kCharacterCond1)[1]) {
				getCharacterCurrentParams(kCharacterCond1)[1] = 1;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
				Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 8:
			if (_gameTime > 2124000 && !getCharacterCurrentParams(kCharacterCond1)[2]) {
				getCharacterCurrentParams(kCharacterCond1)[2] = 1;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
				Cond1Call(&LogicManager::CONS_Cond1_VisitCond2, "CON2010", 0, 0, 0);
				break;
			}

			// fall through
		case 9:
			if (_gameTime > 2146500 && !getCharacterCurrentParams(kCharacterCond1)[3]) {
				getCharacterCurrentParams(kCharacterCond1)[3] = 1;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 10;
				Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 10:
			if (_gameTime > 2169000 && !getCharacterCurrentParams(kCharacterCond1)[4]) {
				getCharacterCurrentParams(kCharacterCond1)[4] = 1;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 11;
				Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
			}

			break;
		default:
			break;
		}

		break;
	case 225358684:
		if (!getCharacterParams(kCharacterCond1, 8)[0]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 16;
			Cond1Call(&LogicManager::CONS_Cond1_CathBuzzing, msg->param, 0, 0, 0);
		}

		break;
	case 225932896:
		if (!getCharacterParams(kCharacterCond1, 8)[16] && !getCharacterParams(kCharacterCond1, 8)[0]) {
			send(kCharacterCond1, kCharacterFrancois, 205346192, 0);
		}

		break;
	case 226078300:
		if (!getCharacterParams(kCharacterCond1, 8)[16] && !getCharacterParams(kCharacterCond1, 8)[0]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 15;
			Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "CON2020", 0, 0, 0);
		}

		break;
	case 305159806:
		if (!getCharacterParams(kCharacterCond1, 8)[16] && !getCharacterParams(kCharacterCond1, 8)[0]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 17;
			Cond1Call(&LogicManager::CONS_Cond1_CathRattling, msg->param, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_StartPart4);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 47;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterCond1);
		getCharacter(kCharacterCond1).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterCond1).characterPosition.position = 1500;
		getCharacter(kCharacterCond1).characterPosition.location = 0;
		getCharacter(kCharacterCond1).inventoryItem = kItemNone;
		getCharacterParams(kCharacterCond1, 8)[18] = 0;
		getCharacterParams(kCharacterCond1, 8)[13] = 0;
		getCharacterParams(kCharacterCond1, 8)[14] = 0;
		getCharacterParams(kCharacterCond1, 8)[12] = 0;
		getCharacterParams(kCharacterCond1, 8)[11] = 0;
		getCharacterParams(kCharacterCond1, 8)[5] = 0;
		getCharacterParams(kCharacterCond1, 8)[9] = 0;
		getCharacterParams(kCharacterCond1, 8)[7] = 0;
		getCharacterParams(kCharacterCond1, 8)[8] = 0;
		getCharacterParams(kCharacterCond1, 8)[10] = 0;
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			CONS_Cond1_OnDuty4(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_OnDuty4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_OnDuty4);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 48;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_OnDuty4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterParams(kCharacterCond1, 8)[18]) {
			getCharacterCurrentParams(kCharacterCond1)[0] = 1;
			setModel(2, 1);
			setModel(3, 1);
			setModel(4, 1);
			getCharacterParams(kCharacterCond1, 8)[13] = 0;
			getCharacterParams(kCharacterCond1, 8)[14] = 0;
			getCharacterParams(kCharacterCond1, 8)[12] = 0;
			getCharacterParams(kCharacterCond1, 8)[11] = 0;
			startCycOtis(kCharacterCond1, "601E");
			getCharacterParams(kCharacterCond1, 8)[18] = 0;
		}

		if (getCharacterParams(kCharacterCond1, 8)[13]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
			Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 1, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[14]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[12]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 1, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterCond1, 8)[11]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 0, 0, 0, 0);
			break;
		}

		if (!getCharacterCurrentParams(kCharacterCond1)[0]) {
			if (_gameTime > 2403000 && !getCharacterCurrentParams(kCharacterCond1)[1]) {
				getCharacterCurrentParams(kCharacterCond1)[1] = 1;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
				Cond1Call(&LogicManager::CONS_Cond1_MakeBeds4, 0, 0, 0, 0);
				break;
			}

			if (_gameTime > 2430000 && !getCharacterCurrentParams(kCharacterCond1)[2]) {
				getCharacterCurrentParams(kCharacterCond1)[2] = 1;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
				Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			if (_gameTime > 2439000 && !getCharacterCurrentParams(kCharacterCond1)[3]) {
				getCharacterCurrentParams(kCharacterCond1)[3] = 1;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
				Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			if (_gameTime > 2448000 && !getCharacterCurrentParams(kCharacterCond1)[4]) {
				getCharacterCurrentParams(kCharacterCond1)[4] = 1;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
				Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
				break;
			}
		}

		if (_gameTime > 2538000 && !getCharacterParams(kCharacterCond1, 8)[0] && !getCharacterParams(kCharacterCond1, 8)[16]) {
			if (getCharacterCurrentParams(kCharacterCond1)[5] || (getCharacterCurrentParams(kCharacterCond1)[5] = _gameTime + 2700, _gameTime != -2700)) {
				if (getCharacterCurrentParams(kCharacterCond1)[5] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterCond1)[5] = 0x7FFFFFFF;
			}

			startCycOtis(kCharacterCond1, "601E");
			getCharacterParams(kCharacterCond1, 8)[0] = 1;
			getCharacterCurrentParams(kCharacterCond1)[5] = 0;
		}

		break;
	case 11:
		if (!getCharacterParams(kCharacterCond1, 8)[16] && !getCharacterParams(kCharacterCond1, 8)[0]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
			Cond1Call(&LogicManager::CONS_Cond1_Passing, msg->param, msg->sender, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterCond1).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterCond1).characterPosition.position = 1500;
		getCharacter(kCharacterCond1).characterPosition.location = 0;
		takeItem(kItem7);
		break;
	case 17:
		if (!getCharacterParams(kCharacterCond1, 8)[16] && !getCharacterParams(kCharacterCond1, 8)[0]) {
			if (checkCathDir(kCarGreenSleeping, 1)) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 10;
				Cond1Call(&LogicManager::CONS_Cond1_Passing, 1, 0, 0, 0);
			} else if (checkCathDir(kCarGreenSleeping, 23)) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 11;
				Cond1Call(&LogicManager::CONS_Cond1_Passing, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			if (getCharacterParams(kCharacterCond1, 8)[14]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
				Cond1Call(&LogicManager::CONS_Cond1_AugustLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacterParams(kCharacterCond1, 8)[12]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
				Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 1, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (getCharacterParams(kCharacterCond1, 8)[11]) {
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
				Cond1Call(&LogicManager::CONS_Cond1_AlexeiLockUnlockMyComp, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (!getCharacterCurrentParams(kCharacterCond1)[0]) {
				if (_gameTime > 2403000 && !getCharacterCurrentParams(kCharacterCond1)[1]) {
					getCharacterCurrentParams(kCharacterCond1)[1] = 1;
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
					Cond1Call(&LogicManager::CONS_Cond1_MakeBeds4, 0, 0, 0, 0);
					break;
				}

				if (_gameTime > 2430000 && !getCharacterCurrentParams(kCharacterCond1)[2]) {
					getCharacterCurrentParams(kCharacterCond1)[2] = 1;
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
					Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
					break;
				}

				if (_gameTime > 2439000 && !getCharacterCurrentParams(kCharacterCond1)[3]) {
					getCharacterCurrentParams(kCharacterCond1)[3] = 1;
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
					Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
					break;
				}

				if (_gameTime > 2448000 && !getCharacterCurrentParams(kCharacterCond1)[4]) {
					getCharacterCurrentParams(kCharacterCond1)[4] = 1;
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
					Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
					break;
				}
			}

			if (_gameTime > 2538000 && !getCharacterParams(kCharacterCond1, 8)[0] && !getCharacterParams(kCharacterCond1, 8)[16]) {
				if (getCharacterCurrentParams(kCharacterCond1)[5] || (getCharacterCurrentParams(kCharacterCond1)[5] = _gameTime + 2700, _gameTime != -2700)) {
					if (getCharacterCurrentParams(kCharacterCond1)[5] >= _gameTime)
						break;

					getCharacterCurrentParams(kCharacterCond1)[5] = 0x7FFFFFFF;
				}

				startCycOtis(kCharacterCond1, "601E");
				getCharacterParams(kCharacterCond1, 8)[0] = 1;
				getCharacterCurrentParams(kCharacterCond1)[5] = 0;
			}

			break;
		case 5:
			if (_gameTime > 2430000 && !getCharacterCurrentParams(kCharacterCond1)[2]) {
				getCharacterCurrentParams(kCharacterCond1)[2] = 1;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
				Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (_gameTime > 2439000 && !getCharacterCurrentParams(kCharacterCond1)[3]) {
				getCharacterCurrentParams(kCharacterCond1)[3] = 1;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
				Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 7:
			if (_gameTime > 2448000 && !getCharacterCurrentParams(kCharacterCond1)[4]) {
				getCharacterCurrentParams(kCharacterCond1)[4] = 1;
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
				Cond1Call(&LogicManager::CONS_Cond1_BathroomTrip, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 8:
			if (_gameTime > 2538000 && !getCharacterParams(kCharacterCond1, 8)[0] && !getCharacterParams(kCharacterCond1, 8)[16]) {
				if (getCharacterCurrentParams(kCharacterCond1)[5] || (getCharacterCurrentParams(kCharacterCond1)[5] = _gameTime + 2700, _gameTime != -2700)) {
					if (getCharacterCurrentParams(kCharacterCond1)[5] >= _gameTime)
						break;

					getCharacterCurrentParams(kCharacterCond1)[5] = 0x7FFFFFFF;
				}

				startCycOtis(kCharacterCond1, "601E");
				getCharacterParams(kCharacterCond1, 8)[0] = 1;
				getCharacterCurrentParams(kCharacterCond1)[5] = 0;
			}

			break;
		default:
			break;
		}

		break;
	case 225358684:
		if (!getCharacterParams(kCharacterCond1, 8)[0]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 13;
			Cond1Call(&LogicManager::CONS_Cond1_CathBuzzing, msg->param, 0, 0, 0);
		}

		break;
	case 226078300:
		if (!getCharacterParams(kCharacterCond1, 8)[16] && !getCharacterParams(kCharacterCond1, 8)[0]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 12;
			Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "CON2020", 0, 0, 0);
		}

		break;
	case 305159806:
		if (!getCharacterParams(kCharacterCond1, 8)[16] && !getCharacterParams(kCharacterCond1, 8)[0]) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 14;
			Cond1Call(&LogicManager::CONS_Cond1_CathRattling, msg->param, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_MakeBeds4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_MakeBeds4);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 49;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_MakeBeds4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_GetUp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 8200, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_EnterTyler, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_DoPending, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBedAlexei, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
			Cond1Call(&LogicManager::CONS_Cond1_DoPending, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBedAugust, 0, 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 8;
			Cond1Call(&LogicManager::CONS_Cond1_DoPending, 0, 0, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 9;
			Cond1Call(&LogicManager::CONS_Cond1_MakeBedMahmud2, 0, 0, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 10;
			Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 2000, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 11;
			Cond1Call(&LogicManager::CONS_Cond1_SitDown, 0, 0, 0, 0);
			break;
		case 11:
			getCharacter(kCharacterCond1).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterCond1, _functionsCond1[getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall]]);
			fedEx(kCharacterCond1, kCharacterCond1, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_StartPart5);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 50;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Cond1_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterCond1);
		getCharacter(kCharacterCond1).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterCond1).characterPosition.position = 3969;
		getCharacter(kCharacterCond1).characterPosition.location = 1;
		getCharacter(kCharacterCond1).inventoryItem = kItemNone;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_Prisoner);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 51;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Cond1_ComfortMadame(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_ComfortMadame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_ComfortMadame);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 52;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_ComfortMadame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterCond1)[1] == 0x7FFFFFFF || !_gameTime)
			break;

		if (getCharacterCurrentParams(kCharacterCond1)[0] >= _gameTime) {
			if (!cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterCond1)[1]) {
				getCharacterCurrentParams(kCharacterCond1)[1] = _gameTime;
				if (!_gameTime) {
					getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
					Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "Mme5010", 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterCond1)[1] >= _gameTime)
				break;
		}

		getCharacterCurrentParams(kCharacterCond1)[1] = 0x7FFFFFFF;
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "Mme5010", 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterCond1).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterCond1).characterPosition.position = 5790;
		getCharacter(kCharacterCond1).characterPosition.location = 1;
		setDoor(35, kCharacterCath, 3, 10, 9);
		getCharacterCurrentParams(kCharacterCond1)[0] = _gameTime + 4500;
		break;
	case 18:
		if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 1) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "671Ad", 35, 0, 0);
		} else if (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] == 2) {
			getCharacter(kCharacterCond1).characterPosition.location = 0;
			send(kCharacterCond1, kCharacterMadame, 155604840, 0);
			CONS_Cond1_HideOut(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_HideOut(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_HideOut);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 53;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_HideOut(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterCond1)[0]) {
			if (getCharacterCurrentParams(kCharacterCond1)[3] || (getCharacterCurrentParams(kCharacterCond1)[3] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterCond1)[3] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterCond1)[3] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterCond1)[0] = 0;
			getCharacterCurrentParams(kCharacterCond1)[1] = 1;
			setDoor(4, kCharacterCond1, 1, 0, 0);
			getCharacterCurrentParams(kCharacterCond1)[3] = 0;
		} else {
			getCharacterCurrentParams(kCharacterCond1)[3] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterCond1)[0]) {
			setDoor(4, kCharacterCond1, 1, 0, 0);
			getCharacterCurrentParams(kCharacterCond1)[0] = 0;
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 3;
			Cond1Call(&LogicManager::CONS_Cond1_DoDialog, getCathJustChecking(), 0, 0, 0);
		} else if (msg->action == 8) {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 4;
			Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "LIB012", 0, 0, 0);
		} else {
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 5;
			Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "LIB013", 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 1;
		Cond1Call(&LogicManager::CONS_Cond1_DoWalk, 3, 5790, 0, 0);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterCond1)[1] || getCharacterCurrentParams(kCharacterCond1)[0]) {
			getCharacterCurrentParams(kCharacterCond1)[1] = 0;
			getCharacterCurrentParams(kCharacterCond1)[0] = 0;
			setDoor(4, kCharacterCond1, 1, 10, 9);
			getCharacterCurrentParams(kCharacterCond1)[2] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 2;
			Cond1Call(&LogicManager::CONS_Cond1_DoCorrOtis, "601ZD", 4, 0, 0);
			break;
		case 2:
			endGraphics(kCharacterCond1);
			getCharacter(kCharacterCond1).characterPosition.location = 1;
			getCharacter(kCharacterCond1).characterPosition.position = 5790;
			setDoor(4, kCharacterCond1, 1, 10, 9);
			break;
		case 3:
			setDoor(4, kCharacterCond1, 1, 10, 9);
			break;
		case 4:
		case 5:
			getCharacterCurrentParams(kCharacterCond1)[2]++;
			if (getCharacterCurrentParams(kCharacterCond1)[2] == 1) {
				setDoor(4, kCharacterCond1, 1, 0, 0);
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 6;
				Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "Con5002", 0, 0, 0);
			} else if (getCharacterCurrentParams(kCharacterCond1)[2] == 2) {
				setDoor(4, kCharacterCond1, 1, 0, 0);
				getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall + 8] = 7;
				Cond1Call(&LogicManager::CONS_Cond1_DoDialog, "Con5002A", 0, 0, 0);
			}

			break;
		case 6:
			getCharacterCurrentParams(kCharacterCond1)[0] = 1;
			setDoor(4, kCharacterCond1, 1, 14, 0);
			break;
		case 7:
			getCharacterCurrentParams(kCharacterCond1)[1] = 1;
			break;
		default:
			break;
		}

		break;
	case 135800432:
		CONS_Cond1_Disappear(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Cond1_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterCond1).callParams[getCharacter(kCharacterCond1).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterCond1, &LogicManager::HAND_Cond1_Disappear);
	getCharacter(kCharacterCond1).callbacks[getCharacter(kCharacterCond1).currentCall] = 54;

	params->clear();

	fedEx(kCharacterCond1, kCharacterCond1, 12, 0);
}

void LogicManager::HAND_Cond1_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsCond1[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Cond1_DebugWalks,
	&LogicManager::HAND_Cond1_DoSeqOtis,
	&LogicManager::HAND_Cond1_DoCorrOtis,
	&LogicManager::HAND_Cond1_DoBriefCorrOtis,
	&LogicManager::HAND_Cond1_DoEnterCorrOtis,
	&LogicManager::HAND_Cond1_FinishSeqOtis,
	&LogicManager::HAND_Cond1_DoDialog,
	&LogicManager::HAND_Cond1_DoDialogFullVol,
	&LogicManager::HAND_Cond1_SaveGame,
	&LogicManager::HAND_Cond1_DoWalk,
	&LogicManager::HAND_Cond1_DoWait,
	&LogicManager::HAND_Cond1_StandAsideDialog,
	&LogicManager::HAND_Cond1_Passing,
	&LogicManager::HAND_Cond1_Listen,
	&LogicManager::HAND_Cond1_AlexeiLockUnlockMyComp,
	&LogicManager::HAND_Cond1_AugustLockUnlockMyComp,
	&LogicManager::HAND_Cond1_SitDown,
	&LogicManager::HAND_Cond1_SitDownFast,
	&LogicManager::HAND_Cond1_GetUp,
	&LogicManager::HAND_Cond1_GetUpListen,
	&LogicManager::HAND_Cond1_MakeBed,
	&LogicManager::HAND_Cond1_MakeBedMahmud,
	&LogicManager::HAND_Cond1_MakeBedMahmud2,
	&LogicManager::HAND_Cond1_MakeBedAugust,
	&LogicManager::HAND_Cond1_MakeBedAlexei,
	&LogicManager::HAND_Cond1_SubMakeBed,
	&LogicManager::HAND_Cond1_EnterTyler,
	&LogicManager::HAND_Cond1_VisitCond2,
	&LogicManager::HAND_Cond1_VisitCond2Double,
	&LogicManager::HAND_Cond1_CathBuzzing,
	&LogicManager::HAND_Cond1_CathRattling,
	&LogicManager::HAND_Cond1_BathroomTrip,
	&LogicManager::HAND_Cond1_DoPending,
	&LogicManager::HAND_Cond1_Birth,
	&LogicManager::HAND_Cond1_DeliverAugustMessage,
	&LogicManager::HAND_Cond1_DeliverKronosMessage,
	&LogicManager::HAND_Cond1_VisitCond2Special,
	&LogicManager::HAND_Cond1_MakeBedTyler,
	&LogicManager::HAND_Cond1_MakeAllBeds,
	&LogicManager::HAND_Cond1_VisitKahina,
	&LogicManager::HAND_Cond1_StartPart1,
	&LogicManager::HAND_Cond1_Sitting,
	&LogicManager::HAND_Cond1_StartPart2,
	&LogicManager::HAND_Cond1_SittingDay,
	&LogicManager::HAND_Cond1_StartPart3,
	&LogicManager::HAND_Cond1_OnDuty,
	&LogicManager::HAND_Cond1_StartPart4,
	&LogicManager::HAND_Cond1_OnDuty4,
	&LogicManager::HAND_Cond1_MakeBeds4,
	&LogicManager::HAND_Cond1_StartPart5,
	&LogicManager::HAND_Cond1_Prisoner,
	&LogicManager::HAND_Cond1_ComfortMadame,
	&LogicManager::HAND_Cond1_HideOut,
	&LogicManager::HAND_Cond1_Disappear
};

} // End of namespace LastExpress
