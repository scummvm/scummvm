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

#include "common/file.h"
#include "image/png.h"
#include "ultima/ultima2/gfx/dungeon_pictures.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Gfx {

constexpr int PICTURE_WIDTH = 256;
constexpr int PICTURE_HEIGHT = 128;
constexpr int PALETTE_COUNT = 256 - DungeonPictures::PALETTE_BASE;

static const char *const KIND_NAMES[DungeonPictures::KIND_COUNT] = {
	"front", "left", "right", "leftopen", "rightopen", "frontdoor", "leftdoor", "rightdoor",
	"ladderup", "ladderdown", "chest"
};

DungeonPictures::Picture DungeonPictures::_pictures[KIND_COUNT][DEPTHS];
DungeonPictures::Picture DungeonPictures::_doorway;
DungeonPictures::Picture DungeonPictures::_monsters[MONSTER_TYPES][DEPTHS];
bool DungeonPictures::_enabled = false;
bool DungeonPictures::_loaded = false;
bool DungeonPictures::_paletteLoaded = false;
byte DungeonPictures::_inverse[256];

bool DungeonPictures::isActive() {
	if (!_enabled || !_paletteLoaded)
		return false;

	if (!_loaded)
		load();

	const Data::RenderMode mode = g_engine->getRenderMode();
	return mode == Data::RENDER_EGA || mode == Data::RENDER_EGA_V1 || mode == Data::RENDER_EGA_WILTSHIRE;
}

bool DungeonPictures::loadPicture(Picture &picture, const Common::String &name) {
	Common::File f;
	Image::PNGDecoder decoder;
	decoder.setKeepTransparencyPaletted(true);

	if (!f.open(Common::Path(name + ".png")) || !decoder.loadStream(f))
		return false;

	const Graphics::Surface *surface = decoder.getSurface();
	if (!surface || surface->format.bytesPerPixel != 1 || surface->w != PICTURE_WIDTH ||
			surface->h != PICTURE_HEIGHT)
		return false;

	// Only the part that isn't clear is kept
	Common::Rect bounds;
	for (int y = 0; y < PICTURE_HEIGHT; ++y) {
		const byte *row = (const byte *)surface->getBasePtr(0, y);
		for (int x = 0; x < PICTURE_WIDTH; ++x) {
			if (row[x]) {
				bounds.left = MIN<int>(bounds.left, x);
				bounds.right = MAX<int>(bounds.right, x + 1);
				bounds.top = MIN<int>(bounds.top, y);
				bounds.bottom = MAX<int>(bounds.bottom, y + 1);
			}
		}
	}

	if (bounds.isEmpty())
		return false;

	delete[] picture._pixels;
	picture._left = bounds.left;
	picture._top = bounds.top;
	picture._width = bounds.width();
	picture._height = bounds.height();
	picture._pixels = new byte[bounds.width() * bounds.height()];
	for (int y = 0; y < bounds.height(); ++y)
		memcpy(picture._pixels + y * bounds.width(), surface->getBasePtr(bounds.left, bounds.top + y), bounds.width());

	return true;
}

void DungeonPictures::freePictures() {
	for (int kind = 0; kind < KIND_COUNT; ++kind) {
		for (int depth = 0; depth < DEPTHS; ++depth) {
			delete[] _pictures[kind][depth]._pixels;
			_pictures[kind][depth]._pixels = nullptr;
		}
	}

	delete[] _doorway._pixels;
	_doorway._pixels = nullptr;

	for (int type = 0; type < MONSTER_TYPES; ++type) {
		for (int depth = 0; depth < DEPTHS; ++depth) {
			delete[] _monsters[type][depth]._pixels;
			_monsters[type][depth]._pixels = nullptr;
		}
	}
}

void DungeonPictures::unload() {
	freePictures();
	_loaded = false;
}

void DungeonPictures::load() {
	freePictures();
	_loaded = true;

	for (int kind = 0; kind < KIND_COUNT; ++kind) {
		for (int depth = 0; depth < DEPTHS; ++depth)
			loadPicture(_pictures[kind][depth], Common::String::format("dungeons/%s_%d", KIND_NAMES[kind], depth));
	}

	loadPicture(_doorway, "dungeons/doorway");

	for (int type = 1; type < MONSTER_TYPES; ++type) {
		for (int depth = 1; depth < DEPTHS; ++depth)
			loadPicture(_monsters[type][depth], Common::String::format("monsters/monster_%d_%d", type, depth));
	}
}

static bool isUsedEntry(int index) {
	return index < Data::EGA_PALETTE_BASE + Data::EGA_PALETTE_SIZE ?
		index >= Data::EGA_PALETTE_BASE : index >= DungeonPictures::PALETTE_BASE;
}

void DungeonPictures::loadPalette(Graphics::Palette &palette) {
	_paletteLoaded = false;

	Common::File f;
	byte colors[PALETTE_COUNT * 3];
	if (!f.open("dungeons/palette.pal") || f.read(colors, sizeof(colors)) != sizeof(colors))
		return;

	palette.resize(256, true);
	palette.set(colors, PALETTE_BASE, PALETTE_COUNT);
	_paletteLoaded = true;

	// The opposite of each color is the nearest of them to what's left from white
	byte rgb[256][3];
	for (int i = 0; i < 256; ++i)
		palette.get(i, rgb[i][0], rgb[i][1], rgb[i][2]);

	for (int i = 0; i < 256; ++i) {
		_inverse[i] = i;
		if (!isUsedEntry(i))
			continue;

		int best = i, bestDistance = 1 << 30;
		for (int j = 0; j < 256; ++j) {
			if (!isUsedEntry(j))
				continue;

			int dr = rgb[j][0] - (255 - rgb[i][0]);
			int dg = rgb[j][1] - (255 - rgb[i][1]);
			int db = rgb[j][2] - (255 - rgb[i][2]);
			int distance = dr * dr + dg * dg + db * db;
			if (distance < bestDistance) {
				bestDistance = distance;
				best = j;
			}
		}

		_inverse[i] = best;
	}
}

byte DungeonPictures::invert(byte color) {
	return _inverse[color];
}

void DungeonPictures::draw(Graphics::Views::GfxSurface &s, const Picture &picture, int offset) {
	const int width = picture._width;

	for (int y = 0; y < picture._height; ++y) {
		const byte *row = picture._pixels + y * width;

		for (int x = 0; x < width; ++x) {
			if (row[x])
				s.setPixel(offset + picture._left + x, offset + picture._top + y, row[x]);
		}
	}
}

bool DungeonPictures::draw(Graphics::Views::GfxSurface &s, Kind kind, int depth, int offset) {
	if (depth < 0 || depth >= DEPTHS || !_pictures[kind][depth].isPresent())
		return false;

	draw(s, _pictures[kind][depth], offset);
	return true;
}

bool DungeonPictures::drawDoorway(Graphics::Views::GfxSurface &s, int offset) {
	if (!_doorway.isPresent())
		return false;

	draw(s, _doorway, offset);
	return true;
}

bool DungeonPictures::drawMonster(Graphics::Views::GfxSurface &s, int type, int depth, int offset) {
	if (type < 1 || type >= MONSTER_TYPES || depth < 1 || depth >= DEPTHS || !_monsters[type][depth].isPresent())
		return false;

	draw(s, _monsters[type][depth], offset);
	return true;
}

} // namespace Gfx
} // namespace Ultima2
} // namespace Ultima
