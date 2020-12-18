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

extern const RoomAction demon0ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::demon0Tick1 },
	{ {ACTION_TICK, 2, 0, 0}, &Room::demon0Tick2 },
	{ {ACTION_TICK, 60, 0, 0}, &Room::demon0Tick60 },
	{ {ACTION_TICK, 100, 0, 0}, &Room::demon0Tick100 },
	{ {ACTION_TICK, 140, 0, 0}, &Room::demon0Tick140 },

	{ {ACTION_TOUCHED_WARP, 0, 0, 0}, &Room::demon0TouchedWarp0 },

	{ {ACTION_WALK, 0x27, 0, 0}, &Room::demon0WalkToBottomDoor },
	{ {ACTION_WALK, 9, 0, 0}, &Room::demon0WalkToBottomDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 1, 0, 0}, &Room::demon0TouchedHotspot1 },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0}, &Room::demon0ReachedBottomDoor },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0}, &Room::demon0ReachedBottomDoor },

	{ {ACTION_WALK, 0x26, 0, 0}, &Room::demon0WalkToTopDoor },
	{ {ACTION_WALK, 10, 0, 0}, &Room::demon0WalkToTopDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0}, &Room::demon0TouchedHotspot0 },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0}, &Room::demon0ReachedTopDoor },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0}, &Room::demon0ReachedTopDoor },

	{ {ACTION_TALK, 8, 0, 0}, &Room::demon0TalkToPrelate },
	{ {ACTION_LOOK, 8, 0, 0}, &Room::demon0LookAtPrelate },

	{ {ACTION_USE, OBJECT_IPHASERS, 0x23, 0}, &Room::demon0UsePhaserOnSnow },
	{ {ACTION_USE, OBJECT_IPHASERK, 0x23, 0}, &Room::demon0UsePhaserOnSnow },
	{ {ACTION_USE, OBJECT_IPHASERS, 0x25, 0}, &Room::demon0UsePhaserOnSign },
	{ {ACTION_USE, OBJECT_IPHASERK, 0x25, 0}, &Room::demon0UsePhaserOnSign },
	{ {ACTION_USE, OBJECT_IPHASERK, 0x20, 0}, &Room::demon0UsePhaserOnShelter },
	{ {ACTION_USE, OBJECT_IPHASERK, 8, 0}, &Room::demon0UsePhaserOnPrelate },

	{ {ACTION_LOOK, 0x25, 0, 0}, &Room::demon0LookAtSign },
	{ {ACTION_LOOK, 0x21, 0, 0}, &Room::demon0LookAtTrees },
	{ {ACTION_LOOK, 0x22, 0, 0}, &Room::demon0LookAtTrees },
	{ {ACTION_LOOK, 0x23, 0, 0}, &Room::demon0LookAtSnow },
	{ {ACTION_LOOK, 0xff, 0, 0}, &Room::demon0LookAnywhere },
	{ {ACTION_LOOK, 0x24, 0, 0}, &Room::demon0LookAtBushes },

	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0}, &Room::demon0LookAtKirk },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0}, &Room::demon0LookAtMcCoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::demon0LookAtRedShirt },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0}, &Room::demon0LookAtSpock },

	{ {ACTION_LOOK, 9, 0, 0}, &Room::demon0LookAtShelter },    // Door 1
	{ {ACTION_LOOK, 10, 0, 0}, &Room::demon0LookAtShelter },   // Door 2
	{ {ACTION_LOOK, 0x20, 0, 0}, &Room::demon0LookAtShelter }, // Shelter itself

	{ {ACTION_TALK, OBJECT_KIRK,     0, 0}, &Room::demon0TalkToKirk },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::demon0TalkToRedshirt },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::demon0TalkToMcCoy },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::demon0TalkToSpock },

	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0}, &Room::demon0UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0}, &Room::demon0UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR, 8, 0}, &Room::demon0UseMTricorderOnPrelate },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::demon0Tick1() {
	playVoc("DEM0LOOP");
	loadActorAnim(9, "s0r0d3", 252, 153, 0);
	loadActorAnim(10, "s0r0d4", 158, 130, 0);
}

void Room::demon0Tick2() {
	if (_awayMission->demon.talkedToPrelate)
		return;
	loadActorAnim(8, "prel", 120, 190, 0);
}

void Room::demon0Tick60() {
	if (_awayMission->demon.talkedToPrelate)
		return;
	loadActorAnim2(8, "prelclap", 120, 190, 0);
}

void Room::demon0Tick100() {
	if (_awayMission->demon.talkedToPrelate || _awayMission->demon.prelateWelcomedCrew)
		return;
	_awayMission->demon.prelateWelcomedCrew = true;
	showText(TX_SPEAKER_ANGIVEN, 36, true);
}

void Room::demon0Tick140() {
	if (_awayMission->demon.mccoyMentionedFlora)
		return;

	showText(TX_SPEAKER_MCCOY, 16, true);

	_awayMission->demon.mccoyMentionedFlora = true;
}

void Room::demon0TouchedWarp0() {
	_awayMission->demon.enteredFrom = 1;
	_awayMission->rdfStillDoDefaultAction = true;
}

void Room::demon0WalkToBottomDoor() {
	_awayMission->disableInput = 1;
	_roomVar.demon.movingToBottomDoor = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 243, 158, 1);
}

void Room::demon0TouchedHotspot1() {
	if (!_roomVar.demon.movingToBottomDoor)
		return;
	loadActorAnim2(9, "s0r0d1", 254, 153, 1);
	playSoundEffectIndex(0x05);
}

void Room::demon0ReachedBottomDoor() {
	if (++_roomVar.demon.bottomDoorCounter != 2)
		return;
	loadRoomIndex(5, 0);
}

void Room::demon0WalkToTopDoor() {
	_awayMission->disableInput = 1;
	_roomVar.demon.movingToTopDoor = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 157, 134, 2);
}

void Room::demon0TouchedHotspot0() {
	if (!_roomVar.demon.movingToTopDoor)
		return;
	loadActorAnim2(10, "s0r0d2", 158, 130, 2);
	playSoundEffectIndex(0x05);
}

void Room::demon0ReachedTopDoor() {
	if (++_roomVar.demon.topDoorCounter != 2)
		return;
	loadRoomIndex(6, 3);
}

void Room::demon0TalkToPrelate() {
	const TextRef options1[] = {
		TX_SPEAKER_KIRK,
		6,
		8,
		3,
		TX_BLANK
	};
	const TextRef firstResponse0[] = {
		TX_SPEAKER_ANGIVEN,
		38,
		TX_BLANK
	};
	const TextRef firstResponse1[] = {
		TX_SPEAKER_ANGIVEN,
		32,
		TX_BLANK
	};
	const TextRef options2[] = {
		TX_SPEAKER_KIRK,
		2,
		5,
		TX_BLANK
	};
	const TextRef secondResponse[] = {
		TX_SPEAKER_ANGIVEN,
		31,
		TX_BLANK
	};
	const TextRef options3[] = {
		TX_SPEAKER_KIRK,
		10,
		12,
		TX_BLANK
	};
	const TextRef thirdResponse[] = {
		TX_SPEAKER_ANGIVEN,
		35,
		TX_BLANK
	};
	const TextRef badConclusion[] = {
		TX_SPEAKER_ANGIVEN,
		37,
		TX_BLANK
	};
	const TextRef goodConclusion[] = {
		TX_SPEAKER_ANGIVEN,
		34,
		TX_BLANK
	};

	if (_awayMission->demon.talkedToPrelate)
		return;

	_awayMission->demon.missionScore += 3;
	_awayMission->demon.talkedToPrelate = true;

	const TextRef *response = nullptr;

	switch (showMultipleTexts(options1, true)) {
	case 0:
		response = firstResponse0;
		break;
	case 1:
		response = firstResponse1;
		break;
	case 2:
		demon0BadResponse();
		break;
	default:
		break;
	}

	if (response != nullptr)
		showMultipleTexts(response, true);

	if (showMultipleTexts(options2, true) == 1)
		demon0BadResponse();

	showMultipleTexts(secondResponse, true);

	if (showMultipleTexts(options3, true) == 1)
		demon0BadResponse();

	showMultipleTexts(thirdResponse, true);

	if (_awayMission->demon.wasRudeToPrelate)
		showMultipleTexts(badConclusion, true);
	else
		showMultipleTexts(goodConclusion, true);
}

void Room::demon0LookAtPrelate() {
	showDescription(4, true);
}

void Room::demon0UsePhaserOnSnow() {
	// BUG: doesn't check if redshirt is dead.
	showText(TX_SPEAKER_EVERTS, 39, true);
}

void Room::demon0UsePhaserOnSign() {
	showText(TX_SPEAKER_MCCOY, 17, true);
}

void Room::demon0UsePhaserOnShelter() {
	showText(TX_SPEAKER_MCCOY, 15, true);
	showText(TX_SPEAKER_SPOCK, 13, true);
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_EVERTS, 40, true);
}

void Room::demon0UsePhaserOnPrelate() {
	showText(TX_SPEAKER_MCCOY, 20, true);
}

void Room::demon0LookAtSign() {
	showText(TX_SPEAKER_SIGN, TX_DEM0C001);
}

void Room::demon0LookAtTrees() {
	showDescription(6, true);
}

void Room::demon0LookAtSnow() {
	showDescription(7, true);
}

void Room::demon0LookAnywhere() {
	showDescription(0, true);
}

void Room::demon0LookAtBushes() {
	showDescription(10, true);
}

void Room::demon0LookAtKirk() {
	showDescription(5, true);
}

void Room::demon0LookAtMcCoy() {
	showDescription(2, true);
}

void Room::demon0LookAtRedShirt() {
	showDescription(3, true);
}

void Room::demon0LookAtSpock() {
	showDescription(9, true);
}

void Room::demon0LookAtShelter() {
	showDescription(1, true);
}

void Room::demon0TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  9, true);
	showText(TX_SPEAKER_MCCOY, 27, true);
	showText(TX_SPEAKER_KIRK,  4, true);
	showText(TX_SPEAKER_MCCOY, 24, true);
}

void Room::demon0TalkToRedshirt() {
	showText(TX_SPEAKER_EVERTS, 43, true);
	showText(TX_SPEAKER_MCCOY,  26, true);
	showText(TX_SPEAKER_EVERTS, 42, true);
	showText(TX_SPEAKER_MCCOY,  25, true);
	showText(TX_SPEAKER_KIRK,   7, true);
	showText(TX_SPEAKER_EVERTS, 44, true);
}

void Room::demon0TalkToMcCoy() {
	if (_awayMission->demon.talkedToPrelate) {
		showText(TX_SPEAKER_KIRK,  11, true);
		showText(TX_SPEAKER_MCCOY, 23, true);
		showText(TX_SPEAKER_SPOCK, 29, true);
		if (!_awayMission->redshirtDead)
			showText(TX_SPEAKER_EVERTS, 41, true);
	} else {
		showText(TX_SPEAKER_MCCOY, 19, true);
		if (!_awayMission->demon.askedPrelateAboutSightings) {
			demon0AskPrelateAboutSightings();
		}
	}
}

void Room::demon0TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 14, true);

	if (!_awayMission->demon.talkedToPrelate && !_awayMission->demon.askedPrelateAboutSightings)
		demon0AskPrelateAboutSightings();
}

void Room::demon0AskPrelateAboutSightings() {
	showText(TX_SPEAKER_KIRK, 1, true);
	showText(TX_SPEAKER_ANGIVEN, 30, true);

	_awayMission->demon.askedPrelateAboutSightings = true;
}

void Room::demon0UseSTricorderAnywhere() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, 28, true);
}

void Room::demon0UseMTricorderAnywhere() {
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, 21, true);
}

void Room::demon0UseMTricorderOnPrelate() {
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_awayMission->demon.talkedToPrelate)
		showText(TX_SPEAKER_MCCOY, 18, true);
	else
		showText(TX_SPEAKER_MCCOY, 22, true);
}

// Helper functions
void Room::demon0BadResponse() {
	if (_awayMission->demon.wasRudeToPrelate)
		return;

	_awayMission->demon.missionScore -= 3;
	_awayMission->demon.wasRudeToPrelate = true;

	showText(TX_SPEAKER_ANGIVEN, 33, true);
}
}

