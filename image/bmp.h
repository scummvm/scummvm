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

/**
 * @file
 * Image decoder used in engines:
 *  - hugo
 *  - mohawk
 *  - wintermute
 */

#ifndef IMAGE_BMP_H
#define IMAGE_BMP_H

#include "common/scummsys.h"
#include "common/str.h"
#include "image/image_decoder.h"

namespace Common {
class SeekableReadStream;
class WriteStream;
}

namespace Graphics {
struct Surface;
}

namespace Image {

class Codec;

class BitmapDecoder : public ImageDecoder {
public:
	BitmapDecoder();
	virtual ~BitmapDecoder();

	// ImageDecoder API
	void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
	const byte *getPalette() const { return _palette; }
	uint16 getPaletteColorCount() const { return _paletteColorCount; }

private:
	Codec *_codec;
	const Graphics::Surface *_surface;
	byte *_palette;
	uint16 _paletteColorCount;
};

/**
 * Outputs an uncompressed BMP stream of the given input surface.
 */
bool writeBMP(Common::WriteStream &out, const Graphics::Surface &input);

} // End of namespace Image

#endif
