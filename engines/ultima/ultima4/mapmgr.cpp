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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/annotation.h"
#include "ultima/ultima4/city.h"
#include "ultima/ultima4/combat.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/dungeon.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/map.h"
#include "ultima/ultima4/maploader.h"
#include "ultima/ultima4/mapmgr.h"
#include "ultima/ultima4/moongate.h"
#include "ultima/ultima4/person.h"
#include "ultima/ultima4/portal.h"
#include "ultima/ultima4/shrine.h"
#include "ultima/ultima4/graphics/tilemap.h"
#include "ultima/ultima4/graphics/tileset.h"
#include "ultima/ultima4/types.h"
#include "ultima/ultima4/u4file.h"
#include "ultima/ultima4/config.h"

namespace Ultima {
namespace Ultima4 {

using Std::vector;
using Std::pair;

MapMgr *MapMgr::_instance = NULL;

extern bool isAbyssOpened(const Portal *p);
extern bool shrineCanEnter(const Portal *p);

MapMgr *MapMgr::getInstance() {
    if (_instance == NULL)
        _instance = new MapMgr();
    return _instance;
}

void MapMgr::destroy() {
    if (_instance != NULL) {
        delete _instance;
        _instance = NULL;
    }
}

MapMgr::MapMgr() {
    _logger = new Debug("debug/mapmgr.txt", "MapMgr"); 
    TRACE(*_logger, "creating MapMgr");

    const Config *config = Config::getInstance();
    Map *map;

    vector<ConfigElement> maps = config->getElement("maps").getChildren();
    for (Std::vector<ConfigElement>::iterator i = maps.begin(); i != maps.end(); i++) {
        map = initMapFromConf(*i);

        /* map actually gets loaded later, when it's needed */        
        registerMap(map);
    }
}

MapMgr::~MapMgr() {
    for (Std::vector<Map *>::iterator i = _mapList.begin(); i != _mapList.end(); i++)
        delete *i;

    delete _logger;
}

void MapMgr::unloadMap(MapId id) {
    delete _mapList[id];
    const Config *config = Config::getInstance();
    vector<ConfigElement> maps = config->getElement("maps").getChildren();

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

    switch(type) {    
    case Map::WORLD:
        map = new Map;
        break;

    case Map::COMBAT:
        map = new CombatMap;
        break;

    case Map::SHRINE:
        map = new Shrine;
        break;

    case Map::DUNGEON:
        map = new Dungeon;
        break;

    case Map::CITY:
        map = new City;
        break;
        
    default:
        error("Error: invalid map type used");
        break;
    }
    
    return map;
}

Map *MapMgr::get(MapId id) {    
    /* if the map hasn't been loaded yet, load it! */
    if (!_mapList[id]->_data.size()) {
        MapLoader *loader = MapLoader::getLoader(_mapList[id]->_type);
        if (loader == NULL)
            errorFatal("can't load map of type \"%d\"", _mapList[id]->_type);

        TRACE_LOCAL(*_logger, Common::String("loading map data for map \'") + _mapList[id]->_fname + "\'");

        loader->load(_mapList[id]);
    }
    return _mapList[id];
}

void MapMgr::registerMap(Map *map) {
    if (_mapList.size() <= map->_id)
        _mapList.resize(map->_id + 1, NULL);

    if (_mapList[map->_id] != NULL)
        errorFatal("Error: A map with id '%d' already exists", map->_id);

    _mapList[map->_id] = map;
}

Map *MapMgr::initMapFromConf(const ConfigElement &mapConf) {
    Map *map;
    static const char *mapTypeEnumStrings[] = { "world", "city", "shrine", "combat", "dungeon", NULL };
    static const char *borderBehaviorEnumStrings[] = { "wrap", "exit", "fixed", NULL };

    map = initMap(static_cast<Map::Type>(mapConf.getEnum("type", mapTypeEnumStrings)));
    if (!map)
        return NULL;

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
        CombatMap *cm = dynamic_cast<CombatMap*>(map);
        cm->setContextual(mapConf.getBool("contextual"));
    }

    TRACE_LOCAL(*_logger, Common::String("loading configuration for map \'") + map->_fname + "\'");

    if (mapConf.getBool("showavatar"))
        map->_flags |= SHOW_AVATAR;

    if (mapConf.getBool("nolineofsight"))
        map->_flags |= NO_LINE_OF_SIGHT;
    
    if (mapConf.getBool("firstperson"))
        map->_flags |= FIRST_PERSON;

    map->_music = static_cast<Music::Type>(mapConf.getInt("music"));
    map->_tileset = Tileset::get(mapConf.getString("tileset"));
    map->_tilemap = TileMap::get(mapConf.getString("tilemap"));

    vector<ConfigElement> children = mapConf.getChildren();
    for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
        if (i->getName() == "city") {
            City *city = dynamic_cast<City*>(map);
            initCityFromConf(*i, city);            
        }
        else if (i->getName() == "shrine") {
            Shrine *shrine = dynamic_cast<Shrine*>(map);
            initShrineFromConf(*i, shrine);
        }            
        else if (i->getName() == "dungeon") {
            Dungeon *dungeon = dynamic_cast<Dungeon*>(map);
            initDungeonFromConf(*i, dungeon);
        }
        else if (i->getName() == "portal")
            map->_portals.push_back(initPortalFromConf(*i));
        else if (i->getName() == "moongate")
            createMoongateFromConf(*i);
        else if (i->getName() == "compressedchunk")
            map->_compressedChunks.push_back(initCompressedChunkFromConf(*i));
        else if (i->getName() == "label")
            map->_labels.insert(initLabelFromConf(*i));
    }
    
    return map;
}

void MapMgr::initCityFromConf(const ConfigElement &cityConf, City *city) {
    city->_name = cityConf.getString("name");
    city->_type = cityConf.getString("type");
    city->_tlkFname = cityConf.getString("tlk_fname");

    vector<ConfigElement> children = cityConf.getChildren();
    for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
        if (i->getName() == "personrole")
            city->_personRoles.push_back(initPersonRoleFromConf(*i));
    }    
}

PersonRole *MapMgr::initPersonRoleFromConf(const ConfigElement &personRoleConf) {
    PersonRole *personrole;
    static const char *roleEnumStrings[] = { "companion", "weaponsvendor", "armorvendor", "foodvendor", "tavernkeeper",
                                             "reagentsvendor", "healer", "innkeeper", "guildvendor", "horsevendor",
                                             "lordbritish", "hawkwind", NULL };

    personrole = new PersonRole;

    personrole->_role = personRoleConf.getEnum("role", roleEnumStrings) + NPC_TALKER_COMPANION;
    personrole->_id = personRoleConf.getInt("id");

    return personrole;
}

Portal *MapMgr::initPortalFromConf(const ConfigElement &portalConf) {
    Portal *portal;

    portal = new Portal;

    portal->_portalConditionsMet = NULL;
    portal->_retroActiveDest = NULL;
 
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
        errorFatal("unknown trigger_action: %s", prop.c_str());
    
    prop = portalConf.getString("condition");
    if (!prop.empty()) {
        if (prop == "shrine")
            portal->_portalConditionsMet = &shrineCanEnter;
        else if (prop == "abyss")
            portal->_portalConditionsMet = &isAbyssOpened;
        else
            errorFatal("unknown portalConditionsMet: %s", prop.c_str());
    }

    portal->_saveLocation = portalConf.getBool("savelocation");

    portal->_message = portalConf.getString("message");

    prop = portalConf.getString("transport");
    if (prop == "foot")
        portal->_portalTransportRequisites = TRANSPORT_FOOT;
    else if (prop == "footorhorse")
        portal->_portalTransportRequisites = TRANSPORT_FOOT_OR_HORSE;
    else
        errorFatal("unknown transport: %s", prop.c_str());

    portal->_exitPortal = portalConf.getBool("exits");

    vector<ConfigElement> children = portalConf.getChildren();
    for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
        if (i->getName() == "retroActiveDest") {
            portal->_retroActiveDest = new PortalDestination;
            
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
    static const char *virtues[] = {"Honesty", "Compassion", "Valor", "Justice", "Sacrifice", "Honor", "Spirituality", "Humility", NULL};

    shrine->setVirtue(static_cast<Virtue>(shrineConf.getEnum("virtue", virtues)));
    shrine->setMantra(shrineConf.getString("mantra"));
}

void MapMgr::initDungeonFromConf(const ConfigElement &dungeonConf, Dungeon *dungeon) {
    dungeon->_nRooms = dungeonConf.getInt("rooms");
    dungeon->_rooms = NULL;
    dungeon->_roomMaps = NULL;
    dungeon->_name = dungeonConf.getString("name");
}

void MapMgr::createMoongateFromConf(const ConfigElement &moongateConf) {
    int phase = moongateConf.getInt("phase");
    Coords coords(moongateConf.getInt("x"), moongateConf.getInt("y"));

    moongateAdd(phase, coords);
}

int MapMgr::initCompressedChunkFromConf(const ConfigElement &compressedChunkConf) {
    return compressedChunkConf.getInt("index");
}

Std::pair<Common::String, MapCoords> MapMgr::initLabelFromConf(const ConfigElement &labelConf) {
    return Std::pair<Common::String, MapCoords>
        (labelConf.getString("name"), 
         MapCoords(labelConf.getInt("x"), labelConf.getInt("y"), labelConf.getInt("z", 0)));
}

} // End of namespace Ultima4
} // End of namespace Ultima
