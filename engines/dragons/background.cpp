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
#include <common/memstream.h>
#include "graphics/screen.h"
#include "common/endian.h"
#include "background.h"

namespace Dragons {

#define TILE_WIDTH 32
#define TILE_HEIGHT 8
#define TILE_SIZE (TILE_WIDTH * TILE_HEIGHT * 2)

struct TileMap {
	uint16 w;
	uint16 h;
	uint32 size;
	byte *map;
	uint8 tileIndexOffset;
};

void Dragons::PriorityLayer::load(TileMap &tileMap, byte *tiles) {
	_width = tileMap.w * TILE_WIDTH;
	_height = tileMap.h * TILE_HEIGHT;
	_mapWidth = tileMap.w;
	_mapHeight = tileMap.h;
	size_t tileSize = (size_t)tileMap.tileIndexOffset * TILE_SIZE;
	_map = new byte[tileMap.size];
	_values = new byte[tileSize];

	memcpy(_map, tileMap.map, tileMap.size);
	memcpy(_values, tiles, tileSize);
}

int16 PriorityLayer::getPriority(Common::Point pos) {
	pos.x = CLIP<int16>(pos.x, 0, _width - 1);
	pos.y = CLIP<int16>(pos.y, 0, _height - 1);
	const int16 tx = pos.x / TILE_WIDTH, sx = pos.x % TILE_WIDTH;
	const int16 ty = pos.y / TILE_HEIGHT, sy = pos.y % TILE_HEIGHT;
	uint16 mapIndex = READ_LE_UINT16(_map + 2 * (tx + ty * _mapWidth));
	return _values[mapIndex * TILE_WIDTH * TILE_HEIGHT + sx + sy * TILE_WIDTH] + 1;

}

bool Background::load(byte *dataStart, Common::SeekableReadStream &stream) {
	stream.read(_palette, 512);
	_palette[0] = 0x80; //FIXME update palette
	_palette[1] = 0x80;

	_points1 = loadPoints(stream); // 0x200
	_points2 = loadPoints(stream); // 0x280
	stream.seek(0x305);
	uint8 tileindexOffset = stream.readByte();
	stream.seek(0x308);

	uint32 tilemapOffset = 0x324;
	TileMap tileMap[4];
	for(int i=0;i< 3;i++) {
		tileMap[i].w = stream.readUint16LE();
		tileMap[i].h = stream.readUint16LE();
		tileMap[i].size = stream.readUint32LE();
		tileMap[i].map = dataStart + tilemapOffset;
		tileMap[i].tileIndexOffset = tileindexOffset;
		debug("Tilemap (%d,%d) map: %X", tileMap[i].w, tileMap[i].h, tilemapOffset);

		tilemapOffset += tileMap[i].size;
	}

	uint32 finalSize = stream.readUint32LE();

	tileMap[3].w = tileMap[0].w;
	tileMap[3].h = tileMap[0].h;
	tileMap[3].size = tileMap[0].size;
	tileMap[3].map = dataStart + tilemapOffset;
	tileMap[3].tileIndexOffset = tileindexOffset;

	uint32 tilesOffset = tilemapOffset + finalSize;

	_priorityLayer = new PriorityLayer();
	_priorityLayer->load(tileMap[3], dataStart + tilesOffset);

	debug("Tiles: %X", tilesOffset);
	debug("tileIndexOffset: %d", tileMap[0].tileIndexOffset);
	_bgLayer = loadGfxLayer(tileMap[0], dataStart + tilesOffset);
	_mgLayer = loadGfxLayer(tileMap[1], dataStart + tilesOffset);
	_fgLayer = loadGfxLayer(tileMap[2], dataStart + tilesOffset);
	return false;
}

Common::Point *Background::loadPoints(Common::SeekableReadStream &stream) {
	Common::Point *points = new Common::Point[0x20];

	for (int i=0;i < 0x20;i++) {
		points[i].x = stream.readUint16LE();
		points[i].y = stream.readUint16LE();
	}

	return points;
}

Graphics::Surface *Background::loadGfxLayer(TileMap &tileMap, byte *tiles) {
	Graphics::Surface *surface = new Graphics::Surface();
	Graphics::PixelFormat pixelFormat16(2, 5, 5, 5, 1, 10, 5, 0, 15); //TODO move this to a better location.
	surface->create(tileMap.w * TILE_WIDTH, tileMap.h * TILE_HEIGHT, pixelFormat16);

	for(int y = 0; y < tileMap.h; y++) {
		for(int x = 0; x < tileMap.w; x++) {
			uint16 idx = READ_LE_UINT16(&tileMap.map[(y * tileMap.w + x) * 2]) + tileMap.tileIndexOffset;
			//debug("tileIdx: %d", idx);
			drawTileToSurface(surface, tiles + idx * 0x100, x * TILE_WIDTH, y * TILE_HEIGHT);
		}
	}
	return surface;
}

void Background::drawTileToSurface(Graphics::Surface *surface, byte *tile, uint32 x, uint32 y) {
	byte *pixels = (byte *)surface->getPixels();
	for(int ty = 0; ty < TILE_HEIGHT; ty++) {
		for(int tx = 0; tx < TILE_WIDTH; tx++) {
			uint32 cidx = *tile;
			uint32 offset = (y + ty) * surface->pitch + (x + tx) * 2;
			pixels[offset] = _palette[cidx * 2];
			pixels[offset + 1] = _palette[cidx * 2 + 1];
			tile++;
		}
	}
}

Common::Point Background::getPoint2(uint32 pointIndex) {
	assert (pointIndex < 0x20);
	return _points2[pointIndex];
}

uint16 Background::getWidth() {
	assert (_bgLayer);
	return _bgLayer->w;
}

uint16 Background::getHeight() {
	assert (_bgLayer);
	return _bgLayer->h;
}

int16 Background::getPriorityAtPoint(Common::Point pos) {
	if (pos.x < 0 || pos.x >= _bgLayer->w || pos.y < 0 || pos.y >= _bgLayer->h) {
		return -1;
	}
	int16 priority = _priorityLayer->getPriority(pos);
	return priority < 0x11 ? priority : 0;
}

BackgroundResourceLoader::BackgroundResourceLoader(BigfileArchive *bigFileArchive, DragonRMS *dragonRMS) : _bigFileArchive(
		bigFileArchive), _dragonRMS(dragonRMS) {}

Background *BackgroundResourceLoader::load(uint32 sceneId) {
	char filename[] = "nnnn.scr";
	memcpy(filename, _dragonRMS->getSceneName(sceneId), 4);
	debug("Loading %s", filename);
	uint32 size;
	byte *scrData = _bigFileArchive->load(filename, size);
	Common::SeekableReadStream *readStream = new Common::MemoryReadStream(scrData, size, DisposeAfterUse::YES);
	Background *bg = new Background();
	bg->load(scrData, *readStream);
	return bg;
}

} // End of namespace Dragons
