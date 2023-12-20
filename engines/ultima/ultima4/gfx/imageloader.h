/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA4_GFX_IMAGELOADER_H
#define ULTIMA4_GFX_IMAGELOADER_H

#include "graphics/pixelformat.h"
#include "image/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Ultima {
namespace Ultima4 {

/**
 * A common base for all Ultima 4 image loaders.
 */
class U4ImageDecoder : public ::Image::ImageDecoder {
public:
	U4ImageDecoder(int width, int height, int bpp);
	virtual ~U4ImageDecoder();

	// ImageDecoder API
	void destroy() override;
	const Graphics::Surface *getSurface() const override { return _surface; }
	const byte *getPalette() const override { return _palette; }
	uint16 getPaletteColorCount() const override { return _paletteColorCount; }

protected:
	Graphics::Surface *_surface;
	const byte *_palette;
	uint16 _paletteColorCount;
	int _width, _height, _bpp;

	/**
	 * Fill in the image pixel data from an uncompressed string of bytes.
	 */
	void setFromRawData(const byte *rawData);

	/**
	 * Get the expected pixel format based on the value of _bpp.
	 */
	Graphics::PixelFormat getPixelFormatForBpp() const;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
