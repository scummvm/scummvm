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
*/

#ifndef TOON_PATH_H
#define TOON_PATH_H

#include "common/array.h"

#include "toon/toon.h"

namespace Toon {

// binary heap system for fast A*
class PathFindingHeap {
public:
	PathFindingHeap();
	~PathFindingHeap();

	void push(int16 x, int16 y, int32 weight);
	void pop(int16 *x, int16 *y, int32 *weight);
	void init(int32 size);
	void clear();
	void unload();
	int32 getCount() { return _count; }

private:
	struct HeapDataGrid {
		int16 _x, _y;
		int16 _weight;
	};

	HeapDataGrid *_data;

	int32 _size;
	int32 _count;
};

class PathFinding {
public:
	PathFinding();
	~PathFinding();

	void init(Picture *mask);

	bool findPath(int32 x, int32 y, int32 destX, int32 destY);
	bool findClosestWalkingPoint(int32 xx, int32 yy, int32 *fxx, int32 *fyy, int origX = -1, int origY = -1);
	bool isWalkable(int32 x, int32 y);
	bool isLikelyWalkable(int32 x, int32 y);
	bool lineIsWalkable(int32 x, int32 y, int32 x2, int32 y2);
	bool walkLine(int32 x, int32 y, int32 x2, int32 y2);

	void resetBlockingRects() { _numBlockingRects = 0; }
	void addBlockingRect(int32 x1, int32 y1, int32 x2, int32 y2);
	void addBlockingEllipse(int32 x1, int32 y1, int32 w, int32 h);

	int32 getPathNodeCount() const { return _tempPath.size(); }
	int32 getPathNodeX(int32 nodeId) const { return _tempPath[ _tempPath.size() - nodeId - 1].x; }
	int32 getPathNodeY(int32 nodeId) const { return _tempPath[ _tempPath.size() - nodeId - 1].y; }

private:
	static const uint8 kMaxBlockingRects = 16;

	Picture *_currentMask;

	PathFindingHeap *_heap;

	int32 *_sq;
	int32 _width;
	int32 _height;

	struct i32Point {
		int32 x, y;
	};

	Common::Array<i32Point> _tempPath;

	int32 _blockingRects[kMaxBlockingRects][5];
	uint8 _numBlockingRects;
};

} // End of namespace Toon

#endif
