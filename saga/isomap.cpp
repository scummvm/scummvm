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
	
	Rect iso_rect(_vm->getDisplayWidth(), _vm->getDisplayInfo().sceneHeight);
	drawRect(ds, &iso_rect, 0);
	drawTiles(ds);

	return SUCCESS;
}

void IsoMap::drawTiles(SURFACE *ds) {
	Point view1;
	Point meta;
	int16 u0, v0, 
		  u1, v1,
		  u2, v2,
		  uc, vc;
	int16 work_area_w;
	int16 work_area_h;
	int16 tx;
	uint16 metaTileIndex;

	_tileScroll.x = _viewScroll.x >> 4;
	_tileScroll.y = _viewScroll.y >> 4;

	view1.x = _tileScroll.x - (8 * SAGA_TILEMAP_W);
	view1.y = (8 * SAGA_TILEMAP_W) - _tileScroll.y;

	u0 = ((view1.y + 64) * 2 + view1.x) >> 4;
	v0 = ((view1.y + 64) * 2 - view1.x) >> 4;
	
	meta.x = (u0 - v0) * 128 - view1.x * 16;
	meta.y = view1.y * 16 - (u0 + v0) * 64;

	work_area_w = _vm->getDisplayWidth();
	work_area_h = _vm->getDisplayInfo().sceneHeight;
	setTileClip(0, work_area_w, 0, work_area_h);


	for (u1 = u0, v1 = v0; meta.y < work_area_h + 128 + 80; u1--, v1-- ) {
		tx = meta.x;

		for (u2 = u1, v2 = v1; tx < work_area_w + 128; u2++, v2--, tx += 256) {

			uc = u2 & (SAGA_TILEMAP_W - 1);
			vc = v2 & (SAGA_TILEMAP_W - 1);

			if (uc != u2 || vc != v2) {					
				metaTileIndex = 0;
				switch ( _tileMap.edgeType) {				
				case kEdgeTypeBlack: 
					continue;
				case kEdgeTypeFill0: 
					metaTileIndex = 0; //TODO:remove
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

			drawMetaTile(ds, metaTileIndex, tx, meta.y, u2 << 3, v2 << 3);
		}

		meta.y += 64;

		tx = meta.x - 128;

		for (u2 = u1 - 1, v2 = v1; tx < work_area_w + 128; u2++, v2--, tx += 256) {

			uc = u2 & (SAGA_TILEMAP_W - 1);
			vc = v2 & (SAGA_TILEMAP_W - 1);

			if (uc != u2 || vc != v2) {
				metaTileIndex = 0;
				switch ( _tileMap.edgeType) {				
				case kEdgeTypeBlack: 
					continue;
				case kEdgeTypeFill0: 
					metaTileIndex = 0; //TODO:remove
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

			drawMetaTile(ds, metaTileIndex, tx, meta.y, u2 << 3, v2 << 3);
		}
		meta.y += 64;
	}

}

void IsoMap::drawMetaTile(SURFACE *ds, uint16 metaTileIndex, int16 x, int16 y, int16 absU, int16 absV) {
	MetaTileData * metaTile;
	uint16 high;
	int16 platformIndex;
	
	if (_metaTilesCount <= metaTileIndex) {
		error("IsoMap::drawMetaTile wrong metaTileIndex");
	}

	metaTile = &_metaTileList[metaTileIndex];

	if (metaTile->highestPlatform > 18) {
		metaTile->highestPlatform = 0;
	}

	for (high = 0; high <= metaTile->highestPlatform; high++, y -= 8) {
		assert(SAGA_MAX_PLATFORM_H > high);
		platformIndex = metaTile->stack[high];

		if (platformIndex >= 0) {
			drawPlatform( ds, platformIndex, x, y, absU, absV, high );
		}
	}	
}

void IsoMap::drawPlatform(SURFACE *ds, uint16 platformIndex, int16 x, int16 y, int16 absU, int16 absV, int16 absH) {
	TilePlatformData *tilePlatform;
	int16 u, v;
	int16 sx0, sy0;
	Point s;
	int16 tileIndex;

	if (_tilePlatformsCount <= platformIndex) {
		error("IsoMap::drawPlatform wrong platformIndex");
	}

	tilePlatform = &_tilePlatformList[platformIndex];

	if ((y <= _tileClipTop) || (y - SAGA_MAX_TILE_H - SAGA_PLATFORM_W * SAGA_TILE_NOMINAL_H >= _tileClipBottom)) {
		return;
	}
		
	sx0 = x;
	sy0 = y - (((SAGA_PLATFORM_W - 1) + (SAGA_PLATFORM_W - 1)) * 8);

	for (v = SAGA_PLATFORM_W - 1; v >= 0 && sy0 - SAGA_MAX_TILE_H < _tileClipBottom && sx0 - 128 < _tileClipRight; v--, sx0 += 16, sy0 += 8) {

		if ((tilePlatform->vBits & (1 << v)) == 0) {
			continue;
		}

		if (sx0 + 128 + 32 < _tileClipLeft) {
			continue;
		}

		s.x = sx0;
		s.y = sy0;

		for (u = SAGA_PLATFORM_W - 1; u >= 0 && s.x + 32 > _tileClipLeft && s.y - SAGA_MAX_TILE_H < _tileClipBottom; u--, s.x -= 16, s.y += 8 ) {
			if (s.x < _tileClipRight && s.y > _tileClipTop) {
				
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

/*int IsoMap::drawMetamap(SURFACE *dst_s, int map_x, int map_y) {
	int meta_base_x = map_x;
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
*/
void IsoMap::drawTile(SURFACE *ds, uint16 tileIndex, const Point &point) {
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


	if (tileIndex >= _tilesCount) {
		error("IsoMap::drawTile wrong tileIndex");
	}


	/* temporary x clip */
	if (point.x < 0) {
		return;
	}

	/* temporary x clip */
	if (point.x >= _tileClipRight - 32) {
		return;
	}

	tilePointer = _tileData + _tilesTable[tileIndex].offset;
	height = _tilesTable[tileIndex].height;

	readPointer = tilePointer;
	drawPointer = (byte *)ds->pixels + point.x + (point.y * ds->pitch);

	drawPoint = point;

	if (point.y - height >= _tileClipBottom) {
		return;
	}

	if (height > SAGA_ISOTILE_BASEHEIGHT) {
		drawPoint.y -= (height - SAGA_ISOTILE_BASEHEIGHT);
	}

	// temporary y clip
	if (drawPoint.y < 0) {
		return;
	}

	for (row = 0; row < height; row++) {
		drawPointer = (byte *)ds->pixels + drawPoint.x + ((drawPoint.y + row) * ds->pitch);
		widthCount = 0;

		// temporary y clip
		if ((drawPoint.y + row) >= _tileClipBottom) {
			return;
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

}

} // End of namespace Saga
