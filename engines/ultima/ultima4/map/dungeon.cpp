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
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/item.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/map/tilemap.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/views/stats.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/core/utils.h"

namespace Ultima {
namespace Ultima4 {

bool isDungeon(Map *punknown) {
	Dungeon *pd;
	if ((pd = dynamic_cast<Dungeon *>(punknown)) != nullptr)
		return true;
	else
		return false;
}

/*-------------------------------------------------------------------*/

void DngRoom::load(Common::SeekableReadStream &s) {
	int i;

	s.read(_creatureTiles, 16);
	for (i = 0; i < 16; ++i)
		_creatureStart[i].x = s.readByte();
	for (i = 0; i < 16; ++i)
		_creatureStart[i].y = s.readByte();

	#define READ_DIR(DIR, XY) \
		for (i = 0; i < 8; ++i) \
			_partyStart[i][DIR].XY = s.readByte()

	READ_DIR(DIR_NORTH, x);
	READ_DIR(DIR_NORTH, y);
	READ_DIR(DIR_EAST, x);
	READ_DIR(DIR_EAST, y);
	READ_DIR(DIR_SOUTH, x);
	READ_DIR(DIR_SOUTH, y);
	READ_DIR(DIR_WEST, x);
	READ_DIR(DIR_WEST, y);

	#undef READ_DIR
}

void DngRoom::hythlothFix7() {
	int i;

	// Update party start positions when entering from the east
	const byte X1[8] = { 0x8, 0x8, 0x9, 0x9, 0x9, 0xA, 0xA, 0xA },
		Y1[8] = { 0x3, 0x2, 0x3, 0x2, 0x1, 0x3, 0x2, 0x1 };

	for (i = 0; i < 8; ++i)
		_partyStart[i]._eastStart.x = X1[i];
	for (i = 0; i < 8; ++i)
		_partyStart[i]._eastStart.y = Y1[i];

	// Update party start positions when entering from the south
	const byte X2[8] = { 0x3, 0x2, 0x3, 0x2, 0x1, 0x3, 0x2, 0x1 },
		Y2[8] = { 0x8, 0x8, 0x9, 0x9, 0x9, 0xA, 0xA, 0xA };

	for (i = 0; i < 8; ++i)
		_partyStart[i]._southStart.x = X2[i];
	for (i = 0; i < 8; ++i)
		_partyStart[i]._southStart.y = Y2[i];
}

void DngRoom::hythlothFix9() {
	int i;

	// Update the starting position of monsters 7, 8, and 9
	const byte X1[3] = { 0x4, 0x6, 0x5 },
		Y1[3] = { 0x5, 0x5, 0x6 };

	for (i = 0; i < 3; ++i)
		_creatureStart[i + 7].x = X1[i];
	for (i = 0; i < 3; ++i)
		_creatureStart[i + 7].y = Y1[i];

	// Update party start positions when entering from the west
	const byte X2[8] = { 0x2, 0x2, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0 },
		Y2[8] = { 0x9, 0x8, 0x9, 0x8, 0x7, 0x9, 0x8, 0x7 };

	for (i = 0; i < 8; ++i)
		_partyStart[i]._westStart.x = X2[i];
	for (i = 0; i < 8; ++i)
		_partyStart[i]._westStart.y = Y2[i];

	// Update the map data, moving the chest to the center of the room,
	// and removing the walls at the lower-left corner thereby creating
	// a connection to room 8
	const Coords tile[6] = {
		Coords(5, 5, 0x3C),  // Chest
		Coords(0, 7, 0x16),  // Floor
		Coords(1, 7, 0x16),
		Coords(0, 8, 0x16),
		Coords(1, 8, 0x16),
		Coords(0, 9, 0x16)
	};

	for (i = 0; i < 6; ++i) {
		const int index = (tile[i].y * CON_WIDTH) + tile[i].x;
		_mapData[index] = g_tileMaps->get("base")->translate(tile[i].z);
	}
}

/*-------------------------------------------------------------------*/

Dungeon::Dungeon() : _nRooms(0), _rooms(nullptr),
		_roomMaps(nullptr), _currentRoom(0) {
	Common::fill(&_partyStartX[0], &_partyStartX[8], 0);
	Common::fill(&_partyStartY[0], &_partyStartY[8], 0);
}

Common::String Dungeon::getName() {
	return _name;
}

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
	Tile *t = _tileSet->get(tile.getId());

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

DungeonToken Dungeon::currentToken() {
	return tokenAt(g_context->_location->_coords);
}

byte Dungeon::currentSubToken() {
	return subTokenAt(g_context->_location->_coords);
}

DungeonToken Dungeon::tokenAt(MapCoords coords) {
	return tokenForTile(*getTileFromData(coords));
}

byte Dungeon::subTokenAt(MapCoords coords) {
	int index = coords.x + (coords.y * _width) + (_width * _height * coords.z);
	return _dataSubTokens[index];
}

void dungeonSearch(void) {
	Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);
	assert(dungeon);

	DungeonToken token = dungeon->currentToken();
	Annotation::List a = dungeon->_annotations->allAt(g_context->_location->_coords);
	const ItemLocation *item;
	if (a.size() > 0)
		token = DUNGEON_CORRIDOR;

	g_screen->screenMessage("Search...\n");

	switch (token) {
	case DUNGEON_MAGIC_ORB: /* magic orb */
		g_screen->screenMessage("You find a Magical Ball...\nWho touches? ");
		dungeonTouchOrb();
		break;

	case DUNGEON_FOUNTAIN: /* fountains */
		dungeonDrinkFountain();
		break;

	default: {
		/* see if there is an item at the current location (stones on altars, etc.) */
		item = g_items->itemAtLocation(dungeon, g_context->_location->_coords);
		if (item) {
			if (item->_isItemInInventory && (g_items->*(item->_isItemInInventory))(item->_data)) {
				g_screen->screenMessage("Nothing Here!\n");
			} else {
				if (item->_name)
					g_screen->screenMessage("You find...\n%s!\n", item->_name);
				(g_items->*(item->_putItemInInventory))(item->_data);
			}
		} else {
			g_screen->screenMessage("\nYou find Nothing!\n");
		}
	}

	break;
	}
}

void dungeonDrinkFountain() {
	g_screen->screenMessage("You find a Fountain.\nWho drinks? ");
	int player = gameGetPlayer(false, false);
	if (player == -1)
		return;

	Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);
	assert(dungeon);
	FountainType type = (FountainType) dungeon->currentSubToken();

	switch (type) {
	/* plain fountain */
	case FOUNTAIN_NORMAL:
		g_screen->screenMessage("\nHmmm--No Effect!\n");
		break;

	/* healing fountain */
	case FOUNTAIN_HEALING:
		if (g_context->_party->member(player)->heal(HT_FULLHEAL))
			g_screen->screenMessage("\nAhh-Refreshing!\n");
		else
			g_screen->screenMessage("\nHmmm--No Effect!\n");
		break;

	/* acid fountain */
	case FOUNTAIN_ACID:
		g_context->_party->member(player)->applyDamage(100); /* 100 damage to drinker */
		g_screen->screenMessage("\nBleck--Nasty!\n");
		break;

	/* cure fountain */
	case FOUNTAIN_CURE:
		if (g_context->_party->member(player)->heal(HT_CURE))
			g_screen->screenMessage("\nHmmm--Delicious!\n");
		else
			g_screen->screenMessage("\nHmmm--No Effect!\n");
		break;

	/* poison fountain */
	case FOUNTAIN_POISON:
		if (g_context->_party->member(player)->getStatus() != STAT_POISONED) {
			soundPlay(SOUND_POISON_DAMAGE);
			g_context->_party->member(player)->applyEffect(EFFECT_POISON);
			g_context->_party->member(player)->applyDamage(100); /* 100 damage to drinker also */
			g_screen->screenMessage("\nArgh-Choke-Gasp!\n");
		} else {
			g_screen->screenMessage("\nHmm--No Effect!\n");
		}
		break;

	default:
		error("Invalid call to dungeonDrinkFountain: no fountain at current location");
	}
}

void dungeonTouchOrb() {
	g_screen->screenMessage("You find a Magical Ball...\nWho touches? ");
	int player = gameGetPlayer(false, false);
	if (player == -1)
		return;

	int stats = 0;
	int damage = 0;

	/* Get current position and find a replacement tile for it */
	Tile *orb_tile = g_context->_location->_map->_tileSet->getByName("magic_orb");
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
		g_screen->screenMessage("Strength + 5\n");
		AdjustValueMax(g_ultima->_saveGame->_players[player]._str, 5, 50);
		damage += 200;
	}
	if (stats & STATSBONUS_DEX) {
		g_screen->screenMessage("Dexterity + 5\n");
		AdjustValueMax(g_ultima->_saveGame->_players[player]._dex, 5, 50);
		damage += 200;
	}
	if (stats & STATSBONUS_INT) {
		g_screen->screenMessage("Intelligence + 5\n");
		AdjustValueMax(g_ultima->_saveGame->_players[player]._intel, 5, 50);
		damage += 200;
	}

	/* deal damage to the party member who touched the orb */
	g_context->_party->member(player)->applyDamage(damage);
	/* remove the orb from the map */
	g_context->_location->_map->_annotations->add(g_context->_location->_coords, replacementTile);
}

bool dungeonHandleTrap(TrapType trap) {
	Dungeon *dungeon = dynamic_cast<Dungeon *>(g_context->_location->_map);
	assert(dungeon);

	switch ((TrapType)dungeon->currentSubToken()) {
	case TRAP_WINDS:
		g_screen->screenMessage("\nWinds!\n");
		g_context->_party->quenchTorch();
		break;
	case TRAP_FALLING_ROCK:
		// Treat falling rocks and pits like bomb traps
		// XXX: That's a little harsh.
		g_screen->screenMessage("\nFalling Rocks!\n");
		g_context->_party->applyEffect(EFFECT_LAVA);
		break;
	case TRAP_PIT:
		g_screen->screenMessage("\nPit!\n");
		g_context->_party->applyEffect(EFFECT_LAVA);
		break;
	default:
		break;
	}

	return true;
}

bool Dungeon::ladderUpAt(MapCoords coords) {
	Annotation::List a = _annotations->allAt(coords);

	if (tokenAt(coords) == DUNGEON_LADDER_UP ||
	        tokenAt(coords) == DUNGEON_LADDER_UPDOWN)
		return true;

	if (a.size() > 0) {
		Annotation::List::iterator i;
		for (i = a.begin(); i != a.end(); i++) {
			if (i->getTile() == _tileSet->getByName("up_ladder")->getId())
				return true;
		}
	}
	return false;
}

bool Dungeon::ladderDownAt(MapCoords coords) {
	Annotation::List a = _annotations->allAt(coords);

	if (tokenAt(coords) == DUNGEON_LADDER_DOWN ||
	        tokenAt(coords) == DUNGEON_LADDER_UPDOWN)
		return true;

	if (a.size() > 0) {
		Annotation::List::iterator i;
		for (i = a.begin(); i != a.end(); i++) {
			if (i->getTile() == _tileSet->getByName("down_ladder")->getId())
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
