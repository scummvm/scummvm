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

#include "ultima/ultima4/map/movement.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/controllers/combat_controller.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/map/dungeon.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/game/creature.h"
#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/map/tile.h"
#include "ultima/ultima4/core/debugger.h"
#include "ultima/ultima4/core/utils.h"

namespace Ultima {
namespace Ultima4 {

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

	// If you're on ship, you must turn first!
	if (g_context->_transportContext == TRANSPORT_SHIP) {
		if (g_context->_party->getDirection() != event._dir) {
			g_context->_party->setDirection(event._dir);
			event._result = (MoveResult)(MOVE_TURNED | MOVE_END_TURN);
			return;
		}
	}

	// Change direction of horse, if necessary
	if (g_context->_transportContext == TRANSPORT_HORSE) {
		if ((event._dir == DIR_WEST || event._dir == DIR_EAST) && (g_context->_party->getDirection() != event._dir))
			g_context->_party->setDirection(event._dir);
	}

	// Figure out our new location we're trying to move to
	newCoords = g_context->_location->_coords;
	newCoords.move(event._dir, g_context->_location->_map);

	// See if we moved off the map
	if (MAP_IS_OOB(g_context->_location->_map, newCoords)) {
		event._result = (MoveResult)(MOVE_MAP_CHANGE | MOVE_EXIT_TO_PARENT | MOVE_SUCCEEDED);
		return;
	}

	if (!g_debugger->_collisionOverride && !g_context->_party->isFlying()) {
		int movementMask = g_context->_location->_map->getValidMoves(g_context->_location->_coords, g_context->_party->getTransport());
		// See if movement was blocked
		if (!DIR_IN_MASK(event._dir, movementMask)) {
			event._result = (MoveResult)(MOVE_BLOCKED | MOVE_END_TURN);
			return;
		}

		// Are we slowed by terrain or by wind direction?
		switch (slowedType) {
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

	// Move succeeded
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

void moveAvatarInDungeon(MoveEvent &event) {
	MapCoords newCoords;
	Direction realDir = dirNormalize((Direction)g_ultima->_saveGame->_orientation, event._dir); /* get our real direction */
	int advancing = realDir == g_ultima->_saveGame->_orientation,
	    retreating = realDir == dirReverse((Direction)g_ultima->_saveGame->_orientation);
	MapTile *tile;

	// We're not in a dungeon, failed!
	assertMsg(g_context->_location->_context & CTX_DUNGEON, "moveAvatarInDungeon() called outside of dungeon, failed!");

	// You must turn first!
	if (!advancing && !retreating) {
		g_ultima->_saveGame->_orientation = realDir;
		event._result = MOVE_TURNED;
		return;
	}

	// Figure out our new location
	newCoords = g_context->_location->_coords;
	newCoords.move(realDir, g_context->_location->_map);

	tile = g_context->_location->_map->tileAt(newCoords, WITH_OBJECTS);

	// See if we moved off the map (really, this should never happen in a dungeon)
	if (MAP_IS_OOB(g_context->_location->_map, newCoords)) {
		event._result = (MoveResult)(MOVE_MAP_CHANGE | MOVE_EXIT_TO_PARENT | MOVE_SUCCEEDED);
		return;
	}

	if (!g_debugger->_collisionOverride) {
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

	// Move succeeded
	g_context->_location->_coords = newCoords;

	event._result = (MoveResult)(MOVE_SUCCEEDED | MOVE_END_TURN);
}

int moveObject(Map *map, Creature *obj, MapCoords avatar) {
	int dirmask = DIR_NONE;
	Direction dir;
	MapCoords new_coords = obj->getCoords();
	int slowed = 0;

	// Determine a direction depending on the object's movement behavior
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

	// Now, get a new x and y for the object
	if (dir)
		new_coords.move(dir, g_context->_location->_map);
	else
		return 0;

	// Figure out what method to use to tell if the object is getting slowed
	SlowedType slowedType = SLOWED_BY_TILE;
	if (obj->getType() == Object::CREATURE)
		slowedType = obj->getSlowedType();

	// Is the object slowed by terrain or by wind direction?
	switch (slowedType) {
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

	// See if the object needed to turn instead of move
	if (obj->setDirection(dir))
		return 0;

	// Was the object slowed?
	if (slowed)
		return 0;

	/**
	 * Set the new coordinates
	 */
	if (!(new_coords == obj->getCoords()) &&
	        !MAP_IS_OOB(map, new_coords)) {
		obj->setCoords(new_coords);
	}
	return 1;
}

int moveCombatObject(int act, Map *map, Creature *obj, MapCoords target) {
	MapCoords new_coords = obj->getCoords();
	int valid_dirs = map->getValidMoves(new_coords, obj->getTile());
	Direction dir;
	CombatAction action = (CombatAction)act;
	SlowedType slowedType = SLOWED_BY_TILE;
	int slowed = 0;

	// Fixed objects cannot move
	if (obj->getMovementBehavior() == MOVEMENT_FIXED)
		return 0;

	if (action == CA_FLEE) {
		// Run away from our target instead!
		dir = new_coords.pathAway(target, valid_dirs);

	} else {
		assertMsg(action == CA_ADVANCE, "action must be CA_ADVANCE or CA_FLEE");
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

	// Figure out what method to use to tell if the object is getting slowed
	if (obj->getType() == Object::CREATURE)
		slowedType = obj->getSlowedType();

	// Is the object slowed by terrain or by wind direction?
	switch (slowedType) {
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

	// If the object wan't slowed...
	if (!slowed) {
		// Set the new coordinates
		obj->setCoords(new_coords);
		return 1;
	}

	return 0;
}

void movePartyMember(MoveEvent &event) {
	CombatController *ct = dynamic_cast<CombatController *>(eventHandler->getController());
	CombatMap *cm = getCombatMap();
	assert(cm && ct);
	int member = ct->getFocus();
	MapCoords newCoords;
	PartyMemberVector *party = ct->getParty();

	event._result = MOVE_SUCCEEDED;

	// Find our new location
	newCoords = (*party)[member]->getCoords();
	newCoords.move(event._dir, g_context->_location->_map);

	if (MAP_IS_OOB(g_context->_location->_map, newCoords)) {
		bool sameExit = (!cm->isDungeonRoom() || (ct->getExitDir() == DIR_NONE) || (event._dir == ct->getExitDir()));
		if (sameExit) {
			// If in a win-or-lose battle and not camping, then it can be bad to flee while healthy
			if (ct->isWinOrLose() && !ct->isCamping()) {
				// A fully-healed party member fled from an evil creature :(
				if (ct->getCreature() && ct->getCreature()->isEvil() &&
				        g_context->_party->member(member)->getHp() == g_context->_party->member(member)->getMaxHp())
					g_context->_party->adjustKarma(KA_HEALTHY_FLED_EVIL);
			}

			ct->setExitDir(event._dir);
			g_context->_location->_map->removeObject((*party)[member]);
			(*party)[member] = nullptr;
			event._result = (MoveResult)(MOVE_EXIT_TO_PARENT | MOVE_MAP_CHANGE | MOVE_SUCCEEDED | MOVE_END_TURN);
			return;
		} else {
			event._result = (MoveResult)(MOVE_MUST_USE_SAME_EXIT | MOVE_END_TURN);
			return;
		}
	}

	int movementMask = g_context->_location->_map->getValidMoves((*party)[member]->getCoords(), (*party)[member]->getTile());
	if (!DIR_IN_MASK(event._dir, movementMask)) {
		event._result = (MoveResult)(MOVE_BLOCKED | MOVE_END_TURN);
		return;
	}

	// is the party member slowed?
	if (!slowedByTile(g_context->_location->_map->tileTypeAt(newCoords, WITHOUT_OBJECTS))) {
		// Move succeeded
		(*party)[member]->setCoords(newCoords);

		// Handle dungeon room triggers
		if (cm->isDungeonRoom()) {
			Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_prev->_map);
			assert(dungeon);
			Trigger *triggers = dungeon->_rooms[dungeon->_currentRoom]._triggers;

			int i;
			for (i = 0; i < 4; i++) {
				/*const Creature *m = creatures.getByTile(triggers[i].tile);*/

				/* FIXME: when a creature is created by a trigger, it can be created over and over and over...
				   how do we fix this? In the c64 version is appears that such triggers (on the world map)
				   wipe the creature table and replace it with the triggered creatures. Thus, retriggering
				   it will reset the creatures.
				   */
				MapCoords trigger(triggers[i].x, triggers[i].y, g_context->_location->_coords.z);

				// See if we're on a trigger
				if (newCoords == trigger) {
					MapCoords change1(triggers[i]._changeX1, triggers[i]._changeY1, g_context->_location->_coords.z),
					          change2(triggers[i].changeX2, triggers[i].changeY2, g_context->_location->_coords.z);

					/**
					 * Remove any previous annotations placed at our target coordinates
					 */
					g_context->_location->_map->_annotations->remove(g_context->_location->_map->_annotations->allAt(change1));
					g_context->_location->_map->_annotations->remove(g_context->_location->_map->_annotations->allAt(change2));

					// Change the tiles!
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
	} else {
		event._result = (MoveResult)(MOVE_SLOWED | MOVE_END_TURN);
		return;
	}
}

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

bool slowedByWind(int direction) {
	// 1 of 4 moves while trying to move into the wind succeeds
	if (direction == g_context->_windDirection)
		return (g_ultima->_saveGame->_moves % 4) != 0;
	// 1 of 4 moves while moving directly away from wind fails
	else if (direction == dirReverse((Direction) g_context->_windDirection))
		return (g_ultima->_saveGame->_moves % 4) == 3;
	else
		return false;
}

} // End of namespace Ultima4
} // End of namespace Ultima
