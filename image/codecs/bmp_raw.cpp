/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "image/codecs/bmp_raw.h"

#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/surface.h"

namespace Image {

BitmapRawDecoder::BitmapRawDecoder(int width, int height, int bitsPerPixel) : Codec(),
		_surface(0), _width(width), _height(height), _bitsPerPixel(bitsPerPixel) {
}

BitmapRawDecoder::~BitmapRawDecoder() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

const Graphics::Surface *BitmapRawDecoder::decodeFrame(Common::SeekableReadStream &stream) {
	Graphics::PixelFormat format = getPixelFormat();

	_surface = new Graphics::Surface();
	_surface->create(_width, _height, format);

	int srcPitch = _width * (_bitsPerPixel >> 3);
	const int extraDataLength = (srcPitch % 4) ? 4 - (srcPitch % 4) : 0;

	if (_bitsPerPixel == 8) {
		byte *dst = (byte *)_surface->getPixels();

		for (int i = 0; i < _height; i++) {
			stream.read(dst + (_height - i - 1) * _width, _width);
			stream.skip(extraDataLength);
		}
	} else if (_bitsPerPixel == 24) {
		byte *dst = (byte *)_surface->getBasePtr(0, _height - 1);

		for (int i = 0; i < _height; i++) {
			for (int j = 0; j < _width; j++) {
				byte b = stream.readByte();
				byte g = stream.readByte();
				byte r = stream.readByte();
				uint32 color = format.RGBToColor(r, g, b);

				*((uint32 *)dst) = color;
				dst += format.bytesPerPixel;
			}

			stream.skip(extraDataLength);
			dst -= _surface->pitch * 2;
		}
	} else { // 32 bpp
		byte *dst = (byte *)_surface->getBasePtr(0, _height - 1);

		for (int i = 0; i < _height; i++) {
			for (int j = 0; j < _width; j++) {
				byte b = stream.readByte();
				byte g = stream.readByte();
				byte r = stream.readByte();
				// Ignore the last byte, as in v3 it is unused
				// and should thus NOT be used as alpha.
				// ref: http://msdn.microsoft.com/en-us/library/windows/desktop/dd183376%28v=vs.85%29.aspx
				stream.readByte();
				uint32 color = format.RGBToColor(r, g, b);

				*((uint32 *)dst) = color;
				dst += format.bytesPerPixel;
			}

			stream.skip(extraDataLength);
			dst -= _surface->pitch * 2;
		}
	}

	return _surface;
}

Graphics::PixelFormat BitmapRawDecoder::getPixelFormat() const {
	switch (_bitsPerPixel) {
	case 8:
		return Graphics::PixelFormat::createFormatCLUT8();
	case 24:
	case 32:
		return Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0);
	}

	error("Unhandled BMP raw %dbpp", _bitsPerPixel);
}

} // End of namespace Image
