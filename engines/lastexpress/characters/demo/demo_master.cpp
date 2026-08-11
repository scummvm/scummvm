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

void LogicManager::CONS_DemoMaster(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMaster,
			_functionsDemoMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]
		);

		break;
	case 1:
		CONS_DemoMaster_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_DemoMaster_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_DemoMaster_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_DemoMaster_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_DemoMaster_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::DemoMasterCall(CALL_PARAMS) {
	getCharacter(kCharacterMaster).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_DemoMaster_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_DemoMaster_SaveGame);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 1;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_DemoMaster_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterMaster).currentCall--;
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMaster,
			_functionsDemoMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]
		);

		fedEx(kCharacterMaster, kCharacterMaster, 18, 0);
		break;
	case 12:
		save(
			kCharacterMaster,
			getCharacterCurrentParams(kCharacterMaster)[0],
			getCharacterCurrentParams(kCharacterMaster)[1]
		);

		getCharacter(kCharacterMaster).currentCall--;

		_engine->getMessageManager()->setMessageHandle(
			kCharacterMaster,
			_functionsDemoMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]
		);

		fedEx(kCharacterMaster, kCharacterMaster, 18, 0);
		break;
	default:
		break;
	}

}

void LogicManager::CONS_DemoMaster_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_DemoMaster_Birth);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 2;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_DemoMaster_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		bool playSndFx = false;
		bool doBumpCath = false;

		if (_globals[kGlobalTrainIsRunning]) {
			if (!getCharacterCurrentParams(kCharacterMaster)[3]) {
				getCharacterCurrentParams(kCharacterMaster)[3] = _realTime + getCharacterCurrentParams(kCharacterMaster)[0];
				if (!getCharacterCurrentParams(kCharacterMaster)[3])
					playSndFx = true;
			}

			if (!playSndFx && _realTime > getCharacterCurrentParams(kCharacterMaster)[3]) {
				getCharacterCurrentParams(kCharacterMaster)[3] = 0x7FFFFFFF;
				playSndFx = true;
			}

			if (playSndFx) {
				if (!rnd(2)) {
					playDialog(kCharacterCath, "ZFX1007B", rnd(15) + 2, 0);
				} else {
					playDialog(kCharacterCath, "ZFX1005", rnd(15) + 2, 0);
				}

				getCharacterCurrentParams(kCharacterMaster)[3] = 0;
				getCharacterCurrentParams(kCharacterMaster)[0] = 900 * (rnd(5) + 5);
			}
		}

		if (_gameTime > 2268000 && !getCharacterCurrentParams(kCharacterMaster)[4]) {
			getCharacterCurrentParams(kCharacterMaster)[4] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 2;
			DemoMasterCall(&LogicManager::CONS_DemoMaster_SaveGame, 2, kEventVergesAnnaDead, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterMaster)[1]) {
			if (!getCharacterCurrentParams(kCharacterMaster)[5]) {
				getCharacterCurrentParams(kCharacterMaster)[5] = _gameTime + 900;
				if (_gameTime == -900)
					doBumpCath = true;
			}

			if (!doBumpCath && _gameTime > getCharacterCurrentParams(kCharacterMaster)[5]) {
				getCharacterCurrentParams(kCharacterMaster)[5] = 0x7FFFFFFF;
				doBumpCath = true;
			}

			if (doBumpCath) {
				bumpCath(kCarRestaurant, 58, 255);
			}
		}

		getCharacterCurrentParams(kCharacterMaster)[5] = 0;

		if (getCharacterCurrentParams(kCharacterMaster)[1]) {
			if (!cathRunningDialog("ZFX1001"))
				playDialog(kCharacterCath, "ZFX1001", -1, 0);
		}

		break;
	}
	case 12:
		_engine->getSoundManager()->startAmbient();
		getCharacterCurrentParams(kCharacterMaster)[0] = 900 * (rnd(5) + 5);
		setDoor(34, kCharacterCath, 2, 255, 255);
		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;
		DemoMasterCall(&LogicManager::CONS_DemoMaster_SaveGame, 1, 0, 0, 0);
		break;
	case 17:
		getCharacter(kCharacterMaster).characterPosition.car = getCharacter(kCharacterCath).characterPosition.car;
		if (checkCathDir(kCarRestaurant, 81)) {
			getCharacterCurrentParams(kCharacterMaster)[1] = 1;
			if (!dialogRunning("ZFX1001"))
				playDialog(kCharacterCath, "ZFX1001", -1, 0);
		} else {
			getCharacterCurrentParams(kCharacterMaster)[1] = 0;
			if (cathRunningDialog("ZFX1001"))
				endDialog("ZFX1001");
		}

		if (getCharacter(kCharacterCath).characterPosition.car != getCharacterCurrentParams(kCharacterMaster)[2]) {
			if (getCharacter(kCharacterCath).characterPosition.car == kCarRestaurant) {
				startCycOtis(kCharacterMaster, "RCWND");
			} else if (getCharacter(kCharacterCath).characterPosition.car == kCarRedSleeping) {
				startCycOtis(kCharacterMaster, "S1WND");
			} else {
				endGraphics(kCharacterMaster);
			}

			getCharacterCurrentParams(kCharacterMaster)[2] = getCharacter(kCharacterCath).characterPosition.car;
		}

		break;
	case 18:
	{
		bool doBumpCath = false;

		if (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 2) {
			playNIS(kEventVergesAnnaDead);
			endGame(1, 2259000, 58, true);

			if (getCharacterCurrentParams(kCharacterMaster)[1]) {
				if (!getCharacterCurrentParams(kCharacterMaster)[5]) {
					getCharacterCurrentParams(kCharacterMaster)[5] = _gameTime + 900;
					if (_gameTime == -900)
						doBumpCath = true;
				}

				if (!doBumpCath && _gameTime > getCharacterCurrentParams(kCharacterMaster)[5]) {
					getCharacterCurrentParams(kCharacterMaster)[5] = 0x7FFFFFFF;
					doBumpCath = true;
				}

				if (doBumpCath) {
					bumpCath(kCarRestaurant, 58, 255);
				}
			}

			getCharacterCurrentParams(kCharacterMaster)[5] = 0;

			if (getCharacterCurrentParams(kCharacterMaster)[1]) {
				if (!cathRunningDialog("ZFX1001"))
					playDialog(kCharacterCath, "ZFX1001", -1, 0);
			}
		}

		break;
	}
	default:
		break;
	}
}

void LogicManager::CONS_DemoMaster_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_DemoMaster_StartPart2);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 3;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_DemoMaster_StartPart2(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoMaster_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_DemoMaster_StartPart3);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 4;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_DemoMaster_StartPart3(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoMaster_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_DemoMaster_StartPart4);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 5;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_DemoMaster_StartPart4(HAND_PARAMS) {
	// No-op
}

void LogicManager::CONS_DemoMaster_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_DemoMaster_StartPart5);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 6;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_DemoMaster_StartPart5(HAND_PARAMS) {
	// No-op
}

void (LogicManager::*LogicManager::_functionsDemoMaster[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_DemoMaster_SaveGame,
	&LogicManager::HAND_DemoMaster_Birth,
	&LogicManager::HAND_DemoMaster_StartPart2,
	&LogicManager::HAND_DemoMaster_StartPart3,
	&LogicManager::HAND_DemoMaster_StartPart4,
	&LogicManager::HAND_DemoMaster_StartPart5
};

} // End of namespace LastExpress
