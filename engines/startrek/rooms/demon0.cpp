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

extern const RoomAction demon0ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::demon0Tick1 },
	{ {ACTION_TICK, 2, 0, 0, 0}, &Room::demon0Tick2 },
	{ {ACTION_TICK, 60, 0, 0, 0}, &Room::demon0Tick60 },
	{ {ACTION_TICK, 100, 0, 0, 0}, &Room::demon0Tick100 },
	{ {ACTION_TICK, 140, 0, 0, 0}, &Room::demon0Tick140 },

	{ {ACTION_TOUCHED_WARP, 0, 0, 0, 0}, &Room::demon0TouchedWarp0 },

	{ {ACTION_WALK, 0x27, 0, 0, 0}, &Room::demon0WalkToBottomDoor },
	{ {ACTION_WALK, 9, 0, 0, 0}, &Room::demon0WalkToBottomDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 1, 0, 0, 0}, &Room::demon0TouchedHotspot1 },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0}, &Room::demon0ReachedBottomDoor },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0}, &Room::demon0ReachedBottomDoor },

	{ {ACTION_WALK, 0x26, 0, 0, 0}, &Room::demon0WalkToTopDoor },
	{ {ACTION_WALK, 10, 0, 0, 0}, &Room::demon0WalkToTopDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0}, &Room::demon0TouchedHotspot0 },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0}, &Room::demon0ReachedTopDoor },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0}, &Room::demon0ReachedTopDoor },

	{ {ACTION_TALK, 8, 0, 0, 0}, &Room::demon0TalkToPrelate },
	{ {ACTION_LOOK, 8, 0, 0, 0}, &Room::demon0LookAtPrelate },

	{ {ACTION_USE, OBJECT_IPHASERS, 0x23, 0, 0}, &Room::demon0UsePhaserOnSnow },
	{ {ACTION_USE, OBJECT_IPHASERK, 0x23, 0, 0}, &Room::demon0UsePhaserOnSnow },
	{ {ACTION_USE, OBJECT_IPHASERS, 0x25, 0, 0}, &Room::demon0UsePhaserOnSign },
	{ {ACTION_USE, OBJECT_IPHASERK, 0x25, 0, 0}, &Room::demon0UsePhaserOnSign },
	{ {ACTION_USE, OBJECT_IPHASERK, 0x20, 0, 0}, &Room::demon0UsePhaserOnShelter },
	{ {ACTION_USE, OBJECT_IPHASERK, 8, 0, 0}, &Room::demon0UsePhaserOnPrelate },

	{ {ACTION_LOOK, 0x25, 0, 0, 0}, &Room::demon0LookAtSign },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::demon0LookAtTrees },
	{ {ACTION_LOOK, 0x22, 0, 0, 0}, &Room::demon0LookAtTrees },
	{ {ACTION_LOOK, 0x23, 0, 0, 0}, &Room::demon0LookAtSnow },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::demon0LookAnywhere },
	{ {ACTION_LOOK, 0x24, 0, 0, 0}, &Room::demon0LookAtBushes },

	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0, 0}, &Room::demon0LookAtKirk },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0, 0}, &Room::demon0LookAtMcCoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::demon0LookAtRedShirt },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0, 0}, &Room::demon0LookAtSpock },

	{ {ACTION_LOOK, 9, 0, 0, 0}, &Room::demon0LookAtShelter },    // Door 1
	{ {ACTION_LOOK, 10, 0, 0, 0}, &Room::demon0LookAtShelter },   // Door 2
	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::demon0LookAtShelter }, // Shelter itself

	{ {ACTION_TALK, OBJECT_KIRK,     0, 0, 0}, &Room::demon0TalkToKirk },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::demon0TalkToRedshirt },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0, 0}, &Room::demon0TalkToMcCoy },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0, 0}, &Room::demon0TalkToSpock },

	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::demon0UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0, 0}, &Room::demon0UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR, 8, 0, 0}, &Room::demon0UseMTricorderOnPrelate },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum demon0TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_ANGIVEN, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_EVERTS,
	TX_SPEAKER_SIGN,
	TX_DEM0_001,	TX_DEM0_002,	TX_DEM0_003,	TX_DEM0_004,	TX_DEM0_005,
	TX_DEM0_006,	TX_DEM0_007,	TX_DEM0_008,	TX_DEM0_009,	TX_DEM0_010,
	TX_DEM0_011,	TX_DEM0_012,	TX_DEM0_013,	TX_DEM0_014,	TX_DEM0_015,
	TX_DEM0_016,	TX_DEM0_017,	TX_DEM0_018,	TX_DEM0_019,	TX_DEM0_020,
	TX_DEM0_021,	TX_DEM0_022,	TX_DEM0_023,	TX_DEM0_024,	TX_DEM0_025,
	TX_DEM0_026,	TX_DEM0_027,	TX_DEM0_028,	TX_DEM0_029,	TX_DEM0_030,
	TX_DEM0_031,	TX_DEM0_032,	TX_DEM0_033,	TX_DEM0_034,	TX_DEM0_035,
	TX_DEM0_036,	TX_DEM0_037,	TX_DEM0_038,	TX_DEM0_039,	TX_DEM0_040,
	TX_DEM0_041,	TX_DEM0_042,	TX_DEM0_043,	TX_DEM0_044,	TX_DEM0N000,
	TX_DEM0N001,	TX_DEM0N002,	TX_DEM0N003,	TX_DEM0N004,	TX_DEM0N005,
	TX_DEM0N006,	TX_DEM0N007,	TX_DEM0N008,	TX_DEM0N009,	TX_DEM0N010,
	TX_DEM0C001
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets demon0TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 2597, 2480, 2676, 2697 },
	{ TX_SPEAKER_ANGIVEN, 2608, 2491, 2687, 2690 },
	{ TX_SPEAKER_MCCOY, 2622, 2505, 2701, 2704 },
	{ TX_SPEAKER_SPOCK, 2632, 2515, 2711, 02714 },
	{ TX_SPEAKER_EVERTS, 2642, 2525, 2721, 2724 },
	{ TX_SPEAKER_SIGN, 2661, 0, 2740, 2743 },
	{ TX_DEM0_001, 7829, 7085, 8665, 8597 },
	{ TX_DEM0_002, 3960, 3739, 4251, 4192 },
	{ TX_DEM0_003, 3148, 2987, 3294, 3303 },
	{ TX_DEM0_004, 7588, 6889, 0, 0 },
	{ TX_DEM0_005, 4061, 3825, 0, 0 },
	{ TX_DEM0_006, 2797, 2665, 2884, 2901 },
	{ TX_DEM0_007, 7380, 6741, 0, 0 },
	{ TX_DEM0_008, 2988, 2842, 3110, 3097 },
	{ TX_DEM0_009, 7460, 6791, 0, 0 },
	{ TX_DEM0_010, 4303, 4052, 0, 0 },
	{ TX_DEM0_011, 6827, 6304, 0, 0 },
	{ TX_DEM0_012, 4362, 4096, 0, 0 },
	{ TX_DEM0_013, 1334, 0, 1387, 1384 },
	{ TX_DEM0_014, 6593, 6085, 0, 0 },
	{ TX_DEM0_015, 1233, 0, 1281, 1270 },
	{ TX_DEM0_016,  473, 0, 474, 474 },
	{ TX_DEM0_017, 1167, 0, 1211, 1193 },
	{ TX_DEM0_018, 8094, 7320, 0, 0 },
	{ TX_DEM0_019, 6427, 5934, 0, 0 },
	{ TX_DEM0_020, 8648, 7829, 0, 0 },
	{ TX_DEM0_021, 8180, 7391, 0, 0 },
	{ TX_DEM0_022, 8050, 7291, 0, 0 },
	{ TX_DEM0_023, 6871, 0, 0, 0 },
	{ TX_DEM0_024, 7656, 0, 0, 0 },
	{ TX_DEM0_025, 7357, 0, 0, 0 },
	{ TX_DEM0_026, 7213, 0, 0, 0 },
	{ TX_DEM0_027, 7526, 0, 0, 0 },
	{ TX_DEM0_028, 8405, 7601, 0, 0 },
	{ TX_DEM0_029, 6995, 0, 0, 0 },
	{ TX_DEM0_030, 7709, 6980, 0, 0 },
	{ TX_DEM0_031, 4125, 0, 0, 0 },
	{ TX_DEM0_032, 3563, 0, 0, 0 },
	{ TX_DEM0_033, 3743, 0, 0, 0 },
	{ TX_DEM0_034, 5325, 0, 0, 0 },
	{ TX_DEM0_035, 4533, 0, 0, 0 },
	{ TX_DEM0_036, 2666, 2549, 2745, 2750 },
	{ TX_DEM0_037, 5196, 4898, 0, 0 },
	{ TX_DEM0_038, 3237, 3061, 0, 0 },
	{ TX_DEM0_039, 1058, 0, 1078, 1064 },
	{ TX_DEM0_040, 1400, 0, 1454, 1456 },
	{ TX_DEM0_041, 7080, 0, 0, 0 },
	{ TX_DEM0_042, 7275, 0, 0, 0 },
	{ TX_DEM0_043, 7144, 0, 0, 0 },
	{ TX_DEM0_044, 7429, 0, 0, 0 },
	{ TX_DEM0N000, 5591, 0, 0, 0 },
	{ TX_DEM0N001, 5805, 0, 0, 0 },
	{ TX_DEM0N002, 6184, 0, 0, 0 },
	{ TX_DEM0N003, 6290, 0, 0, 0 },
	{ TX_DEM0N004, 5467, 0, 0, 0 },
	{ TX_DEM0N005, 6076, 0, 0, 0 },
	{ TX_DEM0N006, 5678, 0, 0, 0 },
	{ TX_DEM0N007, 5897, 0, 0, 0 },
	{ TX_DEM0N008, 5560, 0, 0, 0 },
	{ TX_DEM0N009, 6036, 0, 0, 0 },
	{ TX_DEM0N010, 5960, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText demon0Texts[] = {
	{ TX_DEM0C001, Common::EN_ANY, "All mining equipment use this road." },
	{ -1, Common::UNK_LANG, "" }
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
	showText(TX_SPEAKER_ANGIVEN, TX_DEM0_036);
}

void Room::demon0Tick140() {
	if (_awayMission->demon.mccoyMentionedFlora)
		return;

	showText(TX_SPEAKER_MCCOY, TX_DEM0_016);

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
	playSoundEffectIndex(kSfxDoor);
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
	playSoundEffectIndex(kSfxDoor);
}

void Room::demon0ReachedTopDoor() {
	if (++_roomVar.demon.topDoorCounter != 2)
		return;
	loadRoomIndex(6, 3);
}

void Room::demon0TalkToPrelate() {
	const TextRef options1[] = {
	    TX_SPEAKER_KIRK,
	    TX_DEM0_006,
	    TX_DEM0_008,
	    TX_DEM0_003,
	    TX_END
	};
	const TextRef firstResponse0[] = {
	    TX_SPEAKER_ANGIVEN,
	    TX_DEM0_038,
	    TX_END
	};
	const TextRef firstResponse1[] = {
	    TX_SPEAKER_ANGIVEN,
	    TX_DEM0_032,
	    TX_END
	};
	const TextRef options2[] = {
	    TX_SPEAKER_KIRK,
	    TX_DEM0_002,
	    TX_DEM0_005,
	    TX_END
	};
	const TextRef secondResponse[] = {
	    TX_SPEAKER_ANGIVEN,
	    TX_DEM0_031,
	    TX_END
	};
	const TextRef options3[] = {
	    TX_SPEAKER_KIRK,
	    TX_DEM0_010,
	    TX_DEM0_012,
	    TX_END
	};
	const TextRef thirdResponse[] = {
	    TX_SPEAKER_ANGIVEN,
	    TX_DEM0_035,
	    TX_END
	};
	const TextRef badConclusion[] = {
	    TX_SPEAKER_ANGIVEN,
	    TX_DEM0_037,
	    TX_END
	};
	const TextRef goodConclusion[] = {
	    TX_SPEAKER_ANGIVEN,
	    TX_DEM0_034,
	    TX_END
	};

	if (_awayMission->demon.talkedToPrelate)
		return;

	_awayMission->demon.missionScore += 3;
	_awayMission->demon.talkedToPrelate = true;

	const TextRef *response = nullptr;

	switch (showMultipleTexts(options1)) {
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
		showMultipleTexts(response);

	if (showMultipleTexts(options2) == 1)
		demon0BadResponse();

	showMultipleTexts(secondResponse);

	if (showMultipleTexts(options3) == 1)
		demon0BadResponse();

	showMultipleTexts(thirdResponse);

	if (_awayMission->demon.wasRudeToPrelate)
		showMultipleTexts(badConclusion);
	else
		showMultipleTexts(goodConclusion);
}

void Room::demon0LookAtPrelate() {
	showDescription(TX_DEM0N004);
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
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_EVERTS, TX_DEM0_040);
}

void Room::demon0UsePhaserOnPrelate() {
	showText(TX_SPEAKER_MCCOY, TX_DEM0_020);
}

void Room::demon0LookAtSign() {
	showText(TX_SPEAKER_SIGN, TX_DEM0C001);
}

void Room::demon0LookAtTrees() {
	showDescription(TX_DEM0N006);
}

void Room::demon0LookAtSnow() {
	showDescription(TX_DEM0N007);
}

void Room::demon0LookAnywhere() {
	showDescription(TX_DEM0N000);
}

void Room::demon0LookAtBushes() {
	showDescription(TX_DEM0N010);
}

void Room::demon0LookAtKirk() {
	showDescription(TX_DEM0N005);
}

void Room::demon0LookAtMcCoy() {
	showDescription(TX_DEM0N002);
}

void Room::demon0LookAtRedShirt() {
	showDescription(TX_DEM0N003);
}

void Room::demon0LookAtSpock() {
	showDescription(TX_DEM0N009);
}

void Room::demon0LookAtShelter() {
	showDescription(TX_DEM0N001);
}

void Room::demon0TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_DEM0_009);
	showText(TX_SPEAKER_MCCOY, TX_DEM0_027);
	showText(TX_SPEAKER_KIRK, TX_DEM0_004);
	showText(TX_SPEAKER_MCCOY, TX_DEM0_024);
}

void Room::demon0TalkToRedshirt() {
	showText(TX_SPEAKER_EVERTS, TX_DEM0_043);
	showText(TX_SPEAKER_MCCOY, TX_DEM0_026);
	showText(TX_SPEAKER_EVERTS, TX_DEM0_042);
	showText(TX_SPEAKER_MCCOY, TX_DEM0_025);
	showText(TX_SPEAKER_KIRK, TX_DEM0_007);
	showText(TX_SPEAKER_EVERTS, TX_DEM0_044);
}

void Room::demon0TalkToMcCoy() {
	if (_awayMission->demon.talkedToPrelate) {
		showText(TX_SPEAKER_KIRK, TX_DEM0_011);
		showText(TX_SPEAKER_MCCOY, TX_DEM0_023);
		showText(TX_SPEAKER_SPOCK, TX_DEM0_029);
		if (!_awayMission->redshirtDead)
			showText(TX_SPEAKER_EVERTS, TX_DEM0_041);
	} else {
		showText(TX_SPEAKER_MCCOY, TX_DEM0_019);
		if (!_awayMission->demon.askedPrelateAboutSightings) {
			demon0AskPrelateAboutSightings();
		}
	}
}

void Room::demon0TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_DEM0_014);

	if (!_awayMission->demon.talkedToPrelate && !_awayMission->demon.askedPrelateAboutSightings)
		demon0AskPrelateAboutSightings();
}

void Room::demon0AskPrelateAboutSightings() {
	showText(TX_SPEAKER_KIRK, TX_DEM0_001);
	showText(TX_SPEAKER_ANGIVEN, TX_DEM0_030);

	_awayMission->demon.askedPrelateAboutSightings = true;
}

void Room::demon0UseSTricorderAnywhere() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM0_028);
}

void Room::demon0UseMTricorderAnywhere() {
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, TX_DEM0_021);
}

void Room::demon0UseMTricorderOnPrelate() {
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);

	if (_awayMission->demon.talkedToPrelate)
		showText(TX_SPEAKER_MCCOY, TX_DEM0_018);
	else
		showText(TX_SPEAKER_MCCOY, TX_DEM0_022);
}

// Helper functions
void Room::demon0BadResponse() {
	if (_awayMission->demon.wasRudeToPrelate)
		return;

	_awayMission->demon.missionScore -= 3;
	_awayMission->demon.wasRudeToPrelate = true;

	showText(TX_SPEAKER_ANGIVEN, TX_DEM0_033);
}
}
