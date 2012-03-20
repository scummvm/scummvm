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
 */

#include "graphics/decoders/png.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "common/endian.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/types.h"
#include "common/util.h"
#include "common/zlib.h"

// PNG decoder, based on the W3C specs:
// http://www.w3.org/TR/PNG/
// Parts of the code have been adapted from LodePNG, by Lode Vandevenne:
// http://members.gamedev.net/lode/projects/LodePNG/

/*
LodePNG version 20101211

Copyright (c) 2005-2010 Lode Vandevenne

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

namespace Graphics {

enum PNGChunks {
	// == Critical chunks =====================================================
	kChunkIHDR = MKTAG('I','H','D','R'),	// Image header
	kChunkIDAT = MKTAG('I','D','A','T'),	// Image data
	kChunkPLTE = MKTAG('P','L','T','E'),	// Palette
	kChunkIEND = MKTAG('I','E','N','D'),	// Image trailer
	// == Ancillary chunks ====================================================
	kChunktRNS = MKTAG('t','R','N','S')	// Transparency
	// All of the other ancillary chunks are ignored. They're added here for
	// reference only.
	// cHRM - Primary chromacities and white point
	// gAMA - Image gamma
	// iCCP - Embedded ICC profile
	// sBIT - Significant bits
	// sRGB - Standard RGB color space
	// tEXT - Textual data
	// sTXt - Compressed textual data
	// iTXt - International textual data
	// bKGD - Background color
	// hIST - Image histogram
	// pHYs - Physical pixel dimensions
	// sPLT - Suggested palette
	// tIME - Image last-modification time
};

// Refer to http://www.w3.org/TR/PNG/#9Filters
enum PNGFilters {
	kFilterNone    = 0,
	kFilterSub     = 1,
	kFilterUp      = 2,
	kFilterAverage = 3,
	kFilterPaeth   = 4
};

PNGDecoder::PNGDecoder() : _compressedBuffer(0), _compressedBufferSize(0),
			_transparentColorSpecified(false), _outputSurface(0) {
}

PNGDecoder::~PNGDecoder() {
	destroy();
}

void PNGDecoder::destroy() {
	if (_outputSurface) {
		_outputSurface->free();
		delete _outputSurface;
		_outputSurface = 0;
	}
}

bool PNGDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	uint32 chunkLength = 0, chunkType = 0;
	_stream = &stream;

	// First, check the PNG signature
	if (_stream->readUint32BE() != MKTAG(0x89, 'P', 'N', 'G')) {
		delete _stream;
		return false;
	}
	if (_stream->readUint32BE() != MKTAG(0x0d, 0x0a, 0x1a, 0x0a)) {
		delete _stream;
		return false;
	}

	// Start reading chunks till we reach an IEND chunk
	while (chunkType != kChunkIEND) {
		// The chunk length does not include the type or CRC bytes
		chunkLength = _stream->readUint32BE();
		chunkType = _stream->readUint32BE();

		switch (chunkType) {
		case kChunkIHDR:
			readHeaderChunk();
			break;
		case kChunkIDAT:
			if (_compressedBufferSize == 0) {
				_compressedBufferSize += chunkLength;
				_compressedBuffer = (byte *)malloc(_compressedBufferSize);
				_stream->read(_compressedBuffer, chunkLength);
			} else {
				// Expand the buffer
				uint32 prevSize = _compressedBufferSize;
				_compressedBufferSize += chunkLength;
				byte *tmp = new byte[prevSize];
				memcpy(tmp, _compressedBuffer, prevSize);
				free(_compressedBuffer);
				_compressedBuffer = (byte *)malloc(_compressedBufferSize);
				memcpy(_compressedBuffer, tmp, prevSize);
				delete[] tmp;
				_stream->read(_compressedBuffer + prevSize, chunkLength);
			}
			break;
		case kChunkPLTE:	// only available in indexed PNGs
			if (_header.colorType != kIndexed)
				error("A palette chunk has been found in a non-indexed PNG file");
			if (chunkLength % 3 != 0)
				error("Palette chunk not divisible by 3");

			_paletteEntries = chunkLength / 3;
			_stream->read(_palette, _paletteEntries * 3);
			memset(_paletteTransparency, 0xff, sizeof(_paletteTransparency));
			break;
		case kChunkIEND:
			// End of stream
			break;
		case kChunktRNS:
			readTransparencyChunk(chunkLength);
			break;
		default:
			// Skip the chunk content
			_stream->skip(chunkLength);
			break;
		}

		if (chunkType != kChunkIEND)
			_stream->skip(4);	// skip the chunk CRC checksum
	}

	// We no longer need the file stream, thus close it here
	_stream = 0;

	// Unpack the compressed buffer
	Common::MemoryReadStream *compData = new Common::MemoryReadStream(_compressedBuffer, _compressedBufferSize, DisposeAfterUse::YES);
	_imageData = Common::wrapCompressedReadStream(compData);

	// Construct the final image
	constructImage();

	// Close the uncompressed stream, which will also delete the memory stream,
	// and thus the original compressed buffer
	delete _imageData;

	return true;
}

/**
 * Paeth predictor, used by PNG filter type 4
 * The parameters are of signed 16-bit integers, but should come
 * from unsigned chars. The integers  are only needed to make
 * the paeth calculation correct.
 *
 * Taken from lodePNG, with a slight patch:
 * http://www.atalasoft.com/cs/blogs/stevehawley/archive/2010/02/23/libpng-you-re-doing-it-wrong.aspx
 */
byte PNGDecoder::paethPredictor(int16 a, int16 b, int16 c) {
  int16 pa = ABS<int16>(b - c);
  int16 pb = ABS<int16>(a - c);
  int16 pc = ABS<int16>(a + b - c - c);

  if (pa <= MIN<int16>(pb, pc))
	  return (byte)a;
  else if (pb <= pc)
	  return (byte)b;
  else
	  return (byte)c;
}

/**
 * Unfilters a filtered PNG scan line.
 * PNG filters are defined in: http://www.w3.org/TR/PNG/#9Filters
 * Note that filters are always applied to bytes
 *
 * Taken from lodePNG
 */
void PNGDecoder::unfilterScanLine(byte *dest, const byte *scanLine, const byte *prevLine, uint16 byteWidth, byte filterType, uint16 length) {
	uint16 i;

	switch (filterType) {
	case kFilterNone:		// no change
		for (i = 0; i < length; i++)
			dest[i] = scanLine[i];
		break;
	case kFilterSub:		// add the bytes to the left
		for (i = 0; i < byteWidth; i++)
			dest[i] = scanLine[i];
		for (i = byteWidth; i < length; i++)
			dest[i] = scanLine[i] + dest[i - byteWidth];
		break;
	case kFilterUp:			// add the bytes of the above scanline
		if (prevLine) {
			for (i = 0; i < length; i++)
				dest[i] = scanLine[i] + prevLine[i];
		} else {
			for (i = 0; i < length; i++)
				dest[i] = scanLine[i];
		}
		break;
	case kFilterAverage:	// average value of the left and top left
		if (prevLine) {
			for (i = 0; i < byteWidth; i++)
				dest[i] = scanLine[i] + prevLine[i] / 2;
			for (i = byteWidth; i < length; i++)
				dest[i] = scanLine[i] + ((dest[i - byteWidth] + prevLine[i]) / 2);
		} else {
			for (i = 0; i < byteWidth; i++)
				dest[i] = scanLine[i];
			for (i = byteWidth; i < length; i++)
				dest[i] = scanLine[i] + dest[i - byteWidth] / 2;
		}
		break;
	case kFilterPaeth:		// Paeth filter: http://www.w3.org/TR/PNG/#9Filter-type-4-Paeth
		if (prevLine) {
			for(i = 0; i < byteWidth; i++)
				dest[i] = (scanLine[i] + prevLine[i]); // paethPredictor(0, prevLine[i], 0) is always prevLine[i]
			for(i = byteWidth; i < length; i++)
				dest[i] = (scanLine[i] + paethPredictor(dest[i - byteWidth], prevLine[i], prevLine[i - byteWidth]));
		} else {
			for(i = 0; i < byteWidth; i++)
				dest[i] = scanLine[i];
			for(i = byteWidth; i < length; i++)
				dest[i] = (scanLine[i] + dest[i - byteWidth]); // paethPredictor(dest[i - byteWidth], 0, 0) is always dest[i - byteWidth]
		}
		break;
	default:
		error("Unknown line filter");
	}

}

int PNGDecoder::getBytesPerPixel() const {
	return (getNumColorChannels() * _header.bitDepth + 7) / 8;
}

void PNGDecoder::constructImage() {
	assert (_header.bitDepth != 0);

	int bytesPerPixel = getBytesPerPixel();
	int pitch = bytesPerPixel * _header.width;
	byte *unfilteredSurface = new byte[pitch * _header.height];
	byte *dest = unfilteredSurface;
	uint16 scanLineWidth = (_header.width * getNumColorChannels() * _header.bitDepth + 7) / 8;
	byte *scanLine = new byte[scanLineWidth];
	byte *prevLine = 0;

	switch(_header.interlaceType) {
	case kNonInterlaced:
		for (uint16 y = 0; y < _header.height; y++) {
			byte filterType = _imageData->readByte();
			_imageData->read(scanLine, scanLineWidth);
			unfilterScanLine(dest, scanLine, prevLine, bytesPerPixel, filterType, scanLineWidth);
			prevLine = dest;
			dest += pitch;
		}
		break;
	case kInterlaced:
		// Theoretically, this shouldn't be needed, as interlacing is only
		// useful for web images. Interlaced PNG images require more complex
		// handling, so unless having support for such images is needed, there
		// is no reason to add support for them.
		error("TODO: Support for interlaced PNG images");
		break;
	}

	delete[] scanLine;

	constructOutput(unfilteredSurface);
	delete[] unfilteredSurface;
}

Graphics::PixelFormat PNGDecoder::findPixelFormat() const {
	// Try to find the best pixel format based on what we have here
	// Which is basically 8bpp for paletted non-transparent
	// and 32bpp for everything else

	switch (_header.colorType) {
	case kIndexed:
		if (!_transparentColorSpecified)
			return Graphics::PixelFormat::createFormatCLUT8();
		// fall through
	case kGrayScale:
	case kTrueColor:
	case kGrayScaleWithAlpha:
	case kTrueColorWithAlpha:
		// We'll go with standard RGBA 32-bit
		return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	}

	error("Unknown PNG color type");
	return Graphics::PixelFormat();
}

void PNGDecoder::constructOutput(const byte *surface) {
	_outputSurface = new Graphics::Surface();
	_outputSurface->create(_header.width, _header.height, findPixelFormat());

	const byte *src = surface;
	byte a = 0xFF;
	int bytesPerPixel = getBytesPerPixel();

	if (_header.colorType != kIndexed) {
		if (_header.colorType == kTrueColor || 
			_header.colorType == kTrueColorWithAlpha) {
			if (bytesPerPixel != 3 && bytesPerPixel != 4)
				error("Unsupported truecolor PNG format");
		} else if (_header.colorType == kGrayScale ||
				   _header.colorType == kGrayScaleWithAlpha) {
			if (bytesPerPixel != 1 && bytesPerPixel != 2)
				error("Unsupported grayscale PNG format");
		}

		for (uint16 i = 0; i < _outputSurface->h; i++) {
			for (uint16 j = 0; j < _outputSurface->w; j++) {
				uint32 result = 0;

				switch (bytesPerPixel) {
				case 1:	// Grayscale
					if (_transparentColorSpecified)
						a = (src[0] == _transparentColor[0]) ? 0 : 0xFF;
					result = _outputSurface->format.ARGBToColor(a, src[0], src[0], src[0]);
					break;
				case 2: // Grayscale + alpha
					result = _outputSurface->format.ARGBToColor(src[1], src[0], src[0], src[0]);
					break;
				case 3: // RGB
					if (_transparentColorSpecified) {
						bool isTransparentColor = (src[0] == _transparentColor[0] &&
												   src[1] == _transparentColor[1] &&
												   src[2] == _transparentColor[2]);
						a = isTransparentColor ? 0 : 0xFF;
					}

					result = _outputSurface->format.ARGBToColor(a, src[0], src[1], src[2]);
					break;
				case 4: // RGBA
					result = _outputSurface->format.ARGBToColor(src[3], src[0], src[1], src[2]);
					break;
				}

				*((uint32 *)_outputSurface->getBasePtr(j, i)) = result;
				src += bytesPerPixel;
			}
		}
	} else {
		uint32 mask = (0xff >> (8 - _header.bitDepth)) << (8 - _header.bitDepth);

		// Convert the indexed surface to the target pixel format
		for (uint16 i = 0; i < _outputSurface->h; i++) {
			int data = 0;
			int bitCount = 8;
			const byte *src1 = src;

			for (uint16 j = 0; j < _outputSurface->w; j++) {
				if (bitCount == 8) {
					data = *src;
					src++;
				}

				byte index = (data & mask) >> (8 - _header.bitDepth);
				data = (data << _header.bitDepth) & 0xff;
				bitCount -= _header.bitDepth;

				if (bitCount == 0)
					bitCount = 8;

				if (_transparentColorSpecified) {
					byte r = _palette[index * 3 + 0];
					byte g = _palette[index * 3 + 1];
					byte b = _palette[index * 3 + 2];
					a = _paletteTransparency[index];
					*((uint32 *)_outputSurface->getBasePtr(j, i)) = _outputSurface->format.ARGBToColor(a, r, g, b);
				} else {
					*((byte *)_outputSurface->getBasePtr(j, i)) = index;
				}
			}

			src = src1 + _outputSurface->w;
		}
	}
}

void PNGDecoder::readHeaderChunk() {
	_header.width = _stream->readUint32BE();
	_header.height = _stream->readUint32BE();
	_header.bitDepth = _stream->readByte();
	if (_header.bitDepth > 8)
		error("Only PNGs with a bit depth of 1-8 bits are supported (i.e. PNG24)");
	_header.colorType = (PNGColorType)_stream->readByte();
	_header.compressionMethod = _stream->readByte();
	// Compression methods: http://www.w3.org/TR/PNG/#10Compression
	// Only compression method 0 (deflate) is documented and supported
	if (_header.compressionMethod != 0)
		error("Unknown PNG compression method: %d", _header.compressionMethod);
	_header.filterMethod = _stream->readByte();
	// Filter methods: http://www.w3.org/TR/PNG/#9Filters
	// Only filter method 0 is documented and supported
	if (_header.filterMethod != 0)
		error("Unknown PNG filter method: %d", _header.filterMethod);
	_header.interlaceType = (PNGInterlaceType)_stream->readByte();
}

byte PNGDecoder::getNumColorChannels() const {
	switch (_header.colorType) {
	case kGrayScale:
		return 1; // Gray
	case kTrueColor:
		return 3; // RGB
	case kIndexed:
		return 1; // Indexed
	case kGrayScaleWithAlpha:
		return 2; // Gray + Alpha
	case kTrueColorWithAlpha:
		return 4; // RGBA
	default:
		error("Unknown color type");
	}
}

void PNGDecoder::readTransparencyChunk(uint32 chunkLength) {
	_transparentColorSpecified = true;

	switch(_header.colorType) {
	case kGrayScale:
		_transparentColor[0] = _stream->readUint16BE();
		_transparentColor[1] = _transparentColor[0];
		_transparentColor[2] = _transparentColor[0];
		break;
	case kTrueColor:
		_transparentColor[0] = _stream->readUint16BE();
		_transparentColor[1] = _stream->readUint16BE();
		_transparentColor[2] = _stream->readUint16BE();
		break;
	case kIndexed:
		_stream->read(_paletteTransparency, chunkLength);

		// A transparency chunk may have less entries
		// than the palette entries. The remaining ones
		// are unmodified (set to 255). Check here:
		// http://www.w3.org/TR/PNG/#11tRNS
		break;
	default:
		error("Transparency chunk found in a PNG that has a separate transparency channel");
	}
}

} // End of Graphics namespace
