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

#define OBJECT_VIEWSCREEN 8
#define OBJECT_ALIENDV 9
#define OBJECT_REPAIR_TOOL 10
#define OBJECT_11 11

#define HOTSPOT_SCREEN 0x20
#define HOTSPOT_CONSOLE 0x21
#define HOTSPOT_LEFT_CONSOLE 0x22
#define HOTSPOT_RIGHT_CONSOLE 0x23
#define HOTSPOT_EAST_DOOR 0x24
#define HOTSPOT_WEST_DOOR 0x25

namespace StarTrek {

extern const RoomAction mudd4ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::mudd4Tick1 },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0, 0},    &Room::mudd4UseCommunicator },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0, 0},        &Room::mudd4Timer2Expired },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0, 0},        &Room::mudd4Timer3Expired },
	{ {ACTION_USE, OBJECT_SPOCK, 0x21, 0, 0},    &Room::mudd4UseSpockOnLeftConsole },
	{ {ACTION_USE, OBJECT_SPOCK, 0x22, 0, 0},    &Room::mudd4UseSpockOnLeftConsole },
	{ {ACTION_USE, OBJECT_SPOCK, 0x23, 0, 0},    &Room::mudd4UseSpockOnRightConsole },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0},     &Room::mudd4SpockReachedChair },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0},   &Room::mudd4SpockSatInChair },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0},   &Room::mudd4SpockUsedSensors },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0, 0},   &Room::mudd4SpockUsedEngineering },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0, 0},   &Room::mudd4SpockUsedNavigation },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0, 0},     &Room::mudd4FinishedWalking3 },
	{ {ACTION_USE, OBJECT_KIRK, 0x21, 0, 0},     &Room::mudd4UseKirkOnRightConsole },
	{ {ACTION_USE, OBJECT_KIRK, 0x23, 0, 0},     &Room::mudd4UseKirkOnRightConsole },
	{ {ACTION_USE, OBJECT_KIRK, 0x22, 0, 0},     &Room::mudd4UseKirkOnLeftConsole },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0},     &Room::mudd4KirkReachedRightConsole },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0, 0},   &Room::mudd4KirkSatInChair },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0, 0},   &Room::mudd4KirkUsedViewScreen },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0, 0},        &Room::mudd4Timer1Expired },
	{ {ACTION_FINISHED_ANIMATION, 7, 0, 0, 0},   &Room::mudd4KirkUsedCommunications },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0, 0},     &Room::mudd4KirkReachedPositionToTalkToMudd },
	{ {ACTION_USE, OBJECT_MCCOY, 0x21, 0, 0},    &Room::mudd4UseMccoyOnConsole },
	{ {ACTION_USE, OBJECT_MCCOY, 0x22, 0, 0},    &Room::mudd4UseMccoyOnConsole },
	{ {ACTION_USE, OBJECT_MCCOY, 0x23, 0, 0},    &Room::mudd4UseMccoyOnConsole },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x21, 0, 0}, &Room::mudd4UseRedshirtOnConsole },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x22, 0, 0}, &Room::mudd4UseRedshirtOnConsole },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x23, 0, 0}, &Room::mudd4UseRedshirtOnConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, 10, 0, 0},   &Room::mudd4UseSTricorderOnRepairTool },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0, 0}, &Room::mudd4UseSTricorderOnConsole },

	// ENHANCEMENT: Allow scanning the console to work when scanning the specific stations
	// as well
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x22, 0, 0}, &Room::mudd4UseSTricorderOnConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x23, 0, 0}, &Room::mudd4UseSTricorderOnConsole },

	{ {ACTION_USE, OBJECT_ISTRICOR, 0x20, 0, 0}, &Room::mudd4UseSTricorderOnViewscreen },

	// ENHANCEMENT: Allow scanning the viewscreen when it's on, not just when off
	{ {ACTION_USE, OBJECT_ISTRICOR, 8, 0, 0},    &Room::mudd4UseSTricorderOnViewscreen },

	// Common code (next 4 lines)
	{ {ACTION_USE, OBJECT_ILENSES, OBJECT_IDEGRIME, 0, 0}, &Room::muddaUseLenseOnDegrimer },
	{ {ACTION_USE, OBJECT_IALIENDV, 0xff, 0, 0}, &Room::muddaUseAlienDevice },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0, 0}, &Room::muddaFiredAlienDevice },
	{ {ACTION_USE, OBJECT_IDEGRIME, 0xff, 0, 0}, &Room::muddaUseDegrimer },

	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0, 0},   &Room::mudd4UseMedkit },
	{ {ACTION_GET, 10, 0, 0, 0},                &Room::mudd4GetRepairTool },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0, 0},    &Room::mudd4ReachedRepairTool },
	{ {ACTION_FINISHED_ANIMATION, 10, 0, 0, 0}, &Room::mudd4PickedUpRepairTool },

	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::mudd4LookAtConsole },
	// ENHANCEMENT: Allow look action work with the specific stations as well
	{ {ACTION_LOOK, 0x22, 0, 0, 0}, &Room::mudd4LookAtConsole },
	{ {ACTION_LOOK, 0x23, 0, 0, 0}, &Room::mudd4LookAtConsole },

	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::mudd4LookAtViewscreen },
	// ENHANCEMENT: Allow look action work when viewscreen is on, not just when off
	{ {ACTION_LOOK, 8, 0, 0, 0},    &Room::mudd4LookAtViewscreen },

	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0},     &Room::mudd4LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0},    &Room::mudd4LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0},    &Room::mudd4LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::mudd4LookAtRedshirt },
	{ {ACTION_LOOK, 10, 0, 0, 0},              &Room::mudd4LookAtRepairTool },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0, 0},     &Room::mudd4TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0},    &Room::mudd4TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0},    &Room::mudd4TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::mudd4TalkToRedshirt },
	{ {ACTION_WALK, 0x24, 0, 0, 0},            &Room::mudd4WalkToEastDoor },
	{ {ACTION_WALK, 0x25, 0, 0, 0},            &Room::mudd4WalkToWestDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0},    &Room::mudd4TouchedHotspot0 },

	// Common code (countdown for losing atmosphere when life support malfunctioning)
	{ {ACTION_TICK, 0xff, 0xff, 0xff, 0},           &Room::muddaTick },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum mudd4TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_UHURA, TX_SPEAKER_BUCHERT,
	TX_SPEAKER_SCOTT, TX_SPEAKER_MUDD,
	TX_MUD4_001, TX_MUD4_002, TX_MUD4_004, TX_MUD4_005, TX_MUD4_006,
	TX_MUD4_007, TX_MUD4_008, TX_MUD4_009, TX_MUD4_010, TX_MUD4_011,
	TX_MUD4_012, TX_MUD4_014, TX_MUD4_015, TX_MUD4_021, TX_MUD4_026,
	TX_MUD4_029, TX_MUD4_030, TX_MUD4_034, TX_MUD4_035, TX_MUD4_036,
	TX_MUD4_037, TX_MUD4_046, TX_MUD4_047, TX_MUD4_050, TX_MUD4_051,
	TX_MUD4_063, TX_MUD4_064, TX_MUD4_065, TX_MUD4_067, TX_MUD4_069,
	TX_MUD4_070, TX_MUD4_071, TX_MUD4_073, TX_MUD4_076, TX_MUD4_013,
	TX_MUD4_016, TX_MUD4_017, TX_MUD4_018, TX_MUD4_019, TX_MUD4_020,
	TX_MUD4_023, TX_MUD4_024, TX_MUD4_025, TX_MUD4_027, TX_MUD4_028,
	TX_MUD4_031, TX_MUD4_032, TX_MUD4_033, TX_MUD4_038, TX_MUD4_039,
	TX_MUD4_040, TX_MUD4_041, TX_MUD4_042, TX_MUD4_043, TX_MUD4_045,
	TX_MUD4_048, TX_MUD4_049, TX_MUD4_052, TX_MUD4_053, TX_MUD4_054,
	TX_MUD4_055, TX_MUD4_056, TX_MUD4_057, TX_MUD4_058, TX_MUD4_059,
	TX_MUD4_060, TX_MUD4_061, TX_MUD4_062, TX_MUD4_066, TX_MUD4_068,
	TX_MUD4_072, TX_MUD4_074, TX_MUD4_075, TX_MUD4_A29, TX_MUD4_S02,
	TX_MUD4_S03, TX_MUD4_S04, TX_MUD4N000, TX_MUD4N001, TX_MUD4N003,
	TX_MUD4N004, TX_MUD4N005, TX_MUD4N006, TX_MUD4N007, TX_MUD4N008,
	TX_MUD4N011, TX_MUD4N012, TX_MUD4N013, TX_MUD4_003, TX_STATICU1
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets mudd4TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 4048, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 4059, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 4069, 0, 0, 0 },
	{ TX_SPEAKER_UHURA, 491, 0, 0, 0 },
	{ TX_SPEAKER_BUCHERT, 4079, 0, 0, 0 },
	{ TX_SPEAKER_SCOTT, 4091, 0, 0, 0 },
	{ TX_SPEAKER_MUDD, 4101, 0, 0, 0 },
	{ TX_MUD4_001, 12511, 0, 0, 0 },
	{ TX_MUD4_002, 6069, 0, 0, 0 },
	{ TX_MUD4_004, 6134, 0, 0, 0 },
	{ TX_MUD4_005, 11984, 0, 0, 0 },
	{ TX_MUD4_006, 5958, 0, 0, 0 },
	{ TX_MUD4_007, 6103, 0, 0, 0 },
	{ TX_MUD4_008, 9263, 0, 0, 0 },
	{ TX_MUD4_009, 9687, 0, 0, 0 },
	{ TX_MUD4_010, 10886, 0, 0, 0 },
	{ TX_MUD4_011, 10945, 0, 0, 0 },
	{ TX_MUD4_012, 10793, 0, 0, 0 },
	{ TX_MUD4_013, 4255, 0, 0, 0 },
	{ TX_MUD4_014, 8571, 0, 0, 0 },
	{ TX_MUD4_015, 12129, 0, 0, 0 },
	{ TX_MUD4_016, 10051, 0, 0, 0 },
	{ TX_MUD4_017, 8939, 0, 0, 0 },
	{ TX_MUD4_018, 454, 0, 0, 0 },
	{ TX_MUD4_019, 7470, 0, 0, 0 },
	{ TX_MUD4_020, 7279, 0, 0, 0 },
	{ TX_MUD4_021, 8726, 0, 0, 0 },
	{ TX_MUD4_026, 9960, 0, 0, 0 },
	{ TX_MUD4_029, 10391, 0, 0, 0 },
	{ TX_MUD4_030, 10461, 0, 0, 0 },
	{ TX_MUD4_034, 9772, 0, 0, 0 },
	{ TX_MUD4_035, 11155, 0, 0, 0 },
	{ TX_MUD4_036, 10357, 0, 0, 0 },
	{ TX_MUD4_037, 9869, 0, 0, 0 },
	{ TX_MUD4_046, 12235, 0, 0, 0 },
	{ TX_MUD4_047, 12171, 0, 0, 0 },
	{ TX_MUD4_050, 8178, 0, 0, 0 },
	{ TX_MUD4_051, 12411, 0, 0, 0 },
	{ TX_MUD4_063, 12072, 0, 0, 0 },
	{ TX_MUD4_064, 10677, 0, 0, 0 },
	{ TX_MUD4_065, 8267, 0, 0, 0 },
	{ TX_MUD4_067, 10915, 0, 0, 0 },
	{ TX_MUD4_069, 11221, 0, 0, 0 },
	{ TX_MUD4_070, 10818, 0, 0, 0 },
	{ TX_MUD4_071, 10145, 0, 0, 0 },
	{ TX_MUD4_073, 11092, 0, 0, 0 },
	{ TX_MUD4_076, 12314, 0, 0, 0 },
	{ TX_MUD4_023, 7208, 0, 0, 0 },
	{ TX_MUD4_024, 7106, 0, 0, 0 },
	{ TX_MUD4_025, 6345, 0, 0, 0 },
	{ TX_MUD4_027, 6588, 0, 0, 0 },
	{ TX_MUD4_028, 6812, 0, 0, 0 },
	{ TX_MUD4_031, 4607, 0, 0, 0 },
	{ TX_MUD4_032, 9626, 0, 0, 0 },
	{ TX_MUD4_033, 7060, 0, 0, 0 },
	{ TX_MUD4_038, 5197, 0, 0, 0 },
	{ TX_MUD4_039, 5224, 0, 0, 0 },
	{ TX_MUD4_040, 5254, 0, 0, 0 },
	{ TX_MUD4_041, 5285, 0, 0, 0 },
	{ TX_MUD4_042, 7869, 0, 0, 0 },
	{ TX_MUD4_043, 2510, 0, 0, 0 },
	{ TX_MUD4_045, 7555, 0, 0, 0 },
	{ TX_MUD4_048, 4377, 0, 0, 0 },
	{ TX_MUD4_049, 5786, 0, 0, 0 },
	{ TX_MUD4_052, 5840, 0, 0, 0 },
	{ TX_MUD4_053, 5309, 0, 0, 0 },
	{ TX_MUD4_054, 6219, 0, 0, 0 },
	{ TX_MUD4_055, 5603, 0, 0, 0 },
	{ TX_MUD4_056, 5523, 0, 0, 0 },
	{ TX_MUD4_057, 4895, 0, 0, 0 },
	{ TX_MUD4_058, 5023, 0, 0, 0 },
	{ TX_MUD4_059, 4459, 0, 0, 0 },
	{ TX_MUD4_060, 4676, 0, 0, 0 },
	{ TX_MUD4_061, 4119, 0, 0, 0 },
	{ TX_MUD4_062, 4288, 0, 0, 0 },
	{ TX_MUD4_066, 8396, 0, 0, 0 },
	{ TX_MUD4_068, 9395, 0, 0, 0 },
	{ TX_MUD4_072, 9205, 0, 0, 0 },
	{ TX_MUD4_074, 8881, 0, 0, 0 },
	{ TX_MUD4_075, 7611, 0, 0, 0 },
	{ TX_MUD4_A29, 7503, 0, 0, 0 },
	{ TX_MUD4_S02, 6377, 0, 0, 0 },
	{ TX_MUD4_S03, 7323, 0, 0, 0 },
	{ TX_MUD4_S04, 6912, 0, 0, 0 },
	{ TX_MUD4N000, 7801, 0, 0, 0 },
	{ TX_MUD4N001, 11747, 0, 0, 0 },
	{ TX_MUD4N003, 11615, 0, 0, 0 },
	{ TX_MUD4N004, 11792, 0, 0, 0 },
	{ TX_MUD4N005, 3106, 0, 0, 0 },
	{ TX_MUD4N006, 11677, 0, 0, 0 },
	{ TX_MUD4N007, 3298, 0, 0, 0 },
	{ TX_MUD4N008, 8029, 0, 0, 0 },
	{ TX_MUD4N011, 11912, 0, 0, 0 },
	{ TX_MUD4N012, 11278, 0, 0, 0 },
	{ TX_MUD4N013, 7985, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText mudd4Texts[] = {
	{ TX_MUD4_003, Common::EN_ANY, "#MUD4\\MUD4_003#<<Done>>" },
	{ -1, Common::UNK_LANG, "" }
};

void Room::mudd4Tick1() {
	playVoc("MUD4LOOP");

	if (!_awayMission->mudd.tookRepairTool)
		loadActorAnim(OBJECT_REPAIR_TOOL, "s4crdv", 0xc9, 0x8e);

	if (_awayMission->mudd.viewScreenEnabled)
		loadActorAnim(OBJECT_VIEWSCREEN, "s4crvo", 0xa5, 0x76);
}

void Room::mudd4UseCommunicator() {
	showText(TX_SPEAKER_KIRK,  TX_MUD4_018);
	showText(TX_SPEAKER_UHURA, TX_STATICU1);
}

void Room::mudd4Timer2Expired() {
	playSoundEffectIndex(kSfxButton);
}

void Room::mudd4Timer3Expired() {
	playSoundEffectIndex(kSfxTransporterEnergize);
}

void Room::mudd4UseSpockOnLeftConsole() {
	_roomVar.mudd.usingLeftConsole = true;
	mudd4UseSpockOnConsole();
}

void Room::mudd4UseSpockOnRightConsole() {
	_roomVar.mudd.usingLeftConsole = false;
	mudd4UseSpockOnConsole();
}

void Room::mudd4UseSpockOnConsole() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
	loadActorStandAnim(OBJECT_SPOCK);

	if (_awayMission->mudd.translatedAlienLanguage) {
		if (!_roomVar.mudd.usingLeftConsole)
			showText(TX_SPEAKER_SPOCK, TX_MUD4_057);
	} else if (_awayMission->mudd.discoveredBase3System) {
		showText(TX_SPEAKER_SPOCK, TX_MUD4_062);
	} else {
		showText(TX_SPEAKER_SPOCK, TX_MUD4_061);
		showText(TX_SPEAKER_KIRK,  TX_MUD4_013);
		showText(TX_SPEAKER_SPOCK, TX_MUD4_062);
		showText(TX_SPEAKER_MCCOY, TX_MUD4_048);
		showText(TX_SPEAKER_SPOCK, TX_MUD4_059);
		showText(TX_SPEAKER_KIRK,  TX_MUD4_031);
		showText(TX_SPEAKER_SPOCK, TX_MUD4_060);

		if (!_awayMission->mudd.discoveredBase3System) {
			_awayMission->mudd.discoveredBase3System = true;
			_awayMission->mudd.missionScore++;
		}
	}

	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
	walkCrewmanC(OBJECT_SPOCK, 0x6d, 0x9a, &Room::mudd4SpockReachedChair);
	_awayMission->disableInput = true;
}

void Room::mudd4SpockReachedChair() {
	if (_roomVar.mudd.usingLeftConsole && _awayMission->mudd.translatedAlienLanguage)
		loadActorAnimC(OBJECT_SPOCK, "s4crss", -1, -1, &Room::mudd4SpockSatInChair);
	else
		_awayMission->disableInput = false;
}

void Room::mudd4SpockSatInChair() {
	loadActorAnim2(OBJECT_SPOCK, "s4crsr", 0x55, 0x98);

	showText(TX_SPEAKER_SPOCK, TX_MUD4_058);
	mudd4ShowLeftConsoleMenu();
}

void Room::mudd4ShowLeftConsoleMenu() {
	const TextRef choices[] = {
		TX_SPEAKER_SPOCK,
		TX_MUD4_038,
		TX_MUD4_039,
		TX_MUD4_040,
		TX_MUD4_041,
		TX_END
	};

	int choice = showMultipleTexts(choices);

	switch (choice) {
	case 0: // Sensors
		loadActorAnimC(OBJECT_SPOCK, "s4crsr", 0x55, 0x98, &Room::mudd4SpockUsedSensors);
		_awayMission->timers[2] = 5;
		break;

	case 1: // Navigation
		loadActorAnimC(OBJECT_SPOCK, "s4crsl", 0x55, 0x98, &Room::mudd4SpockUsedNavigation);
		_awayMission->timers[2] = 5;
		break;

	case 2: // Engineering
		loadActorAnimC(OBJECT_SPOCK, "s4crsr", 0x55, 0x98, &Room::mudd4SpockUsedEngineering);
		_awayMission->timers[2] = 5;
		break;

	case 3: // Done
		_awayMission->disableInput = false;
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
		walkCrewman(OBJECT_SPOCK, 0x69, 0xb7);
		break;
	}
}

void Room::mudd4SpockUsedSensors() {
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_SPOCK, TX_MUD4_053);
	mudd4ShowLeftConsoleMenu();
}

void Room::mudd4SpockUsedEngineering() {
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_SPOCK, TX_MUD4_056);
	mudd4ShowLeftConsoleMenu();
}

void Room::mudd4SpockUsedNavigation() {
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_SPOCK, TX_MUD4_055);
	mudd4ShowLeftConsoleMenu();
}

void Room::mudd4FinishedWalking3() { // Spock reaches console after attempting to use Kirk on one
	_awayMission->disableInput = false;
	mudd4UseSpockOnLeftConsole();
}

void Room::mudd4UseKirkOnRightConsole() {
	_roomVar.mudd.kirkUsingRightConsole = true;
	mudd4UseKirkOnConsole();
}

void Room::mudd4UseKirkOnLeftConsole() {
	_roomVar.mudd.kirkUsingRightConsole = false;
	mudd4UseKirkOnConsole();
}

void Room::mudd4UseKirkOnConsole() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
	loadActorStandAnim(OBJECT_KIRK);

	if (!_awayMission->mudd.translatedAlienLanguage) {
		showText(TX_SPEAKER_KIRK, TX_MUD4_006);
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
		walkCrewmanC(OBJECT_SPOCK, 0x6d, 0x9a, &Room::mudd4FinishedWalking3);
		_awayMission->disableInput = true;
	} else {
		if (!_roomVar.mudd.kirkUsingRightConsole)
			showText(TX_SPEAKER_KIRK, TX_MUD4_049);
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewmanC(OBJECT_KIRK, 0x7e, 0x98, &Room::mudd4KirkReachedRightConsole);
		_awayMission->disableInput = true;
	}
}

void Room::mudd4KirkReachedRightConsole() {
	if (_roomVar.mudd.kirkUsingRightConsole && _awayMission->mudd.translatedAlienLanguage)
		loadActorAnimC(OBJECT_KIRK, "s4crks", -1, -1, &Room::mudd4KirkSatInChair);
	else
		_awayMission->disableInput = false;
}

void Room::mudd4KirkSatInChair() {
	loadActorAnim2(OBJECT_KIRK, "s4crkl", 0x94, 0x98);
	_awayMission->timers[2] = 5;
	_awayMission->disableInput = false;

	showText(TX_SPEAKER_SPOCK, TX_MUD4_052); // NOTE: why is Spock talking here?

	mudd4ShowRightConsoleMenu();
}

void Room::mudd4ShowRightConsoleMenu() {
	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		TX_MUD4_002,
		TX_MUD4_007,
		TX_MUD4_003, // BUGFIX: this used Spock's voice instead of Kirk's in original
		TX_END
	};

	int choice = showMultipleTexts(choices);

	switch (choice) {
	case 0: // Communications
		_awayMission->disableInput = true;
		loadActorAnimC(OBJECT_KIRK, "s4crkr", 0x94, 0x98, &Room::mudd4KirkUsedCommunications);
		break;

	case 1: // View Screen
		_awayMission->disableInput = true;
		loadActorAnimC(OBJECT_KIRK, "s4crkl", 0x94, 0x98, &Room::mudd4KirkUsedViewScreen);
		break;

	case 2: // Done
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewman(OBJECT_KIRK, 0x83, 0xb6);
		break;
	}
}

void Room::mudd4KirkUsedViewScreen() {
	if (!_awayMission->mudd.viewScreenEnabled) {
		_awayMission->mudd.viewScreenEnabled = true;
		playVoc("SE2BIGAS");
		loadActorAnim(OBJECT_VIEWSCREEN, "s4crvs", 0xa5, 0x76);
		_awayMission->timers[3] = 5;
		_awayMission->timers[1] = 50;
		if (!_awayMission->mudd.gotPointsForEnablingViewscreen) {
			_awayMission->mudd.gotPointsForEnablingViewscreen = true;
			_awayMission->mudd.missionScore++;
		}
	} else {
		_awayMission->mudd.viewScreenEnabled = false;
		loadActorAnim(OBJECT_VIEWSCREEN, "s4crvf", 0xa5, 0x76);
		_awayMission->timers[3] = 5;
		_awayMission->timers[1] = 30;
	}
}

void Room::mudd4Timer1Expired() {
	_awayMission->disableInput = false;
	if (_awayMission->mudd.viewScreenEnabled)
		showText(TX_SPEAKER_KIRK, TX_MUD4_004);
	mudd4ShowRightConsoleMenu();
}

void Room::mudd4KirkUsedCommunications() {
	_awayMission->disableInput = false;
	if (!_awayMission->mudd.knowAboutTorpedo) {
		showText(TX_SPEAKER_SPOCK, TX_MUD4_054);
		mudd4ShowRightConsoleMenu();
	} else {
		if (!_awayMission->mudd.contactedEnterpriseFirstTime) {
			_awayMission->mudd.missionScore++;
			showText(TX_SPEAKER_KIRK,  TX_MUD4_025);
			showText(TX_SPEAKER_SCOTT, TX_MUD4_S02);

			if (_awayMission->mudd.torpedoStatus == 1)
				showText(TX_SPEAKER_KIRK, TX_MUD4_027);
			else
				showText(TX_SPEAKER_KIRK, TX_MUD4_028);

			showText(TX_SPEAKER_SCOTT, TX_MUD4_S04);

			const TextRef choices[] = {
				TX_SPEAKER_KIRK,
				TX_MUD4_024,
				TX_MUD4_033,
				TX_MUD4_023,
				TX_END
			};

			int choice = showMultipleTexts(choices);

			if (choice == 1) {
				endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
			} else if (choice == 0) {
				_awayMission->mudd.contactedEnterpriseFirstTime = true;
			} else { // choice == 2
				mudd4TalkWithMuddAtMissionEnd();
			}
		} else {
			const TextRef choices[] = {
				TX_SPEAKER_KIRK,
				TX_MUD4_019,
				TX_MUD4_A29,
				TX_MUD4_023,
				TX_END
			};

			showText(TX_SPEAKER_KIRK,  TX_MUD4_020);
			showText(TX_SPEAKER_SCOTT, TX_MUD4_S03);
			int choice = showMultipleTexts(choices);

			if (choice == 1) {
				endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
			} else if (choice == 2) {
				mudd4TalkWithMuddAtMissionEnd();
			}
		}
	}
}

void Room::mudd4TalkWithMuddAtMissionEnd() {
	if (_awayMission->mudd.muddUnavailable)
		return;
	else {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
		walkCrewmanC(OBJECT_KIRK, 0x83, 0xb4, &Room::mudd4KirkReachedPositionToTalkToMudd);

		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
		walkCrewman(OBJECT_SPOCK, 0x69, 0xb7);

		_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
		loadActorStandAnim(OBJECT_MCCOY);

		_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_E;
		loadActorStandAnim(OBJECT_REDSHIRT);

		playMidiMusicTracks(MIDITRACK_3);
		loadActorAnim(OBJECT_11, "s4crhw", 0x13f, 0xc7);
	}
}

void Room::mudd4KirkReachedPositionToTalkToMudd() {
	_awayMission->disableInput = false;

	if (_awayMission->mudd.discoveredLenseAndDegrimerFunction
	        || _awayMission->mudd.muddErasedDatabase
	        || _awayMission->mudd.databaseDestroyed
	        || !_awayMission->mudd.accessedAlienDatabase) { // NOTE: why this last line? Test this...
		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			TX_MUD4_009,
			TX_MUD4_016,
			TX_END
		};

		showText(TX_SPEAKER_MUDD, TX_MUD4_066);
		int choice = showMultipleTexts(choices);

		if (choice == 1) {
			// Copy of code at very bottom of function
			showText(TX_SPEAKER_MUDD, TX_MUD4_074);
			showText(TX_SPEAKER_KIRK, TX_MUD4_017);
			showText(TX_SPEAKER_MUDD, TX_MUD4_072);
			showText(TX_SPEAKER_KIRK, TX_MUD4_008);
			showText(TX_SPEAKER_MUDD, TX_MUD4_068);
			showText(TX_SPEAKER_KIRK, TX_MUD4_032);

			endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
		} else {
			// Threaten to arrest Mudd.
			if (_awayMission->mudd.muddErasedDatabase || _awayMission->mudd.databaseDestroyed)
				showText(TX_SPEAKER_KIRK, TX_MUD4_034);
			if (_awayMission->mudd.discoveredLenseAndDegrimerFunction)
				showText(TX_SPEAKER_KIRK, TX_MUD4_037);

			showText(TX_SPEAKER_KIRK, TX_MUD4_026);

			if (_awayMission->mudd.muddErasedDatabase && _awayMission->mudd.databaseDestroyed) {
				// Mudd offers to copy the lost data to the enterprise computer. You don't
				// get any points for this outcome.

				// NOTE: This combination is probably impossible, making this unused?
				// (Either you give Mudd access to the database, or he destroys it.)

				showText(TX_SPEAKER_MUDD,  TX_MUD4_071);
				showText(TX_SPEAKER_KIRK,  TX_MUD4_036);
				showText(TX_SPEAKER_SPOCK, TX_MUD4_029);
				showText(TX_SPEAKER_KIRK,  TX_MUD4_030);
			} else {
				// Mudd is forced to agree to give samples to a university.
				showText(TX_SPEAKER_MUDD, TX_MUD4_064);
				showText(TX_SPEAKER_KIRK, TX_MUD4_012);
				showText(TX_SPEAKER_MUDD, TX_MUD4_070);
				showText(TX_SPEAKER_KIRK, TX_MUD4_010);
				showText(TX_SPEAKER_MUDD, TX_MUD4_067);
				showText(TX_SPEAKER_KIRK, TX_MUD4_011);

				_awayMission->mudd.missionScore += 4;
				playMidiMusicTracks(MIDITRACK_30);

				showText(TX_SPEAKER_MUDD, TX_MUD4_073);
				showText(TX_SPEAKER_KIRK, TX_MUD4_035);
			}
			showText(TX_SPEAKER_MUDD, TX_MUD4_069);
			showText(TX_SPEAKER_KIRK, TX_MUD4_001);

			endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
		}
	} else {
		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			TX_MUD4_014,
			TX_MUD4_021,
			TX_END
		};

		showText(TX_SPEAKER_MUDD, TX_MUD4_065);
		int choice = showMultipleTexts(choices);

		if (choice == 1) {
			showText(TX_SPEAKER_MUDD, TX_MUD4_074);
			showText(TX_SPEAKER_KIRK, TX_MUD4_017);
		}
		showText(TX_SPEAKER_MUDD, TX_MUD4_072);
		showText(TX_SPEAKER_KIRK, TX_MUD4_008);
		showText(TX_SPEAKER_MUDD, TX_MUD4_068);
		showText(TX_SPEAKER_KIRK, TX_MUD4_032);

		endMission(_awayMission->mudd.missionScore, 0x1b, _awayMission->mudd.torpedoStatus);
	}
}

void Room::mudd4UseMccoyOnConsole() {
	// NOTE: This audio was recorded twice (TX_MUD4_045, and TX_MUD4_044 which is unused)
	showText(TX_SPEAKER_MCCOY, TX_MUD4_045);

	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	walkCrewman(OBJECT_MCCOY, 0x7d, 0xc3);
}

void Room::mudd4UseRedshirtOnConsole() {
	// Floppy version shows two different pieces of text here, but in the CD version,
	// there's no difference
	//if (!_awayMission->mudd.translatedAlienLanguage)
	//showText(TX_SPEAKER_BUCHERT, TX_MUD4_075);
	//else
	showText(TX_SPEAKER_BUCHERT, TX_MUD4_075);

	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_N;
	walkCrewman(OBJECT_REDSHIRT, 0x2a, 0xbb);
}

void Room::mudd4UseSTricorderOnRepairTool() {
	spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_MUD4_051, false);
}

void Room::mudd4UseSTricorderOnConsole() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_MUD4_042, false);
}

void Room::mudd4UseSTricorderOnViewscreen() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_MUD4_050, false);
}

void Room::mudd4UseMedkit() {
	showText(TX_SPEAKER_MCCOY, TX_MUD4_043);
}

void Room::mudd4GetRepairTool() {
	if (_awayMission->mudd.tookRepairTool)
		showDescription(TX_MUD4N012); // NOTE: unused, since the object disappears, can't be selected again
	else {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewmanC(OBJECT_KIRK, 0xcb, 0x9e, &Room::mudd4ReachedRepairTool);
	}
}

void Room::mudd4ReachedRepairTool() {
	loadActorAnimC(OBJECT_KIRK, "kuseln", -1, -1, &Room::mudd4PickedUpRepairTool);
}

void Room::mudd4PickedUpRepairTool() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_REPAIR_TOOL);
	_awayMission->mudd.tookRepairTool = true;
	_awayMission->mudd.missionScore++;
	giveItem(OBJECT_IDOOVER);
}

void Room::mudd4LookAtConsole() {
	showDescription(TX_MUD4N000);
}

void Room::mudd4LookAtViewscreen() {
	if (_awayMission->mudd.viewScreenEnabled)
		showDescription(TX_MUD4N008);
	else
		showDescription(TX_MUD4N013);
}

void Room::mudd4LookAtKirk() {
	showDescription(TX_MUD4N003);
}

void Room::mudd4LookAtSpock() {
	showDescription(TX_MUD4N006);
}

void Room::mudd4LookAtMccoy() {
	showDescription(TX_MUD4N001);
}

void Room::mudd4LookAtRedshirt() {
	showDescription(TX_MUD4N004);
}

void Room::mudd4LookAtRepairTool() {
	showDescription(TX_MUD4N011);
}

void Room::mudd4TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_MUD4_005);
}

void Room::mudd4TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_MUD4_063);
	showText(TX_SPEAKER_KIRK,  TX_MUD4_015);
	showText(TX_SPEAKER_MCCOY, TX_MUD4_047);
}

void Room::mudd4TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_MUD4_046);
}

void Room::mudd4TalkToRedshirt() {
	showText(TX_SPEAKER_BUCHERT, TX_MUD4_076);
}

void Room::mudd4WalkToEastDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0xf0, 0xc7);
}

void Room::mudd4WalkToWestDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0x50, 0xc7);
}

void Room::mudd4TouchedHotspot0() { // Trigger door sound
	if (_roomVar.mudd.walkingToDoor)
		playVoc("SMADOOR3");
}

}
