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

void LogicManager::CONS_DemoMonsieur(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMonsieur,
			_functionsDemoMonsieur[getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall]]
		);

		break;
	case 1:
		CONS_DemoMonsieur_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoMonsieur_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoMonsieur_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoMonsieur_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoMonsieur_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::DemoMonsieurCall(CALL_PARAMS) {
	getCharacter(kCharacterMonsieur).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_DemoMonsieur_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_DemoMonsieur_Birth);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 1;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_DemoMonsieur_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterMonsieur).characterPosition.car = kCarRedSleeping;
		getCharacter(kCharacterMonsieur).characterPosition.position = 8200;
		getCharacter(kCharacterMonsieur).characterPosition.location = 1;
		setDoor(32, kCharacterCath, 2, 255, 255);
		startCycOtis(kCharacterMonsieur, "510");
		break;
	default:
		break;
	}
}

void LogicManager::CONS_DemoMonsieur_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_DemoMonsieur_StartPart2);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 2;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_DemoMonsieur_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoMonsieur_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_DemoMonsieur_StartPart3);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 3;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_DemoMonsieur_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoMonsieur_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_DemoMonsieur_StartPart4);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 4;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_DemoMonsieur_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoMonsieur_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMonsieur).callParams[getCharacter(kCharacterMonsieur).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMonsieur, &LogicManager::HAND_DemoMonsieur_StartPart5);
	getCharacter(kCharacterMonsieur).callbacks[getCharacter(kCharacterMonsieur).currentCall] = 5;

	params->clear();

	fedEx(kCharacterMonsieur, kCharacterMonsieur, 12, 0);
}

void LogicManager::HAND_DemoMonsieur_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoMonsieur[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoMonsieur_Birth,
	&LogicManager::HAND_DemoMonsieur_StartPart2,
	&LogicManager::HAND_DemoMonsieur_StartPart3,
	&LogicManager::HAND_DemoMonsieur_StartPart4,
	&LogicManager::HAND_DemoMonsieur_StartPart5
};

} // End of namespace LastExpress
