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

#ifndef NUVIE_FILES_TMX_MAP_H
#define NUVIE_FILES_TMX_MAP_H

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/files/nuvie_io_file.h"

namespace Ultima {
namespace Nuvie {

class Map;
class ObjManager;
class TileManager;
class NuvieIOFileWrite;

class TMXMap {
private:
	unsigned char *mapdata;
	NuvieIOFileWrite file;
	TileManager *tile_manager;
	Map *map;
	ObjManager *obj_manager;
	Std::string savedir;
	Std::string savename;
	nuvie_game_t game_type;

public:
	TMXMap(TileManager *tm, Map *m, ObjManager *om);
	virtual ~TMXMap();
	bool exportTmxMapFiles(Std::string dir, nuvie_game_t type);
private:
	bool exportMapLevel(uint8 level);
	void writeRoofTileset(uint8 level);
	void writeLayer(NuvieIOFileWrite *tmx, uint16 width, Std::string layerName,
		uint16 gidOffset, uint16 bitsPerTile, const unsigned char *data);
	void writeObjectLayer(NuvieIOFileWrite *tmx, uint8 level);
	void writeObjects(NuvieIOFileWrite *tmx, uint8 level, bool forceLower, bool toptiles);
	Std::string writeObjectTile(Obj *obj, Std::string nameSuffix, uint16 tile_num, uint16 x, uint16 y, bool forceLower, bool toptile);
	Std::string sint32ToString(sint32 value);
	Std::string boolToString(bool value);
	bool canDrawTile(Tile *t, bool forceLower, bool toptile);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
