/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "startrek/room.h"

#define OBJECT_DOOR 8

#define HOTSPOT_DOOR 0x20
#define HOTSPOT_GROUND 0x21
#define HOTSPOT_STATUE 0x22
#define HOTSPOT_PLANET 0x23
#define HOTSPOT_SKY 0x24 // Not mapped anywhere?

namespace StarTrek {

extern const RoomAction sins0ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::sins0Tick1 },

	{ {ACTION_LOOK, 0xff,            0, 0, 0}, &Room::sins0LookAnywhere },
	{ {ACTION_LOOK, OBJECT_DOOR,     0, 0, 0}, &Room::sins0LookAtOpenDoor },
	{ {ACTION_LOOK, HOTSPOT_DOOR,    0, 0, 0}, &Room::sins0LookAtClosedDoor },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0, 0}, &Room::sins0TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0, 0}, &Room::sins0TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::sins0TalkToRedshirt },
	{ {ACTION_LOOK, HOTSPOT_GROUND,  0, 0, 0}, &Room::sins0LookAtGround },
	{ {ACTION_GET, HOTSPOT_GROUND,   0, 0, 0}, &Room::sins0GetRock },
	{ {ACTION_DONE_ANIM, 1,          0, 0, 0}, &Room::sins0PickedUpRock },

	{ {ACTION_USE, OBJECT_ISTRICOR,           0xff, 0, 0}, &Room::sins0UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_GROUND, 0, 0}, &Room::sins0UseSTricorderOnGround },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_PLANET, 0, 0}, &Room::sins0UseSTricorderOnPlanet },

	{ {ACTION_TICK, 40,              0, 0, 0}, &Room::sins0Tick40 },
	{ {ACTION_LOOK, HOTSPOT_STATUE,  0, 0, 0}, &Room::sins0LookAtStatue },
	{ {ACTION_LOOK, HOTSPOT_PLANET,  0, 0, 0}, &Room::sins0LookAtPlanet },
	{ {ACTION_LOOK, HOTSPOT_SKY,     0, 0, 0}, &Room::sins0LookAtSky },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0, 0}, &Room::sins0LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0, 0}, &Room::sins0LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0, 0}, &Room::sins0LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::sins0LookAtRedshirt },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_DOOR,    0, 0}, &Room::sins0UseSTricorderOnClosedDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_STATUE,  0, 0}, &Room::sins0UseSTricorderOnStatue },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_KIRK,     0, 0}, &Room::sins0UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_SPOCK,    0, 0}, &Room::sins0UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_MCCOY,    0, 0}, &Room::sins0UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_REDSHIRT, 0, 0}, &Room::sins0UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK,     0, 0}, &Room::sins0UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK,    0, 0}, &Room::sins0UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY,    0, 0}, &Room::sins0UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0, 0}, &Room::sins0UseMTricorderOnCrewman },

	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0, 0}, &Room::sins0UseCommunicator },
	{ {ACTION_WALK, HOTSPOT_DOOR,   0, 0, 0}, &Room::sins0WalkToDoor },
	{ {ACTION_WALK, OBJECT_DOOR,    0, 0, 0}, &Room::sins0WalkToDoor },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum sins0TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_SCOTT, TX_SPEAKER_UHURA,
	TX_SPEAKER_MOSHER,
	TX_SIN0_001, TX_SIN0_002, TX_SIN0_003, TX_SIN0_004, TX_SIN0_005,
	TX_SIN0_006, TX_SIN0_007, TX_SIN0_008, TX_SIN0_009, TX_SIN0_010,
	TX_SIN0_011, TX_SIN0_012, TX_SIN0_013, TX_SIN0_014, TX_SIN0_015,
	TX_SIN0_016, TX_SIN0_017, TX_SIN0_018, TX_SIN0_019, TX_SIN0_020,
	TX_SIN0_021, TX_SIN0_022, TX_SIN0_023, TX_SIN0_024, TX_SIN0_025,
	TX_SIN0_026, TX_SIN0_027, TX_SIN0_028, TX_SIN0_S01, TX_SIN0_S48,
	TX_SIN0_S49, TX_SIN0_S50, TX_SIN0N000, TX_SIN0N001, TX_SIN0N002,
	TX_SIN0N003, TX_SIN0N004, TX_SIN0N005, TX_SIN0N006, TX_SIN0N007,
	TX_SIN0N008, TX_SIN0N009, TX_SIN0N010, TX_SIN0N011, TX_SIN0U069,
	TX_SIN0U082
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets sins0TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 289, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 300, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 310, 0, 0, 0 },
	{ TX_SPEAKER_SCOTT, 344, 0, 0, 0 },
	{ TX_SPEAKER_UHURA, 334, 0, 0, 0 },
	{ TX_SPEAKER_MOSHER, 320, 0, 0, 0 },
	{ TX_SIN0N000, 3805, 0, 0, 0 },
	{ TX_SIN0N001, 3994, 0, 0, 0 },
	{ TX_SIN0N002, 3594, 0, 0, 0 },
	{ TX_SIN0N003, 3694, 0, 0, 0 },
	{ TX_SIN0N004, 3461, 0, 0, 0 },
	{ TX_SIN0N005, 729, 0, 0, 0 },
	{ TX_SIN0N006, 1252, 0, 0, 0 },
	{ TX_SIN0N007, 3341, 0, 0, 0 },
	{ TX_SIN0N008, 804, 0, 0, 0 },
	{ TX_SIN0N009, 648, 0, 0, 0 },
	{ TX_SIN0N010, 1377, 0, 0, 0 },
	{ TX_SIN0N011, 2840, 0, 0, 0 },
	{ TX_SIN0U069, 5389, 0, 0, 0 },
	{ TX_SIN0U082, 1915, 0, 0, 0 },
	{ TX_SIN0_001, 2544, 0, 0, 0 },
	{ TX_SIN0_002, 5317, 0, 0, 0 },
	{ TX_SIN0_003, 1986, 0, 0, 0 },
	{ TX_SIN0_004, 5131, 0, 0, 0 },
	{ TX_SIN0_005, 2356, 0, 0, 0 },
	{ TX_SIN0_006, 2758, 0, 0, 0 },
	{ TX_SIN0_007, 2108, 0, 0, 0 },
	{ TX_SIN0_008, 4505, 0, 0, 0 },
	{ TX_SIN0_009, 924, 0, 0, 0 },
	{ TX_SIN0_010, 4922, 0, 0, 0 },
	{ TX_SIN0_011, 1054, 0, 0, 0 },
	{ TX_SIN0_012, 4742, 0, 0, 0 },
	{ TX_SIN0_013, 4362, 0, 0, 0 },
	{ TX_SIN0_014, 3214, 0, 0, 0 },
	{ TX_SIN0_015, 2896, 0, 0, 0 },
	{ TX_SIN0_016, 1750, 0, 0, 0 },
	{ TX_SIN0_017, 1613, 0, 0, 0 },
	{ TX_SIN0_018, 1476, 0, 0, 0 },
	{ TX_SIN0_019, 4132, 0, 0, 0 },
	{ TX_SIN0_020, 5506, 0, 0, 0 },
	{ TX_SIN0_021, 2978, 0, 0, 0 },
	{ TX_SIN0_022, 2715, 0, 0, 0 },
	{ TX_SIN0_023, 4595, 0, 0, 0 },
	{ TX_SIN0_024, 1197, 0, 0, 0 },
	{ TX_SIN0_025, 4233, 0, 0, 0 },
	{ TX_SIN0_026, 2634, 0, 0, 0 },
	{ TX_SIN0_027, 3086, 0, 0, 0 },
	{ TX_SIN0_028, 4421, 0, 0, 0 },
	{ TX_SIN0_S01, 5200, 0, 0, 0 },
	{ TX_SIN0_S48, 2023, 0, 0, 0 },
	{ TX_SIN0_S49, 2430, 0, 0, 0 },
	{ TX_SIN0_S50, 2175, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText sins0Texts[] = {
	  { -1, Common::UNK_LANG, "" }
};

void Room::sins0Tick1() {
	playVoc("SIN0LOOP");

	if (!_awayMission->sins.enteredRoom0FirstTime)
		_awayMission->disableInput = 2;

	if (_awayMission->sins.openedInnerDoor)
		loadActorAnim2(OBJECT_DOOR, "s0dr2", 0, 0);
	else if (_awayMission->sins.openedOuterDoor)
		loadActorAnim2(OBJECT_DOOR, "s0dr1", 0, 0);

	playMidiMusicTracks(MIDITRACK_27, kLoopTypeRepeat);
}

void Room::sins0LookAnywhere() {
	showDescription(TX_SIN0N009);
}

void Room::sins0LookAtOpenDoor() {
	showDescription(TX_SIN0N005);
}

void Room::sins0LookAtClosedDoor() {
	showDescription(TX_SIN0N008);
}

void Room::sins0TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_SIN0_009);
}

void Room::sins0TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_SIN0_011);
}

void Room::sins0TalkToRedshirt() {
	// ENHANCEMENT: This function had two implementations. The first (the originally used
	// one) is a generic "area is secure" text. In the second (which was unused), he
	// comments on how the small moon could have an atmosphere. This is more interesting
	// and relevant, so that implementation is used instead.
	if (false)
		showText(TX_SPEAKER_MOSHER, TX_SIN0_024);
	else {
		showText(TX_SPEAKER_MOSHER, TX_SIN0_026);
		showText(TX_SPEAKER_SPOCK,  TX_SIN0_022);
		showText(TX_SPEAKER_KIRK,   TX_SIN0_006);
	}
}

void Room::sins0LookAtGround() {
	showDescription(TX_SIN0N006);
}

void Room::sins0GetRock() {
	_awayMission->disableInput = true;
	loadActorAnimC(OBJECT_KIRK, "kpickw", -1, -1, &Room::sins0PickedUpRock);
	_awayMission->sins.gatheredClues |= 1;
}

void Room::sins0PickedUpRock() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
	showDescription(TX_SIN0N010);
	giveItem(OBJECT_IS8ROCKS);
}

void Room::sins0UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_SIN0_018);
}

void Room::sins0UseSTricorderOnGround() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_SIN0_017);
	_awayMission->sins.gatheredClues |= 1;
}

void Room::sins0UseSTricorderOnPlanet() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_SIN0_016);
}

void Room::sins0Tick40() {
	if (!_awayMission->sins.enteredRoom0FirstTime) {
		_awayMission->disableInput = false;
		showText(TX_SPEAKER_UHURA, TX_SIN0U082);
		showText(TX_SPEAKER_KIRK,  TX_SIN0_003);
		showText(TX_SPEAKER_SCOTT, TX_SIN0_S48);
		showText(TX_SPEAKER_KIRK,  TX_SIN0_007);
		showText(TX_SPEAKER_SCOTT, TX_SIN0_S50);
		showText(TX_SPEAKER_KIRK,  TX_SIN0_005);
		showText(TX_SPEAKER_SCOTT, TX_SIN0_S49);
		showText(TX_SPEAKER_KIRK,  TX_SIN0_001);
		_awayMission->sins.enteredRoom0FirstTime = true;
	}
}

void Room::sins0LookAtStatue() {
	showDescription(TX_SIN0N011);
	showText(TX_SPEAKER_MCCOY,  TX_SIN0_015);
	showText(TX_SPEAKER_SPOCK,  TX_SIN0_021);
	showText(TX_SPEAKER_MOSHER, TX_SIN0_027);
	showText(TX_SPEAKER_MCCOY,  TX_SIN0_014);
}

void Room::sins0LookAtPlanet() {
	showDescription(TX_SIN0N007);
}

void Room::sins0LookAtSky() {
	// This seems unused, due to HOTSPOT_SKY not being mapped anywhere?
	showDescription(TX_SIN0N004);
}

void Room::sins0LookAtKirk() {
	showDescription(TX_SIN0N002);
}

void Room::sins0LookAtSpock() {
	showDescription(TX_SIN0N003);
}

void Room::sins0LookAtMccoy() {
	showDescription(TX_SIN0N000);
}

void Room::sins0LookAtRedshirt() {
	showDescription(TX_SIN0N001);
}

void Room::sins0UseSTricorderOnClosedDoor() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_SIN0_019);
	showText(TX_SPEAKER_MOSHER, TX_SIN0_025);
	showText(TX_SPEAKER_MCCOY,  TX_SIN0_013);
	showText(TX_SPEAKER_MOSHER, TX_SIN0_028);
}

void Room::sins0UseSTricorderOnStatue() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_SIN0_020);
	showText(TX_SPEAKER_KIRK,  TX_SIN0_008);
	showText(TX_SPEAKER_SPOCK, TX_SIN0_023);

	if (!_awayMission->sins.gotPointsForScanningStatue) {
		_awayMission->sins.gotPointsForScanningStatue = true;
		_awayMission->sins.missionScore += 1;
	}
}

void Room::sins0UseMedkitOnCrewman() {
	showText(TX_SPEAKER_MCCOY, TX_SIN0_012);
}

void Room::sins0UseMTricorderOnCrewman() {
	mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_SIN0_010);
}

void Room::sins0UseCommunicator() {
	if (!_awayMission->sins.scottyInformedKirkAboutVirus) {
		showText(TX_SPEAKER_KIRK,  TX_SIN0_004);
		showText(TX_SPEAKER_SCOTT, TX_SIN0_S01);
		showText(TX_SPEAKER_KIRK,  TX_SIN0_002);
	} else
		showText(TX_SPEAKER_UHURA, TX_SIN0U069);
}

void Room::sins0WalkToDoor() {
	walkCrewman(OBJECT_KIRK, 0x5f, 0x8c);
}

}
