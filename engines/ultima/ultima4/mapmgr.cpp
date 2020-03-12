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
#include "ultima/ultima4/tilemap.h"
#include "ultima/ultima4/tileset.h"
#include "ultima/ultima4/types.h"
#include "ultima/ultima4/u4file.h"
#include "ultima/ultima4/config.h"

namespace Ultima {
namespace Ultima4 {

using Std::vector;
using Std::pair;

MapMgr *MapMgr::instance = NULL;

extern bool isAbyssOpened(const Portal *p);
extern bool shrineCanEnter(const Portal *p);

MapMgr *MapMgr::getInstance() {
    if (instance == NULL)
        instance = new MapMgr();
    return instance;
}

void MapMgr::destroy() {
    if (instance != NULL) {
        delete instance;
        instance = NULL;
    }
}

MapMgr::MapMgr() {
    logger = new Debug("debug/mapmgr.txt", "MapMgr"); 
    TRACE(*logger, "creating MapMgr");

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
    for (Std::vector<Map *>::iterator i = mapList.begin(); i != mapList.end(); i++)
        delete *i;

    delete logger;
}

void MapMgr::unloadMap(MapId id) {
    delete mapList[id];
    const Config *config = Config::getInstance();
    vector<ConfigElement> maps = config->getElement("maps").getChildren();

    for (Std::vector<ConfigElement>::const_iterator i = maps.begin(); i != maps.end(); ++i) {
        if (id == static_cast<MapId>((*i).getInt("id"))) {
            Map *map = initMapFromConf(*i);
            mapList[id] = map;
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
        errorFatal("Error: invalid map type used");
        break;
    }
    
    return map;
}

Map *MapMgr::get(MapId id) {    
    /* if the map hasn't been loaded yet, load it! */
    if (!mapList[id]->data.size()) {
        MapLoader *loader = MapLoader::getLoader(mapList[id]->type);
        if (loader == NULL)
            errorFatal("can't load map of type \"%d\"", mapList[id]->type);

        TRACE_LOCAL(*logger, Common::String("loading map data for map \'") + mapList[id]->fname + "\'");

        loader->load(mapList[id]);
    }
    return mapList[id];
}

void MapMgr::registerMap(Map *map) {
    if (mapList.size() <= map->id)
        mapList.resize(map->id + 1, NULL);

    if (mapList[map->id] != NULL)
        errorFatal("Error: A map with id '%d' already exists", map->id);

    mapList[map->id] = map;
}

Map *MapMgr::initMapFromConf(const ConfigElement &mapConf) {
    Map *map;
    static const char *mapTypeEnumStrings[] = { "world", "city", "shrine", "combat", "dungeon", NULL };
    static const char *borderBehaviorEnumStrings[] = { "wrap", "exit", "fixed", NULL };

    map = initMap(static_cast<Map::Type>(mapConf.getEnum("type", mapTypeEnumStrings)));
    if (!map)
        return NULL;

    map->id = static_cast<MapId>(mapConf.getInt("id"));
    map->type = static_cast<Map::Type>(mapConf.getEnum("type", mapTypeEnumStrings));
    map->fname = mapConf.getString("fname");
    map->width = mapConf.getInt("width");
    map->height = mapConf.getInt("height");
    map->levels = mapConf.getInt("levels");
    map->chunk_width = mapConf.getInt("chunkwidth");
    map->chunk_height = mapConf.getInt("chunkheight");
    map->offset = mapConf.getInt("offset");
    map->border_behavior = static_cast<Map::BorderBehavior>(mapConf.getEnum("borderbehavior", borderBehaviorEnumStrings));    

    if (isCombatMap(map)) {
        CombatMap *cm = dynamic_cast<CombatMap*>(map);
        cm->setContextual(mapConf.getBool("contextual"));
    }

    TRACE_LOCAL(*logger, Common::String("loading configuration for map \'") + map->fname + "\'");

    if (mapConf.getBool("showavatar"))
        map->flags |= SHOW_AVATAR;

    if (mapConf.getBool("nolineofsight"))
        map->flags |= NO_LINE_OF_SIGHT;
    
    if (mapConf.getBool("firstperson"))
        map->flags |= FIRST_PERSON;

    map->music = static_cast<Music::Type>(mapConf.getInt("music"));
    map->tileset = Tileset::get(mapConf.getString("tileset"));
    map->tilemap = TileMap::get(mapConf.getString("tilemap"));

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
            map->portals.push_back(initPortalFromConf(*i));
        else if (i->getName() == "moongate")
            createMoongateFromConf(*i);
        else if (i->getName() == "compressedchunk")
            map->compressed_chunks.push_back(initCompressedChunkFromConf(*i));
        else if (i->getName() == "label")
            map->labels.insert(initLabelFromConf(*i));
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
    dungeon->n_rooms = dungeonConf.getInt("rooms");
    dungeon->rooms = NULL;
    dungeon->roomMaps = NULL;
    dungeon->name = dungeonConf.getString("name");
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
