/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Isometric level module - private header

#ifndef SAGA_ISOMAP_H_
#define SAGA_ISOMAP_H_

#include "saga/actor.h"

namespace Saga {

#define SAGA_ISOTILEDATA_LEN 8
#define SAGA_ISOTILE_WIDTH 32
#define SAGA_ISOTILE_BASEHEIGHT 15
#define SAGA_TILE_NOMINAL_H 16
#define SAGA_MAX_TILE_H 64

#define SAGA_TILEPLATFORMDATA_LEN 136
#define SAGA_PLATFORM_W 8
#define SAGA_MAX_PLATFORM_H 16

#define SAGA_TILEMAP_LEN 514
#define SAGA_TILEMAP_W 16
#define SAGA_TILEMAP_H 16

#define SAGA_METATILEDATA_LEN 36

#define SAGA_MULTI_TILE (1 << 15)

#define SAGA_SCROLL_LIMIT_X1 32
#define SAGA_SCROLL_LIMIT_X2 64
#define SAGA_SCROLL_LIMIT_Y1 8
#define SAGA_SCROLL_LIMIT_Y2 32

enum TileMapEdgeType {
	kEdgeTypeBlack	= 0,
	kEdgeTypeFill0	= 1,
	kEdgeTypeFill1	= 2,
	kEdgeTypeRpt	= 3,
	kEdgeTypeWrap	= 4
};

struct IsoTileData {
	byte height;
	int8 attributes;
	size_t offset;
	uint16 terrainMask;
	byte FGBGAttr;
	int8 GetMaskRule() {
		return attributes & 0x0F;
	}
};

struct TilePlatformData {
	int16 metaTile;	
	int16 height;
	int16 highestPixel;
	byte vBits;
	byte uBits;
	int16 tiles[SAGA_PLATFORM_W][SAGA_PLATFORM_W];
};

struct TileMapData {
	byte edgeType;
	int16 tilePlatforms[SAGA_TILEMAP_W][SAGA_TILEMAP_H];
};

struct MetaTileData {
	uint16 highestPlatform;
	uint16 highestPixel;
	int16 stack[SAGA_MAX_PLATFORM_H];
};

struct MultiTileEntryData {
	int16 offset;
	byte u;
	byte v;
	byte h;
	byte uSize;
	byte vSize;
	byte numStates;
	byte currentState;
};

class IsoMap {
public:
	IsoMap(SagaEngine *vm);
	~IsoMap() {
		freeMem();
	}
	void loadImages(const byte * resourcePointer, size_t resourceLength);
	void loadMap(const byte * resourcePointer, size_t resourceLength);
	void loadPlatforms(const byte * resourcePointer, size_t resourceLength);
	void loadMetaTiles(const byte * resourcePointer, size_t resourceLength);
	void loadMulti(const byte * resourcePointer, size_t resourceLength);
	void freeMem();
	int draw(SURFACE *ds);
	void drawSprite(SURFACE *ds, SpriteList &spriteList, int spriteNumber, const Location &location, const Point &screenPosition, int scale);
	void adjustScroll(bool jump);
	void tileCoordsToScreenPoint(const Location &location, Point &position) {
		position.x = location.u() - location.v() + (128 * SAGA_TILEMAP_W) - _viewScroll.x + 16;
		position.y = -((location.u() + location.v()) >> 1) + (128 * SAGA_TILEMAP_W) - _viewScroll.y - location.z;
	}
	
private:
	void drawTiles(SURFACE *ds, const Location *location);
	void drawMetaTile(SURFACE *ds, uint16 metaTileIndex, const Point &point, int16 absU, int16 absV);
	void drawSpriteMetaTile(SURFACE *ds, uint16 metaTileIndex, const Point &point, Location &location, int16 absU, int16 absV);
	void drawPlatform(SURFACE *ds, uint16 platformIndex, const Point &point, int16 absU, int16 absV, int16 absH);	
	void drawSpritePlatform(SURFACE *ds, uint16 platformIndex, const Point &point, const Location &location, int16 absU, int16 absV, int16 absH);	
	void drawTile(SURFACE *ds, uint16 tileIndex, const Point &point);
	void drawSpriteTile(SURFACE *ds, uint16 tileIndex, const Location &location, const Point &point);
	int16 smoothSlide(int16 value, int16 min, int16 max) {
		if (value < min) {
			if (value < min - 100 || value > min - 4) {
				value = min;
			} else {
				value += 4;
			}
		} else {
			if (value > max) {
				if (value > max + 100 || value < max + 4) {
					value = max;
				} else {
					value -= 4;
				}
			}
		}
		return value;
	}	
	int16 findMulti(int16 tileIndex, int16 absU, int16 absV, int16 absH );


	byte *_tileData;
	size_t _tileDataLength;	
	uint16 _tilesCount;
	IsoTileData *_tilesTable;

	uint16 _tilePlatformsCount;
	TilePlatformData *_tilePlatformList;
	uint16 _metaTilesCount;
	MetaTileData *_metaTileList;
	
	uint16 _multiCount;
	MultiTileEntryData *_multiTable;
	uint16 _multiDataCount;
	int16 *_multiTableData;

	TileMapData _tileMap;
	
	Point _mapPosition;
public:
	int _viewDiff;
	Point _viewScroll;
	Rect _tileClip;

	SagaEngine *_vm;
};

} // End of namespace Saga

#endif
