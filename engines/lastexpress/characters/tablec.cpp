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

void LogicManager::CONS_TableC(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterTableC,
			_functionsTableC[getCharacter(kCharacterTableC).callbacks[getCharacter(kCharacterTableC).currentCall]]
		);

		break;
	case 1:
		CONS_TableC_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_TableC_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_TableC_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_TableC_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_TableC_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableC_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableC).callParams[getCharacter(kCharacterTableC).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableC, &LogicManager::HAND_TableC_Birth);
	getCharacter(kCharacterTableC).callbacks[getCharacter(kCharacterTableC).currentCall] = 1;

	params->clear();

	fedEx(kCharacterTableC, kCharacterTableC, 12, 0);
}

void LogicManager::HAND_TableC_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		_engine->getSoundManager()->playSoundFile("LOOP8A.SND", kSoundTypeWalla | kSoundFlagLooped | kVolume8, kCharacterTableC, 0);
		CONS_TableC_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableC_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableC).callParams[getCharacter(kCharacterTableC).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableC, &LogicManager::HAND_TableC_StartPart2);
	getCharacter(kCharacterTableC).callbacks[getCharacter(kCharacterTableC).currentCall] = 2;

	params->clear();

	fedEx(kCharacterTableC, kCharacterTableC, 12, 0);
}

void LogicManager::HAND_TableC_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		_engine->getSoundManager()->playSoundFile("LOOP8A.SND", kSoundTypeWalla | kSoundFlagLooped | kVolume8, kCharacterTableC, 0);
		CONS_TableC_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableC_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableC).callParams[getCharacter(kCharacterTableC).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableC, &LogicManager::HAND_TableC_StartPart3);
	getCharacter(kCharacterTableC).callbacks[getCharacter(kCharacterTableC).currentCall] = 3;

	params->clear();

	fedEx(kCharacterTableC, kCharacterTableC, 12, 0);
}

void LogicManager::HAND_TableC_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		_engine->getSoundManager()->playSoundFile("LOOP8A.SND", kSoundTypeWalla | kSoundFlagLooped | kVolume8, kCharacterTableC, 0);
		CONS_TableC_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableC_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableC).callParams[getCharacter(kCharacterTableC).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableC, &LogicManager::HAND_TableC_StartPart4);
	getCharacter(kCharacterTableC).callbacks[getCharacter(kCharacterTableC).currentCall] = 4;

	params->clear();

	fedEx(kCharacterTableC, kCharacterTableC, 12, 0);
}

void LogicManager::HAND_TableC_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		_engine->getSoundManager()->playSoundFile("LOOP8A.SND", kSoundTypeWalla | kSoundFlagLooped | kVolume8, kCharacterTableC, 0);
		CONS_TableC_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableC_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableC).callParams[getCharacter(kCharacterTableC).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableC, &LogicManager::HAND_TableC_StartPart5);
	getCharacter(kCharacterTableC).callbacks[getCharacter(kCharacterTableC).currentCall] = 5;

	params->clear();

	fedEx(kCharacterTableC, kCharacterTableC, 12, 0);
}

void LogicManager::HAND_TableC_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (whoRunningDialog(kCharacterTableC))
			fadeDialog(kCharacterTableC);

		CONS_TableC_Idling(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_TableC_Idling(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterTableC).callParams[getCharacter(kCharacterTableC).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterTableC, &LogicManager::HAND_TableC_Idling);
	getCharacter(kCharacterTableC).callbacks[getCharacter(kCharacterTableC).currentCall] = 6;

	params->clear();

	fedEx(kCharacterTableC, kCharacterTableC, 12, 0);
}

void LogicManager::HAND_TableC_Idling(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalChapter] == 1) {
			if (_gameTime > 1165500 && !getCharacterCurrentParams(kCharacterTableC)[0]) {
				getCharacterCurrentParams(kCharacterTableC)[0] = 1;
				fadeDialog(kCharacterTableC);
			}
		} else if (_globals[kGlobalChapter] == 3) {
			if (_gameTime > 2052000 && !getCharacterCurrentParams(kCharacterTableC)[1]) {
				getCharacterCurrentParams(kCharacterTableC)[1] = 1;
				fadeDialog(kCharacterTableC);
			}
		} else if (_globals[kGlobalChapter] == 4 && _gameTime > 2488500 && !getCharacterCurrentParams(kCharacterTableC)[2]) {
			getCharacterCurrentParams(kCharacterTableC)[2] = 1;
			fadeDialog(kCharacterTableC);
		}

		break;
	case 12:
		getCharacter(kCharacterTableC).characterPosition.position = 4690;
		getCharacter(kCharacterTableC).characterPosition.location = 1;
		getCharacter(kCharacterTableC).characterPosition.car = kCarRestaurant;
		startCycOtis(kCharacterTableC, "009G");
		break;
	case 103798704:
		if (msg->param.stringParam) {
			startCycOtis(kCharacterTableC, msg->param.stringParam);
		} else {
			startCycOtis(kCharacterTableC, "009G");
		}

		break;
	case 136455232:
		startCycOtis(kCharacterTableC, "BLANK");
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsTableC[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_TableC_Birth,
	&LogicManager::HAND_TableC_StartPart2,
	&LogicManager::HAND_TableC_StartPart3,
	&LogicManager::HAND_TableC_StartPart4,
	&LogicManager::HAND_TableC_StartPart5,
	&LogicManager::HAND_TableC_Idling
};

} // End of namespace LastExpress
