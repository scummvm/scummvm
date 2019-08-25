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

#include "startrek/room.h"

namespace StarTrek {

#define OBJECT_GUARD_1 8
#define OBJECT_GUARD_2 9
#define OBJECT_BRIG 10
#define OBJECT_BOMB 11
#define OBJECT_WIRE 12
#define OBJECT_13 13

#define HOTSPOT_BUTTON 0x20
#define HOTSPOT_DOOR   0x21
#define HOTSPOT_OPEN_BRIG 0x22

#define GUARDSTAT_UP 0
#define GUARDSTAT_STUNNED 1
#define GUARDSTAT_DEAD 2
#define GUARDSTAT_TIED 8

void Room::tug2Tick1() {
	playVoc("TUG2LOOP");

	if (!_awayMission->tug.haveBomb)
		loadActorAnim2(OBJECT_BOMB, "t2bomb", -3, 1, 0);
	if (!_awayMission->tug.gotWires)
		loadActorAnim2(OBJECT_WIRE, "t2wire", 0, 1, 0);

	if (!_awayMission->tug.brigForceFieldDown)
		loadActorAnim2(OBJECT_BRIG, "fld01", 0, 0, 0);
	else
		loadActorAnim2(OBJECT_BRIG, "fld10d", 0, 0, 0);
	if (_awayMission->tug.field35 == 6)
		loadActorAnim2(OBJECT_BRIG, "zapdon", 0, 0, 0);

	// Security guy 1
	if (_awayMission->tug.guard1Status == 8)
		loadActorAnim2(OBJECT_GUARD_1, "p5tied", 0xe0, 0xb1, 0);
	else if (_awayMission->tug.guard1Status == 1 && _awayMission->tug.brigForceFieldDown) {
		loadActorAnim2(OBJECT_GUARD_1, "p5tied", 0xe0, 0xb1, 0);
		_awayMission->tug.guard1Status = 8;
	}
	if (_awayMission->tug.guard1Status == 1 && !_awayMission->tug.brigForceFieldDown) {
		_awayMission->tug.guard1Status = 0;
		_awayMission->tug.crewmanKilled[OBJECT_KIRK] = false;
		_awayMission->tug.crewmanKilled[OBJECT_SPOCK] = false;
		_awayMission->tug.crewmanKilled[OBJECT_MCCOY] = false;
		if (!_awayMission->redshirtDead)
			_awayMission->tug.crewmanKilled[OBJECT_REDSHIRT] = false;
		_awayMission->tug.field5b = 0;
	}
	if (_awayMission->tug.guard1Status == 0)
		loadActorAnim2(OBJECT_GUARD_1, "p5redy", 0xe0, 0xb1, 0);
	if (_awayMission->tug.guard1Status == 2)
		loadActorAnim2(OBJECT_GUARD_1, "p5hat", 0xe0, 0xb1, 0);

	// Security guy 2
	if (_awayMission->tug.guard2Status == 8)
		loadActorAnim2(OBJECT_GUARD_2, "p6tied", 0xfb, 0xc5, 0);
	else if (_awayMission->tug.guard2Status == 1 && _awayMission->tug.brigForceFieldDown) {
		loadActorAnim2(OBJECT_GUARD_2, "p6tied", 0xfb, 0xc5, 0);
		_awayMission->tug.guard2Status = 8;
	}
	if (_awayMission->tug.guard2Status == 1 && !_awayMission->tug.brigForceFieldDown) {
		_awayMission->tug.guard2Status = 0;
		_awayMission->tug.crewmanKilled[OBJECT_KIRK] = false;
		_awayMission->tug.crewmanKilled[OBJECT_SPOCK] = false;
		_awayMission->tug.crewmanKilled[OBJECT_MCCOY] = false;
		if (!_awayMission->redshirtDead)
			_awayMission->tug.crewmanKilled[OBJECT_REDSHIRT] = false;
		_awayMission->tug.field5b = 0;
	}
	if (_awayMission->tug.guard2Status == 0)
		loadActorAnim2(OBJECT_GUARD_2, "p6draw", 0xfb, 0xc5, 0);
	if (_awayMission->tug.guard2Status == 2)
		loadActorAnim2(OBJECT_GUARD_2, "p5hat", 0xfb, 0xc5, 0); // FIXME


	if (_awayMission->tug.field4e == 0)
		loadActorAnim2(OBJECT_13, "wallht", 0, 0, 0);

	if (_awayMission->tug.field5b == 0) {
		_awayMission->disableWalking = true;
		playMidiMusicTracks(32, -1);
	}

	_awayMission->tug.field5b = 1;
	_awayMission->tug.kirkPhaserDrawn = 0;
}

void Room::tug2Tick60() {
	if (!_awayMission->tug.guard1Status || !_awayMission->tug.guard2Status) {
		_awayMission->timers[0] = 60;
		showText(TX_SPEAKER_ELASI_GUARD, 85, true, true);
		tug2ElasiReadyPhaser();
	}
}

void Room::tug2LookAtButton() {
	showDescription(11, true);
}

void Room::tug2LookAtMccoy() {
	showDescription(5, true);
}

void Room::tug2LookAtSpock() {
	showDescription(7, true);
}

void Room::tug2LookAtRedshirt() {
	showDescription(4, true);
}

void Room::tug2GetBomb() {
	if (_awayMission->tug.brigForceFieldDown) {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewman(OBJECT_KIRK, 0xc9, 0xa0, 12);
	} else
		showDescription(24, true);
}

void Room::tug2KirkReachedBomb() {
	loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 13);
}

void Room::tug2KirkGotBomb() {
	loadActorStandAnim(OBJECT_BOMB);
	giveItem(OBJECT_IBOMB);
	_awayMission->tug.haveBomb = true;
	_awayMission->disableInput = false;
}

void Room::tug2LookAtBomb() {
	showDescription(0, true);
}

void Room::tug2LookAtGuard1() {
	if (_awayMission->tug.guard1Status == 0)
		showDescription(1, true);
	else if (_awayMission->tug.guard1Status == 1)
		showDescription(13, true);
	else if (_awayMission->tug.guard1Status == 8)
		showDescription(15, true);
	else if (_awayMission->tug.guard1Status == 2)
		showDescription(106, true);
}

void Room::tug2LookAtGuard2() {
	if (_awayMission->tug.guard2Status == 0)
		showDescription(37, true);
	else if (_awayMission->tug.guard2Status == 1)
		showDescription(14, true);
	else if (_awayMission->tug.guard2Status == 8)
		showDescription(16, true);
	else if (_awayMission->tug.guard2Status == 2)
		showDescription(106, true);
}

void Room::tug2LookAtWires() {
	showDescription(10, true);
}

void Room::tug2UseSTricorderOnButton() {
	if (_awayMission->tug.field35 != 0)
		return;
	spockScan(DIR_E, 7, true, true);
}

void Room::tug2UseMccoyOnWires() {
	if (_awayMission->tug.field35 != 0)
		return;
	showText(TX_SPEAKER_MCCOY, 9, true);
}

void Room::tug2UseMccoyOnBomb() {
	showText(TX_SPEAKER_MCCOY, 11, true);
}

void Room::tug2UseRedshirtOnWires() {
	if (_awayMission->disableWalking || _awayMission->tug.field35 != 0)
		return;
	showText(TX_SPEAKER_CHRISTENSEN, 7, true, true);
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_N;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_REDSHIRT, 0xc9, 0xa0, 6);
}

void Room::tug2RedshirtReachedWires() {
	loadActorAnim2(OBJECT_REDSHIRT, "rusemn", -1, -1, 7);
}

void Room::tug2RedshirtDefusedBomb() {
	showDescription(TX_TUG2C001);
	showText(TX_SPEAKER_CHRISTENSEN, 11, true, true);
	_awayMission->tug.field35 = 4;
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_E;
	walkCrewman(OBJECT_REDSHIRT, 0x44, 0xc2, 8);
}

void Room::tug2RedshirtReturnedToPosition() {
	_awayMission->disableInput = false;
	_awayMission->tug.missionScore += 2;
}

void Room::tug2UseKirkOnWires() {
	if (_awayMission->disableWalking || _awayMission->tug.field35 != 0)
		return;
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 0xc9, 0xa0, 5);
}

void Room::tug2KirkReachedWires() {
	showText(TX_SPEAKER_KIRK,  3, true);
	showText(TX_SPEAKER_SPOCK, 23, true);

	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 0x66, 0xb8, 0);
	_awayMission->disableInput = false;
	tug2UseSpockOnWires();
}

void Room::tug2UseSpockOnWires() {
	if (_awayMission->disableWalking || _awayMission->tug.field35 != 0)
		return;
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
	walkCrewman(OBJECT_SPOCK, 0xc9, 0xa0, 9);
}

void Room::tug2SpockReachedWires() {
	loadActorAnim2(OBJECT_SPOCK, "suseme", -1, -1, 0);
	playVoc("WIRESNIP");
	showText(TX_SPEAKER_SPOCK, 24, true);
	_awayMission->tug.field35 = 4;

	if (_awayMission->tug.spockExaminedTransporter)
		showText(TX_SPEAKER_SPOCK, 8, true);

	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
	walkCrewman(OBJECT_SPOCK, 0x56, 0xa9, 11);
}

void Room::tug2SpockReturnedToPosition() {
	_awayMission->disableInput = false;
	_awayMission->tug.missionScore += 2;
}

void Room::tug2GetWires() {
	if (_awayMission->disableWalking)
		return;
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 0xc9, 0xa0, 28);
}

void Room::tug2KirkReachedWiresToGet() {
	loadActorAnim2(OBJECT_KIRK, "kuseme", -1, -1, 29);
}

void Room::tug2KirkGotWires() {
	if (_awayMission->tug.field35 == 0) {
		playMidiMusicTracks(2, -1);
		loadActorAnim2(OBJECT_BRIG, "zap", 0, 0, 1);
		playVoc("NM8EXPLO");
		loadActorAnim2(OBJECT_BOMB, "t2bbo", -3, 1, 0);
		playSoundEffectIndex(SND_PHASSHOT);
		playSoundEffectIndex(SND_BLANK_16);
		_awayMission->tug.field35 = 6;
		loadActorStandAnim(OBJECT_WIRE);
		giveItem(OBJECT_IWIRING);
		_awayMission->tug.gotWires = true;
		_awayMission->disableInput = false;
		_awayMission->tug.missionScore += 3;
	} else {
		loadActorStandAnim(OBJECT_WIRE);
		giveItem(OBJECT_IWIRING);
		_awayMission->tug.gotWires = true;
		_awayMission->disableInput = false;
		_awayMission->tug.missionScore += 3;
	}
}

void Room::tug2UseKirkOnButton() {
	if (_awayMission->disableWalking)
		return;
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 0xc9, 0xa0, 14);
}

void Room::tug2KirkReachedButton() {
	loadActorAnim2(OBJECT_KIRK, "kusehe", -1, -1, 15);
	playSoundEffectIndex(SND_07);
}

void Room::tug2UseSpockOnButton() {
	if (_awayMission->disableWalking)
		return;
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
	walkCrewman(OBJECT_SPOCK, 0xc9, 0xa0, 16);
}

void Room::tug2SpockReachedButton() {
	loadActorAnim2(OBJECT_SPOCK, "susehe", -1, -1, 17);
	playSoundEffectIndex(SND_07);
}

void Room::tug2UseMccoyOnButton() {
	if (_awayMission->disableWalking)
		return;
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_E;
	walkCrewman(OBJECT_MCCOY, 0xc9, 0xa0, 18);
}

void Room::tug2MccoyReachedButton() {
	loadActorAnim2(OBJECT_MCCOY, "musehe", -1, -1, 19);
	playSoundEffectIndex(SND_07);
}

void Room::tug2UseRedshirtOnButton() {
	if (_awayMission->disableWalking)
		return;
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_E;
	walkCrewman(OBJECT_REDSHIRT, 0xc9, 0xa0, 20);
}

void Room::tug2RedshirtReachedButton() {
	loadActorAnim2(OBJECT_REDSHIRT, "rusehe", -1, -1, 21);
	playSoundEffectIndex(SND_07);
}

void Room::tug2TurnedOffForceField() {
	_awayMission->disableInput = false;
	if (_awayMission->tug.field35 == 0) {
		playMidiMusicTracks(2, -1);
		loadActorAnim2(OBJECT_BRIG, "zap", 0, 0, 1);
		playVoc("NM8EXPLO");
		loadActorAnim2(OBJECT_BOMB, "t2bbo", -3, 1, 0);
		// FIXME: doesn't play "PHASSHOT" sound effect like when kirk removes wires?
		_awayMission->tug.field35 = 6;
	} else if (_awayMission->tug.field35 == 4 && !_awayMission->tug.brigForceFieldDown) {
		_awayMission->tug.brigForceFieldDown = true;
		loadActorAnim2(OBJECT_BRIG, "fld10", 0, 0, 2);
		_awayMission->tug.missionScore += 6;
		playVoc("EFX15");
	}
}

void Room::tug2PrisonersDead() {
	loadActorAnim2(OBJECT_BRIG, "zapdon", 0, 0, 0);
	showDescription(26, true);
}

void Room::tug2PrisonersReleased() {
	loadActorAnim2(OBJECT_BRIG, "fld10d", 0, 0, 0);
	showText(TX_SPEAKER_MASADA_CREWMAN, 29, true);
	showText(TX_SPEAKER_MASADA_CREWMAN, 30, true);
	showText(TX_SPEAKER_MASADA_CREWMAN, 28, true);
	_awayMission->tug.savedPrisoners = true;
}

void Room::tug2UsePhaserOnBrig() {
	if (_awayMission->tug.field35 == 0)
		showText(TX_SPEAKER_SPOCK, 6, true);
	else if (_awayMission->tug.brigForceFieldDown) {
		// BUGFIX: this function had two implementations; one for firing on the brig, and
		// one for firing on the masada crewman (who replaces the brig object). The first
		// took priority, meaning the latter code never ran. That's fixed here.
		showText(TX_SPEAKER_MASADA_CREWMAN, 25, true);
	}
}

void Room::tug2ElasiReadyPhaser() {
	if (_awayMission->tug.guard1Status == 0 && _awayMission->tug.field59 == 0) {
		loadActorAnim2(OBJECT_GUARD_1, "p5draw", -1, -1, 0);
		_awayMission->tug.field59++;
	}
}

void Room::tug2CheckEndFirefight() {
	if (_awayMission->tug.guard1Status == GUARDSTAT_UP || _awayMission->tug.guard2Status == GUARDSTAT_UP)
		return;
	_awayMission->disableWalking = false;
	if (_awayMission->tug.kirkPhaserDrawn)
		loadActorStandAnim(OBJECT_KIRK);
}

void Room::tug2UseStunPhaserOnGuard1() {
	if (_awayMission->tug.guard1Status != GUARDSTAT_UP)
		tug2KirkShootGuard1();
	else {
		tug2ElasiReadyPhaser();
		_awayMission->disableInput = true;
		if (_awayMission->tug.kirkPhaserDrawn)
			tug2KirkShootGuard1();
		else {
			loadActorAnim2(OBJECT_KIRK, "kdrawe", -1, -1, 22);
			_awayMission->tug.kirkPhaserDrawn = true;
		}
	}
}

void Room::tug2KirkShootGuard1() {
	if (_awayMission->tug.guard1Status == GUARDSTAT_UP) {
		loadActorAnim2(OBJECT_GUARD_1, "p5stun", -1, -1, 0);
		_awayMission->tug.guard1Status = GUARDSTAT_STUNNED;
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t2beem03", 5);
		tug2CheckEndFirefight();
	}

	_awayMission->disableInput = false;
}

void Room::tug2UseStunPhaserOnGuard2() {
	if (_awayMission->tug.guard2Status != GUARDSTAT_UP)
		tug2KirkShootGuard2();
	else {
		tug2ElasiReadyPhaser();
		_awayMission->disableInput = true;
		if (_awayMission->tug.kirkPhaserDrawn)
			tug2KirkShootGuard2();
		else {
			loadActorAnim2(OBJECT_KIRK, "kdrawe", -1, -1, 23);
			_awayMission->tug.kirkPhaserDrawn = true;
		}
	}
}

void Room::tug2KirkShootGuard2() {
	if (_awayMission->tug.guard2Status == GUARDSTAT_UP) {
		loadActorAnim2(OBJECT_GUARD_2, "p6stun", -1, -1, 0);
		_awayMission->tug.guard2Status = GUARDSTAT_STUNNED;
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t2beem02", 5);
		tug2CheckEndFirefight();
	}

	_awayMission->disableInput = false;
}

void Room::tug2UseKillPhaserOnGuard1() {
	if (_awayMission->tug.guard1Status != GUARDSTAT_UP)
		tug2KirkKillGuard1();
	else {
		tug2ElasiReadyPhaser();
		_awayMission->disableInput = true;
		if (_awayMission->tug.kirkPhaserDrawn)
			tug2KirkKillGuard1();
		else {
			loadActorAnim2(OBJECT_KIRK, "kdrawe", -1, -1, 24);
			_awayMission->tug.kirkPhaserDrawn = true;
			_awayMission->tug.brigElasiPhasersOnKill = true;
		}
	}
}

void Room::tug2KirkKillGuard1() {
	if (_awayMission->tug.guard1Status == GUARDSTAT_UP) {
		loadActorAnim2(OBJECT_GUARD_1, "p5kill", -1, -1, 0);
		_awayMission->tug.guard1Status = GUARDSTAT_DEAD;
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t2beem05", 5);
		tug2CheckEndFirefight();
	}

	_awayMission->disableInput = false;
}

void Room::tug2UseKillPhaserOnGuard2() {
	if (_awayMission->tug.guard2Status != GUARDSTAT_UP)
		tug2KirkKillGuard2();
	else {
		tug2ElasiReadyPhaser();
		_awayMission->disableInput = true;
		if (_awayMission->tug.kirkPhaserDrawn)
			tug2KirkKillGuard2();
		else {
			loadActorAnim2(OBJECT_KIRK, "kdrawe", -1, -1, 25);
			_awayMission->tug.kirkPhaserDrawn = true;
			_awayMission->tug.brigElasiPhasersOnKill = true;
		}
	}
}

void Room::tug2KirkKillGuard2() {
	if (_awayMission->tug.guard2Status == GUARDSTAT_UP) {
		loadActorAnim2(OBJECT_GUARD_2, "p6kill", -1, -1, 0);
		_awayMission->tug.guard2Status = GUARDSTAT_DEAD;
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t2beem04", 5);
		tug2CheckEndFirefight();
	}

	_awayMission->disableInput = false;
}

void Room::tug2UsePhaserOnWelder() {
	loseItem(OBJECT_IPWE);
	giveItem(OBJECT_IPWF);
	showDescription(6, true);

	// BUGFIX: this following line didn't exist, despite it existing in TUG1; meaning this
	// was supposed to give points, but it only did in a specific room.
	_awayMission->tug.missionScore += 3;
}

void Room::tug2UseWelderOnWireScraps() {
	showDescription(9, true);
	loseItem(OBJECT_IWIRSCRP);
}

void Room::tug2UseWelderOnMetalScraps() {
	showText(TX_SPEAKER_SPOCK, 112, true);
	loseItem(OBJECT_IJNKMETL);
	giveItem(OBJECT_ICOMBBIT);
}

void Room::tug2UseCombBitOnTransmogrifier() {
	showText(TX_SPEAKER_SPOCK, 21, true);
	loseItem(OBJECT_ICOMBBIT);
	loseItem(OBJECT_IRT);
	giveItem(OBJECT_IRTWB);
}

void Room::tug2ShotByElasi() {
	showDescription(25, true);
	showGameOverMenu();

	// Unused: additional textbox that says "***Game over man!***"
}

void Room::tug2WalkToDoor() {
	walkCrewman(OBJECT_KIRK, 0x0b, 0xab, 0);
}

void Room::tug2LookAtDoor() {
	showDescription(12, true);
}

void Room::tug2LookAtKirk() {
	showDescription(3, true);
}

void Room::tug2TalkToKirk() {
	if (_awayMission->tug.field35 == 6)
		showText(TX_SPEAKER_KIRK, 1, true);
	else
		showText(TX_SPEAKER_KIRK, 2, true);
}

void Room::tug2TalkToMccoy() {
	if (_awayMission->tug.field35 == 6)
		showText(TX_SPEAKER_MCCOY, 18, true);
	else
		showText(TX_SPEAKER_MCCOY, 10, true);
}

void Room::tug2TalkToRedshirt() {
	// BUGFIX: the original game has two implementations of this function; the first takes
	// precedence, however, it's just generic, unhelpful text. The second implementation
	// is more interesting, so that one is used instead.
	if (_awayMission->tug.field35 == 6)
		showText(TX_SPEAKER_CHRISTENSEN, 4, true, true);
	else
		showText(TX_SPEAKER_CHRISTENSEN, TX_TUG2J003);

	// Original text: "I'm just a security officer, sir."
	//showText(TX_SPEAKER_CHRISTENSEN, TX_TUG2L002);
}

void Room::tug2TalkToSpock() {
	if (_awayMission->tug.field35 == 6)
		showText(TX_SPEAKER_SPOCK, 5, true);
	else
		showText(TX_SPEAKER_SPOCK, 20, true);
}

void Room::tug2UseCommunicator() {
	showText(TX_SPEAKER_SPOCK, TX_TUG1_011);
}

void Room::tug2DetermineElasiShooter() {
	if (_awayMission->tug.guard2Status == GUARDSTAT_UP)
		_roomVar.tug.shootingObject = OBJECT_GUARD_2;
	if (_awayMission->tug.guard1Status == GUARDSTAT_UP)
		_roomVar.tug.shootingObject = OBJECT_GUARD_1;
}

// Elasi shoots someone
void Room::tug2Timer0Expired() {
	if (_awayMission->tug.guard1Status != GUARDSTAT_UP && _awayMission->tug.guard2Status != GUARDSTAT_UP)
		return;

	_roomVar.tug.elasiPhaserOnKill = 0;
	if (_awayMission->tug.brigElasiPhasersOnKill)
		_roomVar.tug.elasiPhaserOnKill = 1;

	switch (_awayMission->tug.elasiTargetIndex) {
	case 0:
		_awayMission->tug.elasiTargetIndex++;
		if (_awayMission->tug.guard2Status != GUARDSTAT_UP) {
			// BUGFIX: reset the timer to allow guard 1 to continue if guard 2 is down
			_awayMission->timers[0] = 60;
			return;
		}

		// Guard misses and shoots wall
		loadActorAnim2(OBJECT_13, "wallht", 0, 0, 0);
		showBitmapFor5Ticks("t2beem00", 5);
		playSoundEffectIndex(SND_PHASSHOT);
		loadActorAnim2(OBJECT_GUARD_2, "p6fire", -1, -1, 0);
		_awayMission->tug.field4e = _roomVar.tug.elasiPhaserOnKill;
		_awayMission->timers[0] = 60;
		break;

	case 1:
		// Guard shoots redshirt
		_awayMission->tug.elasiTargetIndex++;
		if (!_awayMission->tug.crewmanKilled[OBJECT_REDSHIRT]) {
			tug2DetermineElasiShooter();
			_roomVar.tug.shootingTarget = OBJECT_REDSHIRT;
			_awayMission->timers[0] = 60;
			tug2GuardShootsCrewman();
		}
		break;

	case 2:
		// Guard shoots spock (or kirk)
		_awayMission->tug.elasiTargetIndex++;
		_roomVar.tug.shootKirkOverride = false;
		if (_awayMission->tug.brigElasiPhasersOnKill)
			_roomVar.tug.shootKirkOverride = true;
		tug2DetermineElasiShooter();
		if (_roomVar.tug.shootKirkOverride)
			_roomVar.tug.shootingTarget = OBJECT_KIRK;
		else
			_roomVar.tug.shootingTarget = OBJECT_SPOCK;
		_awayMission->timers[0] = 60;
		tug2GuardShootsCrewman();
		break;

	case 3:
		// Guard shoots Mccoy
		_awayMission->tug.elasiTargetIndex++;
		if (!_awayMission->tug.crewmanKilled[OBJECT_MCCOY]) {
			tug2DetermineElasiShooter();
			_roomVar.tug.shootingTarget = OBJECT_MCCOY;
			_awayMission->timers[0] = 80;
			tug2GuardShootsCrewman();
		}
		break;

	case 4:
		// Guard shoots kirk (or Spock)
		_awayMission->tug.elasiTargetIndex++;
		if (_roomVar.tug.shootKirkOverride) {
			tug2DetermineElasiShooter();
			_roomVar.tug.shootingTarget = OBJECT_SPOCK;
			_awayMission->timers[2] = 40;
			tug2GuardShootsCrewman();
		} else {
			tug2DetermineElasiShooter();
			_roomVar.tug.shootingTarget = OBJECT_KIRK;
			_awayMission->timers[2] = 40;
			tug2GuardShootsCrewman();
		}
		break;
	}
}

void Room::tug2GuardShootsCrewman() {
	const char *g1ShotsOnStun[] = {
		"t2beem15",
		"t2beem14",
		"t2beem17",
		"t2beem16"
	};
	const char *g1ShotsOnKill[] = {
		"t2beem19",
		"t2beem20",
		"t2beem21",
		"t2beem18",
	};
	const char *g2ShotsOnStun[] = { // BUG: these look wrong. But correct animations don't seem to exist.
		"t2beem07",
		"t2beem08",
		"t2beem09",
		"t2beem06"
	};
	const char *g2ShotsOnKill[] = {
		"t2beem11",
		"t2beem12",
		"t2beem13",
		"t2beem10",
	};

	const char *crewPrefixes = "ksmr";

	if (!_roomVar.tug.elasiPhaserOnKill)
		_awayMission->crewDirectionsAfterWalk[_roomVar.tug.shootingTarget] = DIR_W;

	Common::String anim;
	const char **bitmapArray;
	if (_roomVar.tug.elasiPhaserOnKill) {
		anim = "_Kille";
		if (_roomVar.tug.shootingObject == OBJECT_GUARD_1)
			bitmapArray = g1ShotsOnKill;
		else
			bitmapArray = g2ShotsOnKill;
	} else {
		anim = "_stune";
		if (_roomVar.tug.shootingObject == OBJECT_GUARD_1)
			bitmapArray = g1ShotsOnStun;
		else
			bitmapArray = g2ShotsOnStun;
	}
	anim.setChar(crewPrefixes[_roomVar.tug.shootingTarget], 0);

	if (_roomVar.tug.shootingTarget != OBJECT_KIRK)
		loadActorAnim2(_roomVar.tug.shootingTarget, anim, -1, -1, 0);
	else if (_roomVar.tug.elasiPhaserOnKill)
		loadActorAnim2(_roomVar.tug.shootingTarget, anim, -1, -1, 27);
	else
		loadActorAnim2(_roomVar.tug.shootingTarget, anim, -1, -1, 26);

	showBitmapFor5Ticks(bitmapArray[_roomVar.tug.shootingTarget], 5);
	playSoundEffectIndex(SND_PHASSHOT);

	if (_roomVar.tug.shootingObject == OBJECT_GUARD_1)
		loadActorAnim2(OBJECT_GUARD_1, "p5fire", -1, -1, 0);
	else
		loadActorAnim2(OBJECT_GUARD_2, "p6fire", -1, -1, 0);

	_awayMission->tug.crewmanKilled[_roomVar.tug.shootingTarget] = _roomVar.tug.elasiPhaserOnKill;

	if (!_roomVar.tug.elasiPhaserOnKill) {
		// BUG: crew goes down for 120 frames, but they start to get up before the fight
		// is over. Perhaps this number should be increased.
		_awayMission->crewDirectionsAfterWalk[_roomVar.tug.shootingTarget] = DIR_W;
		_awayMission->crewGetupTimers[_roomVar.tug.shootingTarget] = 120;
		_awayMission->crewDownBitset |= (1 << _roomVar.tug.shootingTarget);
	}
	if (_roomVar.tug.elasiPhaserOnKill && _roomVar.tug.shootingTarget == OBJECT_REDSHIRT)
		_awayMission->redshirtDead = true;
}

void Room::tug2Timer2Expired() {
	// NOTE: is this function redundant? There's another that shows the game over menu
	// when Kirk's death animation finishes. That one seems to take precedence.
	showGameOverMenu();
}

void Room::tug2UseSTricorderOnBomb() {
	spockScan(DIR_E, 4, true, true);
}

void Room::tug2UseMTricorderOnGuard1() {
	if (_awayMission->tug.guard1Status == GUARDSTAT_STUNNED)
		mccoyScan(DIR_E, 16, true, true);
}

void Room::tug2UseMTricorderOnGuard2() {
	// BUGFIX: original didn't play audio for this (despite being the same as above).
	if (_awayMission->tug.guard2Status == GUARDSTAT_STUNNED)
		mccoyScan(DIR_E, 16, true, true);
}

void Room::tug2TalkToGuard1() {
	if (_awayMission->tug.guard1Status == GUARDSTAT_TIED)
		showText(TX_SPEAKER_ELASI_GUARD, 86, true, true);
	else if (_awayMission->tug.guard1Status == GUARDSTAT_STUNNED)
		showText(TX_SPEAKER_MCCOY, 12, true);
}

void Room::tug2TalkToGuard2() {
	if (_awayMission->tug.guard2Status == GUARDSTAT_TIED)
		showText(TX_SPEAKER_ELASI_GUARD, 86, true, true);
	else if (_awayMission->tug.guard2Status == GUARDSTAT_STUNNED)
		showText(TX_SPEAKER_MCCOY, 13, true);
}

void Room::tug2UseMedkitOnBomb() {
	showText(TX_SPEAKER_MCCOY, 17, true);
}

void Room::tug2UseMedkitOnGuard1() {
	if (_awayMission->tug.guard1Status == GUARDSTAT_STUNNED)
		showText(TX_SPEAKER_MCCOY, 14, true);
}

void Room::tug2UseMedkitOnGuard2() {
	if (_awayMission->tug.guard2Status == GUARDSTAT_STUNNED)
		showText(TX_SPEAKER_MCCOY, 14, true);
}

void Room::tug2LookAnywhere() {
	if (_awayMission->tug.guard1Status == GUARDSTAT_DEAD && _awayMission->tug.guard2Status == GUARDSTAT_DEAD && _awayMission->tug.field35 == 6)
		showDescription(19, true);
	else if (_awayMission->tug.guard1Status == GUARDSTAT_DEAD && _awayMission->tug.guard2Status == GUARDSTAT_DEAD && !_awayMission->tug.brigForceFieldDown)
		showDescription(17, true);
	else if (_awayMission->tug.guard1Status == GUARDSTAT_STUNNED && _awayMission->tug.guard2Status == GUARDSTAT_STUNNED && _awayMission->tug.field35 == 6)
		showDescription(20, true);
	else if (_awayMission->tug.guard1Status == GUARDSTAT_STUNNED && _awayMission->tug.guard2Status == GUARDSTAT_STUNNED && !_awayMission->tug.brigForceFieldDown)
		showDescription(18, true);
	else if (_awayMission->tug.guard1Status == GUARDSTAT_STUNNED && _awayMission->tug.guard2Status == GUARDSTAT_STUNNED && _awayMission->tug.brigForceFieldDown)
		showDescription(21, true);
	else if (_awayMission->tug.guard1Status == GUARDSTAT_DEAD && _awayMission->tug.guard2Status == GUARDSTAT_DEAD && _awayMission->tug.brigForceFieldDown)
		showDescription(22, true);
	else
		showDescription(23, true);
}

void Room::tug2TalkToBrig() {
	// The "brig" can also be a masada crewman, when the force field is down, so there are
	// at least two cases here.

	if (_awayMission->tug.brigForceFieldDown && !_awayMission->tug.talkedToBrigCrewman) {
		loadActorAnim2(OBJECT_BRIG, "fld10d", 0, 0, 0);
		showText(TX_SPEAKER_MASADA_CREWMAN, 26, true);
		_awayMission->tug.talkedToBrigCrewman = true;
	}

	if (_awayMission->tug.brigForceFieldDown && _awayMission->tug.talkedToBrigCrewman)
		showText(TX_SPEAKER_MASADA_CREWMAN, 27, true);
	else if (!_awayMission->tug.brigForceFieldDown)
		mccoyScan(DIR_E, 15, true, true);
}

void Room::tug2UseMTricorderOnBrig() {
	if (_awayMission->tug.field35 == 6)
		mccoyScan(DIR_E, 19, true, true);
	else if (!_awayMission->tug.brigForceFieldDown)
		mccoyScan(DIR_E, 15, true, true);
}

void Room::tug2UseMTricorderOnOpenBrig() {
	if (_awayMission->tug.brigForceFieldDown)
		mccoyScan(DIR_E, 15, true, true);
}

void Room::tug2UsePhaserAnywhere() {
	// Stub function to suppress "you can't do that" messages.
}

}
