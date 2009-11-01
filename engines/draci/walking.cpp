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
 * $URL$
 * $Id$
 *
 */

#include <stdlib.h>

#include "common/stream.h"

#include "draci/walking.h"
#include "draci/sprite.h"

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

bool WalkingMap::getPixel(int x, int y) const {
	const byte *pMapByte = _data + _byteWidth * y + x / 8;
	return *pMapByte & (1 << x % 8);
}

bool WalkingMap::isWalkable(int x, int y) const {
	// Convert to map pixels
	return getPixel(x / _deltaX, y / _deltaY);
}

Sprite *WalkingMap::newOverlayFromMap(byte colour) const {
	// HACK: Create a visible overlay from the walking map so we can test it
	byte *wlk = new byte[_realWidth * _realHeight];
	memset(wlk, 255, _realWidth * _realHeight);

	for (int i = 0; i < _mapWidth; ++i) {
		for (int j = 0; j < _mapHeight; ++j) {
			if (getPixel(i, j)) {
				drawOverlayRectangle(i, j, colour, wlk);
			}
		}
	}

	Sprite *ov = new Sprite(_realWidth, _realHeight, wlk, 0, 0, false);
	// ov has taken the ownership of wlk.

	return ov;
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

// We don't use Common::Point due to using static initialization.
int WalkingMap::kDirections[][2] = { {0, -1}, {0, +1}, {-1, 0}, {+1, 0} };

bool WalkingMap::findShortestPath(Common::Point p1, Common::Point p2, Path *path) const {
	// Round the positions to map squares.
	p1.x /= _deltaX;
	p2.x /= _deltaX;
	p1.y /= _deltaY;
	p2.y /= _deltaY;

	// Allocate buffers for breadth-first search.  The buffer of points for
	// exploration should be large enough.
	int8 *cameFrom = new int8[_mapWidth * _mapHeight];
	const int bufSize = 4 * _realHeight;
	Common::Point *toSearch = new Common::Point[bufSize];

	// Insert the starting point as a single seed.
	int toRead = 0, toWrite = 0;
	memset(cameFrom, -1, _mapWidth * _mapHeight);	// -1 = not found yet
	cameFrom[p1.y * _mapWidth + p1.x] = 0;
	toSearch[toWrite++] = p1;

	// Search until we empty the whole buffer (not found) or find the
	// destination point.
	while (toRead != toWrite) {
		const Common::Point &here = toSearch[toRead];
		const int from = cameFrom[here.y * _mapWidth + here.x];
		if (here == p2) {
			break;
		}
		// Look into all 4 directions in a particular order depending
		// on the direction we came to this point from.  This is to
		// ensure that among many paths of the same length, the one
		// with the smallest number of turns is preferred.
		for (int addDir = 0; addDir < 4; ++addDir) {
			const int probeDirection = (from + addDir) % 4;
			const Common::Point p(here.x + kDirections[probeDirection][0], here.y + kDirections[probeDirection][1]);
			if (p.x < 0 || p.x >= _mapWidth || p.y < 0 || p.y >= _mapHeight) {
				continue;
			}
			// If this point is walkable and we haven't seen it
			// yet, record how we have reached it and insert it
			// into the round buffer for exploration.
			if (getPixel(p.x, p.y) && cameFrom[p.y * _mapWidth + p.x] == -1) {
				cameFrom[p.y * _mapWidth + p.x] = probeDirection;
				toSearch[toWrite++] = p;
				toWrite %= bufSize;
			}
		}
		++toRead;
		toRead %= bufSize;
	}

	// The path doesn't exist.
	if (toRead == toWrite) {
		return false;
	}

	// Trace the path back and store it.  Count the path length, resize the
	// output array, and then track the pack from the end.
	path->clear();
	int length = 0;
	for (int pass = 0; pass < 2; ++pass) {
		Common::Point p = p2;
		int index = 0;
		while (1) {
			++index;
			if (pass == 1) {
				(*path)[length - index] = p;
			}
			if (p == p1) {
				break;
			}
			const int from = cameFrom[p.y * _mapWidth + p.x];
			p.x -= kDirections[from][0];
			p.y -= kDirections[from][1];
		}
		if (pass == 0) {
			length = index;
			path->resize(length);
		}
	}

	delete[] cameFrom;
	delete[] toSearch;

	return true;
}

void WalkingMap::obliquePath(const WalkingMap::Path& path, WalkingMap::Path *obliquedPath) const {
	// Prune the path to only contain vertices where the direction is changing.
	obliquedPath->clear();
	if (path.empty()) {
		return;
	}
	obliquedPath->push_back(path[0]);
	uint index = 1;
	while (index < path.size()) {
		// index1 points to the last vertex inserted into the
		// simplified path.
		uint index1 = index - 1;
		// Probe the vertical direction.  Notice that the shortest path
		// never turns by 180 degrees and therefore it is sufficient to
		// test that the X coordinates are equal.
		while (index < path.size() && path[index].x == path[index1].x) {
			++index;
		}
		if (index - 1 > index1) {
			index1 = index - 1;
			obliquedPath->push_back(path[index1]);
		}
		// Probe the horizontal direction.
		while (index < path.size() && path[index].y == path[index1].y) {
			++index;
		}
		if (index - 1 > index1) {
			index1 = index - 1;
			obliquedPath->push_back(path[index1]);
		}
	}

	// Making the path oblique works as follows.  If the path has at least
	// 3 remaining vertices, we try to oblique the L-shaped path between
	// them.  If this can be done (i.e., all points on the line between the
	// 1st and 3rd point are walkable), we remove the 2nd vertex (now the
	// path will go directly from the 1st vertex to the 3rd one), and
	// continue obliqueing from the same index, otherwise we leave the
	// first edge (going from the 1st vertex to the 2nd one) as is, move
	// the index to the 2nd vertex, and continue.
	for (uint head = 2; head < obliquedPath->size(); ++head) {
		const Common::Point &v1 = (*obliquedPath)[head-2];
		const Common::Point &v3 = (*obliquedPath)[head];
		const int steps = MAX(abs(v3.x - v1.x), abs(v3.y - v1.y));
		bool allPointsOk = true;
		// Testing only points between (i.e., without the end-points) is OK.
		for (int step = 1; step < steps; ++step) {
			const int x = (v1.x * (steps-step) + v3.x * step + steps/2) / steps;
			const int y = (v1.y * (steps-step) + v3.y * step + steps/2) / steps;
			if (!getPixel(x, y)) {
				allPointsOk = false;
				break;
			}
		}
		if (allPointsOk) {
			obliquedPath->remove_at(--head);
		}
	}
}

Sprite *WalkingMap::newOverlayFromPath(const WalkingMap::Path &path, byte colour) const {
	// HACK: Create a visible overlay from the walking map so we can test it
	byte *wlk = new byte[_realWidth * _realHeight];
	memset(wlk, 255, _realWidth * _realHeight);

	for (uint segment = 1; segment < path.size(); ++segment) {
		const Common::Point &v1 = path[segment-1];
		const Common::Point &v2 = path[segment];
		const int steps = MAX(abs(v2.x - v1.x), abs(v2.y - v1.y));
		// Draw only points in the interval [v1, v2).  These half-open
		// half-closed intervals connect all the way to the last point.
		for (int step = 0; step < steps; ++step) {
			const int x = (v1.x * (steps-step) + v2.x * step + steps/2) / steps;
			const int y = (v1.y * (steps-step) + v2.y * step + steps/2) / steps;
			drawOverlayRectangle(x, y, colour, wlk);
		}
	}
	// Draw the last point.  This works also when the path has no segment,
	// but just one point.
	if (path.size() > 0) {
		const Common::Point &vLast = path[path.size()-1];
		drawOverlayRectangle(vLast.x, vLast.y, colour, wlk);
	}

	Sprite *ov = new Sprite(_realWidth, _realHeight, wlk, 0, 0, false);
	// ov has taken the ownership of wlk.

	return ov;
}

void WalkingMap::drawOverlayRectangle(int x, int y, byte colour, byte *buf) const {
	for (int i = 0; i < _deltaX; ++i) {
		for (int j = 0; j < _deltaY; ++j) {
			buf[(y * _deltaY + j) * _realWidth + (x * _deltaX + i)] = colour;
		}
	}
}

}
