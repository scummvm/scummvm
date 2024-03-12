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

#ifndef M4_GRAPHICS_GR_SURFACE_H
#define M4_GRAPHICS_GR_SURFACE_H

#include "common/types.h"
#include "m4/m4_types.h"

namespace M4 {

class M4Surface : public Buffer {
private:
	DisposeAfterUse::Flag _disposeAfterUse = DisposeAfterUse::NO;

	void drawInner(const Buffer &src, const byte *depthCodes, int x, int y,
		bool forwards, int srcDepth, const byte *palette, const byte *inverseColorTable);

public:
	M4Surface() : Buffer() {}
	M4Surface(const Buffer &src) : Buffer(src) {}
	M4Surface(int sw, int sh);
	M4Surface(const byte *src, int sw, int sh);

	~M4Surface();

	/**
	 * Simple drawing at a given position given source RLE data.
	 * In this simplified version, the sprite must be entirely on-screen
	 */
	void rleDraw(const byte *src, int x = 0, int y = 0);

	/**
	 * Main drawing
	 */
	void draw(const Buffer &src, int x, int y, bool forwards = true,
		const byte *depthCodes = nullptr, int srcDepth = -1,
		const byte *inverseColorTable = nullptr, const byte *palette = nullptr);
};

} // namespace M4

#endif
