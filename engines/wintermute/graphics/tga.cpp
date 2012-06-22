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
 */

/* Based on code from eos https://github.com/DrMcCoy/xoreos/
 * relicensed under GPLv2+ with permission from DrMcCoy and clone2727
 */

#include "common/util.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/error.h"

#include "engines/wintermute/graphics/tga.h"

namespace WinterMute {

TGA::TGA() {

}

TGA::~TGA() {
	destroy();
}

void TGA::destroy() {
	_surface.free();
}

bool TGA::loadStream(Common::SeekableReadStream &tga) {
	byte imageType, pixelDepth;
	bool success;
	success = readHeader(tga, imageType, pixelDepth);
	success = readData(tga, imageType, pixelDepth);

	if (tga.err() || !success) {
		warning("Failed reading TGA-file");
		return false;
	}
	return success;
}

bool TGA::readHeader(Common::SeekableReadStream &tga, byte &imageType, byte &pixelDepth) {
	if (!tga.seek(0)) {
		warning("Failed reading TGA-file");
		return false;
	}

	// TGAs have an optional "id" string in the header
	uint32 idLength = tga.readByte();

	// Number of colors in the color map / palette
	if (tga.readByte() != 0) {
		warning("Unsupported feature: Color map");
		return false;
	}

	// Image type. 2 == unmapped RGB, 3 == Grayscale
	imageType = tga.readByte();
	if ((imageType != 2) && (imageType != 3)) {
		warning("Unsupported image type: %d", imageType);
		return false;
	}

	// Color map specifications + X + Y
	tga.skip(5 + 2 + 2);

	// Image dimensions
	_surface.w = tga.readUint16LE();
	_surface.h = tga.readUint16LE();

	// Bits per pixel
	pixelDepth = tga.readByte();
	_surface.format.bytesPerPixel = pixelDepth / 8;

	if (imageType == 2) {
		if (pixelDepth == 24) {
			_hasAlpha  = false;
			_format = Graphics::PixelFormat(pixelDepth / 8, 8, 8, 8, 0, 16, 8, 0, 0);
		} else if (pixelDepth == 16 || pixelDepth == 32) {
			_hasAlpha  = true;
			_format = Graphics::PixelFormat(pixelDepth / 8, 8, 8, 8, 8, 24, 16, 8, 0);
		} else {
			warning("Unsupported pixel depth: %d, %d", imageType, pixelDepth);
			return false;
		}
	} else if (imageType == 3) {
		if (pixelDepth != 8) {
			warning("Unsupported pixel depth: %d, %d", imageType, pixelDepth);
			return false;
		}

		_hasAlpha  = false;
		_format = Graphics::PixelFormat(1, 0, 0, 0, 0, 0, 0, 0, 0);
	}

	// Image descriptor
	tga.skip(1);

	// Skip the id string
	tga.skip(idLength);
	return true;
}

bool TGA::readData(Common::SeekableReadStream &tga, byte imageType, byte pixelDepth) {
	if (imageType == 2) {
		_surface.create(_surface.w, _surface.h, _format);

		if (pixelDepth == 16) {
			// Convert from 16bpp to 32bpp
			// 16bpp TGA is ARGB1555
			uint16 count = _surface.w * _surface.h;
			byte *dst = (byte *)_surface.pixels;

			while (count--) {
				uint16 pixel = tga.readUint16LE();

				*dst++ = (pixel & 0x1F) << 3;
				*dst++ = (pixel & 0x3E0) >> 2;
				*dst++ = (pixel & 0x7C00) >> 7;
				*dst++ = (pixel & 0x8000) ? 0xFF : 0x00;
			}

		} else {
			// Read it in raw
			tga.read(_surface.pixels, _surface.pitch * _surface.w);
		}
	} else if (imageType == 3) {
		_surface.create(_surface.w, _surface.h, _surface.format);

		byte  *data  = (byte *)_surface.pixels;
		uint32 count = _surface.w * _surface.h;

		while (count-- > 0) {
			byte g = tga.readByte();

			memset(data, g, 3);
			data[3] = 0xFF;

			data += 4;
		}

	}
	return true;
}

} // End of namespace Graphics
