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
#include "image/bmp.h"
#include "image/png.h"
#include "mm/mm1/gfx/symbols.h"

namespace MM {
namespace MM1 {
namespace Gfx {

void Symbols::load() {
	Common::File f;
	if (!f.open("symbols.bin"))
		error("Could not load symbols.bin");

	for (uint i = 0; i < MAX_SYMBOLS; ++i) {
		const Common::String fname1 = Common::String::format("gfx//symbols.bin//image%.2d.bmp", i);
		const Common::String fname2 = Common::String::format("gfx//symbols.bin//image%.2d.bmp", i);

		Common::File fOverride1, fOverride2;
		Image::BitmapDecoder bmpDecoder;
		Image::PNGDecoder pngDecoder;

		if (fOverride1.open(fname1.c_str()) && bmpDecoder.loadStream(fOverride1)) {
			// Load the bitmap
			_data[i].copyFrom(*bmpDecoder.getSurface());
			f.skip(SYMBOL_WIDTH * SYMBOL_HEIGHT);
		} else if (fOverride2.open(fname2.c_str()) && pngDecoder.loadStream(fOverride2)) {
			// Load the png
			_data[i].copyFrom(*pngDecoder.getSurface());
			f.skip(SYMBOL_WIDTH * SYMBOL_HEIGHT);
		} else {
			// Fall back on the default
			_data[i].create(SYMBOL_WIDTH, SYMBOL_HEIGHT);
			f.read((byte *)_data[i].getPixels(), SYMBOL_WIDTH * SYMBOL_HEIGHT);
		}
	}

	f.close();
}

void Symbols::draw(Graphics::ManagedSurface &dest, const Common::Point &destPos, int symbolNum) {
	assert(symbolNum < MAX_SYMBOLS);
	dest.blitFrom(_data[symbolNum], destPos);
}

} // namespace Gfx
} // namespace MM1
} // namespace MM
