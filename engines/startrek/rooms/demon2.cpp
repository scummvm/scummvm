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

namespace StarTrek {

extern const RoomAction demon2ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::demon2Tick1 },
	{ {ACTION_WALK, 0x27, 0, 0, 0}, &Room::demon2WalkToCave },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0}, &Room::demon2ReachedCave },
	{ {ACTION_TOUCHED_WARP, 1, 0, 0, 0}, &Room::demon2TouchedWarp1 },

	{ {ACTION_LOOK, 0x27, 0, 0, 0}, &Room::demon2LookAtCave },
	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::demon2LookAtCave },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::demon2LookAtMountain },
	{ {ACTION_LOOK, 0x22, 0, 0, 0}, &Room::demon2LookAtMountain },
	{ {ACTION_LOOK, 0x23, 0, 0, 0}, &Room::demon2LookAtBerries },
	{ {ACTION_LOOK, 0x24, 0, 0, 0}, &Room::demon2LookAtFern },
	{ {ACTION_LOOK, 0x25, 0, 0, 0}, &Room::demon2LookAtMoss },
	{ {ACTION_LOOK, 0x26, 0, 0, 0}, &Room::demon2LookAtLights },
	{ {ACTION_LOOK, 0xff, 0, 0, 0},   &Room::demon2LookAtAnything },

	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0}, &Room::demon2LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0}, &Room::demon2LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0}, &Room::demon2LookAtMcCoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::demon2LookAtRedshirt },

	{ {ACTION_TALK, OBJECT_KIRK, 0, 0, 0}, &Room::demon2TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0}, &Room::demon2TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0}, &Room::demon2TalkToMcCoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::demon2TalkToRedshirt },

	{ {ACTION_USE, OBJECT_IPHASERK, 0x23, 0, 0}, &Room::demon2UsePhaserOnBerries },
	{ {ACTION_USE, OBJECT_IPHASERS, 0x23, 0, 0}, &Room::demon2UsePhaserOnBerries },

	{ {ACTION_USE, OBJECT_ISTRICOR, 0x23, 0, 0}, &Room::demon2UseSTricorderOnBerries },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x25, 0, 0}, &Room::demon2UseSTricorderOnMoss },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x24, 0, 0}, &Room::demon2UseSTricorderOnFern },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x20, 0, 0}, &Room::demon2UseSTricorderOnCave },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0, 0}, &Room::demon2UseSTricorderOnCave },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x22, 0, 0}, &Room::demon2UseSTricorderOnCave },

	{ {ACTION_USE, OBJECT_IMTRICOR, 0x23, 0, 0}, &Room::demon2UseMTricorderOnBerries },
	{ {ACTION_GET, 0x23, 0, 0, 0}, &Room::demon2GetBerries },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0}, &Room::demon2ReachedBerries },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0}, &Room::demon2PickedUpBerries },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum demon2TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_EVERTS,
	TX_DEM2_001, TX_DEM2_002, TX_DEM2_003, TX_DEM2_004, TX_DEM2_005,
	TX_DEM2_006, TX_DEM2_007, TX_DEM2_008, TX_DEM2_009, TX_DEM2_010,
	TX_DEM2_011, TX_DEM2_012, TX_DEM2_013, TX_DEM2_014, TX_DEM2_015,
	TX_DEM2_016, TX_DEM2_017, TX_DEM2_018, TX_DEM2_019, TX_DEM2_020,
	TX_DEM2N000, TX_DEM2N002, TX_DEM2N003, TX_DEM2N004, TX_DEM2N005,
	TX_DEM2N006, TX_DEM2N007, TX_DEM2N008, TX_DEM2N009, TX_DEM2N010,
	TX_DEM2N011, TX_DEM2N012, TX_DEM2N013
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets demon2TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 2890, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 2901, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 2911, 0, 0, 0 },
	{ TX_SPEAKER_EVERTS, 2921, 0, 0, 0 },
	{ TX_DEM2_001, 3275, 0, 0, 0 },
	{ TX_DEM2_002, 3732, 0, 0, 0 },
	{ TX_DEM2_003, 4228, 0, 0, 0 },
	{ TX_DEM2_004, 3912, 0, 0, 0 },
	{ TX_DEM2_005, 1613, 0, 0, 0 },
	{ TX_DEM2_006, 1466, 0, 0, 0 },
	{ TX_DEM2_007, 1336, 0, 0, 0 },
	{ TX_DEM2_008, 1161, 0, 0, 0 },
	{ TX_DEM2_009, 1963, 0, 0, 0 },
	{ TX_DEM2_010, 2071, 0, 0, 0 },
	{ TX_DEM2_011, 1050, 0, 0, 0 },
	{ TX_DEM2_012, 2212, 0, 0, 0 },
	{ TX_DEM2_013, 1869, 0, 0, 0 },
	{ TX_DEM2_014, 2384, 0, 0, 0 },
	{ TX_DEM2_015, 3346, 0, 0, 0 },
	{ TX_DEM2_016, 3561, 0, 0, 0 },
	{ TX_DEM2_017, 3414, 0, 0, 0 },
	{ TX_DEM2_018, 4111, 0, 0, 0 },
	{ TX_DEM2_019, 3618, 0, 0, 0 },
	{ TX_DEM2_020, 3791, 0, 0, 0 },
	{ TX_DEM2N000, 3197, 0, 0, 0 },
	{ TX_DEM2N002, 2935, 0, 0, 0 },
	{ TX_DEM2N003, 3117, 0, 0, 0 },
	{ TX_DEM2N004, 3014, 0, 0, 0 },
	{ TX_DEM2N005, 586, 0, 0, 0 },
	{ TX_DEM2N006, 710, 0, 0, 0 },
	{ TX_DEM2N007, 405, 0, 0, 0 },
	{ TX_DEM2N008, 454, 0, 0, 0 },
	{ TX_DEM2N009, 357, 0, 0, 0 },
	{ TX_DEM2N010, 637, 0, 0, 0 },
	{ TX_DEM2N011, 785, 0, 0, 0 },
	{ TX_DEM2N012, 503, 0, 0, 0 },
	{ TX_DEM2N013, 2529, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText demon2Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::demon2Tick1() {
	playVoc("DEM2LOOP");
}

void Room::demon2WalkToCave() {
	walkCrewman(OBJECT_KIRK, 0x83, 0x5c, 2);
}

void Room::demon2ReachedCave() {
	loadRoomIndex(3, 1);
}

void Room::demon2TouchedWarp1() {
	_awayMission->demon.enteredFrom = 2;
	_awayMission->rdfStillDoDefaultAction = true;
}

void Room::demon2LookAtCave() {
	showDescription(TX_DEM2N009);
}

void Room::demon2LookAtMountain() {
	showDescription(TX_DEM2N007);
}

void Room::demon2LookAtBerries() {
	showDescription(TX_DEM2N012);
}

void Room::demon2LookAtFern() {
	showDescription(TX_DEM2N005);
}

void Room::demon2LookAtMoss() {
	showDescription(TX_DEM2N010);
}

void Room::demon2LookAtLights() {
	showDescription(TX_DEM2N006);
}

void Room::demon2LookAtAnything() {
	showDescription(TX_DEM2N011);
}

void Room::demon2LookAtKirk() {
	showDescription(TX_DEM2N002);
}

void Room::demon2LookAtSpock() {
	showDescription(TX_DEM2N004);
}

void Room::demon2LookAtMcCoy() {
	showDescription(TX_DEM2N003);
}

void Room::demon2LookAtRedshirt() {
	showDescription(TX_DEM2N000);
}

void Room::demon2TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_DEM2_001);
}

void Room::demon2TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_DEM2_018);
	showText(TX_SPEAKER_KIRK, TX_DEM2_003);
}

void Room::demon2TalkToMcCoy() {
	showText(TX_SPEAKER_MCCOY, TX_DEM2_015);
	showText(TX_SPEAKER_SPOCK, TX_DEM2_017);
	showText(TX_SPEAKER_MCCOY, TX_DEM2_016);
}

void Room::demon2TalkToRedshirt() {
	showText(TX_SPEAKER_EVERTS, TX_DEM2_019);
	showText(TX_SPEAKER_KIRK, TX_DEM2_002);
	showText(TX_SPEAKER_EVERTS, TX_DEM2_020);
	showText(TX_SPEAKER_KIRK, TX_DEM2_004);
}

void Room::demon2UsePhaserOnBerries() {
	showText(TX_SPEAKER_MCCOY, TX_DEM2_011);
}

void Room::demon2UseSTricorderOnBerries() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM2_008);
}

void Room::demon2UseSTricorderOnMoss() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM2_007);
}

void Room::demon2UseSTricorderOnFern() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM2_006);
}

void Room::demon2UseSTricorderOnCave() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);

	// NOTE: audio file for this is empty.
	showText(TX_SPEAKER_SPOCK, TX_DEM2_005);
}

void Room::demon2UseMTricorderOnBerries() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);

	if (_awayMission->demon.curedChub)
		showText(TX_SPEAKER_MCCOY, TX_DEM2_013);
	else if (_awayMission->demon.madeHypoDytoxin)
		showText(TX_SPEAKER_MCCOY, TX_DEM2_009);
	else if (_awayMission->demon.knowAboutHypoDytoxin)
		showText(TX_SPEAKER_MCCOY, TX_DEM2_010);
	else
		showText(TX_SPEAKER_MCCOY, TX_DEM2_012);
}

void Room::demon2GetBerries() {
	if (_awayMission->demon.gotBerries)
		showText(TX_SPEAKER_MCCOY, TX_DEM2_014);
	else
		walkCrewman(OBJECT_KIRK, 0xe8, 0x97, 1);
}

void Room::demon2ReachedBerries() {
	loadActorAnim2(OBJECT_KIRK, "kusehe", -1, -1, 1);
	playVoc("PIKBERRY");
}

void Room::demon2PickedUpBerries() {
	showDescription(TX_DEM2N013);
	giveItem(OBJECT_IBERRY);
	_awayMission->demon.gotBerries = true;
	_awayMission->demon.missionScore += 1;
}

}
