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
			_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]);

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
		return;
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
	if (msg->action) {
		if (msg->action == 12) {
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
		}
	} else {
		getCharacter(kCharacterMaster).currentCall--;
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMaster,
			_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]);

		fedEx(kCharacterMaster, kCharacterMaster, 18, 0);
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
	if (msg->action == 12) {
		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);

			if (getCharacterParams(kCharacterMaster, 8)[1]) {
				send(kCharacterMaster, kCharacterClerk, 191350523, 0);

				if (getCharacter(kCharacterCath).characterPosition.location == 2) {
					if (cathOutHisWindow()) {
						bumpCath(3, 49, 255);
					} else if (cathOutRebeccaWindow()) {
						bumpCath(4, 49, 255);
					} else if (getCharacter(kCharacterCath).characterPosition.car < 4 || getCharacter(kCharacterCath).characterPosition.car > 7) {
						if (getCharacter(kCharacterCath).characterPosition.car && getCharacter(kCharacterCath).characterPosition.car <= 3) {
							if (checkCathDir(3, 98)) {
								playDialog(kCharacterCath, "LIB015", -1, 0);
								bumpCath(3, 71, 255);
							} else {
								bumpCath(3, 82, 255);
							}
						}
					} else {
						bumpCath(5, 82, 255);
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

		_gameProgress[kProgressIsTrainRunning] = 0;

		getCharacterParams(kCharacterMaster, 8)[1] = 1;
		getCharacterParams(kCharacterMaster, 8)[3] = getCharacterCurrentParams(kCharacterMaster)[3];

		getCharacter(kCharacterMaster).currentCall--;
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMaster,
			_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]);

		fedEx(kCharacterMaster, kCharacterMaster, 18, 0);
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
	if (msg->action == 12) {
		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);

			if (getCharacterParams(kCharacterMaster, 8)[1]) {
				send(kCharacterMaster, kCharacterClerk, 191350523, 0);

				if (getCharacter(kCharacterCath).characterPosition.location == 2) {
					if (cathOutHisWindow()) {
						bumpCath(kCarGreenSleeping, 49, 255);
					} else if (cathOutRebeccaWindow()) {
						bumpCath(kCarRedSleeping, 49, 255);
					} else if (getCharacter(kCharacterCath).characterPosition.car < 4 || getCharacter(kCharacterCath).characterPosition.car > 7) {
						if (getCharacter(kCharacterCath).characterPosition.car && getCharacter(kCharacterCath).characterPosition.car <= 3) {
							if (checkCathDir(kCarGreenSleeping, 98)) {
								playDialog(kCharacterCath, "LIB015", -1, 0);
								bumpCath(kCarGreenSleeping, 71, 255);
							} else {
								bumpCath(kCarGreenSleeping, 82, 255);
							}
						}
					} else {
						bumpCath(5, 82, 255);
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

		setDoor(26, 0, 1, 0, 9);
		setDoor(27, 0, 1, 0, 9);

		_gameProgress[kProgressIsTrainRunning] = 1;

		getCharacterParams(kCharacterMaster, 8)[2] = 1;

		getCharacter(kCharacterMaster).currentCall--;
		_engine->getMessageManager()->setMessageHandle(
			kCharacterMaster,
			_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]);

		fedEx(kCharacterMaster, kCharacterMaster, 18, 0);
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
	if (msg->action == 12) {
		autoMessage(kCharacterMaster, 171843264, 0);
		CONS_Master_StartPart1(0, 0, 0, 0);
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
	if (msg->action == 12) {
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
			_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]);

		fedEx(kCharacterMaster, kCharacterMaster, 18, 0);
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
	if (msg->action == 2) {
		playDialog(kCharacterMaster, "MUS009", 16, 0);
	} else if (msg->action == 8) {
		if (!cathRunningDialog("LIB012"))
			playDialog(kCharacterCath, "LIB012", -1, 0);
	} else if (msg->action == 12) {
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

		_inventorySelectedItemIdx = 0;
		fadeToBlack();

		while (dialogRunning("MUS008"))
			_engine->getSoundManager()->soundThread();

		_gameProgress[kProgressField84] = 1;
		bumpCath(kCarLocomotive, 75, 255);
		restoreIcons();

		_gameTime = 1492200;
		_gameProgress[kProgressField18] = 4;
		_gameTimeTicksDelta = 0;

		setDoor(63, kCharacterMaster, 0, 0, 9);
		send(kCharacterMaster, kCharacterClerk, 191350523, 0);

		_gameProgress[kProgressIsTrainRunning] = 0;
		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;

		MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
	} else if (msg->action == 9) {
		if (getCharacterCurrentParams(kCharacterMaster)[0]) {
			endGraphics(kCharacterMaster);

			fadeDialog(kCharacterMaster);
			playDialog(kCharacterCath, "LIB014", -1, 0);

			_engine->getSoundManager()->startAmbient();
			getCharacterParams(kCharacterMaster, 8)[3] = 7;
			_engine->getSoundManager()->startSteam(16);

			playNIS(kEventCathDream);

			_gameTimeTicksDelta = 3;
			_gameProgress[kProgressField18] = 1;

			setDoor(63, 0, 0, 10, 9);

			_gameProgress[kProgressField84] = 0;

			bumpCathTylerComp();

			getCharacter(kCharacterMaster).currentCall--;
			_engine->getMessageManager()->setMessageHandle(
				kCharacterMaster,
				_functionsMaster[getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall]]);

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
	} else if (msg->action == 225358684) {
		getCharacterCurrentParams(kCharacterMaster)[1]++;

		if (getCharacterCurrentParams(kCharacterMaster)[1] >= 3) {
			if (!cathRunningDialog("LIB031"))
				playDialog(kCharacterCath, "LIB031", -1, 0);

			if (getCharacterCurrentParams(kCharacterMaster)[1] == 3) {
				getCharacter(kCharacterMaster).characterPosition.car = 8;
				startCycOtis(kCharacterMaster, "JUGL");
			}
		}
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
	if (msg->action == 12) {
		_gameProgress[kProgressChapter] = 1;

		_engine->getSoundManager()->startAmbient();

		_gameTime = 1061100;
		_gameTimeTicksDelta = 0;
		_gameProgress[kProgressIsTrainRunning] = 1;
		_gameProgress[kProgressPortrait] = 32;
		_gameProgress[kProgressField18] = 1;

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

		_gameProgress[kProgressField7C] = 1;

		setDoor(1, 0, 0, 10, 9);
		setDoor(9, 0, 0, 255, 255);

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

		setDoor(25, 0, 0, 0, 1);
		setDoor(23, 0, 0, 0, 1);
		setDoor(24, 0, 0, 0, 1);
		setDoor(28, 0, 0, 0, 1);
		setDoor(56, 0, 0, 0, 1);
		setDoor(54, 0, 0, 0, 1);
		setDoor(55, 0, 0, 0, 1);
		setDoor(59, 0, 0, 0, 1);
		setDoor(66, 0, 0, 0, 1);
		setDoor(64, 0, 0, 0, 1);
		setDoor(65, 0, 0, 0, 1);
		setDoor(69, 0, 0, 0, 1);
		setDoor(98, 0, 0, 0, 1);
		setDoor(26, 0, 1, 0, 9);
		setDoor(27, 0, 1, 0, 9);
		setDoor(101, 0, 1, 10, 9);

		CONS_Master_MovingNight(0, 0, 0, 0);
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

	int v6;

	uint16 car;
	int v9; 
	int v10;
	int v11;
	int v12;
	int v13;
	int v14;

	if (msg->action > 12) {
		if (msg->action > 169629818) {
			if (msg->action == 190346110) {
				_gameProgress[6] = 3;
				if (_gameTime >= 1494000) {
					CONS_Master_SecondSleep(0, 0, 0, 0);
				} else {
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 23;
					MasterCall(&LogicManager::CONS_Master_FirstDream, 0, 0, 0, 0);
				}
			}
		} else {
			switch (msg->action) {
			case 169629818:
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 22;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Unschedu", 0x10, 0, 0);
				break;
			case 17:
				if (!getCharacterCurrentParams(kCharacterMaster)[2] && checkCathDir(3, 1)) {
					_gameTime = 1062000;
					_gameTimeTicksDelta = 3;
					getCharacterCurrentParams(kCharacterMaster)[2] = 1;
				}
				break;
			case 18:
				switch (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8]) {
				case 1:
					goto LABEL_75;
				case 2:
					goto LABEL_78;
				case 3:
					goto LABEL_81;
				case 4:
					getCharacterCurrentParams(kCharacterMaster)[4] = 0;
					goto LABEL_57;
				case 5:
					goto LABEL_84;
				case 6:
					goto LABEL_87;
				case 7:
					goto LABEL_90;
				case 8:
					goto LABEL_93;
				case 9:
					goto LABEL_99;
				case 10:
					goto LABEL_102;
				case 11:
					goto LABEL_105;
				case 12:
					goto LABEL_108;
				case 13:
					goto LABEL_111;
				case 14:
					goto LABEL_114;
				case 15:
					goto LABEL_117;
				case 16:
					_gameTime = 1424700;
					goto LABEL_65;
				case 17:
					_gameProgress[6] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 18;
					MasterCall(&LogicManager::CONS_Master_Arrive, "Strasbou", 7, 0, 0);
					break;
				case 18:
					goto LABEL_120;
				case 19:
					_gameTimeTicksDelta = 1;
					goto LABEL_70;
				case 20:
					goto LABEL_123;
				case 21:
					goto LABEL_126;
				case 22:
					getCharacterCurrentParams(kCharacterMaster)[4] = 1;
					break;
				case 23:
					getCharacterCurrentParams(kCharacterMaster)[0] = 1;
					break;
				default:
					return;
				}
				break;
			}
		}
		return;
	}

	if (msg->action == 12) {
		getCharacterCurrentParams(kCharacterMaster)[1] = 225 * (4 * rnd(5) + 20);
		return;
	}

	if (!msg->action) {
		if (_gameProgress[20] && _gameTime < 1458000) {
			if (!getCharacterCurrentParams(kCharacterMaster)[5]) {
				getCharacterCurrentParams(kCharacterMaster)[5] = _currentGameSessionTicks + getCharacterCurrentParams(kCharacterMaster)[1];
				if (_currentGameSessionTicks + getCharacterCurrentParams(kCharacterMaster)[1] == 0)
					goto LABEL_19;
			}
			if (getCharacterCurrentParams(kCharacterMaster)[5] < _currentGameSessionTicks) {
				getCharacterCurrentParams(kCharacterMaster)[5] = 0x7FFFFFFF;
			LABEL_19:
				switch (rnd(5)) {
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

				getCharacterCurrentParams(kCharacterMaster)[1] = 225 * (4 * rnd(5) + 20);
				getCharacterCurrentParams(kCharacterMaster)[5] = 0;
			}
		}
		if (_gameTime > 1039500 && !getCharacterCurrentParams(kCharacterMaster)[6]) {
			getCharacterCurrentParams(kCharacterMaster)[6] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;
			MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
			return;
		}
	LABEL_75:
		if (_gameTime > 1147500 && !getCharacterCurrentParams(kCharacterMaster)[7]) {
			getCharacterCurrentParams(kCharacterMaster)[7] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 2;
			MasterCall(&LogicManager::CONS_Master_Arrive, "Epernay", 0, 0, 0);
			return;
		}
	LABEL_78:
		if (_gameTime > 1150200 && !getCharacterCurrentParams(kCharacterMaster)[8]) {
			getCharacterCurrentParams(kCharacterMaster)[8] = 1;
			getCharacterCurrentParams(kCharacterMaster)[3] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 3;
			MasterCall(&LogicManager::CONS_Master_Depart, "Epernay", 0, 0, 0);
			return;
		}
	LABEL_81:
		if (getCharacterCurrentParams(kCharacterMaster)[4] && !getCharacterParams(kCharacterMaster, 8)[1]) {
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 4;
			MasterCall(&LogicManager::CONS_Master_Depart, "Unschedu", 0, 0, 0);
			return;
		}
	LABEL_57:
		if (_gameTime > 1170000 && !getCharacterCurrentParams(kCharacterMaster)[9]) {
			getCharacterCurrentParams(kCharacterMaster)[9] = 1;
			_gameProgress[6] = 2;
		}
		if (!getCharacterCurrentParams(kCharacterMaster)[0]) {
			if (_gameTime > 1170000 && !getCharacterCurrentParams(kCharacterMaster)[10]) {
				getCharacterCurrentParams(kCharacterMaster)[10] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 5;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Chalons", 1, 0, 0);
				return;
			}
		LABEL_84:
			if (_gameTime > 1173600 && !getCharacterCurrentParams(kCharacterMaster)[11]) {
				getCharacterCurrentParams(kCharacterMaster)[11] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 6;
				MasterCall(&LogicManager::CONS_Master_Depart, "Chalons", 0, 0, 0);
				return;
			}
		LABEL_87:
			if (_gameTime > 1228500 && !getCharacterCurrentParams(kCharacterMaster)[12]) {
				getCharacterCurrentParams(kCharacterMaster)[12] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 7;
				MasterCall(&LogicManager::CONS_Master_Arrive, "BarLeDuc", 2, 0, 0);
				return;
			}
		LABEL_90:
			if (_gameTime > 1231200 && !getCharacterCurrentParams(kCharacterMaster)[13]) {
				getCharacterCurrentParams(kCharacterMaster)[13] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 8;
				MasterCall(&LogicManager::CONS_Master_Depart, "BarLeDuc", 0, 0, 0);
				return;
			}
		LABEL_93:
			if (_gameTime > 1260000 && !getCharacterCurrentParams(kCharacterMaster)[14]) {
				getCharacterCurrentParams(kCharacterMaster)[14] = 1;
				_gameTimeTicksDelta = 1;
			}
			if (_gameTime > 1303200 && !getCharacterCurrentParams(kCharacterMaster)[15]) {
				getCharacterCurrentParams(kCharacterMaster)[15] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 9;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Nancy", 3, 0, 0);
				return;
			}
		LABEL_99:
			if (_gameTime > 1307700 && !getCharacterCurrentParams(kCharacterMaster)[16]) {
				getCharacterCurrentParams(kCharacterMaster)[16] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 10;
				MasterCall(&LogicManager::CONS_Master_Depart, "Nancy", 0, 0, 0);
				return;
			}
		LABEL_102:
			if (_gameTime > 1335600 && !getCharacterCurrentParams(kCharacterMaster)[17]) {
				getCharacterCurrentParams(kCharacterMaster)[17] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 11;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Luneville", 4, 0, 0);
				return;
			}
		LABEL_105:
			if (_gameTime > 1338300 && !getCharacterCurrentParams(kCharacterMaster)[18]) {
				getCharacterCurrentParams(kCharacterMaster)[18] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 12;
				MasterCall(&LogicManager::CONS_Master_Depart, "Luneville", 0, 0, 0);
				return;
			}
		LABEL_108:
			if (_gameTime > 1359900 && !getCharacterCurrentParams(kCharacterMaster)[19]) {
				getCharacterCurrentParams(kCharacterMaster)[19] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 13;
				MasterCall(&LogicManager::CONS_Master_Arrive, "Avricourt", 5, 0, 0);
				return;
			}
		LABEL_111:
			if (_gameTime > 1363500 && !getCharacterCurrentParams(kCharacterMaster)[20]) {
				getCharacterCurrentParams(kCharacterMaster)[20] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 14;
				MasterCall(&LogicManager::CONS_Master_Depart, "Avricourt", 0, 0, 0);
				return;
			}
		LABEL_114:
			if (_gameTime > 1367100 && !getCharacterCurrentParams(kCharacterMaster)[21]) {
				getCharacterCurrentParams(kCharacterMaster)[21] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 15;
				MasterCall(&LogicManager::CONS_Master_Arrive, "DeutschA", 6, 0, 0);
				return;
			}
		LABEL_117:
			if (_gameTime > 1370700 && !getCharacterCurrentParams(kCharacterMaster)[22]) {
				getCharacterCurrentParams(kCharacterMaster)[22] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 16;
				MasterCall(&LogicManager::CONS_Master_Depart, "DeutschA", 0, 0, 0);
				return;
			}
		LABEL_65:
			if (_gameTime > 1490400 && !getCharacterCurrentParams(kCharacterMaster)[23]) {
				getCharacterCurrentParams(kCharacterMaster)[23] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 17;
				MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
				return;
			}
		}
	LABEL_120:
		if (_gameTime <= 1493100 || getCharacterCurrentParams(kCharacterMaster)[24]) {
		LABEL_70:
			if (_gameTime <= 1539000 || getCharacterCurrentParams(kCharacterMaster)[25]) {
			LABEL_123:
				if (_gameTime <= 1541700 || getCharacterCurrentParams(kCharacterMaster)[26]) {
				LABEL_126:
					if (_gameTime > 1674000 && !getCharacterCurrentParams(kCharacterMaster)[27]) {
						getCharacterCurrentParams(kCharacterMaster)[27] = 1;
						CONS_Master_SecondSleep(0, 0, 0, 0);
					}
				} else {
					getCharacterCurrentParams(kCharacterMaster)[26] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 21;
					MasterCall(&LogicManager::CONS_Master_Depart, "BadenOos", 0, 0, 0);
				}
			} else {
				getCharacterCurrentParams(kCharacterMaster)[25] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 20;
				MasterCall(&LogicManager::CONS_Master_Arrive, "BadenOos", 8, 0, 0);
			}
		} else {
			getCharacterCurrentParams(kCharacterMaster)[24] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 19;
			MasterCall(&LogicManager::CONS_Master_Depart, "Strasbou", 0, 0, 0);
		}

		return;
	}

	if (msg->action == 2) {
		if (getCharacterParams(kCharacterMaster, 8)[1]) {
			send(kCharacterMaster, 32, 191350523, 0);
			if (getCharacter(kCharacterCath).characterPosition.location == 2) {
				if (cathOutHisWindow()) {
					v14 = 49;
				LABEL_43:
					bumpCath(3, v14, 255);
					goto LABEL_44;
				}
				if (cathOutRebeccaWindow()) {
					bumpCath(4, 49, 255);
				} else {
					if (getCharacter(kCharacterCath).characterPosition.car < 4u || getCharacter(kCharacterCath).characterPosition.car > 7) {
						if (!getCharacter(kCharacterCath).characterPosition.car || getCharacter(kCharacterCath).characterPosition.car > 3)
							goto LABEL_44;
						if (checkCathDir(3, 98)) {
							playDialog(kCharacterCath, "LIB015", -1, 0);
							v14 = 71;
						} else {
							v14 = 82;
						}
						goto LABEL_43;
					}
					bumpCath(5, 82, 255);
				}
			}
		LABEL_44:
			_engine->getSoundManager()->startAmbient();
			_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			return;
		}

		if (getCharacterParams(kCharacterMaster, 8)[2]) {
			_engine->getSoundManager()->startAmbient();
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
			if (getCharacterCurrentParams(kCharacterMaster)[3]) {
				if (_gameProgress[9]) {
					send(kCharacterMaster, 9, 168187490, 0);
				} else {
					send(kCharacterMaster, 3, 224122407, 0);
				}
				getCharacterCurrentParams(kCharacterMaster)[3] = 0;
			}
		}
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
	if (msg->action == 12) {
		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);
			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}

		playDialog(kCharacterCath, "MUS008", 16, 0);

		_inventorySelectedItemIdx = 0;
		fadeToBlack();

		while (dialogRunning("MUS008")) {
			_engine->getSoundManager()->soundThread();
		}

		CONS_Master_StartPart2(0, 0, 0, 0);
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
	if (msg->action == 12) {
		if (_gameProgress[kProgressChapter] != 2) {
			_gameProgress[kProgressChapter] = 2;
			makeAllJump(2);
		}

		_gameTime = 1750500;
		_gameTimeTicksDelta = 5;

		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;

		MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
	} else if (msg->action == 18 && getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 1) {
		if (_engine->getArchiveManager()->lockCD(2)) {
			loadTrain(2);
			CONS_Master_InitPart2(0, 0, 0, 0);
		} else {
			_engine->getMenu()->doEgg(0, 0, 0);
		}
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
	if (msg->action == 12) {
		_gameProgress[20] = 1;
		setDoor(26, 0, 1, 0, 9);
		setDoor(27, 0, 1, 0, 9);
		_gameProgress[6] = 1;
		giveCathItem(21);

		_gameInventory[20].location = 0;
		_gameInventory[20].isPresent = 0;
		_gameProgress[1] = 2;
		_gameProgress[7] = 34;
		_gameProgress[2] = 1;
		_gameProgress[4] = 1;
		dropItem(14, 3);
		dropItem(3, 1);

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

		setDoor(1, 0, 0, 10, 9);
		setDoor(9, 0, 0, 255, 255);

		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);
			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}

		playNIS(kEventTrainPassing);

		if (cathHasItem(8)) {
			bumpCathTylerComp();
		} else {
			bumpCath(3, 79, 255);
		}

		CONS_Master_MovingDay(0, 0, 0, 0);
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
	if (msg->action) {
		if (msg->action == 12) {
			getCharacterCurrentParams(kCharacterMaster)[0] = 225 * (4 * rnd(5) + 20);
		} else if (msg->action == 139122728) {
			CONS_Master_StartPart3(0, 0, 0, 0);
		}
	} else {
		if (!_gameProgress[20])
			return;

		int v3; 
		if (getCharacterCurrentParams(kCharacterMaster)[1] ||
			(v3 = _currentGameSessionTicks + getCharacterCurrentParams(kCharacterMaster)[0] == 0,
				getCharacterCurrentParams(kCharacterMaster)[1] = _currentGameSessionTicks + getCharacterCurrentParams(kCharacterMaster)[0],
				!v3)) {
			if (getCharacterCurrentParams(kCharacterMaster)[1] >= _currentGameSessionTicks)
				return;

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
}

void LogicManager::CONS_Master_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterMaster).callParams[getCharacter(kCharacterMaster).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterMaster, &LogicManager::HAND_Master_StartPart3);
	getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall] = 13;

	params->clear();

	fedEx(kCharacterMaster, kCharacterMaster, 12, 0);
}

void LogicManager::HAND_Master_StartPart3(HAND_PARAMS) {
	if (msg->action == 12) {
		if (_gameProgress[kProgressChapter] != 3) {
			_gameProgress[kProgressChapter] = 3;
			makeAllJump(3);
		}

		_gameTime = 1944000;
		_gameTimeTicksDelta = 5;

		CONS_Master_InitPart3(0, 0, 0, 0);
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
	if (msg->action == 12) {
		send(kCharacterMaster, kCharacterTableA, 103798704, 0);
		send(kCharacterMaster, kCharacterTableB, 103798704, 0);
		send(kCharacterMaster, kCharacterTableC, 103798704, 0);
		send(kCharacterMaster, kCharacterTableD, 103798704, 0);
		send(kCharacterMaster, kCharacterTableE, 103798704, 0);
		send(kCharacterMaster, kCharacterTableF, 103798704, 0);

		_gameProgress[20] = 1;

		setDoor(26, 0, 1, 0, 9);
		setDoor(27, 0, 1, 0, 9);
		dropItem(19, 1);
		dropItem(3, 1);
		setModel(1, 2);
		setDoor(107, 0, 3, 255, 255);

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
	} else if (msg->action == 18 && getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 1) {
		CONS_Master_MovingAfternoon(0, 0, 0, 0);
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
	bool v4;                        // zf
	int v5;                         // eax
	unsigned int v6;                // eax
	uint16 car;           // ax
	int v8;                         // [esp-8h] [ebp-14h]
	int v9;                         // [esp-8h] [ebp-14h]
	int v10;                        // [esp-8h] [ebp-14h]
	int v11;                        // [esp-8h] [ebp-14h]
	int v12;                        // [esp-8h] [ebp-14h]
	int v13;                        // [esp-8h] [ebp-14h]
	int v14;                        // [esp-8h] [ebp-14h]
	int v15;                        // [esp-8h] [ebp-14h]

	switch (msg->action) {
	case 0:
		if (!_gameProgress[20])
			goto LABEL_14;
		if (!getCharacterCurrentParams(kCharacterMaster)[3]) {
			v4 = _currentGameSessionTicks + getCharacterCurrentParams(kCharacterMaster)[0] == 0;
			getCharacterCurrentParams(kCharacterMaster)[3] = _currentGameSessionTicks + getCharacterCurrentParams(kCharacterMaster)[0];
			if (v4)
				goto LABEL_7;
		}
		if (getCharacterCurrentParams(kCharacterMaster)[3] < _currentGameSessionTicks) {
			getCharacterCurrentParams(kCharacterMaster)[3] = 0x7FFFFFFF;
		LABEL_7:
			switch (rnd(5)) {
			case 0:
				v8 = rnd(15) + 2;
				playDialog(kCharacterCath, "ZFX1005", v8, 0);
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
			getCharacterCurrentParams(kCharacterMaster)[3] = 0;
		}
	LABEL_14:
		if (!getCharacterCurrentParams(kCharacterMaster)[4]) {
			v5 = getCharacterCurrentParams(kCharacterMaster)[1];
			v4 = _currentGameSessionTicks + v5 == 0;
			getCharacterCurrentParams(kCharacterMaster)[4] = _currentGameSessionTicks + v5;
			if (v4)
				goto LABEL_18;
		}
		if (getCharacterCurrentParams(kCharacterMaster)[4] < _currentGameSessionTicks) {
			getCharacterCurrentParams(kCharacterMaster)[4] = 0x7FFFFFFF;
		LABEL_18:
			v6 = rnd(2);
			if (v6) {
				if (v6 == 1) {
					v14 = rnd(0xF) + 2;
					playDialog(kCharacterCath, "ZFX1009", v14, 0);
				}
			} else {
				v13 = rnd(0xF) + 2;
				playDialog(kCharacterCath, "ZFX1008", v13, 0);
			}
			getCharacterCurrentParams(kCharacterMaster)[1] = 225 * (4 * rnd(6) + 8);
			getCharacterCurrentParams(kCharacterMaster)[4] = 0;
		}
		if (_gameTime <= 1982700 || getCharacterCurrentParams(kCharacterMaster)[5]) {
		LABEL_49:
			if (_gameTime <= 1989900 || getCharacterCurrentParams(kCharacterMaster)[6]) {
			LABEL_52:
				if (_gameTime <= 2047500 || getCharacterCurrentParams(kCharacterMaster)[7]) {
				LABEL_55:
					if (_gameTime <= 2052900 || getCharacterCurrentParams(kCharacterMaster)[8]) {
					LABEL_58:
						if (_gameTime <= 2073600 || getCharacterCurrentParams(kCharacterMaster)[9]) {
						LABEL_61:
							if (_gameTime <= 2079900 || getCharacterCurrentParams(kCharacterMaster)[10]) {
							LABEL_64:
								if (_gameTime <= 2099700 || getCharacterCurrentParams(kCharacterMaster)[11]) {
								LABEL_67:
									if (_gameTime <= 2105100 || getCharacterCurrentParams(kCharacterMaster)[12]) {
									LABEL_70:
										if (_gameTime > 2187000 && !getCharacterCurrentParams(kCharacterMaster)[13]) {
											getCharacterCurrentParams(kCharacterMaster)[13] = 1;
											_gameTimeTicksDelta = 5;
										}
										if (_gameTime > 2268000 && !getCharacterCurrentParams(kCharacterMaster)[14]) {
											getCharacterCurrentParams(kCharacterMaster)[14] = 1;
											getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 9;
											MasterCall(&LogicManager::CONS_Master_Arrive, "Vienna", 13, 0, 0);
										}
									} else {
										getCharacterCurrentParams(kCharacterMaster)[12] = 1;
										getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 8;
										MasterCall(&LogicManager::CONS_Master_Depart, "Linz", 0, 0, 0);
									}
								} else {
									getCharacterCurrentParams(kCharacterMaster)[11] = 1;
									getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 7;
									MasterCall(&LogicManager::CONS_Master_Arrive, "Linz", 12, 0, 0);
								}
							} else {
								getCharacterCurrentParams(kCharacterMaster)[10] = 1;
								getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 6;
								MasterCall(&LogicManager::CONS_Master_Depart, "Wels", 0, 0, 0);
							}
						} else {
							getCharacterCurrentParams(kCharacterMaster)[9] = 1;
							getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 5;
							MasterCall(&LogicManager::CONS_Master_Arrive, "Wels", 11, 0, 0);
						}
					} else {
						getCharacterCurrentParams(kCharacterMaster)[8] = 1;
						getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 4;
						MasterCall(&LogicManager::CONS_Master_Depart, "Attnang", 0, 0, 0);
					}
				} else {
					getCharacterCurrentParams(kCharacterMaster)[7] = 1;
					getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 3;
					MasterCall(&LogicManager::CONS_Master_Arrive, "Attnang", 10, 0, 0);
				}
			} else {
				getCharacterCurrentParams(kCharacterMaster)[6] = 1;
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 2;
				MasterCall(&LogicManager::CONS_Master_Depart, "Salzburg", 0, 0, 0);
			}
		} else {
			getCharacterCurrentParams(kCharacterMaster)[5] = 1;
			getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;
			MasterCall(&LogicManager::CONS_Master_Arrive, "Salzburg", 9, 0, 0);
		}
		return;
	case 2:
		if (getCharacterParams(kCharacterMaster, 8)[1]) {
			send(31, 32, 191350523, 0);
			if (getCharacter(kCharacterCath).characterPosition.location != 2)
				goto LABEL_42;
			if (cathOutHisWindow()) {
				v15 = 49;
			} else {
				if (cathOutRebeccaWindow()) {
					bumpCath(4, 49, 255);
					goto LABEL_42;
				}
				car = getCharacter(kCharacterCath).characterPosition.car;
				if (car >= 4 && car <= 7) {
					bumpCath(5, 82, 255);
					goto LABEL_42;
				}
				if (!car || car > 3) {
				LABEL_42:
					_engine->getSoundManager()->startAmbient();
					_engine->getSoundManager()->startSteam(getCharacterParams(kCharacterMaster, 8)[3]);
					getCharacterParams(kCharacterMaster, 8)[1] = 0;
					if (getCharacterCurrentParams(kCharacterMaster)[2])
						CONS_Master_EndVienna(0, 0, 0, 0);
					return;
				}
				if (checkCathDir(3, 98)) {
					playDialog(kCharacterCath, "LIB015", -1, 0);
					v15 = 71;
				} else {
					v15 = 82;
				}
			}
			bumpCath(3, v15, 255);
			goto LABEL_42;
		}
		if (getCharacterParams(kCharacterMaster, 8)[2]) {
			_engine->getSoundManager()->startAmbient();
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}
		return;
	case 12:
		getCharacterCurrentParams(kCharacterMaster)[0] = 225 * (4 * rnd(5) + 20);
		getCharacterCurrentParams(kCharacterMaster)[1] = 225 * (4 * rnd(6) + 8);
		return;
	case 18:
		switch (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8]) {
		case 1:
			goto LABEL_49;
		case 2:
			goto LABEL_52;
		case 3:
			goto LABEL_55;
		case 4:
			goto LABEL_58;
		case 5:
			goto LABEL_61;
		case 6:
			goto LABEL_64;
		case 7:
			goto LABEL_67;
		case 8:
			goto LABEL_70;
		case 9:
			getCharacterCurrentParams(kCharacterMaster)[2] = 1;
			break;
		default:
			return;
		}
		return;
	default:
		return;
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
	Character *v1;                  // ecx
	uint8 *p_currentCall; // edx
	int action;                     // eax
	int v4;                         // eax
	int v5;                         // [esp-8h] [ebp-Ch]

	v1 = &getCharacter(kCharacterMaster);
	p_currentCall = &getCharacter(kCharacterMaster).currentCall;
	action = msg->action;
	if (action == 12) {
		getCharacter(kCharacterCath).characterPosition.car = 8;
		if (_gameEvents[30]) {
			if (_gameInventory[18].location == 5) {
				getCharacter(kCharacterMaster).callbacks[*p_currentCall + 8] = 2;
				MasterCall(&LogicManager::CONS_Master_SaveGame, 2, 165, 0, 0);
			} else if (getCharacterParams(kCharacterMaster, 8)[0]) {
				getCharacter(kCharacterMaster).callbacks[*p_currentCall + 8] = 3;
				MasterCall(&LogicManager::CONS_Master_SaveGame, 2, 163, 0, 0);
			} else {
				getCharacter(kCharacterMaster).callbacks[*p_currentCall + 8] = 4;
				MasterCall(&LogicManager::CONS_Master_SaveGame, 2, 166, 0, 0);
			}
		} else {
			getCharacter(kCharacterMaster).callbacks[*p_currentCall + 8] = 1;
			MasterCall(&LogicManager::CONS_Master_SaveGame, 2, 164, 0, 0);
		}
	} else if (action == 18) {
		switch (getCharacter(kCharacterMaster).callbacks[*p_currentCall + 8]) {
		case 1:
			playNIS(164);
			if (_gameEvents[215]) {
				endGame(1, 2187000, 0, 1);
			} else {
				v5 = 0;
				if (!_gameEvents[253])
					goto LABEL_25;
				endGame(2, 253, 0, 1);
			}
			break;
		case 2:
			playNIS(165);
			if (_gameEvents[58]) {
				endGame(3, 58, 60, 1);
			} else if (_gameEvents[59]) {
				endGame(1, 2155500, 60, 1);
			} else {
				v4 = _gameProgress[48];
				if (v4) {
					v5 = 61;
					if (_gameEvents[61]) {
						endGame(1, v4, 61, 1);
						return;
					}
				} else {
					v5 = 59;
					if (_gameEvents[61]) {
						endGame(2, 61, 59, 1);
						return;
					}
				}
			LABEL_25:
				endGame(1, 2155500, v5, 1);
			}
			break;
		case 3:
			playNIS(163);
			endGame(1, 2250000, 58, 1);
			break;
		case 4:
			playNIS(166);
			CONS_Master_StartPart4(0, 0, 0, 0);
			break;
		default:
			return;
		}
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
	if (msg->action == 12) {
		if (_gameProgress[kProgressChapter] != 4) {
			_gameProgress[kProgressChapter] = 4;
			makeAllJump(4);
		}

		_gameTime = 2353500;
		_gameTimeTicksDelta = 5;

		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;

		MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
	} else if (msg->action == 18 && getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 1) {
		if (_engine->getArchiveManager()->lockCD(3)) {
			loadTrain(3);
			CONS_Master_InitPart4(0, 0, 0, 0);
		} else {
			_engine->getMenu()->doEgg(0, 0, 0);
		}
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
	if (msg->action == 12) {
		_engine->getSoundManager()->endAmbient();
		_engine->getSoundManager()->startAmbient();

		_gameProgress[kProgressIsTrainRunning] = 1;

		setDoor(26, 0, 1, 0, 9);
		setDoor(27, 0, 1, 0, 9);

		send(kCharacterMaster, kCharacterClerk, 203419131, 0);
		send(kCharacterMaster, kCharacterTableA, 103798704, 0);
		send(kCharacterMaster, kCharacterTableB, 103798704, 0);
		send(kCharacterMaster, kCharacterTableC, 103798704, 0);
		send(kCharacterMaster, kCharacterTableD, 103798704, 0);
		send(kCharacterMaster, kCharacterTableE, 103798704, 0);
		send(kCharacterMaster, kCharacterTableF, 103798704, 0);

		takeItem(3);
		dropItem(16, 1);

		if (_gameInventory[14].location == 3)
			takeItem(14);

		setModel(25, 2);
		setDoor(107, 0, 3, 255, 255);

		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(kCharacterMaster);

			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}

		fadeToBlack();

		if (cathHasItem(18)) {
			bumpCath(kCarGreenSleeping, 76, 255);
		} else {
			bumpCath(kCarRestaurant, 69, 255);
		}

		restoreIcons();
		CONS_Master_MovingSecondNight(0, 0, 0, 0);
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
	uint8 *p_currentCall; // ebx
	Character *v2;                  // esi
	int currentCall;                // ecx
	CharacterCallParams *v4;        // edi
	unsigned int action;            // eax
	int v6;                         // eax
	bool v7;                        // zf
	int v8;                         // eax
	unsigned int v9;                // eax
	uint16 car;                     // ax
	int v11;                        // [esp-8h] [ebp-14h]
	int v12;                        // [esp-8h] [ebp-14h]
	int v13;                        // [esp-8h] [ebp-14h]
	int v14;                        // [esp-8h] [ebp-14h]
	int v15;                        // [esp-8h] [ebp-14h]
	int v16;                        // [esp-8h] [ebp-14h]
	int v17;                        // [esp-8h] [ebp-14h]
	int v18;                        // [esp-8h] [ebp-14h]

	p_currentCall = &getCharacter(kCharacterMaster).currentCall;
	v2 = &getCharacter(kCharacterMaster);
	currentCall = getCharacter(kCharacterMaster).currentCall;
	v4 = &getCharacter(kCharacterMaster).callParams[currentCall];
	action = msg->action;
	if (action > 12) {
		if (action > 139254416) {
			if (action > 158610240) {
				if (action > 190346110) {
					switch (action) {
					case 191001984:
						_gameTime = 2520000;
						if (whoRunningDialog(31))
							endDialog(31);
						endGraphics(31);
						takeCathItem(4);
						_gameTimeTicksDelta = 5;
						v2->callbacks[*p_currentCall + 8] = 10;
						MasterCall(&LogicManager::CONS_Master_SaveGame, 2, 0x74, 0, 0);
						break;
					case 201959744:
						if (whoRunningDialog(31))
							endDialog(31);
						playDialog(32, "ZFX4001", 16, 0);
						endGame(0, 0, 0, 1);
						break;
					case 225367984:
						v2->callbacks[currentCall + 8] = 9;
						MasterCall(&LogicManager::CONS_Master_SaveGame, 2, 0x108, 0, 0);
						break;
					}
				} else if (action == 190346110) {
					_gameProgress[6] = 3;
					v4->parameters[0] = 1;
					if (v2->callParams[8].parameters[1] || v2->callParams[8].parameters[2]) {
						endDialog(31);
						v2->callParams[8].parameters[1] = 0;
						v2->callParams[8].parameters[2] = 0;
					}
					fadeToBlack();
					playDialog(kCharacterCath, "MUS008", 16, 0);
					_inventorySelectedItemIdx = 0;
					fadeToBlack();
					while (dialogRunning("MUS008"))
						_engine->getSoundManager()->soundThread();
					if (cathHasItem(16)) {
						forceJump(12, &LogicManager::CONS_Alexei_Dead);
						forceJump(1, &LogicManager::CONS_Anna_Sulking);
						forceJump(2, &LogicManager::CONS_August_Asleep4);
						forceJump(3, &LogicManager::CONS_Cond1_OnDuty4);
						forceJump(4, &LogicManager::CONS_Cond2_OnDuty4);
						forceJump(6, &LogicManager::CONS_Waiter1_Serving4);
						forceJump(7, &LogicManager::CONS_Waiter2_Serving4);
						forceJump(5, &LogicManager::CONS_HeadWait_InPart4);
						forceJump(9, &LogicManager::CONS_TrainM_InOffice4);
						forceJump(10, &LogicManager::CONS_Tatiana_Asleep4);
						forceJump(13, &LogicManager::CONS_Abbot_Asleep4);
						forceJump(14, &LogicManager::CONS_Milos_EndPart4);
						forceJump(15, &LogicManager::CONS_Vesna_EndPart4);
						forceJump(16, &LogicManager::CONS_Ivo_EndPart4);
						forceJump(17, &LogicManager::CONS_Salko_EndPart4);
						forceJump(21, &LogicManager::CONS_Madame_Asleep4);
						forceJump(22, &LogicManager::CONS_Monsieur_Asleep4);
						forceJump(23, &LogicManager::CONS_Rebecca_Asleep4);
						forceJump(24, &LogicManager::CONS_Sophie_Asleep4);
						forceJump(26, &LogicManager::CONS_Yasmin_Asleep4);
						forceJump(27, &LogicManager::CONS_Hadija_Asleep4);
						forceJump(28, &LogicManager::CONS_Alouan_Asleep4);
						forceJump(30, &LogicManager::CONS_Max_InCageFriendly);
						send(31, 1, 201431954, 0);
						send(31, 3, 201431954, 0);
						send(31, 4, 201431954, 0);
						send(31, 6, 201431954, 0);
						send(31, 7, 201431954, 0);
						send(31, 5, 201431954, 0);
						send(31, 9, 201431954, 0);
						v2->callbacks[*p_currentCall + 8] = 6;
						MasterCall(&LogicManager::CONS_Master_SaveGame, 2, 93, 0, 0);
					} else {
						_gameTime = 2520000;
						v2->callbacks[*p_currentCall + 8] = 7;
						MasterCall(&LogicManager::CONS_Master_SaveGame, 2, 115, 0, 0);
					}
				} else if (action == 169300225) {
					if (_gameTime < 2519100)
						_gameTime = 2519100;
					v4->parameters[2] = 1;
					startSeqOtis(31, "BOMB");
				}
			} else if (action == 158610240) {
				v2->callbacks[currentCall + 8] = 8;
				MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
			} else if (action == 156435676) {
				send(31, 10, 169360385, 0);
				send(31, 4, 201431954, 0);
				send(31, 9, 201431954, 0);
				_gameTimeTicksDelta = 1;
				_gameTime = 2511900;
				dropItem(2, 1);
				v2->characterPosition.car = 4;
				v2->characterPosition.position = 1500;
				v2->characterPosition.location = 1;
				playDialog(31, "ZFX1001", -1, 0);
			}
		} else if (action == 139254416) {
			CONS_Master_StartPart5(0, 0, 0, 0);
		} else if (action == 18) {
			switch (v2->callbacks[currentCall + 8]) {
			case 1:
				goto LABEL_82;
			case 2:
				goto LABEL_85;
			case 3:
				goto LABEL_93;
			case 4:
				goto LABEL_96;
			case 5:
				if (whoRunningDialog(31))
					endDialog(31);
				playNIS(115);
				endGame(0, 1, 0, 1);
				break;
			case 6:
				_engine->getSoundManager()->endAmbient();
				playNIS(93);
				_engine->getSoundManager()->startAmbient();
				_gameProgress[6] = 1;
				bumpCathTylerComp();
				send(31, 10, 169360385, 0);
				_gameTimeTicksDelta = 1;
				_gameTime = 2511900;
				dropItem(2, 1);
				takeItem(22);
				v2->characterPosition.car = 4;
				v2->characterPosition.position = 1500;
				v2->characterPosition.location = 1;
				playDialog(31, "ZFX1001", -1, 0);
				break;
			case 7:
				playNIS(115);
				endGame(1, 2430000, 0, 1);
				break;
			case 8:
				playDialog(kCharacterCath, "MUS022", -1, 0);
				if (_gameTime < 2517300)
					_gameTime = 2517300;
				break;
			case 9:
				playNIS(264);
				bumpCath(4, 73, 255);
				break;
			case 10:
				playNIS(116);
				forceJump(13, &LogicManager::CONS_Abbot_AfterBomb);
				send(31, 1, 191001984, 0);
				send(31, 4, 191001984, 0);
				takeItem(2);
				_gameInventory[2].location = 0;
				v4->parameters[1] = 1;
				bumpCath(4, 2, 255);
				break;
			case 11:
				bumpCath(4, 74, 255);
				playDialog(32, "ZFX4001", 16, 0);
				endGame(0, 1, 0, 1);
				break;
			default:
				return;
			}
		}
		return;
	}
	if (action == 12) {
		v4->parameters[3] = 225 * (4 * rnd(5) + 20);
		v4->parameters[4] = 225 * (4 * rnd(6) + 8);
		return;
	}
	if (!action) {
		if (_gameProgress[20]) {
			if (v4->parameters[5] || (v6 = v4->parameters[3], v7 = _currentGameSessionTicks + v6 == 0, v4->parameters[5] = _currentGameSessionTicks + v6, !v7)) {
				if (v4->parameters[5] >= _currentGameSessionTicks)
					goto LABEL_35;
				v4->parameters[5] = 0x7FFFFFFF;
			}
			switch (rnd(5)) {
			case 0:
				v11 = rnd(15) + 2;
				playDialog(kCharacterCath, "ZFX1005", v11, 0);
				break;
			case 1:
				v12 = rnd(15) + 2;
				playDialog(kCharacterCath, "ZFX1006", v12, 0);
				break;
			case 2:
				v13 = rnd(15) + 2;
				playDialog(kCharacterCath, "ZFX1007", v13, 0);
				break;
			case 3:
				v14 = rnd(15) + 2;
				playDialog(kCharacterCath, "ZFX1007A", v14, 0);
				break;
			case 4:
				v15 = rnd(15) + 2;
				playDialog(kCharacterCath, "ZFX1007B", v15, 0);
				break;
			default:
				break;
			}
			v4->parameters[3] = 225 * (4 * rnd(5) + 20);
			v4->parameters[5] = 0;
		}
	LABEL_35:
		if (v4->parameters[6] || (v8 = v4->parameters[4], v7 = _currentGameSessionTicks + v8 == 0, v4->parameters[6] = _currentGameSessionTicks + v8, !v7)) {
			if (v4->parameters[6] >= _currentGameSessionTicks) {
			LABEL_45:
				if (_gameTime > 2381400 && !v4->parameters[7]) {
					v4->parameters[7] = 1;
					v2->callbacks[*p_currentCall + 8] = 1;
					MasterCall(&LogicManager::CONS_Master_Arrive, "Pozsony", 0xE, 0, 0);
					return;
				}
			LABEL_82:
				if (_gameTime > 2386800 && !v4->parameters[8]) {
					v4->parameters[8] = 1;
					v2->callbacks[*p_currentCall + 8] = 2;
					MasterCall(&LogicManager::CONS_Master_Depart, "Pozsony", 0, 0, 0);
					return;
				}
			LABEL_85:
				if (getModel(1) == 1 && _gameTime > 2403000 && !v4->parameters[9]) {
					v4->parameters[9] = 1;
					_gameProgress[6] = 2;
				}
				if (v4->parameters[0])
					goto LABEL_96;
				if (_gameTime > 2416500 && !v4->parameters[10]) {
					v4->parameters[10] = 1;
					v2->callbacks[*p_currentCall + 8] = 3;
					MasterCall(&LogicManager::CONS_Master_Arrive, "Galanta", 0xF, 0, 0);
					return;
				}
			LABEL_93:
				if (_gameTime <= 2421900 || v4->parameters[11]) {
				LABEL_96:
					if (_gameTime > 2470500 && !v4->parameters[12]) {
						v4->parameters[12] = 1;
						if (_gameProgress[6] == 2)
							_gameTimeTicksDelta = 1;
					}
					if (_gameTime > 2506500 && !v4->parameters[13]) {
						v4->parameters[13] = 1;
						if (_gameProgress[6] == 2)
							_gameProgress[6] = 1;
					}
					if (_gameTime > 2520000 && !v4->parameters[14]) {
						v4->parameters[14] = 1;
						if (!v4->parameters[1] && !v4->parameters[2]) {
							v2->callbacks[*p_currentCall + 8] = 5;
							MasterCall(&LogicManager::CONS_Master_SaveGame, 2, 0x73, 0, 0);
						}
					}
				} else {
					v4->parameters[11] = 1;
					v2->callbacks[*p_currentCall + 8] = 4;
					MasterCall(&LogicManager::CONS_Master_Depart, "Galanta", 0, 0, 0);
				}
				return;
			}
			v4->parameters[6] = 0x7FFFFFFF;
		}
		v9 = rnd(2);
		if (v9) {
			if (v9 == 1) {
				v17 = rnd(15) + 2;
				playDialog(kCharacterCath, "ZFX1009", v17, 0);
			}
		} else {
			v16 = rnd(15) + 2;
			playDialog(kCharacterCath, "ZFX1008", v16, 0);
		}
		v4->parameters[4] = 225 * (4 * rnd(6) + 8);
		v4->parameters[6] = 0;
		goto LABEL_45;
	}
	if (action != 2) {
		if (action == 3) {
			endGraphics(31);
			v2->callbacks[*p_currentCall + 8] = 11;
			MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
		}
		return;
	}
	if (getCharacterParams(kCharacterMaster, 8)[1]) {
		send(31, 32, 191350523, 0);
		if (getCharacter(kCharacterCath).characterPosition.location == 2) {
			if (cathOutHisWindow()) {
				v18 = 49;
			LABEL_62:
				bumpCath(3, v18, 255);
				goto LABEL_63;
			}
			if (cathOutRebeccaWindow()) {
				bumpCath(4, 49, 255);
			} else {
				car = getCharacter(kCharacterCath).characterPosition.car;
				if (car < 4 || car > 7) {
					if (!car || car > 3)
						goto LABEL_63;
					if (checkCathDir(3, 98)) {
						playDialog(kCharacterCath, "LIB015", -1, 0);
						v18 = 71;
					} else {
						v18 = 82;
					}
					goto LABEL_62;
				}
				bumpCath(5, 82, 255);
			}
		}
	LABEL_63:
		_engine->getSoundManager()->startAmbient();
		_engine->getSoundManager()->startSteam(v2->callParams[8].parameters[3]);
		v2->callParams[8].parameters[1] = 0;
		return;
	}
	if (getCharacterParams(kCharacterMaster, 8)[2]) {
		_engine->getSoundManager()->startAmbient();
		v2->callParams[8].parameters[2] = 0;
	} else if (!v4->parameters[1] && !v4->parameters[2]) {
		playDialog(31, "ZFX1001", -1, 0);
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
	if (msg->action == 12) {
		if (_gameProgress[kProgressChapter] != 5) {
			_gameProgress[kProgressChapter] = 5;
			makeAllJump(5);
		}

		_gameTime = 2844000;
		_gameTimeTicksDelta = 2;

		CONS_Master_InitPart5(0, 0, 0, 0);
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
	if (msg->action == 12) {
		endGraphics(33);
		endGraphics(34);
		endGraphics(35);
		endGraphics(36);
		endGraphics(37);
		endGraphics(38);

		_gameProgress[20] = 1;

		setDoor(26, 0, 1, 0, 9);
		setDoor(27, 0, 1, 0, 9);
		setDoor(1, 0, 0, 10, 9);
		setDoor(2, 0, 0, 10, 9);
		setDoor(3, 0, 0, 10, 9);
		setDoor(4, 0, 3, 10, 9);
		setDoor(5, 0, 3, 10, 9);
		setDoor(6, 0, 3, 10, 9);
		setDoor(7, 0, 3, 10, 9);
		setDoor(8, 0, 3, 10, 9);
		setDoor(32, 0, 0, 10, 9);
		setDoor(33, 0, 0, 10, 9);
		setDoor(34, 0, 0, 10, 9);
		setDoor(35, 0, 0, 10, 9);
		setDoor(36, 0, 0, 10, 9);
		setDoor(37, 0, 0, 10, 9);
		setDoor(38, 0, 3, 10, 9);
		setDoor(39, 0, 3, 10, 9);
		setDoor(17, 0, 0, 10, 9);
		setDoor(18, 0, 0, 10, 9);
		setDoor(19, 0, 0, 10, 9);
		setDoor(20, 0, 3, 10, 9);
		setDoor(21, 0, 3, 10, 9);
		setDoor(22, 0, 3, 10, 9);
		setDoor(48, 0, 0, 10, 9);
		setDoor(49, 0, 0, 10, 9);
		setDoor(50, 0, 0, 10, 9);
		setDoor(51, 0, 0, 10, 9);
		setDoor(52, 0, 0, 10, 9);
		setDoor(53, 0, 0, 10, 9);
		setDoor(26, 0, 0, 10, 9);
		setDoor(27, 0, 0, 10, 9);

		_gameProgress[6] = 1;
		_inventorySelectedItemIdx = 0;
		_gameProgress[33] = 1;
		_gameProgress[7] = 36;

		takeCathItem(15);
		takeCathItem(16);
		takeCathItem(12);

		if (cathHasItem(18)) {
			takeCathItem(18);
			dropItem(18, 3);

			if (cathHasItem(13)) {
				takeCathItem(13);
				dropItem(13, 3);
			}
		}

		setDoor(93, 0, 2, 255, 255);
		setDoor(94, 0, 0, 255, 255);
		setDoor(101, 0, 0, 10, 9);
		setModel(98, 2);
		setModel(55, 2);

		if (getCharacterParams(kCharacterMaster, 8)[1] || getCharacterParams(kCharacterMaster, 8)[2]) {
			endDialog(31);
			getCharacterParams(kCharacterMaster, 8)[1] = 0;
			getCharacterParams(kCharacterMaster, 8)[2] = 0;
		}

		fadeToBlack();
		bumpCath(1, 95, 255);
		restoreIcons();

		getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;

		MasterCall(&LogicManager::CONS_Master_SaveGame, 1, 0, 0, 0);
	} else if (msg->action == 18 && getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 1) {
		CONS_Master_MovingPart5(0, 0, 0, 0);
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
	if (msg->action > 12) {
		if (msg->action == 18) {
			if (getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] == 1) {
				playNIS(46);
				endGame(1, 2898000, 51, 1);
			}
		} else if (msg->action == 135800432) {
			_gameProgress[14] = 1;
			_gameTime = 2916000;
			if (whoRunningDialog(31))
				endDialog(31);
		}
	} else if (msg->action == 12) {
		getCharacterCurrentParams(kCharacterMaster)[0] = 225 * (4 * rnd(10) + 20);
	} else if (msg->action) {
		if (msg->action == 2) {
			if (_gameTime <= 2943000) {
				getCharacter(kCharacterMaster).callbacks[getCharacter(kCharacterMaster).currentCall + 8] = 1;
				MasterCall(&LogicManager::CONS_Master_SaveGame, 2, 0x2E, 0, 0);
			} else {
				endGame(1, 2934000, 51, 1);
			}
		}
	} else {
		if (_gameTime > 2907000 && !getCharacterCurrentParams(kCharacterMaster)[1]) {
			getCharacterCurrentParams(kCharacterMaster)[1] = 1;
			if (!_gameProgress[14]) {
				playDialog(31, "ARRIVE", 8, 0);
				_engine->getSoundManager()->endAmbient();
			}
		}
		if (_gameTime > 2943000 && !getCharacterCurrentParams(kCharacterMaster)[2]) {
			getCharacterCurrentParams(kCharacterMaster)[2] = 1;
			if (!_gameEvents[108] && !_gameEvents[109]) {
				playDialog(31, "ARRIVE", 8, 0);
				_engine->getSoundManager()->endAmbient();
			}
		}
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
