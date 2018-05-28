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

void Room::demon4Tick1() {
	playVoc("DEM4LOOP");

	if (!_vm->_awayMission.demon.healedMiner)
		_vm->_awayMission.demon.minerDead = true;

	if (_vm->_awayMission.demon.field4e) {
		loadActorAnim(10, "bxrise2", 0x122, 0x91, 0);
		loadActorAnim(8, "stpout2", 0x107, 0x92, 0);
		_roomVar.demon4.cb = true;

		if ((_vm->_awayMission.demon.field4f & 0x10) && !_vm->_awayMission.demon.field47)
			_vm->_awayMission.timers[6] = 20;
	}

	if (!_vm->_awayMission.demon.foundAlienRoom) {
		_vm->_awayMission.demon.foundAlienRoom = true;
		playMidiMusicTracks(1, -1);
	}
}

void Room::demon4FinishedAnimation1() {
	// TODO
}

void Room::demon4FinishedAnimation2() {
}

void Room::demon4FinishedAnimation3() {
}

void Room::demon4UsePhaserOnPanel() {
	showText(TX_SPEAKER_SPOCK, TX_DEM4_013);
}

void Room::demon4UsePhaserOnPattern() {
	showText(TX_SPEAKER_SPOCK, TX_DEM4_045);
}

void Room::demon4UsePhaserOnMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_DEM4_018);
}

void Room::demon4LookAtPattern() {
	showText(TX_DEM4N002);

}

void Room::demon4LookAtAlien() {
	showText(TX_DEM4N007);
}

void Room::demon4LookAnywhere() {
	showText(TX_DEM4N004);
}

void Room::demon4LookAtSecurityEquipment() {
	showText(TX_DEM4N008);
}

void Room::demon4LookAtFloor() {
	showText(TX_DEM4N009);
}

void Room::demon4LookAtKirk() {
	showText(TX_DEM4N003);
}

void Room::demon4LookAtMccoy() {
	showText(TX_DEM4N000);
}

void Room::demon4LookAtSpock() {
	showText(TX_DEM4N005);
}

void Room::demon4LookAtRedshirt() {
	showText(TX_DEM4N001);
}

void Room::demon4LookAtChamber() {
	showText(TX_DEM4N006);
}

void Room::demon4LookAtPanel() {
	showText(TX_DEM4N012);
}

void Room::demon4UseKirkOnPanel() {
	_roomVar.demon4.crewmanUsingPanel = OBJECT_KIRK;
	demon4UseCrewmanOnPanel();
}

void Room::demon4UseSpockOnPanel() {
	_roomVar.demon4.crewmanUsingPanel = OBJECT_SPOCK;
	demon4UseCrewmanOnPanel();
}

void Room::demon4UseMccoyOnPanel() {
	_roomVar.demon4.crewmanUsingPanel = OBJECT_MCCOY;
	demon4UseCrewmanOnPanel();
}

void Room::demon4UseRedshirtOnPanel() {
	_roomVar.demon4.crewmanUsingPanel = OBJECT_REDSHIRT;
	demon4UseCrewmanOnPanel();
}

void Room::demon4UseCrewmanOnPanel() {
	if (_vm->_awayMission.demon.field4e)
		return;
	walkCrewman(_roomVar.demon4.crewmanUsingPanel, 0xda, 0x83, 3);
	_vm->_awayMission.crewDirectionsAfterWalk[_roomVar.demon4.crewmanUsingPanel] = DIR_N;
	_vm->_awayMission.transitioningIntoRoom = true;
}

void Room::demon4CrewmanReachedPanel() {
	// TODO: puzzle implementation
}

}
