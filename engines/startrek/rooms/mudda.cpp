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

// The functions here used to be independently implemented in each room of the MUDD
// mission, despite being mostly the same.

void Room::muddaUseLenseOnDegrimer() {
	assert(_roomIndex >= 0 && _roomIndex <= 5);

	const TextRef text[] = {
		TX_MUD0N011, // All of these audio files are identical, but there's one for each room.
		TX_MUD1N013,
		TX_MUD2N010,
		TX_MUD3N016,
		TX_MUD4N009,
		TX_MUD5N009,
	};

	giveItem(OBJECT_IALIENDV);
	loseItem(OBJECT_IDEGRIME);
	loseItem(OBJECT_ILENSES);

	_vm->_awayMission.mudd.missionScore++;
	showText(text[_roomIndex]);
	// TODO: Identical (?) audio files: TX_MUD0N011, TX_MUD1N013, TX_MUD2N010, TX_MUD3N016,
	// TX_MUD4009, TX_MUD5N009
}


void Room::muddaUseAlienDevice() {
	assert(_roomIndex >= 0 && _roomIndex <= 5);

	const int deviceObjectIndices[] = { // Each room's object index for the explosion is different
		9,  // MUDD0
		13, // MUDD1
		11, // MUDD2
		11, // MUDD3
		9,  // MUDD4
		11  // MUDD5
	};

	_vm->_awayMission.disableInput = true;

	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	loadActorStandAnim(OBJECT_KIRK);
	Common::Point pos = getActorPos(OBJECT_KIRK);
	loadActorAnimC(deviceObjectIndices[_roomIndex], "s4cbxp", pos.x, 10, &Room::muddaFiredAlienDevice);
	playVoc("EXPLO3");
}

void Room::muddaFiredAlienDevice() {
	assert(_roomIndex >= 0 && _roomIndex <= 5);

	const TextRef text[] = {
		TX_MUD0_002, // These audio files aren't identical, but the text is mostly the same.
		TX_MUD1_002,
		TX_MUD2_002,
		TX_MUD2_002, // Rooms 3-5 reuse MUD2
		TX_MUD2_002,
		TX_MUD2_002,
	};

	_vm->_awayMission.disableInput = false;
	if (!_vm->_awayMission.mudd.discoveredLenseAndDegrimerFunction) {
		_vm->_awayMission.mudd.discoveredLenseAndDegrimerFunction = true;
		_vm->_awayMission.mudd.missionScore += 5; // BUGFIX: didn't happen if done in MUDD5
		showText(TX_SPEAKER_KIRK, text[_roomIndex]);
	}
}


void Room::muddaUseDegrimer() {
	assert(_roomIndex >= 0 && _roomIndex <= 5);

	const TextRef text[] = {
		TX_MUD0N002, // All of these audio files are identical, but there's one for each room.
		TX_MUD1N004,
		TX_MUD2N001,
		TX_MUD3N001,
		TX_MUD4N002,
		TX_MUD5N001,
	};

	showText(text[_roomIndex]);
}

void Room::muddaTick() {
	assert(_roomIndex >= 0 && _roomIndex <= 5);

	const Common::Point deathPositions[][4] = {
		{ Common::Point(0xbb, 0x8d), Common::Point(0xd0, 0x89), Common::Point(0xaa, 0x85), Common::Point(0xbf, 0x83) },
		{ Common::Point(-1, -1), Common::Point(-1, -1), Common::Point(-1, -1), Common::Point(-1, -1) },
		{ Common::Point(-1, -1), Common::Point(-1, -1), Common::Point(-1, -1), Common::Point(-1, -1) },
		{ Common::Point(-1, -1), Common::Point(-1, -1), Common::Point(-1, -1), Common::Point(-1, -1) },
		{ Common::Point(-1, -1), Common::Point(-1, -1), Common::Point(-1, -1), Common::Point(-1, -1) },
		{ Common::Point(0x8b, 0xac), Common::Point(0x6f, 0x99), Common::Point(-1, -1), Common::Point(-1, -1) },
	};

	const TextRef deathText[] = {
		TX_MUD0N006, 0, 0, 0, TX_MUD5N105
	};

	// This is implemented somewhat differently in each room.
	// MUDD0:

	//const int TIMER_LENGTH = 27000;
	const int TIMER_LENGTH = 60; // FIXME

	if (_vm->_awayMission.mudd.lifeSupportMalfunctioning) {
		if (!_vm->_awayMission.mudd.startedLifeSupportTimer) {
			_vm->_awayMission.mudd.startedLifeSupportTimer = true;
			_vm->_awayMission.mudd.lifeSupportTimer = TIMER_LENGTH;
		}
		_vm->_awayMission.mudd.lifeSupportTimer--;

		if (_vm->_awayMission.mudd.lifeSupportTimer == (int)(TIMER_LENGTH * 0.25)) {
			showText(TX_SPEAKER_SPOCK, TX_MUD0_018);
		} else if (_vm->_awayMission.mudd.lifeSupportTimer == (int)(TIMER_LENGTH * 0.5)) {
			showText(TX_SPEAKER_SPOCK, TX_MUD0_019);
		} else if (_vm->_awayMission.mudd.lifeSupportTimer == (int)(TIMER_LENGTH * 0.75)) {
			showText(TX_SPEAKER_SPOCK, TX_MUD0_020);
		} else if (_vm->_awayMission.mudd.lifeSupportTimer == 1) {
			_vm->_awayMission.disableInput = true;
			for (int i = OBJECT_KIRK; i <= OBJECT_REDSHIRT; i++) {
				if (deathPositions[_roomIndex][i].x != -1)
					walkCrewman(i, deathPositions[_roomIndex][i].x, deathPositions[_roomIndex][i].y, 9 + i);
			}
			showText(deathText[_roomIndex]);
			showGameOverMenu();
		}
	}
}

void Room::muddaKirkReachedDeathPosition() {
	loadActorAnim2(OBJECT_KIRK, "kgetdw");
}

void Room::muddaSpockReachedDeathPosition() {
	loadActorAnim2(OBJECT_SPOCK, "sgetds");
}

void Room::muddaMccoyReachedDeathPosition() {
	loadActorAnim2(OBJECT_MCCOY, "sgetdn");
}

void Room::muddaRedshirtReachedDeathPosition() {
	loadActorAnim2(OBJECT_REDSHIRT, "rgetde");
	// NOTE: there's code to check if he's the last one down... not really implemented
	// properly
}

}
