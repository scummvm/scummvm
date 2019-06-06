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

struct MSMEntry {
	char name[32];
	uint16 width;
	uint16 height;
	uint32 background;
	uint32 foreground;
	uint16 iconNum;
	uint32 iconList;
	uint16 infoNum;
	uint32 infoList;
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

class MapLoader {
public:
	MapLoader();

	bool loadMap(Common::SeekableReadStream *stream);
	int loadTiles();

	bool isLoaded() {
		return _mapLoaded;
	}

private:
	bool _mapLoaded;
};

}

#endif // !HDB_MAP_LOADER_H
