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

	// TODO: Walk events; entered room event; beamed in event

	{ Action(ACTION_TOUCHED_HOTSPOT, 1, 0, 0), &Room::demon0TouchedHotspot1 },
	{ Action(ACTION_TOUCHED_HOTSPOT, 0, 0, 0), &Room::demon0TouchedHotspot0 },

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

	{ Action(ACTION_LOOK, 0, 0, 0), &Room::demon0LookAtKirk },
	{ Action(ACTION_LOOK, 2, 0, 0), &Room::demon0LookAtMcCoy },
	{ Action(ACTION_LOOK, 3, 0, 0), &Room::demon0LookAtRedShirt },
	{ Action(ACTION_LOOK, 1, 0, 0), &Room::demon0LookAtSpock },

	{ Action(ACTION_LOOK, 9, 0, 0), &Room::demon0LookAtShelter },    // Door 1
	{ Action(ACTION_LOOK, 10, 0, 0), &Room::demon0LookAtShelter },   // Door 2
	{ Action(ACTION_LOOK, 0x20, 0, 0), &Room::demon0LookAtShelter }, // Shelter itself

	{ Action(ACTION_TALK, 0, 0, 0), &Room::demon0TalkToKirk },
	{ Action(ACTION_TALK, 3, 0, 0), &Room::demon0TalkToRedshirt },
	{ Action(ACTION_TALK, 2, 0, 0), &Room::demon0TalkToMcCoy },
	{ Action(ACTION_TALK, 1, 0, 0), &Room::demon0TalkToSpock },

	{ Action(ACTION_USE, OBJECT_ISTRICOR, 0xff, 0), &Room::useSTricorderAnywhere },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 0xff, 0), &Room::useMTricorderAnywhere },
	{ Action(ACTION_USE, OBJECT_IMTRICOR, 8, 0), &Room::useMTricorderOnPrelate },
};

}

#endif
