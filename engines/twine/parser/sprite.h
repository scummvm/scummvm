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

#ifndef TWINE_PARSER_SPRITE_H
#define TWINE_PARSER_SPRITE_H

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

// PtrZvExtra
class SpriteBoundingBoxData : public Parser {
private:
	Common::Array<BoundingBox> _boundingBoxes;
	Common::Array<SpriteDim> _dimensions;

public:
	bool loadFromStream(Common::SeekableReadStream &stream, bool lba1) override;

	const BoundingBox *bbox(int index) const; // PtrZvAnim3DS, PtrZvExtra, PtrZvExtraRaw
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
protected:
	Graphics::ManagedSurface _surfaces[2];
	int _offsetX[2] {0};
	int _offsetY[2] {0};
	int _sprites = 0;
	bool _bricks = false;

	bool loadSprite(Common::SeekableReadStream &stream, uint32 offset);
	void reset() override;
public:
	bool loadFromStream(Common::SeekableReadStream &stream, bool lba1) override;

	inline const Graphics::ManagedSurface &surface(int index = 0) const {
		if (index < 0 || index >= _sprites) {
			error("Sprite surface index out of range: %i (max: %i)", index, _sprites);
		}
		return _surfaces[index];
	}

	inline int sprites() const {
		return _sprites;
	}

	inline int offsetX(int index = 0) const {
		if (index < 0 || index >= _sprites) {
			error("Sprite offset index out of range: %i (max: %i)", index, _sprites);
		}
		return _offsetX[index];
	}

	inline int offsetY(int index = 0) const {
		if (index < 0 || index >= _sprites) {
			error("Sprite offset index out of range: %i (max: %i)", index, _sprites);
		}
		return _offsetY[index];
	}
};

class BrickData : public SpriteData {
public:
	BrickData() {
		_bricks = true;
	}
};

} // End of namespace TwinE

#endif
