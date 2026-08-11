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

void LogicManager::CONS_Milos(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMilos,
			_functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]
		);

		break;
	case 1:
		CONS_Milos_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Milos_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Milos_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Milos_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Milos_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::MilosCall(CALL_PARAMS) {
	getCharacter(kCharacterMilos).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Milos_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_DebugWalks);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 1;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterMilos, kCarGreenSleeping, getCharacterCurrentParams(kCharacterMilos)[0])) {
			if (getCharacterCurrentParams(kCharacterMilos)[0] == 10000) {
				getCharacterCurrentParams(kCharacterMilos)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterMilos)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterMilos).characterPosition.position = 0;
		getCharacter(kCharacterMilos).characterPosition.location = 0;
		getCharacter(kCharacterMilos).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterMilos)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_DoSeqOtis);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterMilos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
		fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMilos, (char *)&getCharacterCurrentParams(kCharacterMilos)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_DoCorrOtis);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterMilos, getCharacterCurrentParams(kCharacterMilos)[3]);

		getCharacter(kCharacterMilos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
		fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMilos, (char *)&getCharacterCurrentParams(kCharacterMilos)[0]);
		blockAtDoor(kCharacterMilos, getCharacterCurrentParams(kCharacterMilos)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_DoBriefCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_DoBriefCorrOtis);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_DoBriefCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterMilos, getCharacterCurrentParams(kCharacterMilos)[3]);

		getCharacter(kCharacterMilos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
		fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterMilos, (char *)&getCharacterCurrentParams(kCharacterMilos)[0]);
		blockAtDoor(kCharacterMilos, getCharacterCurrentParams(kCharacterMilos)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_FinishSeqOtis);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 5;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterMilos).direction != 4) {
			getCharacter(kCharacterMilos).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
			fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterMilos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
		fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_DoDialog);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterMilos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
		fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		break;
	case 12:
		playDialog(kCharacterMilos, (char *)&getCharacterCurrentParams(kCharacterMilos)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_DoDialogFullVol(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_DoDialogFullVol);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 7;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_DoDialogFullVol(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterMilos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
		fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		break;
	case 12:
		playDialog(kCharacterMilos, (char *)&getCharacterCurrentParams(kCharacterMilos)[0], 16, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_SaveGame);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 8;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterMilos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
		fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		break;
	case 12:
		save(
			kCharacterMilos,
			getCharacterCurrentParams(kCharacterMilos)[0],
			getCharacterCurrentParams(kCharacterMilos)[1]
		);

		getCharacter(kCharacterMilos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
		fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_DoWait);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 9;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMilos)[1] || (getCharacterCurrentParams(kCharacterMilos)[1] = _gameTime + getCharacterCurrentParams(kCharacterMilos)[0],
															  _gameTime + getCharacterCurrentParams(kCharacterMilos)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterMilos)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterMilos)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterMilos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
		fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_DoWalk);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterMilos, getCharacterCurrentParams(kCharacterMilos)[0], getCharacterCurrentParams(kCharacterMilos)[1])) {
			getCharacter(kCharacterMilos).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
			fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		}

		break;
	case 5:
	case 6:
		if (_doneNIS[kEventMilosTylerCompartmentDefeat]) {
			switch (rnd(3)) {
			case 0:
				playDialog(kCharacterCath, "CAT1014", -1, 0);
				break;
			case 1:
				playDialog(kCharacterCath, "CAT1014A", -1, 0);
				break;
			case 2:
			default:
				playDialog(kCharacterCath, "CAT1014B", -1, 0);
				break;
			}
		} else {
			playCathExcuseMe();
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_CompLogic(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_CompLogic);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 11;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_CompLogic(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		if (!getCharacterCurrentParams(kCharacterMilos)[4] && getCharacterCurrentParams(kCharacterMilos)[0] < _gameTime && !getCharacterCurrentParams(kCharacterMilos)[6]) {
			getCharacterCurrentParams(kCharacterMilos)[6] = 1;
			getCharacter(kCharacterMilos).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
			fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
			break;
		}

		bool skip1 = false;

		if (!getCharacterCurrentParams(kCharacterMilos)[1]) {
			getCharacterCurrentParams(kCharacterMilos)[7] = 0;
			skip1 = true;
		}

		if (!skip1 && !getCharacterCurrentParams(kCharacterMilos)[7]) {
			getCharacterCurrentParams(kCharacterMilos)[7] = _realTime + 75;
			if (_realTime == -75) {
				getCharacterCurrentParams(kCharacterMilos)[1] = 0;
				getCharacterCurrentParams(kCharacterMilos)[2] = 1;
				setDoor(38, kCharacterMilos, 1, 0, 0);
				getCharacterCurrentParams(kCharacterMilos)[7] = 0;
				skip1 = true;
			}
		}

		if (!skip1 && getCharacterCurrentParams(kCharacterMilos)[7] < _realTime) {
			getCharacterCurrentParams(kCharacterMilos)[7] = 0x7FFFFFFF;
			getCharacterCurrentParams(kCharacterMilos)[1] = 0;
			getCharacterCurrentParams(kCharacterMilos)[2] = 1;
			setDoor(38, kCharacterMilos, 1, 0, 0);
			getCharacterCurrentParams(kCharacterMilos)[7] = 0;
		}

		if (_globals[kGlobalChapter] == 1 && !getCharacterCurrentParams(kCharacterMilos)[4]) {
			bool skip2 = false;

			if (getCharacterCurrentParams(kCharacterMilos)[5]) {
				if (getCharacterCurrentParams(kCharacterMilos)[8] || (getCharacterCurrentParams(kCharacterMilos)[8] = _gameTime + 4500, _gameTime != -4500)) {
					if (getCharacterCurrentParams(kCharacterMilos)[8] >= _gameTime) {
						skip2 = true;
					}

					if (!skip2) {
						getCharacterCurrentParams(kCharacterMilos)[8] = 0x7FFFFFFF;
					}
				}

				if (!skip2) {
					getCharacterCurrentParams(kCharacterMilos)[5] = 0;
					getCharacterCurrentParams(kCharacterMilos)[8] = 0;
				}
			}

			if (!_globals[kGlobalMetMilos]) {
				if (getCharacterParams(kCharacterMilos, 8)[2] && !_globals[kGlobalCharacterSearchingForCath] && !getCharacterCurrentParams(kCharacterMilos)[5]) {
					_globals[kGlobalCharacterSearchingForCath] = kCharacterMilos;
					send(kCharacterMilos, kCharacterVesna, 190412928, 0);
					getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
					MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "609Cg", 38, 0, 0);
				}

				break;
			}

			if (!getCharacterCurrentParams(kCharacterMilos)[3])
				getCharacterCurrentParams(kCharacterMilos)[3] = _gameTime + 18000;

			if (getCharacterCurrentParams(kCharacterMilos)[9] != 0x7FFFFFFF && _gameTime) {
				if (getCharacterCurrentParams(kCharacterMilos)[3] >= _gameTime) {
					if (!nearChar(kCharacterCath, kCharacterMilos, 2000) || !getCharacterCurrentParams(kCharacterMilos)[9]) {
						getCharacterCurrentParams(kCharacterMilos)[9] = _gameTime + 150;
						if (_gameTime == -150) {
							if (nearChar(kCharacterCath, kCharacterMilos, 2000))
								_globals[kGlobalOverheardVesnaAndMilosDebatingAboutCath] = 1;

							setDoor(38, kCharacterMilos, 3, 0, 0);
							getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 6;
							MilosCall(&LogicManager::CONS_Milos_DoDialog, "MIL1012", 0, 0, 0);
							break;
						}
					}

					if (getCharacterCurrentParams(kCharacterMilos)[9] >= _gameTime)
						break;
				}

				getCharacterCurrentParams(kCharacterMilos)[9] = 0x7FFFFFFF;
				if (nearChar(kCharacterCath, kCharacterMilos, 2000))
					_globals[kGlobalOverheardVesnaAndMilosDebatingAboutCath] = 1;

				setDoor(38, kCharacterMilos, 3, 0, 0);
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 6;
				MilosCall(&LogicManager::CONS_Milos_DoDialog, "MIL1012", 0, 0, 0);
			}
		}

		break;
	}
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterMilos)[1]) {
			setDoor(38, kCharacterMilos, 3, 0, 0);

			if (cathHasItem(kItemPassengerList)) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 10;
				if (rnd(2) != 0) {
					MilosCall(&LogicManager::CONS_Milos_DoDialog, getCathSorryDialog(), 0, 0, 0);
				} else {
					MilosCall(&LogicManager::CONS_Milos_DoDialog, "CAT1504", 0, 0, 0);
				}
			} else {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 11;
				MilosCall(&LogicManager::CONS_Milos_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else {
			setDoor(38, kCharacterMilos, 3, 0, 0);

			if (msg->action == 8) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 7;
				MilosCall(&LogicManager::CONS_Milos_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 8;
				MilosCall(&LogicManager::CONS_Milos_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		setDoor(38, kCharacterMilos, 3, 10, 9);
		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterMilos)[2] || getCharacterCurrentParams(kCharacterMilos)[1]) {
			setDoor(38, kCharacterMilos, 3, 10, 9);
			getCharacterCurrentParams(kCharacterMilos)[2] = 0;
			getCharacterCurrentParams(kCharacterMilos)[1] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMilos).characterPosition.location = 0;
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
			MilosCall(&LogicManager::CONS_Milos_DoWalk, 3, 8200, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 3;
			MilosCall(&LogicManager::CONS_Milos_KnockTyler, 0, 0, 0, 0);
			break;
		case 3:
			if (_globals[kGlobalCharacterSearchingForCath] == kCharacterMilos)
				_globals[kGlobalCharacterSearchingForCath] = 0;

			getCharacterCurrentParams(kCharacterMilos)[5] = 1;
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 4;
			MilosCall(&LogicManager::CONS_Milos_DoWalk, 4, 3050, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 5;
			MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "609Bg", 38, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterMilos).characterPosition.location = 1;
			endGraphics(kCharacterMilos);
			send(kCharacterMilos, kCharacterVesna, 101687594, 0);
			setDoor(38, kCharacterMilos, 3, 10, 9);
			break;
		case 6:
			setDoor(38, kCharacterMilos, 3, 10, 9);
			break;
		case 7:
		case 8:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 9;
			MilosCall(&LogicManager::CONS_Milos_DoDialog, "MIL1117A", 0, 0, 0);
			break;
		case 9:
			setDoor(38, kCharacterMilos, 3, 14, 0);
			getCharacterCurrentParams(kCharacterMilos)[1] = 1;
			break;
		case 10:
		case 11:
			getCharacterCurrentParams(kCharacterMilos)[1] = 0;
			getCharacterCurrentParams(kCharacterMilos)[2] = 1;
			break;
		case 12:
			startCycOtis(kCharacterMilos, "611Cg");
			softBlockAtDoor(kCharacterMilos, 38);
			send(kCharacterMilos, kCharacterCond2, 88652208, 0);
			break;
		case 13:
			softReleaseAtDoor(kCharacterMilos, 38);
			getCharacter(kCharacterMilos).characterPosition.location = 1;
			endGraphics(kCharacterMilos);
			setDoor(38, kCharacterMilos, 3, 10, 9);
			getCharacterCurrentParams(kCharacterMilos)[4] = 0;
			break;
		default:
			break;
		}

		break;
	case 122865568:
		getCharacter(kCharacterMilos).characterPosition.location = 0;
		getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 12;
		MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "611Bg", 38, 0, 0);
		break;
	case 123852928:
		getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 13;
		MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "611Dg", 38, 0, 0);
		break;
	case 221683008:
		getCharacterCurrentParams(kCharacterMilos)[4] = 1;
		send(kCharacterMilos, kCharacterCond2, 123199584, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_Birth);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 12;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterMilos)[0]) {
			getCharacterCurrentParams(kCharacterMilos)[0] = 1;
			CONS_Milos_AtDinner(0, 0, 0, 0);
		}

		break;
	case 12:
		setDoor(38, kCharacterCath, 3, 10, 9);
		setDoor(46, kCharacterCath, 0, 255, 255);
		getCharacter(kCharacterMilos).characterPosition.position = 4689;
		getCharacter(kCharacterMilos).characterPosition.location = 1;
		getCharacter(kCharacterMilos).characterPosition.car = kCarRestaurant;
		autoMessage(kCharacterMilos, 157691176, 0);
		autoMessage(kCharacterMilos, 208228224, 2);
		autoMessage(kCharacterMilos, 259125998, 3);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_DoOtis5009D(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_DoOtis5009D);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 13;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_DoOtis5009D(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		send(kCharacterMilos, kCharacterTableC, 103798704, "009E");
		endGraphics(kCharacterVesna);
		endGraphics(kCharacterIvo);
		endGraphics(kCharacterSalko);

		getCharacter(kCharacterMilos).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
		fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterSalko, "009D5");
		startSeqOtis(kCharacterTableC, "009D4");
		startSeqOtis(kCharacterIvo, "009D3");
		startSeqOtis(kCharacterVesna, "009D2");
		startSeqOtis(kCharacterMilos, "009D1");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_KnockTyler(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_KnockTyler);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 14;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_KnockTyler(HAND_PARAMS) {
	int fightOutcome;

	switch (msg->action) {
	case 0:
		if (_globals[kGlobalCharacterSearchingForCath] == kCharacterPolice || _globals[kGlobalCharacterSearchingForCath] == kCharacterCond1) {
			if (getCharacterCurrentParams(kCharacterMilos)[1]) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
				MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "609Ca", 1, 0, 0);
			} else {
				softReleaseAtDoor(kCharacterMilos, 1);
				setDoor(1, kCharacterCath, 0, 10, 9);

				getCharacter(kCharacterMilos).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
				fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
			}
		} else if (getCharacterCurrentParams(kCharacterMilos)[0]) {
			if ((!getCharacterCurrentParams(kCharacterMilos)[8] && (getCharacterCurrentParams(kCharacterMilos)[8] = _realTime + 45, _realTime == -45)) || getCharacterCurrentParams(kCharacterMilos)[8] < _realTime) {
				if (checkDoor(1) == 1) {
					if (getCharacterCurrentParams(kCharacterMilos)[9] || (getCharacterCurrentParams(kCharacterMilos)[9] = _realTime + 75, _realTime != -75)) {
						if (getCharacterCurrentParams(kCharacterMilos)[9] >= _realTime)
							break;
						getCharacterCurrentParams(kCharacterMilos)[9] = 0x7FFFFFFF;
					}

					setDoor(1, 14, checkDoor(1), 0, 0);

					getCharacterCurrentParams(kCharacterMilos)[4]++;
					switch (getCharacterCurrentParams(kCharacterMilos)[4]) {
					case 1:
						getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 6;
						MilosCall(&LogicManager::CONS_Milos_DoDialog, "LIB013", 0, 0, 0);
						break;
					case 2:
						getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 8;
						MilosCall(&LogicManager::CONS_Milos_DoDialog, "LIB012", 0, 0, 0);
						break;
					case 3:
						getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 10;
						MilosCall(&LogicManager::CONS_Milos_DoDialog, "LIB012", 0, 0, 0);
						break;
					case 4:
						getCharacterCurrentParams(kCharacterMilos)[6]++;
						if (getCharacterCurrentParams(kCharacterMilos)[6] < 3) {
							getCharacterCurrentParams(kCharacterMilos)[4] = 1;
							setDoor(1, kCharacterMilos, checkDoor(1), getCharacterCurrentParams(kCharacterMilos)[2] == 0 ? 14 : 0, 9);
							getCharacterCurrentParams(kCharacterMilos)[9] = 0;
							break;
						}

						setDoor(1, kCharacterCath, checkDoor(1), 10, 9);

						getCharacter(kCharacterMilos).currentCall--;
						_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
						fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
						break;
					default:
						setDoor(1, kCharacterMilos, checkDoor(1), getCharacterCurrentParams(kCharacterMilos)[2] == 0 ? 14 : 0, 9);
						getCharacterCurrentParams(kCharacterMilos)[9] = 0;
						break;
					}
				} else if (!_globals[kGlobalCorpseMovedFromFloor] || _globals[kGlobalJacket] == 1) {
					setDoor(9, kCharacterCath, 0, 255, 255);
					getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
					MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosCorpseFloor, 0, 0);
				} else {
					getCharacterCurrentParams(kCharacterMilos)[5] = kEventMilosTylerCompartmentBedVisit;

					if (getModel(1) != 1)
						getCharacterCurrentParams(kCharacterMilos)[5] = kEventMilosTylerCompartmentVisit;

					getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 3;
					MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosTylerCompartmentVisit, 0, 0);
				}
			}
		} else if ((!getCharacterCurrentParams(kCharacterMilos)[10] && (getCharacterCurrentParams(kCharacterMilos)[10] = _realTime + 75, _realTime == -75)) || getCharacterCurrentParams(kCharacterMilos)[10] < _realTime) {
			if (!getCharacterCurrentParams(kCharacterMilos)[3]) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 12;
				MilosCall(&LogicManager::CONS_Milos_DoDialog, "MIL1030C", 0, 0, 0);
				break;
			}

			if (getCharacterCurrentParams(kCharacterMilos)[11] || (getCharacterCurrentParams(kCharacterMilos)[11] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterMilos)[11] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterMilos)[11] = 0x7FFFFFFF;
			}

			softReleaseAtDoor(kCharacterMilos, 1);

			if (_globals[kGlobalCorpseMovedFromFloor]) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 13;
				MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "609Ba", 1, 0, 0);
			} else if (checkLoc(kCharacterCath, kCarGreenSleeping)) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 14;
				MilosCall(&LogicManager::CONS_Milos_DoBriefCorrOtis, "609Ba", 1, 0, 0);
			} else {
				bumpCath(kCarNone, 1, 255);
				setDoor(9, kCharacterCath, 0, 255, 255);
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 15;
				MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosCorpseFloor, 0, 0);
			}
		}

		break;
	case 8:
		if (getCharacterCurrentParams(kCharacterMilos)[1]) {
			setDoor(1, kCharacterMilos, 0, 0, 0);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 20;
			MilosCall(&LogicManager::CONS_Milos_DoDialog, "LIB012", 0, 0, 0);
		} else if (!getCharacterCurrentParams(kCharacterMilos)[2]) {
			setDoor(1, kCharacterMilos, checkDoor(1), 0, 0);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 22;
			MilosCall(&LogicManager::CONS_Milos_DoDialogFullVol, "MIL1032", 0, 0, 0);
		}

		break;
	case 9:
		if (!_globals[kGlobalCorpseMovedFromFloor] || _globals[kGlobalJacket] == 1) {
			setDoor(9, kCharacterCath, 0, 255, 255);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 16;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosCorpseFloor, 0, 0);
		} else {
			if (getCharacterCurrentParams(kCharacterMilos)[1]) {
				getCharacter(kCharacterCath).characterPosition.location = 1;
				if (getModel(1) == 1) {
					getCharacterCurrentParams(kCharacterMilos)[5] = kEventMilosTylerCompartmentBed;
				} else {
					getCharacterCurrentParams(kCharacterMilos)[5] = kEventMilosTylerCompartment;
				}
			} else {
				getCharacterCurrentParams(kCharacterMilos)[5] = kEventMilosTylerCompartmentBedVisit;

				if (getModel(1) != 1)
					getCharacterCurrentParams(kCharacterMilos)[5] = kEventMilosTylerCompartmentVisit;
			}

			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 17;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosTylerCompartmentVisit, 0, 0);
		}

		break;
	case 12:
		if (inComp(kCharacterCath, kCarGreenSleeping, 8200) || inComp(kCharacterCath, kCarGreenSleeping, 7850) || cathOutHisWindow()) {
			setDoor(1, kCharacterMilos, checkDoor(1), 0, 0);

			if (cathOutHisWindow())
				bumpCath(kCarGreenSleeping, 49, 255);

			playDialog(0, "LIB012", -1, 0);
			setDoor(1, kCharacterMilos, checkDoor(1), 14, 9);
			getCharacterCurrentParams(kCharacterMilos)[0] = 1;
		} else {
			startCycOtis(kCharacterMilos, "609Aa");
			softBlockAtDoor(kCharacterMilos, 1);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMilos).characterPosition.location = 0;
			setDoor(1, kCharacterCath, 0, 10, 9);

			getCharacter(kCharacterMilos).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
			fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
			break;
		case 2:
			playDialog(0, "LIB014", -1, 0);
			playNIS(kEventMilosCorpseFloor);
			endGame(0, 1, _globals[kGlobalCorpseMovedFromFloor] == 0 ? 57 : 55, 1);
			break;
		case 3:
			setDoor(1, kCharacterCath, 0, 10, 9);
			setDoor(9, kCharacterCath, 0, 255, 255);
			playDialog(0, "LIB014", -1, 0);
			playNIS(getCharacterCurrentParams(kCharacterMilos)[5]);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 4;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 1, 0, 0, 0);
			break;
		case 4:
			fightOutcome = playFight(2001);
			getCharacterCurrentParams(kCharacterMilos)[7] = fightOutcome;

			if (fightOutcome) {
				endGame(0, 0, 0, fightOutcome == 1);
				break;
			}

			_gameTime += 1800;
			_globals[kGlobalMetMilos] = 1;
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 5;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosTylerCompartmentDefeat, 0, 0);
			break;
		case 5:
		case 19:
			playNIS(kEventMilosTylerCompartmentDefeat);
			playDialog(0, "LIB015", -1, 0);
			bumpCathTylerComp();
			getCharacter(kCharacterMilos).characterPosition.location = 0;

			getCharacter(kCharacterMilos).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
			fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
			break;
		case 6:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 7;
			MilosCall(&LogicManager::CONS_Milos_DoDialogFullVol, "MIL1031C", 0, 0, 0);
			break;
		case 7:
		case 9:
		case 11:
			setDoor(1, kCharacterMilos, checkDoor(1), getCharacterCurrentParams(kCharacterMilos)[2] == 0 ? 14 : 0, 9);
			getCharacterCurrentParams(kCharacterMilos)[9] = 0;
			break;
		case 8:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 9;
			MilosCall(&LogicManager::CONS_Milos_DoDialogFullVol, "MIL1031A", 0, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 11;
			MilosCall(&LogicManager::CONS_Milos_DoDialogFullVol, "MIL1031B", 0, 0, 0);
			break;
		case 12:
			getCharacterCurrentParams(kCharacterMilos)[3] = 1;

			if (getCharacterCurrentParams(kCharacterMilos)[11] || (getCharacterCurrentParams(kCharacterMilos)[11] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterMilos)[11] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterMilos)[11] = 0x7FFFFFFF;
			}

			softReleaseAtDoor(kCharacterMilos, 1);
			if (_globals[kGlobalCorpseMovedFromFloor]) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 13;
				MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "609Ba", 1, 0, 0);
			} else if (checkLoc(kCharacterCath, kCarGreenSleeping)) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 14;
				MilosCall(&LogicManager::CONS_Milos_DoBriefCorrOtis, "609Ba", 1, 0, 0);
			} else {
				bumpCath(kCarNone, 1, 255);
				setDoor(9, kCharacterCath, 0, 255, 255);
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 15;
				MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosCorpseFloor, 0, 0);
			}

			break;
		case 13:
			getCharacterCurrentParams(kCharacterMilos)[1] = 1;
			endGraphics(kCharacterMilos);
			getCharacter(kCharacterMilos).characterPosition.location = 1;
			setDoor(1, kCharacterMilos, 0, 10, 9);
			break;
		case 14:
			setDoor(9, kCharacterCath, 0, 255, 255);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 15;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosCorpseFloor, 0, 0);
			break;
		case 15:
			playNIS(kEventMilosCorpseFloor);
			endGame(0, 1, 57, true);
			break;
		case 16:
			if (checkDoor(1) != 1) {
				playDialog(0, "LIB014", -1, 0);
			} else {
				playDialog(0, "LIB032", -1, 0);
			}
			
			playNIS(kEventMilosCorpseFloor);
			endGame(0, 1, _globals[kGlobalCorpseMovedFromFloor] == 0 ? 57 : 55, true);
			break;
		case 17:
			if (checkDoor(1) != 1) {
				playDialog(0, "LIB014", -1, 0);
			} else {
				playDialog(0, "LIB032", -1, 0);
			}

			setDoor(1, kCharacterCath, 0, 10, 9);
			setDoor(9, kCharacterCath, 0, 255, 255);
			playNIS(getCharacterCurrentParams(kCharacterMilos)[5]);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 18;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 1, 0, 0, 0);
			break;
		case 18:
			fightOutcome = playFight(2001);
			getCharacterCurrentParams(kCharacterMilos)[7] = fightOutcome;
			if (fightOutcome) {
				endGame(0, 0, 0, fightOutcome == 1);
			} else {
				_gameTime += 1800;
				_globals[kGlobalMetMilos] = 1;
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 19;
				MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosTylerCompartmentDefeat, 0, 0);
			}

			break;
		case 20:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 21;
			MilosCall(&LogicManager::CONS_Milos_DoDialog, "MIL1117A", 0, 0, 0);
			break;
		case 21:
			setDoor(1, kCharacterMilos, 0, 10, 9);
			break;
		case 22:
			getCharacterCurrentParams(kCharacterMilos)[2] = 1;
			setDoor(1, kCharacterMilos, checkDoor(1), 0, 9);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_AtDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_AtDinner);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 15;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_AtDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1071000 && !getCharacterCurrentParams(kCharacterMilos)[2]) {
			getCharacterCurrentParams(kCharacterMilos)[2] = 1;
			send(kCharacterMilos, kCharacterWaiter2, 223002560, 0);
		}

		if (_gameTime > 1089000 && rcClear()) {
			CONS_Milos_ReturnFromDinner(0, 0, 0, 0);
			break;
		}

		if (checkCathDir(kCarRestaurant, 61) && !getCharacterCurrentParams(kCharacterMilos)[0]) {
			if (getCharacterCurrentParams(kCharacterMilos)[3] || (getCharacterCurrentParams(kCharacterMilos)[3] = _realTime + 45,
																  _realTime != -45)) {
				if (getCharacterCurrentParams(kCharacterMilos)[3] >= _realTime) {
					if (checkCathDir(kCarRestaurant, 70) && !getCharacterCurrentParams(kCharacterMilos)[1]) {
						if (getCharacterCurrentParams(kCharacterMilos)[4] || (getCharacterCurrentParams(kCharacterMilos)[4] = _realTime + 45,
																			  _realTime != -45)) {
							if (getCharacterCurrentParams(kCharacterMilos)[4] >= _realTime)
								break;

							getCharacterCurrentParams(kCharacterMilos)[4] = 0x7FFFFFFF;
						}
						getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
						MilosCall(&LogicManager::CONS_Milos_DoSeqOtis, "009C", 0, 0, 0);
					}

					break;
				}

				getCharacterCurrentParams(kCharacterMilos)[3] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
			MilosCall(&LogicManager::CONS_Milos_DoSeqOtis, "009C", 0, 0, 0);
			break;
		}

		if (checkCathDir(kCarRestaurant, 70) && !getCharacterCurrentParams(kCharacterMilos)[1]) {
			if (getCharacterCurrentParams(kCharacterMilos)[4] || (getCharacterCurrentParams(kCharacterMilos)[4] = _realTime + 45,
																  _realTime != -45)) {
				if (getCharacterCurrentParams(kCharacterMilos)[4] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterMilos)[4] = 0x7FFFFFFF;
			}
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
			MilosCall(&LogicManager::CONS_Milos_DoSeqOtis, "009C", 0, 0, 0);
		}

		break;
	case 12:
		send(kCharacterMilos, kCharacterTableC, 136455232, 0);
		startCycOtis(kCharacterMilos, "009A");
		break;
	case 18:
		if (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] != 1) {
			if (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] == 2) {
				startCycOtis(kCharacterMilos, "009A");
				getCharacterCurrentParams(kCharacterMilos)[1] = 1;
			}

			break;
		}

		startCycOtis(kCharacterMilos, "009A");
		getCharacterCurrentParams(kCharacterMilos)[0] = 1;

		if (checkCathDir(kCarRestaurant, 70) && !getCharacterCurrentParams(kCharacterMilos)[1]) {
			if (getCharacterCurrentParams(kCharacterMilos)[4] || (getCharacterCurrentParams(kCharacterMilos)[4] = _realTime + 45,
																  _realTime != -45)) {
				if (getCharacterCurrentParams(kCharacterMilos)[4] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterMilos)[4] = 0x7FFFFFFF;
			}

			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
			MilosCall(&LogicManager::CONS_Milos_DoSeqOtis, "009C", 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_ReturnFromDinner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_ReturnFromDinner);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 16;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_ReturnFromDinner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMilos)[0] && (nearChar(kCharacterMilos, kCharacterVesna, 750) || nearX(kCharacterVesna, 3050, 500))) {
			send(kCharacterMilos, kCharacterVesna, 123668192, 0);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 5;
			MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "611Ag", 38, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterMilos).characterPosition.location = 0;
		getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
		MilosCall(&LogicManager::CONS_Milos_DoOtis5009D, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8]) {
		case 1:
			send(kCharacterMilos, kCharacterWaiter2, 269485588, 0);
			send(kCharacterMilos, kCharacterIvo, 125242096, 0);
			startSeqOtis(kCharacterMilos, "807DS");
			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterMilos);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
			MilosCall(&LogicManager::CONS_Milos_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 2:
			endGraphics(kCharacterMilos);
			break;
		case 3:
			if (nearChar(kCharacterMilos, kCharacterVesna, 750) || nearX(kCharacterVesna, 3050, 500)) {
				send(kCharacterMilos, kCharacterVesna, 123668192, 0);
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 4;
				MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "611Ag", 38, 0, 0);
			} else {
				getCharacterCurrentParams(kCharacterMilos)[0] = 1;
				startCycOtis(kCharacterMilos, "609Dg");
				softBlockAtDoor(kCharacterMilos, 38);
			}

			break;
		case 4:
			getCharacter(kCharacterMilos).characterPosition.position = 3050;
			getCharacter(kCharacterMilos).characterPosition.location = 1;
			endGraphics(kCharacterMilos);
			CONS_Milos_InComp(0, 0, 0, 0);
			break;
		case 5:
			softReleaseAtDoor(kCharacterMilos, 38);
			getCharacter(kCharacterMilos).characterPosition.position = 3050;
			getCharacter(kCharacterMilos).characterPosition.location = 1;
			endGraphics(kCharacterMilos);
			CONS_Milos_InComp(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 135024800:
		send(kCharacterMilos, kCharacterVesna, 204832737, 0);
		getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 3;
		MilosCall(&LogicManager::CONS_Milos_DoWalk, 4, 3050, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_InComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_InComp);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 17;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_InComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
		MilosCall(&LogicManager::CONS_Milos_CompLogic, 1404000, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_Asleep);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 18;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMilos).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMilos).characterPosition.position = 3050;
		getCharacter(kCharacterMilos).characterPosition.location = 1;
		endGraphics(kCharacterMilos);
		setDoor(38, kCharacterCath, 3, 10, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_StartPart2);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 19;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Milos_OnRearPlat(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMilos);
		getCharacter(kCharacterMilos).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMilos).characterPosition.position = 540;
		getCharacter(kCharacterMilos).characterPosition.location = 0;
		getCharacter(kCharacterMilos).inventoryItem = 0;
		getCharacter(kCharacterMilos).clothes = 0;
		setDoor(38, kCharacterCath, 3, 10, 9);
		setDoor(46, kCharacterCath, 0, 255, 255);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_OnRearPlat(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_OnRearPlat);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 20;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_OnRearPlat(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMilos).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMilos).characterPosition.position = 540;
		getCharacter(kCharacterMilos).characterPosition.location = 0;
		send(kCharacterMilos, kCharacterVesna, 137165825, 0);
		break;
	case 17:
		if (cathInCorridor(kCarRedSleeping) && !checkCathDir(kCarRedSleeping, 1)) {
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
			MilosCall(&LogicManager::CONS_Milos_DoWalk, 4, 3050, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] == 1) {
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
			MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "609Bg", 38, 0, 0);
		} else if (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] == 2) {
			endGraphics(kCharacterMilos);
			getCharacter(kCharacterMilos).characterPosition.location = 1;
			getCharacter(kCharacterMilos).characterPosition.position = 3050;
			send(kCharacterMilos, kCharacterVesna, 101687594, 0);
			CONS_Milos_InPart2(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_InPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_InPart2);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 21;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_InPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterMilos)[1]) {
			getCharacterCurrentParams(kCharacterMilos)[1] = _gameTime + 4500;
			if (_gameTime == -4500) {
				getCharacterCurrentParams(kCharacterMilos)[0] = 1;
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterMilos)[1] < _gameTime) {
			getCharacterCurrentParams(kCharacterMilos)[1] = 0x7FFFFFFF;
			getCharacterCurrentParams(kCharacterMilos)[0] = 1;
		}

		break;
	case 8:
		setDoor(38, kCharacterMilos, 3, 0, 0);
		getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
		MilosCall(&LogicManager::CONS_Milos_DoDialog, "LIB012", 0, 0, 0);
		break;
	case 9:
		setDoor(38, kCharacterMilos, 3, 0, 0);
		getCharacter(kCharacterCath).characterPosition.location = 1;
		getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 3;
		MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosCompartmentVisitAugust, 0, 0);
		break;
	case 12:
		setDoor(38, kCharacterMilos, 3, 10, 9);
		break;
	case 17:
		if (!_doneNIS[kEventMilosCompartmentVisitAugust] && !checkLoc(kCharacterCath, kCarRedSleeping) && getCharacterCurrentParams(kCharacterMilos)[0]) {
			CONS_Milos_OnRearPlat(0, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
			MilosCall(&LogicManager::CONS_Milos_DoDialog, "Mil1118", 0, 0, 0);
			break;
		case 2:
			setDoor(38, kCharacterMilos, 3, 10, 9);
			break;
		case 3:
			playNIS(kEventMilosCompartmentVisitAugust);
			bumpCath(kCarRedSleeping, 5, 255);
			send(kCharacterMilos, kCharacterVesna, 135024800, 0);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 4;
			MilosCall(&LogicManager::CONS_Milos_CompLogic, 15803100, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_StartPart3);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 22;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_doneNIS[kEventMilosCompartmentVisitAugust]) {
			CONS_Milos_Unfriendly(0, 0, 0, 0);
		} else {
			CONS_Milos_OnRearPlat3(0, 0, 0, 0);
		}

		break;
	case 12:
		endGraphics(kCharacterMilos);
		getCharacter(kCharacterMilos).inventoryItem = 0;
		getCharacter(kCharacterMilos).clothes = 0;
		setDoor(38, kCharacterMilos, 3, 10, 9);
		getCharacterParams(kCharacterMilos, 8)[0] = 0;
		getCharacterParams(kCharacterMilos, 8)[3] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_OnRearPlat3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_OnRearPlat3);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 23;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_OnRearPlat3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2106000 && !getCharacterCurrentParams(kCharacterMilos)[0]) {
			getCharacterCurrentParams(kCharacterMilos)[0] = 1;
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
			MilosCall(&LogicManager::CONS_Milos_DoWalk, 4, 3050, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterMilos).characterPosition.position = 540;
		getCharacter(kCharacterMilos).characterPosition.location = 0;
		getCharacter(kCharacterMilos).characterPosition.car = kCarRedSleeping;
		send(kCharacterMilos, kCharacterVesna, 137165825, 0);
		break;
	case 17:
		if (cathInCorridor(kCarRedSleeping) && !checkCathDir(kCarRedSleeping, 1)) {
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 3;
			MilosCall(&LogicManager::CONS_Milos_DoWalk, 4, 0xBEA, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
			MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "609Bg", 38, 0, 0);
			break;
		case 2:
		case 4:
			endGraphics(kCharacterMilos);
			getCharacter(kCharacterMilos).characterPosition.location = 1;
			send(kCharacterMilos, kCharacterVesna, 101687594, 0);
			CONS_Milos_Unfriendly(0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 4;
			MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "609Bg", 38, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_Unfriendly(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_Unfriendly);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 24;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_Unfriendly(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		bool skip = false;

		if (!getCharacterCurrentParams(kCharacterMilos)[3]) {
			getCharacterCurrentParams(kCharacterMilos)[3] = _gameTime + 4500;
			if (_gameTime == -4500) {
				skip = true;
				getCharacterCurrentParams(kCharacterMilos)[2] = 1;
			}
		}

		if (!skip && getCharacterCurrentParams(kCharacterMilos)[3] < _gameTime) {
			getCharacterCurrentParams(kCharacterMilos)[3] = 0x7FFFFFFF;
			getCharacterCurrentParams(kCharacterMilos)[2] = 1;
		}

		if (getCharacterParams(kCharacterMilos, 8)[0]) {
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
			MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "609Cg", 38, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterMilos)[0]) {
			if (getCharacterCurrentParams(kCharacterMilos)[4] || (getCharacterCurrentParams(kCharacterMilos)[4] = _realTime + 75,
																  _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterMilos)[4] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterMilos)[4] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterMilos)[0] = 0;
			getCharacterCurrentParams(kCharacterMilos)[1] = 1;
			setDoor(38, kCharacterMilos, 1, 0, 0);
			getCharacterCurrentParams(kCharacterMilos)[4] = 0;
		} else {
			getCharacterCurrentParams(kCharacterMilos)[4] = 0;
		}

		break;
	}
	case 8:
		if (getCharacterCurrentParams(kCharacterMilos)[0]) {
			setDoor(38, kCharacterMilos, 3, 0, 0);
			if (cathHasItem(kItemPassengerList)) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 9;
				if (rnd(2) != 0) {
					MilosCall(&LogicManager::CONS_Milos_DoDialog, getCathSorryDialog(), 0, 0, 0);
				} else {
					MilosCall(&LogicManager::CONS_Milos_DoDialog, "CAT1504", 0, 0, 0);
				}

			} else {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 10;
				MilosCall(&LogicManager::CONS_Milos_DoDialog, getCathSorryDialog(), 0, 0, 0);
			}
		} else {
			setDoor(38, kCharacterMilos, 3, 0, 0);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 6;
			MilosCall(&LogicManager::CONS_Milos_DoDialog, "LIB012", 0, 0, 0);
		}

		break;
	case 9:
		setDoor(38, kCharacterMilos, 3, 0, 0);
		if (_doneNIS[kEventMilosCompartmentVisitAugust] || _gameTime >= 2106000) {
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 12;
			MilosCall(&LogicManager::CONS_Milos_DoDialog, "LIB013", 0, 0, 0);
		} else {
			getCharacter(kCharacterCath).characterPosition.location = 1;
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 11;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosCompartmentVisitAugust, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterMilos).characterPosition.position = 3050;
		getCharacter(kCharacterMilos).characterPosition.location = 1;
		getCharacter(kCharacterMilos).characterPosition.car = kCarRedSleeping;
		setDoor(38, kCharacterMilos, 3, 10, 9);
		break;
	case 17:
		if (_doneNIS[kEventMilosCompartmentVisitAugust] || checkLoc(kCharacterCath, kCarRedSleeping) || !getCharacterCurrentParams(kCharacterMilos)[2] || _gameTime >= 2106000) {
			if (getCharacterCurrentParams(kCharacterMilos)[1] || getCharacterCurrentParams(kCharacterMilos)[0]) {
				setDoor(38, kCharacterMilos, 3, 10, 9);
				getCharacterCurrentParams(kCharacterMilos)[1] = 0;
				getCharacterCurrentParams(kCharacterMilos)[0] = 0;
			}
		} else {
			CONS_Milos_OnRearPlat3(0, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8]) {
		case 1:
			setDoor(38, kCharacterCath, 3, 10, 9);
			send(kCharacterMilos, kCharacterVesna, 203663744, 0);
			getCharacter(kCharacterMilos).characterPosition.location = 0;
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
			MilosCall(&LogicManager::CONS_Milos_LookingForCath, 2223000, 0, 0, 0);
			break;
		case 2:
			if (!getCharacterParams(kCharacterMilos, 8)[1]) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 4;
				MilosCall(&LogicManager::CONS_Milos_DoWalk, 4, 3050, 0, 0);
				break;
			}

			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 3;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosCorridorThanksD, 0, 0);
			break;
		case 3:
			if (getCharacter(kCharacterMilos).characterPosition.position < getCharacter(kCharacterCath).characterPosition.position) {
				playNIS(kEventMilosCorridorThanksD);
			} else {
				playNIS(kEventMilosCorridorThanks);
			}

			if (getCharacter(kCharacterMilos).characterPosition.car == 4 && nearX(kCharacterMilos, 3050, 500))
				getCharacter(kCharacterMilos).characterPosition.position = 3550;

			walk(kCharacterMilos, kCarRedSleeping, 3050);

			if (getCharacter(kCharacterMilos).direction == 2) {
				bumpCathRx(getCharacter(kCharacterMilos).characterPosition.car, getCharacter(kCharacterMilos).characterPosition.position + 750);
			} else {
				bumpCathFx(getCharacter(kCharacterMilos).characterPosition.car, getCharacter(kCharacterMilos).characterPosition.position - 750);
			}

			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 4;
			MilosCall(&LogicManager::CONS_Milos_DoWalk, 4, 3050, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 5;
			MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "609BG", 38, 0, 0);
			break;
		case 5:
			endGraphics(kCharacterMilos);
			getCharacter(kCharacterMilos).characterPosition.location = 1;
			getCharacterParams(kCharacterMilos, 8)[0] = 0;
			CONS_Milos_Friendly(0, 0, 0, 0);
			break;
		case 6:
			if (_doneNIS[kEventMilosCompartmentVisitAugust] || _gameTime >= 2106000) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 8;
				MilosCall(&LogicManager::CONS_Milos_DoDialog, "Mil1117A", 0, 0, 0);
			} else {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 7;
				MilosCall(&LogicManager::CONS_Milos_DoDialog, "Mil1118", 0, 0, 0);
			}

			break;
		case 7:
			setDoor(38, kCharacterMilos, 3, 10, 9);
			break;
		case 8:
		case 13:
			setDoor(38, kCharacterMilos, 3, 14, 0);
			getCharacterCurrentParams(kCharacterMilos)[0] = 1;
			break;
		case 9:
		case 10:
			getCharacterCurrentParams(kCharacterMilos)[0] = 0;
			getCharacterCurrentParams(kCharacterMilos)[1] = 1;
			break;
		case 11:
			playNIS(kEventMilosCompartmentVisitAugust);
			bumpCath(kCarRedSleeping, 5, 255);
			send(kCharacterMilos, kCharacterVesna, 135024800, 0);
			setDoor(38, kCharacterMilos, 3, 10, 9);
			break;
		case 12:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 13;
			MilosCall(&LogicManager::CONS_Milos_DoDialog, "MIL1117A", 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_Friendly(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_Friendly);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 25;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_Friendly(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!_doneNIS[kEventMilosCompartmentVisitTyler] && !_globals[kGlobalAnnaIsInBaggageCar] && !getCharacterParams(kCharacterMilos, 8)[3]) {
			bool skip = false;

			if (!getCharacterCurrentParams(kCharacterMilos)[2]) {
				getCharacterCurrentParams(kCharacterMilos)[2] = _gameTime + 13500;
				if (_gameTime == -13500) {
					skip = true;
					send(kCharacterMilos, kCharacterVesna, 155913424, 0);
					getCharacterCurrentParams(kCharacterMilos)[2] = 0;
				}
			}

			if (!skip && getCharacterCurrentParams(kCharacterMilos)[2] < _gameTime) {
				getCharacterCurrentParams(kCharacterMilos)[2] = 0x7FFFFFFF;
				send(kCharacterMilos, kCharacterVesna, 155913424, 0);
				getCharacterCurrentParams(kCharacterMilos)[2] = 0;
			}
		}

		if (getCharacterCurrentParams(kCharacterMilos)[0]) {
			if (getCharacterCurrentParams(kCharacterMilos)[3] || (getCharacterCurrentParams(kCharacterMilos)[3] = _realTime + 75,
																	_realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterMilos)[3] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterMilos)[3] = 0x7FFFFFFF;
			}

			getCharacterCurrentParams(kCharacterMilos)[0] = 0;
			getCharacterCurrentParams(kCharacterMilos)[1] = 1;
			setDoor(38, kCharacterMilos, 1, 0, 0);
			getCharacterCurrentParams(kCharacterMilos)[3] = 0;
		} else {
			getCharacterCurrentParams(kCharacterMilos)[3] = 0;
		}

		break;
	case 8:
	case 9:
		if (getCharacterCurrentParams(kCharacterMilos)[0]) {
			setDoor(38, kCharacterMilos, 3, 0, 0);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 5;

			if (rnd(2) == 0) {
				MilosCall(&LogicManager::CONS_Milos_DoDialog, "CAT1505A", 0, 0, 0);
			} else {
				MilosCall(&LogicManager::CONS_Milos_DoDialog, "CAT1505", 0, 0, 0);
			}
		} else {
			setDoor(38, kCharacterMilos, 1, 0, 0);
			if (msg->action == 8) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
				MilosCall(&LogicManager::CONS_Milos_DoDialog, "LIB012", 0, 0, 0);
			} else {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
				MilosCall(&LogicManager::CONS_Milos_DoDialog, "LIB013", 0, 0, 0);
			}
		}

		break;
	case 12:
		setDoor(38, kCharacterMilos, 3, 10, 9);
		if (!_doneNIS[kEventMilosCompartmentVisitTyler] && !_globals[kGlobalAnnaIsInBaggageCar] && !getCharacterParams(kCharacterMilos, 8)[3]) {
			send(kCharacterMilos, kCharacterVesna, 155913424, 0);
		}

		break;
	case 17:
		if (getCharacterCurrentParams(kCharacterMilos)[1] || getCharacterCurrentParams(kCharacterMilos)[0]) {
			setDoor(38, kCharacterMilos, 3, 10, 9);
			getCharacterCurrentParams(kCharacterMilos)[1] = 0;
			getCharacterCurrentParams(kCharacterMilos)[0] = 0;
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8]) {
		case 1:
		case 2:
			if (inComp(kCharacterVesna, kCarRedSleeping, 3050)) {
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 3;
				MilosCall(&LogicManager::CONS_Milos_DoDialog, "VES1015A", 0, 0, 0);
			} else {
				if (_doneNIS[kEventMilosCompartmentVisitTyler] || getCharacterParams(kCharacterMilos, 8)[3]) {
					setDoor(38, kCharacterMilos, 1, 10, 9);
					break;
				}

				forceJump(kCharacterVesna, &LogicManager::CONS_Vesna_InComp);
				getCharacter(kCharacterCath).characterPosition.location = 1;
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 4;
				MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventMilosCompartmentVisitTyler, 0, 0);
			}

			break;
		case 3:
			setDoor(38, kCharacterMilos, 1, 14, 0);
			getCharacterCurrentParams(kCharacterMilos)[0] = 1;
			setDoor(38, kCharacterMilos, 1, 10, 9);
			break;
		case 4:
			playNIS(kEventMilosCompartmentVisitTyler);
			bumpCath(kCarRestaurant, 5, 255);
			setDoor(38, kCharacterMilos, 1, 10, 9);
			break;
		case 5:
			getCharacterCurrentParams(kCharacterMilos)[0] = 0;
			getCharacterCurrentParams(kCharacterMilos)[1] = 1;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_LookingForCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_LookingForCath);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 26;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_LookingForCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterMilos)[0] >= _gameTime || getCharacterCurrentParams(kCharacterMilos)[1]) {
			if (cathInCorridor(kCarGreenSleeping) || cathInCorridor(kCarRedSleeping)) {
				if (checkLoc(kCharacterCath, kCarGreenSleeping)) {
					getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
					MilosCall(&LogicManager::CONS_Milos_DoWalkSearchingForCath, 3, 540, 0, 0);
				} else {
					getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 3;
					MilosCall(&LogicManager::CONS_Milos_DoWalkSearchingForCath, 4, 9460, 0, 0);
				}
			}
		} else {
			getCharacterCurrentParams(kCharacterMilos)[1] = 1;

			getCharacter(kCharacterMilos).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
			fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		}

		break;
	case 12:
		getCharacterParams(kCharacterMilos, 8)[1] = 0;
		getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
		MilosCall(&LogicManager::CONS_Milos_DoWalkSearchingForCath, 4, 540, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8]) {
		case 1:
			if (getCharacterParams(kCharacterMilos, 8)[1]) {
				getCharacter(kCharacterMilos).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
				fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
			} else {
				endGraphics(kCharacterMilos);
			}

			break;
		case 2:
		case 3:
			if (getCharacterParams(kCharacterMilos, 8)[1]) {
				getCharacter(kCharacterMilos).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
				fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
			} else {
				endGraphics(kCharacterMilos);
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 4;
				MilosCall(&LogicManager::CONS_Milos_DoWait, 450, 0, 0, 0);
			}

			break;
		case 4:
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 5;
			MilosCall(&LogicManager::CONS_Milos_DoWalkSearchingForCath, 4, 540, 0, 0);
			break;
		case 5:
			if (getCharacterParams(kCharacterMilos, 8)[1]) {
				getCharacter(kCharacterMilos).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
				fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
			} else {
				endGraphics(kCharacterMilos);
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

void LogicManager::CONS_Milos_DoWalkSearchingForCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_DoWalkSearchingForCath);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 27;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_DoWalkSearchingForCath(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterMilos, getCharacterCurrentParams(kCharacterMilos)[0], getCharacterCurrentParams(kCharacterMilos)[1])) {
			getCharacter(kCharacterMilos).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
			fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		} else if (nearChar(kCharacterMilos, kCharacterCath, 1000) && !onLowPlatform(kCharacterCath) && !inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
			if (getCharacter(kCharacterMilos).characterPosition.car == kCarGreenSleeping || getCharacter(kCharacterMilos).characterPosition.car == kCarRedSleeping) {
				getCharacterParams(kCharacterMilos, 8)[1] = 1;

				getCharacter(kCharacterMilos).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
				fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
			}
		}

		break;
	case 12:
		if (walk(kCharacterMilos, getCharacterCurrentParams(kCharacterMilos)[0], getCharacterCurrentParams(kCharacterMilos)[1])) {
			getCharacter(kCharacterMilos).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterMilos, _functionsMilos[getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall]]);
			fedEx(kCharacterMilos, kCharacterMilos, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_StartPart4);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 28;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Milos_Conspiring(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMilos);
		getCharacter(kCharacterMilos).characterPosition.position = 3050;
		getCharacter(kCharacterMilos).characterPosition.location = 1;
		getCharacter(kCharacterMilos).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMilos).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_Conspiring(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_Conspiring);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 29;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_Conspiring(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterMilos)[0]) {
			if (getCharacterCurrentParams(kCharacterMilos)[1]) {
				CONS_Milos_InCharge(0, 0, 0, 0);
			} else {
				if (_gameTime > 2356200 && !getCharacterCurrentParams(kCharacterMilos)[2]) {
					getCharacterCurrentParams(kCharacterMilos)[2] = 1;
					playDialog(kCharacterMilos, "Mil4013", -1, 0);
					if (nearChar(kCharacterMilos, kCharacterCath, 2000))
						_globals[kGlobalOverheardMilosAndVesnaConspiring] = 1;
				}

				if (_gameTime > 2360700 && !getCharacterCurrentParams(kCharacterMilos)[3]) {
					getCharacterCurrentParams(kCharacterMilos)[3] = 1;
					playDialog(kCharacterMilos, "Mil4014", -1, 0);
					if (nearChar(kCharacterMilos, kCharacterCath, 2000))
						_globals[kGlobalOverheardMilosAndVesnaConspiring] = 1;
				}

				if (_gameTime > 2370600 && !getCharacterCurrentParams(kCharacterMilos)[4]) {
					getCharacterCurrentParams(kCharacterMilos)[4] = 1;
					playDialog(kCharacterMilos, "Mil4015", -1, 0);
					if (nearChar(kCharacterMilos, kCharacterCath, 2000))
						_globals[kGlobalOverheardMilosAndVesnaConspiring] = 1;
				}

				if (_gameTime > 2407500 && !getCharacterCurrentParams(kCharacterMilos)[5]) {
					getCharacterCurrentParams(kCharacterMilos)[5] = 1;
					send(kCharacterMilos, kCharacterVesna, 55996766, 0);
				}
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] == 1) {
			getCharacter(kCharacterMilos).characterPosition.location = 0;
			startCycOtis(kCharacterMilos, "611Cg");
			softBlockAtDoor(kCharacterMilos, 38);
			send(kCharacterMilos, kCharacterCond2, 88652208, 0);
		} else if (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] == 2) {
			softReleaseAtDoor(kCharacterMilos, 38);
			getCharacter(kCharacterMilos).characterPosition.location = 1;
			getCharacter(kCharacterMilos).characterPosition.position = 3050;
			endGraphics(kCharacterMilos);
			getCharacterCurrentParams(kCharacterMilos)[0] = 0;
		}

		break;
	case 122865568:
		getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
		MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "611Bg", 38, 0, 0);
		break;
	case 123852928:
		getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
		MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "611Dg", 38, 0, 0);
		break;
	case 135600432:
		getCharacterCurrentParams(kCharacterMilos)[1] = 1;
		break;
	case 221683008:
		if (whoRunningDialog(kCharacterMilos))
			fadeDialog(kCharacterMilos);

		getCharacterCurrentParams(kCharacterMilos)[0] = 1;
		send(kCharacterMilos, kCharacterCond2, 123199584, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_InCharge(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_InCharge);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 30;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_InCharge(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
		MilosCall(&LogicManager::CONS_Milos_CompLogic, 2410200, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8]) {
		case 1:
			send(kCharacterMilos, kCharacterIvo, 55996766, 0);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
			MilosCall(&LogicManager::CONS_Milos_CompLogic, 2412000, 0, 0, 0);
			break;
		case 2:
			send(kCharacterMilos, kCharacterSalko, 55996766, 0);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 3;
			MilosCall(&LogicManager::CONS_Milos_CompLogic, 2415600, 0, 0, 0);
			break;
		case 3:
			CONS_Milos_LeaveComp4(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_LeaveComp4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_LeaveComp4);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 31;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_LeaveComp4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
		MilosCall(&LogicManager::CONS_Milos_DoCorrOtis, "609CG", 38, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] == 1) {
			getCharacter(kCharacterMilos).characterPosition.location = 0;
			setDoor(38, kCharacterCath, 3, 10, 9);
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
			MilosCall(&LogicManager::CONS_Milos_DoWalk, 3, 540, 0, 0);
		} else if (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] == 2) {
			CONS_Milos_EndPart4(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_EndPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_EndPart4);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 32;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_EndPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterMilos);
		setDoor(38, kCharacterCath, 3, 10, 9);
		getCharacter(kCharacterMilos).characterPosition.location = 1;
		getCharacter(kCharacterMilos).characterPosition.position = 540;
		getCharacter(kCharacterMilos).characterPosition.car = kCarCoalTender;
		getCharacter(kCharacterMilos).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_StartPart5);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 33;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Milos_RunningTrain(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterMilos);
		getCharacter(kCharacterMilos).characterPosition.location = 1;
		getCharacter(kCharacterMilos).characterPosition.position = 540;
		getCharacter(kCharacterMilos).characterPosition.car = kCarCoalTender;
		getCharacter(kCharacterMilos).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_RunningTrain(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_RunningTrain);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 34;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_RunningTrain(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		if (_globals[kGlobalIsDayTime]) {
			endGame(0, 0, 52, true);
		} else {
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 6;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventTrainStopped, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8]) {
		case 1:
			playNIS(((_globals[kGlobalIsDayTime] == 0) + kEventLocomotiveMilosShovelingDay));
			cleanNIS();
			break;
		case 2:
			if (dialogRunning("MUS050"))
				fadeDialog("MUS050");

			if (dialogRunning("ARRIVE"))
				endDialog("ARRIVE");

			_engine->getSoundManager()->endAmbient();
			playNIS(((_globals[kGlobalIsDayTime] == 0) + kEventLocomotiveMilosDay));
			claimNISLink(kCharacterMilos);
			bumpCath(kCarCoalTender, 1, 255);
			break;
		case 3:
			playNIS(kEventLocomotiveAnnaStopsTrain);
			endGame(3, 108, 51, true);
			break;
		case 4:
			playNIS(kEventLocomotiveRestartTrain);
			playNIS(kEventLocomotiveOldBridge);
			_engine->getSoundManager()->startAmbient();
			_gameTime = 2983500;
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 5;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 1, 0, 0, 0);
			break;
		case 5:
			bumpCath(kCarCoalTender, 2, 1);
			send(kCharacterMilos, kCharacterAbbot, 135600432, 0);
			CONS_Milos_Dead(0, 0, 0, 0);
			break;
		case 6:
			playNIS(kEventTrainStopped);
			endGame(0, 1, 51, true);
			break;
		default:
			break;
		}

		break;
	case 168646401:
		if (_doneNIS[kEventLocomotiveMilosShovelingDay] || _doneNIS[kEventLocomotiveMilosShovelingNight]) {
			if (!_doneNIS[kEventLocomotiveMilosDay] && !_doneNIS[kEventLocomotiveMilosNight]) {
				if (_globals[kGlobalIsDayTime] && _gameTime < 2943000)
					_gameTime = 2943000;
				getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 2;
				MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventLocomotiveMilosDay, 0, 0);
			}
		} else {
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 1;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventLocomotiveMilosShovelingDay, 0, 0);
		}

		break;
	case 169773228:
		if (_globals[kGlobalIsDayTime]) {
			fadeDialog(kCharacterMilos);
			if (_gameTime < 2949300)
				_gameTime = 2949300;
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 4;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventLocomotiveRestartTrain, 0, 0);
		} else {
			getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall + 8] = 3;
			MilosCall(&LogicManager::CONS_Milos_SaveGame, 2, kEventLocomotiveAnnaStopsTrain, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Milos_Dead(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMilos).callParams[getCharacter(kCharacterMilos).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMilos, &LogicManager::HAND_Milos_Dead);
	getCharacter(kCharacterMilos).callbacks[getCharacter(kCharacterMilos).currentCall] = 35;

	params->clear();

	fedEx(kCharacterMilos, kCharacterMilos, 12, 0);
}

void LogicManager::HAND_Milos_Dead(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterMilos);
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsMilos[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Milos_DebugWalks,
	&LogicManager::HAND_Milos_DoSeqOtis,
	&LogicManager::HAND_Milos_DoCorrOtis,
	&LogicManager::HAND_Milos_DoBriefCorrOtis,
	&LogicManager::HAND_Milos_FinishSeqOtis,
	&LogicManager::HAND_Milos_DoDialog,
	&LogicManager::HAND_Milos_DoDialogFullVol,
	&LogicManager::HAND_Milos_SaveGame,
	&LogicManager::HAND_Milos_DoWait,
	&LogicManager::HAND_Milos_DoWalk,
	&LogicManager::HAND_Milos_CompLogic,
	&LogicManager::HAND_Milos_Birth,
	&LogicManager::HAND_Milos_DoOtis5009D,
	&LogicManager::HAND_Milos_KnockTyler,
	&LogicManager::HAND_Milos_AtDinner,
	&LogicManager::HAND_Milos_ReturnFromDinner,
	&LogicManager::HAND_Milos_InComp,
	&LogicManager::HAND_Milos_Asleep,
	&LogicManager::HAND_Milos_StartPart2,
	&LogicManager::HAND_Milos_OnRearPlat,
	&LogicManager::HAND_Milos_InPart2,
	&LogicManager::HAND_Milos_StartPart3,
	&LogicManager::HAND_Milos_OnRearPlat3,
	&LogicManager::HAND_Milos_Unfriendly,
	&LogicManager::HAND_Milos_Friendly,
	&LogicManager::HAND_Milos_LookingForCath,
	&LogicManager::HAND_Milos_DoWalkSearchingForCath,
	&LogicManager::HAND_Milos_StartPart4,
	&LogicManager::HAND_Milos_Conspiring,
	&LogicManager::HAND_Milos_InCharge,
	&LogicManager::HAND_Milos_LeaveComp4,
	&LogicManager::HAND_Milos_EndPart4,
	&LogicManager::HAND_Milos_StartPart5,
	&LogicManager::HAND_Milos_RunningTrain,
	&LogicManager::HAND_Milos_Dead
};

} // End of namespace LastExpress
