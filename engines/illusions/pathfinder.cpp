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

#include "illusions/illusions.h"
#include "illusions/pathfinder.h"
#include "camera.h"

namespace Illusions {

PointArray *PathFinder::findPath(Camera *camera, Common::Point sourcePt, Common::Point destPt,
	PointArray *walkPoints, PathLines *walkRects, WidthHeight bgDimensions) {
	Common::Point cameraPt = camera->getScreenOffset();
	_screenRect.p0 = cameraPt;
	_screenRect.p1.x = cameraPt.x + 320; //TODO fix me get screen dimensions here.
	_screenRect.p1.y = cameraPt.y + 200;
	_walkPoints = walkPoints;
	_walkRects = walkRects;
	_bgDimensions = bgDimensions;
	return findPathInternal(sourcePt, destPt);
}

PointArray *PathFinder::findPathInternal(Common::Point sourcePt, Common::Point destPt) {
	PathLine line;
	PointArray *foundPath = new PointArray();
	line.p0 = sourcePt;
	line.p1 = destPt;

	if (_walkRects && _walkPoints && isLineBlocked(line)) {
		Common::Point nextStartPt = sourcePt, outPt;

		if (!findValidDestLine(destPt)) {
			findValidDestPt(destPt);
			line.p1 = destPt;
		}

		_pathBytes = (byte*)calloc(1, _walkPoints->size());

		bool done = false;
		while (!done) {
			line.p0 = nextStartPt;
			if (!isLineBlocked(line)) {
				foundPath->push_back(destPt);
				done = true;
			} else {
				if (foundPath->size() < _walkPoints->size() + 2 && findClosestPt(nextStartPt, outPt, destPt)) {
					foundPath->push_back(outPt);
					nextStartPt = outPt;
				} else {
					if (foundPath->size() == 0)
						foundPath->push_back(sourcePt);
					done = true;
				}
			}
		}

		free(_pathBytes);
		postProcess(sourcePt, foundPath);

	} else {
		foundPath->push_back(destPt);
	}
	return foundPath;
}

void PathFinder::postProcess(Common::Point sourcePt, PointArray *foundPath) {
	// For each three points A, B and C, removes B if the line between A and C is not blocked
	for (uint index = 0; index + 2 < foundPath->size(); ++index) {
		PathLine line;
		line.p0 = index == 0 ? sourcePt : (*foundPath)[index - 1];
		line.p1 = (*foundPath)[index + 1];
		if (!isLineBlocked(line)) {
			debug("remove point");
			foundPath->remove_at(index);
		}
	}
}

bool PathFinder::isLineBlocked(PathLine &line) {
	for (uint i = 0; i < _walkRects->size(); ++i) {
		if (calcLineStatus(line, (*_walkRects)[i], 0) != 3)
			return true;
	}
	return false;
}

int PathFinder::calcLineDistance(PathLine &line) {
	int16 deltaX = line.p0.x - line.p1.x;
	int16 deltaY = line.p0.y - line.p1.y;
	if (deltaX != 0 || deltaY != 0)
		return sqrt(deltaX * deltaX + deltaY * deltaY);
	return 0;
}

bool PathFinder::findClosestPt(Common::Point &sourcePt, Common::Point &closestPt, Common::Point &destPt) {
	PathLine sourceLine, destLine;
	uint minIndex = 0;
	int minDistance = 0xFFFF;
	sourceLine.p0 = sourcePt;
	destLine.p1 = destPt;
	for (uint i = 0; i < _walkPoints->size(); ++i) {
		sourceLine.p1 = (*_walkPoints)[i];
		destLine.p0 = (*_walkPoints)[i];
		if (!_pathBytes[i] && !isLineBlocked(sourceLine)) {
			int currDistance = calcLineDistance(destLine);
			if (currDistance <= minDistance) {
				minDistance = currDistance;
				minIndex = i + 1;
			}
		}
	}
	if (minIndex) {
		closestPt = (*_walkPoints)[minIndex - 1];
		_pathBytes[minIndex - 1] = 1;
		return true;
	}
	return false;
}

bool PathFinder::findValidDestLine(Common::Point &destPt) {
	PathLine destLine;
	destLine.p0 = destPt;
	for (uint i = 0; i < _walkPoints->size(); ++i) {
		destLine.p1 = (*_walkPoints)[i];
		if (!isLineBlocked(destLine))
			return true;
	}
	return false;
}

void PathFinder::findValidDestPt(Common::Point &destPt) {
	Common::Point minPt, outPt, deltaPt;
	int minDistance = 0xFFFF, currDistance;
	PathLine destLine;
	for (uint i = 0; i < _walkRects->size(); ++i) {
		PathLine currRect = (*_walkRects)[i];
		//TODO fix this hack. Used here to get xmas tree scene to work.
		if (currRect.p1.x > _screenRect.p1.x) {
			currRect.p1.x = _screenRect.p1.x;
		}
		if (currRect.p0.x < _screenRect.p0.x) {
			currRect.p0.x = _screenRect.p0.x;
		}
		WidthHeight rectDimensions = calcRectDimensions(currRect);

		adjustRectDimensions(rectDimensions);
		clipLineToBg(destPt, rectDimensions, destLine);
		if (calcLineStatus(destLine, currRect, &outPt) == 3) {
			destLine.p0 = destPt;
			destLine.p1 = currRect.p0;
			currDistance = calcLineDistance(destLine);
			if (currDistance < minDistance) {
				minDistance = currDistance;
				minPt = currRect.p0;
			}
			destLine.p0 = destPt;
			destLine.p1 = currRect.p1;
			currDistance = calcLineDistance(destLine);
			if (currDistance < minDistance) {
				minDistance = currDistance;
				minPt = currRect.p1;
			}
		} else {
			destLine.p0 = destPt;
			destLine.p1 = outPt;
			currDistance = calcLineDistance(destLine);
			if (currDistance < minDistance) {
				minDistance = currDistance;
				minPt = outPt;
			}
		}
	}
	findDeltaPt(minPt, deltaPt);
	destPt.x = deltaPt.x + minPt.x;
	destPt.y = deltaPt.y + minPt.y;
}

WidthHeight PathFinder::calcRectDimensions(PathLine &rect) {
	WidthHeight dimensions;
	dimensions._width = rect.p1.x - rect.p0.x;
	dimensions._height = rect.p1.y - rect.p0.y;
	swapDimensions(dimensions);
	return dimensions;
}

void PathFinder::adjustRectDimensions(WidthHeight &dimensions) {
	dimensions._width = ABS(dimensions._height) * (dimensions._width < 0 ? -1 : 1);
	dimensions._height = ABS(dimensions._width) * (dimensions._height < 0 ? -1 : 1);
	if (dimensions._width)
		dimensions._width = -dimensions._width;
	else
		dimensions._height = -dimensions._height;
	swapDimensions(dimensions);
}

void PathFinder::swapDimensions(WidthHeight &dimensions) {
	if (dimensions._width < 0) {
		dimensions._width = -dimensions._width;
		dimensions._height = -dimensions._height;
	} else if (dimensions._width == 0)
		dimensions._height = abs(dimensions._height);
	else if (dimensions._height == 0)
		dimensions._width = abs(dimensions._width);
}

void PathFinder::clipLineToBg(Common::Point &destPt, WidthHeight &rectDimensions, PathLine &outDestLine) {
	if (rectDimensions._height == 0) {
		outDestLine.p0.x = 0;
		outDestLine.p0.y = destPt.y;
		outDestLine.p1.x = _bgDimensions._width;
		outDestLine.p1.y = destPt.y;
	} else if (rectDimensions._width == 0) {
		outDestLine.p0.y = 0;
		outDestLine.p0.x = destPt.x;
		outDestLine.p1.x = destPt.x;
		outDestLine.p1.y = _bgDimensions._height;
	} else {
		outDestLine.p0 = destPt;
		outDestLine.p1.x = destPt.x + rectDimensions._width;
		outDestLine.p1.y = destPt.y + rectDimensions._height;
		int16 y1 = destPt.y + (rectDimensions._height * -destPt.x / rectDimensions._width);
		int16 y2 = destPt.y + (rectDimensions._height * (_bgDimensions._width - destPt.x) / rectDimensions._width);
		int16 x1 = destPt.x + (rectDimensions._width * -destPt.y / rectDimensions._height);
		int16 x2 = destPt.x + (rectDimensions._width * (_bgDimensions._height - destPt.y) / rectDimensions._height);
		if (ABS(rectDimensions._height) <= ABS(rectDimensions._width)) {
			outDestLine.p0.y = 0;
			outDestLine.p0.x = _bgDimensions._width;
			if (x1 < 0 || _bgDimensions._width < x1)
				outDestLine.p0.y = y2;
			else
				outDestLine.p0.x = x1;
			outDestLine.p1.x = 0;
			outDestLine.p1.y = _bgDimensions._height;
			if (x2 < 0 || _bgDimensions._width < x2)
				outDestLine.p1.y = y1;
			else
				outDestLine.p1.x = x2;
		} else {
			outDestLine.p0.y = 0;
			outDestLine.p0.x = 0;
			if (x1 < 0 || _bgDimensions._width < x1)
				outDestLine.p0.y = y1;
			else
				outDestLine.p0.x = x1;
			outDestLine.p1.x = _bgDimensions._width;
			outDestLine.p1.y = _bgDimensions._height;
			if (x2 < 0 || _bgDimensions._width < x2)
				outDestLine.p1.y = y2;
			else
				outDestLine.p1.x = x2;
		}
	}
}

void PathFinder::findDeltaPt(Common::Point pt, Common::Point &outDeltaPt) {
	static const struct { int16 x, y; } kDeltaPoints[] = {
		{ 0, -4}, {0, 4}, {-4, 0}, { 4,  0}, {-3, -3}, {3, 3}, {-3, 3}, { 3, -3},
		{-2, -4}, {2, 4}, {-2, 4}, { 2, -4}, {-4, -2}, {4, 2}, {-4, 2}, { 4, -2},
		{-1, -4}, {1, 4}, {-1, 4}, { 1, -4}, {-4, -1}, {4, 1}, {-4, 1}, { 4, -1},
		{-2, -3}, {2, 3}, {-2, 3}, { 2, -3}, {-3, -2}, {3, 2}, {-3, 2}, { 3, -2}
	};
	Common::Point testPt;
	for (uint i = 0; i < 32; ++i) {
		testPt.x = pt.x + kDeltaPoints[i].x;
		testPt.y = pt.y + kDeltaPoints[i].y;
		if (findValidDestLine(testPt)) {
			outDeltaPt.x = kDeltaPoints[i].x;
			outDeltaPt.y = kDeltaPoints[i].y;
			break;
		}
	}
}
/**
 * returns true if line is contained within rect.
 */
bool PathFinder::isLineWithinRectangle(PathLine &line, PathLine &rect) {
	return line.p0.x <= rect.p1.x && line.p1.x >= rect.p0.x &&
		line.p0.y <= rect.p1.y && line.p1.y >= rect.p0.y;
}

/**
 * flip line coordinates so it starts top left and finishes bottom right
 */
void PathFinder::swapLine(PathLine &line, PathLine &outLine) {
	if (line.p1.x <= line.p0.x) {
		outLine.p1.x = line.p0.x;
		outLine.p0.x = line.p1.x;
	} else {
		outLine.p0.x = line.p0.x;
		outLine.p1.x = line.p1.x;
	}
	if (line.p1.y <= line.p0.y) {
		outLine.p1.y = line.p0.y;
		outLine.p0.y = line.p1.y;
	} else {
		outLine.p0.y = line.p0.y;
		outLine.p1.y = line.p1.y;
	}
}

int PathFinder::calcLineStatus(PathLine &sourceLine, PathLine &destRect, Common::Point *outPoint) {
	PathLine sourceLine1, destRect1;
	swapLine(sourceLine, sourceLine1);
	swapLine(destRect, destRect1);

	if (!isLineWithinRectangle(sourceLine1, destRect1))
		return 3;

	int sourceDeltaX = sourceLine.p1.x - sourceLine.p0.x;
	int sourceDeltaY = sourceLine.p1.y - sourceLine.p0.y;
	int destDeltaX = destRect.p0.x - destRect.p1.x;
	int destDeltaY = destRect.p0.y - destRect.p1.y;
	int sdDeltaX = sourceLine.p0.x - destRect.p0.x;
	int sdDeltaY = sourceLine.p0.y - destRect.p0.y;
	int delta1 = destDeltaY * sdDeltaX - destDeltaX * sdDeltaY;
	int delta2 = sourceDeltaY * destDeltaX - sourceDeltaX * destDeltaY;
	int delta3 = sourceDeltaX * sdDeltaY - sourceDeltaY * sdDeltaX;

	if ((delta2 <= 0 && (delta1 > 0 || delta2 > delta1)) ||
		(delta2 > 0 && (delta1 < 0 || delta2 < delta1)) ||
		(delta2 <= 0 && (delta3 > 0 || delta2 > delta3)) ||
		(delta2 > 0 && (delta3 < 0 || delta2 < delta3)))
		return 3;

	if (!outPoint)
		return 1;

	if (delta2 == 0)
		return 2;

	int v15 = sourceDeltaX * delta1, v18 = sourceDeltaY * delta1;
	int v16 = 0;
	int v17 = 0;

	if ((v15 >= 0 && delta2 >= 0) || (v15 < 0 && delta2 < 0)) {
		v16 = delta2 / 2;
		v17 = delta2 / 2;
	} else if ((v15 < 0 && delta2 >= 0) || (v15 >= 0 && delta2 < 0)) {
		v17 = delta2 / 2;
		v16 = delta2 / -2;
	}

	outPoint->x = sourceLine.p0.x + (v15 + v16) / delta2;

	if ((v18 >= 0 && delta2 < 0) || (v18 < 0 && delta2 >= 0))
		v17 = -v17;

	outPoint->y = sourceLine.p0.y + (v18 + v17) / delta2;

	return 1;
}

} // End of namespace Illusions
