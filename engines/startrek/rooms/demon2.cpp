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

void Room::demon2Tick1() {
	playVoc("DEM2LOOP");
}

void Room::demon2WalkToCave() {
	walkCrewman(OBJECT_KIRK, 0x83, 0x5c, 2);
}

void Room::demon2ReachedCave() {
	loadRoomIndex(3, 1);
}

void Room::demon2TouchedWarp1() {
	_vm->_awayMission.demon.enteredFrom = 2;
	_vm->_awayMission.rdfStillDoDefaultAction = true;
}

void Room::demon2LookAtCave() {
	showText(TX_DEM2N009);
}

void Room::demon2LookAtMountain() {
	showText(TX_DEM2N007);
}

void Room::demon2LookAtBerries() {
	showText(TX_DEM2N012);
}

void Room::demon2LookAtFern() {
	showText(TX_DEM2N005);
}

void Room::demon2LookAtMoss() {
	showText(TX_DEM2N010);
}

void Room::demon2LookAtLights() {
	showText(TX_DEM2N006);
}

void Room::demon2LookAtAnything() {
	showText(TX_DEM2N011);
}

void Room::demon2LookAtKirk() {
	showText(TX_DEM2N002);
}

void Room::demon2LookAtSpock() {
	showText(TX_DEM2N004);
}

void Room::demon2LookAtMcCoy() {
	showText(TX_DEM2N003);
}

void Room::demon2LookAtRedshirt() {
	showText(TX_DEM2N000);
}

void Room::demon2TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_DEM2_001);
}

void Room::demon2TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_DEM2_018);
	showText(TX_SPEAKER_KIRK,  TX_DEM2_003);
}

void Room::demon2TalkToMcCoy() {
	showText(TX_SPEAKER_MCCOY, TX_DEM2_015);
	showText(TX_SPEAKER_SPOCK, TX_DEM2_017);
	showText(TX_SPEAKER_MCCOY, TX_DEM2_016);
}

void Room::demon2TalkToRedshirt() {
	showText(TX_SPEAKER_EVERTS, TX_DEM2_019);
	showText(TX_SPEAKER_KIRK,   TX_DEM2_002);
	showText(TX_SPEAKER_EVERTS, TX_DEM2_020);
	showText(TX_SPEAKER_KIRK,   TX_DEM2_004);
}

void Room::demon2UsePhaserOnBerries() {
	showText(TX_SPEAKER_MCCOY, TX_DEM2_011);
}

void Room::demon2UseSTricorderOnBerries() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM2_008);
}

void Room::demon2UseSTricorderOnMoss() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM2_007);
}

void Room::demon2UseSTricorderOnFern() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, TX_DEM2_006);
}

void Room::demon2UseSTricorderOnCave() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);

	// NOTE: audio file for this is empty.
	showText(TX_SPEAKER_SPOCK, TX_DEM2_005);
}

void Room::demon2UseMTricorderOnBerries() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_vm->_awayMission.demon.curedChub)
		showText(TX_SPEAKER_MCCOY, TX_DEM2_013);
	else if (_vm->_awayMission.demon.madeHypoDytoxin)
		showText(TX_SPEAKER_MCCOY, TX_DEM2_009);
	else if (_vm->_awayMission.demon.knowAboutHypoDytoxin)
		showText(TX_SPEAKER_MCCOY, TX_DEM2_010);
	else
		showText(TX_SPEAKER_MCCOY, TX_DEM2_012);
}

void Room::demon2GetBerries() {
	if (_vm->_awayMission.demon.gotBerries)
		showText(TX_SPEAKER_MCCOY, TX_DEM2_014);
	else
		walkCrewman(OBJECT_KIRK, 0xe8, 0x97, 1);
}

void Room::demon2ReachedBerries() {
	loadActorAnim2(OBJECT_KIRK, "kusehe", -1, -1, 1);
	playVoc("PIKBERRY");
}

void Room::demon2PickedUpBerries() {
	showText(TX_DEM2N013);
	giveItem(OBJECT_IBERRY);
	_vm->_awayMission.demon.gotBerries = true;
	_vm->_awayMission.demon.missionScore += 1;
}

}
