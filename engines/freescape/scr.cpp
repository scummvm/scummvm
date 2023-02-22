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

#include "freescape/scr.h"

namespace Freescape {

ScrDecoder::ScrDecoder() {
	_surface = nullptr;
}

ScrDecoder::~ScrDecoder() {
	destroy();
}

void ScrDecoder::destroy() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}
}

uint32 ScrDecoder::getPixelAddress(int x, int y) {
	uint32 y76 = y & 0xc0;
	uint32 y53 = y & 0x38;
	uint32 y20 = y & 0x07;
	return (y76 << 5) + (y20 << 8) + (y53 << 2) + (x >> 3);
}

uint32 ScrDecoder::getAttributeAddress(int x, int y) {
	uint32 y73 = y & 0xf8;
	return (y73 << 2) + (x >> 3);
}

bool ScrDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	if (stream.size() != 6912)
		warning("Header check failed for reading scr image");

	byte *data = (byte *)malloc(6144 * sizeof(byte));
	byte *attributes = (byte *)malloc(768 * sizeof(byte));

	stream.read(data, 6144);
	stream.read(attributes, 768);

	int width = 256;
	int height = 192;
	Graphics::PixelFormat format(4, 8, 8, 8, 8, 24, 16, 8, 0);

	_surface = new Graphics::Surface();
	_surface->create(width, height, format);

    for (int y = 0; y < height; y++) {
		for (int col = 0; col < width >> 3; col++) {
			int x = col << 3;
			byte byt = data[getPixelAddress(x, y)];
			byte attr = attributes[getAttributeAddress(x, y)];
			byte ink = attr & 0x07;
			byte paper = (attr >> 3) & 0x07;
			byte bright = (attr >> 6) & 1;
			byte val = bright ? 0xff : 0xcd;
			for (int bit = 0; bit < 8; bit++) {
				bool set = (byt >> (7 - bit)) & 1;
				int color = set ? ink : paper;

				byte r = val * (color >> 1 & 1);
				byte g = val * (color >> 2 & 1);
				byte b = val * (color >> 0 & 1);

				_surface->setPixel(x + bit, y, format.ARGBToColor(0xFF, r, g, b));
			}
		}
	}

	free(data);
	free(attributes);
	return true;
}

} // End of namespace Freescape
