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

#ifndef HDB_MAP_LOADER_H
#define HDB_MAP_LOADER_H

#include "common/system.h"

namespace HDB {

enum {
	kScreenXTiles = 17,
	kScreenYTiles = 16
};

struct MSMIcon {
	uint16	icon;					// index into icon list
	uint16	x;
	uint16	y;

	char	funcInit[32];		// Lua init function for this entity
	char	funcAction[32];
	char	funcUse[32];
	uint16	dir;					// direction entity is facing
	uint16	level;					// which floor level entity is on
	uint16	value1, value2;
};

struct SeeThroughTile {
	uint16 x;
	uint16 y;
	uint16 tile;
};

class Map {
public:
	Map();

	int loadTiles();
	bool load(Common::SeekableReadStream *stream);
	void draw();

	int _mapX, _mapY; // Coordinates of Map
	int _mapTileX, _mapTileY; // Tile Coordinates of Map
	int _mapTileXOff, _mapTileYOff; // Tile Coordinates Offset (0-31)

private:
	char _name[32];
	uint16 _width;
	uint16 _height;
	uint32 _backgroundOffset;
	uint32 _foregroundOffset;
	uint16 _iconNum;
	uint32 _iconListOffset;
	uint16 _infoNum;
	uint32 _infoListOffset;

	uint16 *_background;
	uint16 *_foreground;
	MSMIcon *_iconList;

	byte *_mapExplosions;
	byte *_mapExpBarrels;
	byte *_mapLaserBeams;

	bool _mapLoaded;
};
}

#endif // !HDB_MAP_LOADER_H
