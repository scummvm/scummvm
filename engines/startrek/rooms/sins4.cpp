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

#if 0
#include "startrek/room.h"

#define OBJECT_NORTH_DOOR 8

#define HOTSPOT_NORTH_DOOR 0x20
#define HOTSPOT_PANEL 0x21
#define HOTSPOT_BEAM 0x22
#define HOTSPOT_LIGHT 0x23
#define HOTSPOT_WEST_DOOR 0x24
#define HOTSPOT_EAST_DOOR 0x25

namespace StarTrek {

extern const RoomAction sins4ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::sins4Tick1 },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_PANEL,      0}, &Room::sins4UseSTricorderOnPanel },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_PANEL,      0}, &Room::sins4UseSpockOnPanel },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_NORTH_DOOR, 0}, &Room::sins4UsePhaserOnNorthDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_NORTH_DOOR, 0}, &Room::sins4UsePhaserOnNorthDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_PANEL,      0}, &Room::sins4UsePhaserOnPanel },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_PANEL,      0}, &Room::sins4UsePhaserOnPanel },

	{ {ACTION_USE, OBJECT_IIDCARD,  HOTSPOT_PANEL,      0}, &Room::sins4UseIDCardOnPanel },
	{ {ACTION_DONE_WALK, 2,                          0, 0}, &Room::sins4KirkReachedPanel },
	{ {ACTION_DONE_ANIM, 5,                          0, 0}, &Room::sins4KirkPutCardInPanel },
	{ {ACTION_USE, OBJECT_IS8ROCKS, HOTSPOT_PANEL,      0}, &Room::sins4UseRockOnPanel },
	{ {ACTION_DONE_WALK, 3,                          0, 0}, &Room::sins4KirkReachedPanelWithRock },
	{ {ACTION_DONE_ANIM, 4,                          0, 0}, &Room::sins4KirkUsedRockOnPanel },

	{ {ACTION_LOOK, HOTSPOT_WEST_DOOR,  0, 0}, &Room::sins4LookAtWestDoor },
	{ {ACTION_LOOK, HOTSPOT_EAST_DOOR,  0, 0}, &Room::sins4LookAtEastDoor },
	{ {ACTION_LOOK, 0xff,               0, 0}, &Room::sins4LookAnywhere },
	{ {ACTION_LOOK, HOTSPOT_PANEL,      0, 0}, &Room::sins4LookAtPanel },
	{ {ACTION_TICK, 40,                 0, 0}, &Room::sins4Tick40 },
	{ {ACTION_LOOK, OBJECT_KIRK,        0, 0}, &Room::sins4LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,       0, 0}, &Room::sins4LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,       0, 0}, &Room::sins4LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,    0, 0}, &Room::sins4LookAtRedshirt },
	{ {ACTION_LOOK, HOTSPOT_LIGHT,      0, 0}, &Room::sins4LookAtLight },
	{ {ACTION_LOOK, HOTSPOT_BEAM,       0, 0}, &Room::sins4LookAtBeam },
	{ {ACTION_LOOK, OBJECT_NORTH_DOOR,  0, 0}, &Room::sins4LookAtOpenNorthDoor },
	{ {ACTION_LOOK, HOTSPOT_NORTH_DOOR, 0, 0}, &Room::sins4LookAtClosedNorthDoor },
	{ {ACTION_TALK, OBJECT_KIRK,        0, 0}, &Room::sins4TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,       0, 0}, &Room::sins4TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,       0, 0}, &Room::sins4TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT,    0, 0}, &Room::sins4TalkToRedshirt },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_NORTH_DOOR, 0}, &Room::sins4UseSTricorderOnNorthDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff,               0}, &Room::sins4UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ICOMM,    0xff,               0}, &Room::sins4UseCommunicator },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_NORTH_DOOR, 0}, &Room::sins4UseMccoyOnNorthDoor },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_NORTH_DOOR, 0}, &Room::sins4UseRedshirtOnNorthDoor },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_NORTH_DOOR, 0}, &Room::sins4UseSpockOnNorthDoor },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_PANEL,      0}, &Room::sins4UseMccoyOnPanel },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_PANEL,      0}, &Room::sins4UseRedshirtOnPanel },

	{ {ACTION_WALK, HOTSPOT_NORTH_DOOR, 0, 0}, &Room::sins4WalkToNorthDoor },
	{ {ACTION_WALK, OBJECT_NORTH_DOOR,  0, 0}, &Room::sins4WalkToNorthDoor },
	{ {ACTION_WALK, HOTSPOT_WEST_DOOR, 0, 0}, &Room::sins4WalkToWestDoor },
	{ {ACTION_WALK, HOTSPOT_EAST_DOOR, 0, 0}, &Room::sins4WalkToEastDoor },

	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_KIRK,     0}, &Room::sins4UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_SPOCK,    0}, &Room::sins4UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_MCCOY,    0}, &Room::sins4UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_REDSHIRT, 0}, &Room::sins4UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK,     0}, &Room::sins4UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK,    0}, &Room::sins4UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY,    0}, &Room::sins4UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0}, &Room::sins4UseMTricorderOnCrewman },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

enum sins4TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_EVERTS,
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets sins4TextOffsets[] = {
	//{ TX_SPEAKER_KIRK, 2597, 0 },
	//{ TX_SPEAKER_MCCOY, 2622, 0 },
	//{ TX_SPEAKER_SPOCK, 2632, 0 },
	//{ TX_SPEAKER_EVERTS, 2642, 0 },
	{          -1, 0,    0 }
};

extern const RoomText sins4Texts[] = {
      { -1, Common::UNK_LANG, "" }
};

void Room::sins4Tick1() {
	playVoc("SIN4LOOP");

	if (!_awayMission->sins.enteredRoom4FirstTime)
		_awayMission->disableInput = 2;

	if (_awayMission->sins.unlockedIDCardDoor)
		loadActorAnim2(OBJECT_NORTH_DOOR, "s4dro", 0, 0);

	playMidiMusicTracks(MIDITRACK_27, -3);
}

void Room::sins4UseSTricorderOnPanel() {
	spockScan(DIR_N, 20, false);
	_awayMission->sins.scannedKeycardLock = true;
	_awayMission->sins.gatheredClues |= 4;

	if (_awayMission->sins.gatheredClues == 7) {
		showText(TX_SPEAKER_SPOCK, 10);
		_awayMission->sins.gatheredClues |= 8;
	}
}

void Room::sins4UseSpockOnPanel() {
	// NOTE: two implementations of this function (index 18 and 21), one unused
	showText(TX_SPEAKER_SPOCK, 18);
}

void Room::sins4UsePhaserOnNorthDoor() {
	showText(TX_SPEAKER_SPOCK, 9);
}

void Room::sins4UsePhaserOnPanel() {
	showText(TX_SPEAKER_SPOCK, 8);
}

void Room::sins4UseIDCardOnPanel() {
	if (_awayMission->sins.unlockedIDCardDoor)
		showDescription(6);
	else
		walkCrewmanC(OBJECT_KIRK, 0xb8, 0x86, &Room::sins4KirkReachedPanel);
}

void Room::sins4KirkReachedPanel() {
	_awayMission->sins.unlockedIDCardDoor = true;
	loseItem(OBJECT_IIDCARD);
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::sins4KirkPutCardInPanel);

	_awayMission->disableInput = true; // ENHANCEMENT: disable input
}

void Room::sins4KirkPutCardInPanel() {
	loadActorAnim2(OBJECT_NORTH_DOOR, "s4door", 0, 0, 1); // Callback "1" doesn't exist
	playVoc("MUR4E1");
	_awayMission->disableInput = false;
}

void Room::sins4UseRockOnPanel() {
	walkCrewmanC(OBJECT_KIRK, 0xb8, 0x86, &Room::sins4KirkReachedPanelWithRock);
}

void Room::sins4KirkReachedPanelWithRock() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::sins4KirkUsedRockOnPanel);
}

void Room::sins4KirkUsedRockOnPanel() {
	showDescription(12);
	showText(TX_SPEAKER_SPOCK, 22);

	if (_awayMission->sins.gatheredClues == 7) {
		showText(TX_SPEAKER_SPOCK, 10);
		_awayMission->sins.gatheredClues |= 8;
	}
}

void Room::sins4LookAtWestDoor() {
	showDescription(9);
}

void Room::sins4LookAtEastDoor() {
	showDescription(10);
}

void Room::sins4LookAnywhere() {
	showDescription(13);
}

void Room::sins4LookAtPanel() {
	showDescription(11);
}

void Room::sins4Tick40() {
	if (!_awayMission->sins.enteredRoom4FirstTime) {
		_awayMission->disableInput = false;
		showText(TX_SPEAKER_UHURA_GLOBAL, 88);
		showText(TX_SPEAKER_KIRK,  7);
		showText(TX_SPEAKER_UHURA_GLOBAL, 103);
		showText(TX_SPEAKER_SPOCK, 25);
		showText(TX_SPEAKER_KIRK,  3);
		showText(TX_SPEAKER_UHURA_GLOBAL, 107);
		showText(TX_SPEAKER_UHURA_GLOBAL, 105);
		showText(TX_SPEAKER_KIRK,  5);
		_awayMission->sins.enteredRoom4FirstTime = true;
	}
}

void Room::sins4LookAtKirk() {
	showDescription(2);
}

void Room::sins4LookAtSpock() {
	showDescription(4);
}

void Room::sins4LookAtMccoy() {
	showDescription(3);
}

void Room::sins4LookAtRedshirt() {
	showDescription(1);
}

void Room::sins4LookAtLight() {
	showDescription(0);
}

void Room::sins4LookAtBeam() {
	showDescription(5);
}

void Room::sins4LookAtOpenNorthDoor() {
	showDescription(8);
}

void Room::sins4LookAtClosedNorthDoor() {
	showDescription(7);
}

void Room::sins4TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  1);
	showText(TX_SPEAKER_SPOCK, 28);
	showText(TX_SPEAKER_KIRK,  4);
}

void Room::sins4TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 19);
}

void Room::sins4TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 15);
	showText(TX_SPEAKER_SPOCK, 24);
	showText(TX_SPEAKER_MCCOY, 17);
}

void Room::sins4TalkToRedshirt() {
	showText(TX_SPEAKER_MOSHER, 31);
}

void Room::sins4UseSTricorderOnNorthDoor() {
	spockScan(DIR_N, 27, false);
	showText(TX_SPEAKER_MOSHER, 32);
	showText(TX_SPEAKER_SPOCK,  26);
}

void Room::sins4UseSTricorderAnywhere() {
	spockScan(DIR_S, 11, false);

	if (!_awayMission->sins.gotPointsForScanningRoom4) {
		_awayMission->sins.missionScore += 1;
		_awayMission->sins.gotPointsForScanningRoom4 = true;
	}
}

void Room::sins4UseCommunicator() {
	if (!_awayMission->sins.enteredRoom3FirstTime) {
		showText(TX_SPEAKER_UHURA_GLOBAL, 100);
		showText(TX_SPEAKER_KIRK,  6);
		showText(TX_SPEAKER_UHURA_GLOBAL, TX_SIN4U83B);
		showText(TX_SPEAKER_KIRK,  2);
	} else
		showText(TX_SPEAKER_UHURA_GLOBAL, 73);
}

void Room::sins4UseMccoyOnNorthDoor() {
	showText(TX_SPEAKER_MCCOY, 16);
}

void Room::sins4UseRedshirtOnNorthDoor() {
	showText(TX_SPEAKER_MOSHER, 29);
}

void Room::sins4UseSpockOnNorthDoor() {
	showText(TX_SPEAKER_SPOCK, 23);
}

void Room::sins4UseMccoyOnPanel() {
	showText(TX_SPEAKER_MCCOY, 13);
}

void Room::sins4UseRedshirtOnPanel() {
	showText(TX_SPEAKER_MOSHER, 30);
}

void Room::sins4WalkToNorthDoor() {
	if (_awayMission->sins.unlockedIDCardDoor)
		walkCrewman(OBJECT_KIRK, 0x96, 0x7d);
}

void Room::sins4WalkToWestDoor() {
	walkCrewman(OBJECT_KIRK, 0x1b, 0x95);
}

void Room::sins4WalkToEastDoor() {
	walkCrewman(OBJECT_KIRK, 0x13c, 0x9b);
}

void Room::sins4UseMedkitOnCrewman() {
	showText(TX_SPEAKER_MCCOY, 14);
}

void Room::sins4UseMTricorderOnCrewman() {
	mccoyScan(DIR_S, 12, false);
}

}
#endif
