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

#include "ultima/ultima4/location.h"
#include "ultima/ultima4/annotation.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/combat.h"
#include "ultima/ultima4/creature.h"
#include "ultima/ultima4/event.h"
#include "ultima/ultima4/game.h"
#include "ultima/ultima4/map.h"
#include "ultima/ultima4/object.h"
#include "ultima/ultima4/savegame.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/tileset.h"

namespace Ultima {
namespace Ultima4 {

Location *locationPush(Location *stack, Location *loc);
Location *locationPop(Location **stack);

/**
 * Add a new location to the stack, or
 * start a new stack if 'prev' is NULL
 */
Location::Location(MapCoords coords, Map *map, int viewmode, LocationContext ctx,
                   TurnCompleter *turnCompleter, Location *prev) {

    this->coords = coords;
    this->map = map;
    this->viewMode = viewmode;
    this->context = ctx;
    this->turnCompleter = turnCompleter;

    locationPush(prev, this);
}

/**
 * Return the entire stack of objects at the given location.
 */
Std::vector<MapTile> Location::tilesAt(MapCoords coords, bool &focus) {
    Std::vector<MapTile> tiles;
    Common::List<Annotation *> a = map->annotations->ptrsToAllAt(coords);
    Common::List<Annotation *>::iterator i;
    Object *obj = map->objectAt(coords);
    Creature *m = dynamic_cast<Creature *>(obj);
    focus = false;

    bool avatar = this->coords == coords;

    /* Do not return objects for VIEW_GEM mode, show only the avatar and tiles */
    if (viewMode == VIEW_GEM && (!settings.enhancements || !settings.enhancementsOptions.peerShowsObjects)) {        
        // When viewing a gem, always show the avatar regardless of whether or not
        // it is shown in our normal view
        if (avatar)
            tiles.push_back(c->party->getTransport());
        else             
            tiles.push_back(*map->getTileFromData(coords));

        return tiles;
    }

    /* Add the avatar to gem view */
    if (avatar && viewMode == VIEW_GEM)
        tiles.push_back(c->party->getTransport());
    
    /* Add visual-only annotations to the list */
    for (i = a.begin(); i != a.end(); i++) {
        if ((*i)->isVisualOnly())        
        {
            tiles.push_back((*i)->getTile());

            /* If this is the first cover-up annotation,
			 * everything underneath it will be invisible,
			 * so stop here
			 */
			if ((*i)->isCoverUp())
				return tiles;
        }
    }

    /* then the avatar is drawn (unless on a ship) */
    if ((map->flags & SHOW_AVATAR) && (c->transportContext != TRANSPORT_SHIP) && avatar)
        //tiles.push_back(map->tileset->getByName("avatar")->id);
        tiles.push_back(c->party->getTransport());

    /* then camouflaged creatures that have a disguise */
    if (obj && (obj->getType() == Object::CREATURE) && !obj->isVisible() && (!m->getCamouflageTile().empty())) {
        focus = focus || obj->hasFocus();
        tiles.push_back(map->tileset->getByName(m->getCamouflageTile())->getId());
    }
    /* then visible creatures and objects */
    else if (obj && obj->isVisible()) {
        focus = focus || obj->hasFocus();
        MapTile visibleCreatureAndObjectTile = obj->getTile();
		//Sleeping creatures and persons have their animation frozen
		if (m && m->isAsleep())
			visibleCreatureAndObjectTile.freezeAnimation = true;
        tiles.push_back(visibleCreatureAndObjectTile);
    }

    /* then the party's ship (because twisters and whirlpools get displayed on top of ships) */
    if ((map->flags & SHOW_AVATAR) && (c->transportContext == TRANSPORT_SHIP) && avatar)
        tiles.push_back(c->party->getTransport());

    /* then permanent annotations */
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

    /* finally the base tile */
    MapTile tileFromMapData = *map->getTileFromData(coords);
    const Tile * tileType = map->getTileFromData(coords)->getTileType();
    if (tileType->isLivingObject())
    {
    	//This animation should be frozen because a living object represented on the map data is usually a statue of a monster or something
    	tileFromMapData.freezeAnimation = true;
    }
	tiles.push_back(tileFromMapData);

	/* But if the base tile requires a background, we must find it */
    if (tileType->isLandForeground()	||
    	tileType->isWaterForeground()	||
    	tileType->isLivingObject())
    {

    	tiles.push_back(getReplacementTile(coords, tileType));
    }

    return tiles;
}


/**
 * Finds a valid replacement tile for the given location, using surrounding tiles
 * as guidelines to choose the new tile.  The new tile will only be chosen if it
 * is marked as a valid replacement (or waterReplacement) tile in tiles.xml.  If a valid replacement
 * cannot be found, it returns a "best guess" tile.
 */
TileId Location::getReplacementTile(MapCoords atCoords, const Tile * forTile) {
    Std::map<TileId, int> validMapTileCount;

    const static int dirs[][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    const static int dirs_per_step = sizeof(dirs) / sizeof(*dirs);
    int loop_count = 0;

    Std::set<MapCoords> searched;
    Common::List<MapCoords> searchQueue;

    //Pathfinding to closest traversable tile with appropriate replacement properties.
    //For tiles marked water-replaceable, pathfinding includes swimmables.
    searchQueue.push_back(atCoords);
    do
    {
        MapCoords currentStep = searchQueue.front();
        searchQueue.pop_front();

		searched.insert(currentStep);

    	for (int i = 0; i < dirs_per_step; i++)
		{
			MapCoords newStep(currentStep);
			newStep.move(dirs[i][0], dirs[i][1], map);

			Tile const * tileType = map->tileTypeAt(newStep,WITHOUT_OBJECTS);

			if (!tileType->isOpaque()) {
				//if (searched.find(newStep) == searched.end()) -- the find mechanism doesn't work.
				searchQueue.push_back(newStep);
			}

			if ((tileType->isReplacement() && (forTile->isLandForeground() || forTile->isLivingObject())) ||
				(tileType->isWaterReplacement() && forTile->isWaterForeground()))
			{
				Std::map<TileId, int>::iterator validCount = validMapTileCount.find(tileType->getId());

				if (validCount == validMapTileCount.end())
				{
					validMapTileCount[tileType->getId()] = 1;
				}
				else
				{
					validMapTileCount[tileType->getId()]++;
				}
			}
		}

		if (validMapTileCount.size() > 0)
		{
			Std::map<TileId, int>::iterator itr = validMapTileCount.begin();

			TileId winner = itr->_key;
			int score = itr->_value;

			while (++itr != validMapTileCount.end())
			{
				if (score < itr->_value)
				{
					score = itr->_value;
					winner = itr->_key;
				}
			}

			return winner;
		}
		/* loop_count is an ugly hack to temporarily fix infinite loop */
	} while (++loop_count < 128 && searchQueue.size() > 0 && searchQueue.size() < 64);

    /* couldn't find a tile, give it the classic default */
    return map->tileset->getByName("brick_floor")->getId();
}

/**
 * Returns the current coordinates of the location given:
 *     If in combat - returns the coordinates of party member with focus
 *     If elsewhere - returns the coordinates of the avatar
 */
int Location::getCurrentPosition(MapCoords *coords) {
    if (context & CTX_COMBAT) {
        CombatController *cc = dynamic_cast<CombatController *>(eventHandler->getController());
        PartyMemberVector *party = cc->getParty();
        *coords = (*party)[cc->getFocus()]->getCoords();    
    }
    else
        *coords = this->coords;

    return 1;
}

MoveResult Location::move(Direction dir, bool userEvent) {
    MoveEvent event(dir, userEvent);
    switch (map->type) {

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

    return event.result;
}


/**
 * Pop a location from the stack and free the memory
 */
void locationFree(Location **stack) {
    delete locationPop(stack);
}

/**
 * Push a location onto the stack
 */
Location *locationPush(Location *stack, Location *loc) {
    loc->prev = stack;
    return loc;
}

/**
 * Pop a location off the stack
 */
Location *locationPop(Location **stack) {
    Location *loc = *stack;
    *stack = (*stack)->prev;
    loc->prev = NULL;
    return loc;
}

} // End of namespace Ultima4
} // End of namespace Ultima
