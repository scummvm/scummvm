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
#include "sludge/people.h"
#include "sludge/sludge.h"

#define ANGLEFIX (180.0 / 3.14157)

namespace Sludge {

FloorManager::FloorManager(SludgeEngine *vm) {
	_vm = vm;
	_currentFloor = nullptr;
}

FloorManager::~FloorManager() {
	kill();
}

bool FloorManager::pointInFloorPolygon(FloorPolygon &floorPoly, int x, int y) {
	int i = 0, j, c = 0;
	float xp_i, yp_i;
	float xp_j, yp_j;

	for (j = floorPoly.numVertices - 1; i < floorPoly.numVertices; j = i++) {

		xp_i = _currentFloor->vertex[floorPoly.vertexID[i]].x;
		yp_i = _currentFloor->vertex[floorPoly.vertexID[i]].y;
		xp_j = _currentFloor->vertex[floorPoly.vertexID[j]].x;
		yp_j = _currentFloor->vertex[floorPoly.vertexID[j]].y;

		if ((((yp_i <= y) && (y < yp_j)) || ((yp_j <= y) && (y < yp_i))) && (x < (xp_j - xp_i) * (y - yp_i) / (yp_j - yp_i) + xp_i)) {
			c = !c;
		}
	}
	return c;
}

bool FloorManager::getMatchingCorners(FloorPolygon &a, FloorPolygon &b, int &cornerA, int &cornerB) {
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

bool FloorManager::polysShareSide(FloorPolygon &a, FloorPolygon &b) {
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

bool FloorManager::init() {
	_currentFloor = new Floor;
	if (!checkNew(_currentFloor))
		return false;
	_currentFloor->numPolygons = 0;
	_currentFloor->polygon = nullptr;
	_currentFloor->vertex = nullptr;
	_currentFloor->matrix = nullptr;
	return true;
}

void FloorManager::setFloorNull() {
	if (_currentFloor) {
		for (int i = 0; i < _currentFloor->numPolygons; i++) {
			delete[] _currentFloor->polygon[i].vertexID;
			delete[] _currentFloor->matrix[i];
		}
		_currentFloor->numPolygons = 0;
		delete[] _currentFloor->polygon;
		_currentFloor->polygon = nullptr;
		delete[] _currentFloor->vertex;
		_currentFloor->vertex = nullptr;
		delete[] _currentFloor->matrix;
		_currentFloor->matrix = nullptr;
	}
}

void FloorManager::kill() {
	setFloorNull();
	if (_currentFloor) {
		delete _currentFloor;
		_currentFloor = nullptr;
	}
}

bool FloorManager::setFloor(int fileNum) {

	int i, j;

	setFloorNull();

	setResourceForFatal(fileNum);

	if (!g_sludge->_resMan->openFileFromNum(fileNum))
		return false;

	// Find out how many polygons there are and reserve memory

	_currentFloor->originalNum = fileNum;
	_currentFloor->numPolygons = g_sludge->_resMan->getData()->readByte();
	_currentFloor->polygon = new FloorPolygon[_currentFloor->numPolygons];
	if (!checkNew(_currentFloor->polygon))
		return false;

	// Read in each polygon

	for (i = 0; i < _currentFloor->numPolygons; i++) {

		// Find out how many vertex IDs there are and reserve memory

		_currentFloor->polygon[i].numVertices = g_sludge->_resMan->getData()->readByte();
		_currentFloor->polygon[i].vertexID = new int[_currentFloor->polygon[i].numVertices];
		if (!checkNew(_currentFloor->polygon[i].vertexID))
			return false;

		// Read in each vertex ID

		for (j = 0; j < _currentFloor->polygon[i].numVertices; j++) {
			_currentFloor->polygon[i].vertexID[j] = g_sludge->_resMan->getData()->readUint16BE();
		}
	}

	// Find out how many vertices there are and reserve memory

	i = g_sludge->_resMan->getData()->readUint16BE();
	_currentFloor->vertex = new Common::Point[i];
	if (!checkNew(_currentFloor->vertex))
		return false;

	for (j = 0; j < i; j++) {

		_currentFloor->vertex[j].x = g_sludge->_resMan->getData()->readUint16BE();
		_currentFloor->vertex[j].y = g_sludge->_resMan->getData()->readUint16BE();
	}

	g_sludge->_resMan->finishAccess();

	// Now build the movement martix

	_currentFloor->matrix = new int *[_currentFloor->numPolygons];
	int **distanceMatrix = new int *[_currentFloor->numPolygons];

	if (!checkNew(_currentFloor->matrix))
		return false;

	for (i = 0; i < _currentFloor->numPolygons; i++) {
		_currentFloor->matrix[i] = new int[_currentFloor->numPolygons];
		distanceMatrix[i] = new int[_currentFloor->numPolygons];
		if (!checkNew(_currentFloor->matrix[i]))
			return false;
		for (j = 0; j < _currentFloor->numPolygons; j++) {
			_currentFloor->matrix[i][j] = -1;
			distanceMatrix[i][j] = 10000;
		}
	}

	for (i = 0; i < _currentFloor->numPolygons; i++) {
		for (j = 0; j < _currentFloor->numPolygons; j++) {
			if (i != j) {
				if (polysShareSide(_currentFloor->polygon[i], _currentFloor->polygon[j])) {
					_currentFloor->matrix[i][j] = j;
					distanceMatrix[i][j] = 1;
				}
			} else {
				_currentFloor->matrix[i][j] = -2;
				distanceMatrix[i][j] = 0;
			}
		}
	}

	bool madeChange;
	int lookForDistance = 0;

	do {
		lookForDistance++;
		madeChange = false;
		for (i = 0; i < _currentFloor->numPolygons; i++) {
			for (j = 0; j < _currentFloor->numPolygons; j++) {
				if (_currentFloor->matrix[i][j] == -1) {

					// OK, so we don't know how to get from i to j...
					for (int d = 0; d < _currentFloor->numPolygons; d++) {
						if (d != i && d != j) {
							if (_currentFloor->matrix[i][d] == d && _currentFloor->matrix[d][j] >= 0 && distanceMatrix[d][j] <= lookForDistance) {
								_currentFloor->matrix[i][j] = d;
								distanceMatrix[i][j] = lookForDistance + 1;
								madeChange = true;
							}
						}
					}
				}
			}
		}
	} while (madeChange);

	for (i = 0; i < _currentFloor->numPolygons; i++) {
		delete[] distanceMatrix[i];
	}

	delete []distanceMatrix;
	distanceMatrix = nullptr;

	setResourceForFatal(-1);

	return true;
}

void FloorManager::drawFloor() {
	int i, j, nV;
	for (i = 0; i < _currentFloor->numPolygons; i++) {
		nV = _currentFloor->polygon[i].numVertices;
		if (nV > 1) {
			for (j = 1; j < nV; j++) {
				g_sludge->_gfxMan->drawLine(_currentFloor->vertex[_currentFloor->polygon[i].vertexID[j - 1]].x, _currentFloor->vertex[_currentFloor->polygon[i].vertexID[j - 1]].y,
						_currentFloor->vertex[_currentFloor->polygon[i].vertexID[j]].x, _currentFloor->vertex[_currentFloor->polygon[i].vertexID[j]].y);
			}
			g_sludge->_gfxMan->drawLine(_currentFloor->vertex[_currentFloor->polygon[i].vertexID[0]].x, _currentFloor->vertex[_currentFloor->polygon[i].vertexID[0]].y,
					_currentFloor->vertex[_currentFloor->polygon[i].vertexID[nV - 1]].x, _currentFloor->vertex[_currentFloor->polygon[i].vertexID[nV - 1]].y);
		}
	}
}

int FloorManager::inFloor(int x, int y) {
	int i, r = -1;

	for (i = 0; i < _currentFloor->numPolygons; i++)
		if (pointInFloorPolygon(_currentFloor->polygon[i], x, y))
			r = i;

	return r;
}

bool FloorManager::closestPointOnLine(int &closestX, int &closestY, int x1, int y1, int x2, int y2, int xP, int yP) {
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

bool FloorManager::handleClosestPoint(int &setX, int &setY, int &setPoly) {
	int gotX = 320, gotY = 200, gotPoly = -1, i, j, xTest1, yTest1, xTest2, yTest2, closestX, closestY, oldJ, currentDistance = 0xFFFFF, thisDistance;

	for (i = 0; i < _currentFloor->numPolygons; i++) {
		oldJ = _currentFloor->polygon[i].numVertices - 1;
		for (j = 0; j < _currentFloor->polygon[i].numVertices; j++) {
			xTest1 = _currentFloor->vertex[_currentFloor->polygon[i].vertexID[j]].x;
			yTest1 = _currentFloor->vertex[_currentFloor->polygon[i].vertexID[j]].y;
			xTest2 = _currentFloor->vertex[_currentFloor->polygon[i].vertexID[oldJ]].x;
			yTest2 = _currentFloor->vertex[_currentFloor->polygon[i].vertexID[oldJ]].y;
			closestPointOnLine(closestX, closestY, xTest1, yTest1, xTest2, yTest2, setX, setY);
			xTest1 = setX - closestX;
			yTest1 = setY - closestY;
			thisDistance = xTest1 * xTest1 + yTest1 * yTest1;

			if (thisDistance < currentDistance) {
				currentDistance = thisDistance;
				gotX = closestX;
				gotY = closestY;
				gotPoly = i;
			}
			oldJ = j;
		}
	}

	if (gotPoly == -1)
		return false;
	setX = gotX;
	setY = gotY;
	setPoly = gotPoly;
	return true;
}

bool FloorManager::doBorderStuff(OnScreenPerson *moveMe) {
	if (moveMe->inPoly == moveMe->walkToPoly) {
		moveMe->inPoly = -1;
		moveMe->thisStepX = moveMe->walkToX;
		moveMe->thisStepY = moveMe->walkToY;
	} else {
		// The section in which we need to be next...
		int newPoly = _currentFloor->matrix[moveMe->inPoly][moveMe->walkToPoly];
		if (newPoly == -1)
			return false;

		// Grab the index of the second matching corner...
		int ID, ID2;
		if (!getMatchingCorners(_currentFloor->polygon[moveMe->inPoly], _currentFloor->polygon[newPoly], ID, ID2))
			return fatal("Not a valid floor plan!");

		// Remember that we're walking to the new polygon...
		moveMe->inPoly = newPoly;

		// Calculate the destination position on the coincidantal line...
		int x1 = moveMe->x, y1 = moveMe->y;
		int x2 = moveMe->walkToX, y2 = moveMe->walkToY;
		int x3 = _currentFloor->vertex[ID].x, y3 = _currentFloor->vertex[ID].y;
		int x4 = _currentFloor->vertex[ID2].x, y4 = _currentFloor->vertex[ID2].y;

		int xAB = x1 - x2;
		int yAB = y1 - y2;
		int xCD = x4 - x3;
		int yCD = y4 - y3;

		double m = (yAB * (x3 - x1) - xAB * (y3 - y1));
		m /= ((xAB * yCD) - (yAB * xCD));

		if (m > 0 && m < 1) {
			moveMe->thisStepX = x3 + m * xCD;
			moveMe->thisStepY = y3 + m * yCD;
		} else {
			int dx13 = x1 - x3, dx14 = x1 - x4, dx23 = x2 - x3, dx24 = x2 - x4;
			int dy13 = y1 - y3, dy14 = y1 - y4, dy23 = y2 - y3, dy24 = y2 - y4;

			dx13 *= dx13;
			dx14 *= dx14;
			dx23 *= dx23;
			dx24 *= dx24;
			dy13 *= dy13;
			dy14 *= dy14;
			dy23 *= dy23;
			dy24 *= dy24;

			if (sqrt((double)dx13 + dy13) + sqrt((double)dx23 + dy23) < sqrt((double)dx14 + dy14) + sqrt((double)dx24 + dy24)) {
				moveMe->thisStepX = x3;
				moveMe->thisStepY = y3;
			} else {
				moveMe->thisStepX = x4;
				moveMe->thisStepY = y4;
			}
		}
	}

	float yDiff = moveMe->thisStepY - moveMe->y;
	float xDiff = moveMe->x - moveMe->thisStepX;
	if (xDiff || yDiff) {
		moveMe->wantAngle = 180 + ANGLEFIX * atan2(xDiff, yDiff * 2);
		moveMe->spinning = true;
	}

	moveMe->makeTalker();
	return true;
}

void FloorManager::save(Common::WriteStream *stream) {
	if (_currentFloor->numPolygons) {
		stream->writeByte(1);
		stream->writeUint16BE(_currentFloor->originalNum);
	} else {
		stream->writeByte(0);
	}
}

bool FloorManager::load(Common::SeekableReadStream *stream) {
	if (stream->readByte()) {
		if (!setFloor(stream->readUint16BE()))
			return false;
	} else {
		setFloorNull();
	}
	return true;
}

} // End of namespace Sludge
