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

#define OBJECT_CABLE 8
#define OBJECT_DEAD_GUY 9
#define OBJECT_DOOR 10

#define HOTSPOT_DOOR 0x20
#define HOTSPOT_COLLAPSED_SECTION 0x21

namespace StarTrek {

extern const RoomAction veng7ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::veng7Tick1 },

	{ {ACTION_WALK, OBJECT_DOOR,  0, 0, 0}, &Room::veng7WalkToDoor },
	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0, 0}, &Room::veng7WalkToDoor },
	{ {ACTION_DONE_WALK, 2,       0, 0, 0}, &Room::veng7ReachedDoor },
	{ {ACTION_DONE_ANIM, 1,       0, 0, 0}, &Room::veng7DoorOpened },
	{ {ACTION_TOUCHED_WARP, 1,    0, 0, 0}, &Room::veng7TouchedTurboliftDoor },

	{ {ACTION_LOOK, HOTSPOT_COLLAPSED_SECTION, 0, 0, 0}, &Room::veng7LookAtCollapsedSection },
	{ {ACTION_LOOK, OBJECT_DEAD_GUY,           0, 0, 0}, &Room::veng7LookAtDeadGuy },
	{ {ACTION_LOOK, OBJECT_DOOR,               0, 0, 0}, &Room::veng7LookAtDoor },
	{ {ACTION_LOOK, HOTSPOT_DOOR,              0, 0, 0}, &Room::veng7LookAtDoor },
	{ {ACTION_LOOK, OBJECT_CABLE,              0, 0, 0}, &Room::veng7LookAtCable },
	{ {ACTION_LOOK, OBJECT_KIRK,               0, 0, 0}, &Room::veng7LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,              0, 0, 0}, &Room::veng7LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,              0, 0, 0}, &Room::veng7LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,           0, 0, 0}, &Room::veng7LookAtRedshirt },
	{ {ACTION_LOOK, 0xff,                      0, 0, 0}, &Room::veng7LookAnywhere },
	{ {ACTION_TALK, OBJECT_KIRK,               0, 0, 0}, &Room::veng7TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,              0, 0, 0}, &Room::veng7TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,              0, 0, 0}, &Room::veng7TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT,           0, 0, 0}, &Room::veng7TalkToRedshirt },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_COLLAPSED_SECTION, 0, 0}, &Room::veng7UseSTricorderOnCollapsedSection },

	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0}, &Room::veng7TouchedHotspot0 },
	{ {ACTION_GET, OBJECT_CABLE,  0, 0, 0}, &Room::veng7GetCable },
	{ {ACTION_DONE_WALK, 3,       0, 0, 0}, &Room::veng7ReachedCable },
	{ {ACTION_DONE_ANIM, 4,       0, 0, 0}, &Room::veng7PickedUpCable },

	// Common code
	{ {ACTION_TICK, 0xff, 0xff, 0xff, 0}, &Room::vengaTick },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff,     0, 0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff,     0, 0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_LOOK, OBJECT_IHYPO,          0, 0, 0}, &Room::vengaLookAtHypo },
	{ {ACTION_USE, OBJECT_ICOMM, OBJECT_KIRK, 0, 0}, &Room::vengaUseCommunicator },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_DEAD_GUY, 0, 0}, &Room::vengaUseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_DEAD_GUY, 0, 0}, &Room::vengaUseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_DEAD_GUY, 0, 0}, &Room::vengaUseMccoyOnDeadGuy },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum veng7TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_KIJE,
	TX_BRIDU146, TX_DEM3_019, TX_MUD4_018, TX_VEN0N016, TX_VEN0_016,
	TX_VEN1_004, TX_VEN2U093, TX_VEN2_028, TX_VEN2_066,
	TX_VEN2_098, TX_VEN4N010, TX_VEN4_016, TX_VEN6N007, TX_VEN7N001,
	TX_VEN7N002, TX_VEN7N003, TX_VEN7N004, TX_VEN7N005, TX_VEN7N006,
	TX_VEN7N007, TX_VEN7N008, TX_VEN7N009, TX_VEN7_001, TX_VEN7_003,
	TX_VEN7_005, TX_VEN7_006, TX_VEN7_007, TX_VENA_F32, TX_VENA_F34,
	TX_VENA_F40
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets veng7TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 4142, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 4153, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 4163, 0, 0, 0 },
	{ TX_SPEAKER_KIJE, 4173, 0, 0, 0 },
	{ TX_BRIDU146, 3179, 0, 0, 0 },
	{ TX_DEM3_019, 3663, 0, 0, 0 },
	{ TX_MUD4_018, 3110, 0, 0, 0 },
	{ TX_MUD4_018, 3243, 0, 0, 0 },
	{ TX_VEN0N016, 708, 0, 0, 0 },
	{ TX_VEN0_016, 3959, 0, 0, 0 },
	//{ TX_VEN0_016, 3900, 0, 0, 0 },	// Skip duplicate text
	{ TX_VEN1_004, 1460, 0, 0, 0 },
	{ TX_VEN2U093, 3311, 0, 0, 0 },
	{ TX_VEN2_028, 3418, 0, 0, 0 },
	{ TX_VEN2_066, 3496, 0, 0, 0 },
	{ TX_VEN2_098, 2306, 0, 0, 0 },
	{ TX_VEN4N010, 2960, 0, 0, 0 },
	{ TX_VEN4_016, 3714, 0, 0, 0 },
	{ TX_VEN6N007, 3025, 0, 0, 0 },
	{ TX_VEN7N001, 525, 0, 0, 0 },
	{ TX_VEN7N002, 1071, 0, 0, 0 },
	{ TX_VEN7N003, 928, 0, 0, 0 },
	{ TX_VEN7N004, 1023, 0, 0, 0 },
	{ TX_VEN7N005, 976, 0, 0, 0 },
	{ TX_VEN7N006, 1117, 0, 0, 0 },
	{ TX_VEN7N007, 767, 0, 0, 0 },
	{ TX_VEN7N008, 841, 0, 0, 0 },
	{ TX_VEN7N009, 2161, 0, 0, 0 },
	{ TX_VEN7_001, 1195, 0, 0, 0 },
	{ TX_VEN7_003, 1902, 0, 0, 0 },
	{ TX_VEN7_005, 1320, 0, 0, 0 },
	{ TX_VEN7_006, 1706, 0, 0, 0 },
	{ TX_VEN7_007, 1576, 0, 0, 0 },
	{ TX_VENA_F32, 2536, 0, 0, 0 },
	{ TX_VENA_F34, 2806, 0, 0, 0 },
	{ TX_VENA_F40, 2401, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText veng7Texts[] = {
	  { -1, Common::UNK_LANG, "" }
};

void Room::veng7Tick1() {
	playVoc("VEN7LOOP");

	loadActorAnim(OBJECT_DOOR, "s7r7d1c", 0x59, 0x72);

	if (!_awayMission->veng.tookCableFromTransporterRoomHallway)
		loadActorAnim(OBJECT_CABLE, "s7r7ca", 0xac, 0x8a);

	loadActorAnim(OBJECT_DEAD_GUY, "s7r7c1", 0xa6, 0x49);
}

void Room::veng7WalkToDoor() {
	walkCrewmanC(OBJECT_KIRK, 0x66, 0x76, &Room::veng7ReachedDoor);
}

void Room::veng7ReachedDoor() {
	_awayMission->disableInput = true;
	playSoundEffectIndex(kSfxDoor);
	loadActorAnimC(OBJECT_DOOR, "s7r7d1", 0x59, 0x72, &Room::veng7DoorOpened);
}

void Room::veng7DoorOpened() {
	walkCrewman(OBJECT_KIRK, 0x5b, 0x76);
}

void Room::veng7TouchedTurboliftDoor() {
	playSoundEffectIndex(kSfxDoor);
	showRepublicMap(7, 1);
}

void Room::veng7LookAtCollapsedSection() {
	showDescription(TX_VEN7N001);
}

void Room::veng7LookAtDeadGuy() {
	// ENHANCEMENT: Original played TX_VEN0N016. This is reused and boring, and there is
	// a more interesting unused audio file, so use that instead.
	//showDescription(TX_VEN7N000);
	showDescription(TX_VEN0N016);
}

void Room::veng7LookAtDoor() {
	showDescription(TX_VEN7N007);
}

void Room::veng7LookAtCable() {
	showDescription(TX_VEN7N008);
}

void Room::veng7LookAtKirk() {
	showDescription(TX_VEN7N003);
}

void Room::veng7LookAtSpock() {
	showDescription(TX_VEN7N005);
}

void Room::veng7LookAtMccoy() {
	showDescription(TX_VEN7N004);
}

void Room::veng7LookAtRedshirt() {
	showDescription(TX_VEN7N002);
}

void Room::veng7LookAnywhere() {
	showDescription(TX_VEN7N006);
}

void Room::veng7TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_VEN7_001);
}

void Room::veng7TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_VEN7_005);
}

void Room::veng7TalkToMccoy() {
	// NOTE: TX_VEN7_007 could also fit here. It might be more fitting since it's not
	// reused. However, it mentions that McCoy gave up medical practice in Georgia. Maybe
	// they removed this for continuity reasons or something. I don't want to be
	// responsible for creating any possible confusion over his backstory.
	showText(TX_SPEAKER_MCCOY, TX_VEN1_004);
}

void Room::veng7TalkToRedshirt() {
	showText(TX_SPEAKER_KIJE, TX_VEN7_007);
}

void Room::veng7UseSTricorderOnCollapsedSection() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_VEN7_006);
}

void Room::veng7TouchedHotspot0() { // Got too close to the collapsed section
	showText(TX_SPEAKER_SPOCK, TX_VEN7_003);
}

void Room::veng7GetCable() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xac, 0x8b, &Room::veng7ReachedCable);
}

void Room::veng7ReachedCable() {
	loadActorAnimC(OBJECT_KIRK, "kusehn", -1, -1, &Room::veng7PickedUpCable);
	playVoc("MUR4E6");
}

void Room::veng7PickedUpCable() {
	loadActorStandAnim(OBJECT_CABLE);
	showDescription(TX_VEN7N009);
	giveItem(OBJECT_ICABLE1);
	_awayMission->veng.tookCableFromTransporterRoomHallway = true;
	_awayMission->disableInput = false;
}

}
