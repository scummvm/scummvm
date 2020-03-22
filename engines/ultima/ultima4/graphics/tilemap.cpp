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

#include "ultima/ultima4/graphics/tilemap.h"
#include "ultima/ultima4/graphics/tile.h"
#include "ultima/ultima4/config.h"
#include "ultima/ultima4/debug.h"
#include "ultima/ultima4/error.h"
#include "ultima/ultima4/graphics/tileset.h"

namespace Ultima {
namespace Ultima4 {

using Std::vector;

Debug dbg("debug/tilemap.txt", "TileMap");

/**
 * Static variables
 */
TileMap::TileIndexMapMap TileMap::_tileMaps;

/**
 * Load all tilemaps from the specified xml file
 */
void TileMap::loadAll() {
    const Config *config = Config::getInstance();    
    vector<ConfigElement> conf;

    /* FIXME: make sure tilesets are loaded by now */    

    TRACE_LOCAL(dbg, "Unloading all tilemaps");
    unloadAll();

    /* open the filename for the tileset and parse it! */
    TRACE_LOCAL(dbg, "Loading tilemaps from config");
    conf = config->getElement("tilesets").getChildren();    
    
    /* load all of the tilemaps */
    for (Std::vector<ConfigElement>::iterator i = conf.begin(); i != conf.end(); i++) {
        if (i->getName() == "tilemap") {
        
            /* load the tilemap ! */
            load(*i);
        }
    }
}
 
/**
 * Delete all tilemaps
 */
void TileMap::unloadAll() {    
    TileIndexMapMap::iterator map;       
        
    /* free all the memory for the tile maps */
    for (map = _tileMaps.begin(); map != _tileMaps.end(); map++)
        delete map->_value;
    
    /* Clear the map so we don't attempt to delete the memory again
     * next time.
     */
    _tileMaps.clear();
}
 
/**
 * Loads a tile map which translates between tile indices and tile
 * names.  Tile maps are useful to translate from dos tile indices to
 * xu4 tile ids.
 */
void TileMap::load(const ConfigElement &tilemapConf) {
    TileMap *tm = new TileMap;
    
    Common::String name = tilemapConf.getString("name");
    TRACE_LOCAL(dbg, Common::String("Tilemap name is: ") + name);
    
    Common::String tileset = tilemapConf.getString("tileset");

    int index = 0;
    vector<ConfigElement> children = tilemapConf.getChildren();
    for (Std::vector<ConfigElement>::iterator i = children.begin(); i != children.end(); i++) {
        if (i->getName() != "mapping")
            continue;

        /* we assume tiles have already been loaded at this point,
           so let's do some translations! */
        
        int frames = 1;
        Common::String tile = i->getString("tile");

        TRACE_LOCAL(dbg, Common::String("\tLoading '") + tile + "'");
        
        /* find the tile this references */
        Tile *t = Tileset::get(tileset)->getByName(tile);
        if (!t)
            errorFatal("Error: tile '%s' from '%s' was not found in tileset %s", tile.c_str(), name.c_str(), tileset.c_str());
        
        if (i->exists("index"))
            index = i->getInt("index");        
        if (i->exists("frames"))
            frames = i->getInt("frames");

        /* insert the tile into the tile map */
        for (int idx = 0; idx < frames; idx++) {
            if (idx < t->getFrames())
                tm->_tileMap[index+idx] = MapTile(t->getId(), idx);
            /* frame fell out of the scope of the tile -- frame is set to 0 */
            else
                tm->_tileMap[index+idx] = MapTile(t->getId(), 0);
        }
        
        index += frames;
    }
    
    /* add the tilemap to our list */
    _tileMaps[name] = tm;
}

/**
 * Returns the Tile index map with the specified name
 */
TileMap *TileMap::get(Common::String name) {
    if (_tileMaps.find(name) != _tileMaps.end())
        return _tileMaps[name];
    else return NULL;    
}

/**
 * Translates a raw index to a MapTile.
 */
MapTile TileMap::translate(unsigned int index) {
    return _tileMap[index];
}

unsigned int TileMap::untranslate(MapTile &tile) {
    unsigned int index = 0;

    for (Std::map<unsigned int, MapTile>::iterator i = _tileMap.begin(); i != _tileMap.end(); i++) {
        if (i->_value == tile) {
            index = i->_key;
            break;
        }
    }

    index += tile._frame;

    return index;
}

} // End of namespace Ultima4
} // End of namespace Ultima
