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
#include "common/file.h"
#include "image/bmp.h"
#include "graphics/paletteman.h"
#include "bagel/hodjnpodj/gfx/palette.h"

namespace Bagel {
namespace HodjNPodj {

Palette::Palette() {
}

void Palette::loadInitialPalette() {
	Common::File f;
	Image::BitmapDecoder decoder;
	const char *FILENAME = "meta/art/mlscroll.bmp";

	if (!f.open(FILENAME) || !decoder.loadStream(f))
		error("Error loading - %s", FILENAME);

	loadPalette(decoder.getPalette());
}

void Palette::setPalette(const byte *palette) {
	g_system->getPaletteManager()->setPalette(palette, 0, Graphics::PALETTE_COUNT);
}

void Palette::setPalette(const Graphics::Palette &palette) {
	g_system->getPaletteManager()->setPalette(palette);
}

Graphics::Palette Palette::getPalette() const {
	return g_system->getPaletteManager()->grabPalette(0, Graphics::PALETTE_COUNT);
}

void Palette::loadPalette(const byte *palette) {
	_gamePalette = Graphics::Palette(palette, Graphics::PALETTE_COUNT);
	setPalette(_gamePalette);
}

void Palette::loadPalette(const Graphics::Palette &palette) {
	_gamePalette = palette;
	setPalette(_gamePalette);
}

byte Palette::getPaletteIndex(uint32 color) const {
	byte r = color & 0xff;
	byte g = (color >> 8) & 0xff;
	byte b = (color >> 16) & 0xff;
	return _gamePalette.findBestColor(r, g, b);
}

} // namespace HodjNPodj
} // namespace Bagel
