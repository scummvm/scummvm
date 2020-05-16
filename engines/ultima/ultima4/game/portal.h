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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA4_GAME_PORTAL_H
#define ULTIMA4_GAME_PORTAL_H

#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/map/map.h"

namespace Ultima {
namespace Ultima4 {

class Map;
class Location;
struct Portal;

typedef enum {
	ACTION_NONE         = 0x0,
	ACTION_ENTER        = 0x1,
	ACTION_KLIMB        = 0x2,
	ACTION_DESCEND      = 0x4,
	ACTION_EXIT_NORTH   = 0x8,
	ACTION_EXIT_EAST    = 0x10,
	ACTION_EXIT_SOUTH   = 0x20,
	ACTION_EXIT_WEST    = 0x40
} PortalTriggerAction;

typedef bool (*PortalConditionsMet)(const Portal *p);

struct PortalDestination {
	MapCoords _coords;
	MapId _mapid;
};

struct Portal {
	MapCoords _coords;
	MapId _destid;
	MapCoords _start;
	PortalTriggerAction _triggerAction;
	PortalConditionsMet _portalConditionsMet;
	PortalDestination *_retroActiveDest;
	bool _saveLocation;
	Common::String _message;
	TransportContext _portalTransportRequisites;
	bool _exitPortal;
	int _tile;
};

/**
 * Creates a dungeon ladder portal based on the action given
 */
void createDngLadder(Location *location, PortalTriggerAction action, Portal *p);

/**
 * Finds a portal at the given (x,y,z) coords that will work with the action given
 * and uses it.  If in a dungeon and trying to use a ladder, it creates a portal
 * based on the ladder and uses it.
 */
int usePortalAt(Location *location, MapCoords coords, PortalTriggerAction action);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
