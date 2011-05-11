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

#ifndef GRAPHICS_PICT_H
#define GRAPHICS_PICT_H

#include "common/rect.h"
#include "common/scummsys.h"

#include "graphics/pixelformat.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {

class JPEG;
struct Surface;

class PictDecoder {
public:
	PictDecoder(Graphics::PixelFormat pixelFormat);
	~PictDecoder();
	Surface *decodeImage(Common::SeekableReadStream *stream, byte *palette = 0);

	struct PixMap {
		uint32 baseAddr;
		uint16 rowBytes;
		Common::Rect bounds;
		uint16 pmVersion;
		uint16 packType;
		uint32 packSize;
		uint32 hRes;
		uint32 vRes;
		uint16 pixelType;
		uint16 pixelSize;
		uint16 cmpCount;
		uint16 cmpSize;
		uint32 planeBytes;
		uint32 pmTable;
		uint32 pmReserved;
	};

	static PixMap readPixMap(Common::SeekableReadStream *stream, bool hasBaseAddr = true);

private:
	Common::Rect _imageRect;
	PixelFormat _pixelFormat;
	JPEG *_jpeg;
	byte _palette[256 * 3];
	bool _isPaletted;
	Graphics::Surface *_outputSurface;

	void decodeDirectBitsRect(Common::SeekableReadStream *stream, bool hasPalette);
	void decodeDirectBitsLine(byte *out, uint32 length, Common::SeekableReadStream *data, byte bitsPerPixel, byte bytesPerPixel);
	void decodeCompressedQuickTime(Common::SeekableReadStream *stream);
	void outputPixelBuffer(byte *&out, byte value, byte bitsPerPixel);
};

} // End of namespace Graphics

#endif
