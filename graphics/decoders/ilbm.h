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

/**
 * @file
 * Image decoder used in engines:
 */

#ifndef GRAPHICS_DECODERS_ILBM_H
#define GRAPHICS_DECODERS_ILBM_H

#include "common/endian.h"

#include "graphics/surface.h"
#include "graphics/decoders/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {

struct PixelFormat;
struct Surface;

class ILBMDecoder2 : public ImageDecoder {
public:
	ILBMDecoder2();
	virtual ~ILBMDecoder2();

	// ImageDecoder API
	void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Surface *getSurface() const { return _surface; }
	const byte *getPalette() const { return _palette; }
	uint16 getPaletteColorCount() const { return _paletteColorCount; }
	void setOutPitch(const uint16 outPitch) { _outPitch = outPitch; }
	void setPackedPixels(const bool packedPixels) { _packedPixels = packedPixels; }
private:
	struct BMHD {
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

	enum {
		CHUNK_FORM = MKTAG('F','O','R','M'),
		CHUNK_BMHD = MKTAG('B','M','H','D'),
	    CHUNK_CMAP = MKTAG('C','M','A','P'),
		CHUNK_BODY = MKTAG('B','O','D','Y')
	};

	Surface *_surface;
	byte *_palette;
	BMHD _header;
	int16 _paletteColorCount;
	uint16 _outPitch;
	bool _packedPixels;
};

} // End of namespace Graphics

#endif // GRAPHICS_DECODERS_ILBM_H
