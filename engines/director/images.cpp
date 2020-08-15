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
#include "graphics/macgui/macwindowmanager.h"
#include "image/codecs/bmp_raw.h"

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
	_surface = 0;	// It is deleted by BitmapRawDecoder

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

	_codec = Image::createBitmapCodec(compression, 0, width, height, bitsPerPixel);

	if (!_codec)
		return false;

	_surface = _codec->decodeFrame(subStream);

	return true;
}

/****************************
* BITD
****************************/

BITDDecoder::BITDDecoder(int w, int h, uint16 bitsPerPixel, uint16 pitch, const byte *palette) {
	_surface = new Graphics::Surface();
	_pitch = pitch;

	if (_pitch < w) {
		warning("BITDDecoder: pitch is too small: %d < %d", _pitch, w);

		_pitch = w;
	}

	// HACK: Create a padded surface by adjusting w after create()
	_surface->create(_pitch, h, g_director->_pixelformat);
	_surface->w = w;

	_palette = palette;

	// TODO: Bring this in from the main surface?
	_paletteColorCount = 255;

	_bitsPerPixel = bitsPerPixel;
}

BITDDecoder::~BITDDecoder() {
	destroy();
}

void BITDDecoder::destroy() {
	_surface->free();
	delete _surface;
	_surface = 0;

	_paletteColorCount = 0;
}

void BITDDecoder::loadPalette(Common::SeekableReadStream &stream) {
	// no op
}

void BITDDecoder::convertPixelIntoSurface(void* surfacePointer, uint fromBpp, uint toBpp, int red, int green, int blue) {
	// Initial implementation of 32-bit images to palettised sprites.
	switch (fromBpp) {
	case 4:
		switch (toBpp) {
		case 1:
			if (red == 255 && blue == 255 && green == 255) {
				*((byte*)surfacePointer) = 255;
			} else if (red == 0 && blue == 0 && green == 0) {
				*((byte*)surfacePointer) = 0;
			} else {
				for (byte p = 0; p < _paletteColorCount; p++) {
					if (_palette[p * 3 + 0] == red &&
						_palette[p * 3 + 1] == green &&
						_palette[p * 3 + 2] == blue) {
						*((byte*)surfacePointer) = p;
					}
				}
			}
			break;

		default:
			warning("BITDDecoder::convertPixelIntoSurface(): conversion from %d to %d not implemented",
				fromBpp, toBpp);
		}
		break;

	default:
		warning("BITDDecoder::convertPixelIntoSurface(): could not convert from %d to %d",
			fromBpp, toBpp);
		break;
	}
}

bool BITDDecoder::loadStream(Common::SeekableReadStream &stream) {
	int x = 0, y = 0;

	Common::Array<int> pixels;
	// If the stream has exactly the required number of bits for this image,
	// we assume it is uncompressed.
	if (stream.size() == _pitch * _surface->h * _bitsPerPixel / 8) {
		debugC(6, kDebugImages, "Skipping compression");
		for (int i = 0; i < stream.size(); i++) {
			pixels.push_back((int)stream.readByte());
		}
	} else {
		while (!stream.eos()) {
			// TODO: D3 32-bit bitmap casts seem to just be ARGB pixels in a row and not RLE.
			// Determine how to distinguish these different types. Maybe stage version.
			if (_bitsPerPixel == 32) {
				int data = stream.readByte();
				pixels.push_back(data);
			} else {
				int data = stream.readByte();
				int len = data + 1;
				if ((data & 0x80) != 0) {
					len = ((data ^ 0xFF) & 0xff) + 2;
					data = stream.readByte();
					for (int p = 0; p < len; p++) {
						pixels.push_back(data);
					}
				} else {
					for (int p = 0; p < len; p++) {
						data = stream.readByte();
						pixels.push_back(data);
					}
				}
				if (_bitsPerPixel == 32 && pixels.size() % (_surface->w * 3) == 0)
					stream.readUint16BE();
			}
		}
	}

	if (pixels.size() < (uint32)_surface->w * _surface->h * (_bitsPerPixel / 8)) {
		int tail = (_surface->w * _surface->h * _bitsPerPixel / 8) - pixels.size();

		warning("BITDDecoder::loadStream(): premature end of stream (%d of %d pixels)",
			pixels.size(), pixels.size() + tail);

		for (int i = 0; i < tail; i++)
			pixels.push_back(0);
	}

	int offset = 0;
	if (_surface->w < (pixels.size() / _surface->h))
		offset = (pixels.size() / _surface->h) - _surface->w;

	uint32 color;
	bool paletted = (g_director->_pixelformat.bytesPerPixel == 1);

	if (pixels.size() > 0) {
		for (y = 0; y < _surface->h; y++) {
			for (x = 0; x < _surface->w;) {
				switch (_bitsPerPixel) {
				case 1:
					for (int c = 0; c < 8 && x < _surface->w; c++, x++) {
						color = (pixels[(((y * _pitch) + x) / 8)] & (1 << (7 - c))) ? 0 : 0xff;
						if (paletted) {
							*((byte *)_surface->getBasePtr(x, y)) = color;
						} else {
							*((uint32 *)_surface->getBasePtr(x, y)) = color ? g_director->_wm->_colorWhite : g_director->_wm->_colorBlack;
						}
					}
					break;

				case 8:
					// this calculation is wrong.. need a demo with colours.
					if (paletted) {
						*((byte *)_surface->getBasePtr(x, y)) = g_director->transformColor(pixels[(y * _surface->w) + x + (y * offset)]);
					} else {
						*((uint32 *)_surface->getBasePtr(x, y)) = g_director->transformColor(pixels[(y * _surface->w) + x + (y * offset)]);
					}
					x++;
					break;

				case 16:
					*((uint16*)_surface->getBasePtr(x, y)) = _surface->format.RGBToColor(
						(pixels[((y * _surface->w) * 2) + x] & 0x7c) << 1,
						(pixels[((y * _surface->w) * 2) + x] & 0x03) << 6 |
						(pixels[((y * _surface->w) * 2) + (_surface->w) + x] & 0xe0) >> 2,
						(pixels[((y * _surface->w) * 2) + (_surface->w) + x] & 0x1f) << 3);
					x++;
					break;

				case 32:
					convertPixelIntoSurface(_surface->getBasePtr(x, y),
						(_bitsPerPixel / 8),
						_surface->format.bytesPerPixel,
						pixels[(((y * (_surface->w * 4))) + ((x * 4) + 1))],
						pixels[(((y * (_surface->w * 4))) + ((x * 4) + 2))],
						pixels[(((y * (_surface->w * 4))) + ((x * 4) + 3))]);
					x++;
					break;

				default:
					x++;
					break;
				}
			}
		}
	}

	return true;
}

} // End of namespace Director
