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

#include "common/rect.h"

namespace Sludge {

class SludgeEngine;
struct OnScreenPerson;

struct FloorPolygon {
	int numVertices;
	int *vertexID;
};

struct Floor {
	int originalNum;
	Common::Point *vertex;
	int numPolygons;
	FloorPolygon *polygon;
	int **matrix;
};

class FloorManager {
public:
	FloorManager(SludgeEngine *vm);
	~FloorManager();

	bool init();
	void kill();

	void setFloorNull();
	bool setFloor(int fileNum);
	void drawFloor();
	int inFloor(int x, int y);
	bool isFloorNoPolygon() { return !_currentFloor || _currentFloor->numPolygons == 0; }

	// For Person collision detection
	bool handleClosestPoint(int &setX, int &setY, int &setPoly);
	bool doBorderStuff(OnScreenPerson *moveMe);

	// Save & load
	void save(Common::WriteStream *stream);
	bool load(Common::SeekableReadStream *stream);

private:
	Floor *_currentFloor;
	SludgeEngine *_vm;

	bool getMatchingCorners(FloorPolygon &, FloorPolygon &, int &, int &);
	bool closestPointOnLine(int &closestX, int &closestY, int x1, int y1, int x2, int y2, int xP, int yP);
	bool pointInFloorPolygon(FloorPolygon &floorPoly, int x, int y);
	bool polysShareSide(FloorPolygon &a, FloorPolygon &b);
};

} // End of namespace Sludge

#endif
