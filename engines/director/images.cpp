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

#include "common/stream.h"
#include "common/substream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "graphics/palette.h"
#include "image/codecs/codec.h"
#include "common/util.h"
#include "common/debug.h"
#include "image/codecs/bmp_raw.h"
#include "common/system.h"

#include "director/images.h"

namespace Director {

DIBDecoder::DIBDecoder() {
	_surface = 0;
	_palette = 0;
	_paletteColorCount = 0;
	_codec = 0;
}

DIBDecoder::~DIBDecoder() {
	destroy();
}

void DIBDecoder::destroy() {
	_surface = 0;

	delete[] _palette;
	_palette = 0;
	_paletteColorCount = 0;

	delete _codec;
	_codec = 0;
}

void DIBDecoder::loadPalette(Common::SeekableReadStream &stream) {
	uint16 steps = stream.size() / 6;
	uint16 index = (steps * 3) - 1;
	_paletteColorCount = steps;
	_palette = new byte[index + 1];

	for (uint8 i = 0; i < steps; i++) {
		_palette[index - 2] = stream.readByte();
		stream.readByte();

		_palette[index - 1] = stream.readByte();
		stream.readByte();

		_palette[index] = stream.readByte();
		stream.readByte();
		index -= 3;
	}
}

bool DIBDecoder::loadStream(Common::SeekableReadStream &stream) {
	uint32 headerSize = stream.readUint32LE();
	if (headerSize != 40)
		return false;

	uint32 width = stream.readUint32LE();
	uint32 height = stream.readUint32LE();
	stream.readUint16LE(); // planes
	uint16 bitsPerPixel = stream.readUint16LE();
	uint32 compression = stream.readUint32BE();
	uint32 imageSize = stream.readUint32LE();
	/* uint32 pixelsPerMeterX = */ stream.readUint32LE();
	/* uint32 pixelsPerMeterY = */ stream.readUint32LE();
	_paletteColorCount = stream.readUint32LE();
	/* uint32 colorsImportant = */ stream.readUint32LE();

	_paletteColorCount = (_paletteColorCount == 0) ? 255: _paletteColorCount;

	uint16 imageRawSize = stream.size() - 40;
	Common::SeekableSubReadStream subStream(&stream, 40, stream.size());

	_codec = Image::createBitmapCodec(compression, width, height, bitsPerPixel);

	if (!_codec)
		return false;

	_surface = _codec->decodeFrame(subStream);

	return true;
}

BITDDecoder::BITDDecoder() {
	_surface = 0;
	_palette = 0;
	_paletteColorCount = 0;
	_codec = 0;
}

BITDDecoder::~BITDDecoder() {
	destroy();
}

void BITDDecoder::destroy() {
	_surface = 0;

	delete[] _palette;
	_palette = 0;
	_paletteColorCount = 0;

	delete _codec;
	_codec = 0;
}

void BITDDecoder::loadPalette(Common::SeekableReadStream &stream) {
	_palette = new byte[255 * 3];

	_palette[0] = _palette[1] = _palette[2] = 0;
	_palette[255 * 3 + 0] = _palette[255 * 3 + 1] = _palette[255 * 3 + 2] = 0xff;

	_paletteColorCount = 2;
}

bool BITDDecoder::loadStream(Common::SeekableReadStream &stream) {
	uint32 width = 512; // Should come from the Cast
	uint32 height = 342;

	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	int x = 0, y = 0;
	byte *p = (byte *)_surface->getBasePtr(x, y);

	while (true) {
		byte in = stream.readByte();

		if (stream.eos())
			break;

		for (int i = 0; i < 8; i++) {
			*p++ = (in & 0x80) ? 0xff : 0;
			in <<= 1;
			x++;

			if (x >= width) {
				x = 0;
				y++;

				p = (byte *)_surface->getBasePtr(x, y);
				break;
			}
		}

		if (y >= height) {
			break;
		}
	}

	return true;
}

} // End of namespace Director
