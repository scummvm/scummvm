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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SLUDGE_FLOOR_H
#define SLUDGE_FLOOR_H

namespace Sludge {

struct floorPolygon {
	int numVertices;
	int *vertexID;
};

struct POINT {
	int x;
	int y;
};

struct flor {
	int originalNum;
	POINT *vertex;
	int numPolygons;
	floorPolygon *polygon;
	int * *matrix;
};

bool initFloor();
void setFloorNull();
bool setFloor(int fileNum);
void drawFloor();
int inFloor(int x, int y);
bool getMatchingCorners(floorPolygon &, floorPolygon &, int &, int &);
bool closestPointOnLine(int &closestX, int &closestY, int x1, int y1, int x2, int y2, int xP, int yP);

} // End of namespace Sludge

#endif
