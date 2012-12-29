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

#include "common/stream.h"
#include "common/util.h"

#include "graphics/decoders/ilbm.h"

namespace Graphics {

ILBMDecoder2::ILBMDecoder2() {
	memset(&_header, 0, sizeof(Header));
	_surface = 0;
	_palette = 0;
	_paletteRanges = 0;
	_paletteColorCount = 0;
	_paletteRangeCount = 0;
	_outPitch = 0;
	_numRelevantPlanes = 8;
	_packPixels = false;
}

ILBMDecoder2::~ILBMDecoder2() {
	destroy();
}

void ILBMDecoder2::destroy() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = 0;
	}

	if (_palette) {
		delete[] _palette;
		_palette = 0;
	}

	if (_paletteRanges) {
		delete[] _paletteRanges;
		_paletteRanges = 0;
	}
}

bool ILBMDecoder2::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	const uint32 form = stream.readUint32BE();

	if (form != CHUNK_FORM)
		return false;

	stream.skip(8);

	while (1) {
		const uint32 type = stream.readUint32BE();
		const uint32 size = stream.readUint32BE();

		if (stream.eos())
			break;

		switch (type) {
		case CHUNK_BMHD:
			loadHeader(stream);
			break;
		case CHUNK_CMAP:
			loadPalette(stream, size);
			break;
		case CHUNK_CRNG:
			loadPaletteRange(stream, size);
			break;
		case CHUNK_BODY:
			loadBitmap(stream);
			break;
		default:
			stream.skip(size);
		}
	}

	return true;
}

void ILBMDecoder2::loadHeader(Common::SeekableReadStream &stream) {
	_header.width = stream.readUint16BE();
	_header.height = stream.readUint16BE();
	_header.x = stream.readUint16BE();
	_header.y = stream.readUint16BE();
	_header.numPlanes = stream.readByte();
	_header.masking = stream.readByte();
	_header.compression = stream.readByte();
	_header.flags = stream.readByte();
	_header.transparentColor = stream.readUint16BE();
	_header.xAspect = stream.readByte();
	_header.yAspect = stream.readByte();
	_header.pageWidth = stream.readUint16BE();
	_header.pageHeight = stream.readUint16BE();

	assert(_header.width >= 1);
	assert(_header.height >= 1);
	assert(_header.numPlanes >= 1 && _header.numPlanes <= 8 && _header.numPlanes != 7);
}

void ILBMDecoder2::loadPalette(Common::SeekableReadStream &stream, const uint32 size) {
	_palette = new byte[size];
	stream.read(_palette, size);
	_paletteColorCount = size / 3;
}

void ILBMDecoder2::loadPaletteRange(Common::SeekableReadStream &stream, const uint32 size) {
	if (_paletteRanges)
		delete[] _paletteRanges;

	_paletteRanges = new PaletteRange[_paletteRangeCount + 1];
	PaletteRange &range = _paletteRanges[_paletteRangeCount];

	range.timer = stream.readSint16BE();
	range.step = stream.readSint16BE();
	range.flags = stream.readSint16BE();
	range.first = stream.readByte();
	range.last = stream.readByte();

	++_paletteRangeCount;
}

void ILBMDecoder2::loadBitmap(Common::SeekableReadStream &stream) {
	_numRelevantPlanes = MIN(_numRelevantPlanes, _header.numPlanes);

	if (_numRelevantPlanes != 1 && _numRelevantPlanes != 2 && _numRelevantPlanes != 4)
		_packPixels = false;

	if (_outPitch == 0)
		_outPitch = _header.width;

	if (_packPixels)
		_outPitch /= (8 / _numRelevantPlanes);

	_surface = new Graphics::Surface();
	_surface->create(_outPitch, _header.height, Graphics::PixelFormat::createFormatCLUT8());

	uint32 scanlinePitch = ((_header.width + 15) >> 4) << 1;
	byte *scanlines = new byte[scanlinePitch * _header.numPlanes];
	byte *data = (byte *)_surface->pixels;

	for (uint16 i = 0; i < _header.height; ++i) {
		byte *scanline = scanlines;

		for (uint16 j = 0; j < _header.numPlanes; ++j) {
			uint16 left = scanlinePitch;

			while (left > 0 && !stream.eos() ) {
				uint16 length = scanlinePitch;

				if (_header.compression)
					decompressRLE(stream, scanline, length, left);
				else
					stream.read(scanline, length);

				scanline += length;
				left -= length;
			}
		}

		packPixels(scanlines, data, scanlinePitch);
		data += _outPitch;
	}

	delete[] scanlines;
}

void ILBMDecoder2::decompressRLE(Common::SeekableReadStream &stream, byte *scanline, uint16 &length, const uint16 left) {
	length = 0;
	uint16 code = stream.readByte();

	if (code != 0x80) {
		if (code <= 0x7f) { // literal run
			code++;
			length = MIN(code, left);
			stream.read(scanline, length);

			if(code > length)
				stream.skip(code - length);
		} else { // expand run
			byte value = stream.readByte();
			code = (256 - code) + 1;
			length = MIN(code, left);
			memset(scanline, value, length);
		}
	}
}

void ILBMDecoder2::packPixels(byte *scanlines, byte *data, const uint16 scanlinePitch) {
	uint32 numPixels = _outPitch;

	if (_packPixels)
		numPixels *= (8 / _numRelevantPlanes);

	for (uint32 x = 0; x < numPixels; ++x) {
		byte *scanline = scanlines;
		byte pixel = 0;
		byte offset = x >> 3;
		byte bit = 0x80 >> (x & 7);

		// first build a pixel by scanning all the usable planes in the input
		for (uint32 plane = 0; plane < _numRelevantPlanes; ++plane) {
			if (scanline[offset] & bit)
				pixel |= (1 << plane);

			scanline += scanlinePitch;
		}

		// then output the pixel according to the requested packing
		if (!_packPixels)
			data[x] = pixel;
		else if (_numRelevantPlanes == 1)
			data[x / 8] |= (pixel << (x & 7));
		else if (_numRelevantPlanes == 2)
			data[x / 4] |= (pixel << ((x & 3) << 1));
		else if (_numRelevantPlanes == 4)
			data[x / 2] |= (pixel << ((x & 1) << 2));
	}
}

} // End of namespace Graphics
