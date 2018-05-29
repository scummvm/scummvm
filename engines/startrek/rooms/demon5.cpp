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

void Room::demon5Tick1() {
	playVoc("DEM5LOOP");

	if (!_vm->_awayMission.demon.curedChub || _vm->_awayMission.demon.field37 != 1)
		loadActorAnim(11, "oldman", 0x7a, 0xb0, 0);
	loadActorAnim(8, "brorob", 0xc9, 0xa8, 0);
	loadActorAnim(9, "crip", 0xe3, 0xa8, 0);
	loadActorAnim(10, "brotel", 0xf9, 0xbc, 0);
	loadActorAnim(12, "s0r6d2", 0xa0, 0x92, 0);
}

void Room::demon5WalkToDoor() {
	_vm->_awayMission.disableInput = true;
	_roomVar.demon5.movingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0xa0, 0x94, 1);
}

void Room::demon5TouchedDoorOpenTrigger() {
	if (!_roomVar.demon5.movingToDoor)
		return;
	loadActorAnim(12, "s0r6d1", 0xa0, 0x92, 1);
	playSoundEffectIndex(0x05);
}

void Room::demon5DoorOpenedOrReachedDoor() {
	// This is invoked when the door opens and when Kirk reaches the door.
	// Must wait for both to occur.
	_roomVar.demon5.doorCounter++;
	if (_roomVar.demon5.doorCounter == 2)
		loadRoomIndex(0, 1);
}

void Room::demon5UseSTricorderOnCrate() {
	loadActorAnim(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM5_006);
}

void Room::demon5UsePhaserOnAnything() {
	showText(TX_SPEAKER_SPOCK, TX_DEM5_007);
}

void Room::demon5UseHandOnStephen() {
	if (_vm->_awayMission.demon.field3b)
		showText(TX_SPEAKER_STEPHEN, TX_DEM5_036);
	else
		showText(TX_SPEAKER_STEPHEN, TX_DEM5_039);
}

void Room::demon5UseBerryOnStephen() {
	if (_vm->_awayMission.demon.knowAboutHypoDytoxin)
		showText(TX_SPEAKER_STEPHEN, TX_DEM5_033);
	else
		showText(TX_SPEAKER_STEPHEN, TX_DEM5_037);
}

void Room::demon5UseHypoDytoxinOnChub() {
	walkCrewman(OBJECT_MCCOY, 0xe1, 0xb0, 2);
}

void Room::demon5MccoyReachedChub() {
	loadActorAnim2(OBJECT_MCCOY, "musemn", -1, -1, 2);
}

void Room::demon5MccoyHealedChub() {
	walkCrewman(OBJECT_MCCOY, 0xc8, 0xba, 0);
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
	showText(TX_SPEAKER_CHUB, TX_DEM5L027);
	loseItem(OBJECT_IDETOXIN);
	_vm->_awayMission.demon.curedChub = true;
	_vm->_awayMission.missionScore += 2;
}

void Room::demon5UseHypoDytoxinOnAnything() {
	showText(TX_SPEAKER_MCCOY, TX_DEM5_008);
}

void Room::demon5UseBerryOnChub() {
	showText(TX_SPEAKER_MCCOY, TX_DEM5_009);
}

void Room::demon5LookAtRoberts() {
	showText(TX_DEM5N001);
}

void Room::demon5LookAtGrisnash() {
	showText(TX_DEM5N009);
}

void Room::demon5LookAtStephen() {
	showText(TX_DEM5N003);
}

void Room::demon5LookAtKirk() {
	showText(TX_DEM5N006);
}

void Room::demon5LookAtSpock() {
	showText(TX_DEM5N008);
}

void Room::demon5LookAtMccoy() {
	showText(TX_DEM5N007);
}

void Room::demon5LookAtRedshirt() {
	showText(TX_DEM5N005);
}

void Room::demon5LookAtMountain() {
	showText(TX_DEM5N002);
}

void Room::demon5LookAtCrate() {
	showText(TX_DEM5N004);
}

void Room::demon5LookAnywhere() {
	showText(TX_DEM5N000);
}

void Room::demon5LookAtChub() {
	showText(TX_SPEAKER_CHUB, TX_DEM5L028);
}

void Room::demon5TalkToRoberts() {
	if (_vm->_awayMission.demon.curedChub) {
		showText(TX_SPEAKER_ROBERTS, TX_DEM5_030);
		if (!_roomVar.demon5.talkedToRoberts) {
			_roomVar.demon5.talkedToRoberts = true;
			_roomVar.demon5.numTalkedTo++;
			demon5CheckCompletedStudy();
		}
	}
	else {
		showText(TX_DEM5N010);
	}
}

void Room::demon5TalkToChub() {
	if (_vm->_awayMission.demon.curedChub) {
		showText(TX_SPEAKER_CHUB, TX_DEM5L029);
		showText(TX_SPEAKER_KIRK, TX_DEM5_002);
		showText(TX_SPEAKER_CHUB, TX_DEM5L030);
		showText(TX_SPEAKER_KIRK, TX_DEM5_005);
		showText(TX_SPEAKER_CHUB, TX_DEM5L031);

		if (!_roomVar.demon5.talkedToChub) {
			_roomVar.demon5.talkedToChub = true;
			_roomVar.demon5.numTalkedTo++;
			demon5CheckCompletedStudy();
		}
	}
	else {
		showText(TX_DEM5N013);
	}
}

void Room::demon5TalkToGrisnash() {
	if (_vm->_awayMission.demon.curedChub) {
		showText(TX_SPEAKER_GRISNASH, TX_DEM5_028);
		showText(TX_SPEAKER_SPOCK, TX_DEM5_024);
		showText(TX_SPEAKER_GRISNASH, TX_DEM5_029);
		showText(TX_SPEAKER_SPOCK, TX_DEM5_025);

		if (!_roomVar.demon5.talkedToGrisnash) {
			_roomVar.demon5.talkedToGrisnash = true;
			_roomVar.demon5.numTalkedTo++;
			demon5CheckCompletedStudy();
		}
	}
	else {
		showText(TX_DEM5N011);
	}
}

void Room::demon5TalkToStephen() {
	if (_vm->_awayMission.demon.curedChub) {
		showText(TX_SPEAKER_STEPHEN, TX_DEM5_041);
		showText(TX_SPEAKER_MCCOY, TX_DEM5_022);
		showText(TX_SPEAKER_STEPHEN, TX_DEM5_043);
		showText(TX_SPEAKER_ROBERTS, TX_DEM5_031);
		showText(TX_SPEAKER_STEPHEN, TX_DEM5_042);

		if (!_roomVar.demon5.talkedToStephen) {
			_roomVar.demon5.talkedToStephen = true;
			_roomVar.demon5.numTalkedTo++;
			demon5CheckCompletedStudy();
		}
	}
	else if (!_vm->_awayMission.demon.knowAboutHypoDytoxin) {
		showText(TX_SPEAKER_ROBERTS, TX_DEM5_034);
	}
	else if (_vm->_awayMission.demon.madeHypoDytoxin) {
		showText(TX_SPEAKER_ROBERTS, TX_DEM5_035);
	}
	else if (_vm->_awayMission.demon.gotBerries) {
		showText(TX_SPEAKER_ROBERTS, TX_DEM5_032);
		_vm->_awayMission.demon.field3e = false;
	}
	else if (_vm->_awayMission.demon.field3e) {
		showText(TX_SPEAKER_ROBERTS, TX_DEM5_038);
	}
	else {
		showText(TX_SPEAKER_ROBERTS, TX_DEM5_034);
	}
}

void Room::demon5TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_DEM5_001);
}

void Room::demon5TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_DEM5_027);
	showText(TX_SPEAKER_MCCOY, TX_DEM5_018);
}

void Room::demon5TalkToRedshirt() {
	showText(TX_SPEAKER_EVERTS, TX_DEM5_045);
	showText(TX_SPEAKER_MCCOY,  TX_DEM5_021);
	showText(TX_SPEAKER_KIRK,   TX_DEM5_003);
}

void Room::demon5TalkToMccoy() {
	if (_vm->_awayMission.demon.curedChub) {
		showText(TX_SPEAKER_MCCOY, TX_DEM5_023);
		showText(TX_SPEAKER_KIRK,  TX_DEM5_004);
		if (!_vm->_awayMission.redshirtDead) {
			showText(TX_SPEAKER_EVERTS, TX_DEM5_044);
			showText(TX_SPEAKER_MCCOY, TX_DEM5_020);
			showText(TX_SPEAKER_SPOCK, TX_DEM5_026);
			showText(TX_SPEAKER_MCCOY, TX_DEM5_019);
		}
	}
	else {
		showText(TX_SPEAKER_MCCOY, TX_DEM5_017);
	}
}

void Room::demon5UseMTricorderOnRoberts() {
	if (_roomVar.demon5.scannedRoberts)
		return;
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, TX_DEM5_015);

	_roomVar.demon5.scannedRoberts = true;
	_roomVar.demon5.numScanned++;
	demon5CheckCompletedStudy();
}

void Room::demon5UseMTricorderOnChub() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_vm->_awayMission.demon.curedChub) {
		if (_roomVar.demon5.scannedChub)
			return;
		showText(TX_SPEAKER_MCCOY, TX_DEM5_016);
		_roomVar.demon5.scannedChub = true;
		_roomVar.demon5.numScanned++;
		demon5CheckCompletedStudy();
	}
	else {
		if (_vm->_awayMission.demon.field3e)
			showText(TX_SPEAKER_MCCOY, TX_DEM5_010);
		else {
			showText(TX_SPEAKER_MCCOY, TX_DEM5_012);
			showText(TX_SPEAKER_STEPHEN, TX_DEM5_040);
			_vm->_awayMission.demon.knowAboutHypoDytoxin = true;
			_vm->_awayMission.demon.field3e = true;
		}
	}
}


void Room::demon5UseMTricorderOnGrisnash() {
	if (_roomVar.demon5.scannedGrisnash)
		return;
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, TX_DEM5_013);

	_roomVar.demon5.scannedGrisnash = true;
	_roomVar.demon5.numScanned++;
	demon5CheckCompletedStudy();
}

void Room::demon5UseMTricorderOnStephen() {
	if (_roomVar.demon5.scannedStephen)
		return;
	loadActorAnim2(OBJECT_MCCOY, "mscanw", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, TX_DEM5_014);

	_roomVar.demon5.scannedStephen = true;
	_roomVar.demon5.numScanned++;
	demon5CheckCompletedStudy();
}

void Room::demon5CheckCompletedStudy() {
	if (_roomVar.demon5.numScanned == 4 && _roomVar.demon5.numTalkedTo == 4) {
		showText(TX_SPEAKER_MCCOY, TX_DEM5_011);
		_roomVar.demon5.numTalkedTo = 5;
	}
}

void Room::demon5GetCrate() {
	showText(TX_DEM5N012);
}

}
