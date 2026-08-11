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
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::veng4Tick1 },

	{ {ACTION_TALK, OBJECT_KIRK,     0, 0, 0}, &Room::veng4TalkToKirk },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0, 0}, &Room::veng4TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0, 0}, &Room::veng4TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::veng4TalkToRedshirt },

	{ {ACTION_USE, OBJECT_IMEDKIT,  HOTSPOT_RIGHT_BED, 0, 0}, &Room::veng4UseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_RIGHT_BED, 0, 0}, &Room::veng4UseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_RIGHT_BED, 0, 0}, &Room::veng4UseMccoyOnDeadGuy },

	{ {ACTION_USE, OBJECT_IHYPO,    OBJECT_BRITTANY,   0, 0}, &Room::veng4UseHypoOnBrittany },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_BRITTANY,   0, 0}, &Room::veng4UseMedkitOnBrittany },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_BRITTANY,   0, 0}, &Room::veng4UseMTricorderOnBrittany },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_BRITTANY,   0, 0}, &Room::veng4UseMTricorderOnBrittany },
	{ {ACTION_DONE_WALK, 5,                         0, 0, 0}, &Room::veng4MccoyReachedBrittany },
	{ {ACTION_DONE_ANIM, 6,                         0, 0, 0}, &Room::veng4UsedMedkitOnBrittany },
	{ {ACTION_DONE_ANIM, 7,                         0, 0, 0}, &Room::veng4MccoyScannedBrittany },

	{ {ACTION_TALK, OBJECT_BRITTANY,                0, 0, 0}, &Room::veng4TalkToBrittany },
	{ {ACTION_LOOK, HOTSPOT_RIGHT_BED,              0, 0, 0}, &Room::veng4LookAtDeadGuy },
	{ {ACTION_LOOK, HOTSPOT_RIGHT_READINGS,         0, 0, 0}, &Room::veng4LookAtRightBedReadings },
	{ {ACTION_LOOK, HOTSPOT_DOOR,                   0, 0, 0}, &Room::veng4LookAtDoorHotspot },
	{ {ACTION_LOOK, OBJECT_KIRK,                    0, 0, 0}, &Room::veng4LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,                   0, 0, 0}, &Room::veng4LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,                   0, 0, 0}, &Room::veng4LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,                0, 0, 0}, &Room::veng4LookAtRedshirt },
	{ {ACTION_LOOK, HOTSPOT_LEFT_BED,               0, 0, 0}, &Room::veng4LookAtLeftBed },
	{ {ACTION_LOOK, HOTSPOT_LEFT_READINGS,          0, 0, 0}, &Room::veng4LookAtLeftBedReadings },
	{ {ACTION_LOOK, OBJECT_BRITTANY,                0, 0, 0}, &Room::veng4LookAtBrittany },

	{ {ACTION_USE,  OBJECT_ISTRICOR, OBJECT_DRILL, 0, 0}, &Room::veng4LookAtDrill },
	{ {ACTION_LOOK, OBJECT_DRILL,               0, 0, 0}, &Room::veng4LookAtDrill },
	{ {ACTION_USE,  OBJECT_ISTRICOR, OBJECT_HYPO,  0, 0}, &Room::veng4LookAtHypoOnTable },
	{ {ACTION_LOOK, OBJECT_HYPO,                0, 0, 0}, &Room::veng4LookAtHypoOnTable },
	{ {ACTION_LOOK, OBJECT_DOOR,                0, 0, 0}, &Room::veng4LookAtDoorObject },
	{ {ACTION_LOOK, 0xff,                       0, 0, 0}, &Room::veng4LookAnywhere },

	{ {ACTION_GET, OBJECT_HYPO,                 0, 0, 0}, &Room::veng4GetHypo },
	{ {ACTION_DONE_WALK, 1,                     0, 0, 0}, &Room::veng4ReachedHypo },
	{ {ACTION_DONE_ANIM, 2,                     0, 0, 0}, &Room::veng4PickedUpHypo },

	{ {ACTION_GET, OBJECT_DRILL,                0, 0, 0}, &Room::veng4GetDrill },
	{ {ACTION_DONE_WALK, 3,                     0, 0, 0}, &Room::veng4ReachedDrill },
	{ {ACTION_DONE_ANIM, 4,                     0, 0, 0}, &Room::veng4PickedUpDrill },

	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0}, &Room::veng4TouchedHotspot0 },
	{ {ACTION_WALK, OBJECT_DOOR,  0, 0, 0}, &Room::veng4WalkToDoor },
	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0, 0}, &Room::veng4WalkToDoor },

	// Common code
	{ {ACTION_TICK, 0xff, 0xff, 0xff, 0}, &Room::vengaTick },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff,     0, 0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff,     0, 0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_LOOK, OBJECT_IHYPO,          0, 0, 0}, &Room::vengaLookAtHypo },
	{ {ACTION_USE, OBJECT_ICOMM, OBJECT_KIRK, 0, 0}, &Room::vengaUseCommunicator },

	// ENHANCEMENT (let object count for the "look" action, not just the hotspot)
	{ {ACTION_LOOK, OBJECT_LEFT_READINGS, 0, 0, 0}, &Room::veng4LookAtLeftBedReadings },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum veng4TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_KIJE, TX_SPEAKER_BRITTANY_MARATA,
	TX_VEN4_001, TX_VEN4_002, TX_VEN4_003, TX_VEN4_004, TX_VEN4_005,
	TX_VEN4_006, TX_VEN4_007, TX_VEN4_008, TX_VEN4_009, TX_VEN4_010,
	TX_VEN4_011, TX_VEN4_012, TX_VEN4_013, TX_VEN4_014, TX_VEN4_015,
	TX_VEN4_016, TX_VEN4_017, TX_VEN4_019, TX_VEN4_020, TX_VEN4_021,
	TX_VEN4_022, TX_VEN4N000, TX_VEN4N001, TX_VEN4N002, TX_VEN4N003,
	TX_VEN4N004, TX_VEN4N005, TX_VEN4N006, TX_VEN4N007, TX_VEN4N008,
	TX_VEN4N009, TX_VEN4N010, TX_VEN4N011, TX_VEN4N012, TX_VEN4N013,
	TX_VEN4N014, TX_VEN4N015, TX_VEN4N016, TX_VEN4N017
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets veng4TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 4347, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 4358, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 4368, 0, 0, 0 },
	{ TX_SPEAKER_KIJE, 4378, 0, 0, 0 },
	{ TX_SPEAKER_BRITTANY_MARATA, 4390, 0, 0, 0 },
	{ TX_VEN4_001, 4583, 0, 0, 0 },
	{ TX_VEN4_002, 5485, 0, 0, 0 },
	{ TX_VEN4_003, 6238, 0, 0, 0 },
	{ TX_VEN4_004, 6005, 0, 0, 0 },
	{ TX_VEN4_005, 4882, 0, 0, 0 },
	{ TX_VEN4_006, 6124, 0, 0, 0 },
	{ TX_VEN4_007, 4431, 0, 0, 0 },
	{ TX_VEN4_008,  703, 0, 0, 0 },
	{ TX_VEN4_009, 6387, 0, 0, 0 },
	{ TX_VEN4_010, 1261, 0, 0, 0 },
	{ TX_VEN4_011, 4486, 0, 0, 0 },
	{ TX_VEN4_012, 6333, 0, 0, 0 },
	{ TX_VEN4_013, 6482, 0, 0, 0 },
	{ TX_VEN4_014, 5009, 0, 0, 0 },
	{ TX_VEN4_015, 5636, 0, 0, 0 },
	{ TX_VEN4_016, 6287, 0, 0, 0 },
	{ TX_VEN4_017, 5576, 0, 0, 0 },
	{ TX_VEN4_019, 5964, 0, 0, 0 },
	{ TX_VEN4_020, 6043, 0, 0, 0 },
	{ TX_VEN4_021, 6166, 0, 0, 0 },
	{ TX_VEN4_022, 5894, 0, 0, 0 },
	{ TX_VEN4N000, 4725, 0, 0, 0 },
	{ TX_VEN4N001, 4831, 0, 0, 0 },
	{ TX_VEN4N002, 4661, 0, 0, 0 },
	{ TX_VEN4N003, 4779, 0, 0, 0 },
	{ TX_VEN4N004, 1640, 0, 0, 0 },
	{ TX_VEN4N005, 1498, 0, 0, 0 },
	{ TX_VEN4N006, 5280, 0, 0, 0 },
	{ TX_VEN4N007, 5074, 0, 0, 0 },
	{ TX_VEN4N008, 5417, 0, 0, 0 },
	{ TX_VEN4N009, 2090, 0, 0, 0 },
	{ TX_VEN4N010, 3531, 0, 0, 0 },
	//{ TX_VEN4N010, 2019, 0, 0, 0 },	// ignore duplicate line
	{ TX_VEN4N011, 2185, 0, 0, 0 },
	{ TX_VEN4N012, 1926, 0, 0, 0 },
	{ TX_VEN4N013, 5352, 0, 0, 0 },
	{ TX_VEN4N014, 5192, 0, 0, 0 },
	{ TX_VEN4N015, 2408, 0, 0, 0 },
	{ TX_VEN4N016, 2628, 0, 0, 0 },
	{ TX_VEN4N017, 1768, 0, 0, 0 },
	{          -1,    0, 0, 0, 0 }
};

extern const RoomText veng4Texts[] = {
	  { -1, Common::UNK_LANG, "" }
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
	showText(TX_SPEAKER_KIRK, TX_VEN4_002);
}

void Room::veng4TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_VEN4_015);
}

void Room::veng4TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_VEN4_017);
}

void Room::veng4TalkToRedshirt() {
	showText(TX_SPEAKER_KIJE, TX_VEN4_022);
}

void Room::veng4UseMccoyOnDeadGuy() {
	showText(TX_SPEAKER_MCCOY, TX_VEN4_007);
}

void Room::veng4UseHypoOnBrittany() {
	showText(TX_SPEAKER_MCCOY, TX_VEN4_008);
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
		showText(TX_SPEAKER_MCCOY, TX_VEN4_013);
	else if (_awayMission->veng.usedMedkitOnBrittany)
		showText(TX_SPEAKER_MCCOY, TX_VEN4_009);
	else {
		if (!_awayMission->veng.lookedAtBrittany) {
			_awayMission->veng.lookedAtBrittany = true;
			showText(TX_SPEAKER_KIRK, TX_VEN4_005);
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
	showText(TX_SPEAKER_MCCOY, TX_VEN4_009);
}

void Room::veng4MccoyScannedBrittany() {
	showText(TX_SPEAKER_MCCOY, TX_VEN4_011);
	showText(TX_SPEAKER_KIRK,  TX_VEN4_001);

	_roomVar.veng.usingMedkitOnBrittany = true;
	veng4UseMccoyOnBrittany();
}

void Room::veng4TalkToBrittany() {
	if (_awayMission->veng.talkedToBrittany)
		showText(TX_SPEAKER_MCCOY, TX_VEN4_012);
	else if (!_awayMission->veng.usedMedkitOnBrittany)
		showText(TX_SPEAKER_MCCOY, TX_VEN4_010);
	else {
		_awayMission->veng.talkedToBrittany = true;
		showText(TX_SPEAKER_BRITTANY_MARATA, TX_VEN4_019);
		showText(TX_SPEAKER_KIRK,            TX_VEN4_004);
		showText(TX_SPEAKER_BRITTANY_MARATA, TX_VEN4_020);
		showText(TX_SPEAKER_KIRK,            TX_VEN4_006);
		showText(TX_SPEAKER_BRITTANY_MARATA, TX_VEN4_021);
		showText(TX_SPEAKER_KIRK,            TX_VEN4_003);

		loadActorAnim2(OBJECT_BRITTANY, "s9r1bm", BRITTANY_X, BRITTANY_Y);
		showText(TX_SPEAKER_MCCOY, TX_VEN4_016);
		loadActorAnim2(OBJECT_LEFT_READINGS, "s7r4m1", LEFT_READINGS_X, LEFT_READINGS_Y);
		loadActorAnim2(OBJECT_BRITTANY, "s7r4bd", BRITTANY_X, BRITTANY_Y);
		_awayMission->veng.brittanyDead = true;
		stopAllVocSounds();
		playVoc("VE42LOOP");
	}
}

void Room::veng4LookAtDeadGuy() {
	showDescription(TX_VEN4N014);
}

void Room::veng4LookAtRightBedReadings() {
	showDescription(TX_VEN4N013);
}

void Room::veng4LookAtDoorHotspot() {
	showDescription(TX_VEN4N008);
}

void Room::veng4LookAtKirk() {
	showDescription(TX_VEN4N002);
}

void Room::veng4LookAtSpock() {
	showText(TX_SPEAKER_SPOCK, TX_VEN4N003);
}

void Room::veng4LookAtMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_VEN4N000);
}

void Room::veng4LookAtRedshirt() {
	showText(TX_SPEAKER_KIJE, TX_VEN4N001);
}

void Room::veng4LookAtLeftBed() {
	if (_awayMission->veng.brittanyDead) {
		// ENHANCEMENT: Original game showed a string with no corresponding audio ("This
		// bed contains the body of a recently deceaced crewman." Yes that's a typo.)
		// Instead, show some appropriate voiced text.
		showDescription(TX_VEN4N017);
	} else
		showDescription(TX_VEN4N007);
}

void Room::veng4LookAtLeftBedReadings() {
	if (!_awayMission->veng.brittanyDead)
		showDescription(TX_VEN4N006);
	else
		showDescription(TX_VEN4N005);
}

void Room::veng4LookAtBrittany() {
	if (_awayMission->veng.brittanyDead)
		showDescription(TX_VEN4N017);
	else if (!_awayMission->veng.lookedAtBrittany) {
		_awayMission->veng.lookedAtBrittany = true;
		showText(TX_SPEAKER_KIRK,  TX_VEN4_005);
		showText(TX_SPEAKER_MCCOY, TX_VEN4_014);
	} else if (_awayMission->veng.talkedToBrittany)
		showDescription(TX_VEN4N017);
	else
		showDescription(TX_VEN4N004);
}

void Room::veng4LookAtDrill() {
	showDescription(TX_VEN4N012);
}

void Room::veng4LookAtHypoOnTable() {
	showDescription(TX_VEN4N010);
}

void Room::veng4LookAtDoorObject() {
	showDescription(TX_VEN4N009);
}

void Room::veng4LookAnywhere() {
	showDescription(TX_VEN4N011);
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
	showDescription(TX_VEN4N015);
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
	showDescription(TX_VEN4N016);
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
