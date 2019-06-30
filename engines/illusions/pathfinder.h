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

#ifndef ILLUSIONS_PATHFINDER_H
#define ILLUSIONS_PATHFINDER_H

#include "illusions/graphics.h"
#include "common/array.h"
#include "common/list.h"
#include "common/rect.h"

namespace Illusions {

struct PathLine {
	Common::Point p0, p1;
};

typedef Common::Array<PathLine> PathLines;
typedef Common::Array<Common::Point> PointArray;

class PathFinder {
public:
	PointArray *findPath(Camera *camera, Common::Point sourcePt, Common::Point destPt,
		PointArray *walkPoints, PathLines *walkRects, WidthHeight bgDimensions);
protected:
	PathLine _screenRect;
	PointArray *_walkPoints;
	PathLines *_walkRects;
	WidthHeight _bgDimensions;
	byte *_pathBytes;
	PointArray *findPathInternal(Common::Point sourcePt, Common::Point destPt);
	void postProcess(Common::Point sourcePt, PointArray *foundPath);
	bool isLineBlocked(PathLine &line);
	int calcLineDistance(PathLine &line);
	bool findClosestPt(Common::Point &sourcePt, Common::Point &closestPt, Common::Point &destPt);
	bool findValidDestLine(Common::Point &destPt);
	void findValidDestPt(Common::Point &destPt);
	WidthHeight calcRectDimensions(PathLine &rect);
	void adjustRectDimensions(WidthHeight &dimensions);
	void swapDimensions(WidthHeight &dimensions);
	void clipLineToBg(Common::Point &destPt, WidthHeight &rectDimensions, PathLine &outDestLine);
	void findDeltaPt(Common::Point pt, Common::Point &outDeltaPt);
	bool isLineWithinRectangle(PathLine &line, PathLine &rect);
	void swapLine(PathLine &line, PathLine &outLine);
	int calcLineStatus(PathLine &sourceLine, PathLine &destRect, Common::Point *outPoint);
};

} // End of namespace Illusions

#endif // ILLUSIONS_PATHFINDER_H
