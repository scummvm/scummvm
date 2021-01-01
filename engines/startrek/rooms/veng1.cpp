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
	{ {ACTION_TICK, 1, 0, 0}, &Room::veng1Tick1 },

	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0}, &Room::veng1WalkToDoor },
	{ {ACTION_WALK, OBJECT_DOOR,  0, 0}, &Room::veng1WalkToDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0}, &Room::veng1TouchedHotspot0 },
	{ {ACTION_DONE_WALK, 2,       0, 0}, &Room::veng1ReachedDoor },
	{ {ACTION_DONE_ANIM, 1,       0, 0}, &Room::veng1ReachedDoor },
	{ {ACTION_TOUCHED_WARP, 1,    0, 0}, &Room::veng1TouchedTurbolift },

	{ {ACTION_LOOK, OBJECT_DEAD_GUY, 0, 0}, &Room::veng1LookAtDeadGuy },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_VENT, 0}, &Room::veng1UseSTricorderOnVent },
	{ {ACTION_LOOK, HOTSPOT_VENT,              0, 0}, &Room::veng1UseSTricorderOnVent },

	{ {ACTION_LOOK, OBJECT_PANEL,   0, 0}, &Room::veng1LookAtPanel },
	{ {ACTION_LOOK, HOTSPOT_PANEL,  0, 0}, &Room::veng1LookAtPanel },
	{ {ACTION_LOOK, OBJECT_DOOR,    0, 0}, &Room::veng1LookAtDoor },
	{ {ACTION_LOOK, HOTSPOT_DOOR,   0, 0}, &Room::veng1LookAtDoor },
	{ {ACTION_LOOK, OBJECT_SAW,     0, 0}, &Room::veng1LookAtSaw },
	{ {ACTION_LOOK, OBJECT_DEBRIS,  0, 0}, &Room::veng1LookAtDebris },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0}, &Room::veng1LookAtKirk },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0}, &Room::veng1LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0}, &Room::veng1LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::veng1LookAtRedshirt },
	{ {ACTION_LOOK, 0xff,            0, 0}, &Room::veng1LookAnywhere },
	{ {ACTION_TALK, OBJECT_KIRK,     0, 0}, &Room::veng1TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::veng1TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::veng1TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::veng1TalkToRedshirt },

	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_DOOR,   0}, &Room::veng1UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_DOOR,  0}, &Room::veng1UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_DOOR,   0}, &Room::veng1UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_DOOR,  0}, &Room::veng1UsePhaserOnDoor },

	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_PANEL,  0}, &Room::veng1UsePhaserOnPanel },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_PANEL,  0}, &Room::veng1UsePhaserOnPanel }, // BUGFIX: IPHASERK, not IPHASERS again
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_DEBRIS, 0}, &Room::veng1UsePhaserOnPanel },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_DEBRIS, 0}, &Room::veng1UsePhaserOnPanel },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_PANEL, 0}, &Room::veng1UsePhaserOnPanel },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_PANEL, 0}, &Room::veng1UsePhaserOnPanel },

	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_SAW,    0}, &Room::veng1UseSTricorderOnSaw },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_DOOR,  0}, &Room::veng1UseSTricorderOnDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_DOOR,   0}, &Room::veng1UseSTricorderOnDoor },

	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_PANEL,  0}, &Room::veng1UseSTricorderOnPanel },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_DEBRIS, 0}, &Room::veng1UseSTricorderOnPanel },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_PANEL, 0}, &Room::veng1UseSTricorderOnPanel },

	{ {ACTION_GET, OBJECT_PANEL,           0, 0}, &Room::veng1GetPanel },
	{ {ACTION_USE, OBJECT_KIRK, OBJECT_PANEL, 0}, &Room::veng1GetPanel },
	{ {ACTION_DONE_WALK, 9,                0, 0}, &Room::veng1KirkReachedPanel },
	{ {ACTION_DONE_ANIM, 10,               0, 0}, &Room::veng1KirkAccessedPanel },

	{ {ACTION_USE, OBJECT_ILMD,   HOTSPOT_PANEL, 0}, &Room::veng1UseSawOnPanel },
	{ {ACTION_USE, OBJECT_ILMD,   OBJECT_DOOR,   0}, &Room::veng1UseSawOnDoor },
	{ {ACTION_USE, OBJECT_ILMD,   HOTSPOT_DOOR,  0}, &Room::veng1UseSawOnDoor },
	{ {ACTION_USE, OBJECT_IDRILL, OBJECT_DEBRIS, 0}, &Room::veng1UseDrillOnDebris },
	{ {ACTION_USE, OBJECT_ILMD,   OBJECT_DEBRIS, 0}, &Room::veng1UseSawOnDebris },

	{ {ACTION_USE, OBJECT_IDRILL, OBJECT_PANEL,  0}, &Room::veng1UseDrillOnPanel },
	{ {ACTION_DONE_WALK, 5,                   0, 0}, &Room::veng1KirkReachedPanelToDrill },
	{ {ACTION_DONE_ANIM, 6,                   0, 0}, &Room::veng1KirkDoneDrillingPanel },
	{ {ACTION_USE, OBJECT_IHYPO, OBJECT_DEBRIS,  0}, &Room::veng1UseHypoOnDebris },
	{ {ACTION_DONE_WALK, 7,                   0, 0}, &Room::veng1KirkReachedDebris },
	{ {ACTION_DONE_ANIM, 8,                   0, 0}, &Room::veng1KirkUsedHypoOnDebris },

	{ {ACTION_GET, OBJECT_SAW, 0, 0}, &Room::veng1GetSaw },
	{ {ACTION_DONE_WALK, 3,    0, 0}, &Room::veng1ReachedSaw },
	{ {ACTION_DONE_ANIM, 4,    0, 0}, &Room::veng1PickedUpSaw },
	{ {ACTION_GET, OBJECT_DEBRIS, 0, 0}, &Room::veng1GetDebris },
	{ {ACTION_DONE_WALK, 11,      0, 0}, &Room::veng1ReachedDebrisToGet },
	{ {ACTION_DONE_ANIM, 12,      0, 0}, &Room::veng1TriedToGetDebris },

	// Common code
	{ {ACTION_TICK, 0xff, 0xff, 0xff}, &Room::vengaTick },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff,     0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff,     0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_LOOK, OBJECT_IHYPO,          0, 0}, &Room::vengaLookAtHypo },
	{ {ACTION_USE, OBJECT_ICOMM, OBJECT_KIRK, 0}, &Room::vengaUseCommunicator },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_DEAD_GUY, 0}, &Room::vengaUseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_DEAD_GUY, 0}, &Room::vengaUseMccoyOnDeadGuy },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_DEAD_GUY, 0}, &Room::vengaUseMccoyOnDeadGuy },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
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
		showDescription(7, true);
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
	spockScan(DIR_W, 11, false, true);
}

void Room::veng1LookAtPanel() {
	if (!_awayMission->veng.openedPanel)
		showDescription(22, true);
	else if (!_awayMission->veng.removedPanelDebris)
		showDescription(19, true);
	else
		showDescription(10, true);
}

void Room::veng1LookAtDoor() {
	showDescription(21, true);
}

void Room::veng1LookAtSaw() {
	showDescription(1, true);
}

void Room::veng1LookAtDebris() {
	showDescription(0, true);
}

void Room::veng1LookAtKirk() {
	showDescription(2, true);
}

void Room::veng1LookAtMccoy() {
	showDescription(4, true);
}

void Room::veng1LookAtSpock() {
	showDescription(3, true);
}

void Room::veng1LookAtRedshirt() {
	showDescription(5, true);
}

void Room::veng1LookAnywhere() {
	showDescription(20, true);
}

void Room::veng1TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 1, true);
}

void Room::veng1TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 13, true);
	showText(TX_SPEAKER_MCCOY,  5, true);
}

void Room::veng1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 4, true);
	showText(TX_SPEAKER_KIRK,  2, true);
}

void Room::veng1TalkToRedshirt() {
	showText(TX_SPEAKER_KIJE, 16, true);
}

void Room::veng1UsePhaserOnDoor() {
	showText(TX_SPEAKER_KIJE, 15, true);
}

void Room::veng1UsePhaserOnPanel() {
	showText(TX_SPEAKER_SPOCK, 8, true);
}

void Room::veng1UseSTricorderOnSaw() {
	spockScan(DIR_W, 12, false, true);
}

void Room::veng1UseSTricorderOnDoor() {
	if (_awayMission->veng.removedPanelDebris)
		spockScan(DIR_N, 9, false, true);
	else
		spockScan(DIR_N, 7, false, true);
}

void Room::veng1UseSTricorderOnPanel() {
	if (!_awayMission->veng.clearedPanelDebris)
		spockScan(DIR_E, 14, false, true);
	else if (!_awayMission->veng.removedPanelDebris)
		spockScan(DIR_E, 6, false, true);
	else
		spockScan(DIR_E, 10, false, true);
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
		showDescription(16, true);
	} else if (!_awayMission->veng.openedPanel) {
		_awayMission->veng.openedPanel = true;
		showDescription(17, true);
		loadActorAnim2(OBJECT_DEBRIS, "s7r1p2", 0xd1, 0x6a);
		loadActorStandAnim(OBJECT_PANEL);
	} else
		showDescription(33 + FOLLOWUP_MESSAGE_OFFSET, true);

	walkCrewman(OBJECT_KIRK, 0xc9, 0x98);
}

void Room::veng1UseSawOnPanel() {
	showDescription(13, true);
}

void Room::veng1UseSawOnDoor() {
	showDescription(14, true);
}

void Room::veng1UseDrillOnDebris() {
	showDescription(9, true);
}

void Room::veng1UseSawOnDebris() {
	showDescription(18, true);
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
		showDescription(8, true);
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
		showDescription(12, true);
	else {
		_awayMission->veng.loosenedDebrisWithOil = true;
		showDescription(15, true);
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
	showDescription(23, true);
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
		showDescription(6, true);
	else {
		loadActorStandAnim(OBJECT_DEBRIS);
		loadActorAnim2(OBJECT_PANEL, "s7r1p3", 0xd1, 0x6a);
		showDescription(11, true);
		_awayMission->veng.removedPanelDebris = true;
	}
}

}
