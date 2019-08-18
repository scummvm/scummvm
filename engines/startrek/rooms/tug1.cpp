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

#define OBJECT_JUNKPILE 8
#define OBJECT_PHASERSHOT 9

#define HOTSPOT_BRIDGEDOOR 0x21
#define HOTSPOT_BRIDGEARCH 0x22
#define HOTSPOT_TERMINAL   0x23
#define HOTSPOT_DEBRIS     0x24
#define HOTSPOT_BRIGDOOR   0x25

void Room::tug1Tick1() {
	playVoc("TUG1LOOP");

	if (!_awayMission->tug.gotJunkPile)
		loadActorAnim2(OBJECT_JUNKPILE, "jnkpil", 0xfd, 0xa0, 0);
}

void Room::tug1UseSTricorderOnAnything() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_SPOCK, TX_TUG1_014);

	// NOTE: this action has a second implementation (which is never called). It displayed
	// TX_TUG1_006. Was this meant to be displayed after the force field is down?
}

void Room::tug1LookAtBridgeDoor() {
	if (_awayMission->tug.bridgeForceFieldDown)
		return;
	showDescription(TX_TUG1N005);
}

void Room::tug1UseSTricorderOnBridgeDoor() {
	if (_awayMission->tug.bridgeForceFieldDown)
		return;

	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_SPOCK, TX_TUG1_002);
}

void Room::tug1UsePhaserOnBridgeDoor() {
	if (_awayMission->tug.bridgeForceFieldDown)
		showDescription(TX_TUG1N007);
	else {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewman(OBJECT_KIRK, 0xbe, 0x78, 3);
	}
}

void Room::tug1KirkReachedFiringPosition() {
	loadActorAnim2(OBJECT_KIRK, "kfiren", -1, -1, 4);
}

void Room::tug1KirkPulledOutPhaser() {
	loadActorAnim2(OBJECT_PHASERSHOT, "t1phas", 0, 0, 5);
	playSoundEffectIndex(SND_PHASSHOT);
}

void Room::tug1KirkFinishedFiringPhaser() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
	loadActorStandAnim(OBJECT_PHASERSHOT);
}

void Room::tug1TalkToSpock() {
	if (!_awayMission->tug.bridgeForceFieldDown)
		showText(TX_SPEAKER_SPOCK, TX_TUG1_010);
}

void Room::tug1UseSTricorderOnJunkPile() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_SPOCK, TX_TUG1_009);
}

void Room::tug1LookAtJunkPile() {
	showDescription(TX_TUG1N000);
}

void Room::tug1GetJunkPile() {
	walkCrewman(OBJECT_KIRK, 0xe1, 0xaa, 1);
}

void Room::tug1KirkReachedJunkPile() {
	loadActorAnim2(OBJECT_KIRK, "kpicke", 0xe1, 0xaa, 2);
}

void Room::tug1KirkFinishedTakingJunkPile() {
	_awayMission->tug.gotJunkPile = true;
	giveItem(OBJECT_IDEADPH);
	giveItem(OBJECT_IPWE);
	_awayMission->tug.missionScore++;
	giveItem(OBJECT_IWIRSCRP);
	giveItem(OBJECT_IJNKMETL);

	loadActorStandAnim(OBJECT_JUNKPILE);
	loadActorStandAnim(OBJECT_KIRK);
}

void Room::tug1UsePhaserOnWelder() {
	loseItem(OBJECT_IPWE);
	giveItem(OBJECT_IPWF);
	showDescription(TX_TUG1N004);

	_awayMission->tug.missionScore += 3;
}

void Room::tug1UseWelderOnWireScraps() {
	showDescription(TX_TUG1N009);
	loseItem(OBJECT_IWIRSCRP);
}

void Room::tug1UseWelderOnMetalScraps() {
	showText(TX_SPEAKER_SPOCK, TX_TUG1_012);
	loseItem(OBJECT_IJNKMETL);
	giveItem(OBJECT_ICOMBBIT);
}

void Room::tug1UseCombBitOnTransmogrifier() {
	showText(TX_SPEAKER_SPOCK, TX_TUG1_017);
	loseItem(OBJECT_ICOMBBIT);
	loseItem(OBJECT_IRT);
	giveItem(OBJECT_IRTWB);
}

void Room::tug1UsePhaserWelderOnBridgeDoor() {
	if (_awayMission->tug.bridgeForceFieldDown)
		showDescription(TX_TUG1N008);
	else {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewman(OBJECT_KIRK, 0xc2, 0x6a, 6);
	}
}

void Room::tug1UsePhaserWelderOnBridgeDoorInLeftSpot() {
	if (_awayMission->tug.bridgeForceFieldDown)
		showDescription(TX_TUG1N007);
	else {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewman(OBJECT_KIRK, 0xb1, 0x6a, 8);
	}
}

void Room::tug1KirkReachedBridgeDoorWithWelder() {
	playVoc("MUR4E9S");
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 0);
	loadActorAnim2(OBJECT_PHASERSHOT, "t1weld", 0, 0, 7);
}

void Room::tug1KirkFinishedUsingWelder() {
	loadActorStandAnim(OBJECT_PHASERSHOT);
	showText(TX_SPEAKER_SPOCK, TX_TUG1_016);
	_awayMission->disableInput = false;
}

void Room::tug1KirkReachedBridgeDoorWithWelderInLeftSpot() {
	playVoc("MUR4E9S");
	loadActorAnim2(OBJECT_KIRK, "kuseln", -1, -1, 0);
	loadActorAnim2(OBJECT_PHASERSHOT, "t1weld", 0, 0, 9);
}

void Room::tug1KirkFinishedUsingWelderInLeftSpot() {
	loadActorStandAnim(OBJECT_PHASERSHOT);
	showText(TX_SPEAKER_SPOCK, TX_TUG1_015);
	_awayMission->tug.bridgeForceFieldDown = true;
	_awayMission->disableInput = false;
	_awayMission->tug.missionScore++;
}

void Room::tug1LookAnywhere() {
	showDescription(TX_TUG1N013);
}

void Room::tug1LookAtMccoy() {
	showDescription(TX_TUG1N003);
}

void Room::tug1LookAtSpock() {
	showDescription(TX_TUG1N006);
}

void Room::tug1LookAtRedshirt() {
	showDescription(TX_TUG1N002);
}

void Room::tug1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_SIN3_012); // NOTE: uses "SIN3" text?
}

void Room::tug1TalkToRedshirt() {
	showText(TX_SPEAKER_CHRISTENSEN, TX_TUG1L005);
}

void Room::tug1LookAtTerminal() {
	showDescription(TX_TUG1N010);
}

void Room::tug1LookAtDebris() {
	showDescription(TX_TUG1N011);
}

void Room::tug1LookAtBrigDoor() {
	showDescription(TX_TUG1N001);
}

void Room::tug1UseSTricorderOnBrigDoor() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_SPOCK, TX_TUG1_018);
}

void Room::tug1TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  TX_TUG1_001);
	showText(TX_SPEAKER_MCCOY, TX_TUG1_008);
}

// FIXME: not working
void Room::tug1UseCommunicator() {
	showText(TX_SPEAKER_SPOCK, TX_TUG1_011);
}

void Room::tug1WalkToBridgeDoor() {
	if (!_awayMission->tug.bridgeForceFieldDown)
		return;
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
	walkCrewman(OBJECT_KIRK, 0xc2, 0x6e, 10);
}

void Room::tug1KirkReachedBridgeDoor() {
	_awayMission->disableInput = true;
	loadActorAnim2(OBJECT_PHASERSHOT, "h1do", 0, 0, 11);
}

void Room::tug1BridgeDoorOpened() {
	walkCrewman(OBJECT_KIRK, 0xc2, 0x63, 0);
}

void Room::tug1WalkToBrigDoor() {
	walkCrewman(OBJECT_KIRK, 0xe9, 0x81, 0);
}

void Room::tug1UseMTricorderAnywhere() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_MCCOY, TX_TUG1_003);
}

void Room::tug1UseMTricorderOnBridgeDoor() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_MCCOY, TX_TUG1_007);
}

void Room::tug1UseMTricorderOnBrigDoor() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_MCCOY, TX_TUG1_005);
}

void Room::tug1UseSpockOnBridgeDoor() {
	if (!_awayMission->tug.bridgeForceFieldDown)
		showText(TX_SPEAKER_SPOCK, TX_TUG1_013);
}

void Room::tug1UseRedshirtOnBridgeDoor() {
	if (!_awayMission->tug.bridgeForceFieldDown)
		showText(TX_SPEAKER_CHRISTENSEN, TX_TUG1L000);
}

void Room::tug1UseMedkitOnBridgeDoor() {
	if (!_awayMission->tug.bridgeForceFieldDown)
		showText(TX_SPEAKER_MCCOY, TX_TUG1_004);
}

}
