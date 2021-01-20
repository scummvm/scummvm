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

extern const RoomAction love1ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::love1Tick1 },

	{ {ACTION_WALK, 10, 0, 0}, &Room::love1WalkToDoor3 },
	{ {ACTION_WALK, 0x26, 0, 0}, &Room::love1WalkToDoor3 },
	{ {ACTION_TOUCHED_HOTSPOT, 1, 0, 0}, &Room::love1OpenDoor3 },
	{ {ACTION_FINISHED_WALKING, 15, 0, 0}, &Room::love1ReachedDoor3 },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0}, &Room::love1ReachedDoor3 },

	{ {ACTION_WALK, 8, 0, 0}, &Room::love1WalkToDoor1 },
	{ {ACTION_WALK, 0x27, 0, 0}, &Room::love1WalkToDoor1 },
	{ {ACTION_TOUCHED_HOTSPOT, 2, 0, 0}, &Room::love1OpenDoor1 },
	{ {ACTION_FINISHED_WALKING, 16, 0, 0}, &Room::love1ReachedDoor1 },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0}, &Room::love1ReachedDoor1 },

	{ {ACTION_WALK, 9, 0, 0}, &Room::love1WalkToDoor2 },
	{ {ACTION_WALK, 0x28, 0, 0}, &Room::love1WalkToDoor2 },
	{ {ACTION_TOUCHED_HOTSPOT, 3, 0, 0}, &Room::love1OpenDoor2 },
	{ {ACTION_FINISHED_WALKING, 17, 0, 0}, &Room::love1ReachedDoor2 },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0}, &Room::love1ReachedDoor2 },

	{ {ACTION_LOOK, 0x25, 0, 0}, &Room::love1LookAtLaser },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0}, &Room::love1LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0}, &Room::love1LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0}, &Room::love1LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::love1LookAtRedshirt },
	{ {ACTION_LOOK, 0xff, 0, 0}, &Room::love1LookAnywhere },
	{ {ACTION_LOOK, 15, 0, 0}, &Room::love1LookAtNozzle },
	{ {ACTION_LOOK, 0x24, 0, 0}, &Room::love1LookAtNozzle },
	{ {ACTION_LOOK, 0x20, 0, 0}, &Room::love1LookAtLadder },
	{ {ACTION_LOOK, 9, 0, 0}, &Room::love1LookAtDoor1Or2 },
	{ {ACTION_LOOK, 8, 0, 0}, &Room::love1LookAtDoor1Or2 },
	{ {ACTION_LOOK, 10, 0, 0}, &Room::love1LookAtDoor3 },
	{ {ACTION_LOOK, 0x21, 0, 0}, &Room::love1LookAtDistillator },
	{ {ACTION_LOOK, 14, 0, 0}, &Room::love1LookAtChamber },
	{ {ACTION_LOOK, 12, 0, 0}, &Room::love1LookAtChamber },
	{ {ACTION_LOOK, 0x23, 0, 0}, &Room::love1LookAtReplicator },
	{ {ACTION_LOOK, 11, 0, 0}, &Room::love1LookAtFreezer },
	{ {ACTION_LOOK, 0x22, 0, 0}, &Room::love1LookAtFreezer },
	{ {ACTION_LOOK, OBJECT_IDISHES, 0, 0}, &Room::love1LookAtDishes },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0}, &Room::love1TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0}, &Room::love1TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0}, &Room::love1TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::love1TalkToRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_IDISHES, 0}, &Room::love1UseMTricorderOnDishes },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0}, &Room::love1UseMTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x23, 0}, &Room::love1UseSTricorderOnReplicator },
	{ {ACTION_USE, OBJECT_ISTRICOR, 12, 0}, &Room::love1UseSTricorderOnReplicator },
	{ {ACTION_USE, OBJECT_SPOCK, 0x25, 0}, &Room::love1UseSTricorderOnLaser },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x25, 0}, &Room::love1UseSTricorderOnLaser },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x22, 0}, &Room::love1UseSTricorderOnFreezer },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0}, &Room::love1UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_IDISHES, 0}, &Room::love1UseSTricorderOnDishes },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0}, &Room::love1UseSTricorderOnDistillator },
	{ {ACTION_GET, 11, 0, 0}, &Room::love1GetFreezer },
	{ {ACTION_GET, 0x22, 0, 0}, &Room::love1GetFreezer },
	{ {ACTION_FINISHED_WALKING, 14, 0, 0}, &Room::love1KirkReachedFreezer },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0}, &Room::love1KirkGotVirusCulture },
	{ {ACTION_GET, 14, 0, 0}, &Room::love1GetFromChamber },
	{ {ACTION_GET, 12, 0, 0}, &Room::love1GetFromChamber },
	{ {ACTION_GET, 0x23, 0, 0}, &Room::love1GetFromChamber },
	{ {ACTION_FINISHED_WALKING, 8, 0, 0}, &Room::love1KirkReachedChamber },
	{ {ACTION_FINISHED_ANIMATION, 10, 0, 0}, &Room::love1KirkGotCureSample },
	{ {ACTION_GET, 15, 0, 0}, &Room::love1GetFromNozzle },
	{ {ACTION_GET, 0x24, 0, 0}, &Room::love1GetFromNozzle },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0}, &Room::love1KirkReachedNozzleToGet },
	{ {ACTION_FINISHED_ANIMATION, 11, 0, 0}, &Room::love1KirkGotBottleFromNozzle },
	{ {ACTION_USE, OBJECT_IN2O, 0x24, 0}, &Room::love1UseN2OOnNozzle },
	{ {ACTION_USE, OBJECT_IH2O, 0x24, 0}, &Room::love1UseH2OOnNozzle },
	{ {ACTION_USE, OBJECT_INH3, 0x24, 0}, &Room::love1UseNH3OnNozzle },
	{ {ACTION_USE, OBJECT_IRLG, 0x24, 0}, &Room::love1UseRLGOnNozzle },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0}, &Room::love1KirkReachedNozzleToPut },
	{ {ACTION_FINISHED_ANIMATION, 12, 0, 0}, &Room::love1KirkPutBottleInNozzle },
	{ {ACTION_USE, 0xff, 0x24, 0}, &Room::love1UseAnthingOnNozzle },
	{ {ACTION_USE, OBJECT_SPOCK, 0x23, 0}, &Room::love1UseSpockOnReplicator },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x23, 0}, &Room::love1UseRedshirtOnReplicator },
	{ {ACTION_USE, OBJECT_MCCOY, 14, 0}, &Room::love1UseMccoyOnReplicator },
	{ {ACTION_USE, OBJECT_MCCOY, 15, 0}, &Room::love1UseMccoyOnReplicator },
	{ {ACTION_USE, OBJECT_MCCOY, 0x23, 0}, &Room::love1UseMccoyOnReplicator },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0}, &Room::love1MccoyReachedReplicator },
	{ {ACTION_FINISHED_ANIMATION, 13, 0, 0}, &Room::love1MccoyUsedReplicator },
	{ {ACTION_FINISHED_ANIMATION, 7, 0, 0}, &Room::love1ChamberClosed },
	{ {ACTION_FINISHED_ANIMATION, 8, 0, 0}, &Room::love1ChamberOpened },
	{ {ACTION_USE, 0xff, 12, 0}, &Room::love1UseAnythingOnChamber },
	{ {ACTION_USE, OBJECT_IDISHES, 12, 0}, &Room::love1UseDishesOnChamber },
	{ {ACTION_USE, OBJECT_IDISHES, 0x23, 0}, &Room::love1UseDishesOnChamber },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0}, &Room::love1KirkReachedChamberToPut },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0}, &Room::love1ChamberOpenedForDish },
	{ {ACTION_FINISHED_ANIMATION, 14, 0, 0}, &Room::love1KirkPutDishInChamber },
	{ {ACTION_USE, OBJECT_IINSULAT, 0x21, 0}, &Room::love1UseInsulationOnDistillator },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0}, &Room::love1KirkReachedDistillator },
	{ {ACTION_FINISHED_ANIMATION, 15, 0, 0}, &Room::love1KirkGotPolyberylcarbonate },
	{ {ACTION_USE, OBJECT_KIRK, 0x22, 0}, &Room::love1UseKirkOnFreezer },
	{ {ACTION_USE, OBJECT_KIRK, 11, 0}, &Room::love1UseKirkOnFreezer },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x22, 0}, &Room::love1UseRedshirtOnFreezer },
	{ {ACTION_USE, OBJECT_REDSHIRT, 11, 0}, &Room::love1UseRedshirtOnFreezer },
	{ {ACTION_USE, OBJECT_SPOCK, 0x22, 0}, &Room::love1UseSpockOnFreezer },
	{ {ACTION_USE, OBJECT_SPOCK, 11, 0}, &Room::love1UseSpockOnFreezer },
	{ {ACTION_USE, OBJECT_MCCOY, 0x22, 0}, &Room::love1UseMccoyOnFreezer },
	{ {ACTION_USE, OBJECT_MCCOY, 11, 0}, &Room::love1UseMccoyOnFreezer },
	{ {ACTION_FINISHED_WALKING, 7, 0, 0}, &Room::love1CrewmanReachedFreezer },
	{ {ACTION_FINISHED_ANIMATION, 16, 0, 0}, &Room::love1CrewmanOpenedOrClosedFreezer },
	{ {ACTION_USE, 0xff, 0x22, 0}, &Room::love1UseAnythingOnFreezer },
	{ {ACTION_FINISHED_WALKING, 11, 0, 0}, &Room::love1ReachedFreezerWithArbitraryItem },
	{ {ACTION_FINISHED_ANIMATION, 17, 0, 0}, &Room::love1FinishedUsingArbitraryItemOnFreezer },
	{ {ACTION_USE, 0xff, 0x23, 0}, &Room::love1UseAnythingOnReplicator },
	{ {ACTION_FINISHED_WALKING, 12, 0, 0}, &Room::love1ReachedReplicatorWithArbitraryItem },
	{ {ACTION_FINISHED_ANIMATION, 18, 0, 0}, &Room::love1FinishedUsingArbitraryItemOnReplicator },
	{ {ACTION_USE, 0xff, 0x21, 0}, &Room::love1UseAnythingOnDistillator },
	{ {ACTION_FINISHED_WALKING, 13, 0, 0}, &Room::love1ReachedDistillatorWithArbitraryItem },
	{ {ACTION_FINISHED_ANIMATION, 19, 0, 0}, &Room::love1FinishedUsingArbitraryItemOnDistillator },
	{ {ACTION_USE, OBJECT_KIRK, 0x20, 0}, &Room::love1UseKirkOnLadder },
	{ {ACTION_USE, OBJECT_SPOCK, 0x20, 0}, &Room::love1UseSpockOnLadder },
	{ {ACTION_USE, OBJECT_MCCOY, 0x20, 0}, &Room::love1UseMccoyOnLadder },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x20, 0}, &Room::love1UseRedshirtOnLadder },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0}, &Room::love1CrewmanReachedLadder },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0}, &Room::love1CrewmanDiedFromPhaser },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0}, &Room::love1TouchedHotspot0 },

	// Common code
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0}, &Room::loveaTimer0Expired },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0}, &Room::loveaTimer1Expired },
	{ {ACTION_USE, OBJECT_IMEDKIT, OBJECT_SPOCK, 0}, &Room::loveaUseMedkitOnSpock },
	{ {ACTION_USE, OBJECT_ISAMPLE, OBJECT_SPOCK, 0}, &Room::loveaUseCureSampleOnSpock },
	{ {ACTION_USE, OBJECT_ICURE, OBJECT_SPOCK, 0}, &Room::loveaUseCureOnSpock },
	{ {ACTION_FINISHED_WALKING,   99, 0, 0}, &Room::loveaSpockOrMccoyInPositionToUseCure },
	{ {ACTION_FINISHED_ANIMATION, 99, 0, 0}, &Room::loveaFinishedCuringSpock },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0}, &Room::loveaTimer2Expired },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK, 0}, &Room::loveaUseMTricorderOnSpock },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IRLG, 0xff, 0}, &Room::loveaUseRomulanLaughingGas },
	{ {ACTION_USE, OBJECT_IN2O, 0xff, 0}, &Room::loveaUseHumanLaughingGas },
	{ {ACTION_USE, OBJECT_INH3, 0xff, 0}, &Room::loveaUseAmmonia },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0}, &Room::loveaUseCommunicator },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

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
		loadActorAnim(OBJECT_BOTTLE, _roomVar.love.bottleAnimation, 0xa3, 0x72, 0);
		break;
	case BOTTLETYPE_NH3:
		strcpy(_roomVar.love.bottleAnimation, "btle2");
		_roomVar.love.itemInNozzle = OBJECT_INH3;
		loadActorAnim(OBJECT_BOTTLE, _roomVar.love.bottleAnimation, 0xa3, 0x72, 0);
		break;
	case BOTTLETYPE_H2O:
		strcpy(_roomVar.love.bottleAnimation, "btle3");
		_roomVar.love.itemInNozzle = OBJECT_IH2O;
		loadActorAnim(OBJECT_BOTTLE, _roomVar.love.bottleAnimation, 0xa3, 0x72, 0);
		break;
	case BOTTLETYPE_RLG:
		strcpy(_roomVar.love.bottleAnimation, "btle4");
		_roomVar.love.itemInNozzle = OBJECT_IRLG;
		loadActorAnim(OBJECT_BOTTLE, _roomVar.love.bottleAnimation, 0xa3, 0x72, 0);
		break;

	case BOTTLETYPE_NONE:
	default:
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
		playSoundEffectIndex(kSfxDoor);
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
		playSoundEffectIndex(kSfxDoor);
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
		playSoundEffectIndex(kSfxDoor);
	}
}

// Door 2 opened, or door 2 reached
void Room::love1ReachedDoor2() {
	_roomVar.love.doorOpenCounter++;
	if (_roomVar.love.doorOpenCounter == 2)
		loadRoomIndex(2, 0);
}

void Room::love1LookAtLaser() {
	showDescription(0, true);
}

void Room::love1LookAtKirk() {
	showDescription(3, true);
}

void Room::love1LookAtSpock() {
	showDescription(4, true);
}

void Room::love1LookAtMccoy() {
	showDescription(1, true);
}

void Room::love1LookAtRedshirt() {
	showDescription(2, true);
}

void Room::love1LookAnywhere() {
	showDescription(19, true);
}

void Room::love1LookAtNozzle() {
	showDescription(22, true);
}

void Room::love1LookAtLadder() {
	showDescription(21, true);
}

void Room::love1LookAtDoor1Or2() {
	showDescription(20, true);
}

void Room::love1LookAtDoor3() {
	showDescription(17, true);
}

void Room::love1LookAtDistillator() {
	showDescription(16, true);
}

void Room::love1LookAtChamber() {
	if (_awayMission->love.chamberHasCure)
		showDescription(13, true);
	else
		showDescription(14, true);
}

void Room::love1LookAtReplicator() {
	showDescription(15, true);
}

void Room::love1LookAtFreezer() {
	if (_awayMission->love.freezerOpen)
		showDescription(23, true);
	else
		showDescription(18, true);
}

void Room::love1LookAtDishes() {
	showDescription(12, true);
}

void Room::love1TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 1, true);
}

void Room::love1TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 5, true);
}

void Room::love1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 11, true);
}

void Room::love1TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, 27, true);
}

void Room::love1UseMTricorderOnDishes() {
	mccoyScan(DIR_W, 17, false, true);
	// TODO: only works in this room, despite being inventory item?
}

void Room::love1UseMTricorderAnywhere() {
	if (_awayMission->love.knowAboutVirus)
		mccoyScan(DIR_S, 9, false, true);
	else
		mccoyScan(DIR_S, 10, false, true);
}

void Room::love1UseSTricorderOnReplicator() {
	spockScan(DIR_N, 22, false, true);
}

void Room::love1UseSTricorderOnLaser() {
	spockScan(DIR_E, 6, false, true);
}

void Room::love1UseSTricorderOnFreezer() {
	spockScan(DIR_W, 23, false, true);
}

void Room::love1UseSTricorderAnywhere() {
	spockScan(DIR_S, 3, false, true);
}

void Room::love1UseSTricorderOnDishes() {
	spockScan(DIR_W, 4, false, true);
	// TODO: only works in this room, despite being inventory item?
}

void Room::love1UseSTricorderOnDistillator() {
	spockScan(DIR_S, 7, false, true);
}

void Room::love1GetFreezer() {
	walkCrewman(OBJECT_KIRK, 0x71, 0x8e, 14);
}

void Room::love1KirkReachedFreezer() {
	if (_awayMission->love.freezerOpen)
		loadActorAnim2(OBJECT_KIRK, "kusehw", -1, -1, 9);
	else
		showDescription(10, true);
}

void Room::love1KirkGotVirusCulture() {
	giveItem(OBJECT_IDISHES);
	showDescription(6, true);
}

void Room::love1GetFromChamber() {
	if (!_awayMission->love.chamberHasDish)
		showDescription(9, true);
	else
		walkCrewman(OBJECT_KIRK, 0xb5, 0x8c, 8);
}

void Room::love1KirkReachedChamber() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 10);
}

void Room::love1KirkGotCureSample() {
	if (_awayMission->love.chamberHasCure) {
		giveItem(OBJECT_ISAMPLE);
		showDescription(35, true);

		// BUGFIX: after removing the cure, unset this variable.
		// Otherwise, any normal dish inserted afterward automagically becomes a cure.
		_awayMission->love.chamberHasCure = false;
	} else {
		giveItem(OBJECT_IDISHES);
		showDescription(6, true);
	}

	loadActorStandAnim(OBJECT_DISH_IN_CHAMBER);
	loadActorAnim2(OBJECT_CHAMBER, "s3r2d6", 0xb4, 0x75, 0);
	playSoundEffectIndex(kSfxDoor);
	_awayMission->love.chamberHasDish = false;
}

void Room::love1GetFromNozzle() {
	walkCrewman(OBJECT_KIRK, 0xa6, 0x90, 2);
}

void Room::love1KirkReachedNozzleToGet() {
	if (_awayMission->love.bottleInNozzle == 0)
		showDescription(11, true);
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
	showDescription(7, true);
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
	showText(TX_SPEAKER_MCCOY, 12, true);
}

void Room::love1UseSpockOnReplicator() {
	showText(TX_SPEAKER_SPOCK, 2, true);
}

void Room::love1UseRedshirtOnReplicator() {
	showText(TX_SPEAKER_FERRIS, 26, true);
}

void Room::love1UseMccoyOnReplicator() {
	walkCrewman(OBJECT_MCCOY, 0xb1, 0x8c, 4);
}

void Room::love1MccoyReachedReplicator() {
	if (_awayMission->love.bottleInNozzle == BOTTLETYPE_NONE) {
		showText(TX_SPEAKER_MCCOY, 21, true);
		walkCrewman(OBJECT_MCCOY, 0xbf, 0x98, 0);
	} else if (!_awayMission->love.chamberHasDish) {
		showText(TX_SPEAKER_MCCOY, 15, true);
		walkCrewman(OBJECT_MCCOY, 0xbf, 0x98, 0);
	} else
		loadActorAnim2(OBJECT_MCCOY, "musehn", -1, -1, 13);
}

void Room::love1MccoyUsedReplicator() {
	if (_roomVar.love.itemInNozzle == OBJECT_INH3) {
		loadActorStandAnim(OBJECT_DISH_IN_CHAMBER);
		loadActorAnim2(OBJECT_CHAMBER, "s3r2d6", 0xb4, 0x75, 7);
		playSoundEffectIndex(kSfxDoor);
		walkCrewman(OBJECT_MCCOY, 0xbf, 0x98, 0);
	} else {
		showText(TX_SPEAKER_MCCOY, 19, true);
		walkCrewman(OBJECT_MCCOY, 0xbf, 0x98, 0);
	}
}

void Room::love1ChamberClosed() {
	loadActorAnim2(OBJECT_CHAMBER, "s3r2d5", 0xb4, 0x75, 8);
	playSoundEffectIndex(kSfxDoor);
}

void Room::love1ChamberOpened() {
	loadActorAnim(OBJECT_DISH_IN_CHAMBER, "dishes", 0xb4, 0x71, 0);
	showText(TX_SPEAKER_MCCOY, 18, true);
	_awayMission->love.chamberHasCure = true;
}

void Room::love1UseAnythingOnChamber() {
	showText(TX_SPEAKER_MCCOY, 13, true);
}

void Room::love1UseDishesOnChamber() {
	walkCrewman(OBJECT_KIRK, 0xb3, 0x8c, 5);
}

void Room::love1KirkReachedChamberToPut() {
	if (_awayMission->love.chamberHasDish)
		showText(TX_SPEAKER_MCCOY, 14, true); // TODO: test
	else {
		loadActorAnim(OBJECT_CHAMBER, "s3r2d5", 0xb4, 0x75, 1);
		playSoundEffectIndex(kSfxDoor);
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
	showDescription(34, true);
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

	playSoundEffectIndex(kSfxDoor);
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
	showDescription(8, true);
}

void Room::love1UseAnythingOnReplicator() {
	walkCrewman(OBJECT_KIRK, 0xb5, 0x8c, 12);
}

void Room::love1ReachedReplicatorWithArbitraryItem() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 18);
}

void Room::love1FinishedUsingArbitraryItemOnReplicator() {
	showText(TX_SPEAKER_MCCOY, 16, true);
}

void Room::love1UseAnythingOnDistillator() {
	walkCrewman(OBJECT_KIRK, 0xc2, 0xb2, 13);
}

void Room::love1ReachedDistillatorWithArbitraryItem() {
	loadActorAnim2(OBJECT_KIRK, "kuseme", -1, -1, 19);
}

void Room::love1FinishedUsingArbitraryItemOnDistillator() {
	showText(TX_SPEAKER_MCCOY, 8, true);
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
		playSoundEffectIndex(kSfxPhaser);

		Common::String anim = getCrewmanAnimFilename(_roomVar.love.crewmanUsingDevice, "killw");
		loadActorAnim(_roomVar.love.crewmanUsingDevice, anim, 0x102, 0x89, 6);
		_roomVar.love.dyingSpeaker = speakers[_roomVar.love.crewmanUsingDevice];
	}
}

void Room::love1CrewmanDiedFromPhaser() {
	if (_roomVar.love.crewmanUsingDevice == OBJECT_REDSHIRT) {
		_awayMission->disableInput = false;
		showText(TX_SPEAKER_FERRIS, 24, true);
		_awayMission->redshirtDead = true;
	} else {
		showText(_roomVar.love.dyingSpeaker, 25, true);
		showGameOverMenu();
	}
}

// Romulans fire a "warning shot" when you go to the right part of the room.
void Room::love1TouchedHotspot0() {
	if (_awayMission->love.romulansUnconsciousFromLaughingGas || _awayMission->love.romulansUnconsciousFromVirus)
		return;
	loadActorAnim(OBJECT_PHASERSHOT, "s3r2s1", 0xf3, 0x89, 0);
	playSoundEffectIndex(kSfxPhaser);
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_FERRIS, 28, true);
}

}
