/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEENWALK_H
#define QUEENWALK_H

#include "queen/queen.h"
#include "queen/structs.h"

namespace Queen {


#define MAX_AREAS 11


struct WalkData {
//	int16 sign; // never used
	int16 dx, dy;
	int16 area;
};

struct MovePersonAnim {
	int16 wx;
	int16 wy;
	uint16 firstFrame;
	uint16 lastFrame;
	uint16 facing;
	const Area *walkingArea;

	void setFrames(uint16 ff, uint16 lf, uint16 face) {
		firstFrame = ff;
		lastFrame = lf;
		facing = face;
	}
};


struct MovePersonData {
	const char *name;
	int16 walkLeft1, walkLeft2;
	int16 walkRight1, walkRight2;
	int16 walkBack1, walkBack2;
	int16 walkFront1, walkFront2;
	uint16 frontStandingFrame;
	uint16 backStandingFrame;
	uint16 animSpeed;
	uint16 moveSpeed;
};


class Logic;
class Graphics;

class Walk {
public:

	Walk(Logic* logic, Graphics* graphics);

	void joeSetup();
	
	//! MOVE_JOE()
	void joeMove(int dir, uint16 oldx, uint16 oldy, uint16 newx, uint16 newy, bool inCutaway);
	
	//! FACE_JOE()
	uint16 joeFace(uint16 prevFacing);

	//! MOVE_OTHER
	void personMove(const char* name, uint16 endx, uint16 endy, uint16 image, int dir);


private:

	void joeMoveBlock();

	void animatePersonPrepare();

	void animatePerson();

	//! CALC_X, CALC_Y
	static uint16 calcC(uint16 c1, uint16 c2, uint16 c3, uint16 c4, uint16 lastc);
	
	//! FIND_OLDP, FIND_NEWP
	int16 findAreaPosition(uint16 *x, uint16 *y, bool recalibrate);

	//! FIND_FREE_AREA, find an area not already struck
	uint16 findFreeArea(uint16 area) const;

	//! 
	bool isAreaStruck(uint16 area) const;

	//! CALC_PATH, calculates the path list from oldArea to newArea
	bool calcPath(uint16 oldArea, uint16 newArea);
	
	//! resets path computed in calcPath()
	void initWalkData();
	
	//! CALC_WALK
	void incWalkData(uint16 px, uint16 py, uint16 x, uint16 y, uint16 area);
	
	//! equivalent to l.2432,2469 MOVE_OTHER() and l.2696,2744 MOVE_JOE()
    void calc(uint16 oldPos, uint16 newPos, uint16 oldx, uint16 oldy, uint16 x, uint16 y);


	MovePersonAnim _moveAnim[15];
	static MovePersonData _moveData[];

	uint16 _walkDataCount;
	WalkData _walkData[16];	
	
	uint16 _areaStrikeCount;
	uint16 _areaStrike[MAX_AREAS + 1];
	uint16 _areaListCount;
	uint16 _areaList[MAX_AREAS + 1];

	Logic *_logic;
	Graphics *_graphics;

};


} // End of namespace Queen

#endif
