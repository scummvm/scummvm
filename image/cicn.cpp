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

#include "image/cicn.h"
#include "image/pict.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Image {

CicnDecoder::CicnDecoder() {
	_surface = nullptr;
	_palette = nullptr;
	_paletteColorCount = 0;
	_mask = nullptr;
}

CicnDecoder::~CicnDecoder() {
	destroy();
}

void CicnDecoder::destroy() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}

	delete[] _palette;
	_palette = nullptr;
	_paletteColorCount = 0;

	if (_mask) {
		_mask->free();
		delete _mask;
		_mask = nullptr;
	}
}



bool CicnDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	Image::PICTDecoder::PixMap pixMap = Image::PICTDecoder::readPixMap(stream);

	// Mask header
	stream.skip(4);
	uint16 maskRowBytes = stream.readUint16BE();
	stream.readUint16BE(); // top
	stream.readUint16BE(); // left
	uint16 maskHeight = stream.readUint16BE(); // bottom
	uint16 maskWidth = stream.readUint16BE(); // right

	// Bitmap header
	stream.skip(4);
	uint16 bitmapRowBytes = stream.readUint16BE();
	stream.readUint16BE(); // top
	stream.readUint16BE(); // left
	uint16 bitmapHeight = stream.readUint16BE(); // bottom
	stream.readUint16BE(); // right

	// Mask and bitmap data
	stream.skip(4);

	if (maskRowBytes && maskHeight) {
		_mask = new Graphics::Surface();
		_mask->create(maskWidth, maskHeight, Graphics::PixelFormat::createFormatCLUT8());

		byte *mask = new byte[maskRowBytes * maskHeight];
		stream.read(mask, maskRowBytes * maskHeight);

		for (uint y = 0; y < maskHeight; y++) {
			for (uint x = 0; x < maskWidth; x++) {
				if ((mask[y * maskRowBytes + x / 8] & (0x80 >> (x % 8))) == 0)
					_mask->setPixel(x, y, 0);
				else
					_mask->setPixel(x, y, 255);
			}
		}

		delete[] mask;
	}
	stream.skip(bitmapRowBytes * bitmapHeight);

	// Palette
	stream.skip(6);
	_paletteColorCount = stream.readUint16BE() + 1;

	_palette = new byte[3 * _paletteColorCount];

	byte *p = _palette;

	for (uint i = 0; i < _paletteColorCount; i++) {
		stream.skip(2);
		*p++ = stream.readUint16BE() >> 8;
		*p++ = stream.readUint16BE() >> 8;
		*p++ = stream.readUint16BE() >> 8;
	}

	_surface = new Graphics::Surface();
	_surface->create(pixMap.bounds.width(), pixMap.bounds.height(), Graphics::PixelFormat::createFormatCLUT8());

	if (pixMap.pixelSize == 2) {
		byte *buf = new byte[pixMap.rowBytes];
		for (int y = 0; y < pixMap.bounds.height(); y++) {
			stream.read(buf, pixMap.rowBytes);
			for (int x = 0; x < pixMap.bounds.width(); x += 4) {
				for (int i = 0; i < 4 && x + i < pixMap.bounds.width(); i++) {
					_surface->setPixel(x + i, y, (buf[x / 4] >> (6 - 2 * i)) & 0x03);
				}
			}
		}
		delete[] buf;
	} else if (pixMap.pixelSize == 4) {
		byte *buf = new byte[pixMap.rowBytes];
		for (int y = 0; y < pixMap.bounds.height(); y++) {
			stream.read(buf, pixMap.rowBytes);
			for (int x = 0; x < pixMap.bounds.width(); x += 2) {
				for (int i = 0; i < 2 && x + i < pixMap.bounds.width(); i++) {
					_surface->setPixel(x + i, y, (buf[x / 2] >> (4 - 4 * i)) & 0x0F);
				}
			}
		}
		delete[] buf;
	} else if (pixMap.pixelSize == 8) {
		stream.read(_surface->getPixels(), pixMap.rowBytes * pixMap.bounds.height());
	} else {
		error("CicnDecoder::loadStream(): Invalid pixel size %d", pixMap.pixelSize);
	}

	return true;
}

} // End of namespace Image
