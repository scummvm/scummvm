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

#ifndef IMAGE_CICN_H
#define IMAGE_CICN_H

#include "image/image_decoder.h"

namespace Image {

/**
 * @defgroup image_cicn cicn decoder
 * @ingroup image
 *
 * @brief Decoder for cicn images.
 *
 * Used in engines:
 * - SCUMM
 * @{
 */

class CicnDecoder : public ImageDecoder {
public:
	CicnDecoder();
	virtual ~CicnDecoder();

	// ImageDecoder API
	void destroy() override;
	bool loadStream(Common::SeekableReadStream &stream) override;
	const Graphics::Surface *getSurface() const override { return _surface; }
	const byte *getPalette() const override { return _palette; }
	uint16 getPaletteColorCount() const override { return _paletteColorCount; }
	const Graphics::Surface *getMask() const override { return _mask; }

private:
	Graphics::Surface *_surface;
	byte *_palette;
	uint16 _paletteColorCount;
	Graphics::Surface *_mask;
};

/** @} */
} // End of namespace Image

#endif
