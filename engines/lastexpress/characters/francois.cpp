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

void LogicManager::CONS_Francois(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterFrancois,
			_functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]
		);

		break;
	case 1:
		CONS_Francois_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Francois_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Francois_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Francois_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Francois_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::FrancoisCall(CALL_PARAMS) {
	getCharacter(kCharacterFrancois).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Francois_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_DebugWalks);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 1;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterFrancois, kCarGreenSleeping, getCharacterCurrentParams(kCharacterFrancois)[0])) {
			if (getCharacterCurrentParams(kCharacterFrancois)[0] == 10000) {
				getCharacterCurrentParams(kCharacterFrancois)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterFrancois)[0] = 10000;
			}
		}

		break;
	case 1:
		getCharacter(kCharacterFrancois).clothes++;
		if (getCharacter(kCharacterFrancois).clothes > 1)
			getCharacter(kCharacterFrancois).clothes = 0;

		break;
	case 12:
		getCharacter(kCharacterFrancois).characterPosition.position = 0;
		getCharacter(kCharacterFrancois).characterPosition.location = 0;
		getCharacter(kCharacterFrancois).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterFrancois).inventoryItem = 0x80;
		getCharacterCurrentParams(kCharacterFrancois)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_DoWait);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 2;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterFrancois)[1] ||
			(getCharacterCurrentParams(kCharacterFrancois)[1] = _gameTime + getCharacterCurrentParams(kCharacterFrancois)[0], _gameTime + getCharacterCurrentParams(kCharacterFrancois)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterFrancois)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterFrancois)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterFrancois).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
		fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_DoSeqOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_DoSeqOtis);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_DoSeqOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		getCharacter(kCharacterFrancois).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
		fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterFrancois, (char *)&getCharacterCurrentParams(kCharacterFrancois)[0]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_DoCorrOtis);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 4;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterFrancois, getCharacterCurrentParams(kCharacterFrancois)[3]);

		getCharacter(kCharacterFrancois).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
		fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterFrancois, (char *)&getCharacterCurrentParams(kCharacterFrancois)[0]);
		blockAtDoor(kCharacterFrancois, getCharacterCurrentParams(kCharacterFrancois)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_DoEnterCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_DoEnterCorrOtis);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 5;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_DoEnterCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterFrancois, getCharacterCurrentParams(kCharacterFrancois)[3]);
		getCharacter(kCharacterFrancois).characterPosition.position = 5790;

		getCharacter(kCharacterFrancois).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
		fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterFrancois, (char *)&getCharacterCurrentParams(kCharacterFrancois)[0]);
		blockAtDoor(kCharacterFrancois, getCharacterCurrentParams(kCharacterFrancois)[3]);
		getCharacter(kCharacterFrancois).characterPosition.location = 1;
		if (inComp(kCharacterCath, kCarRedSleeping, 5790) || inComp(kCharacterCath, kCarRedSleeping, 6130)) {
			if(isNight()) {
				playNIS(kEventCathTurningNight);
			} else {
				playNIS(kEventCathTurningDay);
			}

			playDialog(0, "BUMP", -1, 0);
			bumpCathFDoor(35);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_DoDialog);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 6;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterFrancois).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
		fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		break;
	case 12:
		playDialog(kCharacterFrancois, (char *)&getCharacterCurrentParams(kCharacterFrancois)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_SaveGame);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 7;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterFrancois).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
		fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		break;
	case 12:
		save(
			kCharacterFrancois,
			getCharacterCurrentParams(kCharacterFrancois)[0],
			getCharacterCurrentParams(kCharacterFrancois)[1]
		);

		getCharacter(kCharacterFrancois).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
		fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_DoWalk);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 8;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterFrancois, getCharacterCurrentParams(kCharacterFrancois)[0], getCharacterCurrentParams(kCharacterFrancois)[1])) {
			getCharacter(kCharacterFrancois).inventoryItem = 0;

			getCharacter(kCharacterFrancois).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
			fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		} else {
			if (!nearChar(kCharacterFrancois, kCharacterCath, 2000) ||
				!cathHasItem(kItemFirebird) ||
				_doneNIS[kEventFrancoisShowEgg] ||
				_doneNIS[kEventFrancoisShowEggD] ||
				_doneNIS[kEventFrancoisShowEggNight] ||
				_doneNIS[kEventFrancoisShowEggNightD]) {
				getCharacter(kCharacterFrancois).inventoryItem = nearChar(kCharacterFrancois, kCharacterCath, 2000) && _items[kItemBeetle].floating == 1 && !_doneNIS[kEventFrancoisShowBeetle] && !_doneNIS[kEventFrancoisShowBeetleD];
			} else {
				getCharacter(kCharacterFrancois).inventoryItem = kItemFirebird;
			}

			if (getCharacterParams(kCharacterFrancois, 8)[0] && nearChar(kCharacterFrancois, kCharacterCath, 1000) &&
				!inComp(kCharacterCath) && !whoOutside(kCharacterCath)) {
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
				FrancoisCall(&LogicManager::CONS_Francois_SaveGame, 2, kEventFrancoisTradeWhistle, 0, 0);
			}
		}

		break;
	case 1:
		if (msg->param.intParam == 18) {
			if (!isNight()) {
				if (getCharacter(kCharacterCath).characterPosition.position <= getCharacter(kCharacterFrancois).characterPosition.position) {
					playNIS(kEventFrancoisShowEgg);
				} else {
					playNIS(kEventFrancoisShowEggD);
				}
			} else {
				if (getCharacter(kCharacterCath).characterPosition.position <= getCharacter(kCharacterFrancois).characterPosition.position) {
					playNIS(kEventFrancoisShowEggNight);
				} else {
					playNIS(kEventFrancoisShowEggNightD);
				}
				
			}
			
			if (getCharacter(kCharacterFrancois).direction == 1) {
				bumpCathFx(getCharacter(kCharacterFrancois).characterPosition.car, getCharacter(kCharacterFrancois).characterPosition.position - 750);
			} else {
				bumpCathRx(getCharacter(kCharacterFrancois).characterPosition.car, getCharacter(kCharacterFrancois).characterPosition.position + 750);
			}
		} else if (msg->param.intParam == 1) {
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
			FrancoisCall(&LogicManager::CONS_Francois_SaveGame, 2, kEventFrancoisShowBeetle, 0, 0);
		}

		break;
	case 5:
	case 6:
		playChrExcuseMe(kCharacterFrancois, kCharacterCath, 0);
		break;
	case 12:
		if (walk(kCharacterFrancois, getCharacterCurrentParams(kCharacterFrancois)[0], getCharacterCurrentParams(kCharacterFrancois)[1])) {
			getCharacter(kCharacterFrancois).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
			fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] == 1) {
			if (getCharacter(kCharacterCath).characterPosition.position <= getCharacter(kCharacterFrancois).characterPosition.position) {
				playNIS(kEventFrancoisTradeWhistle);
			} else {
				playNIS(kEventFrancoisTradeWhistleD);
			}

			giveCathItem(kItemWhistle);
			takeCathItem(kItemMatchBox);

			_items[kItemBeetle].floating = 2;

			if (getCharacter(kCharacterFrancois).direction == 1) {
				bumpCathFx(getCharacter(kCharacterFrancois).characterPosition.car, getCharacter(kCharacterFrancois).characterPosition.position - 750);
			} else {
				bumpCathRx(getCharacter(kCharacterFrancois).characterPosition.car, getCharacter(kCharacterFrancois).characterPosition.position + 750);
			}

			getCharacterParams(kCharacterFrancois, 8)[0] = 0;
		} else if (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] == 2) {
			if (getCharacter(kCharacterCath).characterPosition.position <= getCharacter(kCharacterFrancois).characterPosition.position) {
				playNIS(kEventFrancoisShowBeetle);
			} else {
				playNIS(kEventFrancoisShowBeetleD);
			}

			if (getCharacter(kCharacterFrancois).direction == 1) {
				bumpCathFx(getCharacter(kCharacterFrancois).characterPosition.car, getCharacter(kCharacterFrancois).characterPosition.position - 750);
			} else {
				bumpCathRx(getCharacter(kCharacterFrancois).characterPosition.car, getCharacter(kCharacterFrancois).characterPosition.position + 750);
			}

			getCharacter(kCharacterFrancois).inventoryItem = 0;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_ExitComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_ExitComp);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 9;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_ExitComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (checkDoor(35) == 2) {
			setDoor(35, kCharacterCath, 0, 255, 255);
			send(kCharacterFrancois, kCharacterMadame, 134289824, 0);
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
			FrancoisCall(&LogicManager::CONS_Francois_DoCorrOtis, "605Cd", 35, 0, 0);
		} else {
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
			FrancoisCall(&LogicManager::CONS_Francois_DoCorrOtis, "605Ed", 35, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] == 1) {
			setDoor(35, kCharacterCath, 2, 255, 255);
			getCharacter(kCharacterFrancois).characterPosition.location = 0;

			getCharacter(kCharacterFrancois).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
			fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		} else if (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] == 2) {
			getCharacter(kCharacterFrancois).characterPosition.location = 0;

			getCharacter(kCharacterFrancois).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
			fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_EnterComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_EnterComp);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 10;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_EnterComp(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (checkDoor(35) == 2) {
			setDoor(35, kCharacterCath, 0, 255, 255);
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
			FrancoisCall(&LogicManager::CONS_Francois_DoEnterCorrOtis, "605Bd", 35, 0, 0);
		} else {
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
			FrancoisCall(&LogicManager::CONS_Francois_DoEnterCorrOtis, "605Dd", 35, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8]) {
		case 1:
			setDoor(35, kCharacterCath, 2, 255, 255);
			send(kCharacterFrancois, kCharacterMadame, 102484312, 0);

			// fall through
		case 2:
			getCharacter(kCharacterFrancois).characterPosition.location = 1;
			endGraphics(kCharacterFrancois);

			getCharacter(kCharacterFrancois).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
			fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_Rampage(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_Rampage);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 11;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_Rampage(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (!whoRunningDialog(kCharacterFrancois)) {
			bool skip = false; // Horrible way to unroll a goto...

			if (!getCharacterCurrentParams(kCharacterFrancois)[8]) {
				getCharacterCurrentParams(kCharacterFrancois)[8] = _realTime + getCharacterCurrentParams(kCharacterFrancois)[5];
				if (_realTime + getCharacterCurrentParams(kCharacterFrancois)[5] == 0) {
					skip = true;
				}
			}

			if (skip || getCharacterCurrentParams(kCharacterFrancois)[8] < _realTime) {
				if (!skip) {
					getCharacterCurrentParams(kCharacterFrancois)[8] = 0x7FFFFFFF;
				}

				switch (rnd(7)) {
				case 0:
					playDialog(kCharacterFrancois, "Fra1002A", -1, 0);
					break;
				case 1:
					playDialog(kCharacterFrancois, "Fra1002B", -1, 0);
					break;
				case 2:
					playDialog(kCharacterFrancois, "Fra1002C", -1, 0);
					break;
				case 3:
					playDialog(kCharacterFrancois, "Fra1002D", -1, 0);
					break;
				case 4:
					playDialog(kCharacterFrancois, "Fra1002E", -1, 0);
					break;
				case 5:
				case 6:
					playDialog(kCharacterFrancois, "Fra1002F", -1, 0);
					break;
				default:
					break;
				}

				getCharacterCurrentParams(kCharacterFrancois)[5] = 15 * rnd(7);
				getCharacterCurrentParams(kCharacterFrancois)[8] = 0;
			}
		}

		if (!whoOnScreen(kCharacterFrancois) || !whoFacingCath(kCharacterFrancois))
			getCharacter(kCharacterFrancois).inventoryItem = 0;

		if (walk(kCharacterFrancois, getCharacterCurrentParams(kCharacterFrancois)[1], getCharacterCurrentParams(kCharacterFrancois)[2])) {
			getCharacterCurrentParams(kCharacterFrancois)[4] = 0;

			if (getCharacterCurrentParams(kCharacterFrancois)[2] == 540) {
				getCharacterCurrentParams(kCharacterFrancois)[1] = 4;

				if (_globals[kGlobalChapter] != 1)
					getCharacterCurrentParams(kCharacterFrancois)[1] = 3;

				getCharacterCurrentParams(kCharacterFrancois)[2] = 9460;
			} else {
				getCharacterCurrentParams(kCharacterFrancois)[1] = 3;
				getCharacterCurrentParams(kCharacterFrancois)[2] = 540;
				getCharacterCurrentParams(kCharacterFrancois)[7] = 0;
				getCharacterCurrentParams(kCharacterFrancois)[6] = 0;
				send(kCharacterFrancois, kCharacterCond2, 225932896, 0);
				send(kCharacterFrancois, kCharacterCond1, 225932896, 0);
			}
		}

		if (!nearX(kCharacterFrancois, 2000, 500) || getCharacter(kCharacterFrancois).direction != 2) {
			if (_globals[kGlobalChapter] == 1) {
				if (checkLoc(kCharacterFrancois, kCarRedSleeping) &&
					(whoOnScreen(kCharacterFrancois) || getCharacterCurrentParams(kCharacterFrancois)[0] < _gameTime || getCharacterCurrentParams(kCharacterFrancois)[3]) &&
					!getCharacterCurrentParams(kCharacterFrancois)[4] && getCharacter(kCharacterFrancois).characterPosition.position < getCharacter(kCharacterMadame).characterPosition.position) {

					if (getCharacter(kCharacterFrancois).direction == 2) {
						send(kCharacterFrancois, kCharacterMadame, 202221040, 0);
						getCharacterCurrentParams(kCharacterFrancois)[4] = 1;
						getCharacterCurrentParams(kCharacterFrancois)[3] = 1;
					} else if (getCharacterCurrentParams(kCharacterFrancois)[3] && nearChar(kCharacterFrancois, kCharacterMadame, 1000)) {
						send(kCharacterFrancois, kCharacterMadame, 168986720, 0);
						getCharacterCurrentParams(kCharacterFrancois)[4] = 1;
					}
				}
			} else if (getCharacterCurrentParams(kCharacterFrancois)[0] < _gameTime) {
				getCharacter(kCharacterFrancois).clothes = 0;
				getCharacter(kCharacterFrancois).walkStepSize = 30;
				getCharacter(kCharacterFrancois).inventoryItem = 0;

				if (whoRunningDialog(kCharacterFrancois))
					fadeDialog(kCharacterFrancois);

				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 4;
				FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 5790, 0, 0);
			}

			break;
		}

		if (checkLoc(kCharacterFrancois, kCarRedSleeping) && getCharacterCurrentParams(kCharacterFrancois)[7]) {
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
			FrancoisCall(&LogicManager::CONS_Francois_DoSeqOtis, "605A", 0, 0, 0);
			break;
		}

		if (checkLoc(kCharacterFrancois, kCarGreenSleeping) && getCharacterCurrentParams(kCharacterFrancois)[6]) {
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 3;
			FrancoisCall(&LogicManager::CONS_Francois_DoSeqOtis, "605A", 0, 0, 0);
		} else {
			if (_globals[kGlobalChapter] == 1) {
				if (checkLoc(kCharacterFrancois, kCarRedSleeping) &&
					(whoOnScreen(kCharacterFrancois) || getCharacterCurrentParams(kCharacterFrancois)[0] < _gameTime || getCharacterCurrentParams(kCharacterFrancois)[3]) &&
					!getCharacterCurrentParams(kCharacterFrancois)[4] && getCharacter(kCharacterFrancois).characterPosition.position < getCharacter(kCharacterMadame).characterPosition.position) {

					if (getCharacter(kCharacterFrancois).direction == 2) {
						send(kCharacterFrancois, kCharacterMadame, 202221040, 0);
						getCharacterCurrentParams(kCharacterFrancois)[4] = 1;
						getCharacterCurrentParams(kCharacterFrancois)[3] = 1;
					} else if (getCharacterCurrentParams(kCharacterFrancois)[3] && nearChar(kCharacterFrancois, kCharacterMadame, 1000)) {
						send(kCharacterFrancois, kCharacterMadame, 168986720, 0);
						getCharacterCurrentParams(kCharacterFrancois)[4] = 1;
					}
				}
			} else if (getCharacterCurrentParams(kCharacterFrancois)[0] < _gameTime) {
				getCharacter(kCharacterFrancois).clothes = 0;
				getCharacter(kCharacterFrancois).walkStepSize = 30;
				getCharacter(kCharacterFrancois).inventoryItem = 0;

				if (whoRunningDialog(kCharacterFrancois))
					fadeDialog(kCharacterFrancois);

				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 4;
				FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 5790, 0, 0);
			}
		}

		break;
	case 1:
		getCharacter(kCharacterFrancois).inventoryItem = 0;

		if (whoRunningDialog(kCharacterFrancois))
			fadeDialog(kCharacterFrancois);

		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
		FrancoisCall(&LogicManager::CONS_Francois_SaveGame, 2, kEventFrancoisWhistle, 0, 0);
		break;
	case 5:
		if (_globals[kGlobalJacket] == 2 && !_doneNIS[kEventFrancoisWhistle] && !_doneNIS[kEventFrancoisWhistleD] && !_doneNIS[kEventFrancoisWhistleNight] && !_doneNIS[kEventFrancoisWhistleNightD]) {
			getCharacter(kCharacterFrancois).inventoryItem = 0x80;
		}

		break;
	case 12:
		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
		FrancoisCall(&LogicManager::CONS_Francois_ExitComp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterFrancois).clothes = 1;
			getCharacter(kCharacterFrancois).walkStepSize = 100;
			getCharacter(kCharacterFrancois).inventoryItem = 0;
			getCharacterCurrentParams(kCharacterFrancois)[1] = 3;
			getCharacterCurrentParams(kCharacterFrancois)[2] = 540;
			walk(kCharacterFrancois, getCharacterCurrentParams(kCharacterFrancois)[1], 540);
			getCharacterCurrentParams(kCharacterFrancois)[5] = 15 * rnd(7);
			break;
		case 2:
			send(kCharacterFrancois, kCharacterCond2, 168253822, 0);

			// fall through
		case 3:
			getCharacterCurrentParams(kCharacterFrancois)[4] = 0;
			getCharacterCurrentParams(kCharacterFrancois)[1] = 4;
			getCharacterCurrentParams(kCharacterFrancois)[2] = 9460;
			getCharacter(kCharacterFrancois).characterPosition.position = 2088;
			walk(kCharacterFrancois, getCharacterCurrentParams(kCharacterFrancois)[1], getCharacterCurrentParams(kCharacterFrancois)[2]);

			if (_globals[kGlobalChapter] == 1) {
				if (checkLoc(kCharacterFrancois, kCarRedSleeping) &&
					(whoOnScreen(kCharacterFrancois) || getCharacterCurrentParams(kCharacterFrancois)[0] < _gameTime || getCharacterCurrentParams(kCharacterFrancois)[3]) &&
					!getCharacterCurrentParams(kCharacterFrancois)[4] && getCharacter(kCharacterFrancois).characterPosition.position < getCharacter(kCharacterMadame).characterPosition.position) {

					if (getCharacter(kCharacterFrancois).direction == 2) {
						send(kCharacterFrancois, kCharacterMadame, 202221040, 0);
						getCharacterCurrentParams(kCharacterFrancois)[4] = 1;
						getCharacterCurrentParams(kCharacterFrancois)[3] = 1;
					} else if (getCharacterCurrentParams(kCharacterFrancois)[3] && nearChar(kCharacterFrancois, kCharacterMadame, 1000)) {
						send(kCharacterFrancois, kCharacterMadame, 168986720, 0);
						getCharacterCurrentParams(kCharacterFrancois)[4] = 1;
					}
				}
			} else if (getCharacterCurrentParams(kCharacterFrancois)[0] < _gameTime) {
				getCharacter(kCharacterFrancois).clothes = 0;
				getCharacter(kCharacterFrancois).walkStepSize = 30;
				getCharacter(kCharacterFrancois).inventoryItem = 0;
				if (whoRunningDialog(kCharacterFrancois))
					fadeDialog(kCharacterFrancois);
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 4;
				FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 5790, 0, 0);
			}

			break;
		case 4:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 5;
			FrancoisCall(&LogicManager::CONS_Francois_EnterComp, 0, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterFrancois).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
			fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
			break;
		case 6:
			if (_globals[kGlobalJacket] == 2) {
				if (!isNight()) {
					if (getCharacter(kCharacterFrancois).characterPosition.position <= getCharacter(kCharacterCath).characterPosition.position) {
						playNIS(kEventFrancoisWhistleD);
					} else {
						playNIS(kEventFrancoisWhistle);
					}
				} else if (getCharacter(kCharacterFrancois).characterPosition.position <= getCharacter(kCharacterCath).characterPosition.position) {
					playNIS(kEventFrancoisWhistleNightD);
				} else {
					playNIS(kEventFrancoisWhistleNight);
				}
			}

			if (getCharacter(kCharacterFrancois).direction == 1) {
				bumpCathFx(getCharacter(kCharacterFrancois).characterPosition.car, getCharacter(kCharacterFrancois).characterPosition.position - 750);
			} else {
				bumpCathRx(getCharacter(kCharacterFrancois).characterPosition.car, getCharacter(kCharacterFrancois).characterPosition.position + 750);
			}

			break;
		default:
			break;
		}

		break;
	case 102752636:
		endGraphics(kCharacterFrancois);
		getCharacter(kCharacterFrancois).characterPosition.location = 1;
		getCharacter(kCharacterFrancois).characterPosition.position = 5790;
		getCharacter(kCharacterFrancois).clothes = 0;
		getCharacter(kCharacterFrancois).walkStepSize = 30;
		getCharacter(kCharacterFrancois).inventoryItem = 0;

		getCharacter(kCharacterFrancois).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
		fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
		break;
	case 205346192:
		if (msg->sender == 4) {
			getCharacterCurrentParams(kCharacterFrancois)[7] = 1;
		} else if (msg->sender == 3) {
			getCharacterCurrentParams(kCharacterFrancois)[6] = 1;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_TakeWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_TakeWalk);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 12;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_TakeWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
		FrancoisCall(&LogicManager::CONS_Francois_ExitComp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 9460, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 3;
			FrancoisCall(&LogicManager::CONS_Francois_DoWait, 675, 0, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 4;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 540, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 5;
			FrancoisCall(&LogicManager::CONS_Francois_DoWait, 675, 0, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 5790, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 7;
			FrancoisCall(&LogicManager::CONS_Francois_EnterComp, 0, 0, 0, 0);
			break;
		case 7:
			getCharacter(kCharacterFrancois).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
			fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_HaremVisit(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_HaremVisit);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 13;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_HaremVisit(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
		FrancoisCall(&LogicManager::CONS_Francois_ExitComp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 540, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 3;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 3, 4070, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 4;
			FrancoisCall(&LogicManager::CONS_Francois_DoCorrOtis, "605Df", 6, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterFrancois).characterPosition.location = 1;
			endGraphics(kCharacterFrancois);
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 5;
			FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Har2010", 0, 0, 0);
			break;
		case 5:
			send(kCharacterFrancois, kCharacterAlouan, 189489753, 0);
			break;
		case 6:
			getCharacter(kCharacterFrancois).characterPosition.location = 0;
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 7;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 4840, 0, 0);
			break;
		case 7:
			if (cathHasItem(kItemWhistle) || _items[kItemWhistle].floating == 3) {
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 10;
				FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 5790, 0, 0);
				break;
			}

			startCycOtis(kCharacterFrancois, "605He");
			softBlockAtDoor(kCharacterFrancois, 36);
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 8;

			if (rnd(2) == 0) {
				FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2005C", 0, 0, 0);
			} else {
				FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2005B", 0, 0, 0);
			}
			
			break;
		case 8:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 9;
			FrancoisCall(&LogicManager::CONS_Francois_DoWait, 450, 0, 0, 0);
			break;
		case 9:
			softReleaseAtDoor(kCharacterFrancois, 36);
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 10;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 5790, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 11;
			FrancoisCall(&LogicManager::CONS_Francois_EnterComp, 0, 0, 0, 0);
			break;
		case 11:
			getCharacter(kCharacterFrancois).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
			fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 190219584:
		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
		FrancoisCall(&LogicManager::CONS_Francois_DoCorrOtis, "605Ef", 6, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_ChaseBeetle(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_ChaseBeetle);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 14;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	strncpy((char *)&params->parameters[2], param3.stringParam, 12);

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_ChaseBeetle(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterFrancois)[5], 12, "605H");
		Common::strcat_s((char *)&getCharacterCurrentParams(kCharacterFrancois)[5], 12, (char *)&getCharacterCurrentParams(kCharacterFrancois)[2]);
		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
		FrancoisCall(&LogicManager::CONS_Francois_ExitComp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, getCharacterCurrentParams(kCharacterFrancois)[1], 0, 0);
			break;
		case 2:
			if (_items[kItemBeetle].floating == 3) {
				startCycOtis(kCharacterFrancois, (char *)&getCharacterCurrentParams(kCharacterFrancois)[5]);
				softBlockAtDoor(kCharacterFrancois, getCharacterCurrentParams(kCharacterFrancois)[0]);
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 3;
				FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2005A", 0, 0, 0);
			} else {
				if (getCharacterCurrentParams(kCharacterFrancois)[1] >= 5790) {
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 10;
					FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 9460, 0, 0);
				} else {
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 9;
					FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 540, 0, 0);
				}
			}

			break;
		case 3:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 4;
			FrancoisCall(&LogicManager::CONS_Francois_DoWait, rnd(450), 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 5;

			if (rnd(2) == 0) {
				FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2005C", 0, 0, 0);
			} else {
				FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2005B", 0, 0, 0);
			}

			break;
		case 5:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
			FrancoisCall(&LogicManager::CONS_Francois_DoWait, rnd(450), 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 7;

			if (rnd(2) == 0) {
				FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2005C", 0, 0, 0);
			} else {
				FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2005B", 0, 0, 0);
			}

			break;
		case 7:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 8;
			FrancoisCall(&LogicManager::CONS_Francois_DoWait, rnd(150), 0, 0, 0);
			break;
		case 8:
			softReleaseAtDoor(kCharacterFrancois, getCharacterCurrentParams(kCharacterFrancois)[0]);
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 10;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 9460, 0, 0);
			break;
		case 9:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 10;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 9460, 0, 0);
			break;
		case 10:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 11;
			FrancoisCall(&LogicManager::CONS_Francois_DoWait, 900, 0, 0, 0);
			break;
		case 11:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 12;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 5790, 0, 0);
			break;
		case 12:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 13;
			FrancoisCall(&LogicManager::CONS_Francois_EnterComp, 0, 0, 0, 0);
			break;
		case 13:
			getCharacter(kCharacterFrancois).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
			fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_FindCath(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_FindCath);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 15;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_FindCath(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
		FrancoisCall(&LogicManager::CONS_Francois_ExitComp, 0, 0, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8]) {
		case 1:
			if (getCharacter(kCharacterCath).characterPosition.position <= getCharacter(kCharacterFrancois).characterPosition.position) {
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 3;
				FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 540, 0, 0);
			} else {
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
				FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 9460, 0, 0);
			}

			break;
		case 2:
		case 3:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 4;
			FrancoisCall(&LogicManager::CONS_Francois_DoWait, 450, 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 5;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 5790, 0, 0);
			break;
		case 5:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
			FrancoisCall(&LogicManager::CONS_Francois_EnterComp, 0, 0, 0, 0);
			break;
		case 6:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 7;
			FrancoisCall(&LogicManager::CONS_Francois_DoWait, 900, 0, 0, 0);
			break;
		case 7:
			if (!inComp(kCharacterMadame, kCarRedSleeping, 5790)) {
				getCharacter(kCharacterFrancois).currentCall--;
				_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
				fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
			} else {
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 8;
				FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2012", 0, 0, 0);
			}

			break;
		case 8:
			getCharacter(kCharacterFrancois).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
			fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_LetsGo(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_LetsGo);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 16;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_LetsGo(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterFrancois).characterPosition = getCharacter(kCharacterMonsieur).characterPosition;
		break;
	case 12:
		setDoor(35, kCharacterCath, 0, 0, 0);
		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
		FrancoisCall(&LogicManager::CONS_Francois_DoCorrOtis, "605Cd", 35, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterFrancois).characterPosition.location = 0;
			getCharacter(kCharacterFrancois).characterPosition.position = 5890;
			send(kCharacterFrancois, kCharacterMadame, 101107728, 0);
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
			FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 5, 850, 0, 0);
			break;
		case 2:
			endGraphics(kCharacterFrancois);
			send(kCharacterFrancois, kCharacterMonsieur, 237889408, 0);
			break;
		case 3:
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 4;
			FrancoisCall(&LogicManager::CONS_Francois_DoCorrOtis, "605Id", 35, 0, 0);
			break;
		case 4:
			setDoor(35, kCharacterCath, 2, 255, 255);
			send(kCharacterFrancois, kCharacterMadame, 100957716, 0);
			getCharacter(kCharacterFrancois).characterPosition.position = 5790;
			getCharacter(kCharacterFrancois).characterPosition.location = 1;
			endGraphics(kCharacterFrancois);

			getCharacter(kCharacterFrancois).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, _functionsFrancois[getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall]]);
			fedEx(kCharacterFrancois, kCharacterFrancois, 18, 0);
			break;
		default:
			break;
		}

		break;
	case 100901266:
		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 3;
		FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 5790, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_Birth);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 17;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterFrancois)[0]) {
			getCharacterCurrentParams(kCharacterFrancois)[0] = 1;
			CONS_Francois_StartPart1(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterFrancois).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterFrancois).characterPosition.position = 5790;
		getCharacter(kCharacterFrancois).characterPosition.location = 1;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_StartPart1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_StartPart1);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 18;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_StartPart1(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1075500 && !getCharacterCurrentParams(kCharacterFrancois)[0]) {
			getCharacterCurrentParams(kCharacterFrancois)[0] = 1;
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
			FrancoisCall(&LogicManager::CONS_Francois_Rampage, 1093500, 0, 0, 0);
		}

		break;
	case 18:
		if (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] == 1)
			CONS_Francois_InComp(0, 0, 0, 0);

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_InComp(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_InComp);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 19;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_InComp(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1161000 && !getCharacterCurrentParams(kCharacterFrancois)[0]) {
			getCharacterCurrentParams(kCharacterFrancois)[0] = 1;
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
			FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
		}

		break;
	case 101107728:
		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
		FrancoisCall(&LogicManager::CONS_Francois_LetsGo, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_Asleep);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 20;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterFrancois).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterFrancois).characterPosition.position = 5790;
		getCharacter(kCharacterFrancois).characterPosition.location = 1;
		endGraphics(kCharacterFrancois);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_StartPart2);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 21;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Francois_AtBreakfast(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterFrancois);
		getCharacter(kCharacterFrancois).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterFrancois).characterPosition.position = 4689;
		getCharacter(kCharacterFrancois).characterPosition.location = 1;
		getCharacter(kCharacterFrancois).clothes = 0;
		getCharacter(kCharacterFrancois).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_AtBreakfast(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_AtBreakfast);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 22;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_AtBreakfast(HAND_PARAMS) {
	switch (msg->action) {
	case 18:
		if (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] == 1) {
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
			FrancoisCall(&LogicManager::CONS_Francois_DoCorrOtis, "605Id", 35, 0, 0);
		} else if (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] == 2) {
			setDoor(35, kCharacterCath, 2, 255, 255);
			send(kCharacterFrancois, kCharacterMadame, 100957716, 0);
			getCharacter(kCharacterFrancois).characterPosition.position = 5790;
			getCharacter(kCharacterFrancois).characterPosition.location = 1;
			endGraphics(kCharacterFrancois);
			CONS_Francois_WithMama(0, 0, 0, 0);
		}

		break;
	case 100901266:
		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
		FrancoisCall(&LogicManager::CONS_Francois_DoWalk, 4, 5790, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_WithMama(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_WithMama);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 23;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_WithMama(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if ((_doneNIS[kEventFrancoisShowBeetle] || _doneNIS[kEventFrancoisShowBeetleD]) &&
			!_doneNIS[kEventFrancoisTradeWhistle] && !_doneNIS[kEventFrancoisTradeWhistleD]) {
			getCharacterParams(kCharacterFrancois, 8)[0] = 1;
		}

		if (getCharacterParams(kCharacterFrancois, 8)[0] && cathInCorridor(kCarRedSleeping)) {
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
			FrancoisCall(&LogicManager::CONS_Francois_FindCath, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 1764000 && !getCharacterCurrentParams(kCharacterFrancois)[0]) {
			getCharacterCurrentParams(kCharacterFrancois)[0] = 1;
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
			FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2011", 0, 0, 0);
			break;
		}

		if (_gameTime > 1800000 && !getCharacterCurrentParams(kCharacterFrancois)[1]) {
			getCharacterCurrentParams(kCharacterFrancois)[1] = 1;
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 3;
			FrancoisCall(&LogicManager::CONS_Francois_HaremVisit, 0, 0, 0, 0);
			break;
		}

		if (!cathHasItem(kItemWhistle) && _items[kItemWhistle].floating != 3) {
			if (_gameTime > 1768500 && !getCharacterCurrentParams(kCharacterFrancois)[2]) {
				getCharacterCurrentParams(kCharacterFrancois)[2] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 4;
				FrancoisCall(&LogicManager::CONS_Francois_Rampage, 1773000, 0, 0, 0);
				break;
			}

			if (_gameTime > 1827000 && !getCharacterCurrentParams(kCharacterFrancois)[3]) {
				getCharacterCurrentParams(kCharacterFrancois)[3] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 5;
				FrancoisCall(&LogicManager::CONS_Francois_Rampage, 1831500, 0, 0, 0);
				break;
			}
		}

		if (_items[kItemBeetle].floating != 3) {
			cathHasItem(kItemWhistle);
			break;
		}

		if (getCharacterCurrentParams(kCharacterFrancois)[4] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime > 15803100) {
				getCharacterCurrentParams(kCharacterFrancois)[4] = 0x7FFFFFFF;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
				FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
				break;
			}

			if (!nearChar(kCharacterFrancois, kCharacterCath, 2000) || !getCharacterCurrentParams(kCharacterFrancois)[4]) {
				getCharacterCurrentParams(kCharacterFrancois)[4] = _gameTime + 75;
				if (_gameTime == -75) {
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
					FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterFrancois)[4] < _gameTime) {
				getCharacterCurrentParams(kCharacterFrancois)[4] = 0x7FFFFFFF;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
				FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
				break;
			}
		}

		if (_gameTime > 1782000 && !getCharacterCurrentParams(kCharacterFrancois)[5]) {
			getCharacterCurrentParams(kCharacterFrancois)[5] = 1;
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 7;
			FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 37, 4070, "f", 0);
			break;
		}

		if (_gameTime > 1813500 && !getCharacterCurrentParams(kCharacterFrancois)[6]) {
			getCharacterCurrentParams(kCharacterFrancois)[6] = 1;
			getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 8;
			FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 34, 6470, "c", 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8]) {
		case 1:
			if (_gameTime > 1764000 && !getCharacterCurrentParams(kCharacterFrancois)[0]) {
				getCharacterCurrentParams(kCharacterFrancois)[0] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
				FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2011", 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (_gameTime > 1800000 && !getCharacterCurrentParams(kCharacterFrancois)[1]) {
				getCharacterCurrentParams(kCharacterFrancois)[1] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 3;
				FrancoisCall(&LogicManager::CONS_Francois_HaremVisit, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (!cathHasItem(kItemWhistle) && _items[kItemWhistle].floating != 3) {
				if (_gameTime > 1768500 && !getCharacterCurrentParams(kCharacterFrancois)[2]) {
					getCharacterCurrentParams(kCharacterFrancois)[2] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 4;
					FrancoisCall(&LogicManager::CONS_Francois_Rampage, 1773000, 0, 0, 0);
					break;
				}

				if (_gameTime > 1827000 && !getCharacterCurrentParams(kCharacterFrancois)[3]) {
					getCharacterCurrentParams(kCharacterFrancois)[3] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 5;
					FrancoisCall(&LogicManager::CONS_Francois_Rampage, 1831500, 0, 0, 0);
					break;
				}
			}

			if (_items[kItemBeetle].floating != 3) {
				cathHasItem(kItemWhistle);
				break;
			}

			if (getCharacterCurrentParams(kCharacterFrancois)[4] != 0x7FFFFFFF && _gameTime) {
				if (_gameTime > 15803100) {
					getCharacterCurrentParams(kCharacterFrancois)[4] = 0x7FFFFFFF;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
					FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
					break;
				}

				if (!nearChar(kCharacterFrancois, kCharacterCath, 2000) || !getCharacterCurrentParams(kCharacterFrancois)[4]) {
					getCharacterCurrentParams(kCharacterFrancois)[4] = _gameTime + 75;
					if (_gameTime == -75) {
						getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
						FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterFrancois)[4] < _gameTime) {
					getCharacterCurrentParams(kCharacterFrancois)[4] = 0x7FFFFFFF;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
					FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
					break;
				}
			}

			if (_gameTime > 1782000 && !getCharacterCurrentParams(kCharacterFrancois)[5]) {
				getCharacterCurrentParams(kCharacterFrancois)[5] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 7;
				FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 37, 4070, "f", 0);
				break;
			}

			if (_gameTime > 1813500 && !getCharacterCurrentParams(kCharacterFrancois)[6]) {
				getCharacterCurrentParams(kCharacterFrancois)[6] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 8;
				FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 34, 6470, "c", 0);
			}

			break;
		case 4:
			if (_gameTime > 1827000 && !getCharacterCurrentParams(kCharacterFrancois)[3]) {
				getCharacterCurrentParams(kCharacterFrancois)[3] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 5;
				FrancoisCall(&LogicManager::CONS_Francois_Rampage, 1831500, 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (_items[kItemBeetle].floating != 3) {
				cathHasItem(kItemWhistle);
				break;
			}

			if (getCharacterCurrentParams(kCharacterFrancois)[4] != 0x7FFFFFFF && _gameTime) {
				if (_gameTime > 15803100) {
					getCharacterCurrentParams(kCharacterFrancois)[4] = 0x7FFFFFFF;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
					FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
					break;
				}

				if (!nearChar(kCharacterFrancois, kCharacterCath, 2000) || !getCharacterCurrentParams(kCharacterFrancois)[4]) {
					getCharacterCurrentParams(kCharacterFrancois)[4] = _gameTime + 75;
					if (_gameTime == -75) {
						getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
						FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterFrancois)[4] < _gameTime) {
					getCharacterCurrentParams(kCharacterFrancois)[4] = 0x7FFFFFFF;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
					FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
					break;
				}
			}

			// fall through
		case 6:
			if (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] == 6) {
				_globals[kGlobalFrancoisSawABlackBeetle] = 1;
			}

			if (_gameTime > 1782000 && !getCharacterCurrentParams(kCharacterFrancois)[5]) {
				getCharacterCurrentParams(kCharacterFrancois)[5] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 7;
				FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 37, 4070, "f", 0);
				break;
			}

			// fall through
		case 7:
			if (_gameTime > 1813500 && !getCharacterCurrentParams(kCharacterFrancois)[6]) {
				getCharacterCurrentParams(kCharacterFrancois)[6] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 8;
				FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 34, 6470, "c", 0);
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

void LogicManager::CONS_Francois_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_StartPart3);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 24;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Francois_InPart3(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterFrancois);
		getCharacter(kCharacterFrancois).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterFrancois).characterPosition.position = 5790;
		getCharacter(kCharacterFrancois).characterPosition.location = 1;
		getCharacter(kCharacterFrancois).clothes = 0;
		getCharacter(kCharacterFrancois).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_InPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_InPart3);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 25;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_InPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if ((_doneNIS[kEventFrancoisShowBeetle] || _doneNIS[kEventFrancoisShowBeetleD]) && !_doneNIS[kEventFrancoisTradeWhistle] && !_doneNIS[kEventFrancoisTradeWhistleD]) {
			getCharacterParams(kCharacterFrancois, 8)[0] = 1;
		}

		if (getCharacterCurrentParams(kCharacterFrancois)[1] && inComp(kCharacterMadame, kCarRedSleeping, 5790) && !getCharacterCurrentParams(kCharacterFrancois)[0]) {
			if (getCharacterParams(kCharacterFrancois, 8)[0] && cathInCorridor(kCarRedSleeping)) {
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 2;
				FrancoisCall(&LogicManager::CONS_Francois_FindCath, 0, 0, 0, 0);
				break;
			}

			if (_gameTime > 2025000 && !getCharacterCurrentParams(kCharacterFrancois)[2]) {
				getCharacterCurrentParams(kCharacterFrancois)[2] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 3;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			if (_gameTime > 2052000 && !getCharacterCurrentParams(kCharacterFrancois)[3]) {
				getCharacterCurrentParams(kCharacterFrancois)[3] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 4;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			if (_gameTime > 2079000 && !getCharacterCurrentParams(kCharacterFrancois)[4]) {
				getCharacterCurrentParams(kCharacterFrancois)[4] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 5;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			if (_gameTime > 2092500 && !getCharacterCurrentParams(kCharacterFrancois)[5]) {
				getCharacterCurrentParams(kCharacterFrancois)[5] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			if (_gameTime > 2173500 && !getCharacterCurrentParams(kCharacterFrancois)[6]) {
				getCharacterCurrentParams(kCharacterFrancois)[6] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 7;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			if (_gameTime > 2182500 && !getCharacterCurrentParams(kCharacterFrancois)[7]) {
				getCharacterCurrentParams(kCharacterFrancois)[7] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 8;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			if (_gameTime > 2241000 && !getCharacterCurrentParams(kCharacterFrancois)[8]) {
				getCharacterCurrentParams(kCharacterFrancois)[8] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 9;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			if (!cathHasItem(kItemWhistle) && _items[kItemWhistle].floating != 3) {
				if (_gameTime > 2011500 && !getCharacterCurrentParams(kCharacterFrancois)[9]) {
					getCharacterCurrentParams(kCharacterFrancois)[9] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 10;
					FrancoisCall(&LogicManager::CONS_Francois_Rampage, 2016000, 0, 0, 0);
					break;
				}

				if (_gameTime > 2115000 && !getCharacterCurrentParams(kCharacterFrancois)[10]) {
					getCharacterCurrentParams(kCharacterFrancois)[10] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 11;
					FrancoisCall(&LogicManager::CONS_Francois_Rampage, 2119500, 0, 0, 0);
					break;
				}
			}

			if (_items[kItemBeetle].floating == 3) {
				if (getCharacterCurrentParams(kCharacterFrancois)[11] != 0x7FFFFFFF && _gameTime) {
					if (_gameTime > 15803100) {
						getCharacterCurrentParams(kCharacterFrancois)[11] = 0x7FFFFFFF;
						getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 12;
						FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
						break;
					}

					if (!nearChar(kCharacterFrancois, kCharacterCath, 2000) || !getCharacterCurrentParams(kCharacterFrancois)[11]) {
						getCharacterCurrentParams(kCharacterFrancois)[11] = _gameTime + 75;
						if (_gameTime == -75) {
							getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 12;
							FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
							break;
						}
					}

					if (getCharacterCurrentParams(kCharacterFrancois)[11] < _gameTime) {
						getCharacterCurrentParams(kCharacterFrancois)[11] = 0x7FFFFFFF;
						getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 12;
						FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
						break;
					}
				}

				if (_gameTime > 2040300 && !getCharacterCurrentParams(kCharacterFrancois)[12]) {
					getCharacterCurrentParams(kCharacterFrancois)[12] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 13;
					FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 36, 4840, "e", 0);
					break;
				}

				if (_gameTime > 2146500 && !getCharacterCurrentParams(kCharacterFrancois)[13]) {
					getCharacterCurrentParams(kCharacterFrancois)[13] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 14;
					FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 37, 4070, "f", 0);
					break;
				}

				if (_gameTime > 2218500 && !getCharacterCurrentParams(kCharacterFrancois)[14]) {
					getCharacterCurrentParams(kCharacterFrancois)[14] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 15;
					FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 33, 7500, "b", 0);
					break;
				}
			} else {
				cathHasItem(kItemWhistle);
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8]) {
		case 1:
			getCharacterCurrentParams(kCharacterFrancois)[1] = 1;
			break;
		case 2:
			if (_gameTime > 2025000 && !getCharacterCurrentParams(kCharacterFrancois)[2]) {
				getCharacterCurrentParams(kCharacterFrancois)[2] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 3;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (_gameTime > 2052000 && !getCharacterCurrentParams(kCharacterFrancois)[3]) {
				getCharacterCurrentParams(kCharacterFrancois)[3] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 4;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (_gameTime > 2079000 && !getCharacterCurrentParams(kCharacterFrancois)[4]) {
				getCharacterCurrentParams(kCharacterFrancois)[4] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 5;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (_gameTime > 2092500 && !getCharacterCurrentParams(kCharacterFrancois)[5]) {
				getCharacterCurrentParams(kCharacterFrancois)[5] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 6;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (_gameTime > 2173500 && !getCharacterCurrentParams(kCharacterFrancois)[6]) {
				getCharacterCurrentParams(kCharacterFrancois)[6] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 7;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 7:
			if (_gameTime > 2182500 && !getCharacterCurrentParams(kCharacterFrancois)[7]) {
				getCharacterCurrentParams(kCharacterFrancois)[7] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 8;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 8:
			if (_gameTime > 2241000 && !getCharacterCurrentParams(kCharacterFrancois)[8]) {
				getCharacterCurrentParams(kCharacterFrancois)[8] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 9;
				FrancoisCall(&LogicManager::CONS_Francois_TakeWalk, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 9:
			if (!cathHasItem(kItemWhistle) && _items[kItemWhistle].floating != 3) {
				if (_gameTime > 2011500 && !getCharacterCurrentParams(kCharacterFrancois)[9]) {
					getCharacterCurrentParams(kCharacterFrancois)[9] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 10;
					FrancoisCall(&LogicManager::CONS_Francois_Rampage, 2016000, 0, 0, 0);
					break;
				}

				if (_gameTime > 2115000 && !getCharacterCurrentParams(kCharacterFrancois)[10]) {
					getCharacterCurrentParams(kCharacterFrancois)[10] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 11;
					FrancoisCall(&LogicManager::CONS_Francois_Rampage, 2119500, 0, 0, 0);
					break;
				}
			}

			if (_items[kItemBeetle].floating == 3) {
				if (getCharacterCurrentParams(kCharacterFrancois)[11] != 0x7FFFFFFF && _gameTime) {
					if (_gameTime > 15803100) {
						getCharacterCurrentParams(kCharacterFrancois)[11] = 0x7FFFFFFF;
						getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 12;
						FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
						break;
					}

					if (!nearChar(kCharacterFrancois, kCharacterCath, 2000) || !getCharacterCurrentParams(kCharacterFrancois)[11]) {
						getCharacterCurrentParams(kCharacterFrancois)[11] = _gameTime + 75;
						if (_gameTime == -75) {
							getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 12;
							FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
							break;
						}
					}

					if (getCharacterCurrentParams(kCharacterFrancois)[11] < _gameTime) {
						getCharacterCurrentParams(kCharacterFrancois)[11] = 0x7FFFFFFF;
						getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 12;
						FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
						break;
					}
				}

				if (_gameTime > 2040300 && !getCharacterCurrentParams(kCharacterFrancois)[12]) {
					getCharacterCurrentParams(kCharacterFrancois)[12] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 13;
					FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 36, 4840, "e", 0);
					break;
				}

				if (_gameTime > 2146500 && !getCharacterCurrentParams(kCharacterFrancois)[13]) {
					getCharacterCurrentParams(kCharacterFrancois)[13] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 14;
					FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 37, 4070, "f", 0);
					break;
				}

				if (_gameTime > 2218500 && !getCharacterCurrentParams(kCharacterFrancois)[14]) {
					getCharacterCurrentParams(kCharacterFrancois)[14] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 15;
					FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 33, 7500, "b", 0);
					break;
				}
			} else {
				cathHasItem(kItemWhistle);
			}

			break;
		case 10:
			if (_gameTime > 2115000 && !getCharacterCurrentParams(kCharacterFrancois)[10]) {
				getCharacterCurrentParams(kCharacterFrancois)[10] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 11;
				FrancoisCall(&LogicManager::CONS_Francois_Rampage, 2119500, 0, 0, 0);
				break;
			}

			// fall through
		case 11:
			if (_items[kItemBeetle].floating == 3) {
				if (getCharacterCurrentParams(kCharacterFrancois)[11] != 0x7FFFFFFF && _gameTime) {
					if (_gameTime > 15803100) {
						getCharacterCurrentParams(kCharacterFrancois)[11] = 0x7FFFFFFF;
						getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 12;
						FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
						break;
					}

					if (!nearChar(kCharacterFrancois, kCharacterCath, 2000) || !getCharacterCurrentParams(kCharacterFrancois)[11]) {
						getCharacterCurrentParams(kCharacterFrancois)[11] = _gameTime + 75;
						if (_gameTime == -75) {
							getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 12;
							FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
							break;
						}
					}

					if (getCharacterCurrentParams(kCharacterFrancois)[11] < _gameTime) {
						getCharacterCurrentParams(kCharacterFrancois)[11] = 0x7FFFFFFF;
						getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 12;
						FrancoisCall(&LogicManager::CONS_Francois_DoDialog, "Fra2010", 0, 0, 0);
						break;
					}
				}

				if (_gameTime > 2040300 && !getCharacterCurrentParams(kCharacterFrancois)[12]) {
					getCharacterCurrentParams(kCharacterFrancois)[12] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 13;
					FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 36, 4840, "e", 0);
					break;
				}

				if (_gameTime > 2146500 && !getCharacterCurrentParams(kCharacterFrancois)[13]) {
					getCharacterCurrentParams(kCharacterFrancois)[13] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 14;
					FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 37, 4070, "f", 0);
					break;
				}

				if (_gameTime > 2218500 && !getCharacterCurrentParams(kCharacterFrancois)[14]) {
					getCharacterCurrentParams(kCharacterFrancois)[14] = 1;
					getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 15;
					FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 33, 7500, "b", 0);
					break;
				}
			} else {
				cathHasItem(kItemWhistle);
			}

			break;
		case 12:
			if (getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] == 12) {
				_globals[kGlobalFrancoisSawABlackBeetle] = 1;
			}

			if (_gameTime > 2040300 && !getCharacterCurrentParams(kCharacterFrancois)[12]) {
				getCharacterCurrentParams(kCharacterFrancois)[12] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 13;
				FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 36, 4840, "e", 0);
				break;
			}

			// fall through
		case 13:
			if (_gameTime > 2146500 && !getCharacterCurrentParams(kCharacterFrancois)[13]) {
				getCharacterCurrentParams(kCharacterFrancois)[13] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 14;
				FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 37, 4070, "f", 0);
				break;
			}

			// fall through
		case 14:
			if (_gameTime > 2218500 && !getCharacterCurrentParams(kCharacterFrancois)[14]) {
				getCharacterCurrentParams(kCharacterFrancois)[14] = 1;
				getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 15;
				FrancoisCall(&LogicManager::CONS_Francois_ChaseBeetle, 33, 7500, "b", 0);
			}

			break;
		default:
			break;
		}

		break;
	case 101107728:
		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
		FrancoisCall(&LogicManager::CONS_Francois_LetsGo, 0, 0, 0, 0);
		break;
	case 189872836:
		getCharacterCurrentParams(kCharacterFrancois)[0] = 1;
		break;
	case 190390860:
		getCharacterCurrentParams(kCharacterFrancois)[0] = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_StartPart4);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 26;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Francois_InPart4(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterFrancois);
		getCharacter(kCharacterFrancois).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterFrancois).characterPosition.position = 5790;
		getCharacter(kCharacterFrancois).characterPosition.location = 1;
		getCharacter(kCharacterFrancois).clothes = 0;
		getCharacter(kCharacterFrancois).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_InPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_InPart4);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 27;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_InPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 101107728:
		getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall + 8] = 1;
		FrancoisCall(&LogicManager::CONS_Francois_LetsGo, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_StartPart5);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 28;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Francois_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterFrancois);
		getCharacter(kCharacterFrancois).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterFrancois).characterPosition.position = 3969;
		getCharacter(kCharacterFrancois).characterPosition.location = 1;
		getCharacter(kCharacterFrancois).clothes = 0;
		getCharacter(kCharacterFrancois).inventoryItem = 0;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_Prisoner);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 29;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		if (!cathHasItem(kItemWhistle) && _items[kItemWhistle].floating != 3)
			dropItem(kItemWhistle, 1);

		CONS_Francois_Hiding(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_Hiding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_Hiding);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 30;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_Hiding(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterFrancois).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterFrancois).characterPosition.position = 5790;
		getCharacter(kCharacterFrancois).characterPosition.location = 1;
		getCharacter(kCharacterFrancois).clothes = 0;
		getCharacter(kCharacterFrancois).inventoryItem = 0;
		break;
	case 135800432:
		CONS_Francois_Disappear(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Francois_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterFrancois).callParams[getCharacter(kCharacterFrancois).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterFrancois, &LogicManager::HAND_Francois_Disappear);
	getCharacter(kCharacterFrancois).callbacks[getCharacter(kCharacterFrancois).currentCall] = 31;

	params->clear();

	fedEx(kCharacterFrancois, kCharacterFrancois, 12, 0);
}

void LogicManager::HAND_Francois_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsFrancois[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Francois_DebugWalks,
	&LogicManager::HAND_Francois_DoWait,
	&LogicManager::HAND_Francois_DoSeqOtis,
	&LogicManager::HAND_Francois_DoCorrOtis,
	&LogicManager::HAND_Francois_DoEnterCorrOtis,
	&LogicManager::HAND_Francois_DoDialog,
	&LogicManager::HAND_Francois_SaveGame,
	&LogicManager::HAND_Francois_DoWalk,
	&LogicManager::HAND_Francois_ExitComp,
	&LogicManager::HAND_Francois_EnterComp,
	&LogicManager::HAND_Francois_Rampage,
	&LogicManager::HAND_Francois_TakeWalk,
	&LogicManager::HAND_Francois_HaremVisit,
	&LogicManager::HAND_Francois_ChaseBeetle,
	&LogicManager::HAND_Francois_FindCath,
	&LogicManager::HAND_Francois_LetsGo,
	&LogicManager::HAND_Francois_Birth,
	&LogicManager::HAND_Francois_StartPart1,
	&LogicManager::HAND_Francois_InComp,
	&LogicManager::HAND_Francois_Asleep,
	&LogicManager::HAND_Francois_StartPart2,
	&LogicManager::HAND_Francois_AtBreakfast,
	&LogicManager::HAND_Francois_WithMama,
	&LogicManager::HAND_Francois_StartPart3,
	&LogicManager::HAND_Francois_InPart3,
	&LogicManager::HAND_Francois_StartPart4,
	&LogicManager::HAND_Francois_InPart4,
	&LogicManager::HAND_Francois_StartPart5,
	&LogicManager::HAND_Francois_Prisoner,
	&LogicManager::HAND_Francois_Hiding,
	&LogicManager::HAND_Francois_Disappear
};

} // End of namespace LastExpress
