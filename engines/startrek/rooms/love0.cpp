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
	if (!_vm->_awayMission.love.alreadyStartedMission) {
		_vm->_awayMission.love.field36 = 1;
		_vm->_awayMission.love.field37 = 2;
		_vm->_awayMission.love.alreadyStartedMission = true;
		playVoc("LOV0LOOP"); // FIXME: no audio after first entry?
	}

	if (_vm->_awayMission.love.field2c)
		_vm->_awayMission.timers[0] = getRandomWordInRange(200, 400);
	if (_vm->_awayMission.love.field2d)
		_vm->_awayMission.timers[1] = getRandomWordInRange(200, 400);
	_vm->_awayMission.timers[2] = 200;

	loadActorAnim(OBJECT_DOOR2, "s3r0d2a", 0xe6, 0x80, 0);
	loadActorAnim(OBJECT_DOOR1, "s3r0d1a", 0x123, 0x8d, 0);
	_roomVar.love0._8ab = 0xf4;
	_roomVar.love0._8ac = 0x8f;
}

void Room::love0Tick10() {
}

void Room::love0WalkToDoor2() {
	_vm->_awayMission.disableInput = true;
	_roomVar.love0._de = 2;
	walkCrewman(OBJECT_KIRK, 0xe6, 0x81, 4);
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love0OpenDoor2() {
	if (_roomVar.love0._de == 2) {
		loadActorAnim(OBJECT_DOOR2, "s3r0d2", 0xe6, 0x80, 3);
		playSoundEffectIndex(SND_DOOR1);
	}
}

void Room::love0ReachedDoor2() {
	_roomVar.love0.door2OpenCounter++;
	if (_roomVar.love0.door2OpenCounter == 2)
		loadRoomIndex(2, 1);
}

void Room::love0WalkToDoor1() {
	_vm->_awayMission.disableInput = true;
	_roomVar.love0._de = 1;
	walkCrewman(OBJECT_KIRK, 0x125, 0x8d, 5);
}

void Room::love0OpenDoor1() {
	if (_roomVar.love0._de == 1) {
		loadActorAnim(OBJECT_DOOR1, "s3r0d1", 0x123, 0x8d, 4);
		playSoundEffectIndex(SND_DOOR1);
	}
}

void Room::love0ReachedDoor1() {
	_roomVar.love0.door1OpenCounter++;
	if (_roomVar.love0.door1OpenCounter == 2)
		loadRoomIndex(3, 1);
}

void Room::love0LookAtConsole() {
	showText(TX_LOV0N006);
}

void Room::love0LookAtViewscreen() {
	showText(TX_LOV0N000);
}

void Room::love0LookAnywhere() {
	showText(TX_LOV0N009);
}

void Room::love0LookAtDoor1() {
	showText(TX_LOV0N008);
}

void Room::love0LookAtDoor2() {
	showText(TX_LOV0N007);
}

void Room::love0LookAtKirk() {
	showText(TX_LOV0N002);
}

void Room::love0LookAtMccoy() {
	showText(TX_LOV0N004);
}

void Room::love0LookAtSpock() {
	showText(TX_LOV0N005);
}

void Room::love0LookAtRedshirt() {
	showText(TX_LOV0N003);
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
	if (_vm->_awayMission.love.knowAboutVirus)
		mccoyScan(DIR_N, TX_LOV0_008, false);
	else
		mccoyScan(DIR_N, TX_LOV0_006, false);
}

void Room::love0UseSTricorderOnConsole() {
	spockScan(DIR_N, TX_LOV0_031, false);
}

void Room::love0UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_LOV0_028, false);
}

void Room::love0UseKirkOnConsole() {
	showText(TX_SPEAKER_KIRK, TX_LOV0_002);
	love0UseSpockOnConsole();
}

void Room::love0UseRedshirtOnConsole() {
	showText(TX_SPEAKER_FERRIS, TX_LOV0_036);
}

void Room::love0UseSpockOnConsole() {
	_roomVar.love0.consoleCrewman = OBJECT_SPOCK;
	_roomVar.love0.consoleSpeaker = TX_SPEAKER_SPOCK;
	_roomVar.love0.consoleText = TX_LOV0_005;
	strcpy(_roomVar.love0.consoleAnimation, "susemn");

	walkCrewman(_roomVar.love0.consoleCrewman, 0x9a, 0x9a, 2);
	if (!_vm->_awayMission.love.spockAccessedConsole) {
		_vm->_awayMission.love.spockAccessedConsole = true;
		_vm->_awayMission.love.missionScore += 4;
	}
}

void Room::love0SpockReachedConsole() {
	loadActorAnim2(_roomVar.love0.consoleCrewman, _roomVar.love0.consoleAnimation, -1, -1, 5);
}

void Room::love0SpockAccessedConsole() {
	playVoc("V6KIRKTY");
	if (_vm->_awayMission.love.knowAboutVirus)
		love0InteractWithConsole();
	else {
		showText(TX_SPEAKER_COMPUTER, TX_COMPU188);
		showText(_roomVar.love0.consoleSpeaker, _roomVar.love0.consoleText);
		_roomVar.love0.heardSummaryOfVirus = true;
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
	if (!_vm->_awayMission.love.mccoyAccessedConsole) {
		_vm->_awayMission.love.mccoyAccessedConsole = true;
		_vm->_awayMission.love.missionScore += 2;
	}

	if (_vm->_awayMission.love.knowAboutVirus)
		love0InteractWithConsole();
	else {
		if (!_roomVar.love0.heardSummaryOfVirus) {
			showText(TX_SPEAKER_COMPUTER, TX_COMPU188);
			_roomVar.love0.heardSummaryOfVirus = true;
		}
		showText(TX_SPEAKER_MCCOY, TX_LOV0_024);
		showText(TX_SPEAKER_SPOCK, TX_LOV0_035);
		showText(TX_SPEAKER_MCCOY, TX_LOV0_023);
		showText(TX_SPEAKER_KIRK,  TX_LOV0_004);
		showText(TX_SPEAKER_MCCOY, TX_LOV0_009);
		_vm->_awayMission.love.knowAboutVirus = true;
	}
}

// Interact with computer console, by selecting topics for the computer to talk about
void Room::love0InteractWithConsole() {
	const int choices[] = {
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
		int choice = showText(choices);

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
			showText(TX_DIALOG_ERROR);
			break;
		}
	}
}

}
