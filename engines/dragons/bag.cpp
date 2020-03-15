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
#include "dragons/bigfile.h"
#include "dragons/screen.h"
#include "dragons/bag.h"
#include "dragons/background.h"

namespace Dragons {

#define TILEMAP_WIDTH 10
#define TILEMAP_HEIGHT 25

Bag::Bag(BigfileArchive *bigFileArchive, Screen *screen): _screen(screen) {
	_surface = nullptr;
	_position.x = 0;
	_position.y = 0;
	load(bigFileArchive);
}

Bag::~Bag() {
	_surface->free();
	delete _surface;
}

void Bag::updatePosition(Common::Point newPos) {
	_position = newPos;
}

void Bag::draw() {
	_screen->copyRectToSurface(*_surface, _position.x, _position.y, Common::Rect(_surface->w, _surface->h));
}

void Bag::load(BigfileArchive *bigFileArchive) {
	byte pal[512];
	byte tilemap[0x1f4];
	byte *tiles;
	uint32 size;
	byte *scrData = bigFileArchive->load("bag.scr", size);
	Common::MemoryReadStream stream(scrData, size, DisposeAfterUse::YES);

	stream.seek(0x4);
	stream.read(pal, 512);

	pal[0] = 0x0;
	pal[1] = 0x0;
	for (int i = 1; i < 0x100; i++) {
		uint c = READ_LE_INT16(&pal[i * 2]);
		if (c == 0) {
			c = 0x8000;
		} else {
			//c = (uint16)(((uint)c & 0x1f) << 10) | (uint16)(((uint)c & 0x7c00) >> 10) | c & 0x3e0;
		}
		WRITE_LE_UINT16(&pal[i * 2], c);
	}

	stream.seek(0x308);
	stream.read(tilemap, 0x1f4);

	stream.seek(0xadc);

	size = stream.readUint32LE();
	tiles = (byte *)malloc(size);
	stream.read(tiles, size);

	_surface = new Graphics::Surface();
	Graphics::PixelFormat pixelFormat16(2, 5, 5, 5, 1, 10, 5, 0, 15); //TODO move this to a better location.
	_surface->create(320, 200, pixelFormat16);

	for (int y = 0; y < TILEMAP_HEIGHT; y++) {
		for (int x = 0; x < TILEMAP_WIDTH; x++) {
			uint16 idx = READ_LE_UINT16(&tilemap[(y * TILEMAP_WIDTH + x) * 2]);
			//debug("tileIdx: %d", idx);
			drawTileToSurface(_surface, pal, tiles + idx * 0x100, x * 32, y * 8);
		}
	}

	free(tiles);
}

} // End of namespace Dragons
