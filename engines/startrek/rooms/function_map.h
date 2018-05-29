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

#ifndef STARTREK_FUNCTIONMAP_H
#define STARTREK_FUNCTIONMAP_H

#include "startrek/action.h"
#include "startrek/room.h"

namespace StarTrek {

RoomAction demon0ActionList[] = {
	{ Action(ACTION_TICK, 1, 0, 0), &Room::demon0Tick1 },
	{ Action(ACTION_TICK, 2, 0, 0), &Room::demon0Tick2 },
	{ Action(ACTION_TICK, 60, 0, 0), &Room::demon0Tick60 },
	{ Action(ACTION_TICK, 100, 0, 0), &Room::demon0Tick100 },
	{ Action(ACTION_TICK, 140, 0, 0), &Room::demon0Tick140 },

	{ Action(ACTION_TOUCHED_WARP, 0, 0, 0), &Room::demon0TouchedWarp0 },

	{ Action(ACTION_WALK, 0x27, 0, 0), &Room::demon0WalkToBottomDoor },
	{ Action(ACTION_WALK, 9, 0, 0), &Room::demon0WalkToBottomDoor },
	{ Action(ACTION_TOUCHED_HOTSPOT, 1, 0, 0), &Room::demon0TouchedHotspot1 },
	{ Action(ACTION_FINISHED_WALKING, 1, 0, 0), &Room::demon0ReachedBottomDoor },
	{ Action(ACTION_FINISHED_ANIMATION, 1, 0, 0), &Room::demon0ReachedBottomDoor },

	{ Action(ACTION_WALK, 0x26, 0, 0), &Room::demon0WalkToTopDoor },
	{ Action(ACTION_WALK, 10, 0, 0), &Room::demon0WalkToTopDoor },
	{ Action(ACTION_TOUCHED_HOTSPOT, 0, 0, 0), &Room::demon0TouchedHotspot0 },
	{ Action(ACTION_FINISHED_WALKING, 2, 0, 0), &Room::demon0ReachedTopDoor },
	{ Action(ACTION_FINISHED_ANIMATION, 2, 0, 0), &Room::demon0ReachedTopDoor },

	{ Action(ACTION_TALK, 8, 0, 0), &Room::demon0TalkToPrelate },
	{ Action(ACTION_LOOK, 8, 0, 0), &Room::demon0LookAtPrelate },

	{ Action(ACTION_USE, OBJECT_IPHASERS, 0x23, 0), &Room::demon0UsePhaserOnSnow },
	{ Action(ACTION_USE, OBJECT_IPHASERK, 0x23, 0), &Room::demon0UsePhaserOnSnow },
	{ Action(ACTION_USE, OBJECT_IPHASERS, 0x25, 0), &Room::demon0UsePhaserOnSign },
	{ Action(ACTION_USE, OBJECT_IPHASERK, 0x25, 0), &Room::demon0UsePhaserOnSign },
	{ Action(ACTION_USE, OBJECT_IPHASERK, 0x20, 0), &Room::demon0UsePhaserOnShelter },
	{ Action(ACTION_USE, OBJECT_IPHASERK, 8, 0), &Room::demon0UsePhaserOnPrelate },

	{ Action(ACTION_LOOK, 0x25, 0, 0), &Room::demon0LookAtSign },
	{ Action(ACTION_LOOK, 0x21, 0, 0), &Room::demon0LookAtTrees },
	{ Action(ACTION_LOOK, 0x22, 0, 0), &Room::demon0LookAtTrees },
	{ Action(ACTION_LOOK, 0x23, 0, 0), &Room::demon0LookAtSnow },
	{ Action(ACTION_LOOK, 0xff, 0, 0), &Room::demon0LookAnywhere },
	{ Action(ACTION_LOOK, 0x24, 0, 0), &Room::demon0LookAtBushes },

	{ Action(ACTION_LOOK, OBJECT_KIRK,     0, 0), &Room::demon0LookAtKirk },
	{ Action(ACTION_LOOK, OBJECT_MCCOY,    0, 0), &Room::demon0LookAtMcCoy },
	{ Action(ACTION_LOOK, OBJECT_REDSHIRT, 0, 0), &Room::demon0LookAtRedShirt },
	{ Action(ACTION_LOOK, OBJECT_SPOCK,    0, 0), &Room::demon0LookAtSpock },

	{ Action(ACTION_LOOK, 9, 0, 0), &Room::demon0LookAtShelter },    // Door 1
	{ Action(ACTION_LOOK, 10, 0, 0), &Room::demon0LookAtShelter },   // Door 2
	{ Action(ACTION_LOOK, 0x20, 0, 0), &Room::demon0LookAtShelter }, // Shelter itself

	{ Action(ACTION_TALK, OBJECT_KIRK,     0, 0), &Room::demon0TalkToKirk },
	{ Action(ACTION_TALK, OBJECT_REDSHIRT, 0, 0), &Room::demon0TalkToRedshirt },
	{ Action(ACTION_TALK, OBJECT_MCCOY,    0, 0), &Room::demon0TalkToMcCoy },
	{ Action(ACTION_TALK, OBJECT_SPOCK,    0, 0), &Room::demon0TalkToSpock },

	{ Action(ACTION_USE, OBJECT_ISTRICOR, -1, 0), &Room::demon0UseSTricorderAnywhere },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, -1, 0), &Room::demon0UseMTricorderAnywhere },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 8, 0), &Room::demon0UseMTricorderOnPrelate },
};

RoomAction demon1ActionList[] = {
	{ Action(ACTION_TICK, 1, 0, 0), &Room::demon1Tick1 },

	{ Action(ACTION_WALK, 0x25, 0, 0), &Room::demon1WalkToCave },
	{ Action(ACTION_FINISHED_WALKING, 2, 0, 0), &Room::demon1TouchedTopWarp },
	{ Action(ACTION_TOUCHED_WARP, 0, 0, 0), &Room::demon1TouchedTopWarp },
	{ Action(ACTION_TOUCHED_WARP, 1, 0, 0), &Room::demon1TouchedBottomWarp },

	{ Action(ACTION_TIMER_EXPIRED, 2, 0, 0), &Room::demon1Timer2Expired },
	{ Action(ACTION_TIMER_EXPIRED, 0, 0, 0), &Room::demon1Timer0Expired },
	{ Action(ACTION_TIMER_EXPIRED, 3, 0, 0), &Room::demon1Timer3Expired },
	{ Action(ACTION_TIMER_EXPIRED, 1, 0, 0), &Room::demon1Timer1Expired },

	{ Action(ACTION_FINISHED_ANIMATION, 2, 0, 0), &Room::demon1KlingonFinishedAimingWeapon },
	{ Action(ACTION_FINISHED_ANIMATION, 1, 0, 0), &Room::demon1KirkShot },

	{ Action(ACTION_USE, OBJECT_IPHASERK, -1, 0), &Room::demon1UsePhaserOnAnything },
	{ Action(ACTION_USE, OBJECT_IPHASERS, -1, 0), &Room::demon1UsePhaserOnAnything },

	{ Action(ACTION_USE, OBJECT_IPHASERK, 8, 0), &Room::demon1UsePhaserOnKlingon1 },
	{ Action(ACTION_USE, OBJECT_IPHASERS, 8, 0), &Room::demon1UsePhaserOnKlingon1 },
	{ Action(ACTION_FINISHED_ANIMATION, 3, 0, 0), &Room::demon1ShootKlingon1 },
	{ Action(ACTION_FINISHED_ANIMATION, 7, 0, 0), &Room::demon1KlingonDropsHand },

	{ Action(ACTION_USE, OBJECT_IPHASERK, 9, 0), &Room::demon1UsePhaserOnKlingon2 },
	{ Action(ACTION_USE, OBJECT_IPHASERS, 9, 0), &Room::demon1UsePhaserOnKlingon2 },
	{ Action(ACTION_FINISHED_ANIMATION, 4, 0, 0), &Room::demon1ShootKlingon2 },

	{ Action(ACTION_USE, OBJECT_IPHASERK, 10, 0), &Room::demon1UsePhaserOnKlingon3 },
	{ Action(ACTION_USE, OBJECT_IPHASERS, 10, 0), &Room::demon1UsePhaserOnKlingon3 },
	{ Action(ACTION_FINISHED_ANIMATION, 5, 0, 0), &Room::demon1ShootKlingon3 },

	{ Action(ACTION_TIMER_EXPIRED, 6, 0, 0), &Room::demon1AllKlingonsDead },
	{ Action(ACTION_TIMER_EXPIRED, 5, 0, 0), &Room::demon1Timer5Expired },

	{ Action(ACTION_USE, OBJECT_IMTRICOR, 13, 0), &Room::demon1UseMTricorderOnKlingon },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x21, 0), &Room::demon1UseSTricorderOnTulips },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x22, 0), &Room::demon1UseSTricorderOnPods },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x20, 0), &Room::demon1UseSTricorderOnCattails },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x23, 0), &Room::demon1UseSTricorderOnFerns },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 13, 0), &Room::demon1UseSTricorderOnHand },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 8, 0), &Room::demon1UseSTricorderOnKlingon1 },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 10, 0), &Room::demon1UseSTricorderOnKlingon2Or3 },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 9, 0), &Room::demon1UseSTricorderOnKlingon2Or3 },
	{ Action(ACTION_USE, OBJECT_IMEDKIT,  8, 0), &Room::demon1UseMTricorderOnKlingon },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 8, 0), &Room::demon1UseMTricorderOnKlingon },
	{ Action(ACTION_USE, OBJECT_IMEDKIT, 10, 0), &Room::demon1UseMTricorderOnKlingon },
	{ Action(ACTION_USE, OBJECT_IMEDKIT,  9, 0), &Room::demon1UseMTricorderOnKlingon },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 10, 0), &Room::demon1UseMTricorderOnKlingon },
	{ Action(ACTION_USE, OBJECT_IMTRICOR,  9, 0), &Room::demon1UseMTricorderOnKlingon },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 8, 0), &Room::demon1UseSTricorderOnKlingon2Or3 }, // This is redundant

	{ Action(ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK, 0), &Room::demon1UseMTricorderOnKirk },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK, 0), &Room::demon1UseMTricorderOnSpock },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0), &Room::demon1UseMTricorderOnRedshirt },

	{ Action(ACTION_GET, 13, 0, 0), &Room::demon1GetHand },
	{ Action(ACTION_FINISHED_WALKING, 1, 0, 0), &Room::demon1ReachedHand },
	{ Action(ACTION_FINISHED_ANIMATION, 6, 0, 0), &Room::demon1PickedUpHand },
	{ Action(ACTION_TIMER_EXPIRED, 4, 0, 0), &Room::demon1FinishedGettingHand },

	{ Action(ACTION_LOOK, 8, 0, 0), &Room::demon1LookAtKlingon },
	{ Action(ACTION_LOOK, 9, 0, 0), &Room::demon1LookAtKlingon },
	{ Action(ACTION_LOOK, 10, 0, 0), &Room::demon1LookAtKlingon },

	{ Action(ACTION_LOOK, 0x20, 0, 0), &Room::demon1LookAtCattails },
	{ Action(ACTION_LOOK, 0x21, 0, 0), &Room::demon1LookAtTulips },
	{ Action(ACTION_LOOK, 0x22, 0, 0), &Room::demon1LookAtPods },
	{ Action(ACTION_LOOK, 0x23, 0, 0), &Room::demon1LookAtFerns },
	{ Action(ACTION_LOOK, 0x24, 0, 0), &Room::demon1LookAtStream },
	{ Action(ACTION_LOOK, 0x25, 0, 0), &Room::demon1LookAtMine },
	{ Action(ACTION_LOOK, 0x26, 0, 0), &Room::demon1LookAtMountain },
	{ Action(ACTION_LOOK, 13, 0, 0), &Room::demon1LookAtHand },
	{ Action(ACTION_LOOK, -1, 0, 0), &Room::demon1LookAnywhere },
	{ Action(ACTION_LOOK, OBJECT_KIRK, 0, 0), &Room::demon1LookAtKirk },
	{ Action(ACTION_LOOK, OBJECT_SPOCK, 0, 0), &Room::demon1LookAtSpock },
	{ Action(ACTION_LOOK, OBJECT_MCCOY, 0, 0), &Room::demon1LookAtMcCoy },
	{ Action(ACTION_LOOK, OBJECT_REDSHIRT, 0, 0), &Room::demon1LookAtRedshirt },

	{ Action(ACTION_TALK, OBJECT_KIRK, 0, 0), &Room::demon1TalkToKirk },
	{ Action(ACTION_TALK, OBJECT_SPOCK, 0, 0), &Room::demon1TalkToSpock },
	{ Action(ACTION_TALK, OBJECT_MCCOY, 0, 0), &Room::demon1TalkToMcCoy },
	{ Action(ACTION_TALK, OBJECT_REDSHIRT, 0, 0), &Room::demon1TalkToRedshirt },
};

RoomAction demon2ActionList[] = {
	{ Action(ACTION_TICK, 1, 0, 0), &Room::demon2Tick1 },
	{ Action(ACTION_WALK, 0x27, 0, 0), &Room::demon2WalkToCave },
	{ Action(ACTION_FINISHED_WALKING, 2, 0, 0), &Room::demon2ReachedCave },
	{ Action(ACTION_TOUCHED_WARP, 1, 0, 0), &Room::demon2TouchedWarp1 },

	{ Action(ACTION_LOOK, 0x27, 0, 0), &Room::demon2LookAtCave },
	{ Action(ACTION_LOOK, 0x20, 0, 0), &Room::demon2LookAtCave },
	{ Action(ACTION_LOOK, 0x21, 0, 0), &Room::demon2LookAtMountain },
	{ Action(ACTION_LOOK, 0x22, 0, 0), &Room::demon2LookAtMountain },
	{ Action(ACTION_LOOK, 0x23, 0, 0), &Room::demon2LookAtBerries },
	{ Action(ACTION_LOOK, 0x24, 0, 0), &Room::demon2LookAtFern },
	{ Action(ACTION_LOOK, 0x25, 0, 0), &Room::demon2LookAtMoss },
	{ Action(ACTION_LOOK, 0x26, 0, 0), &Room::demon2LookAtLights },
	{ Action(ACTION_LOOK, -1, 0, 0),   &Room::demon2LookAtAnything },

	{ Action(ACTION_LOOK, OBJECT_KIRK, 0, 0), &Room::demon2LookAtKirk },
	{ Action(ACTION_LOOK, OBJECT_SPOCK, 0, 0), &Room::demon2LookAtSpock },
	{ Action(ACTION_LOOK, OBJECT_MCCOY, 0, 0), &Room::demon2LookAtMcCoy },
	{ Action(ACTION_LOOK, OBJECT_REDSHIRT, 0, 0), &Room::demon2LookAtRedshirt },

	{ Action(ACTION_TALK, OBJECT_KIRK, 0, 0), &Room::demon2TalkToKirk },
	{ Action(ACTION_TALK, OBJECT_SPOCK, 0, 0), &Room::demon2TalkToSpock },
	{ Action(ACTION_TALK, OBJECT_MCCOY, 0, 0), &Room::demon2TalkToMcCoy },
	{ Action(ACTION_TALK, OBJECT_REDSHIRT, 0, 0), &Room::demon2TalkToRedshirt },

	{ Action(ACTION_USE, OBJECT_IPHASERK, 0x23, 0), &Room::demon2UsePhaserOnBerries },
	{ Action(ACTION_USE, OBJECT_IPHASERS, 0x23, 0), &Room::demon2UsePhaserOnBerries },

	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x23, 0), &Room::demon2UseSTricorderOnBerries },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x25, 0), &Room::demon2UseSTricorderOnMoss },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x24, 0), &Room::demon2UseSTricorderOnFern },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x20, 0), &Room::demon2UseSTricorderOnCave },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x21, 0), &Room::demon2UseSTricorderOnCave },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x22, 0), &Room::demon2UseSTricorderOnCave },

	{ Action(ACTION_USE, OBJECT_IMTRICOR, 0x23, 0), &Room::demon2UseMTricorderOnBerries },
	{ Action(ACTION_GET, 0x23, 0, 0), &Room::demon2GetBerries },
	{ Action(ACTION_FINISHED_WALKING, 1, 0, 0), &Room::demon2ReachedBerries },
	{ Action(ACTION_FINISHED_ANIMATION, 1, 0, 0), &Room::demon2PickedUpBerries },
};

RoomAction demon3ActionList[] = {
	{ Action(ACTION_TICK, 1, 0, 0), &Room::demon3Tick1 },
	{ Action(ACTION_TIMER_EXPIRED, 0, 0, 0), &Room::demon3Timer0Expired },
	{ Action(ACTION_TIMER_EXPIRED, 1, 0, 0), &Room::demon3Timer1Expired },
	{ Action(ACTION_TIMER_EXPIRED, 3, 0, 0), &Room::demon3Timer3Expired },

	{ Action(ACTION_FINISHED_ANIMATION, 1, 0, 0), &Room::demon3FinishedAnimation1 },
	{ Action(ACTION_FINISHED_ANIMATION, 2, 0, 0), &Room::demon3FinishedAnimation2 },
	{ Action(ACTION_FINISHED_WALKING, 5, 0, 0), &Room::demon3FinishedWalking5 },

	{ Action(ACTION_FINISHED_WALKING, 3, 0, 0), &Room::demon3McCoyInFiringPosition },
	{ Action(ACTION_FINISHED_WALKING, 2, 0, 0), &Room::demon3SpockInFiringPosition },
	{ Action(ACTION_FINISHED_WALKING, 4, 0, 0), &Room::demon3RedShirtInFiringPosition },
	{ Action(ACTION_FINISHED_WALKING, 1, 0, 0), &Room::demon3KirkInFiringPosition },

	{ Action(ACTION_FINISHED_ANIMATION, 4, 0, 0), &Room::demon3FireAtBoulder },

	{ Action(ACTION_USE, OBJECT_IPHASERS, OBJECT_REDSHIRT, 0), &Room::demon3UsePhaserOnRedshirt },
	{ Action(ACTION_USE, OBJECT_IPHASERK, OBJECT_REDSHIRT, 0), &Room::demon3UsePhaserOnRedshirt },

	{ Action(ACTION_USE, OBJECT_IPHASERS,  9, 0), &Room::demon3UseStunPhaserOnBoulder },
	{ Action(ACTION_USE, OBJECT_IPHASERS, 10, 0), &Room::demon3UseStunPhaserOnBoulder },
	{ Action(ACTION_USE, OBJECT_IPHASERS, 11, 0), &Room::demon3UseStunPhaserOnBoulder },
	{ Action(ACTION_USE, OBJECT_IPHASERS, 12, 0), &Room::demon3UseStunPhaserOnBoulder },

	{ Action(ACTION_USE, OBJECT_IPHASERK,  9, 0), &Room::demon3UsePhaserOnBoulder1 },
	{ Action(ACTION_USE, OBJECT_IPHASERK, 10, 0), &Room::demon3UsePhaserOnBoulder2 },
	{ Action(ACTION_USE, OBJECT_IPHASERK, 11, 0), &Room::demon3UsePhaserOnBoulder3 },
	{ Action(ACTION_USE, OBJECT_IPHASERK, 12, 0), &Room::demon3UsePhaserOnBoulder4 },

	{ Action(ACTION_USE, OBJECT_ISTRICOR, 13, 0), &Room::demon3UseSTricorderOnMiner },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x20, 0), &Room::demon3UseSTricorderOnPanel },
	{ Action(ACTION_USE, OBJECT_ISTRICOR,  9, 0), &Room::demon3UseSTricorderOnBoulder },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 10, 0), &Room::demon3UseSTricorderOnBoulder },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 11, 0), &Room::demon3UseSTricorderOnBoulder },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, 12, 0), &Room::demon3UseSTricorderOnBoulder },
	{ Action(ACTION_USE, OBJECT_IMTRICOR,  9, 0), &Room::demon3UseMTricorderOnBoulder },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 10, 0), &Room::demon3UseMTricorderOnBoulder },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 11, 0), &Room::demon3UseMTricorderOnBoulder },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 12, 0), &Room::demon3UseMTricorderOnBoulder },

	{ Action(ACTION_USE, OBJECT_KIRK, 0x20, 0), &Room::demon3UseCrewmanOnPanel },
	{ Action(ACTION_USE, OBJECT_SPOCK, 0x20, 0), &Room::demon3UseCrewmanOnPanel },
	{ Action(ACTION_USE, OBJECT_MCCOY, 0x20, 0), &Room::demon3UseCrewmanOnPanel },
	{ Action(ACTION_USE, OBJECT_REDSHIRT, 0x20, 0), &Room::demon3UseRedshirtOnPanel },
	{ Action(ACTION_FINISHED_WALKING, 8, 0, 0), &Room::demon3RedshirtReachedPanel },
	{ Action(ACTION_FINISHED_ANIMATION, 5, 0, 0), &Room::demon3RedshirtUsedPanel },
	{ Action(ACTION_FINISHED_ANIMATION, 3, 0, 0), &Room::demon3RedshirtElectrocuted },

	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x21, 0), &Room::demon3UseSTricorderOnDoor },
	{ Action(ACTION_USE, OBJECT_ISTRICOR, -1, 0), &Room::demon3UseSTricorderOnAnything },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 14, 0), &Room::demon3UseMTricorderOnDoor },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 0x21, 0), &Room::demon3UseMTricorderOnDoor },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 0x22, 0), &Room::demon3UseMTricorderOnDoor },
	{ Action(ACTION_USE, OBJECT_IPHASERS, 14, 0), &Room::demon3UsePhaserOnDoor },
	{ Action(ACTION_USE, OBJECT_IPHASERK, 14, 0), &Room::demon3UsePhaserOnDoor },
	{ Action(ACTION_USE, OBJECT_IPHASERS, 0x21, 0), &Room::demon3UsePhaserOnDoor },
	{ Action(ACTION_USE, OBJECT_IPHASERK, 0x21, 0), &Room::demon3UsePhaserOnDoor },

	{ Action(ACTION_USE, OBJECT_IHAND, 0x20, 0), &Room::demon3UseHandOnPanel },
	{ Action(ACTION_FINISHED_WALKING, 7, 0, 0), &Room::demon3KirkReachedHandPanel },
	{ Action(ACTION_FINISHED_ANIMATION, 6, 0, 0), &Room::demon3KirkUsedHandPanel },

	{ Action(ACTION_USE, OBJECT_IMTRICOR, 13, 0), &Room::demon3UseMTricorderOnMiner },
	{ Action(ACTION_USE, OBJECT_MCCOY, 13, 0), &Room::demon3UseMedkitOnMiner },
	{ Action(ACTION_USE, OBJECT_IMEDKIT, 13, 0), &Room::demon3UseMedkitOnMiner },
	{ Action(ACTION_FINISHED_WALKING, 6, 0, 0), &Room::demon3McCoyReachedMiner },
	{ Action(ACTION_FINISHED_ANIMATION, 7, 0, 0), &Room::demon3McCoyFinishedHealingMiner },

	{ Action(ACTION_GET, 13, 0, 0), &Room::demon3GetMiner },
	{ Action(ACTION_TALK, 13, 0, 0), &Room::demon3TalkToMiner },

	{ Action(ACTION_TALK, OBJECT_KIRK, 0, 0), &Room::demon3TalkToKirk },
	{ Action(ACTION_TALK, OBJECT_SPOCK, 0, 0), &Room::demon3TalkToSpock },
	{ Action(ACTION_TALK, OBJECT_MCCOY, 0, 0), &Room::demon3TalkToMccoy },
	{ Action(ACTION_TALK, OBJECT_REDSHIRT, 0, 0), &Room::demon3TalkToRedshirt },

	{ Action(ACTION_LOOK, OBJECT_KIRK, 0, 0), &Room::demon3LookAtKirk },
	{ Action(ACTION_LOOK, OBJECT_SPOCK, 0, 0), &Room::demon3LookAtSpock },
	{ Action(ACTION_LOOK, OBJECT_MCCOY, 0, 0), &Room::demon3LookAtMccoy },
	{ Action(ACTION_LOOK, OBJECT_REDSHIRT, 0, 0), &Room::demon3LookAtRedshirt },

	{ Action(ACTION_LOOK, -1, 0, 0), &Room::demon3LookAnywhere },
	{ Action(ACTION_LOOK, 13, 0, 0), &Room::demon3LookAtMiner },
	{ Action(ACTION_LOOK,  9, 0, 0), &Room::demon3LookAtBoulder1 },
	{ Action(ACTION_LOOK, 10, 0, 0), &Room::demon3LookAtBoulder2 },
	{ Action(ACTION_LOOK, 11, 0, 0), &Room::demon3LookAtBoulder3 },
	{ Action(ACTION_LOOK, 12, 0, 0), &Room::demon3LookAtBoulder4 },
	{ Action(ACTION_LOOK, 0x22, 0, 0), &Room::demon3LookAtStructure },
	{ Action(ACTION_LOOK, 14, 0, 0), &Room::demon3LookAtDoor },
	{ Action(ACTION_LOOK, 0x21, 0, 0), &Room::demon3LookAtDoor },
	{ Action(ACTION_LOOK, 15, 0, 0), &Room::demon3LookAtPanel },
	{ Action(ACTION_LOOK, 0x20, 0, 0), &Room::demon3LookAtPanel },
	{ Action(ACTION_LOOK, 8, 0, 0), &Room::demon3LookAtLight },

};

RoomAction demon4ActionList[] = {
	{ Action(ACTION_TICK, 1, 0, 0), &Room::demon4Tick1 },
	{ Action(ACTION_FINISHED_ANIMATION, 1, 0, 0), &Room::demon4FinishedAnimation1 },
	{ Action(ACTION_FINISHED_ANIMATION, 2, 0, 0), &Room::demon4FinishedAnimation2 },
	{ Action(ACTION_FINISHED_ANIMATION, 3, 0, 0), &Room::demon4FinishedAnimation3 },

	// TODO: everything

	{ Action(ACTION_USE, OBJECT_IPHASERS, 0x21, 0), &Room::demon4UsePhaserOnPanel },
	{ Action(ACTION_USE, OBJECT_IPHASERS, 0x20, 0), &Room::demon4UsePhaserOnPattern },
	{ Action(ACTION_USE, OBJECT_IPHASERS, OBJECT_MCCOY, 0), &Room::demon4UsePhaserOnMccoy },

	// TODO

	{ Action(ACTION_LOOK, 0x20, 0, 0), &Room::demon4LookAtPattern },
	{ Action(ACTION_LOOK, 8, 0, 0), &Room::demon4LookAtAlien },
	{ Action(ACTION_LOOK, -1, 0, 0), &Room::demon4LookAnywhere },
	{ Action(ACTION_LOOK, 0x22, 0, 0), &Room::demon4LookAtSecurityEquipment },
	{ Action(ACTION_LOOK, 0x23, 0, 0), &Room::demon4LookAtFloor },

	{ Action(ACTION_LOOK, OBJECT_KIRK, 0, 0), &Room::demon4LookAtKirk },
	{ Action(ACTION_LOOK, OBJECT_MCCOY, 0, 0), &Room::demon4LookAtMccoy },
	{ Action(ACTION_LOOK, OBJECT_SPOCK, 0, 0), &Room::demon4LookAtSpock },
	{ Action(ACTION_LOOK, OBJECT_REDSHIRT, 0, 0), &Room::demon4LookAtRedshirt },

	{ Action(ACTION_LOOK, 10, 0, 0), &Room::demon4LookAtChamber },
	{ Action(ACTION_LOOK, 0x21, 0, 0), &Room::demon4LookAtPanel },

	{ Action(ACTION_USE, OBJECT_KIRK, 0x21, 0), &Room::demon4UseKirkOnPanel },
	{ Action(ACTION_USE, OBJECT_SPOCK, 0x21, 0), &Room::demon4UseSpockOnPanel },
	{ Action(ACTION_USE, OBJECT_MCCOY, 0x21, 0), &Room::demon4UseMccoyOnPanel },
	{ Action(ACTION_USE, OBJECT_REDSHIRT, 0x21, 0), &Room::demon4UseRedshirtOnPanel },
	{ Action(ACTION_FINISHED_WALKING, 3, 0, 0), &Room::demon4CrewmanReachedPanel },
};

RoomAction demon5ActionList[] = {
	{ Action(ACTION_TICK, 1, 0, 0), &Room::demon5Tick1 },
	{ Action(ACTION_WALK, 0x22, 0, 0), &Room::demon5WalkToDoor },
	{ Action(ACTION_WALK, 12, 0, 0), &Room::demon5WalkToDoor },
	{ Action(ACTION_TOUCHED_HOTSPOT, 0, 0, 0), &Room::demon5TouchedDoorOpenTrigger },
	{ Action(ACTION_FINISHED_WALKING, 1, 0, 0), &Room::demon5DoorOpenedOrReachedDoor },
	{ Action(ACTION_FINISHED_ANIMATION, 1, 0, 0), &Room::demon5DoorOpenedOrReachedDoor },

	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0x21, 0), &Room::demon5UseSTricorderOnCrate },
	{ Action(ACTION_USE, OBJECT_IPHASERK, -1, 0), &Room::demon5UsePhaserOnAnything },
	{ Action(ACTION_USE, OBJECT_IPHASERS, -1, 0), &Room::demon5UsePhaserOnAnything },

	{ Action(ACTION_USE, OBJECT_IHAND, 11, 0), &Room::demon5UseHandOnStephen },
	{ Action(ACTION_USE, OBJECT_IBERRY, 11, 0), &Room::demon5UseBerryOnStephen },
	{ Action(ACTION_USE, OBJECT_IDETOXIN, 9, 0), &Room::demon5UseHypoDytoxinOnChub },
	{ Action(ACTION_FINISHED_WALKING, 2, 0, 0), &Room::demon5MccoyReachedChub },
	{ Action(ACTION_FINISHED_ANIMATION, 2, 0, 0), &Room::demon5MccoyHealedChub },

	{ Action(ACTION_USE, OBJECT_IDETOXIN, -1, 0), &Room::demon5UseHypoDytoxinOnAnything },
	{ Action(ACTION_USE, OBJECT_IBERRY, 9, 0), &Room::demon5UseBerryOnChub },

	{ Action(ACTION_LOOK, 8, 0, 0), &Room::demon5LookAtRoberts },
	{ Action(ACTION_LOOK, 10, 0, 0), &Room::demon5LookAtGrisnash },
	{ Action(ACTION_LOOK, 11, 0, 0), &Room::demon5LookAtStephen },

	{ Action(ACTION_LOOK, OBJECT_KIRK, 0, 0), &Room::demon5LookAtKirk },
	{ Action(ACTION_LOOK, OBJECT_SPOCK, 0, 0), &Room::demon5LookAtSpock },
	{ Action(ACTION_LOOK, OBJECT_MCCOY, 0, 0), &Room::demon5LookAtMccoy },
	{ Action(ACTION_LOOK, OBJECT_REDSHIRT, 0, 0), &Room::demon5LookAtRedshirt },

	{ Action(ACTION_LOOK, 0x20, 0, 0), &Room::demon5LookAtMountain },
	{ Action(ACTION_LOOK, 0x21, 0, 0), &Room::demon5LookAtCrate },
	{ Action(ACTION_LOOK, -1, 0, 0), &Room::demon5LookAnywhere },
	{ Action(ACTION_LOOK, 9, 0, 0), &Room::demon5LookAtChub },

	{ Action(ACTION_TALK, 8, 0, 0), &Room::demon5TalkToRoberts },
	{ Action(ACTION_TALK, 9, 0, 0), &Room::demon5TalkToChub },
	{ Action(ACTION_TALK, 10, 0, 0), &Room::demon5TalkToGrisnash },
	{ Action(ACTION_TALK, 11, 0, 0), &Room::demon5TalkToStephen },

	{ Action(ACTION_TALK, OBJECT_KIRK, 0, 0), &Room::demon5TalkToKirk },
	{ Action(ACTION_TALK, OBJECT_SPOCK, 0, 0), &Room::demon5TalkToSpock },
	{ Action(ACTION_TALK, OBJECT_REDSHIRT, 0, 0), &Room::demon5TalkToRedshirt },
	{ Action(ACTION_TALK, OBJECT_MCCOY, 0, 0), &Room::demon5TalkToMccoy },

	{ Action(ACTION_USE, OBJECT_IMTRICOR, 8, 0), &Room::demon5UseMTricorderOnRoberts },
	{ Action(ACTION_USE, OBJECT_IMEDKIT,  9, 0), &Room::demon5UseMTricorderOnChub },
	{ Action(ACTION_USE, OBJECT_MCCOY,    9, 0), &Room::demon5UseMTricorderOnChub },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 9, 0), &Room::demon5UseMTricorderOnChub },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 10, 0), &Room::demon5UseMTricorderOnGrisnash },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 11, 0), &Room::demon5UseMTricorderOnStephen },

	{ Action(ACTION_GET, 0x21, 0, 0), &Room::demon5GetCrate },
};

}

#endif
