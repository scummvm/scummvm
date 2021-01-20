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

extern const RoomAction tug1ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::tug1Tick1 },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0}, &Room::tug1UseSTricorderOnAnything },
	{ {ACTION_LOOK, 0x21, 0, 0}, &Room::tug1LookAtBridgeDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0}, &Room::tug1UseSTricorderOnBridgeDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, 0x21, 0}, &Room::tug1UsePhaserOnBridgeDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, 0x21, 0}, &Room::tug1UsePhaserOnBridgeDoor },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0}, &Room::tug1KirkReachedFiringPosition },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0}, &Room::tug1KirkPulledOutPhaser },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0}, &Room::tug1KirkFinishedFiringPhaser },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0}, &Room::tug1TalkToSpock },
	{ {ACTION_USE, OBJECT_ISTRICOR, 8, 0}, &Room::tug1UseSTricorderOnJunkPile },
	{ {ACTION_LOOK, 8, 0, 0}, &Room::tug1LookAtJunkPile },
	{ {ACTION_GET, 8, 0, 0}, &Room::tug1GetJunkPile },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0}, &Room::tug1KirkReachedJunkPile },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0}, &Room::tug1KirkFinishedTakingJunkPile },

	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_IPWE, 0}, &Room::tug1UsePhaserOnWelder },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_IPWE, 0}, &Room::tug1UsePhaserOnWelder },
	{ {ACTION_USE, OBJECT_IPWF, OBJECT_IWIRSCRP, 0}, &Room::tug1UseWelderOnWireScraps },
	{ {ACTION_USE, OBJECT_IPWF, OBJECT_IJNKMETL, 0}, &Room::tug1UseWelderOnMetalScraps },
	{ {ACTION_USE, OBJECT_ICOMBBIT, OBJECT_IRT, 0}, &Room::tug1UseCombBitOnTransmogrifier },

	{ {ACTION_USE, OBJECT_IPWF, 0x22, 0}, &Room::tug1UsePhaserWelderOnBridgeDoor },
	{ {ACTION_USE, OBJECT_IPWF, 0x21, 0}, &Room::tug1UsePhaserWelderOnBridgeDoor },
	{ {ACTION_USE, OBJECT_IPWF, 0x20, 0}, &Room::tug1UsePhaserWelderOnBridgeDoorInLeftSpot },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0}, &Room::tug1KirkReachedBridgeDoorWithWelder },
	{ {ACTION_FINISHED_ANIMATION, 7, 0, 0}, &Room::tug1KirkFinishedUsingWelder },
	{ {ACTION_FINISHED_WALKING, 8, 0, 0}, &Room::tug1KirkReachedBridgeDoorWithWelderInLeftSpot },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0}, &Room::tug1KirkFinishedUsingWelderInLeftSpot },
	{ {ACTION_LOOK, 0xff, 0, 0}, &Room::tug1LookAnywhere },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0}, &Room::tug1LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0}, &Room::tug1LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::tug1LookAtRedshirt },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0}, &Room::tug1TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::tug1TalkToRedshirt },
	{ {ACTION_LOOK, 0x23, 0, 0}, &Room::tug1LookAtTerminal },
	{ {ACTION_LOOK, 0x24, 0, 0}, &Room::tug1LookAtDebris },
	{ {ACTION_LOOK, 0x25, 0, 0}, &Room::tug1LookAtBrigDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x25, 0}, &Room::tug1UseSTricorderOnBrigDoor },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0}, &Room::tug1TalkToKirk },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0}, &Room::tug1UseCommunicator },
	{ {ACTION_WALK, 0x21, 0, 0}, &Room::tug1WalkToBridgeDoor },
	{ {ACTION_FINISHED_WALKING, 10, 0, 0}, &Room::tug1KirkReachedBridgeDoor },
	{ {ACTION_FINISHED_ANIMATION, 11, 0, 0}, &Room::tug1BridgeDoorOpened },
	{ {ACTION_WALK, 0x25, 0, 0}, &Room::tug1WalkToBrigDoor },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0}, &Room::tug1UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x21, 0}, &Room::tug1UseMTricorderOnBridgeDoor },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x20, 0}, &Room::tug1UseMTricorderOnBridgeDoor },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x22, 0}, &Room::tug1UseMTricorderOnBridgeDoor },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x25, 0}, &Room::tug1UseMTricorderOnBrigDoor },
	{ {ACTION_USE, OBJECT_SPOCK, 0x20, 0}, &Room::tug1UseSpockOnBridgeDoor },
	{ {ACTION_USE, OBJECT_SPOCK, 0x21, 0}, &Room::tug1UseSpockOnBridgeDoor },
	{ {ACTION_USE, OBJECT_SPOCK, 0x22, 0}, &Room::tug1UseSpockOnBridgeDoor },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x20, 0}, &Room::tug1UseRedshirtOnBridgeDoor },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x21, 0}, &Room::tug1UseRedshirtOnBridgeDoor },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x22, 0}, &Room::tug1UseRedshirtOnBridgeDoor },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0x20, 0}, &Room::tug1UseMedkitOnBridgeDoor },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0x21, 0}, &Room::tug1UseMedkitOnBridgeDoor },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0x22, 0}, &Room::tug1UseMedkitOnBridgeDoor },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::tug1Tick1() {
	playVoc("TUG1LOOP");

	if (!_awayMission->tug.gotJunkPile)
		loadActorAnim2(OBJECT_JUNKPILE, "jnkpil", 0xfd, 0xa0, 0);
}

void Room::tug1UseSTricorderOnAnything() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, 14, true);

	// NOTE: this action has a second implementation (which is never called). It displayed
	// TX_TUG1_006. Was this meant to be displayed after the force field is down?
}

void Room::tug1LookAtBridgeDoor() {
	if (_awayMission->tug.bridgeForceFieldDown)
		return;
	showDescription(5, true);
}

void Room::tug1UseSTricorderOnBridgeDoor() {
	if (_awayMission->tug.bridgeForceFieldDown)
		return;

	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, 2, true);
}

void Room::tug1UsePhaserOnBridgeDoor() {
	if (_awayMission->tug.bridgeForceFieldDown)
		showDescription(7, true);
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
	playSoundEffectIndex(kSfxPhaser);
}

void Room::tug1KirkFinishedFiringPhaser() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
	loadActorStandAnim(OBJECT_PHASERSHOT);
}

void Room::tug1TalkToSpock() {
	if (!_awayMission->tug.bridgeForceFieldDown)
		showText(TX_SPEAKER_SPOCK, 10, true);
}

void Room::tug1UseSTricorderOnJunkPile() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, 9, true);
}

void Room::tug1LookAtJunkPile() {
	showDescription(0, true);
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
	showDescription(4, true);

	_awayMission->tug.missionScore += 3;
}

void Room::tug1UseWelderOnWireScraps() {
	showDescription(9, true);
	loseItem(OBJECT_IWIRSCRP);
}

void Room::tug1UseWelderOnMetalScraps() {
	showText(TX_SPEAKER_SPOCK, 12, true);
	loseItem(OBJECT_IJNKMETL);
	giveItem(OBJECT_ICOMBBIT);
}

void Room::tug1UseCombBitOnTransmogrifier() {
	showText(TX_SPEAKER_SPOCK, 17, true);
	loseItem(OBJECT_ICOMBBIT);
	loseItem(OBJECT_IRT);
	giveItem(OBJECT_IRTWB);
}

void Room::tug1UsePhaserWelderOnBridgeDoor() {
	if (_awayMission->tug.bridgeForceFieldDown)
		showDescription(8, true);
	else {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		walkCrewman(OBJECT_KIRK, 0xc2, 0x6a, 6);
	}
}

void Room::tug1UsePhaserWelderOnBridgeDoorInLeftSpot() {
	if (_awayMission->tug.bridgeForceFieldDown)
		showDescription(7, true);
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
	showText(TX_SPEAKER_SPOCK, 16, true);
	_awayMission->disableInput = false;
}

void Room::tug1KirkReachedBridgeDoorWithWelderInLeftSpot() {
	playVoc("MUR4E9S");
	loadActorAnim2(OBJECT_KIRK, "kuseln", -1, -1, 0);
	loadActorAnim2(OBJECT_PHASERSHOT, "t1weld", 0, 0, 9);
}

void Room::tug1KirkFinishedUsingWelderInLeftSpot() {
	loadActorStandAnim(OBJECT_PHASERSHOT);
	showText(TX_SPEAKER_SPOCK, 15, true);
	_awayMission->tug.bridgeForceFieldDown = true;
	_awayMission->disableInput = false;
	_awayMission->tug.missionScore++;
}

void Room::tug1LookAnywhere() {
	showDescription(13, true);
}

void Room::tug1LookAtMccoy() {
	showDescription(3, true);
}

void Room::tug1LookAtSpock() {
	showDescription(6, true);
}

void Room::tug1LookAtRedshirt() {
	showDescription(2, true);
}

void Room::tug1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_SIN3_012); // NOTE: uses "SIN3" text?
}

void Room::tug1TalkToRedshirt() {
	showText(TX_SPEAKER_CHRISTENSEN, 5, true, true);
}

void Room::tug1LookAtTerminal() {
	showDescription(10, true);
}

void Room::tug1LookAtDebris() {
	showDescription(11, true);
}

void Room::tug1LookAtBrigDoor() {
	showDescription(1, true);
}

void Room::tug1UseSTricorderOnBrigDoor() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, 18, true);
}

void Room::tug1TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  1, true);
	showText(TX_SPEAKER_MCCOY, 8, true);
}

// FIXME: not working
void Room::tug1UseCommunicator() {
	showText(TX_SPEAKER_SPOCK, 11, true);
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
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, 3, true);
}

void Room::tug1UseMTricorderOnBridgeDoor() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, 7, true);
}

void Room::tug1UseMTricorderOnBrigDoor() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, 5, true);
}

void Room::tug1UseSpockOnBridgeDoor() {
	if (!_awayMission->tug.bridgeForceFieldDown)
		showText(TX_SPEAKER_SPOCK, 13, true);
}

void Room::tug1UseRedshirtOnBridgeDoor() {
	if (!_awayMission->tug.bridgeForceFieldDown)
		showText(TX_SPEAKER_CHRISTENSEN, 0, true, true);
}

void Room::tug1UseMedkitOnBridgeDoor() {
	if (!_awayMission->tug.bridgeForceFieldDown)
		showText(TX_SPEAKER_MCCOY, 4, true);
}

}
