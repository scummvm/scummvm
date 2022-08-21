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

#ifndef ULTIMA4_MAP_MAPMGR_H
#define ULTIMA4_MAP_MAPMGR_H

#include "ultima/ultima4/map/map.h"

namespace Ultima {
namespace Ultima4 {

class City;
class ConfigElement;
class Debug;
class Dungeon;
struct PersonRole;
struct Portal;
class Shrine;

/*
 * The map manager is responsible for loading and keeping track of the
 * various maps.
 */

#define MAP_NONE 255
#define MAP_WORLD 0
#define MAP_CASTLE_OF_LORD_BRITISH 1
#define MAP_LYCAEUM 2
#define MAP_EMPATH_ABBEY 3
#define MAP_SERPENTS_HOLD 4
#define MAP_MOONGLOW 5
#define MAP_BRITAIN 6
#define MAP_JHELOM 7
#define MAP_YEW 8
#define MAP_MINOC 9
#define MAP_TRINSIC 10
#define MAP_SKARABRAE 11
#define MAP_MAGINCIA 12
#define MAP_PAWS 13
#define MAP_BUCCANEERS_DEN 14
#define MAP_VESPER 15
#define MAP_COVE 16
#define MAP_DECEIT 17
#define MAP_DESPISE 18
#define MAP_DESTARD 19
#define MAP_WRONG 20
#define MAP_COVETOUS 21
#define MAP_SHAME 22
#define MAP_HYTHLOTH 23
#define MAP_ABYSS 24
#define MAP_SHRINE_HONESTY 25
#define MAP_SHRINE_COMPASSION 26
#define MAP_SHRINE_VALOR 27
#define MAP_SHRINE_JUSTICE 28
#define MAP_SHRINE_SACRIFICE 29
#define MAP_SHRINE_HONOR 30
#define MAP_SHRINE_SPIRITUALITY 31
#define MAP_SHRINE_HUMILITY 32
#define MAP_BRICK_CON 33
#define MAP_BRIDGE_CON 34
#define MAP_BRUSH_CON 35
#define MAP_CAMP_CON 36
#define MAP_DNG0_CON 37
#define MAP_DNG1_CON 38
#define MAP_DNG2_CON 39
#define MAP_DNG3_CON 40
#define MAP_DNG4_CON 41
#define MAP_DNG5_CON 42
#define MAP_DNG6_CON 43
#define MAP_DUNGEON_CON 44
#define MAP_FOREST_CON 45
#define MAP_GRASS_CON 46
#define MAP_HILL_CON 47
#define MAP_INN_CON 48
#define MAP_MARSH_CON 49
#define MAP_SHIPSEA_CON 50
#define MAP_SHIPSHIP_CON 51
#define MAP_SHIPSHOR_CON 52
#define MAP_SHORE_CON 53
#define MAP_SHORSHIP_CON 54
#define MAP_CAMP_DNG 55
#define MAP_CASTLE_OF_LORD_BRITISH2 100
#define MAP_SCUMMVM 101

/**
 * The map manager singleton that keeps track of all the maps.
 */
class MapMgr {
public:
	static MapMgr *getInstance();
	static void destroy();

	Map *get(MapId id);
	Map *initMap(Map::Type type);
	void unloadMap(MapId id);

private:
	MapMgr();
	~MapMgr();

	void registerMap(Map *map);

	Map *initMapFromConf(const ConfigElement &mapConf);
	void initCityFromConf(const ConfigElement &cityConf, City *city);
	PersonRole *initPersonRoleFromConf(const ConfigElement &cityConf);
	Portal *initPortalFromConf(const ConfigElement &portalConf);
	void initShrineFromConf(const ConfigElement &shrineConf, Shrine *shrine);
	void initDungeonFromConf(const ConfigElement &dungeonConf, Dungeon *dungeon);
	void initDungeonRoom(Dungeon *dng, int room);
	void createMoongateFromConf(const ConfigElement &moongateConf);
	int initCompressedChunkFromConf(const ConfigElement &compressedChunkConf);

	static MapMgr *_instance;
	Std::vector<Map *> _mapList;
};

#define mapMgr (MapMgr::getInstance())

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
