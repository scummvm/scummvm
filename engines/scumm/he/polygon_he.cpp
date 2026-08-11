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

#ifdef ENABLE_HE

#include "common/system.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/wiz_he.h"

namespace Scumm {

void Wiz::deleteLocalPolygons() {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if (_polygons[i].flag == 1)
			_polygons[i].reset();
	}
}

void Wiz::polygonLoad(const uint8 *polData) {
	int slots = READ_LE_UINT32(polData);
	polData += 4;

	bool flag = 1;
	int id, points, vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y;
	while (slots--) {
		id = READ_LE_UINT32(polData);
		points = READ_LE_UINT32(polData + 4);
		if (points != 4)
			error("Illegal polygon with %d points", points);
		vert1x = READ_LE_UINT32(polData + 8);
		vert1y = READ_LE_UINT32(polData + 12);
		vert2x = READ_LE_UINT32(polData + 16);
		vert2y = READ_LE_UINT32(polData + 20);
		vert3x = READ_LE_UINT32(polData + 24);
		vert3y = READ_LE_UINT32(polData + 28);
		vert4x = READ_LE_UINT32(polData + 32);
		vert4y = READ_LE_UINT32(polData + 36);

		polData += 40;
		set4Polygon(id, flag, vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y);
	}
}

void Wiz::set4Polygon(int id, bool localFlag, int vert1x, int vert1y, int vert2x, int vert2y, int vert3x, int vert3y, int vert4x, int vert4y) {
	for (int i = 0; i < ARRAYSIZE(_polygons); ++i) {
		if (_polygons[i].id == 0) {
			_polygons[i].points[0].x = vert1x;
			_polygons[i].points[0].y = vert1y;
			_polygons[i].points[1].x = vert2x;
			_polygons[i].points[1].y = vert2y;
			_polygons[i].points[2].x = vert3x;
			_polygons[i].points[2].y = vert3y;
			_polygons[i].points[3].x = vert4x;
			_polygons[i].points[3].y = vert4y;
			_polygons[i].points[4].x = vert1x;
			_polygons[i].points[4].y = vert1y;
			_polygons[i].id = id;
			_polygons[i].numPoints = 5;
			_polygons[i].flag = localFlag;

			polyBuildBoundingRect(_polygons[i].points, _polygons[i].numPoints, _polygons[i].boundingRect);

			return;
		}
	}

	error("Wiz::set4Polygon: out of polygon slot, max = %d", ARRAYSIZE(_polygons));
}

void Wiz::polyRotatePoints(Common::Point *pts, int num, int angle) {
	double alpha = ((double)angle) * M_PI / 180.;
	double cos_alpha = cos(alpha);
	double sin_alpha = sin(alpha);

	for (int i = 0; i < num; ++i) {
		int16 x = pts[i].x;
		int16 y = pts[i].y;
		pts[i].x = (int16)(((double)x * cos_alpha) - ((double)y * sin_alpha));
		pts[i].y = (int16)(((double)y * cos_alpha) + ((double)x * sin_alpha));
	}
}

void Wiz::polyMovePolygonPoints(Common::Point *listOfPoints, int numverts, int deltaX, int deltaY) {
	for (int i = 0; i < numverts; i++) {
		listOfPoints->x += deltaX;
		listOfPoints->y += deltaY;
		listOfPoints++;
	}
}

bool Wiz::polyIsRectangle(const Common::Point *points, int numverts) {
	// assume it's not
	bool bRetVal = false;

	// if there aren't four points, it's not a rectangle, duh!
	if (numverts == 4) {
		// check if point 0 is on same x and y axis as neighbors
		if (points[0].x == points[1].x) {
			// point 3 must be on y axis to form a right angle
			if (points[0].y == points[3].y) {
				// check opposite point to make sure that its neighbors form a right angle
				if ((points[2].x == points[3].x) && (points[2].y == points[1].y)) {
					bRetVal = true;
				}
			}
		} else if (points[0].x == points[3].x) { // the second of two possible orientations
			// point 1 must be on y axis to form right angle
			if (points[0].y == points[1].y) {
				// check opposite point to make sure that its neighbors form a right angle
				if ((points[2].x == points[1].x) && (points[2].y == points[3].y)) {
					bRetVal = true;
				}
			}
		}
	}

	return bRetVal;
}

void Wiz::polyBuildBoundingRect(Common::Point *points, int numVerts, Common::Rect &rect) {
	rect.left = 10000;
	rect.top = 10000;
	rect.right = -10000;
	rect.bottom = -10000;

	// compute bounding box
	for (int j = 0; j < numVerts; j++) {
		if (points[j].x < rect.left) rect.left = points[j].x;
		if (points[j].y < rect.top) rect.top = points[j].y;
		if (points[j].x > rect.right) rect.right = points[j].x;
		if (points[j].y > rect.bottom) rect.bottom = points[j].y;
	}
}

void Wiz::deletePolygon(int fromId, int toId) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if (_polygons[i].id >= fromId && _polygons[i].id <= toId)
			_polygons[i].reset();
	}
}

int Wiz::findPolygon(int x, int y) {
	Common::Point checkPoint((int16)x, (int16)y);

	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if (isPointInRect(&_polygons[i].boundingRect, &checkPoint)) {
			if (polyIsPointInsidePoly(_polygons[i].points, _polygons[i].numPoints, &checkPoint)) {
				return _polygons[i].id;
			}
		}
	}
	return 0;
}

int Wiz::testForObjectPolygon(int object, int x, int y) {
	Common::Point checkPoint((int16)x, (int16)y);

	if (object == 0)
		return 0;

	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if (_polygons[i].id == object) {
			if (isPointInRect(&_polygons[i].boundingRect, &checkPoint)) {
				if (polyIsPointInsidePoly(_polygons[i].points, _polygons[i].numPoints, &checkPoint)) {
					return 1;
				}
			}
		}
	}

	return 0;
}

bool Wiz::doesObjectHavePolygon(int object) {
	if (object == 0)
		return false;

	for (int i = 0; i < ARRAYSIZE(_polygons); i++)
		if (_polygons[i].id == object)
			return true;
	return false;
}

bool Wiz::polyIsPointInsidePoly(Common::Point *listOfPoints, int numverts, Common::Point *checkPoint) {
	int ty, tx, yflag0, yflag1;
	bool insideFlag = false;
	Common::Point *vtx0, *vtx1;

	tx = checkPoint->x;
	ty = checkPoint->y;
	vtx0 = listOfPoints + (numverts - 1);
	yflag0 = (vtx0->y >= ty);
	vtx1 = listOfPoints;

	for (int j = numverts + 1; --j;) {
		yflag1 = (vtx1->y >= ty);

		if (yflag0 != yflag1) {
			if (((vtx1->y - ty) * (vtx0->x - vtx1->x) >=
				 (vtx1->x - tx) * (vtx0->y - vtx1->y)) == yflag0) {
				insideFlag = !insideFlag;
			}
		}

		yflag0 = yflag1;
		vtx0 = vtx1;
		vtx1++;
	}

	// Special case the vert and horz edges.
	if (_vm->_game.heversion >= 80) {
		if (!insideFlag) {
			int a, b, minX, maxX, minY, maxY;

			for (int j = 0; j < numverts; j++) {
				a = j % numverts;
				b = (j + 1) % numverts;

				if ((ty == listOfPoints[a].y) && (listOfPoints[a].y == listOfPoints[b].y)) {

					minX = MIN(listOfPoints[a].x, listOfPoints[b].x);
					maxX = MAX(listOfPoints[a].x, listOfPoints[b].x);

					if ((tx >= minX) && (tx <= maxX)) {
						return true;
					}

				} else if ((tx == listOfPoints[a].x) && (listOfPoints[a].x == listOfPoints[b].x)) {
					minY = MIN(listOfPoints[a].y, listOfPoints[b].y);
					maxY = MAX(listOfPoints[a].y, listOfPoints[b].y);

					if ((ty >= minY) && (ty <= maxY)) {
						return true;
					}
				}
			}
		}
	}

	return insideFlag;
}

} // End of namespace Scumm

#endif // ENABLE_HE
