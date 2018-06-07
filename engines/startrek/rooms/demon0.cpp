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

namespace StarTrek {

void Room::demon0Tick1() {
	playVoc("DEM0LOOP");
	loadActorAnim(9, "s0r0d3", 252, 153, 0);
	loadActorAnim(10, "s0r0d4", 158, 130, 0);
}

void Room::demon0Tick2() {
	if (_vm->_awayMission.demon.talkedToPrelate)
		return;
	loadActorAnim(8, "prel", 120, 190, 0);
}

void Room::demon0Tick60() {
	if (_vm->_awayMission.demon.talkedToPrelate)
		return;
	loadActorAnim2(8, "prelclap", 120, 190, 0);
}

void Room::demon0Tick100() {
	if (_vm->_awayMission.demon.talkedToPrelate || _vm->_awayMission.demon.prelateWelcomedCrew)
		return;
	_vm->_awayMission.demon.prelateWelcomedCrew = true;
	showText(TX_SPEAKER_ANGIVEN, TX_DEM0_036);
}

void Room::demon0Tick140() {
	if (_vm->_awayMission.demon.mccoyMentionedFlora)
		return;

	showText(TX_SPEAKER_MCCOY, TX_DEM0_016);

	_vm->_awayMission.demon.mccoyMentionedFlora = true;
}

void Room::demon0TouchedWarp0() {
	_vm->_awayMission.demon.enteredFrom = 1;
	_vm->_awayMission.rdfStillDoDefaultAction = true;
}

void Room::demon0WalkToBottomDoor() {
	_vm->_awayMission.disableInput = 1;
	_rdfData[0xcd] = 1; // FIXME
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 243, 158, 1);
}

void Room::demon0TouchedHotspot1() {
	if (_rdfData[0xcd] != 1) // FIXME: not portable to mac/amiga?
		return;
	loadActorAnim2(9, "s0r0d1", 254, 153, 1);
	playSoundEffectIndex(0x05);
}

void Room::demon0ReachedBottomDoor() {
	if (++_rdfData[0xca] != 2) // FIXME
		return;
	loadRoomIndex(5, 0);
}

void Room::demon0WalkToTopDoor() {
	_vm->_awayMission.disableInput = 1;
	_rdfData[0xcc] = 1; // FIXME
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 157, 134, 2);
}

void Room::demon0TouchedHotspot0() {
	if (_rdfData[0xcc] != 1) // FIXME: not portable to mac/amiga?
		return;
	loadActorAnim2(10, "s0r0d2", 158, 130, 2);
	playSoundEffectIndex(0x05);
}

void Room::demon0ReachedTopDoor() {
	if (++_rdfData[0xcb] != 2) // FIXME
		return;
	loadRoomIndex(6, 3);
}

void Room::demon0TalkToPrelate() {
	const int options1[] = {
		TX_SPEAKER_KIRK,
		TX_DEM0_006,
		TX_DEM0_008,
		TX_DEM0_003,
		TX_BLANK
	};
	const int firstResponse0[] = {
		TX_SPEAKER_ANGIVEN,
		TX_DEM0_038,
		TX_BLANK
	};
	const int firstResponse1[] = {
		TX_SPEAKER_ANGIVEN,
		TX_DEM0_032,
		TX_BLANK
	};
	const int options2[] = {
		TX_SPEAKER_KIRK,
		TX_DEM0_002,
		TX_DEM0_005,
		TX_BLANK
	};
	const int secondResponse[] = {
		TX_SPEAKER_ANGIVEN,
		TX_DEM0_031,
		TX_BLANK
	};
	const int options3[] = {
		TX_SPEAKER_KIRK,
		TX_DEM0_010,
		TX_DEM0_012,
		TX_BLANK
	};
	const int thirdResponse[] = {
		TX_SPEAKER_ANGIVEN,
		TX_DEM0_035,
		TX_BLANK
	};
	const int badConclusion[] = {
		TX_SPEAKER_ANGIVEN,
		TX_DEM0_037,
		TX_BLANK
	};
	const int goodConclusion[] = {
		TX_SPEAKER_ANGIVEN,
		TX_DEM0_034,
		TX_BLANK
	};

	if (_vm->_awayMission.demon.talkedToPrelate)
		return;

	_vm->_awayMission.demon.missionScore += 3;
	_vm->_awayMission.demon.talkedToPrelate = true;

	const int *response = nullptr;

	switch (showText(options1)) {
	case 0:
		response = firstResponse0;
		break;
	case 1:
		response = firstResponse1;
		break;
	case 2:
		demon0BadResponse();
		break;
	}

	if (response != nullptr)
		showText(response);

	if (showText(options2) == 1)
		demon0BadResponse();

	showText(secondResponse);

	if (showText(options3) == 1)
		demon0BadResponse();

	showText(thirdResponse);

	if (_vm->_awayMission.demon.wasRudeToPrelate)
		showText(badConclusion);
	else
		showText(goodConclusion);
}

void Room::demon0LookAtPrelate() {
	showText(TX_DEM0N004);
}

void Room::demon0UsePhaserOnSnow() {
	// BUG: doesn't check if redshirt is dead.
	showText(TX_SPEAKER_EVERTS, TX_DEM0_039);
}

void Room::demon0UsePhaserOnSign() {
	showText(TX_SPEAKER_MCCOY, TX_DEM0_017);
}

void Room::demon0UsePhaserOnShelter() {
	showText(TX_SPEAKER_MCCOY, TX_DEM0_015);
	showText(TX_SPEAKER_SPOCK, TX_DEM0_013);
	if (!_vm->_awayMission.redshirtDead)
		showText(TX_SPEAKER_EVERTS, TX_DEM0_040);
}

void Room::demon0UsePhaserOnPrelate() {
	showText(TX_SPEAKER_MCCOY, TX_DEM0_020);
}

void Room::demon0LookAtSign() {
	showText(TX_SPEAKER_SIGN, TX_DEM0C001);
}

void Room::demon0LookAtTrees() {
	showText(TX_DEM0N006);
}

void Room::demon0LookAtSnow() {
	showText(TX_DEM0N007);
}

void Room::demon0LookAnywhere() {
	showText(TX_DEM0N000);
}

void Room::demon0LookAtBushes() {
	showText(TX_DEM0N010);
}

void Room::demon0LookAtKirk() {
	showText(TX_DEM0N005);
}

void Room::demon0LookAtMcCoy() {
	showText(TX_DEM0N002);
}

void Room::demon0LookAtRedShirt() {
	showText(TX_DEM0N003);
}

void Room::demon0LookAtSpock() {
	showText(TX_DEM0N009);
}

void Room::demon0LookAtShelter() {
	showText(TX_DEM0N001);
}

void Room::demon0TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  TX_DEM0_009);
	showText(TX_SPEAKER_MCCOY, TX_DEM0_027);
	showText(TX_SPEAKER_KIRK,  TX_DEM0_004);
	showText(TX_SPEAKER_MCCOY, TX_DEM0_024);
}

void Room::demon0TalkToRedshirt() {
	showText(TX_SPEAKER_EVERTS, TX_DEM0_043);
	showText(TX_SPEAKER_MCCOY,  TX_DEM0_026);
	showText(TX_SPEAKER_EVERTS, TX_DEM0_042);
	showText(TX_SPEAKER_MCCOY,  TX_DEM0_025);
	showText(TX_SPEAKER_KIRK,   TX_DEM0_007);
	showText(TX_SPEAKER_EVERTS, TX_DEM0_044);
}

void Room::demon0TalkToMcCoy() {
	if (_vm->_awayMission.demon.talkedToPrelate) {
		showText(TX_SPEAKER_KIRK,  TX_DEM0_011);
		showText(TX_SPEAKER_MCCOY, TX_DEM0_023);
		showText(TX_SPEAKER_SPOCK, TX_DEM0_029);
		if (!_vm->_awayMission.redshirtDead)
			showText(TX_SPEAKER_EVERTS, TX_DEM0_041);
	}
	else {
		showText(TX_SPEAKER_MCCOY, TX_DEM0_019);
		if (!_vm->_awayMission.demon.askedPrelateAboutSightings) {
			demon0AskPrelateAboutSightings();
		}
	}
}

void Room::demon0TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_DEM0_014);

	if (!_vm->_awayMission.demon.talkedToPrelate && !_vm->_awayMission.demon.askedPrelateAboutSightings)
		demon0AskPrelateAboutSightings();
}

void Room::demon0AskPrelateAboutSightings() {
	showText(TX_SPEAKER_KIRK, TX_DEM0_001);
	showText(TX_SPEAKER_ANGIVEN, TX_DEM0_030);

	_vm->_awayMission.demon.askedPrelateAboutSightings = true;
}

void Room::demon0UseSTricorderAnywhere() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM0_028);
}

void Room::demon0UseMTricorderAnywhere() {
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, TX_DEM0_021);
}

void Room::demon0UseMTricorderOnPrelate() {
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_vm->_awayMission.demon.talkedToPrelate)
		showText(TX_SPEAKER_MCCOY, TX_DEM0_018);
	else
		showText(TX_SPEAKER_MCCOY, TX_DEM0_022);
}

// Helper functions
void Room::demon0BadResponse() {
	if (_vm->_awayMission.demon.wasRudeToPrelate)
		return;

	_vm->_awayMission.demon.missionScore -= 3;
	_vm->_awayMission.demon.wasRudeToPrelate = true;

	showText(TX_SPEAKER_ANGIVEN, TX_DEM0_033);
}
}

