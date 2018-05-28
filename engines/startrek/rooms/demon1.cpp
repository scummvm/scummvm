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

// BUG: under certain circumstances, the klingons just stop firing?

void Room::demon1Tick1() {
	playVoc("DEM1LOOP");

	if (!_vm->_awayMission.demon.beatKlingons)
		playMidiMusicTracks(1, -1);

	if (_vm->_awayMission.demon.enteredFrom == 1) { // Entered from south
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_N;
	}
	else if (_vm->_awayMission.demon.enteredFrom == 2) { // Entered from north?
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_S;
	}

	if (!_vm->_awayMission.demon.beatKlingons) {
		_vm->_awayMission.disableWalking = true;
		_vm->_awayMission.timers[2] = 50;
	}
	else {
		loadActorAnim(8, "klg1d2", 0x120, 0x82, 0);
		if (!_vm->_awayMission.demon.tookKlingonHand)
			loadActorAnim(13, "klghnd", 0x10b, 0x8e, 0);
		loadActorAnim(9, "klg2d2", 0xaa, 0x7c, 0);
		loadActorAnim(10, "klg3d2", 0, 0, 0);

		_rdfData[0xca] = 1; // FIXME
		_rdfData[0xcb] = 1;
		_rdfData[0xcc] = 1;
		_roomVar.demon1.numKlingonsKilled = 3;
	}
}

void Room::demon1WalkToCave() {
	if (_roomVar.demon1.numKlingonsKilled != 3)
		return;
	walkCrewman(OBJECT_KIRK, 0xa3, 0x56, 2);
}

void Room::demon1TouchedTopWarp() {
	if (_vm->_awayMission.demon.warpsDisabled)
		return;
	loadRoomIndex(2, 1);
}

void Room::demon1TouchedBottomWarp() {
	if (_vm->_awayMission.demon.warpsDisabled)
		return;
	loadRoomIndex(0, 0);
}

void Room::demon1Timer2Expired() {
	if (_vm->_awayMission.demon.beatKlingons)
		return;
	_vm->_awayMission.demon.field37 = 1;
	_vm->_awayMission.demon.beatKlingons = true;
	_vm->_awayMission.demon.warpsDisabled = true;
	_vm->_awayMission.timers[0] = 5;
	_vm->_awayMission.timers[3] = 2;
	_vm->_awayMission.timers[1] = 100;
	loadActorAnim(8, "klg1u", 0x120, 0x82, 0);
}

void Room::demon1Timer0Expired() {
	loadActorAnim(9, "klg2u", 0xaa, 0x7c, 0);
}

void Room::demon1Timer3Expired() {
	loadActorAnim(10, "klg3u", 0xdb, 0x6d, 0);
}

void Room::demon1Timer1Expired() {
	int shooter;
	Common::String anim;

	if (_rdfData[0xca] != 1) {
		anim = "klg1f";
		shooter = 8;

		switch (_roomVar.demon1.attackIndex) {
		case 0:
			strcpy(_roomVar.demon1.d6, "klg1fr");
			break;
		case 1:
			strcpy(_roomVar.demon1.d6, "klg1fm");
			break;
		case 2:
			strcpy(_roomVar.demon1.d6, "klg1fs");
			break;
		case 3:
			strcpy(_roomVar.demon1.d6, "klg1fk");
			break;
		default:
			// TODO
			return;
		}
	}
	else if (_rdfData[0xcb] != 1) {
		anim = "klg2f";
		shooter = 9;
		switch (_roomVar.demon1.attackIndex) {
		case 0:
			strcpy(_roomVar.demon1.d6, "klg2fr");
			break;
		case 1:
			strcpy(_roomVar.demon1.d6, "klg2fm");
			break;
		case 2:
			strcpy(_roomVar.demon1.d6, "klg2fs");
			break;
		case 3:
			strcpy(_roomVar.demon1.d6, "klg2fk");
			break;
		default:
			// TODO
			return;
		}
	}
	else {
		anim = "klg3f";
		shooter = 10;
		switch (_roomVar.demon1.attackIndex) {
		case 0:
			strcpy(_roomVar.demon1.d6, "klg3fr");
			break;
		case 1:
			strcpy(_roomVar.demon1.d6, "klg3fm");
			break;
		case 2:
			strcpy(_roomVar.demon1.d6, "klg3fs");
			break;
		case 3:
			strcpy(_roomVar.demon1.d6, "klg3fk");
			break;
		default:
			// TODO
			return;
		}
	}

	loadActorAnim2(shooter, anim, -1, -1, 2);
}

void Room::demon1KlingonFinishedAimingWeapon() {
	loadActorAnim(11, _roomVar.demon1.d6, 0, 0, 0);
	playSoundEffectIndex(0x06);

	_roomVar.demon1.attackIndex++;
	int crewman;
	if (_roomVar.demon1.attackIndex == 1) {
		loadActorAnim2(OBJECT_REDSHIRT, "rstunn", -1, -1, 0);
		crewman = OBJECT_REDSHIRT;
	}
	else if (_roomVar.demon1.attackIndex == 2) {
		loadActorAnim2(OBJECT_SPOCK, "sstunn", -1, -1, 0);
		crewman = OBJECT_SPOCK;
	}
	else if (_roomVar.demon1.attackIndex == 3) {
		loadActorAnim2(OBJECT_MCCOY, "mstune", -1, -1, 0);
		crewman = OBJECT_MCCOY;
	}
	else if (_roomVar.demon1.attackIndex == 4) {
		loadActorAnim2(OBJECT_KIRK, "kstunn", -1, -1, 1);
		crewman = OBJECT_KIRK;
	}

	_vm->_awayMission.crewDirectionsAfterWalk[crewman] = DIR_N;
	_vm->_awayMission.crewGetupTimers[crewman] = 650;
	_vm->_awayMission.crewDownBitset |= (1 << crewman);

	_vm->_awayMission.timers[1] = 100;
}

void Room::demon1KirkShot() {
	showText(TX_DEM1N000);
	showGameOverMenu();
}

void Room::demon1UsePhaserOnAnything() {
	if (_roomVar.demon1.numKlingonsKilled == 3)
		showText(TX_SPEAKER_MCCOY, TX_DEM1_011);
}

void Room::demon1UsePhaserOnKlingon1() {
	if (_rdfData[0xca] != 0)
		return;
	_vm->_awayMission.transitioningIntoRoom = 1;
	_rdfData[0xca] = 1;

	if (_roomVar.demon1.kirkShooting) {
		demon1ShootKlingon1();
	}
	else {
		loadActorAnim2(OBJECT_KIRK, "kfiren", -1, -1, 3);
		_roomVar.demon1.kirkShooting = true;
	}
}

void Room::demon1ShootKlingon1() {
	_roomVar.demon1.numKlingonsKilled++;
	if (_roomVar.demon1.numKlingonsKilled == 3) {
		_vm->_awayMission.timers[1] = 0;
		_vm->_awayMission.timers[5] = 180;
		_vm->_awayMission.timers[6] = 1;
		_vm->_awayMission.demon.warpsDisabled = false;
		_vm->_awayMission.disableWalking = false;
	}
	loadActorAnim(12, "s0ks1", 0, 0, 0);
	playSoundEffectIndex(0x06);
	playVoc("EFX19");
	loadActorAnim2(8, "klg1d", 0x120, 0x84, 7);
}

void Room::demon1KlingonDropsHand() {
	loadActorAnim(13, "klghnd", 0x10b, 0x8e, 0);
	_vm->_awayMission.transitioningIntoRoom = 0;
	_vm->_awayMission.timers[1] = 0;
	showText(TX_DEM1N020);

	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_REDSHIRT))
		return;

	showText(TX_SPEAKER_EVERTS, TX_DEM1_025);

	if (_roomVar.demon1.numKlingonsKilled == 3)
		return;

	showText(TX_SPEAKER_KLINGON, TX_DEM1_F23);

	_vm->_awayMission.timers[1] = 1;
}

void Room::demon1UsePhaserOnKlingon2() {
	if (_rdfData[0xcb] != 0)
		return;
	_vm->_awayMission.transitioningIntoRoom = 1;
	_rdfData[0xcb] = 1;

	if (_roomVar.demon1.kirkShooting) {
		demon1ShootKlingon2();
	}
	else {
		loadActorAnim2(OBJECT_KIRK, "kfiren", -1, -1, 4);
		_roomVar.demon1.kirkShooting = true;
	}
}

void Room::demon1ShootKlingon2() {
	_roomVar.demon1.numKlingonsKilled++;
	if (_roomVar.demon1.numKlingonsKilled == 3) {
		_vm->_awayMission.timers[1] = 0;
		_vm->_awayMission.timers[5] = 180;
		_vm->_awayMission.timers[6] = 1;
		_vm->_awayMission.demon.warpsDisabled = false;
		_vm->_awayMission.disableWalking = false;
	}
	loadActorAnim(12, "s0ks2", 0, 0, 0);
	playSoundEffectIndex(0x06);
	loadActorAnim2(9, "klg2d", 0xaa, 0x7c, 0);
	_vm->_awayMission.transitioningIntoRoom = 0;
}

void Room::demon1UsePhaserOnKlingon3() {
	if (_rdfData[0xcc] != 0)
		return;
	_vm->_awayMission.transitioningIntoRoom = 1;
	_rdfData[0xcc] = 1;

	if (_roomVar.demon1.kirkShooting) {
		demon1ShootKlingon3();
	}
	else {
		loadActorAnim2(OBJECT_KIRK, "kfiren", -1, -1, 5);
		_roomVar.demon1.kirkShooting = true;
	}
}

void Room::demon1ShootKlingon3() {
	_roomVar.demon1.numKlingonsKilled++;
	if (_roomVar.demon1.numKlingonsKilled == 3) {
		_vm->_awayMission.timers[1] = 0;
		_vm->_awayMission.timers[5] = 180;
		_vm->_awayMission.timers[6] = 1;
		_vm->_awayMission.demon.warpsDisabled = false;
		_vm->_awayMission.disableWalking = false;
	}
	loadActorAnim(12, "s0ks3", 0, 0, 0);
	playSoundEffectIndex(0x06);
	loadActorAnim2(10, "klg3d", 0, 0, 0);
	_vm->_awayMission.transitioningIntoRoom = 0;
}

// Timer 6 expired
void Room::demon1AllKlingonsDead() {
	_vm->_awayMission.crewGetupTimers[OBJECT_KIRK] = 45;
	_vm->_awayMission.crewGetupTimers[OBJECT_SPOCK] = 45;
	_vm->_awayMission.crewGetupTimers[OBJECT_MCCOY] = 45;
	_vm->_awayMission.crewGetupTimers[OBJECT_REDSHIRT] = 45;
}

void Room::demon1Timer5Expired() {
	if (_vm->_awayMission.crewDownBitset != 0)
		return;

	showText(TX_SPEAKER_UHURA, TX_DEM1U077);
	showText(TX_SPEAKER_KIRK,  TX_DEM1_003);
	showText(TX_SPEAKER_UHURA, TX_DEM1U078);
	showText(TX_SPEAKER_KIRK,  TX_DEM1_002);
	showText(TX_SPEAKER_SPOCK, TX_DEM1_020);
	showText(TX_SPEAKER_KIRK,  TX_DEM1_004);
	showText(TX_SPEAKER_SPOCK, TX_DEM1_021);
}


void Room::demon1UseMTricorderOnKlingon() {
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, TX_DEM1_012);
}

void Room::demon1UseSTricorderOnTulips() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM1_007);
}

void Room::demon1UseSTricorderOnPods() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM1_008);
}

void Room::demon1UseSTricorderOnCattails() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM1_005);
}

void Room::demon1UseSTricorderOnFerns() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM1_006);
}

void Room::demon1UseSTricorderOnHand() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM1_017);
}

void Room::demon1UseSTricorderOnKlingon1() {
	// NOTE: it sets up the stack, but doesn't call the function...
	//loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_roomVar.demon1.numKlingonsKilled == 3 && !_vm->_awayMission.demon.tookKlingonHand && _rdfData[0xcf] != 1) {
		showText(TX_SPEAKER_SPOCK, TX_DEM1_018);
		_rdfData[0xcf] = 1;
	}
	else {
		showText(TX_SPEAKER_SPOCK, TX_DEM1_019);
	}
}

void Room::demon1UseSTricorderOnKlingon2Or3() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM1_019);
}

void Room::demon1UseMTricorderOnKirk() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_KIRK))
		demon1UseMTricorderOnCrewman();
}

void Room::demon1UseMTricorderOnSpock() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_SPOCK))
		demon1UseMTricorderOnCrewman();
}

void Room::demon1UseMTricorderOnRedshirt() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_REDSHIRT))
		demon1UseMTricorderOnCrewman();
}

void Room::demon1UseMTricorderOnCrewman() {
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, TX_DEM1_009);
}

void Room::demon1GetHand() {
	if (_roomVar.demon1.numKlingonsKilled != 3)
		return;
	walkCrewman(OBJECT_KIRK, 0x104, 0x8e, 1);
}

void Room::demon1ReachedHand() {
	loadActorAnim2(OBJECT_KIRK, "s5r1kg", -1, -1, 6);
}

void Room::demon1PickedUpHand() {
	loadActorStandAnim(13);
	_vm->_awayMission.missionScore += 3;
	loadActorStandAnim(OBJECT_KIRK);
	_vm->_awayMission.timers[4] = 2;
}

// Timer 4 expired
void Room::demon1FinishedGettingHand() {

	if (_vm->_awayMission.demon.tookKlingonHand)
		showText(TX_DEM1N005);
	else {
		_vm->_awayMission.demon.tookKlingonHand = true;
		giveItem(OBJECT_IHAND);
		showText(TX_DEM1N007);
	}
}

void Room::demon1LookAtKlingon() {
	showText(TX_DEM1N004);
}

void Room::demon1LookAtCattails() {
	showText(TX_DEM1N018);
}

void Room::demon1LookAtTulips() {
	showText(TX_DEM1N010);
}

void Room::demon1LookAtPods() {
	showText(TX_DEM1N019);
}

void Room::demon1LookAtFerns() {
	showText(TX_DEM1N009);
}

void Room::demon1LookAtStream() {
	showText(TX_DEM1N011);
}

void Room::demon1LookAtMine() {
	showText(TX_DEM1N021);
}

void Room::demon1LookAtMountain() {
	showText(TX_DEM1N016);
}

void Room::demon1LookAtHand() {
	showText(TX_DEM1N023);
}
void Room::demon1LookAnywhere() {
	showText(TX_DEM1N017);
}

void Room::demon1LookAtKirk() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_KIRK))
		showText(TX_DEM1N012);
	else
		showText(TX_DEM1N003);
}

void Room::demon1LookAtSpock() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_SPOCK))
		showText(TX_DEM1N015);
	else
		showText(TX_DEM1N008);
}

void Room::demon1LookAtMcCoy() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_MCCOY))
		showText(TX_DEM1N013);
	else
		showText(TX_DEM1N001);
}

void Room::demon1LookAtRedshirt() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_REDSHIRT))
		showText(TX_DEM1N014);
	else
		showText(TX_DEM1N002);
}

void Room::demon1TalkToKirk() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_KIRK))
		demon1TalkToUnconsciousCrewman();
	else if (_roomVar.demon1.numKlingonsKilled == 3)
		showText(TX_SPEAKER_KIRK, TX_DEM1_001);
}

void Room::demon1TalkToSpock() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_SPOCK))
		demon1TalkToUnconsciousCrewman();
	else {
		showText(TX_SPEAKER_SPOCK, TX_DEM1_022);
		showText(TX_SPEAKER_MCCOY, TX_DEM1_013);
		showText(TX_SPEAKER_SPOCK, TX_DEM1_024);
		showText(TX_SPEAKER_MCCOY, TX_DEM1_014);
	}
}

void Room::demon1TalkToMcCoy() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_MCCOY))
		demon1TalkToUnconsciousCrewman();
	else {
		showText(TX_SPEAKER_MCCOY, TX_DEM1_015);
		showText(TX_SPEAKER_SPOCK, TX_DEM1_023);
	}
}

void Room::demon1TalkToRedshirt() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_REDSHIRT))
		demon1TalkToUnconsciousCrewman();
	else
		showText(TX_SPEAKER_EVERTS, TX_DEM1_026);
}

// FIXME: this doesn't happen in actual game? (does the event get blocked from higher up?)
void Room::demon1TalkToUnconsciousCrewman() {
	const char *text[] = {
		nullptr,
		"Zzzzz....",
		""
	};
	showRoomSpecificText(text);
}

}
