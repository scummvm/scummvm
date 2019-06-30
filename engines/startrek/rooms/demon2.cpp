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
	_awayMission->demon.enteredFrom = 2;
	_awayMission->rdfStillDoDefaultAction = true;
}

void Room::demon2LookAtCave() {
	showDescription(9, true);
}

void Room::demon2LookAtMountain() {
	showDescription(7, true);
}

void Room::demon2LookAtBerries() {
	showDescription(12, true);
}

void Room::demon2LookAtFern() {
	showDescription(5, true);
}

void Room::demon2LookAtMoss() {
	showDescription(10, true);
}

void Room::demon2LookAtLights() {
	showDescription(6, true);
}

void Room::demon2LookAtAnything() {
	showDescription(11, true);
}

void Room::demon2LookAtKirk() {
	showDescription(2, true);
}

void Room::demon2LookAtSpock() {
	showDescription(4, true);
}

void Room::demon2LookAtMcCoy() {
	showDescription(3, true);
}

void Room::demon2LookAtRedshirt() {
	showDescription(0, true);
}

void Room::demon2TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 1, true);
}

void Room::demon2TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 18, true);
	showText(TX_SPEAKER_KIRK,  3, true);
}

void Room::demon2TalkToMcCoy() {
	showText(TX_SPEAKER_MCCOY, 15, true);
	showText(TX_SPEAKER_SPOCK, 17, true);
	showText(TX_SPEAKER_MCCOY, 16, true);
}

void Room::demon2TalkToRedshirt() {
	showText(TX_SPEAKER_EVERTS, 19, true);
	showText(TX_SPEAKER_KIRK,   2, true);
	showText(TX_SPEAKER_EVERTS, 20, true);
	showText(TX_SPEAKER_KIRK,   4, true);
}

void Room::demon2UsePhaserOnBerries() {
	showText(TX_SPEAKER_MCCOY, 11, true);
}

void Room::demon2UseSTricorderOnBerries() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, 8, true);
}

void Room::demon2UseSTricorderOnMoss() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, 7, true);
}

void Room::demon2UseSTricorderOnFern() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, 6, true);
}

void Room::demon2UseSTricorderOnCave() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);

	// NOTE: audio file for this is empty.
	showText(TX_SPEAKER_SPOCK, 5, true);
}

void Room::demon2UseMTricorderOnBerries() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_awayMission->demon.curedChub)
		showText(TX_SPEAKER_MCCOY, 13, true);
	else if (_awayMission->demon.madeHypoDytoxin)
		showText(TX_SPEAKER_MCCOY, 9, true);
	else if (_awayMission->demon.knowAboutHypoDytoxin)
		showText(TX_SPEAKER_MCCOY, 10, true);
	else
		showText(TX_SPEAKER_MCCOY, 12, true);
}

void Room::demon2GetBerries() {
	if (_awayMission->demon.gotBerries)
		showText(TX_SPEAKER_MCCOY, 14, true);
	else
		walkCrewman(OBJECT_KIRK, 0xe8, 0x97, 1);
}

void Room::demon2ReachedBerries() {
	loadActorAnim2(OBJECT_KIRK, "kusehe", -1, -1, 1);
	playVoc("PIKBERRY");
}

void Room::demon2PickedUpBerries() {
	showDescription(13, true);
	giveItem(OBJECT_IBERRY);
	_awayMission->demon.gotBerries = true;
	_awayMission->demon.missionScore += 1;
}

}
