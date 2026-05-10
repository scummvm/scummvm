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

#include "image/pcx.h"

#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

/**
 * Based on the PCX specs:
 * https://web.archive.org/web/20151028182714/http://www.fileformat.info/format/pcx/spec/a10e75307b3a4cc49c3bbe6db4c41fa2/view.htm
 * and the PCX decoder of FFmpeg (libavcodec/pcx.c):
 * https://git.ffmpeg.org/gitweb/ffmpeg.git/blob/tags/n0.11.1:/libavcodec/pcx.c
 */

namespace Image {

PCXDecoder::PCXDecoder(): _surface(nullptr), _palette(0) {
}

PCXDecoder::~PCXDecoder() {
	destroy();
}

void PCXDecoder::destroy() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}

	_palette.clear();
}

bool PCXDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	if (stream.readByte() != 0x0a)	// ZSoft PCX
		return false;

	byte version = stream.readByte();	// 0 - 5
	if (version > 5)
		return false;

	bool compressed = stream.readByte(); // encoding, 1 = run length encoding
	byte bitsPerPixel = stream.readByte();	// 1, 2, 4 or 8

	// Window
	uint16 xMin = stream.readUint16LE();
	uint16 yMin = stream.readUint16LE();
	uint16 xMax = stream.readUint16LE();
	uint16 yMax = stream.readUint16LE();

	uint16 width  = xMax - xMin + 1;
	uint16 height = yMax - yMin + 1;

	if (xMax < xMin || yMax < yMin) {
		warning("Invalid PCX image dimensions");
		return false;
	}

	stream.skip(4);	// HDpi, VDpi

	// Read the EGA palette (colormap)
	_palette.resize(16, false);
	for (uint16 i = 0; i < 16; i++) {
		byte r = stream.readByte();
		byte g = stream.readByte();
		byte b = stream.readByte();
		_palette.set(i, r, g, b);
	}

	if (stream.readByte() != 0)	// reserved, should be set to 0
		return false;

	byte nPlanes = stream.readByte();
	uint16 bytesPerLine = stream.readUint16LE();
	uint16 bytesPerscanLine = nPlanes * bytesPerLine;

	if (bytesPerscanLine < width * bitsPerPixel * nPlanes / 8) {
		warning("PCX data is corrupted");
		return false;
	}

	stream.skip(60);	// PaletteInfo, HscreenSize, VscreenSize, Filler

	_surface = new Graphics::Surface();

	byte *scanLine = new byte[bytesPerscanLine];
	byte *dst;
	int x, y;

	if (nPlanes == 3 && bitsPerPixel == 8) {	// 24bpp
		_surface->create(width, height, Graphics::PixelFormat::createFormatRGB24());
		dst = (byte *)_surface->getPixels();
		_palette.clear();

		for (y = 0; y < height; y++) {
			decodeRLE(stream, scanLine, bytesPerscanLine, compressed);

			for (x = 0; x < width; x++) {
				*dst++ = scanLine[x];
				*dst++ = scanLine[x +  bytesPerLine];
				*dst++ = scanLine[x + (bytesPerLine << 1)];
			}
		}
	} else if (nPlanes == 1 && bitsPerPixel == 8) {	// 8bpp indexed
		_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
		dst = (byte *)_surface->getPixels();
		_palette.resize(16, true);

		for (y = 0; y < height; y++, dst += _surface->pitch) {
			decodeRLE(stream, scanLine, bytesPerscanLine, compressed);
			memcpy(dst, scanLine, width);
		}

		if (version == 5) {
			if (stream.readByte() != 12) {
				warning("Expected a palette after the PCX image data");
				delete[] scanLine;
				return false;
			}

			// Read the VGA palette
			_palette.resize(256, false);
			for (uint16 i = 0; i < 256; i++) {
				byte r = stream.readByte();
				byte g = stream.readByte();
				byte b = stream.readByte();
				_palette.set(i, r, g, b);
			}
		}
	} else if ((nPlanes == 2 || nPlanes == 3 || nPlanes == 4) && bitsPerPixel == 1) {	// planar, 4, 8 or 16 colors
		_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
		dst = (byte *)_surface->getPixels();
		_palette.resize(16, true);

		for (y = 0; y < height; y++, dst += _surface->pitch) {
			decodeRLE(stream, scanLine, bytesPerscanLine, compressed);

			for (x = 0; x < width; x++) {
				int m = 0x80 >> (x & 7), v = 0;
				for (int i = nPlanes - 1; i >= 0; i--) {
					v <<= 1;
					v  += (scanLine[i * bytesPerLine + (x >> 3)] & m) == 0 ? 0 : 1;
				}
				dst[x] = v;
			}
		}
	} else {
		// Known unsupported case: 1 plane and bpp < 8 (1, 2 or 4)
		warning("Invalid PCX file (%d planes, %d bpp)", nPlanes, bitsPerPixel);
		delete[] scanLine;
		return false;
	}

	delete[] scanLine;

	return true;
}

void PCXDecoder::decodeRLE(Common::SeekableReadStream &stream, byte *dst, uint32 bytesPerscanLine, bool compressed) {
	uint32 i = 0;
	byte run, value;

	if (compressed) {
		while (i < bytesPerscanLine) {
			run = 1;
			value = stream.readByte();
			if (value >= 0xc0) {
				run = value & 0x3f;
				value = stream.readByte();
			}
			while (i < bytesPerscanLine && run--)
				dst[i++] = value;
		}
	} else {
		stream.read(dst, bytesPerscanLine);
	}
}

} // End of namespace Image
