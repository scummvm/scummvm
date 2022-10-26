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

#include "neo.h"

#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Image {

NeoDecoder::NeoDecoder(byte *palette) {
	_surface = nullptr;
	_paletteDestroy = palette ? false : true;
	_palette = palette;
	_paletteColorCount = 0;
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

	if (_paletteDestroy) {
		delete[] _palette;
		_palette = nullptr;
	}
	_paletteColorCount = 0;
}

bool NeoDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	if (!_palette) {
		int start = stream.pos();

		if (stream.readUint16LE() != 0x00)
			warning("Header check failed for reading neo image");

		if (stream.readUint16LE() != 0x00)
			warning("Header check failed for reading neo image");

		_palette = new byte[16 * 3];
		for (int i = 0; i < 16; ++i) {
			byte v1 = stream.readByte();
			byte v2 = stream.readByte();

			_palette[i * 3 + 0] = floor((v1 & 0x07) * 255.0 / 7.0);
			_palette[i * 3 + 1] = floor((v2 & 0x70) * 255.0 / 7.0 / 16.0);
			_palette[i * 3 + 2] = floor((v2 & 0x07) * 255.0 / 7.0);
		}

		stream.seek(start + 128);
	}

	int width = 320;
	int height = 200;
	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	_paletteColorCount = 16;

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
