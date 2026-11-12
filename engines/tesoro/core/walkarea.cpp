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

#include "tesoro/core/walkarea.h"

#include "tesoro/core/animbank.h"

namespace Tesoro {

static int32 pointX(const AnimStep &seq, uint i) { return (int16)(seq.entries[i] & 0xffff); }
static int32 pointY(const AnimStep &seq, uint i) { return (int16)(seq.entries[i] >> 16); }

int32 WalkArea::scaleAt(int32 x, int32 y) const {
	if (!_shape)
		return 0x100;
	const AnimStep &seq = *_shape;
	const int32 dx = x - _x;
	const int32 dy = y - _y;
	bool found = false;
	bool below = false;			// the point is under the first segment found
	int32 firstY = 0, firstScale = 0x100;

	for (uint i = 0; i + 1 < seq.entries.size(); ++i) {
		// The segment from its left end to its right end, with the scale at each end.
		int32 lx = pointX(seq, i), ly = pointY(seq, i), rx = pointX(seq, i + 1), ry = pointY(seq, i + 1);
		int32 leftScale = seq.scales[i], rightScale = seq.scales[i + 1];
		if (!(lx < rx)) {
			SWAP(lx, rx);
			SWAP(ly, ry);
			SWAP(leftScale, rightScale);
		}
		if (lx > dx || dx > rx || rx == lx)
			continue;

		const int32 width = rx - lx;
		const int32 segmentY = ((ry - ly) * (dx - lx)) / width + ly;
		if (!found) {
			found = true;
			firstScale = ((rightScale - leftScale) * (dx - lx)) / width + leftScale;
			below = dy < segmentY;
			firstY = segmentY;
			continue;
		}

		// The second segment has to be on the other side of the point.
		if (below ? segmentY > dy : dy > segmentY)
			continue;
		const int32 scale = ((dx - lx) * (rightScale - leftScale)) / width + leftScale;
		if (firstY == segmentY)
			return scale;
		if (!below)
			return ((dy - firstY) * (scale - firstScale)) / (segmentY - firstY) + firstScale;
		return ((dy - segmentY) * (firstScale - scale)) / (firstY - segmentY) + scale;
	}
	return found ? firstScale : 0x100;
}

} // End of namespace Tesoro
