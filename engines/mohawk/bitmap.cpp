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

#include "mohawk/bitmap.h"

#include "common/debug.h"
#include "common/util.h"
#include "common/endian.h"
#include "common/system.h"

namespace Mohawk {

#define PACK_COMPRESSION (_header.format & kPackMASK)
#define DRAW_COMPRESSION (_header.format & kDrawMASK)
	
MohawkBitmap::MohawkBitmap() {
}
	
MohawkBitmap::~MohawkBitmap() {
}

ImageData *MohawkBitmap::decodeImage(Common::SeekableReadStream *stream) {
	_data = stream;
	_header.colorTable.palette = NULL;
	
	// NOTE: Only the bottom 12 bits of width/height/bytesPerRow are
	// considered valid and bytesPerRow has to be an even number.
	_header.width = _data->readUint16BE() & 0x3FFF;
	_header.height = _data->readUint16BE() & 0x3FFF;
	_header.bytesPerRow = _data->readSint16BE() & 0x3FFE;
	_header.format = _data->readUint16BE();

	debug (2, "Decoding Mohawk Bitmap (%dx%d, %dbpp, %s Packing + %s Drawing)", _header.width, _header.height, getBitsPerPixel(), getPackName(), getDrawName());

	if (getBitsPerPixel() != 8)
		error ("Unhandled bpp %d", getBitsPerPixel());
	
	// Read in the palette if it's here.
	if (_header.format & kBitmapHasCLUT || (PACK_COMPRESSION == kPackRiven && getBitsPerPixel() == 8)) {
		_header.colorTable.tableSize = _data->readUint16BE();
		_header.colorTable.rgbBits = _data->readByte();
		_header.colorTable.colorCount = _data->readByte();
		_header.colorTable.palette = (byte *)malloc(256 * 4);

		for (uint16 i = 0; i < 256; i++) {
			_header.colorTable.palette[i * 4 + 2] = _data->readByte();
			_header.colorTable.palette[i * 4 + 1] = _data->readByte();
			_header.colorTable.palette[i * 4] = _data->readByte();
			_header.colorTable.palette[i * 4 + 3] = 0;
		}
	}

	_surface = new Graphics::Surface();
	_surface->create(_header.width, _header.height, getBitsPerPixel() >> 3);
	
	unpackImage();
	drawImage();
	delete _data;

	return new ImageData(_surface, _header.colorTable.palette);
}

byte MohawkBitmap::getBitsPerPixel() {
	switch (_header.format & kBitsPerPixelMask) {
	case kBitsPerPixel1:
		return 1;
	case kBitsPerPixel4:
		return 4;
	case kBitsPerPixel8:
		return 8;
	case kBitsPerPixel16:
		return 16;
	case kBitsPerPixel24:
		return 24;
	default:
		error ("Unknown bits per pixel");
	}
	
	return 0;
}

struct CompressionInfo {
	uint16 flag;
	const char *name;
	void (MohawkBitmap::*func)();
};

static const CompressionInfo packTable[] = {
	{ kPackNone, "Raw", &MohawkBitmap::unpackRaw },
	{ kPackLZ, "LZ", &MohawkBitmap::unpackLZ },
	{ kPackLZ1, "LZ1", &MohawkBitmap::unpackLZ1 },
	{ kPackRiven, "Riven", &MohawkBitmap::unpackRiven }
};
	
const char *MohawkBitmap::getPackName() {
	for (uint32 i = 0; i < ARRAYSIZE(packTable); i++)
		if (PACK_COMPRESSION == packTable[i].flag)
			return packTable[i].name;
			
	return "Unknown";
}

void MohawkBitmap::unpackImage() {
	for (uint32 i = 0; i < ARRAYSIZE(packTable); i++)
		if (PACK_COMPRESSION == packTable[i].flag) {
			(this->*packTable[i].func)();
			return;
		}
		
	warning("Unknown Pack Compression");
}

static const CompressionInfo drawTable[] = {
	{ kDrawRaw, "Raw", &MohawkBitmap::drawRaw },
	{ kDrawRLE8, "RLE8", &MohawkBitmap::drawRLE8 },
	{ kDrawRLE, "RLE", &MohawkBitmap::drawRLE }
};

const char *MohawkBitmap::getDrawName() {
	for (uint32 i = 0; i < ARRAYSIZE(drawTable); i++)
		if (DRAW_COMPRESSION == drawTable[i].flag)
			return drawTable[i].name;
			
	return "Unknown";
}

void MohawkBitmap::drawImage() {
	for (uint32 i = 0; i < ARRAYSIZE(drawTable); i++)
		if (DRAW_COMPRESSION == drawTable[i].flag) {
			(this->*drawTable[i].func)();
			return;
		}
		
	warning("Unknown Draw Compression");
}

//////////////////////////////////////////
// Raw "Unpacker"
//////////////////////////////////////////

void MohawkBitmap::unpackRaw() {
	// Do nothing :D
}

//////////////////////////////////////////
// LZ Unpacker
//////////////////////////////////////////

#define LEN_BITS		6
#define MIN_STRING		3									// lower limit for string length
#define POS_BITS		(16 - LEN_BITS)
#define MAX_STRING		((1 << LEN_BITS) + MIN_STRING - 1)	// upper limit for string length
#define CBUFFERSIZE		(1 << POS_BITS)						// size of the circular buffer
#define POS_MASK		(CBUFFERSIZE - 1)

Common::SeekableReadStream *MohawkBitmap::decompressLZ(Common::SeekableReadStream *stream, uint32 uncompressedSize) {
	uint16 flags = 0;
	uint32 bytesOut = 0;
	uint16 insertPos = 0;

	// Expand the output buffer to at least the ring buffer size
	uint32 outBufSize = MAX<int>(uncompressedSize, CBUFFERSIZE);

	byte *outputData = (byte *)malloc(outBufSize);
	byte *dst = outputData;
	byte *buf = dst;

	// Clear the buffer to all 0's
	memset(outputData, 0, outBufSize);

	while (stream->pos() < stream->size()) {
		flags >>= 1;

		if (!(flags & 0x100))
			flags = stream->readByte() | 0xff00;

		if (flags & 1) {
			if (++bytesOut > uncompressedSize)
				break;
			*dst++ = stream->readByte();
			if (++insertPos > POS_MASK) {
				insertPos = 0;
				buf += CBUFFERSIZE;
			}
		} else {
			uint16 offLen = stream->readUint16BE();
			uint16 stringLen = (offLen >> POS_BITS) + MIN_STRING;
			uint16 stringPos = (offLen + MAX_STRING) & POS_MASK;

			bytesOut += stringLen;
			if (bytesOut > uncompressedSize)
				stringLen -= bytesOut - uncompressedSize;

			byte *strPtr = buf + stringPos;
			if (stringPos > insertPos) {
				if (bytesOut >= CBUFFERSIZE)
					strPtr -= CBUFFERSIZE;
				else if (stringPos + stringLen > POS_MASK) {
					for (uint16 k = 0; k < stringLen; k++) {
						*dst++ = *strPtr++;
						if (++stringPos > POS_MASK) {
							stringPos = 0;
							strPtr = outputData;
						}
					}
					insertPos = (insertPos + stringLen) & POS_MASK;
					if (bytesOut >= uncompressedSize)
						break;
					continue;
				}
			}

			insertPos += stringLen;

			if (insertPos > POS_MASK) {
				insertPos &= POS_MASK;
				buf += CBUFFERSIZE;
			}

			for (uint16 k = 0; k < stringLen; k++)
				*dst++ = *strPtr++;

			if (bytesOut >= uncompressedSize)
				break;
		}
	}

	return new Common::MemoryReadStream(outputData, uncompressedSize, Common::DisposeAfterUse::YES);
}

void MohawkBitmap::unpackLZ() {
	uint32 uncompressedSize = _data->readUint32BE();
	/* uint32 compressedSize = */ _data->readUint32BE();
	uint16 dictSize = _data->readUint16BE();
	
	// We only support the buffer size of 0x400
	if (dictSize != CBUFFERSIZE)
		error("Unsupported dictionary size of %04x", dictSize);
	
	// Now go and decompress the data
	Common::SeekableReadStream *decompressedData = decompressLZ(_data, uncompressedSize);
	delete _data;
	_data = decompressedData;
}

//////////////////////////////////////////
// LZ Unpacker
//////////////////////////////////////////

void MohawkBitmap::unpackLZ1() {
	error("STUB: unpackLZ1()");
}

//////////////////////////////////////////
// Riven Unpacker
//////////////////////////////////////////

void MohawkBitmap::unpackRiven() {
	_data->readUint32BE(); // Unknown, the number is close to bytesPerRow * height. Could be bufSize. 
	
	byte *uncompressedData = (byte *)malloc(_header.bytesPerRow * _header.height);
	byte *dst = uncompressedData;

	while (!_data->eos() && dst < (uncompressedData + _header.bytesPerRow * _header.height)) {
		byte cmd = _data->readByte();
		debug (8, "Riven Pack Command %02x", cmd);

		if (cmd == 0x00) {                       // End of stream
			break;
		} else if (cmd >= 0x01 && cmd <= 0x3f) { // Simple Pixel Duplet Output
			for (byte i = 0; i < cmd; i++) {
				*dst++ = _data->readByte();
				*dst++ = _data->readByte();
			}
		} else if (cmd >= 0x40 && cmd <= 0x7f) { // Simple Repetition of last 2 pixels (cmd - 0x40) times
			byte pixel[] = { *(dst - 2), *(dst - 1) };

			for (byte i = 0; i < (cmd - 0x40); i++) {
				*dst++ = pixel[0];
				*dst++ = pixel[1];
			}
		} else if (cmd >= 0x80 && cmd <= 0xbf) { // Simple Repetition of last 4 pixels (cmd - 0x80) times
			byte pixel[] = { *(dst - 4), *(dst - 3), *(dst - 2), *(dst - 1) };

			for (byte i = 0; i < (cmd - 0x80); i++) {
				*dst++ = pixel[0];
				*dst++ = pixel[1];
				*dst++ = pixel[2];
				*dst++ = pixel[3];
			}
		} else {                                 // Subcommand Stream of (cmd - 0xc0) subcommands
			handleRivenSubcommandStream(cmd - 0xc0, dst);
		}
	}

	delete _data;
	_data = new Common::MemoryReadStream(uncompressedData, _header.bytesPerRow * _header.height, Common::DisposeAfterUse::YES);
}

static byte getLastTwoBits(byte c) {
	return (c & 0x03);
}

static byte getLastThreeBits(byte c) {
	return (c & 0x07);
}

static byte getLastFourBits(byte c) {
	return (c & 0x0f);
}

#define B_BYTE()				\
	*dst = _data->readByte();	\
	dst++

#define B_LASTDUPLET()			\
	*dst = *(dst - 2);			\
	dst++

#define B_LASTDUPLET_PLUS_M()	\
	*dst = *(dst - 2) + m;		\
	dst++

#define B_LASTDUPLET_MINUS_M()	\
	*dst = *(dst - 2) - m;		\
	dst++

#define B_LASTDUPLET_PLUS(m)	\
	*dst = *(dst - 2) + (m);	\
	dst++

#define B_LASTDUPLET_MINUS(m)	\
	*dst = *(dst - 2) - (m);	\
	dst++

#define B_PIXEL_MINUS(m)		\
	*dst = *(dst - (m));		\
	dst++

#define B_NDUPLETS(n)													\
	uint16 m1 = ((getLastTwoBits(cmd) << 8) + _data->readByte());		\
		for (uint16 j = 0; j < (n); j++) {								\
			*dst = *(dst - m1);											\
			dst++;														\
		}																\
		void dummyFuncToAllowTrailingSemicolon()



void MohawkBitmap::handleRivenSubcommandStream(byte count, byte *&dst) {
	for (byte i = 0; i < count; i++) {
		byte cmd = _data->readByte();
		uint16 m = getLastFourBits(cmd);
		debug (9, "Riven Pack Subcommand %02x", cmd);

		// Notes: p = value of the next byte, m = last four bits of the command

		// Arithmetic operations
		if (cmd >= 0x01 && cmd <= 0x0f) {
			// Repeat duplet at relative position of -m duplets
			B_PIXEL_MINUS(m * 2);
			B_PIXEL_MINUS(m * 2);
		} else if (cmd == 0x10) {
			// Repeat last duplet, but set the value of the second pixel to p
			B_LASTDUPLET();
			B_BYTE();
		} else if (cmd >= 0x11 && cmd <= 0x1f) {
			// Repeat last duplet, but set the value of the second pixel to the value of the -m pixel
			B_LASTDUPLET();
			B_PIXEL_MINUS(m);
		} else if (cmd >= 0x20 && cmd <= 0x2f) {
			// Repeat last duplet, but add x to second pixel
			B_LASTDUPLET();
			B_LASTDUPLET_PLUS_M();
		} else if (cmd >= 0x30 && cmd <= 0x3f) {
			// Repeat last duplet, but subtract x from second pixel
			B_LASTDUPLET();
			B_LASTDUPLET_MINUS_M();
		} else if (cmd == 0x40) {
			// Repeat last duplet, but set the value of the first pixel to p
			B_BYTE();
			B_LASTDUPLET();
		} else if (cmd >= 0x41 && cmd <= 0x4f) {
			// Output pixel at relative position -m, then second pixel of last duplet
			B_PIXEL_MINUS(m);
			B_LASTDUPLET();
		} else if (cmd == 0x50) {
			// Output two absolute pixel values, p1 and p2
			B_BYTE();
			B_BYTE();
		} else if (cmd >= 0x51 && cmd <= 0x57) {
			// Output pixel at relative position -m, then absolute pixel value p
			// m is the last 3 bits of cmd here, not last 4
			B_PIXEL_MINUS(getLastThreeBits(cmd));
			B_BYTE();
		} else if (cmd >= 0x59 && cmd <= 0x5f) {
			// Output absolute pixel value p, then pixel at relative position -m
			// m is the last 3 bits of cmd here, not last 4
			B_BYTE();
			B_PIXEL_MINUS(getLastThreeBits(cmd));
		} else if (cmd >= 0x60 && cmd <= 0x6f) {
			// Output absolute pixel value p, then (second pixel of last duplet) + x
			B_BYTE();
			B_LASTDUPLET_PLUS_M();
		} else if (cmd >= 0x70 && cmd <= 0x7f) {
			// Output absolute pixel value p, then (second pixel of last duplet) - x
			B_BYTE();
			B_LASTDUPLET_MINUS_M();
		} else if (cmd >= 0x80 && cmd <= 0x8f) {
			// Repeat last duplet adding x to the first pixel
			B_LASTDUPLET_PLUS_M();
			B_LASTDUPLET();
		} else if (cmd >= 0x90 && cmd <= 0x9f) {
			// Output (first pixel of last duplet) + x, then absolute pixel value p
			B_LASTDUPLET_PLUS_M();
			B_BYTE();
		} else if (cmd == 0xa0) {
			// Repeat last duplet, adding first 4 bits of the next byte
			// to first pixel and last 4 bits to second
			byte pattern = _data->readByte();
			B_LASTDUPLET_PLUS(pattern >> 4);
			B_LASTDUPLET_PLUS(getLastFourBits(pattern));
		} else if (cmd == 0xb0) {
			// Repeat last duplet, adding first 4 bits of the next byte
			// to first pixel and subtracting last 4 bits from second
			byte pattern = _data->readByte();
			B_LASTDUPLET_PLUS(pattern >> 4);
			B_LASTDUPLET_MINUS(getLastFourBits(pattern));
		} else if (cmd >= 0xc0 && cmd <= 0xcf) {
			// Repeat last duplet subtracting x from first pixel
			B_LASTDUPLET_MINUS_M();
			B_LASTDUPLET();
		} else if (cmd >= 0xd0 && cmd <= 0xdf) {
			// Output (first pixel of last duplet) - x, then absolute pixel value p
			B_LASTDUPLET_MINUS_M();
			B_BYTE();
		} else if (cmd == 0xe0) {
			// Repeat last duplet, subtracting first 4 bits of the next byte
			// to first pixel and adding last 4 bits to second
			byte pattern = _data->readByte();
			B_LASTDUPLET_MINUS(pattern >> 4);
			B_LASTDUPLET_PLUS(getLastFourBits(pattern));
		} else if (cmd == 0xf0 || cmd == 0xff) {
			// Repeat last duplet, subtracting first 4 bits from the next byte
			// to first pixel and last 4 bits from second
			byte pattern = _data->readByte();
			B_LASTDUPLET_MINUS(pattern >> 4);
			B_LASTDUPLET_MINUS(getLastFourBits(pattern));
		
		// Repeat operations
		// Repeat n duplets from relative position -m (given in pixels, not duplets). 
		// If r is 0, another byte follows and the last pixel is set to that value
		} else if (cmd >= 0xa4 && cmd <= 0xa7) {
			B_NDUPLETS(3);
			B_BYTE();
		} else if (cmd >= 0xa8 && cmd <= 0xab) {
			B_NDUPLETS(4);
		} else if (cmd >= 0xac && cmd <= 0xaf) {
			B_NDUPLETS(5);
			B_BYTE();
		} else if (cmd >= 0xb4 && cmd <= 0xb7) {
			B_NDUPLETS(6);
		} else if (cmd >= 0xb8 && cmd <= 0xbb) {
			B_NDUPLETS(7);
			B_BYTE();
		} else if (cmd >= 0xbc && cmd <= 0xbf) {
			B_NDUPLETS(8);
		} else if (cmd >= 0xe4 && cmd <= 0xe7) {
			B_NDUPLETS(9);
			B_BYTE();
		} else if (cmd >= 0xe8 && cmd <= 0xeb) {
			B_NDUPLETS(10); // 5 duplets
		} else if (cmd >= 0xec && cmd <= 0xef) {
			B_NDUPLETS(11);
			B_BYTE();
		} else if (cmd >= 0xf4 && cmd <= 0xf7) {
			B_NDUPLETS(12);
		} else if (cmd >= 0xf8 && cmd <= 0xfb) {
			B_NDUPLETS(13);
			B_BYTE();
		} else if (cmd == 0xfc) {
			byte b1 = _data->readByte();
			byte b2 = _data->readByte();
			uint16 m1 = ((getLastTwoBits(b1) << 8) + b2);

			for (uint16 j = 0; j < ((b1 >> 3) + 1); j++) { // one less iteration
				B_PIXEL_MINUS(m1);
				B_PIXEL_MINUS(m1);
			}

			// last iteration
			B_PIXEL_MINUS(m1);

			if ((b1 & (1 << 2)) == 0) {
				B_BYTE();
			} else {
				B_PIXEL_MINUS(m1);
			}
		} else
			warning("Unknown Riven Pack Subcommand 0x%02x", cmd);
	}
}

//////////////////////////////////////////
// Raw Drawer
//////////////////////////////////////////

void MohawkBitmap::drawRaw() {
	for (uint16 y = 0; y < _header.height; y++) {
		_data->read((byte *)_surface->pixels + y * _header.width, _header.width);
		_data->skip(_header.bytesPerRow - _header.width);
	}
}

//////////////////////////////////////////
// RLE8 Drawer
//////////////////////////////////////////

void MohawkBitmap::drawRLE8() {
	// A very simple RLE8 scheme is used as a secondary compression on
	// most images in non-Riven tBMP's.
	
	for (uint16 i = 0; i < _header.height; i++) {
		uint16 rowByteCount = _data->readUint16BE();
		int32 startPos = _data->pos();
		byte *dst = (byte *)_surface->pixels + i * _header.width;
		int16 remaining = _header.width;
		
		// HACK: It seems only the bottom 9 bits are valid for images
		// TODO: Verify if this is still needed after the buffer clearing fix.
		rowByteCount &= 0x1ff;
		
		while (remaining > 0) {
			byte code = _data->readByte();
			uint16 runLen = (code & 0x7F) + 1;
			
			if (runLen > remaining)
				runLen = remaining;
				
			if (code & 0x80) {
				byte val = _data->readByte();
				for (uint16 j = 0; j < runLen; j++)
					*dst++ = val;
			} else {
				for (uint16 j = 0; j < runLen; j++)
					*dst++ = _data->readByte();
			}
			
			remaining -= runLen;
		}
		
		_data->seek(startPos + rowByteCount);
	}
}

//////////////////////////////////////////
// RLE Drawer
//////////////////////////////////////////

void MohawkBitmap::drawRLE() {
	warning("STUB: drawRLE()");
}

//////////////////////////////////////////
// Myst Bitmap Decoder
//////////////////////////////////////////

ImageData* MystBitmap::decodeImage(Common::SeekableReadStream* stream) {
	uint32 uncompressedSize = stream->readUint32LE();
	Common::SeekableReadStream* bmpStream = decompressLZ(stream, uncompressedSize);
	delete stream;
	
	_header.type = bmpStream->readUint16BE();

	if (_header.type != 'BM')
		error("BMP header not detected");

	_header.size = bmpStream->readUint32LE();
	assert (_header.size > 0);
	_header.res1 = bmpStream->readUint16LE();
	_header.res2 = bmpStream->readUint16LE();
	_header.imageOffset = bmpStream->readUint32LE();
	
	_info.size = bmpStream->readUint32LE();

	if (_info.size != 40)
		error("Only Windows v3 BMP's are supported");

	_info.width = bmpStream->readUint32LE();
	_info.height = bmpStream->readUint32LE();
	_info.planes = bmpStream->readUint16LE();
	_info.bitsPerPixel = bmpStream->readUint16LE();
	_info.compression = bmpStream->readUint32LE();
	_info.imageSize = bmpStream->readUint32LE();
	_info.pixelsPerMeterX = bmpStream->readUint32LE();
	_info.pixelsPerMeterY = bmpStream->readUint32LE();
	_info.colorsUsed = bmpStream->readUint32LE();
	_info.colorsImportant = bmpStream->readUint32LE();
	
	if (_info.compression != 0)
		error("Unhandled BMP compression %d", _info.compression);
	
	if (_info.colorsUsed == 0)
		_info.colorsUsed = 256;
		
	// TODO: Myst ME's Help.dat contains WDIB's with 24bpp color.
	if (_info.bitsPerPixel != 8 && _info.bitsPerPixel != 24)
		error("%dbpp Bitmaps not supported", _info.bitsPerPixel);
		
	byte *palData = NULL;
	
	if (_info.bitsPerPixel == 8) {
		palData = (byte *)malloc(256 * 4);
		for (uint16 i = 0; i < _info.colorsUsed; i++) {			
			palData[i * 4 + 2] = bmpStream->readByte();
			palData[i * 4 + 1] = bmpStream->readByte();
			palData[i * 4] = bmpStream->readByte();			
			palData[i * 4 + 3] = bmpStream->readByte();
		}
	}
	
	bmpStream->seek(_header.imageOffset);
	
	Graphics::Surface *surface = new Graphics::Surface();
	int srcPitch = _info.width * (_info.bitsPerPixel >> 3);
	const int extraDataLength = (srcPitch % 4) ? 4 - (srcPitch % 4) : 0;
	
	if (_info.bitsPerPixel == 8) {
		surface->create(_info.width, _info.height, 1);
		byte *dst = (byte *)surface->pixels;

		for (uint32 i = 0; i < _info.height; i++) {
			bmpStream->read(dst + (_info.height - i - 1) * _info.width, _info.width);
			bmpStream->skip(extraDataLength);
		}
	} else {
		Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();
		surface->create(_info.width, _info.height, pixelFormat.bytesPerPixel);
		
		byte *dst = (byte *)surface->pixels + (surface->h - 1) * surface->pitch;
		
		for (uint32 i = 0; i < _info.height; i++) {
			for (uint32 j = 0; j < _info.width; j++) {
				byte b = bmpStream->readByte();
				byte g = bmpStream->readByte();
				byte r = bmpStream->readByte();
				
				if (pixelFormat.bytesPerPixel == 2)
					*((uint16 *)dst) = pixelFormat.RGBToColor(r, g, b);
				else 
					*((uint32 *)dst) = pixelFormat.RGBToColor(r, g, b);
				
				dst += pixelFormat.bytesPerPixel;
			}
			
			bmpStream->skip(extraDataLength);
			dst -= surface->pitch * 2;
		}
	}
		
	delete bmpStream;
	
	return new ImageData(surface, palData);
}

ImageData *OldMohawkBitmap::decodeImage(Common::SeekableReadStream *stream) {
	Common::SeekableSubReadStreamEndian *endianStream = (Common::SeekableSubReadStreamEndian *)stream;

	// The format part is just a guess at this point. Note that the width and height roles have been reversed!

	_header.height = endianStream->readUint16() & 0x3FF;
	_header.width = endianStream->readUint16() & 0x3FF;
	_header.bytesPerRow = endianStream->readUint16() & 0x3FE;
	_header.format = endianStream->readUint16();

	debug(2, "Decoding Old Mohawk Bitmap (%dx%d, %04x Format)", _header.width, _header.height, _header.format);
	
	warning("Unhandled old Mohawk Bitmap decoding");

	delete stream;
	return new ImageData(NULL, NULL);
}

} // End of namespace Mohawk
