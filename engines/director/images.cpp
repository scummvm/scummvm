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

#include "common/substream.h"
#include "graphics/macgui/macwindowmanager.h"
#include "image/codecs/bmp_raw.h"

#include "director/director.h"
#include "director/images.h"

namespace Director {

DIBDecoder::DIBDecoder() {
	_surface = nullptr;
	_palette = nullptr;
	_paletteColorCount = 0;
	_codec = nullptr;
}

DIBDecoder::~DIBDecoder() {
	destroy();
}

void DIBDecoder::destroy() {
	_surface = nullptr;	// It is deleted by BitmapRawDecoder

	delete[] _palette;
	_palette = nullptr;
	_paletteColorCount = 0;

	delete _codec;
	_codec = nullptr;
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

BITDDecoder::BITDDecoder(int w, int h, uint16 bitsPerPixel, uint16 pitch, const byte *palette, uint16 version) {
	_surface = new Graphics::Surface();
	_pitch = pitch;
	_version = version;

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
	_surface = nullptr;

	_paletteColorCount = 0;
}

void BITDDecoder::loadPalette(Common::SeekableReadStream &stream) {
	// no op
}

void BITDDecoder::convertPixelIntoSurface(void* surfacePointer, uint fromBpp, uint toBpp, int red, int green, int blue) {
	switch (toBpp) {
	case 1:
		*((byte*)surfacePointer) = g_director->_wm->findBestColor(red, green, blue);
		break;

	case 4:
		*((uint32 *)surfacePointer) = g_director->_wm->findBestColor(red, green, blue);
		break;

	default:
		warning("BITDDecoder::convertPixelIntoSurface(): conversion from %d to %d not implemented", fromBpp, toBpp);
		break;
	}
}

bool BITDDecoder::loadStream(Common::SeekableReadStream &stream) {
	int x = 0, y = 0;

	Common::Array<uint> pixels;
	// If the stream has exactly the required number of bits for this image,
	// we assume it is uncompressed.
	// logic above does not fit the situation when _bitsPerPixel == 1, need to fix.
	int bytesNeed = _surface->w * _surface->h * _bitsPerPixel / 8;
	bool skipCompression = false;
	if (_bitsPerPixel != 1) {
		if (_version < kFileVer300) {
			skipCompression = stream.size() >= bytesNeed;
		} else if (_version < kFileVer400) {
			// for D3, looks like it will round up the _surface->w to align 2
			// not sure whether D2 will have the same logic.
			// check lzone-mac data/r-c/tank.a-1 and lzone-mac data/r-a/station-b.01.
			if (_surface->w & 1)
				bytesNeed += _surface->h * _bitsPerPixel / 8;
			skipCompression = stream.size() == bytesNeed;
		}
	}

	if ((stream.size() == _pitch * _surface->h * _bitsPerPixel / 8) || skipCompression) {
		debugC(6, kDebugImages, "Skipping compression");
		for (int i = 0; i < stream.size(); i++) {
			pixels.push_back((int)stream.readByte());
		}
	} else {
		while (!stream.eos()) {
			// TODO: D3 32-bit bitmap casts seem to just be ARGB pixels in a row and not RLE.
			// Determine how to distinguish these different types. Maybe stage version.
			// for D4, 32-bit bitmap is RLE, and the encoding format is every line contains the a? r g b at the same line of the original image.
			// i.e. for every line, we shall combine 4 parts to create the original image.
			if (_bitsPerPixel == 32 && _version < kFileVer400) {
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
	if (_bitsPerPixel == 8 && _surface->w < (int)(pixels.size() / _surface->h))
		offset = (pixels.size() / _surface->h) - _surface->w;
	// looks like the data want to round up to 2, so we either got offset 1 or 0.
	// but we may met situation when the pixel size is exactly equals to w * h, thus we add a check here.
	if (offset)
		offset = _surface->w % 2;

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
					if (_version < kFileVer400) {
						convertPixelIntoSurface(_surface->getBasePtr(x, y),
							(_bitsPerPixel / 8),
							_surface->format.bytesPerPixel,
							(pixels[((y * _surface->w) * 2) + x * 2] & 0x7c) << 1,
							(pixels[((y * _surface->w) * 2) + x * 2] & 0x03) << 6 |
							(pixels[((y * _surface->w) * 2) + x * 2 + 1] & 0xe0) >> 2,
							(pixels[((y * _surface->w) * 2) + x * 2 + 1] & 0x1f) << 3);
					} else {
						convertPixelIntoSurface(_surface->getBasePtr(x, y),
							(_bitsPerPixel / 8),
							_surface->format.bytesPerPixel,
							(pixels[((y * _surface->w) * 2) + x] & 0x7c) << 1,
							(pixels[((y * _surface->w) * 2) + x] & 0x03) << 6 |
							(pixels[((y * _surface->w) * 2) + (_surface->w) + x] & 0xe0) >> 2,
							(pixels[((y * _surface->w) * 2) + (_surface->w) + x] & 0x1f) << 3);
					}
					x++;
					break;

				case 32:
					// if we have the issue in D3 32bpp images, then the way to fix it should be the same as 16bpp images.
					// check the code above, there is different behaviour between in D4 and D3. Currently we are only using D4.
					if (_version < kFileVer400) {
						convertPixelIntoSurface(_surface->getBasePtr(x, y),
							(_bitsPerPixel / 8),
							_surface->format.bytesPerPixel,
							pixels[(((y * _surface->w * 4)) + (x * 4 + 1))],
							pixels[(((y * _surface->w * 4)) + (x * 4 + 2))],
							pixels[(((y * _surface->w * 4)) + (x * 4 + 3))]);
					} else {
						convertPixelIntoSurface(_surface->getBasePtr(x, y),
							(_bitsPerPixel / 8),
							_surface->format.bytesPerPixel,
							pixels[(((y * _surface->w * 4)) + (x + _surface->w))],
							pixels[(((y * _surface->w * 4)) + (x + 2 * _surface->w))],
							pixels[(((y * _surface->w * 4)) + (x + 3 * _surface->w))]);
					}
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
