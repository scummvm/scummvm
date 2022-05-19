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

bool ScreenDecoder::loadFile(const Common::String &fname) {
	Common::File f;
	return f.open(fname) && loadStream(f);
}

bool ScreenDecoder::loadStream(Common::SeekableReadStream &stream) {
	byte bytes[IMAGE_SIZE];
	byte v;
	int len;
	const byte *srcP;
	byte *destP = &bytes[0];
	int index = 0;

	int size = stream.readUint16LE();
	if (size < 0 || size > stream.size())
		return false;

	// Decompress the image bytes
	while (size > 0) {
		v = stream.readByte();
		if (v != 0x7B) {
			len = 1;
			--size;
		} else {
			len = stream.readByte() + 1;
			v = stream.readByte();
			size -= 3;
		}

		for (; len > 0; --len) {
			destP[index] = v;

			index += 80;
			if (index >= IMAGE_SIZE) {
				index = 0;
				++destP;
			}
		}
	}

	// Create surface from splitting up the nibbles
	_surface.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	srcP = &bytes[0];
	destP = (byte *)_surface.getPixels();

	for (size_t i = 0; i < IMAGE_SIZE; ++i, ++srcP) {
		v = *srcP;
		for (int j = 0; j < 4; ++j, v <<= 2)
			*destP++ = v >> 6;
	}

	return true;
}

} // namespace Gfx
} // End of namespace Xeen
} // End of namespace MM
