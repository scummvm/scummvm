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

#ifndef MOHAWK_BITMAP_H
#define MOHAWK_BITMAP_H

#include "mohawk/graphics.h"

#include "common/scummsys.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace Mohawk {

class ImageData;

enum BitmapFormat {
	kBitsPerPixel1 = 0x0000,
	kBitsPerPixel4 = 0x0001,
	kBitsPerPixel8 = 0x0002,
	kBitsPerPixel16 = 0x0003,
	kBitsPerPixel24 = 0x0004,
	kBitsPerPixelMask = 0x0007,
	kBitmapHasCLUT = 0x0008,
	kDrawMASK = 0x00f0,
	kDrawRaw = 0x0000,
	kDrawRLE8 = 0x0010,
	kDrawMSRLE8 = 0x0020,
	kDrawRLE = 0x0030,
	kPackMASK = 0x0f00,
	kPackNone = 0x0000,
	kPackLZ = 0x0100,
	kPackLZ1 = 0x0200,
	kPackRiven = 0x0400,
	kPackXDec = 0x0f00,
	kFlagMASK = 0xf000,
	kFlag16_80X86 = 0x1000, // 16 bit pixel data has been converted to 80X86 format
	kFlag24_MAC = 0x1000 // 24 bit pixel data has been converted to MAC 32 bit format
};

struct BitmapHeader {
	uint16 width;
	uint16 height;
	int16 bytesPerRow;
	uint16 format;

	struct ColorTable {
		uint16 tableSize;
		byte rgbBits;
		byte colorCount;
		byte* palette;   // In 8bpp only
	} colorTable;
};

class MohawkBitmap {
public:
	MohawkBitmap();
	virtual ~MohawkBitmap();

	virtual ImageData *decodeImage(Common::SeekableReadStream *stream);

	// Unpack Functions
	void unpackRaw();
	void unpackLZ();
	void unpackLZ1();
	void unpackRiven();

	// Draw Functions
	void drawRaw();
	void drawRLE8();
	void drawRLE();
	
protected:
	BitmapHeader _header;
	byte getBitsPerPixel();

	// The actual LZ decoder
	static Common::SeekableReadStream *decompressLZ(Common::SeekableReadStream *stream, uint32 uncompressedSize);

private:
	Common::SeekableReadStream *_data;
	Graphics::Surface *_surface;

	const char *getPackName();
	void unpackImage();
	const char *getDrawName();
	void drawImage();

	// Riven Decoding
	void handleRivenSubcommandStream(byte count, byte *&dst);
};

// Myst uses a different image format than that of other Mohawk games.
// It essentially uses a Windows bitmap with the LZ encoding from the
// Mohawk Bitmap format.
class MystBitmap : public MohawkBitmap {
public:
	MystBitmap() : MohawkBitmap() {}
	~MystBitmap() {}

	ImageData *decodeImage(Common::SeekableReadStream *stream);

private:	
	struct BitmapHeader {
		uint16 type;
		uint32 size;
		uint16 res1;
		uint16 res2;
		uint32 imageOffset;
	} _header;

	struct InfoHeader {
		uint32 size;
		uint32 width;
		uint32 height;
		uint16 planes;
		uint16 bitsPerPixel;
		uint32 compression;
		uint32 imageSize;
		uint32 pixelsPerMeterX;
		uint32 pixelsPerMeterY;
		uint32 colorsUsed;
		uint32 colorsImportant;
	} _info;
};

class OldMohawkBitmap : public MohawkBitmap {
public:
	OldMohawkBitmap() : MohawkBitmap() {}
	~OldMohawkBitmap() {}

	ImageData *decodeImage(Common::SeekableReadStream *stream);
};

} // End of namespace Mohawk

#endif
