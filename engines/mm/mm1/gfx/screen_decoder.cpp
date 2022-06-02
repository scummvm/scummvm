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
#include "common/system.h"
#include "graphics/palette.h"
#include "graphics/screen.h"
#include "mm/mm1/gfx/screen_decoder.h"

namespace MM {
namespace MM1 {
namespace Gfx {

#define IMAGE_SIZE 16000

ScreenDecoder::~ScreenDecoder() {
	destroy();
}

void ScreenDecoder::destroy() {
	_surface.free();
}

bool ScreenDecoder::loadFile(const Common::String &fname,
		int16 w, int16 h) {
	Common::File f;
	if (!f.open(fname))
		return false;

	f.skip(2);		// Skip size word
	return loadStream(f, w, h);
}

bool ScreenDecoder::loadStream(Common::SeekableReadStream &stream,
		int16 w, int16 h) {
	byte bytes[IMAGE_SIZE];
	byte v;
	int len;
	const byte *srcP;
	byte *destP = &bytes[0];
	int index = 0;
	int imgSize = w * h / 4;

	byte indexes[4];
	getPaletteIndexes(indexes);

	// Decompress the image bytes
	int x = 0;
	while (x < (w / 4) && !stream.eos()) {
		v = stream.readByte();
		if (v != 0x7B) {
			len = 1;
		} else {
			len = stream.readByte() + 1;
			v = stream.readByte();
		}

		for (; len > 0; --len) {
			destP[index] = v;

			index += (w / 4);
			if (index >= imgSize) {
				index = 0;
				++destP;
				++x;
			}
		}
	}

	// Create surface from splitting up the pairs of bits
	_surface.free();
	_surface.create(w, h, Graphics::PixelFormat::createFormatCLUT8());
	srcP = &bytes[0];
	destP = (byte *)_surface.getPixels();

	for (int i = 0; i < w * h / 4; ++i, ++srcP) {
		v = *srcP;
		for (int j = 0; j < 4; ++j, v <<= 2)
			*destP++ = indexes[v >> 6];
	}

	return true;
}

void ScreenDecoder::getPaletteIndexes(byte indexes[4]) {
	byte pal[PALETTE_SIZE];
	g_system->getPaletteManager()->grabPalette(pal, 0, PALETTE_COUNT);

	indexes[0] = 0;
	indexes[1] = findPalette(pal, 168, 84, 0);
	indexes[2] = findPalette(pal, 252, 252, 84);
	indexes[3] = findPalette(pal, 0xff, 0xff, 0xff);
}

byte ScreenDecoder::findPalette(const byte *pal, byte r, byte g, byte b) {
	int closestDiff = 0x7fffffff;
	byte closest = 0;

	for (int i = 0; i < PALETTE_COUNT; ++i, pal += 3) {
		int diff = ABS((int)r - (int)pal[0]) +
			ABS((int)g - (int)pal[1]) +
			ABS((int)b - (int)pal[2]);
		if (diff < closestDiff) {
			closestDiff = diff;
			closest = i;
		}
	}

	return closest;
}


} // namespace Gfx
} // End of namespace Xeen
} // End of namespace MM
