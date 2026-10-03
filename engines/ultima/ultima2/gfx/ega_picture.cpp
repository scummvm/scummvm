/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/ptr.h"
#include "ultima/ultima2/data/tiles.h"
#include "ultima/ultima2/gfx/ega_picture.h"

namespace Ultima {
namespace Ultima2 {
namespace Gfx {

constexpr int PIC_WIDTH = 320;
constexpr int PIC_HEIGHT = 200;

// Tile-map pictures: one tile code per 16x16 cell, row by row
constexpr int MAP_COLUMNS = PIC_WIDTH / Data::TILE_WIDTH;
constexpr int MAP_ROWS = 10;
constexpr int MAP_CELLS = MAP_COLUMNS * MAP_ROWS;

static bool isTileMapPicture(const Common::String &name) {
	return name == "picout" || name == "pictwn" || name == "piccas" || name == "picmin";
}

static bool loadFullPicture(const Common::String &name, Data::RenderMode mode,
		Graphics::ManagedSurface &surf) {
	Common::ScopedPtr<Common::SeekableReadStream> f(Data::openEgaFile(mode, name + ".ega"));
	if (!f)
		return false;

	surf.create(PIC_WIDTH, PIC_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	byte *dst = (byte *)surf.getPixels();

	// Each byte is a direct EGA color index, one per pixel
	byte row[PIC_WIDTH];
	for (int y = 0; y < PIC_HEIGHT; ++y) {
		if (f->read(row, PIC_WIDTH) != (uint32)PIC_WIDTH)
			return false;

		for (int x = 0; x < PIC_WIDTH; ++x)
			dst[y * PIC_WIDTH + x] = Data::EGA_PALETTE_BASE + (row[x] & 0xF);
	}

	return true;
}

static bool loadTileMapPicture(const Common::String &name, Data::RenderMode mode,
		Graphics::ManagedSurface &surf) {
	Common::ScopedPtr<Common::SeekableReadStream> f(Data::openEgaFile(mode, name + ".idx"));
	if (!f)
		return false;

	byte codes[MAP_CELLS];
	if (f->read(codes, MAP_CELLS) != (uint32)MAP_CELLS)
		return false;

	Graphics::Surface tiles[Data::TILE_COUNT];
	Data::loadTiles(tiles, mode);

	// Below the map's 10 rows of tiles is the caption area, left black
	surf.create(PIC_WIDTH, PIC_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	surf.clear(Data::EGA_PALETTE_BASE);

	for (int cell = 0; cell < MAP_CELLS; ++cell) {
		// Tile codes are the same as in the map files: tile id * 4
		int tileId = codes[cell] / 4;
		if (tileId >= Data::TILE_COUNT)
			continue;

		surf.blitFrom(tiles[tileId], Common::Point((cell % MAP_COLUMNS) * Data::TILE_WIDTH,
			(cell / MAP_COLUMNS) * Data::TILE_HEIGHT));
	}

	for (int t = 0; t < Data::TILE_COUNT; ++t)
		tiles[t].free();

	return true;
}

bool loadEgaPicture(const Common::String &name, Data::RenderMode mode,
		Graphics::ManagedSurface &surf) {
	return isTileMapPicture(name) ? loadTileMapPicture(name, mode, surf) :
		loadFullPicture(name, mode, surf);
}

} // namespace Gfx
} // namespace Ultima2
} // namespace Ultima
