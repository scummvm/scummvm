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

// Isometric level module
#include "saga/saga.h"

#include "saga/gfx.h"

#include "saga/isomap.h"
#include "saga/stream.h"

namespace Saga {

IsoMap::IsoMap(SagaEngine *vm) : _vm(vm) {
	_tileData = NULL;
	_tilesCount = 0;
	_tilePlatformList = NULL;
	_tilePlatformsCount = 0;
	_metaTileList = NULL;
	_metaTilesCount = 0;
	_multiTable = NULL;
	_multiCount = 0;
	_viewScroll.x = (128 - 8) * 16;
	_viewScroll.x = (128 - 8) * 16 - 64;
	_viewDiff = 20;
}

void IsoMap::loadImages(const byte *resourcePointer, size_t resourceLength) {
	IsoTileData *tileData;
	uint16 i;

	if (resourceLength == 0) {
		error("IsoMap::loadImages wrong resourceLength");
	}

	_tileData = (byte*)malloc(resourceLength);
	_tileDataLength = resourceLength;
	memcpy(_tileData, resourcePointer, resourceLength);
	
	MemoryReadStreamEndian readS(_tileData, _tileDataLength, IS_BIG_ENDIAN);
	readS.readUint16(); // skip
	_tilesCount = readS.readUint16();
	_tilesCount = _tilesCount / SAGA_ISOTILEDATA_LEN;

	readS.seek(0);

	_tilesTable = (IsoTileData *)malloc(_tilesCount * sizeof(*_tilesTable));
	if (_tilesTable == NULL) {
		memoryError("IsoMap::loadImages");
	}

	for (i = 0; i < _tilesCount; i++) {
		tileData = &_tilesTable[i];
		tileData->height = readS.readByte();
		tileData->attributes = readS.readByte();
		tileData->offset = readS.readUint16();
		tileData->terrainMask = readS.readSint16();
		tileData->FGBGAttr = readS.readByte();
		readS.readByte(); //skip
	}

}

void IsoMap::loadPlatforms(const byte * resourcePointer, size_t resourceLength) {
	TilePlatformData *tilePlatformData;
	uint16 i, x, y;

	if (resourceLength == 0) {
		error("IsoMap::loadPlatforms wrong resourceLength");
	}

	MemoryReadStreamEndian readS(resourcePointer, resourceLength, IS_BIG_ENDIAN);

	_tilePlatformsCount = resourceLength / SAGA_TILEPLATFORMDATA_LEN;
	_tilePlatformList = (TilePlatformData *)malloc(_tilePlatformsCount * sizeof(*_tilePlatformList));
	if (_tilePlatformList == NULL) {
		memoryError("IsoMap::loadPlatforms");
	}

	for (i = 0; i < _tilePlatformsCount; i++) {
		tilePlatformData = &_tilePlatformList[i];
		tilePlatformData->metaTile = readS.readSint16();
		tilePlatformData->height = readS.readSint16();
		tilePlatformData->highestPixel = readS.readSint16();
		tilePlatformData->vBits = readS.readByte();
		tilePlatformData->uBits = readS.readByte();
		for (x = 0; x < SAGA_PLATFORM_W; x++) {
			for (y = 0; y < SAGA_PLATFORM_W; y++) {
				tilePlatformData->tiles[x][y] = readS.readSint16();
			}
		}
	}

}

void IsoMap::loadMap(const byte * resourcePointer, size_t resourceLength) {
	uint16 x, y;

	if (resourceLength != SAGA_TILEMAP_LEN) {
		error("IsoMap::loadMap wrong resourceLength");
	}

	MemoryReadStreamEndian readS(resourcePointer, resourceLength, IS_BIG_ENDIAN);
	_tileMap.edgeType = readS.readByte();
	readS.readByte(); //skip

	for (x = 0; x < SAGA_TILEMAP_W; x++) {
		for (y = 0; y < SAGA_TILEMAP_H; y++) {
			_tileMap.tilePlatforms[x][y] = readS.readSint16();
		}
	}

}

void IsoMap::loadMetaTiles(const byte * resourcePointer, size_t resourceLength) {
	MetaTileData *metaTileData;
	uint16 i, j;

	if (resourceLength == 0) {
		error("IsoMap::loadMetaTiles wrong resourceLength");
	}

	MemoryReadStreamEndian readS(resourcePointer, resourceLength, IS_BIG_ENDIAN);
	_metaTilesCount = resourceLength / SAGA_METATILEDATA_LEN;

	_metaTileList = (MetaTileData *)malloc(_metaTilesCount * sizeof(*_metaTileList));
	if (_metaTileList == NULL) {
		memoryError("IsoMap::loadMetaTiles");
	}

	for (i = 0; i < _metaTilesCount; i++) {
		metaTileData = &_metaTileList[i];
		metaTileData->highestPlatform = readS.readUint16();
		metaTileData->highestPixel = readS.readUint16();
		for (j = 0; j < SAGA_MAX_PLATFORM_H; j++) {
			metaTileData->stack[j] = readS.readSint16();
		}
	}
}

void IsoMap::loadMulti(const byte * resourcePointer, size_t resourceLength) {
	MultiTileEntryData *multiTileEntryData;
	uint16 i;

	if (resourceLength < 2) {
		error("IsoMap::loadMetaTiles wrong resourceLength");
	}

	MemoryReadStreamEndian readS(resourcePointer, resourceLength, IS_BIG_ENDIAN);
	_multiCount = readS.readUint16();
	_multiTable = (MultiTileEntryData *)malloc(_multiCount * sizeof(*_multiTable));
	if (_multiTable == NULL) {
		memoryError("IsoMap::loadMulti");
	}
	debug(0,"resourceLength=%d but should be %d",resourceLength, 14*_multiCount + 2);
	for (i = 0; i < _multiCount; i++) {
		multiTileEntryData = &_multiTable[i];
		readS.readUint32();//skip
		multiTileEntryData->offset = readS.readSint16();
		multiTileEntryData->u = readS.readByte();
		multiTileEntryData->v = readS.readByte();
		multiTileEntryData->h = readS.readByte();
		multiTileEntryData->uSize = readS.readByte();
		multiTileEntryData->vSize = readS.readByte();
		multiTileEntryData->numStates = readS.readByte();
		multiTileEntryData->currentState = readS.readByte();
		readS.readByte();//skip
	}
}

void IsoMap::freeMem() {
	free(_tileData);
	_tilesCount = 0;
	free(_tilePlatformList);
	_tilePlatformsCount = 0;
	free(_metaTileList);
	_metaTilesCount = 0;
	free(_multiTable);
	_multiCount = 0;
}

int IsoMap::draw(SURFACE *ds) {
	
	Rect isoRect(_vm->getDisplayWidth(), _vm->getDisplayInfo().sceneHeight);
	drawRect(ds, &isoRect, 0);
	_tileClip = isoRect;
	drawTiles(ds);

	return SUCCESS;
}

void IsoMap::drawTiles(SURFACE *ds) {
	Point view1;
	Point metaTileY;
	Point metaTileX;
	int16 u0, v0, 
		  u1, v1,
		  u2, v2,
		  uc, vc;
	int16 workAreaWidth;
	int16 workAreaHeight;
	uint16 metaTileIndex;

	_tileScroll.x = _viewScroll.x >> 4;
	_tileScroll.y = _viewScroll.y >> 4;

	view1.x = _tileScroll.x - (8 * SAGA_TILEMAP_W);
	view1.y = (8 * SAGA_TILEMAP_W) - _tileScroll.y;

	u0 = ((view1.y + 64) * 2 + view1.x) >> 4;
	v0 = ((view1.y + 64) * 2 - view1.x) >> 4;
	
	metaTileY.x = (u0 - v0) * 128 - view1.x * 16;
	metaTileY.y = view1.y * 16 - (u0 + v0) * 64;

	workAreaWidth = _vm->getDisplayWidth() + 128;
	workAreaHeight = _vm->getDisplayInfo().sceneHeight + 128 + 80;

	for (u1 = u0, v1 = v0; metaTileY.y < workAreaHeight; u1--, v1-- ) {
		metaTileX = metaTileY;

		for (u2 = u1, v2 = v1; metaTileX.x < workAreaWidth; u2++, v2--, metaTileX.x += 256) {

			uc = u2 & (SAGA_TILEMAP_W - 1);
			vc = v2 & (SAGA_TILEMAP_W - 1);

			if (uc != u2 || vc != v2) {					
				metaTileIndex = 0;
				switch ( _tileMap.edgeType) {				
				case kEdgeTypeBlack: 
					continue;
				case kEdgeTypeFill0: 
					break;
				case kEdgeTypeFill1:
					metaTileIndex = 1; 
					break;
				case kEdgeTypeRpt:
					uc = clamp( 0, u2, SAGA_TILEMAP_W - 1);
					vc = clamp( 0, v2, SAGA_TILEMAP_W - 1);
					metaTileIndex = _tileMap.tilePlatforms[uc][vc];
					break;
				case kEdgeTypeWrap:
					metaTileIndex = _tileMap.tilePlatforms[uc][vc];
					break;
				}
			} else {
				metaTileIndex = _tileMap.tilePlatforms[uc][vc];
			}

			drawMetaTile(ds, metaTileIndex, metaTileX, u2 << 3, v2 << 3);
		}

		metaTileY.y += 64;

		metaTileX = metaTileY;

		metaTileX.x -= 128;

		for (u2 = u1 - 1, v2 = v1; metaTileX.x < workAreaWidth; u2++, v2--, metaTileX.x += 256) {

			uc = u2 & (SAGA_TILEMAP_W - 1);
			vc = v2 & (SAGA_TILEMAP_W - 1);

			if (uc != u2 || vc != v2) {
				metaTileIndex = 0;
				switch ( _tileMap.edgeType) {				
				case kEdgeTypeBlack: 
					continue;
				case kEdgeTypeFill0: 
					break;
				case kEdgeTypeFill1:
					metaTileIndex = 1; 
					break;
				case kEdgeTypeRpt:
					uc = clamp( 0, u2, SAGA_TILEMAP_W - 1);
					vc = clamp( 0, v2, SAGA_TILEMAP_W - 1);
					metaTileIndex = _tileMap.tilePlatforms[uc][vc];
					break;
				case kEdgeTypeWrap:
					metaTileIndex = _tileMap.tilePlatforms[uc][vc];
					break;
				}
			} else {
				metaTileIndex = _tileMap.tilePlatforms[uc][vc];
			}

			drawMetaTile(ds, metaTileIndex, metaTileX, u2 << 3, v2 << 3);
		}
		metaTileY.y += 64;
	}

}

void IsoMap::drawMetaTile(SURFACE *ds, uint16 metaTileIndex, const Point &point, int16 absU, int16 absV) {
	MetaTileData * metaTile;
	uint16 high;
	int16 platformIndex;
	Point platformPoint;
	platformPoint = point;

	if (_metaTilesCount <= metaTileIndex) {
		error("IsoMap::drawMetaTile wrong metaTileIndex");
	}

	metaTile = &_metaTileList[metaTileIndex];

	if (metaTile->highestPlatform > 18) {
		metaTile->highestPlatform = 0;
	}

	for (high = 0; high <= metaTile->highestPlatform; high++, platformPoint.y -= 8) {
		assert(SAGA_MAX_PLATFORM_H > high);
		platformIndex = metaTile->stack[high];

		if (platformIndex >= 0) {
			drawPlatform( ds, platformIndex, platformPoint, absU, absV, high );
		}
	}	
}

void IsoMap::drawPlatform(SURFACE *ds, uint16 platformIndex, const Point &point, int16 absU, int16 absV, int16 absH) {
	TilePlatformData *tilePlatform;
	int16 u, v;
	Point s;
	Point s0;
	int16 tileIndex;

	if (_tilePlatformsCount <= platformIndex) {
		error("IsoMap::drawPlatform wrong platformIndex");
	}

	tilePlatform = &_tilePlatformList[platformIndex];

	if ((point.y <= _tileClip.top) || (point.y - SAGA_MAX_TILE_H - SAGA_PLATFORM_W * SAGA_TILE_NOMINAL_H >= _tileClip.bottom)) {
		return;
	}
		
	s0 = point;
	s0.y -= (((SAGA_PLATFORM_W - 1) + (SAGA_PLATFORM_W - 1)) * 8);

	for (v = SAGA_PLATFORM_W - 1; v >= 0 && s0.y - SAGA_MAX_TILE_H < _tileClip.bottom && s0.x - 128 < _tileClip.right; v--, s0.x += 16, s0.y += 8) {

		if ((tilePlatform->vBits & (1 << v)) == 0) {
			continue;
		}

		if (s0.x + 128 + 32 < _tileClip.left) {
			continue;
		}

		s = s0;

		for (u = SAGA_PLATFORM_W - 1; u >= 0 && s.x + 32 > _tileClip.left && s.y - SAGA_MAX_TILE_H < _tileClip.bottom; u--, s.x -= 16, s.y += 8 ) {
			if (s.x < _tileClip.right && s.y > _tileClip.top) {
				
				tileIndex = tilePlatform->tiles[u][v];
				if (tileIndex > 1) {
					if (tileIndex & SAGA_MULTI_TILE) {
						warning("SAGA_MULTI_TILE"); //TODO: do it !
					}

					drawTile(ds, tileIndex, s);
				}
			}
		}
	}
}

void IsoMap::drawTile(SURFACE *ds, uint16 tileIndex, const Point &point) {
	const byte *tilePointer;
	const byte *readPointer;
	byte *drawPointer;
	Point drawPoint;
	int height;
	int widthCount = 0;
	int row, col, count, lowBound;
	int bgRunCount;
	int fgRunCount;


	if (tileIndex >= _tilesCount) {
		error("IsoMap::drawTile wrong tileIndex");
	}


	if (point.x + SAGA_ISOTILE_WIDTH < _tileClip.left) {
		return;
	}

	if (point.x - SAGA_ISOTILE_WIDTH >= _tileClip.right) {
		return;
	}

	tilePointer = _tileData + _tilesTable[tileIndex].offset;
	height = _tilesTable[tileIndex].height;



	if ((height <= 8) || (height > 64)) {
		return;
	}

	drawPoint = point;
	drawPoint.y -= height;

	if (drawPoint.y >= _tileClip.bottom) {
		return;
	}

	readPointer = tilePointer;
	lowBound = MIN((int)(drawPoint.y + height), (int)_tileClip.bottom);
	for (row = drawPoint.y; row < lowBound; row++) {
		widthCount = 0;
		if (row >= _tileClip.top) {
			drawPointer = (byte *)ds->pixels + drawPoint.x + (row * ds->pitch);
			col = drawPoint.x;
			for (;;) {
				bgRunCount = *readPointer++;
				widthCount += bgRunCount;
				if (widthCount >= SAGA_ISOTILE_WIDTH) {
					break;
				}

				drawPointer += bgRunCount;
				col += bgRunCount;
				fgRunCount = *readPointer++;
				widthCount += fgRunCount;
				
				count = 0;
				while ((col < _tileClip.left) && (count < fgRunCount)) {
					count++;
					col++;
				}
				while ((col < _tileClip.right) && (count < fgRunCount)) {
					drawPointer[count] = readPointer[count];
					count++;
					col++;
				}
				readPointer += fgRunCount;
				drawPointer += fgRunCount;
			}
		} else {
			for (;;) {
				bgRunCount = *readPointer++;
				widthCount += bgRunCount;
				if (widthCount >= SAGA_ISOTILE_WIDTH) {
					break;
				}

				fgRunCount = *readPointer++;
				widthCount += fgRunCount;
				
				readPointer += fgRunCount;
			}
		}
	}

}

} // End of namespace Saga
