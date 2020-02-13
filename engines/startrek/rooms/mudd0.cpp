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

// BUG-ish: Sometimes when looking at Mudd, Kirk walks toward him, but not always. Does
// this happen in original?

namespace StarTrek {

void Room::mudd0Tick1() {
	playVoc("MUD0LOOP");

	// NOTE: Using the floppy version's code here; Mudd disappears from this room after
	// the first meeting. Fixes an issue where Mudd appears in the first room when he's
	// supposed to be in the medbay.
	// TODO: verify this matches with floppy version's code
	if (_awayMission->mudd.muddFirstRoomState != 0)
		_awayMission->mudd.muddFirstRoomState = 2;
	else {
		_awayMission->disableInput = 2;
		loadActorAnim(OBJECT_MUDD, "s4cbhr", 0xa2, 0xa9, 0);
	}
}

void Room::mudd0Tick50() {
	if (!_awayMission->mudd.enteredRoom0ForFirstTime) {
		playMidiMusicTracks(3);
		_awayMission->mudd.enteredRoom0ForFirstTime = true;
	}
}

void Room::mudd0Tick60() {
	if (_awayMission->mudd.muddFirstRoomState == 0) {
		// ENHANCEMENT: Move this into if statement (related to above enhancement)
		_awayMission->disableInput = false;

		_awayMission->mudd.muddFirstRoomState++;

		showText(TX_SPEAKER_MUDD, 37, true);
		showText(TX_SPEAKER_KIRK,  9, true);
		showText(TX_SPEAKER_MUDD, 28, true);
		showText(TX_SPEAKER_MCCOY,17, true);
		showText(TX_SPEAKER_MUDD, 26, true);
		showText(TX_SPEAKER_KIRK,  6, true);
		showText(TX_SPEAKER_MUDD, 33, true);
		showText(TX_SPEAKER_KIRK,  5, true);
		showText(TX_SPEAKER_MUDD, 29, true);
	}
}

void Room::mudd0UsePhaserOnMudd() {
	showText(TX_SPEAKER_MUDD, 25, true);
}

void Room::mudd0UseCommunicator() {
	showText(TX_SPEAKER_KIRK, 1, true);
	showText(TX_SPEAKER_UHURA, TX_STATICU1);
}

void Room::mudd0LookAtFoodBox() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	loadActorStandAnim(OBJECT_KIRK);
	showDescription(12, true);
}

void Room::mudd0LookAtComponentBox() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	loadActorStandAnim(OBJECT_KIRK);
	showDescription(13, true);
}

void Room::mudd0LookAnywhere() {
	showDescription(19, true);
}

void Room::mudd0LookAtMemoryDiskBox() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
	loadActorStandAnim(OBJECT_KIRK);
	showDescription(10, true);
}

void Room::mudd0LookAtDegrimerBox() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
	loadActorStandAnim(OBJECT_KIRK);
	showDescription(8, true);
}

void Room::mudd0LookAtLense() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	loadActorStandAnim(OBJECT_KIRK);
	showDescription(14, true);
}

void Room::mudd0UseSTricorderAnywhere() {
	// Narrator says something, not Spock (so we don't use "spockScan" function)
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showDescription(15, true);
}

void Room::mudd0UseMTricorderAnywhere() {
	// Narrator says something, not Mccoy (so we don't use "mccoyScan" function)
	// BUGFIX-ish: original game had "McCoy" as the speaker, which is inconsistent with
	// above.
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showDescription(15, true);
}

void Room::mudd0UseSTricorderOnMemoryDiskBox() {
	spockScan(DIR_N, 21, true);
}

void Room::mudd0UseSTricorderOnDegrimerBox() {
	// Narrator says something, not Spock (so we don't use "spockScan" function)
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	loadActorAnim2(OBJECT_SPOCK, "sscanw", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showDescription(0, true);
}

void Room::mudd0UseMTricorderOnLense() {
	if (_awayMission->mudd.discoveredLenseAndDegrimerFunction)
		mccoyScan(DIR_E, 12, true);
	else
		mccoyScan(DIR_E, 13, true);
}

void Room::mudd0GetLense() {
	if (_awayMission->mudd.gotLense)
		showDescription(16, true);
	else {
		_awayMission->mudd.gotLense = true;
		_awayMission->mudd.missionScore++;
		giveItem(OBJECT_ILENSES);
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
		loadActorAnim2(OBJECT_KIRK, "s5r1kg", -1, -1);
		_awayMission->timers[1] = 27;
	}
}

void Room::mudd0GetMemoryDisk() {
	if (_awayMission->mudd.gotMemoryDisk)
		showDescription(16, true);
	else {
		_awayMission->mudd.gotMemoryDisk = true;
		_awayMission->mudd.missionScore++;
		giveItem(OBJECT_IDISKS);
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		loadActorAnimC(OBJECT_KIRK, "kuseln", -1, -1, &Room::mudd0PickedUpItem);
	}
}

void Room::mudd0GetDegrimer() {
	if (_awayMission->mudd.gotDegrimer)
		showDescription(16, true);
	else {
		_awayMission->mudd.gotDegrimer = true;
		_awayMission->mudd.missionScore++;
		giveItem(OBJECT_IDEGRIME);
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
		loadActorAnimC(OBJECT_KIRK, "kusemw", -1, -1, &Room::mudd0PickedUpItem);
	}
}

// Timer 1 expired
void Room::mudd0PickedUpLense() {
	loadActorStandAnim(OBJECT_KIRK);
	mudd0PickedUpItem();
}

void Room::mudd0PickedUpItem() {
	_awayMission->disableInput = false; // NOTE: this was never set to true
	showDescription(COMMON_MESSAGE_OFFSET + 7, true);	// TX_LOV1N007
}


void Room::mudd0WalkToSouthDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0x5a, 0xc7);
}

void Room::mudd0TouchedHotspot1() { // Trigger bottom door opening
	if (_roomVar.mudd.walkingToDoor)
		playVoc("SMADOOR3");
}

void Room::mudd0WalkToNorthDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0xa0, 0x68);
}

void Room::mudd0TouchedHotspot0() { // Trigger top door opening
	if (_roomVar.mudd.walkingToDoor) {
		playVoc("SMADOOR3");
		loadActorAnim2(OBJECT_DOOR, "s4cbdo", 0xa0, 0x6b);
	}
}

void Room::mudd0UseMedkit() {
	showText(TX_SPEAKER_MCCOY, 11, true);
}

void Room::mudd0LookAtKirk() {
	showDescription(4, true);
}

void Room::mudd0LookAtSpock() {
	showDescription(7, true);
}

void Room::mudd0LookAtMccoy() {
	showDescription(1, true);
}

void Room::mudd0LookAtRedshirt() {
	showDescription(5, true);
}

void Room::mudd0LookAtMudd() {
	showDescription(3, true);
}

void Room::mudd0TalkToKirk() {
	if (_awayMission->mudd.muddFirstRoomState == 2)
		showText(TX_SPEAKER_KIRK, 10, true);
	else {
		showText(TX_SPEAKER_KIRK, 10, true);
		showText(TX_SPEAKER_MUDD, 31, true);
		showText(TX_SPEAKER_KIRK,  7, true);
	}
}

void Room::mudd0TalkToSpock() {
	if (_awayMission->mudd.muddFirstRoomState == 2)
		showText(TX_SPEAKER_SPOCK, 22, true);
	else {
		showText(TX_SPEAKER_SPOCK, 22, true);
		showText(TX_SPEAKER_MUDD,  34, true);
		showText(TX_SPEAKER_SPOCK, 23, true);
		showText(TX_SPEAKER_MCCOY, 15, true);
		showText(TX_SPEAKER_MUDD,  38, true);
	}
}

void Room::mudd0TalkToMccoy() {
	if (_awayMission->mudd.muddFirstRoomState == 2)
		showText(TX_SPEAKER_MCCOY, 16, true);
	else {
		showText(TX_SPEAKER_MCCOY, 16, true);
		showText(TX_SPEAKER_KIRK,   3, true);
		showText(TX_SPEAKER_MUDD,  27, true);
		showText(TX_SPEAKER_MCCOY, 14, true);
	}
}

void Room::mudd0TalkToRedshirt() {
	if (_awayMission->mudd.muddFirstRoomState == 2) {
		showText(TX_SPEAKER_BUCHERT, 39, true);
		showText(TX_SPEAKER_KIRK,     8, true);
	} else {
		showText(TX_SPEAKER_BUCHERT, 40, true);
		showText(TX_SPEAKER_MUDD,    30, true);
		showText(TX_SPEAKER_BUCHERT, 41, true);
		showText(TX_SPEAKER_MUDD,    36, true);
	}
}

void Room::mudd0TalkToMudd() {
	showText(TX_SPEAKER_MUDD,  32, true);
	showText(TX_SPEAKER_SPOCK, 24, true);
	showText(TX_SPEAKER_MUDD,  35, true);
	showText(TX_SPEAKER_KIRK,   4, true);
}

}
