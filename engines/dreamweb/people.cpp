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
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, &DreamGenContext::poolGuard,
	NULL, &DreamGenContext::businessMan,
	NULL, NULL,
	&DreamGenContext::mugger, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, &DreamGenContext::endGameSeq,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL, NULL,
	NULL
};

static void (DreamGenContext::*reelCallbacksCPP[57])(ReelRoutine &) = {
	&DreamGenContext::gamer, &DreamGenContext::sparkyDrip,
	&DreamGenContext::eden, &DreamGenContext::edenInBath,
	&DreamGenContext::sparky, &DreamGenContext::smokeBloke,
	&DreamGenContext::manAsleep, &DreamGenContext::drunk,
	&DreamGenContext::receptionist, &DreamGenContext::genericPerson /*maleFan*/,
	&DreamGenContext::genericPerson /*femaleFan*/, &DreamGenContext::louis,
	&DreamGenContext::louisChair, &DreamGenContext::soldier1,
	&DreamGenContext::bossMan, &DreamGenContext::interviewer,
	&DreamGenContext::heavy, &DreamGenContext::manAsleep /*manAsleep2*/,
	&DreamGenContext::genericPerson /*manSatStill*/, &DreamGenContext::drinker,
	&DreamGenContext::bartender, &DreamGenContext::genericPerson /*otherSmoker*/,
	&DreamGenContext::genericPerson /*tattooMan*/, &DreamGenContext::attendant,
	&DreamGenContext::keeper, &DreamGenContext::candles1,
	&DreamGenContext::smallCandle, &DreamGenContext::security,
	&DreamGenContext::copper, /*&DreamGenContext::poolGuard*/NULL,
	&DreamGenContext::rockstar, /*&DreamGenContext::businessMan*/NULL,
	&DreamGenContext::train, &DreamGenContext::genericPerson /*aide*/,
	/*&DreamGenContext::mugger*/NULL, &DreamGenContext::helicopter,
	&DreamGenContext::introMagic1, &DreamGenContext::introMusic,
	&DreamGenContext::introMagic2, &DreamGenContext::candles2,
	&DreamGenContext::gates, &DreamGenContext::introMagic3,
	&DreamGenContext::introMonks1, &DreamGenContext::candles,
	&DreamGenContext::introMonks2, &DreamGenContext::handClap,
	&DreamGenContext::monkAndRyan, /*&DreamGenContext::endGameSeq*/NULL,
	&DreamGenContext::priest, &DreamGenContext::madman,
	&DreamGenContext::madmansTelly, &DreamGenContext::alleyBarkSound,
	&DreamGenContext::foghornSound, &DreamGenContext::carParkDrip,
	&DreamGenContext::carParkDrip, &DreamGenContext::carParkDrip,
	&DreamGenContext::carParkDrip
};

void DreamGenContext::updatePeople() {
	data.word(kListpos) = kPeoplelist;
	memset(getSegment(data.word(kBuffers)).ptr(kPeoplelist, 12 * sizeof(People)), 0xff, 12 * sizeof(People));
	++data.word(kMaintimer);

	// The original callbacks are called with es:bx pointing to their reelRoutine entry.
	// The new callbacks take a mutable ReelRoutine parameter.

	es = data;
	ReelRoutine *r = (ReelRoutine *)data.ptr(kReelroutines, 0);

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
	uint16 routinePointer = (const uint8 *)routine - data.ptr(0, 0);

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

void DreamGenContext::manAsleep(ReelRoutine &routine) {
	routine.b7 &= 127;
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::attendant(ReelRoutine &routine) {
	showGameReel(&routine);
	addToPeopleList(&routine);
	if (routine.b7 & 128)
		data.byte(kTalkedtoattendant) = 1;
}

void DreamGenContext::keeper(ReelRoutine &routine) {
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

void DreamGenContext::drunk(ReelRoutine &routine) {
	if (data.byte(kGeneraldead))
		return;
	routine.b7 &= 127;
	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::interviewer(ReelRoutine &routine) {
	if (data.word(kReeltowatch) == 68)
		routine.incReelPointer();

	if (routine.reelPointer() != 250 && routine.reelPointer() != 259 && checkSpeed(routine))
		routine.incReelPointer();
	
	showGameReel(&routine);
}

void DreamGenContext::drinker(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		routine.incReelPointer();

		if ( routine.reelPointer() == 115 ||
			(routine.reelPointer() == 106 && engine->randomNumber() >= 3))
			routine.setReelPointer(105);
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::alleyBarkSound(ReelRoutine &routine) {
	uint16 prevReelPointer = routine.reelPointer() - 1;
	if (prevReelPointer == 0) {
		playChannel1(14);
		routine.setReelPointer(1000);
	} else {
		routine.setReelPointer(prevReelPointer);
	}
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

void DreamGenContext::introMagic2(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 216)
			nextReelPointer = 192;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamGenContext::introMagic3(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 218)
			data.byte(kGetback) = 1;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
	routine.mapX = data.byte(kMapx);
}

void DreamGenContext::candles1(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 44)
			nextReelPointer = 39;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamGenContext::candles2(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 238)
			nextReelPointer = 233;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamGenContext::smallCandle(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 37)
			nextReelPointer = 25;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamGenContext::introMusic(ReelRoutine &routine) {
}

void DreamGenContext::candles(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 167)
			nextReelPointer = 162;
		routine.setReelPointer(nextReelPointer);
	}
	showGameReel(&routine);
}

void DreamGenContext::gates(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 116)
			playChannel1(17);
		if (nextReelPointer >= 110)
			routine.period = 2;
		if (nextReelPointer == 120) {
			data.byte(kGetback) = 1;
			nextReelPointer = 119;
		}
		routine.setReelPointer(nextReelPointer);
		push(es);
		push(bx);
		ax = nextReelPointer;
		intro3Text();
		bx = pop();
		es = pop();
	}
	showGameReel(&routine);
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

void DreamGenContext::edenInBath(ReelRoutine &routine) {
	if (data.byte(kGeneraldead) == 0 || data.byte(kSartaindead) != 0)
		return;

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::louis(ReelRoutine &routine) {
	if (data.byte(kRockstardead) != 0)
		return;

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::handClap(ReelRoutine &routine) {
}

void DreamGenContext::carParkDrip(ReelRoutine &routine) {
	if (!checkSpeed(routine))
		return; // cantdrip2

	playChannel1(14);
}

void DreamGenContext::foghornSound(ReelRoutine &routine) {
	if (engine->randomNumber() == 198)
		playChannel1(13);
}

void DreamGenContext::train(ReelRoutine &routine) {
	// The original code has logic for this, but it is disabled
}

void DreamGenContext::louisChair(ReelRoutine &routine) {
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

void DreamGenContext::bossMan(ReelRoutine &routine) {
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

void DreamGenContext::priest(ReelRoutine &routine) {
	if (routine.reelPointer() == 8)
		return; // priestspoken

	data.byte(kPointermode) = 0;
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

void DreamGenContext::monkAndRyan(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 83) {
			data.byte(kIntrocount)++;
			textForMonk();
			nextReelPointer = 77;

			if (data.byte(kIntrocount) == 57) {
				data.byte(kGetback) = 1;
				return;
			}
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
}

void DreamGenContext::copper(ReelRoutine &routine) {
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

void DreamGenContext::introMonks1(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;

		if (nextReelPointer == 80) {
			data.byte(kMapy) += 10;
			data.byte(kNowinnewroom) = 1;
			showGameReel(&routine);
			return;
		} else if (nextReelPointer == 30) {
			data.byte(kMapy) -= 10;
			data.byte(kNowinnewroom) = 1;
			nextReelPointer = 51;
		}

		routine.setReelPointer(nextReelPointer);

		if (nextReelPointer ==  5 || nextReelPointer == 15 ||
			nextReelPointer == 25 || nextReelPointer == 61 ||
			nextReelPointer == 71) {
			// Wait step
			push(es);
			push(bx);
			intro2Text();
			bx = pop();
			es = pop();
			routine.counter = (uint8)-20;
		}
	}

	showGameReel(&routine);
	routine.mapY = data.byte(kMapy);
}

void DreamGenContext::introMonks2(ReelRoutine &routine) {
	if (checkSpeed(routine)) {
		uint16 nextReelPointer = routine.reelPointer() + 1;
		if (nextReelPointer == 87) {
			data.byte(kIntrocount)++;
			push(es);
			push(bx);
			monks2text();
			bx = pop();
			es = pop();

			if (data.byte(kIntrocount), 19)
				nextReelPointer = 87;
			else
				nextReelPointer = 74;
		}

		if (nextReelPointer == 110) {
			data.byte(kIntrocount)++;
			push(es);
			push(bx);
			monks2text();
			bx = pop();
			es = pop();
		
			if (data.byte(kIntrocount) == 35)
				nextReelPointer = 111;
			else
				nextReelPointer = 98;
		} else if (nextReelPointer == 176) {
			data.byte(kGetback) = 1;
		} else if (nextReelPointer == 125) {
			nextReelPointer = 140;
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
}

void DreamGenContext::soldier1(ReelRoutine &routine) {
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
		if (data.byte(kManspath) == 2 && data.byte(kFacing) == 4)
			routine.incReelPointer();
		data.byte(kLastweapon) = 0xFF;
		data.byte(kCombatcount) = 0;
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::receptionist(ReelRoutine &routine) {
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

void DreamGenContext::bartender(ReelRoutine &routine) {
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

void DreamGenContext::heavy(ReelRoutine &routine) {
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
	} else if (data.byte(kLastweapon) == 1 && data.byte(kManspath) == 5 && data.byte(kFacing) == 4) {
		// Heavy wait
		data.byte(kLastweapon) = (byte)-1;
		routine.incReelPointer();
		data.byte(kCombatcount) = 0;
	}

	showGameReel(&routine);
	addToPeopleList(&routine);
}

void DreamGenContext::helicopter(ReelRoutine &routine) {
	if (routine.reelPointer() == 203) {
		// Won helicopter
		data.byte(kPointermode) = 0;
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
		} else {
			// Not waiting helicopter
			data.byte(kPointermode) = 0;
			data.word(kWatchingtime) = 2;
		}

		routine.setReelPointer(nextReelPointer);
	}

	showGameReel(&routine);
	routine.mapX = data.byte(kMapx);
	if (routine.reelPointer() < 9 && data.byte(kCombatcount) >= 7) {
		data.byte(kPointermode) = 2;
		data.word(kWatchingtime) = 0;
	} else {
		// Not waiting helicopter
		data.byte(kPointermode) = 0;
		data.word(kWatchingtime) = 2;
	}
}

} // End of namespace DreamGen
