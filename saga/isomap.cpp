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
		for (y = 0; y < SAGA_PLATFORM_W; y++) {
			for (x = 0; x < SAGA_PLATFORM_W; x++) {
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

	for (y = 0; y < SAGA_TILEMAP_H; y++) {
		for (x = 0; x < SAGA_TILEMAP_W; x++) {
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
	debug(0,"resourceLength=%d but should be %d",resourceLength, 12*_multiCount + 2);
	for (i = 0; i < _multiCount; i++) {
		multiTileEntryData = &_multiTable[i];
		readS.readUint16();//skip
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

int IsoMap::draw(SURFACE *dst_s) {
	
/*	Rect iso_rect(disp_info.logical_w, disp_info.scene_h);
	drawRect(dst_s, &iso_rect, 0);
	drawMetamap(dst_s, -1000, -500);
*/
	return SUCCESS;
}

int IsoMap::drawMetamap(SURFACE *dst_s, int map_x, int map_y) {
/*	int meta_base_x = map_x;
	int meta_base_y = map_y;
	int meta_xi;
	int meta_yi;
	int meta_idx;
	Point platformPoint;

	for (meta_yi = SAGA_METAMAP_H - 1; meta_yi >= 0; meta_yi--) {
		platformPoint.x = meta_base_x;
		platformPoint.y = meta_base_y;
		for (meta_xi = SAGA_METAMAP_W - 1; meta_xi >= 0; meta_xi--) {
			meta_idx = meta_xi + (meta_yi * 16);
			drawMetaTile(dst_s, _metamap_tbl[meta_idx], platformPoint);
			platformPoint.x += 128;
			platformPoint.y += 64;
		}

		meta_base_x -= 128;
		meta_base_y += 64;
	}
*/
	return SUCCESS;
}

int IsoMap::drawMetaTile(SURFACE *ds, uint16 platformNumber, const Point &point) {
	int x;
	int y;
	
	TilePlatformData *tilePlatformData;
	Point tilePoint;
	Point tileBasePoint;

	if (platformNumber >= _tilePlatformsCount) {
		return FAILURE;
	}

	tilePlatformData = &_tilePlatformList[platformNumber];

	tileBasePoint = point;

	for (y = SAGA_PLATFORM_W - 1; y >= 0; y--) {
		tilePoint = tileBasePoint;
		for (x = SAGA_PLATFORM_W - 1; x >= 0; x--) {
			drawTile(ds, tilePlatformData->tiles[x][y], tilePoint);
			tilePoint.x += SAGA_ISOTILE_WIDTH / 2;
			tilePoint.y += SAGA_ISOTILE_BASEHEIGHT / 2 + 1;
		}
		tileBasePoint.x -= SAGA_ISOTILE_WIDTH / 2;
		tileBasePoint.y += SAGA_ISOTILE_BASEHEIGHT / 2 + 1;
	}

	return SUCCESS;
}

int IsoMap::drawTile(SURFACE *ds, uint16 tileNumber, const Point &point) {
	const byte *tilePointer;
	const byte *readPointer;
	byte *drawPointer;
	Point drawPoint;
	int height;
	int widthCount = 0;
	int row;
	int bg_runct;
	int fg_runct;
	int ct;


	if (tileNumber >= _tilesCount) {
		return FAILURE;
	}

	/* temporary x clip */
	if (point.x < 0) {
		return SUCCESS;
	}

	/* temporary x clip */
	if (point.x >= ds->w - 32) {
		return SUCCESS;
	}

	tilePointer = _tileData + _tilesTable[tileNumber].offset;
	height = _tilesTable[tileNumber].height;

	readPointer = tilePointer;
	drawPointer = (byte *)ds->pixels + point.x + (point.y * ds->pitch);

	drawPoint = point;

	if (height > SAGA_ISOTILE_BASEHEIGHT) {
		drawPoint.y -= (height - SAGA_ISOTILE_BASEHEIGHT);
	}

	// temporary y clip
	if (drawPoint.y < 0) {
		return SUCCESS;
	}

	for (row = 0; row < height; row++) {
		drawPointer = (byte *)ds->pixels + drawPoint.x + ((drawPoint.y + row) * ds->pitch);
		widthCount = 0;

		// temporary y clip
		if ((drawPoint.y + row) >= _vm->getDisplayInfo().sceneHeight) {
			return SUCCESS;
		}

		for (;;) {
			bg_runct = *readPointer++;
			widthCount += bg_runct;
			if (widthCount >= SAGA_ISOTILE_WIDTH) {
				break;
			}

			drawPointer += bg_runct;
			fg_runct = *readPointer++;
			widthCount += fg_runct;

			for (ct = 0; ct < fg_runct; ct++) {
				*drawPointer++ = *readPointer++;
			}
		}
	}

	return SUCCESS;
}

} // End of namespace Saga
