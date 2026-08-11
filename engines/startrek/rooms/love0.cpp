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

#define OBJECT_DOOR1 8
#define OBJECT_DOOR2 9

#define HOTSPOT_CONSOLE 0x20
#define HOTSPOT_DOOR2 0x21
#define HOTSPOT_DOOR1 0x22
#define HOTSPOT_VIEWSCREEN 0x23

namespace StarTrek {

extern const RoomAction love0ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::love0Tick1 },
	{ {ACTION_TICK, 10, 0, 0, 0}, &Room::love0Tick10 },
	{ {ACTION_WALK, 0x21, 0, 0, 0}, &Room::love0WalkToDoor2 },
	{ {ACTION_WALK, 9, 0, 0, 0}, &Room::love0WalkToDoor2 },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0}, &Room::love0OpenDoor2 },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0, 0}, &Room::love0ReachedDoor2 },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0, 0}, &Room::love0ReachedDoor2 },
	{ {ACTION_WALK, 0x22, 0, 0, 0}, &Room::love0WalkToDoor1 },
	{ {ACTION_WALK, 8, 0, 0, 0}, &Room::love0WalkToDoor1 },
	{ {ACTION_TOUCHED_HOTSPOT, 1, 0, 0, 0}, &Room::love0OpenDoor1 },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0, 0}, &Room::love0ReachedDoor1 },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0, 0}, &Room::love0ReachedDoor1 },
	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::love0LookAtConsole },
	{ {ACTION_LOOK, 0x23, 0, 0, 0}, &Room::love0LookAtViewscreen },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::love0LookAnywhere },
	{ {ACTION_LOOK, 8, 0, 0, 0}, &Room::love0LookAtDoor1 },
	{ {ACTION_LOOK, 9, 0, 0, 0}, &Room::love0LookAtDoor2 },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0}, &Room::love0LookAtKirk },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0}, &Room::love0LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0}, &Room::love0LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::love0LookAtRedshirt },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0, 0}, &Room::love0TalkToKirk },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0}, &Room::love0TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0}, &Room::love0TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::love0TalkToRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0, 0}, &Room::love0UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x20, 0, 0}, &Room::love0UseSTricorderOnConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::love0UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_KIRK, 0x20, 0, 0}, &Room::love0UseKirkOnConsole },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x20, 0, 0}, &Room::love0UseRedshirtOnConsole },
	{ {ACTION_USE, OBJECT_SPOCK, 0x20, 0, 0}, &Room::love0UseSpockOnConsole },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0}, &Room::love0SpockReachedConsole },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0, 0}, &Room::love0SpockAccessedConsole },
	{ {ACTION_USE, OBJECT_MCCOY, 0x20, 0, 0}, &Room::love0UseMccoyOnConsole },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0, 0}, &Room::love0MccoyReachedConsole },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0, 0}, &Room::love0MccoyAccessedConsole },
	{ {ACTION_GET,    8, 0, 0, 0}, &Room::love0GetDoorOrConsole },
	{ {ACTION_GET,    9, 0, 0, 0}, &Room::love0GetDoorOrConsole },
	{ {ACTION_GET, 0x20, 0, 0, 0}, &Room::love0GetDoorOrConsole },

	// Common code
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0, 0}, &Room::loveaTimer0Expired },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0, 0}, &Room::loveaTimer1Expired },
	{ {ACTION_USE, OBJECT_IMEDKIT, OBJECT_SPOCK, 0, 0}, &Room::loveaUseMedkitOnSpock },
	{ {ACTION_USE, OBJECT_ISAMPLE, OBJECT_SPOCK, 0, 0}, &Room::loveaUseCureSampleOnSpock },
	{ {ACTION_USE, OBJECT_ICURE, OBJECT_SPOCK, 0, 0}, &Room::loveaUseCureOnSpock },
	{ {ACTION_FINISHED_WALKING,   99, 0, 0, 0}, &Room::loveaSpockOrMccoyInPositionToUseCure },
	{ {ACTION_FINISHED_ANIMATION, 99, 0, 0, 0}, &Room::loveaFinishedCuringSpock },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0, 0}, &Room::loveaTimer2Expired },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK, 0, 0}, &Room::loveaUseMTricorderOnSpock },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK, 0, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY, 0, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IRLG, 0xff, 0, 0}, &Room::loveaUseRomulanLaughingGas },
	{ {ACTION_USE, OBJECT_IN2O, 0xff, 0, 0}, &Room::loveaUseHumanLaughingGas },
	{ {ACTION_USE, OBJECT_INH3, 0xff, 0, 0}, &Room::loveaUseAmmonia },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0, 0}, &Room::loveaUseCommunicator },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum love0TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_FERRIS, TX_SPEAKER_COMPUTER,
	TX_LOV0_001, TX_LOV0_002, TX_LOV0_003, TX_LOV0_004, TX_LOV0_005,
	TX_LOV0_006, TX_LOV0_007, TX_LOV0_008, TX_LOV0_009, TX_LOV0_010,
	TX_LOV0_023, TX_LOV0_024, TX_LOV0_027, TX_LOV0_028, TX_LOV0_030,
	TX_LOV0_031, TX_LOV0_035, TX_LOV0_036, TX_LOV0_037, TX_LOV0_038,
	TX_LOV0_049, TX_LOV0N000, TX_LOV0N001, TX_LOV0N002,	TX_LOV0N003,
	TX_LOV0N004, TX_LOV0N005, TX_LOV0N006, TX_LOV0N007, TX_LOV0N008,
	TX_LOV0N009, TX_COMPA186, TX_COMPA187, TX_COMPA189, TX_COMPA190,
	TX_COMPA191, TX_COMPA193, TX_COMPU186, TX_COMPU187, TX_COMPU188,
	TX_COMPU189, TX_COMPU190, TX_COMPU192, TX_COMPU193, TX_DIALOG_ERROR
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets love0TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 5915, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 5926, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 5936, 0, 0, 0 },
	{ TX_SPEAKER_FERRIS, 5946, 0, 0, 0 },
	{ TX_SPEAKER_COMPUTER, 5957, 0, 0, 0 },
	{ TX_LOV0_001, 10075, 0, 0, 0 },
	{ TX_LOV0_002,  1488, 0, 0, 0 },
	{ TX_LOV0_003,   959, 0, 0, 0 },
	{ TX_LOV0_004,  7933, 0, 0, 0 },
	{ TX_LOV0_005, 10149, 0, 0, 0 },
	{ TX_LOV0_006,  1256, 0, 0, 0 },
	{ TX_LOV0_007,  1105, 0, 0, 0 },
	{ TX_LOV0_008,  6848, 0, 0, 0 },
	{ TX_LOV0_009,  7999, 0, 0, 0 },
	{ TX_LOV0_010, 10477, 0, 0, 0 },
	{ TX_LOV0_023,  7881, 0, 0, 0 },
	{ TX_LOV0_024,  7536, 0, 0, 0 },
	{ TX_LOV0_027,  6650, 0, 0, 0 },
	{ TX_LOV0_028,  7062, 0, 0, 0 },
	{ TX_LOV0_030, 10335, 0, 0, 0 },
	{ TX_LOV0_031,  6931, 0, 0, 0 },
	{ TX_LOV0_035,  7805, 0, 0, 0 },
	{ TX_LOV0_036,  1591, 0, 0, 0 },
	{ TX_LOV0_037, 10240, 0, 0, 0 },
	{ TX_LOV0_038,  6760, 0, 0, 0 },
	{ TX_LOV0_049, 11461, 0, 0, 0 },
	{ TX_LOV0N000,  6449, 0, 0, 0 },
	{ TX_LOV0N001, 10380, 0, 0, 0 },
	{ TX_LOV0N002,  6227, 0, 0, 0 },
	{ TX_LOV0N003,  6344, 0, 0, 0 },
	{ TX_LOV0N004,  6185, 0, 0, 0 },
	{ TX_LOV0N005,  6292, 0, 0, 0 },
	{ TX_LOV0N006,  6049, 0, 0, 0 },
	{ TX_LOV0N007,  6525, 0, 0, 0 },
	{ TX_LOV0N008,  5971, 0, 0, 0 },
	{ TX_LOV0N009,  6399, 0, 0, 0 },
	{ TX_COMPA186,  8349, 0, 0, 0 },
	{ TX_COMPA187,  8372, 0, 0, 0 },
	{ TX_COMPA189,  8320, 0, 0, 0 },
	{ TX_COMPA190,  8265, 0, 0, 0 },
	{ TX_COMPA191,  8405, 0, 0, 0 },
	{ TX_COMPA193,  8295, 0, 0, 0 },
	{ TX_COMPU186,  9642, 0, 0, 0 },
	{ TX_COMPU187,  9864, 0, 0, 0 },
	{ TX_COMPU188,  7106, 0, 0, 0 },
	{ TX_COMPU189,  9381, 0, 0, 0 },
	{ TX_COMPU190,  8434, 0, 0, 0 },
	{ TX_COMPU192,  8220, 0, 0, 0 },
	{ TX_COMPU193,  8858, 0, 0, 0 },
	{ TX_DIALOG_ERROR, 12536, 0, 0, 0 },
	{          -1,  0,    0, 0, 0 }
};

extern const RoomText love0Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::love0Tick1() {
	// BUGFIX: moved this out of the if statement below.
	playVoc("LOV0LOOP");

	if (!_awayMission->love.alreadyStartedMission) {
		_awayMission->love.canister1 = CANTYPE_O2;
		_awayMission->love.canister2 = CANTYPE_H2;
		_awayMission->love.alreadyStartedMission = true;
	}

	if (_awayMission->love.releasedHumanLaughingGas)
		_awayMission->timers[0] = getRandomWordInRange(200, 400);
	if (_awayMission->love.releasedRomulanLaughingGas)
		_awayMission->timers[1] = getRandomWordInRange(200, 400);
	_awayMission->timers[2] = 200;

	loadActorAnim(OBJECT_DOOR2, "s3r0d2a", 0xe6, 0x80, 0);
	loadActorAnim(OBJECT_DOOR1, "s3r0d1a", 0x123, 0x8d, 0);

	_roomVar.love.cmnXPosToCureSpock = 0xf4;
	_roomVar.love.cmnYPosToCureSpock = 0x8f;
}

void Room::love0Tick10() {
}

void Room::love0WalkToDoor2() {
	_awayMission->disableInput = true;
	_roomVar.love.walkingToDoor = 2;
	walkCrewman(OBJECT_KIRK, 0xe6, 0x81, 4);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love0OpenDoor2() {
	if (_roomVar.love.walkingToDoor == 2) {
		loadActorAnim(OBJECT_DOOR2, "s3r0d2", 0xe6, 0x80, 3);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::love0ReachedDoor2() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(2, 1);
}

void Room::love0WalkToDoor1() {
	_awayMission->disableInput = true;
	_roomVar.love.walkingToDoor = 1;
	walkCrewman(OBJECT_KIRK, 0x125, 0x8d, 5);
}

void Room::love0OpenDoor1() {
	if (_roomVar.love.walkingToDoor == 1) {
		loadActorAnim(OBJECT_DOOR1, "s3r0d1", 0x123, 0x8d, 4);
		playSoundEffectIndex(kSfxDoor);
	}
}

void Room::love0ReachedDoor1() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(1, 3);
}

void Room::love0LookAtConsole() {
	showDescription(TX_LOV0N006);
}

void Room::love0LookAtViewscreen() {
	showDescription(TX_LOV0N000);
}

void Room::love0LookAnywhere() {
	showDescription(TX_LOV0N009);
}

void Room::love0LookAtDoor1() {
	showDescription(TX_LOV0N008);
}

void Room::love0LookAtDoor2() {
	showDescription(TX_LOV0N007);
}

void Room::love0LookAtKirk() {
	showDescription(TX_LOV0N002);
}

void Room::love0LookAtMccoy() {
	showDescription(TX_LOV0N004);
}

void Room::love0LookAtSpock() {
	showDescription(TX_LOV0N005);
}

void Room::love0LookAtRedshirt() {
	showDescription(TX_LOV0N003);
}

void Room::love0TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_LOV0_003);
}

void Room::love0TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_LOV0_007);
}

void Room::love0TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_LOV0_027);
}

void Room::love0TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, TX_LOV0_038);
}

void Room::love0UseMTricorderAnywhere() {
	if (_awayMission->love.knowAboutVirus)
		mccoyScan(DIR_N, TX_SPEAKER_MCCOY, TX_LOV0_008, false);
	else
		mccoyScan(DIR_N, TX_SPEAKER_MCCOY, TX_LOV0_006, false);
}

void Room::love0UseSTricorderOnConsole() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK,TX_LOV0_031, false);
}

void Room::love0UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK,TX_LOV0_028, false);
}

void Room::love0UseKirkOnConsole() {
	showText(TX_SPEAKER_KIRK, TX_LOV0_002);
	love0UseSpockOnConsole();
}

void Room::love0UseRedshirtOnConsole() {
	showText(TX_SPEAKER_FERRIS, TX_LOV0_036);
}

void Room::love0UseSpockOnConsole() {
	walkCrewman(OBJECT_SPOCK, 0x9a, 0x9a, 2);
	if (!_awayMission->love.spockAccessedConsole) {
		_awayMission->love.spockAccessedConsole = true;
		_awayMission->love.missionScore += 4;
	}
}

void Room::love0SpockReachedConsole() {
	loadActorAnim2(OBJECT_SPOCK, "susemn", -1, -1, 5);
}

void Room::love0SpockAccessedConsole() {
	playVoc("V6KIRKTY");
	if (_awayMission->love.knowAboutVirus)
		love0InteractWithConsole();
	else {
		showText(TX_SPEAKER_COMPUTER, TX_COMPU188);
		showText(TX_SPEAKER_SPOCK, TX_LOV0_005);
		_roomVar.love.heardSummaryOfVirus = true;
	}
}

void Room::love0UseMccoyOnConsole() {
	walkCrewman(OBJECT_MCCOY, 0x78, 0x98, 3);
}

void Room::love0MccoyReachedConsole() {
	loadActorAnim2(OBJECT_MCCOY, "musemw", -1, -1, 6);
}

void Room::love0MccoyAccessedConsole() {
	playVoc("V6KIRKTY");
	if (!_awayMission->love.mccoyAccessedConsole) {
		_awayMission->love.mccoyAccessedConsole = true;
		_awayMission->love.missionScore += 2;
	}

	if (_awayMission->love.knowAboutVirus)
		love0InteractWithConsole();
	else {
		if (!_roomVar.love.heardSummaryOfVirus) {
			showText(TX_SPEAKER_COMPUTER, TX_COMPU188);
			_roomVar.love.heardSummaryOfVirus = true;
		}
		showText(TX_SPEAKER_MCCOY, TX_LOV0_024);
		showText(TX_SPEAKER_SPOCK, TX_LOV0_035);
		showText(TX_SPEAKER_MCCOY, TX_LOV0_023);
		showText(TX_SPEAKER_KIRK,  TX_LOV0_004);
		showText(TX_SPEAKER_MCCOY, TX_LOV0_009);
		_awayMission->love.knowAboutVirus = true;
	}
}

// Interact with computer console, by selecting topics for the computer to talk about
void Room::love0InteractWithConsole() {
	const TextRef choices[] = {
		TX_SPEAKER_COMPUTER,
		TX_COMPA190,
		TX_COMPA193,
		TX_COMPA189,
		TX_COMPA186,
		TX_COMPA187,
		TX_COMPA191,
		TX_END
	};

	while (true) {
		showText(TX_SPEAKER_COMPUTER, TX_COMPU192);
		int choice = showMultipleTexts(choices);

		switch (choice) {
		case 0:
			showText(TX_SPEAKER_COMPUTER, TX_COMPU190);
			break;
		case 1:
			showText(TX_SPEAKER_COMPUTER, TX_COMPU193);
			break;
		case 2:
			showText(TX_SPEAKER_COMPUTER, TX_COMPU189);
			break;
		case 3:
			showText(TX_SPEAKER_COMPUTER, TX_COMPU186);
			break;
		case 4:
			showText(TX_SPEAKER_COMPUTER, TX_COMPU187);
			break;
		case 5:
			return;
		default:
			showDescription(TX_DIALOG_ERROR);
			break;
		}
	}
}

void Room::love0GetDoorOrConsole() {
	// BUGFIX: There was a problem with "get door 1" where it would execute a bit of
	// non-code before reaching where it was supposed to be. Not sure if it had any actual
	// effect.
	showDescription(TX_LOV0N001);

	// NOTE: There is an unused block of code that's jumped over in the "get door 2"
	// function. (Spock says "that's not logical".)
	//showText(TX_SPEAKER_SPOCK, TX_LOV0_030);
}

}
