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
 * but WITHOUT ANY WARRANTY {} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "lastexpress/lastexpress.h"

namespace LastExpress {

void LogicManager::CONS_DemoAnna(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterAnna,
			_functionsDemoAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);

		break;
	case 1:
		CONS_DemoAnna_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoAnna_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoAnna_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoAnna_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoAnna_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::DemoAnnaCall(CALL_PARAMS) {
	getCharacter(kCharacterAnna).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_DemoAnna_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_DoSeqOtis);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 1;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsDemoAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAnna, (char *)&getCharacterCurrentParams(kCharacterAnna)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAnna_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_FinishSeqOtis);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 2;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_FinishSeqOtis(HAND_PARAMS) {
	if (msg->action == 3) {
		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsDemoAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
	}
}

void LogicManager::CONS_DemoAnna_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_DoCorrOtis);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[3]);
		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsDemoAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
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

void LogicManager::CONS_DemoAnna_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_WaitRCClear);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 4;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsDemoAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAnna_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_SaveGame);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 5;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsDemoAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	case 12:
		save(
			kCharacterAnna,
			getCharacterCurrentParams(kCharacterAnna)[0],
			getCharacterCurrentParams(kCharacterAnna)[1]
		);

		getCharacter(kCharacterAnna).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsDemoAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
		fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAnna_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_DoWalk);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 6;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (walk(kCharacterAnna, getCharacterCurrentParams(kCharacterAnna)[0], getCharacterCurrentParams(kCharacterAnna)[1])) {
			getCharacter(kCharacterAnna).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAnna, _functionsDemoAnna[getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall]]);
			fedEx(kCharacterAnna, kCharacterAnna, 18, 0);
		}

		break;
	case 5:
		playDialog(kCharacterCath, "CAT1001", -1, 0);
		break;
	case 6:
		playChrExcuseMe(kCharacterAnna, kCharacterCath, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAnna_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_Birth);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 7;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2254500 && !getCharacterCurrentParams(kCharacterAnna)[0]) {
			getCharacterCurrentParams(kCharacterAnna)[0] = 1;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
			DemoAnnaCall(&LogicManager::CONS_DemoAnna_GoBagg, 0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterAnna).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterAnna).characterPosition.position = 4070;
		getCharacter(kCharacterAnna).characterPosition.location = 1;
		getCharacter(kCharacterAnna).clothes = 2;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAnna_GoBagg(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_GoBagg);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 8;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_GoBagg(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(53, kCharacterCath, 1, 10, 9);
		getCharacter(kCharacterAnna).characterPosition.location = 0;
		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		DemoAnnaCall(&LogicManager::CONS_DemoAnna_DoCorrOtis, "625Bf", 37, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			setDoor(37, kCharacterCath, 1, 10, 9);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			DemoAnnaCall(&LogicManager::CONS_DemoAnna_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
			DemoAnnaCall(&LogicManager::CONS_DemoAnna_WaitRCClear, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAnna).characterPosition.position = 1540;
			getCharacter(kCharacterAnna).characterPosition.location = 0;
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 4;
			DemoAnnaCall(&LogicManager::CONS_DemoAnna_DoSeqOtis, "802US", 0, 0, 0);
			break;
		case 4:
			startSeqOtis(kCharacterAnna, "802UD");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterAnna);

			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 5;
			DemoAnnaCall(&LogicManager::CONS_DemoAnna_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 5:
			_globals[kGlobalUnknownDebugFlag] = 1;
			endGraphics(kCharacterAnna);
			CONS_DemoAnna_InBagg(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAnna_InBagg(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_InBagg);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 9;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_InBagg(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAnna)[0] && _gameTime > 2265300 && !getCharacterCurrentParams(kCharacterAnna)[1]) {
			getCharacterCurrentParams(kCharacterAnna)[1] = 1;
			if (checkLoc(kCharacterCath, 6)) {
				CONS_DemoAnna_BaggageFight(0, 0, 0, 0);
			} else {
				send(kCharacterAnna, kCharacterVesna, 189299008, 0);
				CONS_DemoAnna_DeadBagg(0, 0, 0, 0);
			}
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

void LogicManager::CONS_DemoAnna_DeadBagg(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_DeadBagg);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 10;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_DeadBagg(HAND_PARAMS) {
	switch (msg->action) {
	case 18:
		if (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] == 1) {
			playNIS(kEventAnnaKilled);
			endGame(1, 2259000, 58, true);
		}

		break;
	case 272177921:
		if (dialogRunning("MUS021"))
			fadeDialog("MUS021");

		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		DemoAnnaCall(&LogicManager::CONS_DemoAnna_SaveGame, 2, kEventAnnaKilled, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoAnna_BaggageFight(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_BaggageFight);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 11;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_BaggageFight(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterAnna);

		if (dialogRunning("MUS021"))
			fadeDialog("MUS021");

		getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 1;
		DemoAnnaCall(&LogicManager::CONS_DemoAnna_SaveGame, 2, kEventAnnaBaggageArgument, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8]) {
		case 1:
			playNIS(kEventAnnaBaggageArgument);
			getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 2;
			DemoAnnaCall(&LogicManager::CONS_DemoAnna_SaveGame, 1, 0, 0, 0);
			break;
		case 2:
			getCharacterCurrentParams(kCharacterAnna)[0] = playFight(2002);
			if (getCharacterCurrentParams(kCharacterAnna)[0]) {
				endGame(0, 0, 0, getCharacterCurrentParams(kCharacterAnna)[0] == 1);
			} else {
				_gameTime = 2268000;
				getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall + 8] = 3;
				DemoAnnaCall(&LogicManager::CONS_DemoAnna_SaveGame, 2, kEventAnnaBaggagePart2, 0, 0);
			}

			break;
		case 3:
			playNIS(kEventAnnaBaggagePart2);
			_globals[kGlobalAnnaIsInBaggageCar] = 0;
			forceJump(kCharacterVesna, &LogicManager::CONS_DemoVesna_InComp);
			winGame();
			break;
		default:
			break;
		}

		break;

	default:
		break;
	}
}

void LogicManager::CONS_DemoAnna_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_StartPart2);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 12;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoAnna_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_StartPart3);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 13;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoAnna_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_StartPart4);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 14;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoAnna_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAnna).callParams[getCharacter(kCharacterAnna).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAnna, &LogicManager::HAND_DemoAnna_StartPart5);
	getCharacter(kCharacterAnna).callbacks[getCharacter(kCharacterAnna).currentCall] = 15;

	params->clear();

	fedEx(kCharacterAnna, kCharacterAnna, 12, 0);
}

void LogicManager::HAND_DemoAnna_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoAnna[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoAnna_DoSeqOtis,
	&LogicManager::HAND_DemoAnna_FinishSeqOtis,
	&LogicManager::HAND_DemoAnna_DoCorrOtis,
	&LogicManager::HAND_DemoAnna_WaitRCClear,
	&LogicManager::HAND_DemoAnna_SaveGame,
	&LogicManager::HAND_DemoAnna_DoWalk,
	&LogicManager::HAND_DemoAnna_Birth,
	&LogicManager::HAND_DemoAnna_GoBagg,
	&LogicManager::HAND_DemoAnna_InBagg,
	&LogicManager::HAND_DemoAnna_DeadBagg,
	&LogicManager::HAND_DemoAnna_BaggageFight,
	&LogicManager::HAND_DemoAnna_StartPart2,
	&LogicManager::HAND_DemoAnna_StartPart3,
	&LogicManager::HAND_DemoAnna_StartPart4,
	&LogicManager::HAND_DemoAnna_StartPart5
};

} // End of namespace LastExpress
