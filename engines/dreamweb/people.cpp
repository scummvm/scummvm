/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "dreamweb/dreamweb.h"

namespace DreamGen {

static void (DreamBase::*reelCallbacks[57])(ReelRoutine &) = {
	&DreamBase::gamer, &DreamBase::sparkyDrip,
	&DreamBase::eden, &DreamBase::edenInBath,
	&DreamBase::sparky, &DreamBase::smokeBloke,
	&DreamBase::manAsleep, &DreamBase::drunk,
	&DreamBase::receptionist, &DreamBase::genericPerson /*maleFan*/,
	&DreamBase::genericPerson /*femaleFan*/, &DreamBase::louis,
	&DreamBase::louisChair, &DreamBase::soldier1,
	&DreamBase::bossMan, &DreamBase::interviewer,
	&DreamBase::heavy, &DreamBase::manAsleep /*manAsleep2*/,
	&DreamBase::genericPerson /*manSatStill*/, &DreamBase::drinker,
	&DreamBase::bartender, &DreamBase::genericPerson /*otherSmoker*/,
	&DreamBase::genericPerson /*tattooMan*/, &DreamBase::attendant,
	&DreamBase::keeper, &DreamBase::candles1,
	&DreamBase::smallCandle, &DreamBase::security,
	&DreamBase::copper, &DreamBase::poolGuard,
	&DreamBase::rockstar, &DreamBase::businessMan,
	&DreamBase::train, &DreamBase::genericPerson /*aide*/,
	&DreamBase::mugger, &DreamBase::helicopter,
	&DreamBase::introMagic1, &DreamBase::introMusic,
	&DreamBase::introMagic2, &DreamBase::candles2,
	&DreamBase::gates, &DreamBase::introMagic3,
	&DreamBase::introMonks1, &DreamBase::candles,
	&DreamBase::introMonks2, &DreamBase::handClap,
	&DreamBase::monkAndRyan, &DreamBase::endGameSeq,
	&DreamBase::priest, &DreamBase::madman,
	&DreamBase::madmansTelly, &DreamBase::alleyBarkSound,
	&DreamBase::foghornSound, &DreamBase::carParkDrip,
	&DreamBase::carParkDrip, &DreamBase::carParkDrip,
	&DreamBase::carParkDrip
};

static const ReelRoutine g_initialReelRoutines[] = {
// Room number and x,y
// reel pointer
// speed,speed count,convers. no.
	{ 1,44,0, 20, 2,0,1 },
	{ 1,55,0, 0, 50,20,0 },
	{ 24,22,0, 74, 1,0,0 },
	{ 24,33,10, 75, 1,0,1 },
	{ 1,44,0, 27, 2,0,2 },
	{ 1,44,0, 96, 3,0,4 },
	{ 1,44,0, 118, 2,0,5 },
	{ 1,44,10, 0, 2,0,0 },
	{ 5,22,20, 53, 3,0,0 },
	{ 5,22,20, 40, 1,0,2 },
	{ 5,22,20, 50, 1,0,3 },
	{ 2,11,10, 192, 1,0,0 },
	{ 2,11,10, 182, 2,0,1 },
	{ 8,11,10, 0, 2,0,1 },
	{ 23,0,50, 0, 3,0,0 },
	{ 28,11,20, 250, 4,0,0 },
	{ 23,0,50, 43, 2,0,8 },
	{ 23,11,40, 130, 2,0,1 },
	{ 23,22,40, 122, 2,0,2 },
	{ 23,22,40, 105, 2,0,3 },
	{ 23,22,40, 81, 2,0,4 },
	{ 23,11,40, 135, 2,0,5 },
	{ 23,22,40, 145, 2,0,6 },
	{ 4,22,30, 0, 2,0,0 },
	{ 45,22,30, 200, 0,0,20 },
	{ 45,22,30, 39, 2,0,0 },
	{ 45,22,30, 25, 2,0,0 },
	{ 8,22,40, 32, 2,0,0 },
	{ 7,11,20, 64, 2,0,0 },
	{ 22,22,20, 82, 2,0,0 },
	{ 27,11,30, 0, 2,0,0 },
	{ 20,0,30, 0, 2,0,0 },
	{ 14,33,40, 21, 1,0,0 },
	{ 29,11,10, 0, 1,0,0 },
	{ 2,22,0, 2, 2,0,0 },
	{ 25,0,50, 4, 2,0,0 },
	{ 50,22,30, 121, 2,0,0 },
	{ 50,22,30, 0, 20,0,0 },
	{ 52,22,30, 192, 2,0,0 },
	{ 52,22,30, 233, 2,0,0 },
	{ 50,22,40, 104, 55,0,0 }, // ...., 65,0,0 for German CD
	{ 53,33,0, 99, 2,0,0 },
	{ 50,22,40, 0, 3,0,0 },
	{ 50,22,30, 162, 2,0,0 },
	{ 52,22,30, 57, 2,0,0 },
	{ 52,22,30, 0, 2,0,0 },
	{ 54,0,0, 72, 3,0,0 },
	{ 55,44,0, 0, 2,0,0 },
	{ 19,0,0, 0, 28,0,0 },
	{ 14,22,0, 2, 2,0,0 },
	{ 14,22,0, 300, 1,0,0 },
	{ 10,22,30, 174, 0,0,0 },
	{ 12,22,20, 0, 1,0,0 },
	{ 11,11,20, 0, 50,20,0 },
	{ 11,11,30, 0, 50,20,0 },
	{ 11,22,20, 0, 50,20,0 },
	{ 14,33,40, 0, 50,20,0 },
	{ 255,0,0, 0, 0,0,0 }
};

void DreamBase::setupInitialReelRoutines() {
	for (unsigned int i = 0; i < kNumReelRoutines + 1; ++i) {
		_reelRoutines[i] = g_initialReelRoutines[i];
		if (_reelRoutines[i].period == 55 && isCD() && engine->getLanguage() == Common::DE_DEU)
			_reelRoutines[i].period = 65;
	}
}

void DreamBase::updatePeople() {
	_peopleList.clear();
	++_mainTimer;

	for (int i = 0; _reelRoutines[i].reallocation != 255; ++i) {
		if (_reelRoutines[i].reallocation == _realLocation &&
		        _reelRoutines[i].mapX == _mapX &&
		        _reelRoutines[i].mapY == _mapY) {
			assert(reelCallbacks[i]);
			(this->*(reelCallbacks[i]))(_reelRoutines[i]);
		}
	}
}

void DreamBase::madmanText() {
	byte origCount;

	if (isCD()) {
		if (_speechCount >= 63)
			return;
		if (_channel1Playing != 255)
			return;
		origCount = _speechCount;
		++_speechCount;
	} else {
		if (data.byte(kCombatcount) >= 61)
			return;
		if (data.byte(kCombatcount) & 3)
			return;
		origCount = data.byte(kCombatcount) / 4;
	}
	setupTimedTemp(47 + origCount, 82, 72, 80, 90, 1);
}

void DreamBase::madman(ReelRoutine &routine) {
	data.word(kWatchingtime) = 2;
	if (checkSpeed(routine)) {
		uint16 newReelPointer = routine.reelPointer();
		if (newReelPointer >= 364) {
			data.byte(kMandead) = 2;
			showGameReel(&routine);
			return;
		}
		if (newReelPointer == 10) {
			loadTempText("DREAMWEB.T82");
			data.byte(kCombatcount) = (uint8)-1;
			_speechCount = 0;
		}
		++newReelPointer;
		if (newReelPointer == 294) {
			if (!_wonGame) {
				_wonGame = true;
				getRidOfTempText();
			}
			return;
		}
		if (newReelPointer == 66) {
			++data.byte(kCombatcount);
			madmanText();
			newReelPointer = 53;
			if (data.byte(kCombatcount) >= (isCD() ? 64 : 62)) {
				if (data.byte(kCombatcount) == (isCD() ? 70 : 68))
					newReelPointer = 310;
				else {
					if (data.byte(kLastweapon) == 8) {
						data.byte(kCombatcount) = isCD() ? 72 : 70;
						data.byte(kLastweapon) = (uint8)-1;
						data.byte(kMadmanflag) = 1;
						newReelPointer = 67;
					}
				}
			}
		}
		routine.setReelPointer(newReelPointer);
	}
	showGameReel(&routine);
	routine.mapX = _mapX;
	madMode();
}

void DreamBase::madMode() {
	data.word(kWatchingtime) = 2;
	_pointerMode = 0;
	if (data.byte(kCombatcount) < (isCD() ? 65 : 63))
		return;
	if (data.byte(kCombatcount) >= (isCD() ? 70 : 68))
		return;
	_pointerMode = 2;
}

void DreamBase::addToPeopleList(ReelRoutine *routine) {
	People people;
	people._reelPointer = routine->reelPointer();
	people._routinePointer = routine;
	people.b4 = routine->b7;

	_peopleList.push_back(people);
}

bool DreamBase::checkSpeed(ReelRoutine &routine) {
	if (data.byte(kLastweapon) != (uint8)-1)
		return true;
	++routine.counter;
	if (routine.counter != routine.period)
		return false;
	routine.counter = 0;
	return true;
}

void DreamBase::sparkyDrip(ReelRoutine &routine) {
	if (checkSpeed(routine))
		playChannel0(14, 0);
}

void DreamBase::genericPerson(ReelRoutine &routine) {
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::gamer(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint8 v;
		do {
			v = 20 + engine->randomNumber() % 5;
		} while (v == routine.reelPointer());
		routine.setReelPointer(v);
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::eden(ReelRoutine &routine) {
	if (data.byte(kGeneraldead))
		return;
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::sparky(ReelRoutine &routine) {
	if (data.word(kCard1money))
		routine.b7 = 3;
	if (checkSpeed(routine)) {
		if (routine.reelPointer() == 34) {
			if (engine->randomNumber() < 30)
				routine.incReelPointer();
			else
				routine.setReelPointer(27);
		} else {
			if (routine.reelPointer() != 48)
				routine.incReelPointer();
			else
				routine.setReelPointer(27);
		}
	}
	showGameReel(&routine);
	addToPeopleList(&routine);
	if (routine.b7 & 128)
		data.byte(kTalkedtosparky) = 1;
}

void DreamBase::rockstar(ReelRoutine &routine) {
	if ((routine.reelPointer() == 303) || (routine.reelPointer() == 118)) {
		_newLocation = 45;
		showGameReel(&routine);
		return;
	}
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 118) {
			data.byte(kMandead) = 2;
		} else if (nextReelPointer == 79) {
			--nextReelPointer;
			if (data.byte(kLastweapon) != 1) {
				++data.byte(kCombatcount);
				if (data.byte(kCombatcount) == 40) {
					data.byte(kCombatcount) = 0;
					nextReelPointer = 79;
				}
			} else {
				data.byte(kLastweapon) = (uint8)-1;
				nextReelPointer = 123;
			}
		}
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
	if (routine.reelPointer() == 78) {
		addToPeopleList(&routine);
		_pointerMode = 2;
		data.word(kWatchingtime) = 0;
	} else {
		data.word(kWatchingtime) = 2;
		_pointerMode = 0;
		routine.mapY = _mapY;
	}
}

void DreamBase::madmansTelly(ReelRoutine &routine) {
	uint16 nextReelPointer = routine.reelPointer() + 1;
	if (nextReelPointer == 307)
		nextReelPointer = 300;
	routine.setReelPointer(nextReelPointer);
	showGameReel(&routine);
}


void DreamBase::smokeBloke(ReelRoutine &routine) {
	if (data.byte(kRockstardead) == 0) {
		if (routine.b7 & 128)
			DreamBase::setLocation(5);
	}
	if (checkSpeed(routine)) {
		if (routine.reelPointer() == 100) {
			if (engine->randomNumber() < 30)
				routine.incReelPointer();
			else
				routine.setReelPointer(96);
		} else if (routine.reelPointer() == 117)
			routine.setReelPointer(96);
		else
			routine.incReelPointer();
	}
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::manAsleep(ReelRoutine &routine) {
	routine.b7 &= 127;
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::attendant(ReelRoutine &routine) {
	showGameReel(&routine);
	addToPeopleList(&routine);
	if (routine.b7 & 128)
		data.byte(kTalkedtoattendant) = 1;
}

void DreamBase::keeper(ReelRoutine &routine) {
	if (data.byte(kKeeperflag) != 0) {
		// Not waiting
		addToPeopleList(&routine);
		showGameReel(&routine);
		return;
	}

	if (data.word(kReeltowatch) < 190)
		return; // waiting

	data.byte(kKeeperflag)++;

	if ((routine.b7 & 127) != data.byte(kDreamnumber))
		routine.b7 = data.byte(kDreamnumber);
}

void DreamBase::drunk(ReelRoutine &routine) {
	if (data.byte(kGeneraldead))
		return;
	routine.b7 &= 127;
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::interviewer(ReelRoutine &routine) {
	if (data.word(kReeltowatch) == 68)
		routine.incReelPointer();

	if (routine.reelPointer() != 250 && routine.reelPointer() != 259 && checkSpeed(routine))
		routine.incReelPointer();
	
	showGameReel(&routine);
}

void DreamBase::drinker(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		routine.incReelPointer();

		if ( routine.reelPointer() == 115 ||
			(routine.reelPointer() == 106 && engine->randomNumber() >= 3))
			routine.setReelPointer(105);
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::alleyBarkSound(ReelRoutine &routine) {
	uint16 prevReelPointer = routine.reelPointer() - 1;
	if (prevReelPointer == 0) {
		playChannel1(14);
		routine.setReelPointer(1000);
	} else {
		routine.setReelPointer(prevReelPointer);
	}
}

void DreamBase::introMagic1(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 145)
			nextReelPointer = 121;
		routine.setReelPointer(nextReelPointer);
		if (nextReelPointer == 121) {
			++_introCount;
			intro1Text();
			if (_introCount == 8) {
				_mapY += 10;
				_nowInNewRoom = 1;
			}
		}
	}
	showGameReel(&routine);
}

void DreamBase::introMagic2(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 216)
			nextReelPointer = 192;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamBase::introMagic3(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 218)
			_getBack = 1;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
	routine.mapX = _mapX;
}

void DreamBase::candles1(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 44)
			nextReelPointer = 39;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamBase::candles2(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 238)
			nextReelPointer = 233;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamBase::smallCandle(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 37)
			nextReelPointer = 25;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamBase::introMusic(ReelRoutine &routine) {
}

void DreamBase::candles(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 167)
			nextReelPointer = 162;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamBase::gates(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 116)
			playChannel1(17);
		if (nextReelPointer >= 110)
			routine.period = 2;
		if (nextReelPointer == 120) {
			_getBack = 1;
			nextReelPointer = 119;
		}
		routine.setReelPointer(nextReelPointer);
		intro3Text(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamBase::security(ReelRoutine &routine) {
	if (routine.reelPointer() == 32) {
		if (data.byte(kLastweapon) == 1) {
			data.word(kWatchingtime) = 10;
			if ((_mansPath == 9) && (_facing == 0)) {
				data.byte(kLastweapon) = (uint8)-1;
				routine.incReelPointer();
			}
		}
	} else if (routine.reelPointer() == 69)
		return;
	else {
		data.word(kWatchingtime) = 10;
		if (checkSpeed(routine))
			routine.incReelPointer();
	}
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::edenInBath(ReelRoutine &routine) {
	if (data.byte(kGeneraldead) == 0 || data.byte(kSartaindead) != 0)
		return;

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::louis(ReelRoutine &routine) {
	if (data.byte(kRockstardead) != 0)
		return;

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::handClap(ReelRoutine &routine) {
}

void DreamBase::carParkDrip(ReelRoutine &routine) {
	if (!checkSpeed(routine))
		return; // cantdrip2

	playChannel1(14);
}

void DreamBase::foghornSound(ReelRoutine &routine) {
	if (engine->randomNumber() == 198)
		playChannel1(13);
}

void DreamBase::train(ReelRoutine &routine) {
	// The original code has logic for this, but it is disabled
}

void DreamBase::louisChair(ReelRoutine &routine) {
	if (data.byte(kRockstardead) == 0)
		return; // notlouis2

	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 191) {
			routine.setReelPointer(182);	// Restart Louis
		} else if (nextReelPointer != 185) {
			routine.setReelPointer(nextReelPointer);
		} else {
			if (engine->randomNumber() < 245)
				routine.setReelPointer(182);	// Restart Louis
			else
				routine.setReelPointer(nextReelPointer);
		}
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::bossMan(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;

		if (nextReelPointer == 4) {
			if (data.byte(kGunpassflag) != 1 && engine->randomNumber() >= 10)
				nextReelPointer = 0;
		} else if (nextReelPointer == 20) {
			if (data.byte(kGunpassflag) != 1)
				nextReelPointer = 0;
		} else if (nextReelPointer == 41) {
			nextReelPointer = 0;
			data.byte(kGunpassflag)++;
			routine.b7 = 10;
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
	addToPeopleList(&routine);

	if (routine.b7 & 128)
		data.byte(kTalkedtoboss) = 1;
}

void DreamBase::priest(ReelRoutine &routine) {
	if (routine.reelPointer() == 8)
		return; // priestspoken

	_pointerMode = 0;
	data.word(kWatchingtime) = 2;

	if (checkSpeed(routine)) {
		routine.incReelPointer();
		priestText(routine);
	}
}

void DreamBase::priestText(ReelRoutine &routine) {
	uint16 reel = routine.reelPointer();
	if (reel < 2 || reel >= 7 || (reel & 1))
		return; // nopriesttext

	setupTimedUse((reel >> 1) + 50, 54, 1, 72, 80);
}

void DreamBase::monkAndRyan(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 83) {
			_introCount++;
			textForMonk();
			nextReelPointer = 77;

			if (_introCount == 57) {
				_getBack = 1;
				return;
			}
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
}

void DreamBase::copper(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 94) {
			nextReelPointer = 64;
		} else if (nextReelPointer == 81 || nextReelPointer == 66) {
			// Might wait
			if (engine->randomNumber() >= 7)
				nextReelPointer--;
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::introMonks1(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;

		if (nextReelPointer == 80) {
			_mapY += 10;
			_nowInNewRoom = 1;
			showGameReel(&routine);
			return;
		} else if (nextReelPointer == 30) {
			_mapY -= 10;
			_nowInNewRoom = 1;
			nextReelPointer = 51;
		}

		routine.setReelPointer(nextReelPointer);

		if (nextReelPointer ==  5 || nextReelPointer == 15 ||
			nextReelPointer == 25 || nextReelPointer == 61 ||
			nextReelPointer == 71) {
			// Wait step
			intro2Text(nextReelPointer);
			routine.counter = (uint8)-20;
		}
	}

	showGameReel(&routine);
	routine.mapY = _mapY;
}

void DreamBase::introMonks2(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 87) {
			_introCount++;
			monks2text();

			if (_introCount == 19)
				nextReelPointer = 87;
			else
				nextReelPointer = 74;
		}

		if (nextReelPointer == 110) {
			_introCount++;
			monks2text();
		
			if (_introCount == 35)
				nextReelPointer = 111;
			else
				nextReelPointer = 98;
		} else if (nextReelPointer == 176) {
			_getBack = 1;
		} else if (nextReelPointer == 125) {
			nextReelPointer = 140;
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
}

void DreamBase::soldier1(ReelRoutine &routine) {
	if (routine.reelPointer() != 0) {
		data.word(kWatchingtime) = 10;
		if (routine.reelPointer() == 30) {
			data.byte(kCombatcount)++;
			if (data.byte(kCombatcount) == 40)
				data.byte(kMandead) = 2;
		} else if (checkSpeed(routine)) {
			// Not after shot
			routine.incReelPointer();
		}
	} else if (data.byte(kLastweapon) == 1) {
		data.word(kWatchingtime) = 10;
		if (_mansPath == 2 && _facing == 4)
			routine.incReelPointer();
		data.byte(kLastweapon) = 0xFF;
		data.byte(kCombatcount) = 0;
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::receptionist(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		if (data.byte(kCardpassflag) == 1) {
			// Set card
			data.byte(kCardpassflag)++;
			routine.b7 = 1;
			routine.setReelPointer(64);
		}

		if (routine.reelPointer() != 58) {
			// notdes1
			if (routine.reelPointer() != 60) {
				// notdes2
				if (routine.reelPointer() != 88)
					routine.incReelPointer();	// not end card
				else
					routine.setReelPointer(53);
			} else if (engine->randomNumber() >= 240) {
				routine.setReelPointer(53);
			}
		} else if (engine->randomNumber() >= 30) {
			routine.setReelPointer(55);
		} else {
			// notdes2
			if (routine.reelPointer() != 88)
				routine.incReelPointer();	// not end card
			else
				routine.setReelPointer(53);
		}
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
	if (routine.b7 & 128)
		data.byte(kTalkedtorecep) = 1;
}

void DreamBase::bartender(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		if (routine.reelPointer() == 86) {
			if (engine->randomNumber() >= 18)
				routine.setReelPointer(81);
			else
				routine.incReelPointer();	// notsmoket2
		} else if (routine.reelPointer() == 103) {
			routine.setReelPointer(81);	// notsmoket1
		} else {
			routine.incReelPointer();	// notsmoket2
		}
	}

	showGameReel(&routine);
	if (data.byte(kGunpassflag) == 1)
		routine.b7 = 9;	// got gun

	addToPeopleList(&routine);
}

void DreamBase::heavy(ReelRoutine &routine) {
	routine.b7 &= 127;
	if (routine.reelPointer() != 43) {
		data.word(kWatchingtime) = 10;
		if (routine.reelPointer() == 70) {
			// After shot
			data.byte(kCombatcount)++;
			if (data.byte(kCombatcount) == 80)
				data.byte(kMandead) = 2;
		} else {
			if (checkSpeed(routine))
				routine.incReelPointer();
		}
	} else if (data.byte(kLastweapon) == 1 && _mansPath == 5 && _facing == 4) {
		// Heavy wait
		data.byte(kLastweapon) = (byte)-1;
		routine.incReelPointer();
		data.byte(kCombatcount) = 0;
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamBase::helicopter(ReelRoutine &routine) {
	if (routine.reelPointer() == 203) {
		// Won helicopter
		_pointerMode = 0;
		return;
	}

	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 53) {
			// Before killing helicopter
			data.byte(kCombatcount)++;
			if (data.byte(kCombatcount) >= 8)
				data.byte(kMandead) = 2;
			nextReelPointer = 49;
		} else if (nextReelPointer == 9) {
			nextReelPointer--;
			if (data.byte(kLastweapon) == 1) {
				data.byte(kLastweapon) = (byte)-1;
				nextReelPointer = 55;
			} else {
				nextReelPointer = 5;
				data.byte(kCombatcount)++;
				if (data.byte(kCombatcount) == 20) {
					data.byte(kCombatcount) = 0;
					nextReelPointer = 9;
				}
			}
		} 

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
	routine.mapX = _mapX;
	if (routine.reelPointer() < 9 && data.byte(kCombatcount) >= 7) {
		_pointerMode = 2;
		data.word(kWatchingtime) = 0;
	} else {
		// Not waiting helicopter
		_pointerMode = 0;
		data.word(kWatchingtime) = 2;
	}
}

void DreamBase::mugger(ReelRoutine &routine) {
	if (routine.reelPointer() != 138) {
		if (routine.reelPointer() == 176)
			return; // endmugger2

		if (routine.reelPointer() == 2)
			data.word(kWatchingtime) = 175 * 2;	// set watch

		if (checkSpeed(routine))
			routine.incReelPointer();

		showGameReel(&routine);
		routine.mapX = _mapX;
	} else {
		createPanel2();
		showIcon();

		const uint8 *string = (const uint8 *)_puzzleText.getString(41);
		uint16 y = 104;
		printDirect(&string, 33 + 20, &y, 241, 241 & 1);
		workToScreen();
		hangOn(300);
		routine.setReelPointer(140);
		_mansPath = 2;
		_finalDest = 2;
		findXYFromPath();
		_resetManXY = 1;
		_command = findExObject("WETA");
		_objectType = kExObjectType;
		removeObFromInv();
		_command = findExObject("WETB");
		_objectType = kExObjectType;
		removeObFromInv();
		makeMainScreen();
		DreamBase::setupTimedUse(48, 70, 10, 68 - 32, 54 + 64);
		data.byte(kBeenmugged) = 1;
	}
}

// Exiting the elevator of Sartain's industries, Sartain (the businessman) and
// two bodyguards are expecting Ryan.
void DreamBase::businessMan(ReelRoutine &routine) {
	_pointerMode = 0;
	data.word(kWatchingtime) = 2;
	if (routine.reelPointer() == 2)
		DreamBase::setupTimedUse(49, 30, 1, 68, 174);	// First

	if (routine.reelPointer() == 95) {
		// Businessman combat won - end
		_pointerMode = 0;
		data.word(kWatchingtime) = 0;
		return;
	}

	if (routine.reelPointer() == 49)
		return; // Businessman combat end

	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 48) {
			data.byte(kMandead) = 2;	// before dead body
		} else if (nextReelPointer == 15) {
			nextReelPointer--;
			if (data.byte(kLastweapon) == 3) {
				// Shield on bus
				data.byte(kLastweapon) = (byte)-1;
				data.byte(kCombatcount) = 0;
				nextReelPointer = 51;
			} else {
				// No shield on businessman
				data.byte(kCombatcount)++;
				if (data.byte(kCombatcount) == 20) {
					data.byte(kCombatcount) = 0;
					nextReelPointer = 15;
				}
			}
		} else {
			// Businessman combat won
			if (nextReelPointer == 91) {
				turnPathOn(0);
				turnPathOn(1);
				turnPathOn(2);
				turnPathOff(3);
				_mansPath = 5;
				_finalDest = 5;
				findXYFromPath();
				_resetManXY = 1;
				nextReelPointer = 92;
			}
		}
		
		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
	routine.mapY = _mapY;
	if (routine.reelPointer() == 14) {
		data.word(kWatchingtime) = 0;
		_pointerMode = 2;
	}
}

void DreamBase::endGameSeq(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 51 && _introCount != 140) {
			_introCount++;
			textForEnd();
			nextReelPointer = 50;
		}

		routine.setReelPointer(nextReelPointer);
		if (nextReelPointer == 134) {
			fadeScreenDownHalf();
		} else if (nextReelPointer == 324) {
			fadeScreenDowns();
			_volumeTo = 7;
			_volumeDirection = 1;
		}

		if (nextReelPointer == 340)
			_getBack = 1;
	}

	showGameReel(&routine);
	routine.mapY = _mapY;
	
	if (routine.reelPointer() == 145) {
		routine.setReelPointer(146);
		rollEndCreditsGameWon();
	}
}

void DreamBase::poolGuard(ReelRoutine &routine) {
	if (routine.reelPointer() == 214 || routine.reelPointer() == 258) {
		// Combat over 2
		showGameReel(&routine);
		data.word(kWatchingtime) = 2;
		_pointerMode = 0;
		data.byte(kCombatcount)++;
		if (data.byte(kCombatcount) < 100)
			return; // doneover2
		data.word(kWatchingtime) = 0;
		data.byte(kMandead) = 2;
		return;
	} else if (routine.reelPointer() == 185) {
		// Combat over 1
		data.word(kWatchingtime) = 0;
		_pointerMode = 0;
		turnPathOn(0);
		turnPathOff(1);
		return;
	}

	if (routine.reelPointer() == 0)
		turnPathOn(0);	// first pool

	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		
		if (nextReelPointer != 122) {
			// Not end guard 1
			if (nextReelPointer == 147) {
				nextReelPointer--;
				if (data.byte(kLastweapon) == 1) {
					// Gun on pool
					data.byte(kLastweapon) = (byte)-1;
					nextReelPointer = 147;
				} else {
					// Gun not on pool
					data.byte(kCombatcount)++;
					if (data.byte(kCombatcount) == 40) {
						data.byte(kCombatcount) = 0;
						nextReelPointer = 220;
					}
				}
			}
		} else {
			nextReelPointer--;

			if (data.byte(kLastweapon) == 2) {
				// Axe on pool
				data.byte(kLastweapon) = (byte)-1;
				nextReelPointer = 122;
			} else {
				data.byte(kCombatcount)++;
				if (data.byte(kCombatcount) == 40) {
					data.byte(kCombatcount) = 0;
					nextReelPointer = 195;
				}
			}
		}
		
		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
	
	if (routine.reelPointer() != 121 && routine.reelPointer() != 146) {
		_pointerMode = 0;
		data.word(kWatchingtime) = 2;
	} else {
		_pointerMode = 2;
		data.word(kWatchingtime) = 0;
	}
}

} // End of namespace DreamGen
