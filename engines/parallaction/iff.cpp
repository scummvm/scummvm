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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "common/iff_container.h"
#include "common/stream.h"
#include "common/util.h"
#include "parallaction/iff.h"

namespace Parallaction {


void IFFParser::setInputStream(Common::SeekableReadStream *stream) {
	destroy();

	assert(stream);
	_stream = stream;
	_startOffset = 0;
	_endOffset = _stream->size();

	_formType = 0;
	_formSize = (uint32)-1;

	if (_stream->size() < 12) {
		// this file is too small to be a valid IFF container
		return;
	}

	if (_stream->readUint32BE() != ID_FORM) {
		// no FORM header was found
		return;
	}

	_formSize = _stream->readUint32BE();
	_formType = _stream->readUint32BE();
}

void IFFParser::destroy() {
	_stream = 0;
	_startOffset = _endOffset = 0;
}

uint32 IFFParser::getFORMSize() const {
	return _formSize;
}

Common::IFF_ID IFFParser::getFORMType() const {
	return _formType;
}

uint32 IFFParser::moveToIFFBlock(Common::IFF_ID chunkName) {
	uint32 size = (uint32)-1;

	_stream->seek(_startOffset + 0x0C);

	while ((uint)_stream->pos() < _endOffset) {
		uint32 chunk = _stream->readUint32BE();
		uint32 size_temp = _stream->readUint32BE();

		if (chunk != chunkName) {
			_stream->seek((size_temp + 1) & (~1), SEEK_CUR);
			assert((uint)_stream->pos() <= _endOffset);
		} else {
			size = size_temp;
			break;
		}
	}

	return size;
}

uint32 IFFParser::getIFFBlockSize(Common::IFF_ID chunkName) {
	uint32 size = moveToIFFBlock(chunkName);
	return size;
}

bool IFFParser::loadIFFBlock(Common::IFF_ID chunkName, void *loadTo, uint32 ptrSize) {
	uint32 chunkSize = moveToIFFBlock(chunkName);

	if (chunkSize == (uint32)-1) {
		return false;
	}

	uint32 loadSize = 0;
	loadSize = MIN(ptrSize, chunkSize);
	_stream->read(loadTo, loadSize);
	return true;
}

Common::SeekableReadStream *IFFParser::getIFFBlockStream(Common::IFF_ID chunkName) {
	uint32 chunkSize = moveToIFFBlock(chunkName);

	if (chunkSize == (uint32)-1) {
		return 0;
	}

	uint32 pos = _stream->pos();
	return new Common::SeekableSubReadStream(_stream, pos, pos + chunkSize, false);
}


// ILBM decoder implementation

ILBMDecoder::ILBMDecoder(Common::SeekableReadStream *in, bool disposeStream) : _in(in), _hasHeader(false), _bodySize((uint32)-1), _paletteSize((uint32)-1) {
	assert(in);
	_parser.setInputStream(in);

	if (_parser.getFORMType() != ID_ILBM) {
		return;
	}

	_hasHeader = _parser.loadIFFBlock(ID_BMHD, &_header, sizeof(_header));
	if (!_hasHeader) {
		return;
	}

	_header.width = TO_BE_16(_header.width);
	_header.height = TO_BE_16(_header.height);

	_paletteSize = _parser.getIFFBlockSize(ID_CMAP);
	_bodySize = _parser.getIFFBlockSize(ID_BODY);
}


ILBMDecoder::~ILBMDecoder() {
	if (_disposeStream) {
		delete _in;
	}
}

uint32 ILBMDecoder::getWidth() {
	assert(_hasHeader);
	return _header.width;
}

uint32 ILBMDecoder::getHeight() {
	assert(_hasHeader);
	return _header.height;
}

uint32 ILBMDecoder::getNumColors() {
	assert(_hasHeader);
	return (1 << _header.depth);
}

byte *ILBMDecoder::getPalette() {
	assert(_paletteSize != (uint32)-1);
	byte *palette = new byte[_paletteSize];
	assert(palette);
	_parser.loadIFFBlock(ID_CMAP, palette, _paletteSize);
	return palette;
}

byte *ILBMDecoder::getBitmap(uint32 numPlanes, bool packPlanes) {
	assert(_bodySize != (uint32)-1);
	assert(numPlanes == 1 || numPlanes == 2 || numPlanes == 3 || numPlanes == 4 || numPlanes == 5 || numPlanes == 8);

	numPlanes = MIN(numPlanes, (uint32)_header.depth);
	if (numPlanes > 4) {
		packPlanes = false;
	}

	uint32 bitmapSize = _header.width * _header.height;
	uint32 bitmapWidth = _header.width;
	if (packPlanes) {
		bitmapSize /= (8 / numPlanes);
		bitmapWidth /= (8 / numPlanes);
	}

	Common::SeekableReadStream *bodyStream = _parser.getIFFBlockStream(ID_BODY);
	assert(bodyStream);

	byte *bitmap = new byte[bitmapSize];
	assert(bitmap);
	memset(bitmap, 0, bitmapSize);

	switch (_header.pack) {
	case 1: {	// PackBits compressed bitmap
		Graphics::PackBitsReadStream stream(*bodyStream);

		byte *out = bitmap;

		// setup a buffer to hold enough data to build a line in the output
		uint32 scanWidth = ((_header.width + 15)/16) << 1;
		byte *scanBuffer = (byte*)malloc(scanWidth * _header.depth);

		for (uint i = 0; i < _header.height; ++i) {
			byte *s = scanBuffer;
			for (uint32 j = 0; j < _header.depth; ++j) {
				stream.read(s, scanWidth);
				s += scanWidth;
			}

			planarToChunky(out, bitmapWidth, scanBuffer, scanWidth, numPlanes, packPlanes);
			out += bitmapWidth;
		}

		free(scanBuffer);
		break;
	}
	default:
		error("only RLE compressed ILBM files are supported");
		break;
	}

	delete bodyStream;

	return bitmap;
}


void ILBMDecoder::planarToChunky(byte *out, uint32 width, byte *in, uint32 planeWidth, uint32 nPlanes, bool packPlanes) {
	byte pix, ofs, bit;
	byte *s;

	uint32 pixels = width;
	if (packPlanes) {
		pixels *= (8 / nPlanes);
	}

	for (uint32 x = 0; x < pixels; ++x) {

		pix = 0;
		ofs = x >> 3;
		bit = 0x80 >> (x & 7);

		// first build a pixel by scanning all the usable planes in the input
		s = in;
		for (uint32 plane = 0; plane < nPlanes; ++plane) {
			if (s[ofs] & bit) {
				pix |= (1 << plane);
			}
			s += planeWidth;
		}


		// then output the pixel according to the requested packing
		if (!packPlanes) {
			out[x] = pix;
		} else
		if (nPlanes == 1) {
			out[x/8] |= (pix << (x & 7));
		} else
		if (nPlanes == 2) {
			out[x/4] |= (pix << ((x & 3) << 1));
		} else
		if (nPlanes == 4) {
			out[x/2] |= (pix << ((x & 1) << 2));
		}
	}

}


} // End of namespace Parallaction
