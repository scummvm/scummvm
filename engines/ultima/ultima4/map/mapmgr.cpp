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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/map/annotation.h"
#include "ultima/ultima4/map/city.h"
#include "ultima/ultima4/controllers/combat_controller.h"
#include "ultima/ultima4/map/dungeon.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/map/maploader.h"
#include "ultima/ultima4/map/mapmgr.h"
#include "ultima/ultima4/game/item.h"
#include "ultima/ultima4/game/moongate.h"
#include "ultima/ultima4/game/person.h"
#include "ultima/ultima4/game/portal.h"
#include "ultima/ultima4/map/shrine.h"
#include "ultima/ultima4/map/tilemap.h"
#include "ultima/ultima4/map/tileset.h"
#include "ultima/ultima4/map/xml_map.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/ultima4/core/config.h"

namespace Ultima {
namespace Ultima4 {

MapMgr *MapMgr::_instance = nullptr;

MapMgr *MapMgr::getInstance() {
	if (_instance == nullptr)
		_instance = new MapMgr();
	return _instance;
}

void MapMgr::destroy() {
	if (_instance != nullptr) {
		delete _instance;
		_instance = nullptr;
	}
}

MapMgr::MapMgr() {
	const Config *config = Config::getInstance();
	Map *map;

	Std::vector<ConfigElement> maps = config->getElement("maps").getChildren();
	for (Std::vector<ConfigElement>::iterator i = maps.begin(); i != maps.end(); i++) {
		map = initMapFromConf(*i);

		// Map actually gets loaded later, when it's needed
		registerMap(map);
	}
}

MapMgr::~MapMgr() {
	for (Std::vector<Map *>::iterator i = _mapList.begin(); i != _mapList.end(); i++)
		delete *i;
}

void MapMgr::unloadMap(MapId id) {
	delete _mapList[id];
	const Config *config = Config::getInstance();
	Std::vector<ConfigElement> maps = config->getElement("maps").getChildren();

	for (Std::vector<ConfigElement>::const_iterator i = maps.begin(); i != maps.end(); ++i) {
		if (id == static_cast<MapId>((*i).getInt("id"))) {
			Map *map = initMapFromConf(*i);
			_mapList[id] = map;
			break;
		}
	}

}

Map *MapMgr::initMap(Map::Type type) {
	Map *map;

	switch (type) {
	case Map::WORLD:
		map = new Map();
		break;

	case Map::COMBAT:
		map = new CombatMap();
		break;

	case Map::SHRINE:
		map = new Shrine();
		break;

	case Map::DUNGEON:
		map = new Dungeon();
		break;

	case Map::CITY:
		map = new City();
		break;

	case Map::XML:
		map = new XMLMap();
		break;

	default:
		error("Error: invalid map type used");
		break;
	}

	return map;
}

Map *MapMgr::get(MapId id) {
	// if the map hasn't been loaded yet, load it!
	if (!_mapList[id]->_data.size()) {
		MapLoader *loader = g_mapLoaders->getLoader(_mapList[id]->_type);
		if (loader == nullptr)
			error("can't load map of type \"%d\"", _mapList[id]->_type);

		loader->load(_mapList[id]);
	}

	return _mapList[id];
}

void MapMgr::registerMap(Map *map) {
	if (_mapList.size() <= map->_id)
		_mapList.resize(map->_id + 1, nullptr);

	if (_mapList[map->_id] != nullptr)
		error("Error: A map with id '%d' already exists", map->_id);

	_mapList[map->_id] = map;
}

Map *MapMgr::initMapFromConf(const ConfigElement &mapConf) {
	Map *map;
	static const char *const mapTypeEnumStrings[] = { "world", "city", "shrine", "combat", "dungeon", "xml", nullptr };
	static const char *const borderBehaviorEnumStrings[] = { "wrap", "exit", "fixed", nullptr };

	map = initMap(static_cast<Map::Type>(mapConf.getEnum("type", mapTypeEnumStrings)));
	if (!map)
		return nullptr;

	map->_id = static_cast<MapId>(mapConf.getInt("id"));
	map->_type = static_cast<Map::Type>(mapConf.getEnum("type", mapTypeEnumStrings));
	map->_fname = mapConf.getString("fname");
	map->_width = mapConf.getInt("width");
	map->_height = mapConf.getInt("height");
	map->_levels = mapConf.getInt("levels");
	map->_chunkWidth = mapConf.getInt("chunkwidth");
	map->_chunkHeight = mapConf.getInt("chunkheight");
	map->_offset = mapConf.getInt("offset");
	map->_borderBehavior = static_cast<Map::BorderBehavior>(mapConf.getEnum("borderbehavior", borderBehaviorEnumStrings));

	if (isCombatMap(map)) {
		CombatMap *cm = dynamic_cast<CombatMap *>(map);
		assert(cm);
		cm->setContextual(mapConf.getBool("contextual"));
	}

	if (mapConf.getBool("showavatar"))
		map->_flags |= SHOW_AVATAR;

	if (mapConf.getBool("nolineofsight"))
		map->_flags |= NO_LINE_OF_SIGHT;

	if (mapConf.getBool("firstperson"))
		map->_flags |= FIRST_PERSON;

	map->_music = static_cast<Music::Type>(mapConf.getInt("music"));
	map->_tileSet = g_tileSets->get(mapConf.getString("tileset"));
	map->_tileMap = g_tileMaps->get(mapConf.getString("tilemap"));

	Std::vector<ConfigElement> children = mapConf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
		if (i->getName() == "city") {
			City *city = dynamic_cast<City *>(map);
			assert(city);
			initCityFromConf(*i, city);
		} else if (i->getName() == "shrine") {
			Shrine *shrine = dynamic_cast<Shrine *>(map);
			assert(shrine);
			initShrineFromConf(*i, shrine);
		} else if (i->getName() == "dungeon") {
			Dungeon *dungeon = dynamic_cast<Dungeon *>(map);
			assert(dungeon);
			initDungeonFromConf(*i, dungeon);
		} else if (i->getName() == "portal")
			map->_portals.push_back(initPortalFromConf(*i));
		else if (i->getName() == "moongate")
			createMoongateFromConf(*i);
		else if (i->getName() == "compressedchunk")
			map->_compressedChunks.push_back(initCompressedChunkFromConf(*i));
		else if (i->getName() == "label")
			map->_labels[i->getString("name")] = MapCoords(i->getInt("x"), i->getInt("y"), i->getInt("z", 0));
		else if (i->getName() == "tiles" && map->_type == Map::XML)
			static_cast<XMLMap *>(map)->_tilesText = i->getNode()->firstChild()->text();
	}

	return map;
}

void MapMgr::initCityFromConf(const ConfigElement &cityConf, City *city) {
	city->_name = cityConf.getString("name");
	city->_type = cityConf.getString("type");
	city->_tlkFname = cityConf.getString("tlk_fname");

	Std::vector<ConfigElement> children = cityConf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
		if (i->getName() == "personrole")
			city->_personRoles.push_back(initPersonRoleFromConf(*i));
	}
}

PersonRole *MapMgr::initPersonRoleFromConf(const ConfigElement &personRoleConf) {
	PersonRole *personrole;
	static const char *const roleEnumStrings[] = { "companion", "weaponsvendor", "armorvendor", "foodvendor", "tavernkeeper",
	                                         "reagentsvendor", "healer", "innkeeper", "guildvendor", "horsevendor",
	                                         "lordbritish", "hawkwind", nullptr
	                                       };

	personrole = new PersonRole();

	personrole->_role = personRoleConf.getEnum("role", roleEnumStrings) + NPC_TALKER_COMPANION;
	personrole->_id = personRoleConf.getInt("id");

	return personrole;
}

Portal *MapMgr::initPortalFromConf(const ConfigElement &portalConf) {
	Portal *portal;

	portal = new Portal();

	portal->_portalConditionsMet = nullptr;
	portal->_retroActiveDest = nullptr;

	portal->_coords = MapCoords(
	                      portalConf.getInt("x"),
	                      portalConf.getInt("y"),
	                      portalConf.getInt("z", 0));
	portal->_destid = static_cast<MapId>(portalConf.getInt("destmapid"));

	portal->_start.x = static_cast<unsigned short>(portalConf.getInt("startx"));
	portal->_start.y = static_cast<unsigned short>(portalConf.getInt("starty"));
	portal->_start.z = static_cast<unsigned short>(portalConf.getInt("startlevel", 0));

	Common::String prop = portalConf.getString("action");
	if (prop == "none")
		portal->_triggerAction = ACTION_NONE;
	else if (prop == "enter")
		portal->_triggerAction = ACTION_ENTER;
	else if (prop == "klimb")
		portal->_triggerAction = ACTION_KLIMB;
	else if (prop == "descend")
		portal->_triggerAction = ACTION_DESCEND;
	else if (prop == "exit_north")
		portal->_triggerAction = ACTION_EXIT_NORTH;
	else if (prop == "exit_east")
		portal->_triggerAction = ACTION_EXIT_EAST;
	else if (prop == "exit_south")
		portal->_triggerAction = ACTION_EXIT_SOUTH;
	else if (prop == "exit_west")
		portal->_triggerAction = ACTION_EXIT_WEST;
	else
		error("unknown trigger_action: %s", prop.c_str());

	prop = portalConf.getString("condition");
	if (!prop.empty()) {
		if (prop == "shrine")
			portal->_portalConditionsMet = &shrineCanEnter;
		else if (prop == "abyss")
			portal->_portalConditionsMet = &Items::isAbyssOpened;
		else
			error("unknown portalConditionsMet: %s", prop.c_str());
	}

	portal->_saveLocation = portalConf.getBool("savelocation");

	portal->_message = portalConf.getString("message");

	prop = portalConf.getString("transport");
	if (prop == "foot")
		portal->_portalTransportRequisites = TRANSPORT_FOOT;
	else if (prop == "footorhorse")
		portal->_portalTransportRequisites = TRANSPORT_FOOT_OR_HORSE;
	else
		error("unknown transport: %s", prop.c_str());

	portal->_exitPortal = portalConf.getBool("exits");

	// Used as a shortcut for specifying the display tile
	// for new/fan maps being added to the overworld
	portal->_tile = portalConf.exists("tile") ? portalConf.getInt("tile") : -1;

	Std::vector<ConfigElement> children = portalConf.getChildren();
	for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
		if (i->getName() == "retroActiveDest") {
			portal->_retroActiveDest = new PortalDestination();

			portal->_retroActiveDest->_coords = MapCoords(
			                                        i->getInt("x"),
			                                        i->getInt("y"),
			                                        i->getInt("z", 0));
			portal->_retroActiveDest->_mapid = static_cast<MapId>(i->getInt("mapid"));
		}
	}

	return portal;
}

void MapMgr::initShrineFromConf(const ConfigElement &shrineConf, Shrine *shrine) {
	static const char *const virtues[] = {"Honesty", "Compassion", "Valor", "Justice", "Sacrifice", "Honor", "Spirituality", "Humility", nullptr};

	shrine->setVirtue(static_cast<Virtue>(shrineConf.getEnum("virtue", virtues)));
	shrine->setMantra(shrineConf.getString("mantra"));
}

void MapMgr::initDungeonFromConf(const ConfigElement &dungeonConf, Dungeon *dungeon) {
	dungeon->_nRooms = dungeonConf.getInt("rooms");
	dungeon->_rooms = nullptr;
	dungeon->_roomMaps = nullptr;
	dungeon->_name = dungeonConf.getString("name");
}

void MapMgr::createMoongateFromConf(const ConfigElement &moongateConf) {
	int phase = moongateConf.getInt("phase");
	Coords coords(moongateConf.getInt("x"), moongateConf.getInt("y"));

	g_moongates->add(phase, coords);
}

int MapMgr::initCompressedChunkFromConf(const ConfigElement &compressedChunkConf) {
	return compressedChunkConf.getInt("index");
}

} // End of namespace Ultima4
} // End of namespace Ultima
