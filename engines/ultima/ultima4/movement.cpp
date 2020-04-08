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

#include "ultima/ultima4/movement.h"
#include "ultima/ultima4/annotation.h"
#include "ultima/ultima4/combat.h"
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/dungeon.h"
#include "ultima/ultima4/event.h"
#include "ultima/ultima4/location.h"
#include "ultima/ultima4/creature.h"
#include "ultima/ultima4/object.h"
#include "ultima/ultima4/player.h"
#include "ultima/ultima4/savegame.h"
#include "ultima/ultima4/tile.h"
#include "ultima/ultima4/utils.h"

namespace Ultima {
namespace Ultima4 {

bool collisionOverride = false;

/**
 * Attempt to move the avatar in the given direction.  User event
 * should be set if the avatar is being moved in response to a
 * keystroke.  Returns zero if the avatar is blocked.
 */
void moveAvatar(MoveEvent &event) {    
    MapCoords newCoords;
    int slowed = 0;
    SlowedType slowedType = SLOWED_BY_TILE;

    /* Check to see if we're on the balloon */
    if (g_context->_transportContext == TRANSPORT_BALLOON && event._userEvent) {
        event._result = (MoveResult)(MOVE_DRIFT_ONLY | MOVE_END_TURN);
        return;
    }

    if (g_context->_transportContext == TRANSPORT_SHIP)
        slowedType = SLOWED_BY_WIND;
    else if (g_context->_transportContext == TRANSPORT_BALLOON)
        slowedType = SLOWED_BY_NOTHING;

    /* if you're on ship, you must turn first! */
    if (g_context->_transportContext == TRANSPORT_SHIP) {
        if (g_context->_party->getDirection() != event._dir) {
            g_context->_party->setDirection(event._dir);
            event._result = (MoveResult)(MOVE_TURNED | MOVE_END_TURN);
            return;
        }
    }
    
    /* change direction of horse, if necessary */
    if (g_context->_transportContext == TRANSPORT_HORSE) {
        if ((event._dir == DIR_WEST || event._dir == DIR_EAST) && (g_context->_party->getDirection() != event._dir))
            g_context->_party->setDirection(event._dir);
    }

    /* figure out our new location we're trying to move to */
    newCoords = g_context->_location->_coords;    
    newCoords.move(event._dir, g_context->_location->_map);

    /* see if we moved off the map */
    if (MAP_IS_OOB(g_context->_location->_map, newCoords)) {
        event._result = (MoveResult)(MOVE_MAP_CHANGE | MOVE_EXIT_TO_PARENT | MOVE_SUCCEEDED);
        return;
    }

    if (!collisionOverride && !g_context->_party->isFlying()) {
        int movementMask = g_context->_location->_map->getValidMoves(g_context->_location->_coords, g_context->_party->getTransport());
        /* See if movement was blocked */
        if (!DIR_IN_MASK(event._dir, movementMask)) {
            event._result = (MoveResult)(MOVE_BLOCKED | MOVE_END_TURN);
            return;
        }

        /* Are we slowed by terrain or by wind direction? */
        switch(slowedType) {
        case SLOWED_BY_TILE:
          // TODO: CHEST: Make a user option to not make chests always fast to
          // travel over
            slowed = slowedByTile(g_context->_location->_map->tileTypeAt(newCoords, WITH_OBJECTS));
            break;
        case SLOWED_BY_WIND:
            slowed = slowedByWind(event._dir);
            break;
        case SLOWED_BY_NOTHING:
        default:
            break;
        }
        
        if (slowed) {
            event._result = (MoveResult)(MOVE_SLOWED | MOVE_END_TURN);
            return;
        }
    }

    /* move succeeded */
    g_context->_location->_coords = newCoords;    

    /* if the avatar moved onto a creature (whirlpool, twister), then do the creature's special effect (this current code does double damage according to changeset 2753.
      
    Object *destObj = c->location->map->objectAt(newCoords);
    if (destObj && destObj->getType() == Object::CREATURE) {
        Creature *m = dynamic_cast<Creature*>(destObj);
        //m->specialEffect();
    }
    */

    event._result = (MoveResult)(MOVE_SUCCEEDED | MOVE_END_TURN);
}

/**
 * Moves the avatar while in dungeon view
 */
void moveAvatarInDungeon(MoveEvent &event) {
    MapCoords newCoords;
    Direction realDir = dirNormalize((Direction)g_context->_saveGame->_orientation, event._dir); /* get our real direction */  
    int advancing = realDir == g_context->_saveGame->_orientation,
        retreating = realDir == dirReverse((Direction)g_context->_saveGame->_orientation);
    MapTile *tile;
    
    /* we're not in a dungeon, failed! */
    ASSERT(g_context->_location->_context & CTX_DUNGEON, "moveAvatarInDungeon() called outside of dungeon, failed!");    
        
    /* you must turn first! */
    if (!advancing && !retreating) {        
        g_context->_saveGame->_orientation = realDir;
        event._result = MOVE_TURNED;
        return;
    }
    
    /* figure out our new location */
    newCoords = g_context->_location->_coords;    
    newCoords.move(realDir, g_context->_location->_map);

    tile = g_context->_location->_map->tileAt(newCoords, WITH_OBJECTS);

    /* see if we moved off the map (really, this should never happen in a dungeon) */
    if (MAP_IS_OOB(g_context->_location->_map, newCoords)) {
        event._result = (MoveResult)(MOVE_MAP_CHANGE | MOVE_EXIT_TO_PARENT | MOVE_SUCCEEDED);
        return;
    }

    if (!collisionOverride) {
        int movementMask = g_context->_location->_map->getValidMoves(g_context->_location->_coords, g_context->_party->getTransport());

        if (advancing && !tile->getTileType()->canWalkOn(DIR_ADVANCE))
            movementMask = DIR_REMOVE_FROM_MASK(realDir, movementMask);
        else if (retreating && !tile->getTileType()->canWalkOn(DIR_RETREAT))
            movementMask = DIR_REMOVE_FROM_MASK(realDir, movementMask);

        if (!DIR_IN_MASK(realDir, movementMask)) {
            event._result = (MoveResult)(MOVE_BLOCKED | MOVE_END_TURN);
            return;
        }
    }

    /* move succeeded */
    g_context->_location->_coords = newCoords;    

    event._result = (MoveResult)(MOVE_SUCCEEDED | MOVE_END_TURN);
}

/**
 * Moves an object on the map according to its movement behavior
 * Returns 1 if the object was moved successfully, 0 if slowed,
 * tile direction changed, or object simply cannot move
 * (fixed objects, nowhere to go, etc.)
 */
int moveObject(Map *map, Creature *obj, MapCoords avatar) {
    int dirmask = DIR_NONE;
    Direction dir;
    MapCoords new_coords = obj->getCoords();    
    int slowed = 0;    
    
    /* determine a direction depending on the object's movement behavior */
    dir = DIR_NONE;
    switch (obj->getMovementBehavior()) {
    case MOVEMENT_FIXED:
        break;

    case MOVEMENT_WANDER:
        /* World map wandering creatures always move, whereas
           town creatures that wander sometimes stay put */
        if (map->isWorldMap() || xu4_random(2) == 0)
            dir = dirRandomDir(map->getValidMoves(new_coords, obj->getTile()));
        break;

    case MOVEMENT_FOLLOW_AVATAR:
    case MOVEMENT_ATTACK_AVATAR:
        dirmask = map->getValidMoves(new_coords, obj->getTile());
        
        /* If the pirate ship turned last move instead of moving, this time it must
           try to move, not turn again */
        if (obj->getTile().getTileType()->isPirateShip() && DIR_IN_MASK(obj->getTile().getDirection(), dirmask) &&
            (obj->getTile() != obj->getPrevTile()) && (obj->getPrevCoords() == obj->getCoords())) {
            dir = obj->getTile().getDirection();
            break;
        }

        dir = new_coords.pathTo(avatar, dirmask, true, g_context->_location->_map);
        break;
    }
    
    /* now, get a new x and y for the object */
    if (dir)
        new_coords.move(dir, g_context->_location->_map);        
    else
        return 0;

    /* figure out what method to use to tell if the object is getting slowed */   
    SlowedType slowedType = SLOWED_BY_TILE;
    if (obj->getType() == Object::CREATURE)
        slowedType = obj->getSlowedType();
    
    /* is the object slowed by terrain or by wind direction? */
    switch(slowedType) {
    case SLOWED_BY_TILE:
        slowed = slowedByTile(map->tileTypeAt(new_coords, WITHOUT_OBJECTS));
        break;
    case SLOWED_BY_WIND:
        slowed = slowedByWind(obj->getTile().getDirection());
        break;
    case SLOWED_BY_NOTHING:
    default:
        break;
    }
    
    obj->setPrevCoords(obj->getCoords());
    
    /* see if the object needed to turn instead of move */
    if (obj->setDirection(dir))
        return 0;    
    
    /* was the object slowed? */
    if (slowed)
        return 0;

    /**
     * Set the new coordinates
     */ 
    if (!(new_coords == obj->getCoords()) && 
        !MAP_IS_OOB(map, new_coords))
    {
    	obj->setCoords(new_coords);
    }
    return 1;
}

/**
 * Moves an object in combat according to its chosen combat action
 */
int moveCombatObject(int act, Map *map, Creature *obj, MapCoords target) {
    MapCoords new_coords = obj->getCoords();
    int valid_dirs = map->getValidMoves(new_coords, obj->getTile());
    Direction dir;
    CombatAction action = (CombatAction)act;
    SlowedType slowedType = SLOWED_BY_TILE;
    int slowed = 0;

    /* fixed objects cannot move */
    if (obj->getMovementBehavior() == MOVEMENT_FIXED)
        return 0;

    if (action == CA_FLEE) {
        /* run away from our target instead! */
        dir = new_coords.pathAway(target, valid_dirs);
    
    } else {
        ASSERT(action == CA_ADVANCE, "action must be CA_ADVANCE or CA_FLEE");
        // If they're not fleeing, make sure they don't flee on accident
        if (new_coords.x == 0)
            valid_dirs = DIR_REMOVE_FROM_MASK(DIR_WEST, valid_dirs);
        else if (new_coords.x >= (signed)(map->_width - 1))
            valid_dirs = DIR_REMOVE_FROM_MASK(DIR_EAST, valid_dirs);
        if (new_coords.y == 0)
            valid_dirs = DIR_REMOVE_FROM_MASK(DIR_NORTH, valid_dirs);
        else if (new_coords.y >= (signed)(map->_height - 1))
            valid_dirs = DIR_REMOVE_FROM_MASK(DIR_SOUTH, valid_dirs);        

        dir = new_coords.pathTo(target, valid_dirs);
    }

    if (dir)
        new_coords.move(dir, g_context->_location->_map);
    else
        return 0;

    /* figure out what method to use to tell if the object is getting slowed */   
    if (obj->getType() == Object::CREATURE)
        slowedType = obj->getSlowedType();

    /* is the object slowed by terrain or by wind direction? */
    switch(slowedType) {
    case SLOWED_BY_TILE:
        slowed = slowedByTile(map->tileTypeAt(new_coords, WITHOUT_OBJECTS));
        break;
    case SLOWED_BY_WIND:
        slowed = slowedByWind(obj->getTile().getDirection());
        break;
    case SLOWED_BY_NOTHING:
    default:
        break;
    }

    /* if the object wan't slowed... */
    if (!slowed) {        
        // Set the new coordinates
    	obj->setCoords(new_coords);
        return 1;
    }

    return 0;
}

/**
 * Moves a party member during combat screens
 */
void movePartyMember(MoveEvent &event) {
    CombatController *ct = dynamic_cast<CombatController *>(eventHandler->getController());
    CombatMap *cm = getCombatMap();
    int member = ct->getFocus();
    MapCoords newCoords;
    PartyMemberVector *party = ct->getParty();

    event._result = MOVE_SUCCEEDED;    

    /* find our new location */
    newCoords = (*party)[member]->getCoords();
    newCoords.move(event._dir, g_context->_location->_map);

    if (MAP_IS_OOB(g_context->_location->_map, newCoords)) {
        bool sameExit = (!cm->isDungeonRoom() || (ct->getExitDir() == DIR_NONE) || (event._dir == ct->getExitDir()));
        if (sameExit) {
            /* if in a win-or-lose battle and not camping, then it can be bad to flee while healthy */
            if (ct->isWinOrLose() && !ct->isCamping()) {
                /* A fully-healed party member fled from an evil creature :( */
                if (ct->getCreature() && ct->getCreature()->isEvil() && 
                    g_context->_party->member(member)->getHp() == g_context->_party->member(member)->getMaxHp())
                    g_context->_party->adjustKarma(KA_HEALTHY_FLED_EVIL);
            }

            ct->setExitDir(event._dir);
            g_context->_location->_map->removeObject((*party)[member]);
            (*party)[member] = NULL;
            event._result = (MoveResult)(MOVE_EXIT_TO_PARENT | MOVE_MAP_CHANGE | MOVE_SUCCEEDED | MOVE_END_TURN);
            return;
        }
        else {
            event._result = (MoveResult)(MOVE_MUST_USE_SAME_EXIT | MOVE_END_TURN);
            return;
        }
    }

    int movementMask = g_context->_location->_map->getValidMoves((*party)[member]->getCoords(), (*party)[member]->getTile());
    if (!DIR_IN_MASK(event._dir, movementMask)) {
        event._result = (MoveResult)(MOVE_BLOCKED | MOVE_END_TURN);
        return;
    }

    /* is the party member slowed? */
    if (!slowedByTile(g_context->_location->_map->tileTypeAt(newCoords, WITHOUT_OBJECTS)))
    {
        /* move succeeded */        
        (*party)[member]->setCoords(newCoords);

        /* handle dungeon room triggers */
        if (cm->isDungeonRoom()) {
            Dungeon *dungeon = dynamic_cast<Dungeon*>(g_context->_location->_prev->_map);
            int i;
            Trigger *triggers = dungeon->_rooms[dungeon->_currentRoom]._triggers;            

            for (i = 0; i < 4; i++) {
                /*const Creature *m = creatures.getByTile(triggers[i].tile);*/

                /* FIXME: when a creature is created by a trigger, it can be created over and over and over...
                   how do we fix this? In the c64 version is appears that such triggers (on the world map)
                   wipe the creature table and replace it with the triggered creatures. Thus, retriggering
                   it will reset the creatures.
                   */
                MapCoords trigger(triggers[i].x, triggers[i].y, g_context->_location->_coords.z);

                /* see if we're on a trigger */
                if (newCoords == trigger) {
                    MapCoords change1(triggers[i]._changeX1, triggers[i]._changeY1, g_context->_location->_coords.z),
                              change2(triggers[i].changeX2, triggers[i].changeY2, g_context->_location->_coords.z);

                    /**
                     * Remove any previous annotations placed at our target coordinates
                     */ 
                    g_context->_location->_map->_annotations->remove(g_context->_location->_map->_annotations->allAt(change1));
                    g_context->_location->_map->_annotations->remove(g_context->_location->_map->_annotations->allAt(change2));

                    /* change the tiles! */
                    if (change1.x || change1.y) {
                        /*if (m) combatAddCreature(m, triggers[i].change_x1, triggers[i].change_y1, c->location->coords.z);
                        else*/ g_context->_location->_map->_annotations->add(change1, triggers[i]._tile, false, true);
                    }
                    if (change2.x || change2.y) {
                        /*if (m) combatAddCreature(m, triggers[i].change_x2, triggers[i].change_y2, c->location->coords.z);
                        else*/ g_context->_location->_map->_annotations->add(change2, triggers[i]._tile, false, true);
                    }
                }
            }
        }    
    }
    else {
        event._result = (MoveResult)(MOVE_SLOWED | MOVE_END_TURN);
        return;
    }
}
 
/**
 * Default handler for slowing movement.
 * Returns true if slowed, false if not slowed
 */
bool slowedByTile(const Tile *tile) {
    bool slow;
    
    switch (tile->getSpeed()) {
    case SLOW:
        slow = xu4_random(8) == 0;
        break;
    case VSLOW:
        slow = xu4_random(4) == 0;
        break;
    case VVSLOW:
        slow = xu4_random(2) == 0;
        break;
    case FAST:
    default:
        slow = false;
        break;
    }

    return slow;
}

/**
 * Slowed depending on the direction of object with respect to wind direction
 * Returns true if slowed, false if not slowed
 */
bool slowedByWind(int direction) {
    /* 1 of 4 moves while trying to move into the wind succeeds */
    if (direction == g_context->_windDirection)
        return (g_context->_saveGame->_moves % 4) != 0;
    /* 1 of 4 moves while moving directly away from wind fails */
    else if (direction == dirReverse((Direction) g_context->_windDirection))
        return (g_context->_saveGame->_moves % 4) == 3;    
    else
        return false;
}

} // End of namespace Ultima4
} // End of namespace Ultima
