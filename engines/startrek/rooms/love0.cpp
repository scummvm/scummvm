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

#define OBJECT_DOOR1 8
#define OBJECT_DOOR2 9

#define HOTSPOT_CONSOLE 0x20
#define HOTSPOT_DOOR2 0x21
#define HOTSPOT_DOOR1 0x22
#define HOTSPOT_VIEWSCREEN 0x23

namespace StarTrek {

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
		playSoundEffectIndex(SND_DOOR1);
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
		playSoundEffectIndex(SND_DOOR1);
	}
}

void Room::love0ReachedDoor1() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(1, 3);
}

void Room::love0LookAtConsole() {
	showDescription(6, true);
}

void Room::love0LookAtViewscreen() {
	showDescription(0, true);
}

void Room::love0LookAnywhere() {
	showDescription(9, true);
}

void Room::love0LookAtDoor1() {
	showDescription(8, true);
}

void Room::love0LookAtDoor2() {
	showDescription(7, true);
}

void Room::love0LookAtKirk() {
	showDescription(2, true);
}

void Room::love0LookAtMccoy() {
	showDescription(4, true);
}

void Room::love0LookAtSpock() {
	showDescription(5, true);
}

void Room::love0LookAtRedshirt() {
	showDescription(3, true);
}

void Room::love0TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 3, true);
}

void Room::love0TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 7, true);
}

void Room::love0TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 27, true);
}

void Room::love0TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, 38, true);
}

void Room::love0UseMTricorderAnywhere() {
	if (_awayMission->love.knowAboutVirus)
		mccoyScan(DIR_N, 8, false, true);
	else
		mccoyScan(DIR_N, 6, false, true);
}

void Room::love0UseSTricorderOnConsole() {
	spockScan(DIR_N, 31, false, true);
}

void Room::love0UseSTricorderAnywhere() {
	spockScan(DIR_S, 28, false, true);
}

void Room::love0UseKirkOnConsole() {
	showText(TX_SPEAKER_KIRK, 2, true);
	love0UseSpockOnConsole();
}

void Room::love0UseRedshirtOnConsole() {
	showText(TX_SPEAKER_FERRIS, 36, true);
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
		showText(TX_SPEAKER_SPOCK, 5, true);
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
		showText(TX_SPEAKER_MCCOY, 24, true);
		showText(TX_SPEAKER_SPOCK, 35, true);
		showText(TX_SPEAKER_MCCOY, 23, true);
		showText(TX_SPEAKER_KIRK,  4, true);
		showText(TX_SPEAKER_MCCOY, 9, true);
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
		TX_BLANK
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
	showDescription(1, true);

	// NOTE: There is an unused block of code that's jumped over in the "get door 2"
	// function. (Spock says "that's not logical".)
	//showText(TX_SPEAKER_SPOCK, TX_LOV0_030);
}

}
