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

void Room::demon3Tick1() {
	playVoc("DEM3LOOP");
	loadActorAnim(8, "light", 0xab, 0x03, 0);

	_roomVar.demon3.boulder1Shot = true;
	if (!_vm->_awayMission.demon.boulder1Gone) {
		loadActorAnim(9, "bldr1", 0x26, 0, 0);
		_roomVar.demon3.boulder1Shot = false;
	}
	if (!_vm->_awayMission.demon.boulder2Gone) {
		loadActorAnim(10, "bldr2", 0x22, 0, 0);
	}
	if (!_vm->_awayMission.demon.boulder3Gone) {
		loadActorAnim(11, "bldr3", 0x22, 0, 0);
	}
	if (!_vm->_awayMission.demon.boulder4Gone) {
		loadActorAnim(12, "bldr4", 0x22, 0, 0);
	}

	if (_vm->_awayMission.demon.doorOpened) {
		loadActorAnim(14, "door2", 0x82, 0x0c, 0);
	}
	else {
		loadMapFile("demon3a");
	}

	if (!_vm->_awayMission.demon.healedMiner && _vm->_awayMission.demon.boulder4Gone)
		loadActorAnim(13, "miner", 0xda, 0x6c, 0);
	if (_vm->_awayMission.redshirtDead && !_vm->_awayMission.demon.field45)
		loadActorAnim(OBJECT_REDSHIRT, "deadre", 0x46, 0xa0, 0);

	if (_vm->_awayMission.demon.field41 == 0)
		_vm->_awayMission.demon.field41++;

	if (!_vm->_awayMission.demon.field56) {
		_vm->_awayMission.demon.field56 = true;
		playMidiMusicTracks(0, -1);
	}
}

// Boulder falling over
void Room::demon3Timer0Expired() {
	_roomVar.demon3.boulder1Shot = true;
	loadActorAnim2(9, "drbldr", 0x26, 0, 0);
	_vm->_awayMission.timers[1] = 13;
	_vm->_awayMission.demon.boulder1Gone = true;
	_vm->_awayMission.demon.numBouldersGone++;
	_vm->_awayMission.disableInput = true;
	playMidiMusicTracks(2, -1);
	playVoc("BOULDERK");
}

// Redshirt pushing Kirk away
void Room::demon3Timer1Expired() {
	_roomVar.demon3.kirkPhaserOut = false;
	loadActorAnim2(OBJECT_REDSHIRT, "redkil", 0x46, 0xa0, 0);
	loadActorAnim2(OBJECT_KIRK, "redkil", 0x46, 0xa0, 1);
}

// Door just opened
void Room::demon3Timer3Expired() {
	if (_vm->_awayMission.demon.repairedHand) {
		showText(TX_DEM3N008);
		loadActorAnim(14, "door", 0x82, 0xc, 0);
		loadMapFile("demon3");
		_vm->_awayMission.demon.doorOpened = true;
		_vm->_awayMission.demon.missionScore += 2;
	}
	else {
		showText(TX_DEM3N007);
	}
}

void Room::demon3FinishedAnimation1() {
	loadActorAnim2(OBJECT_KIRK, "kstnds", 0xee, 0xa6, 0);
	loadActorAnim2(OBJECT_REDSHIRT, "deadre", 0x46, 0xa0, 0);
	loadActorStandAnim(OBJECT_SPOCK);
	walkCrewman(OBJECT_MCCOY, 0x9c, 0xb2, 5);
	_roomVar.demon3.inFiringPosition = false;
	_vm->_awayMission.redshirtDead = true;
}

// Just destroyed boulder 4 (revealing the miner)
void Room::demon3FinishedAnimation2() {
	loadActorAnim(13, "miner", 0xda, 0x6c, 0);
}

void Room::demon3FinishedWalking5() {
	loadActorAnim2(OBJECT_MCCOY, "mscanw", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, TX_DEM3_019);

	_vm->_awayMission.disableInput = false;
}

void Room::demon3McCoyInFiringPosition() {
	loadActorAnim2(OBJECT_MCCOY, "mwaitn", 0xd6, 0xb8, 0);
	_roomVar.demon3.mccoyInPosition = true;
	demon3CrewmanInFiringPosition();
}

void Room::demon3SpockInFiringPosition() {
	loadActorAnim2(OBJECT_SPOCK, "swaitn", 0xae, 0xb4, 0);
	_roomVar.demon3.spockInPosition = true;
	demon3CrewmanInFiringPosition();
}

void Room::demon3RedShirtInFiringPosition() {
	loadActorAnim2(OBJECT_REDSHIRT, "rwaitn", 0x46, 0xa0, 0);
	_roomVar.demon3.redshirtInPosition = true;
	demon3CrewmanInFiringPosition();
}

void Room::demon3KirkInFiringPosition() {
	loadActorAnim2(OBJECT_KIRK, "kwaitn", 0x79, 0xa0, 0);
	_roomVar.demon3.kirkInPosition = true;
	demon3CrewmanInFiringPosition();
}

void Room::demon3CrewmanInFiringPosition() {
	if (_roomVar.demon3.kirkInPosition && _roomVar.demon3.spockInPosition && _roomVar.demon3.mccoyInPosition && (_vm->_awayMission.redshirtDead || _roomVar.demon3.redshirtInPosition))
		demon3PullOutPhaserAndFireAtBoulder();
}

void Room::demon3PullOutPhaserAndFireAtBoulder() {
	if (_roomVar.demon3.kirkPhaserOut)
		demon3FireAtBoulder();
	else {
		loadActorAnim2(OBJECT_KIRK, "kfiren", -1, -1, 4);
		_roomVar.demon3.kirkPhaserOut = true;
	}
}

void Room::demon3FireAtBoulder() {
	_roomVar.demon3.kirkInPosition = false;
	_roomVar.demon3.spockInPosition = false;
	_roomVar.demon3.mccoyInPosition = false;
	_roomVar.demon3.redshirtInPosition = false;
	_roomVar.demon3.shootingBoulder = false;

	switch (_roomVar.demon3.boulderBeingShot) {
	case 1: // FIXME: animation overlaps on Kirk, probably due to sprite sorting inaccuracy
		loadActorAnim2(9, "sbldr1", 0x22, 0, 0);
		break;
	case 2:
		loadActorAnim2(10, "sbldr2", 0x22, 0, 0);
		break;
	case 3:
		loadActorAnim2(11, "sbldr3", 0x22, 0, 0);
		break;
	case 4:
		loadActorAnim2(12, "sbldr4", 0x22, 0, 2);
		break;
	default: {
		// This should never happen (in the original game, this would cause a stack
		// misalignment?
		const char *text[] = {
			"wes",
			"There is a boulder check error.",
			""
		};
		showRoomSpecificText(text);
		break;
	}
	}

	loadActorAnim(17, _roomVar.demon3.boulderAnim, 0, 0, 0);
	playSoundEffectIndex(0x06);
	if (!_roomVar.demon3.boulder1Shot)
		_vm->_awayMission.timers[0] = 1;
	_vm->_awayMission.disableInput = false;
}

void Room::demon3UsePhaserOnRedshirt() {
	if (!_vm->_awayMission.redshirtDead)
		return;
	showText(TX_SPEAKER_MCCOY, TX_DEM3_013);
}

void Room::demon3UseStunPhaserOnBoulder() {
	showText(TX_SPEAKER_SPOCK, TX_DEM3_007);
}

void Room::demon3UsePhaserOnBoulder1() {
	if (_roomVar.demon3.shootingBoulder || _vm->_awayMission.demon.boulder1Gone)
		return;
	_vm->_awayMission.demon.boulder1Gone = true;
	_vm->_awayMission.demon.numBouldersGone++;
	_roomVar.demon3.boulderBeingShot = 1;
	_roomVar.demon3.boulder1Shot = true;
	strcpy(_roomVar.demon3.boulderAnim, "s0r3s2");
	demon3BoulderCommon();
}

void Room::demon3UsePhaserOnBoulder2() {
	if (_roomVar.demon3.shootingBoulder || _vm->_awayMission.demon.boulder2Gone)
		return;
	_vm->_awayMission.demon.boulder2Gone = true;
	_vm->_awayMission.demon.numBouldersGone++;
	_roomVar.demon3.boulderBeingShot = 2;
	strcpy(_roomVar.demon3.boulderAnim, "s0r3s3");
	demon3BoulderCommon();
}

void Room::demon3UsePhaserOnBoulder3() {
	if (_vm->_awayMission.demon.boulder1Gone) {
		if (_roomVar.demon3.shootingBoulder || _vm->_awayMission.demon.boulder3Gone)
			return;
		_vm->_awayMission.demon.boulder3Gone = true;
		_vm->_awayMission.demon.numBouldersGone++;
		_roomVar.demon3.boulderBeingShot = 3;
		strcpy(_roomVar.demon3.boulderAnim, "s0r3s1");
		demon3BoulderCommon();
	}
	else {
		showText(TX_SPEAKER_SPOCK, TX_DEM3_006);
	}
}

void Room::demon3UsePhaserOnBoulder4() {
	if (_vm->_awayMission.demon.boulder2Gone) {
		if (_roomVar.demon3.shootingBoulder || _vm->_awayMission.demon.boulder4Gone)
			return;
		_vm->_awayMission.demon.boulder4Gone = true;
		_vm->_awayMission.demon.numBouldersGone++;
		_roomVar.demon3.boulderBeingShot = 4;
		strcpy(_roomVar.demon3.boulderAnim, "s0r3s4");
		_vm->_awayMission.demon.foundMiner = true;
		demon3BoulderCommon();
	}
	else {
		// BUGFIX: In the original, the audio didn't play, despite the file existing (and
		// despite it being used for the boulder on the left).
		showText(TX_SPEAKER_SPOCK, TX_DEM3_006);
	}
}

void Room::demon3BoulderCommon() {
	_vm->_awayMission.disableInput = true;
	Common::Point pos = getActorPos(OBJECT_KIRK);
	if (!(pos.x == 0x79 && pos.y == 0xa0)) {
		_roomVar.demon3.inFiringPosition = false;
		_roomVar.demon3.kirkPhaserOut = false;
	}

	if (_roomVar.demon3.inFiringPosition) {
		demon3PullOutPhaserAndFireAtBoulder();
	}
	else {
		showText(TX_SPEAKER_KIRK, TX_DEM3_001);

		walkCrewman(OBJECT_KIRK, 0x79, 0xa0, 1);
		walkCrewman(OBJECT_SPOCK, 0xae, 0xb4, 2);
		walkCrewman(OBJECT_MCCOY, 0xd6, 0xb8, 3);
		if (!_vm->_awayMission.redshirtDead)
			walkCrewman(OBJECT_REDSHIRT, 0x46, 0xa0, 4);

		_roomVar.demon3.shootingBoulder = true;
		_roomVar.demon3.inFiringPosition = true;
	}
}

void Room::demon3UseSTricorderOnMiner() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM3_008);
}

void Room::demon3UseSTricorderOnPanel() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM3_009);
}

void Room::demon3UseSTricorderOnBoulder() {
	if (_vm->_awayMission.demon.foundMiner)
		return;
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM3_010);
}

void Room::demon3UseMTricorderOnBoulder() {
	if (_vm->_awayMission.demon.foundMiner)
		return;
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, TX_DEM3_020);
}

void Room::demon3UseCrewmanOnPanel() {
	if (_vm->_awayMission.demon.numBouldersGone != 4 || _vm->_awayMission.redshirtDead)
		return;
	showText(TX_SPEAKER_EVERTS, TX_DEM3_031);
	demon3UseRedshirtOnPanel();
}

void Room::demon3UseRedshirtOnPanel() {
	if (_vm->_awayMission.redshirtDead)
		return;

	if (_vm->_awayMission.demon.numBouldersGone != 4)
		showText(TX_SPEAKER_EVERTS, TX_DEM3_030);
	else
		walkCrewman(OBJECT_REDSHIRT, 0xd8, 0x70, 8);
}

void Room::demon3RedshirtReachedPanel() {
	loadActorAnim2(OBJECT_REDSHIRT, "rusehn", -1, -1, 5);
}

void Room::demon3RedshirtUsedPanel() {
	if (!_vm->_awayMission.demon.foundMiner)
		return;

	_vm->_awayMission.demon.field41++;
	if (_vm->_awayMission.demon.field41 == 5) {
		playVoc("EFX3");
		showText(TX_SPEAKER_EVERTS, TX_DEM3_A32);

		loadActorAnim2(OBJECT_REDSHIRT, "rkille", -1, -1, 3);
		playSoundEffectIndex(0x06);
		_vm->_awayMission.redshirtDead = true;
		_vm->_awayMission.demon.field45 = true;
	}
	else {
		const int textTable[] = {
			TX_DEM3N005,
			TX_DEM3_033,
			TX_DEM3_035,
			TX_DEM3_034,
		};

		int text[] = {
			TX_SPEAKER_EVERTS,
			TX_NULL,
			TX_BLANK
		};
		text[1] = textTable[_vm->_awayMission.demon.field41 - 1];
		showText(text);
		walkCrewman(OBJECT_REDSHIRT, 0xbe, 0x9b, 0);
	}
}

void Room::demon3RedshirtElectrocuted() {
	showText(TX_SPEAKER_MCCOY, TX_DEM3_018);
}

void Room::demon3UseSTricorderOnDoor() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM3_012);
}

void Room::demon3UseSTricorderOnAnything() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM3_027);
}

void Room::demon3UseMTricorderOnDoor() {
	loadActorAnim2(OBJECT_SPOCK, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, TX_DEM3_016);
}

void Room::demon3UsePhaserOnDoor() {
	_roomVar.demon3.usedPhaserOnDoor++;

	if (_roomVar.demon3.usedPhaserOnDoor == 1)
		showText(TX_DEM3N009);
	else if (_roomVar.demon3.usedPhaserOnDoor == 2)
		showText(TX_SPEAKER_SPOCK, TX_DEM3_011);
}

void Room::demon3UseHandOnPanel() {
	if (_vm->_awayMission.demon.numBouldersGone == 4) {
		if (_vm->_awayMission.demon.doorOpened)
			return;
		walkCrewman(OBJECT_KIRK, 0xd8, 0x70, 7);
	}
	else {
		showText(TX_SPEAKER_SPOCK, TX_DEM3_005);
	}
}

void Room::demon3KirkReachedHandPanel() {
	loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 6);
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
	playVoc("MUR4E1");
}

void Room::demon3KirkUsedHandPanel() {
	loadActorAnim(15, "panel", 0xd6, 0x3d, 0);
	_vm->_awayMission.timers[3] = 10;
	playVoc("SE0FORCE");
}

void Room::demon3UseMTricorderOnMiner() {
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_vm->_awayMission.demon.minerDead) {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_022);
	}
	else {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_023);
	}
}

void Room::demon3UseMedkitOnMiner() {
	if (_vm->_awayMission.demon.healedMiner) {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_015);
	}
	else {
		walkCrewman(OBJECT_MCCOY, 0xe6, 0x7b, 6);
		_roomVar.demon3.inFiringPosition = false;
		_roomVar.demon3.kirkPhaserOut = false;
	}
}

void Room::demon3McCoyReachedMiner() {
	loadActorAnim2(OBJECT_MCCOY, "museln", -1, -1, 7);
}

void Room::demon3McCoyFinishedHealingMiner() {
	if (_vm->_awayMission.demon.minerDead) {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_022);
	}
	else {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_021);
		_vm->_awayMission.demon.healedMiner = true;
		loadActorAnim2(13, "drmine", 0xda, 0x6c, 0);
		_vm->_awayMission.demon.missionScore += 2;
		walkCrewman(OBJECT_MCCOY, 0x104, 0x96, 0);
	}
}

void Room::demon3GetMiner() {
	showText(TX_SPEAKER_MCCOY, TX_DEM3_017);
}

void Room::demon3TalkToMiner() {
	if (!_vm->_awayMission.demon.healedMiner)
		return;
	showText(TX_SPEAKER_KANDREY, TX_DEM3_F21);
}

void Room::demon3TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  TX_DEM3_002);
	showText(TX_SPEAKER_SPOCK, TX_DEM3_029);
	showText(TX_SPEAKER_MCCOY, TX_DEM3_026);
}

void Room::demon3TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_DEM3_028);
}

void Room::demon3TalkToMccoy() {
	if (_vm->_awayMission.redshirtDead) {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_014);
	}
	else {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_025);
		showText(TX_SPEAKER_KIRK,  TX_DEM3_003);
		showText(TX_SPEAKER_MCCOY, TX_DEM3_024);

	}
}

void Room::demon3TalkToRedshirt() {
	// FIXME: this shouldn't work if he's dead. Should it check higher up whether that's
	// the case?
	showText(TX_SPEAKER_EVERTS, TX_DEM3_036);
	showText(TX_SPEAKER_KIRK,   TX_DEM3_004);
}

void Room::demon3LookAtKirk() {
	showText(TX_DEM3N004);
}

void Room::demon3LookAtSpock() {
	showText(TX_DEM3N001);
}

void Room::demon3LookAtMccoy() {
	showText(TX_DEM3N002);
}

void Room::demon3LookAtRedshirt() {
	if (_vm->_awayMission.redshirtDead) {
		showText(TX_DEM3N017);
		// NOTE: there's an alternate string that isn't used? (TX_DEM3N018)
	}
	else {
		showText(TX_DEM3N003);
	}
}

void Room::demon3LookAnywhere() {
	showText(TX_DEM3N010);
}

void Room::demon3LookAtMiner() {
	if (_vm->_awayMission.demon.healedMiner) {
		showText(TX_DEM3N000);
	}
	else {
		showText(TX_DEM3N006);
	}
}

void Room::demon3LookAtBoulder1() {
	showText(TX_DEM3N011);
}

void Room::demon3LookAtBoulder2() {
	showText(TX_DEM3N013);
}

void Room::demon3LookAtBoulder3() {
	showText(TX_DEM3N013);
}

void Room::demon3LookAtBoulder4() {
	showText(TX_DEM3N020);
}

void Room::demon3LookAtStructure() {
	showText(TX_DEM3N016);
}

void Room::demon3LookAtDoor() {
	if (_vm->_awayMission.demon.doorOpened) {
		showText(TX_DEM3N014);
	}
	else {
		showText(TX_DEM3N012);
	}
}

void Room::demon3LookAtPanel() {
	showText(TX_DEM3N019);
}

void Room::demon3LookAtLight() {
	showText(TX_DEM3N015);
}

}
