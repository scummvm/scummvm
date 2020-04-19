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

#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/game.h"
#include "ultima/ultima4/game/item.h"
#include "ultima/ultima4/game/object.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/game/spell.h"
#include "ultima/ultima4/game/stats.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/mapmgr.h"

namespace Ultima {
namespace Ultima4 {

void SaveGame::save(Common::WriteStream *stream) {
	Common::Serializer ser(nullptr, stream);

	if (g_context->_location) {
		if (g_context->_location->_prev) {
			_x = g_context->_location->_coords.x;
			_y = g_context->_location->_coords.y;
			_dngLevel = g_context->_location->_coords.z;
			_dngX = g_context->_location->_prev->_coords.x;
			_dngY = g_context->_location->_prev->_coords.y;
		} else {
			_x = g_context->_location->_coords.x;
			_y = g_context->_location->_coords.y;
			_dngLevel = g_context->_location->_coords.z;
		}

		_location = g_context->_location->_map->_id;
	}

	synchronize(ser);

	/*
	 * Save monsters
	 */

	if (g_context->_location) {
		// fix creature animations. This was done for compatibility with u4dos,
		// so may be redundant now
		g_context->_location->_map->resetObjectAnimations();
		g_context->_location->_map->fillMonsterTable();

		SaveGameMonsterRecord::synchronize(g_context->_location->_map->_monsterTable, ser);
	} else {
		SaveGameMonsterRecord::synchronize(nullptr, ser);
	}

	/**
	 * Write dungeon info
	 */
	if (g_context->_location && g_context->_location->_context & CTX_DUNGEON) {
		uint x, y, z;

		typedef Std::map<const Creature *, int, Std::PointerHash> DngCreatureIdMap;
		static DngCreatureIdMap id_map;

		/**
		 * Map creatures to u4dos dungeon creature Ids
		 */
		if (id_map.size() == 0) {
			id_map[creatureMgr->getById(RAT_ID)] = 1;
			id_map[creatureMgr->getById(BAT_ID)] = 2;
			id_map[creatureMgr->getById(GIANT_SPIDER_ID)] = 3;
			id_map[creatureMgr->getById(GHOST_ID)] = 4;
			id_map[creatureMgr->getById(SLIME_ID)] = 5;
			id_map[creatureMgr->getById(TROLL_ID)] = 6;
			id_map[creatureMgr->getById(GREMLIN_ID)] = 7;
			id_map[creatureMgr->getById(MIMIC_ID)] = 8;
			id_map[creatureMgr->getById(REAPER_ID)] = 9;
			id_map[creatureMgr->getById(INSECT_SWARM_ID)] = 10;
			id_map[creatureMgr->getById(GAZER_ID)] = 11;
			id_map[creatureMgr->getById(PHANTOM_ID)] = 12;
			id_map[creatureMgr->getById(ORC_ID)] = 13;
			id_map[creatureMgr->getById(SKELETON_ID)] = 14;
			id_map[creatureMgr->getById(ROGUE_ID)] = 15;
		}

		for (z = 0; z < g_context->_location->_map->_levels; z++) {
			for (y = 0; y < g_context->_location->_map->_height; y++) {
				for (x = 0; x < g_context->_location->_map->_width; x++) {
					byte tile = g_context->_location->_map->translateToRawTileIndex(*g_context->_location->_map->getTileFromData(MapCoords(x, y, z)));
					Object *obj = g_context->_location->_map->objectAt(MapCoords(x, y, z));

					/**
					 * Add the creature to the tile
					 */
					if (obj && obj->getType() == Object::CREATURE) {
						const Creature *m = dynamic_cast<Creature *>(obj);
						DngCreatureIdMap::iterator m_id = id_map.find(m);
						if (m_id != id_map.end())
							tile |= m_id->_value;
					}

					// Write the tile
					stream->writeByte(tile);
				}
			}
		}


		/**
		 * Write out monsters
		 */

		// fix creature animations so they are compatible with u4dos.
		// This may be redundant now for ScummVM
		g_context->_location->_prev->_map->resetObjectAnimations();
		g_context->_location->_prev->_map->fillMonsterTable(); /* fill the monster table so we can save it */

		SaveGameMonsterRecord::synchronize(g_context->_location->_prev->_map->_monsterTable, ser);
	}
}

void SaveGame::load(Common::SeekableReadStream *stream) {
	Common::Serializer *ser = nullptr;
	if (stream) {
		ser = new Common::Serializer(stream, nullptr);
		synchronize(*ser);
	}

	// initialize our party
	if (g_context->_party) {
		g_context->_party->deleteObserver(g_game);
	}
	g_context->_party = new Party(this);
	g_context->_party->addObserver(g_game);

	// Delete any prior map
	if (g_context->_location && g_context->_location->_prev) {
		g_context->_location->_prev->deleteObserver(g_game);
		delete g_context->_location->_prev;
		g_context->_location->_prev = nullptr;
	}
	if (g_context->_location) {
		g_context->_location->deleteObserver(g_game);
		delete g_context->_location;
		g_context->_location = nullptr;
	}

	// set the map to the world map
	g_game->setMap(mapMgr->get(MAP_WORLD), 0, nullptr);
	g_context->_location->_map->clearObjects();

	// initialize our start location
	Map *map = mapMgr->get(MapId(_location));

	// if our map is not the world map, then load our map
	if (map->_type != Map::WORLD)
		g_game->setMap(map, 1, nullptr);
	else
		// initialize the moons (must be done from the world map)
		g_game->initMoons();


	/**
	 * Translate info from the savegame to something we can use
	 */
	if (g_context->_location->_prev) {
		g_context->_location->_coords = MapCoords(_x, _y, _dngLevel);
		g_context->_location->_prev->_coords = MapCoords(_dngX, _dngY);
	} else {
		g_context->_location->_coords = MapCoords(_x, _y, (int)_dngLevel);
	}

	/**
	 * Fix the coordinates if they're out of bounds.  This happens every
	 * time on the world map because (z == -1) is no longer valid.
	 * To maintain compatibility with u4dos, this value gets translated
	 * when the game is saved and loaded
	 */
	if (MAP_IS_OOB(g_context->_location->_map, g_context->_location->_coords))
		g_context->_location->_coords.putInBounds(g_context->_location->_map);

	// load in creatures
	if (ser)
		SaveGameMonsterRecord::synchronize(g_context->_location->_map->_monsterTable, *ser);
	gameFixupObjects(g_context->_location->_map);

	/* we have previous creature information as well, load it! */
	if (g_context->_location->_prev) {
		if (ser)
			SaveGameMonsterRecord::synchronize(g_context->_location->_prev->_map->_monsterTable, *ser);
		gameFixupObjects(g_context->_location->_prev->_map);
	}

	spellSetEffectCallback(&gameSpellEffect);
	itemSetDestroyAllCreaturesCallback(&gameDestroyAllCreatures);

	g_context->_stats->resetReagentsMenu();

	/* add some observers */
	g_context->_aura->addObserver(g_context->_stats);
	g_context->_party->addObserver(g_context->_stats);

	g_game->initScreenWithoutReloadingState();

	delete ser;
}

void SaveGame::newGame() {
	// Most default state has already been set up by the IntroController.
	// Call the load method with no stream to handle pre-game setup
	load(nullptr);
}

void SaveGame::synchronize(Common::Serializer &s) {
	int i;

	s.syncAsUint32LE(_unknown1);
	s.syncAsUint32LE(_moves);

	for (i = 0; i < 8; ++i)
		_players[i].synchronize(s);

	s.syncAsUint32LE(_food);
	s.syncAsUint16LE(_gold);

	for (i = 0; i < 8; ++i)
		s.syncAsUint16LE(_karma[i]);

	s.syncAsUint16LE(_torches);
	s.syncAsUint16LE(_gems);
	s.syncAsUint16LE(_keys);
	s.syncAsUint16LE(_sextants);

	for (i = 0; i < ARMR_MAX; ++i)
		s.syncAsUint16LE(_armor[i]);

	for (i = 0; i < WEAP_MAX; ++i)
		s.syncAsUint16LE(_weapons[i]);

	for (i = 0; i < REAG_MAX; ++i)
		s.syncAsUint16LE(_reagents[i]);

	for (i = 0; i < SPELL_MAX; ++i)
		s.syncAsUint16LE(_mixtures[i]);

	s.syncAsUint16LE(_items);
	s.syncAsByte(_x);
	s.syncAsByte(_y);
	s.syncAsByte(_stones);
	s.syncAsByte(_runes);
	s.syncAsUint16LE(_members);
	s.syncAsUint16LE(_transport);
	s.syncAsUint16LE(_balloonState);
	s.syncAsUint16LE(_trammelPhase);
	s.syncAsUint16LE(_feluccaPhase);
	s.syncAsUint16LE(_shipHull);
	s.syncAsUint16LE(_lbIntro);
	s.syncAsUint16LE(_lastCamp);
	s.syncAsUint16LE(_lastReagent);
	s.syncAsUint16LE(_lastMeditation);
	s.syncAsUint16LE(_lastVirtue);
	s.syncAsByte(_dngX);
	s.syncAsByte(_dngY);
	s.syncAsUint16LE(_orientation);
	s.syncAsUint16LE(_dngLevel);
	s.syncAsUint16LE(_location);
}

void SaveGame::init(const SaveGamePlayerRecord *avatarInfo) {
	int i;

	_unknown1 = 0;
	_moves = 0;

	_players[0] = *avatarInfo;
	for (i = 1; i < 8; ++i)
		_players[i].init();

	_food = 0;
	_gold = 0;

	for (i = 0; i < 8; ++i)
		_karma[i] = 20;

	_torches = 0;
	_gems = 0;
	_keys = 0;
	_sextants = 0;

	for (i = 0; i < ARMR_MAX; ++i)
		_armor[i] = 0;

	for (i = 0; i < WEAP_MAX; ++i)
		_weapons[i] = 0;

	for (i = 0; i < REAG_MAX; ++i)
		_reagents[i] = 0;

	for (i = 0; i < SPELL_MAX; ++i)
		_mixtures[i] = 0;

	_items = 0;
	_x = 0;
	_y = 0;
	_stones = 0;
	_runes = 0;
	_members = 1;
	_transport = 0x1f;
	_balloonState = 0;
	_trammelPhase = 0;
	_feluccaPhase = 0;
	_shipHull = 50;
	_lbIntro = 0;
	_lastCamp = 0;
	_lastReagent = 0;
	_lastMeditation = 0;
	_lastVirtue = 0;
	_dngX = 0;
	_dngY = 0;
	_orientation = 0;
	_dngLevel = 0xFFFF;
	_location = 0;
}

/*-------------------------------------------------------------------*/

void SaveGamePlayerRecord::synchronize(Common::Serializer &s) {
	s.syncAsUint16LE(_hp);
	s.syncAsUint16LE(_hpMax);
	s.syncAsUint16LE(_xp);
	s.syncAsUint16LE(_str);
	s.syncAsUint16LE(_dex);
	s.syncAsUint16LE(_intel);
	s.syncAsUint16LE(_mp);
	s.syncAsUint16LE(_unknown);
	s.syncAsUint16LE(_weapon);
	s.syncAsUint16LE(armor);
	s.syncBytes((byte *)name, 16);
	s.syncAsByte(_sex);
	s.syncAsByte(_class);
	s.syncAsByte(_status);
}

void SaveGamePlayerRecord::init() {
	int i;

	_hp = 0;
	_hpMax = 0;
	_xp = 0;
	_str = 0;
	_dex = 0;
	_intel = 0;
	_mp = 0;
	_unknown = 0;
	_weapon = WEAP_HANDS;
	armor = ARMR_NONE;

	for (i = 0; i < 16; ++i)
		name[i] = '\0';

	_sex = SEX_MALE;
	_class = CLASS_MAGE;
	_status = STAT_GOOD;
}

void SaveGameMonsterRecord::synchronize(SaveGameMonsterRecord *monsterTable, Common::Serializer &s) {
	int i;
	const uint32 IDENT = MKTAG('M', 'O', 'N', 'S');
	uint32 val = IDENT;

	s.syncAsUint32BE(val);
	if (s.isLoading() && val != IDENT)
		error("Invalid savegame");

	if (s.isSaving() && !monsterTable) {
		int dataSize = MONSTERTABLE_SIZE * 8;
		byte b = 0;
		while (dataSize-- > 0)
			s.syncAsByte(b);

		return;
	}

	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._tile);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._x);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._y);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._prevTile);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._prevX);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._prevY);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._unused1);
	for (i = 0; i < MONSTERTABLE_SIZE; ++i)
		s.syncAsByte(monsterTable[i]._unused2);
}

} // End of namespace Ultima4
} // End of namespace Ultima
