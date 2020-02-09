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

#ifndef GLK_RAW_DECODER_H
#define GLK_RAW_DECODER_H

#include "graphics/surface.h"
#include "image/image_decoder.h"

namespace Glk {

/**
 * This image decoder class implements loading of a simplified image format.
 * It's used for sub-engines like Frotz with custom picture formats. They can
 * expose their picture archives using Common::Archive, and have the individual
 * picture files stored in the format for this decoder to load
 * Format:
 * width		2 bytes
 * height		2 bytes
 * pal size		1 byte
 * palette		3 bytes * pal size
 * pixels		width * height pixels
 */
class RawDecoder : public Image::ImageDecoder {
private:
	Graphics::Surface _surface;
	byte *_palette;
	uint16 _paletteColorCount;
	int _transColor;
public:
	RawDecoder();
	~RawDecoder() override;

	bool loadStream(Common::SeekableReadStream &stream) override;
	void destroy() override;
	const Graphics::Surface *getSurface() const override { return &_surface; }
	const byte *getPalette() const override { return _palette; }
	uint16 getPaletteColorCount() const override { return _paletteColorCount; }
	int getTransparentColor() const { return _transColor; }
};

} // End of namespace Glk

#endif
