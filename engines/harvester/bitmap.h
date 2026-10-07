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

#ifndef HARVESTER_BITMAP_H
#define HARVESTER_BITMAP_H

#include "common/array.h"

namespace Harvester {

struct IndexedBitmap {
	uint32 width = 0;
	uint32 height = 0;
	Common::Array<byte> pixels;

	bool isValid() const {
		return width != 0 && height != 0 && pixels.size() >= width * height;
	}
};

struct AbmFrame : IndexedBitmap {
	int32 xOffset = 0;
	int32 yOffset = 0;
};

// Callers choose when to clear output. Failed decoding can leave partial header or frame data.
bool decodeBitmap(const Common::Array<byte> &data, IndexedBitmap &bitmap);
bool decodeAbm(const Common::Array<byte> &data, Common::Array<AbmFrame> &frames);

} // End of namespace Harvester

#endif // HARVESTER_BITMAP_H
