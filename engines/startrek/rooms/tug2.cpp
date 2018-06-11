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

	if (!_vm->_awayMission.tug.haveBomb)
		loadActorAnim2(OBJECT_BOMB, "t2bomb", -3, 1, 0);
	if (!_vm->_awayMission.tug.gotWires)
		loadActorAnim2(OBJECT_WIRE, "t2wire", 0, 1, 0);

	if (!_vm->_awayMission.tug.brigForceFieldDown)
		loadActorAnim2(OBJECT_BRIG, "fld01", 0, 0, 0);
	else
		loadActorAnim2(OBJECT_BRIG, "fld10d", 0, 0, 0);
	if (_vm->_awayMission.tug.field35 == 6)
		loadActorAnim2(OBJECT_BRIG, "zapdon", 0, 0, 0);

	// Security guy 1
	if (_vm->_awayMission.tug.guard1Status == 8)
		loadActorAnim2(OBJECT_GUARD_1, "p5tied", 0xe0, 0xb1, 0);
	else if (_vm->_awayMission.tug.guard1Status == 1 && _vm->_awayMission.tug.brigForceFieldDown) {
		loadActorAnim2(OBJECT_GUARD_1, "p5tied", 0xe0, 0xb1, 0);
		_vm->_awayMission.tug.guard1Status = 8;
	}
	if (_vm->_awayMission.tug.guard1Status == 1 && !_vm->_awayMission.tug.brigForceFieldDown) {
		_vm->_awayMission.tug.guard1Status = 0;
		_vm->_awayMission.tug.crewmanKilled[OBJECT_KIRK] = false;
		_vm->_awayMission.tug.crewmanKilled[OBJECT_SPOCK] = false;
		_vm->_awayMission.tug.crewmanKilled[OBJECT_MCCOY] = false;
		if (!_vm->_awayMission.redshirtDead)
			_vm->_awayMission.tug.crewmanKilled[OBJECT_REDSHIRT] = false;
		_vm->_awayMission.tug.field5b = 0;
	}
	if (_vm->_awayMission.tug.guard1Status == 0)
		loadActorAnim2(OBJECT_GUARD_1, "p5redy", 0xe0, 0xb1, 0);
	if (_vm->_awayMission.tug.guard1Status == 2)
		loadActorAnim2(OBJECT_GUARD_1, "p5hat", 0xe0, 0xb1, 0);

	// Security guy 2
	if (_vm->_awayMission.tug.guard2Status == 8)
		loadActorAnim2(OBJECT_GUARD_2, "p6tied", 0xfb, 0xc5, 0);
	else if (_vm->_awayMission.tug.guard2Status == 1 && _vm->_awayMission.tug.brigForceFieldDown) {
		loadActorAnim2(OBJECT_GUARD_2, "p6tied", 0xfb, 0xc5, 0);
		_vm->_awayMission.tug.guard2Status = 8;
	}
	if (_vm->_awayMission.tug.guard2Status == 1 && !_vm->_awayMission.tug.brigForceFieldDown) {
		_vm->_awayMission.tug.guard2Status = 0;
		_vm->_awayMission.tug.crewmanKilled[OBJECT_KIRK] = false;
		_vm->_awayMission.tug.crewmanKilled[OBJECT_SPOCK] = false;
		_vm->_awayMission.tug.crewmanKilled[OBJECT_MCCOY] = false;
		if (!_vm->_awayMission.redshirtDead)
			_vm->_awayMission.tug.crewmanKilled[OBJECT_REDSHIRT] = false;
		_vm->_awayMission.tug.field5b = 0;
	}
	if (_vm->_awayMission.tug.guard2Status == 0)
		loadActorAnim2(OBJECT_GUARD_2, "p6draw", 0xfb, 0xc5, 0);
	if (_vm->_awayMission.tug.guard2Status == 2)
		loadActorAnim2(OBJECT_GUARD_2, "p5hat", 0xfb, 0xc5, 0); // FIXME


	if (_vm->_awayMission.tug.field4e == 0)
		loadActorAnim2(OBJECT_13, "wallht", 0, 0, 0);

	if (_vm->_awayMission.tug.field5b == 0) {
		_vm->_awayMission.disableWalking = true;
		playMidiMusicTracks(32, -1);
	}

	_vm->_awayMission.tug.field5b = 1;
	_vm->_awayMission.tug.kirkPhaserDrawn = 0;
}

void Room::tug2Tick60() {
	if (!_vm->_awayMission.tug.guard1Status || !_vm->_awayMission.tug.guard2Status) {
		_vm->_awayMission.timers[0] = 60;
		showText(TX_SPEAKER_ELASI_GUARD, TX_TUG2L085);
		tug2ElasiReadyPhaser();
	}
}

void Room::tug2LookAtButton() {
	showText(TX_TUG2N011);
}

void Room::tug2LookAtMccoy() {
	showText(TX_TUG2N005);
}

void Room::tug2LookAtSpock() {
	showText(TX_TUG2N007);
}

void Room::tug2LookAtRedshirt() {
	showText(TX_TUG2N004);
}

void Room::tug2GetBomb() {
	if (_vm->_awayMission.tug.brigForceFieldDown) {
		_vm->_awayMission.disableInput = true;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewman(OBJECT_KIRK, 0xc9, 0xa0, 12);
	}
	else
		showText(TX_TUG2N024);
}

void Room::tug2KirkReachedBomb() {
	loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 13);
}

void Room::tug2KirkGotBomb() {
	loadActorStandAnim(OBJECT_BOMB);
	giveItem(OBJECT_IBOMB);
	_vm->_awayMission.tug.haveBomb = true;
	_vm->_awayMission.disableInput = false;
}

void Room::tug2LookAtBomb() {
	showText(TX_TUG2N000);
}

void Room::tug2LookAtGuard1() {
	if (_vm->_awayMission.tug.guard1Status == 0)
		showText(TX_TUG2N001);
	else if (_vm->_awayMission.tug.guard1Status == 1)
		showText(TX_TUG2N013);
	else if (_vm->_awayMission.tug.guard1Status == 8)
		showText(TX_TUG2N015);
	else if (_vm->_awayMission.tug.guard1Status == 2)
		showText(TX_TUG2N106);
}

void Room::tug2LookAtGuard2() {
	if (_vm->_awayMission.tug.guard2Status == 0)
		showText(TX_TUG2N037);
	else if (_vm->_awayMission.tug.guard2Status == 1)
		showText(TX_TUG2N014);
	else if (_vm->_awayMission.tug.guard2Status == 8)
		showText(TX_TUG2N016);
	else if (_vm->_awayMission.tug.guard2Status == 2)
		showText(TX_TUG2N106);
}

void Room::tug2LookAtWires() {
	showText(TX_TUG2N010);
}

void Room::tug2UseSTricorderOnButton() {
	if (_vm->_awayMission.tug.field35 != 0)
		return;
	spockScan(DIR_E, TX_TUG2_007);
}

void Room::tug2UseMccoyOnWires() {
	if (_vm->_awayMission.tug.field35 != 0)
		return;
	showText(TX_SPEAKER_MCCOY, TX_TUG2_009);
}

void Room::tug2UseMccoyOnBomb() {
	showText(TX_SPEAKER_MCCOY, TX_TUG2_011);
}

void Room::tug2UseRedshirtOnWires() {
	if (_vm->_awayMission.disableWalking || _vm->_awayMission.tug.field35 != 0)
		return;
	showText(TX_SPEAKER_CHRISTENSEN, TX_TUG2L007);
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_N;
	_vm->_awayMission.disableInput = true;
	walkCrewman(OBJECT_REDSHIRT, 0xc9, 0xa0, 6);
}

void Room::tug2RedshirtReachedWires() {
	loadActorAnim2(OBJECT_REDSHIRT, "rusemn", -1, -1, 7);
}

void Room::tug2RedshirtDefusedBomb() {
	showText(TX_TUG2C001);
	showText(TX_SPEAKER_CHRISTENSEN, TX_TUG2L011);
	_vm->_awayMission.tug.field35 = 4;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_E;
	walkCrewman(OBJECT_REDSHIRT, 0x44, 0xc2, 8);
}

void Room::tug2RedshirtReturnedToPosition() {
	_vm->_awayMission.disableInput = false;
	_vm->_awayMission.tug.missionScore += 2;
}

void Room::tug2UseKirkOnWires() {
	if (_vm->_awayMission.disableWalking || _vm->_awayMission.tug.field35 != 0)
		return;
	_vm->_awayMission.disableInput = true;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 0xc9, 0xa0, 5);
}

void Room::tug2KirkReachedWires() {
	showText(TX_SPEAKER_KIRK,  TX_TUG2_003);
	showText(TX_SPEAKER_SPOCK, TX_TUG2_023);

	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 0x66, 0xb8, 0);
	_vm->_awayMission.disableInput = false;
	tug2UseSpockOnWires();
}

void Room::tug2UseSpockOnWires() {
	if (_vm->_awayMission.disableWalking || _vm->_awayMission.tug.field35 != 0)
		return;
	_vm->_awayMission.disableInput = true;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
	walkCrewman(OBJECT_SPOCK, 0xc9, 0xa0, 9);
}

void Room::tug2SpockReachedWires() {
	loadActorAnim2(OBJECT_SPOCK, "suseme", -1, -1, 0);
	playVoc("WIRESNIP");
	showText(TX_SPEAKER_SPOCK, TX_TUG2_024);
	_vm->_awayMission.tug.field35 = 4;

	if (_vm->_awayMission.tug.spockExaminedTransporter)
		showText(TX_SPEAKER_SPOCK, TX_TUG2_008);

	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
	walkCrewman(OBJECT_SPOCK, 0x56, 0xa9, 11);
}

void Room::tug2SpockReturnedToPosition() {
	_vm->_awayMission.disableInput = false;
	_vm->_awayMission.tug.missionScore += 2;
}

void Room::tug2GetWires() {
	if (_vm->_awayMission.disableWalking)
		return;
	_vm->_awayMission.disableInput = true;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 0xc9, 0xa0, 28);
}

void Room::tug2KirkReachedWiresToGet() {
	loadActorAnim2(OBJECT_KIRK, "kuseme", -1, -1, 29);
}

void Room::tug2KirkGotWires() {
	if (_vm->_awayMission.tug.field35 == 0) {
		playMidiMusicTracks(2, -1);
		loadActorAnim2(OBJECT_BRIG, "zap", 0, 0, 1);
		playVoc("NM8EXPLO");
		loadActorAnim2(OBJECT_BOMB, "t2bbo", -3, 1, 0);
		playSoundEffectIndex(SND_PHASSHOT);
		playSoundEffectIndex(SND_BLANK_16);
		_vm->_awayMission.tug.field35 = 6;
		loadActorStandAnim(OBJECT_WIRE);
		giveItem(OBJECT_IWIRING);
		_vm->_awayMission.tug.gotWires = true;
		_vm->_awayMission.disableInput = false;
		_vm->_awayMission.tug.missionScore += 3;
	}
	else {
		loadActorStandAnim(OBJECT_WIRE);
		giveItem(OBJECT_IWIRING);
		_vm->_awayMission.tug.gotWires = true;
		_vm->_awayMission.disableInput = false;
		_vm->_awayMission.tug.missionScore += 3;
	}
}

void Room::tug2UseKirkOnButton() {
	if (_vm->_awayMission.disableWalking)
		return;
	_vm->_awayMission.disableInput = true;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 0xc9, 0xa0, 14);
}

void Room::tug2KirkReachedButton() {
	loadActorAnim2(OBJECT_KIRK, "kusehe", -1, -1, 15);
	playSoundEffectIndex(SND_07);
}

void Room::tug2UseSpockOnButton() {
	if (_vm->_awayMission.disableWalking)
		return;
	_vm->_awayMission.disableInput = true;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
	walkCrewman(OBJECT_SPOCK, 0xc9, 0xa0, 16);
}

void Room::tug2SpockReachedButton() {
	loadActorAnim2(OBJECT_SPOCK, "susehe", -1, -1, 17);
	playSoundEffectIndex(SND_07);
}

void Room::tug2UseMccoyOnButton() {
	if (_vm->_awayMission.disableWalking)
		return;
	_vm->_awayMission.disableInput = true;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_E;
	walkCrewman(OBJECT_MCCOY, 0xc9, 0xa0, 18);
}

void Room::tug2MccoyReachedButton() {
	loadActorAnim2(OBJECT_MCCOY, "musehe", -1, -1, 19);
	playSoundEffectIndex(SND_07);
}

void Room::tug2UseRedshirtOnButton() {
	if (_vm->_awayMission.disableWalking)
		return;
	_vm->_awayMission.disableInput = true;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_E;
	walkCrewman(OBJECT_REDSHIRT, 0xc9, 0xa0, 20);
}

void Room::tug2RedshirtReachedButton() {
	loadActorAnim2(OBJECT_REDSHIRT, "rusehe", -1, -1, 21);
	playSoundEffectIndex(SND_07);
}

void Room::tug2TurnedOffForceField() {
	_vm->_awayMission.disableInput = false;
	if (_vm->_awayMission.tug.field35 == 0) {
		playMidiMusicTracks(2, -1);
		loadActorAnim2(OBJECT_BRIG, "zap", 0, 0, 1);
		playVoc("NM8EXPLO");
		loadActorAnim2(OBJECT_BOMB, "t2bbo", -3, 1, 0);
		// FIXME: doesn't play "PHASSHOT" sound effect like when kirk removes wires?
		_vm->_awayMission.tug.field35 = 6;
	}
	else if (_vm->_awayMission.tug.field35 == 4 && !_vm->_awayMission.tug.brigForceFieldDown) {
		_vm->_awayMission.tug.brigForceFieldDown = true;
		loadActorAnim2(OBJECT_BRIG, "fld10", 0, 0, 2);
		_vm->_awayMission.tug.missionScore += 6;
		playVoc("EFX15");
	}
}

void Room::tug2PrisonersDead() {
	loadActorAnim2(OBJECT_BRIG, "zapdon", 0, 0, 0);
	showText(TX_TUG2N026);
}

void Room::tug2PrisonersReleased() {
	loadActorAnim2(OBJECT_BRIG, "fld10d", 0, 0, 0);
	showText(TX_SPEAKER_MASADA_CREWMAN, TX_TUG2_029);
	showText(TX_SPEAKER_MASADA_CREWMAN, TX_TUG2_030);
	showText(TX_SPEAKER_MASADA_CREWMAN, TX_TUG2_028);
	_vm->_awayMission.tug.savedPrisoners = true;
}

void Room::tug2UsePhaserOnBrig() {
	if (_vm->_awayMission.tug.field35 == 0)
		showText(TX_SPEAKER_SPOCK, TX_TUG2_006);
	else if (_vm->_awayMission.tug.brigForceFieldDown) {
		// BUGFIX: this function had two implementations; one for firing on the brig, and
		// one for firing on the masada crewman (who replaces the brig object). The first
		// took priority, meaning the latter code never ran. That's fixed here.
		showText(TX_SPEAKER_MASADA_CREWMAN, TX_TUG2_025);
	}
}

void Room::tug2ElasiReadyPhaser() {
	if (_vm->_awayMission.tug.guard1Status == 0 && _vm->_awayMission.tug.field59 == 0) {
		loadActorAnim2(OBJECT_GUARD_1, "p5draw", -1, -1, 0);
		_vm->_awayMission.tug.field59++;
	}
}

void Room::tug2CheckEndFirefight() {
	if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_UP || _vm->_awayMission.tug.guard2Status == GUARDSTAT_UP)
		return;
	_vm->_awayMission.disableWalking = false;
	if (_vm->_awayMission.tug.kirkPhaserDrawn)
		loadActorStandAnim(OBJECT_KIRK);
}

void Room::tug2UseStunPhaserOnGuard1() {
	if (_vm->_awayMission.tug.guard1Status != GUARDSTAT_UP)
		tug2KirkShootGuard1();
	else {
		tug2ElasiReadyPhaser();
		_vm->_awayMission.disableInput = true;
		if (_vm->_awayMission.tug.kirkPhaserDrawn)
			tug2KirkShootGuard1();
		else {
			loadActorAnim2(OBJECT_KIRK, "kdrawe", -1, -1, 22);
			_vm->_awayMission.tug.kirkPhaserDrawn = true;
		}
	}
}

void Room::tug2KirkShootGuard1() {
	if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_UP) {
		loadActorAnim2(OBJECT_GUARD_1, "p5stun", -1, -1, 0);
		_vm->_awayMission.tug.guard1Status = GUARDSTAT_STUNNED;
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t2beem03", 5);
		tug2CheckEndFirefight();
	}

	_vm->_awayMission.disableInput = false;
}

void Room::tug2UseStunPhaserOnGuard2() {
	if (_vm->_awayMission.tug.guard2Status != GUARDSTAT_UP)
		tug2KirkShootGuard2();
	else {
		tug2ElasiReadyPhaser();
		_vm->_awayMission.disableInput = true;
		if (_vm->_awayMission.tug.kirkPhaserDrawn)
			tug2KirkShootGuard2();
		else {
			loadActorAnim2(OBJECT_KIRK, "kdrawe", -1, -1, 23);
			_vm->_awayMission.tug.kirkPhaserDrawn = true;
		}
	}
}

void Room::tug2KirkShootGuard2() {
	if (_vm->_awayMission.tug.guard2Status == GUARDSTAT_UP){
		loadActorAnim2(OBJECT_GUARD_2, "p6stun", -1, -1, 0);
		_vm->_awayMission.tug.guard2Status = GUARDSTAT_STUNNED;
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t2beem02", 5);
		tug2CheckEndFirefight();
	}

	_vm->_awayMission.disableInput = false;
}

void Room::tug2UseKillPhaserOnGuard1() {
	if (_vm->_awayMission.tug.guard1Status != GUARDSTAT_UP)
		tug2KirkKillGuard1();
	else {
		tug2ElasiReadyPhaser();
		_vm->_awayMission.disableInput = true;
		if (_vm->_awayMission.tug.kirkPhaserDrawn)
			tug2KirkKillGuard1();
		else {
			loadActorAnim2(OBJECT_KIRK, "kdrawe", -1, -1, 24);
			_vm->_awayMission.tug.kirkPhaserDrawn = true;
			_vm->_awayMission.tug.field4a = true;
		}
	}
}

void Room::tug2KirkKillGuard1() {
	if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_UP){
		loadActorAnim2(OBJECT_GUARD_1, "p5kill", -1, -1, 0);
		_vm->_awayMission.tug.guard1Status = GUARDSTAT_DEAD;
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t2beem05", 5);
		tug2CheckEndFirefight();
	}

	_vm->_awayMission.disableInput = false;
}

void Room::tug2UseKillPhaserOnGuard2() {
	if (_vm->_awayMission.tug.guard2Status != GUARDSTAT_UP)
		tug2KirkKillGuard2();
	else {
		tug2ElasiReadyPhaser();
		_vm->_awayMission.disableInput = true;
		if (_vm->_awayMission.tug.kirkPhaserDrawn)
			tug2KirkKillGuard2();
		else {
			loadActorAnim2(OBJECT_KIRK, "kdrawe", -1, -1, 25);
			_vm->_awayMission.tug.kirkPhaserDrawn = true;
			_vm->_awayMission.tug.field4a = true;
		}
	}
}

void Room::tug2KirkKillGuard2() {
	if (_vm->_awayMission.tug.guard2Status == GUARDSTAT_UP){
		loadActorAnim2(OBJECT_GUARD_2, "p6kill", -1, -1, 0);
		_vm->_awayMission.tug.guard2Status = GUARDSTAT_DEAD;
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t2beem04", 5);
		tug2CheckEndFirefight();
	}

	_vm->_awayMission.disableInput = false;
}

void Room::tug2UsePhaserOnWelder() {
	loseItem(OBJECT_IPWE);
	giveItem(OBJECT_IPWF);
	showText(TX_TUG2N006);

	// BUGFIX: this following line didn't exist, despite it existing in TUG1; meaning this
	// was supposed to give points, but it only did in a specific room.
	_vm->_awayMission.tug.missionScore += 3;
}

void Room::tug2UseWelderOnWireScraps() {
	showText(TX_TUG2N009);
	loseItem(OBJECT_IWIRSCRP);
}

void Room::tug2UseWelderOnMetalScraps() {
	showText(TX_SPEAKER_SPOCK, TX_TUG2_112);
	loseItem(OBJECT_IJNKMETL);
	giveItem(OBJECT_ICOMBBIT);
}

void Room::tug2UseCombBitOnTransmogrifier() {
	showText(TX_SPEAKER_SPOCK, TX_TUG2_021);
	loseItem(OBJECT_ICOMBBIT);
	loseItem(OBJECT_IRT);
	giveItem(OBJECT_IRTWB);
}

void Room::tug2ShotByElasi() {
	showText(TX_TUG2N025);
	showGameOverMenu();

	// Unused: additional textbox that says "***Game over man!***"
}

void Room::tug2WalkToDoor() {
	walkCrewman(OBJECT_KIRK, 0x0b, 0xab, 0);
}

void Room::tug2LookAtDoor() {
	showText(TX_TUG2N012);
}

void Room::tug2LookAtKirk() {
	showText(TX_TUG2N003);
}

void Room::tug2TalkToKirk() {
	if (_vm->_awayMission.tug.field35 == 6)
		showText(TX_SPEAKER_KIRK, TX_TUG2_001);
	else
		showText(TX_SPEAKER_KIRK, TX_TUG2_002);
}

void Room::tug2TalkToMccoy() {
	if (_vm->_awayMission.tug.field35 == 6)
		showText(TX_SPEAKER_MCCOY, TX_TUG2_018);
	else
		showText(TX_SPEAKER_MCCOY, TX_TUG2_010);
}

void Room::tug2TalkToRedshirt() {
	// BUGFIX: the original game has two implementations of this function; the first takes
	// precedence, however, it's just generic, unhelpful text. The second implementation
	// is more interesting, so that one is used instead.
	if (_vm->_awayMission.tug.field35 == 6)
		showText(TX_SPEAKER_CHRISTENSEN, TX_TUG2L004);
	else
		showText(TX_SPEAKER_CHRISTENSEN, TX_TUG2J003);

	// Original text: "I'm just a security officer, sir."
	//showText(TX_SPEAKER_CHRISTENSEN, TX_TUG2L002);
}

void Room::tug2TalkToSpock() {
	if (_vm->_awayMission.tug.field35 == 6)
		showText(TX_SPEAKER_SPOCK, TX_TUG2_005);
	else
		showText(TX_SPEAKER_SPOCK, TX_TUG2_020);
}

void Room::tug2UseCommunicator() {
	showText(TX_SPEAKER_SPOCK, TX_TUG1_011);
}

void Room::tug2DetermineElasiShooter() {
	if (_vm->_awayMission.tug.guard2Status == GUARDSTAT_UP)
		_roomVar.tug2.shootingObject = OBJECT_GUARD_2;
	if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_UP)
		_roomVar.tug2.shootingObject = OBJECT_GUARD_1;
}

// Elasi shoots someone
void Room::tug2Timer0Expired() {
	if (_vm->_awayMission.tug.guard1Status != GUARDSTAT_UP && _vm->_awayMission.tug.guard2Status != GUARDSTAT_UP)
		return;

	_roomVar.tug2.elasiPhaserOnKill = 0;
	if (_vm->_awayMission.tug.field4a)
		_roomVar.tug2.elasiPhaserOnKill = 1;

	switch (_vm->_awayMission.tug.field4b) {
	case 0:
		_vm->_awayMission.tug.field4b++;
		if (_vm->_awayMission.tug.guard2Status != GUARDSTAT_UP) {
			// BUGFIX: reset the timer to allow guard 1 to continue if guard 2 is down
			_vm->_awayMission.timers[0] = 60;
			return;
		}

		// Guard misses and shoots wall
		loadActorAnim2(OBJECT_13, "wallht", 0, 0, 0);
		showBitmapFor5Ticks("t2beem00", 5);
		playSoundEffectIndex(SND_PHASSHOT);
		loadActorAnim2(OBJECT_GUARD_2, "p6fire", -1, -1, 0);
		_vm->_awayMission.tug.field4e = _roomVar.tug2.elasiPhaserOnKill;
		_vm->_awayMission.timers[0] = 60;
		break;

	case 1:
		// Guard shoots redshirt
		_vm->_awayMission.tug.field4b++;
		if (!_vm->_awayMission.tug.crewmanKilled[OBJECT_REDSHIRT]) {
			tug2DetermineElasiShooter();
			_roomVar.tug2.shootingTarget = OBJECT_REDSHIRT;
			_vm->_awayMission.timers[0] = 60;
			tug2GuardShootsCrewman();
		}
		break;

	case 2:
		// Guard shoots spock (or kirk)
		_vm->_awayMission.tug.field4b++;
		_roomVar.tug2._1ec4 = 0;
		if (_vm->_awayMission.tug.field4a)
			_roomVar.tug2._1ec4 = 1;
		tug2DetermineElasiShooter();
		if (_roomVar.tug2._1ec4 == 0)
			_roomVar.tug2.shootingTarget = OBJECT_SPOCK;
		else
			_roomVar.tug2.shootingTarget = OBJECT_KIRK;
		_vm->_awayMission.timers[0] = 60;
		tug2GuardShootsCrewman();
		break;

	case 3:
		_vm->_awayMission.tug.field4b++;
		if (!_vm->_awayMission.tug.crewmanKilled[OBJECT_MCCOY]) {
			tug2DetermineElasiShooter();
			_roomVar.tug2.shootingTarget = OBJECT_MCCOY;
			_vm->_awayMission.timers[0] = 80;
			tug2GuardShootsCrewman();
		}
		break;

	case 4:
		_vm->_awayMission.tug.field4b++;
		if (_roomVar.tug2._1ec4 == 0) {
			tug2DetermineElasiShooter();
			_roomVar.tug2.shootingTarget = OBJECT_KIRK;
			_vm->_awayMission.timers[2] = 40; // TODO
			tug2GuardShootsCrewman();
		}
		else if (_roomVar.tug2._1ec4 == 1) {
			tug2DetermineElasiShooter();
			_roomVar.tug2.shootingTarget = OBJECT_SPOCK;
			_vm->_awayMission.timers[2] = 40;
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

	if (!_roomVar.tug2.elasiPhaserOnKill)
		_vm->_awayMission.crewDirectionsAfterWalk[_roomVar.tug2.shootingTarget] = DIR_W;

	Common::String anim;
	const char **bitmapArray;
	if (_roomVar.tug2.elasiPhaserOnKill) {
		anim = "_Kille";
		if (_roomVar.tug2.shootingObject == OBJECT_GUARD_1)
			bitmapArray = g1ShotsOnKill;
		else
			bitmapArray = g2ShotsOnKill;
	}
	else {
		anim = "_stune";
		if (_roomVar.tug2.shootingObject == OBJECT_GUARD_1)
			bitmapArray = g1ShotsOnStun;
		else
			bitmapArray = g2ShotsOnStun;
	}
	anim.setChar(crewPrefixes[_roomVar.tug2.shootingTarget], 0);

	if (_roomVar.tug2.shootingTarget != OBJECT_KIRK)
		loadActorAnim2(_roomVar.tug2.shootingTarget, anim, -1, -1, 0);
	else if (_roomVar.tug2.elasiPhaserOnKill)
		loadActorAnim2(_roomVar.tug2.shootingTarget, anim, -1, -1, 27);
	else
		loadActorAnim2(_roomVar.tug2.shootingTarget, anim, -1, -1, 26);

	showBitmapFor5Ticks(bitmapArray[_roomVar.tug2.shootingTarget], 5);
	playSoundEffectIndex(SND_PHASSHOT);

	if (_roomVar.tug2.shootingObject == OBJECT_GUARD_1)
		loadActorAnim2(OBJECT_GUARD_1, "p5fire", -1, -1, 0);
	else
		loadActorAnim2(OBJECT_GUARD_2, "p6fire", -1, -1, 0);

	_vm->_awayMission.tug.crewmanKilled[_roomVar.tug2.shootingTarget] = _roomVar.tug2.elasiPhaserOnKill;

	if (!_roomVar.tug2.elasiPhaserOnKill) {
		// BUG: crew goes down for 120 frames, but they start to get up before the fight
		// is over. Perhaps this number should be increased.
		_vm->_awayMission.crewDirectionsAfterWalk[_roomVar.tug2.shootingTarget] = DIR_W;
		_vm->_awayMission.crewGetupTimers[_roomVar.tug2.shootingTarget] = 120;
		_vm->_awayMission.crewDownBitset |= (1 << _roomVar.tug2.shootingTarget);
	}
	if (_roomVar.tug2.elasiPhaserOnKill && _roomVar.tug2.shootingTarget == OBJECT_REDSHIRT)
		_vm->_awayMission.redshirtDead = true;
}

void Room::tug2Timer2Expired() {
	// NOTE: is this function redundant? There's another that shows the game over menu
	// when Kirk's death animation finishes. That one seems to take precedence.
	showGameOverMenu();
}

void Room::tug2UseSTricorderOnBomb() {
	spockScan(DIR_E, TX_TUG2_004);
}

void Room::tug2UseMTricorderOnGuard1() {
	if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_STUNNED)
		mccoyScan(DIR_E, TX_TUG2_016);
}

void Room::tug2UseMTricorderOnGuard2() {
	// BUGFIX: original didn't play audio for this (despite being the same as above).
	if (_vm->_awayMission.tug.guard2Status == GUARDSTAT_STUNNED)
		mccoyScan(DIR_E, TX_TUG2_016);
}

void Room::tug2TalkToGuard1() {
	if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_TIED)
		showText(TX_SPEAKER_ELASI_GUARD, TX_TUG2L086);
	else if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_STUNNED)
		showText(TX_SPEAKER_MCCOY, TX_TUG2_012);
}

void Room::tug2TalkToGuard2() {
	if (_vm->_awayMission.tug.guard2Status == GUARDSTAT_TIED)
		showText(TX_SPEAKER_ELASI_GUARD, TX_TUG2L086);
	else if (_vm->_awayMission.tug.guard2Status == GUARDSTAT_STUNNED)
		showText(TX_SPEAKER_MCCOY, TX_TUG2_013);
}

void Room::tug2UseMedkitOnBomb() {
	showText(TX_SPEAKER_MCCOY, TX_TUG2_017);
}

void Room::tug2UseMedkitOnGuard1() {
	if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_STUNNED)
		showText(TX_SPEAKER_MCCOY, TX_TUG2_014);
}

void Room::tug2UseMedkitOnGuard2() {
	if (_vm->_awayMission.tug.guard2Status == GUARDSTAT_STUNNED)
		showText(TX_SPEAKER_MCCOY, TX_TUG2_014);
}

void Room::tug2LookAnywhere() {
	if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_DEAD && _vm->_awayMission.tug.guard2Status == GUARDSTAT_DEAD && _vm->_awayMission.tug.field35 == 6)
		showText(TX_TUG2N019);
	else if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_DEAD && _vm->_awayMission.tug.guard2Status == GUARDSTAT_DEAD && !_vm->_awayMission.tug.brigForceFieldDown)
		showText(TX_TUG2N017);
	else if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_STUNNED && _vm->_awayMission.tug.guard2Status == GUARDSTAT_STUNNED && _vm->_awayMission.tug.field35 == 6)
		showText(TX_TUG2N020);
	else if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_STUNNED && _vm->_awayMission.tug.guard2Status == GUARDSTAT_STUNNED && !_vm->_awayMission.tug.brigForceFieldDown)
		showText(TX_TUG2N018);
	else if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_STUNNED && _vm->_awayMission.tug.guard2Status == GUARDSTAT_STUNNED && _vm->_awayMission.tug.brigForceFieldDown)
		showText(TX_TUG2N021);
	else if (_vm->_awayMission.tug.guard1Status == GUARDSTAT_DEAD && _vm->_awayMission.tug.guard2Status == GUARDSTAT_DEAD && _vm->_awayMission.tug.brigForceFieldDown)
		showText(TX_TUG2N022);
	else
		showText(TX_TUG2N023);
}

void Room::tug2TalkToBrig() {
	// The "brig" can also be a masada crewman, when the force field is down, so there are
	// at least two cases here.

	if (_vm->_awayMission.tug.brigForceFieldDown && !_vm->_awayMission.tug.talkedToBrigCrewman) {
		loadActorAnim2(OBJECT_BRIG, "fld10d", 0, 0, 0);
		showText(TX_SPEAKER_MASADA_CREWMAN, TX_TUG2_026);
		_vm->_awayMission.tug.talkedToBrigCrewman = true;
	}

	if (_vm->_awayMission.tug.brigForceFieldDown && _vm->_awayMission.tug.talkedToBrigCrewman)
		showText(TX_SPEAKER_MASADA_CREWMAN, TX_TUG2_027);
	else if (!_vm->_awayMission.tug.brigForceFieldDown)
		mccoyScan(DIR_E, TX_TUG2_015);
}

void Room::tug2UseMTricorderOnBrig() {
	if (_vm->_awayMission.tug.field35 == 6)
		mccoyScan(DIR_E, TX_TUG2_019);
	else if (!_vm->_awayMission.tug.brigForceFieldDown)
		mccoyScan(DIR_E, TX_TUG2_015);
}

void Room::tug2UseMTricorderOnOpenBrig() {
	if (_vm->_awayMission.tug.brigForceFieldDown)
		mccoyScan(DIR_E, TX_TUG2_015);
}

void Room::tug2UsePhaserAnywhere() {
	// Stub function to suppress "you can't do that" messages.
}

}
