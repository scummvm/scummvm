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

void LogicManager::CONS_Mitchell(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMitchell,
			_functionsMitchell[getCharacter(kCharacterMitchell).callbacks[getCharacter(kCharacterMitchell).currentCall]]
		);

		break;
	case 1:
		CONS_Mitchell_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Mitchell_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Mitchell_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Mitchell_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Mitchell_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mitchell_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMitchell).callParams[getCharacter(kCharacterMitchell).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMitchell, &LogicManager::HAND_Mitchell_Birth);
	getCharacter(kCharacterMitchell).callbacks[getCharacter(kCharacterMitchell).currentCall] = 1;

	params->clear();

	fedEx(kCharacterMitchell, kCharacterMitchell, 12, 0);
}

void LogicManager::HAND_Mitchell_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_Mitchell_OhBabyBaby(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mitchell_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMitchell).callParams[getCharacter(kCharacterMitchell).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMitchell, &LogicManager::HAND_Mitchell_StartPart2);
	getCharacter(kCharacterMitchell).callbacks[getCharacter(kCharacterMitchell).currentCall] = 2;

	params->clear();

	fedEx(kCharacterMitchell, kCharacterMitchell, 12, 0);
}

void LogicManager::HAND_Mitchell_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_Mitchell_OhBabyBaby(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mitchell_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMitchell).callParams[getCharacter(kCharacterMitchell).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMitchell, &LogicManager::HAND_Mitchell_StartPart3);
	getCharacter(kCharacterMitchell).callbacks[getCharacter(kCharacterMitchell).currentCall] = 3;

	params->clear();

	fedEx(kCharacterMitchell, kCharacterMitchell, 12, 0);
}

void LogicManager::HAND_Mitchell_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_Mitchell_OhBabyBaby(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mitchell_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMitchell).callParams[getCharacter(kCharacterMitchell).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMitchell, &LogicManager::HAND_Mitchell_StartPart4);
	getCharacter(kCharacterMitchell).callbacks[getCharacter(kCharacterMitchell).currentCall] = 4;

	params->clear();

	fedEx(kCharacterMitchell, kCharacterMitchell, 12, 0);
}

void LogicManager::HAND_Mitchell_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_Mitchell_OhBabyBaby(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mitchell_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMitchell).callParams[getCharacter(kCharacterMitchell).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMitchell, &LogicManager::HAND_Mitchell_StartPart5);
	getCharacter(kCharacterMitchell).callbacks[getCharacter(kCharacterMitchell).currentCall] = 5;

	params->clear();

	fedEx(kCharacterMitchell, kCharacterMitchell, 12, 0);
}

void LogicManager::HAND_Mitchell_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_Mitchell_OhBabyBaby(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Mitchell_OhBabyBaby(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMitchell).callParams[getCharacter(kCharacterMitchell).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMitchell, &LogicManager::HAND_Mitchell_OhBabyBaby);
	getCharacter(kCharacterMitchell).callbacks[getCharacter(kCharacterMitchell).currentCall] = 6;

	params->clear();

	fedEx(kCharacterMitchell, kCharacterMitchell, 12, 0);
}

void LogicManager::HAND_Mitchell_OhBabyBaby(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterMitchell).characterPosition.car = getCharacter(kCharacterCath).characterPosition.car;
		if (_mitchellSequence.c_str()[0] && !_mitchellFlag) {
			_mitchellFlag = true;
			startSeqOtis(kCharacterMitchell, _mitchellSequence.c_str());
		}

		break;
	case 3:
		startSeqOtis(kCharacterMitchell, _mitchellSequence.c_str());
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsMitchell[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Mitchell_Birth,
	&LogicManager::HAND_Mitchell_StartPart2,
	&LogicManager::HAND_Mitchell_StartPart3,
	&LogicManager::HAND_Mitchell_StartPart4,
	&LogicManager::HAND_Mitchell_StartPart5,
	&LogicManager::HAND_Mitchell_OhBabyBaby,
};

} // End of namespace LastExpress
