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

#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/map/city.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/map/dungeon.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/shrine.h"
#include "ultima/ultima4/map/tile.h"

namespace Ultima {
namespace Ultima4 {

void createDngLadder(Location *location, PortalTriggerAction action, Portal *p) {
	if (!p) return;
	else {
		p->_destid = location->_map->_id;
		if (action == ACTION_KLIMB && location->_coords.z == 0) {
			p->_exitPortal = true;
			p->_destid = 1;
		} else p->_exitPortal = false;
		p->_message = "";
		p->_portalConditionsMet = nullptr;
		p->_portalTransportRequisites = TRANSPORT_FOOT_OR_HORSE;
		p->_retroActiveDest = nullptr;
		p->_saveLocation = false;
		p->_start = location->_coords;
		p->_start.z += (action == ACTION_KLIMB) ? -1 : 1;
	}
}

int usePortalAt(Location *location, MapCoords coords, PortalTriggerAction action) {
	Map *destination;
	char msg[32] = {0};

	const Portal *portal = location->_map->portalAt(coords, action);
	Portal dngLadder;

	/* didn't find a portal there */
	if (!portal) {

		/* if it's a dungeon, then ladders are predictable.  Create one! */
		if (location->_context == CTX_DUNGEON) {
			Dungeon *dungeon = dynamic_cast<Dungeon *>(location->_map);
			assert(dungeon);

			if ((action & ACTION_KLIMB) && dungeon->ladderUpAt(coords))
				createDngLadder(location, action, &dngLadder);
			else if ((action & ACTION_DESCEND) && dungeon->ladderDownAt(coords))
				createDngLadder(location, action, &dngLadder);
			else
				return 0;
			portal = &dngLadder;
		} else {
			return 0;
		}
	}

	/* conditions not met for portal to work */
	if (portal && portal->_portalConditionsMet && !(*portal->_portalConditionsMet)(portal))
		return 0;
	/* must klimb or descend on foot! */
	else if (g_context->_transportContext & ~TRANSPORT_FOOT && (action == ACTION_KLIMB || action == ACTION_DESCEND)) {
		g_screen->screenMessage("%sOnly on foot!\n", action == ACTION_KLIMB ? "Klimb\n" : "");
		return 1;
	}

	destination = mapMgr->get(portal->_destid);

	if (portal->_message.empty()) {

		switch (action) {
		case ACTION_DESCEND:
			sprintf(msg, "Descend down to level %d\n", portal->_start.z + 1);
			break;
		case ACTION_KLIMB:
			if (portal->_exitPortal)
				sprintf(msg, "Klimb up!\nLeaving...\n");
			else
				sprintf(msg, "Klimb up!\nTo level %d\n", portal->_start.z + 1);
			break;
		case ACTION_ENTER:
			switch (destination->_type) {
			case Map::CITY: {
				City *city = dynamic_cast<City *>(destination);
				assert(city);
				g_screen->screenMessage("Enter %s!\n\n%s\n\n", city->_type.c_str(), city->getName().c_str());
			}
			break;
			case Map::SHRINE:
				g_screen->screenMessage("Enter the %s!\n\n", destination->getName().c_str());
				break;
			case Map::DUNGEON:
#ifdef IOS_ULTIMA4
				U4IOS::testFlightPassCheckPoint("Enter " + destination->getName());
#endif
				g_screen->screenMessage("Enter dungeon!\n\n%s\n\n", destination->getName().c_str());
				break;
			default:
				break;
			}
			break;
		case ACTION_NONE:
		default:
			break;
		}
	}

	/* check the transportation requisites of the portal */
	if (g_context->_transportContext & ~portal->_portalTransportRequisites) {
		g_screen->screenMessage("Only on foot!\n");
		return 1;
	}
	/* ok, we know the portal is going to work -- now display the custom message, if any */
	else if (!portal->_message.empty() || strlen(msg))
		g_screen->screenMessage("%s", portal->_message.empty() ? msg : portal->_message.c_str());

	/* portal just exits to parent map */
	if (portal->_exitPortal) {
		g_game->exitToParentMap();
		g_music->playMapMusic();
		return 1;
	} else if (portal->_destid == location->_map->_id)
		location->_coords = portal->_start;

	else {
		g_game->setMap(destination, portal->_saveLocation, portal);
		g_music->playMapMusic();
	}

	/* if the portal changes the map retroactively, do it here */
	/*
	 * note that we use c->location instead of location, since
	 * location has probably been invalidated above
	 */
	if (portal->_retroActiveDest && g_context->_location->_prev) {
		g_context->_location->_prev->_coords = portal->_retroActiveDest->_coords;
		g_context->_location->_prev->_map = mapMgr->get(portal->_retroActiveDest->_mapid);
	}

	if (destination->_type == Map::SHRINE) {
		Shrine *shrine = dynamic_cast<Shrine *>(destination);
		assert(shrine);
		shrine->enter();
	}

	return 1;
}

} // End of namespace Ultima4
} // End of namespace Ultima
