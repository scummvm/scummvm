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

#include "queen/walk.h"
#include "queen/logic.h"
#include "queen/graphics.h"
#include "queen/defs.h"

namespace Queen {

MovePersonData Walk::_moveData[] = {
   {"COMPY",-1,-6,1,6,0,0,0,0,12,12,1,14},
   {"DEINO",-1,-8,1,8,0,0,0,0,11,11,1,10},
   {"FAYE",-1,-6,1,6,13,18,7,12,19,22,2,5},
   {"GUARDS",-1,-6,1,6,0,0,0,0,7,7,2,5},
   {"PRINCESS1",-1,-6,1,6,13,18,7,12,19,21,2,5},
   {"PRINCESS2",-1,-6,1,6,13,18,7,12,19,21,2,5},
   {"AMGUARD",-1,-6,1,6,13,18,7,12,19,21,2,5},
   {"SPARKY",-1,-6,1,6,13,18,7,12,21,20,2,5},
   {"LOLA_SHOWER",-1,-6,55,60,0,0,0,0,7,7,2,5},
   {"LOLA",-24,-29,24,29,0,0,0,0,30,30,2,5},
   {"BOB",-15,-20,15,20,21,26,0,0,27,29,2,5},
   {"CHEF",-1,-4,1,4,0,0,0,0,1,5,2,4},
   {"HENRY",-1,-6,1,6,0,0,0,0,7,7,2,6},
   {"ANDERSON",-1,-6,1,6,0,0,0,0,7,7,2,5},
   {"JASPAR",-4,-9,4,9,16,21,10,15,1,3,1,10},
   {"PYGMY",-7,-12,7,12,0,0,0,0,27,27,2,5},
   {"FRANK",7,12,1,6,0,0,0,0,13,13,2,4},
   {"WEDGEWOOD",-20,-25,20,25,0,0,0,0,1,1,1,5},
   {"TMPD",-1,-6,1,6,13,18,7,12,19,21,2,5},
   {"IAN",-1,-6,1,6,0,0,0,0,7,7,2,6},
   {"*",0,0,0,0,0,0,0,0,0,0,0}
};


Walk::Walk(Logic *logic, Graphics *graphics)
	: _logic(logic), _graphics(graphics) {
}


uint16 Walk::joeFace(uint16 prevFacing) {
	BobSlot *pbs = _graphics->bob(0);
	uint16 frame;
	if (_logic->currentRoom() == 108) {
		frame = 1;
	}
	else {
		frame = 33;
		if (_logic->joeFacing() == DIR_FRONT) {
			frame = 34;
		}
		if (_logic->joeFacing() == DIR_BACK) {
			frame = 35;
		}
		// FIXME: handle prevFacing
		pbs->frameNum = frame + FRAMES_JOE_XTRA;
		pbs->scale = _logic->joeScale();
		pbs->xflip = (_logic->joeFacing() == DIR_LEFT);
		_graphics->update();
		// joePrevFacing = joeFacing;
		switch (frame) {
		case 33: frame = 1; break;
		case 34: frame = 3; break;
		case 35: frame = 5; break;
		}
	}
	pbs->frameNum = 29 + FRAMES_JOE_XTRA;
	_graphics->bankUnpack(frame, pbs->frameNum, 7);
	return frame;
}


void Walk::joeMoveBlock() {
	warning("Walk::moveJoeBlock() unimplemented");
}


void Walk::animatePersonPrepare() {
	// queen.c l.2748-2788
	int i;
	for (i = 1; i <= _walkDataCount; ++i) {

		MovePersonAnim *mpa = &_moveAnim[i];
		WalkData *pwd = &_walkData[i];
		mpa->wx = pwd->dx;
		mpa->wy = pwd->dy;
		mpa->walkingArea = _logic->currentRoomArea(pwd->area); // &_roomAreas[ pwd->area ];

		if (mpa->wx < 0) {
			mpa->setFrames(11, 16 + FRAMES_JOE_XTRA, DIR_LEFT);
		}
		else {
			mpa->setFrames(11, 16 + FRAMES_JOE_XTRA, DIR_RIGHT);
		}

		int16 k = ABS(mpa->wy);
		int16 ds = mpa->walkingArea->scaleDiff();
		if (ds > 0) {
			k *= ((k * ds) / mpa->walkingArea->box.yDiff()) / 2;
		}

		if (ABS(mpa->wx) < k) {
			if ((mpa->wy < 0 && ds < 0) || (mpa->wy > 0 && ds > 0)) {
				mpa->setFrames(17 + FRAMES_JOE_XTRA, 22 + FRAMES_JOE_XTRA, DIR_FRONT);
			}
			else {
				mpa->setFrames(23 + FRAMES_JOE_XTRA, 28 + FRAMES_JOE_XTRA, DIR_BACK);
			}
		}	
	}
}


void Walk::animatePerson() {
	// queen.c l.2789-2835
	uint16 lastDirection = 0;
	uint16 i;
	BobSlot *pbs = _graphics->bob(0);
	_logic->joeFacing(_moveAnim[1].facing);
	_logic->joeScale(_moveAnim[1].walkingArea->calcScale(pbs->y));
	joeFace(_logic->joeFacing());
	bool interrupted = false;
	for (i = 1; i <= _walkDataCount && !interrupted; ++i) {
		MovePersonAnim *mpa = &_moveAnim[i];
		if (mpa->walkingArea->mapNeighbours < 0) {
			joeMoveBlock();
			return;
		}
		if (lastDirection != mpa->facing) {
			_graphics->bobAnimNormal(0, mpa->firstFrame, mpa->lastFrame, 1, false, false);
		}
		uint16 scale = _logic->findScale(pbs->x, pbs->y);
		_graphics->bobMove(0, pbs->x + mpa->wx, pbs->y + mpa->wy, scale * 6 / 100);
		pbs->xflip = (pbs->xdir < 0);
		while (pbs->moving) {
			// adjust Joe's movespeed according to scale
			pbs->scale = mpa->walkingArea->calcScale(pbs->y);
			_logic->joeScale(pbs->scale);
			if (pbs->xmajor) {
				pbs->speed = pbs->scale * 6 / 100;
			}
			else {
				pbs->speed = pbs->scale * 3 / 100;
			}
			if (pbs->speed == 0) {
				pbs->speed = 1;
			}
			_graphics->update(); // CHECK_PLAYER();
			if (_logic->joeWalk() == 2) { // || cutQuit 
				// we are about to do something else, so stop walking
				interrupted = true;
				pbs->moving = false;
			}
		}
		lastDirection = mpa->facing;
	}
//	if (!cutQuit) {
	pbs->animating = false;
	_logic->joeFacing(lastDirection);
//	}
}


void Walk::joeSetup() {
	int i;

	_graphics->bankLoad("joe_a.BBK", 13);
	for (i = 11; i <= 28 + FRAMES_JOE_XTRA; ++i) {
		_graphics->bankUnpack(i - 10, i, 13);
	}
	_graphics->bankErase(13);

	_graphics->bankLoad("joe_b.BBK", 7);
	_graphics->bankUnpack(1, 33 + FRAMES_JOE_XTRA, 7);
	_graphics->bankUnpack(3, 34 + FRAMES_JOE_XTRA, 7);
	_graphics->bankUnpack(5, 35 + FRAMES_JOE_XTRA, 7);

	_logic->joeFacing(DIR_FRONT);
}

void Walk::joeMove(int direction, uint16 oldx, uint16 oldy, uint16 newx, uint16 newy, bool inCutaway) {

//   CAN=0
	initWalkData();

	_logic->joeWalk(1);

	uint16 oldPos = _logic->zoneInArea(ZONE_ROOM, oldx, oldy);
	uint16 newPos = _logic->zoneInArea(ZONE_ROOM, newx, newy);

	debug(9, "Walk::joeMove(%d, %d, %d, %d, %d), old = %d, new = %d", direction, oldx, oldy, newx, newy, oldPos, newPos);

	// if in cutaway, allow Joe to walk anywhere
	if(newPos == 0 && inCutaway) {
		incWalkData(oldx, oldy, newx, newy, oldPos);
	}
	else {
		calc(oldPos, newPos, oldx, oldy, newx, newy);
	}

	if (_walkDataCount > 0) {
//MOVE_JOE2:
		animatePersonPrepare();
		animatePerson();
	}
	else {
//		SPEAK(JOE_RESPstr[4],"JOE",find_cd_desc(4))
	}
//MOVE_JOE_EXIT:
	if (direction > 0) {
		_logic->joeFacing(direction);
	}
//	joePrevFacing = _logic->joeFacing();
	joeFace(0);
}


void Walk::personMove(const char* name, uint16 endx, uint16 endy, uint16 image, int dir) {
	warning("Walk::personMove() unimplemented");
}


void Walk::calc(uint16 oldPos, uint16 newPos, uint16 oldx, uint16 oldy, uint16 x, uint16 y) {
	
	// if newPos is outside of an AREA then travers Y axis until an AREA is found
	if (newPos == 0) { 
		newPos = findAreaPosition(&x, &y, true);
	}
	
	// do the same for oldPos in case Joe somehow sits on the border of an AREA
	// and does not register
	if (oldPos == 0) {
		oldPos = findAreaPosition(&oldx, &oldy, false);
	}

	if (oldPos == newPos) {
		incWalkData(oldx, oldy, x, y, newPos);
	}
	else if (calcPath(oldPos, newPos)) {
		uint16 i;
		uint16 px = oldx;
		uint16 py = oldy;
		for (i = 2; i <= _areaListCount; ++i) {
			uint16 a1 = _areaList[i - 1];
			uint16 a2 = _areaList[i];
			const Area *pa1 = _logic->currentRoomArea(a1); //&_roomAreas[ a1 ];
			const Area *pa2 = _logic->currentRoomArea(a2);
			uint16 x1 = calcC(pa1->box.x1, pa1->box.x2, pa2->box.x1, pa2->box.x2, px);
			uint16 y1 = calcC(pa1->box.y1, pa1->box.y2, pa2->box.y1, pa2->box.y2, py);
			incWalkData(px, py, x1, y1, a1);
			px = x1;
			py = y1;
		}
		incWalkData(px, py, x, y, newPos);
	}
}


uint16 Walk::calcC(uint16 c1, uint16 c2, uint16 c3, uint16 c4, uint16 lastc) {
	uint16 s1 = MAX(c1, c3);
	uint16 s2 = MIN(c2, c4);
	uint16 c = (s1 + s2) / 2;
	if ((lastc >= s1 && lastc <= s2) || (lastc >= s2 && lastc <= s1)) {
		c = lastc;
	}
	return c;
}


int16 Walk::findAreaPosition(uint16 *x, uint16 *y, bool recalibrate) {
	uint16 i;
	uint16 pos = 1;
	const Box *b = &_logic->currentRoomArea(1)->box;
	uint16 tx = b->x1;
	uint16 bx = b->x2;
	uint16 ty = b->y1;
	uint16 by = b->y2;
	uint16 prevClosestFace = 640;
	for (i = 1; i <= _logic->currentRoomAreaMax(); ++i) {
		b = &_logic->currentRoomArea(i)->box;
		uint16 dx1 = ABS(b->x1 - *x);
		uint16 dx2 = ABS(b->x2 - *x);
		uint16 dy1 = ABS(b->y1 - *y);
		uint16 dy2 = ABS(b->y2 - *y);
		uint16 csx = MIN(dx1, dx2);
		uint16 csy = MIN(dy1, dy2);
		uint16 curClosestFace = 640;
		// check to see if X lies in X range		
		if (*x >= b->x1 && *x <= b->x2) {
			// it is, so record closest Y face distance
			curClosestFace = csy;
		}
		else if (*y >= b->y1 && *y <= b->y2) {
			// record, closest X face distance
			curClosestFace = csx;		
		}
		if (curClosestFace < prevClosestFace) {
			tx = dx1;
			ty = dy1;
			bx = dx2;
			by = dy2;
			pos = i;
			prevClosestFace = curClosestFace;
		}
	}
 	// we now have the closest area near X,Y, so we can recalibrate
 	// the X,Y coord to be in this area
	if (recalibrate) {
		b = &_logic->currentRoomArea(pos)->box;
		if(*x < b->x1) *x = b->x1;
		if(*x > b->x2) *x = b->x2;
		if(*y < b->y1) *y = b->y1;
		if(*y > b->y2) *y = b->y2;
	}
	return pos;
}


uint16 Walk::findFreeArea(uint16 area) const {

	uint16 testArea;
	uint16 freeArea = 0;
	uint16 map = ABS(_logic->currentRoomArea(area)->mapNeighbours);
	for (testArea = 1; testArea <= _logic->currentRoomAreaMax(); ++testArea) {
		int b = _logic->currentRoomAreaMax() - testArea;
		if (map & (1 << b)) {
			// connecting area, check if it's been struck off
			if(!isAreaStruck(testArea)) {
				// legitimate connecting area, keep it
				freeArea = testArea;
				break;
			}
		}
	}
	return freeArea;
}


bool Walk::isAreaStruck(uint16 area) const {

	uint16 i;
	bool found = false;
	for (i = 1; i <= _areaStrikeCount; ++i) {
		if (_areaStrike[i] == area) {
			found = true;
			break;
		}
	}
	return found;
}


bool Walk::calcPath(uint16 oldArea, uint16 newArea) {

	debug(9, "Walk::calcPath(%d, %d)", oldArea, newArea);
	_areaList[1] = _areaStrike[1] = oldArea;
	_areaListCount = _areaStrikeCount = 1;	
	uint16 area = oldArea;
	while (_areaListCount > 0 && area != newArea) {
		area = findFreeArea(area);
		if (!area) {
			// wrong path, rolling back
			_areaList[_areaListCount] = 0;
			--_areaListCount;
			area = _areaList[_areaListCount];	
		}
		else {
			++_areaListCount;
			_areaList[_areaListCount] = area;
			if(!isAreaStruck(area)) {
				++_areaStrikeCount;
				_areaStrike[_areaStrikeCount] = area;
			}
		}		
	}
	// CAN = -1 if no connection is made, else 0
	return _areaList[1] != 0;
}


void Walk::initWalkData() {
	_walkDataCount = 0;
	memset(_walkData, 0, sizeof(_walkData));
	_areaStrikeCount = 0;
	memset(_areaStrike, 0, sizeof(_areaStrike));
	_areaListCount = 0;
	memset(_areaList, 0, sizeof(_areaList));
}


void Walk::incWalkData(uint16 px, uint16 py, uint16 x, uint16 y, uint16 area) {

	debug(9, "Walk::incWalkData(%d, %d, %d)", (int16)(x - px), (int16)(y - py), area);

	if (px != x || py != y) {
		++_walkDataCount;
		WalkData *pwd = &_walkData[_walkDataCount];
		pwd->dx = x - px;
		pwd->dy = y - py;
		pwd->area = area;
//		pwd->sign = ((pwd->dx < 0) ? -1 : ((pwd->dx > 0) ? 1 : 0)) ;
	}
}


} // End of namespace Queen
