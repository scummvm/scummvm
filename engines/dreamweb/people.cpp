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

static void (DreamGenContext::*reelCallbacks[57])() = {
	NULL, NULL,
	NULL, &DreamGenContext::edenInBath,
	NULL, NULL,
	NULL, NULL,
	&DreamGenContext::receptionist, NULL,
	NULL, &DreamGenContext::louis,
	&DreamGenContext::louisChair, &DreamGenContext::soldier1,
	&DreamGenContext::bossMan, &DreamGenContext::interviewer,
	&DreamGenContext::heavy, &DreamGenContext::manAsleep2,
	NULL, &DreamGenContext::drinker,
	&DreamGenContext::bartender, NULL,
	NULL, &DreamGenContext::attendant,
	&DreamGenContext::keeper, &DreamGenContext::candles1,
	&DreamGenContext::smallCandle, NULL,
	&DreamGenContext::copper, &DreamGenContext::poolGuard,
	NULL, &DreamGenContext::businessMan,
	&DreamGenContext::train, NULL,
	&DreamGenContext::mugger, &DreamGenContext::helicopter,
	NULL, NULL,
	&DreamGenContext::introMagic2, &DreamGenContext::candles2,
	&DreamGenContext::gates, &DreamGenContext::introMagic3,
	&DreamGenContext::introMonks1, &DreamGenContext::candles,
	&DreamGenContext::introMonks2, &DreamGenContext::handClap,
	&DreamGenContext::monkAndRyan, &DreamGenContext::endGameSeq,
	&DreamGenContext::priest, NULL,
	NULL, &DreamGenContext::alleyBarkSound,
	&DreamGenContext::foghornSound, &DreamGenContext::carParkDrip,
	&DreamGenContext::carParkDrip, &DreamGenContext::carParkDrip,
	&DreamGenContext::carParkDrip
};

static void (DreamGenContext::*reelCallbacksCPP[57])(ReelRoutine &) = {
	&DreamGenContext::gamer, &DreamGenContext::sparkyDrip,
	&DreamGenContext::eden, /*&DreamGenContext::edenInBath*/NULL,
	&DreamGenContext::sparky, &DreamGenContext::smokeBloke,
	&DreamGenContext::manAsleep, &DreamGenContext::drunk,
	/*&DreamGenContext::receptionist*/NULL, &DreamGenContext::genericPerson /*maleFan*/,
	&DreamGenContext::genericPerson /*femaleFan*/, /*&DreamGenContext::louis*/NULL,
	/*&DreamGenContext::louisChair*/NULL, /*&DreamGenContext::soldier1*/NULL,
	/*&DreamGenContext::bossMan*/NULL, /*&DreamGenContext::interviewer*/NULL,
	/*&DreamGenContext::heavy*/NULL, /*&DreamGenContext::manAsleep2*/NULL,
	&DreamGenContext::genericPerson /*manSatStill*/, /*&DreamGenContext::drinker*/NULL,
	/*&DreamGenContext::bartender*/NULL, &DreamGenContext::genericPerson /*otherSmoker*/,
	&DreamGenContext::genericPerson /*tattooMan*/, /*&DreamGenContext::attendant*/NULL,
	/*&DreamGenContext::keeper*/NULL, /*&DreamGenContext::candles1*/NULL,
	/*&DreamGenContext::smallcandle*/NULL, &DreamGenContext::security,
	/*&DreamGenContext::copper*/NULL, /*&DreamGenContext::poolGuard*/NULL,
	&DreamGenContext::rockstar, /*&DreamGenContext::businessMan*/NULL,
	/*&DreamGenContext::train*/NULL, &DreamGenContext::genericPerson /*aide*/,
	/*&DreamGenContext::mugger*/NULL, /*&DreamGenContext::helicopter*/NULL,
	&DreamGenContext::introMagic1, &DreamGenContext::introMusic,
	/*&DreamGenContext::introMagic2*/NULL, /*&DreamGenContext::candles2*/NULL,
	/*&DreamGenContext::gates*/NULL, /*&DreamGenContext::introMagic3*/NULL,
	/*&DreamGenContext::intromonks1*/NULL, /*&DreamGenContext::candles*/NULL,
	/*&DreamGenContext::intromonks2*/NULL, /*&DreamGenContext::handClap*/NULL,
	/*&DreamGenContext::monkAndRyan*/NULL, /*&DreamGenContext::endGameSeq*/NULL,
	/*&DreamGenContext::priest*/NULL, &DreamGenContext::madman,
	&DreamGenContext::madmansTelly, /*&DreamGenContext::alleyBarkSound*/NULL,
	/*&DreamGenContext::foghornSound*/NULL, /*&DreamGenContext::carParkDrip*/NULL,
	/*&DreamGenContext::carParkDrip*/NULL, /*&DreamGenContext::carParkDrip*/NULL,
	/*&DreamGenContext::carParkDrip*/NULL
};

void DreamGenContext::updatePeople() {
	data.word(kListpos) = kPeoplelist;
	memset(getSegment(data.word(kBuffers)).ptr(kPeoplelist, 12 * sizeof(People)), 0xff, 12 * sizeof(People));
	++data.word(kMaintimer);

	// The original callbacks are called with es:bx pointing to their reelRoutine entry.
	// The new callbacks take a mutable ReelRoutine parameter.

	es = cs;
	ReelRoutine *r = (ReelRoutine *)cs.ptr(kReelroutines, 0);

	for (int i = 0; r[i].reallocation != 255; ++i) {
		bx = kReelroutines + 8*i;
		if (r[i].reallocation == data.byte(kReallocation) &&
		        r[i].mapX == data.byte(kMapx) &&
		        r[i].mapY == data.byte(kMapy)) {
			if (reelCallbacks[i]) {
				assert(!reelCallbacksCPP[i]);
				(this->*(reelCallbacks[i]))();
			} else {
				assert(reelCallbacksCPP[i]);
				(this->*(reelCallbacksCPP[i]))(r[i]);
			}
		}
	}
}

void DreamGenContext::madmanText() {
	if (isCD()) {
		if (data.byte(kSpeechcount) >= 63)
			return;
		if (data.byte(kCh1playing) != 255)
			return;
		al = data.byte(kSpeechcount);
		++data.byte(kSpeechcount);
	} else {
		if (data.byte(kCombatcount) >= 61)
			return;
		al = data.byte(kCombatcount);
		_and(al, 3);
		if (!flags.z())
			return;
		al = data.byte(kCombatcount) / 4;
	}
	setupTimedTemp(47 + al, 82, 72, 80, 90, 1);
}

void DreamGenContext::madman(ReelRoutine &routine) {
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
			data.byte(kSpeechcount) = 0;
		}
		++newReelPointer;
		if (newReelPointer == 294) {
			if (data.byte(kWongame) != 1) {
				data.byte(kWongame) = 1;
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
	routine.mapX = data.byte(kMapx);
	madMode();
}

void DreamGenContext::madMode() {
	data.word(kWatchingtime) = 2;
	data.byte(kPointermode) = 0;
	if (data.byte(kCombatcount) < (isCD() ? 65 : 63))
		return;
	if (data.byte(kCombatcount) >= (isCD() ? 70 : 68))
		return;
	data.byte(kPointermode) = 2;
}

void DreamGenContext::addToPeopleList() {
	addToPeopleList((ReelRoutine *)es.ptr(bx, sizeof(ReelRoutine)));
}

void DreamGenContext::addToPeopleList(ReelRoutine *routine) {
	uint16 routinePointer = (const uint8 *)routine - cs.ptr(0, 0);

	People *people = (People *)getSegment(data.word(kBuffers)).ptr(data.word(kListpos), sizeof(People));
	people->setReelPointer(routine->reelPointer());
	people->setRoutinePointer(routinePointer);
	people->b4 = routine->b7;
	data.word(kListpos) += sizeof(People);
}

void DreamGenContext::checkSpeed() {
	ReelRoutine *routine = (ReelRoutine *)es.ptr(bx, sizeof(ReelRoutine));
	flags._z = checkSpeed(*routine);
}

bool DreamGenContext::checkSpeed(ReelRoutine &routine) {
	if (data.byte(kLastweapon) != (uint8)-1)
		return true;
	++routine.counter;
	if (routine.counter != routine.period)
		return false;
	routine.counter = 0;
	return true;
}

void DreamGenContext::sparkyDrip(ReelRoutine &routine) {
	if (checkSpeed(routine))
		playChannel0(14, 0);
}

void DreamGenContext::genericPerson(ReelRoutine &routine) {
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::gamer(ReelRoutine &routine) {
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

void DreamGenContext::eden(ReelRoutine &routine) {
	if (data.byte(kGeneraldead))
		return;
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::sparky(ReelRoutine &routine) {
	if (data.word(kCard1money))
		routine.b7 = 3;
	if (checkSpeed(routine)) {
		if (routine.reelPointer() != 34) {
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

void DreamGenContext::rockstar(ReelRoutine &routine) {
	if ((routine.reelPointer() == 303) || (routine.reelPointer() == 118)) {
		data.byte(kNewlocation) = 45;
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
		data.byte(kPointermode) = 2;
		data.word(kWatchingtime) = 0;
	} else {
		data.word(kWatchingtime) = 2;
		data.byte(kPointermode) = 0;
		routine.mapY = data.byte(kMapy);
	}
}

void DreamGenContext::madmansTelly(ReelRoutine &routine) {
	uint16 nextReelPointer = routine.reelPointer() + 1;
	if (nextReelPointer == 307)
		nextReelPointer = 300;
	routine.setReelPointer(nextReelPointer);
	showGameReel(&routine);
}


void DreamGenContext::smokeBloke(ReelRoutine &routine) {
	if (data.byte(kRockstardead) == 0) {
		if (routine.b7 & 128)
			setLocation(5);
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

void DreamGenContext::manAsleep(ReelRoutine &routine) {
	routine.b7 &= 127;
	showGameReel(&routine);
	addToPeopleList(&routine);
}


void DreamGenContext::drunk(ReelRoutine &routine) {
	if (data.byte(kGeneraldead))
		return;
	routine.b7 &= 127;
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::introMagic1(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 145)
			nextReelPointer = 121;
		routine.setReelPointer(nextReelPointer);
		if (nextReelPointer == 121) {
			++data.byte(kIntrocount);
			push(es);
			push(bx);
			intro1Text();
			bx = pop();
			es = pop();
			if (data.byte(kIntrocount) == 8) {
				data.byte(kMapy) += 10;
				data.byte(kNowinnewroom) = 1;
			}
		}
	}
	showGameReel(&routine);
}

void DreamGenContext::introMusic(ReelRoutine &routine) {
}

void DreamGenContext::security(ReelRoutine &routine) {
	if (routine.reelPointer() == 32) {
		if (data.byte(kLastweapon) == 1) {
			data.word(kWatchingtime) = 10;
			if ((data.byte(kManspath) == 9) && (data.byte(kFacing) == 0)) {
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

} /*namespace dreamgen */

