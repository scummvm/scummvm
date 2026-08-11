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

#ifndef TWINE_SCENE_GRAPH_H
#define TWINE_SCENE_GRAPH_H

#include "graphics/screen.h"
#include "twine/shared.h"

namespace TwinE {

// WARNING: Rewrite this function to have better performance
inline bool drawGraph(int32 posX, int32 posY, const uint8 *pGraph, bool isSprite, Graphics::Screen &frontVideoBuffer, const Common::Rect &clip) { // AffGraph
	if (!clip.isValidRect()) {
		return false;
	}

	const int32 left = posX + pGraph[2];
	if (left >= clip.right) {
		return false;
	}
	const int32 top = posY + pGraph[3];
	if (top >= clip.bottom) {
		return false;
	}
	const int32 right = pGraph[0] + left;
	if (right < clip.left) {
		return false;
	}
	const int32 bottom = pGraph[1] + top;
	if (bottom < clip.top) {
		return false;
	}
	const int32 maxY = MIN(bottom, (int32)clip.bottom);

	pGraph += 4; // skip the header

	int32 x = left;

	// if (left >= textWindowLeft-2 && top >= textWindowTop-2 && right <= textWindowRight-2 && bottom <= textWindowBottom-2) // crop
	{
		for (int32 y = top; y < maxY; ++y) {
			const uint8 rleAmount = *pGraph++;
			for (int32 run = 0; run < rleAmount; ++run) {
				const uint8 rleMask = *pGraph++;
				const uint8 iterations = bits(rleMask, 0, 6) + 1;
				const uint8 opCode = bits(rleMask, 6, 2);
				if (opCode == 0) {
					x += iterations;
					continue;
				}
				if (y < clip.top || x >= clip.right || x + iterations < clip.left) {
					if (opCode == 1) {
						pGraph += iterations;
					} else {
						++pGraph;
					}
					x += iterations;
					continue;
				}
				if (opCode == 1) { // 0x40
					uint8 *out = (uint8 *)frontVideoBuffer.getBasePtr(x, y);
					for (uint8 i = 0; i < iterations; i++) {
						if (x >= clip.left && x < clip.right) {
							*out = *pGraph;
						}

						++out;
						++x;
						++pGraph;
					}
				} else {
					const uint8 pixel = *pGraph++;
					uint8 *out = (uint8 *)frontVideoBuffer.getBasePtr(x, y);
					for (uint8 i = 0; i < iterations; i++) {
						if (x >= clip.left && x < clip.right) {
							*out = pixel;
						}

						++out;
						++x;
					}
				}
			}
			x = left;
		}
	}

	Common::Rect rect(left, top, right, bottom);
	frontVideoBuffer.addDirtyRect(rect);

	return true;
}

} // namespace TwinE

#endif // TWINE_SCENE_GRAPH_H
