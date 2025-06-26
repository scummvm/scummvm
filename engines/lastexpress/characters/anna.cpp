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

void LogicManager::CONS_Anna(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterAnna,
			_functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]
		);

		break;
	case 1:
		CONS_Anna_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Anna_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Anna_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Anna_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Anna_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::AnnaCall(CALL_PARAMS) {
	getCharacter(kCharacterAnna).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Anna_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DebugWalks);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 1;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterAnna, kCarGreenSleeping, getCharacterCurrentParams(kCharacterAnna)[0])) {
			if (getCharacterCurrentParams(kCharacterAnna)[0] == 10000)
				getCharacterCurrentParams(kCharacterAnna)[0] = 0;
			else
				getCharacterCurrentParams(kCharacterAnna)[0] = 10000;
		}

		break;
	case 1:
		getCharacter(kCharacterAnna).clothes++;
		if (getCharacter(kCharacterAnna).clothes > 3)
			getCharacter(kCharacterAnna).clothes = 0;

		break;
	case 12:
		getCharacter(kCharacterAnna).characterPosition.position = 0;
		getCharacter(kCharacterAnna).characterPosition.location = 0;
		getCharacter(kCharacterAnna).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAnna).inventoryItem = 0x80;
		getCharacterCurrentParams(kCharacterAnna)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoSeqOtis);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAnna, (char *)&getCharacterCurrentParams(kCharacterAnna)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DoBlockSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoBlockSeqOtis);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoBlockSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseView(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[3], getCharacterCurrentParams(kCharacterAnna)[4]);
		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAnna, (char *)&getCharacterCurrentParams(kCharacterAnna)[0]);
		blockView(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[3], getCharacterCurrentParams(kCharacterAnna)[4]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoCorrOtis);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[3]);
		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAnna, (char *)&getCharacterCurrentParams(kCharacterAnna)[0]);
		blockAtDoor(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_FinishSeqOtis);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 5;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterAnna).direction != 4) {
			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DoJoinedSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoJoinedSeqOtis);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;
	params->parameters[4] = param3.intParam;

	strncpy((char *)&params->parameters[5], param4.stringParam, 12);


	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoJoinedSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		if (!getCharacterCurrentParams(kCharacterAnna)[8])
			fedEx(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[3], getCharacterCurrentParams(kCharacterAnna)[4], (char *)&getCharacterCurrentParams(kCharacterAnna)[5]);

		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	case 10:
		if (!getCharacterCurrentParams(kCharacterAnna)[8]) {
			fedEx(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[3], getCharacterCurrentParams(kCharacterAnna)[4], (char *)&getCharacterCurrentParams(kCharacterAnna)[5]);
			getCharacterCurrentParams(kCharacterAnna)[8] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterAnna, (char *)&getCharacterCurrentParams(kCharacterAnna)[0]);
		break;
	}
}

void LogicManager::CONS_Anna_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoDialog);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 7;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	case 12:
		playDialog(kCharacterAnna, (char *)&getCharacterCurrentParams(kCharacterAnna)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_WaitRCClear);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 8;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_SaveGame);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 9;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	case 12:
		save(
			kCharacterAnna,
			getCharacterCurrentParams(kCharacterAnna)[0],
			getCharacterCurrentParams(kCharacterAnna)[1]
		);

		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoWalk);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[0], getCharacterCurrentParams(kCharacterAnna)[1])) {
			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		}

		break;
	case 5:
		if (_doneNIS[kEventAugustPresentAnna] || _doneNIS[kEventAugustPresentAnnaFirstIntroduction] || _globals[kGlobalChapter] >= 2) {
			playDialog(kCharacterCath, "CAT1001", -1, 0);
		} else {
			playCathExcuseMe();
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterAnna, kCharacterCath, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoWait);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 11;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[1] ||
			(getCharacterCurrentParams(kCharacterAnna)[1] = _gameTime + getCharacterCurrentParams(kCharacterAnna)[0], _gameTime + getCharacterCurrentParams(kCharacterAnna)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterAnna)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterAnna)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_PracticeMusic(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_PracticeMusic);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 12;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_PracticeMusic(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterAnna)[1] && getCharacterParams(kCharacterAnna, 8)[0])
			getCharacterCurrentParams(kCharacterAnna)[1] = 1;

		if (getCharacterCurrentParams(kCharacterAnna)[5]) {
			bool skip = false; // Horrible way to unroll a goto...

			if (!getCharacterCurrentParams(kCharacterAnna)[6]) {
				getCharacterCurrentParams(kCharacterAnna)[6] = _realTime + 75;
				if (_realTime == -75) {
					skip = true;
					send(kCharacterAnna, kCharacterAnna, 2, 0);
					getCharacterCurrentParams(kCharacterAnna)[5] = 0;
					getCharacterCurrentParams(kCharacterAnna)[6] = 0;
				}
			}

			if (!skip && getCharacterCurrentParams(kCharacterAnna)[6] < _realTime) {
				getCharacterCurrentParams(kCharacterAnna)[6] = 0x7FFFFFFF;
				send(kCharacterAnna, kCharacterAnna, 2, 0);
				getCharacterCurrentParams(kCharacterAnna)[5] = 0;
				getCharacterCurrentParams(kCharacterAnna)[6] = 0;
			}
		}

		if (getCharacterCurrentParams(kCharacterAnna)[3]) {
			if (getCharacterCurrentParams(kCharacterAnna)[7] || (getCharacterCurrentParams(kCharacterAnna)[7] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterAnna)[7] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAnna)[7] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterAnna)[3] = 0;
			getCharacterCurrentParams(kCharacterAnna)[4] = 1;
			setDoor(37, kCharacterAnna, 1, 0, 9);
			setDoor(53, kCharacterAnna, 1, 0, 9);
			getCharacterCurrentParams(kCharacterAnna)[0]--;
			send(kCharacterAnna, kCharacterAnna, 2, 0);
			getCharacterCurrentParams(kCharacterAnna)[7] = 0;
		} else {
			getCharacterCurrentParams(kCharacterAnna)[7] = 0;
		}

		break;
	case 2:
		if (getCharacterCurrentParams(kCharacterAnna)[1]) {
			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		} else {
			getCharacterCurrentParams(kCharacterAnna)[0]++;
			switch (getCharacterCurrentParams(kCharacterAnna)[0]) {
			case 1:
				playDialog(kCharacterAnna, "ANN2135A", -1, 0);
				break;
			case 2:
				playDialog(kCharacterAnna, "ANN2135B", -1, 0);
				break;
			case 3:
			case 4:
				playDialog(kCharacterAnna, "ANN2135C", -1, 0);
				break;
			case 5:
			case 12:
				playDialog(kCharacterAnna, "ANN2135L", -1, 0);
				break;
			case 6:
			case 8:
				playDialog(kCharacterAnna, "ANN2135K", -1, 0);
				break;
			case 7:
				playDialog(kCharacterAnna, "ANN2135H", -1, 0);
				break;
			case 9:
				playDialog(kCharacterAnna, "ANN2135I", -1, 0);
				break;
			case 10:
				playDialog(kCharacterAnna, "ANN2135J", -1, 0);
				break;
			case 11:
				playDialog(kCharacterAnna, "ANN2135M", -1, 0);
				break;
			case 13:
				setDoor(37, kCharacterAnna, 1, 10, 9);
				setDoor(53, kCharacterAnna, 1, 10, 9);

				getCharacter(kCharacterAnna).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
				fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
				break;
			default:
				break;
			}
		}

		break;
	case 8:
		if (getCharacterCurrentParams(kCharacterAnna)[3]) {
			setDoor(37, kCharacterAnna, 1, 0, 9);
			setDoor(53, kCharacterAnna, 1, 0, 9);

			if (msg->param.intParam == 53) {
				playDialog(kCharacterCath, getCathWCDialog(), -1, 0);
			} else if (cathHasItem(kItemPassengerList)) {
				if (rnd(2)) {
					playDialog(kCharacterCath, getCathSorryDialog(), -1, 0);
				} else {
					if (rnd(2) == 0) {
						playDialog(kCharacterCath, "CAT1506A", -1, 0);
					} else {
						playDialog(kCharacterCath, "CAT1506", -1, 0);
					}
				}
			} else {
				playDialog(kCharacterCath, getCathSorryDialog(), -1, 0);
			}

			getCharacterCurrentParams(kCharacterAnna)[3] = 0;
			getCharacterCurrentParams(kCharacterAnna)[4] = 1;
		} else {
			endDialog(kCharacterAnna);
			setDoor(37, kCharacterAnna, 1, 0, 0);
			setDoor(53, kCharacterAnna, 1, 0, 0);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB012", 0, 0, 0);
		}

		break;
	case 9:
		endDialog(kCharacterAnna);
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
		AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB013", 0, 0, 0);
		break;
	case 12:
		getCharacterCurrentParams(kCharacterAnna)[0] = 1;
		setDoor(37, kCharacterAnna, 1, 10, 9);
		setDoor(53, kCharacterAnna, 1, 10, 9);
		setDoor(45, kCharacterCath, 0, 255, 255);

		if (checkCathDir(kCarRedSleeping, 78))
			bumpCath(kCarRedSleeping, 49, 255);

		startCycOtis(kCharacterAnna, "418C");

		if (whoRunningDialog(kCharacterAnna))
			fadeDialog(kCharacterAnna);

		playDialog(kCharacterAnna, "ANN2135A", -1, 0);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterAnna)[4] || getCharacterCurrentParams(kCharacterAnna)[3]) {
			setDoor(37, kCharacterAnna, 1, 10, 9);
			setDoor(53, kCharacterAnna, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAnna)[4] = 0;
			getCharacterCurrentParams(kCharacterAnna)[3] = 0;
		}

		if (checkCathDir(kCarRedSleeping, 60)) {
			getCharacterCurrentParams(kCharacterAnna)[2]++;
			if (getCharacterCurrentParams(kCharacterAnna)[2] == 2) {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
				AnnaCall(&LogicManager::CONS_Anna_DoSeqOtis, "418B", 0, 0, 0);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Ann1016", 0, 0, 0);
			break;
		case 2:
			setDoor(37, kCharacterAnna, 1, 14, 9);
			setDoor(53, kCharacterAnna, 1, 14, 9);
			getCharacterCurrentParams(kCharacterAnna)[3] = 1;
			break;
		case 3:
			if (whoRunningDialog(kCharacterMax)) {
				getCharacterCurrentParams(kCharacterAnna)[0]--;
				getCharacterCurrentParams(kCharacterAnna)[5] = 1;
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "MAX1120", 0, 0, 0);
			}

			break;
		case 4:
			getCharacterCurrentParams(kCharacterAnna)[0]--;
			getCharacterCurrentParams(kCharacterAnna)[5] = 1;
			break;
		case 5:
			startCycOtis(kCharacterAnna, "418A");
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DoComplexSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoComplexSeqOtis);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 13;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);
	strncpy((char *)&params->parameters[3], param2.stringParam, 12);

	params->parameters[6] = param3.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoComplexSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAnna, (char *)&getCharacterCurrentParams(kCharacterAnna)[0]);
		startSeqOtis(getCharacterCurrentParams(kCharacterAnna)[6], (char *)&getCharacterCurrentParams(kCharacterAnna)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DoWaitReal(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoWaitReal);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 14;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoWaitReal(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[1] ||
			(getCharacterCurrentParams(kCharacterAnna)[1] = _realTime + getCharacterCurrentParams(kCharacterAnna)[0],
			_realTime + getCharacterCurrentParams(kCharacterAnna)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterAnna)[1] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterAnna)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_CompLogic(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_CompLogic);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 15;

	params->clear();

	params->parameters[0] = param1.intParam;

	strncpy((char *)&params->parameters[1], param2.stringParam, 12);

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_CompLogic(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[0] < _gameTime && !getCharacterCurrentParams(kCharacterAnna)[6]) {
			getCharacterCurrentParams(kCharacterAnna)[6] = 1;
			setDoor(37, kCharacterCath, 1, 10, 9);
			setDoor(53, kCharacterCath, 1, 10, 9);

			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterAnna)[4]) {
			if (getCharacterCurrentParams(kCharacterAnna)[7] || (getCharacterCurrentParams(kCharacterAnna)[7] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterAnna)[7] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAnna)[7] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterAnna)[4] = 0;
			getCharacterCurrentParams(kCharacterAnna)[5] = 1;

			setDoor(37, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
			setDoor(53, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
			getCharacterCurrentParams(kCharacterAnna)[7] = 0;
		} else {
			getCharacterCurrentParams(kCharacterAnna)[7] = 0;
		}

		break;
	case 8:
	case 9:
		if (msg->action == 9 && inComp(kCharacterMax, kCarRedSleeping, 4070)) {
			setDoor(37, kCharacterAnna, 1, 0, 0);
			setDoor(53, kCharacterAnna, 1, 0, 0);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB013", 0, 0, 0);
		} else {
			if (getCharacterCurrentParams(kCharacterAnna)[4]) {
				setDoor(37, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
				setDoor(53, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);

				if (msg->param.intParam == 53) {
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 6;
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathWCDialog(), 0, 0, 0);
				} else if (cathHasItem(kItemPassengerList)) {
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 7;
					if (rnd(2)) {
						AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathSorryDialog(), 0, 0, 0);
					} else {
						if (rnd(2) == 0) {
							AnnaCall(&LogicManager::CONS_Anna_DoDialog, "CAT1506A", 0, 0, 0);
						} else {
							AnnaCall(&LogicManager::CONS_Anna_DoDialog, "CAT1506", 0, 0, 0);
						}
					}
				} else {
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 8;
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathSorryDialog(), 0, 0, 0);
				}
			} else {
				setDoor(37, kCharacterAnna, 1, 0, 0);
				setDoor(53, kCharacterAnna, 1, 0, 0);
				if (msg->action == 8) {
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB012", 0, 0, 0);
				} else {
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB013", 0, 0, 0);
				}
			}
		}

		break;
	case 12:
		setDoor(37, kCharacterAnna, 1, 10, 9);
		setDoor(53, kCharacterAnna, 1, 10, 9);
		startCycOtis(kCharacterAnna, (char *)&getCharacterCurrentParams(kCharacterAnna)[1]);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterAnna)[5] || getCharacterCurrentParams(kCharacterAnna)[4]) {
			setDoor(37, kCharacterAnna, 1, 10, 9);
			setDoor(53, kCharacterAnna, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAnna)[5] = 0;
			getCharacterCurrentParams(kCharacterAnna)[4] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			if (whoRunningDialog(kCharacterMax)) {
				setDoor(37, kCharacterAnna, 1, 10, 9);
				setDoor(53, kCharacterAnna, 1, 10, 9);
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "MAX1120", 0, 0, 0);
			}

			break;
		case 2:
			setDoor(37, kCharacterAnna, 1, 10, 9);
			setDoor(53, kCharacterAnna, 1, 10, 9);
			break;
		case 3:
		case 4:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "ANN1016", 0, 0, 0);
			break;
		case 5:
			setDoor(37, kCharacterAnna, 1, 14, 0);
			setDoor(53, kCharacterAnna, 1, 14, 0);
			getCharacterCurrentParams(kCharacterAnna)[4] = 1;
			break;
		case 6:
		case 7:
		case 8:
			getCharacterCurrentParams(kCharacterAnna)[4] = 0;
			getCharacterCurrentParams(kCharacterAnna)[5] = 1;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_Birth);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 16;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterAnna)[0]) {
			getCharacterCurrentParams(kCharacterAnna)[0] = 1;
			CONS_Anna_FleeTyler(0, 0, 0, 0);
		}

		break;
	case 12:
		autoMessage(kCharacterAnna, 291662081, 0);
		autoMessage(kCharacterAnna, 238936000, 1);
		setDoor(37, kCharacterCath, 1, 10, 9);
		setDoor(53, kCharacterCath, 1, 10, 9);
		setDoor(45, kCharacterCath, 1, 255, 255);
		getCharacter(kCharacterAnna).characterPosition.position = 8200;
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		getCharacter(kCharacterAnna).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAnna).clothes = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DoWalkP1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoWalkP1);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 17;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoWalkP1(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[2] && nearChar(kCharacterAnna, kCharacterCath, 2000)) {
			getCharacter(kCharacterAnna).inventoryItem = getCharacterCurrentParams(kCharacterAnna)[2];
		} else {
			getCharacter(kCharacterAnna).inventoryItem = 0;
		}

		if (walk(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[0], getCharacterCurrentParams(kCharacterAnna)[1])) {
			getCharacter(kCharacterAnna).inventoryItem = 0;

			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		}

		break;
	case 1:
		if (msg->param.intParam == 8) {
			getCharacter(kCharacterAnna).inventoryItem &= ~8;
			getCharacterCurrentParams(kCharacterAnna)[2] &= ~8;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaGiveScarf, 0, 0);
		} else {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventGotALight, 0, 0);
		}

		break;
	case 5:
		if (_doneNIS[kEventAugustPresentAnna] || _doneNIS[kEventAugustPresentAnnaFirstIntroduction] || _globals[kGlobalChapter] >= 2) {
			playDialog(kCharacterCath, "CAT1001", -1, 0);
		} else {
			playCathExcuseMe();
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterAnna, kCharacterCath, 0);
		break;
	case 12:
		getCharacter(kCharacterAnna).inventoryItem = 0;
		if (_globals[kGlobalJacket] == 2 && !_doneNIS[kEventGotALight] && !_doneNIS[kEventGotALightD] && !_doneNIS[kEventAugustPresentAnna] && !_doneNIS[kEventAugustPresentAnnaFirstIntroduction]) {
			getCharacterCurrentParams(kCharacterAnna)[2] = 128;
		}

		if (_globals[kGlobalJacket] == 2 && !getCharacterCurrentParams(kCharacterAnna)[2] && !_doneNIS[kEventAnnaGiveScarfAsk] && !_doneNIS[kEventAnnaGiveScarfDinerAsk] && !_doneNIS[kEventAnnaGiveScarfSalonAsk]) {
			getCharacterCurrentParams(kCharacterAnna)[2] = 8;
		}

		if (walk(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[0], getCharacterCurrentParams(kCharacterAnna)[1])) {
			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			if (_doneNIS[kEventAnnaGiveScarf] || _doneNIS[kEventAnnaGiveScarfDiner] || _doneNIS[kEventAnnaGiveScarfSalon] || _doneNIS[kEventAnnaGiveScarfMonogram] || _doneNIS[kEventAnnaGiveScarfDinerMonogram] || _doneNIS[kEventAnnaGiveScarfSalonMonogram]) {
				playNIS(kEventAnnaGiveScarfAsk);
			} else if (_doneNIS[kEventAugustPresentAnna] || _doneNIS[kEventAugustPresentAnnaFirstIntroduction]) {
				playNIS(kEventAnnaGiveScarfMonogram);
			} else {
				playNIS(kEventAnnaGiveScarf);
			}

			if (getCharacter(kCharacterAnna).direction != 1) {
				bumpCathRx(getCharacter(kCharacterAnna).characterPosition.car, getCharacter(kCharacterAnna).characterPosition.position + 750);
			}
		} else {
			if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] != 2)
				break;

			if (getCharacter(kCharacterAnna).direction == 1) {
				playNIS(kEventGotALightD);
			} else {
				playNIS(kEventGotALight);
			}

			getCharacter(kCharacterAnna).inventoryItem &= ~0x80;

			int32 tmp = getCharacterCurrentParams(kCharacterAnna)[2] & 0xFFFFFF7F;
			getCharacterCurrentParams(kCharacterAnna)[2] = tmp;
			if (_globals[kGlobalJacket] == 2 && !_doneNIS[kEventAnnaGiveScarfAsk] && !_doneNIS[kEventAnnaGiveScarfDinerAsk] && !_doneNIS[kEventAnnaGiveScarfSalonAsk])
				getCharacterCurrentParams(kCharacterAnna)[2] = tmp | 8;

			if (getCharacter(kCharacterAnna).direction != 1) {
				bumpCathRx(getCharacter(kCharacterAnna).characterPosition.car, getCharacter(kCharacterAnna).characterPosition.position + 750);
			}
		}

		bumpCathFx(getCharacter(kCharacterAnna).characterPosition.car, getCharacter(kCharacterAnna).characterPosition.position - 750);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DiningLogic(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DiningLogic);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 18;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DiningLogic(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[0] && _gameTime > getCharacterCurrentParams(kCharacterAnna)[0] && rcClear()) {
			getCharacter(kCharacterAnna).inventoryItem = 0;

			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterAnna)[4] && !getCharacterCurrentParams(kCharacterAnna)[3]) {
			if (getCharacterCurrentParams(kCharacterAnna)[5] || (getCharacterCurrentParams(kCharacterAnna)[5] = _gameTime + 900, _gameTime != -900)) {
				if (getCharacterCurrentParams(kCharacterAnna)[5] >= _gameTime) {
					if (getCharacterCurrentParams(kCharacterAnna)[2]) {
						if (getCharacterCurrentParams(kCharacterAnna)[6] || (getCharacterCurrentParams(kCharacterAnna)[6] = _realTime + 90, _realTime != -90)) {
							if (getCharacterCurrentParams(kCharacterAnna)[6] >= _realTime)
								break;

							getCharacterCurrentParams(kCharacterAnna)[6] = 0x7FFFFFFF;
						}

						bumpCath(kCarRestaurant, 61, 255);
					} else {
						getCharacterCurrentParams(kCharacterAnna)[6] = 0;
					}

					break;
				}

				getCharacterCurrentParams(kCharacterAnna)[5] = 0x7FFFFFFF;
			}
			getCharacterCurrentParams(kCharacterAnna)[1] |= 8;
			getCharacterCurrentParams(kCharacterAnna)[4] = 0;
			getCharacterCurrentParams(kCharacterAnna)[5] = 0;
		}

		if (getCharacterCurrentParams(kCharacterAnna)[2]) {
			if (getCharacterCurrentParams(kCharacterAnna)[6] || (getCharacterCurrentParams(kCharacterAnna)[6] = _realTime + 90, _realTime != -90)) {
				if (getCharacterCurrentParams(kCharacterAnna)[6] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAnna)[6] = 0x7FFFFFFF;
			}

			bumpCath(kCarRestaurant, 61, 255);
		} else {
			getCharacterCurrentParams(kCharacterAnna)[6] = 0;
		}

		break;
	case 1:
		if (msg->param.intParam == 8) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaGiveScarf, 0, 0);
		} else {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventDinerMindJoin, 0, 0);
		}

		break;
	case 12:
		if (_globals[kGlobalJacket] == 2 && !_doneNIS[kEventDinerMindJoin] && !_doneNIS[kEventAugustPresentAnna] && !_doneNIS[kEventAugustPresentAnnaFirstIntroduction]) {
			getCharacterCurrentParams(kCharacterAnna)[1] |= 0x80;
		}

		if (_globals[kGlobalJacket] == 2 && !getCharacterCurrentParams(kCharacterAnna)[1] && !_doneNIS[kEventAnnaGiveScarfAsk] && !_doneNIS[kEventAnnaGiveScarfDinerAsk] && !_doneNIS[kEventAnnaGiveScarfSalonAsk]) {
			getCharacterCurrentParams(kCharacterAnna)[1] = 8;
		}

		getCharacter(kCharacterAnna).inventoryItem = getCharacterCurrentParams(kCharacterAnna)[1];
		break;
	case 17:
		getCharacterCurrentParams(kCharacterAnna)[2] = checkCathDir(kCarRestaurant, 62);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			if (_doneNIS[kEventAnnaGiveScarf] || _doneNIS[kEventAnnaGiveScarfDiner] || _doneNIS[kEventAnnaGiveScarfSalon] || _doneNIS[kEventAnnaGiveScarfMonogram] || _doneNIS[kEventAnnaGiveScarfDinerMonogram] || _doneNIS[kEventAnnaGiveScarfSalonMonogram]) {
				playNIS(kEventAnnaGiveScarfDinerAsk);
			} else {
				if (_doneNIS[kEventAugustPresentAnna] || _doneNIS[kEventAugustPresentAnnaFirstIntroduction])
					playNIS(kEventAnnaGiveScarfDinerMonogram);
				else
					playNIS(kEventAnnaGiveScarfDiner);

				getCharacterCurrentParams(kCharacterAnna)[4] = 1;
			}

			int32 tmp = getCharacterCurrentParams(kCharacterAnna)[1] & 0xFFFFFFF7;
			getCharacterCurrentParams(kCharacterAnna)[1] = tmp;
			getCharacter(kCharacterAnna).inventoryItem = tmp;
			bumpCath(kCarRestaurant, 61, 255);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			playNIS(kEventDinerMindJoin);
			int32 tmp = getCharacterCurrentParams(kCharacterAnna)[1] & 0xFFFFFF7F;
			getCharacterCurrentParams(kCharacterAnna)[1] = tmp;

			if (_globals[kGlobalJacket] == 2 && !_doneNIS[kEventAnnaGiveScarfAsk] && !_doneNIS[kEventAnnaGiveScarfDinerAsk] && !_doneNIS[kEventAnnaGiveScarfSalonAsk]) {
				getCharacterCurrentParams(kCharacterAnna)[1] = tmp | 8;
			}

			getCharacter(kCharacterAnna).inventoryItem = getCharacterCurrentParams(kCharacterAnna)[1];
			bumpCath(kCarRestaurant, 61, 255);
		}

		break;
	case 168046720:
		getCharacter(kCharacterAnna).inventoryItem = 0;
		getCharacterCurrentParams(kCharacterAnna)[3] = 1;
		break;
	case 168627977:
		getCharacter(kCharacterAnna).inventoryItem = getCharacterCurrentParams(kCharacterAnna)[1];
		getCharacterCurrentParams(kCharacterAnna)[3] = 0;
		break;
	case 170016384:
	case 259136835:
	case 268773672:
		getCharacter(kCharacterAnna).inventoryItem = 0;

		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	default:
		break;
	}
	return;
}

void LogicManager::CONS_Anna_FleeTyler(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_FleeTyler);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 19;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_FleeTyler(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "618Ca", 1, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			getCharacter(kCharacterAnna).characterPosition.position = 8514;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoWalk, 4, 4070, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "618Af", 37, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterAnna);
			getCharacter(kCharacterAnna).characterPosition.position = 4070;
			getCharacter(kCharacterAnna).characterPosition.location = 1;
			CONS_Anna_WaitDinner(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_WaitDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_WaitDinner);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 20;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_WaitDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_CompLogic, 1093500, "NONE", 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "618Bf", 37, 0, 0);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			send(kCharacterAnna, kCharacterMax, 71277948, 0);
			CONS_Anna_GoDinner(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_GoDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_GoDinner);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 21;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_GoDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoWalkP1, 5, 850, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAnna).characterPosition.position = 1540;
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoSeqOtis, "801US", 0, 0, 0);
			break;
		case 3:
			startSeqOtis(kCharacterAnna, "001B");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterAnna);

			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
			AnnaCall(&LogicManager::CONS_Anna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 4:
			CONS_Anna_WaitHW(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_WaitHW(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_WaitHW);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 22;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_WaitHW(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		startCycOtis(kCharacterAnna, "001A");
		send(kCharacterAnna, kCharacterHeadWait, 223262556, 0);
		break;
	case 157370960:
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		CONS_Anna_WaitingDinner(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_WaitingDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_WaitingDinner);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 23;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_WaitingDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		startCycOtis(kCharacterAnna, "001D");
		send(kCharacterAnna, kCharacterWaiter1, 270410280, 0);
		send(kCharacterAnna, kCharacterTableA, 136455232, 0);
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DiningLogic, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterAnna, "001E");
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "ANN1048", 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoSeqOtis, "001F", 0, 0, 0);
			break;
		case 3:
			send(kCharacterAnna, kCharacterWaiter1, 203859488, 0);
			CONS_Anna_WaitingDinner2(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_WaitingDinner2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_WaitingDinner2);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 24;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_WaitingDinner2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		startCycOtis(kCharacterAnna, "001G");
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DiningLogic, 0, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			startCycOtis(kCharacterAnna, "001H");
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "ANN1049", 0, 0, 0);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			send(kCharacterAnna, kCharacterWaiter1, 136702400, 0);
			CONS_Anna_EatingDinner(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_EatingDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_EatingDinner);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 25;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_EatingDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		startCycOtis(kCharacterAnna, "001J");
		_globals[kGlobalAnnaIsEating] = 1;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DiningLogic, 0, 0, 0, 0);

		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_WaitRCClear, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 3) {
			CONS_Anna_LeaveDinner(0, 0, 0, 0);
		}

		break;
	case 122358304:
		startCycOtis(kCharacterAnna, "BLANK");
		break;
	case 201437056:
		startCycOtis(kCharacterAnna, "001J");
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
		AnnaCall(&LogicManager::CONS_Anna_DiningLogic, 1138500, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_LeaveDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_LeaveDinner);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 26;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_LeaveDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).characterPosition.location = 0;
		blockView(kCharacterAnna, kCarRestaurant, 62);
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoJoinedSeqOtis, "001L", 33, 103798704, "001M");
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			releaseView(kCharacterAnna, kCarRestaurant, 62);
			send(kCharacterAnna, kCharacterWaiter1, 237485916, 0);
			startSeqOtis(kCharacterAnna, "801DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterAnna);

			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoWalkP1, 4, 4070, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "618Af", 37, 0, 0);
			break;
		case 4:
			endGraphics(kCharacterAnna);
			getCharacter(kCharacterAnna).characterPosition.position = 4070;
			getCharacter(kCharacterAnna).characterPosition.location = 1;
			CONS_Anna_FreshenUp(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_FreshenUp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_FreshenUp);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 27;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_FreshenUp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		send(kCharacterAnna, kCharacterMax, 101687594, 0);
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_CompLogic, 1156500, "NONE", 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1 ||
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterPolice) {
				getCharacterCurrentParams(kCharacterAnna)[0] = _gameTime + 900;
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
				AnnaCall(&LogicManager::CONS_Anna_CompLogic, getCharacterCurrentParams(kCharacterAnna)[0], "NONE", 0, 0);
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
				AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "618Bf", 37, 0, 0);
			}
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 3) {
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			send(kCharacterAnna, kCharacterMax, 71277948, 0);
			CONS_Anna_GoSalon(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_GoSalon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_GoSalon);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 28;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_GoSalon(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoWalkP1, 5, 850, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAnna).characterPosition.position = 1540;
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			takeItem(kItem3);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoBlockSeqOtis, "104A", 5, 56, 0);
			break;
		case 3:
			getCharacter(kCharacterAnna).characterPosition.location = 1;
			CONS_Anna_WaitAugust(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_WaitAugust(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_WaitAugust);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 29;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_WaitAugust(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[1]) {
			if (getCharacterCurrentParams(kCharacterAnna)[2] || (getCharacterCurrentParams(kCharacterAnna)[2] = _gameTime + 900, _gameTime != -900)) {
				if (getCharacterCurrentParams(kCharacterAnna)[2] >= _gameTime) {
					if (getCharacterCurrentParams(kCharacterAnna)[0]) {
						if (getCharacterCurrentParams(kCharacterAnna)[3] || (getCharacterCurrentParams(kCharacterAnna)[3] = _realTime + 90, _realTime != -90)) {
							if (getCharacterCurrentParams(kCharacterAnna)[3] >= _realTime)
								break;

							getCharacterCurrentParams(kCharacterAnna)[3] = 0x7FFFFFFF;
						}

						bumpCath(kCarRestaurant, 55, 255);
					} else {
						getCharacterCurrentParams(kCharacterAnna)[3] = 0;
					}

					break;
				}

				getCharacterCurrentParams(kCharacterAnna)[2] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterAnna).inventoryItem |= kItemScarf;
			getCharacterCurrentParams(kCharacterAnna)[1] = 0;
			getCharacterCurrentParams(kCharacterAnna)[2] = 0;
		}

		if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			if (getCharacterCurrentParams(kCharacterAnna)[3] || (getCharacterCurrentParams(kCharacterAnna)[3] = _realTime + 90, _realTime != -90)) {
				if (getCharacterCurrentParams(kCharacterAnna)[3] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAnna)[3] = 0x7FFFFFFF;
			}

			bumpCath(kCarRestaurant, 55, 255);
		} else {
			getCharacterCurrentParams(kCharacterAnna)[3] = 0;
		}

		break;
	case 1:
		if (msg->param.intParam == 8) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaGiveScarf, 0, 0);
		} else {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaIntroductionRejected, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterAnna).inventoryItem = 0;

		if (_globals[kGlobalJacket] == 2 && !_doneNIS[kEventAnnaConversationGoodNight] && !_doneNIS[kEventAnnaIntroductionRejected])
			getCharacter(kCharacterAnna).inventoryItem = 0x80;

		if (_globals[kGlobalJacket] == 2 && !getCharacter(kCharacterAnna).inventoryItem && !_doneNIS[kEventAnnaGiveScarfAsk] && !_doneNIS[kEventAnnaGiveScarfDinerAsk] && !_doneNIS[kEventAnnaGiveScarfSalonAsk]) {
			getCharacter(kCharacterAnna).inventoryItem = 8;
		}

		startCycOtis(kCharacterAnna, "104B");
		break;
	case 17:
		getCharacterCurrentParams(kCharacterAnna)[0] = checkCathDir(kCarRestaurant, 56);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			if (_doneNIS[kEventAnnaGiveScarf] ||
				_doneNIS[kEventAnnaGiveScarfDiner] ||
				_doneNIS[kEventAnnaGiveScarfSalon] ||
				_doneNIS[kEventAnnaGiveScarfMonogram] ||
				_doneNIS[kEventAnnaGiveScarfDinerMonogram] ||
				_doneNIS[kEventAnnaGiveScarfSalonMonogram]) {
				playNIS(kEventAnnaGiveScarfSalonAsk);
			} else {
				if (_doneNIS[kEventAugustPresentAnna] || _doneNIS[kEventAugustPresentAnnaFirstIntroduction]) {
					playNIS(kEventAnnaGiveScarfSalonMonogram);
				} else {
					playNIS(kEventAnnaGiveScarfSalon);
				}

				getCharacterCurrentParams(kCharacterAnna)[1] = 1;
			}

			getCharacter(kCharacterAnna).inventoryItem &= ~8;
			bumpCath(kCarRestaurant, 51, 255);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			if (_doneNIS[kEventAugustPresentAnna] || _doneNIS[kEventAugustPresentAnnaFirstIntroduction]) {
				playNIS(kEventAnnaConversationGoodNight);
			} else {
				playNIS(kEventAnnaIntroductionRejected);
			}

			getCharacter(kCharacterAnna).inventoryItem &= ~0x80;
			if (_globals[kGlobalJacket] == 2 &&
				!_doneNIS[kEventAnnaGiveScarfAsk] &&
				!_doneNIS[kEventAnnaGiveScarfDinerAsk] &&
				!_doneNIS[kEventAnnaGiveScarfSalonAsk]) {
				getCharacter(kCharacterAnna).inventoryItem |= kItemScarf;
			}

			bumpCath(kCarRestaurant, 51, 255);
		}

		break;
	case 123712592:
		getCharacter(kCharacterAnna).inventoryItem = 0;
		CONS_Anna_FlirtAugust(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_FlirtAugust(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_FlirtAugust);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 30;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_FlirtAugust(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[2] != 0x7FFFFFFF && _gameTime != 0) {
			if (_gameTime <= 1188000) {
				bool skip = false; // Horrible way to unroll a goto...

				if (!inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterAnna)[2]) {
					getCharacterCurrentParams(kCharacterAnna)[2] = _gameTime + 450;
					if (_gameTime == -450) {
						skip = true;
						playDialog(kCharacterAnna, "AUG1004", -1, 0);
					}
				}

				if (!skip && getCharacterCurrentParams(kCharacterAnna)[2] < _gameTime) {
					getCharacterCurrentParams(kCharacterAnna)[2] = 0x7FFFFFFF;
					playDialog(kCharacterAnna, "AUG1004", -1, 0);
				}
			} else {
				getCharacterCurrentParams(kCharacterAnna)[2] = 0x7FFFFFFF;
				playDialog(kCharacterAnna, "AUG1004", -1, 0);
			}
		}

		if (getCharacterCurrentParams(kCharacterAnna)[1] != 0 && getCharacterCurrentParams(kCharacterAnna)[3] != 0x7FFFFFFF && _gameTime > 1179000) {
			if (_gameTime <= 1192500) {
				if (!inSalon(kCharacterCath) || !getCharacterCurrentParams(kCharacterAnna)[3]) {
					getCharacterCurrentParams(kCharacterAnna)[3] = _gameTime + 150;
					if (_gameTime == -150) {
						CONS_Anna_LeaveAugust(0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterAnna)[3] < _gameTime) {
					getCharacterCurrentParams(kCharacterAnna)[3] = 0x7FFFFFFF;
					CONS_Anna_LeaveAugust(0, 0, 0, 0);
					break;
				}
			} else {
				getCharacterCurrentParams(kCharacterAnna)[3] = 0x7FFFFFFF;
				CONS_Anna_LeaveAugust(0, 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			if (getCharacterCurrentParams(kCharacterAnna)[4] || (getCharacterCurrentParams(kCharacterAnna)[4] = _realTime + 90, _realTime != -90)) {
				if (getCharacterCurrentParams(kCharacterAnna)[4] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAnna)[4] = 0x7FFFFFFF;
			}

			bumpCath(kCarRestaurant, 55, 255);
		} else {
			getCharacterCurrentParams(kCharacterAnna)[4] = 0;
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterAnna)[1] = 1;
		break;
	case 12:
		send(kCharacterAnna, kCharacterAugust, 122358304, 0);
		startCycOtis(kCharacterAnna, "106B");
		break;
	case 17:
		getCharacterCurrentParams(kCharacterAnna)[0] = checkCathDir(kCarRestaurant, 56);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_LeaveAugust(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_LeaveAugust);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 31;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_LeaveAugust(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			playDialog(kCharacterAnna, "AUG1005", -1, 0);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoWaitReal, 0x96, 0, 0, 0);
			break;
		case 2:
			blockView(kCharacterAnna, kCarRestaurant, 56);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoComplexSeqOtis, "106C1", "106C2", 2, 0);
			break;
		case 3:
			releaseView(kCharacterAnna, kCarRestaurant, 56);
			dropItem(kItem3, 1);
			send(kCharacterAnna, kCharacterAugust, 159332865, 0);
			CONS_Anna_ReturnComp(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_ReturnComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_ReturnComp);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 32;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_ReturnComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoWalkP1, 4, 4070, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "618Af", 37, 0, 0);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			endGraphics(kCharacterAnna);
			getCharacter(kCharacterAnna).characterPosition.position = 4070;
			getCharacter(kCharacterAnna).characterPosition.location = 1;
			CONS_Anna_ReadyForBed(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_ReadyForBed(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_ReadyForBed);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 33;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_ReadyForBed(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		send(kCharacterAnna, kCharacterMax, 101687594, 0);
		getCharacterCurrentParams(kCharacterAnna)[0] = _gameTime + 4500;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_CompLogic, getCharacterCurrentParams(kCharacterAnna)[0], "NONE", 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			setModel(37, 1);
			CONS_Anna_Asleep(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_Asleep);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 34;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterAnna)[0] && checkCathDir(kCarRedSleeping, 60)) {
			if (!getCharacterCurrentParams(kCharacterAnna)[1]) {
				getCharacterCurrentParams(kCharacterAnna)[1] = _gameTime + 150;
				if (_gameTime == -150) {
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
					AnnaCall(&LogicManager::CONS_Anna_DoSeqOtis, "419B", 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterAnna)[1] < _gameTime) {
				getCharacterCurrentParams(kCharacterAnna)[1] = 0x7FFFFFFF;
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
				AnnaCall(&LogicManager::CONS_Anna_DoSeqOtis, "419B", 0, 0, 0);
				break;
			}
		}

		if (_gameTime > 1489500 && !getCharacterCurrentParams(kCharacterAnna)[2]) {
			getCharacterCurrentParams(kCharacterAnna)[2] = 1;
			CONS_Anna_WakeNight(0, 0, 0, 0);
		}

		break;
	case 8:
	case 9:
		setDoor(37, kCharacterAnna, 1, 0, 0);
		setDoor(53, kCharacterAnna, 1, 0, 0);
		if (msg->action == 8) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB012", 0, 0, 0);
		} else {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB013", 0, 0, 0);
		}

		break;
	case 12:
		setDoor(37, kCharacterAnna, 1, 10, 9);
		setDoor(53, kCharacterAnna, 1, 10, 9);
		setDoor(45, kCharacterCath, 0, 255, 255);

		if (checkCathDir(kCarRedSleeping, 78))
			bumpCath(kCarRedSleeping, 49, 255);

		getCharacter(kCharacterAnna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterAnna).characterPosition.position = 4070;
		getCharacter(kCharacterAnna).characterPosition.location = 0;
		startCycOtis(kCharacterAnna, "419A");
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			startCycOtis(kCharacterAnna, "419C");
			getCharacterCurrentParams(kCharacterAnna)[0] = 1;
			if (_gameTime > 1489500 && !getCharacterCurrentParams(kCharacterAnna)[2]) {
				getCharacterCurrentParams(kCharacterAnna)[2] = 1;
				CONS_Anna_WakeNight(0, 0, 0, 0);
			}

			break;
		case 2:
		case 3:
			if (whoRunningDialog(kCharacterMax)) {
				setDoor(37, kCharacterAnna, 1, 10, 9);
				setDoor(53, kCharacterAnna, 1, 10, 9);
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "MAX1120", 0, 0, 0);
			}

			break;
		case 4:
			setDoor(37, kCharacterAnna, 1, 10, 9);
			setDoor(53, kCharacterAnna, 1, 10, 9);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_WakeNight(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_WakeNight);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 35;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_WakeNight(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			if (getCharacterCurrentParams(kCharacterAnna)[2] || (getCharacterCurrentParams(kCharacterAnna)[2] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterAnna)[2] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAnna)[2] = 0x7FFFFFFF;
			}

			switch (getCharacterCurrentParams(kCharacterAnna)[1]) {
			case 0:
				playDialog(kCharacterAnna, "ANN2135E", -1, 0);
				break;
			case 1:
				playDialog(kCharacterAnna, "ANN2135F", -1, 0);
				break;
			case 2:
				playDialog(kCharacterAnna, "ANN2135G", -1, 0);
				break;
			case 3:
				playDialog(kCharacterAnna, "ANN2135D", -1, 0);
				break;
			default:
				break;
			}

			getCharacterCurrentParams(kCharacterAnna)[2] = 0;
			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterAnna)[1]++;
		if (getCharacterCurrentParams(kCharacterAnna)[1] > 3)
			getCharacterCurrentParams(kCharacterAnna)[1] = 0;

		getCharacterCurrentParams(kCharacterAnna)[0] = 1;
		break;
	case 8:
	case 9:
		if (whoRunningDialog(kCharacterAnna))
			fadeDialog(kCharacterAnna);

		if (msg->action == 8)
			playDialog(0, "LIB012", -1, 0);

		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaVisitToCompartmentGun, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterAnna).clothes = 1;
		getCharacterCurrentParams(kCharacterAnna)[0] = 1;
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			playNIS(kEventAnnaVisitToCompartmentGun);
			playDialog(0, "LIB015", -1, 0);
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			getCharacter(kCharacterAnna).characterPosition.position = 4840;
			walk(kCharacterAnna, kCarRedSleeping, 8200);
			bumpCathFDoor(37);
			send(kCharacterAnna, kCharacterVassili, 339669520, 0);
			send(kCharacterAnna, kCharacterTrainM, 339669520, 0);
			send(kCharacterAnna, kCharacterCond2, 339669520, 0);
			send(kCharacterAnna, kCharacterMax, 71277948, 0);
			CONS_Anna_GoVassili(0, 0, 0, 0);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			CONS_Anna_GoVassili(0, 0, 0, 0);
		}

		break;
	case 226031488:
		if (whoRunningDialog(kCharacterAnna))
			fadeDialog(kCharacterAnna);

		send(kCharacterAnna, kCharacterMax, 71277948, 0);
		break;
	case 238358920:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
		AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "608Cf", 37, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_GoVassili(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_GoVassili);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 36;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_GoVassili(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(37, kCharacterCath, 1, 10, 9);
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoWalk, 4, 8200, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			setDoor(32, kCharacterCath, 1, 255, 255);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "608Aa", 32, 0, 0);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			setDoor(32, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterAnna).characterPosition.location = 1;
			endGraphics(kCharacterAnna);
			CONS_Anna_AtSeizure(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_AtSeizure(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_AtSeizure);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 37;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_AtSeizure(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterAnna).characterPosition.position = 8200;
		getCharacter(kCharacterAnna).characterPosition.location = 0;
		break;
	case 191477936:
		CONS_Anna_SpeakTatiana(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_SpeakTatiana(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_SpeakTatiana);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 38;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_SpeakTatiana(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).characterPosition.position = 7500;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoDialog, "ANN1010", 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			playDialog(0, "MUS043", -1, 0);
			CONS_Anna_LeaveTatiana(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DoWalk1019(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoWalk1019);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 39;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoWalk1019(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (msg->action == 12) {
			getCharacter(kCharacterAnna).inventoryItem = 0;

			if (!_doneNIS[kEventAnnaGoodNight] && !_doneNIS[kEventAnnaGoodNightInverse])
				getCharacter(kCharacterAnna).inventoryItem = 0x80;
		}

		if (walk(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[0], getCharacterCurrentParams(kCharacterAnna)[1])) {
			getCharacter(kCharacterAnna).inventoryItem = 0;

			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaGoodNight, 0, 0);
		break;
	case 6:
		playDialog(kCharacterAnna, "ANN1107A", -1, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			playNIS((kEventAnnaGoodNightInverse - (getCharacter(kCharacterAnna).direction == 1)));
			getCharacter(kCharacterAnna).inventoryItem = 0;

			if (getCharacter(kCharacterAnna).direction == 1)
				bumpCathFx(getCharacter(kCharacterAnna).characterPosition.car, getCharacter(kCharacterAnna).characterPosition.position - 750);
			else
				bumpCathRx(getCharacter(kCharacterAnna).characterPosition.car, getCharacter(kCharacterAnna).characterPosition.position + 750);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_LeaveTatiana(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_LeaveTatiana);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 40;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_LeaveTatiana(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "608Cb", 33, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoWalk1019, 4, 4070, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "608Bf", 37, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterAnna);
			getCharacter(kCharacterAnna).characterPosition.location = 1;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
			AnnaCall(&LogicManager::CONS_Anna_DoWait, 150, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "608Cf", 37, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 6;
			AnnaCall(&LogicManager::CONS_Anna_DoWalk1019, 4, 7500, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 7;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "608Bb", 33, 0, 0);
			break;
		case 7:
			endGraphics(kCharacterAnna);
			getCharacter(kCharacterAnna).characterPosition.location = 1;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 8;
			AnnaCall(&LogicManager::CONS_Anna_DoWait, 150, 0, 0, 0);
			break;
		case 8:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 9;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "608Cb", 33, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 10;
			AnnaCall(&LogicManager::CONS_Anna_DoWalk1019, 4, 4070, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 11;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "608Bf", 37, 0, 0);
			break;
		case 11:
			endGraphics(kCharacterAnna);
			getCharacter(kCharacterAnna).characterPosition.location = 1;
			getCharacter(kCharacterAnna).characterPosition.position = 4070;
			CONS_Anna_GoBackToSleep(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_GoBackToSleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_GoBackToSleep);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 41;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_GoBackToSleep(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[1] || (getCharacterCurrentParams(kCharacterAnna)[1] = _gameTime + 2700, _gameTime != -2700)) {
			if (getCharacterCurrentParams(kCharacterAnna)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterAnna)[1] = 0x7FFFFFFF;
		}

		getCharacterCurrentParams(kCharacterAnna)[0]++;
		switch (getCharacterCurrentParams(kCharacterAnna)[0]) {
		case 1:
			startCycOtis(kCharacterAnna, "419A");
			break;
		case 2:
			startCycOtis(kCharacterAnna, "419B");
			break;
		case 3:
			startCycOtis(kCharacterAnna, "419C");
			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
			break;
		default:
			break;
		}

		getCharacterCurrentParams(kCharacterAnna)[1] = 0;
		break;
	case 8:
	case 9:
		setDoor(37, kCharacterAnna, 1, 0, 0);
		setDoor(53, kCharacterAnna, 1, 0, 0);

		if (msg->action == 8) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB012", 0, 0, 0);
		} else {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB013", 0, 0, 0);
		}

		break;
	case 12:
		send(kCharacterAnna, kCharacterMax, 101687594, 0);
		setDoor(37, kCharacterAnna, 1, 10, 9);
		setDoor(53, kCharacterAnna, 1, 10, 9);
		startCycOtis(kCharacterAnna, "419C");
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
			if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] <= 2) {
				if (!whoRunningDialog(kCharacterMax)) {
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, "MAX1120", 0, 0, 0);
					break;
				}
			} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] != 3) {
				break;
			}

			setDoor(37, kCharacterAnna, 1, 10, 9);
			setDoor(53, kCharacterAnna, 1, 10, 9);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_StartPart2);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 42;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Anna_InPart2(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAnna);
		getCharacter(kCharacterAnna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterAnna).characterPosition.position = 4070;
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		getCharacter(kCharacterAnna).inventoryItem = kItemNone;
		getCharacter(kCharacterAnna).clothes = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_InPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_InPart2);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 43;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_InPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(45, kCharacterCath, 0, 255, 255);
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_PracticeMusic, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_CompLogic, 1786500, "418C", 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_PracticeMusic, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
			AnnaCall(&LogicManager::CONS_Anna_CompLogic, 1818000, "418C", 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
			AnnaCall(&LogicManager::CONS_Anna_PracticeMusic, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 6;
			AnnaCall(&LogicManager::CONS_Anna_CompLogic, 15803100, "418C", 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_StartPart3);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 44;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Anna_Practicing(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAnna);
		getCharacter(kCharacterAnna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterAnna).characterPosition.position = 4070;
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		getCharacter(kCharacterAnna).clothes = 3;
		getCharacter(kCharacterAnna).inventoryItem = 0;
		setDoor(37, kCharacterCath, 1, 10, 9);
		setDoor(45, kCharacterCath, 0, 255, 255);
		setDoor(53, kCharacterCath, 1, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_ExitComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_ExitComp);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 45;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_ExitComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).characterPosition.location = 0;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "625Bf", 37, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			if (getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall].parameters[0])
				send(kCharacterAnna, kCharacterCond2, 185737168, 0);
			else
				send(kCharacterAnna, kCharacterCond2, 185671840, 0);

			playDialog(kCharacterAnna, "Ann3147", -1, 0);
			startCycOtis(kCharacterAnna, "625EF");
			softBlockAtDoor(kCharacterAnna, 37);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			softReleaseAtDoor(kCharacterAnna, 37);

			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		}

		break;
	case 157894320:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
		AnnaCall(&LogicManager::CONS_Anna_DoWait, 75, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_Practicing(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_Practicing);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 46;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_Practicing(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (checkCathDir(kCarRedSleeping, 60))
			bumpCath(kCarRedSleeping, 49, 255);

		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_PracticeMusic, 0, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1 ||
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			if (getCharacterParams(kCharacterAnna, 8)[0]) {
				CONS_Anna_GoLunch(0, 0, 0, 0);
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
				AnnaCall(&LogicManager::CONS_Anna_CompLogic, _gameTime + 4500, "418C", 0, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_GoLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_GoLunch);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 47;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_GoLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(37, kCharacterCath, 1, 10, 9);
		setDoor(53, kCharacterCath, 1, 10, 9);
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "688Bf", 0x25, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			send(kCharacterAnna, kCharacterMax, 71277948, 0);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAnna).characterPosition.position = 1540;
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
			AnnaCall(&LogicManager::CONS_Anna_DoSeqOtis, "801VS", 0, 0, 0);
			break;
		case 4:
			if (!_doneNIS[kEventAugustLunch]) {
				playDialog(kCharacterAnna, "Ann3136A", -1, 30);
			} else {
				playDialog(kCharacterAnna, "Ann3136", -1, 30);
			}

			send(kCharacterAnna, kCharacterAugust, 122358304, 0);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
			AnnaCall(&LogicManager::CONS_Anna_DoComplexSeqOtis, "026B1", "026B2", 2, 0);
			break;
		case 5:
			startCycOtis(kCharacterAugust, "BLANK");
			CONS_Anna_Lunch(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_Lunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_Lunch);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 48;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_Lunch(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			if (getCharacterCurrentParams(kCharacterAnna)[2] != 0x7FFFFFFF && _gameTime > 1969200) {
				if (_gameTime > 1983600) {
					getCharacterCurrentParams(kCharacterAnna)[2] = 0x7FFFFFFF;
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Aug3007A", 0, 0, 0);
					break;
				}

				if (!inDiningRoom(kCharacterCath) || whoRunningDialog(kCharacterMonsieur) || !getCharacterCurrentParams(kCharacterAnna)[2]) {
					getCharacterCurrentParams(kCharacterAnna)[2] = _gameTime + 150;
					if (_gameTime == -150) {
						getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
						AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Aug3007A", 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterAnna)[2] < _gameTime) {
					getCharacterCurrentParams(kCharacterAnna)[2] = 0x7FFFFFFF;
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Aug3007A", 0, 0, 0);
					break;
				}
			}

			if (getCharacterParams(kCharacterAnna, 8)[1]) {
				if (!getCharacterCurrentParams(kCharacterAnna)[1])
					getCharacterCurrentParams(kCharacterAnna)[1] = _gameTime + 4500;

				if (getCharacterCurrentParams(kCharacterAnna)[3] != 0x7FFFFFFF && _gameTime) {
					if (getCharacterCurrentParams(kCharacterAnna)[1] >= _gameTime) {
						if (!inDiningRoom(kCharacterCath) || !getCharacterCurrentParams(kCharacterAnna)[3]) {
							getCharacterCurrentParams(kCharacterAnna)[3] = _gameTime + 450;
							if (_gameTime == -450) {
								CONS_Anna_LeaveLunch(0, 0, 0, 0);
								break;
							}
						}

						if (getCharacterCurrentParams(kCharacterAnna)[3] >= _gameTime)
							break;
					}

					getCharacterCurrentParams(kCharacterAnna)[3] = 0x7FFFFFFF;
					CONS_Anna_LeaveLunch(0, 0, 0, 0);
				}
			}
		}

		break;
	case 12:
		startCycOtis(kCharacterAnna, "026c");
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoWait, 450, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Ann3137B", 0, 0, 0);
			break;
		case 2:
			send(kCharacterAnna, kCharacterWaiter1, 218983616, 0);
			break;
		case 3:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Aug3006A", 0, 0, 0);
			break;
		case 4:
			if (getCharacterParams(kCharacterAnna, 8)[1]) {
				if (!getCharacterCurrentParams(kCharacterAnna)[1])
					getCharacterCurrentParams(kCharacterAnna)[1] = _gameTime + 4500;

				if (getCharacterCurrentParams(kCharacterAnna)[3] != 0x7FFFFFFF && _gameTime) {
					if (getCharacterCurrentParams(kCharacterAnna)[1] >= _gameTime) {
						if (!inDiningRoom(kCharacterCath) || !getCharacterCurrentParams(kCharacterAnna)[3]) {
							getCharacterCurrentParams(kCharacterAnna)[3] = _gameTime + 450;
							if (_gameTime == -450) {
								CONS_Anna_LeaveLunch(0, 0, 0, 0);
								break;
							}
						}

						if (getCharacterCurrentParams(kCharacterAnna)[3] >= _gameTime)
							break;
					}

					getCharacterCurrentParams(kCharacterAnna)[3] = 0x7FFFFFFF;
					CONS_Anna_LeaveLunch(0, 0, 0, 0);
				}
			}

			break;
		case 5:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 6;
			AnnaCall(&LogicManager::CONS_Anna_DoWait, 900, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 7;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Aug3006", 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 8;
			AnnaCall(&LogicManager::CONS_Anna_DoWait, 2700, 0, 0, 0);
			break;
		case 8:
			startCycOtis(kCharacterAnna, "026H");
			getCharacterCurrentParams(kCharacterAnna)[0] = 1;
			break;
		default:
			break;
		}

		break;
	case 122288808:
		startCycOtis(kCharacterAnna, "026C");
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
		AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Ann3138A", 0, 0, 0);
		break;
	case 122358304:
		startCycOtis(kCharacterAnna, "BLANK");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DoOtis5026J(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoOtis5026J);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 49;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoOtis5026J(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		send(kCharacterAnna, kCharacterTableD, 103798704, "010M");
		endGraphics(kCharacterAugust);

		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterTableD, "026J3");
		startSeqOtis(kCharacterAugust, "026J2");
		startSeqOtis(kCharacterAnna, "026J1");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_LeaveLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_LeaveLunch);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 50;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_LeaveLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Ann3141", 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoOtis5026J, 0, 0, 0, 0);
			break;
		case 3:
			CONS_Anna_AfterLunch(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_AfterLunch(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_AfterLunch);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 51;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_AfterLunch(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			if (rcClear()) {
				playDialog(kCharacterAnna, "Aug3008", -1, 0);
				getCharacter(kCharacterAnna).characterPosition.location = 0;
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
				AnnaCall(&LogicManager::CONS_Anna_DoComplexSeqOtis, "112E1", "112E2", 2, 0);
			}
		}

		break;
	case 12:
		playDialog(kCharacterAnna, "Ann3142", -1, 30);
		blockView(kCharacterCath, kCarRestaurant, 57);
		startSeqOtis(kCharacterAnna, "112A");

		if (inDiningRoom(kCharacterCath))
			advanceFrame(kCharacterAnna);

		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_FinishSeqOtis, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).characterPosition.location = 1;
			startCycOtis(kCharacterAnna, "112B");
			releaseView(kCharacterAnna, kCarRestaurant, 57);
			send(kCharacterAnna, kCharacterWaiter2, 219377792, 0);
			break;
		case 2:
			send(kCharacterAnna, kCharacterAugust, 122288808, 0);
			CONS_Anna_ReturnComp3(0, 0, 0, 0);
			break;
		case 3:
			startCycOtis(kCharacterAnna, "112D");

			if (_gameTime >= 2047500) {
				getCharacterCurrentParams(kCharacterAnna)[0] = 1;
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Ann3142A", 0, 0, 0);
			}

			break;
		case 4:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
			AnnaCall(&LogicManager::CONS_Anna_DoWait, 1800, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 6;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Aug3007", 0, 0, 0);
			break;
		case 6:
			getCharacterCurrentParams(kCharacterAnna)[0] = 1;
			break;
		default:
			break;
		}

		break;
	case 101169422:
		if (_doneNIS[kEventKronosVisit]) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoBlockSeqOtis, "112J", 5, 0x39, 0);
		} else {
			if (_gameTime >= 2047500) {
				getCharacterCurrentParams(kCharacterAnna)[0] = 1;
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Ann3142A", 0, 0, 0);
			}
		}

		break;
	case 122288808:
		startCycOtis(kCharacterAnna, "112D");
		send(kCharacterAnna, kCharacterKronos, 157159392, 0);
		break;
	case 122358304:
		startCycOtis(kCharacterAnna, "BLANK");
		break;
	
	default:
		break;
	}
}

void LogicManager::CONS_Anna_ReturnComp3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_ReturnComp3);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 52;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_ReturnComp3(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterAnna, 37);
		getCharacter(kCharacterAnna).characterPosition.position = 4070;
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		endGraphics(kCharacterAnna);
		CONS_Anna_Dressing(0, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoWalk, 4, 4070, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			startSeqOtis(kCharacterAnna, "688Af");
			blockAtDoor(kCharacterAnna, 37);
			getCharacter(kCharacterAnna).characterPosition.location = 1;
			if (inComp(kCharacterCath, kCarRedSleeping, 4070) || inComp(kCharacterCath, kCarRedSleeping, 4455)) {
				if (isNight()) {
					playNIS(kEventCathTurningNight);
				} else {
					playNIS(kEventCathTurningDay);
				}

				playDialog(0, "BUMP", -1, 0);
				bumpCathRDoor(37);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_Dressing(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_Dressing);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 53;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_Dressing(HAND_PARAMS) {
	bool tmp;

	switch (msg->action) {
	case 0:
		if (_globals[kGlobalMaxHasToStayInBaggage] != 0 && getCharacterCurrentParams(kCharacterAnna)[4] != 0x7FFFFFFF && _gameTime != 0) {
			if (_gameTime > 2065500) {
				getCharacterCurrentParams(kCharacterAnna)[4] = 0x7FFFFFFF;
				CONS_Anna_GiveMaxToCond2(0, 0, 0, 0);
				break;
			}

			if (!cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterAnna)[4]) {
				getCharacterCurrentParams(kCharacterAnna)[4] = _gameTime + 150;
				if (_gameTime == -150) {
					CONS_Anna_GiveMaxToCond2(0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterAnna)[4] < _gameTime) {
				getCharacterCurrentParams(kCharacterAnna)[4] = 0x7FFFFFFF;
				CONS_Anna_GiveMaxToCond2(0, 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterAnna)[2] != 0) {
			bool skip = false; // Horrible way to unroll a goto...

			if (!getCharacterCurrentParams(kCharacterAnna)[5]) {
				getCharacterCurrentParams(kCharacterAnna)[5] = _gameTime + 9000;
				if (_gameTime == -9000) {
					skip = true;
					tmp = getCharacterCurrentParams(kCharacterAnna)[3] == 0;

					getCharacterCurrentParams(kCharacterAnna)[3] = tmp ? 1 : 0;

					if (!tmp) {
						startCycOtis(kCharacterAnna, "417B");
					} else {
						startCycOtis(kCharacterAnna, "417A");
					}

					getCharacterCurrentParams(kCharacterAnna)[5] = 0;
				}
			}

			if (!skip && getCharacterCurrentParams(kCharacterAnna)[5] < _gameTime) {
				getCharacterCurrentParams(kCharacterAnna)[5] = 0x7FFFFFFF;
				tmp = getCharacterCurrentParams(kCharacterAnna)[3] == 0;

				getCharacterCurrentParams(kCharacterAnna)[3] = tmp ? 1 : 0;

				if (!tmp) {
					startCycOtis(kCharacterAnna, "417B");
				} else {
					startCycOtis(kCharacterAnna, "417A");
				}

				getCharacterCurrentParams(kCharacterAnna)[5] = 0;
			}
		}

		if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			if (getCharacterCurrentParams(kCharacterAnna)[6] || (getCharacterCurrentParams(kCharacterAnna)[6] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterAnna)[6] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAnna)[6] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
			getCharacterCurrentParams(kCharacterAnna)[1] = 1;

			setDoor(37, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
			setDoor(53, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
			getCharacterCurrentParams(kCharacterAnna)[6] = 0;
		} else {
			getCharacterCurrentParams(kCharacterAnna)[6] = 0;
		}

		break;
	case 8:
	case 9:
		if (msg->action == 9 && inComp(kCharacterMax, kCarRedSleeping, 4070)) {
			setDoor(37, kCharacterAnna, 1, 0, 0);
			setDoor(53, kCharacterAnna, 1, 0, 0);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB013", 0, 0, 0);
		} else {
			if (getCharacterCurrentParams(kCharacterAnna)[0]) {
				setDoor(37, kCharacterAnna, 1, 0, 0);
				setDoor(53, kCharacterAnna, 1, 0, 0);

				if (msg->param.intParam == 53) {
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 6;
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathWCDialog(), 0, 0, 0);
				} else if (cathHasItem(kItemPassengerList)) {
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 7;

					if (rnd(2)) {
						AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathSorryDialog(), 0, 0, 0);
					} else {
						if (rnd(2) == 0) {
							AnnaCall(&LogicManager::CONS_Anna_DoDialog, "CAT1506A", 0, 0, 0);
						} else {
							AnnaCall(&LogicManager::CONS_Anna_DoDialog, "CAT1506", 0, 0, 0);
						}
					}
						
				} else {
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 8;
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathSorryDialog(), 0, 0, 0);
				}
			} else {
				setDoor(37, kCharacterAnna, 1, 0, 0);
				setDoor(53, kCharacterAnna, 1, 0, 0);
				if (msg->action == 8) {
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB012", 0, 0, 0);
				} else {
					getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB013", 0, 0, 0);
				}
			}
		}

		break;
	case 12:
		send(kCharacterAnna, kCharacterMax, 101687594, 0);
		setDoor(37, kCharacterAnna, 1, 10, 9);
		setDoor(53, kCharacterAnna, 1, 10, 9);
		getCharacter(kCharacterAnna).clothes = 2;
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterAnna)[1] || getCharacterCurrentParams(kCharacterAnna)[0]) {
			setDoor(37, kCharacterAnna, 1, 10, 9);
			setDoor(53, kCharacterAnna, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAnna)[1] = 0;
			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
		}

		if (!getCharacterCurrentParams(kCharacterAnna)[2] && (checkCathDir(kCarRedSleeping, 60) || _gameTime > 0x1F0950)) {
			getCharacterCurrentParams(kCharacterAnna)[2] = 1;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 9;
			AnnaCall(&LogicManager::CONS_Anna_DoSeqOtis, "416", 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			if (whoRunningDialog(kCharacterMax)) {
				setDoor(37, kCharacterAnna, 1, 10, 9);
				setDoor(53, kCharacterAnna, 1, 10, 9);
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "MAX1120", 0, 0, 0);
			}

			break;
		case 2:
			setDoor(37, kCharacterAnna, 1, 10, 9);
			setDoor(53, kCharacterAnna, 1, 10, 9);
			break;
		case 3:
		case 4:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "ANN1016", 0, 0, 0);
			break;
		case 5:
			setDoor(37, kCharacterAnna, 1, 14, 0);
			setDoor(53, kCharacterAnna, 1, 14, 0);
			getCharacterCurrentParams(kCharacterAnna)[0] = 1;
			break;
		case 6:
		case 7:
		case 8:
			if (inComp(kCharacterMax, kCarRedSleeping, 4070)) {
				setDoor(37, kCharacterAnna, 1, 0, 9);
				setDoor(53, kCharacterAnna, 1, 0, 9);
			}

			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
			getCharacterCurrentParams(kCharacterAnna)[1] = 1;
			break;
		case 9:
			if (checkCathDir(kCarRedSleeping, 60))
				bumpCath(kCarRedSleeping, 78, 255);

			startCycOtis(kCharacterAnna, "417B");
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_GiveMaxToCond2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_GiveMaxToCond2);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 54;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_GiveMaxToCond2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[2] != 0) {
			if (_gameTime > 2079000 && !getCharacterCurrentParams(kCharacterAnna)[4]) {
				getCharacterCurrentParams(kCharacterAnna)[4] = 1;
				CONS_Anna_GoConcert(0, 0, 0, 0);
				break;
			}

			if (getCharacterCurrentParams(kCharacterAnna)[5] || (getCharacterCurrentParams(kCharacterAnna)[5] = _gameTime + 9000, _gameTime != -9000)) {
				if (getCharacterCurrentParams(kCharacterAnna)[5] >= _gameTime) {
					if (getCharacterCurrentParams(kCharacterAnna)[0]) {
						if (getCharacterCurrentParams(kCharacterAnna)[6] || (getCharacterCurrentParams(kCharacterAnna)[6] = _realTime + 75, _realTime != -75)) {
							if (getCharacterCurrentParams(kCharacterAnna)[6] >= _realTime)
								break;

							getCharacterCurrentParams(kCharacterAnna)[6] = 0x7FFFFFFF;
						}

						getCharacterCurrentParams(kCharacterAnna)[0] = 0;
						getCharacterCurrentParams(kCharacterAnna)[1] = 1;

						setDoor(37, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
						setDoor(53, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
						getCharacterCurrentParams(kCharacterAnna)[6] = 0;
					} else {
						getCharacterCurrentParams(kCharacterAnna)[6] = 0;
					}

					break;
				}

				getCharacterCurrentParams(kCharacterAnna)[5] = 0x7FFFFFFF;
			}

			bool tmp = getCharacterCurrentParams(kCharacterAnna)[3] == 0;
			getCharacterCurrentParams(kCharacterAnna)[3] = tmp ? 1 : 0;

			if (!tmp) {
				startCycOtis(kCharacterAnna, "417B");
			} else {
				startCycOtis(kCharacterAnna, "417A");
			}

			getCharacterCurrentParams(kCharacterAnna)[5] = 0;
		}

		if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			if (getCharacterCurrentParams(kCharacterAnna)[6] || (getCharacterCurrentParams(kCharacterAnna)[6] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterAnna)[6] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAnna)[6] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
			getCharacterCurrentParams(kCharacterAnna)[1] = 1;

			setDoor(37, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
			setDoor(53, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
			getCharacterCurrentParams(kCharacterAnna)[6] = 0;
		} else {
			getCharacterCurrentParams(kCharacterAnna)[6] = 0;
		}

		break;
	case 8:
	case 9:
		if (msg->action == 9 && inComp(kCharacterMax, kCarRedSleeping, 4070)) {
			setDoor(37, kCharacterAnna, 1, 0, 0);
			setDoor(53, kCharacterAnna, 1, 0, 0);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB013", 0, 0, 0);
		} else if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			setDoor(37, kCharacterAnna, 1, 0, 0);
			setDoor(53, kCharacterAnna, 1, 0, 0);
			if (msg->param.intParam == 53) {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 6;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathWCDialog(), 0, 0, 0);
			} else if (cathHasItem(kItemPassengerList)) {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 7;
				if (rnd(2)) {
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathSorryDialog(), 0, 0, 0);
				} else {
					if (rnd(2) == 0) {
						AnnaCall(&LogicManager::CONS_Anna_DoDialog, "CAT1506A", 0, 0, 0);
					} else {
						AnnaCall(&LogicManager::CONS_Anna_DoDialog, "CAT1506", 0, 0, 0);
					}
				}

			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 8;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else {
			setDoor(37, kCharacterAnna, 1, 0, 0);
			setDoor(53, kCharacterAnna, 1, 0, 0);
			if (msg->action == 8) {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		setDoor(45, kCharacterCath, 1, 255, 255);
		if (checkCathDir(kCarRedSleeping, 60))
			bumpCath(kCarRedSleeping, 78, 255);

		send(kCharacterAnna, kCharacterCond2, 189750912, 0);
		break;

	case 17:
		if (getCharacterCurrentParams(kCharacterAnna)[1] || getCharacterCurrentParams(kCharacterAnna)[0]) {
			setDoor(37, kCharacterAnna, 1, 10, 9);
			setDoor(53, kCharacterAnna, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAnna)[1] = 0;
			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			if (whoRunningDialog(kCharacterMax)) {
				setDoor(37, kCharacterAnna, 1, 10, 9);
				setDoor(53, kCharacterAnna, 1, 10, 9);
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "MAX1120", 0, 0, 0);
			}

			break;
		case 2:
			setDoor(37, kCharacterAnna, 1, 10, 9);
			setDoor(53, kCharacterAnna, 1, 10, 9);
			break;
		case 3:
		case 4:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "ANN1016", 0, 0, 0);
			break;
		case 5:
			setDoor(37, kCharacterAnna, 1, 14, 0);
			setDoor(53, kCharacterAnna, 1, 14, 0);
			getCharacterCurrentParams(kCharacterAnna)[0] = 1;
			break;
		case 6:
		case 7:
		case 8:
			if (inComp(kCharacterMax, kCarRedSleeping, 4070)) {
				setDoor(37, kCharacterAnna, 1, 0, 9);
				setDoor(53, kCharacterAnna, 1, 0, 9);
			}

			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
			getCharacterCurrentParams(kCharacterAnna)[1] = 1;
			break;
		case 9:
			softReleaseAtDoor(kCharacterAnna, 37);
			endGraphics(kCharacterAnna);
			getCharacter(kCharacterAnna).characterPosition.location = kCharacterAnna;
			getCharacter(kCharacterAnna).characterPosition.position = 4070;
			getCharacterCurrentParams(kCharacterAnna)[2] = kCharacterAnna;
			setDoor(45, kCharacterCath, 0, 255, 255);
			setDoor(53, kCharacterAnna, kCharacterAnna, 10, 9);

			if (checkCathDir(kCarRedSleeping, 78))
				bumpCath(kCarRedSleeping, 49, 255);

			startCycOtis(kCharacterAnna, "417B");
			break;
		default:
			break;
		}

		break;
	case 123733488:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 9;
		AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "629Ef", 37, 0, 0);
		break;
	case 156049968:
		startCycOtis(kCharacterAnna, "629DF");
		softBlockAtDoor(kCharacterAnna, 37);
		break;
	case 253868128:
		setDoor(53, kCharacterAnna, 1, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_GoConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_GoConcert);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 55;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_GoConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(45, kCharacterCath, 2, 255, 255);
		if (checkCathDir(kCarRedSleeping, 78))
			bumpCath(kCarRedSleeping, 49, 255);

		setDoor(37, kCharacterCath, 1, 0, 0);
		setDoor(53, kCharacterCath, 1, 10, 9);
		dropItem(kItemKey, 1);
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_ExitComp, 1, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			setDoor(37, kCharacterCath, 1, 10, 9);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoWalk, 2, 9270, 0, 0);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			CONS_Anna_Concert(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_Concert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_Concert);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 56;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_Concert(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterAnna);
		getCharacter(kCharacterAnna).characterPosition.car = kCarKronos;
		getCharacter(kCharacterAnna).characterPosition.position = 6000;
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		break;
	case 191668032:
		COND_Anna_LeaveConcert(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::COND_Anna_LeaveConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_LeaveConcert);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 57;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_LeaveConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAnna).characterPosition.position = 850;
		getCharacter(kCharacterAnna).characterPosition.location = 0;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoWalk, 3, 5790, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			send(kCharacterAnna, kCharacterAugust, 191668032, 0);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoWalk, 4, 4070, 0, 0);
			break;
		case 2:
			setDoor(37, kCharacterAlexei, 1, 0, 0);
			send(kCharacterAnna, kCharacterCond2, 205033696, 0);
			startCycOtis(kCharacterAnna, "625Ef");
			softBlockAtDoor(kCharacterAnna, 37);
			break;
		case 3:
			startCycOtis(kCharacterAnna, "625Gf");
			softBlockAtDoor(kCharacterAnna, 37);
			send(kCharacterAnna, kCharacterAugust, 169032608, 0);
			break;
		case 4:
			if (whoRunningDialog(kCharacterAugust)) {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
				AnnaCall(&LogicManager::CONS_Anna_DoWait, 75, 0, 0, 0);
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Aug3009", 0, 0, 0);
			}

			break;
		case 5:
			playDialog(kCharacterAnna, "Aug3009A", -1, 0);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 6;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "628Bf", 37, 0, 0);
			break;
		case 6:
			softReleaseAtDoor(kCharacterAnna, 37);
			send(kCharacterAnna, kCharacterAugust, 122288808, 0);
			CONS_Anna_AfterConcert(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 123712592:
		startCycOtis(kCharacterAnna, "628Af");

		if (whoRunningDialog(kCharacterAugust)) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
			AnnaCall(&LogicManager::CONS_Anna_DoWait, 75, 0, 0, 0);
		} else {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Aug3009", 0, 0, 0);
		}

		break;
	case 192063264:
		if (inComp(kCharacterCath, kCarRedSleeping, 4070) || inComp(kCharacterCath, kCarRedSleeping, 4455)) {
			softReleaseAtDoor(kCharacterAnna, 37);
			CONS_Anna_LeaveConcertCathInComp(0, 0, 0, 0);
		} else {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "625Ff", 37, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_LeaveConcertCathInComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_LeaveConcertCathInComp);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 58;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_LeaveConcertCathInComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaSearchingCompartment, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			playNIS(kEventAnnaSearchingCompartment);
			endGraphics(kCharacterAnna);
			bumpCath(kCarRedSleeping, 8, 255);
			playDialog(kCharacterAnna, "lib015", -1, 0);
			send(kCharacterAnna, kCharacterAugust, 122288808, 0);
			CONS_Anna_AfterConcert(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_AfterConcert(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_AfterConcert);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 59;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_AfterConcert(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (cathHasItem(kItemKey) && getCharacterCurrentParams(kCharacterAnna)[3] != 0x7FFFFFFF && _gameTime > 2218500) {
			if (_gameTime > 2248200) {
				getCharacterCurrentParams(kCharacterAnna)[3] = 0x7FFFFFFF;
				CONS_Anna_GoBagg(0, 0, 0, 0);
				break;
			}

			if (!getCharacterCurrentParams(kCharacterAnna)[2] || (!cathInCorridor(kCarRedSleeping) && !inSalon(kCharacterCath) && !inDiningRoom(kCharacterCath)) || !getCharacterCurrentParams(kCharacterAnna)[3]) {
				getCharacterCurrentParams(kCharacterAnna)[3] = _gameTime;
				if (!getCharacterCurrentParams(kCharacterAnna)[3]) {
					CONS_Anna_GoBagg(0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterAnna)[3] < _gameTime) {
				getCharacterCurrentParams(kCharacterAnna)[3] = 0x7FFFFFFF;
				CONS_Anna_GoBagg(0, 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			if (getCharacterCurrentParams(kCharacterAnna)[4] || (getCharacterCurrentParams(kCharacterAnna)[4] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterAnna)[4] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAnna)[4] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
			getCharacterCurrentParams(kCharacterAnna)[1] = 1;

			setDoor(37, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
			setDoor(53, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
			getCharacterCurrentParams(kCharacterAnna)[4] = 0;
		} else {
			getCharacterCurrentParams(kCharacterAnna)[4] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			setDoor(37, kCharacterAnna, 1, 0, 0);
			setDoor(53, kCharacterAnna, 1, 0, 0);

			if (msg->param.intParam == 53) {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathWCDialog(), 0, 0, 0);
			} else if (cathHasItem(kItemPassengerList)) {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;

				if (rnd(2)) {
					AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathSorryDialog(), 0, 0, 0);
				} else {
					if (rnd(2) == 0) {
						AnnaCall(&LogicManager::CONS_Anna_DoDialog, "CAT1506A", 0, 0, 0);
					} else {
						AnnaCall(&LogicManager::CONS_Anna_DoDialog, "CAT1506", 0, 0, 0);
					}
				}
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 6;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else {
			setDoor(37, kCharacterAnna, 1, 0, 0);
			setDoor(53, kCharacterAnna, 1, 0, 0);
			if (msg->action == 8) {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		getCharacter(kCharacterAnna).characterPosition.position = 4070;
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		endGraphics(kCharacterAnna);
		setDoor(107, kCharacterCath, 2, 255, 255);
		setDoor(45, kCharacterCath, 1, 255, 255);
		setDoor(37, kCharacterAnna, 1, 10, 9);
		setDoor(53, kCharacterAnna, 1, 10, 9);

		if (checkCathDir(kCarRedSleeping, 60))
			bumpCath(kCarRedSleeping, 78, 255);

		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterAnna)[1] || getCharacterCurrentParams(kCharacterAnna)[0]) {
			setDoor(37, kCharacterAnna, 1, 10, 9);
			setDoor(53, kCharacterAnna, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAnna)[1] = 0;
			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
		case 2:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "ANN1016", 0, 0, 0);
			break;
		case 3:
			setDoor(37, kCharacterAnna, 1, 14, 0);
			setDoor(53, kCharacterAnna, 1, 14, 0);
			getCharacterCurrentParams(kCharacterAnna)[0] = 1;
			break;
		case 4:
		case 5:
		case 6:
			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
			getCharacterCurrentParams(kCharacterAnna)[1] = 1;
			break;
		case 7:
			send(kCharacterAnna, kCharacterTatiana, 100906246, 0);
			break;
		default:
			break;
		}

		break;
	case 156622016:
		if (getCharacterCurrentParams(kCharacterAnna)[2]) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 8;
			AnnaCall(&LogicManager::CONS_Anna_GiveMaxBack, 0, 0, 0, 0);
		}

		break;
	case 236241630:
		setDoor(37, kCharacterAnna, 1, 0, 0);
		setDoor(53, kCharacterAnna, 1, 0, 0);
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 7;
		AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Ann1016A", 0, 0, 0);
		break;
	case 236517970:
		getCharacterCurrentParams(kCharacterAnna)[2] = 1;
		setDoor(37, kCharacterAnna, 1, 10, 9);
		setDoor(53, kCharacterAnna, 1, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_GiveMaxBack(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_GiveMaxBack);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 60;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_GiveMaxBack(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		send(kCharacterAnna, kCharacterMax, 122358304, 0);

		if (rnd(2) == 0) {
			playDialog(kCharacterAnna, "Ann3127", -1, 0);
		} else {
			playDialog(kCharacterAnna, "Ann3126", -1, 0);
		}

		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "630Cf", 37, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "630Df", 37, 0, 0);
			break;
		case 2:
			endGraphics(kCharacterAnna);
			send(kCharacterAnna, kCharacterCond2, 189026624, 0);
			break;
		case 3:
			getCharacter(kCharacterAnna).characterPosition.position = 4070;
			getCharacter(kCharacterAnna).characterPosition.location = 1;
			endGraphics(kCharacterAnna);

			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 156049968:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
		AnnaCall(&LogicManager::CONS_Anna_DoCorrOtis, "629EF", 37, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_GoBagg(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_GoBagg);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 61;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_GoBagg(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		_timeSpeed = 3;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 1, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			setDoor(53, kCharacterCath, 1, 10, 9);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_ExitComp, 0, 0, 0, 0);
			break;
		case 2:
			setDoor(37, kCharacterCath, 1, 10, 9);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoWalk, 5, 850, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
			AnnaCall(&LogicManager::CONS_Anna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAnna).characterPosition.position = 1540;
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
			AnnaCall(&LogicManager::CONS_Anna_DoSeqOtis, "802US", 0, 0, 0);
			break;
		case 5:
			startSeqOtis(kCharacterAnna, "802UD");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterAnna);

			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 6;
			AnnaCall(&LogicManager::CONS_Anna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 6:
			endGraphics(kCharacterAnna);
			CONS_Anna_InBagg(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_InBagg(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_InBagg);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 62;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_InBagg(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[0] && _gameTime > 2259000 && !getCharacterCurrentParams(kCharacterAnna)[1]) {
			getCharacterCurrentParams(kCharacterAnna)[1] = 1;
			send(kCharacterAnna, kCharacterVesna, 189299008, 0);
			CONS_Anna_DeadBagg(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterAnna).characterPosition.car = kCarBaggage;
		_globals[kGlobalAnnaIsInBaggageCar] = 1;
		break;
	case 235856512:
		getCharacterCurrentParams(kCharacterAnna)[0] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DeadBagg(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DeadBagg);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 63;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DeadBagg(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		send(kCharacterAnna, kCharacterMaster, 171843264, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			playNIS(kEventAnnaKilled);
			endGame(1, 2250000, 58, true);
		}

		break;
	case 272177921:
		if (dialogRunning("MUS012"))
			fadeDialog("MUS012");

		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaKilled, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_BaggageFight(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_BaggageFight);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 64;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_BaggageFight(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterAnna);
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaBaggageArgument, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			playNIS(kEventAnnaBaggageArgument);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 1, 0, 0, 0);
			break;
		case 2:
			getCharacterCurrentParams(kCharacterAnna)[0] = playFight(2002);
			if (getCharacterCurrentParams(kCharacterAnna)[0]) {
				endGame(0, 0, 0, getCharacterCurrentParams(kCharacterAnna)[0] == 1);
			} else {
				_gameTime += 1800;
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
				AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaBaggagePart2, 0, 0);
			}
			break;
		case 3:
			playNIS(kEventAnnaBaggagePart2);
			bumpCath(kCarBaggage, 96, 255);
			_globals[kGlobalAnnaIsInBaggageCar] = 0;
			forceJump(kCharacterVesna, &LogicManager::CONS_Vesna_InComp);
			_gameTime = 2266200;
			CONS_Anna_PrepareVienna(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_PrepareVienna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_PrepareVienna);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 65;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_PrepareVienna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterAnna).characterPosition.position = 4070;
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		getCharacter(kCharacterAnna).clothes = 3;
		getCharacter(kCharacterAnna).inventoryItem = 0;
		setDoor(45, kCharacterCath, 1, 255, 255);
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_CompLogic, 15803100, "NONE", 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_StartPart4);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 66;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Anna_Reading(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAnna);
		getCharacter(kCharacterAnna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterAnna).characterPosition.position = 4070;
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		getCharacter(kCharacterAnna).clothes = 2;
		getCharacter(kCharacterAnna).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_Reading(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_Reading);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 67;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_Reading(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (checkCathDir(kCarRedSleeping, 46)) {
			if (getCharacterCurrentParams(kCharacterAnna)[3] || (getCharacterCurrentParams(kCharacterAnna)[3] = _realTime + 30, _realTime != -30)) {
				if (getCharacterCurrentParams(kCharacterAnna)[3] >= _realTime) {
					if (getCharacterCurrentParams(kCharacterAnna)[0]) {
						if (getCharacterCurrentParams(kCharacterAnna)[4] || (getCharacterCurrentParams(kCharacterAnna)[4] = _realTime + 75, _realTime != -75)) {
							if (getCharacterCurrentParams(kCharacterAnna)[4] >= _realTime)
								break;

							getCharacterCurrentParams(kCharacterAnna)[4] = 0x7FFFFFFF;
						}

						getCharacterCurrentParams(kCharacterAnna)[0] = 0;
						getCharacterCurrentParams(kCharacterAnna)[1] = 1;

						setDoor(53, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
						getCharacterCurrentParams(kCharacterAnna)[4] = 0;
					} else {
						getCharacterCurrentParams(kCharacterAnna)[4] = 0;
					}

					break;
				}

				getCharacterCurrentParams(kCharacterAnna)[3] = 0x7FFFFFFF;
			}

			bumpCath(kCarRedSleeping, 8, 255);
		}

		getCharacterCurrentParams(kCharacterAnna)[3] = 0;

		if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			if (getCharacterCurrentParams(kCharacterAnna)[4] || (getCharacterCurrentParams(kCharacterAnna)[4] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterAnna)[4] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterAnna)[4] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
			getCharacterCurrentParams(kCharacterAnna)[1] = 1;

			setDoor(53, kCharacterAnna, 1, 0, !inComp(kCharacterMax, kCarRedSleeping, 4070) ? 0 : 9);
			getCharacterCurrentParams(kCharacterAnna)[4] = 0;
		} else {
			getCharacterCurrentParams(kCharacterAnna)[4] = 0;
		}

		break;
	case 1:
		getCharacter(kCharacterAnna).inventoryItem = 0;
		getCharacter(kCharacterCath).characterPosition.location = 1;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, 34, 0, 0);
		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			setDoor(53, kCharacterAnna, 1, 0, 0);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, getCathWCDialog(), 0, 0, 0);
		} else {
			setDoor(53, kCharacterAnna, 1, 0, 0);

			if (msg->action == 8) {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
				AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		setDoor(37, kCharacterCath, 2, 0, 0);
		setDoor(45, kCharacterCath, 1, 255, 255);
		setDoor(53, kCharacterAnna, 1, 10, 9);
		startCycOtis(kCharacterAnna, "511B");
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterAnna)[1] || getCharacterCurrentParams(kCharacterAnna)[0]) {
			setDoor(53, kCharacterAnna, 1, 10, 9);
			getCharacterCurrentParams(kCharacterAnna)[1] = 0;
			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			playNIS(kEventAnnaConversation_34);
			playDialog(0, "LIB015", -1, 0);
			bumpCath(kCarRedSleeping, 8, 255);
			CONS_Anna_Sulking(0, 0, 0, 0);
			break;
		case 2:
		case 3:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "ANN1016", 0, 0, 0);
			break;
		case 4:
			setDoor(53, kCharacterAnna, 1, 14, 0);
			getCharacterCurrentParams(kCharacterAnna)[0] = 1;
			break;
		case 5:
			getCharacterCurrentParams(kCharacterAnna)[0] = 0;
			getCharacterCurrentParams(kCharacterAnna)[1] = 1;
			break;
		default:
			break;
		}

		break;
	case 191001984:
		setDoor(37, kCharacterCath, 0, 10, 9);
		getCharacter(kCharacterAnna).inventoryItem = 0;
		CONS_Anna_GoSalon4(0, 0, 0, 0);
		break;
	case 219971920:
		getCharacterCurrentParams(kCharacterAnna)[2] = 1;
		getCharacter(kCharacterAnna).inventoryItem = 0x80;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_Sulking(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_Sulking);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 68;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_Sulking(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterAnna)[0]) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_CompLogic, 2511900, "NONE", 0, 0);
		}

		break;
	case 12:
		setDoor(37, kCharacterCath, 1, 10, 9);
		setDoor(53, kCharacterCath, 1, 10, 9);
		getCharacter(kCharacterAnna).characterPosition.car = 4;
		getCharacter(kCharacterAnna).characterPosition.position = 4070;
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		break;
	case 191001984:
		setDoor(37, kCharacterCath, 0, 10, 9);
		CONS_Anna_GoSalon4(0, 0, 0, 0);
		break;
	case 201431954:
		getCharacterCurrentParams(kCharacterAnna)[0] = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_GoSalon4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_GoSalon4);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 69;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_GoSalon4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[0]) {
			if (getCharacterCurrentParams(kCharacterAnna)[1] || (getCharacterCurrentParams(kCharacterAnna)[1] = _gameTime + 4500, _gameTime != -4500)) {
				if (getCharacterCurrentParams(kCharacterAnna)[1] >= _gameTime)
					break;

				getCharacterCurrentParams(kCharacterAnna)[1] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterAnna).characterPosition.car = kCarRedSleeping;
			getCharacter(kCharacterAnna).characterPosition.position = 9270;
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			CONS_Anna_ReturnToComp4(0, 0, 0, 0);
		} else if (_gameTime > 2535300 && !getCharacterCurrentParams(kCharacterAnna)[2]) {
			getCharacterCurrentParams(kCharacterAnna)[2] = 1;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
			AnnaCall(&LogicManager::CONS_Anna_WaitRCClear, 0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterAnna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterAnna).characterPosition.position = 4070;
		getCharacter(kCharacterAnna).characterPosition.location = 0;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoWalk, 5, 850, 0, 0);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterAnna)[0] && checkLoc(kCharacterCath, kCarRedSleeping)) {
			getCharacter(kCharacterAnna).characterPosition.car = kCarRedSleeping;
			getCharacter(kCharacterAnna).characterPosition.position = 8200;
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			CONS_Anna_ReturnToComp4(0, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAnna).characterPosition.position = 1540;
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoBlockSeqOtis, "127A", 5, 56, 0);
			break;
		case 3:
			getCharacter(kCharacterAnna).characterPosition.location = 1;
			startCycOtis(kCharacterAnna, "127B");
			send(kCharacterAnna, kCharacterWaiter2, 258136010, 0);
			break;
		case 4:
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
			AnnaCall(&LogicManager::CONS_Anna_DoBlockSeqOtis, "127G", 5, 56, 0);
			break;
		case 5:
			CONS_Anna_ReturnToComp4(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 100969180:
		endGraphics(kCharacterAnna);
		getCharacterCurrentParams(kCharacterAnna)[0] = 1;
		break;
	case 122288808:
		startCycOtis(kCharacterAnna, "127E");
		send(kCharacterAnna, kCharacterAbbot, 203073664, 0);
		break;
	case 122358304:
		startCycOtis(kCharacterAnna, "BLANK");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_ReturnToComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_ReturnToComp4);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 70;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_ReturnToComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_DoWalkCathFollowsAnna, 4, 4070, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_EnterCompCathFollowsAnna, 0, 0, 0, 0);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			endGraphics(kCharacterAnna);
			CONS_Anna_LetDownHair(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_EnterCompCathFollowsAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_EnterCompCathFollowsAnna);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 71;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_EnterCompCathFollowsAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterAnna, 37);
		getCharacter(kCharacterAnna).characterPosition.position = 4070;

		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAnna, "625Af");
		if (checkCathDir(kCarRedSleeping, 7) || checkCathDir(kCarRedSleeping, 28) || checkCathDir(kCarRedSleeping, 56)) {
			smartBumpCath();
		}

		blockAtDoor(kCharacterAnna, 37);
		getCharacter(kCharacterAnna).characterPosition.location = 1;

		if (inComp(kCharacterCath, kCarRedSleeping, 4070) || inComp(kCharacterCath, kCarRedSleeping, 4455)) {
			if (isNight()) {
				playNIS(kEventCathTurningNight);
			} else {
				playNIS(kEventCathTurningDay);
			}

			playDialog(0, "BUMP", -1, 0);
			bumpCathFDoor(37);
		}

		break;
	case 17:
		if (!_doneNIS[kEventAnnaTiredKiss] && nearChar(kCharacterCath, kCharacterAnna, 2000) && whoOnScreen(kCharacterAnna) &&
			getCharacter(kCharacterAnna).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaTiredKiss, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			playNIS(kEventAnnaTiredKiss);
			bumpCath(kCarRestaurant, 29, 255);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_DoWalkCathFollowsAnna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_DoWalkCathFollowsAnna);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 72;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_DoWalkCathFollowsAnna(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_doneNIS[kEventAnnaTired] || whoFacingCath(kCharacterAnna)) {
			getCharacter(kCharacterAnna).inventoryItem = 0;
		} else {
			getCharacter(kCharacterAnna).inventoryItem = 0x80;
		}

		if (walk(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[0], getCharacterCurrentParams(kCharacterAnna)[1])) {
			getCharacter(kCharacterAnna).inventoryItem = 0;

			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		}

		break;
	case 1:
		getCharacter(kCharacterAnna).inventoryItem = 0;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaTired, 0, 0);
		break;
	case 12:
		if (walk(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[0], getCharacterCurrentParams(kCharacterAnna)[1])) {
			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		} else if (!_doneNIS[kEventAnnaTired]) {
			getCharacter(kCharacterAnna).inventoryItem = 0x80;
		}

		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			playNIS(kEventAnnaTired);
			if (getCharacter(kCharacterAnna).direction == 1)
				bumpCathFx(getCharacter(kCharacterAnna).characterPosition.car, getCharacter(kCharacterAnna).characterPosition.position - 750);
			else
				bumpCathRx(getCharacter(kCharacterAnna).characterPosition.car, getCharacter(kCharacterAnna).characterPosition.position + 750);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_LetDownHair(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_LetDownHair);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 73;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_LetDownHair(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[2] != 0x7FFFFFFF && getCharacterCurrentParams(kCharacterAnna)[0] < _gameTime) {
			if (getCharacterCurrentParams(kCharacterAnna)[1] < _gameTime) {
				getCharacterCurrentParams(kCharacterAnna)[2] = 0x7FFFFFFF;
			} else if (((cathInCorridor(kCarGreenSleeping) || cathInCorridor(kCarRedSleeping)) && getCharacterCurrentParams(kCharacterAnna)[2]) || (getCharacterCurrentParams(kCharacterAnna)[2] = _gameTime, _gameTime != 0)) {
				if (getCharacterCurrentParams(kCharacterAnna)[2] >= _gameTime)
					break;
	
				getCharacterCurrentParams(kCharacterAnna)[2] = 0x7FFFFFFF;
			}

			if (!cathInCorridor(kCarGreenSleeping) && !cathInCorridor(kCarRedSleeping))
				playDialog(0, "BUMP", -1, 0);

			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventTrainHijacked, 0, 0);
		}

		break;
	case 8:
		setDoor(37, kCharacterAnna, 0, 0, 0);
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
		AnnaCall(&LogicManager::CONS_Anna_DoDialog, "LIB012", 0, 0, 0);
		break;
	case 9:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaKissTrainHijacked, 0, 0);
		break;
	case 12:
		setDoor(37, kCharacterAnna, 0, 10, 9);
		_timeSpeed = 1;
		getCharacterCurrentParams(kCharacterAnna)[0] = _gameTime + 4500;
		getCharacterCurrentParams(kCharacterAnna)[1] = _gameTime + 9000;
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			playNIS(kEventTrainHijacked);
			send(kCharacterAnna, kCharacterMaster, 139254416, 0);
			break;
		case 2:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_DoDialog, "Ann4200", 0, 0, 0);
			break;
		case 3:
			setDoor(37, kCharacterAnna, 0, 10, 9);
			break;
		case 4:
			playNIS(kEventAnnaKissTrainHijacked);
			send(kCharacterAnna, kCharacterMaster, 139254416, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_StartPart5);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 74;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Anna_TiedUp(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAnna);
		getCharacter(kCharacterAnna).characterPosition.car = 1;
		getCharacter(kCharacterAnna).characterPosition.position = 3969;
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		getCharacter(kCharacterAnna).clothes = 3;
		getCharacter(kCharacterAnna).inventoryItem = 0;
		setDoor(45, kCharacterCath, 0, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_TiedUp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_TiedUp);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 75;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_TiedUp(HAND_PARAMS) {
	switch (msg->action) {
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			if (_globals[kGlobalReadLetterInAugustSuitcase]) {
				if (_doneNIS[kEventAnnaKissTrainHijacked]) {
					playNIS(kEventAnnaBaggageTies2);
				} else {
					playNIS(kEventAnnaBaggageTies);
				}
			} else if (_doneNIS[kEventAnnaKissTrainHijacked]) {
				playNIS(kEventAnnaBaggageTies3);
			} else {
				playNIS(kEventAnnaBaggageTies4);
			}

			bumpCath(kCarBaggageRear, 88, 255);
			CONS_Anna_Outside(0, 0, 0, 0);
		}

		break;
	case 272177921:
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaBaggageTies, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_Outside(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_Outside);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 76;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_Outside(HAND_PARAMS) {
	switch (msg->action) {
	case 158480160:
		CONS_Anna_ReadyToScore(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_ReadyToScore(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_ReadyToScore);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 77;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_ReadyToScore(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 3645000 && !getCharacterCurrentParams(kCharacterAnna)[1]) {
			getCharacterCurrentParams(kCharacterAnna)[1] = 1;
			_timeSpeed = 0;
		}

		break;
	case 8:
	case 9:
		if (msg->action == 8) {
			playDialog(0, "LIB012", -1, 0);
		} else {
			playDialog(0, "LIB014", -1, 0);
		}

		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventAnnaDialogGoToJerusalem, 0, 0);
		break;
	case 12:
		setDoor(106, kCharacterAnna, 1, 10, 9);
		break;
	case 17:
		if (!getCharacterCurrentParams(kCharacterAnna)[0] && checkLoc(kCharacterCath, kCarBaggage)) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 1, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			getCharacterCurrentParams(kCharacterAnna)[0] = 1;
			break;
		case 2:
			setDoor(106, kCharacterCath, 0, 10, 9);
			playNIS(kEventAnnaDialogGoToJerusalem);
			_gameTime = 4914000;
			_timeSpeed = 0;
			send(kCharacterAnna, kCharacterTatiana, 236060709, 0);
			bumpCath(kCarBaggage, 97, 1);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 1, 0, 0, 0);
			break;
		case 3:
			CONS_Anna_Kidnapped(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_Kidnapped(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_Kidnapped);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 78;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_Kidnapped(HAND_PARAMS) {
	switch (msg->action) {
	case 17:
		if (inDiningRoom(kCharacterCath) && cathHasItem(kItemFirebird)) {
			CONS_Anna_FinalSequence(0, 0, 0, 0);
			break;
		}

		if (!inSalon(kCharacterCath))
			break;

		if (cathHasItem(kItemFirebird)) {
			CONS_Anna_FinalSequence(0, 0, 0, 0);
			break;
		}

		_gameTime = 4920300;

		if (_items[kItemFirebird].floating == 4) {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventKronosHostageAnna, 0, 0);
		} else {
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventKronosHostageAnnaNoFirebird, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			playNIS(kEventKronosHostageAnnaNoFirebird);
			endGame(3, 42, 0, 1);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			playNIS(kEventKronosHostageAnna);
			bumpCath(kCarRestaurant, 61, 1);
			playDialog(kCharacterAnna, "Mus024", 16, 0);
			CONS_Anna_Waiting(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_Waiting(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_Waiting);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 79;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_Waiting(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		_gameTime = 4923000;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventKahinaPunchBaggageCarEntrance, 0, 0);
		break;
	case 17:
		if (inDiningRoom(kCharacterCath) && cathHasItem(kItemFirebird)) {
			CONS_Anna_FinalSequence(0, 0, 0, 0);
		} else if (inSalon(kCharacterCath) && !_doneNIS[kEventKahinaPunch]) {
			_gameTime = 4923000;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventKahinaPunch, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			if (inSalon(kCharacterCath)) {
				playNIS(kEventKahinaPunchSalon);
			} else if (inDiningRoom(kCharacterCath)) {
				playNIS(kEventKahinaPunchRestaurant);
			} else if (inKitchen(kCharacterCath)) {
				playNIS(kEventKahinaPunchKitchen);
			} else if (inOffice(kCharacterCath)) {
				playNIS(kEventKahinaPunchBaggageCarEntrance);
			} else if (checkLoc(kCharacterCath, kCarBaggage)) {
				playNIS(kEventKahinaPunchBaggageCar);
			}

			endGame(0, 1, 0, true);
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			playNIS(kEventKahinaPunch);
			endGame(0, 1, 0, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_FinalSequence(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_FinalSequence);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 80;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_FinalSequence(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[0] || (getCharacterCurrentParams(kCharacterAnna)[0] = _realTime + 450, _realTime != -450)) {
			if (getCharacterCurrentParams(kCharacterAnna)[0] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterAnna)[0] = 0x7FFFFFFF;
		}

		playDialog(0, "Kro5001", 16, 0);
		break;
	case 2:
		playDialog(0, "Kro5002", 16, 0);
		_gameTime = 4929300;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventKahinaPunch, 0, 0);
		break;
	case 12:
		_gameTime = 4923000;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventKronosBringFirebird, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			if (whoRunningDialog(kCharacterAnna))
				fadeDialog(kCharacterAnna);

			playNIS(kEventKronosBringFirebird);
			bumpCathCloseUp(kItemFirebird);
			playDialog(1, "Mus025", 16, 0);
			break;
		case 2:
			playNIS(kEventKahinaPunch);
			endGame(0, 1, 0, true);
			break;
		case 3:
			_globals[kGlobalEggIsOpen] = 1;

			if (whoRunningDialog(kCharacterAnna))
				fadeDialog(kCharacterAnna);

			playNIS(kEventKronosOpenFirebird);
			bumpCath(kCarRestaurant, 3, 255);
			CONS_Anna_OpenFirebird(0, 0, 0, 0);
			break;
		}

		break;
	case 205294778:
		_gameTime = 4929300;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventKronosOpenFirebird, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Anna_OpenFirebird(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_Anna_OpenFirebird);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 81;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_Anna_OpenFirebird(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[0] || (getCharacterCurrentParams(kCharacterAnna)[0] = _realTime + 180, _realTime != -180)) {
			if (getCharacterCurrentParams(kCharacterAnna)[0] >= _realTime)
				break;

			getCharacterCurrentParams(kCharacterAnna)[0] = 0x7FFFFFFF;
		}

		playDialog(kCharacterClerk, "LIB069", 16, 0);
		endGame(0, 0, 0, true);
		break;
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			playNIS(kEventCathCloseEggNoBackground);
			playNIS(kEventKronosGiveFirebird);
			if (cathHasItem(kItemWhistle)) {
				endGame(0, 1, 53, true);
			} else if (_items[kItemWhistle].floating == 1) {
				endGame(3, 208, 0, true);
			} else {
				endGame(3, 42, 54, true);
			}
		} else if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 2) {
			takeCathItem(kItemWhistle);
			winGame();
		}

		break;
	case 224309120:
		_globals[kGlobalEggIsOpen] = 0;
		_gameTime = 4941000;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventKronosGiveFirebird, 0, 0);
		break;
	case 270751616:
		_globals[kGlobalEggIsOpen] = 0;
		killGracePeriod();
		_gameTime = 4941000;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
		AnnaCall(&LogicManager::CONS_Anna_SaveGame, 2, kEventFinalSequence, 0, 0);
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsAnna[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Anna_DebugWalks,
	&LogicManager::HAND_Anna_DoSeqOtis,
	&LogicManager::HAND_Anna_DoBlockSeqOtis,
	&LogicManager::HAND_Anna_DoCorrOtis,
	&LogicManager::HAND_Anna_FinishSeqOtis,
	&LogicManager::HAND_Anna_DoJoinedSeqOtis,
	&LogicManager::HAND_Anna_DoDialog,
	&LogicManager::HAND_Anna_WaitRCClear,
	&LogicManager::HAND_Anna_SaveGame,
	&LogicManager::HAND_Anna_DoWalk,
	&LogicManager::HAND_Anna_DoWait,
	&LogicManager::HAND_Anna_PracticeMusic,
	&LogicManager::HAND_Anna_DoComplexSeqOtis,
	&LogicManager::HAND_Anna_DoWaitReal,
	&LogicManager::HAND_Anna_CompLogic,
	&LogicManager::HAND_Anna_Birth,
	&LogicManager::HAND_Anna_DoWalkP1,
	&LogicManager::HAND_Anna_DiningLogic,
	&LogicManager::HAND_Anna_FleeTyler,
	&LogicManager::HAND_Anna_WaitDinner,
	&LogicManager::HAND_Anna_GoDinner,
	&LogicManager::HAND_Anna_WaitHW,
	&LogicManager::HAND_Anna_WaitingDinner,
	&LogicManager::HAND_Anna_WaitingDinner2,
	&LogicManager::HAND_Anna_EatingDinner,
	&LogicManager::HAND_Anna_LeaveDinner,
	&LogicManager::HAND_Anna_FreshenUp,
	&LogicManager::HAND_Anna_GoSalon,
	&LogicManager::HAND_Anna_WaitAugust,
	&LogicManager::HAND_Anna_FlirtAugust,
	&LogicManager::HAND_Anna_LeaveAugust,
	&LogicManager::HAND_Anna_ReturnComp,
	&LogicManager::HAND_Anna_ReadyForBed,
	&LogicManager::HAND_Anna_Asleep,
	&LogicManager::HAND_Anna_WakeNight,
	&LogicManager::HAND_Anna_GoVassili,
	&LogicManager::HAND_Anna_AtSeizure,
	&LogicManager::HAND_Anna_SpeakTatiana,
	&LogicManager::HAND_Anna_DoWalk1019,
	&LogicManager::HAND_Anna_LeaveTatiana,
	&LogicManager::HAND_Anna_GoBackToSleep,
	&LogicManager::HAND_Anna_StartPart2,
	&LogicManager::HAND_Anna_InPart2,
	&LogicManager::HAND_Anna_StartPart3,
	&LogicManager::HAND_Anna_ExitComp,
	&LogicManager::HAND_Anna_Practicing,
	&LogicManager::HAND_Anna_GoLunch,
	&LogicManager::HAND_Anna_Lunch,
	&LogicManager::HAND_Anna_DoOtis5026J,
	&LogicManager::HAND_Anna_LeaveLunch,
	&LogicManager::HAND_Anna_AfterLunch,
	&LogicManager::HAND_Anna_ReturnComp3,
	&LogicManager::HAND_Anna_Dressing,
	&LogicManager::HAND_Anna_GiveMaxToCond2,
	&LogicManager::HAND_Anna_GoConcert,
	&LogicManager::HAND_Anna_Concert,
	&LogicManager::HAND_Anna_LeaveConcert,
	&LogicManager::HAND_Anna_LeaveConcertCathInComp,
	&LogicManager::HAND_Anna_AfterConcert,
	&LogicManager::HAND_Anna_GiveMaxBack,
	&LogicManager::HAND_Anna_GoBagg,
	&LogicManager::HAND_Anna_InBagg,
	&LogicManager::HAND_Anna_DeadBagg,
	&LogicManager::HAND_Anna_BaggageFight,
	&LogicManager::HAND_Anna_PrepareVienna,
	&LogicManager::HAND_Anna_StartPart4,
	&LogicManager::HAND_Anna_Reading,
	&LogicManager::HAND_Anna_Sulking,
	&LogicManager::HAND_Anna_GoSalon4,
	&LogicManager::HAND_Anna_ReturnToComp4,
	&LogicManager::HAND_Anna_EnterCompCathFollowsAnna,
	&LogicManager::HAND_Anna_DoWalkCathFollowsAnna,
	&LogicManager::HAND_Anna_LetDownHair,
	&LogicManager::HAND_Anna_StartPart5,
	&LogicManager::HAND_Anna_TiedUp,
	&LogicManager::HAND_Anna_Outside,
	&LogicManager::HAND_Anna_ReadyToScore,
	&LogicManager::HAND_Anna_Kidnapped,
	&LogicManager::HAND_Anna_Waiting,
	&LogicManager::HAND_Anna_FinalSequence,
	&LogicManager::HAND_Anna_OpenFirebird
};

} // End of namespace LastExpress
