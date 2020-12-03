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

#ifndef IMAGE_PNG_H
#define IMAGE_PNG_H

#include "common/scummsys.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "image/image_decoder.h"

namespace Common {
class SeekableReadStream;
class WriteStream;
}

namespace Graphics {
struct Surface;
}

namespace Image {

/**
 * @defgroup image_png PNG decoder
 * @ingroup image
 *
 * @brief Decoder for PNG images.
 *
 * This decoder has a dependency on the libpng library.
 *
 * Used in engines:
 * - Sword25
 * - Wintermute
 * @{
 */

class PNGDecoder : public ImageDecoder {
public:
	PNGDecoder();
	~PNGDecoder();

	bool loadStream(Common::SeekableReadStream &stream);
	void destroy();
	const Graphics::Surface *getSurface() const { return _outputSurface; }
	const byte *getPalette() const { return _palette; }
	uint16 getPaletteColorCount() const { return _paletteColorCount; }
	int getTransparentColor() const { return _transparentColor; }
	void setSkipSignature(bool skip) { _skipSignature = skip; }
	void setKeepTransparencyPaletted(bool keep) { _keepTransparencyPaletted = keep; }
private:
	Graphics::PixelFormat getByteOrderRgbaPixelFormat() const;

	byte *_palette;
	uint16 _paletteColorCount;

	// flag to skip the png signature check for headless png files
	bool _skipSignature;

	// Flag to keep paletted images paletted, even when the image has transparency
	bool _keepTransparencyPaletted;
	int _transparentColor;

	Graphics::Surface *_outputSurface;
};

/**
 * Outputs a compressed PNG stream of the given input surface.
 */
bool writePNG(Common::WriteStream &out, const Graphics::Surface &input);
/** @} */
} // End of namespace Image

#endif
