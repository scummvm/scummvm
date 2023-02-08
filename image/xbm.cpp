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

#include "image/xbm.h"

#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Image {

const byte XBMDecoder::_palette[2 * 3] = {
	0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00
};

XBMDecoder::XBMDecoder() {
	_surface = 0;
}

XBMDecoder::~XBMDecoder() {
	destroy();
}

void XBMDecoder::destroy() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = 0;
	}

}

bool XBMDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	warning("External XBM files are not yet supported");
	return false;
}

bool XBMDecoder::loadBits(const unsigned char *bits, int width, int height) {
	destroy();

	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	for (int i = 0; i < height; i++) {
		byte *dst = (byte *)_surface->getBasePtr(0, i);
		for (int j = 0; j != width;) {
			byte color = *bits++;
			for (int k = 0; k < 8; k++) {
				*dst++ = (color & 1);
				color >>= 1;
				j++;
				if (j == width) {
					break;
				}
			}
		}
	}

	return true;
}

} // End of namespace Image
