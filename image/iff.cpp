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

#include "image/iff.h"

#include "common/iff_container.h"
#include "common/stream.h"
#include "common/util.h"

namespace Image {

IFFDecoder::IFFDecoder() {
	_surface = 0;
	_palette = 0;

	// these 2 properties are not reset by destroy(), so the default is set here.
	_numRelevantPlanes = 8;
	_pixelPacking = false;

	destroy();
}

IFFDecoder::~IFFDecoder() {
	destroy();
}

void IFFDecoder::destroy() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = 0;
	}

	if (_palette) {
		delete[] _palette;
		_palette = 0;
	}

	memset(&_header, 0, sizeof(Header));
	_paletteRanges.clear();
	_type = TYPE_UNKNOWN;
	_paletteColorCount = 0;
}

bool IFFDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	const uint32 form = stream.readUint32BE();

	if (form != ID_FORM) {
		warning("Failed reading IFF-file");
		return false;
	}

	stream.skip(4);

	const uint32 type = stream.readUint32BE();

	switch (type) {
		case ID_ILBM:
			_type = TYPE_ILBM;
			break;
		case ID_PBM:
			_type = TYPE_PBM;
			break;
		case TYPE_UNKNOWN:
		default:
			_type = TYPE_UNKNOWN;
			break;
	}

	if (type == TYPE_UNKNOWN) {
		warning("Failed reading IFF-file");
		return false;
	}

	while (1) {
		if (stream.size() < stream.pos() + 8)
			break;

		const uint32 chunkType = stream.readUint32BE();
		uint32 chunkSize = stream.readUint32BE();
		// According to the format specs:
		// "If ckData is an odd number of bytes long, a 0 pad byte follows which is not included in ckSize."
		// => fix the length
		if (chunkSize % 2)
			chunkSize++;

		if (stream.eos())
			break;

		switch (chunkType) {
		case ID_BMHD:
			loadHeader(stream);
			break;
		case ID_CMAP:
			loadPalette(stream, chunkSize);
			break;
		case ID_CRNG:
			loadPaletteRange(stream, chunkSize);
			break;
		case ID_BODY:
			loadBitmap(stream);
			break;
		default:
			if (stream.size() < stream.pos() + (int32)chunkSize)
				break;

			stream.skip(chunkSize);
		}
	}

	return true;
}

void IFFDecoder::loadHeader(Common::SeekableReadStream &stream) {
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

void IFFDecoder::loadPalette(Common::SeekableReadStream &stream, const uint32 size) {
	_palette = new byte[size];
	stream.read(_palette, size);
	_paletteColorCount = size / 3;
}

void IFFDecoder::loadPaletteRange(Common::SeekableReadStream &stream, const uint32 size) {
	PaletteRange range;

	range.timer = stream.readSint16BE();
	range.step = stream.readSint16BE();
	range.flags = stream.readSint16BE();
	range.first = stream.readByte();
	range.last = stream.readByte();

	_paletteRanges.push_back(range);
}

void IFFDecoder::loadBitmap(Common::SeekableReadStream &stream) {
	_numRelevantPlanes = MIN(_numRelevantPlanes, _header.numPlanes);

	if (_numRelevantPlanes != 1 && _numRelevantPlanes != 2 && _numRelevantPlanes != 4)
		_pixelPacking = false;

	uint16 outPitch = _header.width;

	if (_pixelPacking)
		outPitch /= (8 / _numRelevantPlanes);

	// FIXME: CLUT8 is not a proper format for packed bitmaps but there is no way to tell it to use 1, 2 or 4 bits per pixel
	_surface = new Graphics::Surface();
	_surface->create(outPitch, _header.height, Graphics::PixelFormat::createFormatCLUT8());

	if (_type == TYPE_ILBM) {
		uint32 scanlinePitch = ((_header.width + 15) >> 4) << 1;
		byte *scanlines = new byte[scanlinePitch * _header.numPlanes];
		byte *data = (byte *)_surface->getPixels();

		for (uint16 i = 0; i < _header.height; ++i) {
			byte *scanline = scanlines;

			for (uint16 j = 0; j < _header.numPlanes; ++j) {
				uint16 outSize = scanlinePitch;

				if (_header.compression) {
					Common::PackBitsReadStream packStream(stream);
					packStream.read(scanline, outSize);
				} else {
					stream.read(scanline, outSize);
				}

				scanline += outSize;
			}

			packPixels(scanlines, data, scanlinePitch, outPitch);
			data += outPitch;
		}

		delete[] scanlines;
	} else if (_type == TYPE_PBM) {
		byte *data = (byte *)_surface->getPixels();
		uint32 outSize = _header.width * _header.height;

		if (_header.compression) {
			Common::PackBitsReadStream packStream(stream);
			packStream.read(data, outSize);
		} else {
			stream.read(data, outSize);
		}
	}
}

void IFFDecoder::packPixels(byte *scanlines, byte *data, const uint16 scanlinePitch, const uint16 outPitch) {
	uint32 numPixels = _header.width;

	if (_pixelPacking)
		numPixels = outPitch * (8 / _numRelevantPlanes);

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
		if (!_pixelPacking)
			data[x] = pixel;
		else if (_numRelevantPlanes == 1)
			data[x / 8] |= (pixel << (x & 7));
		else if (_numRelevantPlanes == 2)
			data[x / 4] |= (pixel << ((x & 3) << 1));
		else if (_numRelevantPlanes == 4)
			data[x / 2] |= (pixel << ((x & 1) << 2));
	}
}

} // End of namespace Image
