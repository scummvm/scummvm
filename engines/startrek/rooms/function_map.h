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

}

#endif
