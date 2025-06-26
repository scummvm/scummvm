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

void LogicManager::CONS_Master(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMaster,
			_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]
		);

		break;
	case 1:
		CONS_Master_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Master_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Master_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Master_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Master_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::MasterCall(CALL_PARAMS) {
	getCharacter(kCharacterMaster).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Master_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_SaveGame);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 1;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterMaster).currentCall--;
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMaster,
			_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]
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
			_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]);

		fedEx(kCharacterMaster, kCharacterMaster, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_Arrive(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_Arrive);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 2;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);
	params->parameters[3] = param2.intParam;

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_Arrive(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);

			if (getCharacterParams(kCharacterMaster, 8)[1]) {
				send(kCharacterMaster, kCharacterClerk, 191350523, 0);

				if (getCharacter(kCharacterCath).characterPosition.location == 2) {
					if (cathOutHisWindow()) {
						bumpCath(kCarGreenSleeping, 49, 255);
					} else if (cathOutRebeccaWindow()) {
						bumpCath(kCarRedSleeping, 49, 255);
					} else if (getCharacter(kCharacterCath).characterPosition.car < kCarRedSleeping || getCharacter(kCharacterCath).characterPosition.car > kCarCoalTender) {
						if (getCharacter(kCharacterCath).characterPosition.car && getCharacter(kCharacterCath).characterPosition.car <= kCarGreenSleeping) {
							if (checkCathDir(kCarGreenSleeping, 98)) {
								playDialog(kCharacterCath, "LIB015", -1, 0);
								bumpCath(kCarGreenSleeping, 71, 255);
							} else {
								bumpCath(kCarGreenSleeping, 82, 255);
							}
						}
					} else {
						bumpCath(kCarRestaurant, 82, 255);
					}
				}

				getCharacterParams(kCharacterMaster, 8)[1] = 0;
			} else if (getCharacterParams(kCharacterMaster, 8)[2]) {
				getCharacterParams(kCharacterMaster, 8)[2] = 0;
			}
		}

		setDoor(26, kCharacterCath, 1, 0, 0);
		setDoor(27, kCharacterCath, 1, 0, 0);

		playDialog(kCharacterMaster, "ARRIVE", 8, 0);
		_engine->getSoundManager()->endAmbient();

		_globals[kGlobalTrainIsRunning] = 0;

		getCharacterParams(kCharacterMaster, 8)[1] = 1;
		getCharacterParams(kCharacterMaster, 8)[3] = getCharacterCurrentParams(kCharacterMaster)[3];

		getCharacter(kCharacterMaster).currentCall--;
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMaster,
			_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]
		);

		fedEx(kCharacterMaster, kCharacterMaster, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_Depart(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_Depart);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 3;

	params->clear();

	strncpy((char *)&params->parameters[0], param1.stringParam, 12);

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_Depart(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);

			if (getCharacterParams(kCharacterMaster, 8)[1]) {
				send(kCharacterMaster, kCharacterClerk, 191350523, 0);

				if (getCharacter(kCharacterCath).characterPosition.location == 2) {
					if (cathOutHisWindow()) {
						bumpCath(kCarGreenSleeping, 49, 255);
					} else if (cathOutRebeccaWindow()) {
						bumpCath(kCarRedSleeping, 49, 255);
					} else if (getCharacter(kCharacterCath).characterPosition.car < kCarRedSleeping || getCharacter(kCharacterCath).characterPosition.car > kCarCoalTender) {
						if (getCharacter(kCharacterCath).characterPosition.car && getCharacter(kCharacterCath).characterPosition.car <= kCarGreenSleeping) {
							if (checkCathDir(kCarGreenSleeping, 98)) {
								playDialog(kCharacterCath, "LIB015", -1, 0);
								bumpCath(kCarGreenSleeping, 71, 255);
							} else {
								bumpCath(kCarGreenSleeping, 82, 255);
							}
						}
					} else {
						bumpCath(kCarRestaurant, 82, 255);
					}
				}
				getCharacterParams(kCharacterMaster, 8)[1] = 0;
			} else if (getCharacterParams(kCharacterMaster, 8)[2]) {
				getCharacterParams(kCharacterMaster, 8)[2] = 0;
			}
		}

		playDialog(kCharacterMaster, "DEPART", 8, 0);
		_engine->getSoundManager()->endAmbient();

		send(kCharacterMaster, kCharacterClerk, 203419131, 0);

		setDoor(26, kCharacterCath, 1, 0, 9);
		setDoor(27, kCharacterCath, 1, 0, 9);

		_globals[kGlobalTrainIsRunning] = 1;

		getCharacterParams(kCharacterMaster, 8)[2] = 1;

		getCharacter(kCharacterMaster).currentCall--;
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMaster,
			_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]
		);

		fedEx(kCharacterMaster, kCharacterMaster, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_Birth);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 4;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		autoMessage(kCharacterMaster, 171843264, 0);
		CONS_Master_StartPart1(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_WalkTest(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_WalkTest);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 5;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_WalkTest(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		forceJump(kCharacterAbbot,    &LogicManager::CONS_Abbot_DebugWalks);
		forceJump(kCharacterAlexei,   &LogicManager::CONS_Alexei_DebugWalks);
		forceJump(kCharacterAlouan,   &LogicManager::CONS_Alouan_DebugWalks);
		forceJump(kCharacterAnna,     &LogicManager::CONS_Anna_DebugWalks);
		forceJump(kCharacterAugust,   &LogicManager::CONS_August_DebugWalks);
		forceJump(kCharacterCond1,    &LogicManager::CONS_Cond1_DebugWalks);
		forceJump(kCharacterCond2,    &LogicManager::CONS_Cond2_DebugWalks);
		forceJump(kCharacterFrancois, &LogicManager::CONS_Francois_DebugWalks);
		forceJump(kCharacterHadija,   &LogicManager::CONS_Hadija_DebugWalks);
		forceJump(kCharacterIvo,      &LogicManager::CONS_Ivo_DebugWalks);
		forceJump(kCharacterKahina,   &LogicManager::CONS_Kahina_DebugWalks);
		forceJump(kCharacterKronos,   &LogicManager::CONS_Kronos_DebugWalks);
		forceJump(kCharacterMadame,   &LogicManager::CONS_Madame_DebugWalks);
		forceJump(kCharacterMahmud,   &LogicManager::CONS_Mahmud_DebugWalks);
		forceJump(kCharacterMax,      &LogicManager::CONS_Max_DebugWalks);
		forceJump(kCharacterMilos,    &LogicManager::CONS_Milos_DebugWalks);
		forceJump(kCharacterMonsieur, &LogicManager::CONS_Monsieur_DebugWalks);
		forceJump(kCharacterPolice,   &LogicManager::CONS_Police_DebugWalks);
		forceJump(kCharacterRebecca,  &LogicManager::CONS_Rebecca_DebugWalks);
		forceJump(kCharacterSalko,    &LogicManager::CONS_Salko_DebugWalks);
		forceJump(kCharacterSophie,   &LogicManager::CONS_Sophie_DebugWalks);
		forceJump(kCharacterTatiana,  &LogicManager::CONS_Tatiana_DebugWalks);
		forceJump(kCharacterTrainM,   &LogicManager::CONS_TrainM_DebugWalks);
		forceJump(kCharacterVassili,  &LogicManager::CONS_Vassili_DebugWalks);
		forceJump(kCharacterVesna,    &LogicManager::CONS_Vesna_DebugWalks);
		forceJump(kCharacterYasmin,   &LogicManager::CONS_Yasmin_DebugWalks);

		getCharacter(kCharacterMaster).currentCall--;
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMaster,
			_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]
		);

		fedEx(kCharacterMaster, kCharacterMaster, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_FirstDream(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_FirstDream);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 6;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_FirstDream(HAND_PARAMS) {
	switch (msg->action) {
	case 2:
		playDialog(kCharacterMaster, "MUS009", 16, 0);
		break;
	case 8:
		if (!cathRunningDialog("LIB012"))
			playDialog(kCharacterCath, "LIB012", -1, 0);

		break;
	case 12:
		forceJump(kCharacterHeadWait, &LogicManager::CONS_HeadWait_LockUp);
		forceJump(kCharacterWaiter1,  &LogicManager::CONS_Waiter1_LockUp);
		forceJump(kCharacterWaiter2,  &LogicManager::CONS_Waiter2_LockUp);
		forceJump(kCharacterCook,     &LogicManager::CONS_Cook_LockUp);
		forceJump(kCharacterCond1,    &LogicManager::CONS_Cond1_Sitting);
		forceJump(kCharacterCond2,    &LogicManager::CONS_Cond2_Sitting);
		forceJump(kCharacterTrainM,   &LogicManager::CONS_TrainM_InOffice);

		send(kCharacterMaster, kCharacterCond1,  201431954, 0);
		send(kCharacterMaster, kCharacterCond2,  201431954, 0);
		send(kCharacterMaster, kCharacterTrainM, 201431954, 0);

		forceJump(kCharacterKronos,   &LogicManager::CONS_Kronos_CathDone);
		forceJump(kCharacterKahina,   &LogicManager::CONS_Kahina_CathDone);
		forceJump(kCharacterAnna,     &LogicManager::CONS_Anna_Asleep);
		forceJump(kCharacterAugust,   &LogicManager::CONS_August_Asleep);
		forceJump(kCharacterTatiana,  &LogicManager::CONS_Tatiana_Asleep);
		forceJump(kCharacterVassili,  &LogicManager::CONS_Vassili_InBed2);
		forceJump(kCharacterAlexei,   &LogicManager::CONS_Alexei_Asleep);
		forceJump(kCharacterMilos,    &LogicManager::CONS_Milos_Asleep);
		forceJump(kCharacterVesna,    &LogicManager::CONS_Vesna_Asleep);
		forceJump(kCharacterIvo,      &LogicManager::CONS_Ivo_Asleep);
		forceJump(kCharacterSalko,    &LogicManager::CONS_Salko_Asleep);
		forceJump(kCharacterFrancois, &LogicManager::CONS_Francois_Asleep);
		forceJump(kCharacterMadame,   &LogicManager::CONS_Madame_Asleep);
		forceJump(kCharacterMonsieur, &LogicManager::CONS_Monsieur_Asleep);
		forceJump(kCharacterRebecca,  &LogicManager::CONS_Rebecca_Asleep);
		forceJump(kCharacterSophie,   &LogicManager::CONS_Sophie_Asleep);
		forceJump(kCharacterMahmud,   &LogicManager::CONS_Mahmud_Asleep);
		forceJump(kCharacterYasmin,   &LogicManager::CONS_Yasmin_Asleep);
		forceJump(kCharacterHadija,   &LogicManager::CONS_Hadija_Asleep);
		forceJump(kCharacterHadija,   &LogicManager::CONS_Alouan_Asleep); // Not a mistake!

		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);

			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}

		_engine->getSoundManager()->endAmbient();

		if (dialogRunning("CON1505"))
			fadeDialog("CON1505");

		if (dialogRunning("AUG1057"))
			fadeDialog("AUG1057");

		if (dialogRunning("ZFX1005")) {
			fadeDialog("ZFX1005");
		} else if (dialogRunning("ZFX1006")) {
			fadeDialog("ZFX1006");
		} else if (dialogRunning("ZFX1007")) {
			fadeDialog("ZFX1007");
		} else if (dialogRunning("ZFX1007A")) {
			fadeDialog("ZFX1007A");
		} else if (dialogRunning("ZFX1007B")) {
			fadeDialog("ZFX1007B");
		}

		playDialog(kCharacterCath, "MUS008", 16, 0);

		_activeItem = 0;
		fadeToBlack();

		while (dialogRunning("MUS008")) {
			_engine->getSoundManager()->soundThread();
			_engine->waitForTimer(4);
		}

		_globals[kGlobalCathInSpecialState] = 1;
		bumpCath(kCarLocomotive, 75, 255);
		restoreIcons();

		_gameTime = 1492200;
		_globals[kGlobalPhaseOfTheNight] = 4;
		_timeSpeed = 0;

		setDoor(63, kCharacterMaster, 0, 0, 9);
		send(kCharacterMaster, kCharacterClerk, 191350523, 0);

		_globals[kGlobalTrainIsRunning] = 0;
		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;

		MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
		break;
	case 9:
		if (getCharacterCurrentParams(kCharacterMaster)[0]) {
			endGraphics(kCharacterMaster);

			fadeDialog(kCharacterMaster);
			playDialog(kCharacterCath, "LIB014", -1, 0);

			_engine->getSoundManager()->startAmbient();
			getCharacterParams(kCharacterMaster, 8)[3] = 7;
			_engine->getSoundManager()->startSteam(16);

			playNIS(kEventCathDream);

			_timeSpeed = 3;
			_globals[kGlobalPhaseOfTheNight] = 1;

			setDoor(63, kCharacterCath, 0, 10, 9);

			_globals[kGlobalCathInSpecialState] = 0;

			bumpCathTylerComp();

			getCharacter(kCharacterMaster).currentCall--;
			_engine->getMessageManager()->setMessageHandle(
				kCharacterMaster,
				_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]
			);

			fedEx(kCharacterMaster, kCharacterMaster, 18, 0);
		} else {
			playDialog(kCharacterCath, "LIB014", -1, 0);
			playDialog(kCharacterCath, "LIB015", 16, 15);

			if (!whoRunningDialog(kCharacterMaster))
				playDialog(kCharacterMaster, "MUS009", 16, 0);

			bumpCath(kCarLocomotive, 38, 255);

			setDoor(63, kCharacterMaster, 0, 10, 9);
			getCharacterCurrentParams(kCharacterMaster)[0] = 1;
		}

		break;
	case 225358684:
		getCharacterCurrentParams(kCharacterMaster)[1]++;

		if (getCharacterCurrentParams(kCharacterMaster)[1] >= 3) {
			if (!cathRunningDialog("LIB031"))
				playDialog(kCharacterCath, "LIB031", -1, 0);

			if (getCharacterCurrentParams(kCharacterMaster)[1] == 3) {
				getCharacter(kCharacterMaster).characterPosition.car = kCarLocomotive;
				startCycOtis(kCharacterMaster, "JUGL");
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_StartPart1(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_StartPart1);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 7;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_StartPart1(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		_globals[kGlobalChapter] = 1;

		_engine->getSoundManager()->startAmbient();

		_gameTime = 1061100;
		_timeSpeed = 0;
		_globals[kGlobalTrainIsRunning] = 1;
		_globals[kGlobalCathIcon] = 32;
		_globals[kGlobalPhaseOfTheNight] = 1;

		giveCathItem(kItemTelegram);
		giveCathItem(kItemArticle);

		dropItem(kItemScarf, 1);
		dropItem(kItemParchemin, 1);
		dropItem(kItemGreenJacket, 1);
		dropItem(kItemCorpse, 1);
		dropItem(kItemPassengerList, 1);
		dropItem(kItem5, 1);
		dropItem(kItem7, 1);
		dropItem(kItem3, 1);
		dropItem(kItemMatch, 1);
		dropItem(kItem22, 1);
		dropItem(kItemPaper, 1);

		_globals[kGlobalCanPlayEggSuitcaseMusic] = 1;

		setDoor(1, kCharacterCath, 0, 10, 9);
		setDoor(9, kCharacterCath, 0, 255, 255);

		getCharacterCurrentParams(kCharacterMaster)[0] = 1;

		do {
			setModel(getCharacterCurrentParams(kCharacterMaster)[0], 2);
			getCharacterCurrentParams(kCharacterMaster)[0]++;
		} while (getCharacterCurrentParams(kCharacterMaster)[0] <= 8);

		getCharacterCurrentParams(kCharacterMaster)[0] = 32;

		do {
			setModel(getCharacterCurrentParams(kCharacterMaster)[0], 2);
			getCharacterCurrentParams(kCharacterMaster)[0]++;
		} while (getCharacterCurrentParams(kCharacterMaster)[0] <= 39);

		setModel(25, 1);
		setModel(23, 1);
		setModel(98, 1);
		setModel(55, 1);

		setDoor(25, kCharacterCath, 0, 0, 1);
		setDoor(23, kCharacterCath, 0, 0, 1);
		setDoor(24, kCharacterCath, 0, 0, 1);
		setDoor(28, kCharacterCath, 0, 0, 1);
		setDoor(56, kCharacterCath, 0, 0, 1);
		setDoor(54, kCharacterCath, 0, 0, 1);
		setDoor(55, kCharacterCath, 0, 0, 1);
		setDoor(59, kCharacterCath, 0, 0, 1);
		setDoor(66, kCharacterCath, 0, 0, 1);
		setDoor(64, kCharacterCath, 0, 0, 1);
		setDoor(65, kCharacterCath, 0, 0, 1);
		setDoor(69, kCharacterCath, 0, 0, 1);
		setDoor(98, kCharacterCath, 0, 0, 1);
		setDoor(26, kCharacterCath, 1, 0, 9);
		setDoor(27, kCharacterCath, 1, 0, 9);
		setDoor(101, kCharacterCath, 1, 10, 9);

		CONS_Master_MovingNight(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_MovingNight(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_MovingNight);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 8;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_MovingNight(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalTrainIsRunning] && _gameTime < 1458000) {
			bool skip = false;

			if (!getCharacterCurrentParams(kCharacterMaster)[5]) {
				getCharacterCurrentParams(kCharacterMaster)[5] = _realTime + getCharacterCurrentParams(kCharacterMaster)[1];
				if (getCharacterCurrentParams(kCharacterMaster)[5] == 0) {
					skip = true;
				}
			}
			if (skip || getCharacterCurrentParams(kCharacterMaster)[5] < _realTime) {
				if (!skip) {
					getCharacterCurrentParams(kCharacterMaster)[5] = 0x7FFFFFFF;
				}

				switch (rnd(5)) {
				case 0:
					playDialog(0, "ZFX1005", rnd(15) + 2, 0);
					break;
				case 1:
					playDialog(0, "ZFX1006", rnd(15) + 2, 0);
					break;
				case 2:
					playDialog(0, "ZFX1007", rnd(15) + 2, 0);
					break;
				case 3:
					playDialog(0, "ZFX1007A", rnd(15) + 2, 0);
					break;
				case 4:
					playDialog(0, "ZFX1007B", rnd(15) + 2, 0);
					break;
				default:
					break;
				}

				getCharacterCurrentParams(kCharacterMaster)[1] = 225 * (4 * rnd(5) + 20);
				getCharacterCurrentParams(kCharacterMaster)[5] = 0;
			}
		}

		if (_gameTime > 1039500 && !getCharacterCurrentParams(kCharacterMaster)[6]) {
			getCharacterCurrentParams(kCharacterMaster)[6] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;
			MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
			break;
		}

		if (_gameTime > 1147500 && !getCharacterCurrentParams(kCharacterMaster)[7]) {
			getCharacterCurrentParams(kCharacterMaster)[7] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 2;
			MasterCall(&LogicManager::CONS_Master_Arrive, "Epernay", 0, 0, 0);
			break;
		}

		if (_gameTime > 1150200 && !getCharacterCurrentParams(kCharacterMaster)[8]) {
			getCharacterCurrentParams(kCharacterMaster)[8] = 1;
			getCharacterCurrentParams(kCharacterMaster)[3] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 3;
			MasterCall(&LogicManager::CONS_Master_Depart, "Epernay", 0, 0, 0);
			break;
		}

		if (getCharacterCurrentParams(kCharacterMaster)[4] && !getCharacterParams(kCharacterMaster, 8)[1]) {
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 4;
			MasterCall(&LogicManager::CONS_Master_Depart, "Unschedu", 0, 0, 0);
			break;
		}

		if (_gameTime > 1170000 && !getCharacterCurrentParams(kCharacterMaster)[9]) {
			getCharacterCurrentParams(kCharacterMaster)[9] = 1;
			_globals[kGlobalPhaseOfTheNight] = 2;
		}

		if (!getCharacterCurrentParams(kCharacterMaster)[0]) {
			if (_gameTime > 1170000 && !getCharacterCurrentParams(kCharacterMaster)[10]) {
				getCharacterCurrentParams(kCharacterMaster)[10] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 5;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Chalons", 1, 0, 0);
				break;
			}

			if (_gameTime > 1173600 && !getCharacterCurrentParams(kCharacterMaster)[11]) {
				getCharacterCurrentParams(kCharacterMaster)[11] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 6;
				MasterCall(&LogicManager::CONS_Master_Depart, "Chalons", 0, 0, 0);
				break;
			}

			if (_gameTime > 1228500 && !getCharacterCurrentParams(kCharacterMaster)[12]) {
				getCharacterCurrentParams(kCharacterMaster)[12] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 7;
				MasterCall(&LogicManager::CONS_Master_Arrive, "BarLeDuc", 2, 0, 0);
				break;
			}

			if (_gameTime > 1231200 && !getCharacterCurrentParams(kCharacterMaster)[13]) {
				getCharacterCurrentParams(kCharacterMaster)[13] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 8;
				MasterCall(&LogicManager::CONS_Master_Depart, "BarLeDuc", 0, 0, 0);
				break;
			}

			if (_gameTime > 1260000 && !getCharacterCurrentParams(kCharacterMaster)[14]) {
				getCharacterCurrentParams(kCharacterMaster)[14] = 1;
				_timeSpeed = 1;
			}
			if (_gameTime > 1303200 && !getCharacterCurrentParams(kCharacterMaster)[15]) {
				getCharacterCurrentParams(kCharacterMaster)[15] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 9;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Nancy", 3, 0, 0);
				break;
			}

			if (_gameTime > 1307700 && !getCharacterCurrentParams(kCharacterMaster)[16]) {
				getCharacterCurrentParams(kCharacterMaster)[16] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 10;
				MasterCall(&LogicManager::CONS_Master_Depart, "Nancy", 0, 0, 0);
				break;
			}

			if (_gameTime > 1335600 && !getCharacterCurrentParams(kCharacterMaster)[17]) {
				getCharacterCurrentParams(kCharacterMaster)[17] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 11;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Luneville", 4, 0, 0);
				break;
			}

			if (_gameTime > 1338300 && !getCharacterCurrentParams(kCharacterMaster)[18]) {
				getCharacterCurrentParams(kCharacterMaster)[18] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 12;
				MasterCall(&LogicManager::CONS_Master_Depart, "Luneville", 0, 0, 0);
				break;
			}

			if (_gameTime > 1359900 && !getCharacterCurrentParams(kCharacterMaster)[19]) {
				getCharacterCurrentParams(kCharacterMaster)[19] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 13;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Avricourt", 5, 0, 0);
				break;
			}

			if (_gameTime > 1363500 && !getCharacterCurrentParams(kCharacterMaster)[20]) {
				getCharacterCurrentParams(kCharacterMaster)[20] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 14;
				MasterCall(&LogicManager::CONS_Master_Depart, "Avricourt", 0, 0, 0);
				break;
			}

			if (_gameTime > 1367100 && !getCharacterCurrentParams(kCharacterMaster)[21]) {
				getCharacterCurrentParams(kCharacterMaster)[21] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 15;
				MasterCall(&LogicManager::CONS_Master_Arrive, "DeutschA", 6, 0, 0);
				break;
			}

			if (_gameTime > 1370700 && !getCharacterCurrentParams(kCharacterMaster)[22]) {
				getCharacterCurrentParams(kCharacterMaster)[22] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 16;
				MasterCall(&LogicManager::CONS_Master_Depart, "DeutschA", 0, 0, 0);
				break;
			}

			if (_gameTime > 1490400 && !getCharacterCurrentParams(kCharacterMaster)[23]) {
				getCharacterCurrentParams(kCharacterMaster)[23] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 17;
				MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
				break;
			}
		}

		if (_gameTime > 1493100 && !getCharacterCurrentParams(kCharacterMaster)[24]) {
			getCharacterCurrentParams(kCharacterMaster)[24] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 19;
			MasterCall(&LogicManager::CONS_Master_Depart, "Strasbou", 0, 0, 0);
			break;
		}

		if (_gameTime > 1539000 && !getCharacterCurrentParams(kCharacterMaster)[25]) {
			getCharacterCurrentParams(kCharacterMaster)[25] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 20;
			MasterCall(&LogicManager::CONS_Master_Arrive, "BadenOos", 8, 0, 0);
			break;
		}

		if (_gameTime > 1541700 && !getCharacterCurrentParams(kCharacterMaster)[26]) {
			getCharacterCurrentParams(kCharacterMaster)[26] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 21;
			MasterCall(&LogicManager::CONS_Master_Depart, "BadenOos", 0, 0, 0);
			break;
		}

		if (_gameTime > 1674000 && !getCharacterCurrentParams(kCharacterMaster)[27]) {
			getCharacterCurrentParams(kCharacterMaster)[27] = 1;
			CONS_Master_SecondSleep(0, 0, 0, 0);
		}

		break;
	case 2:
		if (getCharacterParams(kCharacterMaster, 8)[1]) {
			send(kCharacterMaster, kCharacterClerk, 191350523, 0);
			if (getCharacter(kCharacterCath).characterPosition.location == 2) {
				if (cathOutHisWindow()) {
					bumpCath(kCarGreenSleeping, 49, 255);
					_engine->getSoundManager()->startAmbient();
					_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
					getCharacterParams(kCharacterMaster, 8)[1] = 0;
					break;
				}

				if (cathOutRebeccaWindow()) {
					bumpCath(kCarRedSleeping, 49, 255);
				} else {
					if (getCharacter(kCharacterCath).characterPosition.car < kCarRedSleeping || getCharacter(kCharacterCath).characterPosition.car > kCarCoalTender) {
						if (!getCharacter(kCharacterCath).characterPosition.car || getCharacter(kCharacterCath).characterPosition.car > kCarGreenSleeping) {
							_engine->getSoundManager()->startAmbient();
							_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
							getCharacterParams(kCharacterMaster, 8)[1] = 0;
							break;
						}

						if (checkCathDir(kCarGreenSleeping, 98)) {
							playDialog(0, "LIB015", -1, 0);
							bumpCath(kCarGreenSleeping, 71, 255);
						} else {
							bumpCath(kCarGreenSleeping, 82, 255);
						}

						_engine->getSoundManager()->startAmbient();
						_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
						getCharacterParams(kCharacterMaster, 8)[1] = 0;
						break;
					}

					bumpCath(kCarRestaurant, 82, 255);
				}
			}

			_engine->getSoundManager()->startAmbient();
			_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			break;
		}

		if (getCharacterParams(kCharacterMaster, 8)[2]) {
			_engine->getSoundManager()->startAmbient();
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
			if (getCharacterCurrentParams(kCharacterMaster)[3]) {
				if (_globals[kGlobalFrancoisHasSeenCorpseThrown])
					send(kCharacterMaster, kCharacterTrainM, 168187490, 0);
				else
					send(kCharacterMaster, kCharacterCond1, 224122407, 0);

				getCharacterCurrentParams(kCharacterMaster)[3] = 0;
			}
		}

		break;
	case 190346110:
		_globals[kGlobalPhaseOfTheNight] = 3;
		if (_gameTime >= 1494000) {
			CONS_Master_SecondSleep(0, 0, 0, 0);
		} else {
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 23;
			MasterCall(&LogicManager::CONS_Master_FirstDream, 0, 0, 0, 0);
		}

		break;
	case 169629818:
		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 22;
		MasterCall(&LogicManager::CONS_Master_Arrive, "Unschedu", 16, 0, 0);
		break;
	case 12:
		getCharacterCurrentParams(kCharacterMaster)[1] = 225 * (4 * rnd(5) + 20);
		break;
	case 17:
		if (!getCharacterCurrentParams(kCharacterMaster)[2] && checkCathDir(kCarGreenSleeping, 1)) {
			_gameTime = 1062000;
			_timeSpeed = 3;
			getCharacterCurrentParams(kCharacterMaster)[2] = 1;
		}
		break;
	case 18:
		switch (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8]) {
		case 1:
			if (_gameTime > 1147500 && !getCharacterCurrentParams(kCharacterMaster)[7]) {
				getCharacterCurrentParams(kCharacterMaster)[7] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 2;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Epernay", 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (_gameTime > 1150200 && !getCharacterCurrentParams(kCharacterMaster)[8]) {
				getCharacterCurrentParams(kCharacterMaster)[8] = 1;
				getCharacterCurrentParams(kCharacterMaster)[3] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 3;
				MasterCall(&LogicManager::CONS_Master_Depart, "Epernay", 0, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (getCharacterCurrentParams(kCharacterMaster)[4] && !getCharacterParams(kCharacterMaster, 8)[1]) {
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 4;
				MasterCall(&LogicManager::CONS_Master_Depart, "Unschedu", 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 4) {
				getCharacterCurrentParams(kCharacterMaster)[4] = 0;
			}

			if (_gameTime > 1170000 && !getCharacterCurrentParams(kCharacterMaster)[9]) {
				getCharacterCurrentParams(kCharacterMaster)[9] = 1;
				_globals[kGlobalPhaseOfTheNight] = 2;
			}

			if (!getCharacterCurrentParams(kCharacterMaster)[0]) {
				if (_gameTime > 1170000 && !getCharacterCurrentParams(kCharacterMaster)[10]) {
					getCharacterCurrentParams(kCharacterMaster)[10] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 5;
					MasterCall(&LogicManager::CONS_Master_Arrive, "Chalons", 1, 0, 0);
					break;
				}

				if (_gameTime > 1173600 && !getCharacterCurrentParams(kCharacterMaster)[11]) {
					getCharacterCurrentParams(kCharacterMaster)[11] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 6;
					MasterCall(&LogicManager::CONS_Master_Depart, "Chalons", 0, 0, 0);
					break;
				}

				if (_gameTime > 1228500 && !getCharacterCurrentParams(kCharacterMaster)[12]) {
					getCharacterCurrentParams(kCharacterMaster)[12] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 7;
					MasterCall(&LogicManager::CONS_Master_Arrive, "BarLeDuc", 2, 0, 0);
					break;
				}

				if (_gameTime > 1231200 && !getCharacterCurrentParams(kCharacterMaster)[13]) {
					getCharacterCurrentParams(kCharacterMaster)[13] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 8;
					MasterCall(&LogicManager::CONS_Master_Depart, "BarLeDuc", 0, 0, 0);
					break;
				}

				if (_gameTime > 1260000 && !getCharacterCurrentParams(kCharacterMaster)[14]) {
					getCharacterCurrentParams(kCharacterMaster)[14] = 1;
					_timeSpeed = 1;
				}
				if (_gameTime > 1303200 && !getCharacterCurrentParams(kCharacterMaster)[15]) {
					getCharacterCurrentParams(kCharacterMaster)[15] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 9;
					MasterCall(&LogicManager::CONS_Master_Arrive, "Nancy", 3, 0, 0);
					break;
				}

				if (_gameTime > 1307700 && !getCharacterCurrentParams(kCharacterMaster)[16]) {
					getCharacterCurrentParams(kCharacterMaster)[16] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 10;
					MasterCall(&LogicManager::CONS_Master_Depart, "Nancy", 0, 0, 0);
					break;
				}

				if (_gameTime > 1335600 && !getCharacterCurrentParams(kCharacterMaster)[17]) {
					getCharacterCurrentParams(kCharacterMaster)[17] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 11;
					MasterCall(&LogicManager::CONS_Master_Arrive, "Luneville", 4, 0, 0);
					break;
				}

				if (_gameTime > 1338300 && !getCharacterCurrentParams(kCharacterMaster)[18]) {
					getCharacterCurrentParams(kCharacterMaster)[18] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 12;
					MasterCall(&LogicManager::CONS_Master_Depart, "Luneville", 0, 0, 0);
					break;
				}

				if (_gameTime > 1359900 && !getCharacterCurrentParams(kCharacterMaster)[19]) {
					getCharacterCurrentParams(kCharacterMaster)[19] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 13;
					MasterCall(&LogicManager::CONS_Master_Arrive, "Avricourt", 5, 0, 0);
					break;
				}

				if (_gameTime > 1363500 && !getCharacterCurrentParams(kCharacterMaster)[20]) {
					getCharacterCurrentParams(kCharacterMaster)[20] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 14;
					MasterCall(&LogicManager::CONS_Master_Depart, "Avricourt", 0, 0, 0);
					break;
				}

				if (_gameTime > 1367100 && !getCharacterCurrentParams(kCharacterMaster)[21]) {
					getCharacterCurrentParams(kCharacterMaster)[21] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 15;
					MasterCall(&LogicManager::CONS_Master_Arrive, "DeutschA", 6, 0, 0);
					break;
				}

				if (_gameTime > 1370700 && !getCharacterCurrentParams(kCharacterMaster)[22]) {
					getCharacterCurrentParams(kCharacterMaster)[22] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 16;
					MasterCall(&LogicManager::CONS_Master_Depart, "DeutschA", 0, 0, 0);
					break;
				}

				if (_gameTime > 1490400 && !getCharacterCurrentParams(kCharacterMaster)[23]) {
					getCharacterCurrentParams(kCharacterMaster)[23] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 17;
					MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
					break;
				}
			}

			if (_gameTime > 1493100 && !getCharacterCurrentParams(kCharacterMaster)[24]) {
				getCharacterCurrentParams(kCharacterMaster)[24] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 19;
				MasterCall(&LogicManager::CONS_Master_Depart, "Strasbou", 0, 0, 0);
				break;
			}

			if (_gameTime > 1539000 && !getCharacterCurrentParams(kCharacterMaster)[25]) {
				getCharacterCurrentParams(kCharacterMaster)[25] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 20;
				MasterCall(&LogicManager::CONS_Master_Arrive, "BadenOos", 8, 0, 0);
				break;
			}

			if (_gameTime > 1541700 && !getCharacterCurrentParams(kCharacterMaster)[26]) {
				getCharacterCurrentParams(kCharacterMaster)[26] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 21;
				MasterCall(&LogicManager::CONS_Master_Depart, "BadenOos", 0, 0, 0);
				break;
			}

			if (_gameTime > 1674000 && !getCharacterCurrentParams(kCharacterMaster)[27]) {
				getCharacterCurrentParams(kCharacterMaster)[27] = 1;
				CONS_Master_SecondSleep(0, 0, 0, 0);
			}

			break;
		case 5:
			if (_gameTime > 1173600 && !getCharacterCurrentParams(kCharacterMaster)[11]) {
				getCharacterCurrentParams(kCharacterMaster)[11] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 6;
				MasterCall(&LogicManager::CONS_Master_Depart, "Chalons", 0, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (_gameTime > 1228500 && !getCharacterCurrentParams(kCharacterMaster)[12]) {
				getCharacterCurrentParams(kCharacterMaster)[12] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 7;
				MasterCall(&LogicManager::CONS_Master_Arrive, "BarLeDuc", 2, 0, 0);
				break;
			}

			// fall through
		case 7:
			if (_gameTime > 1231200 && !getCharacterCurrentParams(kCharacterMaster)[13]) {
				getCharacterCurrentParams(kCharacterMaster)[13] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 8;
				MasterCall(&LogicManager::CONS_Master_Depart, "BarLeDuc", 0, 0, 0);
				break;
			}

			// fall through
		case 8:
			if (_gameTime > 1260000 && !getCharacterCurrentParams(kCharacterMaster)[14]) {
				getCharacterCurrentParams(kCharacterMaster)[14] = 1;
				_timeSpeed = 1;
			}

			if (_gameTime > 1303200 && !getCharacterCurrentParams(kCharacterMaster)[15]) {
				getCharacterCurrentParams(kCharacterMaster)[15] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 9;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Nancy", 3, 0, 0);
				break;
			}

			// fall through
		case 9:
			if (_gameTime > 1307700 && !getCharacterCurrentParams(kCharacterMaster)[16]) {
				getCharacterCurrentParams(kCharacterMaster)[16] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 10;
				MasterCall(&LogicManager::CONS_Master_Depart, "Nancy", 0, 0, 0);
				break;
			}

			// fall through
		case 10:
			if (_gameTime > 1335600 && !getCharacterCurrentParams(kCharacterMaster)[17]) {
				getCharacterCurrentParams(kCharacterMaster)[17] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 11;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Luneville", 4, 0, 0);
				break;
			}

			// fall through
		case 11:
			if (_gameTime > 1338300 && !getCharacterCurrentParams(kCharacterMaster)[18]) {
				getCharacterCurrentParams(kCharacterMaster)[18] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 12;
				MasterCall(&LogicManager::CONS_Master_Depart, "Luneville", 0, 0, 0);
				break;
			}

			// fall through
		case 12:
			if (_gameTime > 1359900 && !getCharacterCurrentParams(kCharacterMaster)[19]) {
				getCharacterCurrentParams(kCharacterMaster)[19] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 13;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Avricourt", 5, 0, 0);
				break;
			}

			// fall through
		case 13:
			if (_gameTime > 1363500 && !getCharacterCurrentParams(kCharacterMaster)[20]) {
				getCharacterCurrentParams(kCharacterMaster)[20] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 14;
				MasterCall(&LogicManager::CONS_Master_Depart, "Avricourt", 0, 0, 0);
				break;
			}

			// fall through
		case 14:
			if (_gameTime > 1367100 && !getCharacterCurrentParams(kCharacterMaster)[21]) {
				getCharacterCurrentParams(kCharacterMaster)[21] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 15;
				MasterCall(&LogicManager::CONS_Master_Arrive, "DeutschA", 6, 0, 0);
				break;
			}

			// fall through
		case 15:
			if (_gameTime > 1370700 && !getCharacterCurrentParams(kCharacterMaster)[22]) {
				getCharacterCurrentParams(kCharacterMaster)[22] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 16;
				MasterCall(&LogicManager::CONS_Master_Depart, "DeutschA", 0, 0, 0);
				break;
			}

			// fall through
		case 16:
			if (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 16) {
				_gameTime = 1424700;
			}

			if (_gameTime > 1490400 && !getCharacterCurrentParams(kCharacterMaster)[23]) {
				getCharacterCurrentParams(kCharacterMaster)[23] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 17;
				MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
				break;
			}

			// fall through
		case 17:
			if (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 17) {
				_globals[kGlobalPhaseOfTheNight] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 18;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Strasbou", 7, 0, 0);
			}

			break;
		case 18:
			if (_gameTime > 1493100 && !getCharacterCurrentParams(kCharacterMaster)[24]) {
				getCharacterCurrentParams(kCharacterMaster)[24] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 19;
				MasterCall(&LogicManager::CONS_Master_Depart, "Strasbou", 0, 0, 0);
				break;
			}

			// fall through
		case 19:
			if (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 19) {
				_timeSpeed = 1;
			}

			if (_gameTime > 1539000 && !getCharacterCurrentParams(kCharacterMaster)[25]) {
				getCharacterCurrentParams(kCharacterMaster)[25] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 20;
				MasterCall(&LogicManager::CONS_Master_Arrive, "BadenOos", 8, 0, 0);
				break;
			}

			// fall through
		case 20:
			if (_gameTime > 1541700 && !getCharacterCurrentParams(kCharacterMaster)[26]) {
				getCharacterCurrentParams(kCharacterMaster)[26] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 21;
				MasterCall(&LogicManager::CONS_Master_Depart, "BadenOos", 0, 0, 0);
				break;
			}

			// fall through
		case 21:
			if (_gameTime > 1674000 && !getCharacterCurrentParams(kCharacterMaster)[27]) {
				getCharacterCurrentParams(kCharacterMaster)[27] = 1;
				CONS_Master_SecondSleep(0, 0, 0, 0);
			}

			break;
		case 22:
			getCharacterCurrentParams(kCharacterMaster)[4] = 1;
			break;
		case 23:
			getCharacterCurrentParams(kCharacterMaster)[0] = 1;
			break;
		default:
			break;
		}

		break;
	}
}

void LogicManager::CONS_Master_SecondSleep(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_SecondSleep);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 9;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_SecondSleep(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);
			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}

		playDialog(kCharacterCath, "MUS008", 16, 0);

		_activeItem = 0;
		fadeToBlack();

		while (dialogRunning("MUS008")) {
			_engine->getSoundManager()->soundThread();
			_engine->waitForTimer(4);
		}

		CONS_Master_StartPart2(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_StartPart2);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 10;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (_globals[kGlobalChapter] != 2) {
			_globals[kGlobalChapter] = 2;
			makeAllJump(2);
		}

		_gameTime = 1750500;
		_timeSpeed = 5;

		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;

		MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 1) {
			if (_engine->getArchiveManager()->lockCD(2)) {
				loadTrain(2);
				CONS_Master_InitPart2(0, 0, 0, 0);
			} else {
				_engine->getMenu()->doEgg(false, 0, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_InitPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_InitPart2);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 11;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_InitPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		_globals[kGlobalTrainIsRunning] = 1;
		setDoor(26, kCharacterCath, 1, 0, 9);
		setDoor(27, kCharacterCath, 1, 0, 9);
		_globals[kGlobalPhaseOfTheNight] = 1;
		giveCathItem(kItemGreenJacket);

		_items[kItemCorpse].floating = 0;
		_items[kItemCorpse].haveIt = 0;
		_globals[kGlobalJacket] = 2;
		_globals[kGlobalCathIcon] = 34;
		_globals[kGlobalCorpseMovedFromFloor] = 1;
		_globals[kGlobalFoundCorpse] = 1;
		dropItem(kItemBeetle, 3);
		dropItem(kItem3, 1);

		getCharacterCurrentParams(kCharacterMaster)[0] = 1;
		do {
			setModel(getCharacterCurrentParams(kCharacterMaster)[0], 2);
			getCharacterCurrentParams(kCharacterMaster)[0]++;
		} while (getCharacterCurrentParams(kCharacterMaster)[0] <= 8);

		getCharacterCurrentParams(kCharacterMaster)[0] = 33;
		do {
			setModel(getCharacterCurrentParams(kCharacterMaster)[0], 2);
			getCharacterCurrentParams(kCharacterMaster)[0]++;
		} while (getCharacterCurrentParams(kCharacterMaster)[0] <= 39);

		send(kCharacterMaster, kCharacterTableA, 103798704, 0);
		send(kCharacterMaster, kCharacterTableB, 103798704, 0);
		send(kCharacterMaster, kCharacterTableC, 103798704, 0);
		send(kCharacterMaster, kCharacterTableD, 103798704, 0);
		send(kCharacterMaster, kCharacterTableE, 103798704, 0);

		setDoor(1, kCharacterCath, 0, 10, 9);
		setDoor(9, kCharacterCath, 0, 255, 255);

		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);
			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}

		playNIS(kEventTrainPassing);

		if (cathHasItem(kItemScarf)) {
			bumpCathTylerComp();
		} else {
			bumpCath(kCarGreenSleeping, 79, 255);
		}

		CONS_Master_MovingDay(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_MovingDay(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_MovingDay);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 12;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_MovingDay(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_globals[kGlobalTrainIsRunning]) {
			if (getCharacterCurrentParams(kCharacterMaster)[1] ||
				(getCharacterCurrentParams(kCharacterMaster)[1] = _realTime + getCharacterCurrentParams(kCharacterMaster)[0],
				 _realTime + getCharacterCurrentParams(kCharacterMaster)[0] != 0)) {
				if (getCharacterCurrentParams(kCharacterMaster)[1] >= _realTime)
					break;

				getCharacterCurrentParams(kCharacterMaster)[1] = 0x7FFFFFFF;
			}

			switch (rnd(15)) {
			case 0:
				playDialog(kCharacterCath, "ZFX1005", rnd(15) + 2, 0);
				break;
			case 1:
				playDialog(kCharacterCath, "ZFX1006", rnd(15) + 2, 0);
				break;
			case 2:
				playDialog(kCharacterCath, "ZFX1007", rnd(15) + 2, 0);
				break;
			case 3:
				playDialog(kCharacterCath, "ZFX1007A", rnd(15) + 2, 0);
				break;
			case 4:
				playDialog(kCharacterCath, "ZFX1007B", rnd(15) + 2, 0);
				break;
			default:
				break;
			}

			getCharacterCurrentParams(kCharacterMaster)[0] = 225 * (4 * rnd(5) + 20);
			getCharacterCurrentParams(kCharacterMaster)[1] = 0;
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterMaster)[0] = 225 * (4 * rnd(5) + 20);
		break;
	case 139122728:
		CONS_Master_StartPart3(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_StartPart3);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 13;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (_globals[kGlobalChapter] != 3) {
			_globals[kGlobalChapter] = 3;
			makeAllJump(3);
		}

		_gameTime = 1944000;
		_timeSpeed = 5;

		CONS_Master_InitPart3(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_InitPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_InitPart3);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 14;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_InitPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		send(kCharacterMaster, kCharacterTableA, 103798704, 0);
		send(kCharacterMaster, kCharacterTableB, 103798704, 0);
		send(kCharacterMaster, kCharacterTableC, 103798704, 0);
		send(kCharacterMaster, kCharacterTableD, 103798704, 0);
		send(kCharacterMaster, kCharacterTableE, 103798704, 0);
		send(kCharacterMaster, kCharacterTableF, 103798704, 0);

		_globals[kGlobalTrainIsRunning] = 1;

		setDoor(26, kCharacterCath, 1, 0, 9);
		setDoor(27, kCharacterCath, 1, 0, 9);
		dropItem(kItemBriefcase, 1);
		dropItem(kItem3, 1);
		setModel(1, 2);
		setDoor(107, kCharacterCath, 3, 255, 255);

		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);
			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}

		fadeToBlack();
		bumpCath(kCarRestaurant, 60, 255);
		restoreIcons();

		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;

		MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 1) {
			CONS_Master_MovingAfternoon(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_MovingAfternoon(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_MovingAfternoon);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 15;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_MovingAfternoon(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		if (_globals[kGlobalTrainIsRunning]) {
			bool skip1 = false;

			if (!getCharacterCurrentParams(kCharacterMaster)[3]) {
				getCharacterCurrentParams(kCharacterMaster)[3] = _realTime + getCharacterCurrentParams(kCharacterMaster)[0];
				if (getCharacterCurrentParams(kCharacterMaster)[3] == 0) {
					skip1 = true;
				}
			}

			if (skip1 || getCharacterCurrentParams(kCharacterMaster)[3] < _realTime) {
				if (!skip1) {
					getCharacterCurrentParams(kCharacterMaster)[3] = 0x7FFFFFFF;
				}

				switch (rnd(5)) {
				case 0:
					playDialog(0, "ZFX1005", rnd(15) + 2, 0);
					break;
				case 1:
					playDialog(0, "ZFX1006", rnd(15) + 2, 0);
					break;
				case 2:
					playDialog(0, "ZFX1007", rnd(15) + 2, 0);
					break;
				case 3:
					playDialog(0, "ZFX1007A", rnd(15) + 2, 0);
					break;
				case 4:
					playDialog(0, "ZFX1007B", rnd(15) + 2, 0);
					break;
				default:
					break;
				}

				getCharacterCurrentParams(kCharacterMaster)[0] = 225 * (4 * rnd(5) + 20);
				getCharacterCurrentParams(kCharacterMaster)[3] = 0;
			}
		}

		bool skip2 = false;

		if (!getCharacterCurrentParams(kCharacterMaster)[4]) {
			getCharacterCurrentParams(kCharacterMaster)[4] = _realTime + getCharacterCurrentParams(kCharacterMaster)[1];
			if (getCharacterCurrentParams(kCharacterMaster)[4] == 0) {
				skip2 = true;
			}
		}

		if (skip2 || getCharacterCurrentParams(kCharacterMaster)[4] < _realTime) {
			if (!skip2) {
				getCharacterCurrentParams(kCharacterMaster)[4] = 0x7FFFFFFF;
			}

			switch (rnd(2)) {
			case 0:
				playDialog(0, "ZFX1008", rnd(15) + 2, 0);
				break;
			case 1:
			default:
				playDialog(0, "ZFX1009", rnd(15) + 2, 0);
				break;
			}

			getCharacterCurrentParams(kCharacterMaster)[1] = 225 * (4 * rnd(6) + 8);
			getCharacterCurrentParams(kCharacterMaster)[4] = 0;
		}

		if (_gameTime > 1982700 && !getCharacterCurrentParams(kCharacterMaster)[5]) {
			getCharacterCurrentParams(kCharacterMaster)[5] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;
			MasterCall(&LogicManager::CONS_Master_Arrive, "Salzburg", 9, 0, 0);
			break;
		}

		if (_gameTime > 1989900 && !getCharacterCurrentParams(kCharacterMaster)[6]) {
			getCharacterCurrentParams(kCharacterMaster)[6] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 2;
			MasterCall(&LogicManager::CONS_Master_Depart, "Salzburg", 0, 0, 0);
			break;
		}

		if (_gameTime > 2047500 && !getCharacterCurrentParams(kCharacterMaster)[7]) {
			getCharacterCurrentParams(kCharacterMaster)[7] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 3;
			MasterCall(&LogicManager::CONS_Master_Arrive, "Attnang", 10, 0, 0);
			break;
		}

		if (_gameTime > 2052900 && !getCharacterCurrentParams(kCharacterMaster)[8]) {
			getCharacterCurrentParams(kCharacterMaster)[8] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 4;
			MasterCall(&LogicManager::CONS_Master_Depart, "Attnang", 0, 0, 0);
			break;
		}

		if (_gameTime > 2073600 && !getCharacterCurrentParams(kCharacterMaster)[9]) {
			getCharacterCurrentParams(kCharacterMaster)[9] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 5;
			MasterCall(&LogicManager::CONS_Master_Arrive, "Wels", 11, 0, 0);
			break;
		}

		if (_gameTime > 2079900 && !getCharacterCurrentParams(kCharacterMaster)[10]) {
			getCharacterCurrentParams(kCharacterMaster)[10] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 6;
			MasterCall(&LogicManager::CONS_Master_Depart, "Wels", 0, 0, 0);
			break;
		}

		if (_gameTime > 2099700 && !getCharacterCurrentParams(kCharacterMaster)[11]) {
			getCharacterCurrentParams(kCharacterMaster)[11] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 7;
			MasterCall(&LogicManager::CONS_Master_Arrive, "Linz", 12, 0, 0);
			break;
		}

		if (_gameTime > 2105100 && !getCharacterCurrentParams(kCharacterMaster)[12]) {
			getCharacterCurrentParams(kCharacterMaster)[12] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 8;
			MasterCall(&LogicManager::CONS_Master_Depart, "Linz", 0, 0, 0);
			break;
		}

		if (_gameTime > 2187000 && !getCharacterCurrentParams(kCharacterMaster)[13]) {
			getCharacterCurrentParams(kCharacterMaster)[13] = 1;
			_timeSpeed = 5;
		}

		if (_gameTime > 2268000 && !getCharacterCurrentParams(kCharacterMaster)[14]) {
			getCharacterCurrentParams(kCharacterMaster)[14] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 9;
			MasterCall(&LogicManager::CONS_Master_Arrive, "Vienna", 13, 0, 0);
		}

		break;
	}
	case 2:
		if (getCharacterParams(kCharacterMaster, 8)[1]) {
			send(kCharacterMaster, kCharacterClerk, 191350523, 0);
			if (getCharacter(kCharacterCath).characterPosition.location == 2) {
				if (cathOutHisWindow()) {
					bumpCath(kCarGreenSleeping, 49, 255);
				} else {
					if (cathOutRebeccaWindow()) {
						bumpCath(kCarRedSleeping, 49, 255);
						_engine->getSoundManager()->startAmbient();
						_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
						getCharacterParams(kCharacterMaster, 8)[1] = 0;

						if (getCharacterCurrentParams(kCharacterMaster)[2])
							CONS_Master_EndVienna(0, 0, 0, 0);
					} else if (getCharacter(kCharacterCath).characterPosition.car >= 4 && getCharacter(kCharacterCath).characterPosition.car <= 7) {
						bumpCath(kCarRestaurant, 82, 255);
						_engine->getSoundManager()->startAmbient();
						_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
						getCharacterParams(kCharacterMaster, 8)[1] = 0;

						if (getCharacterCurrentParams(kCharacterMaster)[2])
							CONS_Master_EndVienna(0, 0, 0, 0);
					} else if (!getCharacter(kCharacterCath).characterPosition.car || getCharacter(kCharacterCath).characterPosition.car > 3) {
						_engine->getSoundManager()->startAmbient();
						_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
						getCharacterParams(kCharacterMaster, 8)[1] = 0;

						if (getCharacterCurrentParams(kCharacterMaster)[2])
							CONS_Master_EndVienna(0, 0, 0, 0);
					} else {
						if (checkCathDir(kCarGreenSleeping, 98)) {
							playDialog(0, "LIB015", -1, 0);
							bumpCath(kCarGreenSleeping, 71, 255);
						} else {
							bumpCath(kCarGreenSleeping, 82, 255);
						}
					}
				}
			}
			
			_engine->getSoundManager()->startAmbient();
			_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
			getCharacterParams(kCharacterMaster, 8)[1] = 0;

			if (getCharacterCurrentParams(kCharacterMaster)[2])
				CONS_Master_EndVienna(0, 0, 0, 0);
		} else if (getCharacterParams(kCharacterMaster, 8)[2]) {
			_engine->getSoundManager()->startAmbient();
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterMaster)[0] = 225 * (4 * rnd(5) + 20);
		getCharacterCurrentParams(kCharacterMaster)[1] = 225 * (4 * rnd(6) + 8);
		break;
	case 18:
		switch (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8]) {
		case 1:
			if (_gameTime > 1989900 && !getCharacterCurrentParams(kCharacterMaster)[6]) {
				getCharacterCurrentParams(kCharacterMaster)[6] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 2;
				MasterCall(&LogicManager::CONS_Master_Depart, "Salzburg", 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (_gameTime > 2047500 && !getCharacterCurrentParams(kCharacterMaster)[7]) {
				getCharacterCurrentParams(kCharacterMaster)[7] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 3;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Attnang", 10, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (_gameTime > 2052900 && !getCharacterCurrentParams(kCharacterMaster)[8]) {
				getCharacterCurrentParams(kCharacterMaster)[8] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 4;
				MasterCall(&LogicManager::CONS_Master_Depart, "Attnang", 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (_gameTime > 2073600 && !getCharacterCurrentParams(kCharacterMaster)[9]) {
				getCharacterCurrentParams(kCharacterMaster)[9] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 5;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Wels", 11, 0, 0);
				break;
			}

			// fall through
		case 5:
			if (_gameTime > 2079900 && !getCharacterCurrentParams(kCharacterMaster)[10]) {
				getCharacterCurrentParams(kCharacterMaster)[10] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 6;
				MasterCall(&LogicManager::CONS_Master_Depart, "Wels", 0, 0, 0);
				break;
			}

			// fall through
		case 6:
			if (_gameTime > 2099700 && !getCharacterCurrentParams(kCharacterMaster)[11]) {
				getCharacterCurrentParams(kCharacterMaster)[11] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 7;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Linz", 12, 0, 0);
				break;
			}

			// fall through
		case 7:
			if (_gameTime > 2105100 && !getCharacterCurrentParams(kCharacterMaster)[12]) {
				getCharacterCurrentParams(kCharacterMaster)[12] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 8;
				MasterCall(&LogicManager::CONS_Master_Depart, "Linz", 0, 0, 0);
				break;
			}

			// fall through
		case 8:
			if (_gameTime > 2187000 && !getCharacterCurrentParams(kCharacterMaster)[13]) {
				getCharacterCurrentParams(kCharacterMaster)[13] = 1;
				_timeSpeed = 5;
			}

			if (_gameTime > 2268000 && !getCharacterCurrentParams(kCharacterMaster)[14]) {
				getCharacterCurrentParams(kCharacterMaster)[14] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 9;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Vienna", 13, 0, 0);
			}

			break;
		case 9:
			getCharacterCurrentParams(kCharacterMaster)[2] = 1;
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_EndVienna(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_EndVienna);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 16;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_EndVienna(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		getCharacter(kCharacterCath).characterPosition.car = kCarLocomotive;
		if (_doneNIS[kEventAugustBringBriefcase]) {
			if (_items[kItemFirebird].floating == 5) {
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 2;
				MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventViennaKronosFirebird, 0, 0);
			} else if (getCharacterParams(kCharacterMaster, 8)[0]) {
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 3;
				MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventVergesAnnaDead, 0, 0);
			} else {
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 4;
				MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventViennaContinueGame, 0, 0);
			}
		} else {
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;
			MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventViennaAugustUnloadGuns, 0, 0);
		}

		break;
	case 18:
		switch (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8]) {
		case 1:
			playNIS(kEventViennaAugustUnloadGuns);

			if (_doneNIS[kEventConcertLeaveWithBriefcase]) {
				endGame(1, 2187000, 0, true);
			} else {
				if (!_doneNIS[kEventCathJumpDownCeiling]) {
					endGame(1, 2155500, 0, true);
				} else {
					endGame(2, 253, 0, true);
				}
			}

			break;
		case 2:
			playNIS(kEventViennaKronosFirebird);

			if (_doneNIS[kEventKronosBringEggCeiling]) {
				endGame(3, 58, 60, true);
			} else if (_doneNIS[kEventKronosBringEgg]) {
				endGame(1, 2155500, 60, true);
			} else {
				if (_globals[kGlobalRegisteredTimeAtWhichCathGaveFirebirdToKronos]) {
					if (_doneNIS[kEventKronosReturnBriefcase]) {
						endGame(1, 61, 61, true);
						return;
					} else {
						endGame(1, 2155500, 59, true);
					}
				} else {
					if (_doneNIS[kEventKronosReturnBriefcase]) {
						endGame(2, 61, 59, true);
						return;
					} else {
						endGame(1, 2155500, 59, true);
					}
				}
			}

			break;
		case 3:
			playNIS(kEventVergesAnnaDead);
			endGame(1, 2250000, 58, true);
			break;
		case 4:
			playNIS(kEventViennaContinueGame);
			CONS_Master_StartPart4(0, 0, 0, 0);
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_StartPart4);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 17;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (_globals[kGlobalChapter] != 4) {
			_globals[kGlobalChapter] = 4;
			makeAllJump(4);
		}

		_gameTime = 2353500;
		_timeSpeed = 5;

		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;

		MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 1) {
			if (_engine->getArchiveManager()->lockCD(3)) {
				loadTrain(3);
				CONS_Master_InitPart4(0, 0, 0, 0);
			} else {
				_engine->getMenu()->doEgg(false, 0, 0);
			}
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_InitPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_InitPart4);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 18;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_InitPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		_engine->getSoundManager()->endAmbient();
		_engine->getSoundManager()->startAmbient();

		_globals[kGlobalTrainIsRunning] = 1;

		setDoor(26, kCharacterCath, 1, 0, 9);
		setDoor(27, kCharacterCath, 1, 0, 9);

		send(kCharacterMaster, kCharacterClerk, 203419131, 0);
		send(kCharacterMaster, kCharacterTableA, 103798704, 0);
		send(kCharacterMaster, kCharacterTableB, 103798704, 0);
		send(kCharacterMaster, kCharacterTableC, 103798704, 0);
		send(kCharacterMaster, kCharacterTableD, 103798704, 0);
		send(kCharacterMaster, kCharacterTableE, 103798704, 0);
		send(kCharacterMaster, kCharacterTableF, 103798704, 0);

		takeItem(kItem3);
		dropItem(kItemBomb, 1);

		if (_items[kItemBeetle].floating == 3)
			takeItem(kItemBeetle);

		setModel(25, 2);
		setDoor(107, kCharacterCath, 3, 255, 255);

		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);

			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}

		fadeToBlack();

		if (cathHasItem(kItemFirebird)) {
			bumpCath(kCarGreenSleeping, 76, 255);
		} else {
			bumpCath(kCarRestaurant, 69, 255);
		}

		restoreIcons();
		CONS_Master_MovingSecondNight(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_MovingSecondNight(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_MovingSecondNight);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 19;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_MovingSecondNight(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
	{
		bool skip = false;
		if (_globals[kGlobalTrainIsRunning]) {
			if (getCharacterCurrentParams(kCharacterMaster)[5] ||
				(getCharacterCurrentParams(kCharacterMaster)[5] = _realTime + getCharacterCurrentParams(kCharacterMaster)[3], _realTime + getCharacterCurrentParams(kCharacterMaster)[3] != 0)) {
				if (getCharacterCurrentParams(kCharacterMaster)[5] >= _realTime) {
					skip = true;
				}

				if (!skip) {
					getCharacterCurrentParams(kCharacterMaster)[5] = 0x7FFFFFFF;
				}
			}

			if (!skip) {
				switch (rnd(5)) {
				case 0:
					playDialog(0, "ZFX1005", rnd(15) + 2, 0);
					break;
				case 1:
					playDialog(0, "ZFX1006", rnd(15) + 2, 0);
					break;
				case 2:
					playDialog(0, "ZFX1007", rnd(15) + 2, 0);
					break;
				case 3:
					playDialog(0, "ZFX1007A", rnd(15) + 2, 0);
					break;
				case 4:
					playDialog(0, "ZFX1007B", rnd(15) + 2, 0);
					break;
				default:
					break;
				}

				getCharacterCurrentParams(kCharacterMaster)[3] = 225 * (4 * rnd(5) + 20);
				getCharacterCurrentParams(kCharacterMaster)[5] = 0;
			}
		}

		if (getCharacterCurrentParams(kCharacterMaster)[6] ||
			(getCharacterCurrentParams(kCharacterMaster)[6] = _realTime + getCharacterCurrentParams(kCharacterMaster)[4], _realTime + getCharacterCurrentParams(kCharacterMaster)[4] != 0)) {
			if (getCharacterCurrentParams(kCharacterMaster)[6] >= _realTime) {
				if (_gameTime > 2381400 && !getCharacterCurrentParams(kCharacterMaster)[7]) {
					getCharacterCurrentParams(kCharacterMaster)[7] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;
					MasterCall(&LogicManager::CONS_Master_Arrive, "Pozsony", 14, 0, 0);
					break;
				}

				if (_gameTime > 2386800 && !getCharacterCurrentParams(kCharacterMaster)[8]) {
					getCharacterCurrentParams(kCharacterMaster)[8] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 2;
					MasterCall(&LogicManager::CONS_Master_Depart, "Pozsony", 0, 0, 0);
					break;
				}
	
				if (getModel(1) == 1 && _gameTime > 2403000 && !getCharacterCurrentParams(kCharacterMaster)[9]) {
					getCharacterCurrentParams(kCharacterMaster)[9] = 1;
					_globals[kGlobalPhaseOfTheNight] = 2;
				}

				if (!getCharacterCurrentParams(kCharacterMaster)[0]) {
					if (_gameTime > 2416500 && !getCharacterCurrentParams(kCharacterMaster)[10]) {
						getCharacterCurrentParams(kCharacterMaster)[10] = 1;
						getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 3;
						MasterCall(&LogicManager::CONS_Master_Arrive, "Galanta", 15, 0, 0);
						break;
					}

					if (_gameTime > 2421900 && !getCharacterCurrentParams(kCharacterMaster)[11]) {
						getCharacterCurrentParams(kCharacterMaster)[11] = 1;
						getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 4;
						MasterCall(&LogicManager::CONS_Master_Depart, "Galanta", 0, 0, 0);
						break;
					}
				}

				if (_gameTime > 2470500 && !getCharacterCurrentParams(kCharacterMaster)[12]) {
					getCharacterCurrentParams(kCharacterMaster)[12] = 1;
					if (_globals[kGlobalPhaseOfTheNight] == 2)
						_timeSpeed = 1;
				}

				if (_gameTime > 2506500 && !getCharacterCurrentParams(kCharacterMaster)[13]) {
					getCharacterCurrentParams(kCharacterMaster)[13] = 1;
					if (_globals[kGlobalPhaseOfTheNight] == 2)
						_globals[kGlobalPhaseOfTheNight] = 1;
				}

				if (_gameTime > 2520000 && !getCharacterCurrentParams(kCharacterMaster)[14]) {
					getCharacterCurrentParams(kCharacterMaster)[14] = 1;
					if (!getCharacterCurrentParams(kCharacterMaster)[1] && !getCharacterCurrentParams(kCharacterMaster)[2]) {
						getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 5;
						MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventTrainExplosionBridge, 0, 0);
					}
				}

				break;
			}

			getCharacterCurrentParams(kCharacterMaster)[6] = 0x7FFFFFFF;
		}

		switch (rnd(2)) {
		case 0:
			playDialog(0, "ZFX1008", rnd(15) + 2, 0);
			break;
		case 1:
		default:
			playDialog(0, "ZFX1009", rnd(15) + 2, 0);
			break;
		}

		getCharacterCurrentParams(kCharacterMaster)[4] = 225 * (4 * rnd(6) + 8);
		getCharacterCurrentParams(kCharacterMaster)[6] = 0;

		if (_gameTime > 2381400 && !getCharacterCurrentParams(kCharacterMaster)[7]) {
			getCharacterCurrentParams(kCharacterMaster)[7] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;
			MasterCall(&LogicManager::CONS_Master_Arrive, "Pozsony", 14, 0, 0);
			break;
		}

		if (_gameTime > 2386800 && !getCharacterCurrentParams(kCharacterMaster)[8]) {
			getCharacterCurrentParams(kCharacterMaster)[8] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 2;
			MasterCall(&LogicManager::CONS_Master_Depart, "Pozsony", 0, 0, 0);
			break;
		}

		if (getModel(1) == 1 && _gameTime > 2403000 && !getCharacterCurrentParams(kCharacterMaster)[9]) {
			getCharacterCurrentParams(kCharacterMaster)[9] = 1;
			_globals[kGlobalPhaseOfTheNight] = 2;
		}

		if (!getCharacterCurrentParams(kCharacterMaster)[0]) {
			if (_gameTime > 2416500 && !getCharacterCurrentParams(kCharacterMaster)[10]) {
				getCharacterCurrentParams(kCharacterMaster)[10] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 3;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Galanta", 15, 0, 0);
				break;
			}

			if (_gameTime > 2421900 && !getCharacterCurrentParams(kCharacterMaster)[11]) {
				getCharacterCurrentParams(kCharacterMaster)[11] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 4;
				MasterCall(&LogicManager::CONS_Master_Depart, "Galanta", 0, 0, 0);
				break;
			}
		}

		if (_gameTime > 2470500 && !getCharacterCurrentParams(kCharacterMaster)[12]) {
			getCharacterCurrentParams(kCharacterMaster)[12] = 1;
			if (_globals[kGlobalPhaseOfTheNight] == 2)
				_timeSpeed = 1;
		}

		if (_gameTime > 2506500 && !getCharacterCurrentParams(kCharacterMaster)[13]) {
			getCharacterCurrentParams(kCharacterMaster)[13] = 1;
			if (_globals[kGlobalPhaseOfTheNight] == 2)
				_globals[kGlobalPhaseOfTheNight] = 1;
		}

		if (_gameTime > 2520000 && !getCharacterCurrentParams(kCharacterMaster)[14]) {
			getCharacterCurrentParams(kCharacterMaster)[14] = 1;
			if (!getCharacterCurrentParams(kCharacterMaster)[1] && !getCharacterCurrentParams(kCharacterMaster)[2]) {
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 5;
				MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventTrainExplosionBridge, 0, 0);
			}
		}

		break;
	}
	case 2:
		if (getCharacterParams(kCharacterMaster, 8)[1]) {
			send(kCharacterMaster, kCharacterClerk, 191350523, 0);
			if (getCharacter(kCharacterCath).characterPosition.location == 2) {
				if (cathOutHisWindow()) {
					bumpCath(kCarGreenSleeping, 49, 255);
					_engine->getSoundManager()->startAmbient();
					_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
					getCharacterParams(kCharacterMaster, 8)[1] = 0;
					break;
				}

				if (cathOutRebeccaWindow()) {
					bumpCath(kCarRedSleeping, 49, 255);
				} else {
					if (getCharacter(kCharacterCath).characterPosition.car < 4 || getCharacter(kCharacterCath).characterPosition.car > 7) {
						if (!getCharacter(kCharacterCath).characterPosition.car || getCharacter(kCharacterCath).characterPosition.car > 3) {
							_engine->getSoundManager()->startAmbient();
							_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
							getCharacterParams(kCharacterMaster, 8)[1] = 0;
							break;
						}

						if (checkCathDir(kCarGreenSleeping, 98)) {
							playDialog(0, "LIB015", -1, 0);
							bumpCath(kCarGreenSleeping, 71, 255);
						} else {
							bumpCath(kCarGreenSleeping, 82, 255);
						}

						_engine->getSoundManager()->startAmbient();
						_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
						getCharacterParams(kCharacterMaster, 8)[1] = 0;
						break;
					}

					bumpCath(kCarRestaurant, 82, 255);
				}
			}

			_engine->getSoundManager()->startAmbient();
			_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			break;
		}

		if (getCharacterParams(kCharacterMaster, 8)[2]) {
			_engine->getSoundManager()->startAmbient();
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		} else if (!getCharacterCurrentParams(kCharacterMaster)[1] && !getCharacterCurrentParams(kCharacterMaster)[2]) {
			playDialog(kCharacterMaster, "ZFX1001", -1, 0);
		}

		break;
	case 3:
		endGraphics(kCharacterMaster);
		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 11;
		MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
		break;
	case 12:
		getCharacterCurrentParams(kCharacterMaster)[3] = 225 * (4 * rnd(5) + 20);
		getCharacterCurrentParams(kCharacterMaster)[4] = 225 * (4 * rnd(6) + 8);
		break;
	case 18:
		switch (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8]) {
		case 1:
			if (_gameTime > 2386800 && !getCharacterCurrentParams(kCharacterMaster)[8]) {
				getCharacterCurrentParams(kCharacterMaster)[8] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 2;
				MasterCall(&LogicManager::CONS_Master_Depart, "Pozsony", 0, 0, 0);
				break;
			}

			// fall through
		case 2:
			if (getModel(1) == 1 && _gameTime > 2403000 && !getCharacterCurrentParams(kCharacterMaster)[9]) {
				getCharacterCurrentParams(kCharacterMaster)[9] = 1;
				_globals[kGlobalPhaseOfTheNight] = 2;
			}

			if (getCharacterCurrentParams(kCharacterMaster)[0]) {
				if (_gameTime > 2470500 && !getCharacterCurrentParams(kCharacterMaster)[12]) {
					getCharacterCurrentParams(kCharacterMaster)[12] = 1;
					if (_globals[kGlobalPhaseOfTheNight] == 2)
						_timeSpeed = 1;
				}

				if (_gameTime > 2506500 && !getCharacterCurrentParams(kCharacterMaster)[13]) {
					getCharacterCurrentParams(kCharacterMaster)[13] = 1;
					if (_globals[kGlobalPhaseOfTheNight] == 2)
						_globals[kGlobalPhaseOfTheNight] = 1;
				}

				if (_gameTime > 2520000 && !getCharacterCurrentParams(kCharacterMaster)[14]) {
					getCharacterCurrentParams(kCharacterMaster)[14] = 1;
					if (!getCharacterCurrentParams(kCharacterMaster)[1] && !getCharacterCurrentParams(kCharacterMaster)[2]) {
						getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 5;
						MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventTrainExplosionBridge, 0, 0);
					}
				}

				break;
			}

			if (_gameTime > 2416500 && !getCharacterCurrentParams(kCharacterMaster)[10]) {
				getCharacterCurrentParams(kCharacterMaster)[10] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 3;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Galanta", 15, 0, 0);
				break;
			}

			// fall through
		case 3:
			if (_gameTime > 2421900 && !getCharacterCurrentParams(kCharacterMaster)[11]) {
				getCharacterCurrentParams(kCharacterMaster)[11] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 4;
				MasterCall(&LogicManager::CONS_Master_Depart, "Galanta", 0, 0, 0);
				break;
			}

			// fall through
		case 4:
			if (_gameTime > 2470500 && !getCharacterCurrentParams(kCharacterMaster)[12]) {
				getCharacterCurrentParams(kCharacterMaster)[12] = 1;
				if (_globals[kGlobalPhaseOfTheNight] == 2)
					_timeSpeed = 1;
			}

			if (_gameTime > 2506500 && !getCharacterCurrentParams(kCharacterMaster)[13]) {
				getCharacterCurrentParams(kCharacterMaster)[13] = 1;
				if (_globals[kGlobalPhaseOfTheNight] == 2)
					_globals[kGlobalPhaseOfTheNight] = 1;
			}

			if (_gameTime > 2520000 && !getCharacterCurrentParams(kCharacterMaster)[14]) {
				getCharacterCurrentParams(kCharacterMaster)[14] = 1;
				if (!getCharacterCurrentParams(kCharacterMaster)[1] && !getCharacterCurrentParams(kCharacterMaster)[2]) {
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 5;
					MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventTrainExplosionBridge, 0, 0);
				}
			}

			break;
		case 5:
			if (whoRunningDialog(kCharacterMaster))
				endDialog(kCharacterMaster);

			playNIS(kEventTrainExplosionBridge);
			endGame(0, 1, 0, true);
			break;
		case 6:
			_engine->getSoundManager()->endAmbient();
			playNIS(kEventTylerCastleDream);
			_engine->getSoundManager()->startAmbient();
			_globals[kGlobalPhaseOfTheNight] = 1;
			bumpCathTylerComp();
			send(kCharacterMaster, kCharacterTatiana, 169360385, 0);
			_timeSpeed = 1;
			_gameTime = 2511900;
			dropItem(kItem2, 1);
			takeItem(kItem22);
			getCharacter(kCharacterMaster).characterPosition.car = 4;
			getCharacter(kCharacterMaster).characterPosition.position = 1500;
			getCharacter(kCharacterMaster).characterPosition.location = 1;
			playDialog(kCharacterMaster, "ZFX1001", -1, 0);
			break;
		case 7:
			playNIS(kEventTrainExplosionBridge);
			endGame(1, 2430000, 0, true);
			break;
		case 8:
			playDialog(0, "MUS022", -1, 0);
			if (_gameTime < 2517300)
				_gameTime = 2517300;

			break;
		case 9:
			playNIS(kEventCathDefusingBomb);
			bumpCath(kCarRedSleeping, 73, 255);
			break;
		case 10:
			playNIS(kEventDefuseBomb);
			forceJump(kCharacterAbbot, &LogicManager::CONS_Abbot_AfterBomb);
			send(kCharacterMaster, kCharacterAnna, 191001984, 0);
			send(kCharacterMaster, kCharacterCond2, 191001984, 0);
			takeItem(kItem2);
			_items[kItem2].floating = 0;
			getCharacterCurrentParams(kCharacterMaster)[1] = 1;
			bumpCath(kCarRedSleeping, 2, 255);
			break;
		case 11:
			bumpCath(kCarRedSleeping, 74, 255);
			playDialog(kCharacterClerk, "ZFX4001", 16, 0);
			endGame(0, 1, 0, true);
			break;
		default:
			break;
		}

		break;
	case 139254416:
		CONS_Master_StartPart5(0, 0, 0, 0);
		break;
	case 156435676:
		send(kCharacterMaster, kCharacterTatiana, 169360385, 0);
		send(kCharacterMaster, kCharacterCond2, 201431954, 0);
		send(kCharacterMaster, kCharacterTrainM, 201431954, 0);
		_timeSpeed = 1;
		_gameTime = 2511900;
		dropItem(kItem2, 1);
		getCharacter(kCharacterMaster).characterPosition.car = 4;
		getCharacter(kCharacterMaster).characterPosition.position = 1500;
		getCharacter(kCharacterMaster).characterPosition.location = 1;
		playDialog(kCharacterMaster, "ZFX1001", -1, 0);
		break;
	case 158610240:
		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 8;
		MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
		break;
	case 169300225:
		if (_gameTime < 2519100)
			_gameTime = 2519100;

		getCharacterCurrentParams(kCharacterMaster)[2] = 1;
		startSeqOtis(kCharacterMaster, "BOMB");
		break;
	case 190346110:
		_globals[kGlobalPhaseOfTheNight] = 3;
		getCharacterCurrentParams(kCharacterMaster)[0] = 1;
		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);
			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}

		fadeToBlack();
		playDialog(0, "MUS008", 16, 0);
		_activeItem = 0;
		fadeToBlack();

		while (dialogRunning("MUS008")) {
			_engine->getSoundManager()->soundThread();
			_engine->waitForTimer(4);
		}

		if (cathHasItem(kItemBomb)) {
			forceJump(kCharacterAlexei, &LogicManager::CONS_Alexei_Dead);
			forceJump(kCharacterAnna, &LogicManager::CONS_Anna_Sulking);
			forceJump(kCharacterAugust, &LogicManager::CONS_August_Asleep4);
			forceJump(kCharacterCond1, &LogicManager::CONS_Cond1_OnDuty4);
			forceJump(kCharacterCond2, &LogicManager::CONS_Cond2_OnDuty4);
			forceJump(kCharacterWaiter1, &LogicManager::CONS_Waiter1_Serving4);
			forceJump(kCharacterWaiter2, &LogicManager::CONS_Waiter2_Serving4);
			forceJump(kCharacterHeadWait, &LogicManager::CONS_HeadWait_InPart4);
			forceJump(kCharacterTrainM, &LogicManager::CONS_TrainM_InOffice4);
			forceJump(kCharacterTatiana, &LogicManager::CONS_Tatiana_Asleep4);
			forceJump(kCharacterAbbot, &LogicManager::CONS_Abbot_Asleep4);
			forceJump(kCharacterMilos, &LogicManager::CONS_Milos_EndPart4);
			forceJump(kCharacterVesna, &LogicManager::CONS_Vesna_EndPart4);
			forceJump(kCharacterIvo, &LogicManager::CONS_Ivo_EndPart4);
			forceJump(kCharacterSalko, &LogicManager::CONS_Salko_EndPart4);
			forceJump(kCharacterMadame, &LogicManager::CONS_Madame_Asleep4);
			forceJump(kCharacterMonsieur, &LogicManager::CONS_Monsieur_Asleep4);
			forceJump(kCharacterRebecca, &LogicManager::CONS_Rebecca_Asleep4);
			forceJump(kCharacterSophie, &LogicManager::CONS_Sophie_Asleep4);
			forceJump(kCharacterYasmin, &LogicManager::CONS_Yasmin_Asleep4);
			forceJump(kCharacterHadija, &LogicManager::CONS_Hadija_Asleep4);
			forceJump(kCharacterAlouan, &LogicManager::CONS_Alouan_Asleep4);
			forceJump(kCharacterMax, &LogicManager::CONS_Max_InCageFriendly);

			send(kCharacterMaster, kCharacterAnna, 201431954, 0);
			send(kCharacterMaster, kCharacterCond1, 201431954, 0);
			send(kCharacterMaster, kCharacterCond2, 201431954, 0);
			send(kCharacterMaster, kCharacterWaiter1, 201431954, 0);
			send(kCharacterMaster, kCharacterWaiter2, 201431954, 0);
			send(kCharacterMaster, kCharacterHeadWait, 201431954, 0);
			send(kCharacterMaster, kCharacterTrainM, 201431954, 0);

			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 6;
			MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventTylerCastleDream, 0, 0);
		} else {
			_gameTime = 2520000;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 7;
			MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventTrainExplosionBridge, 0, 0);
		}

		break;
	case 191001984:
		_gameTime = 2520000;

		if (whoRunningDialog(kCharacterMaster))
			endDialog(kCharacterMaster);

		endGraphics(kCharacterMaster);
		takeCathItem(kItemTelegram);
		_timeSpeed = 5;
		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 10;
		MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventDefuseBomb, 0, 0);
		break;
	case 201959744:
		if (whoRunningDialog(kCharacterMaster))
			endDialog(kCharacterMaster);

		playDialog(kCharacterClerk, "ZFX4001", 16, 0);
		endGame(0, 0, 0, true);
		break;
	case 225367984:
		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 9;
		MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventCathDefusingBomb, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_StartPart5);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 20;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		if (_globals[kGlobalChapter] != 5) {
			_globals[kGlobalChapter] = 5;
			makeAllJump(5);
		}

		_gameTime = 2844000;
		_timeSpeed = 2;

		CONS_Master_InitPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_InitPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_InitPart5);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 21;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_InitPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		endGraphics(kCharacterTableA);
		endGraphics(kCharacterTableB);
		endGraphics(kCharacterTableC);
		endGraphics(kCharacterTableD);
		endGraphics(kCharacterTableE);
		endGraphics(kCharacterTableF);

		_globals[kGlobalTrainIsRunning] = 1;

		setDoor(26, kCharacterCath, 1, 0, 9);
		setDoor(27, kCharacterCath, 1, 0, 9);
		setDoor(1, kCharacterCath, 0, 10, 9);
		setDoor(2, kCharacterCath, 0, 10, 9);
		setDoor(3, kCharacterCath, 0, 10, 9);
		setDoor(4, kCharacterCath, 3, 10, 9);
		setDoor(5, kCharacterCath, 3, 10, 9);
		setDoor(6, kCharacterCath, 3, 10, 9);
		setDoor(7, kCharacterCath, 3, 10, 9);
		setDoor(8, kCharacterCath, 3, 10, 9);
		setDoor(32, kCharacterCath, 0, 10, 9);
		setDoor(33, kCharacterCath, 0, 10, 9);
		setDoor(34, kCharacterCath, 0, 10, 9);
		setDoor(35, kCharacterCath, 0, 10, 9);
		setDoor(36, kCharacterCath, 0, 10, 9);
		setDoor(37, kCharacterCath, 0, 10, 9);
		setDoor(38, kCharacterCath, 3, 10, 9);
		setDoor(39, kCharacterCath, 3, 10, 9);
		setDoor(17, kCharacterCath, 0, 10, 9);
		setDoor(18, kCharacterCath, 0, 10, 9);
		setDoor(19, kCharacterCath, 0, 10, 9);
		setDoor(20, kCharacterCath, 3, 10, 9);
		setDoor(21, kCharacterCath, 3, 10, 9);
		setDoor(22, kCharacterCath, 3, 10, 9);
		setDoor(48, kCharacterCath, 0, 10, 9);
		setDoor(49, kCharacterCath, 0, 10, 9);
		setDoor(50, kCharacterCath, 0, 10, 9);
		setDoor(51, kCharacterCath, 0, 10, 9);
		setDoor(52, kCharacterCath, 0, 10, 9);
		setDoor(53, kCharacterCath, 0, 10, 9);
		setDoor(26, kCharacterCath, 0, 10, 9);
		setDoor(27, kCharacterCath, 0, 10, 9);

		_globals[kGlobalPhaseOfTheNight] = 1;
		_activeItem = 0;
		_globals[kGlobalCathInSpecialState] = 1;
		_globals[kGlobalCathIcon] = 36;

		takeCathItem(kItemKey);
		takeCathItem(kItemBomb);
		takeCathItem(kItemMatch);

		if (cathHasItem(kItemFirebird)) {
			takeCathItem(kItemFirebird);
			dropItem(kItemFirebird, 3);

			if (cathHasItem(kItemWhistle)) {
				takeCathItem(kItemWhistle);
				dropItem(kItemWhistle, 3);
			}
		}

		setDoor(93, kCharacterCath, 2, 255, 255);
		setDoor(94, kCharacterCath, 0, 255, 255);
		setDoor(101, kCharacterCath, 0, 10, 9);
		setModel(98, 2);
		setModel(55, 2);

		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);
			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}

		fadeToBlack();
		bumpCath(kCarBaggageRear, 95, 255);
		restoreIcons();

		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;

		MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
		break;
	case 18:
		if (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 1) {
			CONS_Master_MovingPart5(0, 0, 0, 0);
		}

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Master_MovingPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_MovingPart5);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 22;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_MovingPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		if (_gameTime > 2907000 && !getCharacterCurrentParams(kCharacterMaster)[1]) {
			getCharacterCurrentParams(kCharacterMaster)[1] = 1;
			if (!_globals[kGlobalIsDayTime]) {
				playDialog(kCharacterMaster, "ARRIVE", 8, 0);
				_engine->getSoundManager()->endAmbient();
			}
		}

		if (_gameTime > 2943000 && !getCharacterCurrentParams(kCharacterMaster)[2]) {
			getCharacterCurrentParams(kCharacterMaster)[2] = 1;
			if (!_doneNIS[kEventLocomotiveMilosDay] && !_doneNIS[kEventLocomotiveMilosNight]) {
				playDialog(kCharacterMaster, "ARRIVE", 8, 0);
				_engine->getSoundManager()->endAmbient();
			}
		}

		break;
	case 2:
		if (_gameTime <= 2943000) {
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;
			MasterCall(&LogicManager::CONS_Master_SaveGame, 2, kEventTrainStopped, 0, 0);
		} else {
			endGame(1, 2934000, 51, true);
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterMaster)[0] = 225 * (4 * rnd(10) + 20);
		break;
	case 18:
		if (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 1) {
			playNIS(kEventTrainStopped);
			endGame(1, 2898000, 51, true);
		}

		break;
	case 135800432:
		_globals[kGlobalIsDayTime] = 1;
		_gameTime = 2916000;

		if (whoRunningDialog(kCharacterMaster))
			endDialog(kCharacterMaster);

		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsMaster[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Master_SaveGame,
	&LogicManager::HAND_Master_Arrive,
	&LogicManager::HAND_Master_Depart,
	&LogicManager::HAND_Master_Birth,
	&LogicManager::HAND_Master_WalkTest,
	&LogicManager::HAND_Master_FirstDream,
	&LogicManager::HAND_Master_StartPart1,
	&LogicManager::HAND_Master_MovingNight,
	&LogicManager::HAND_Master_SecondSleep,
	&LogicManager::HAND_Master_StartPart2,
	&LogicManager::HAND_Master_InitPart2,
	&LogicManager::HAND_Master_MovingDay,
	&LogicManager::HAND_Master_StartPart3,
	&LogicManager::HAND_Master_InitPart3,
	&LogicManager::HAND_Master_MovingAfternoon,
	&LogicManager::HAND_Master_EndVienna,
	&LogicManager::HAND_Master_StartPart4,
	&LogicManager::HAND_Master_InitPart4,
	&LogicManager::HAND_Master_MovingSecondNight,
	&LogicManager::HAND_Master_StartPart5,
	&LogicManager::HAND_Master_InitPart5,
	&LogicManager::HAND_Master_MovingPart5
};

} // End of namespace LastExpress
