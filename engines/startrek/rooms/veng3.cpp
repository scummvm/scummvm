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
#define OBJECT_SICKBAY_DOOR 9
#define OBJECT_DEBRIS 10
#define OBJECT_CABLE 11
#define OBJECT_DEAD_GUY 12
#define OBJECT_SUPPORT_BEAM 13

#define HOTSPOT_TURBOLIFT_2_DOOR 0x20
#define HOTSPOT_SICKBAY_DOOR     0x21

namespace StarTrek {

extern const RoomAction veng3ActionList[] = {
	{ {ACTION_TICK, 1,  0, 0}, &Room::veng3Tick1 },
	{ {ACTION_TICK, 15, 0, 0}, &Room::veng3Tick15 },
	{ {ACTION_TOUCHED_WARP, 0, 0, 0}, &Room::veng3TouchedTurbolift2 },
	{ {ACTION_TOUCHED_WARP, 1, 0, 0}, &Room::veng3TouchedTurbolift1 },

	{ {ACTION_WALK, OBJECT_DEBRIS,             0, 0}, &Room::veng3WalkToDoor },
	{ {ACTION_WALK, HOTSPOT_TURBOLIFT_2_DOOR,  0, 0}, &Room::veng3WalkToDoor },
	{ {ACTION_DONE_WALK, 2,                    0, 0}, &Room::veng3ReachedTurbolift2 },
	{ {ACTION_DONE_ANIM, 1,                    0, 0}, &Room::veng3Turbolift2Opened },

	{ {ACTION_WALK, OBJECT_SICKBAY_DOOR,  0, 0}, &Room::veng3WalkToSickbayDoor },
	{ {ACTION_WALK, HOTSPOT_SICKBAY_DOOR, 0, 0}, &Room::veng3WalkToSickbayDoor },
	{ {ACTION_DONE_WALK, 9,               0, 0}, &Room::veng3ReachedSickbayDoor },
	{ {ACTION_DONE_ANIM, 10,              0, 0}, &Room::veng3SickbayDoorOpened },

	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_SUPPORT_BEAM, 0}, &Room::veng3LookAtSupportBeam },
	{ {ACTION_LOOK, OBJECT_SUPPORT_BEAM,              0, 0}, &Room::veng3LookAtSupportBeam },

	{ {ACTION_LOOK, OBJECT_DEBRIS,             0, 0}, &Room::veng3LookAtDebris },
	{ {ACTION_LOOK, HOTSPOT_TURBOLIFT_2_DOOR,  0, 0}, &Room::veng3LookAtTurbolift2Door },
	{ {ACTION_LOOK, OBJECT_SICKBAY_DOOR,       0, 0}, &Room::veng3LookAtSickbayDoor },
	{ {ACTION_LOOK, HOTSPOT_SICKBAY_DOOR,      0, 0}, &Room::veng3LookAtSickbayDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_CABLE, 0}, &Room::veng3LookAtCable },
	{ {ACTION_LOOK, OBJECT_CABLE,              0, 0}, &Room::veng3LookAtCable },
	{ {ACTION_LOOK, OBJECT_DEAD_GUY,           0, 0}, &Room::veng3LookAtDeadGuy },
	{ {ACTION_LOOK, OBJECT_KIRK,               0, 0}, &Room::veng3LookAtKirk },
	{ {ACTION_LOOK, OBJECT_MCCOY,              0, 0}, &Room::veng3LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK,              0, 0}, &Room::veng3LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,           0, 0}, &Room::veng3LookAtRedshirt },
	{ {ACTION_LOOK, 0xff,                      0, 0}, &Room::veng3LookAnywhere },

	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_DEBRIS, 0}, &Room::veng3UseStunPhaserOnDebris },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_DEBRIS, 0}, &Room::veng3UseKillPhaserOnDebris },
	{ {ACTION_DONE_WALK, 4,                     0, 0}, &Room::veng3ReachedPositionToShootDebris },
	{ {ACTION_DONE_ANIM, 5,                     0, 0}, &Room::veng3DrewPhaserToShootDebris },
	{ {ACTION_DONE_ANIM, 3,                     0, 0}, &Room::veng3DebrisVaporized },
	{ {ACTION_DONE_ANIM, 6,                     0, 0}, &Room::veng3DebrisFellAgain },

	{ {ACTION_USE, OBJECT_SPOCK,    OBJECT_DEBRIS,    0}, &Room::veng3UseSpockOnDebris },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_DEBRIS,    0}, &Room::veng3UseRedshirtOnDebris },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_DEBRIS,    0}, &Room::veng3UseMccoyOnDebris },
	{ {ACTION_USE, OBJECT_IBEAM,    OBJECT_DEBRIS,    0}, &Room::veng3UseBeamOnDebris },
	{ {ACTION_DONE_WALK, 7,                        0, 0}, &Room::veng3ReachedPositionToPlaceBeam },
	{ {ACTION_DONE_ANIM, 8,                        0, 0}, &Room::veng3PlacedBeam },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_DEBRIS,    0}, &Room::veng3UseSTricorderOnDebris },
	{ {ACTION_USE, OBJECT_ILMD,     OBJECT_DEBRIS,    0}, &Room::veng3UseMolecularSawOnDebris },

	{ {ACTION_TALK, OBJECT_KIRK,               0, 0}, &Room::veng3TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,              0, 0}, &Room::veng3TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,              0, 0}, &Room::veng3TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT,           0, 0}, &Room::veng3TalkToRedshirt },
	{ {ACTION_GET,  OBJECT_DEBRIS,             0, 0}, &Room::veng3GetDebris },
	{ {ACTION_GET,  OBJECT_CABLE,              0, 0}, &Room::veng3GetCable },
	{ {ACTION_DONE_WALK, 11,                   0, 0}, &Room::veng3ReachedCable },
	{ {ACTION_DONE_ANIM, 12,                   0, 0}, &Room::veng3PickedUpCable },

	// Common code
	{ {ACTION_TICK, 0xff, 0xff, 0xff}, &Room::vengaTick },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff,     0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff,     0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_LOOK, OBJECT_IHYPO,          0, 0}, &Room::vengaLookAtHypo },
	{ {ACTION_USE, OBJECT_ICOMM, OBJECT_KIRK, 0}, &Room::vengaUseCommunicator },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_DEAD_GUY, 0}, &Room::vengaUseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_DEAD_GUY, 0}, &Room::vengaUseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_DEAD_GUY, 0}, &Room::vengaUseMccoyOnDeadGuy },
};

extern const int veng3NumActions = ARRAYSIZE(veng3ActionList);


void Room::veng3Tick1() {
	playVoc("VEN3LOOP");

	if (_awayMission->veng.putSupportBeamInSickbayHallway)
		loadActorAnim2(OBJECT_SUPPORT_BEAM, "s7r3bm", 0x8c, 0x61);

	loadMapFile("veng3");

	if (!_awayMission->veng.clearedDebris) {
		loadActorAnim2(OBJECT_DEBRIS, "s7r3d2", 0xa5, 0x68);
		loadMapFile("veng3b");
	}

	loadActorAnim2(OBJECT_DEAD_GUY, "s7r3c1", 0xc5, 0x99);

	if (!_awayMission->veng.tookCableFromSickbayHallway)
		loadActorAnim2(OBJECT_CABLE, "s7r3ca", 0xb7, 0x84);

	loadActorAnim2(OBJECT_SICKBAY_DOOR, "s7r3dc", 0x5d, 0x80);
}

void Room::veng3Tick15() {
	if (_awayMission->veng.kirkShouldSuggestReestablishingPower && !_awayMission->veng.impulseEnginesOn) {
		showText(TX_SPEAKER_KIRK, TX_VEN0_010);
		_awayMission->veng.kirkShouldSuggestReestablishingPower = false;
	}
}

void Room::veng3TouchedTurbolift2() {
	playSoundEffectIndex(SND_DOOR1);
	showRepublicMap(3, 1);
}

void Room::veng3TouchedTurbolift1() {
	playSoundEffectIndex(SND_DOOR1);
	showRepublicMap(3, 0);
}

void Room::veng3WalkToDoor() {
	if (_awayMission->veng.clearedDebris)
		walkCrewmanC(OBJECT_KIRK, 0xa6, 0x6e, &Room::veng3ReachedTurbolift2);
	else
		showDescription(TX_VEN3N006);
}

void Room::veng3ReachedTurbolift2() {
	_awayMission->disableInput = true;
	loadActorAnimC(OBJECT_DOOR, "s7r3db", 0xa6, 0x60, &Room::veng3Turbolift2Opened);
	playSoundEffectIndex(SND_DOOR1);
}

void Room::veng3Turbolift2Opened() {
	walkCrewman(OBJECT_KIRK, 0xa6, 0x61);
}

void Room::veng3WalkToSickbayDoor() {
	walkCrewmanC(OBJECT_KIRK, 0x68, 0x83, &Room::veng3ReachedSickbayDoor);
}

void Room::veng3ReachedSickbayDoor() {
	_awayMission->disableInput = true;
	loadActorAnimC(OBJECT_SICKBAY_DOOR, "s7r3da", 0x5b, 0x80, &Room::veng3SickbayDoorOpened);
	playSoundEffectIndex(SND_DOOR1);
}

void Room::veng3SickbayDoorOpened() {
	walkCrewman(OBJECT_KIRK, 0x5e, 0x83);
}

void Room::veng3LookAtSupportBeam() {
	showDescription(TX_VEN3N007);
}

void Room::veng3LookAtDebris() {
	if (!_awayMission->veng.clearedDebris)
		showDescription(TX_VEN3N009);
	else
		showDescription(TX_VEN3N007); // UNUSED (debris is gone)
}

void Room::veng3LookAtTurbolift2Door() {
	showDescription(TX_VEN3N012);

	if (!_awayMission->veng.lookedAtTurbolift2Door) {
		_awayMission->veng.lookedAtTurbolift2Door = true;
		if (!_awayMission->veng.clearedDebris) {
			showText(TX_SPEAKER_SPOCK, TX_VEN3_013);
			showText(TX_SPEAKER_MCCOY, TX_VEN3_006);
		}
	}
}

void Room::veng3LookAtSickbayDoor() {
	showDescription(TX_VEN3N011);
}

void Room::veng3LookAtCable() {
	showDescription(TX_VEN3N013);
}

void Room::veng3LookAtDeadGuy() {
	showDescription(TX_VEN3N005);
}

void Room::veng3LookAtKirk() {
	showDescription(TX_VEN3N001);
}

void Room::veng3LookAtMccoy() {
	showDescription(TX_VEN3N002);
}

void Room::veng3LookAtSpock() {
	showDescription(TX_VEN3N004);
}

void Room::veng3LookAtRedshirt() {
	showDescription(TX_VEN3N000);
}

void Room::veng3LookAnywhere() {
	showDescription(TX_VEN3N003);
}

void Room::veng3UseStunPhaserOnDebris() {
	showText(TX_SPEAKER_SPOCK, TX_VEN3_002);
}

void Room::veng3UseKillPhaserOnDebris() {
	if (!_awayMission->veng.clearedDebris) {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_KIRK, 0x96, 0xa8, &Room::veng3ReachedPositionToShootDebris);
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N; // ENHANCEMENT
	}
}

void Room::veng3ReachedPositionToShootDebris() {
	loadActorAnimC(OBJECT_KIRK, "kfiren", -1, -1, &Room::veng3DrewPhaserToShootDebris);
}

void Room::veng3DrewPhaserToShootDebris() {
	loadActorAnimC(OBJECT_DEBRIS, "s7r3ph", 0x9c, 0x83, &Room::veng3DebrisVaporized);
	playSoundEffectIndex(SND_PHASSHOT);
}

void Room::veng3DebrisVaporized() {
	if (!_awayMission->veng.putSupportBeamInSickbayHallway) {
		playVoc("MUR1E4");
		loadActorAnimC(OBJECT_DEBRIS, "s7r3df", 0xa6, 0x60, &Room::veng3DebrisFellAgain);
		loadActorStandAnim(OBJECT_KIRK);
	} else {
		_awayMission->disableInput = false;
		loadMapFile("veng3");
		showText(TX_SPEAKER_SPOCK, TX_VEN3_015);
		_awayMission->veng.clearedDebris = true;
	}
}

void Room::veng3DebrisFellAgain() {
	loadActorAnim2(OBJECT_DEBRIS, "s7r3d2", 0xa5, 0x68);
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_SPOCK, TX_VEN3_014);
}

void Room::veng3UseSpockOnDebris() {
	if (!_awayMission->veng.putSupportBeamInSickbayHallway)
		showText(TX_SPEAKER_SPOCK, TX_VEN3_008);
	else {
		showText(TX_SPEAKER_SPOCK, TX_VEN3_011);
		// BUGFIX: Game would crash here due to jumping to non-code.
	}
}

void Room::veng3UseRedshirtOnDebris() {
	if (!_awayMission->veng.putSupportBeamInSickbayHallway)
		showText(TX_SPEAKER_KIJE, TX_VEN3_017);
	else {
		// BUGFIX: Original had a "jne" opcode with no comparison beforehand, causing it
		// to jump to some of Spock's code (TX_VEN3_007)? In any case, this makes more sense.
		showText(TX_SPEAKER_KIJE, TX_VEN3_016);
	}
}

void Room::veng3UseMccoyOnDebris() {
	showText(TX_SPEAKER_MCCOY, TX_VEN3_004);
}

void Room::veng3UseBeamOnDebris() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0x94, 0x6d, &Room::veng3ReachedPositionToPlaceBeam);
}

void Room::veng3ReachedPositionToPlaceBeam() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::veng3PlacedBeam);
	playVoc("MUR3E1S");
}

void Room::veng3PlacedBeam() {
	loadActorAnim2(OBJECT_SUPPORT_BEAM, "s7r3bm", 0x8c, 0x61);
	_awayMission->disableInput = false;
	_awayMission->veng.putSupportBeamInSickbayHallway = true;
	loseItem(OBJECT_IBEAM);
}

void Room::veng3UseSTricorderOnDebris() {
	if (!_awayMission->veng.putSupportBeamInSickbayHallway)
		spockScan(DIR_N, TX_VEN3_009);
	else
		spockScan(DIR_N, TX_VEN3_007);
}

void Room::veng3UseMolecularSawOnDebris() {
	showText(TX_SPEAKER_SPOCK, TX_VEN3_010);
}

void Room::veng3TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_VEN3_001);
}

void Room::veng3TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_VEN3_012);
}

void Room::veng3TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_VEN3_003);
}

void Room::veng3TalkToRedshirt() {
	showText(TX_SPEAKER_KIJE,  TX_VEN3_018);
	showText(TX_SPEAKER_MCCOY, TX_VEN3_005);
}

void Room::veng3GetDebris() {
	showDescription(TX_VEN3N010);
}

void Room::veng3GetCable() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xb7, 0x85, &Room::veng3ReachedCable);
}

void Room::veng3ReachedCable() {
	loadActorAnimC(OBJECT_KIRK, "kusehn", -1, -1, &Room::veng3PickedUpCable);
	playVoc("MUR4E6");
}

void Room::veng3PickedUpCable() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_CABLE);
	showDescription(TX_VEN3N014);
	giveItem(OBJECT_ICABLE2);
	_awayMission->veng.tookCableFromSickbayHallway = true;
}

}
