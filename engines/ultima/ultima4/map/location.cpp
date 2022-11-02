/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/controllers/combat_controller.h"
#include "ultima/ultima4/game/creature.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/map/tileset.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Push a location onto the stack
 */
static Location *locationPush(Location *stack, Location *loc) {
	loc->_prev = stack;
	return loc;
}

/**
 * Pop a location off the stack
 */
static Location *locationPop(Location **stack) {
	Location *loc = *stack;
	*stack = (*stack)->_prev;
	loc->_prev = nullptr;
	return loc;
}

/*-------------------------------------------------------------------*/

Location::Location(MapCoords coords, Map *map, int viewmode, LocationContext ctx,
				   TurnCompleter *turnCompleter, Location *prev) {

	this->_coords = coords;
	this->_map = map;
	this->_viewMode = viewmode;
	this->_context = ctx;
	this->_turnCompleter = turnCompleter;

	locationPush(prev, this);
}

Std::vector<MapTile> Location::tilesAt(MapCoords coords, bool &focus) {
	Std::vector<MapTile> tiles;
	Common::List<Annotation *> a = _map->_annotations->ptrsToAllAt(coords);
	Common::List<Annotation *>::iterator i;
	Object *obj = _map->objectAt(coords);
	Creature *m = dynamic_cast<Creature *>(obj);
	focus = false;

	bool avatar = this->_coords == coords;

	// Do not return objects for VIEW_GEM mode, show only the avatar and tiles
	if (_viewMode == VIEW_GEM && (!settings._enhancements || !settings._enhancementsOptions._peerShowsObjects)) {
		// When viewing a gem, always show the avatar regardless of whether or not
		// it is shown in our normal view
		if (avatar)
			tiles.push_back(g_context->_party->getTransport());
		else
			tiles.push_back(*_map->getTileFromData(coords));

		return tiles;
	}

	// Add the avatar to gem view
	if (avatar && _viewMode == VIEW_GEM)
		tiles.push_back(g_context->_party->getTransport());

	// Add visual-only annotations to the list
	for (i = a.begin(); i != a.end(); i++) {
		if ((*i)->isVisualOnly()) {
			tiles.push_back((*i)->getTile());

			/* If this is the first cover-up annotation,
			 * everything underneath it will be invisible,
			 * so stop here
			 */
			if ((*i)->isCoverUp())
				return tiles;
		}
	}

	// then the avatar is drawn (unless on a ship)
	if ((_map->_flags & SHOW_AVATAR) && (g_context->_transportContext != TRANSPORT_SHIP) && avatar)
		//tiles.push_back(map->tileset->getByName("avatar")->id);
		tiles.push_back(g_context->_party->getTransport());

	// then camouflaged creatures that have a disguise
	if (obj && (obj->getType() == Object::CREATURE) && !obj->isVisible() && (!m->getCamouflageTile().empty())) {
		focus = focus || obj->hasFocus();
		tiles.push_back(_map->_tileSet->getByName(m->getCamouflageTile())->getId());
	}
	// then visible creatures and objects
	else if (obj && obj->isVisible()) {
		focus = focus || obj->hasFocus();
		MapTile visibleCreatureAndObjectTile = obj->getTile();
		//Sleeping creatures and persons have their animation frozen
		if (m && m->isAsleep())
			visibleCreatureAndObjectTile._freezeAnimation = true;
		tiles.push_back(visibleCreatureAndObjectTile);
	}

	// then the party's ship (because twisters and whirlpools get displayed on top of ships)
	if ((_map->_flags & SHOW_AVATAR) && (g_context->_transportContext == TRANSPORT_SHIP) && avatar)
		tiles.push_back(g_context->_party->getTransport());

	// then permanent annotations
	for (i = a.begin(); i != a.end(); i++) {
		if (!(*i)->isVisualOnly()) {
			tiles.push_back((*i)->getTile());

			/* If this is the first cover-up annotation,
			 * everything underneath it will be invisible,
			 * so stop here
			 */
			if ((*i)->isCoverUp())
				return tiles;
		}
	}

	// finally the base tile
	MapTile tileFromMapData = *_map->getTileFromData(coords);
	const Tile *tileType = _map->getTileFromData(coords)->getTileType();
	if (tileType->isLivingObject()) {
		//This animation should be frozen because a living object represented on the map data is usually a statue of a monster or something
		tileFromMapData._freezeAnimation = true;
	}
	tiles.push_back(tileFromMapData);

	// But if the base tile requires a background, we must find it
	if (tileType->isLandForeground()    ||
	        tileType->isWaterForeground()   ||
	        tileType->isLivingObject()) {

		tiles.push_back(getReplacementTile(coords, tileType));
	}

	return tiles;
}

TileId Location::getReplacementTile(MapCoords atCoords, const Tile *forTile) {
	Common::HashMap<TileId, int> validMapTileCount;

	const static int dirs[][2] = {{ -1, 0}, {1, 0}, {0, -1}, {0, 1}};
	const static int dirs_per_step = sizeof(dirs) / sizeof(*dirs);
	int loop_count = 0;

	Std::set<MapCoords> searched;
	Common::List<MapCoords> searchQueue;

	//Pathfinding to closest traversable tile with appropriate replacement properties.
	//For tiles marked water-replaceable, pathfinding includes swimmables.
	searchQueue.push_back(atCoords);
	do {
		MapCoords currentStep = searchQueue.front();
		searchQueue.pop_front();

		searched.insert(currentStep);

		for (int i = 0; i < dirs_per_step; i++) {
			MapCoords newStep(currentStep);
			newStep.move(dirs[i][0], dirs[i][1], _map);

			Tile const *tileType = _map->tileTypeAt(newStep, WITHOUT_OBJECTS);

			if (!tileType->isOpaque()) {
				//if (searched.find(newStep) == searched.end()) -- the find mechanism doesn't work.
				searchQueue.push_back(newStep);
			}

			if ((tileType->isReplacement() && (forTile->isLandForeground() || forTile->isLivingObject())) ||
			        (tileType->isWaterReplacement() && forTile->isWaterForeground())) {
				Common::HashMap<TileId, int>::iterator validCount = validMapTileCount.find(tileType->getId());

				if (validCount == validMapTileCount.end()) {
					validMapTileCount[tileType->getId()] = 1;
				} else {
					validMapTileCount[tileType->getId()]++;
				}
			}
		}

		if (validMapTileCount.size() > 0) {
			Common::HashMap<TileId, int>::iterator itr = validMapTileCount.begin();

			TileId winner = itr->_key;
			int score = itr->_value;

			while (++itr != validMapTileCount.end()) {
				if (score < itr->_value) {
					score = itr->_value;
					winner = itr->_key;
				}
			}

			return winner;
		}
		// loop_count is an ugly hack to temporarily fix infinite loop
	} while (++loop_count < 128 && searchQueue.size() > 0 && searchQueue.size() < 64);

	// couldn't find a tile, give it the classic default
	return _map->_tileSet->getByName("brick_floor")->getId();
}

int Location::getCurrentPosition(MapCoords *coords) {
	if (_context & CTX_COMBAT) {
		CombatController *cc = dynamic_cast<CombatController *>(eventHandler->getController());
		assert(cc);
		PartyMemberVector *party = cc->getParty();
		*coords = (*party)[cc->getFocus()]->getCoords();
	} else {
		*coords = this->_coords;
	}

	return 1;
}

MoveResult Location::move(Direction dir, bool userEvent) {
	MoveEvent event(dir, userEvent);

	switch (_map->_type) {
	case Map::DUNGEON:
		moveAvatarInDungeon(event);
		break;

	case Map::COMBAT:
		movePartyMember(event);
		break;

	default:
		moveAvatar(event);
		break;
	}

	setChanged();
	notifyObservers(event);

	return event._result;
}

void locationFree(Location **stack) {
	delete locationPop(stack);
}

} // End of namespace Ultima4
} // End of namespace Ultima
