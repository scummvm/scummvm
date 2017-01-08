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

#include "common/substream.h"
#include "common/debug.h"
#include "common/textconsole.h"

#include "director/director.h"
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
	delete _surface;
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
	/* uint32 imageSize = */ stream.readUint32LE();
	/* uint32 pixelsPerMeterX = */ stream.readUint32LE();
	/* uint32 pixelsPerMeterY = */ stream.readUint32LE();
	_paletteColorCount = stream.readUint32LE();
	/* uint32 colorsImportant = */ stream.readUint32LE();

	_paletteColorCount = (_paletteColorCount == 0) ? 255: _paletteColorCount;

	Common::SeekableSubReadStream subStream(&stream, 40, stream.size());

	_codec = Image::createBitmapCodec(compression, width, height, bitsPerPixel);

	if (!_codec)
		return false;

	_surface = _codec->decodeFrame(subStream);

	return true;
}

/****************************
 * BITD
 ****************************/

BITDDecoder::BITDDecoder(int w, int h) {
	_surface = new Graphics::Surface();

	// We make the surface pitch a multiple of 16.
	int pitch = w;
	if (w % 16)
		pitch += 16 - (w % 16);

	// HACK: Create a padded surface by adjusting w after create()
	_surface->create(pitch, h, Graphics::PixelFormat::createFormatCLUT8());
	_surface->w = w;

	_palette = new byte[256 * 3];

	_palette[0] = _palette[1] = _palette[2] = 0;
	_palette[255 * 3 + 0] = _palette[255 * 3 + 1] = _palette[255 * 3 + 2] = 0xff;

	_paletteColorCount = 2;
}

BITDDecoder::~BITDDecoder() {
	destroy();
}

void BITDDecoder::destroy() {
	_surface = 0;

	delete[] _palette;
	_palette = 0;
	_paletteColorCount = 0;
}

void BITDDecoder::loadPalette(Common::SeekableReadStream &stream) {
	// no op
}

bool BITDDecoder::loadStream(Common::SeekableReadStream &stream) {
	int x = 0, y = 0;

	// If the stream has exactly the required number of bits for this image,
	// we assume it is uncompressed.
	if (stream.size() * 8 == _surface->pitch * _surface->h) {
		debugC(3, kDebugImages, "Skipping compression");
		for (y = 0; y < _surface->h; y++) {
			for (x = 0; x < _surface->pitch; ) {
				byte color = stream.readByte();
				for (int c = 0; c < 8; c++)
					*((byte *)_surface->getBasePtr(x++, y)) = (color & (1 << (7 - c))) ? 0 : 0xff;
			}
		}

		return true;
	}

	while (y < _surface->h) {
		int n = stream.readSByte();
		int count;
		int b = 0;
		int state = 0;

		if (stream.eos())
			break;

		if ((n >= 0) && (n <= 127)) { // If n is between 0 and 127 inclusive, copy the next n+1 bytes literally.
			count = n + 1;
			state = 1;
		} else if ((n >= -127) && (n <= -1)) { // Else if n is between -127 and -1 inclusive, copy the next byte -n+1 times.
			b = stream.readByte();
			count = -n + 1;
			state = 2;
		} else { // Else if n is -128, noop.
			count = 0;
		}

		for (int i = 0; i < count && y < _surface->h; i++) {
			byte color = 0;
			if (state == 1) {
				color = stream.readByte();
			} else if (state == 2)
				color = b;

			for (int c = 0; c < 8; c++) {
				*((byte *)_surface->getBasePtr(x, y)) = (color & (1 << (7 - c))) ? 0 : 0xff;
				x++;
				if (x == _surface->pitch) {
					y++;
					x = 0;
					break;
				}
			}
		}
	}

	return true;
}

/****************************
* BITD V4+
****************************/

BITDDecoderV4::BITDDecoderV4(int w, int h) {
	_surface = new Graphics::Surface();

	// We make the surface pitch a multiple of 16.
	int pitch = w;
	if (w % 16)
		pitch += 16 - (w % 16);

	// HACK: Create a padded surface by adjusting w after create()
	_surface->create(pitch, h, Graphics::PixelFormat::createFormatCLUT8());
	_surface->w = w;

	_palette = new byte[256 * 3];

	_palette[0] = _palette[1] = _palette[2] = 0;
	_palette[255 * 3 + 0] = _palette[255 * 3 + 1] = _palette[255 * 3 + 2] = 0xff;

	_paletteColorCount = 2;
}

BITDDecoderV4::~BITDDecoderV4() {
	destroy();
}

void BITDDecoderV4::destroy() {
	_surface = 0;

	delete[] _palette;
	_palette = 0;
	_paletteColorCount = 0;
}

void BITDDecoderV4::loadPalette(Common::SeekableReadStream &stream) {
	// no op
}

bool BITDDecoderV4::loadStream(Common::SeekableReadStream &stream) {
	int x = 0, y = 0;

	// If the stream has exactly the required number of bits for this image,
	// we assume it is uncompressed.
	if (stream.size() * 8 == _surface->pitch * _surface->h) {
		debugC(3, kDebugImages, "Skipping compression");
		for (y = 0; y < _surface->h; y++) {
			for (x = 0; x < _surface->pitch; ) {
				byte color = stream.readByte();
				for (int c = 0; c < 8; c++)
					*((byte *)_surface->getBasePtr(x++, y)) = (color & (1 << (7 - c))) ? 0 : 0xff;
			}
		}

		return true;
	}

	Common::Array<int> pixels;
	pixels.reserve(4096);

	while (!stream.eos()) {
		int data = stream.readByte();
		int len = data + 1;
		if ((data & 0x80) != 0) {
			len = ((data ^ 0xFF) & 0xff) + 2;
			data = stream.readByte();
			for (int p = 0; p < len; p++) {
				pixels.push_back(data);
				//*((byte *)_surface->getBasePtr(x, y)) = data;
			}
			//data = stream.readByte();
		} else {
			for (int p = 0; p < len; p++) {
				data = stream.readByte();
				pixels.push_back(data);
			}
		}
		//if (bpp == 32 && pixels.Count % (w * 3) == 0)
		//	source += 2;
	}

	if (pixels.size() > 0) {
		//this needs to be calculated a lot earlier!
		int bpp = pixels.size() / (_surface->w * _surface->h);
		switch (bpp) {
		case 1:
			for (uint pix = 0; pix < pixels.size(); pix++) {
				//this calculation is wrong.. need a demo with colours.
				*((byte *)_surface->getBasePtr(x, y)) = 0xff - pixels[pix];
				x++;
				if (x == _surface->w) {
					y++;
					x = 0;
				}
			}
			break;
		/*
		case 32:
			SetPixel(x, y, Color.FromArgb(
			pixels[((y * dxr.nodes[c].w) * 3) + x],
			pixels[(((y * dxr.nodes[c].w) * 3) + (dxr.nodes[c].w)) + x],
			pixels[(((y * dxr.nodes[c].w) * 3) + (2 * dxr.nodes[c].w)) + x]));
			break;
		case 16:
			SetPixel(x, y, Color.FromArgb(
			(pixels[((y * dxr.nodes[c].w) * 2) + x] & 0x7c) << 1,
			(pixels[((y * dxr.nodes[c].w) * 2) + x] & 0x03) << 6 |
			(pixels[((y * dxr.nodes[c].w) * 2) + (dxr.nodes[c].w) + x] & 0xe0) >> 2,
			(pixels[((y * dxr.nodes[c].w) * 2) + (dxr.nodes[c].w) + x] & 0x1f) << 3));
			break;
		*/
		}
	}

	return true;
}

} // End of namespace Director
