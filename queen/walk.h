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


struct MovePersonAnim {
	int16 firstFrame;
	int16 lastFrame;
	uint16 facing;

	void set(int16 ff, int16 lf, uint16 face) {
		firstFrame = ff;
		lastFrame = lf;
		facing = face;
	}
};


struct WalkData {
//	int16 sign; // never used
	int16 dx, dy;
	const Area *area;
	uint16 areaNum; // extra stuff for customMoveJoe
	MovePersonAnim anim;
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

	Walk(Logic *logic, Graphics *graphics);

	int16 joeMove(int direction, uint16 endx, uint16 endy, bool inCutaway);
	
	int16 personMove(const Person *pp, uint16 endx, uint16 endy, uint16 curImage, int direction);

	enum {
		MAX_WALK_DATA = 16
	};

private:

	void animateJoePrepare();
	bool animateJoe();

	void animatePersonPrepare(const MovePersonData *mpd, int direction);
	void animatePerson(const MovePersonData *mpd, uint16 image, uint16 bobNum, uint16 bankNum, int direction);

	//! compute transition coordinate
	static uint16 calcC(uint16 c1, uint16 c2, uint16 c3, uint16 c4, uint16 lastc);
	
	//! find area for position
	int16 findAreaPosition(uint16 *x, uint16 *y, bool recalibrate);

	//! find an area not already struck
	uint16 findFreeArea(uint16 area) const;

	//! return true if the area is already on the walking path
	bool isAreaStruck(uint16 area) const;

	//! calculates the path list from oldArea to newArea
	bool calcPath(uint16 oldArea, uint16 newArea);
	
	//! resets path computed in calcPath()
	void initWalkData();
	
	//! add an area to the path
	void incWalkData(uint16 px, uint16 py, uint16 x, uint16 y, uint16 area);
	
	//! compute path (and populates _walkData) from current position to the new one
    void calc(uint16 oldPos, uint16 newPos, uint16 oldx, uint16 oldy, uint16 x, uint16 y);


	WalkData _walkData[MAX_WALK_DATA];	
	uint16 _walkDataCount;
	
	uint16 _areaStrike[MAX_WALK_DATA];
	uint16 _areaStrikeCount;

	uint16 _areaList[MAX_WALK_DATA];
	uint16 _areaListCount;

	//! set if customMoveJoe() is called in joeAnimate()
	bool _joeMoveBlock;

	Logic *_logic;
	Graphics *_graphics;


	static const MovePersonData MOVE_DATA[];
};


} // End of namespace Queen

#endif
