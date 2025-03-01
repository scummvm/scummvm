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

#include "common/system.h"
#include "graphics/paletteman.h"
#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/gfx/bmp_button.h"

namespace Bagel {
namespace HodjNPodj {

void BmpButton::loadBitmaps(const char *base, const char *selected,
		const char *focused, const char *disabled) {
	loadBitmap(base, _base);
	loadBitmap(selected, _selected);
	loadBitmap(focused, _focused);
	loadBitmap(disabled, _disabled);
}

void BmpButton::loadBitmap(const char *name, Graphics::ManagedSurface &field) {
	assert(name);

	Image::BitmapDecoder decoder;
	Common::File f;
	if (!f.open(name) || !decoder.loadStream(f))
		error("Could not load bitmap - %s", name);

	// Create a lookup between the bitmap palette and game palette
	byte destPal[PALETTE_SIZE];
	g_system->getPaletteManager()->grabPalette(destPal, 0, PALETTE_COUNT);
	Graphics::PaletteLookup lookup(destPal, PALETTE_COUNT);
	uint32 *map = lookup.createMap(decoder.getPalette(),
		decoder.getPaletteColorCount());

	// Get the bitmap
	field.copyFrom(decoder.getSurface());

	// Translate the pixels using the lookup
	byte *pixel = (byte *)field.getPixels();
	for (int i = 0; i < field.w * field.h; ++i, ++pixel)
		*pixel = map[*pixel];

	delete[] map;
}

void BmpButton::clear() {
	_base.clear();
	_selected.clear();
	_focused.clear();
	_disabled.clear();
}

void BmpButton::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_base);
}

} // namespace HodjNPodj
} // namespace Bagel
