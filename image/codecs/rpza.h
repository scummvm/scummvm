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

#ifndef IMAGE_CODECS_RPZA_H
#define IMAGE_CODECS_RPZA_H

#include "graphics/pixelformat.h"
#include "image/codecs/codec.h"

namespace Image {

/**
 * Apple RPZA decoder.
 *
 * Used by PICT/QuickTime.
 */
class RPZADecoder : public Codec {
public:
	RPZADecoder(uint16 width, uint16 height);
	~RPZADecoder();

	const Graphics::Surface *decodeFrame(Common::SeekableReadStream &stream);
	Graphics::PixelFormat getPixelFormat() const { return _format; }

	bool containsPalette() const { return _ditherPalette != 0; }
	const byte *getPalette() { _dirtyPalette = false; return _ditherPalette; }
	bool hasDirtyPalette() const { return _dirtyPalette; }
	bool canDither(DitherType type) const;
	void setDither(DitherType type, const byte *palette);

private:
	Graphics::PixelFormat _format;
	Graphics::Surface *_surface;
	byte *_ditherPalette;
	bool _dirtyPalette;
	byte *_colorMap;
	uint16 _width, _height;
	uint16 _blockWidth, _blockHeight;
};

} // End of namespace Image

#endif
