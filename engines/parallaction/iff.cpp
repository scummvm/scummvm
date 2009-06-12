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


void IFFParser::setInputStream(Common::ReadStream *stream) {
	assert(stream);
	_formChunk.setInputStream(stream);
	_chunk.setInputStream(stream);

	_formChunk.readHeader();
	if (_formChunk.id != ID_FORM) {
		error("IFFParser input is not a FORM type IFF file");
	}
	_formSize = _formChunk.size;
	_formType = _formChunk.readUint32BE();
}

uint32 IFFParser::getFORMSize() const {
	return _formSize;
}

Common::IFF_ID IFFParser::getFORMType() const {
	return _formType;
}

void IFFParser::parse(IFFCallback &callback) {
	bool stop;
	do {
		_chunk.feed();
		_formChunk.incBytesRead(_chunk.size);

		if (_formChunk.hasReadAll()) {
			break;
		}

		_formChunk.incBytesRead(8);
		_chunk.readHeader();

		// invoke the callback
		Common::SubReadStream stream(&_chunk, _chunk.size);
		IFFChunk chunk(_chunk.id, _chunk.size, &stream);
		stop = callback(chunk);

		// eats up all the remaining data in the chunk
		while (!stream.eos()) {
			stream.readByte();
		}


	} while (!stop);
}



void ILBMDecoder::loadHeader(Common::ReadStream *stream) {
	assert(stream);
	stream->read(&_header, sizeof(_header));
	_header.width = FROM_BE_16(_header.width);
	_header.height = FROM_BE_16(_header.height);
	_header.x = FROM_BE_16(_header.x);
	_header.y = FROM_BE_16(_header.y);
	_header.transparentColor = FROM_BE_16(_header.transparentColor);
	_header.pageWidth = FROM_BE_16(_header.pageWidth);
	_header.pageHeight = FROM_BE_16(_header.pageHeight);
}

void ILBMDecoder::loadBitmap(uint32 mode, byte *buffer, Common::ReadStream *stream) {
	assert(stream);
	uint32 numPlanes = MIN(mode & ILBM_UNPACK_PLANES, (uint32)_header.depth);
	assert(numPlanes == 1 || numPlanes == 2 || numPlanes == 3 || numPlanes == 4 || numPlanes == 5 || numPlanes == 8);

	bool packPixels = (mode & ILBM_PACK_PLANES) != 0;
	if (numPlanes != 1 && numPlanes != 2 && numPlanes != 4) {
		packPixels = false;
	}

	uint32 outPitch = _header.width;
	if (packPixels) {
		outPitch /= (8 / numPlanes);
	}
	byte *out = buffer;

	switch (_header.pack) {
	case 1: {	// PackBits compressed bitmap
		Graphics::PackBitsReadStream packStream(*stream);

		// setup a buffer to hold enough data to build a line in the output
		uint32 scanlineWidth = ((_header.width + 15)/16) << 1;
		byte *scanline = new byte[scanlineWidth * _header.depth];

		for (uint i = 0; i < _header.height; ++i) {
			byte *s = scanline;
			for (uint32 j = 0; j < _header.depth; ++j) {
				packStream.read(s, scanlineWidth);
				s += scanlineWidth;
			}

			planarToChunky(out, outPitch, scanline, scanlineWidth, numPlanes, packPixels);
			out += outPitch;
		}

		delete []scanline;
		break;
	}

	default:
		// implement other compression types here!
		error("only RLE compressed ILBM files are supported");
		break;
	}
}

void ILBMDecoder::planarToChunky(byte *out, uint32 outPitch, byte *in, uint32 inWidth, uint32 nPlanes, bool packPlanes) {
	byte pix, ofs, bit;
	byte *s;

	uint32 pixels = outPitch;
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
			s += inWidth;
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
