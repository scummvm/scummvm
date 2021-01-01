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

#define OBJECT_BRITTANY 8
#define OBJECT_DOOR 9
#define OBJECT_HYPO 10
#define OBJECT_DRILL 11
#define OBJECT_LEFT_READINGS 12

#define HOTSPOT_DOOR 0x20
#define HOTSPOT_LEFT_BED 0x21
#define HOTSPOT_RIGHT_BED 0x22
#define HOTSPOT_LEFT_READINGS 0x23
#define HOTSPOT_RIGHT_READINGS 0x24

namespace StarTrek {

extern const RoomAction veng4ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::veng4Tick1 },

	{ {ACTION_TALK, OBJECT_KIRK,     0, 0}, &Room::veng4TalkToKirk },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::veng4TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::veng4TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::veng4TalkToRedshirt },

	{ {ACTION_USE, OBJECT_IMEDKIT,  HOTSPOT_RIGHT_BED, 0}, &Room::veng4UseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_RIGHT_BED, 0}, &Room::veng4UseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_RIGHT_BED, 0}, &Room::veng4UseMccoyOnDeadGuy },

	{ {ACTION_USE, OBJECT_IHYPO,    OBJECT_BRITTANY,   0}, &Room::veng4UseHypoOnBrittany },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_BRITTANY,   0}, &Room::veng4UseMedkitOnBrittany },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_BRITTANY,   0}, &Room::veng4UseMTricorderOnBrittany },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_BRITTANY,   0}, &Room::veng4UseMTricorderOnBrittany },
	{ {ACTION_DONE_WALK, 5,                         0, 0}, &Room::veng4MccoyReachedBrittany },
	{ {ACTION_DONE_ANIM, 6,                         0, 0}, &Room::veng4UsedMedkitOnBrittany },
	{ {ACTION_DONE_ANIM, 7,                         0, 0}, &Room::veng4MccoyScannedBrittany },

	{ {ACTION_TALK, OBJECT_BRITTANY,                0, 0}, &Room::veng4TalkToBrittany },
	{ {ACTION_LOOK, HOTSPOT_RIGHT_BED,              0, 0}, &Room::veng4LookAtDeadGuy },
	{ {ACTION_LOOK, HOTSPOT_RIGHT_READINGS,         0, 0}, &Room::veng4LookAtRightBedReadings },
	{ {ACTION_LOOK, HOTSPOT_DOOR,                   0, 0}, &Room::veng4LookAtDoorHotspot },
	{ {ACTION_LOOK, OBJECT_KIRK,                    0, 0}, &Room::veng4LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,                   0, 0}, &Room::veng4LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,                   0, 0}, &Room::veng4LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,                0, 0}, &Room::veng4LookAtRedshirt },
	{ {ACTION_LOOK, HOTSPOT_LEFT_BED,               0, 0}, &Room::veng4LookAtLeftBed },
	{ {ACTION_LOOK, HOTSPOT_LEFT_READINGS,          0, 0}, &Room::veng4LookAtLeftBedReadings },
	{ {ACTION_LOOK, OBJECT_BRITTANY,                0, 0}, &Room::veng4LookAtBrittany },

	{ {ACTION_USE,  OBJECT_ISTRICOR, OBJECT_DRILL, 0}, &Room::veng4LookAtDrill },
	{ {ACTION_LOOK, OBJECT_DRILL,               0, 0}, &Room::veng4LookAtDrill },
	{ {ACTION_USE,  OBJECT_ISTRICOR, OBJECT_HYPO,  0}, &Room::veng4LookAtHypoOnTable },
	{ {ACTION_LOOK, OBJECT_HYPO,                0, 0}, &Room::veng4LookAtHypoOnTable },
	{ {ACTION_LOOK, OBJECT_DOOR,                0, 0}, &Room::veng4LookAtDoorObject },
	{ {ACTION_LOOK, 0xff,                       0, 0}, &Room::veng4LookAnywhere },

	{ {ACTION_GET, OBJECT_HYPO,                 0, 0}, &Room::veng4GetHypo },
	{ {ACTION_DONE_WALK, 1,                     0, 0}, &Room::veng4ReachedHypo },
	{ {ACTION_DONE_ANIM, 2,                     0, 0}, &Room::veng4PickedUpHypo },

	{ {ACTION_GET, OBJECT_DRILL,                0, 0}, &Room::veng4GetDrill },
	{ {ACTION_DONE_WALK, 3,                     0, 0}, &Room::veng4ReachedDrill },
	{ {ACTION_DONE_ANIM, 4,                     0, 0}, &Room::veng4PickedUpDrill },

	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0}, &Room::veng4TouchedHotspot0 },
	{ {ACTION_WALK, OBJECT_DOOR,  0, 0}, &Room::veng4WalkToDoor },
	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0}, &Room::veng4WalkToDoor },

	// Common code
	{ {ACTION_TICK, 0xff, 0xff, 0xff}, &Room::vengaTick },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff,     0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff,     0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_LOOK, OBJECT_IHYPO,          0, 0}, &Room::vengaLookAtHypo },
	{ {ACTION_USE, OBJECT_ICOMM, OBJECT_KIRK, 0}, &Room::vengaUseCommunicator },

	// ENHANCEMENT (let object count for the "look" action, not just the hotspot)
	{ {ACTION_LOOK, OBJECT_LEFT_READINGS, 0, 0}, &Room::veng4LookAtLeftBedReadings },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

#define DOOR_X 0x13f
#define DOOR_Y 0xab

#define BRITTANY_X 0x5a
#define BRITTANY_Y 0xa5

#define LEFT_READINGS_X 0x5c
#define LEFT_READINGS_Y 0x75

void Room::veng4Tick1() {
	playMidiMusicTracks(MIDITRACK_31);

	loadActorAnim2(OBJECT_DOOR,  "s9r1dc", DOOR_X, DOOR_Y);
	loadActorAnim2(OBJECT_LEFT_READINGS, "s7r4m2", LEFT_READINGS_X, LEFT_READINGS_Y);

	if (!_awayMission->veng.usedMedkitOnBrittany) {
		loadActorAnim2(OBJECT_BRITTANY, "s9r1bm", BRITTANY_X, BRITTANY_Y);
		playVoc("VEN4LOOP");
	} else {
		playVoc("VE42LOOP");
		loadActorAnim2(OBJECT_BRITTANY, "s9r1bh", BRITTANY_X, BRITTANY_Y);
	}

	if (!_awayMission->veng.tookHypoFromSickbay)
		loadActorAnim2(OBJECT_HYPO, "s7r4h1", 0xea, 0x8b);
	if (!_awayMission->veng.tookDrillFromSickbay)
		loadActorAnim2(OBJECT_DRILL, "s7r4dl", 0xea, 0x87);

	if (_awayMission->veng.brittanyDead) {
		loadActorAnim2(OBJECT_BRITTANY, "s7r4bd", BRITTANY_X, BRITTANY_Y);
		loadActorAnim2(OBJECT_LEFT_READINGS, "s7r4m1d",  LEFT_READINGS_X, LEFT_READINGS_Y);
	}
}

void Room::veng4TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 2, true);
}

void Room::veng4TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 15, true);
}

void Room::veng4TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 17, true);
}

void Room::veng4TalkToRedshirt() {
	showText(TX_SPEAKER_KIJE, 22, true);
}

void Room::veng4UseMccoyOnDeadGuy() {
	showText(TX_SPEAKER_MCCOY, 7, true);
}

void Room::veng4UseHypoOnBrittany() {
	showText(TX_SPEAKER_MCCOY, 8, true);
}

void Room::veng4UseMedkitOnBrittany() {
	_roomVar.veng.usingMedkitOnBrittany = true;
	veng4UseMccoyOnBrittany();
}

void Room::veng4UseMTricorderOnBrittany() {
	_roomVar.veng.usingMedkitOnBrittany = false;
	veng4UseMccoyOnBrittany();
}

void Room::veng4UseMccoyOnBrittany() {
	if (_awayMission->veng.brittanyDead)
		showText(TX_SPEAKER_MCCOY, 13, true);
	else if (_awayMission->veng.usedMedkitOnBrittany)
		showText(TX_SPEAKER_MCCOY,  9, true);
	else {
		if (!_awayMission->veng.lookedAtBrittany) {
			_awayMission->veng.lookedAtBrittany = true;
			showText(TX_SPEAKER_KIRK,  5, true);
		}
		_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_E;
		walkCrewmanC(OBJECT_MCCOY, 0x3c, 0xa0, &Room::veng4MccoyReachedBrittany);
	}
}

void Room::veng4MccoyReachedBrittany() {
	if (!_roomVar.veng.usingMedkitOnBrittany) {
		loadActorAnimC(OBJECT_MCCOY, "mscane", -1, -1, &Room::veng4MccoyScannedBrittany);
		playSoundEffectIndex(kSfxTricorder);
	} else {
		loadActorAnimC(OBJECT_MCCOY, "museme", -1, -1, &Room::veng4UsedMedkitOnBrittany);
	}
}

void Room::veng4UsedMedkitOnBrittany() {
	loadActorAnim2(OBJECT_BRITTANY, "s9r1bh");
	_awayMission->veng.getPointsForHealingBrittany = true;
	_awayMission->veng.usedMedkitOnBrittany = true;
	_awayMission->veng.kirkShouldSuggestReestablishingPower = true;
	showText(TX_SPEAKER_MCCOY, 9, true);
}

void Room::veng4MccoyScannedBrittany() {
	showText(TX_SPEAKER_MCCOY, 11, true);
	showText(TX_SPEAKER_KIRK,   1, true);

	_roomVar.veng.usingMedkitOnBrittany = true;
	veng4UseMccoyOnBrittany();
}

void Room::veng4TalkToBrittany() {
	if (_awayMission->veng.talkedToBrittany)
		showText(TX_SPEAKER_MCCOY, 12, true);
	else if (!_awayMission->veng.usedMedkitOnBrittany)
		showText(TX_SPEAKER_MCCOY, 10, true);
	else {
		_awayMission->veng.talkedToBrittany = true;
		showText(TX_SPEAKER_BRITTANY_MARATA, 19, true);
		showText(TX_SPEAKER_KIRK,             4, true);
		showText(TX_SPEAKER_BRITTANY_MARATA, 20, true);
		showText(TX_SPEAKER_KIRK,             6, true);
		showText(TX_SPEAKER_BRITTANY_MARATA, 21, true);
		showText(TX_SPEAKER_KIRK,             3, true);

		loadActorAnim2(OBJECT_BRITTANY, "s9r1bm", BRITTANY_X, BRITTANY_Y);
		showText(TX_SPEAKER_MCCOY, 16, true);
		loadActorAnim2(OBJECT_LEFT_READINGS, "s7r4m1", LEFT_READINGS_X, LEFT_READINGS_Y);
		loadActorAnim2(OBJECT_BRITTANY, "s7r4bd", BRITTANY_X, BRITTANY_Y);
		_awayMission->veng.brittanyDead = true;
		stopAllVocSounds();
		playVoc("VE42LOOP");
	}
}

void Room::veng4LookAtDeadGuy() {
	showDescription(14, true);
}

void Room::veng4LookAtRightBedReadings() {
	showDescription(13, true);
}

void Room::veng4LookAtDoorHotspot() {
	showDescription(8, true);
}

void Room::veng4LookAtKirk() {
	showDescription(2, true);
}

void Room::veng4LookAtSpock() {
	showText(TX_SPEAKER_SPOCK, 3, true);
}

void Room::veng4LookAtMccoy() {
	showText(TX_SPEAKER_MCCOY, 0, true);
}

void Room::veng4LookAtRedshirt() {
	showText(TX_SPEAKER_KIJE, 1, true);
}

void Room::veng4LookAtLeftBed() {
	if (_awayMission->veng.brittanyDead) {
		// ENHANCEMENT: Original game showed a string with no corresponding audio ("This
		// bed contains the body of a recently deceaced crewman." Yes that's a typo.)
		// Instead, show some appropriate voiced text.
		showDescription(17, true);
	} else
		showDescription( 7, true);
}

void Room::veng4LookAtLeftBedReadings() {
	if (!_awayMission->veng.brittanyDead)
		showDescription( 6, true);
	else
		showDescription( 5, true);
}

void Room::veng4LookAtBrittany() {
	if (_awayMission->veng.brittanyDead)
		showDescription(17, true);
	else if (!_awayMission->veng.lookedAtBrittany) {
		_awayMission->veng.lookedAtBrittany = true;
		showText(TX_SPEAKER_KIRK,   5, true);
		showText(TX_SPEAKER_MCCOY, 14, true);
	} else if (_awayMission->veng.talkedToBrittany)
		showDescription(17, true);
	else
		showDescription( 4, true);
}

void Room::veng4LookAtDrill() {
	showDescription(12, true);
}

void Room::veng4LookAtHypoOnTable() {
	showDescription(10, true);
}

void Room::veng4LookAtDoorObject() {
	showDescription( 9, true);
}

void Room::veng4LookAnywhere() {
	showDescription(11, true);
}

void Room::veng4GetHypo() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xea, 0xae, &Room::veng4ReachedHypo);
}

void Room::veng4ReachedHypo() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::veng4PickedUpHypo);
}

void Room::veng4PickedUpHypo() {
	giveItem(OBJECT_IHYPO);
	_awayMission->veng.tookHypoFromSickbay = true;
	loadActorStandAnim(OBJECT_HYPO);
	showDescription(15, true);
	walkCrewman(OBJECT_KIRK, 0xcd, 0xc2);
	_awayMission->disableInput = false;
}

void Room::veng4GetDrill() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xea, 0xae, &Room::veng4ReachedDrill);
}

void Room::veng4ReachedDrill() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::veng4PickedUpDrill);
}

void Room::veng4PickedUpDrill() {
	giveItem(OBJECT_IDRILL);
	_awayMission->veng.tookDrillFromSickbay = true;
	loadActorStandAnim(OBJECT_DRILL);
	showDescription(16, true);
	walkCrewman(OBJECT_KIRK, 0xcd, 0xc2);
	_awayMission->disableInput = false;
}

void Room::veng4TouchedHotspot0() { // Trigger door opening
	if (_roomVar.veng.walkingToDoor) {
		playSoundEffectIndex(kSfxDoor);
		loadActorAnim(OBJECT_DOOR, "s9r1do", DOOR_X, DOOR_Y);
	}
}

void Room::veng4WalkToDoor() {
	_roomVar.veng.walkingToDoor = true;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0x12e, 0xab);
}

}
