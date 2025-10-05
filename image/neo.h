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

#ifndef IMAGE_NEO_H
#define IMAGE_NEO_H

#include "graphics/palette.h"
#include "image/image_decoder.h"

/**
 * @defgroup image_neo Neochrome decoder
 * @ingroup image
 *
 * @brief Atari-ST Neochrome decoder based on NEOLoad by Jason "Joefish" Railton
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

class NeoDecoder : public Image::ImageDecoder {
public:
	NeoDecoder(byte *palette = nullptr);
	virtual ~NeoDecoder();

	// ImageDecoder API
	void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
	const Graphics::Palette &getPalette() const { return _palette; }
	uint16 getPaletteColorCount() const { return _palette.size(); }

private:
	Graphics::Surface *_surface;
	Graphics::Palette _palette;
};
} // End of namespace Image

#endif // IMAGE_NEO_H
