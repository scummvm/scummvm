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

#ifndef IMAGE_GIF_H
#define IMAGE_GIF_H

#include "image/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Image {

/**
 * @defgroup image_gif GIF decoder
 * @ingroup image
 *
 * @brief Decoder for images encoded as Graphics Interchange Format (GIF).
 *
 * This decoder has a dependency on the libgif library.
 *
 * Used in engines:
 * - TwinE
 * @{
 */
class GIFDecoder : public ImageDecoder {
public:
	GIFDecoder();
	~GIFDecoder();

	bool loadStream(Common::SeekableReadStream &stream) override;
	void destroy() override;
	const byte *getPalette() const override { return _palette; }
	uint16 getPaletteColorCount() const override { return _colorCount; }
	const Graphics::Surface *getSurface() const override { return _outputSurface; }
	int getTransparentColor() const { return _transparentColor; }
private:
	Graphics::Surface *_outputSurface;
	uint8 *_palette;
	uint16 _colorCount;
	int _transparentColor;
};

/** @} */
} // End of namespace Image

#endif
