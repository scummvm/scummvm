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

#include "graphics/surface.h"

#include "sludge/allfiles.h"
#include "sludge/fileset.h"
#include "sludge/floor.h"
#include "sludge/graphics.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"

namespace Sludge {

Floor *currentFloor = NULL;

bool pointInFloorPolygon(FloorPolygon &floorPoly, int x, int y) {
	int i = 0, j, c = 0;
	float xp_i, yp_i;
	float xp_j, yp_j;

	for (j = floorPoly.numVertices - 1; i < floorPoly.numVertices; j = i++) {

		xp_i = currentFloor->vertex[floorPoly.vertexID[i]].x;
		yp_i = currentFloor->vertex[floorPoly.vertexID[i]].y;
		xp_j = currentFloor->vertex[floorPoly.vertexID[j]].x;
		yp_j = currentFloor->vertex[floorPoly.vertexID[j]].y;

		if ((((yp_i <= y) && (y < yp_j)) || ((yp_j <= y) && (y < yp_i))) && (x < (xp_j - xp_i) * (y - yp_i) / (yp_j - yp_i) + xp_i)) {
			c = !c;
		}
	}
	return c;
}

bool getMatchingCorners(FloorPolygon &a, FloorPolygon &b, int &cornerA, int &cornerB) {
	int sharedVertices = 0;
	int i, j;

	for (i = 0; i < a.numVertices; i++) {
		for (j = 0; j < b.numVertices; j++) {
			if (a.vertexID[i] == b.vertexID[j]) {
				if (sharedVertices++) {
					cornerB = a.vertexID[i];
					return true;
				} else {
					cornerA = a.vertexID[i];
				}
			}
		}
	}

	return false;
}

bool polysShareSide(FloorPolygon &a, FloorPolygon &b) {
	int sharedVertices = 0;
	int i, j;

	for (i = 0; i < a.numVertices; i++) {
		for (j = 0; j < b.numVertices; j++) {
			if (a.vertexID[i] == b.vertexID[j]) {
				if (sharedVertices++)
					return true;
			}
		}
	}

	return false;
}

void noFloor() {
	currentFloor->numPolygons = 0;
	currentFloor->polygon = NULL;
	currentFloor->vertex = NULL;
	currentFloor->matrix = NULL;
}

bool initFloor() {
	currentFloor = new Floor;
	if (!checkNew(currentFloor))
		return false;
	noFloor();
	return true;
}

void killFloor() {
	if (currentFloor) {
		for (int i = 0; i < currentFloor->numPolygons; i++) {
			delete []currentFloor->polygon[i].vertexID;
			delete []currentFloor->matrix[i];
		}
		delete []currentFloor->polygon;
		currentFloor->polygon = NULL;
		delete []currentFloor->vertex;
		currentFloor->vertex = NULL;
		delete []currentFloor->matrix;
		currentFloor->matrix = NULL;
	}
}

void setFloorNull() {
	killFloor();
	noFloor();
}

bool setFloor(int fileNum) {

	int i, j;

	killFloor();

	setResourceForFatal(fileNum);

	if (!g_sludge->_resMan->openFileFromNum(fileNum))
		return false;

	// Find out how many polygons there are and reserve memory

	currentFloor->originalNum = fileNum;
	currentFloor->numPolygons = g_sludge->_resMan->getData()->readByte();
	currentFloor->polygon = new FloorPolygon[currentFloor->numPolygons];
	if (!checkNew(currentFloor->polygon))
		return false;

	// Read in each polygon

	for (i = 0; i < currentFloor->numPolygons; i++) {

		// Find out how many vertex IDs there are and reserve memory

		currentFloor->polygon[i].numVertices = g_sludge->_resMan->getData()->readByte();
		currentFloor->polygon[i].vertexID = new int[currentFloor->polygon[i].numVertices];
		if (!checkNew(currentFloor->polygon[i].vertexID))
			return false;

		// Read in each vertex ID

		for (j = 0; j < currentFloor->polygon[i].numVertices; j++) {
			currentFloor->polygon[i].vertexID[j] = g_sludge->_resMan->getData()->readUint16BE();
		}
	}

	// Find out how many vertices there are and reserve memory

	i = g_sludge->_resMan->getData()->readUint16BE();
	currentFloor->vertex = new Common::Point[i];
	if (!checkNew(currentFloor->vertex))
		return false;

	for (j = 0; j < i; j++) {

		currentFloor->vertex[j].x = g_sludge->_resMan->getData()->readUint16BE();
		currentFloor->vertex[j].y = g_sludge->_resMan->getData()->readUint16BE();
	}

	g_sludge->_resMan->finishAccess();

	// Now build the movement martix

	currentFloor->matrix = new int *[currentFloor->numPolygons];
	int **distanceMatrix = new int *[currentFloor->numPolygons];

	if (!checkNew(currentFloor->matrix))
		return false;

	for (i = 0; i < currentFloor->numPolygons; i++) {
		currentFloor->matrix[i] = new int[currentFloor->numPolygons];
		distanceMatrix[i] = new int[currentFloor->numPolygons];
		if (!checkNew(currentFloor->matrix[i]))
			return false;
		for (j = 0; j < currentFloor->numPolygons; j++) {
			currentFloor->matrix[i][j] = -1;
			distanceMatrix[i][j] = 10000;
		}
	}

	for (i = 0; i < currentFloor->numPolygons; i++) {
		for (j = 0; j < currentFloor->numPolygons; j++) {
			if (i != j) {
				if (polysShareSide(currentFloor->polygon[i], currentFloor->polygon[j])) {
					currentFloor->matrix[i][j] = j;
					distanceMatrix[i][j] = 1;
				}
			} else {
				currentFloor->matrix[i][j] = -2;
				distanceMatrix[i][j] = 0;
			}
		}
	}

	bool madeChange;
	int lookForDistance = 0;

	do {
		lookForDistance++;
//		debugMatrix ();
		madeChange = false;
		for (i = 0; i < currentFloor->numPolygons; i++) {
			for (j = 0; j < currentFloor->numPolygons; j++) {
				if (currentFloor->matrix[i][j] == -1) {

					// OK, so we don't know how to get from i to j...
					for (int d = 0; d < currentFloor->numPolygons; d++) {
						if (d != i && d != j) {
							if (currentFloor->matrix[i][d] == d && currentFloor->matrix[d][j] >= 0 && distanceMatrix[d][j] <= lookForDistance) {
								currentFloor->matrix[i][j] = d;
								distanceMatrix[i][j] = lookForDistance + 1;
								madeChange = true;
							}
						}
					}
				}
			}
		}
	} while (madeChange);

	for (i = 0; i < currentFloor->numPolygons; i++) {
		delete[] distanceMatrix[i];
	}

	delete []distanceMatrix;
	distanceMatrix = NULL;

	setResourceForFatal(-1);

	return true;
}

void drawFloor() {
	int i, j, nV;
	for (i = 0; i < currentFloor->numPolygons; i++) {
		nV = currentFloor->polygon[i].numVertices;
		if (nV > 1) {
			for (j = 1; j < nV; j++) {
				g_sludge->_gfxMan->drawLine(currentFloor->vertex[currentFloor->polygon[i].vertexID[j - 1]].x, currentFloor->vertex[currentFloor->polygon[i].vertexID[j - 1]].y,
						currentFloor->vertex[currentFloor->polygon[i].vertexID[j]].x, currentFloor->vertex[currentFloor->polygon[i].vertexID[j]].y);
			}
			g_sludge->_gfxMan->drawLine(currentFloor->vertex[currentFloor->polygon[i].vertexID[0]].x, currentFloor->vertex[currentFloor->polygon[i].vertexID[0]].y,
					currentFloor->vertex[currentFloor->polygon[i].vertexID[nV - 1]].x, currentFloor->vertex[currentFloor->polygon[i].vertexID[nV - 1]].y);
		}
	}
}

int inFloor(int x, int y) {
	int i, r = -1;

	for (i = 0; i < currentFloor->numPolygons; i++)
		if (pointInFloorPolygon(currentFloor->polygon[i], x, y))
			r = i;

	return r;
}

bool closestPointOnLine(int &closestX, int &closestY, int x1, int y1, int x2, int y2, int xP, int yP) {
	int xDiff = x2 - x1;
	int yDiff = y2 - y1;

	double m = xDiff * (xP - x1) + yDiff * (yP - y1);
	m /= (xDiff * xDiff) + (yDiff * yDiff);

	if (m < 0) {
		closestX = x1;
		closestY = y1;
	} else if (m > 1) {
		closestX = x2;
		closestY = y2;
	} else {
		closestX = x1 + m * xDiff;
		closestY = y1 + m * yDiff;
		return true;
	}
	return false;
}

} // End of namespace Sludge
