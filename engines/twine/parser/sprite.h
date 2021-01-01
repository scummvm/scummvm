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

#ifndef TWINE_SPRITE_H
#define TWINE_SPRITE_H

#include "common/array.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "graphics/managed_surface.h"
#include "twine/parser/parser.h"
#include "twine/shared.h"

namespace TwinE {

struct SpriteDim {
	int16 x = 0;
	int16 y = 0;
	int16 w = 0;
	int16 h = 0;
};

class SpriteBoundingBoxData : public Parser {
private:
	Common::Array<BoundingBox> _boundingBoxes;
	Common::Array<SpriteDim> _dimensions;

public:
	bool loadFromStream(Common::SeekableReadStream &stream) override;

	const BoundingBox *bbox(int index) const;
	const SpriteDim *dim(int index) const;
};

inline const BoundingBox *SpriteBoundingBoxData::bbox(int index) const {
	if (index < 0) {
		return nullptr;
	}
	return &_boundingBoxes[index];
}

inline const SpriteDim *SpriteBoundingBoxData::dim(int index) const {
	if (index < 0) {
		return nullptr;
	}
	return &_dimensions[index];
}

class SpriteData : public Parser {
private:
	Graphics::ManagedSurface _surface;
	int _offsetX = 0;
	int _offsetY = 0;

public:
	bool loadFromStream(Common::SeekableReadStream &stream) override;

	inline const Graphics::ManagedSurface &surface() const {
		return _surface;
	}

	inline int offsetX() const {
		return _offsetX;
	}

	inline int offsetY() const {
		return _offsetY;
	}
};

} // End of namespace TwinE

#endif
