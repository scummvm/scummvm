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

const MovePersonData Walk::_moveData[] = {
   {"COMPY",       -1, -6, 1, 6, 0, 0, 0, 0,12,12,1,14},
   {"DEINO",       -1, -8, 1, 8, 0, 0, 0, 0,11,11,1,10},
   {"FAYE",        -1, -6, 1, 6,13,18, 7,12,19,22,2, 5},
   {"GUARDS",      -1, -6, 1, 6, 0, 0, 0, 0, 7, 7,2, 5},
   {"PRINCESS1",   -1, -6, 1, 6,13,18, 7,12,19,21,2, 5},
   {"PRINCESS2",   -1, -6, 1, 6,13,18, 7,12,19,21,2, 5},
   {"AMGUARD",     -1, -6, 1, 6,13,18, 7,12,19,21,2, 5},
   {"SPARKY",      -1, -6, 1, 6,13,18, 7,12,21,20,2, 5},
   {"LOLA_SHOWER", -1, -6,55,60, 0, 0, 0, 0, 7, 7,2, 5},
   {"LOLA",       -24,-29,24,29, 0, 0, 0, 0,30,30,2, 5},
   {"BOB",        -15,-20,15,20,21,26, 0, 0,27,29,2, 5},
   {"CHEF",        -1, -4, 1, 4, 0, 0, 0, 0, 1, 5,2, 4},
   {"HENRY",       -1, -6, 1, 6, 0, 0, 0, 0, 7, 7,2, 6},
   {"ANDERSON",    -1, -6, 1, 6, 0, 0, 0, 0, 7, 7,2, 5},
   {"JASPAR",      -4, -9, 4, 9,16,21,10,15, 1, 3,1,10},
   {"PYGMY",       -7,-12, 7,12, 0, 0, 0, 0,27,27,2, 5},
   {"FRANK",        7, 12, 1, 6, 0, 0, 0, 0,13,13,2, 4},
   {"WEDGEWOOD",  -20,-25,20,25, 0, 0, 0, 0, 1, 1,1, 5},
   {"TMPD",        -1, -6, 1, 6,13,18, 7,12,19,21,2, 5},
   {"IAN",         -1, -6, 1, 6, 0, 0, 0, 0, 7, 7,2, 6},
   {"*",            0,  0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0}
};



Walk::Walk(Logic *logic, Graphics *graphics)
	: _logic(logic), _graphics(graphics) {
}


uint16 Walk::joeFace() {

	debug(9, "Walk::joeFace() - curFace = %d, prevFace = %d", _logic->joeFacing(), _joePrevFacing);
	BobSlot *pbs = _graphics->bob(0);
	uint16 frame;
	if (_logic->currentRoom() == 108) {
		frame = 1;
	}
	else {
		frame = 33;
		if (_logic->joeFacing() == DIR_FRONT) {
			if (_joePrevFacing == DIR_BACK) {
				pbs->frameNum = 33 + FRAMES_JOE_XTRA;
				_graphics->update();
			}
			frame = 34;
		}
		else if (_logic->joeFacing() == DIR_BACK) {
			if (_joePrevFacing == DIR_FRONT) {
				pbs->frameNum = 33 + FRAMES_JOE_XTRA;
				_graphics->update();
			}
			frame = 35;
		}
		else if ((_logic->joeFacing() == DIR_LEFT && _joePrevFacing == DIR_RIGHT) 
			|| 	(_logic->joeFacing() == DIR_RIGHT && _joePrevFacing == DIR_LEFT)) {
			pbs->frameNum = 34 + FRAMES_JOE_XTRA;
			_graphics->update();
		}
		pbs->frameNum = frame + FRAMES_JOE_XTRA;
		pbs->scale = _logic->joeScale();
		pbs->xflip = (_logic->joeFacing() == DIR_LEFT);
		_graphics->update();
		_joePrevFacing = _logic->joeFacing();
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


void Walk::joeMoveBlock(int facing) {
	warning("Walk::moveJoeBlock() partially implemented");
	_graphics->bob(0)->animating = false;
//    CAN=-2;
    // Make Joe face the right direction
	_logic->joeFacing(facing);
	joeFace();

	// TODO: cutaway calls
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


void Walk::animateJoe() {
	// queen.c l.2789-2835
	uint16 lastDirection = 0;
	uint16 i;
	BobSlot *pbs = _graphics->bob(0);
	_logic->joeFacing(_walkData[1].anim.facing);
	_logic->joeScale(_walkData[1].area->calcScale(pbs->y));
	joeFace();
	bool interrupted = false;
	for (i = 1; i <= _walkDataCount && !interrupted; ++i) {

		WalkData *pwd = &_walkData[i];

		if (pwd->area->mapNeighbours < 0) {
			joeMoveBlock(pwd->anim.facing);
			return;
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
			_graphics->update(); // CHECK_PLAYER();
			if (_logic->joeWalk() == 2) { // || cutQuit 
				// we are about to do something else, so stop walking
				interrupted = true;
				pbs->moving = false;
			}
		}
		lastDirection = pwd->anim.facing;
	}
//	if (!cutQuit) {
	pbs->animating = false;
	_logic->joeFacing(lastDirection);
//	}
}


void Walk::animatePersonPrepare(const MovePersonData *mpd, const Person *pp) {
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
				if (pp->direction == -3) {
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
					if (pp->direction == -3) {
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


void Walk::animatePerson(const MovePersonData *mpd, const Person *pp) {
	// queen.c l.2572-2651

	BobSlot *pbs = _graphics->bob(pp->bobNum);

	// check to see which way person should be facing
	if (mpd->walkLeft1 == mpd->walkRight1) {
		pbs->xflip = (pp->direction == -3);
	}
	else {
		// they have special walk for left and right, so don't flip
		pbs->xflip = false;
	}

	uint16 i;
	for (i = 1; i <= _walkDataCount; ++i) {
		WalkData *pwd = &_walkData[i];
		// unpack necessary frames for bob animation
		uint16 dstFrame = pp->image;
		uint16 srcFrame = ABS(pwd->anim.firstFrame);
		while (srcFrame <= ABS(pwd->anim.lastFrame)) {
			_graphics->bankUnpack(srcFrame, dstFrame, pp->bankNum);
			++dstFrame;
			++srcFrame;
		}
		// pass across bobs direction ONLY if walk is a mirror flip!
		if (ABS(mpd->walkLeft1) == ABS(mpd->walkRight1)) {
			_graphics->bobAnimNormal(pp->bobNum, pp->image, dstFrame - 1, mpd->animSpeed, false, pbs->xflip);
		}
		else {
			_graphics->bobAnimNormal(pp->bobNum, pp->image, dstFrame - 1, mpd->animSpeed, false, false);
		}

		// move other actors at correct speed relative to scale
		uint16 moveSpeed = _logic->findScale(pbs->x, pbs->y) * mpd->moveSpeed / 100;
		_graphics->bobMove(pp->bobNum, pbs->x + pwd->dx, pbs->y + pwd->dy, moveSpeed);

		// flip if one set of frames for actor
		if (mpd->walkLeft1 < 0 || ABS(mpd->walkLeft1) == ABS(mpd->walkRight1)) {
			pbs->xflip = pwd->dx < 0;
		}

		while (pbs->moving) {
			_graphics->update();
			uint16 scale = pwd->area->calcScale(pbs->y);
			pbs->scale = scale;
			if (pbs->xmajor) {
				pbs->speed = scale * mpd->moveSpeed / 100;
			}
			else {
				pbs->speed = scale * (mpd->moveSpeed / 2) / 100;
			}
//			if (cutQuit)
		}
	}
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


ObjectData *Walk::joeSetupInRoom(bool autoPosition, uint16 scale) {
	// queen.c SETUP_HERO()

	uint16 oldx;
	uint16 oldy;
	WalkOffData *pwo = NULL;
	ObjectData *pod = _logic->objectData(_logic->entryObj());
	if (pod == NULL) {
		error("Walk::joeSetupInRoom() - No object data for obj %d", _logic->entryObj());
	}

	if (!autoPosition || _logic->joeX() != 0 || _logic->joeY() != 0) {
		oldx = _logic->joeX();
		oldy = _logic->joeY();
	}
	else {
		// find the walk off point for the entry object and make 
		// Joe walking to that point
		pwo = _logic->walkOffPointForObject(_logic->entryObj());
		if (pwo != NULL) {
			oldx = pwo->x;
			oldy = pwo->y;
		}
		else {
			// no walk off point, use object position
			oldx = pod->x;
			oldy = pod->y;
		}
	}

	debug(9, "Walk::joeSetupInRoom() - oldx=%d, oldy=%d", oldx, oldy);

	if (scale > 0 && scale < 100) {
		_logic->joeScale(scale);
	}
	else {
		uint16 area = _logic->zoneInArea(ZONE_ROOM, oldx, oldy);
		if (area > 0) {
			_logic->joeScale(_logic->currentRoomArea(area)->calcScale(oldy));
		}
		else {
			_logic->joeScale(100);
		}
	}

	// TODO: cutawayJoeFacing

    // check to see which way Joe entered room
	// TODO: JoeFacing with _objectData[entryObj].state
	_logic->joeFacing(DIR_FRONT);

	_joePrevFacing = _logic->joeFacing();
	BobSlot *pbs = _graphics->bob(0);
	pbs->scale = _logic->joeScale();

	// TODO: room 108 specific

	joeFace();
	pbs->active = true;
	pbs->x = oldx;
	pbs->y = oldy;
	pbs->frameNum = 29 + FRAMES_JOE_XTRA;
	_logic->joeX(0);
	_logic->joeY(0);

	if (pwo != NULL) {
		// entryObj has a walk off point, then walk from there to object x,y
		return pod;
	}
	return NULL;
}


void Walk::joeMove(int direction, uint16 endx, uint16 endy, bool inCutaway) {

//   CAN=0
	initWalkData();

	uint16 oldx = _graphics->bob(0)->x;
	uint16 oldy = _graphics->bob(0)->y;

	_logic->joeWalk(1);

	uint16 oldPos = _logic->zoneInArea(ZONE_ROOM, oldx, oldy);
	uint16 newPos = _logic->zoneInArea(ZONE_ROOM, endx, endy);

	debug(9, "Walk::joeMove(%d, %d, %d, %d, %d), old = %d, new = %d", direction, oldx, oldy, endx, endy, oldPos, newPos);

	// if in cutaway, allow Joe to walk anywhere
	if(newPos == 0 && inCutaway) {
		incWalkData(oldx, oldy, endx, endy, oldPos);
	}
	else {
		calc(oldPos, newPos, oldx, oldy, endx, endy);
	}

	if (_walkDataCount > 0) {
//MOVE_JOE2:
		animateJoePrepare();
		animateJoe();
	}
	else {
//		SPEAK(JOE_RESPstr[4],"JOE",find_cd_desc(4));
	}
//MOVE_JOE_EXIT:
	if (direction > 0) {
		_logic->joeFacing(direction);
	}
	_joePrevFacing = _logic->joeFacing();
	joeFace();
}



void Walk::personMove(Person* pp, uint16 endx, uint16 endy) {

	// CAN = 0;
	initWalkData();

	uint16 oldx = _graphics->bob(pp->bobNum)->x;
	uint16 oldy = _graphics->bob(pp->bobNum)->y;

	uint16 oldPos = _logic->zoneInArea(ZONE_ROOM, oldx, oldy);
	uint16 newPos = _logic->zoneInArea(ZONE_ROOM, endx, endy);

	debug(9, "Walk::personMove(%d, %d, %d, %d, %d), old = %d, new = %d", pp->direction, oldx, oldy, endx, endy, oldPos, newPos);

	calc(oldPos, newPos, oldx, oldy, endx, endy);

	// find MovePersonData associated to Person
	const MovePersonData *mpd = _moveData;
	while (mpd->name[0] != '*') {
		if (scumm_stricmp(mpd->name, pp->name) == 0) {
			break;
		}
		++mpd;
	}
	if (mpd->name[0] == '*') {
		error("Walk::personMove() - Unknown person : '%s'", pp->name);
	}

	if (_walkDataCount > 0) {
		animatePersonPrepare(mpd, pp);
		animatePerson(mpd, pp);
	}

	uint16 standingFrame = 0;
	if (pp->bobNum <= 3) {
		standingFrame = 29 + FRAMES_JOE_XTRA + pp->bobNum;
	}
	else {
		warning("Walk::personMove() - Wrong bob number : %d", pp->bobNum);
	}
	// make other person face the right direction
	BobSlot *pbs = _graphics->bob(pp->bobNum);
	pbs->endx = endx;
	pbs->endy = endy;
	pbs->animating = false;
	pbs->scale = _walkData[_walkDataCount].area->calcScale(endy);
	if (_walkData[_walkDataCount].anim.facing == DIR_BACK) {
		_graphics->bankUnpack(mpd->backStandingFrame, standingFrame, pp->bankNum);
	}
	else {
		_graphics->bankUnpack(mpd->frontStandingFrame, standingFrame, pp->bankNum);
	}
	uint16 obj = _logic->objectForPerson(pp->bobNum);
	if (_walkData[_walkDataCount].dx < 0) {
		_logic->objectData(obj)->image = -3;
		pbs->xflip = true;
	}
	else {
		_logic->objectData(obj)->image = -4;
		pbs->xflip = false;
	}
	pbs->frameNum = standingFrame;
}


void Walk::calc(uint16 oldPos, uint16 newPos, uint16 oldx, uint16 oldy, uint16 x, uint16 y) {
	
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
		uint16 px = oldx;
		uint16 py = oldy;
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


uint16 Walk::calcC(uint16 c1, uint16 c2, uint16 c3, uint16 c4, uint16 lastc) {

	uint16 s1 = MAX(c1, c3);
	uint16 s2 = MIN(c2, c4);
	uint16 c;
	if ((lastc >= s1 && lastc <= s2) || (lastc >= s2 && lastc <= s1)) {
		c = lastc;
	}
	else {
		c = (s1 + s2) / 2;
	}
	return c;
}


int16 Walk::findAreaPosition(uint16 *x, uint16 *y, bool recalibrate) {
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
		pwd->area = _logic->currentRoomArea(area); //area;
//		pwd->sign = ((pwd->dx < 0) ? -1 : ((pwd->dx > 0) ? 1 : 0)) ;
	}
}



} // End of namespace Queen
