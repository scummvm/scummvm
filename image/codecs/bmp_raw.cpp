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

#include "image/codecs/bmp_raw.h"

#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/surface.h"

namespace Image {

BitmapRawDecoder::BitmapRawDecoder(int width, int height, int bitsPerPixel, bool ignoreAlpha, bool flip) : Codec(),
		_width(width), _height(height), _bitsPerPixel(bitsPerPixel), _ignoreAlpha(ignoreAlpha), _flip(flip)  {
	_surface.create(_width, _height, getPixelFormat());
}

BitmapRawDecoder::~BitmapRawDecoder() {
	_surface.free();
}

const Graphics::Surface *BitmapRawDecoder::decodeFrame(Common::SeekableReadStream &stream) {
	Graphics::PixelFormat format = getPixelFormat();

	int srcPitch = _width * (_bitsPerPixel >> 3);
	int extraDataLength = (srcPitch % 4) ? 4 - (srcPitch % 4) : 0;

	if (_bitsPerPixel == 1) {
		srcPitch = (_width + 7) / 8;
		extraDataLength = (srcPitch % 2) ? 2 - (srcPitch % 2) : 0;
	} else if (_bitsPerPixel == 4) {
		srcPitch = (_width + 1) / 2;
		extraDataLength = (srcPitch % 4) ? 4 - (srcPitch % 4) : 0;
	}

	if (_bitsPerPixel == 1) {
		for (int i = 0; i < _height; i++) {
			byte *dst = (byte *)_surface.getBasePtr(0, i);
			for (int j = 0; j != _width;) {
				byte color = stream.readByte();
				for (int k = 0; k < 8; k++) {
					*dst++ = (color & 0x80) ? 0x0f : 0x00;
					color <<= 1;
					j++;
					if (j == _width) {
						break;
					}
				}
			}
			stream.skip(extraDataLength);
		}
	} else if (_bitsPerPixel == 4) {
		for (int i = 0; i < _height; i++) {
			byte *dst = (byte *)_surface.getBasePtr(0, _height - i - 1);
			for (int j = 0; j < _width; j++) {
				byte color = stream.readByte();

				*dst++ = (color & 0xf0) >> 4;
				j++;

				if (j ==_width)
					break;

				*dst++ = color & 0x0f;
			}

			stream.skip(extraDataLength);
		}
	} else if (_bitsPerPixel == 8) {
		// flip the 8bpp images when we are decoding QTvideo
		byte *dst = (byte *)_surface.getPixels();

		for (int i = 0; i < _height; i++) {
			stream.read(dst + (_flip ? i : _height - i - 1) * _width, _width);
			stream.skip(extraDataLength);
		}
#ifndef SCUMM_LITTLE_ENDIAN
	} else if (_bitsPerPixel == 16) {
		byte *dst = (byte *)_surface.getBasePtr(0, _height - 1);

		for (int i = 0; i < _height; i++) {
			for (int j = 0; j < _width; j++) {
				uint16 color = stream.readUint16LE();

				*(uint16 *)dst = color;
				dst += format.bytesPerPixel;
			}

			stream.skip(extraDataLength);
			dst -= _surface.pitch * 2;
		}
#endif
	} else {
		byte *dst = (byte *)_surface.getBasePtr(0, _height - 1);
		uint bpp = format.bytesPerPixel;

		for (int i = 0; i < _height; i++) {
			stream.read(dst, _width * bpp);
			stream.skip(extraDataLength);
			dst -= _surface.pitch;
		}
	}

	return &_surface;
}

Graphics::PixelFormat BitmapRawDecoder::getPixelFormat() const {
	switch (_bitsPerPixel) {
	case 1:
	case 4:
	case 8:
		return Graphics::PixelFormat::createFormatCLUT8();
	case 16:
		return Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
#ifdef SCUMM_LITTLE_ENDIAN
	case 24:
		return Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0);
	case 32:
		return Graphics::PixelFormat(4, 8, 8, 8, _ignoreAlpha ? 0 : 8, 16, 8, 0, 24);
#else
	case 24:
		return Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);
	case 32:
		return Graphics::PixelFormat(4, 8, 8, 8, _ignoreAlpha ? 0 : 8, 8, 16, 24, 0);
#endif
	default:
		break;
	}

	error("Unhandled BMP raw %dbpp", _bitsPerPixel);
}

} // End of namespace Image
