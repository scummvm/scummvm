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

#include "stdafx.h"
#include "queen/walk.h"
#include "queen/logic.h"
#include "queen/graphics.h"
#include "queen/defs.h"

namespace Queen {


const MovePersonData Walk::MOVE_DATA[] = {
	{"COMPY",       -1,  -6,  1,  6,  0,  0,  0,  0, 12, 12, 1, 14},
	{"DEINO",       -1,  -8,  1,  8,  0,  0,  0,  0, 11, 11, 1, 10},
	{"FAYE",        -1,  -6,  1,  6, 13, 18,  7, 12, 19, 22, 2,  5},
	{"GUARDS",      -1,  -6,  1,  6,  0,  0,  0,  0,  7,  7, 2,  5},
	{"PRINCESS1",   -1,  -6,  1,  6, 13, 18,  7, 12, 19, 21, 2,  5},
	{"PRINCESS2",   -1,  -6,  1,  6, 13, 18,  7, 12, 19, 21, 2,  5},
	{"AMGUARD",     -1,  -6,  1,  6, 13, 18,  7, 12, 19, 21, 2,  5},
	{"SPARKY",      -1,  -6,  1,  6, 13, 18,  7, 12, 21, 20, 2,  5},
	{"LOLA_SHOWER", -1,  -6, 55, 60,  0,  0,  0,  0,  7,  7, 2,  5},
	{"LOLA",       -24, -29, 24, 29,  0,  0,  0,  0, 30, 30, 2,  5},
	{"BOB",        -15, -20, 15, 20, 21, 26,  0,  0, 27, 29, 2,  5},
	{"CHEF",        -1,  -4,  1,  4,  0,  0,  0,  0,  1,  5, 2,  4},
	{"HENRY",       -1,  -6,  1,  6,  0,  0,  0,  0,  7,  7, 2,  6},
	{"ANDERSON",    -1,  -6,  1,  6,  0,  0,  0,  0,  7,  7, 2,  5},
	{"JASPAR",      -4,  -9,  4,  9, 16, 21, 10, 15,  1,  3, 1, 10},
	{"PYGMY",       -7, -12,  7, 12,  0,  0,  0,  0, 27, 27, 2,  5},
	{"FRANK",        7,  12,  1,  6,  0,  0,  0,  0, 13, 13, 2,  4},
	{"WEDGEWOOD",  -20, -25, 20, 25,  0,  0,  0,  0,  1,  1, 1,  5},
	{"TMPD",        -1,  -6,  1,  6, 13, 18,  7, 12, 19, 21, 2,  5},
	{"IAN",         -1,  -6,  1,  6,  0,  0,  0,  0,  7,  7, 2,  6},
	{"*",            0,   0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0}
};



Walk::Walk(Logic *logic, Graphics *graphics)
	: _logic(logic), _graphics(graphics) {
}


void Walk::animateJoePrepare() {
	// queen.c l.2748-2788
	uint16 i;
	for (i = 1; i <= _walkDataCount; ++i) {

		WalkData *pwd = &_walkData[i];

		if (pwd->dx < 0) {
			pwd->anim.set(11, 16 + FRAMES_JOE_XTRA, DIR_LEFT);
		}
		else {
			pwd->anim.set(11, 16 + FRAMES_JOE_XTRA, DIR_RIGHT);
		}

		int16 k = ABS(pwd->dy);
		int16 ds = pwd->area->scaleDiff();
		if (ds > 0) {
			k *= ((k * ds) / pwd->area->box.yDiff()) / 2;
		}

		if (ABS(pwd->dx) < k) {
			if (pwd->dy < 0) {
				if (ds < 0) {
					pwd->anim.set(17 + FRAMES_JOE_XTRA, 22 + FRAMES_JOE_XTRA, DIR_FRONT);
				}
				else {
					pwd->anim.set(23 + FRAMES_JOE_XTRA, 28 + FRAMES_JOE_XTRA, DIR_BACK);
				}
			}
			else if (pwd->dy > 0) {
				if (ds < 0) {
					pwd->anim.set(23 + FRAMES_JOE_XTRA, 28 + FRAMES_JOE_XTRA, DIR_BACK);
				}
				else {
					pwd->anim.set(17 + FRAMES_JOE_XTRA, 22 + FRAMES_JOE_XTRA, DIR_FRONT);
				}
			}
		}	
	}
}


bool Walk::animateJoe() {
	// queen.c l.2789-2835
	uint16 lastDirection = 0;
	uint16 i;
	BobSlot *pbs = _graphics->bob(0);
	_logic->joeFacing(_walkData[1].anim.facing);
	_logic->joeScale(_walkData[1].area->calcScale(pbs->y));
	_logic->joeFace();
	bool interrupted = false;
	for (i = 1; i <= _walkDataCount && !interrupted; ++i) {

		WalkData *pwd = &_walkData[i];

		// area has been turned off, see if we should execute a cutaway
		if (pwd->area->mapNeighbours < 0) {
			// queen.c l.2838-2911
			_logic->customMoveJoe(pwd->anim.facing, pwd->areaNum, i);
			_joeMoveBlock = true;
			return interrupted;
		}
		if (lastDirection != pwd->anim.facing) {
			_graphics->bobAnimNormal(0, pwd->anim.firstFrame, pwd->anim.lastFrame, 1, false, false);
		}

		uint16 moveSpeed = _logic->findScale(pbs->x, pbs->y) * 6 / 100;
		_graphics->bobMove(0, pbs->x + pwd->dx, pbs->y + pwd->dy, moveSpeed);
		pbs->xflip = (pbs->xdir < 0);
		while (pbs->moving) {
			// adjust Joe's movespeed according to scale
			pbs->scale = pwd->area->calcScale(pbs->y);
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
			_logic->checkPlayer();
			if (_logic->joeWalk() == JWM_EXECUTE) { // XXX || cutQuit 
				// we are about to do something else, so stop walking
				interrupted = true;
				pbs->moving = false;
			}
		}
		lastDirection = pwd->anim.facing;
	}
	_logic->joeFacing(lastDirection);
	return interrupted;
}


void Walk::animatePersonPrepare(const MovePersonData *mpd, int direction) {
	// queen.c l.2469-2572
	int i;
	for (i = 1; i <= _walkDataCount; ++i) {

		WalkData *pwd = &_walkData[i];

		if (pwd->dx < 0) {
			pwd->anim.set(mpd->walkLeft1, mpd->walkLeft2, DIR_LEFT);
		}
		else if (pwd->dx > 0) {
			pwd->anim.set(mpd->walkRight1, mpd->walkRight2, DIR_RIGHT);
		}
		else {
			if (ABS(mpd->walkLeft1) == ABS(mpd->walkRight1)) {
				pwd->anim.set(mpd->walkRight1, mpd->walkRight2, DIR_RIGHT);
			}
			else {
				// we have specific moves for this actor, see what direction they were last facing
				if (direction == -3) {
					// previously facing right
					pwd->anim.set(mpd->walkLeft1, mpd->walkLeft2, DIR_LEFT);
				}
				else {
					// previously facing left
					pwd->anim.set(mpd->walkRight1, mpd->walkRight2, DIR_RIGHT);
				}
			}
		}

		int16 k = ABS(pwd->dy);
		int16 ds = pwd->area->scaleDiff();
		if (ds > 0) {
			k *= ((k * ds) / pwd->area->box.yDiff()) / 2;
		}

		if(ABS(pwd->dx) < k) {
			if (pwd->dy < 0) {
				if (mpd->walkBack1 > 0) {
					pwd->anim.set(mpd->walkBack1, mpd->walkBack2, DIR_BACK);
				}
				else if (pwd->dx < 0) {
					pwd->anim.set(mpd->walkLeft1, mpd->walkLeft2, DIR_BACK);
				}
				else {
					pwd->anim.set(mpd->walkRight1, mpd->walkRight2, DIR_BACK);
				}
			}
			else if (pwd->dy > 0) {
				if (mpd->walkFront1 > 0) {
					pwd->anim.set(mpd->walkFront1, mpd->walkFront2, DIR_FRONT);
				}
				else if(ABS(mpd->walkLeft1) == ABS(mpd->walkRight1)) {
					if (pwd->dx < 0) {
						pwd->anim.set(mpd->walkLeft1, mpd->walkLeft2, DIR_FRONT);
					}
					else {
						pwd->anim.set(mpd->walkRight1, mpd->walkRight2, DIR_FRONT);
					}
				}
				else {
					// we have a special move for left/right, so select that instead!
					if (direction == -3) {
						// previously facing right
						pwd->anim.set(mpd->walkLeft1, mpd->walkLeft2, DIR_FRONT);
					}
					else {
						// previously facing left
						pwd->anim.set(mpd->walkRight1, mpd->walkRight2, DIR_FRONT);
					}
				}
			}
		}
	}
}


void Walk::animatePerson(const MovePersonData *mpd, uint16 image, uint16 bobNum, uint16 bankNum, int direction) {
	// queen.c l.2572-2651

	BobSlot *pbs = _graphics->bob(bobNum);

	// check to see which way person should be facing
	if (mpd->walkLeft1 == mpd->walkRight1) {
		pbs->xflip = (direction == -3);
	}
	else {
		// they have special walk for left and right, so don't flip
		pbs->xflip = false;
	}

	uint16 i;
	for (i = 1; i <= _walkDataCount; ++i) {
		WalkData *pwd = &_walkData[i];
		// unpack necessary frames for bob animation
		uint16 dstFrame = image;
		uint16 srcFrame = ABS(pwd->anim.firstFrame);
		while (srcFrame <= ABS(pwd->anim.lastFrame)) {
			_graphics->bankUnpack(srcFrame, dstFrame, bankNum);
			++dstFrame;
			++srcFrame;
		}
		// pass across bobs direction ONLY if walk is a mirror flip!
		if (ABS(mpd->walkLeft1) == ABS(mpd->walkRight1)) {
			_graphics->bobAnimNormal(bobNum, image, dstFrame - 1, mpd->animSpeed, false, pbs->xflip);
		}
		else {
			_graphics->bobAnimNormal(bobNum, image, dstFrame - 1, mpd->animSpeed, false, false);
		}

		// move other actors at correct speed relative to scale
		uint16 moveSpeed = _logic->findScale(pbs->x, pbs->y) * mpd->moveSpeed / 100;
		_graphics->bobMove(bobNum, pbs->x + pwd->dx, pbs->y + pwd->dy, moveSpeed);

		// flip if one set of frames for actor
		if (mpd->walkLeft1 < 0 || ABS(mpd->walkLeft1) == ABS(mpd->walkRight1)) {
			pbs->xflip = pwd->dx < 0;
		}

		while (pbs->moving) {
			_logic->update();
			uint16 scale = pwd->area->calcScale(pbs->y);
			pbs->scale = scale;
			if (pbs->xmajor) {
				pbs->speed = scale * mpd->moveSpeed / 100;
			}
			else {
				pbs->speed = scale * (mpd->moveSpeed / 2) / 100;
			}
			// XXX if (cutQuit)
		}
	}
}


int16 Walk::joeMove(int direction, int16 endx, int16 endy, bool inCutaway) {

	_joeMoveBlock = false;
	int16 can = 0;
	initWalkData();

	uint16 oldx = _graphics->bob(0)->x;
	uint16 oldy = _graphics->bob(0)->y;

	_logic->joeWalk(JWM_MOVE);

	uint16 oldPos = _logic->zoneInArea(ZONE_ROOM, oldx, oldy);
	uint16 newPos = _logic->zoneInArea(ZONE_ROOM, endx, endy);

	debug(9, "Walk::joeMove(%d, %d, %d, %d, %d) - old = %d, new = %d", direction, oldx, oldy, endx, endy, oldPos, newPos);

	// if in cutaway, allow Joe to walk anywhere
	if(newPos == 0 && inCutaway) {
		incWalkData(oldx, oldy, endx, endy, oldPos);
	}
	else {
		calc(oldPos, newPos, oldx, oldy, endx, endy);
	}

	if (_walkDataCount > 0) {
		animateJoePrepare();
		if(animateJoe()) {
			can = -1;
		}
	}
	else {
		// path has been blocked, make Joe say so
		_logic->joeSpeak(4);
		can = -1;
	}

	_graphics->bob(0)->animating = false;
	// cyx: the NEW_ROOM = 0 is done in Command::grabCurrentSelection()
	// XXX if ((CAN==-1) && (walkgameload==0)) NEW_ROOM=0;
	// XXX walkgameload=0;
	if (_joeMoveBlock) {
		can = -2;
		_joeMoveBlock = false;
	}
	else if (direction > 0) {
		_logic->joeFacing(direction);
	}
	_logic->joePrevFacing(_logic->joeFacing());
	_logic->joeFace();
	return can;
}


int16 Walk::personMove(const Person *pp, int16 endx, int16 endy, uint16 curImage, int direction) {

	if (curImage > MAX_FRAMES_NUMBER) {
		error("[Walk::personMove] curImage is invalid: %i", curImage);
	}

	if (endx == 0 && endy == 0) {
		warning("Walk::personMove() - endx == 0 && endy == 0");
		return 0;
	}

	// no longer walk characters in ending
	if (_logic->currentRoom() == 69) {
		if (strcmp(pp->name, "SPARKY") == 0 || strcmp(pp->name, "FAYE") == 0) {
			return 0;
		}
	}

	int16 can = 0;
	initWalkData();

	uint16 bobNum = pp->actor->bobNum;
	uint16 bankNum = pp->bankNum;

	uint16 oldx = _graphics->bob(bobNum)->x;
	uint16 oldy = _graphics->bob(bobNum)->y;

	uint16 oldPos = _logic->zoneInArea(ZONE_ROOM, oldx, oldy);
	uint16 newPos = _logic->zoneInArea(ZONE_ROOM, endx, endy);

	debug(9, "Walk::personMove(%d, %d, %d, %d, %d) - old = %d, new = %d", direction, oldx, oldy, endx, endy, oldPos, newPos);

	calc(oldPos, newPos, oldx, oldy, endx, endy);

	// find MovePersonData associated to Person
	const MovePersonData *mpd = MOVE_DATA;
	while (mpd->name[0] != '*') {
		if (scumm_stricmp(mpd->name, pp->name) == 0) {
			break;
		}
		++mpd;
	}

	if (_walkDataCount > 0) {
		animatePersonPrepare(mpd, direction);
		animatePerson(mpd, curImage, bobNum, bankNum, direction);
	}
	else {
		can = -1;
	}

	uint16 standingFrame = 0;
	if (bobNum <= 3) {
		standingFrame = 29 + FRAMES_JOE_XTRA + bobNum;
	}
	else {
		warning("Walk::personMove() - Wrong bob number : %d", bobNum);
	}
	// make other person face the right direction
	BobSlot *pbs = _graphics->bob(bobNum);
	pbs->endx = endx;
	pbs->endy = endy;
	pbs->animating = false;
	pbs->scale = _walkData[_walkDataCount].area->calcScale(endy);
	if (_walkData[_walkDataCount].anim.facing == DIR_BACK) {
		_graphics->bankUnpack(mpd->backStandingFrame, standingFrame, bankNum);
	}
	else {
		_graphics->bankUnpack(mpd->frontStandingFrame, standingFrame, bankNum);
	}
	uint16 obj = _logic->objectForPerson(bobNum);
	if (_walkData[_walkDataCount].dx < 0) {
		_logic->objectData(obj)->image = -3;
		pbs->xflip = true;
	}
	else {
		_logic->objectData(obj)->image = -4;
		pbs->xflip = false;
	}
	pbs->frameNum = standingFrame;
	return can;
}


void Walk::calc(uint16 oldPos, uint16 newPos, int16 oldx, int16 oldy, int16 x, int16 y) {
	
	// if newPos is outside of an AREA then traverse Y axis until an AREA is found
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
		int16 px = oldx;
		int16 py = oldy;
		for (i = 2; i <= _areaListCount; ++i) {
			uint16 a1 = _areaList[i - 1];
			uint16 a2 = _areaList[i];
			const Area *pa1 = _logic->currentRoomArea(a1);
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


int16 Walk::calcC(int16 c1, int16 c2, int16 c3, int16 c4, int16 lastc) {

	int16 s1 = MAX(c1, c3);
	int16 s2 = MIN(c2, c4);
	int16 c;
	if ((lastc >= s1 && lastc <= s2) || (lastc >= s2 && lastc <= s1)) {
		c = lastc;
	}
	else {
		c = (s1 + s2) / 2;
	}
	return c;
}


int16 Walk::findAreaPosition(int16 *x, int16 *y, bool recalibrate) {
	// In order to locate the nearest available area, the original  algorithm
	// computes the X (or Y) closest face distance for each available area. We
	// simply added the case where the pointer is neither lying in the X range
	// nor in the Y one.
	// To get an example of this in action, in the room D1, make Joe walking
	// to the wall at the right of the window (just above the radiator). On the
	// original game, Joe will go to the left door...
	uint16 i;
	uint16 pos = 1;
	uint32 minDist = ~0;
	const Box *b = &_logic->currentRoomArea(1)->box;
	for (i = 1; i <= _logic->currentRoomAreaMax(); ++i) {

		b = &_logic->currentRoomArea(i)->box;

		uint16 dx1 = ABS(b->x1 - *x);
		uint16 dx2 = ABS(b->x2 - *x);
		uint16 dy1 = ABS(b->y1 - *y);
		uint16 dy2 = ABS(b->y2 - *y);
		uint16 csx = MIN(dx1, dx2);
		uint16 csy = MIN(dy1, dy2);

		bool inX = (*x >= b->x1) && (*x <= b->x2);
		bool inY = (*y >= b->y1) && (*y <= b->y2);

		uint32 dist = minDist;
		if (!inX && !inY) {
			dist = csx * csx + csy * csy;
		}
		else if (inX) {
			dist = csy * csy;
		}
		else if (inY) {
			dist = csx * csx;
		}

		if (dist < minDist) {
			minDist = dist;
			pos = i;
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


void Walk::incWalkData(int16 px, int16 py, int16 x, int16 y, uint16 areaNum) {

	debug(9, "Walk::incWalkData(%d, %d, %d)", (x - px), (y - py), areaNum);

	if (px != x || py != y) {
		++_walkDataCount;
		WalkData *pwd = &_walkData[_walkDataCount];
		pwd->dx = x - px;
		pwd->dy = y - py;
		pwd->area = _logic->currentRoomArea(areaNum);
		pwd->areaNum = areaNum;
//		pwd->sign = ((pwd->dx < 0) ? -1 : ((pwd->dx > 0) ? 1 : 0)) ;
	}
}


} // End of namespace Queen
