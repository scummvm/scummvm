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
#define OBJECT_DOOR3 10
#define OBJECT_FREEZER 11
#define OBJECT_CHAMBER 12
#define OBJECT_PHASERSHOT 13
#define OBJECT_DISH_IN_CHAMBER 14
#define OBJECT_BOTTLE 15

#define HOTSPOT_LADDER 0x20
#define HOTSPOT_DISTILLATOR 0x21
#define HOTSPOT_FREEZER 0x22
#define HOTSPOT_REPLICATOR 0x23
#define HOTSPOT_NOZZLE 0x24
#define HOTSPOT_LASER 0x25
#define HOTSPOT_DOOR3 0x26
#define HOTSPOT_DOOR1 0x27
#define HOTSPOT_DOOR2 0x28

namespace StarTrek {

void Room::love1Tick1() {
	playVoc("LOV1LOOP");

	if (_awayMission->love.freezerOpen)
		loadActorAnim(OBJECT_FREEZER, "s3r2d4o", 0x67, 0x8d, 0);

	if (_awayMission->love.chamberHasDish)
		loadActorAnim(OBJECT_CHAMBER, "s3r2d5o", 0xb4, 0x75, 0);
	else
		loadActorAnim(OBJECT_CHAMBER, "s3r2d5c", 0xb4, 0x75, 0);

	if (_awayMission->love.chamberHasDish)
		loadActorAnim(OBJECT_DISH_IN_CHAMBER, "dishes", 0xb4, 0x71, 0);

	switch (_awayMission->love.bottleInNozzle) {
	case BOTTLETYPE_N2O:
		strcpy(_roomVar.love.bottleAnimation, "btle1");
		_roomVar.love.itemInNozzle = OBJECT_IN2O;
		goto common;
	case BOTTLETYPE_NH3:
		strcpy(_roomVar.love.bottleAnimation, "btle2");
		_roomVar.love.itemInNozzle = OBJECT_INH3;
		goto common;
	case BOTTLETYPE_H2O:
		strcpy(_roomVar.love.bottleAnimation, "btle3");
		_roomVar.love.itemInNozzle = OBJECT_IH2O;
		goto common;
	case BOTTLETYPE_RLG:
		strcpy(_roomVar.love.bottleAnimation, "btle4");
		_roomVar.love.itemInNozzle = OBJECT_IRLG;
		goto common;

common:
		loadActorAnim(OBJECT_BOTTLE, _roomVar.love.bottleAnimation, 0xa3, 0x72, 0);
	// fall through

	case BOTTLETYPE_NONE:
		break;
	}

	if (_awayMission->love.releasedHumanLaughingGas)
		_awayMission->timers[0] = getRandomWordInRange(200, 400);
	if (_awayMission->love.releasedRomulanLaughingGas)
		_awayMission->timers[1] = getRandomWordInRange(200, 400);
	_awayMission->timers[2] = 200;

	loadActorAnim(OBJECT_DOOR3, "s3r2d3a", 0xdb, 0x7e, 0);
	loadActorAnim(OBJECT_DOOR1, "s3r2d1a", 0, 0, 0);
	loadActorAnim(OBJECT_DOOR2, "s3r2d2a", 0, 0, 0);

	_roomVar.love.cmnXPosToCureSpock = 0x90;
	_roomVar.love.cmnYPosToCureSpock = 0xa7;
}

void Room::love1WalkToDoor3() {
	_awayMission->disableInput = true;
	_roomVar.love.walkingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0xd9, 0x81, 15);
}

void Room::love1OpenDoor3() {
	if (_roomVar.love.walkingToDoor) {
		loadActorAnim(OBJECT_DOOR3, "s3r2d3", 0xdb, 0x7e, 2);
		playSoundEffectIndex(SND_DOOR1);
	}
}

// Door 3 opened, or door 3 reached (both must occur before transition happens)
void Room::love1ReachedDoor3() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(3, 1);
}

void Room::love1WalkToDoor1() {
	_awayMission->disableInput = true;
	_roomVar.love.walkingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0x42, 0x97, 16);
}

void Room::love1OpenDoor1() {
	if (_roomVar.love.walkingToDoor) {
		loadActorAnim(OBJECT_DOOR1, "s3r2d1", 0, 0, 3);
		playSoundEffectIndex(SND_DOOR1);
	}
}

// Door 1 opened, or door 1 reached
void Room::love1ReachedDoor1() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(0, 2);
}

void Room::love1WalkToDoor2() {
	_awayMission->disableInput = true;
	_roomVar.love.walkingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0x79, 0x85, 17);
}

void Room::love1OpenDoor2() {
	if (_roomVar.love.walkingToDoor) {
		loadActorAnim(OBJECT_DOOR2, "s3r2d2", 0, 0, 4);
		playSoundEffectIndex(SND_DOOR1);
	}
}

// Door 2 opened, or door 2 reached
void Room::love1ReachedDoor2() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(2, 0);
}

void Room::love1LookAtLaser() {
	showDescription(TX_LOV1N000);
}

void Room::love1LookAtKirk() {
	showDescription(TX_LOV1N003);
}

void Room::love1LookAtSpock() {
	showDescription(TX_LOV1N004);
}

void Room::love1LookAtMccoy() {
	showDescription(TX_LOV1N001);
}

void Room::love1LookAtRedshirt() {
	showDescription(TX_LOV1N002);
}

void Room::love1LookAnywhere() {
	showDescription(TX_LOV1N019);
}

void Room::love1LookAtNozzle() {
	showDescription(TX_LOV1N022);
}

void Room::love1LookAtLadder() {
	showDescription(TX_LOV1N021);
}

void Room::love1LookAtDoor1Or2() {
	showDescription(TX_LOV1N020);
}

void Room::love1LookAtDoor3() {
	showDescription(TX_LOV1N017);
}

void Room::love1LookAtDistillator() {
	showDescription(TX_LOV1N016);
}

void Room::love1LookAtChamber() {
	if (_awayMission->love.chamberHasCure)
		showDescription(TX_LOV1N013);
	else
		showDescription(TX_LOV1N014);
}

void Room::love1LookAtReplicator() {
	showDescription(TX_LOV1N015);
}

void Room::love1LookAtFreezer() {
	if (_awayMission->love.freezerOpen)
		showDescription(TX_LOV1N023);
	else
		showDescription(TX_LOV1N018);
}

void Room::love1LookAtDishes() {
	showDescription(TX_LOV1N012);
}

void Room::love1TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_LOV1_001);
}

void Room::love1TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_LOV1_005);
}

void Room::love1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_LOV1_011);
}

void Room::love1TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, TX_LOV1_027);
}

void Room::love1UseMTricorderOnDishes() {
	mccoyScan(DIR_W, TX_LOV1_017, false);
	// TODO: only works in this room, despite being inventory item?
}

void Room::love1UseMTricorderAnywhere() {
	if (_awayMission->love.knowAboutVirus)
		mccoyScan(DIR_S, TX_LOV1_009, false);
	else
		mccoyScan(DIR_S, TX_LOV1_010, false);
}

void Room::love1UseSTricorderOnReplicator() {
	spockScan(DIR_N, TX_LOV1_022, false);
}

void Room::love1UseSTricorderOnLaser() {
	spockScan(DIR_E, TX_LOV1_006, false);
}

void Room::love1UseSTricorderOnFreezer() {
	spockScan(DIR_W, TX_LOV1_023, false);
}

void Room::love1UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_LOV1_003, false);
}

void Room::love1UseSTricorderOnDishes() {
	spockScan(DIR_W, TX_LOV1_004, false);
	// TODO: only works in this room, despite being inventory item?
}

void Room::love1UseSTricorderOnDistillator() {
	spockScan(DIR_S, TX_LOV1_007, false);
}

void Room::love1GetFreezer() {
	walkCrewman(OBJECT_KIRK, 0x71, 0x8e, 14);
}

void Room::love1KirkReachedFreezer() {
	if (_awayMission->love.freezerOpen)
		loadActorAnim2(OBJECT_KIRK, "kusehw", -1, -1, 9);
	else
		showDescription(TX_LOV1N010);
}

void Room::love1KirkGotVirusCulture() {
	giveItem(OBJECT_IDISHES);
	showDescription(TX_LOV1N006);
}

void Room::love1GetFromChamber() {
	if (!_awayMission->love.chamberHasDish)
		showDescription(TX_LOV1N009);
	else
		walkCrewman(OBJECT_KIRK, 0xb5, 0x8c, 8);
}

void Room::love1KirkReachedChamber() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 10);
}

void Room::love1KirkGotCureSample() {
	if (_awayMission->love.chamberHasCure) {
		giveItem(OBJECT_ISAMPLE);
		showDescription(TX_LOV1N035);

		// BUGFIX: after removing the cure, unset this variable.
		// Otherwise, any normal dish inserted afterward automagically becomes a cure.
		_awayMission->love.chamberHasCure = false;
	} else {
		giveItem(OBJECT_IDISHES);
		showDescription(TX_LOV1N006);
	}

	loadActorStandAnim(OBJECT_DISH_IN_CHAMBER);
	loadActorAnim2(OBJECT_CHAMBER, "s3r2d6", 0xb4, 0x75, 0);
	playSoundEffectIndex(SND_DOOR1);
	_awayMission->love.chamberHasDish = false;
}

void Room::love1GetFromNozzle() {
	walkCrewman(OBJECT_KIRK, 0xa6, 0x90, 2);
}

void Room::love1KirkReachedNozzleToGet() {
	if (_awayMission->love.bottleInNozzle == 0)
		showDescription(TX_LOV1N011);
	else
		loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 11);
}

void Room::love1KirkGotBottleFromNozzle() {
	switch (_awayMission->love.bottleInNozzle) {
	case BOTTLETYPE_N2O:
		giveItem(OBJECT_IN2O);
		break;
	case BOTTLETYPE_NH3:
		giveItem(OBJECT_INH3);
		break;
	case BOTTLETYPE_H2O:
		giveItem(OBJECT_IH2O);
		break;
	case BOTTLETYPE_RLG:
		giveItem(OBJECT_IRLG);
		break;
	default:
		showDescription(TX_DIALOG_ERROR);
		break;
	}

	_awayMission->love.bottleInNozzle = BOTTLETYPE_NONE;
	loadActorStandAnim(OBJECT_BOTTLE);
	showDescription(TX_LOV1N007);
	_roomVar.love.itemInNozzle = 0;
}

void Room::love1UseN2OOnNozzle() {
	if (_awayMission->love.bottleInNozzle == BOTTLETYPE_NONE) {
		_roomVar.love.itemInNozzle = OBJECT_IN2O;
		strcpy(_roomVar.love.bottleAnimation, "btle1");
		_awayMission->love.bottleInNozzle = BOTTLETYPE_N2O;
		walkCrewman(OBJECT_KIRK, 0xa6, 0x90, 3);
	}
}

void Room::love1UseH2OOnNozzle() {
	if (_awayMission->love.bottleInNozzle == BOTTLETYPE_NONE) {
		_roomVar.love.itemInNozzle = OBJECT_IH2O;
		strcpy(_roomVar.love.bottleAnimation, "btle3");
		_awayMission->love.bottleInNozzle = BOTTLETYPE_H2O;
		walkCrewman(OBJECT_KIRK, 0xa6, 0x90, 3);
	}
}

void Room::love1UseNH3OnNozzle() {
	if (_awayMission->love.bottleInNozzle == BOTTLETYPE_NONE) {
		_roomVar.love.itemInNozzle = OBJECT_INH3;
		strcpy(_roomVar.love.bottleAnimation, "btle2");
		_awayMission->love.bottleInNozzle = BOTTLETYPE_NH3;
		walkCrewman(OBJECT_KIRK, 0xa6, 0x90, 3);
	}
}

void Room::love1UseRLGOnNozzle() {
	if (_awayMission->love.bottleInNozzle == BOTTLETYPE_NONE) {
		_roomVar.love.itemInNozzle = OBJECT_IRLG;
		strcpy(_roomVar.love.bottleAnimation, "btle4");
		_awayMission->love.bottleInNozzle = BOTTLETYPE_RLG;
		walkCrewman(OBJECT_KIRK, 0xa6, 0x90, 3);
	}
}

void Room::love1KirkReachedNozzleToPut() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 12);
}

void Room::love1KirkPutBottleInNozzle() {
	loadActorAnim(OBJECT_BOTTLE, _roomVar.love.bottleAnimation, 0xa3, 0x72, 0);
	loseItem(_roomVar.love.itemInNozzle);
}

void Room::love1UseAnthingOnNozzle() {
	showText(TX_SPEAKER_MCCOY, TX_LOV1_012);
}

void Room::love1UseSpockOnReplicator() {
	showText(TX_SPEAKER_SPOCK, TX_LOV1_002);
}

void Room::love1UseRedshirtOnReplicator() {
	showText(TX_SPEAKER_FERRIS, TX_LOV1_026);
}

void Room::love1UseMccoyOnReplicator() {
	walkCrewman(OBJECT_MCCOY, 0xb1, 0x8c, 4);
}

void Room::love1MccoyReachedReplicator() {
	if (_awayMission->love.bottleInNozzle == BOTTLETYPE_NONE) {
		showText(TX_SPEAKER_MCCOY, TX_LOV1_021);
		walkCrewman(OBJECT_MCCOY, 0xbf, 0x98, 0);
	} else if (!_awayMission->love.chamberHasDish) {
		showText(TX_SPEAKER_MCCOY, TX_LOV1_015);
		walkCrewman(OBJECT_MCCOY, 0xbf, 0x98, 0);
	} else
		loadActorAnim2(OBJECT_MCCOY, "musehn", -1, -1, 13);
}

void Room::love1MccoyUsedReplicator() {
	if (_roomVar.love.itemInNozzle == OBJECT_INH3) {
		loadActorStandAnim(OBJECT_DISH_IN_CHAMBER);
		loadActorAnim2(OBJECT_CHAMBER, "s3r2d6", 0xb4, 0x75, 7);
		playSoundEffectIndex(SND_DOOR1);
		walkCrewman(OBJECT_MCCOY, 0xbf, 0x98, 0);
	} else {
		showText(TX_SPEAKER_MCCOY, TX_LOV1_019);
		walkCrewman(OBJECT_MCCOY, 0xbf, 0x98, 0);
	}
}

void Room::love1ChamberClosed() {
	loadActorAnim2(OBJECT_CHAMBER, "s3r2d5", 0xb4, 0x75, 8);
	playSoundEffectIndex(SND_DOOR1);
}

void Room::love1ChamberOpened() {
	loadActorAnim(OBJECT_DISH_IN_CHAMBER, "dishes", 0xb4, 0x71, 0);
	showText(TX_SPEAKER_MCCOY, TX_LOV1_018);
	_awayMission->love.chamberHasCure = true;
}

void Room::love1UseAnythingOnChamber() {
	showText(TX_SPEAKER_MCCOY, TX_LOV1_013);
}

void Room::love1UseDishesOnChamber() {
	walkCrewman(OBJECT_KIRK, 0xb3, 0x8c, 5);
}

void Room::love1KirkReachedChamberToPut() {
	if (_awayMission->love.chamberHasDish)
		showText(TX_SPEAKER_MCCOY, TX_LOV1_014); // TODO: test
	else {
		loadActorAnim(OBJECT_CHAMBER, "s3r2d5", 0xb4, 0x75, 1);
		playSoundEffectIndex(SND_DOOR1);
	}
}

void Room::love1ChamberOpenedForDish() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 14);
}

void Room::love1KirkPutDishInChamber() {
	loadActorAnim(OBJECT_DISH_IN_CHAMBER, "dishes", 0xb4, 0x71, 0);
	loseItem(OBJECT_IDISHES);
	_awayMission->love.chamberHasDish = true;
}

void Room::love1UseInsulationOnDistillator() {
	walkCrewman(OBJECT_KIRK, 0xbe, 0xc1, 6);
}

void Room::love1KirkReachedDistillator() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 15);
	playVoc("LD1PROCE");
}

void Room::love1KirkGotPolyberylcarbonate() {
	// Result of using insulation on distillator
	showDescription(TX_LOV1N034);
	if (!_awayMission->love.gotPolyberylcarbonate) {
		_awayMission->love.gotPolyberylcarbonate = true;
		_awayMission->love.missionScore++;
	}

	giveItem(OBJECT_IPBC);
	loseItem(OBJECT_IINSULAT);
}

void Room::love1UseKirkOnFreezer() {
	_roomVar.love.crewmanUsingFreezerRetY = 0xa0;
	_roomVar.love.crewmanUsingFreezerRetX = 0x96;
	_roomVar.love.crewmanUsingDevice = OBJECT_KIRK;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x71, 0x8e, 7);
	_awayMission->disableInput = true;
}

void Room::love1UseRedshirtOnFreezer() {
	_roomVar.love.crewmanUsingFreezerRetY = 0x89;
	_roomVar.love.crewmanUsingFreezerRetX = 0x95;
	_roomVar.love.crewmanUsingDevice = OBJECT_REDSHIRT;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x71, 0x8e, 7);
	_awayMission->disableInput = true;
}

void Room::love1UseSpockOnFreezer() {
	_roomVar.love.crewmanUsingFreezerRetY = 0xa9;
	_roomVar.love.crewmanUsingFreezerRetX = 0x61;
	_roomVar.love.crewmanUsingDevice = OBJECT_SPOCK;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x71, 0x8e, 7);
	_awayMission->disableInput = true;
}

void Room::love1UseMccoyOnFreezer() {
	_roomVar.love.crewmanUsingFreezerRetY = 0x98;
	_roomVar.love.crewmanUsingFreezerRetX = 0xbf;
	_roomVar.love.crewmanUsingDevice = OBJECT_MCCOY;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x71, 0x8e, 7);
	_awayMission->disableInput = true;
}

void Room::love1CrewmanReachedFreezer() {
	Common::String useAnim = getCrewmanAnimFilename(_roomVar.love.crewmanUsingDevice, "usehw");

	loadActorAnim2(_roomVar.love.crewmanUsingDevice, useAnim, -1, -1, 16);
}

void Room::love1CrewmanOpenedOrClosedFreezer() {
	if (_awayMission->love.freezerOpen)
		loadActorAnim(OBJECT_FREEZER, "s3r2d4a", 0x67, 0x8d, 0);
	else
		loadActorAnim(OBJECT_FREEZER, "s3r2d4", 0x67, 0x8d, 0);

	playSoundEffectIndex(SND_DOOR1);
	_awayMission->love.freezerOpen = !_awayMission->love.freezerOpen;

	walkCrewman(_roomVar.love.crewmanUsingDevice, _roomVar.love.crewmanUsingFreezerRetX, _roomVar.love.crewmanUsingFreezerRetY, 0);
	_awayMission->disableInput = false;
}

void Room::love1UseAnythingOnFreezer() {
	walkCrewman(OBJECT_KIRK, 0x6f, 0x8e, 11);
}

void Room::love1ReachedFreezerWithArbitraryItem() {
	loadActorAnim2(OBJECT_KIRK, "kusemw", -1, -1, 17);
}

void Room::love1FinishedUsingArbitraryItemOnFreezer() {
	showDescription(TX_LOV1N008);
}

void Room::love1UseAnythingOnReplicator() {
	walkCrewman(OBJECT_KIRK, 0xb5, 0x8c, 12);
}

void Room::love1ReachedReplicatorWithArbitraryItem() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 18);
}

void Room::love1FinishedUsingArbitraryItemOnReplicator() {
	showText(TX_SPEAKER_MCCOY, TX_LOV1_016);
}

void Room::love1UseAnythingOnDistillator() {
	walkCrewman(OBJECT_KIRK, 0xc2, 0xb2, 13);
}

void Room::love1ReachedDistillatorWithArbitraryItem() {
	loadActorAnim2(OBJECT_KIRK, "kuseme", -1, -1, 19);
}

void Room::love1FinishedUsingArbitraryItemOnDistillator() {
	showText(TX_SPEAKER_MCCOY, TX_LOV1_008);
}

void Room::love1UseKirkOnLadder() {
	_roomVar.love.crewmanUsingDevice = OBJECT_KIRK;
	_awayMission->crewDirectionsAfterWalk[_roomVar.love.crewmanUsingDevice] = DIR_N;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x102, 0x89, 1);
	_awayMission->disableInput = true;
}

void Room::love1UseSpockOnLadder() {
	_roomVar.love.crewmanUsingDevice = OBJECT_SPOCK;
	_awayMission->crewDirectionsAfterWalk[_roomVar.love.crewmanUsingDevice] = DIR_N;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x102, 0x89, 1);
	_awayMission->disableInput = true;
}

void Room::love1UseMccoyOnLadder() {
	_roomVar.love.crewmanUsingDevice = OBJECT_MCCOY;
	_awayMission->crewDirectionsAfterWalk[_roomVar.love.crewmanUsingDevice] = DIR_N;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x102, 0x89, 1);
	_awayMission->disableInput = true;
}

void Room::love1UseRedshirtOnLadder() {
	_roomVar.love.crewmanUsingDevice = OBJECT_REDSHIRT;
	_awayMission->crewDirectionsAfterWalk[_roomVar.love.crewmanUsingDevice] = DIR_N;
	walkCrewman(_roomVar.love.crewmanUsingDevice, 0x102, 0x89, 1);
	_awayMission->disableInput = true;
}

void Room::love1CrewmanReachedLadder() {
	const int32 speakers[] = {
		TX_SPEAKER_KIRK,
		TX_SPEAKER_SPOCK,
		TX_SPEAKER_MCCOY,
		TX_SPEAKER_FERRIS
	};

	if (_awayMission->love.romulansUnconsciousFromLaughingGas || _awayMission->love.romulansUnconsciousFromVirus) // Romulans unconscious
		loadRoomIndex(4, 3);
	else { // Romulans still conscious, they shoot you
		loadActorAnim(OBJECT_PHASERSHOT, "s3r2s2", 0xf3, 0x89, 0);
		playSoundEffectIndex(SND_PHASSHOT);

		Common::String anim = getCrewmanAnimFilename(_roomVar.love.crewmanUsingDevice, "killw");
		loadActorAnim(_roomVar.love.crewmanUsingDevice, anim, 0x102, 0x89, 6);
		_roomVar.love.dyingSpeaker = speakers[_roomVar.love.crewmanUsingDevice];
	}
}

void Room::love1CrewmanDiedFromPhaser() {
	if (_roomVar.love.crewmanUsingDevice == OBJECT_REDSHIRT) {
		_awayMission->disableInput = false;
		showText(TX_SPEAKER_FERRIS, TX_LOV1_024);
		_awayMission->redshirtDead = true;
	} else {
		showText(_roomVar.love.dyingSpeaker, TX_LOV1_025);
		showGameOverMenu();
	}
}

// Romulans fire a "warning shot" when you go to the right part of the room.
void Room::love1TouchedHotspot0() {
	if (_awayMission->love.romulansUnconsciousFromLaughingGas || _awayMission->love.romulansUnconsciousFromVirus)
		return;
	loadActorAnim(OBJECT_PHASERSHOT, "s3r2s1", 0xf3, 0x89, 0);
	playSoundEffectIndex(SND_PHASSHOT);
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_FERRIS, TX_LOV1_028);
}

}
