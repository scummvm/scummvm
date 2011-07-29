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

#include "common/debug.h"

#include "toon/path.h"

namespace Toon {

PathFindingHeap::PathFindingHeap() {
	_count = 0;
	_size = 0;
	_data = NULL;
}

PathFindingHeap::~PathFindingHeap() {
	delete[] _data;
}

void PathFindingHeap::init(int32 size) {
	debugC(1, kDebugPath, "init(%d)", size);
	_size = size;

	delete[] _data;
	_data = new HeapDataGrid[_size];
	memset(_data, 0, sizeof(HeapDataGrid) * _size);
	_count = 0;
}

void PathFindingHeap::unload() {
	_count = 0;
	_size = 0;
	delete[] _data;
	_data = NULL;
}

void PathFindingHeap::clear() {
	debugC(1, kDebugPath, "clear()");

	_count = 0;
	memset(_data, 0, sizeof(HeapDataGrid) * _size);
}

void PathFindingHeap::push(int32 x, int32 y, int32 weight) {
	debugC(2, kDebugPath, "push(%d, %d, %d)", x, y, weight);

	if (_count == _size) {
		warning("Aborting attempt to push onto PathFindingHeap at maximum size: %d", _count);
		return;
	}

	_data[_count]._x = x;
	_data[_count]._y = y;
	_data[_count]._weight = weight;
	_count++;

	int32 lMax = _count-1;
	int32 lT = 0;

	while (1) {
		if (lMax <= 0)
			break;
		lT = (lMax-1) / 2;

		if (_data[lT]._weight > _data[lMax]._weight) {
			HeapDataGrid temp;
			temp = _data[lT];
			_data[lT] = _data[lMax];
			_data[lMax] = temp;
			lMax = lT;
		} else {
			break;
		}
	}
}

void PathFindingHeap::pop(int32 *x, int32 *y, int32 *weight) {
	debugC(2, kDebugPath, "pop(x, y, weight)");

	if (!_count) {
		warning("Attempt to pop empty PathFindingHeap!");
		return;
	}

	*x = _data[0]._x;
	*y = _data[0]._y;
	*weight = _data[0]._weight;

	_data[0] = _data[--_count];
	if (!_count)
		return;

	int32 lMin = 0;
	int32 lT = 0;

	while (1) {
		lT = (lMin << 1) + 1;
		if (lT < _count) {
			if (lT < _count-1) {
				if (_data[lT + 1]._weight < _data[lT]._weight)
					lT++;
			}
			if (_data[lT]._weight <= _data[lMin]._weight) {
				HeapDataGrid temp;
				temp = _data[lMin];
				_data[lMin] = _data[lT];
				_data[lT] = temp;

				lMin = lT;
			} else {
				break;
			}
		} else {
			break;
		}
	}
}

PathFinding::PathFinding(ToonEngine *vm) : _vm(vm) {
	_width = 0;
	_height = 0;
	_heap = new PathFindingHeap();
	_gridTemp = NULL;
	_numBlockingRects = 0;
}

PathFinding::~PathFinding(void) {
	if (_heap)
		_heap->unload();
	delete _heap;
	delete[] _gridTemp;
}

bool PathFinding::isLikelyWalkable(int32 x, int32 y) {
	for (int32 i = 0; i < _numBlockingRects; i++) {
		if (_blockingRects[i][4] == 0) {
			if (x >= _blockingRects[i][0] && x <= _blockingRects[i][2] && y >= _blockingRects[i][1] && y < _blockingRects[i][3])
				return false;
		} else {
			int32 dx = abs(_blockingRects[i][0] - x);
			int32 dy = abs(_blockingRects[i][1] - y);
			if ((dx << 8) / _blockingRects[i][2] < (1 << 8) && (dy << 8) / _blockingRects[i][3] < (1 << 8)) {
				return false;
			}
		}
	}
	return true;
}

bool PathFinding::isWalkable(int32 x, int32 y) {
	debugC(2, kDebugPath, "isWalkable(%d, %d)", x, y);

	bool maskWalk = (_currentMask->getData(x, y) & 0x1f) > 0;

	return maskWalk;
}

int32 PathFinding::findClosestWalkingPoint(int32 xx, int32 yy, int32 *fxx, int32 *fyy, int origX, int origY) {
	debugC(1, kDebugPath, "findClosestWalkingPoint(%d, %d, fxx, fyy, %d, %d)", xx, yy, origX, origY);

	int32 currentFound = -1;
	int32 dist = -1;
	int32 dist2 = -1;

	if (origX == -1)
		origX = xx;
	if (origY == -1)
		origY = yy;

	for (int y = 0; y < _height; y++) {
		for (int x = 0; x < _width; x++) {
			if (isWalkable(x, y) && isLikelyWalkable(x, y)) {
				int32 ndist = (x - xx) * (x - xx) + (y - yy) * (y - yy);
				int32 ndist2 = (x - origX) * (x - origX) + (y - origY) * (y - origY);
				if (currentFound < 0 || ndist < dist || (ndist == dist && ndist2 < dist2)) {
					dist = ndist;
					dist2 = ndist2;
					currentFound = y * _width + x;
				}
			}
		}
	}

	if (currentFound != -1) {
		*fxx = currentFound % _width;
		*fyy = currentFound / _width;
		return 1;
	} else {
		*fxx = 0;
		*fyy = 0;
		return 0;
	}
}

bool PathFinding::walkLine(int32 x, int32 y, int32 x2, int32 y2) {
	uint32 bx = x << 16;
	int32 dx = x2 - x;
	uint32 by = y << 16;
	int32 dy = y2 - y;
	uint32 adx = abs(dx);
	uint32 ady = abs(dy);
	int32 t = 0;
	if (adx <= ady)
		t = ady;
	else
		t = adx;

	int32 cdx = (dx << 16) / t;
	int32 cdy = (dy << 16) / t;

	int32 i = t;
	_gridPathCount = 0;
	while (i) {
		_tempPathX[i] = bx >> 16;
		_tempPathY[i] = by >> 16;
		_gridPathCount++;
		bx += cdx;
		by += cdy;
		i--;
	}

	_tempPathX[0] = x2;
	_tempPathY[0] = y2;

	return true;
}

bool PathFinding::lineIsWalkable(int32 x, int32 y, int32 x2, int32 y2) {
	uint32 bx = x << 16;
	int32 dx = x2 - x;
	uint32 by = y << 16;
	int32 dy = y2 - y;
	uint32 adx = abs(dx);
	uint32 ady = abs(dy);
	int32 t = 0;
	if (adx <= ady)
		t = ady;
	else
		t = adx;

	int32 cdx = (dx << 16) / t;
	int32 cdy = (dy << 16) / t;

	int32 i = t;
	while (i) {
		if (!isWalkable(bx >> 16, by >> 16))
			return false;
		bx += cdx;
		by += cdy;
		i--;
	}
	return true;
}
int32 PathFinding::findPath(int32 x, int32 y, int32 destx, int32 desty) {
	debugC(1, kDebugPath, "findPath(%d, %d, %d, %d)", x, y, destx, desty);

	if (x == destx && y == desty) {
		_gridPathCount = 0;
		return true;
	}

	// ignore path finding if the character is outside the screen
	if (x < 0 || x > 1280 || y < 0 || y > 400 || destx < 0 || destx > 1280 || desty < 0 || desty > 400) {
		_gridPathCount = 0;
		return true;
	}

	// first test direct line
	if (lineIsWalkable(x, y, destx, desty)) {
		walkLine(x, y, destx, desty);
		return true;
	}

	// no direct line, we use the standard A* algorithm
	memset(_gridTemp , 0, _width * _height * sizeof(int32));
	_heap->clear();
	int32 curX = x;
	int32 curY = y;
	int32 curWeight = 0;
	int32 *sq = _gridTemp;

	sq[curX + curY *_width] = 1;
	_heap->push(curX, curY, abs(destx - x) + abs(desty - y));
	int wei = 0;

	while (_heap->getCount()) {
		wei = 0;
		_heap->pop(&curX, &curY, &curWeight);
		int curNode = curX + curY * _width;

		int32 endX = MIN<int32>(curX + 1, _width - 1);
		int32 endY = MIN<int32>(curY + 1, _height - 1);
		int32 startX = MAX<int32>(curX - 1, 0);
		int32 startY = MAX<int32>(curY - 1, 0);

		for (int32 px = startX; px <= endX; px++) {
			for (int py = startY; py <= endY; py++) {
				if (px != curX || py != curY) {
					wei = ((abs(px - curX) + abs(py - curY)));

					int32 curPNode = px + py * _width;
					if (isWalkable(px, py)) { // walkable ?
						int sum = sq[curNode] + wei * (1 + (isLikelyWalkable(px, py) ? 5 : 0));
						if (sq[curPNode] > sum || !sq[curPNode]) {
							int newWeight = abs(destx - px) + abs(desty - py);
							sq[curPNode] = sum;
							_heap->push(px, py, sq[curPNode] + newWeight);
							if (!newWeight)
								goto next; // we found it !
						}
					}
				}
			}
		}
	}

next:

	// let's see if we found a result !
	if (!_gridTemp[destx + desty * _width]) {
		// didn't find anything
		_gridPathCount = 0;
		return false;
	}

	curX = destx;
	curY = desty;

	int32 *retPathX = (int32 *)malloc(4096 * sizeof(int32));
	int32 *retPathY = (int32 *)malloc(4096 * sizeof(int32));
	if (!retPathX || !retPathY) {
		free(retPathX);
		free(retPathY);

		error("[PathFinding::findPath] Cannot allocate pathfinding buffers");
	}

	int32 numpath = 0;

	retPathX[numpath] = curX;
	retPathY[numpath] = curY;
	numpath++;
	int32 bestscore = sq[destx + desty * _width];

	while (1) {
		int32 bestX = -1;
		int32 bestY = -1;

		int32 endX = MIN<int32>(curX + 1, _width - 1);
		int32 endY = MIN<int32>(curY + 1, _height - 1);
		int32 startX = MAX<int32>(curX - 1, 0);
		int32 startY = MAX<int32>(curY - 1, 0);

		for (int32 px = startX; px <= endX; px++) {
			for (int32 py = startY; py <= endY; py++) {
				if (px != curX || py != curY) {
					wei = abs(px - curX) + abs(py - curY);

					int PNode = px + py * _width;
					if (sq[PNode] && (isWalkable(px, py))) {
						if (sq[PNode] < bestscore) {
							bestscore = sq[PNode];
							bestX = px;
							bestY = py;
						}
					}
				}
			}
		}

		if (bestX < 0 || bestY < 0) {
			free(retPathX);
			free(retPathY);

			return 0;
		}

		retPathX[numpath] = bestX;
		retPathY[numpath] = bestY;
		numpath++;

		if ((bestX == x && bestY == y)) {
			_gridPathCount = numpath;

			memcpy(_tempPathX, retPathX, sizeof(int32) * numpath);
			memcpy(_tempPathY, retPathY, sizeof(int32) * numpath);

			free(retPathX);
			free(retPathY);

			return true;
		}

		curX = bestX;
		curY = bestY;
	}

	free(retPathX);
	free(retPathY);

	return false;
}

void PathFinding::init(Picture *mask) {
	debugC(1, kDebugPath, "init(mask)");

	_width = mask->getWidth();
	_height = mask->getHeight();
	_currentMask = mask;
	_heap->unload();
	// In order to reduce memory fragmentation on small devices, we use the maximum
	// possible size here which is TOON_BACKBUFFER_WIDTH. Even though this is
	// 1280 as opposed to the possible 640, it actually helps memory allocation on
	// those devices.
	_heap->init(TOON_BACKBUFFER_WIDTH * _height);	// should really be _width
	delete[] _gridTemp;
	_gridTemp = new int32[_width*_height];
}

void PathFinding::resetBlockingRects() {
	_numBlockingRects = 0;
}

void PathFinding::addBlockingRect(int32 x1, int32 y1, int32 x2, int32 y2) {
	debugC(1, kDebugPath, "addBlockingRect(%d, %d, %d, %d)", x1, y1, x2, y2);

	_blockingRects[_numBlockingRects][0] = x1;
	_blockingRects[_numBlockingRects][1] = y1;
	_blockingRects[_numBlockingRects][2] = x2;
	_blockingRects[_numBlockingRects][3] = y2;
	_blockingRects[_numBlockingRects][4] = 0;
	_numBlockingRects++;
}

void PathFinding::addBlockingEllipse(int32 x1, int32 y1, int32 w, int32 h) {
	debugC(1, kDebugPath, "addBlockingRect(%d, %d, %d, %d)", x1, y1, w, h);

	_blockingRects[_numBlockingRects][0] = x1;
	_blockingRects[_numBlockingRects][1] = y1;
	_blockingRects[_numBlockingRects][2] = w;
	_blockingRects[_numBlockingRects][3] = h;
	_blockingRects[_numBlockingRects][4] = 1;
	_numBlockingRects++;
}

int32 PathFinding::getPathNodeCount() const {
	return _gridPathCount;
}

int32 PathFinding::getPathNodeX(int32 nodeId) const {
	return _tempPathX[ _gridPathCount - nodeId - 1];
}

int32 PathFinding::getPathNodeY(int32 nodeId) const {
	return _tempPathY[ _gridPathCount - nodeId - 1];
}

} // End of namespace Toon
