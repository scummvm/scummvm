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

void LogicManager::CONS_Hadija(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterHadija,
			_functionsHadija[getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall]]
		);

		break;
	case 1:
		CONS_Hadija_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Hadija_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Hadija_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Hadija_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Hadija_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::HadijaCall(CALL_PARAMS) {
	getCharacter(kCharacterHadija).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Hadija_DebugWalks(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_DebugWalks);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 1;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_DebugWalks(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterHadija, kCarGreenSleeping, getCharacterCurrentParams(kCharacterHadija)[0])) {
			if (getCharacterCurrentParams(kCharacterHadija)[0] == 10000) {
				getCharacterCurrentParams(kCharacterHadija)[0] = 0;
			} else {
				getCharacterCurrentParams(kCharacterHadija)[0] = 10000;
			}
		}

		break;
	case 12:
		getCharacter(kCharacterHadija).characterPosition.position = 0;
		getCharacter(kCharacterHadija).characterPosition.location = 0;
		getCharacter(kCharacterHadija).characterPosition.car = kCarGreenSleeping;
		getCharacterCurrentParams(kCharacterHadija)[0] = 10000;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_DoCorrOtis(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_DoCorrOtis);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	params->parameters[3] = param2.intParam;

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_DoCorrOtis(HAND_PARAMS) {
	switch (msg->action) {
	case 3:
		releaseAtDoor(kCharacterHadija, getCharacterCurrentParams(kCharacterHadija)[3]);
		getCharacter(kCharacterHadija).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHadija, _functionsHadija[getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall]]);
		fedEx(kCharacterHadija, kCharacterHadija, 18, 0);
		break;
	case 12:
		startSeqOtis(kCharacterHadija, (char *)&getCharacterCurrentParams(kCharacterHadija)[0]);
		blockAtDoor(kCharacterHadija, getCharacterCurrentParams(kCharacterHadija)[3]);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_DoDialog(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_DoDialog);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_DoDialog(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		getCharacter(kCharacterHadija).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHadija, _functionsHadija[getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall]]);
		fedEx(kCharacterHadija, kCharacterHadija, 18, 0);
		break;
	case 12:
		playDialog(kCharacterHadija, (char *)&getCharacterCurrentParams(kCharacterHadija)[0], -1, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_DoWait(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_DoWait);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 4;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_DoWait(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterHadija)[1] || (getCharacterCurrentParams(kCharacterHadija)[1] = _gameTime + getCharacterCurrentParams(kCharacterHadija)[0], _gameTime + getCharacterCurrentParams(kCharacterHadija)[0] != 0)) {
			if (getCharacterCurrentParams(kCharacterHadija)[1] >= _gameTime)
				return;

			getCharacterCurrentParams(kCharacterHadija)[1] = 0x7FFFFFFF;
		}

		getCharacter(kCharacterHadija).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterHadija, _functionsHadija[getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall]]);
		fedEx(kCharacterHadija, kCharacterHadija, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_DoWalk(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_DoWalk);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 5;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_DoWalk(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (walk(kCharacterHadija, getCharacterCurrentParams(kCharacterHadija)[0], getCharacterCurrentParams(kCharacterHadija)[1])) {
			getCharacter(kCharacterHadija).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHadija, _functionsHadija[getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall]]);
			fedEx(kCharacterHadija, kCharacterHadija, 18, 0);
		}

		break;
	case 5:
		playCathExcuseMe();
		break;
	case 6:
		playChrExcuseMe(kCharacterHadija, kCharacterCath, 0);
		break;
	case 12:
		if (walk(kCharacterHadija, getCharacterCurrentParams(kCharacterHadija)[0], getCharacterCurrentParams(kCharacterHadija)[1])) {
			getCharacter(kCharacterHadija).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHadija, _functionsHadija[getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall]]);
			fedEx(kCharacterHadija, kCharacterHadija, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_PeekF(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_PeekF);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 6;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_PeekF(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHadija).characterPosition.position = 2740;
		getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 1;
		HadijaCall(&LogicManager::CONS_Hadija_DoCorrOtis, "619Cf", 6, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] == 1) {
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
			HadijaCall(&LogicManager::CONS_Hadija_DoCorrOtis, "619Df", 6, 0, 0);
		} else if (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] == 2) {
			getCharacter(kCharacterHadija).characterPosition.position = 4070;
			endGraphics(kCharacterHadija);

			getCharacter(kCharacterHadija).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHadija, _functionsHadija[getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall]]);
			fedEx(kCharacterHadija, kCharacterHadija, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_PeekH(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_PeekH);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 7;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_PeekH(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHadija).characterPosition.position = 2740;
		getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 1;
		HadijaCall(&LogicManager::CONS_Hadija_DoCorrOtis, "619Ch", 8, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] == 1) {
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
			HadijaCall(&LogicManager::CONS_Hadija_DoCorrOtis, "619Dh", 8, 0, 0);
		} else if (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] == 2) {
			getCharacter(kCharacterHadija).characterPosition.position = 2740;
			endGraphics(kCharacterHadija);

			getCharacter(kCharacterHadija).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHadija, _functionsHadija[getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall]]);
			fedEx(kCharacterHadija, kCharacterHadija, 18, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_GoFtoH(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_GoFtoH);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 8;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_GoFtoH(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHadija).characterPosition.position = 4070;
		getCharacter(kCharacterHadija).characterPosition.location = 0;
		getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 1;
		HadijaCall(&LogicManager::CONS_Hadija_DoCorrOtis, "619Bf", 6, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
			HadijaCall(&LogicManager::CONS_Hadija_DoWalk, 3, 2740, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
			HadijaCall(&LogicManager::CONS_Hadija_DoCorrOtis, "619Ah", 8, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterHadija).characterPosition.location = 1;
			endGraphics(kCharacterHadija);

			getCharacter(kCharacterHadija).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHadija, _functionsHadija[getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall]]);
			fedEx(kCharacterHadija, kCharacterHadija, 18, 0);
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_GoHtoF(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_GoHtoF);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 9;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_GoHtoF(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHadija).characterPosition.position = 2740;
		getCharacter(kCharacterHadija).characterPosition.location = 0;
		getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 1;
		HadijaCall(&LogicManager::CONS_Hadija_DoCorrOtis, "619Bh", 8, 0, 0);
		break;
	case 18:
		switch (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8]) {
		case 1:
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
			HadijaCall(&LogicManager::CONS_Hadija_DoWalk, 3, 4070, 0, 0);
			break;
		case 2:
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
			HadijaCall(&LogicManager::CONS_Hadija_DoCorrOtis, "619Af", 6, 0, 0);
			break;
		case 3:
			getCharacter(kCharacterHadija).characterPosition.location = 1;
			endGraphics(kCharacterHadija);

			getCharacter(kCharacterHadija).currentCall--;
			_engine->getMessageManager()->setMessageHandle(kCharacterHadija, _functionsHadija[getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall]]);
			fedEx(kCharacterHadija, kCharacterHadija, 18, 0);
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_Birth);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 10;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1062000 && !getCharacterCurrentParams(kCharacterHadija)[0]) {
			getCharacterCurrentParams(kCharacterHadija)[0] = 1;
			CONS_Hadija_Part1(0, 0, 0, 0);
		}

		break;
	case 12:
		getCharacter(kCharacterHadija).characterPosition.position = 4070;
		getCharacter(kCharacterHadija).characterPosition.location = 1;
		getCharacter(kCharacterHadija).characterPosition.car = kCarGreenSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_Part1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_Part1);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 11;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_Part1(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1075500 && !getCharacterCurrentParams(kCharacterHadija)[0]) {
			getCharacterCurrentParams(kCharacterHadija)[0] = 1;
			getCharacter(kCharacterHadija).characterPosition.position = 4840;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 1;
			HadijaCall(&LogicManager::CONS_Hadija_DoDialog, "Har1100", 0, 0, 0);
			break;
		}

		if (_gameTime > 1084500 && !getCharacterCurrentParams(kCharacterHadija)[1]) {
			getCharacterCurrentParams(kCharacterHadija)[1] = 1;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
			HadijaCall(&LogicManager::CONS_Hadija_GoFtoH, 0, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterHadija)[2] != 0x7FFFFFFF && _gameTime > 1093500) {
			if (_gameTime > 1134000) {
				getCharacterCurrentParams(kCharacterHadija)[2] = 0x7FFFFFFF;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
				HadijaCall(&LogicManager::CONS_Hadija_PeekH, 0, 0, 0, 0);
				break;
			}

			if (!cathInCorridor(kCarGreenSleeping) ||
				!inComp(kCharacterMahmud, kCarGreenSleeping, 5790) ||
				!getCharacterCurrentParams(kCharacterHadija)[2]) {
				getCharacterCurrentParams(kCharacterHadija)[2] = _gameTime + 75;
				if (_gameTime == -75) {
					getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
					HadijaCall(&LogicManager::CONS_Hadija_PeekH, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterHadija)[2] < _gameTime) {
				getCharacterCurrentParams(kCharacterHadija)[2] = 0x7FFFFFFF;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
				HadijaCall(&LogicManager::CONS_Hadija_PeekH, 0, 0, 0, 0);
				break;
			}
		}

		if (_gameTime > 1156500 && !getCharacterCurrentParams(kCharacterHadija)[3]) {
			getCharacterCurrentParams(kCharacterHadija)[3] = 1;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 4;
			HadijaCall(&LogicManager::CONS_Hadija_GoHtoF, 0, 0, 0, 0);
			return;
		}

		if (getCharacterCurrentParams(kCharacterHadija)[4] != 0x7FFFFFFF && _gameTime > 1165500) {
			if (_gameTime <= 1188000) {
				if (!cathInCorridor(kCarGreenSleeping) || !inComp(kCharacterMahmud, kCarGreenSleeping, 5790) ||
					!getCharacterCurrentParams(kCharacterHadija)[4]) {
					getCharacterCurrentParams(kCharacterHadija)[4] = _gameTime + 75;
					if (_gameTime == -75) {
						getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 5;
						HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterHadija)[4] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterHadija)[4] = 0x7FFFFFFF;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 5;
			HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8]) {
		case 1:
			if (_gameTime > 1084500 && !getCharacterCurrentParams(kCharacterHadija)[1]) {
				getCharacterCurrentParams(kCharacterHadija)[1] = 1;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
				HadijaCall(&LogicManager::CONS_Hadija_GoFtoH, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getCharacterCurrentParams(kCharacterHadija)[2] != 0x7FFFFFFF && _gameTime > 1093500) {
				if (_gameTime > 1134000) {
					getCharacterCurrentParams(kCharacterHadija)[2] = 0x7FFFFFFF;
					getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
					HadijaCall(&LogicManager::CONS_Hadija_PeekH, 0, 0, 0, 0);
					break;
				}

				if (!cathInCorridor(kCarGreenSleeping) ||
					!inComp(kCharacterMahmud, kCarGreenSleeping, 5790) ||
					!getCharacterCurrentParams(kCharacterHadija)[2]) {
					getCharacterCurrentParams(kCharacterHadija)[2] = _gameTime + 75;
					if (_gameTime == -75) {
						getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
						HadijaCall(&LogicManager::CONS_Hadija_PeekH, 0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterHadija)[2] < _gameTime) {
					getCharacterCurrentParams(kCharacterHadija)[2] = 0x7FFFFFFF;
					getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
					HadijaCall(&LogicManager::CONS_Hadija_PeekH, 0, 0, 0, 0);
					break;
				}
			}

			// fall through
		case 3:
			if (_gameTime > 1156500 && !getCharacterCurrentParams(kCharacterHadija)[3]) {
				getCharacterCurrentParams(kCharacterHadija)[3] = 1;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 4;
				HadijaCall(&LogicManager::CONS_Hadija_GoHtoF, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (getCharacterCurrentParams(kCharacterHadija)[4] != 0x7FFFFFFF && _gameTime > 1165500) {
				if (_gameTime <= 1188000) {
					if (!cathInCorridor(kCarGreenSleeping) || !inComp(kCharacterMahmud, kCarGreenSleeping, 5790) ||
						!getCharacterCurrentParams(kCharacterHadija)[4]) {
						getCharacterCurrentParams(kCharacterHadija)[4] = _gameTime + 75;
						if (_gameTime == -75) {
							getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 5;
							HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
							break;
						}
					}

					if (getCharacterCurrentParams(kCharacterHadija)[4] >= _gameTime)
						break;
				}

				getCharacterCurrentParams(kCharacterHadija)[4] = 0x7FFFFFFF;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 5;
				HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
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

void LogicManager::CONS_Hadija_Asleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_Asleep);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 12;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_Asleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(8, kCharacterCath, 3, 10, 9);
		getCharacter(kCharacterHadija).characterPosition.position = 2740;
		getCharacter(kCharacterHadija).characterPosition.location = 1;
		getCharacter(kCharacterHadija).characterPosition.car = kCarGreenSleeping;
		endGraphics(kCharacterHadija);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_StartPart2);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 13;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterHadija);
		getCharacter(kCharacterHadija).characterPosition.position = 3050;
		getCharacter(kCharacterHadija).characterPosition.location = 1;
		getCharacter(kCharacterHadija).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterHadija).clothes = 0;
		getCharacter(kCharacterHadija).inventoryItem = kItemNone;
		CONS_Hadija_Part2(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_Part2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_Part2);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 14;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_Part2(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1782000 && !getCharacterCurrentParams(kCharacterHadija)[0]) {
			getCharacterCurrentParams(kCharacterHadija)[0] = 1;
			getCharacter(kCharacterHadija).characterPosition.position = 2740;
		}

		if (getCharacterCurrentParams(kCharacterHadija)[1] == 0x7FFFFFFF || _gameTime <= 1786500) {
			if (_gameTime > 1822500 && !getCharacterCurrentParams(kCharacterHadija)[2]) {
				getCharacterCurrentParams(kCharacterHadija)[2] = 1;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
				HadijaCall(&LogicManager::CONS_Hadija_GoHtoF, 0, 0, 0, 0);
			}

			break;
		}

		if (_gameTime <= 1818000) {
			if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterHadija)[1]) {
				getCharacterCurrentParams(kCharacterHadija)[1] = _gameTime + 75;
				if (_gameTime == -75) {
					getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 1;
					HadijaCall(&LogicManager::CONS_Hadija_PeekH, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterHadija)[1] >= _gameTime) {
				if (_gameTime > 1822500 && !getCharacterCurrentParams(kCharacterHadija)[2]) {
					getCharacterCurrentParams(kCharacterHadija)[2] = 1;
					getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
					HadijaCall(&LogicManager::CONS_Hadija_GoHtoF, 0, 0, 0, 0);
				}

				break;
			}
		}

		getCharacterCurrentParams(kCharacterHadija)[1] = 0x7FFFFFFF;
		getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 1;
		HadijaCall(&LogicManager::CONS_Hadija_PeekH, 0, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] != 1) {
			if (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] == 2) {
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
				HadijaCall(&LogicManager::CONS_Hadija_DoDialog, "Har2012", 0, 0, 0);
			}

			break;
		}

		if (_gameTime > 1822500 && !getCharacterCurrentParams(kCharacterHadija)[2]) {
			getCharacterCurrentParams(kCharacterHadija)[2] = 1;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
			HadijaCall(&LogicManager::CONS_Hadija_GoHtoF, 0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_StartPart3);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 15;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Hadija_Part3(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterHadija);
		getCharacter(kCharacterHadija).characterPosition.position = 4070;
		getCharacter(kCharacterHadija).characterPosition.location = 1;
		getCharacter(kCharacterHadija).characterPosition.car = kCarGreenSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_Part3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_Part3);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 16;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_Part3(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 1998000 && !getCharacterCurrentParams(kCharacterHadija)[0]) {
			getCharacterCurrentParams(kCharacterHadija)[0] = 1;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 1;
			HadijaCall(&LogicManager::CONS_Hadija_GoFtoH, 0, 0, 0, 0);
			return;
		}

		if (_gameTime > 2020500 && !getCharacterCurrentParams(kCharacterHadija)[1]) {
			getCharacterCurrentParams(kCharacterHadija)[1] = 1;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
			HadijaCall(&LogicManager::CONS_Hadija_GoHtoF, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2079000 && !getCharacterCurrentParams(kCharacterHadija)[2]) {
			getCharacterCurrentParams(kCharacterHadija)[2] = 1;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
			HadijaCall(&LogicManager::CONS_Hadija_GoFtoH, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2187000 && !getCharacterCurrentParams(kCharacterHadija)[3]) {
			getCharacterCurrentParams(kCharacterHadija)[3] = 1;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 4;
			HadijaCall(&LogicManager::CONS_Hadija_GoHtoF, 0, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterHadija)[4] != 0x7FFFFFFF && _gameTime > 2196000) {
			if (_gameTime <= 2254500) {
				if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterHadija)[4]) {
					getCharacterCurrentParams(kCharacterHadija)[4] = _gameTime + 75;
					if (_gameTime == -75) {
						getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 5;
						HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterHadija)[4] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterHadija)[4] = 0x7FFFFFFF;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 5;
			HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8]) {
		case 1:
			if (_gameTime > 2020500 && !getCharacterCurrentParams(kCharacterHadija)[1]) {
				getCharacterCurrentParams(kCharacterHadija)[1] = 1;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
				HadijaCall(&LogicManager::CONS_Hadija_GoHtoF, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (_gameTime > 2079000 && !getCharacterCurrentParams(kCharacterHadija)[2]) {
				getCharacterCurrentParams(kCharacterHadija)[2] = 1;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
				HadijaCall(&LogicManager::CONS_Hadija_GoFtoH, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (_gameTime > 2187000 && !getCharacterCurrentParams(kCharacterHadija)[3]) {
				getCharacterCurrentParams(kCharacterHadija)[3] = 1;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 4;
				HadijaCall(&LogicManager::CONS_Hadija_GoHtoF, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (getCharacterCurrentParams(kCharacterHadija)[4] != 0x7FFFFFFF && _gameTime > 2196000) {
				if (_gameTime <= 2254500) {
					if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterHadija)[4]) {
						getCharacterCurrentParams(kCharacterHadija)[4] = _gameTime + 75;
						if (_gameTime == -75) {
							getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 5;
							HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
							break;
						}
					}

					if (getCharacterCurrentParams(kCharacterHadija)[4] >= _gameTime)
						break;
				}

				getCharacterCurrentParams(kCharacterHadija)[4] = 0x7FFFFFFF;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 5;
				HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
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

void LogicManager::CONS_Hadija_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_StartPart4);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 17;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Hadija_Part4(0, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterHadija).characterPosition.position = 4070;
		getCharacter(kCharacterHadija).characterPosition.location = 1;
		getCharacter(kCharacterHadija).characterPosition.car = kCarGreenSleeping;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_Part4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_Part4);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 18;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_Part4(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterHadija)[0] != 0x7FFFFFFF && _gameTime) {
			if (_gameTime > 1714500) {
				getCharacterCurrentParams(kCharacterHadija)[0] = 0x7FFFFFFF;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 1;
				HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
				break;
			}

			if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterHadija)[0]) {
				getCharacterCurrentParams(kCharacterHadija)[0] = _gameTime + 75;
				if (_gameTime == -75) {
					getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 1;
					HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
					break;
				}
			}

			if (getCharacterCurrentParams(kCharacterHadija)[0] < _gameTime) {
				getCharacterCurrentParams(kCharacterHadija)[0] = 0x7FFFFFFF;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 1;
				HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
				break;
			}
		}

		if (_gameTime > 2367000 && !getCharacterCurrentParams(kCharacterHadija)[1]) {
			getCharacterCurrentParams(kCharacterHadija)[1] = 1;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
			HadijaCall(&LogicManager::CONS_Hadija_GoFtoH, 0, 0, 0, 0);
			break;
		}

		if (_gameTime > 2421000 && !getCharacterCurrentParams(kCharacterHadija)[2]) {
			getCharacterCurrentParams(kCharacterHadija)[2] = 1;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
			HadijaCall(&LogicManager::CONS_Hadija_GoHtoF, 0, 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterHadija)[3] != 0x7FFFFFFF && _gameTime > 2425500) {
			if (_gameTime <= 2484000) {
				if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterHadija)[3]) {
					getCharacterCurrentParams(kCharacterHadija)[3] = _gameTime + 75;
					if (_gameTime == -75) {
						getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 4;
						HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
						break;
					}
				}

				if (getCharacterCurrentParams(kCharacterHadija)[3] >= _gameTime)
					break;
			}

			getCharacterCurrentParams(kCharacterHadija)[3] = 0x7FFFFFFF;
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 4;
			HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8]) {
		case 1:
			if (_gameTime > 2367000 && !getCharacterCurrentParams(kCharacterHadija)[1]) {
				getCharacterCurrentParams(kCharacterHadija)[1] = 1;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
				HadijaCall(&LogicManager::CONS_Hadija_GoFtoH, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (_gameTime > 2421000 && !getCharacterCurrentParams(kCharacterHadija)[2]) {
				getCharacterCurrentParams(kCharacterHadija)[2] = 1;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 3;
				HadijaCall(&LogicManager::CONS_Hadija_GoHtoF, 0, 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (getCharacterCurrentParams(kCharacterHadija)[3] != 0x7FFFFFFF && _gameTime > 2425500) {
				if (_gameTime <= 2484000) {
					if (!cathInCorridor(kCarGreenSleeping) || !getCharacterCurrentParams(kCharacterHadija)[3]) {
						getCharacterCurrentParams(kCharacterHadija)[3] = _gameTime + 75;
						if (_gameTime == -75) {
							getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 4;
							HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
							break;
						}
					}

					if (getCharacterCurrentParams(kCharacterHadija)[3] >= _gameTime)
						break;
				}

				getCharacterCurrentParams(kCharacterHadija)[3] = 0x7FFFFFFF;
				getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 4;
				HadijaCall(&LogicManager::CONS_Hadija_PeekF, 0, 0, 0, 0);
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

void LogicManager::CONS_Hadija_Asleep4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_Asleep4);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 19;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_Asleep4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		setDoor(8, kCharacterCath, 3, 10, 9);
		getCharacter(kCharacterHadija).characterPosition.position = 4070;
		getCharacter(kCharacterHadija).characterPosition.location = 1;
		getCharacter(kCharacterHadija).characterPosition.car = kCarGreenSleeping;
		endGraphics(kCharacterHadija);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_StartPart5);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 20;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		CONS_Hadija_Prisoner(0, 0, 0, 0);
		break;
	case 12:
		endGraphics(kCharacterHadija);
		getCharacter(kCharacterHadija).characterPosition.car = kCarRestaurant;
		getCharacter(kCharacterHadija).characterPosition.position = 3969;
		getCharacter(kCharacterHadija).characterPosition.location = 1;
		getCharacter(kCharacterHadija).clothes = 0;
		getCharacter(kCharacterHadija).inventoryItem = kItemNone;
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_Prisoner(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_Prisoner);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 21;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_Prisoner(HAND_PARAMS) {
	switch (msg->action) {
	case 70549068:
		CONS_Hadija_Free(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_Free(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_Free);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 22;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_Free(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (getCharacterCurrentParams(kCharacterHadija)[0] ||
			(getCharacterCurrentParams(kCharacterHadija)[0] = _gameTime + 2700, _gameTime != -2700)) {
			if (getCharacterCurrentParams(kCharacterHadija)[0] >= _gameTime)
				return;
			getCharacterCurrentParams(kCharacterHadija)[0] = 0x7FFFFFFF;
		}

		CONS_Hadija_Hiding(0, 0, 0, 0);
		break;
	case 12:
		getCharacter(kCharacterHadija).characterPosition.car = kCarGreenSleeping;
		getCharacter(kCharacterHadija).characterPosition.position = 5000;
		getCharacter(kCharacterHadija).characterPosition.location = 0;
		break;
	case 17:
		if (checkLoc(kCharacterCath, kCarGreenSleeping)) {
			CONS_Hadija_Hiding(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_Hiding(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_Hiding);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 23;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_Hiding(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 1;
		HadijaCall(&LogicManager::CONS_Hadija_DoWalk, 3, 4070, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] == 1) {
			getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] = 2;
			HadijaCall(&LogicManager::CONS_Hadija_DoCorrOtis, "619AF", 6, 0, 0);
		} else if (getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall + 8] == 2) {
			endGraphics(kCharacterHadija);
			getCharacter(kCharacterHadija).characterPosition.location = 1;
			getCharacter(kCharacterHadija).characterPosition.position = 4840;
			setDoor(5, kCharacterCath, 1, 10, 9);
		}

		break;
	case 135800432:
		CONS_Hadija_Disappear(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Hadija_Disappear(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterHadija).callParams[getCharacter(kCharacterHadija).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterHadija, &LogicManager::HAND_Hadija_Disappear);
	getCharacter(kCharacterHadija).callbacks[getCharacter(kCharacterHadija).currentCall] = 24;

	params->clear();

	fedEx(kCharacterHadija, kCharacterHadija, 12, 0);
}

void LogicManager::HAND_Hadija_Disappear(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsHadija[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Hadija_DebugWalks,
	&LogicManager::HAND_Hadija_DoCorrOtis,
	&LogicManager::HAND_Hadija_DoDialog,
	&LogicManager::HAND_Hadija_DoWait,
	&LogicManager::HAND_Hadija_DoWalk,
	&LogicManager::HAND_Hadija_PeekF,
	&LogicManager::HAND_Hadija_PeekH,
	&LogicManager::HAND_Hadija_GoFtoH,
	&LogicManager::HAND_Hadija_GoHtoF,
	&LogicManager::HAND_Hadija_Birth,
	&LogicManager::HAND_Hadija_Part1,
	&LogicManager::HAND_Hadija_Asleep,
	&LogicManager::HAND_Hadija_StartPart2,
	&LogicManager::HAND_Hadija_Part2,
	&LogicManager::HAND_Hadija_StartPart3,
	&LogicManager::HAND_Hadija_Part3,
	&LogicManager::HAND_Hadija_StartPart4,
	&LogicManager::HAND_Hadija_Part4,
	&LogicManager::HAND_Hadija_Asleep4,
	&LogicManager::HAND_Hadija_StartPart5,
	&LogicManager::HAND_Hadija_Prisoner,
	&LogicManager::HAND_Hadija_Free,
	&LogicManager::HAND_Hadija_Hiding,
	&LogicManager::HAND_Hadija_Disappear
};

} // End of namespace LastExpress
