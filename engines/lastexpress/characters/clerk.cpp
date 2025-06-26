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

void LogicManager::CONS_Clerk(int chapter) {
	switch (chapter) {
	case 0:
		_engine->getMessageManager()->setMessageHandle(
			kCharacterClerk,
			_functionsClerk[getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall]]
		);

		break;
	case 1:
		CONS_Clerk_Birth(0, 0, 0, 0);
		break;
	case 2:
		CONS_Clerk_StartPart2(0, 0, 0, 0);
		break;
	case 3:
		CONS_Clerk_StartPart3(0, 0, 0, 0);
		break;
	case 4:
		CONS_Clerk_StartPart4(0, 0, 0, 0);
		break;
	case 5:
		CONS_Clerk_StartPart5(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::ClerkCall(CALL_PARAMS) {
	getCharacter(kCharacterClerk).currentCall++;

	(this->*functionPointer)(param1, param2, param3, param4);
}

void LogicManager::CONS_Clerk_SaveGame(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterClerk).callParams[getCharacter(kCharacterClerk).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterClerk, &LogicManager::HAND_Clerk_SaveGame);
	getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall] = 1;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterClerk, kCharacterClerk, 12, 0);
}

void LogicManager::HAND_Clerk_SaveGame(HAND_PARAMS) {
	switch (msg->action) {
	case 0:
		getCharacter(kCharacterClerk).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterClerk, _functionsClerk[getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall]]);
		fedEx(kCharacterClerk, kCharacterClerk, 18, 0);
		break;
	case 12:
		save(
			kCharacterClerk,
			getCharacterCurrentParams(kCharacterClerk)[0],
			getCharacterCurrentParams(kCharacterClerk)[1]
		);

		getCharacter(kCharacterClerk).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterClerk, _functionsClerk[getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall]]);
		fedEx(kCharacterClerk, kCharacterClerk, 18, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Clerk_Birth(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterClerk).callParams[getCharacter(kCharacterClerk).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterClerk, &LogicManager::HAND_Clerk_Birth);
	getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall] = 2;

	params->clear();

	fedEx(kCharacterClerk, kCharacterClerk, 12, 0);
}

void LogicManager::HAND_Clerk_Birth(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_Clerk_Processing(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Clerk_StartPart2(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterClerk).callParams[getCharacter(kCharacterClerk).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterClerk, &LogicManager::HAND_Clerk_StartPart2);
	getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall] = 3;

	params->clear();

	fedEx(kCharacterClerk, kCharacterClerk, 12, 0);
}

void LogicManager::HAND_Clerk_StartPart2(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_Clerk_Processing(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Clerk_StartPart3(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterClerk).callParams[getCharacter(kCharacterClerk).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterClerk, &LogicManager::HAND_Clerk_StartPart3);
	getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall] = 4;

	params->clear();

	fedEx(kCharacterClerk, kCharacterClerk, 12, 0);
}

void LogicManager::HAND_Clerk_StartPart3(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_Clerk_Processing(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Clerk_StartPart4(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterClerk).callParams[getCharacter(kCharacterClerk).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterClerk, &LogicManager::HAND_Clerk_StartPart4);
	getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall] = 5;

	params->clear();

	fedEx(kCharacterClerk, kCharacterClerk, 12, 0);
}

void LogicManager::HAND_Clerk_StartPart4(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_Clerk_Processing(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Clerk_StartPart5(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterClerk).callParams[getCharacter(kCharacterClerk).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterClerk, &LogicManager::HAND_Clerk_StartPart5);
	getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall] = 6;

	params->clear();

	fedEx(kCharacterClerk, kCharacterClerk, 12, 0);
}

void LogicManager::HAND_Clerk_StartPart5(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		CONS_Clerk_Processing(0, 0, 0, 0);
		break;
	default:
		break;
	}
}

void LogicManager::CONS_Clerk_DoHaremKnock(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterClerk).callParams[getCharacter(kCharacterClerk).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterClerk, &LogicManager::HAND_Clerk_DoHaremKnock);
	getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall] = 7;

	params->clear();

	params->parameters[0] = param1.intParam;
	params->parameters[1] = param2.intParam;

	fedEx(kCharacterClerk, kCharacterClerk, 12, 0);
}

void LogicManager::HAND_Clerk_DoHaremKnock(HAND_PARAMS) {
	switch (msg->action) {
	case 12:
		switch (getCharacterCurrentParams(kCharacterClerk)[0]) {
		case 5:
			getCharacterCurrentParams(kCharacterClerk)[2] = 4840;
			break;
		case 6:
			getCharacterCurrentParams(kCharacterClerk)[2] = 4070;
			break;
		case 7:
			getCharacterCurrentParams(kCharacterClerk)[2] = 3050;
			break;
		case 8:
			getCharacterCurrentParams(kCharacterClerk)[2] = 2740;
			break;
		default:
			break;
		}

		getCharacterCurrentParams(kCharacterClerk)[3] =
			inComp(kCharacterAlouan, kCarGreenSleeping, getCharacterCurrentParams(kCharacterClerk)[2]);

		getCharacterCurrentParams(kCharacterClerk)[4] =
			getCharacterParams(kCharacterClerk, 8)[6] == getCharacterCurrentParams(kCharacterClerk)[2];

		getCharacterCurrentParams(kCharacterClerk)[5] =
			inComp(kCharacterYasmin, kCarGreenSleeping, getCharacterCurrentParams(kCharacterClerk)[2]);

		getCharacterCurrentParams(kCharacterClerk)[6] =
			inComp(kCharacterHadija, kCarGreenSleeping, getCharacterCurrentParams(kCharacterClerk)[2]);

		setDoor(getCharacterCurrentParams(kCharacterClerk)[0], kCharacterClerk, 3, 0, 0);

		if (getCharacterCurrentParams(kCharacterClerk)[1] == 8) {
			playDialog(kCharacterTableF, "LIB012", 16, 0);
		} else {
			playDialog(kCharacterTableF, "LIB013", 16, 0);
		}

		if (getCharacterCurrentParams(kCharacterClerk)[3] && getCharacterCurrentParams(kCharacterClerk)[4]) {
			getCharacterParams(kCharacterClerk, 8)[4]++;
			switch (getCharacterParams(kCharacterClerk, 8)[4]) {
			case 1:
				playDialog(kCharacterTableF, "Har1014", 16, 15);
				break;
			case 2:
				playDialog(kCharacterTableF, "Har1013", 16, 15);
				playDialog(kCharacterTableF, "Har1016", 16, 150);
				break;
			case 3:
				playDialog(kCharacterTableF, "Har1015A", 16, 15);
				playDialog(kCharacterTableF, "Har1015", 16, 150);
				break;
			default:
				getCharacterCurrentParams(kCharacterClerk)[7] = 1;
				break;
			}

			_globals[kGlobalMetAlouan] = 1;
			_globals[kGlobalMetFatima] = 1;
		} else {
			if (getCharacterCurrentParams(kCharacterClerk)[5] && getCharacterCurrentParams(kCharacterClerk)[6]) {
				getCharacterParams(kCharacterClerk, 8)[5]++;
				switch (getCharacterParams(kCharacterClerk, 8)[5]) {
				case 1:
					playDialog(kCharacterTableF, "Har1014", 16, 15);
					break;
				case 2:
					playDialog(kCharacterTableF, "Har1013", 16, 15);
					break;
				case 3:
					playDialog(kCharacterTableF, "Har1013A", 16, 15);
					break;
				default:
					getCharacterCurrentParams(kCharacterClerk)[7] = 1;
					break;
				}
			} else {
				if (getCharacterCurrentParams(kCharacterClerk)[4]) {
					getCharacterParams(kCharacterClerk, 8)[1]++;
					if (getCharacterParams(kCharacterClerk, 8)[1] == 1) {
						playDialog(kCharacterTableF, "Har1013", 16, 15);
					} else if (getCharacterParams(kCharacterClerk, 8)[1] == 2) {
						playDialog(kCharacterTableF, "Har1013A", 16, 15);
					} else {
						getCharacterCurrentParams(kCharacterClerk)[7] = 1;
					}

					_globals[kGlobalMetFatima] = 1;

				} else {
					if (getCharacterCurrentParams(kCharacterClerk)[5]) {
						getCharacterParams(kCharacterClerk, 8)[2]++;
						if (getCharacterParams(kCharacterClerk, 8)[2] == 1) {
							playDialog(kCharacterTableF, "Har1012", 16, 15);
						} else {
							if (getCharacterParams(kCharacterClerk, 8)[2] == 2) {
								playDialog(kCharacterTableF, "Har1012A", 16, 15);
							} else {
								getCharacterCurrentParams(kCharacterClerk)[7] = 1;
							}
						}
					} else {
						if (getCharacterCurrentParams(kCharacterClerk)[3]) {
							getCharacterParams(kCharacterClerk, 8)[0]++;
							if (getCharacterParams(kCharacterClerk, 8)[0] <= 1) {
								playDialog(kCharacterTableF, "Har1014", 16, 15);
							} else {
								getCharacterCurrentParams(kCharacterClerk)[7] = 1;
							}

							_globals[kGlobalMetAlouan] = 1;
						} else {
							if (getCharacterCurrentParams(kCharacterClerk)[6]) {
								getCharacterParams(kCharacterClerk, 8)[3]++;
								if (getCharacterParams(kCharacterClerk, 8)[3] <= 1) {
									playDialog(kCharacterTableF, "Har1011", 16, 15);
								} else {
									getCharacterCurrentParams(kCharacterClerk)[7] = 1;
								}
							} else {
								getCharacterCurrentParams(kCharacterClerk)[7] = 1;
							}
						}
					}
				}
			}
		}

		if (getCharacterCurrentParams(kCharacterClerk)[7]) {
			send(kCharacterClerk, kCharacterMahmud, 290410610, getCharacterCurrentParams(kCharacterClerk)[0]);
			bumpCathTowardsCond(getCharacterCurrentParams(kCharacterClerk)[0], false, false);
		} else {
			bumpCathTowardsCond(getCharacterCurrentParams(kCharacterClerk)[0], false, true);
		}

		getCharacterParams(kCharacterClerk, 8)[7] = getCharacterCurrentParams(kCharacterClerk)[0];

		getCharacter(kCharacterClerk).currentCall--;
		_engine->getMessageManager()->setMessageHandle(kCharacterClerk, _functionsClerk[getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall]]);
		fedEx(kCharacterClerk, kCharacterClerk, 18, 0);

		break;
	default:
		break;
	}
}

void LogicManager::CONS_Clerk_Processing(CONS_PARAMS) {
	CharacterCallParams *params = &getCharacter(kCharacterClerk).callParams[getCharacter(kCharacterClerk).currentCall];
	_engine->getMessageManager()->setMessageHandle(kCharacterClerk, &LogicManager::HAND_Clerk_Processing);
	getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall] = 8;

	params->clear();

	fedEx(kCharacterClerk, kCharacterClerk, 12, 0);
}

void LogicManager::HAND_Clerk_Processing(HAND_PARAMS) {
	switch (msg->action) {
	case 0: {
		if (cathInCorridor(kCarGreenSleeping) || cathInCorridor(kCarRedSleeping)) {
			if (getCharacterCurrentParams(kCharacterClerk)[3]) {
				if (!getCharacterCurrentParams(kCharacterClerk)[4]) {
					getCharacterCurrentParams(kCharacterClerk)[3]--;

					if (!getCharacterCurrentParams(kCharacterClerk)[3] && _globals[kGlobalJacket] == 2) {
						if (isNight()) {
							playNIS(kEventCathSmokeNight);
						} else {
							playNIS(kEventCathSmokeDay);
						}

						getCharacterCurrentParams(kCharacterClerk)[4] = 1;
						cleanNIS();
					}
				}
			}
		}

		// Horrible way to unroll a goto...
		bool skip = false;

		if (getCharacterCurrentParams(kCharacterClerk)[5]) {
			if (getCharacterCurrentParams(kCharacterClerk)[14] || (getCharacterCurrentParams(kCharacterClerk)[14] = _gameTime + 900, (_gameTime != -900))) {
				if (getCharacterCurrentParams(kCharacterClerk)[14] >= _gameTime)
					skip = true;

				if (!skip)
					getCharacterCurrentParams(kCharacterClerk)[14] = 0x7FFFFFFF;
			}

			if (!skip)
				bumpCath(kCarRestaurant, 58, 255);
		}

		if (!skip)
			getCharacterCurrentParams(kCharacterClerk)[14] = 0;

		if (!getCharacterCurrentParams(kCharacterClerk)[6]) {
			if (getCharacterParams(kCharacterClerk, 8)[7] && !whoRunningDialog(kCharacterTableF)) {
				setDoor(
					getCharacterParams(kCharacterClerk, 8)[7],
					_doors[getCharacterParams(kCharacterClerk, 8)[7]].who,
					3,
					10,
					9
				);

				getCharacterParams(kCharacterClerk, 8)[7] = 0;
			}

			if (getCharacterCurrentParams(kCharacterClerk)[5]) {
				if (!cathRunningDialog("ZFX1001"))
					playDialog(0, "ZFX1001", -1, 0);
			}

			break;
		}

		if (getCharacterCurrentParams(kCharacterClerk)[15] || (getCharacterCurrentParams(kCharacterClerk)[15] = _gameTime + 4500, (_gameTime != -4500))) {
			if (getCharacterCurrentParams(kCharacterClerk)[15] >= _gameTime) {
				if (getCharacterParams(kCharacterClerk, 8)[7] && !whoRunningDialog(kCharacterTableF)) {
					setDoor(
						getCharacterParams(kCharacterClerk, 8)[7],
						_doors[getCharacterParams(kCharacterClerk, 8)[7]].who,
						3,
						10,
						9
					);

					getCharacterParams(kCharacterClerk, 8)[7] = 0;
				}

				if (getCharacterCurrentParams(kCharacterClerk)[5]) {
					if (!cathRunningDialog("ZFX1001"))
						playDialog(0, "ZFX1001", -1, 0);
				}

				break;
			}

			getCharacterCurrentParams(kCharacterClerk)[15] = 0x7FFFFFFF;
		}

		getCharacterCurrentParams(kCharacterClerk)[6] = 0;
		getCharacterCurrentParams(kCharacterClerk)[15] = 0;

		if (getCharacterParams(kCharacterClerk, 8)[7] && !whoRunningDialog(kCharacterTableF)) {
			setDoor(
				getCharacterParams(kCharacterClerk, 8)[7],
				_doors[getCharacterParams(kCharacterClerk, 8)[7]].who,
				3,
				10,
				9);

			getCharacterParams(kCharacterClerk, 8)[7] = 0;
		}

		if (getCharacterCurrentParams(kCharacterClerk)[5]) {
			if (!cathRunningDialog("ZFX1001"))
				playDialog(0, "ZFX1001", -1, 0);
		}

		break;
	}
	case 8:
		if (msg->param.intParam == 5 || msg->param.intParam == 6 || msg->param.intParam == 7 || msg->param.intParam == 8) {
			getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall + 8] = 3;
			ClerkCall(&LogicManager::CONS_Clerk_DoHaremKnock, msg->param.intParam, 8, 0, 0);
		}

		break;
	case 9:
		if (msg->param.intParam == 5 || msg->param.intParam == 6 || msg->param.intParam == 7 || msg->param.intParam == 8) {
			getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall + 8] = 4;
			ClerkCall(&LogicManager::CONS_Clerk_DoHaremKnock, msg->param.intParam, 9, 0, 0);
		}

		break;
	case 12:
		getCharacterCurrentParams(kCharacterClerk)[2] = 1;
		if (_globals[kGlobalChapter] < 5) {
			setDoor(5, kCharacterClerk, 3, 10, 9);
			setDoor(6, kCharacterClerk, 3, 10, 9);
			setDoor(7, kCharacterClerk, 3, 10, 9);
			setDoor(8, kCharacterClerk, 3, 10, 9);
		}

		getCharacter(kCharacterClerk).characterPosition.position = 30000;
		break;
	case 17:
		getCharacter(kCharacterClerk).characterPosition.car = getCharacter(kCharacterCath).characterPosition.car;

		if (checkCathDir(kCarRestaurant, 81)) {
			getCharacterCurrentParams(kCharacterClerk)[5] = 1;
			if (!dialogRunning("ZFX1001"))
				playDialog(0, "ZFX1001", -1, 0);
		} else {
			getCharacterCurrentParams(kCharacterClerk)[5] = 0;
			if (cathRunningDialog("ZFX1001"))
				endDialog("ZFX1001");
		}

		if ((getCharacterCurrentParams(kCharacterClerk)[2] && (getCharacter(kCharacterCath).characterPosition.car != getCharacterCurrentParams(kCharacterClerk)[0])) ||
			 (isNight() != (getCharacterCurrentParams(kCharacterClerk)[1] != 0 ? true : false))) {

			switch (getCharacter(kCharacterCath).characterPosition.car) {
			case 1:
			case 6:
				if (_globals[kGlobalIsDayTime]) {
					startCycOtis(kCharacterClerk, "B1WNM");
				} else if (isNight()) {
					startCycOtis(kCharacterClerk, "B1WNN");
				} else {
					startCycOtis(kCharacterClerk, "B1WND");
				}

				break;
			case 3:
			case 4:
				if (_globals[kGlobalIsDayTime]) {
					startCycOtis(kCharacterClerk, "S1WNM");
				} else if (isNight()) {
					startCycOtis(kCharacterClerk, "S1WNN");
				} else {
					startCycOtis(kCharacterClerk, "S1WND");
				}

				break;
			case 5:
				if (_globals[kGlobalIsDayTime]) {
					startCycOtis(kCharacterClerk, "RCWNM");
				} else if (isNight()) {
					startCycOtis(kCharacterClerk, "RCWNN");
				} else {
					startCycOtis(kCharacterClerk, "RCWND");
				}

				break;
			default:
				endGraphics(kCharacterClerk);
				break;
			}

			getCharacterCurrentParams(kCharacterClerk)[0] = getCharacter(kCharacterCath).characterPosition.car;
			getCharacterCurrentParams(kCharacterClerk)[1] = isNight();
		}

		if (!getCharacterCurrentParams(kCharacterClerk)[4]) {
			getCharacterCurrentParams(kCharacterClerk)[3] = 2700;
			getCharacterCurrentParams(kCharacterClerk)[4] = 0;
		}

		if (_globals[kGlobalJacket] != 1) {
			if (getCharacterCurrentParams(kCharacterClerk)[7] && !inComp(kCharacterCath, getCharacterCurrentParams(kCharacterClerk)[8], getCharacterCurrentParams(kCharacterClerk)[9]) && !inComp(kCharacterCath, getCharacterCurrentParams(kCharacterClerk)[8], getCharacterCurrentParams(kCharacterClerk)[10])) {
				if (dialogRunning((char *)&getCharacterCurrentParams(kCharacterClerk)[11]))
					fadeDialog((char *)&getCharacterCurrentParams(kCharacterClerk)[11]);
				getCharacterCurrentParams(kCharacterClerk)[7] = 0;
			}

			break;
		}

		if (checkCathDir(kCarRedSleeping, 18)) {
			getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall + 8] = 1;
			ClerkCall(&LogicManager::CONS_Clerk_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
			break;
		}

		if (checkCathDir(kCarGreenSleeping, 22)) {
			getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall + 8] = 2;
			ClerkCall(&LogicManager::CONS_Clerk_SaveGame, 2, kEventMertensBloodJacket, 0, 0);
		} else {
			if (getCharacterCurrentParams(kCharacterClerk)[7] && !inComp(kCharacterCath, getCharacterCurrentParams(kCharacterClerk)[8], getCharacterCurrentParams(kCharacterClerk)[9]) && !inComp(kCharacterCath, getCharacterCurrentParams(kCharacterClerk)[8], getCharacterCurrentParams(kCharacterClerk)[10])) {
				if (dialogRunning((char *)&getCharacterCurrentParams(kCharacterClerk)[11]))
					fadeDialog((char *)&getCharacterCurrentParams(kCharacterClerk)[11]);

				getCharacterCurrentParams(kCharacterClerk)[7] = 0;
			}
		}

		break;
	case 18:
		switch (getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall + 8]) {
		case 1:
			playNIS(kEventCoudertBloodJacket);
			endGame(0, 1, 55, true);

			if (getCharacterCurrentParams(kCharacterClerk)[7] && !inComp(kCharacterCath, getCharacterCurrentParams(kCharacterClerk)[8], getCharacterCurrentParams(kCharacterClerk)[9]) && !inComp(kCharacterCath, getCharacterCurrentParams(kCharacterClerk)[8], getCharacterCurrentParams(kCharacterClerk)[10])) {
				if (dialogRunning((char *)&getCharacterCurrentParams(kCharacterClerk)[11]))
					fadeDialog((char *)&getCharacterCurrentParams(kCharacterClerk)[11]);

				getCharacterCurrentParams(kCharacterClerk)[7] = 0;
			}

			break;
		case 2:
			playNIS(kEventMertensBloodJacket);
			endGame(0, 1, 55, true);

			if (getCharacterCurrentParams(kCharacterClerk)[7] && !inComp(kCharacterCath, getCharacterCurrentParams(kCharacterClerk)[8], getCharacterCurrentParams(kCharacterClerk)[9]) && !inComp(kCharacterCath, getCharacterCurrentParams(kCharacterClerk)[8], getCharacterCurrentParams(kCharacterClerk)[10])) {
				if (dialogRunning((char *)&getCharacterCurrentParams(kCharacterClerk)[11]))
					fadeDialog((char *)&getCharacterCurrentParams(kCharacterClerk)[11]);

				getCharacterCurrentParams(kCharacterClerk)[7] = 0;
			}

			break;
		case 5:
			playNIS(kEventLocomotiveConductorsDiscovered);
			endGame(0, 1, 63, true);
			break;
		case 6:
			playNIS(kEventCathBreakCeiling);
			setDoor(73, kCharacterCath, 2, 255, 255);
			cleanNIS();
			break;
		case 7:
			playNIS(kEventCathJumpDownCeiling);
			bumpCath(kCarKronos, 89, 255);
			break;
		case 8:
			playNIS(kEventCloseMatchbox);
			bumpCath(kCarRestaurant, 51, 255);
			break;
		default:
			break;
		}

		break;
	case 191070912:
		getCharacterParams(kCharacterClerk, 8)[6] = msg->param.intParam;
		break;
	case 191350523:
		getCharacterCurrentParams(kCharacterClerk)[2] = 0;
		endGraphics(kCharacterClerk);
		break;
	case 202613084:
		getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall + 8] = 8;
		ClerkCall(&LogicManager::CONS_Clerk_SaveGame, 2, kEventCloseMatchbox, 0, 0);
		break;
	case 222746496:
		if (msg->param.intParam) {
			switch (msg->param.intParam) {
			case 1:
				getCharacterCurrentParams(kCharacterClerk)[8] = 3;
				getCharacterCurrentParams(kCharacterClerk)[9] = 8200;
				getCharacterCurrentParams(kCharacterClerk)[10] = 7850;
				break;
			case 2:
				getCharacterCurrentParams(kCharacterClerk)[8] = 3;
				getCharacterCurrentParams(kCharacterClerk)[9] = 7500;
				getCharacterCurrentParams(kCharacterClerk)[10] = 7850;
				break;
			case 3:
				getCharacterCurrentParams(kCharacterClerk)[8] = 3;
				getCharacterCurrentParams(kCharacterClerk)[9] = 6470;
				getCharacterCurrentParams(kCharacterClerk)[10] = 6130;
				break;
			case 4:
				getCharacterCurrentParams(kCharacterClerk)[8] = 3;
				getCharacterCurrentParams(kCharacterClerk)[9] = 5790;
				getCharacterCurrentParams(kCharacterClerk)[10] = 6130;
				break;
			case 5:
				getCharacterCurrentParams(kCharacterClerk)[8] = 3;
				getCharacterCurrentParams(kCharacterClerk)[9] = 4840;
				getCharacterCurrentParams(kCharacterClerk)[10] = 4455;
				break;
			case 6:
				getCharacterCurrentParams(kCharacterClerk)[8] = 3;
				getCharacterCurrentParams(kCharacterClerk)[9] = 4070;
				getCharacterCurrentParams(kCharacterClerk)[10] = 4455;
				break;
			case 7:
				getCharacterCurrentParams(kCharacterClerk)[8] = 3;
				getCharacterCurrentParams(kCharacterClerk)[9] = 3050;
				getCharacterCurrentParams(kCharacterClerk)[10] = 0;
				break;
			case 8:
				getCharacterCurrentParams(kCharacterClerk)[8] = 3;
				getCharacterCurrentParams(kCharacterClerk)[9] = 2740;
				getCharacterCurrentParams(kCharacterClerk)[10] = 0;
				break;
			case 32:
				getCharacterCurrentParams(kCharacterClerk)[8] = 4;
				getCharacterCurrentParams(kCharacterClerk)[9] = 8200;
				getCharacterCurrentParams(kCharacterClerk)[10] = 7850;
				break;
			case 33:
				getCharacterCurrentParams(kCharacterClerk)[8] = 4;
				getCharacterCurrentParams(kCharacterClerk)[9] = 7500;
				getCharacterCurrentParams(kCharacterClerk)[10] = 7850;
				break;
			case 34:
				getCharacterCurrentParams(kCharacterClerk)[8] = 4;
				getCharacterCurrentParams(kCharacterClerk)[9] = 6470;
				getCharacterCurrentParams(kCharacterClerk)[10] = 6130;
				break;
			case 35:
				getCharacterCurrentParams(kCharacterClerk)[8] = 4;
				getCharacterCurrentParams(kCharacterClerk)[9] = 5790;
				getCharacterCurrentParams(kCharacterClerk)[10] = 6130;
				break;
			case 36:
				getCharacterCurrentParams(kCharacterClerk)[8] = 4;
				getCharacterCurrentParams(kCharacterClerk)[9] = 4840;
				getCharacterCurrentParams(kCharacterClerk)[10] = 4455;
				break;
			case 37:
				getCharacterCurrentParams(kCharacterClerk)[8] = 4;
				getCharacterCurrentParams(kCharacterClerk)[9] = 4070;
				getCharacterCurrentParams(kCharacterClerk)[10] = 4455;
				break;
			case 38:
				getCharacterCurrentParams(kCharacterClerk)[8] = 4;
				getCharacterCurrentParams(kCharacterClerk)[9] = 3050;
				getCharacterCurrentParams(kCharacterClerk)[10] = 0;
				break;
			case 39:
				getCharacterCurrentParams(kCharacterClerk)[8] = 4;
				getCharacterCurrentParams(kCharacterClerk)[9] = 2740;
				getCharacterCurrentParams(kCharacterClerk)[10] = 0;
				break;
			default:
				break;
			}
		}

		break;
	case 203339360:
		if (getCharacterCurrentParams(kCharacterClerk)[6]) {
			getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall + 8] = 5;
			ClerkCall(&LogicManager::CONS_Clerk_SaveGame, 2, kEventLocomotiveConductorsDiscovered, 0, 0);
		} else {
			getCharacterCurrentParams(kCharacterClerk)[6] = 1;
			playNIS(kEventLocomotiveConductorsLook);
			bumpCath(kCarCoalTender, 2, 255);
		}

		break;
	case 203419131:
		if (!getCharacterCurrentParams(kCharacterClerk)[2]) {
			getCharacterCurrentParams(kCharacterClerk)[2] = 1;
			getCharacterCurrentParams(kCharacterClerk)[0] = 0;
			send(kCharacterClerk, kCharacterClerk, 17, 0);
		}

		break;
	case 203863200:
		if (msg->param.stringParam) {
			getCharacterCurrentParams(kCharacterClerk)[7] = 1;
			Common::strcpy_s((char *)&getCharacterCurrentParams(kCharacterClerk)[11], 12, msg->param.stringParam);
		}

		break;
	case 225056224:
		getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall + 8] = 6;
		ClerkCall(&LogicManager::CONS_Clerk_SaveGame, 2, kEventCathBreakCeiling, 0, 0);
		break;
	case 338494260:
		getCharacter(kCharacterClerk).callbacks[getCharacter(kCharacterClerk).currentCall + 8] = 7;
		ClerkCall(&LogicManager::CONS_Clerk_SaveGame, 2, kEventCathJumpDownCeiling, 0, 0);
		break;
	default:
		break;
	}
}

void (LogicManager::*LogicManager::_functionsClerk[])(HAND_PARAMS) = {
	nullptr,
	&LogicManager::HAND_Clerk_SaveGame,
	&LogicManager::HAND_Clerk_Birth,
	&LogicManager::HAND_Clerk_StartPart2,
	&LogicManager::HAND_Clerk_StartPart3,
	&LogicManager::HAND_Clerk_StartPart4,
	&LogicManager::HAND_Clerk_StartPart5,
	&LogicManager::HAND_Clerk_DoHaremKnock,
	&LogicManager::HAND_Clerk_Processing
};

} // End of namespace LastExpress
