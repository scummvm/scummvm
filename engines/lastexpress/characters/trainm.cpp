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

void LogicManager::CONS_TrainM(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterTrainM,
			_functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]
		);

		break;
	case 1:
		CONS_TrainM_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_TrainM_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_TrainM_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_TrainM_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_TrainM_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::TrainMCall(CALL_PARAMS) {
	getCharacter(kCharacterTrainM).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_TrainM_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DebugWalks);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 1;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterTrainM, kCarGreenSleeping, getCharacterCurrentParams(kCharacterTrainM)[0])) {
			if (getCharacterCurrentParams(kCharacterTrainM)[0] == 10000) {
				getCharacterCurrentParams(kCharacterTrainM)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterTrainM)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterTrainM).characterPosition.position = 0;
		getCharacter(kCharacterTrainM).characterPosition.location = 0;
		getCharacter(kCharacterTrainM).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterTrainM)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoSeqOtis);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterTrainM).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
		fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterTrainM)[3]) {
			playChrExcuseMe(kCharacterTrainM, 0, 0);
			getCharacterCurrentParams(kCharacterTrainM)[3] = 1;
		}

		break;
	case 12:
		startSeqOtis(kCharacterTrainM, (char *)&getCharacterCurrentParams(kCharacterTrainM));
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_FinishSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_FinishSeqOtis);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 3;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_FinishSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacter(kCharacterTrainM).direction != 4) {
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		}

		break;
	case 3:
		getCharacter(kCharacterTrainM).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
		fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		break;
	case 5:
		if (!getCharacterCurrentParams(kCharacterTrainM)[0]) {
			playChrExcuseMe(kCharacterTrainM, kCharacterCath, 0);
			getCharacterCurrentParams(kCharacterTrainM)[0] = 1;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoDialog);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterTrainM).currentCall--;
		_engine->getMessageManager()->setMessageHandle(9, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
		fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		break;
	case 12:
		playDialog(kCharacterTrainM, (char *)&getCharacterCurrentParams(kCharacterTrainM), -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoDialogFullVol(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoDialogFullVol);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoDialogFullVol(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterTrainM).currentCall--;
		_engine->getMessageManager()->setMessageHandle(9, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
		fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		break;
	case 12:
		playDialog(kCharacterTrainM, (char *)&getCharacterCurrentParams(kCharacterTrainM), 16, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_WaitRCClear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_WaitRCClear);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 6;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_WaitRCClear(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	case 12:
		if (rcClear()) {
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_SaveGame);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 7;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterTrainM).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
		fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		break;
	case 12:
		save(
			kCharacterTrainM,
			getCharacterCurrentParams(kCharacterTrainM)[0],
			getCharacterCurrentParams(kCharacterTrainM)[1]
		);

		getCharacter(kCharacterTrainM).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
		fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoWalk);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 8;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterTrainM, getCharacterCurrentParams(kCharacterTrainM)[0], getCharacterCurrentParams(kCharacterTrainM)[1])) {
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		}

		break;
	case 5:
		if (!whoRunningDialog(kCharacterTrainM)) {
			playDialog(kCharacterCath, "TRA1113", getVolume(kCharacterTrainM), 0);
		}

		break;
	case 6:
		playChrExcuseMe(kCharacterTrainM, 0, 0);
		break;
	case 12:
		if (walk(kCharacterTrainM, getCharacterCurrentParams(kCharacterTrainM)[0], getCharacterCurrentParams(kCharacterTrainM)[1])) {
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_Announce(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_Announce);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 9;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_Announce(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(104, kCharacterCath, 0, 0, 9);
		setDoor(105, kCharacterCath, 0, 0, 9);

		if (inLowBaggage(kCharacterCath)) {
			playNIS(kEventVergesBaggageCarOffLimits);
		} else {
			if (!inKitchen(kCharacterCath)) {
				takeItem(kItem9);
				getCharacter(kCharacterTrainM).characterPosition.car = kCarRestaurant;
				getCharacter(kCharacterTrainM).characterPosition.position = 5900;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
				TrainMCall(&LogicManager::CONS_TrainM_WaitRCClear, 0, 0, 0, 0);
				return;
			}

			playNIS(kEventVergesCanIHelpYou);
		}

		playDialog(kCharacterCath, "BUMP", -1, 0);
		bumpCath(kCarRestaurant, 65, 255);
		takeItem(kItem9);

		getCharacter(kCharacterTrainM).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterTrainM).characterPosition.position = 5900;
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;

		TrainMCall(&LogicManager::CONS_TrainM_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).characterPosition.position = 5800;
			getCharacter(kCharacterTrainM).characterPosition.location = 0;
			playDialog(kCharacterTrainM, (char *)&getCharacterCurrentParams(kCharacterTrainM), -1, 0);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoSeqOtis, "813DD", 0, 0, 0);
			break;
		case 2:
			if (!whoRunningDialog(kCharacterTrainM))
				playDialog(kCharacterTrainM, (char *)&getCharacterCurrentParams(kCharacterTrainM), -1, 0);

			startSeqOtis(kCharacterTrainM, "813DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterTrainM);

			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_FinishSeqOtis, 0, 0, 0, 0);

			break;
		case 3:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_DoAnnounceWalk, 3, 540, (char *)&getCharacterCurrentParams(kCharacterTrainM), 0);
			break;
		case 4:
			endGraphics(kCharacterTrainM);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_DoWait, 225, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoAnnounceWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoAnnounceWalk);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 10;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	strncpy((char *)&params->parameters[2], param3.stringParam, 12);

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoAnnounceWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!getCharacterCurrentParams(kCharacterTrainM)[6] && !whoRunningDialog(kCharacterTrainM)) {
			playDialog(kCharacterTrainM, (char *)&getCharacterCurrentParams(kCharacterTrainM)[2], -1, 0);
			getCharacterCurrentParams(kCharacterTrainM)[6] = 1;
		}

		if (walk(kCharacterTrainM, getCharacterCurrentParams(kCharacterTrainM)[0], getCharacterCurrentParams(kCharacterTrainM)[1])) {
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterTrainM)[5]) {
			if (getCharacterCurrentParams(kCharacterTrainM)[7] || (getCharacterCurrentParams(kCharacterTrainM)[7] = _realTime + 75, _realTime != -75)) {
				if (getCharacterCurrentParams(kCharacterTrainM)[7] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterTrainM)[7] = 0x7FFFFFFF;
			}

			playDialog(kCharacterTrainM, (char *)&getCharacterCurrentParams(kCharacterTrainM)[2], -1, 0);
			getCharacterCurrentParams(kCharacterTrainM)[5] = 0;
			getCharacterCurrentParams(kCharacterTrainM)[7] = 0;
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterTrainM)[5] = 1;
		break;
	case 12:
		if (!whoRunningDialog(kCharacterTrainM)) {
			playDialog(kCharacterTrainM, (char *)&getCharacterCurrentParams(kCharacterTrainM)[2], -1, 0);
			getCharacterCurrentParams(kCharacterTrainM)[6] = 1;
		}

		if (walk(kCharacterTrainM, getCharacterCurrentParams(kCharacterTrainM)[0], getCharacterCurrentParams(kCharacterTrainM)[1])) {
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_WalkBackToOffice(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_WalkBackToOffice);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 11;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_WalkBackToOffice(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 5, 540, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_WaitRCClear, 0, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterTrainM).characterPosition.position = 1540;
			getCharacter(kCharacterTrainM).characterPosition.location = 0;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_DoSeqOtis, "813US", 0, 0, 0);

			break;
		case 3:
			startSeqOtis(kCharacterTrainM, "813UD");

			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterTrainM);

			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_FinishSeqOtis, 0, 0, 0, 0);

			break;
		case 4:
			endGraphics(kCharacterTrainM);

			if (inOffice(kCharacterCath)) {
				playNIS(kEventVergesEscortToDiningCar);
			} else if (inLowBaggage(kCharacterCath)) {
				playNIS(kEventVergesBaggageCarOffLimits);
			} else {
				if (!inKitchen(kCharacterCath)) {
					dropItem(kCharacterTrainM, 1);
					getCharacter(kCharacterTrainM).characterPosition.car = kCarBaggage;
					getCharacter(kCharacterTrainM).characterPosition.position = 5000;
					setDoor(104, kCharacterTrainM, 0, 0, 9);
					setDoor(105, kCharacterTrainM, 0, 0, 9);

					getCharacter(kCharacterTrainM).currentCall--;
					_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
					fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);

					break;
				}

				playNIS(kEventVergesCanIHelpYou);
			}

			playDialog(kCharacterCath, "BUMP", -1, 0);
			bumpCath(kCarRestaurant, 65, 255);
			dropItem(kCharacterTrainM, 1);
			getCharacter(kCharacterTrainM).characterPosition.car = kCarBaggage;
			getCharacter(kCharacterTrainM).characterPosition.position = 5000;
			setDoor(104, kCharacterTrainM, 0, 0, 9);
			setDoor(105, kCharacterTrainM, 0, 0, 9);

			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);

			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_LeaveOffice(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_LeaveOffice);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 12;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_LeaveOffice(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(104, kCharacterCath, 0, 0, 9);
		setDoor(105, kCharacterCath, 0, 0, 9);
		if (inLowBaggage(kCharacterCath)) {
			playNIS(kEventVergesBaggageCarOffLimits);
		} else {
			if (!inKitchen(kCharacterCath)) {
				takeItem(kItem9);
				getCharacter(kCharacterTrainM).characterPosition.car = kCarRestaurant;
				getCharacter(kCharacterTrainM).characterPosition.position = 5900;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
				TrainMCall(&LogicManager::CONS_TrainM_WaitRCClear, 0, 0, 0, 0);
				break;
			}

			playNIS(kEventVergesCanIHelpYou);
		}

		playDialog(0, "BUMP", -1, 0);
		bumpCath(kCarRestaurant, 65, 255);
		takeItem(kItem9);
		getCharacter(kCharacterTrainM).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterTrainM).characterPosition.position = 5900;
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).characterPosition.position = 5800;
			getCharacter(kCharacterTrainM).characterPosition.location = 0;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoSeqOtis, "813DD", 0, 0, 0);
			break;
		case 2:
			startSeqOtis(kCharacterTrainM, "813DS");
			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterTrainM);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterTrainM).characterPosition.position = 850;
			endGraphics(kCharacterTrainM);
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_KickCathOut(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_KickCathOut);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 13;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_KickCathOut(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_SaveGame, 2, kEventVergesSuitcase, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] == 1) {
			if (_doneNIS[kEventVergesSuitcase] || _doneNIS[kEventVergesSuitcaseNight] || _doneNIS[kEventVergesSuitcaseOtherEntry] || _doneNIS[kEventVergesSuitcaseNightOtherEntry]) {
				getCharacterCurrentParams(kCharacterTrainM)[1] = 1;
			}

			if (isNight() && _globals[kGlobalChapter] != 1)
				getCharacterCurrentParams(kCharacterTrainM)[1] = 1;

			if (getCharacterCurrentParams(kCharacterTrainM)[0]) {
				if (isNight()) {
					if (getCharacterCurrentParams(kCharacterTrainM)[1] == 0) {
						playNIS(kEventVergesSuitcaseNightOtherEntry);
					} else {
						playNIS(kEventVergesSuitcaseNightOtherEntryStart);
					}
				} else {
					if (getCharacterCurrentParams(kCharacterTrainM)[1] == 0) {
						playNIS(kEventVergesSuitcaseOtherEntry);
					} else {
						playNIS(kEventVergesSuitcaseOtherEntryStart);
					}
				}
			} else if (isNight()) {
				if (getCharacterCurrentParams(kCharacterTrainM)[1] == 0) {
					playNIS(kEventVergesSuitcaseNight);
				} else {
					playNIS(kEventVergesSuitcaseNightStart);
				}
			} else {
				if (getCharacterCurrentParams(kCharacterTrainM)[1] == 0) {
					playNIS(kEventVergesSuitcase);
				} else {
					playNIS(kEventVergesSuitcaseStart);
				}
			}

			endGraphics(kCharacterTrainM);
			bumpCath(kCarBaggage, 91, 255);
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoWait);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 14;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterTrainM)[1] ||
			(getCharacterCurrentParams(kCharacterTrainM)[1] = _gameTime + getCharacterCurrentParams(kCharacterTrainM)[0], (_gameTime + getCharacterCurrentParams(kCharacterTrainM)[0] != 0))) {
			if (getCharacterCurrentParams(kCharacterTrainM)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterTrainM)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterTrainM).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
		fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoCond(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoCond);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 15;

	params->clear();

	params->parameters[0] = param1.intParam;

	strncpy((char *)&params->parameters[1], param2.stringParam, 12);

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoCond(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterTrainM)[4] && getCharacterCurrentParams(kCharacterTrainM)[5]) {
			send(kCharacterTrainM, getCharacterCurrentParams(kCharacterTrainM)[0], 125499160, 0);
			if (!checkCathDir(kCarGreenSleeping, 2) && !checkCathDir(kCarRedSleeping, 2))
				getCharacter(kCharacterTrainM).characterPosition.position = 2088;

			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterTrainM)[4] = 1;
		break;
	case 12:
		startCycOtis(kCharacterTrainM, "620F");
		send(kCharacterTrainM, getCharacterCurrentParams(kCharacterTrainM)[0], 171394341, 0);
		break;
	case 155853632:
		getCharacterCurrentParams(kCharacterTrainM)[5] = 1;
		break;
	case 202558662:
		startCycOtis(kCharacterTrainM, "620E");
		playDialog(kCharacterTrainM, (char *)&getCharacterCurrentParams(kCharacterTrainM)[1], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoCondDoubl(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoCondDoubl);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 16;

	params->clear();

	params->parameters[0] = param1.intParam;

	strncpy((char *)&params->parameters[1], param2.stringParam, 12);
	strncpy((char *)&params->parameters[4], param3.stringParam, 12);

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoCondDoubl(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterTrainM)[8] >= 2 && getCharacterCurrentParams(kCharacterTrainM)[7]) {
			send(kCharacterTrainM, getCharacterCurrentParams(kCharacterTrainM)[0], 125499160, 0);
			if (!checkCathDir(kCarGreenSleeping, 2) && !checkCathDir(kCarRedSleeping, 2))
				getCharacter(kCharacterTrainM).characterPosition.position = 2088;

			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		}

		break;
	case 2:
		getCharacterCurrentParams(kCharacterTrainM)[8]++;
		if (getCharacterCurrentParams(kCharacterTrainM)[8] == 1)
			playDialog(kCharacterTrainM, (char *)&getCharacterCurrentParams(kCharacterTrainM)[4], -1, 0);

		break;
	case 12:
		startCycOtis(kCharacterTrainM, "620F");
		send(kCharacterTrainM, getCharacterCurrentParams(kCharacterTrainM)[0], 171394341, 0);
		break;
	case 155853632:
		getCharacterCurrentParams(kCharacterTrainM)[7] = 1;
		break;
	case 202558662:
		startCycOtis(kCharacterTrainM, "620E");
		playDialog(kCharacterTrainM, (char *)&getCharacterCurrentParams(kCharacterTrainM)[1], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoCond1ListMessage(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoCond1ListMessage);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 17;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoCond1ListMessage(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 3, 2000, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond, 3, "TRA1291", 0, 0);
			break;
		case 3:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 4:
			getCharacterParams(kCharacterTrainM, 8)[2] = 0;
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(9, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_Birth);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 18;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterTrainM)[0]) {
			getCharacterCurrentParams(kCharacterTrainM)[0] = 1;
			CONS_TrainM_InOffice(0, 0, 0, 0);
		}

		break;
	case 12:
		autoMessage(kCharacterTrainM, 191337656, 0);
		autoMessage(kCharacterTrainM, 226031488, 1);
		autoMessage(kCharacterTrainM, 339669520, 1);
		autoMessage(kCharacterTrainM, 167854368, 4);
		autoMessage(kCharacterTrainM, 158617345, 2);
		autoMessage(kCharacterTrainM, 168255788, 3);
		autoMessage(kCharacterTrainM, 201431954, 5);
		autoMessage(kCharacterTrainM, 168187490, 6);

		getCharacter(kCharacterTrainM).characterPosition.car = kCarBaggage;
		getCharacter(kCharacterTrainM).characterPosition.position = 5000;
		getCharacter(kCharacterTrainM).characterPosition.location = 0;

		setDoor(104, kCharacterTrainM, 0, 0, 9);
		setDoor(105, kCharacterTrainM, 0, 0, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoMadeBedsMessages(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoMadeBedsMessages);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 19;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoMadeBedsMessages(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 4, 2000, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond, 4, "TRA1202", 0, 0);
			break;
		case 3:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 3, 2000, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond, 3, "TRA1201", 0, 0);
			break;
		case 5:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoMissingListMessages(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoMissingListMessages);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 20;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoMissingListMessages(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 4, 2000, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond, 4, "TRA1205", 0, 0);
			break;
		case 3:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 3, 2000, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond, 3, "TRA1206", 0, 0);
			break;
		case 5:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoAfterPoliceMessages(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoAfterPoliceMessages);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 21;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoAfterPoliceMessages(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 4, 2000, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond, 4, "TRA1250", 0, 0);
			break;
		case 3:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 3, 2000, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond, 3, "TRA1251", 0, 0);
			break;
		case 5:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoHWMessage(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoHWMessage);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 22;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoHWMessage(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 3, 2000, 0, 0);
			break;
		case 2:
			if (_doneNIS[kEventMertensAskTylerCompartment] || _doneNIS[kEventMertensAskTylerCompartmentD] || _doneNIS[kEventMertensAugustWaiting]) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
				TrainMCall(&LogicManager::CONS_TrainM_DoCondDoubl, 3, "TRA1200", "TRA1201", 0);
			} else {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
				TrainMCall(&LogicManager::CONS_TrainM_DoCondDoubl, 3, "TRA1200A", "TRA1201", 0);
			}
			break;
		case 3:
		case 4:
			send(kCharacterTrainM, kCharacterCond1, 169633856, 0);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoSeizure(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoSeizure);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 23;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoSeizure(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		takeItem(kItem9);
		getCharacter(kCharacterTrainM).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterTrainM).characterPosition.position = 8200;
		getCharacter(kCharacterTrainM).characterPosition.location = 1;
		break;
	case 191477936:
		getCharacter(kCharacterTrainM).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterTrainM).characterPosition.position = 8200;
		getCharacter(kCharacterTrainM).characterPosition.location = 0;
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoPoliceDoneDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoPoliceDoneDialog);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 24;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoPoliceDoneDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (nearChar(kCharacterTrainM, kCharacterCath, 1000) && !getCharacter(kCharacterCath).characterPosition.location) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
			TrainMCall(&LogicManager::CONS_TrainM_SaveGame, 2, kEventGendarmesArrestation, 0, 0);
		}

		break;
	case 2:
		getCharacter(kCharacterTrainM).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
		fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
		break;
	case 12:
		playDialog(kCharacterTrainM, "POL1101", 16, 0);
		break;
	case 18:
		if (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] == 1) {
			fadeDialog(kCharacterTrainM);
			playNIS(kEventGendarmesArrestation);
			endGame(0, 1, 57, true);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_BoardPolice(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_BoardPolice);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 25;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_BoardPolice(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_SaveGame, 1, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			takeItem(kItem9);
			if (inOuterSanctum(kCharacterCath)) {
				if (_doneNIS[kEventKronosConversation]) {
					_globals[kGlobalPoliceHasBoardedAndGone] = 1;
					getCharacter(kCharacterTrainM).characterPosition.car = kCarGreenSleeping;
					getCharacter(kCharacterTrainM).characterPosition.position = 540;
					getCharacter(kCharacterTrainM).characterPosition.location = 0;
					_timeSpeed = 3;
					send(kCharacterTrainM, kCharacterMaster, 169629818, 0);
					getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
					TrainMCall(&LogicManager::CONS_TrainM_DoPoliceDoneDialog, 0, 0, 0, 0);
				} else {
					getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
					TrainMCall(&LogicManager::CONS_TrainM_DoWait, 150, 0, 0, 0);
				}

				break;
			}

			if (getCharacter(kCharacterCath).characterPosition.car > kCarRedSleeping || (getCharacter(kCharacterCath).characterPosition.car == kCarRedSleeping && getCharacter(kCharacterCath).characterPosition.position > 9270)) {
				playDialog(0, "BUMP", -1, 0);
				bumpCath(kCarRedSleeping, 40, 255);
				getCharacter(kCharacterTrainM).characterPosition.car = kCarRedSleeping;
				getCharacter(kCharacterTrainM).characterPosition.position = 9270;
			} else {
				if (getCharacter(kCharacterCath).characterPosition.car < kCarGreenSleeping || (getCharacter(kCharacterCath).characterPosition.car == kCarGreenSleeping && getCharacter(kCharacterCath).characterPosition.position < 4840)) {
					playDialog(0, "BUMP", -1, 0);
					bumpCathFDoor(5);
				}

				getCharacter(kCharacterTrainM).characterPosition.car = kCarGreenSleeping;
				getCharacter(kCharacterTrainM).characterPosition.position = 850;
			}

			getCharacter(kCharacterTrainM).characterPosition.location = 0;
			setDoor(55, kCharacterCath, 1, 0, 1);
			setDoor(36, kCharacterCath, 1, 10, 9);

			if (cathOutRebeccaWindow())
				bumpCath(kCarRedSleeping, 49, 255);

			if (inComp(kCharacterCath, kCarRedSleeping, 4840) || inComp(kCharacterCath, kCarRedSleeping, 4455)) {
				if (isNight()) {
					playNIS(kEventCathTurningNight);
				} else {
					playNIS(kEventCathTurningDay);
				}

				playDialog(kCharacterCath, "BUMP", -1, 0);
				bumpCathFDoor(36);
			}

			send(kCharacterTrainM, kCharacterPolice, 169499649, 0);
			_globals[kGlobalPoliceHasBoardedAndGone] = 1;
			_timeSpeed = 1;
			if (getCharacter(kCharacterTrainM).characterPosition.car == kCarRedSleeping) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
				TrainMCall(&LogicManager::CONS_TrainM_DoAnnounceWalk, 3, 540, "TRA1005", 0);
			} else {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 7;
				TrainMCall(&LogicManager::CONS_TrainM_DoAnnounceWalk, 4, 9460, "TRA1005", 0);
			}

			break;
		case 2:
			if (_doneNIS[kEventKronosConversation]) {
				_globals[kGlobalPoliceHasBoardedAndGone] = 1;
				getCharacter(kCharacterTrainM).characterPosition.car = kCarGreenSleeping;
				getCharacter(kCharacterTrainM).characterPosition.position = 540;
				getCharacter(kCharacterTrainM).characterPosition.location = 0;
				_timeSpeed = 3;
				send(kCharacterTrainM, kCharacterMaster, 169629818, 0);
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
				TrainMCall(&LogicManager::CONS_TrainM_DoPoliceDoneDialog, 0, 0, 0, 0);
			} else {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
				TrainMCall(&LogicManager::CONS_TrainM_DoWait, 150, 0, 0, 0);
			}

			break;
		case 3:
			send(kCharacterTrainM, kCharacterCond2, 168254872, 0);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_DoAnnounceWalk, 4, 9460, "TRA1006", 0);
			break;
		case 4:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 5:
		case 11:
			getCharacterParams(kCharacterTrainM, 8)[6] = 0;
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);

			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		case 6:
		case 7:
			endGraphics(kCharacterTrainM);
			break;
		case 8:
			send(kCharacterTrainM, kCharacterMaster, 169629818, 0);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 9;
			TrainMCall(&LogicManager::CONS_TrainM_DoPoliceDoneDialog, 0, 0, 0, 0);
			break;
		case 9:
			setDoor(55, kCharacterCath, 0, 0, 1);
			setDoor(36, kCharacterCath, 0, 10, 9);
			send(kCharacterTrainM, kCharacterCond2, 168254872, 0);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 10;
			TrainMCall(&LogicManager::CONS_TrainM_DoAnnounceWalk, 3, 540, "TRA1006", 0);
			break;
		case 10:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 11;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 168710784:
		getCharacter(kCharacterTrainM).characterPosition.car = kCarGreenSleeping;

		if (getCharacter(kCharacterTrainM).characterPosition.car != kCarGreenSleeping)
			getCharacter(kCharacterTrainM).characterPosition.car = kCarRedSleeping;

		getCharacter(kCharacterTrainM).characterPosition.position = 8200;
		getCharacter(kCharacterTrainM).characterPosition.location = 0;

		_timeSpeed = 3;

		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 8;
		TrainMCall(&LogicManager::CONS_TrainM_SaveGame, 1, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_InOffice(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_InOffice);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 26;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_InOffice(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterParams(kCharacterTrainM, 8)[5]) {
			getCharacterCurrentParams(kCharacterTrainM)[0] = 1;
			getCharacterCurrentParams(kCharacterTrainM)[1] = 1;
			getCharacterCurrentParams(kCharacterTrainM)[2] = 1;
			getCharacterCurrentParams(kCharacterTrainM)[3] = 1;
			getCharacterCurrentParams(kCharacterTrainM)[4] = 1;
			getCharacterParams(kCharacterTrainM, 8)[5] = 0;
			getCharacterCurrentParams(kCharacterTrainM)[5] = 1;
		}

		if (getCharacterParams(kCharacterTrainM, 8)[1]) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
			TrainMCall(&LogicManager::CONS_TrainM_DoSeizure, 0, 0, 0, 0);
			break;
		}

		if (inOffice(kCharacterCath)) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_KickCathOut, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterTrainM, 8)[6]) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_BoardPolice, 0, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterTrainM)[5]) {
			if (getCharacterParams(kCharacterTrainM, 8)[4] && !getCharacterCurrentParams(kCharacterTrainM)[1]) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 13;
				TrainMCall(&LogicManager::CONS_TrainM_DoAfterPoliceMessages, 0, 0, 0, 0);
				break;
			}

			if (cathHasItem(kItemPassengerList) && !getCharacterCurrentParams(kCharacterTrainM)[2] && !(_gameTime >= 1134000 && _gameTime <= 1156500)) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 14;
				TrainMCall(&LogicManager::CONS_TrainM_DoMissingListMessages, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterTrainM, 8)[2] && !getCharacterCurrentParams(kCharacterTrainM)[3] && !(_gameTime >= 1134000 && _gameTime <= 1156500)) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 15;
				TrainMCall(&LogicManager::CONS_TrainM_DoCond1ListMessage, 0, 0, 0, 0);
				break;
			}

			if (getCharacterParams(kCharacterTrainM, 8)[0] && !getCharacterCurrentParams(kCharacterTrainM)[4] && (_gameTime < 1134000 || _gameTime > 1156500)) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 16;
				TrainMCall(&LogicManager::CONS_TrainM_DoHWMessage, 0, 0, 0, 0);
			}

			break;
		}

		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterTrainM)[6]) {
			getCharacterCurrentParams(kCharacterTrainM)[6] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "TRA1001", 0, 0, 0);
			break;
		}

		if (_gameTime > 1089000 && !getCharacterCurrentParams(kCharacterTrainM)[7] && (getCharacterCurrentParams(kCharacterTrainM)[7] = 1, !getCharacterCurrentParams(kCharacterTrainM)[4])) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 1107000 && !getCharacterCurrentParams(kCharacterTrainM)[8]) {
			getCharacterCurrentParams(kCharacterTrainM)[8] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 9;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "TRA1001A", 0, 0, 0);
			break;
		}

		if (_gameTime > 1134000 && !getCharacterCurrentParams(kCharacterTrainM)[9]) {
			getCharacterCurrentParams(kCharacterTrainM)[9] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 10;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "TRA1002", 0, 0, 0);
			break;
		}

		if (_gameTime > 1165500 && !getCharacterCurrentParams(kCharacterTrainM)[10]) {
			getCharacterCurrentParams(kCharacterTrainM)[10] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 11;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "TRA1003", 0, 0, 0);
			break;
		}

		if (_gameTime > 1225800 && !getCharacterCurrentParams(kCharacterTrainM)[11]) {
			getCharacterCurrentParams(kCharacterTrainM)[11] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 12;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "TRA1004", 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterTrainM, 8)[4] && !getCharacterCurrentParams(kCharacterTrainM)[1]) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 13;
			TrainMCall(&LogicManager::CONS_TrainM_DoAfterPoliceMessages, 0, 0, 0, 0);
			break;
		}

		if (cathHasItem(kItemPassengerList) && !getCharacterCurrentParams(kCharacterTrainM)[2] && !(_gameTime >= 1134000 && _gameTime <= 1156500)) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 14;
			TrainMCall(&LogicManager::CONS_TrainM_DoMissingListMessages, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterTrainM, 8)[2] && !getCharacterCurrentParams(kCharacterTrainM)[3] && !(_gameTime >= 1134000 && _gameTime <= 1156500)) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 15;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond1ListMessage, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterTrainM, 8)[0] && !getCharacterCurrentParams(kCharacterTrainM)[4] && (_gameTime < 1134000 || _gameTime > 1156500)) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 16;
			TrainMCall(&LogicManager::CONS_TrainM_DoHWMessage, 0, 0, 0, 0);
		}

		break;
	case 9:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 17;
		TrainMCall(&LogicManager::CONS_TrainM_KickCathOut, (msg->param.intParam == 105) ? 1 : 0, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterTrainM).characterPosition.car = kCarBaggage;
		getCharacter(kCharacterTrainM).characterPosition.position = 5000;
		getCharacter(kCharacterTrainM).characterPosition.location = 0;
		endGraphics(kCharacterTrainM);
		dropItem(kItem9, 1);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			if (inOffice(kCharacterCath)) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
				TrainMCall(&LogicManager::CONS_TrainM_KickCathOut, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacterParams(kCharacterTrainM, 8)[6]) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
				TrainMCall(&LogicManager::CONS_TrainM_BoardPolice, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (getCharacterCurrentParams(kCharacterTrainM)[5]) {
				if (getCharacterParams(kCharacterTrainM, 8)[4] && !getCharacterCurrentParams(kCharacterTrainM)[1]) {
					getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 13;
					TrainMCall(&LogicManager::CONS_TrainM_DoAfterPoliceMessages, 0, 0, 0, 0);
					break;
				}

				if (cathHasItem(kItemPassengerList) && !getCharacterCurrentParams(kCharacterTrainM)[2] && !(_gameTime >= 1134000 && _gameTime <= 1156500)) {
					getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 14;
					TrainMCall(&LogicManager::CONS_TrainM_DoMissingListMessages, 0, 0, 0, 0);
					break;
				}

				if (getCharacterParams(kCharacterTrainM, 8)[2] && !getCharacterCurrentParams(kCharacterTrainM)[3] && !(_gameTime >= 1134000 && _gameTime <= 1156500)) {
					getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 15;
					TrainMCall(&LogicManager::CONS_TrainM_DoCond1ListMessage, 0, 0, 0, 0);
					break;
				}

				if (getCharacterParams(kCharacterTrainM, 8)[0] && !getCharacterCurrentParams(kCharacterTrainM)[4] && (_gameTime < 1134000 || _gameTime > 1156500)) {
					getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 16;
					TrainMCall(&LogicManager::CONS_TrainM_DoHWMessage, 0, 0, 0, 0);
				}

				break;
			}

			if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterTrainM)[6]) {
				getCharacterCurrentParams(kCharacterTrainM)[6] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "TRA1001", 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (_gameTime > 1089000 && !getCharacterCurrentParams(kCharacterTrainM)[7] && (getCharacterCurrentParams(kCharacterTrainM)[7] = 1, !getCharacterCurrentParams(kCharacterTrainM)[4])) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
				TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] == 5) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
				TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 3, 2000, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] == 6) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 7;
				TrainMCall(&LogicManager::CONS_TrainM_DoCond, 3, "TRA1202", 0, 0);
				break;
			}

			// fall through
		case 7:
			if (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] == 7) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 8;
				TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 8:
			if (_gameTime > 1107000 && !getCharacterCurrentParams(kCharacterTrainM)[8]) {
				getCharacterCurrentParams(kCharacterTrainM)[8] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 9;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "TRA1001A", 0, 0, 0);
				break;
			}

			// fall through
		case 9:
			if (_gameTime > 1134000 && !getCharacterCurrentParams(kCharacterTrainM)[9]) {
				getCharacterCurrentParams(kCharacterTrainM)[9] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 10;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "TRA1002", 0, 0, 0);
				break;
			}

			// fall through
		case 10:
			if (_gameTime > 1165500 && !getCharacterCurrentParams(kCharacterTrainM)[10]) {
				getCharacterCurrentParams(kCharacterTrainM)[10] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 11;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "TRA1003", 0, 0, 0);
				break;
			}

			// fall through
		case 11:
			if (_gameTime > 1225800 && !getCharacterCurrentParams(kCharacterTrainM)[11]) {
				getCharacterCurrentParams(kCharacterTrainM)[11] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 12;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "TRA1004", 0, 0, 0);
				break;
			}

			// fall through
		case 12:
			if (getCharacterParams(kCharacterTrainM, 8)[4] && !getCharacterCurrentParams(kCharacterTrainM)[1]) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 13;
				TrainMCall(&LogicManager::CONS_TrainM_DoAfterPoliceMessages, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 13:
			if (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] == 13) {
				getCharacterCurrentParams(kCharacterTrainM)[1] = 1;
			}

			if (cathHasItem(kItemPassengerList) && !getCharacterCurrentParams(kCharacterTrainM)[2] && !(_gameTime >= 1134000 && _gameTime <= 1156500)) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 14;
				TrainMCall(&LogicManager::CONS_TrainM_DoMissingListMessages, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 14:
			if (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] == 14) {
				getCharacterCurrentParams(kCharacterTrainM)[2] = 1;
			}

			if (getCharacterParams(kCharacterTrainM, 8)[2] && !getCharacterCurrentParams(kCharacterTrainM)[3] && !(_gameTime >= 1134000 && _gameTime <= 1156500)) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 15;
				TrainMCall(&LogicManager::CONS_TrainM_DoCond1ListMessage, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 15:
			if (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] == 15) {
				getCharacterCurrentParams(kCharacterTrainM)[3] = 1;
			}

			if (getCharacterParams(kCharacterTrainM, 8)[0] && !getCharacterCurrentParams(kCharacterTrainM)[4] && (_gameTime < 1134000 || _gameTime > 1156500)) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 16;
				TrainMCall(&LogicManager::CONS_TrainM_DoHWMessage, 0, 0, 0, 0);
			}

			break;
		case 16:
			getCharacterCurrentParams(kCharacterTrainM)[4] = 1;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_StartPart2);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 27;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_TrainM_InPart2(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterTrainM);
		getCharacter(kCharacterTrainM).characterPosition.car = kCarBaggage;
		getCharacter(kCharacterTrainM).characterPosition.position = 5000;
		getCharacter(kCharacterTrainM).characterPosition.location = 0;
		getCharacter(kCharacterTrainM).inventoryItem = 0;

		setDoor(104, kCharacterTrainM, 0, 0, 9);
		setDoor(105, kCharacterTrainM, 0, 0, 9);

		getCharacterParams(kCharacterTrainM, 8)[2] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_InPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_InPart2);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 28;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_InPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inOffice(kCharacterCath)) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
			TrainMCall(&LogicManager::CONS_TrainM_KickCathOut, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 1818900 && !getCharacterCurrentParams(kCharacterTrainM)[0]) {
			getCharacterCurrentParams(kCharacterTrainM)[0] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra2177", 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterTrainM)[1] == 0x7FFFFFFF || !_gameTime) {
			if (getCharacterParams(kCharacterTrainM, 8)[2]) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 7;
				TrainMCall(&LogicManager::CONS_TrainM_DoCond1ListMessage, 0, 0, 0, 0);
			}
		}

		if (_gameTime > 1836000) {
			getCharacterCurrentParams(kCharacterTrainM)[1] = 0x7FFFFFFF;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
			break;
		}

		if (!cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterTrainM)[1]) {
			getCharacterCurrentParams(kCharacterTrainM)[1] = _gameTime;
			if (!_gameTime) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
				TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
				break;
			}
		}

		if (getCharacterCurrentParams(kCharacterTrainM)[1] >= _gameTime) {
			if (getCharacterParams(kCharacterTrainM, 8)[2]) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 7;
				TrainMCall(&LogicManager::CONS_TrainM_DoCond1ListMessage, 0, 0, 0, 0);
			}
		} else {
			getCharacterCurrentParams(kCharacterTrainM)[1] = 0x7FFFFFFF;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
		}

		break;
	case 9:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 8;
		TrainMCall(&LogicManager::CONS_TrainM_KickCathOut, msg->param.intParam == 105, 0, 0, 0);
		break;
	case 12:
		dropItem(kItem9, 1);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			if (_gameTime > 1818900 && !getCharacterCurrentParams(kCharacterTrainM)[0]) {
				getCharacterCurrentParams(kCharacterTrainM)[0] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra2177", 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacterCurrentParams(kCharacterTrainM)[1] == 0x7FFFFFFF || !_gameTime) {
				if (getCharacterParams(kCharacterTrainM, 8)[2]) {
					getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 7;
					TrainMCall(&LogicManager::CONS_TrainM_DoCond1ListMessage, 0, 0, 0, 0);
				}
			}

			if (_gameTime > 1836000) {
				getCharacterCurrentParams(kCharacterTrainM)[1] = 0x7FFFFFFF;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
				TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
				break;
			}

			if (!cathInCorridor(kCarRedSleeping) || !getCharacterCurrentParams(kCharacterTrainM)[1]) {
				getCharacterCurrentParams(kCharacterTrainM)[1] = _gameTime;
				if (!_gameTime) {
					getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
					TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterTrainM)[1] >= _gameTime) {
				if (getCharacterParams(kCharacterTrainM, 8)[2]) {
					getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 7;
					TrainMCall(&LogicManager::CONS_TrainM_DoCond1ListMessage, 0, 0, 0, 0);
				}
			} else {
				getCharacterCurrentParams(kCharacterTrainM)[1] = 0x7FFFFFFF;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
				TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
			}

			break;
		case 3:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 4, 2000, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond, 4, "TRA2100", 0, 0);
			break;
		case 5:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 6:
			if (getCharacterParams(kCharacterTrainM, 8)[2]) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 7;
				TrainMCall(&LogicManager::CONS_TrainM_DoCond1ListMessage, 0, 0, 0, 0);
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

void LogicManager::CONS_TrainM_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_StartPart3);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 29;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_TrainM_OnRearPlatform(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterTrainM);
		getCharacter(kCharacterTrainM).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterTrainM).characterPosition.position = 540;
		getCharacter(kCharacterTrainM).characterPosition.location = 0;
		getCharacter(kCharacterTrainM).inventoryItem = 0;
		getCharacter(kCharacterTrainM).clothes = 0;

		setDoor(104, kCharacterTrainM, 0, 0, 9);
		setDoor(105, kCharacterTrainM, 0, 0, 9);

		getCharacterParams(kCharacterTrainM, 8)[3] = 0;
		getCharacterParams(kCharacterTrainM, 8)[2] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_VisitCond2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_VisitCond2);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 30;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_VisitCond2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 4, 2000, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond, 4, (char *)&getCharacterCurrentParams(kCharacterTrainM)[0], 0, 0);
			break;
		case 3:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_DoDogProblem(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_DoDogProblem);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 31;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_DoDogProblem(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 4, 2000, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond, 4, "TRA3015", 0, 0);
			break;
		case 3:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 4:
			_globals[kGlobalMaxHasToStayInBaggage] = 1;
			getCharacterParams(kCharacterTrainM, 8)[3] = 0;

			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);

			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_AnnounceVienna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_AnnounceVienna);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 32;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_AnnounceVienna(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2263500 && !getCharacterCurrentParams(kCharacterTrainM)[0]) {
			getCharacterCurrentParams(kCharacterTrainM)[0] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_DoAnnounceWalk, 4, 9460, "TRA3006", 0);
		}

		break;
	case 12:
		setDoor(104, kCharacterCath, 0, 0, 9);
		setDoor(105, kCharacterCath, 0, 0, 9);

		if (inLowBaggage(kCharacterCath)) {
			playNIS(kEventVergesBaggageCarOffLimits);
		} else {
			if (!inKitchen(kEventNone)) {
				takeItem(kCharacterTrainM);
				getCharacter(kCharacterTrainM).characterPosition.car = kCarRestaurant;
				getCharacter(kCharacterTrainM).characterPosition.position = 5900;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
				TrainMCall(&LogicManager::CONS_TrainM_WaitRCClear, 0, 0, 0, 0);
				break;
			} else {
				playNIS(kEventVergesCanIHelpYou);
			}
		}

		playDialog(kCharacterCath, "BUMP", -1, 0);
		bumpCath(kCarRestaurant, 65, 255);
		takeItem(kCharacterTrainM);
		getCharacter(kCharacterTrainM).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterTrainM).characterPosition.position = 5900;
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).characterPosition.position = 5800;
			getCharacter(kCharacterTrainM).characterPosition.location = 0;
			playDialog(kCharacterTrainM, "TRA3004", -1, 0);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoSeqOtis, "813DD", 0, 0, 0);
			break;
		case 2:
			if (!whoRunningDialog(kCharacterTrainM))
				playDialog(kCharacterTrainM, "TRA3004", -1, 0);

			startSeqOtis(kCharacterTrainM, "813DS");

			if (inDiningRoom(kCharacterCath))
				advanceFrame(kCharacterTrainM);

			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_DoAnnounceWalk, 3, 540, "TRA3004", 0);
			break;
		case 4:
			endGraphics(kCharacterTrainM);
			break;
		case 5:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_OnRearPlatform(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_OnRearPlatform);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 33;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_OnRearPlatform(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_WaitRCClear, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).characterPosition.position = 1540;
			getCharacter(kCharacterTrainM).characterPosition.location = 0;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoSeqOtis, "813US", 0, 0, 0);
			break;
		case 2:
			startSeqOtis(kCharacterTrainM, "813UD");
			if (inSalon(kCharacterCath))
				advanceFrame(kCharacterTrainM);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_FinishSeqOtis, 0, 0, 0, 0);
			break;
		case 3:
			endGraphics(kCharacterTrainM);
			getCharacter(kCharacterTrainM).characterPosition.location = 1;
			getCharacter(kCharacterTrainM).characterPosition.position = 5799;

			if (!_globals[kGlobalPoliceHasBoardedAndGone]) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
				TrainMCall(&LogicManager::CONS_TrainM_DoDialog, "Abb3035", 0, 0, 0);
			} else {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
				TrainMCall(&LogicManager::CONS_TrainM_DoDialog, "ABB3035A", 0, 0, 0);
			}

			break;
		case 4:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_DoDialog, "Abb3035", 0, 0, 0);
			break;
		case 5:
			send(kCharacterTrainM, kCharacterAbbot, 192054567, 0);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra3010", 0, 0, 0);
			break;
		case 6:
			CONS_TrainM_InPart3(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_InPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_InPart3);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 34;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_InPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inOffice(kCharacterCath)) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
			TrainMCall(&LogicManager::CONS_TrainM_KickCathOut, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterTrainM, 8)[3]) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoDogProblem, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterTrainM, 8)[2]) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond1ListMessage, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 1971000 && !getCharacterCurrentParams(kCharacterTrainM)[0]) {
			getCharacterCurrentParams(kCharacterTrainM)[0] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra3001", 0, 0, 0);
			break;
		}

		if (_gameTime > 1998000 && !getCharacterCurrentParams(kCharacterTrainM)[1]) {
			getCharacterCurrentParams(kCharacterTrainM)[1] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra3010a", 0, 0, 0);
			break;
		}

		if (_gameTime > 2016000 && !getCharacterCurrentParams(kCharacterTrainM)[2]) {
			getCharacterCurrentParams(kCharacterTrainM)[2] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
			TrainMCall(&LogicManager::CONS_TrainM_VisitKron, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2070000 && !getCharacterCurrentParams(kCharacterTrainM)[3]) {
			getCharacterCurrentParams(kCharacterTrainM)[3] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 7;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra3002", 0, 0, 0);
			break;
		}

		if (_gameTime > 2142000 && !getCharacterCurrentParams(kCharacterTrainM)[4]) {
			getCharacterCurrentParams(kCharacterTrainM)[4] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 8;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra3003", 0, 0, 0);
			break;
		}

		if (_gameTime > 2173500 && !getCharacterCurrentParams(kCharacterTrainM)[5]) {
			getCharacterCurrentParams(kCharacterTrainM)[5] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 9;
			TrainMCall(&LogicManager::CONS_TrainM_VisitCond2, "Tra3012", 0, 0, 0);
			break;
		}

		if (_gameTime > 2218500 && !getCharacterCurrentParams(kCharacterTrainM)[6]) {
			getCharacterCurrentParams(kCharacterTrainM)[6] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 10;
			TrainMCall(&LogicManager::CONS_TrainM_AnnounceVienna, 0, 0, 0, 0);
		}

		break;
	case 9:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 11;
		TrainMCall(&LogicManager::CONS_TrainM_KickCathOut, (msg->param.intParam == 105) ? 1 : 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			if (getCharacterParams(kCharacterTrainM, 8)[3]) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
				TrainMCall(&LogicManager::CONS_TrainM_DoDogProblem, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacterParams(kCharacterTrainM, 8)[2]) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
				TrainMCall(&LogicManager::CONS_TrainM_DoCond1ListMessage, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (_gameTime > 1971000 && !getCharacterCurrentParams(kCharacterTrainM)[0]) {
				getCharacterCurrentParams(kCharacterTrainM)[0] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra3001", 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (_gameTime > 1998000 && !getCharacterCurrentParams(kCharacterTrainM)[1]) {
				getCharacterCurrentParams(kCharacterTrainM)[1] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra3010a", 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (_gameTime > 2016000 && !getCharacterCurrentParams(kCharacterTrainM)[2]) {
				getCharacterCurrentParams(kCharacterTrainM)[2] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
				TrainMCall(&LogicManager::CONS_TrainM_VisitKron, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (_gameTime > 2070000 && !getCharacterCurrentParams(kCharacterTrainM)[3]) {
				getCharacterCurrentParams(kCharacterTrainM)[3] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 7;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra3002", 0, 0, 0);
				break;
			}

			// fall through
		case 7:
			if (_gameTime > 2142000 && !getCharacterCurrentParams(kCharacterTrainM)[4]) {
				getCharacterCurrentParams(kCharacterTrainM)[4] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 8;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra3003", 0, 0, 0);
				break;
			}

			// fall through
		case 8:
			if (_gameTime > 2173500 && !getCharacterCurrentParams(kCharacterTrainM)[5]) {
				getCharacterCurrentParams(kCharacterTrainM)[5] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 9;
				TrainMCall(&LogicManager::CONS_TrainM_VisitCond2, "Tra3012", 0, 0, 0);
				break;
			}

			// fall through
		case 9:
			if (_gameTime > 2218500 && !getCharacterCurrentParams(kCharacterTrainM)[6]) {
				getCharacterCurrentParams(kCharacterTrainM)[6] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 10;
				TrainMCall(&LogicManager::CONS_TrainM_AnnounceVienna, 0, 0, 0, 0);
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

void LogicManager::CONS_TrainM_VisitKron(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_VisitKron);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 35;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_VisitKron(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_LeaveOffice, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 4, 2000, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond, 4, "Tra3011A", 0, 0);
			break;
		case 3:
			send(kCharacterTrainM, kCharacterCond2, 188570113, 0);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 3, 2000, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond, 3, "Tra3011", 0, 0);
			break;
		case 5:
			send(kCharacterTrainM, kCharacterCond1, 188635520, 0);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterTrainM).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, _functionsTrainM[getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall]]);
			fedEx(kCharacterTrainM, kCharacterTrainM, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_StartPart4);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 36;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_TrainM_InOffice4(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterTrainM);

		getCharacter(kCharacterTrainM).characterPosition.car = kCarBaggage;
		getCharacter(kCharacterTrainM).characterPosition.position = 5000;
		getCharacter(kCharacterTrainM).characterPosition.location = 0;
		getCharacter(kCharacterTrainM).inventoryItem = 0;
		getCharacter(kCharacterTrainM).clothes = 0;

		setDoor(104, kCharacterTrainM, 0, 0, 9);
		setDoor(105, kCharacterTrainM, 0, 0, 9);

		getCharacterParams(kCharacterTrainM, 8)[2] = 0;
		getCharacterParams(kCharacterTrainM, 8)[5] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_InOffice4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_InOffice4);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 37;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_InOffice4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inOffice(kCharacterCath)) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
			TrainMCall(&LogicManager::CONS_TrainM_KickCathOut, 0, 0, 0, 0);
			break;
		}

		if (getCharacterParams(kCharacterTrainM, 8)[5]) {
			if (_gameTime > 2538000 && !getCharacterCurrentParams(kCharacterTrainM)[6]) {
				getCharacterCurrentParams(kCharacterTrainM)[6] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 9;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra4005", 0, 0, 0);
			}

			break;
		}

		if (getCharacterParams(kCharacterTrainM, 8)[2]) {
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoCond1ListMessage, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2349000 && !getCharacterCurrentParams(kCharacterTrainM)[0]) {
			getCharacterCurrentParams(kCharacterTrainM)[0] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra1001", 0, 0, 0);
			break;
		}

		if (_gameTime > 2378700 && !getCharacterCurrentParams(kCharacterTrainM)[1]) {
			getCharacterCurrentParams(kCharacterTrainM)[1] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra4001", 0, 0, 0);
			break;
		}

		if (_gameTime > 2403000 && !getCharacterCurrentParams(kCharacterTrainM)[2]) {
			getCharacterCurrentParams(kCharacterTrainM)[2] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra1001A", 0, 0, 0);
			break;
		}

		if (_gameTime > 2414700 && !getCharacterCurrentParams(kCharacterTrainM)[3]) {
			getCharacterCurrentParams(kCharacterTrainM)[3] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra4002", 0, 0, 0);
			break;
		}

		if (_gameTime > 2484000 && !getCharacterCurrentParams(kCharacterTrainM)[4]) {
			getCharacterCurrentParams(kCharacterTrainM)[4] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 7;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra4003", 0, 0, 0);
			break;
		}

		if (_gameTime > 2511000 && !getCharacterCurrentParams(kCharacterTrainM)[5]) {
			getCharacterCurrentParams(kCharacterTrainM)[5] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 8;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra4004", 0, 0, 0);
			break;
		}

		if (_gameTime > 2538000 && !getCharacterCurrentParams(kCharacterTrainM)[6]) {
			getCharacterCurrentParams(kCharacterTrainM)[6] = 1;
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 9;
			TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra4005", 0, 0, 0);
		}

		break;
	case 9:
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 10;
		TrainMCall(&LogicManager::CONS_TrainM_KickCathOut, (msg->param.intParam == 105) ? 1 : 0, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterTrainM).characterPosition.car = kCarBaggage;
		getCharacter(kCharacterTrainM).characterPosition.position = 5000;
		getCharacter(kCharacterTrainM).characterPosition.location = 0;
		dropItem(kItem9, 1);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			if (getCharacterParams(kCharacterTrainM, 8)[5]) {
				if (_gameTime > 2538000 && !getCharacterCurrentParams(kCharacterTrainM)[6]) {
					getCharacterCurrentParams(kCharacterTrainM)[6] = 1;
					getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 9;
					TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra4005", 0, 0, 0);
				}

				break;
			}

			// fall through
		case 2:
			if (_gameTime > 2349000 && !getCharacterCurrentParams(kCharacterTrainM)[0]) {
				getCharacterCurrentParams(kCharacterTrainM)[0] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra1001", 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (_gameTime > 2378700 && !getCharacterCurrentParams(kCharacterTrainM)[1]) {
				getCharacterCurrentParams(kCharacterTrainM)[1] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra4001", 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (_gameTime > 2403000 && !getCharacterCurrentParams(kCharacterTrainM)[2]) {
				getCharacterCurrentParams(kCharacterTrainM)[2] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 5;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra1001A", 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (_gameTime > 2414700 && !getCharacterCurrentParams(kCharacterTrainM)[3]) {
				getCharacterCurrentParams(kCharacterTrainM)[3] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 6;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra4002", 0, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (_gameTime > 2484000 && !getCharacterCurrentParams(kCharacterTrainM)[4]) {
				getCharacterCurrentParams(kCharacterTrainM)[4] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 7;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra4003", 0, 0, 0);
				break;
			}

			// fall through
		case 7:
			if (_gameTime > 2511000 && !getCharacterCurrentParams(kCharacterTrainM)[5]) {
				getCharacterCurrentParams(kCharacterTrainM)[5] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 8;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra4004", 0, 0, 0);
				break;
			}

			// fall through
		case 8:
			if (_gameTime > 2538000 && !getCharacterCurrentParams(kCharacterTrainM)[6]) {
				getCharacterCurrentParams(kCharacterTrainM)[6] = 1;
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 9;
				TrainMCall(&LogicManager::CONS_TrainM_Announce, "Tra4005", 0, 0, 0);
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

void LogicManager::CONS_TrainM_Conferring(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_Conferring);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 38;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_Conferring(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(104, kCharacterCath, 0, 0, 9);
		setDoor(105, kCharacterCath, 0, 0, 9);
		takeItem(kCharacterTrainM);
		endGraphics(kCharacterTrainM);

		getCharacter(kCharacterTrainM).characterPosition.position = 6469;
		getCharacter(kCharacterTrainM).characterPosition.location = 0;
		getCharacter(kCharacterTrainM).characterPosition.car = kCarRedSleeping;

		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
			endGraphics(kCharacterTrainM);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
			TrainMCall(&LogicManager::CONS_TrainM_DoWait, 1800, 0, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
			TrainMCall(&LogicManager::CONS_TrainM_WalkBackToOffice, 0, 0, 0, 0);
			break;
		case 3:
			CONS_TrainM_InOffice4(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	case 125233040:
		getCharacter(kCharacterTrainM).characterPosition.position = 5790;
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 4, 540, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_StartPart5);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 39;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_TrainM_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterTrainM);
		getCharacter(kCharacterTrainM).characterPosition.location = 1;
		getCharacter(kCharacterTrainM).characterPosition.position = 3650;
		getCharacter(kCharacterTrainM).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterTrainM).inventoryItem = kItemNone;
		getCharacter(kCharacterTrainM).clothes = 0;
		setDoor(104, kCharacterCath, 0, 0, 9);
		setDoor(105, kCharacterCath, 0, 0, 9);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_Prisoner);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 40;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (inSalon(kCharacterCath) && !whoRunningDialog(kCharacterTrainM))
			playDialog(kCharacterTrainM, "WAT5000", -1, 0);

		break;
	case 9:
		if (whoRunningDialog(kCharacterTrainM))
			fadeDialog(kCharacterTrainM);

		if (dialogRunning("MUS050"))
			fadeDialog("MUS050");

		setDoor(65, kCharacterCath, 0, 0, 1);
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;
		TrainMCall(&LogicManager::CONS_TrainM_SaveGame, 2, kEventCathFreePassengers, 0, 0);
		break;
	case 12:
		takeItem(kItem9);
		setDoor(65, kCharacterTrainM, 1, 0, 1);
		break;
	case 18:
		if (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] == 1) {
			playNIS(kEventCathFreePassengers);
			sendAll(kCharacterTrainM, 70549068, 0);
			bumpCath(kCarRedSleeping, 40, 255);
			CONS_TrainM_InCharge(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_InCharge(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_InCharge);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 41;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_InCharge(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setModel(55, 3);

		getCharacter(kCharacterTrainM).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterTrainM).characterPosition.position = 9460;
		getCharacter(kCharacterTrainM).characterPosition.location = 1;
		getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 1;

		TrainMCall(&LogicManager::CONS_TrainM_DoAnnounceWalk, 4, 2000, "Tra5001", 0);
		break;
	case 18:
		switch (getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8]) {
		case 1:
		case 2:
			if (msg->action == 1) {
				startCycOtis(kCharacterTrainM, "620E");
			}

			if (whoRunningDialog(kCharacterTrainM)) {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 2;
				TrainMCall(&LogicManager::CONS_TrainM_DoWait, 225, 0, 0, 0);
			} else {
				getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 3;
				TrainMCall(&LogicManager::CONS_TrainM_DoDialog, "Con5001", 0, 0, 0);
			}

			break;
		case 3:
			send(kCharacterTrainM, kCharacterCond2, 155991520, 0);
			getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall + 8] = 4;
			TrainMCall(&LogicManager::CONS_TrainM_DoWalk, 1, 9460, 0, 0);
			break;
		case 4:
			CONS_TrainM_Disappear(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_TrainM_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTrainM).callParams[getCharacter(kCharacterTrainM).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTrainM, &LogicManager::HAND_TrainM_Disappear);
	getCharacter(kCharacterTrainM).callbacks[getCharacter(kCharacterTrainM).currentCall] = 42;

	params->clear();

	fedEx(kCharacterTrainM, kCharacterTrainM, 12, 0);
}

void LogicManager::HAND_TrainM_Disappear(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterTrainM);
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsTrainM[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_TrainM_DebugWalks,
	&LogicManager::HAND_TrainM_DoSeqOtis,
	&LogicManager::HAND_TrainM_FinishSeqOtis,
	&LogicManager::HAND_TrainM_DoDialog,
	&LogicManager::HAND_TrainM_DoDialogFullVol,
	&LogicManager::HAND_TrainM_WaitRCClear,
	&LogicManager::HAND_TrainM_SaveGame,
	&LogicManager::HAND_TrainM_DoWalk,
	&LogicManager::HAND_TrainM_Announce,
	&LogicManager::HAND_TrainM_DoAnnounceWalk,
	&LogicManager::HAND_TrainM_WalkBackToOffice,
	&LogicManager::HAND_TrainM_LeaveOffice,
	&LogicManager::HAND_TrainM_KickCathOut,
	&LogicManager::HAND_TrainM_DoWait,
	&LogicManager::HAND_TrainM_DoCond,
	&LogicManager::HAND_TrainM_DoCondDoubl,
	&LogicManager::HAND_TrainM_DoCond1ListMessage,
	&LogicManager::HAND_TrainM_Birth,
	&LogicManager::HAND_TrainM_DoMadeBedsMessages,
	&LogicManager::HAND_TrainM_DoMissingListMessages,
	&LogicManager::HAND_TrainM_DoAfterPoliceMessages,
	&LogicManager::HAND_TrainM_DoHWMessage,
	&LogicManager::HAND_TrainM_DoSeizure,
	&LogicManager::HAND_TrainM_DoPoliceDoneDialog,
	&LogicManager::HAND_TrainM_BoardPolice,
	&LogicManager::HAND_TrainM_InOffice,
	&LogicManager::HAND_TrainM_StartPart2,
	&LogicManager::HAND_TrainM_InPart2,
	&LogicManager::HAND_TrainM_StartPart3,
	&LogicManager::HAND_TrainM_VisitCond2,
	&LogicManager::HAND_TrainM_DoDogProblem,
	&LogicManager::HAND_TrainM_AnnounceVienna,
	&LogicManager::HAND_TrainM_OnRearPlatform,
	&LogicManager::HAND_TrainM_InPart3,
	&LogicManager::HAND_TrainM_VisitKron,
	&LogicManager::HAND_TrainM_StartPart4,
	&LogicManager::HAND_TrainM_InOffice4,
	&LogicManager::HAND_TrainM_Conferring,
	&LogicManager::HAND_TrainM_StartPart5,
	&LogicManager::HAND_TrainM_Prisoner,
	&LogicManager::HAND_TrainM_InCharge,
	&LogicManager::HAND_TrainM_Disappear
};

} // End of namespace LastExpress
