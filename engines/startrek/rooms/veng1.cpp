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

// OBJECT_DEBRIS, OBJECT_PANEL, and HOTSPOT_PANEL are all just about the same thing, in
// different contexts. It's OBJECT_PANEL before being opened, OBJECT_DEBRIS after being
// opened, and OBJECT_PANEL again after clearing the debris. Confusingly, HOTSPOT_PANEL is
// always selectable if you click the top or right edge of the panel, but one of the other
// objects always seems to be on top of it.

#define OBJECT_DOOR 8
#define OBJECT_DEBRIS 9
#define OBJECT_SAW 10
#define OBJECT_DEAD_GUY 11
#define OBJECT_PANEL 12 // Closed panel

#define HOTSPOT_DOOR 0x20
#define HOTSPOT_PANEL 0x21 // Opened panel
#define HOTSPOT_VENT 0x22

namespace StarTrek {

extern const RoomAction veng1ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::veng1Tick1 },

	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0, 0}, &Room::veng1WalkToDoor },
	{ {ACTION_WALK, OBJECT_DOOR,  0, 0, 0}, &Room::veng1WalkToDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0}, &Room::veng1TouchedHotspot0 },
	{ {ACTION_DONE_WALK, 2,       0, 0, 0}, &Room::veng1ReachedDoor },
	{ {ACTION_DONE_ANIM, 1,       0, 0, 0}, &Room::veng1ReachedDoor },
	{ {ACTION_TOUCHED_WARP, 1,    0, 0, 0}, &Room::veng1TouchedTurbolift },

	{ {ACTION_LOOK, OBJECT_DEAD_GUY, 0, 0, 0}, &Room::veng1LookAtDeadGuy },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_VENT, 0, 0}, &Room::veng1UseSTricorderOnVent },
	{ {ACTION_LOOK, HOTSPOT_VENT,              0, 0, 0}, &Room::veng1UseSTricorderOnVent },

	{ {ACTION_LOOK, OBJECT_PANEL,   0, 0, 0}, &Room::veng1LookAtPanel },
	{ {ACTION_LOOK, HOTSPOT_PANEL,  0, 0, 0}, &Room::veng1LookAtPanel },
	{ {ACTION_LOOK, OBJECT_DOOR,    0, 0, 0}, &Room::veng1LookAtDoor },
	{ {ACTION_LOOK, HOTSPOT_DOOR,   0, 0, 0}, &Room::veng1LookAtDoor },
	{ {ACTION_LOOK, OBJECT_SAW,     0, 0, 0}, &Room::veng1LookAtSaw },
	{ {ACTION_LOOK, OBJECT_DEBRIS,  0, 0, 0}, &Room::veng1LookAtDebris },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0, 0}, &Room::veng1LookAtKirk },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0, 0}, &Room::veng1LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0, 0}, &Room::veng1LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::veng1LookAtRedshirt },
	{ {ACTION_LOOK, 0xff,            0, 0, 0}, &Room::veng1LookAnywhere },
	{ {ACTION_TALK, OBJECT_KIRK,     0, 0, 0}, &Room::veng1TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0, 0}, &Room::veng1TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0, 0}, &Room::veng1TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::veng1TalkToRedshirt },

	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_DOOR,   0, 0}, &Room::veng1UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_DOOR,  0, 0}, &Room::veng1UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_DOOR,   0, 0}, &Room::veng1UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_DOOR,  0, 0}, &Room::veng1UsePhaserOnDoor },

	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_PANEL,  0, 0}, &Room::veng1UsePhaserOnPanel },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_PANEL,  0, 0}, &Room::veng1UsePhaserOnPanel }, // BUGFIX: IPHASERK, not IPHASERS again
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_DEBRIS, 0, 0}, &Room::veng1UsePhaserOnPanel },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_DEBRIS, 0, 0}, &Room::veng1UsePhaserOnPanel },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_PANEL, 0, 0}, &Room::veng1UsePhaserOnPanel },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_PANEL, 0, 0}, &Room::veng1UsePhaserOnPanel },

	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_SAW,    0, 0}, &Room::veng1UseSTricorderOnSaw },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_DOOR,  0, 0}, &Room::veng1UseSTricorderOnDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_DOOR,   0, 0}, &Room::veng1UseSTricorderOnDoor },

	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_PANEL,  0, 0}, &Room::veng1UseSTricorderOnPanel },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_DEBRIS, 0, 0}, &Room::veng1UseSTricorderOnPanel },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_PANEL, 0, 0}, &Room::veng1UseSTricorderOnPanel },

	{ {ACTION_GET, OBJECT_PANEL,           0, 0, 0}, &Room::veng1GetPanel },
	{ {ACTION_USE, OBJECT_KIRK, OBJECT_PANEL, 0, 0}, &Room::veng1GetPanel },
	{ {ACTION_DONE_WALK, 9,                0, 0, 0}, &Room::veng1KirkReachedPanel },
	{ {ACTION_DONE_ANIM, 10,               0, 0, 0}, &Room::veng1KirkAccessedPanel },

	{ {ACTION_USE, OBJECT_ILMD,   HOTSPOT_PANEL, 0, 0}, &Room::veng1UseSawOnPanel },
	{ {ACTION_USE, OBJECT_ILMD,   OBJECT_DOOR,   0, 0}, &Room::veng1UseSawOnDoor },
	{ {ACTION_USE, OBJECT_ILMD,   HOTSPOT_DOOR,  0, 0}, &Room::veng1UseSawOnDoor },
	{ {ACTION_USE, OBJECT_IDRILL, OBJECT_DEBRIS, 0, 0}, &Room::veng1UseDrillOnDebris },
	{ {ACTION_USE, OBJECT_ILMD,   OBJECT_DEBRIS, 0, 0}, &Room::veng1UseSawOnDebris },

	{ {ACTION_USE, OBJECT_IDRILL, OBJECT_PANEL,  0, 0}, &Room::veng1UseDrillOnPanel },
	{ {ACTION_DONE_WALK, 5,                   0, 0, 0}, &Room::veng1KirkReachedPanelToDrill },
	{ {ACTION_DONE_ANIM, 6,                   0, 0, 0}, &Room::veng1KirkDoneDrillingPanel },
	{ {ACTION_USE, OBJECT_IHYPO, OBJECT_DEBRIS,  0, 0}, &Room::veng1UseHypoOnDebris },
	{ {ACTION_DONE_WALK, 7,                   0, 0, 0}, &Room::veng1KirkReachedDebris },
	{ {ACTION_DONE_ANIM, 8,                   0, 0, 0}, &Room::veng1KirkUsedHypoOnDebris },

	{ {ACTION_GET, OBJECT_SAW, 0, 0, 0}, &Room::veng1GetSaw },
	{ {ACTION_DONE_WALK, 3,    0, 0, 0}, &Room::veng1ReachedSaw },
	{ {ACTION_DONE_ANIM, 4,    0, 0, 0}, &Room::veng1PickedUpSaw },
	{ {ACTION_GET, OBJECT_DEBRIS, 0, 0, 0}, &Room::veng1GetDebris },
	{ {ACTION_DONE_WALK, 11,      0, 0, 0}, &Room::veng1ReachedDebrisToGet },
	{ {ACTION_DONE_ANIM, 12,      0, 0, 0}, &Room::veng1TriedToGetDebris },

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

enum veng1TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_KIJE,
	TX_VEN0_016, TX_VEN1_001, TX_VEN1_002, TX_VEN1_004, TX_VEN1_005,
	TX_VEN1_006, TX_VEN1_007, TX_VEN1_008, TX_VEN1_009, TX_VEN1_010,
	TX_VEN1_011, TX_VEN1_012, TX_VEN1_013, TX_VEN1_014, TX_VEN1_015,
	TX_VEN1_016, TX_VEN1_F33, TX_VEN2_028, TX_VEN2_066, TX_VEN2_098,
	TX_VEN4_016, TX_VENA_F32, TX_VENA_F34, TX_VENA_F40, TX_DEM3_019,
	TX_MUD4_018, /*TX_BRIDU146,*/ TX_VEN1N000, TX_VEN1N001, TX_VEN1N002,
	TX_VEN1N003, TX_VEN1N004, TX_VEN1N005, TX_VEN1N006, TX_VEN1N007,
	TX_VEN1N008, TX_VEN1N009, TX_VEN1N010, TX_VEN1N011, TX_VEN1N012,
	TX_VEN1N013, TX_VEN1N014, TX_VEN1N015, TX_VEN1N016, TX_VEN1N017,
	TX_VEN1N018, TX_VEN1N019, TX_VEN1N020, TX_VEN1N021, TX_VEN1N022,
	TX_VEN1N023, TX_VEN2U093, TX_VEN4N010, TX_VEN4N014, TX_VEN6N007
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets veng1TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 7612, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 7623, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 7633, 0, 0, 0 },
	{ TX_SPEAKER_KIJE, 7643, 0, 0, 0 },
	{ TX_VEN0_016, 7429, 0, 0, 0 },
	//{ TX_VEN0_016, 7370, 0, 0, 0 },	// ignore duplicate line
	{ TX_VEN1_001, 2202, 0, 0, 0 },
	{ TX_VEN1_002, 2602, 0, 0, 0 },
	{ TX_VEN1_004, 2493, 0, 0, 0 },
	{ TX_VEN1_005, 2415, 0, 0, 0 },
	{ TX_VEN1_006, 3777, 0, 0, 0 },
	{ TX_VEN1_007, 3415, 0, 0, 0 },
	{ TX_VEN1_008, 3016, 0, 0, 0 },
	{ TX_VEN1_009, 3501, 0, 0, 0 },
	{ TX_VEN1_010, 3889, 0, 0, 0 },
	{ TX_VEN1_011, 938, 0, 0, 0 },
	{ TX_VEN1_012, 3195, 0, 0, 0 },
	{ TX_VEN1_013, 2293, 0, 0, 0 },
	{ TX_VEN1_014, 3640, 0, 0, 0 },
	{ TX_VEN1_015, 2777, 0, 0, 0 },
	{ TX_VEN1_016, 2677, 0, 0, 0 },
	{ TX_VEN1_F33, 4328, 0, 0, 0 },
	{ TX_VEN2_028, 6888, 0, 0, 0 },
	{ TX_VEN2_066, 6966, 0, 0, 0 },
	{ TX_VEN2_098, 5776, 0, 0, 0 },
	{ TX_VEN4_016, 7184, 0, 0, 0 },
	{ TX_VENA_F32, 6006, 0, 0, 0 },
	{ TX_VENA_F34, 6276, 0, 0, 0 },
	{ TX_VENA_F40, 5871, 0, 0, 0 },
	{ TX_DEM3_019, 7133, 0, 0, 0 },
	//{ TX_G_014, 7256, 0, 0, 0 },
	//{ TX_G_014, 7313, 0, 0, 0 },
	{ TX_MUD4_018, 6580, 0, 0, 0 },
	//{ TX_MUD4_018, 6713, 0, 0, 0 },		// ignore duplicate line
	//{ TX_BRIDU146, 6649, 0, 0, 0 },
	{ TX_VEN1N000, 1545, 0, 0, 0 },
	{ TX_VEN1N001, 1453, 0, 0, 0 },
	{ TX_VEN1N002, 1626, 0, 0, 0 },
	{ TX_VEN1N003, 1840, 0, 0, 0 },
	{ TX_VEN1N004, 1753, 0, 0, 0 },
	{ TX_VEN1N005, 2002, 0, 0, 0 },
	{ TX_VEN1N006, 5529, 0, 0, 0 },
	{ TX_VEN1N007, 549, 0, 0, 0 },
	{ TX_VEN1N008, 4906, 0, 0, 0 },
	{ TX_VEN1N009, 4587, 0, 0, 0 },
	{ TX_VEN1N010, 1284, 0, 0, 0 },
	{ TX_VEN1N011, 5638, 0, 0, 0 },
	{ TX_VEN1N012, 5108, 0, 0, 0 },
	{ TX_VEN1N013, 4408, 0, 0, 0 },
	{ TX_VEN1N014, 4500, 0, 0, 0 },
	{ TX_VEN1N015, 5158, 0, 0, 0 },
	{ TX_VEN1N016, 4074, 0, 0, 0 },
	{ TX_VEN1N017, 4202, 0, 0, 0 },
	{ TX_VEN1N018, 4680, 0, 0, 0 },
	{ TX_VEN1N019, 1200, 0, 0, 0 },
	{ TX_VEN1N020, 2108, 0, 0, 0 },
	{ TX_VEN1N021, 1382, 0, 0, 0 },
	{ TX_VEN1N022, 1100, 0, 0, 0 },
	{ TX_VEN1N023, 5326, 0, 0, 0 },
	{ TX_VEN2U093, 6781, 0, 0, 0 },
	{ TX_VEN4N010, 6430, 0, 0, 0 },
	{ TX_VEN4N014,  777, 0, 0, 0 },
	{ TX_VEN6N007, 6495, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText veng1Texts[] = {
	  { -1, Common::UNK_LANG, "" }
};

void Room::veng1Tick1() {
	playVoc("VEN1LOOP");
	loadActorAnim2(OBJECT_DOOR, "s7r1d1c", 0xa1, 0x7e);
	loadActorAnim2(OBJECT_DEAD_GUY, "s7r1c1", 0x103, 0xa2);

	if (!_awayMission->veng.tookMolecularSaw)
		loadActorAnim2(OBJECT_SAW, "s7r1md", 0x3d, 0x9f);

	loadActorAnim2(OBJECT_PANEL, "s7r1p1", 0xd1, 0x6a);

	if (_awayMission->veng.openedPanel) {
		loadActorAnim2(OBJECT_DEBRIS, "s7r1p2", 0xd1, 0x6a);
		loadActorStandAnim(OBJECT_PANEL);
	}

	if (_awayMission->veng.removedPanelDebris) {
		loadActorAnim2(OBJECT_PANEL, "s7r1p3", 0xd1, 0x6a);
		loadActorStandAnim(OBJECT_DEBRIS);
	}
}

void Room::veng1WalkToDoor() {
	if (_awayMission->veng.removedPanelDebris) {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_KIRK, 0xa0, 0x82, &Room::veng1ReachedDoor);
		_roomVar.veng.walkingToDoor = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
	} else
		showDescription(TX_VEN1N007);
}

void Room::veng1TouchedHotspot0() {
	if (_roomVar.veng.walkingToDoor) {
		loadActorAnimC(OBJECT_DOOR, "s7r1d1", 0xa1, 0x7e, &Room::veng1ReachedDoor);
		loadActorAnim2(OBJECT_PANEL, "s7r1g1", 0xd8, 0x79);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::veng1ReachedDoor() {
	if (++_roomVar.veng.doorOpenCounter == 2)
		loadRoomIndex(2, 1);
}

void Room::veng1TouchedTurbolift() {
	playSoundEffectIndex(kSfxDoor);
	showRepublicMap(1, 0);
}

void Room::veng1LookAtDeadGuy() {
	showDescription(TX_VEN4N014);
}

void Room::veng1UseSTricorderOnVent() {
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_VEN1_011);
}

void Room::veng1LookAtPanel() {
	if (!_awayMission->veng.openedPanel)
		showDescription(TX_VEN1N022);
	else if (!_awayMission->veng.removedPanelDebris)
		showDescription(TX_VEN1N019);
	else
		showDescription(TX_VEN1N010);
}

void Room::veng1LookAtDoor() {
	showDescription(TX_VEN1N021);
}

void Room::veng1LookAtSaw() {
	showDescription(TX_VEN1N001);
}

void Room::veng1LookAtDebris() {
	showDescription(TX_VEN1N000);
}

void Room::veng1LookAtKirk() {
	showDescription(TX_VEN1N002);
}

void Room::veng1LookAtMccoy() {
	showDescription(TX_VEN1N004);
}

void Room::veng1LookAtSpock() {
	showDescription(TX_VEN1N003);
}

void Room::veng1LookAtRedshirt() {
	showDescription(TX_VEN1N005);
}

void Room::veng1LookAnywhere() {
	showDescription(TX_VEN1N020);
}

void Room::veng1TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_VEN1_001);
}

void Room::veng1TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_VEN1_013);
	showText(TX_SPEAKER_MCCOY, TX_VEN1_005);
}

void Room::veng1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_VEN1_004);
	showText(TX_SPEAKER_KIRK,  TX_VEN1_002);
}

void Room::veng1TalkToRedshirt() {
	showText(TX_SPEAKER_KIJE, TX_VEN1_016);
}

void Room::veng1UsePhaserOnDoor() {
	showText(TX_SPEAKER_KIJE, TX_VEN1_015);
}

void Room::veng1UsePhaserOnPanel() {
	showText(TX_SPEAKER_SPOCK, TX_VEN1_008);
}

void Room::veng1UseSTricorderOnSaw() {
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_VEN1_012);
}

void Room::veng1UseSTricorderOnDoor() {
	if (_awayMission->veng.removedPanelDebris)
		spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_VEN1_009);
	else
		spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_VEN1_007);
}

void Room::veng1UseSTricorderOnPanel() {
	if (!_awayMission->veng.clearedPanelDebris)
		spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_VEN1_014);
	else if (!_awayMission->veng.removedPanelDebris)
		spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_VEN1_006);
	else
		spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_VEN1_010);
}

void Room::veng1GetPanel() {
	walkCrewmanC(OBJECT_KIRK, 0xd3, 0x84, &Room::veng1KirkReachedPanel);
	_awayMission->disableInput = true;
}

void Room::veng1KirkReachedPanel() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::veng1KirkAccessedPanel);
}

void Room::veng1KirkAccessedPanel() {
	_awayMission->disableInput = false;

	if (!_awayMission->veng.clearedPanelDebris) {
		showDescription(TX_VEN1N016);
	} else if (!_awayMission->veng.openedPanel) {
		_awayMission->veng.openedPanel = true;
		showDescription(TX_VEN1N017);
		loadActorAnim2(OBJECT_DEBRIS, "s7r1p2", 0xd1, 0x6a);
		loadActorStandAnim(OBJECT_PANEL);
	} else
		showDescription(TX_VEN1_F33);

	walkCrewman(OBJECT_KIRK, 0xc9, 0x98);
}

void Room::veng1UseSawOnPanel() {
	showDescription(TX_VEN1N013);
}

void Room::veng1UseSawOnDoor() {
	showDescription(TX_VEN1N014);
}

void Room::veng1UseDrillOnDebris() {
	showDescription(TX_VEN1N009);
}

void Room::veng1UseSawOnDebris() {
	showDescription(TX_VEN1N018);
}

void Room::veng1UseDrillOnPanel() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xd3, 0x84, &Room::veng1KirkReachedPanelToDrill);
}

void Room::veng1KirkReachedPanelToDrill() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::veng1KirkDoneDrillingPanel);
	playVoc("MADR7E2A");
}

void Room::veng1KirkDoneDrillingPanel() {
	walkCrewman(OBJECT_KIRK, 0xc9, 0x98);
	_awayMission->disableInput = false;
	if (!_awayMission->veng.clearedPanelDebris) {
		_awayMission->veng.clearedPanelDebris = true;
		showDescription(TX_VEN1N008);
	}
}

void Room::veng1UseHypoOnDebris() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xd3, 0x84, &Room::veng1KirkReachedDebris);
}

void Room::veng1KirkReachedDebris() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::veng1KirkUsedHypoOnDebris);
}

void Room::veng1KirkUsedHypoOnDebris() {
	walkCrewman(OBJECT_KIRK, 0xc9, 0x98);
	_awayMission->disableInput = false;

	if (!_awayMission->veng.oilInHypo)
		showDescription(TX_VEN1N012);
	else {
		_awayMission->veng.loosenedDebrisWithOil = true;
		showDescription(TX_VEN1N015);
	}
}

void Room::veng1GetSaw() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0x32, 0xa4, &Room::veng1ReachedSaw);
}

void Room::veng1ReachedSaw() {
	loadActorAnimC(OBJECT_KIRK, "kuseln", -1, -1, &Room::veng1PickedUpSaw);
}

void Room::veng1PickedUpSaw() {
	loadActorStandAnim(OBJECT_SAW);
	showDescription(TX_VEN1N023);
	giveItem(OBJECT_ILMD);

	_awayMission->veng.tookMolecularSaw = true;
	_awayMission->disableInput = false;
}

void Room::veng1GetDebris() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xd3, 0x84, &Room::veng1ReachedDebrisToGet);
}

void Room::veng1ReachedDebrisToGet() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::veng1TriedToGetDebris);
}

void Room::veng1TriedToGetDebris() {
	walkCrewman(OBJECT_KIRK, 0xc9, 0x98);
	_awayMission->disableInput = false;

	if (!_awayMission->veng.loosenedDebrisWithOil)
		showDescription(TX_VEN1N006);
	else {
		loadActorStandAnim(OBJECT_DEBRIS);
		loadActorAnim2(OBJECT_PANEL, "s7r1p3", 0xd1, 0x6a);
		showDescription(TX_VEN1N011);
		_awayMission->veng.removedPanelDebris = true;
	}
}

}
