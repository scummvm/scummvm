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
#include "graphics/screen.h"
#include "mm/mm1/gfx/gfx.h"
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

bool ScreenDecoder::loadFile(const Common::Path &fname,
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
			*destP++ = EGA_INDEXES[_indexes[v >> 6]];
	}

	return true;
}

} // namespace Gfx
} // End of namespace Xeen
} // End of namespace MM
