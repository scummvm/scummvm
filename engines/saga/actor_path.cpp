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

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/scene.h"

namespace Saga {

static const PathDirectionData pathDirectionLUT[8][3] = {
	{ { 0,  0, -1 }, { 7, -1, -1 }, { 4,  1, -1 } },
	{ { 1,  1,  0 }, { 4,  1, -1 }, { 5,  1,  1 } },
	{ { 2,  0,  1 }, { 5,  1,  1 }, { 6, -1,  1 } },
	{ { 3, -1,  0 }, { 6, -1,  1 }, { 7, -1, -1 } },
	{ { 0,  0, -1 }, { 1,  1,  0 }, { 4,  1, -1 } },
	{ { 1,  1,  0 }, { 2,  0,  1 }, { 5,  1,  1 } },
	{ { 2,  0,  1 }, { 3, -1,  0 }, { 6, -1,  1 } },
	{ { 3, -1,  0 }, { 0,  0, -1 }, { 7, -1, -1 } }
};

static const int pathDirectionLUT2[8][2] = {
	{  0, -1 },
	{  1,  0 },
	{  0,  1 },
	{ -1,  0 },
	{  1, -1 },
	{  1,  1 },
	{ -1,  1 },
	{ -1, -1 }
};

inline int16 int16Compare(int16 i1, int16 i2) {
	return ((i1) > (i2) ? 1 : ((i1) < (i2) ? -1 : 0));
}

inline int16 quickDistance(const Point &point1, const Point &point2, int16 compressX) {
	Point delta;
	delta.x = ABS(point1.x - point2.x) / compressX;
	delta.y = ABS(point1.y - point2.y);
	return ((delta.x < delta.y) ? (delta.y + delta.x / 2) : (delta.x + delta.y / 2));
}

inline void calcDeltaS(const Point &point1, const Point &point2, Point &delta, Point &s) {

	delta.x = point2.x - point1.x;
	if (delta.x == 0) {
		s.x = 0;
	} else {
		if (delta.x > 0) {
			s.x = 1;
		} else {
			s.x = -1;
			delta.x = -delta.x;
		}
	}


	delta.y = point2.y - point1.y;
	if (delta.y == 0) {
		s.y = 0;
	} else {
		if (delta.y > 0) {
			s.y = 1;
		} else {
			s.y = -1;
			delta.y = -delta.y;
		}
	}
}

void Actor::findActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint) {
	Point iteratorPoint;
	Point bestPoint;
	int maskType;
	int i;
	Rect intersect;

#ifdef ACTOR_DEBUG
	_debugPointsCount = 0;
#endif

	actor->_walkStepsCount = 0;
	if (fromPoint == toPoint) {
		actor->addWalkStepPoint(toPoint);
		return;
	}

	for (iteratorPoint.y = 0; iteratorPoint.y < _yCellCount; iteratorPoint.y++) {
		for (iteratorPoint.x = 0; iteratorPoint.x < _xCellCount; iteratorPoint.x++) {
			if (_vm->_scene->validBGMaskPoint(iteratorPoint)) {
				maskType = _vm->_scene->getBGMaskType(iteratorPoint);
				setPathCell(iteratorPoint, _vm->_scene->getDoorState(maskType) ? kPathCellBarrier : kPathCellEmpty);
			} else {
				setPathCell(iteratorPoint, kPathCellBarrier);
			}
		}
	}

	for (i = 0; i < _barrierCount; i++) {
		intersect.left = MAX(_pathRect.left, _barrierList[i].left);
		intersect.top = MAX(_pathRect.top, _barrierList[i].top);
		intersect.right = MIN(_pathRect.right, _barrierList[i].right);
		intersect.bottom = MIN(_pathRect.bottom, _barrierList[i].bottom);

		for (iteratorPoint.y = intersect.top; iteratorPoint.y < intersect.bottom; iteratorPoint.y++) {
			for (iteratorPoint.x = intersect.left; iteratorPoint.x < intersect.right; iteratorPoint.x++) {
				setPathCell(iteratorPoint, kPathCellBarrier);
			}
		}
	}

#ifdef ACTOR_DEBUG
	for (iteratorPoint.y = 0; iteratorPoint.y < _yCellCount; iteratorPoint.y++) {
		for (iteratorPoint.x = 0; iteratorPoint.x < _xCellCount; iteratorPoint.x++) {
			if (getPathCell(iteratorPoint) == kPathCellBarrier) {
				addDebugPoint(iteratorPoint, 24);
			}
		}
	}
#endif

	if (scanPathLine(fromPoint, toPoint)) {
		actor->addWalkStepPoint(fromPoint);
		actor->addWalkStepPoint(toPoint);
		return;
	}

	i = fillPathArray(fromPoint, toPoint, bestPoint);

	if (fromPoint == bestPoint) {
		actor->addWalkStepPoint(bestPoint);
		return;
	}

	if (i == 0) {
		error("fillPathArray returns zero");
	}

	setActorPath(actor, fromPoint, bestPoint);
}

bool Actor::scanPathLine(const Point &point1, const Point &point2) {
	Point point;
	Point delta;
	Point s;
	Point fDelta;
	int16 errterm;

	calcDeltaS(point1, point2, delta, s);
	point = point1;

	fDelta.x = delta.x * 2;
	fDelta.y = delta.y * 2;

	if (delta.y > delta.x) {

		errterm = fDelta.x - delta.y;

		while (delta.y > 0) {
			while (errterm >= 0) {
				point.x += s.x;
				errterm -= fDelta.y;
			}

			point.y += s.y;
			errterm += fDelta.x;

			if (!validPathCellPoint(point)) {
				return false;
			}
			if (getPathCell(point) == kPathCellBarrier) {
				return false;
			}
			delta.y--;
		}
	} else {

		errterm = fDelta.y - delta.x;

		while (delta.x > 0) {
			while (errterm >= 0) {
				point.y += s.y;
				errterm -= fDelta.x;
			}

			point.x += s.x;
			errterm += fDelta.y;

			if (!validPathCellPoint(point)) {
				return false;
			}
			if (getPathCell(point) == kPathCellBarrier) {
				return false;
			}
			delta.x--;
		}
	}
	return true;
}

int Actor::fillPathArray(const Point &fromPoint, const Point &toPoint, Point &bestPoint) {
	int bestRating;
	int currentRating;
	int i;
	Point bestPath;
	int pointCounter;
	int startDirection;
	PathDirectionData *pathDirection;
	PathDirectionData *newPathDirection;
	const PathDirectionData *samplePathDirection;
	Point nextPoint;
	int directionCount;
	int16 compressX = (_vm->getGameType() == GType_ITE) ? 2 : 1;

	_pathDirectionListCount = 0;
	pointCounter = 0;
	bestRating = quickDistance(fromPoint, toPoint, compressX);
	bestPath = fromPoint;

	for (startDirection = 0; startDirection < 4; startDirection++) {
		newPathDirection = addPathDirectionListData();
		newPathDirection->x = fromPoint.x;
		newPathDirection->y = fromPoint.y;
		newPathDirection->direction = startDirection;
	}

	if (validPathCellPoint(fromPoint)) {
		setPathCell(fromPoint, kDirUp);

#ifdef ACTOR_DEBUG
		addDebugPoint(fromPoint, 24+36);
#endif
	}

	i = 0;

	do {
		pathDirection = &_pathDirectionList[i];
		for (directionCount = 0; directionCount < 3; directionCount++) {
			samplePathDirection = &pathDirectionLUT[pathDirection->direction][directionCount];
			nextPoint = Point(pathDirection->x, pathDirection->y);
			nextPoint.x += samplePathDirection->x;
			nextPoint.y += samplePathDirection->y;

			if (!validPathCellPoint(nextPoint)) {
				continue;
			}

			if (getPathCell(nextPoint) != kPathCellEmpty) {
				continue;
			}

			setPathCell(nextPoint, samplePathDirection->direction);

#ifdef ACTOR_DEBUG
			addDebugPoint(nextPoint, samplePathDirection->direction + 96);
#endif
			newPathDirection = addPathDirectionListData();
			newPathDirection->x = nextPoint.x;
			newPathDirection->y = nextPoint.y;
			newPathDirection->direction = samplePathDirection->direction;
			++pointCounter;
			if (nextPoint == toPoint) {
				bestPoint = toPoint;
				return pointCounter;
			}
			currentRating = quickDistance(nextPoint, toPoint, compressX);
			if (currentRating < bestRating) {
				bestRating = currentRating;
				bestPath = nextPoint;
			}
			pathDirection = &_pathDirectionList[i];
		}
		++i;
	} while (i < _pathDirectionListCount);

	bestPoint = bestPath;
	return pointCounter;
}

void Actor::setActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint) {
	Point nextPoint;
	int8 direction;
	int i;

	_pathListIndex = -1;
	addPathListPoint(toPoint);
	nextPoint = toPoint;

	while (!(nextPoint == fromPoint)) {
		direction = getPathCell(nextPoint);
		if ((direction < 0) || (direction >= 8)) {
			error("Actor::setActorPath error direction 0x%X", direction);
		}
		nextPoint.x -= pathDirectionLUT2[direction][0];
		nextPoint.y -= pathDirectionLUT2[direction][1];
		addPathListPoint(nextPoint);

#ifdef ACTOR_DEBUG
		addDebugPoint(nextPoint, 0x8a);
#endif
	}

	pathToNode();
	removeNodes();
	nodeToPath();
	removePathPoints();

	for (i = 0; i <= _pathNodeListIndex; i++) {
		actor->addWalkStepPoint(_pathNodeList[i].point);
	}
}

void Actor::pathToNode() {
	Point point1, point2, delta;
	int direction;
	int i;
	Point *point;

	point= &_pathList[_pathListIndex];
	direction = 0;

	_pathNodeListIndex = -1;
	addPathNodeListPoint(*point);

	for (i = _pathListIndex; i > 0; i--) {
		point1 = *point;
		--point;
		point2 = *point;
		if (direction == 0) {
			delta.x = int16Compare(point2.x, point1.x);
			delta.y = int16Compare(point2.y, point1.y);
			direction++;
		}
		if ((point1.x + delta.x != point2.x) || (point1.y + delta.y != point2.y)) {
			addPathNodeListPoint(point1);
			direction--;
			i++;
			point++;
		}
	}
	addPathNodeListPoint(*_pathList);
}

int pathLine(Point *pointList, const Point &point1, const Point &point2) {
	Point point;
	Point delta;
	Point tempPoint;
	Point s;
	int16 errterm;
	int16 res;

	calcDeltaS(point1, point2, delta, s);

	point = point1;

	tempPoint.x = delta.x * 2;
	tempPoint.y = delta.y * 2;

	if (delta.y > delta.x) {

		errterm = tempPoint.x - delta.y;
		res = delta.y;

		while (delta.y > 0) {
			while (errterm >= 0) {
				point.x += s.x;
				errterm -= tempPoint.y;
			}

			point.y += s.y;
			errterm += tempPoint.x;

			*pointList = point;
			pointList++;
			delta.y--;
		}
	} else {

		errterm = tempPoint.y - delta.x;
		res = delta.x;

		while (delta.x > 0) {
			while (errterm >= 0) {
				point.y += s.y;
				errterm -= tempPoint.x;
			}

			point.x += s.x;
			errterm += tempPoint.y;

			*pointList = point;
			pointList++;
			delta.x--;
		}
	}
	return res;
}

void Actor::nodeToPath() {
	int i;
	Point point1, point2;
	PathNode *node;
	Point *point;

	for (i = 0, point = _pathList; i < _pathListAlloced; i++, point++) {
		point->x = point->y = PATH_NODE_EMPTY;
	}

	_pathListIndex = 1;
	_pathList[0] = _pathNodeList[0].point;
	_pathNodeList[0].link = 0;
	for (i = 0, node = _pathNodeList; i < _pathNodeListIndex; i++) {
		point1 = node->point;
		node++;
		point2 = node->point;
		_pathListIndex += pathLine(&_pathList[_pathListIndex], point1, point2);
		node->link = _pathListIndex - 1;
	}
	_pathListIndex--;
	_pathNodeList[_pathNodeListIndex].link = _pathListIndex;

}

void Actor::removeNodes() {
	int i, j, k;
	PathNode *iNode, *jNode, *kNode, *fNode;
	fNode = &_pathNodeList[_pathNodeListIndex];

	if (scanPathLine(_pathNodeList[0].point, fNode->point)) {
		_pathNodeList[1] = *fNode;
		_pathNodeListIndex = 1;
	}

	if (_pathNodeListIndex < 4) {
		return;
	}

	for (i = _pathNodeListIndex - 1, iNode = fNode-1; i > 1 ; i--, iNode--) {
		if (iNode->point.x == PATH_NODE_EMPTY) {
			continue;
		}

		if (scanPathLine(_pathNodeList[0].point, iNode->point)) {
			for (j = 1, jNode = _pathNodeList + 1; j < i; j++, jNode++) {
				jNode->point.x = PATH_NODE_EMPTY;
			}
		}
	}

	for (i = 1, iNode = _pathNodeList + 1; i < _pathNodeListIndex - 1; i++, iNode++) {
		if (iNode->point.x == PATH_NODE_EMPTY) {
			continue;
		}

		if (scanPathLine(fNode->point, iNode->point)) {
			for (j = i + 1, jNode = iNode + 1; j < _pathNodeListIndex; j++, jNode++) {
				jNode->point.x = PATH_NODE_EMPTY;
			}
		}
	}
	condenseNodeList();

	for (i = 1, iNode = _pathNodeList + 1; i < _pathNodeListIndex - 1; i++, iNode++) {
		if (iNode->point.x == PATH_NODE_EMPTY) {
			continue;
		}
		for (j = i + 2, jNode = iNode + 2; j < _pathNodeListIndex; j++, jNode++) {
			if (jNode->point.x == PATH_NODE_EMPTY) {
				continue;
			}

			if (scanPathLine(iNode->point, jNode->point)) {
				for (k = i + 1,kNode = iNode + 1; k < j; k++, kNode++) {
					kNode->point.x = PATH_NODE_EMPTY;
				}
			}
		}
	}
	condenseNodeList();
}

void Actor::condenseNodeList() {
	int i, j, count;
	PathNode *iNode, *jNode;

	count = _pathNodeListIndex;

	for (i = 1, iNode = _pathNodeList + 1; i < _pathNodeListIndex; i++, iNode++) {
		if (iNode->point.x == PATH_NODE_EMPTY) {
			j = i + 1;
			jNode = iNode + 1;
			while (jNode->point.x == PATH_NODE_EMPTY) {
				j++;
				jNode++;
			}
			*iNode = *jNode;
			count = i;
			jNode->point.x = PATH_NODE_EMPTY;
			if (j == _pathNodeListIndex) {
				break;
			}
		}
	}
	_pathNodeListIndex = count;
}

void Actor::removePathPoints() {
	int i, j, k, l;
	PathNode *node;
	int start;
	int end;
	Point point1, point2;

	if (_pathNodeListIndex < 2)
		return;

	_newPathNodeListIndex = -1;
	addNewPathNodeListPoint(_pathNodeList[0]);

	for (i = 1, node = _pathNodeList + 1; i < _pathNodeListIndex; i++, node++) {
		addNewPathNodeListPoint(*node);

		for (j = 5; j > 0; j--) {
			start = node->link - j;
			end = node->link + j;

			if (start < 0 || end > _pathListIndex) {
				continue;
			}

			point1 = _pathList[start];
			point2 = _pathList[end];
			if ((point1.x == PATH_NODE_EMPTY) || (point2.x == PATH_NODE_EMPTY)) {
				continue;
			}

			if (scanPathLine(point1, point2)) {
				for (l = 1; l <= _newPathNodeListIndex; l++) {
					if (start <= _newPathNodeList[l].link) {
						_newPathNodeListIndex = l;
						_newPathNodeList[_newPathNodeListIndex].point = point1;
						_newPathNodeList[_newPathNodeListIndex].link = start;
						incrementNewPathNodeListIndex();
						break;
					}
				}
				_newPathNodeList[_newPathNodeListIndex].point = point2;
				_newPathNodeList[_newPathNodeListIndex].link = end;

				for (k = start + 1; k < end; k++) {
					_pathList[k].x = PATH_NODE_EMPTY;
				}
				break;
			}
		}
	}

	addNewPathNodeListPoint(_pathNodeList[_pathNodeListIndex]);

	for (i = 0, j = 0; i <= _newPathNodeListIndex; i++) {
		if (_newPathNodeListIndex == i || (_newPathNodeList[i].point != _newPathNodeList[i+1].point)) {
			_pathNodeList[j++] = _newPathNodeList[i];
		}
	}
	_pathNodeListIndex = j - 1;
}

#ifdef ACTOR_DEBUG
void Actor::drawPathTest() {
	int i;
	Surface *surface;
	surface = _vm->_gfx->getBackBuffer();
	if (_debugPoints == NULL) {
		return;
	}

	for (i = 0; i < _debugPointsCount; i++) {
		*((byte *)surface->pixels + (_debugPoints[i].point.y * surface->pitch) + _debugPoints[i].point.x) = _debugPoints[i].color;
	}
}
#endif

} // End of namespace Saga
