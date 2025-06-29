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

#include "image/scr.h"

namespace Image {

static const byte scrPalette[16 * 3] = {
	0x00, 0x00, 0x00, /* black */
	0x00, 0x00, 0xD8, /* blue */
	0xD8, 0x00, 0x00, /* red */
	0xD8, 0x00, 0xD8, /* magenta */
	0x00, 0xD8, 0x00, /* green */
	0x00, 0xD8, 0xD8, /* cyan */
	0xD8, 0xD8, 0x00, /* yellow */
	0xD8, 0xD8, 0xD8, /* white */
	0x00, 0x00, 0x00, /* bright black */
	0x00, 0x00, 0xFF, /* bright blue */
	0xFF, 0x00, 0x00, /* bright red */
	0xFF, 0x00, 0xFF, /* bright magenta */
	0x00, 0xFF, 0x00, /* bright green */
	0x00, 0xFF, 0xFF, /* bright cyan */
	0xFF, 0xFF, 0x00, /* bright yellow */
	0xFF, 0xFF, 0xFF, /* bright white */
};

ScrDecoder::ScrDecoder() : _surface(nullptr), _palette(scrPalette, 16) {
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
	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	for (int y = 0; y < height; y++) {
		for (int col = 0; col < width >> 3; col++) {
			int x = col << 3;
			byte byt = data[getPixelAddress(x, y)];
			byte attr = attributes[getAttributeAddress(x, y)];
			byte ink = attr & 0x07;
			byte paper = (attr >> 3) & 0x07;
			byte bright = (attr >> 6) & 1;
			for (int bit = 0; bit < 8; bit++) {
				bool set = (byt >> (7 - bit)) & 1;
				int color = (bright << 3) | (set ? ink : paper);
				_surface->setPixel(x + bit, y, color);
			}
		}
	}

	free(data);
	free(attributes);
	return true;
}

} // End of namespace Image
