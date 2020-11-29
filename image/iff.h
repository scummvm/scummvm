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

#ifndef IMAGE_IFF_H
#define IMAGE_IFF_H

#include "common/array.h"
#include "common/endian.h"
#include "graphics/surface.h"

#include "image/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Image {

/**
 * @defgroup image_iff IFF decoder
 * @ingroup image
 *
 * @brief Decoder for images encoded as Interchange File Format (IFF).
 *
 * Used in engines:
 * - Gob
 * - Parallaction
 * - Queen
 * - Saga
 * @{
 */

class IFFDecoder : public ImageDecoder {
public:
	struct Header {
		uint16 width, height;
		uint16 x, y;
		byte numPlanes;
		byte masking;
		byte compression;
		byte flags;
		uint16 transparentColor;
		byte xAspect, yAspect;
		uint16 pageWidth, pageHeight;
	};

	struct PaletteRange {
		int16  timer, step, flags;
		byte first, last;
	};

	enum Type {
		TYPE_UNKNOWN = 0,
		TYPE_ILBM,
		TYPE_PBM
	};

	IFFDecoder();
	virtual ~IFFDecoder();

	// ImageDecoder API
	void destroy();
	bool loadStream(Common::SeekableReadStream &stream);
	const Header *getHeader() const { return &_header; }
	const Graphics::Surface *getSurface() const { return _surface; }
	const byte *getPalette() const { return _palette; }
	const Common::Array<PaletteRange> &getPaletteRanges() const { return _paletteRanges; }
	uint16 getPaletteColorCount() const { return _paletteColorCount; }

	/**
	* The number of planes to decode, also determines the pixel packing if _packPixels is true.
	* 8 == decode all planes, map 1 pixel in 1 byte. (default, no packing even if _packPixels is true)
	*
	* NOTE: this property must be reset manually, and is not reset by a call to destroy().
	*/
	void setNumRelevantPlanes(const uint8 numRelevantPlanes) { _numRelevantPlanes = numRelevantPlanes; }

	/**
	* Enables pixel packing, the amount of packing is determined by _numRelevantPlanes
	* 1 == decode first plane, pack 8 pixels in 1 byte. This makes _surface->w 1/8th of _header.width
	* 2 == decode first 2 planes, pack 4 pixels in 1 byte. This makes _surface->w 1/4th of _header.width
	* 4 == decode first 4 planes, pack 2 pixels in 1 byte. This makes _surface->w half of _header.width
	* Packed bitmaps won't have a proper surface format since there is no way to tell it to use 1, 2 or 4 bits per pixel
	*
	* NOTE: this property must be reset manually, and is not reset by a call to destroy().
	*/
	void setPixelPacking(const bool pixelPacking) { _pixelPacking = pixelPacking; }
private:

	Header _header;
	Graphics::Surface *_surface;
	byte *_palette;
	Common::Array<PaletteRange> _paletteRanges;
	Type _type;
	uint16 _paletteColorCount;
	uint8 _numRelevantPlanes;
	bool _pixelPacking;

	void loadHeader(Common::SeekableReadStream &stream);
	void loadPalette(Common::SeekableReadStream &stream, const uint32 size);
	void loadPaletteRange(Common::SeekableReadStream &stream, const uint32 size);
	void loadBitmap(Common::SeekableReadStream &stream);
	void packPixels(byte *scanlines, byte *data, const uint16 scanlinePitch, const uint16 outPitch);
};
/** @} */	
} // End of namespace Image

#endif
