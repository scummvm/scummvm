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
	kScreenYTiles = 16,
	kMaxGratings = 250,
	kMaxForegrounds = 250
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

	MSMIcon(): icon(0), x(0), y(0), funcInit(""), funcAction(""), funcUse(""), dir(0), level(0), value1(0), value2(0) {}
};

struct Foreground {
	uint16 x;
	uint16 y;
	uint16 tile;

	Foreground() : x(0), y(0), tile(0) {}
};

struct SeeThroughTile {
	uint16 x;
	uint16 y;
	uint16 tile;

	SeeThroughTile() : x(0), y(0), tile(0) {}
};

class Map {
public:
	Map();
	~Map();

	int loadTiles();
	bool load(Common::SeekableReadStream *stream);
	void draw();
	void drawEnts();
	void drawGratings();
	void drawForegrounds();

	bool isLoaded() {
		return _mapLoaded;
	}

	bool onScreen(int x, int y) {
		if ((x >= _mapX / kTileWidth) && (x < (_mapX / kTileWidth) + kScreenXTiles) && (y >= _mapY / kTileHeight) && (y < (_mapY / kTileHeight) + kScreenYTiles))
			return true;
		return false;
	}

	uint32 getMapBGTileFlags(int x, int y);
	uint32 getMapFGTileFlags(int x, int y);
	uint16 getMapBGTileIndex(int x, int y);
	uint16 getMapFGTileIndex(int x, int y);
	void setMapBGTileIndex(int x, int y, int index);
	void setMapFGTileIndex(int x, int y, int index);
	void addBGTileAnimation(int x, int y);
	void addFGTileAnimation(int x, int y);
	void removeBGTileAnimation(int x, int y);
	void removeFGTileAnimation(int x, int y);

	void getMapXY(int *x, int *y);
	void setMapXY(int x, int y);
	void centerMapXY(int x, int y);
	bool checkEntOnScreen(AIEntity *);
	bool checkXYOnScreen(int x, int y);

	// Check if one of the tiles in a range exists in the map on either layer
	bool checkOneTileExistInRange(int tileIndex, int count);

	uint16 _width, _height;
	int _mapX, _mapY; // Coordinates of Map
	int _mapTileX, _mapTileY; // Tile Coordinates of Map
	int _mapTileXOff, _mapTileYOff; // Tile Coordinates Offset (0-31)

	Foreground *_gratings[kMaxGratings], *_foregrounds[kMaxForegrounds];
	int _numGratings, _numForegrounds;

	int _animCycle;	// Tile Animation Counter
	Common::Array<uint32> _listBGAnimSlow;
	Common::Array<uint32> _listBGAnimMedium;
	Common::Array<uint32> _listBGAnimFast;
	Common::Array<uint32> _listFGAnimSlow;
	Common::Array<uint32> _listFGAnimMedium;
	Common::Array<uint32> _listFGAnimFast;

private:
	char _name[32];
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
