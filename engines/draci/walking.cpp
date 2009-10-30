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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/draci/game.cpp $
 * $Id: game.cpp 45505 2009-10-29 18:15:12Z eriktorbjorn $
 *
 */

#include "common/stream.h"

#include "draci/walking.h"

namespace Draci {

void WalkingMap::load(const byte *data, uint length) {
	Common::MemoryReadStream mapReader(data, length);

	_realWidth = mapReader.readUint16LE();
	_realHeight = mapReader.readUint16LE();
	_deltaX = mapReader.readUint16LE();
	_deltaY = mapReader.readUint16LE();
	_mapWidth = mapReader.readUint16LE();
	_mapHeight = mapReader.readUint16LE();
	_byteWidth = mapReader.readUint16LE();

	// Set the data pointer to raw map data
	_data = data + mapReader.pos();
}

bool WalkingMap::isWalkable(int x, int y) const {
	// Convert to map pixels
	x = x / _deltaX;
	y = y / _deltaY;

	int pixelIndex = _mapWidth * y + x;
	int byteIndex = pixelIndex / 8;
	int mapByte = _data[byteIndex];

	return mapByte & (1 << pixelIndex % 8);
}

/**
 * @brief For a given point, find a nearest walkable point on the walking map
 *
 * @param startX    x coordinate of the point
 * @param startY    y coordinate of the point
 *
 * @return A Common::Point representing the nearest walkable point
 *
 *  The algorithm was copied from the original engine for exactness.
 *  TODO: Study this algorithm in more detail so it can be documented properly and
 *  possibly improved / simplified.
 */
Common::Point WalkingMap::findNearestWalkable(int startX, int startY, Common::Rect searchRect) const {
	// If the starting point is walkable, just return that
	if (searchRect.contains(startX, startY) && isWalkable(startX, startY)) {
		return Common::Point(startX, startY);
	}

	int signs[] = { 1, -1 };
	const uint kSignsNum = 2;

	int radius = 0;
	int x, y;
	int dx, dy;
	int prediction;

	// The place where, eventually, the result coordinates will be stored
	int finalX, finalY;

	// The algorithm appears to start off with an ellipse with the minor radius equal to
	// zero and the major radius equal to the walking map delta (the number of pixels
	// one map pixel represents). It then uses a heuristic to gradually reshape it into
	// a circle (by shortening the major radius and lengthening the minor one). At each
	// such resizing step, it checks some select points on the ellipse for walkability.
	// It also does the same check for the ellipse perpendicular to it (rotated by 90 degrees).

	while (1) {
		// The default major radius
		radius += _deltaX;

		// The ellipse radii (minor, major) that get resized
		x = 0;
		y = radius;

		// Heuristic variables
		prediction = 1 - radius;
		dx = 3;
		dy = 2 * radius - 2;

		do {
			// The following two loops serve the purpose of checking the points on the two
			// ellipses for walkability. The signs[] array is there to obliterate the need
			// of writing out all combinations manually.

			for (uint i = 0; i < kSignsNum; ++i) {
				finalY = startY + y * signs[i];

				for (uint j = 0; j < kSignsNum; ++j) {
					finalX = startX + x * signs[j];

					// If the current point is walkable, return it
					if (searchRect.contains(finalX, finalY) && isWalkable(finalX, finalY)) {
						return Common::Point(finalX, finalY);
					}
				}
			}

			if (x == y) {
				// If the starting point is walkable, just return that
				if (searchRect.contains(finalX, finalY) && isWalkable(finalX, finalY)) {
					return Common::Point(finalX, finalY);
				}
			}

			for (uint i = 0; i < kSignsNum; ++i) {
				finalY = startY + x * signs[i];

				for (uint j = 0; j < kSignsNum; ++j) {
					finalX = startX + y * signs[j];

					// If the current point is walkable, return it
					if (searchRect.contains(finalX, finalY) && isWalkable(finalX, finalY)) {
						return Common::Point(finalX, finalY);
					}
				}
			}

			// If prediction is non-negative, we need to decrease the major radius of the
			// ellipse
			if (prediction >= 0) {
				prediction -= dy;
				dy -= 2 * _deltaX;
				y -= _deltaX;
			}

			// Increase the minor radius of the ellipse and update heuristic variables
			prediction += dx;
			dx += 2 * _deltaX;
			x += _deltaX;

		// If the current ellipse has been reshaped into a circle,
		// end this loop and enlarge the radius
		} while (x <= y);
	}
}

}
