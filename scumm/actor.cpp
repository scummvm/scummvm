/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "scumm.h"
#include "actor.h"
#include "akos.h"
#include "charset.h"
#include "costume.h"
#include "resource.h"
#include "sound.h"
#include "usage_bits.h"

#include <math.h>

byte Actor::kInvalidBox = 0;
Scumm *Actor::_vm = 0;

void Actor::initActorClass(Scumm *scumm) {
	_vm = scumm;
	if (_vm->_features & GF_SMALL_HEADER) {
		kInvalidBox = 255;
	}
}

void Actor::initActor(int mode) {
	if (mode == 1) {
		costume = 0;
		room = 0;
		x = 0;
		y = 0;
		facing = 180;
		newDirection = 180;
		talkFrequency = 256;
	} else if (mode == 2) {
		facing = 180;
		newDirection = 180;
	}

	elevation = 0;
	width = 24;
	talkColor = 15;
	talkPosX = 0;
	talkPosY = -80;
	scaley = scalex = 0xFF;
	charset = 0;
	memset(sound, 0, sizeof(sound));
	newDirection = 0;

	stopActorMoving();

	shadow_mode = 0;
	layer = 0;

	setActorWalkSpeed(8, 2);
	animSpeed = 0;

	ignoreBoxes = false;
	forceClip = 0;
	ignoreTurns = false;
	
	if (_vm->_features & GF_AFTER_V2) {
		initFrame = 2;
		walkFrame = 0;
		standFrame = 1;
		talkFrame1 = 5;
		talkFrame2 = 4;
	} else {
		initFrame = 1;
		walkFrame = 2;
		standFrame = 3;
		talkFrame1 = 4;
		talkFrame2 = 5;
	}

	walk_script = 0;
	talk_script = 0;

	_vm->_classData[number] = (_vm->_features & GF_AFTER_V7) ? _vm->_classData[0] : 0;
}

void Actor::stopActorMoving() {
	_vm->stopScript(walk_script);
	moving = 0;
}

void Actor::setActorWalkSpeed(uint newSpeedX, uint newSpeedY) {
	if (newSpeedX == speedx && newSpeedY == speedy)
		return;

	speedx = newSpeedX;
	speedy = newSpeedY;

	if (moving) {
		calcMovementFactor(walkdata.newx, walkdata.newy);
	}
}

int Scumm::getAngleFromPos(int x, int y) {
	if (_gameId == GID_DIG || _gameId == GID_CMI) {
		double temp = atan2((double)x, (double)-y);
		return normalizeAngle((int)(temp * 180 / 3.1415926535));
	} else {
		if (abs(y) * 2 < abs(x)) {
			if (x > 0)
				return 90;
			return 270;
		} else {
			if (y > 0)
				return 180;
			return 0;
		}
	}
}

int Actor::calcMovementFactor(int newX, int newY) {
	int actorX, actorY;
	int diffX, diffY;
	int32 deltaXFactor, deltaYFactor;

	actorX = x;
	actorY = y;

	if (actorX == newX && actorY == newY)
		return 0;

	diffX = newX - actorX;
	diffY = newY - actorY;
	deltaYFactor = speedy << 16;

	if (diffY < 0)
		deltaYFactor = -deltaYFactor;

	deltaXFactor = deltaYFactor * diffX;
	if (diffY != 0) {
		deltaXFactor /= diffY;
	} else {
		deltaYFactor = 0;
	}

	if ((uint) abs((int)(deltaXFactor >> 16)) > speedx) {
		deltaXFactor = speedx << 16;
		if (diffX < 0)
			deltaXFactor = -deltaXFactor;

		deltaYFactor = deltaXFactor * diffY;
		if (diffX != 0) {
			deltaYFactor /= diffX;
		} else {
			deltaXFactor = 0;
		}
	}

	walkdata.x = actorX;
	walkdata.y = actorY;
	walkdata.newx = newX;
	walkdata.newy = newY;
	walkdata.deltaXFactor = deltaXFactor;
	walkdata.deltaYFactor = deltaYFactor;
	walkdata.xfrac = 0;
	walkdata.yfrac = 0;

	newDirection = _vm->getAngleFromPos(deltaXFactor, deltaYFactor);

	return actorWalkStep();
}

int Actor::remapDirection(int dir, bool is_walking) {
	int specdir;
	byte flags;
	bool flipX;
	bool flipY;

	// FIXME - It seems that at least in The Dig the original code does
	// check ignoreBoxes here. However, it breaks some animations in Loom,
	// causing Bobbin to face towards the camera instead of away from it
	// in some places: After the tree has been destroyed by lightning, and
	// when entering the dark tunnels beyond the dragon's lair at the very
	// least. Possibly other places as well.
	//
	// The Dig also checks if the actor is in the current room, but that's
	// not necessary here because we never call the function unless the
	// actor is in the current room anyway.
	
	if (!ignoreBoxes || _vm->_gameId == GID_LOOM || _vm->_gameId == GID_LOOM256) {
		specdir = _vm->_extraBoxFlags[walkbox];
		if (specdir) {
			if (specdir & 0x8000) {
				dir = specdir & 0x3FFF;
			} else {
				specdir = specdir & 0x3FFF;
				if (specdir - 90 < dir && dir < specdir + 90)
					dir = specdir;
				else
					dir = specdir + 180;
			}
		}

		flags = _vm->getBoxFlags(walkbox);

		flipX = (walkdata.deltaXFactor > 0);
		flipY = (walkdata.deltaYFactor > 0);

		// FIXME - this special cases for the class might be necesary
		// for other games besides Loom!

		// Check for X-Flip
		if ((flags & kBoxXFlip) || isInClass(kObjectClassXFlip)) {
			dir = 360 - dir;
			flipX = !flipX;
		}
		// Check for Y-Flip
		if ((flags & kBoxYFlip) || isInClass(kObjectClassYFlip)) {
			dir = 180 - dir;
			flipY = !flipY;
		}

		switch (flags & 7) {
		case 1:
			if (_vm->_features & GF_AFTER_V7) {
				if (dir < 180)
					return 90;
				else
					return 270;
			} else {
				if (is_walking)	                       // Actor is walking
					return flipX ? 90 : 270;
				else	                               // Actor is standing/turning
					return (dir == 90) ? 90 : 270;
			}
		case 2:
			if (_vm->_features & GF_AFTER_V7) {
				if (dir > 90 && dir < 270)
					return 180;
				else
					return 0;
			} else {
				if (is_walking)	                       // Actor is walking
					return flipY ? 180 : 0;
				else	                               // Actor is standing/turning
					return (dir == 0) ? 0 : 180;
			}
		case 3:
			return 270;
		case 4:
			return 90;
		case 5:
			return 0;
		case 6:
			return 180;
		}
	}
	// OR 1024 in to signal direction interpolation should be done
	return normalizeAngle(dir) | 1024;
}

int Actor::updateActorDirection(bool is_walking) {
	int from;
	int dirType;
	int dir;
	bool shouldInterpolate;

	dirType = (_vm->_features & GF_NEW_COSTUMES) ? _vm->akos_hasManyDirections(this) : false;

	from = toSimpleDir(dirType, facing);
	dir = remapDirection(newDirection, is_walking);

	if (_vm->_features & GF_NEW_COSTUMES)
		// Direction interpolation interfers with walk scripts in Dig; they perform
		// (much better) interpolation themselves.
		shouldInterpolate = false;	
	else
		shouldInterpolate = (dir & 1024) ? true : false;
	dir &= 1023;

	if (shouldInterpolate) {
		int to = toSimpleDir(dirType, dir);
		int num = dirType ? 8 : 4;

		// Turn left or right, depending on which is shorter.
		int diff = to - from;
		if (abs(diff) > (num >> 1))
			diff = -diff;

		if (diff > 0) {
			to = from + 1;
		} else if (diff < 0){
			to = from - 1;
		}

		dir = fromSimpleDir(dirType, (to + num) % num);
	}

	return dir;
}

void Actor::setBox(int box) {
	walkbox = box;
	setupActorScale();
}

int Actor::actorWalkStep() {
	int tmpX, tmpY;
	int actorX, actorY;
	int distX, distY;
	int direction;

	needRedraw = true;
	needBgReset = true;

	direction = updateActorDirection(true);
	if (!(moving & MF_IN_LEG) || facing != direction) {
		if (walkFrame != frame || facing != direction) {
			startWalkAnim(1, direction);
		}
		moving |= MF_IN_LEG;
	}

	actorX = x;
	actorY = y;

	if (walkbox != walkdata.curbox && _vm->checkXYInBoxBounds(walkdata.curbox, actorX, actorY)) {
		setBox(walkdata.curbox);
	}

	distX = abs(walkdata.newx - walkdata.x);
	distY = abs(walkdata.newy - walkdata.y);

	if (abs(actorX - walkdata.x) >= distX && abs(actorY - walkdata.y) >= distY) {
		moving &= ~MF_IN_LEG;
		return 0;
	}

	tmpX = (actorX << 16) + walkdata.xfrac + (walkdata.deltaXFactor >> 8) * scalex;
	walkdata.xfrac = (uint16)tmpX;
	actorX = (tmpX >> 16);

	tmpY = (actorY << 16) + walkdata.yfrac + (walkdata.deltaYFactor >> 8) * scaley;
	walkdata.yfrac = (uint16)tmpY;
	actorY = (tmpY >> 16);

	if (abs(actorX - walkdata.x) > distX) {
		actorX = walkdata.newx;
	}

	if (abs(actorY - walkdata.y) > distY) {
		actorY = walkdata.newy;
	}

	x = actorX;
	y = actorY;
	return 1;
}


void Actor::setupActorScale() {
	uint16 scale;

	if (_vm->_features & GF_NO_SCALLING) {
		scalex = 0xFF;
		scaley = 0xFF;
		return;
	}

	if (ignoreBoxes)
		return;

	if (_vm->getBoxFlags(walkbox) & kBoxPlayerOnly)
		return;

	scale = _vm->getScale(walkbox, x, y);
	if (_vm->_features & GF_AFTER_V8) {
		// At least in COMI, scale values are clipped to range 1-255
		if (scale < 1)
			scale = 1;
		else if (scale > 255)
			scale = 255;
	}

	// FIXME - Hack for The Dig 'Tomb' (room 88)
	//	Otherwise walking to the far-left door causes the actor
	//	to shrink to a one-pixel dot. (!?!?)
	if (_vm->_gameId == GID_DIG && _vm->_currentRoom == 88) {
		scale = 0xFF;
	}


	// FIXME - Quick fix to ft's fuel tower bug (by yazoo)
	//
	// Ben's Y position can be anything between 272 and 398 inclusive
	// (which by the way means that we're always looking at the same
	// element in the scale table... hmmm...)
	//
	// When standing at the bottom of the ladder, Ben's Y position is
	// 356, and by the looks of it he ought to be unscaled there.

	if (_vm->_gameId == GID_FT && scale == 1 && _vm->_currentRoom == 76) {
		scale = 0xff;
		if (y < 356)
			scale -= 2 * (356 - y);
	}

	if (scale > 255) {
			warning("Actor %d at %d, scale %d out of range", number, y, scale);
	}
	scalex = (byte)scale;
	scaley = (byte)scale;
}

void Actor::startAnimActor(int f) {
	if (_vm->_features & GF_NEW_COSTUMES) {
		switch (f) {
		case 1001:
			f = initFrame;
			break;
		case 1002:
			f = walkFrame;
			break;
		case 1003:
			f = standFrame;
			break;
		case 1004:
			f = talkFrame1;
			break;
		case 1005:
			f = talkFrame2;
			break;
		}

		frame = f;

		if (costume != 0) {
			animProgress = 0;
			needRedraw = true;
			needBgReset = true;
			if (f == initFrame)
				cost.reset();
			_vm->akos_decodeData(this, f, (uint) - 1);
		}

	} else {
		switch (f) {
		case 0x38:
			f = initFrame;
			break;
		case 0x39:
			f = walkFrame;
			break;
		case 0x3A:
			f = standFrame;
			break;
		case 0x3B:
			f = talkFrame1;
			break;
		case 0x3C:
			f = talkFrame2;
			break;
		}
		
		assert(f != 0x3E);
		frame = f;

		if (isInCurrentRoom() && costume != 0) {
			animProgress = 0;
			cost.animCounter1 = 0;
			needRedraw = true;
			if (f == initFrame)
				cost.reset();
			_vm->cost_decodeData(this, f, (uint) - 1);
		}

		needBgReset = true;
	}
}

void Actor::animateActor(int anim) {
	int cmd, dir;

	if (_vm->_features & GF_NEW_COSTUMES) {

		if (anim == 0xFF)
			anim = 2000;

		cmd = anim / 1000;
		dir = anim % 1000;

	} else {

		cmd = anim / 4;
		dir = oldDirToNewDir(anim % 4);

		// Convert into old cmd code
		cmd = 0x3F - cmd + 2;

	}

	switch (cmd) {
	case 2:				// stop walking
		stopActorMoving();
		startAnimActor(standFrame);
		break;
	case 3:				// change direction immediatly
		moving &= ~MF_TURN;
		setDirection(dir);
		break;
	case 4:				// turn to new direction
		turnToDirection(dir);
		break;
	default:
		startAnimActor(anim);
	}
}

void Actor::setDirection(int direction) {
	uint aMask;
	int i;
	uint16 vald;

	if (facing == direction)
		return;

	facing = normalizeAngle(direction);

	if (costume == 0)
		return;

	aMask = 0x8000;
	for (i = 0; i < 16; i++, aMask >>= 1) {
		vald = cost.frame[i];
		if (vald == 0xFFFF)
			continue;
		if (_vm->_features & GF_NEW_COSTUMES)
			_vm->akos_decodeData(this, vald, aMask);
		else
			_vm->cost_decodeData(this, vald, aMask);
	}

	needRedraw = true;
	needBgReset = true;
}

void Actor::putActor(int dstX, int dstY, byte newRoom) {
	if (visible && _vm->_currentRoom != newRoom && _vm->VAR(_vm->VAR_TALK_ACTOR) == number) {
		_vm->clearMsgQueue();
	}

	x = dstX;
	y = dstY;
	room = newRoom;
	needRedraw = true;
	needBgReset = true;

	if (_vm->VAR(_vm->VAR_EGO) == number) {
		_vm->_egoPositioned = true;
	}

	if (visible) {
		if (isInCurrentRoom()) {
			if (moving) {
				startAnimActor(standFrame);
				moving = 0;
			}
			adjustActorPos();
		} else {
			hideActor();
		}
	} else {
		if (isInCurrentRoom())
			showActor();
	}
}

int Actor::getActorXYPos(int &xPos, int &yPos) {
	if (!isInCurrentRoom())
		return -1;

	xPos = x;
	yPos = y;
	return 0;
}

AdjustBoxResult Actor::adjustXYToBeInBox(int dstX, int dstY) {
	const uint thresholdTable[] = { 30, 80, 0 };
	AdjustBoxResult abr, tmp;
	uint threshold;
	uint bestDist;
	int numBoxes;
	int box;
	byte flags, bestBox;
	const int firstValidBox = (_vm->_features & GF_SMALL_HEADER) ? 0 : 1;

	abr.x = dstX;
	abr.y = dstY;
	abr.dist = kInvalidBox;

	if (ignoreBoxes)
		return abr;

	for (int tIdx = 0; tIdx < ARRAYSIZE(thresholdTable); tIdx++) {
		threshold = thresholdTable[tIdx];

		numBoxes = _vm->getNumBoxes() - 1;
		if (numBoxes < firstValidBox)
			return abr;

		bestDist = (uint) 0xFFFF;
		bestBox = kInvalidBox;

		// We iterate (backwards) over all boxes, searching the one closest
		// to the desired coordinates.
		for (box = numBoxes; box >= firstValidBox; box--) {
			flags = _vm->getBoxFlags(box);

			// Skip over invisible boxes
			if (flags & kBoxInvisible && !(flags & kBoxPlayerOnly && !isInClass(kObjectClassPlayer)))
				continue;
			
			// For increased performance, we perform a quick test if
			// the coordinates can even be within a distance of 'threshold'
			// pixels of the box.
			if (!_vm->inBoxQuickReject(box, dstX, dstY, threshold))
				continue;

			// Check if the point is contained in the box. If it is,
			// we don't have to search anymore.
			if (_vm->checkXYInBoxBounds(box, dstX, dstY)) {
				abr.x = dstX;
				abr.y = dstY;
				abr.dist = box;
				return abr;
			}

			// Find the point in the box which is closest to our point.
			tmp = _vm->getClosestPtOnBox(box, dstX, dstY);

			// Check if the box is closer than the previous boxes.
			if (tmp.dist < bestDist) {
				abr.x = tmp.x;
				abr.y = tmp.y;
	
				if (tmp.dist == 0) {
					abr.dist = box;
					return abr;
				}
				bestDist = tmp.dist;
				bestBox = box;
			}
		}

		// If the closest ('best') box we found is within the threshold, or if
		// we are on the last run (i.e. threshold == 0), return that box.
		if (threshold == 0 || threshold * threshold >= bestDist) {
			abr.dist = bestBox;
			return abr;
		}
	}

	return abr;
}

void Actor::adjustActorPos() {
	AdjustBoxResult abr;

	abr = adjustXYToBeInBox(x, y);

	x = abr.x;
	y = abr.y;
	walkdata.destbox = (byte)abr.dist;

	setBox(abr.dist);

	walkdata.destx = -1;

	moving = 0;
	cost.animCounter2 = 0;

	if (_vm->_features & GF_NEW_COSTUMES) {
		stopActorMoving();
	}

	if (walkbox != kInvalidBox) {
		byte flags = _vm->getBoxFlags(walkbox);
		if (flags & 7) {
			turnToDirection(facing);
		}
	}
}

void Actor::faceToObject(int obj) {
	int x2, y2, dir;
	
	if (!isInCurrentRoom())
		return;

	if (_vm->getObjectOrActorXY(obj, x2, y2) == -1)
		return;

	dir = (x2 > x) ? 90 : 270;
	turnToDirection(dir);
}

void Actor::turnToDirection(int newdir) {
	if (newdir == -1)
		return;

	moving &= ~MF_TURN;

	if (newdir != facing) {
		moving |= MF_TURN;
		newDirection = newdir;
	}
}

void Actor::hideActor() {
	if (!visible)
		return;

	if (moving) {
		startAnimActor(standFrame);
		moving = 0;
	}
	visible = false;
	cost.animCounter2 = 0;
	needRedraw = false;
	needBgReset = true;
}

void Actor::showActor() {
	if (_vm->_currentRoom == 0 || visible)
		return;

	adjustActorPos();

	_vm->ensureResourceLoaded(rtCostume, costume);

	if (costumeNeedsInit) {
		startAnimActor(initFrame);
		if (_vm->_features & GF_AFTER_V2)
			startAnimActor(standFrame);
		costumeNeedsInit = false;
	}
	moving = 0;
	visible = true;
	needRedraw = true;
}

void Scumm::showActors() {
	int i;
	Actor *a;

	for (i = 1; i < _numActors; i++) {
		a = derefActor(i);
		if (a->isInCurrentRoom())
			a->showActor();
	}
}

void Scumm::walkActors() {
	int i;
	Actor *a;

	for (i = 1; i < _numActors; i++) {
		a = derefActor(i);
		if (a->isInCurrentRoom())
			if (_features & GF_AFTER_V2 || _features & GF_AFTER_V3)
				a->walkActorOld();
			else
				a->walkActor();
	}
}

/* Used in Scumm v5 only. Play sounds associated with actors */
void Scumm::playActorSounds() {
	int i;
	Actor *a;

	for (i = 1; i < _numActors; i++) {
		a = derefActor(i);
		if (a->cost.animCounter2 && a->isInCurrentRoom() && a->sound) {
			_currentScript = 0xFF;
			_sound->addSoundToQueue(a->sound[0]);
			for (i = 1; i < _numActors; i++) {
				a = derefActor(i);
				a->cost.animCounter2 = 0;
			}
			return;
		}
	}
}


#define DRAW_ORDER(x)	((x)->y - ((x)->layer << 11))

void Scumm::processActors() {
	int i;
	Actor **actors, *a, **ac, **ac2, *tmp, **end;
	int numactors = 0;

	actors = new Actor * [_numActors];
	
	// Make a list of all actors in this room
	for (i = 1; i < _numActors; i++) {
		a = derefActor(i);
		if ((_features & GF_AFTER_V8) && a->layer < 0)
			continue;
		if (a->isInCurrentRoom())
			actors[numactors++] = a;
	}
	if (!numactors) {
		delete [] actors;
		return;
	}

	end = actors + numactors;

	// Sort actors by position before we draw them (to ensure that actors in
	// front are drawn after those "behind" them).
	for (ac = end - 1; ac >= actors; --ac) {
		for (ac2 = actors; ac2 != ac; ++ac2) {
			if (DRAW_ORDER(*ac2) > DRAW_ORDER(*(ac2 + 1))) {
				tmp = *(ac2 + 1);
				*(ac2 + 1) = *ac2;
				*ac2 = tmp;
			}
		}
	}

	// Finally draw the now sorted actors
	for (ac = actors; ac != end; ++ac) {
		a = *ac;
		if (a->costume) {
			CHECK_HEAP
			a->drawActorCostume();
			CHECK_HEAP
			a->animateCostume();
		}
	}
	
	delete [] actors;
}

// Used in Scumm v8, to allow the verb coin to be drawn over the inventory
// chest. I'm assuming that draw order won't matter here.
void Scumm::processUpperActors() {
	Actor *a;
	int i;

	for (i = 1; i < _numActors; i++) {
		a = derefActor(i);
		if (a->isInCurrentRoom() && a->costume && a->layer < 0) {
			CHECK_HEAP
			a->drawActorCostume();
			CHECK_HEAP
			a->animateCostume();
		}
	}
}

void Actor::drawActorCostume() {
	if (!needRedraw)
		return;

	needRedraw = false;

	setupActorScale();

	BaseCostumeRenderer *bcr;

	if (_vm->_features & GF_NEW_COSTUMES)
		bcr = new AkosRenderer(_vm);
	else
		bcr = new CostumeRenderer(_vm);

	bcr->_actorX = x - _vm->virtscr[0].xstart;
	bcr->_actorY = y - elevation;
	bcr->_scaleX = scalex;
	bcr->_scaleY = scaley;

	bcr->_shadow_mode = shadow_mode;
	if (_vm->_features & GF_SMALL_HEADER)
		bcr->_shadow_table = NULL;
	else
		bcr->_shadow_table = _vm->_shadowPalette;

	bcr->setCostume(costume);
	bcr->setPalette(palette);
	bcr->setFacing(this);

	bcr->_dirty_id = number;

	if (!(_vm->_features & GF_NEW_COSTUMES)) {
		CostumeRenderer& cr = *(CostumeRenderer *)bcr;

		cr._outheight = _vm->virtscr[0].height;

		if (forceClip)
			cr._zbuf = forceClip;
		else if (isInClass(kObjectClassNeverClip))
			cr._zbuf = 0;
		else {
			cr._zbuf = _vm->getMaskFromBox(walkbox);
			if (cr._zbuf > _vm->gdi._numZBuffer)
				cr._zbuf = _vm->gdi._numZBuffer;
		}

		cr._draw_top = top = 0xFF;
		cr._draw_bottom = bottom = 0;

		// if the actor is partially hidden, redraw it next frame
		if (cr.drawCostume(cost) & 1) {
			needBgReset = true;
			needRedraw = true;
		}
	} else {
		AkosRenderer& ar = *(AkosRenderer *)bcr;

		ar._zbuf = forceClip;
		if (ar._zbuf == 100) {
			ar._zbuf = _vm->getMaskFromBox(walkbox);
			if (ar._zbuf > _vm->gdi._numZBuffer)
				ar._zbuf = _vm->gdi._numZBuffer;
		}

		ar.outptr = _vm->virtscr[0].screenPtr + _vm->virtscr[0].xstart;
		ar.outwidth = _vm->virtscr[0].width;
		ar.outheight = _vm->virtscr[0].height;

		ar._draw_top = top = 0x7fffffff;
		ar._draw_bottom = bottom = 0;

		if (ar.drawCostume(cost)) {
			// FIXME: this breaks talking in The Dig because the actor
			// is redrawn too often, thus breaking the waitForActor opcode.
			// Note that I originally added this to correct some redraw issues.
			// A "better" implementation would work like the corresponding code
			// for "old" costumes, that is, only trigger a redraw if the actor
			// is partially hidden / offscreen... but I am not sure the original
			// actually does this, so before we spend time on implementing this,
			// we should first figure out what the original code does here...
			//needBgReset = true;
			//needRedraw = true;
		}
	}

	top = bcr->_draw_top;
	bottom = bcr->_draw_bottom;
	
	delete bcr;
}

void Actor::animateCostume() {
	if (costume == 0)
		return;

	animProgress++;
	if (animProgress >= animSpeed) {
		animProgress = 0;

		if (_vm->_features & GF_NEW_COSTUMES) {
			byte *akos = _vm->getResourceAddress(rtCostume, costume);
			assert(akos);
			if (_vm->akos_increaseAnims(akos, this)) {
				needRedraw = true;
				needBgReset = true;
			}
		} else {
			LoadedCostume lc(_vm);
			lc.loadCostume(costume);
			if (lc.increaseAnims(this)) {
				needRedraw = true;
				needBgReset = true;
			}
		}
	}
}

void Actor::animateLimb(int limb, int f) {
	// This methods is very similiar to animateCostume(). 
	// However, instead of animating *all* the limbs, it only animates
	// the specified limb to be at the frame specified by "f". 

	if (!f)
		return;

	animProgress++;
	if (animProgress >= animSpeed) {
		animProgress = 0;

		if (costume == 0)
			return;

		byte *aksq, *akfo;
		uint size;
		byte *akos = _vm->getResourceAddress(rtCostume, costume);
		assert(akos);

		aksq = _vm->findResourceData(MKID('AKSQ'), akos);
		akfo = _vm->findResourceData(MKID('AKFO'), akos);
	
		size = _vm->getResourceDataSize(akfo) >> 1;
	
		while (f--) {
			if (cost.active[limb] != 0)
				_vm->akos_increaseAnim(this, limb, aksq, (uint16 *)akfo, size);
		}

//		needRedraw = true;
//		needBgReset = true;
	}
}

void Scumm::setActorRedrawFlags(bool fg, bool bg) {
	int i, j;

	if (_fullRedraw) {
		for (j = 1; j < _numActors; j++) {
			Actor *a = derefActor(j);
			a->needRedraw |= fg;
			a->needBgReset |= bg;
		}
	} else {
		for (i = 0; i < gdi._numStrips; i++) {
			int strip = _screenStartStrip + i;
			if (testGfxAnyUsageBits(strip)) {
				for (j = 1; j < _numActors; j++) {
					if (testGfxUsageBit(strip, j) && testGfxOtherUsageBits(strip, j)) {
						Actor *a = derefActor(j);
						assert(a->number == j);
						a->needRedraw |= fg;
						a->needBgReset |= bg;
					}
				}
			}
		}
	}
}

int Scumm::getActorFromPos(int x, int y) {
	int i;

	if (!testGfxAnyUsageBits(x >> 3))
		return 0;
	for (i = 1; i < _numActors; i++) {
		Actor *a = derefActor(i);
		assert(a->number == i);
		if (testGfxUsageBit(x >> 3, i) && !getClass(i, kObjectClassUntouchable)
			&& y >= a->top && y <= a->bottom) {
			return i;
		}
	}
	return 0;
}

void Scumm::actorTalk() {
	Actor *a;

	_msgPtrToAdd = _charsetBuffer;
	_messagePtr = addMessageToStack(_messagePtr);
	assert((int)(_msgPtrToAdd - _charsetBuffer) < (int)(sizeof(_charsetBuffer)));

	if (_actorToPrintStrFor == 0xFF) {
		if (!_keepText)
			stopTalk();
		VAR(VAR_TALK_ACTOR) = 0xFF;
	} else {
		int oldact;
		a = derefActorSafe(_actorToPrintStrFor, "actorTalk");
		assert(a);
		if (!a->isInCurrentRoom() && !(_features & GF_NEW_COSTUMES)) {
			oldact = 0xFF;
		} else {
			if (!_keepText)
				stopTalk();
			VAR(VAR_TALK_ACTOR) = a->number;
			if (!_string[0].no_talk_anim) {
				a->startAnimActor(a->talkFrame1);
				_useTalkAnims = true;
			}
			oldact = VAR(VAR_TALK_ACTOR);
		}
		if (oldact >= 0x80)
			return;
	}

	if (VAR(VAR_TALK_ACTOR) > 0x7F) {
		_charsetColor = (byte)_string[0].color;
	} else {
		a = derefActorSafe(VAR(VAR_TALK_ACTOR), "actorTalk(2)");
		_charsetColor = a->talkColor;
	}
	_charsetBufPos = 0;
	_talkDelay = 0;
	_haveMsg = 0xFF;
	VAR(VAR_HAVE_MSG) = 0xFF;
	if (VAR_CHARCOUNT != 0xFF)
		VAR(VAR_CHARCOUNT) = 0;
	CHARSET_1();
}

void Scumm::stopTalk() {
	int act;

	_sound->stopTalkSound();

	_haveMsg = 0;
	_talkDelay = 0;

	act = VAR(VAR_TALK_ACTOR);
	if (act && act < 0x80) {
		Actor *a = derefActorSafe(act, "stopTalk");
		if ((a->isInCurrentRoom() && _useTalkAnims) || (_features & GF_NEW_COSTUMES)) {
			a->startAnimActor(a->talkFrame2);
			_useTalkAnims = false;
		}
		VAR(VAR_TALK_ACTOR) = 0xFF;
	}
	_keepText = false;
	restoreCharsetBg();
}

void Scumm::clearMsgQueue() {
	_messagePtr = (const byte *)" ";
	stopTalk();
}

void Actor::setActorCostume(int c) {
	int i;

	costumeNeedsInit = true;

	if (visible) {
		hideActor();
		cost.reset();
		costume = c;
		showActor();
	} else {
		costume = c;
		cost.reset();
	}

	if (_vm->_features & GF_NEW_COSTUMES) {
		for (i = 0; i < 256; i++)
			palette[i] = 0xFF;
	} else if (_vm->_features & GF_OLD_BUNDLE) {
		for (i = 0; i < 16; i++)
			palette[i] = i;
	} else {
		for (i = 0; i < 32; i++)
			palette[i] = 0xFF;
	}

}

void Actor::startWalkActor(int destX, int destY, int dir) {
	AdjustBoxResult abr;

	if (_vm->_features & GF_AFTER_V2 || _vm->_features & GF_AFTER_V3) {
		abr.x = destX;
		abr.y = destY;
	} else {
		abr = adjustXYToBeInBox(destX, destY);
	}

	if (!isInCurrentRoom()) {
		x = abr.x;
		y = abr.y;
		if (dir != -1)
			setDirection(dir);
		return;
	}

	if (ignoreBoxes) {
		abr.dist = kInvalidBox;
		walkbox = kInvalidBox;
	} else {
		if (_vm->checkXYInBoxBounds(walkdata.destbox, abr.x, abr.y)) {
			abr.dist = walkdata.destbox;
		} else {
			abr = adjustXYToBeInBox(abr.x, abr.y);
		}
		if (moving && walkdata.destdir == dir && walkdata.destx == abr.x && walkdata.desty == abr.y)
			return;
	}

	if (x == abr.x && y == abr.y) {
		turnToDirection(dir);
		return;
	}

	walkdata.destx = abr.x;
	walkdata.desty = abr.y;
	walkdata.destbox = (byte)abr.dist;	/* a box */
	walkdata.destdir = dir;
	moving = (moving & MF_IN_LEG) | MF_NEW_LEG;
	walkdata.point3x = 32000;

	walkdata.curbox = walkbox;
}

void Actor::startWalkAnim(int cmd, int angle) {
	if (angle == -1)
		angle = facing;

	/* FIXME: (yazoo/fingolfin): using the walk script is buggy in Dig,
	 * troubles while walking. It's disabled until we can figure out how
	 * to fix this properly.
	 * Note: walk scripts aren't required to make the game
	 * work as usual
	 */
#if 1
	if (walk_script != 0) {
		int args[16];
		args[0] = number;
		args[1] = cmd;
		args[2] = angle;
		_vm->runScript(walk_script, 1, 0, args);
	} else
#endif
	{
		switch (cmd) {
		case 1:										/* start walk */
			setDirection(angle);
			startAnimActor(walkFrame);
			break;
		case 2:										/* change dir only */
			setDirection(angle);
			break;
		case 3:										/* stop walk */
			turnToDirection(angle);
			startAnimActor(standFrame);
			break;
		}
	}
}

void Actor::walkActor() {
	int new_dir, box;
	int16 foundPathX, foundPathY;

	if (_vm->_features & GF_AFTER_V7) {
		// FIXME - this is kind of a hack right now but it fixes the
		// walk scripts in The Dig.
		if (moving & MF_FROZEN) {
			if (moving & MF_TURN) {
				new_dir = updateActorDirection(false);
				if (facing != new_dir)
					setDirection(new_dir);
				else
					moving &= ~MF_TURN;
			}
			return;
		}
	}

	if (!moving)
		return;

	if (!(moving & MF_NEW_LEG)) {
		if (moving & MF_IN_LEG && actorWalkStep())
			return;

		if (moving & MF_LAST_LEG) {
			moving = 0;
			setBox(walkdata.destbox);
			startWalkAnim(3, walkdata.destdir);
			return;
		}

		if (moving & MF_TURN) {
			new_dir = updateActorDirection(false);
			if (facing != new_dir)
				setDirection(new_dir);
			else
				moving = 0;
			return;
		}

		setBox(walkdata.curbox);
		moving &= MF_IN_LEG;
	}

	do {
		moving &= ~MF_NEW_LEG;

		if (walkbox == kInvalidBox) {
			setBox(walkdata.destbox);
			walkdata.curbox = walkdata.destbox;
			break;
		}

		if (walkbox == walkdata.destbox)
			break;

		box = _vm->getPathToDestBox(walkbox, walkdata.destbox);
		if (box < 0) {
			walkdata.destbox = walkbox;
			moving |= MF_LAST_LEG;
			return;
		}

		walkdata.curbox = box;
		
		if (_vm->findPathTowards(this, walkbox, box, walkdata.destbox, foundPathX, foundPathY))
			break;

		if (calcMovementFactor(foundPathX, foundPathY))
			return;

		setBox(walkdata.curbox);
	} while (1);

	moving |= MF_LAST_LEG;
	calcMovementFactor(walkdata.destx, walkdata.desty);
}

void Actor::walkActorOld() {
	ScummVM::Point gateLoc[5];	// Gate locations
	int new_dir, next_box;

	if (!moving)
		return;

	if (!(moving & MF_NEW_LEG)) {
		if (moving & MF_IN_LEG && actorWalkStep())
			return;
	
		if (moving & MF_LAST_LEG) {
			moving = 0;
			startWalkAnim(3, walkdata.destdir);
			return;
		}
	
		if (moving & MF_TURN) {
			new_dir = updateActorDirection(false);
			if (facing != new_dir)
				setDirection(new_dir);
			else
				moving = 0;
			return;
		}
	
		if (walkdata.point3x != 32000) {
			if (calcMovementFactor(walkdata.point3x, walkdata.point3y)) {
				walkdata.point3x = 32000;
				return;
			}
			walkdata.point3x = 32000;
		}
	
		walkbox = walkdata.curbox;
		moving &= MF_IN_LEG;
	}

	do {
		moving &= ~MF_NEW_LEG;

		if (walkbox == kInvalidBox) {
			walkbox = walkdata.destbox;
			walkdata.curbox = walkdata.destbox;
			break;
		}

		if (walkbox == walkdata.destbox)
			break;

		next_box = _vm->getPathToDestBox(walkbox, walkdata.destbox);

		if (next_box < 0) {
			moving |= MF_LAST_LEG;
			return;
		}

		// FIXME: not sure if this is needed in non-Zak games, but I think it shouldn't
		// hurt there either.
		int flags = _vm->getBoxFlags(next_box);
		if (flags & kBoxLocked && !(flags & kBoxPlayerOnly && !isInClass(kObjectClassPlayer))) {
			moving |= MF_LAST_LEG;
			return;
		}


		walkdata.curbox = next_box;

		_vm->findPathTowardsOld(this, walkbox, next_box, walkdata.destbox, gateLoc);
		if (gateLoc[2].x == 32000 && gateLoc[3].x == 32000) {
			break;
		}

		if (gateLoc[2].x != 32000) {
			if (calcMovementFactor(gateLoc[2].x, gateLoc[2].y)) {
				walkdata.point3x = gateLoc[3].x; 
				walkdata.point3y = gateLoc[3].y;
				return;
			}
		}

		if (calcMovementFactor(gateLoc[3].x, gateLoc[3].y))
			return;

		walkbox = walkdata.destbox;
	} while(1);

	moving |= MF_LAST_LEG;
	calcMovementFactor(walkdata.destx, walkdata.desty);
}

byte *Actor::getActorName() {
	byte *ptr = _vm->getResourceAddress(rtActorName, number);
	if (ptr == NULL) {
		warning("Failed to find name of actor %d\n", number);
	}
	return ptr;
}

void Actor::remapActorPalette(int r_fact, int g_fact, int b_fact, int threshold) {
	byte *akos, *rgbs, *akpl;
	int akpl_size, i;
	int r, g, b;
	byte akpl_color;

	if (!isInCurrentRoom()) {
		warning("Remap actor %d not in current room", number);
		return;
	}

	if (costume < 1 || costume >= _vm->_numCostumes - 1) {
		warning("Remap actor %d invalid costume %d", number, costume);
		return;
	}

	akos = _vm->getResourceAddress(rtCostume, costume);
	if (!akos) {
		warning("Can't remap actor %d, costume %d not found", number, costume);
		return;
	}

	akpl = findResource(MKID('AKPL'), akos);
	if (!akpl) {
		warning("Can't remap actor %d, costume %d doesn't contain an AKPL block", number, costume);
		return;
	}

	//get num palette entries
	akpl_size = RES_SIZE(akpl) - 8;

	//skip resource header
	akpl = RES_DATA(akpl);

	rgbs = findResource(MKID('RGBS'), akos);

	if (!rgbs) {
		warning("Can't remap actor %d costume %d doesn't contain an RGB block", number, costume);
		return;
	}
	// skip resource header
	rgbs = RES_DATA(rgbs);

	for (i = 0; i < akpl_size; i++) {
		r = *rgbs++;
		g = *rgbs++;
		b = *rgbs++;

		akpl_color = *akpl++;

		// allow remap of generic palette entry?
		if (!shadow_mode || akpl_color >= 16) {
			if (r_fact != 256)
				r = (r * r_fact) >> 8;
			if (g_fact != 256)
				g = (g * g_fact) >> 8;
			if (b_fact != 256)
				b = (b * b_fact) >> 8;
			palette[i] = _vm->remapPaletteColor(r, g, b, threshold);
		}
	}
}

void Scumm::resetActorBgs() {
	Actor *a;
	int i, j;

	for (i = 0; i < gdi._numStrips; i++) {
		int strip = _screenStartStrip + i;
		for (j = 1; j < _numActors; j++) {
			a = derefActor(j);
			if (testGfxUsageBit(strip, j) && a->top != 0xFF && a->needBgReset) {
				clearGfxUsageBit(strip, j);
				if ((a->bottom - a->top) >= 0)
					gdi.resetBackground(a->top, a->bottom, i);
			}
		}
	}

	for (i = 1; i < _numActors; i++) {
		a = derefActor(i);
		a->needBgReset = false;
	}
}

void Actor::classChanged(int cls, bool value) {
	if (cls == kObjectClassAlwaysClip)
		forceClip = value;
	if (cls == kObjectClassIgnoreBoxes)
		ignoreBoxes = value;
}

bool Actor::isInClass(int cls) {
	return _vm->getClass(number, cls);
}
