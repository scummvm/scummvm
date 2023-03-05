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

#ifndef STARK_GFX_BITMAP_H
#define STARK_GFX_BITMAP_H

#include "common/hash-str.h"

namespace Graphics {
	struct PixelFormat;
	struct Surface;
}

namespace Stark {
namespace Gfx {

/**
 * An abstract bitmap
 */
class Bitmap {
public:
	Bitmap() : _width(0), _height(0) {}
	virtual ~Bitmap() {}

	enum SamplingFilter {
		kNearest,
		kLinear
	};

	/** Make the texture active */
	virtual void bind() const = 0;

	/** Define or update the texture pixel data */
	virtual void update(const Graphics::Surface *surface, const byte *palette = nullptr) = 0;

	/** Set the filter used when sampling the texture */
	virtual void setSamplingFilter(SamplingFilter filter) = 0;

	/** Get the most ideal pixel format for uploading to a texture */
	virtual Graphics::PixelFormat getBestPixelFormat() const = 0;

	/** Get the texture width */
	uint32 width() const { return _width; }

	/** Get the texture height */
	uint32 height() const { return _height; }

protected:
	uint32 _width;
	uint32 _height;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // STARK_GFX_BITMAP_H
