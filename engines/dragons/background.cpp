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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "common/memstream.h"
#include "common/endian.h"
#include "dragons/background.h"
#include "dragons/screen.h"

namespace Dragons {

#define TILE_WIDTH 32
#define TILE_HEIGHT 8
#define TILE_SIZE (TILE_WIDTH * TILE_HEIGHT)


void PriorityLayer::load(TileMap &tileMap, byte *tiles) {
	_width = tileMap.w * TILE_WIDTH;
	_height = tileMap.h * TILE_HEIGHT;
	_mapWidth = tileMap.w;
	_mapHeight = tileMap.h;
	size_t tileSize = (size_t)tileMap.tileIndexOffset * TILE_SIZE;
	_map = new byte[tileMap.size];
	_mapBase = new byte[tileMap.size];
	_values = new byte[tileSize];

	memcpy(_map, tileMap.map, tileMap.size);
	memcpy(_mapBase, tileMap.map, tileMap.size);
	memcpy(_values, tiles, tileSize);
}

int16 PriorityLayer::getPriority(Common::Point pos) {
	pos.x = CLIP<int16>(pos.x, 0, _width - 1);
	pos.y = CLIP<int16>(pos.y, 0, _height - 1);
	const int16 tx = pos.x / TILE_WIDTH, sx = pos.x % TILE_WIDTH;
	const int16 ty = pos.y / TILE_HEIGHT, sy = pos.y % TILE_HEIGHT;
	uint16 mapIndex = READ_LE_UINT16(_map + 2 * (tx + ty * _mapWidth));

//
//	byte priority = *(byte *)((((uint)*(uint16 *)
//			(((int)pos.x / 32) * 2 +
//			 ((int)pos.y / 8) * (uint)_mapWidth * 2 +
//					_map) * 32) * 8) +
//			_values + (((int)pos.y % 8) * 32) +
//						 ((int)pos.x % 32));
	return _values[mapIndex * TILE_WIDTH * TILE_HEIGHT + sx + sy * TILE_WIDTH] + 1;
}

void PriorityLayer::overlayTileMap(byte *data, int16 x, int16 y, int16 w, int16 h) {
	byte *ptr = _map + (x + y * _mapWidth) * 2;
	byte *src = data;
	for (int i = 0; i < h; i++) {
		memcpy(ptr, src, w * 2);
		src += w * 2;
		ptr += _mapWidth * 2;
	}
}

void PriorityLayer::restoreTileMap(int16 x, int16 y, int16 w, int16 h) {
	byte *ptr = _map + (x + y * _mapWidth) * 2;
	byte *src = _mapBase + (x + y * _mapWidth) * 2;
	for (int i = 0; i < h; i++) {
		memcpy(ptr, src, w * 2);
		src += _mapWidth * 2;
		ptr += _mapWidth * 2;
	}

}

Background::Background() : _priorityLayer(0), _points2(0), _data(0) {
	_layerSurface[0] = nullptr;
	_layerSurface[1] = nullptr;
	_layerSurface[2] = nullptr;
	_layerOffset[0] = Common::Point(0, 0);
	_layerOffset[1] = Common::Point(0, 0);
	_layerOffset[2] = Common::Point(0, 0);
	_layerAlphaMode[0] = NONE;
	_layerAlphaMode[1] = NONE;
	_layerAlphaMode[2] = NONE;

	_tileDataOffset = nullptr;
}

Background::~Background() {
	if (_data) {
		delete _data;
	}

	for (int i = 0; i < 3; i++) {
		if (_layerSurface[i]) {
			_layerSurface[i]->free();
			delete _layerSurface[i];
		}
	}

}

bool Background::load(byte *dataStart, uint32 size) {
	Common::MemoryReadStream stream(dataStart, size, DisposeAfterUse::NO);
	_data = dataStart;

	stream.read(_palette, 512);
	_palette[0] = 0x00; //FIXME update palette
	_palette[1] = 0x00;

	_scaleLayer.load(stream); // 0x200
	_points2 = loadPoints(stream); // 0x280
	stream.seek(0x305);
	uint8 tileindexOffset = stream.readByte();
	stream.seek(0x308);

	uint32 tilemapOffset = 0x324;
	for (int i = 0; i < 3; i++) {
		_tileMap[i].w = stream.readUint16LE();
		_tileMap[i].h = stream.readUint16LE();
		_tileMap[i].size = stream.readUint32LE();
		_tileMap[i].map = dataStart + tilemapOffset;
		_tileMap[i].tileIndexOffset = tileindexOffset;
		debug(3, "Tilemap (%d, %d) map: %X", _tileMap[i].w, _tileMap[i].h, tilemapOffset);

		tilemapOffset += _tileMap[i].size;
	}

	uint32 finalSize = stream.readUint32LE();

	TileMap priorityTilemap;

	priorityTilemap.w = _tileMap[0].w;
	priorityTilemap.h = _tileMap[0].h;
	priorityTilemap.size = _tileMap[0].size;
	priorityTilemap.map = dataStart + tilemapOffset;
	priorityTilemap.tileIndexOffset = tileindexOffset;

	uint32 tilesOffset = tilemapOffset + finalSize;

	_tileDataOffset = _data + tilesOffset;

	_priorityLayer = new PriorityLayer();
	_priorityLayer->load(priorityTilemap, _tileDataOffset);

	debug(3, "Tiles: %X", tilesOffset);
	debug(3, "tileIndexOffset: %d", _tileMap[0].tileIndexOffset);

	for (int i = 0; i < 3; i++) {
		_layerSurface[i] = initGfxLayer(_tileMap[i]);
		loadGfxLayer(_layerSurface[i], _tileMap[i], _tileDataOffset);
	}

	_layerPriority[0] = 1;
	_layerPriority[1] = 2;
	_layerPriority[2] = 3;

	return false;
}

Common::Point *Background::loadPoints(Common::SeekableReadStream &stream) {
	Common::Point *points = new Common::Point[0x20];

	for (int i = 0; i < 0x20; i++) {
		points[i].x = stream.readUint16LE();
		points[i].y = stream.readUint16LE();
	}

	return points;
}

Graphics::Surface *Background::initGfxLayer(TileMap &tileMap) {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(tileMap.w * TILE_WIDTH, tileMap.h * TILE_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	return surface;
}

void Background::loadGfxLayer(Graphics::Surface *surface, TileMap &tileMap, byte *tiles) {
	for (int y = 0; y < tileMap.h; y++) {
		for (int x = 0; x < tileMap.w; x++) {
			uint16 idx = READ_LE_UINT16(&tileMap.map[(y * tileMap.w + x) * 2]) + tileMap.tileIndexOffset;
			//debug("tileIdx: %d", idx);
			drawTileToSurface(surface, _palette, tiles + idx * 0x100, x * TILE_WIDTH, y * TILE_HEIGHT);
		}
	}
}

void drawTileToSurface(Graphics::Surface *surface, byte *palette, byte *tile, uint32 x, uint32 y) {
	byte *pixels = (byte *)surface->getPixels();
	if (surface->format.bpp() == 16) {
		for (int ty = 0; ty < TILE_HEIGHT; ty++) {
			for (int tx = 0; tx < TILE_WIDTH; tx++) {
				uint32 cidx = *tile;
				uint32 offset = (y + ty) * surface->pitch + (x + tx) * 2;
				pixels[offset] = palette[cidx * 2];
				pixels[offset + 1] = palette[cidx * 2 + 1];
				tile++;
			}
		}
	} else {
		for (int ty = 0; ty < TILE_HEIGHT; ty++) {
			memcpy(&pixels[(y + ty) * surface->pitch + x], tile, TILE_WIDTH);
			tile += TILE_WIDTH;
		}
	}
}

Common::Point Background::getPoint2(uint32 pointIndex) {
	assert (pointIndex < 0x20);
	return _points2[pointIndex];
}

uint16 Background::getWidth() {
	assert (_layerSurface[1]);
	return _layerSurface[1]->w;
}

uint16 Background::getHeight() {
	assert (_layerSurface[1]);
	return _layerSurface[1]->h;
}

int16 Background::getPriorityAtPoint(Common::Point pos) {
	if (pos.x < 0 || pos.x >= getWidth() || pos.y < 0 || pos.y >= getHeight()) {
		return -1;
	}
	int16 priority = _priorityLayer->getPriority(pos);
	return priority < 0x11 ? priority : 0;
}

void Background::overlayImage(uint16 layerNum, byte *data, int16 x, int16 y, int16 w, int16 h) {
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			int16 idx = READ_LE_UINT16(data) + _tileMap[layerNum].tileIndexOffset;
			drawTileToSurface(_layerSurface[layerNum],
					_palette,
					_tileDataOffset + idx * 0x100,
					(j + x) * TILE_WIDTH,
					(i + y) * TILE_HEIGHT);
			data += 2;
		}
	}
}

void Background::restoreTiles(uint16 layerNum, int16 x1, int16 y1, int16 w, int16 h) {
	int16 tmw = x1 + w;
	int16 tmh = y1 + h;
	for (int y = y1; y < tmh; y++) {
		for (int x = x1; x < tmw; x++) {
			uint16 idx = READ_LE_UINT16(&_tileMap[layerNum].map[(y * _tileMap[layerNum].w + x) * 2]) + _tileMap[layerNum].tileIndexOffset;
			//debug("tileIdx: %d", idx);
			drawTileToSurface(_layerSurface[layerNum], _palette, _tileDataOffset + idx * 0x100, x * TILE_WIDTH, y * TILE_HEIGHT);
		}
	}
}

void Background::overlayPriorityTileMap(byte *data, int16 x, int16 y, int16 w, int16 h) {
	_priorityLayer->overlayTileMap(data, x, y, w, h);
}

void Background::restorePriorityTileMap(int16 x, int16 y, int16 w, int16 h) {
	_priorityLayer->restoreTileMap(x, y, w, h);
}

void Background::setPalette(byte *newPalette) {
	memcpy(_palette, newPalette, 512);
	for (int i = 0; i < 3; i++) {
		loadGfxLayer(_layerSurface[i], _tileMap[i], _tileDataOffset);
	}
}

void Background::setLayerOffset(uint8 layerNumber, Common::Point offset) {
	assert(layerNumber < 4);
	_layerOffset[layerNumber] = offset;
}

Common::Point Background::getLayerOffset(uint8 layerNumber) {
	assert(layerNumber < 4);
	return _layerOffset[layerNumber];
}

AlphaBlendMode Background::getLayerAlphaMode(uint8 layerNumber) {
	assert(layerNumber < 4);
	return _layerAlphaMode[layerNumber];
}

void Background::setLayerAlphaMode(uint8 layerNumber, AlphaBlendMode mode) {
	assert(layerNumber < 4);
	_layerAlphaMode[layerNumber] = mode;
}

BackgroundResourceLoader::BackgroundResourceLoader(BigfileArchive *bigFileArchive, DragonRMS *dragonRMS) : _bigFileArchive(
	bigFileArchive), _dragonRMS(dragonRMS) {}

Background *BackgroundResourceLoader::load(uint32 sceneId) {
	char filename[] = "nnnn.scr";
	memcpy(filename, _dragonRMS->getSceneName(sceneId), 4);
	return load(filename);
}

Background *BackgroundResourceLoader::load(const char *filename) {
	debug(1, "Loading %s", filename);
	uint32 size;
	byte *scrData = _bigFileArchive->load(filename, size);
	Background *bg = new Background();
	bg->load(scrData, size);
	return bg;
}

void ScaleLayer::load(Common::SeekableReadStream &stream) {
	for (int i = 0; i < 32; i++) {
		_bands[i]._y = stream.readSint16LE();
		_bands[i]._priority = stream.readSint16LE();
	}
}

uint16 ScaleLayer::getScale(uint16 y) {
	short yBand;
	uint uVar1;
	short local_v0_368;
	int iVar3;
	short lowerYBandIdx;
	ScaleBand *pSVar4;
	short upperYBandIdx;
	uint uVar5;
	ScaleBand *pSVar6;
	int uVar7;


	upperYBandIdx = -1;
	for (int16 i = 0x1f; i >= 0; i--) {
		yBand = _bands[i]._y;
		if (yBand != -1 && yBand <= y) {
			upperYBandIdx = i;
			break;
		}
	}
//	iVar3 = 0x1f0000;
//	do {
//		yBand = *(uint16 *)((int)&scaleBandTbl->y + (iVar3 >> 0xe));
//		if ((yBand != 0xffff) && (yBand <= y)) break;
//		i = i + -1;
//		iVar3 = i * 0x10000;
//	} while (-1 < i * 0x10000);
	lowerYBandIdx = 32;
	for (int i = 0; i < 32; i++) {
		yBand = _bands[i]._y;
		if (yBand != -1 && y <= yBand) {
			lowerYBandIdx = i;
			break;
		}
	}
//	j = 0;
//	iVar3 = 0;
//	do {
//		lowerYBandIdx = (short)j;
//		yBand = *(uint16 *)((int)&scaleBandTbl->y + (iVar3 >> 0xe));
//		if ((yBand != 0xffff) && (y <= yBand)) break;
//		j = j + 1;
//		lowerYBandIdx = (short)j;
//		iVar3 = j * 0x10000;
//	} while (j * 0x10000 >> 0x10 < 0x20);
	if ((upperYBandIdx == -1) && (lowerYBandIdx == 0x20)) {
		return 0x100;
	}
	if (upperYBandIdx < 0 || lowerYBandIdx >= 32) {
		if (upperYBandIdx >= 0) {
			lowerYBandIdx = upperYBandIdx;
		}
		upperYBandIdx = lowerYBandIdx;
	}


	pSVar6 = &_bands[upperYBandIdx];  //scaleBandTbl + (int)upperYBandIdx;
	uVar7 = (0x21 - (uint)pSVar6->_priority) * 8;
	uVar1 = uVar7;
	if (y != pSVar6->_y) {
		pSVar4 = &_bands[lowerYBandIdx]; //scaleBandTbl + (int)lowerYBandIdx;
		uVar5 = (0x21 - (uint)pSVar4->_priority) * 8;
		uVar1 = uVar5;
		if ((y != pSVar4->_y) && (uVar1 = uVar7, (int)upperYBandIdx != (int)lowerYBandIdx)) {
			local_v0_368 = pSVar4->_y - pSVar6->_y;
			uVar1 = uVar5;
			if (local_v0_368 != 0) {
				iVar3 = ((uVar5 & 0xffffu) - (uVar7 & 0xffffu)) * (uint)(uint16)(y - pSVar6->_y);

				assert(((uint16)local_v0_368 != 0xffffu) || (iVar3 != (int)-0x80000000));

				return (uVar7 + iVar3 / (int)(uint)(uint16)local_v0_368) & 0xffff;
			}
		}
	}
	return uVar1 & 0xfff8u;
}

ScaleLayer::ScaleLayer(): _savedBands(nullptr) {
	for (int i = 0; i < 32; i++) {
		_bands[i]._y = -1;
		_bands[i]._priority = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
	}
}

ScaleLayer::~ScaleLayer() {
	delete _savedBands;
}

void ScaleLayer::backup() {
	delete _savedBands;
	_savedBands = new ScaleBand[32];
	memcpy(_savedBands, _bands, sizeof(_bands));
}

void ScaleLayer::restore() {
	assert(_savedBands);
	memcpy(_bands, _savedBands, sizeof(_bands));
}

void ScaleLayer::clearAll() {
	for (int i = 0; i < 32; i++) {
		_bands[i]._y = -1;
	}
}

void ScaleLayer::setValue(uint8 index, int16 y, int16 value) {
	assert(index < 32);
	_bands[index]._y = y;
	_bands[index]._priority = value;
}

} // End of namespace Dragons
