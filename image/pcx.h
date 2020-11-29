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

#ifndef IMAGE_PCX_H
#define IMAGE_PCX_H

#include "common/scummsys.h"
#include "common/str.h"
#include "image/image_decoder.h"

namespace Common{
class SeekableReadStream;
}

namespace Image {

/**
 * @defgroup image_pcx PCX decoder
 * @ingroup image
 *
 * @brief Decoder for PCX images.
 *
 * Used in engines:
 * - Dreamweb
 * - Hugo
 * - Queen
 * - Tucker
 * @{
 */

class PCXDecoder : public ImageDecoder {
public:
	PCXDecoder();
	virtual ~PCXDecoder();

	// ImageDecoder API
	void destroy();
	virtual bool loadStream(Common::SeekableReadStream &stream);
	virtual const Graphics::Surface *getSurface() const { return _surface; }
	const byte *getPalette() const { return _palette; }
	uint16 getPaletteColorCount() const { return _paletteColorCount; }

private:
	void decodeRLE(Common::SeekableReadStream &stream, byte *dst, uint32 bytesPerScanline, bool compressed);

	Graphics::Surface *_surface;
	byte *_palette;
	uint16 _paletteColorCount;
};
/** @} */
} // End of namespace Image

#endif
