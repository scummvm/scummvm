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

#ifndef ULTIMA4_GAME_GAME_H
#define ULTIMA4_GAME_GAME_H

#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/controllers/game_controller.h"
#include "ultima/ultima4/core/observer.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/views/tileview.h"
#include "ultima/ultima4/sound/sound.h"

namespace Ultima {
namespace Ultima4 {

class Map;
struct Portal;
class Creature;
class Location;
class MoveEvent;
class Party;
class PartyEvent;
class PartyMember;

typedef enum {
	VIEW_NORMAL,
	VIEW_GEM,
	VIEW_RUNE,
	VIEW_DUNGEON,
	VIEW_DEAD,
	VIEW_CODEX,
	VIEW_MIXTURES
} ViewMode;

/* map and screen functions */

/**
 * Sets the view mode.
 */
void gameSetViewMode(ViewMode newMode);
void gameUpdateScreen();

/* spell functions */
void gameSpellEffect(int spell, int player, Sound sound);

/* action functions */
/**
 * Peers at a city from A-P (Lycaeum telescope) and functions like a gem
 */
bool gamePeerCity(int city, void *data);

/**
 * Peers at a gem
 */
void peer(bool useGem = true);
bool fireAt(const Coords &coords, bool originAvatar);
Direction gameGetDirection();
uint32 gameTimeSinceLastCommand();

/* checking functions */
/**
 * Checks the hull integrity of the ship and handles
 * the ship sinking, if necessary
 */
void gameCheckHullIntegrity();

/* creature functions */
/**
 * Performs a ranged attack for the creature at x,y on the world map
 */
bool creatureRangeAttack(const Coords &coords, Creature *m);
void gameCreatureCleanup();

/**
 * Spawns a creature (m) just offscreen of the avatar.
 * If (m==nullptr) then it finds its own creature to spawn and spawns it.
 */
bool gameSpawnCreature(const class Creature *m);

/**
 * Fixes objects initially loaded by saveGameMonstersRead,
 * and alters movement behavior accordingly to match the creature
 */
void gameFixupObjects(Map *map);

/**
 * Destroys all creatures on the current map.
 */
void gameDestroyAllCreatures();

/**
 * Handles what happens when a creature attacks you
 */
void gameCreatureAttack(Creature *obj);

/* etc */
Common::String gameGetInput(int maxlen = 32);
int gameGetPlayer(bool canBeDisabled, bool canBeActivePlayer);
void gameGetPlayerForCommand(bool (*commandFn)(int player), bool canBeDisabled, bool canBeActivePlayer);

/**
 * Deals an amount of damage between 'minDamage' and 'maxDamage'
 * to each party member, with a 50% chance for each member to
 * avoid the damage.  If (minDamage == -1) or (minDamage >= maxDamage),
 * deals 'maxDamage' damage to each member.
 */
void gameDamageParty(int minDamage, int maxDamage);

/**
 * Deals an amount of damage between 'minDamage' and 'maxDamage'
 * to the ship.  If (minDamage == -1) or (minDamage >= maxDamage),
 * deals 'maxDamage' damage to the ship.
 */
void gameDamageShip(int minDamage, int maxDamage);

/**
 * Sets (or unsets) the active player
 */
void gameSetActivePlayer(int player);

/**
 * Gets the path of coordinates for an action.  Each tile in the
 * direction specified by dirmask, between the minimum and maximum
 * distances given, is included in the path, until blockedPredicate
 * fails.  If a tile is blocked, that tile is included in the path
 * only if includeBlocked is true.
 */
Std::vector<Coords> gameGetDirectionalActionPath(int dirmask, int validDirections, const Coords &origin, int minDistance, int maxDistance, bool (*blockedPredicate)(const Tile *tile), bool includeBlocked);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
