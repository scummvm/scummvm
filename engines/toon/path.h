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

#include "toon/toon.h"

namespace Toon {

// binary heap system for fast A*
struct HeapDataGrid {
	int16 _x, _y;
	int16 _weight;
};

class PathFindingHeap {
public:
	PathFindingHeap();
	~PathFindingHeap();

	void push(int32 x, int32 y, int32 weight);
	void pop(int32 *x, int32 *y, int32 *weight);
	void init(int32 size);
	void clear();
	void unload();
	int32 getCount() { return _count; }

private:
	HeapDataGrid *_data;

	int32 _size;
	int32 _count;
};

class PathFinding {
public:
	PathFinding(ToonEngine *vm);
	~PathFinding();

	int32 findPath(int32 x, int32 y, int32 destX, int32 destY);
	int32 findClosestWalkingPoint(int32 xx, int32 yy, int32 *fxx, int32 *fyy, int origX = -1, int origY = -1);
	bool isWalkable(int32 x, int32 y);
	bool isLikelyWalkable(int32 x, int32 y);
	bool lineIsWalkable(int32 x, int32 y, int32 x2, int32 y2);
	bool walkLine(int32 x, int32 y, int32 x2, int32 y2);
	void init(Picture *mask);

	void resetBlockingRects();
	void addBlockingRect(int32 x1, int32 y1, int32 x2, int32 y2);
	void addBlockingEllipse(int32 x1, int32 y1, int32 w, int32 h);

	int32 getPathNodeCount() const;
	int32 getPathNodeX(int32 nodeId) const;
	int32 getPathNodeY(int32 nodeId) const;
protected:
	Picture *_currentMask;

	PathFindingHeap *_heap;

	int32 *_gridTemp;
	int32 _width;
	int32 _height;

	int32 _tempPathX[4096];
	int32 _tempPathY[4096];
	int32 _blockingRects[16][5];
	int32 _numBlockingRects;
	int32 _allocatedGridPathCount;
	int32 _gridPathCount;

	ToonEngine *_vm;
};

} // End of namespace Toon

#endif
