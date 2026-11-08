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

#ifndef TESORO_CORE_ANIMIMAGE_H
#define TESORO_CORE_ANIMIMAGE_H

#include "common/array.h"
#include "common/stream.h"

namespace Tesoro {

/**
 * Single image of an animation bank, kept as the blob stored in the archive.
 * Layout: pixel data offset, pixel data size, type, a list of spans, each pointing at an array of 8 byte items.
 */
class AnimImage {
public:
	enum Type {
		kLiteral = 1,	// the first dword of an item is the offset of its pixels
		kShadow = 2		// the first dword of an item is a shadow intensity
		// any other value: the first dword of an item is a solid color
	};

	/** Reads a size-prefixed blob. */
	bool load(Common::SeekableReadStream &stream);

	bool isLoaded() const { return !_data.empty(); }
	const byte *data() const { return _data.empty() ? nullptr : &_data[0]; }
	uint32 size() const { return _data.size(); }
	uint32 pixelOffset() const;
	uint32 pixelBytes() const;
	/** The pixels, for the game to draw its own picture into; null when the image is not loaded or its pixels do not fit. */
	byte *pixelData();
	uint16 type() const;

private:
	bool convert();

	Common::Array<byte> _data;
};

} // End of namespace Tesoro

#endif
