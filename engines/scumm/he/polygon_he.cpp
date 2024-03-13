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

#include "common/math.h"
#include "common/system.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/wiz_he.h"

namespace Scumm {

void Wiz::polygonClear() {
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
		polygonStore(id, flag, vert1x, vert1y, vert2x, vert2y, vert3x, vert3y, vert4x, vert4y);
	}
}

void Wiz::polygonStore(int id, bool localFlag, int vert1x, int vert1y, int vert2x, int vert2y, int vert3x, int vert3y, int vert4x, int vert4y) {
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

	error("Wiz::polygonStore: out of polygon slot, max = %d", ARRAYSIZE(_polygons));
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

void Wiz::polygonTransform(int resNum, int state, int po_x, int po_y, int angle, int scale, Common::Point *pts) {
	int32 w, h;

	getWizImageDim(resNum, state, w, h);

	// set the transformation origin to the center of the image
	if (_vm->_game.heversion >= 99) {
		pts[0].x = pts[3].x = -(w / 2);
		pts[1].x = pts[2].x = w / 2 - 1;
		pts[0].y = pts[1].y = -(h / 2);
		pts[2].y = pts[3].y = h / 2 - 1;
	} else {
		pts[1].x = pts[2].x = w / 2 - 1;
		pts[0].x = pts[0].y = pts[1].y = pts[3].x = -(w / 2);
		pts[2].y = pts[3].y = h / 2 - 1;
	}

	// scale
	if (scale != 0 && scale != 256) {
		for (int i = 0; i < 4; ++i) {
			pts[i].x = pts[i].x * scale / 256;
			pts[i].y = pts[i].y * scale / 256;
		}
	}

	// rotate
	if (angle != 0)
		polyRotatePoints(pts, 4, angle);

	// translate
	for (int i = 0; i < 4; ++i) {
		pts[i].x += po_x;
		pts[i].y += po_y;
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

void Wiz::polygonErase(int fromId, int toId) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if (_polygons[i].id >= fromId && _polygons[i].id <= toId)
			_polygons[i].reset();
	}
}

int Wiz::polygonTestForObjectHit(int id, int x, int y) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++) {
		if ((id == 0 || _polygons[i].id == id) && _polygons[i].boundingRect.contains(x, y)) {
			if (polyIsPointInsidePoly(_polygons[i], x, y)) {
				return _polygons[i].id;
			}
		}
	}

	return 0;
}

bool Wiz::polygonDefined(int id) {
	for (int i = 0; i < ARRAYSIZE(_polygons); i++)
		if (_polygons[i].id == id)
			return true;
	return false;
}

bool Wiz::polyIsPointInsidePoly(const WizPolygon &pol, int x, int y) {
	int pi = pol.numPoints - 1;
	bool diry = (y < pol.points[pi].y);
	bool curdir;
	bool r = false;

	for (int i = 0; i < pol.numPoints; i++) {
		curdir = (y < pol.points[i].y);

		if (curdir != diry) {
			if (((pol.points[pi].y - pol.points[i].y) * (pol.points[i].x - x) <
				 (pol.points[pi].x - pol.points[i].x) * (pol.points[i].y - y)) == diry)
				r = !r;
		}

		pi = i;
		diry = curdir;
	}

	if (_vm->_game.heversion >= 80) {
		int a, b;
		pi = pol.numPoints - 1;
		if (r == 0) {
			for (int i = 0; i < pol.numPoints; i++) {
				if (pol.points[i].y == y && pol.points[i].y == pol.points[pi].y) {

					a = pol.points[i].x;
					b = pol.points[pi].x;

					if (pol.points[i].x >= pol.points[pi].x)
						a = pol.points[pi].x;

					if (pol.points[i].x > pol.points[pi].x)
						b = pol.points[i].x;

					if (x >= a && x <= b)
						return 1;

				} else if (pol.points[i].x == x && pol.points[i].x == pol.points[pi].x) {

					a = pol.points[i].y;
					b = pol.points[i].y;

					if (pol.points[i].y >= pol.points[pi].y)
						a = pol.points[pi].y;

					if (pol.points[i].y <= pol.points[pi].y)
						b = pol.points[pi].y;

					if (y >= a && y <= b)
						return 1;
				}
				pi = i;
			}
		}
	}

	return r;
}

} // End of namespace Scumm

#endif // ENABLE_HE
