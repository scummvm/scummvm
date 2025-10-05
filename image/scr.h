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

#ifndef IMAGE_SCR_H
#define IMAGE_SCR_H

#include "graphics/palette.h"
#include "image/image_decoder.h"

/**
 * @defgroup image_scr SCR decoder
 * @ingroup image
 *
 * @brief Decoder for ZX-Spectrum SCREEN$ based on:
 * https://gist.github.com/alexanderk23/f459c76847d9412548f7
 *
 *
 * Used in engines:
 * - Freescape
 * @{
 */

namespace Common {
class SeekableReadStream;
}

namespace Image {

class ScrDecoder : public Image::ImageDecoder {
public:
	ScrDecoder();
	virtual ~ScrDecoder();

	// ImageDecoder API
	void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
	const Graphics::Palette &getPalette() const { return _palette; }
private:
	Graphics::Surface *_surface;
	Graphics::Palette _palette;
	uint32 getPixelAddress(int x, int y);
	uint32 getAttributeAddress(int x, int y);
};
} // End of namespace Image

#endif // IMAGE_SCR_H
