/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEENGRID_H
#define QUEENGRID_H

#include "common/util.h"
#include "queen/structs.h"

namespace Queen {

enum GridScreen {
	GS_ROOM  = 0,
	GS_PANEL = 1,
	GS_COUNT = 2
};

class QueenEngine;

class Grid {
public:

	Grid(QueenEngine *vm);

	void readDataFrom(uint16 numObjects, uint16 numRooms, byte *&ptr);

	void setZone(GridScreen screen, uint16 zoneNum, uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void setZone(GridScreen screen, uint16 zoneNum, const Box &box);
	uint16 findZoneForPos(GridScreen screen, uint16 x, uint16 y) const;
	uint16 findAreaForPos(GridScreen screen, uint16 x, uint16 y) const;
	void clear(GridScreen screen);
	void setupNewRoom(uint16 room, uint16 firstRoomObjNum);
	void setupPanel();
	void drawZones();
	const Box *zone(GridScreen screen, uint16 index) const;

	Verb findVerbUnderCursor(int16 cursorx, int16 cursory) const;
	uint16 findObjectUnderCursor(int16 cursorx, int16 cursory) const;
	uint16 findObjectNumber(uint16 zoneNum) const;
	uint16 findScale(uint16 x, uint16 y) const;

	Area *area(int room, int num) const { return &_area[room][num]; }
	uint16 areaMax(int room) const { return _areaMax[room]; }
	uint16 objMax(int room) const { return _objMax[room]; } 

	enum {
		MAX_ZONES_NUMBER = 32,
		MAX_AREAS_NUMBER = 11
	};


private:

	struct ZoneSlot {
		bool valid;
		Box box;
	};

	ZoneSlot _zones[GS_COUNT][MAX_ZONES_NUMBER];
	
	int16 *_objMax;

	int16 *_areaMax;

	Area (*_area)[MAX_AREAS_NUMBER];

	Box *_objectBox;

	QueenEngine *_vm;
};


} // End of namespace Queen

#endif
