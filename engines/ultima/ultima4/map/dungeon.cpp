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

#include "ultima/ultima4/map/dungeon.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/item.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/game/stats.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/core/utils.h"

namespace Ultima {
namespace Ultima4 {

/**
 * Returns true if 'map' points to a dungeon map
 */
bool isDungeon(Map *punknown) {
	Dungeon *pd;
	if ((pd = dynamic_cast<Dungeon *>(punknown)) != NULL)
		return true;
	else
		return false;
}

/**
 * Returns the name of the dungeon
 */
Common::String Dungeon::getName() {
	return _name;
}

/**
 * Returns the dungeon token associated with the given dungeon tile
 */
DungeonToken Dungeon::tokenForTile(MapTile tile) {
	const static Common::String tileNames[] = {
		"brick_floor", "up_ladder", "down_ladder", "up_down_ladder", "chest",
		"unimpl_ceiling_hole", "unimpl_floor_hole", "magic_orb",
		"ceiling_hole", "fountain",
		"brick_floor", "dungeon_altar", "dungeon_door", "dungeon_room",
		"secret_door", "brick_wall", ""
	};

	const static Common::String fieldNames[] = { "poison_field", "energy_field", "fire_field", "sleep_field", "" };

	int i;
	Tile *t = _tileset->get(tile.getId());

	for (i = 0; !tileNames[i].empty(); i++) {
		if (t->getName() == tileNames[i])
			return DungeonToken(i << 4);
	}

	for (i = 0; !fieldNames[i].empty(); i++) {
		if (t->getName() == fieldNames[i])
			return DUNGEON_FIELD;
	}

	return (DungeonToken)0;
}

/**
 * Returns the dungeon token for the current location
 */
DungeonToken Dungeon::currentToken() {
	return tokenAt(g_context->_location->_coords);
}

/**
 * Return the dungeon sub-token associated with the given dungeon tile.
 *
 */
/**
 * Returns the dungeon sub-token for the current location
 */
unsigned char Dungeon::currentSubToken() {
	return subTokenAt(g_context->_location->_coords);
}

/**
 * Returns the dungeon token for the given coordinates
 */
DungeonToken Dungeon::tokenAt(MapCoords coords) {
	return tokenForTile(*getTileFromData(coords));
}

/**
 * Returns the dungeon sub-token for the given coordinates.  The
 * subtoken is encoded in the lower bits of the map raw data.  For
 * instance, for the raw value 0x91, returns FOUNTAIN_HEALING NOTE:
 * This function will always need type-casting to the token type
 * necessary
 */
unsigned char Dungeon::subTokenAt(MapCoords coords) {
	int index = coords.x + (coords.y * _width) + (_width * _height * coords.z);
	return _dataSubTokens[index];
}

/**
 * Handles 's'earching while in dungeons
 */
void dungeonSearch(void) {
	Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);
	DungeonToken token = dungeon->currentToken();
	Annotation::List a = dungeon->_annotations->allAt(g_context->_location->_coords);
	const ItemLocation *item;
	if (a.size() > 0)
		token = DUNGEON_CORRIDOR;

	screenMessage("Search...\n");

	switch (token) {
	case DUNGEON_MAGIC_ORB: /* magic orb */
		screenMessage("You find a Magical Ball...\nWho touches? ");
		dungeonTouchOrb();
		break;

	case DUNGEON_FOUNTAIN: /* fountains */
		dungeonDrinkFountain();
		break;

	default: {
		/* see if there is an item at the current location (stones on altars, etc.) */
		item = itemAtLocation(dungeon, g_context->_location->_coords);
		if (item) {
			if (*item->_isItemInInventory != NULL && (*item->_isItemInInventory)(item->_data))
				screenMessage("Nothing Here!\n");
			else {
				if (item->_name)
					screenMessage("You find...\n%s!\n", item->_name);
				(*item->_putItemInInventory)(item->_data);
			}
		} else
			screenMessage("\nYou find Nothing!\n");
	}

	break;
	}
}

/**
 * Drink from the fountain at the current location
 */
void dungeonDrinkFountain() {
	screenMessage("You find a Fountain.\nWho drinks? ");
	int player = gameGetPlayer(false, false);
	if (player == -1)
		return;

	Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);
	FountainType type = (FountainType) dungeon->currentSubToken();

	switch (type) {
	/* plain fountain */
	case FOUNTAIN_NORMAL:
		screenMessage("\nHmmm--No Effect!\n");
		break;

	/* healing fountain */
	case FOUNTAIN_HEALING:
		if (g_context->_party->member(player)->heal(HT_FULLHEAL))
			screenMessage("\nAhh-Refreshing!\n");
		else screenMessage("\nHmmm--No Effect!\n");
		break;

	/* acid fountain */
	case FOUNTAIN_ACID:
		g_context->_party->member(player)->applyDamage(100); /* 100 damage to drinker */
		screenMessage("\nBleck--Nasty!\n");
		break;

	/* cure fountain */
	case FOUNTAIN_CURE:
		if (g_context->_party->member(player)->heal(HT_CURE))
			screenMessage("\nHmmm--Delicious!\n");
		else screenMessage("\nHmmm--No Effect!\n");
		break;

	/* poison fountain */
	case FOUNTAIN_POISON:
		if (g_context->_party->member(player)->getStatus() != STAT_POISONED) {
			soundPlay(SOUND_POISON_DAMAGE);
			g_context->_party->member(player)->applyEffect(EFFECT_POISON);
			g_context->_party->member(player)->applyDamage(100); /* 100 damage to drinker also */
			screenMessage("\nArgh-Choke-Gasp!\n");
		} else screenMessage("\nHmm--No Effect!\n");
		break;

	default:
		ASSERT(0, "Invalid call to dungeonDrinkFountain: no fountain at current location");
	}
}

/**
 * Touch the magical ball at the current location
 */
void dungeonTouchOrb() {
	screenMessage("You find a Magical Ball...\nWho touches? ");
	int player = gameGetPlayer(false, false);
	if (player == -1)
		return;

	int stats = 0;
	int damage = 0;

	/* Get current position and find a replacement tile for it */
	Tile *orb_tile = g_context->_location->_map->_tileset->getByName("magic_orb");
	MapTile replacementTile(g_context->_location->getReplacementTile(g_context->_location->_coords, orb_tile));

	switch (g_context->_location->_map->_id) {
	case MAP_DECEIT:
		stats = STATSBONUS_INT;
		break;
	case MAP_DESPISE:
		stats = STATSBONUS_DEX;
		break;
	case MAP_DESTARD:
		stats = STATSBONUS_STR;
		break;
	case MAP_WRONG:
		stats = STATSBONUS_INT | STATSBONUS_DEX;
		break;
	case MAP_COVETOUS:
		stats = STATSBONUS_DEX | STATSBONUS_STR;
		break;
	case MAP_SHAME:
		stats = STATSBONUS_INT | STATSBONUS_STR;
		break;
	case MAP_HYTHLOTH:
		stats = STATSBONUS_INT | STATSBONUS_DEX | STATSBONUS_STR;
		break;
	default:
		break;
	}

	/* give stats bonuses */
	if (stats & STATSBONUS_STR) {
		screenMessage("Strength + 5\n");
		AdjustValueMax(g_context->_saveGame->_players[player]._str, 5, 50);
		damage += 200;
	}
	if (stats & STATSBONUS_DEX) {
		screenMessage("Dexterity + 5\n");
		AdjustValueMax(g_context->_saveGame->_players[player]._dex, 5, 50);
		damage += 200;
	}
	if (stats & STATSBONUS_INT) {
		screenMessage("Intelligence + 5\n");
		AdjustValueMax(g_context->_saveGame->_players[player]._intel, 5, 50);
		damage += 200;
	}

	/* deal damage to the party member who touched the orb */
	g_context->_party->member(player)->applyDamage(damage);
	/* remove the orb from the map */
	g_context->_location->_map->_annotations->add(g_context->_location->_coords, replacementTile);
}

/**
 * Handles dungeon traps
 */
bool dungeonHandleTrap(TrapType trap) {
	Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);
	switch ((TrapType)dungeon->currentSubToken()) {
	case TRAP_WINDS:
		screenMessage("\nWinds!\n");
		g_context->_party->quenchTorch();
		break;
	case TRAP_FALLING_ROCK:
		// Treat falling rocks and pits like bomb traps
		// XXX: That's a little harsh.
		screenMessage("\nFalling Rocks!\n");
		g_context->_party->applyEffect(EFFECT_LAVA);
		break;
	case TRAP_PIT:
		screenMessage("\nPit!\n");
		g_context->_party->applyEffect(EFFECT_LAVA);
		break;
	default:
		break;
	}

	return true;
}

/**
 * Returns true if a ladder-up is found at the given coordinates
 */
bool Dungeon::ladderUpAt(MapCoords coords) {
	Annotation::List a = _annotations->allAt(coords);

	if (tokenAt(coords) == DUNGEON_LADDER_UP ||
	        tokenAt(coords) == DUNGEON_LADDER_UPDOWN)
		return true;

	if (a.size() > 0) {
		Annotation::List::iterator i;
		for (i = a.begin(); i != a.end(); i++) {
			if (i->getTile() == _tileset->getByName("up_ladder")->getId())
				return true;
		}
	}
	return false;
}

/**
 * Returns true if a ladder-down is found at the given coordinates
 */
bool Dungeon::ladderDownAt(MapCoords coords) {
	Annotation::List a = _annotations->allAt(coords);

	if (tokenAt(coords) == DUNGEON_LADDER_DOWN ||
	        tokenAt(coords) == DUNGEON_LADDER_UPDOWN)
		return true;

	if (a.size() > 0) {
		Annotation::List::iterator i;
		for (i = a.begin(); i != a.end(); i++) {
			if (i->getTile() == _tileset->getByName("down_ladder")->getId())
				return true;
		}
	}
	return false;
}

bool Dungeon::validTeleportLocation(MapCoords coords) {
	MapTile *tile = tileAt(coords, WITH_OBJECTS);
	return tokenForTile(*tile) == DUNGEON_CORRIDOR;
}

} // End of namespace Ultima4
} // End of namespace Ultima
