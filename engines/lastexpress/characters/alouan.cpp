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

void LogicManager::CONS_Alouan(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterAlouan,
			_functionsAlouan[getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall]]
		);

		break;
	case 1:
		CONS_Alouan_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Alouan_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Alouan_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Alouan_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Alouan_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::AlouanCall(CALL_PARAMS) {
	getCharacter(kCharacterAlouan).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Alouan_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_DebugWalks);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 1;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterAlouan, kCarGreenSleeping, getCharacterCurrentParams(kCharacterAlouan)[0])) {
			if (getCharacterCurrentParams(kCharacterAlouan)[0] == 10000)
				getCharacterCurrentParams(kCharacterAlouan)[0] = 0;
			else
				getCharacterCurrentParams(kCharacterAlouan)[0] = 10000;
		}

		break;
	case 12:
		getCharacter(kCharacterAlouan).characterPosition.position = 0;
		getCharacter(kCharacterAlouan).characterPosition.location = 0;
		getCharacter(kCharacterAlouan).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterAlouan)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_DoCorrOtis);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterAlouan, getCharacterCurrentParams(kCharacterAlouan)[3]);
		getCharacter(kCharacterAlouan).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, _functionsAlouan[getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall]]);
		fedEx(kCharacterAlouan, kCharacterAlouan, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterAlouan, (char *)&getCharacterCurrentParams(kCharacterAlouan)[0]);
		blockAtDoor(kCharacterAlouan, getCharacterCurrentParams(kCharacterAlouan)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_DoDialog);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterAlouan).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, _functionsAlouan[getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall]]);
		fedEx(kCharacterAlouan, kCharacterAlouan, 18, 0);
		break;
	case 12:
		playDialog(kCharacterAlouan, (char *)&getCharacterCurrentParams(kCharacterAlouan)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_DoWait);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 4;

	params->clear();

	params->parameters[0] = param1.intParam;

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAlouan)[1] || (getCharacterCurrentParams(kCharacterAlouan)[1] = _gameTime + getCharacterCurrentParams(kCharacterAlouan)[0], _gameTime + getCharacterCurrentParams(kCharacterAlouan)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterAlouan)[1] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterAlouan)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterAlouan).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, _functionsAlouan[getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall]]);
		fedEx(kCharacterAlouan, kCharacterAlouan, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_DoWalk);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 5;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterAlouan, getCharacterCurrentParams(kCharacterAlouan)[0], getCharacterCurrentParams(kCharacterAlouan)[1])) {
			getCharacter(kCharacterAlouan).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, _functionsAlouan[getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall]]);
			fedEx(kCharacterAlouan, kCharacterAlouan, 18, 0);
		}

		break;
	case 5:
		playCathExcuseMe();
		break;
	case 6:
		playChrExcuseMe(kCharacterAlouan, kCharacterCath, 0);
		break;
	case 12:
		if (walk(kCharacterAlouan, getCharacterCurrentParams(kCharacterAlouan)[0], getCharacterCurrentParams(kCharacterAlouan)[1])) {
			getCharacter(kCharacterAlouan).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, _functionsAlouan[getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall]]);
			fedEx(kCharacterAlouan, kCharacterAlouan, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_PeekF(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_PeekF);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 6;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_PeekF(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlouan).characterPosition.position = 4070;
		getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 1;
		AlouanCall(&LogicManager::CONS_Alouan_DoCorrOtis, "621Cf", 6, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] == 1) {
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
			AlouanCall(&LogicManager::CONS_Alouan_DoCorrOtis, "621Df", 6, 0, 0);
		} else if (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] == 2) {
			getCharacter(kCharacterAlouan).characterPosition.position = 4070;
			endGraphics(kCharacterAlouan);
			getCharacter(kCharacterAlouan).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, _functionsAlouan[getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall]]);
			fedEx(kCharacterAlouan, kCharacterAlouan, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_PeekH(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_PeekH);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 7;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_PeekH(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlouan).characterPosition.position = 2740;
		getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 1;
		AlouanCall(&LogicManager::CONS_Alouan_DoCorrOtis, "621Ch", 8, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] == 1) {
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
			AlouanCall(&LogicManager::CONS_Alouan_DoCorrOtis, "621Dh", 8, 0, 0);
		} else if (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] == 2) {
			getCharacter(kCharacterAlouan).characterPosition.position = 2740;
			endGraphics(kCharacterAlouan);
			getCharacter(kCharacterAlouan).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, _functionsAlouan[getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall]]);
			fedEx(kCharacterAlouan, kCharacterAlouan, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_GoFtoH(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_GoFtoH);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 8;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_GoFtoH(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlouan).characterPosition.position = 4070;
		getCharacter(kCharacterAlouan).characterPosition.location = 0;
		getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 1;
		AlouanCall(&LogicManager::CONS_Alouan_DoCorrOtis, "621Bf", 6, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
			AlouanCall(&LogicManager::CONS_Alouan_DoWalk, 3, 2740, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 3;
			AlouanCall(&LogicManager::CONS_Alouan_DoCorrOtis, "621Ah", 8, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAlouan).characterPosition.location = 1;
			endGraphics(kCharacterAlouan);

			getCharacter(kCharacterAlouan).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, _functionsAlouan[getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall]]);
			fedEx(kCharacterAlouan, kCharacterAlouan, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_GoHtoF(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_GoHtoF);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 9;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_GoHtoF(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlouan).characterPosition.position = 2740;
		getCharacter(kCharacterAlouan).characterPosition.location = 0;
		getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 1;
		AlouanCall(&LogicManager::CONS_Alouan_DoCorrOtis, "621Bh", 8, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
			AlouanCall(&LogicManager::CONS_Alouan_DoWalk, 3, 4070, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 3;
			AlouanCall(&LogicManager::CONS_Alouan_DoCorrOtis, "621Af", 6, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterAlouan).characterPosition.location = 1;
			endGraphics(kCharacterAlouan);

			getCharacter(kCharacterAlouan).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, _functionsAlouan[getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall]]);
			fedEx(kCharacterAlouan, kCharacterAlouan, 18, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_Birth);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 10;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterAlouan)[0]) {
			getCharacterCurrentParams(kCharacterAlouan)[0] = 1;
			CONS_Alouan_Part1(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterAlouan).characterPosition.position = 2740;
		getCharacter(kCharacterAlouan).characterPosition.location = 1;
		getCharacter(kCharacterAlouan).characterPosition.car = kCarGreenSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_Part1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_Part1);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 11;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_Part1(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1096200 && !getCharacterCurrentParams(kCharacterAlouan)[0]) {
			getCharacterCurrentParams(kCharacterAlouan)[0] = 1;
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 1;
			AlouanCall(&LogicManager::CONS_Alouan_GoHtoF, 0, 0, 0, 0);
		} else {
			if (_gameTime > 1162800 && !getCharacterCurrentParams(kCharacterAlouan)[1]) {
				getCharacterCurrentParams(kCharacterAlouan)[1] = 1;
				send(kCharacterAlouan, kCharacterClerk, 191070912, 4070);
				getCharacter(kCharacterAlouan).characterPosition.position = 4070;
			}
			if (_gameTime > 1179000 && !getCharacterCurrentParams(kCharacterAlouan)[2]) {
				getCharacterCurrentParams(kCharacterAlouan)[2] = 1;
				send(kCharacterAlouan, kCharacterClerk, 191070912, 4840);
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
				AlouanCall(&LogicManager::CONS_Alouan_GoFtoH, 0, 0, 0, 0);
			}
		}

		break;
	case 18:
		if (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] != 1)
			getCharacter(kCharacterAlouan).characterPosition.position = 4840;

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_Asleep);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 12;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(7, kCharacterCath, 3, 10, 9);
		setDoor(5, kCharacterCath, 3, 10, 9);
		getCharacter(kCharacterAlouan).characterPosition.position = 4070;
		getCharacter(kCharacterAlouan).characterPosition.location = 1;
		getCharacter(kCharacterAlouan).characterPosition.car = kCarGreenSleeping;
		endGraphics(kCharacterAlouan);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_StartPart2);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 13;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterAlouan);
		getCharacter(kCharacterAlouan).characterPosition.position = 2740;
		getCharacter(kCharacterAlouan).characterPosition.location = 1;
		getCharacter(kCharacterAlouan).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAlouan).clothes = 0;
		getCharacter(kCharacterAlouan).inventoryItem = kItemNone;
		CONS_Alouan_Part2(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_Part2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_Part2);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 14;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_Part2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAlouan)[1] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime <= 1777500) {
				if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterAlouan)[1]) {
					getCharacterCurrentParams(kCharacterAlouan)[1] = _gameTime + 75;
					if (_gameTime == -75) {
						if (getCharacterCurrentParams(kCharacterAlouan)[0]) {
							getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 1;
							AlouanCall(&LogicManager::CONS_Alouan_PeekH, 0, 0, 0, 0);
						} else {
							getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
							AlouanCall(&LogicManager::CONS_Alouan_PeekF, 0, 0, 0, 0);
						}
					}
				}

				if (getCharacterCurrentParams(kCharacterAlouan)[1] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterAlouan)[1] = 0x7FFFFFFF;

			if (getCharacterCurrentParams(kCharacterAlouan)[0]) {
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 1;
				AlouanCall(&LogicManager::CONS_Alouan_PeekH, 0, 0, 0, 0);
			} else {
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
				AlouanCall(&LogicManager::CONS_Alouan_PeekF, 0, 0, 0, 0);
			}
		}

		break;
	case 12:
		send(kCharacterAlouan, kCharacterClerk, 191070912, 4840);
		getCharacterCurrentParams(kCharacterAlouan)[0] = 1;
		break;
	case 18:
		switch (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8]) {
		case 3:
			getCharacterCurrentParams(kCharacterAlouan)[0] = 0;
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 4;
			AlouanCall(&LogicManager::CONS_Alouan_DoDialog, "Har2011", 0, 0, 0);
			break;
		case 4:
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 5;
			AlouanCall(&LogicManager::CONS_Alouan_DoWait, 900, 0, 0, 0);
			break;
		case 5:
			send(kCharacterAlouan, kCharacterFrancois, 190219584, 0);
			break;
		default:
			break;
		}

		break;
	case 189489753:
		getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 3;
		AlouanCall(&LogicManager::CONS_Alouan_GoHtoF, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_StartPart3);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 15;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Alouan_Part3(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAlouan);
		getCharacter(kCharacterAlouan).characterPosition.position = 2740;
		getCharacter(kCharacterAlouan).characterPosition.location = 1;
		getCharacter(kCharacterAlouan).characterPosition.car = kCarGreenSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_Part3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_Part3);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 16;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_Part3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1984500 && !getCharacterCurrentParams(kCharacterAlouan)[0]) {
			getCharacterCurrentParams(kCharacterAlouan)[0] = 1;
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 1;
			AlouanCall(&LogicManager::CONS_Alouan_GoHtoF, 0, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterAlouan)[1] != 0x7FFFFFFF && _gameTime > 1989000) {
			if (_gameTime > 2119500) {
				getCharacterCurrentParams(kCharacterAlouan)[1] = 0x7FFFFFFF;
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
				AlouanCall(&LogicManager::CONS_Alouan_PeekF, 0, 0, 0, 0);
				break;
			}

			if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterAlouan)[1]) {
				getCharacterCurrentParams(kCharacterAlouan)[1] = _gameTime + 75;
				if (_gameTime == -75) {
					getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
					AlouanCall(&LogicManager::CONS_Alouan_PeekF, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterAlouan)[1] < _gameTime) {
				getCharacterCurrentParams(kCharacterAlouan)[1] = 0x7FFFFFFF;
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
				AlouanCall(&LogicManager::CONS_Alouan_PeekF, 0, 0, 0, 0);
				break;
			}
		}

		if (_gameTime > 2052000 && !getCharacterCurrentParams(kCharacterAlouan)[2]) {
			getCharacterCurrentParams(kCharacterAlouan)[2] = 1;
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 3;
			AlouanCall(&LogicManager::CONS_Alouan_DoDialog, "Har1005", 0, 0, 0);
			break;
		}

		if (_gameTime > 2133000 && !getCharacterCurrentParams(kCharacterAlouan)[3]) {
			getCharacterCurrentParams(kCharacterAlouan)[3] = 1;
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 4;
			AlouanCall(&LogicManager::CONS_Alouan_GoFtoH, 0, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterAlouan)[4] != 0x7FFFFFFF && _gameTime > 2151000) {
			if (_gameTime <= 2241000) {
				if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterAlouan)[4]) {
					getCharacterCurrentParams(kCharacterAlouan)[4] = _gameTime + 75;
					if (_gameTime == -75) {
						getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 5;
						AlouanCall(&LogicManager::CONS_Alouan_PeekH, 0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterAlouan)[4] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterAlouan)[4] = 0x7FFFFFFF;
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 5;
			AlouanCall(&LogicManager::CONS_Alouan_PeekH, 0, 0, 0, 0);
		}

		break;
	case 12:
		send(kCharacterAlouan, kCharacterClerk, 191070912, 4840);
		break;
	case 18:
		switch (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterAlouan).characterPosition.position = 4840;
			if (getCharacterCurrentParams(kCharacterAlouan)[1] != 0x7FFFFFFF && _gameTime > 1989000) {
				if (_gameTime > 2119500) {
					getCharacterCurrentParams(kCharacterAlouan)[1] = 0x7FFFFFFF;
					getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
					AlouanCall(&LogicManager::CONS_Alouan_PeekF, 0, 0, 0, 0);
					break;
				}

				if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterAlouan)[1]) {
					getCharacterCurrentParams(kCharacterAlouan)[1] = _gameTime + 75;
					if (_gameTime == -75) {
						getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
						AlouanCall(&LogicManager::CONS_Alouan_PeekF, 0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterAlouan)[1] < _gameTime) {
					getCharacterCurrentParams(kCharacterAlouan)[1] = 0x7FFFFFFF;
					getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
					AlouanCall(&LogicManager::CONS_Alouan_PeekF, 0, 0, 0, 0);
					break;
				}
			}

			// fall through
		case 2:
			if (_gameTime > 2052000 && !getCharacterCurrentParams(kCharacterAlouan)[2]) {
				getCharacterCurrentParams(kCharacterAlouan)[2] = 1;
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 3;
				AlouanCall(&LogicManager::CONS_Alouan_DoDialog, "Har1005", 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (_gameTime > 2133000 && !getCharacterCurrentParams(kCharacterAlouan)[3]) {
				getCharacterCurrentParams(kCharacterAlouan)[3] = 1;
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 4;
				AlouanCall(&LogicManager::CONS_Alouan_GoFtoH, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (getCharacterCurrentParams(kCharacterAlouan)[4] != 0x7FFFFFFF && _gameTime > 2151000) {
				if (_gameTime <= 2241000) {
					if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterAlouan)[4]) {
						getCharacterCurrentParams(kCharacterAlouan)[4] = _gameTime + 75;
						if (_gameTime == -75) {
							getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 5;
							AlouanCall(&LogicManager::CONS_Alouan_PeekH, 0, 0, 0, 0);
							break;
						}
					}

					if (getCharacterCurrentParams(kCharacterAlouan)[4] >= _gameTime)
						break;
				}

				getCharacterCurrentParams(kCharacterAlouan)[4] = 0x7FFFFFFF;
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 5;
				AlouanCall(&LogicManager::CONS_Alouan_PeekH, 0, 0, 0, 0);
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

void LogicManager::CONS_Alouan_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_StartPart4);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 17;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Alouan_Part4(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAlouan);
		getCharacter(kCharacterAlouan).characterPosition.position = 2740;
		getCharacter(kCharacterAlouan).characterPosition.location = 1;
		getCharacter(kCharacterAlouan).characterPosition.car = kCarGreenSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_Part4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_Part4);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 18;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_Part4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAlouan)[0] == 0x7FFFFFFF || !_gameTime) {
			if (_gameTime > 2455200 && !getCharacterCurrentParams(kCharacterAlouan)[1]) {
				getCharacterCurrentParams(kCharacterAlouan)[1] = 1;
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
				AlouanCall(&LogicManager::CONS_Alouan_GoHtoF, 0, 0, 0, 0);
			} else if (_gameTime > 2475000 && !getCharacterCurrentParams(kCharacterAlouan)[2]) {
				getCharacterCurrentParams(kCharacterAlouan)[2] = 1;
				send(kCharacterAlouan, kCharacterClerk, 191070912, 4840);
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 3;
				AlouanCall(&LogicManager::CONS_Alouan_GoFtoH, 0, 0, 0, 0);
			}

			break;
		}

		if (_gameTime <= 2443500) {
			if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterAlouan)[0]) {
				getCharacterCurrentParams(kCharacterAlouan)[0] = _gameTime + 75;
				if (_gameTime == -75) {
					getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 1;
					AlouanCall(&LogicManager::CONS_Alouan_PeekH, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterAlouan)[0] >= _gameTime) {
				if (_gameTime > 2455200 && !getCharacterCurrentParams(kCharacterAlouan)[1]) {
					getCharacterCurrentParams(kCharacterAlouan)[1] = 1;
					getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
					AlouanCall(&LogicManager::CONS_Alouan_GoHtoF, 0, 0, 0, 0);
				} else if (_gameTime > 2475000 && !getCharacterCurrentParams(kCharacterAlouan)[2]) {
					getCharacterCurrentParams(kCharacterAlouan)[2] = 1;
					send(kCharacterAlouan, kCharacterClerk, 191070912, 4840);
					getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 3;
					AlouanCall(&LogicManager::CONS_Alouan_GoFtoH, 0, 0, 0, 0);
				}

				break;
			}
		}

		getCharacterCurrentParams(kCharacterAlouan)[0] = 0x7FFFFFFF;
		getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 1;
		AlouanCall(&LogicManager::CONS_Alouan_PeekH, 0, 0, 0, 0);
		break;
	case 12:
		send(kCharacterAlouan, kCharacterClerk, 191070912, 4840);
		break;
	case 18:
		if (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] != 1) {
			if (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] != 2)
				break;

			send(kCharacterAlouan, kCharacterClerk, 191070912, 4070);

			if (_gameTime > 2475000 && !getCharacterCurrentParams(kCharacterAlouan)[2]) {
				getCharacterCurrentParams(kCharacterAlouan)[2] = 1;
				send(kCharacterAlouan, kCharacterClerk, 191070912, 4840);
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 3;
				AlouanCall(&LogicManager::CONS_Alouan_GoFtoH, 0, 0, 0, 0);
			}
		} else {
			if (_gameTime > 2455200 && !getCharacterCurrentParams(kCharacterAlouan)[1]) {
				getCharacterCurrentParams(kCharacterAlouan)[1] = 1;
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
				AlouanCall(&LogicManager::CONS_Alouan_GoHtoF, 0, 0, 0, 0);
			} else if (_gameTime > 2475000 && !getCharacterCurrentParams(kCharacterAlouan)[2]) {
				getCharacterCurrentParams(kCharacterAlouan)[2] = 1;
				send(kCharacterAlouan, kCharacterClerk, 191070912, 4840);
				getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 3;
				AlouanCall(&LogicManager::CONS_Alouan_GoFtoH, 0, 0, 0, 0);
			}
		}

		break;
	}
}

void LogicManager::CONS_Alouan_Asleep4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_Asleep4);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 19;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_Asleep4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(7, kCharacterCath, 3, 10, 9);
		setDoor(5, kCharacterCath, 3, 10, 9);
		getCharacter(kCharacterAlouan).characterPosition.position = 2740;
		getCharacter(kCharacterAlouan).characterPosition.location = 1;
		getCharacter(kCharacterAlouan).characterPosition.car = kCarGreenSleeping;
		endGraphics(kCharacterAlouan);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_StartPart5);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 20;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Alouan_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterAlouan);
		getCharacter(kCharacterAlouan).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterAlouan).characterPosition.position = 3969;
		getCharacter(kCharacterAlouan).characterPosition.location = 1;
		getCharacter(kCharacterAlouan).clothes = 0;
		getCharacter(kCharacterAlouan).inventoryItem = kItemNone;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_Prisoner);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 21;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Alouan_Free(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_Free(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_Free);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 22;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_Free(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterAlouan)[0] ||
			(getCharacterCurrentParams(kCharacterAlouan)[0] = _gameTime + 2700, _gameTime != -2700)) {
			if (getCharacterCurrentParams(kCharacterAlouan)[0] >= _gameTime)
				break;

			getCharacterCurrentParams(kCharacterAlouan)[0] = 0x7FFFFFFF;
		}

		CONS_Alouan_Hiding(0, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterAlouan).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterAlouan).characterPosition.position = 5000;
		getCharacter(kCharacterAlouan).characterPosition.location = 0;
		break;
	case 17:
		if (checkLoc(kCharacterCath, 3)) {
			CONS_Alouan_Hiding(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_Hiding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_Hiding);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 23;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_Hiding(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 1;
		AlouanCall(&LogicManager::CONS_Alouan_DoWalk, 3, 4070, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] == 1) {
			getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] = 2;
			AlouanCall(&LogicManager::CONS_Alouan_DoCorrOtis, "619AF", 6, 0, 0);
		} else if (getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall + 8] == 2) {
			endGraphics(kCharacterAlouan);
			getCharacter(kCharacterAlouan).characterPosition.location = 1;
			getCharacter(kCharacterAlouan).characterPosition.position = 4070;
			setDoor(6, kCharacterCath, 1, 10, 9);
		}

		break;
	case 135800432:
		CONS_Alouan_Disappear(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Alouan_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterAlouan).callParams[getCharacter(kCharacterAlouan).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterAlouan, &LogicManager::HAND_Alouan_Disappear);
	getCharacter(kCharacterAlouan).callbacks[getCharacter(kCharacterAlouan).currentCall] = 24;

	params->clear();

	fedEx(kCharacterAlouan, kCharacterAlouan, 12, 0);
}

void LogicManager::HAND_Alouan_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsAlouan[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Alouan_DebugWalks,
	&LogicManager::HAND_Alouan_DoCorrOtis,
	&LogicManager::HAND_Alouan_DoDialog,
	&LogicManager::HAND_Alouan_DoWait,
	&LogicManager::HAND_Alouan_DoWalk,
	&LogicManager::HAND_Alouan_PeekF,
	&LogicManager::HAND_Alouan_PeekH,
	&LogicManager::HAND_Alouan_GoFtoH,
	&LogicManager::HAND_Alouan_GoHtoF,
	&LogicManager::HAND_Alouan_Birth,
	&LogicManager::HAND_Alouan_Part1,
	&LogicManager::HAND_Alouan_Asleep,
	&LogicManager::HAND_Alouan_StartPart2,
	&LogicManager::HAND_Alouan_Part2,
	&LogicManager::HAND_Alouan_StartPart3,
	&LogicManager::HAND_Alouan_Part3,
	&LogicManager::HAND_Alouan_StartPart4,
	&LogicManager::HAND_Alouan_Part4,
	&LogicManager::HAND_Alouan_Asleep4,
	&LogicManager::HAND_Alouan_StartPart5,
	&LogicManager::HAND_Alouan_Prisoner,
	&LogicManager::HAND_Alouan_Free,
	&LogicManager::HAND_Alouan_Hiding,
	&LogicManager::HAND_Alouan_Disappear
};

} // End of namespace LastExpress
