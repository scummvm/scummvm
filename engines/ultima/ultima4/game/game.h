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

#ifndef ULTIMA4_GAME_H
#define ULTIMA4_GAME_H

#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/controllers/game_controller.h"
#include "ultima/ultima4/core/observer.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/map/tileview.h"
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
void gameSetViewMode(ViewMode newMode);
void gameUpdateScreen();

/* spell functions */
void gameSpellEffect(int spell, int player, Sound sound);

/* action functions */
bool gamePeerCity(int city, void *data);
void peer(bool useGem = true);
bool fireAt(const Coords &coords, bool originAvatar);
Direction gameGetDirection();
uint32 gameTimeSinceLastCommand();

/* checking functions */
void gameCheckHullIntegrity();

/* creature functions */
bool creatureRangeAttack(const Coords &coords, Creature *m);
void gameCreatureCleanup();
bool gameSpawnCreature(const class Creature *m);
void gameFixupObjects(Map *map);
void gameDestroyAllCreatures();
void gameCreatureAttack(Creature *obj);

/* etc */
Common::String gameGetInput(int maxlen = 32);
int gameGetPlayer(bool canBeDisabled, bool canBeActivePlayer);
void gameGetPlayerForCommand(bool (*commandFn)(int player), bool canBeDisabled, bool canBeActivePlayer);
void gameDamageParty(int minDamage, int maxDamage);
void gameDamageShip(int minDamage, int maxDamage);
void gameSetActivePlayer(int player);
Std::vector<Coords> gameGetDirectionalActionPath(int dirmask, int validDirections, const Coords &origin, int minDistance, int maxDistance, bool (*blockedPredicate)(const Tile *tile), bool includeBlocked);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
