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

#define OBJECT_MUDD 8
#define OBJECT_ALIENDV 9
#define OBJECT_DOOR 10

#define HOTSPOT_MEMORY_DISK_BOX 0x20
#define HOTSPOT_LENSE 0x21
#define HOTSPOT_DEGRIMER_BOX 0x22
#define HOTSPOT_FOOD_BOX 0x23
#define HOTSPOT_COMPONENT_BOX 0x24
#define HOTSPOT_DOOR 0x25
#define HOTSPOT_BOTTOM_OF_ROOM 0x26

// BUG: Mudd is sometimes present when he shouldn't be.

namespace StarTrek {

void Room::mudd0Tick1() {
	playVoc("MUD0LOOP");

	_vm->_awayMission.disableInput = 2;
	loadActorAnim(OBJECT_MUDD, "s4cbhr", 0xa2, 0xa9, 0);

	// Floppy version's code.
	/*
	if (_vm->_awayMission.mudd.field29 == 0)
		_vm->_awayMission.mudd.field29 = 2;
	else {
		_vm->_awayMission.mudd.field1d = 2;
		loadActorAnim(OBJECT_MUDD, "s4cbhr", 0xa2, 0xa9, 0);
	}
	*/
}

void Room::mudd0Tick50() {
	if (!_vm->_awayMission.mudd.enteredRoom0ForFirstTime) {
		playMidiMusicTracks(3);
		_vm->_awayMission.mudd.enteredRoom0ForFirstTime = true;
	}
}

void Room::mudd0Tick60() {
	_vm->_awayMission.disableInput = false;

	if (_vm->_awayMission.mudd.field29 == 0) {
		_vm->_awayMission.mudd.field29++;

		showText(TX_SPEAKER_MUDD,  TX_MUD0_037);
		showText(TX_SPEAKER_KIRK,  TX_MUD0_009);
		showText(TX_SPEAKER_MUDD,  TX_MUD0_028);
		showText(TX_SPEAKER_MCCOY, TX_MUD0_017);
		showText(TX_SPEAKER_MUDD,  TX_MUD0_026);
		showText(TX_SPEAKER_KIRK,  TX_MUD0_006);
		showText(TX_SPEAKER_MUDD,  TX_MUD0_033);
		showText(TX_SPEAKER_KIRK,  TX_MUD0_005);
		showText(TX_SPEAKER_MUDD,  TX_MUD0_029);
	}
}

void Room::mudd0UsePhaserOnMudd() {
	showText(TX_SPEAKER_MUDD, TX_MUD0_025);
}

void Room::mudd0UseCommunicator() {
	showText(TX_SPEAKER_KIRK,  TX_MUD0_001);
	showText(TX_SPEAKER_UHURA, TX_STATICU1);
}

void Room::mudd0LookAtFoodBox() {
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	loadActorStandAnim(OBJECT_KIRK);
	showText(TX_MUD0N012);
}

void Room::mudd0LookAtComponentBox() {
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	loadActorStandAnim(OBJECT_KIRK);
	showText(TX_MUD0N013);
}

void Room::mudd0LookAnywhere() {
	showText(TX_MUD0N019);
}

void Room::mudd0LookAtMemoryDiskBox() {
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
	loadActorStandAnim(OBJECT_KIRK);
	showText(TX_MUD0N010);
}

void Room::mudd0LookAtDegrimerBox() {
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
	loadActorStandAnim(OBJECT_KIRK);
	showText(TX_MUD0N008);
}

void Room::mudd0LookAtLense() {
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	loadActorStandAnim(OBJECT_KIRK);
	showText(TX_MUD0N014);
}

void Room::mudd0UseSTricorderAnywhere() {
	// Narrator says something, not Spock (so we don't use "spockScan" function)
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_MUD0N015);
}

void Room::mudd0UseMTricorderAnywhere() {
	// Narrator says something, not Mccoy (so we don't use "mccoyScan" function)
	// BUGFIX-ish: original game had "McCoy" as the speaker, which is inconsistent with
	// above.
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_MUD0N015);
}

void Room::mudd0UseSTricorderOnMemoryDiskBox() {
	spockScan(DIR_N, TX_MUD0_021, true);
}

void Room::mudd0UseSTricorderOnDegrimerBox() {
	// Narrator says something, not Spock (so we don't use "spockScan" function)
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	loadActorAnim2(OBJECT_SPOCK, "sscanw", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_MUD0N000);
}

void Room::mudd0UseMTricorderOnLense() {
	if (_vm->_awayMission.mudd.discoveredLenseAndDegrimerFunction)
		mccoyScan(DIR_E, TX_MUD0_012, true);
	else
		mccoyScan(DIR_E, TX_MUD0_013, true);
}

void Room::mudd0UseLenseOnDegrimer() {
	giveItem(OBJECT_IALIENDV);
	loseItem(OBJECT_IDEGRIME);
	loseItem(OBJECT_ILENSES);

	_vm->_awayMission.mudd.missionScore++;
	showText(TX_MUD0N011);
	// Identical (?) audio files: TX_MUD0N011, TX_MUD1N013, TX_MUD2N010, TX_MUD3N016
}


void Room::mudd0UseAlienDevice() {
	const int deviceObjectIndices[] = {
		9,  // MUDD0
		13, // MUDD1
		11, // MUDD2
		11, // MUDD3
	};

	_vm->_awayMission.disableInput = true;

	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	loadActorStandAnim(OBJECT_KIRK);
	Common::Point pos = getActorPos(OBJECT_KIRK);
	loadActorAnimC(deviceObjectIndices[_roomIndex], "s4cbxp", pos.x, 10, &Room::mudd0FiredAlienDevice);
	playVoc("EXPLO3");
}

void Room::mudd0FiredAlienDevice() {
	_vm->_awayMission.disableInput = false;
	if (!_vm->_awayMission.mudd.discoveredLenseAndDegrimerFunction) {
		_vm->_awayMission.mudd.discoveredLenseAndDegrimerFunction = true;
		_vm->_awayMission.mudd.missionScore += 5;
		showText(TX_SPEAKER_KIRK, TX_MUD0_002);
		// Identical (?) audio files: TX_MUD0_002, TX_MUD1_002, TX_MUD2_002
	}
}


void Room::mudd0UseDegrimer() {
	// Identical (?) audio files: TX_MUD0N002, TX_MUD1N004, TX_MUD2N001, TX_MUD3N001...
	showText(TX_MUD0N002);
}


void Room::mudd0GetLense() {
	if (_vm->_awayMission.mudd.gotLense)
		showText(TX_MUD0N016);
	else {
		_vm->_awayMission.mudd.gotLense = true;
		_vm->_awayMission.mudd.missionScore++;
		giveItem(OBJECT_ILENSES);
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
		loadActorAnim2(OBJECT_KIRK, "s5r1kg", -1, -1);
		_vm->_awayMission.timers[1] = 27;
	}
}

void Room::mudd0GetMemoryDisk() {
	if (_vm->_awayMission.mudd.gotMemoryDisk)
		showText(TX_MUD0N016);
	else {
		_vm->_awayMission.mudd.gotMemoryDisk = true;
		_vm->_awayMission.mudd.missionScore++;
		giveItem(OBJECT_IDISKS);
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		loadActorAnimC(OBJECT_KIRK, "kuseln", -1, -1, &Room::mudd0PickedUpItem);
	}
}

void Room::mudd0GetDegrimer() {
	if (_vm->_awayMission.mudd.gotDegrimer)
		showText(TX_MUD0N016);
	else {
		_vm->_awayMission.mudd.gotDegrimer = true;
		_vm->_awayMission.mudd.missionScore++;
		giveItem(OBJECT_IDEGRIME);
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
		loadActorAnimC(OBJECT_KIRK, "kusemw", -1, -1, &Room::mudd0PickedUpItem);
	}
}

// Timer 1 expired
void Room::mudd0PickedUpLense() {
	loadActorStandAnim(OBJECT_KIRK);
	mudd0PickedUpItem();
}

void Room::mudd0PickedUpItem() {
	_vm->_awayMission.disableInput = false; // NOTE: this was never set to true
	showText(TX_LOV1N007);
}


void Room::mudd0WalkToSouthDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_vm->_awayMission.disableInput = true;
	walkCrewman(OBJECT_KIRK, 0x5a, 0xc7);
}

void Room::mudd0TouchedHotspot1() { // Trigger bottom door opening
	if (_roomVar.mudd.walkingToDoor)
		playVoc("SMADOOR3");
}

void Room::mudd0WalkToNorthDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_vm->_awayMission.disableInput = true;
	walkCrewman(OBJECT_KIRK, 0xa0, 0x68);
}

void Room::mudd0TouchedHotspot0() { // Trigger top door opening
	if (_roomVar.mudd.walkingToDoor) {
		playVoc("SMADOOR3");
		loadActorAnim2(OBJECT_DOOR, "s4cbdo", 0xa0, 0x6b);
	}
}

void Room::mudd0UseMedkit() {
	showText(TX_SPEAKER_MCCOY, TX_MUD0_011);
}

void Room::mudd0LookAtKirk() {
	showText(TX_MUD0N004);
}

void Room::mudd0LookAtSpock() {
	showText(TX_MUD0N007);
}

void Room::mudd0LookAtMccoy() {
	showText(TX_MUD0N001);
}

void Room::mudd0LookAtRedshirt() {
	showText(TX_MUD0N005);
}

void Room::mudd0LookAtMudd() {
	showText(TX_MUD0N003);
}

void Room::mudd0TalkToKirk() {
	if (_vm->_awayMission.mudd.field29 == 2)
		showText(TX_SPEAKER_KIRK, TX_MUD0_010);
	else {
		showText(TX_SPEAKER_KIRK, TX_MUD0_010);
		showText(TX_SPEAKER_MUDD, TX_MUD0_031);
		showText(TX_SPEAKER_KIRK, TX_MUD0_007);
	}
}

void Room::mudd0TalkToSpock() {
	if (_vm->_awayMission.mudd.field29 == 2)
		showText(TX_SPEAKER_SPOCK, TX_MUD0_022);
	else {
		showText(TX_SPEAKER_SPOCK, TX_MUD0_022);
		showText(TX_SPEAKER_MUDD,  TX_MUD0_034);
		showText(TX_SPEAKER_SPOCK, TX_MUD0_023);
		showText(TX_SPEAKER_MCCOY, TX_MUD0_015);
		showText(TX_SPEAKER_MUDD,  TX_MUD0_038);
	}
}

void Room::mudd0TalkToMccoy() {
	if (_vm->_awayMission.mudd.field29 == 2)
		showText(TX_SPEAKER_MCCOY, TX_MUD0_016);
	else {
		showText(TX_SPEAKER_MCCOY, TX_MUD0_016);
		showText(TX_SPEAKER_KIRK,  TX_MUD0_003);
		showText(TX_SPEAKER_MUDD,  TX_MUD0_027);
		showText(TX_SPEAKER_MCCOY, TX_MUD0_014);
	}
}

void Room::mudd0TalkToRedshirt() {
	if (_vm->_awayMission.mudd.field29 == 2)
		showText(TX_SPEAKER_BUCHERT, TX_MUD0_016);
	else {
		showText(TX_SPEAKER_BUCHERT, TX_MUD0_040);
		showText(TX_SPEAKER_MUDD,    TX_MUD0_030);
		showText(TX_SPEAKER_BUCHERT, TX_MUD0_041);
		showText(TX_SPEAKER_MUDD,    TX_MUD0_036);
		showText(TX_SPEAKER_BUCHERT, TX_MUD0_039);
		showText(TX_SPEAKER_KIRK,    TX_MUD0_008);
	}
}

void Room::mudd0TalkToMudd() {
	showText(TX_SPEAKER_MUDD,  TX_MUD0_032);
	showText(TX_SPEAKER_SPOCK, TX_MUD0_024);
	showText(TX_SPEAKER_MUDD,  TX_MUD0_035);
	showText(TX_SPEAKER_KIRK,  TX_MUD0_004);
}

}
