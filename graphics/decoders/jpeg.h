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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/**
 * @file
 * Image decoder used in engines:
 *  - groovie
 *  - mohawk
 *  - wintermute
 */

#ifndef GRAPHICS_JPEG_H
#define GRAPHICS_JPEG_H

#include "graphics/surface.h"
#include "graphics/decoders/image_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {

class JPEGDecoder : public ImageDecoder {
public:
	JPEGDecoder();
	~JPEGDecoder();

	// ImageDecoder API
	void destroy();
	bool loadStream(Common::SeekableReadStream &str);
	const Surface *getSurface() const;

	const Surface &getYComponent() const { return _yComponent; }
	const Surface &getUComponent() const { return _uComponent; }
	const Surface &getVComponent() const { return _vComponent; }

private:
	// mutable so that we can convert to RGB only during
	// a getSurface() call while still upholding the
	// const requirement in other ImageDecoders
	mutable Graphics::Surface *_rgbSurface;

	Graphics::Surface _yComponent;
	Graphics::Surface _uComponent;
	Graphics::Surface _vComponent;
};

} // End of Graphics namespace

#endif // GRAPHICS_JPEG_H
