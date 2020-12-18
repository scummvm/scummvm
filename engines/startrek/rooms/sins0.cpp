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

#define OBJECT_DOOR 8

#define HOTSPOT_DOOR 0x20
#define HOTSPOT_GROUND 0x21
#define HOTSPOT_STATUE 0x22
#define HOTSPOT_PLANET 0x23
#define HOTSPOT_SKY 0x24 // Not mapped anywhere?

namespace StarTrek {

extern const RoomAction sins0ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::sins0Tick1 },

	{ {ACTION_LOOK, 0xff,            0, 0}, &Room::sins0LookAnywhere },
	{ {ACTION_LOOK, OBJECT_DOOR,     0, 0}, &Room::sins0LookAtOpenDoor },
	{ {ACTION_LOOK, HOTSPOT_DOOR,    0, 0}, &Room::sins0LookAtClosedDoor },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::sins0TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::sins0TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::sins0TalkToRedshirt },
	{ {ACTION_LOOK, HOTSPOT_GROUND,  0, 0}, &Room::sins0LookAtGround },
	{ {ACTION_GET, HOTSPOT_GROUND,   0, 0}, &Room::sins0GetRock },
	{ {ACTION_DONE_ANIM, 1,          0, 0}, &Room::sins0PickedUpRock },

	{ {ACTION_USE, OBJECT_ISTRICOR,           0xff, 0}, &Room::sins0UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_GROUND, 0}, &Room::sins0UseSTricorderOnGround },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_PLANET, 0}, &Room::sins0UseSTricorderOnPlanet },

	{ {ACTION_TICK, 40,              0, 0}, &Room::sins0Tick40 },
	{ {ACTION_LOOK, HOTSPOT_STATUE,  0, 0}, &Room::sins0LookAtStatue },
	{ {ACTION_LOOK, HOTSPOT_PLANET,  0, 0}, &Room::sins0LookAtPlanet },
	{ {ACTION_LOOK, HOTSPOT_SKY,     0, 0}, &Room::sins0LookAtSky },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0}, &Room::sins0LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0}, &Room::sins0LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0}, &Room::sins0LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::sins0LookAtRedshirt },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_DOOR,    0}, &Room::sins0UseSTricorderOnClosedDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_STATUE,  0}, &Room::sins0UseSTricorderOnStatue },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_KIRK,     0}, &Room::sins0UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_SPOCK,    0}, &Room::sins0UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_MCCOY,    0}, &Room::sins0UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_REDSHIRT, 0}, &Room::sins0UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK,     0}, &Room::sins0UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK,    0}, &Room::sins0UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY,    0}, &Room::sins0UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0}, &Room::sins0UseMTricorderOnCrewman },

	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0}, &Room::sins0UseCommunicator },
	{ {ACTION_WALK, HOTSPOT_DOOR,   0, 0}, &Room::sins0WalkToDoor },
	{ {ACTION_WALK, OBJECT_DOOR,    0, 0}, &Room::sins0WalkToDoor },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::sins0Tick1() {
	playVoc("SIN0LOOP");

	if (!_awayMission->sins.enteredRoom0FirstTime)
		_awayMission->disableInput = 2;

	if (_awayMission->sins.openedInnerDoor)
		loadActorAnim2(OBJECT_DOOR, "s0dr2", 0, 0);
	else if (_awayMission->sins.openedOuterDoor)
		loadActorAnim2(OBJECT_DOOR, "s0dr1", 0, 0);

	playMidiMusicTracks(MIDITRACK_27, -3);
}

void Room::sins0LookAnywhere() {
	showDescription(9, true);
}

void Room::sins0LookAtOpenDoor() {
	showDescription(5, true);
}

void Room::sins0LookAtClosedDoor() {
	showDescription(8, true);
}

void Room::sins0TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 9, true);
}

void Room::sins0TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 11, true);
}

void Room::sins0TalkToRedshirt() {
	// ENHANCEMENT: This function had two implementations. The first (the originally used
	// one) is a generic "area is secure" text. In the second (which was unused), he
	// comments on how the small moon could have an atmosphere. This is more interesting
	// and relevant, so that implementation is used instead.
	if (false)
		showText(TX_SPEAKER_MOSHER, 24, true);
	else {
		showText(TX_SPEAKER_MOSHER, 26, true);
		showText(TX_SPEAKER_SPOCK,  22, true);
		showText(TX_SPEAKER_KIRK,   6, true);
	}
}

void Room::sins0LookAtGround() {
	showDescription(6, true);
}

void Room::sins0GetRock() {
	_awayMission->disableInput = true;
	loadActorAnimC(OBJECT_KIRK, "kpickw", -1, -1, &Room::sins0PickedUpRock);
	_awayMission->sins.gatheredClues |= 1;
}

void Room::sins0PickedUpRock() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
	showDescription(10, true);
	giveItem(OBJECT_IS8ROCKS);
}

void Room::sins0UseSTricorderAnywhere() {
	spockScan(DIR_S, 18, false, true);
}

void Room::sins0UseSTricorderOnGround() {
	spockScan(DIR_S, 17, false, true);
	_awayMission->sins.gatheredClues |= 1;
}

void Room::sins0UseSTricorderOnPlanet() {
	spockScan(DIR_S, 16, false, true);
}

void Room::sins0Tick40() {
	if (!_awayMission->sins.enteredRoom0FirstTime) {
		_awayMission->disableInput = false;
		showText(TX_SPEAKER_UHURA, 82, true);
		showText(TX_SPEAKER_KIRK,  3, true);
		showText(TX_SPEAKER_SCOTT, 48 + SCOTTY_MESSAGE_OFFSET);
		showText(TX_SPEAKER_KIRK,  7, true);
		showText(TX_SPEAKER_SCOTT, 50 + SCOTTY_MESSAGE_OFFSET);
		showText(TX_SPEAKER_KIRK,  5, true);
		showText(TX_SPEAKER_SCOTT, 49 + SCOTTY_MESSAGE_OFFSET);
		showText(TX_SPEAKER_KIRK,  1, true);
		_awayMission->sins.enteredRoom0FirstTime = true;
	}
}

void Room::sins0LookAtStatue() {
	showDescription(11, true);
	showText(TX_SPEAKER_MCCOY,  15, true);
	showText(TX_SPEAKER_SPOCK,  21, true);
	showText(TX_SPEAKER_MOSHER, 27, true);
	showText(TX_SPEAKER_MCCOY,  14, true);
}

void Room::sins0LookAtPlanet() {
	showDescription(7, true);
}

void Room::sins0LookAtSky() {
	// This seems unused, due to HOTSPOT_SKY not being mapped anywhere?
	showDescription(4, true);
}

void Room::sins0LookAtKirk() {
	showDescription(2, true);
}

void Room::sins0LookAtSpock() {
	showDescription(3, true);
}

void Room::sins0LookAtMccoy() {
	showDescription(0, true);
}

void Room::sins0LookAtRedshirt() {
	showDescription(1, true);
}

void Room::sins0UseSTricorderOnClosedDoor() {
	spockScan(DIR_S, 19, false, true);
	showText(TX_SPEAKER_MOSHER, 25, true);
	showText(TX_SPEAKER_MCCOY,  13, true);
	showText(TX_SPEAKER_MOSHER, 28, true);
}

void Room::sins0UseSTricorderOnStatue() {
	spockScan(DIR_S, 20, false, true);
	showText(TX_SPEAKER_KIRK,  8, true);
	showText(TX_SPEAKER_SPOCK, 23, true);

	if (!_awayMission->sins.gotPointsForScanningStatue) {
		_awayMission->sins.gotPointsForScanningStatue = true;
		_awayMission->sins.missionScore += 1;
	}
}

void Room::sins0UseMedkitOnCrewman() {
	showText(TX_SPEAKER_MCCOY, 12, true);
}

void Room::sins0UseMTricorderOnCrewman() {
	mccoyScan(DIR_S, 10, false, true);
}

void Room::sins0UseCommunicator() {
	if (!_awayMission->sins.scottyInformedKirkAboutVirus) {
		showText(TX_SPEAKER_KIRK,  4, true);
		showText(TX_SPEAKER_SCOTT, 1 + SCOTTY_MESSAGE_OFFSET, true);
		showText(TX_SPEAKER_KIRK,  2, true);
	} else
		showText(TX_SPEAKER_UHURA, 69, true);
}

void Room::sins0WalkToDoor() {
	walkCrewman(OBJECT_KIRK, 0x5f, 0x8c);
}

}
