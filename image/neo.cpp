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

#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "image/neo.h"

namespace Image {

NeoDecoder::NeoDecoder(byte *palette) : _palette(0) {
	_surface = nullptr;
	if (palette) {
		_palette.resize(16, false);
		_palette.set(palette, 0, 16);
	}
}

NeoDecoder::~NeoDecoder() {
	destroy();
}

void NeoDecoder::destroy() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}

	_palette.clear();
}

bool NeoDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	if (_palette.empty()) {
		int start = stream.pos();

		if (stream.readUint16LE() != 0x00)
			warning("Header check failed for reading neo image");

		if (stream.readUint16LE() != 0x00)
			warning("Header check failed for reading neo image");

		_palette.resize(16, false);
		for (int i = 0; i < 16; ++i) {
			byte v1 = stream.readByte();
			byte v2 = stream.readByte();

			byte r = floor((v1 & 0x07) * 255.0 / 7.0);
			byte g = floor((v2 & 0x70) * 255.0 / 7.0 / 16.0);
			byte b = floor((v2 & 0x07) * 255.0 / 7.0);
			_palette.set(i, r, g, b);
		}

		stream.seek(start + 128);
	}

	int width = 320;
	int height = 200;
	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	// 200 rows of image:
	for (int y = 0; y < 200; y++) {
		// 20 column blocks:
		for (int x = 0; x < 20; x++) {
			// Fetch the 4 words that make up the
			// next 16 pixels across 4 bitplanes:
			uint16 uW0 = stream.readUint16BE();
			uint16 uW1 = stream.readUint16BE();
			uint16 uW2 = stream.readUint16BE();
			uint16 uW3 = stream.readUint16BE();

			// The first pixel is found in the highest bit:
			uint32 uBit = 0x8000;

			// 16 pixels to process:
			for (int z = 0; z < 16; z++) {
				// Work out the colour index:
				int idx = 0;
				if (uW0 & uBit)
					idx += 1;
				if (uW1 & uBit)
					idx += 2;
				if (uW2 & uBit)
					idx += 4;
				if (uW3 & uBit)
					idx += 8;

				_surface->setPixel(x * 16 + z, y, idx);
				uBit >>= 1;
			}
		}
	}
	return true;
}

} // End of namespace Image