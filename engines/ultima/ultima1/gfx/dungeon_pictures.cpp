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
#include "ultima/ultima1/gfx/dungeon_pictures.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Gfx {

constexpr int PICTURE_WIDTH = 288;
constexpr int PICTURE_HEIGHT = 144;
constexpr int PALETTE_COUNT = 256 - DungeonPictures::PALETTE_BASE;

static const char *const KIND_NAMES[DungeonPictures::KIND_COUNT] = {
	"front", "left", "right", "leftopen", "rightopen", "leftedge", "rightedge", "frontdoor",
	"leftdoor", "rightdoor", "doorway", "beams"
};

static const char *const WIDGET_NAMES[DungeonPictures::WIDGET_COUNT] = {
	"ladderuphole", "ladderdownhole", "ladderupfaceon", "ladderdownfaceon", "ladderupsideways",
	"ladderdownsideways", "chest", "coffin"
};

DungeonPictures::Picture DungeonPictures::_pictures[KIND_COUNT][DEPTHS];
DungeonPictures::Picture DungeonPictures::_widgets[WIDGET_COUNT][TIERS];
DungeonPictures::Picture DungeonPictures::_monsters[MONSTER_TYPES][MONSTER_DISTANCES];
bool DungeonPictures::_enabled = false;
bool DungeonPictures::_loaded = false;
bool DungeonPictures::_paletteLoaded = false;

bool DungeonPictures::isActive() {
	if (!_enabled || !_paletteLoaded)
		return false;

	if (!_loaded)
		load();

	return g_engine->_renderMode == Data::RENDER_EGA;
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

	for (int widget = 0; widget < WIDGET_COUNT; ++widget) {
		for (int tier = 0; tier < TIERS; ++tier) {
			delete[] _widgets[widget][tier]._pixels;
			_widgets[widget][tier]._pixels = nullptr;
		}
	}

	for (int type = 0; type < MONSTER_TYPES; ++type) {
		for (int distance = 0; distance < MONSTER_DISTANCES; ++distance) {
			delete[] _monsters[type][distance]._pixels;
			_monsters[type][distance]._pixels = nullptr;
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

	for (int widget = 0; widget < WIDGET_COUNT; ++widget) {
		for (int tier = 0; tier < TIERS; ++tier)
			loadPicture(_widgets[widget][tier], Common::String::format("dungeons/%s_%d", WIDGET_NAMES[widget], tier));
	}

	for (int type = 0; type < MONSTER_TYPES; ++type) {
		for (int distance = 1; distance < MONSTER_DISTANCES; ++distance)
			loadPicture(_monsters[type][distance], Common::String::format("monsters/monster_%d_%d", type, distance));
	}
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
}

void DungeonPictures::draw(Graphics::Views::GfxSurface &s, const Picture &picture) {
	const int width = picture._width;

	for (int y = 0; y < picture._height; ++y) {
		const byte *row = picture._pixels + y * width;

		for (int x = 0; x < width; ++x) {
			if (row[x])
				s.setPixel(VIEW_LEFT + picture._left + x, VIEW_TOP + picture._top + y, row[x]);
		}
	}
}

bool DungeonPictures::draw(Graphics::Views::GfxSurface &s, Kind kind, int depth) {
	if (depth < 0 || depth >= DEPTHS || !_pictures[kind][depth].isPresent())
		return false;

	draw(s, _pictures[kind][depth]);
	return true;
}

bool DungeonPictures::drawWidget(Graphics::Views::GfxSurface &s, Widget widget, int tier) {
	if (tier < 0 || tier >= TIERS || !_widgets[widget][tier].isPresent())
		return false;

	draw(s, _widgets[widget][tier]);
	return true;
}

bool DungeonPictures::drawMonster(Graphics::Views::GfxSurface &s, int type, int distance) {
	if (type < 0 || type >= MONSTER_TYPES || distance < 1 || distance >= MONSTER_DISTANCES ||
			!_monsters[type][distance].isPresent())
		return false;

	draw(s, _monsters[type][distance]);
	return true;
}

} // namespace Gfx
} // namespace Ultima1
} // namespace Ultima
