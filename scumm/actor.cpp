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
	talkPosX = -80;
	talkPosY = 0;
	scaley = scalex = 0xFF;
	charset = 0;
	memset(sound, 0, sizeof(sound));
	newDirection = 0;

	stopActorMoving();

	shadow_mode = 0;
	layer = 0;

	setActorWalkSpeed(8, 2);
	animSpeed = 0;

	ignoreBoxes = 0;
	forceClip = 0;
	ignoreTurns = false;
	
	initFrame = 1;
	walkFrame = 2;
	standFrame = 3;
	talkFrame1 = 4;
	talkFrame2 = 5;

	walk_script = 0;
	talk_script = 0;

	if (_vm) {
		_vm->_classData[number] = (_vm->_features & GF_AFTER_V7) ? _vm->_classData[0] : 0;
	}
}

void Actor::stopActorMoving() {
	if (_vm)
		_vm->stopScriptNr(walk_script);
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
	int32 XYFactor, YXFactor;

	actorX = x;
	actorY = y;

	if (actorX == newX && actorY == newY)
		return 0;

	diffX = newX - actorX;
	diffY = newY - actorY;
	YXFactor = speedy << 16;

	if (diffY < 0)
		YXFactor = -YXFactor;

	XYFactor = YXFactor * diffX;
	if (diffY != 0) {
		XYFactor /= diffY;
	} else {
		YXFactor = 0;
	}

	if ((uint) abs((int)(XYFactor >> 16)) > speedx) {
		XYFactor = speedx << 16;
		if (diffX < 0)
			XYFactor = -XYFactor;

		YXFactor = XYFactor * diffY;
		if (diffX != 0) {
			YXFactor /= diffX;
		} else {
			XYFactor = 0;
		}
	}

	walkdata.x = actorX;
	walkdata.y = actorY;
	walkdata.newx = newX;
	walkdata.newy = newY;
	walkdata.XYFactor = XYFactor;
	walkdata.YXFactor = YXFactor;
	walkdata.xfrac = 0;
	walkdata.yfrac = 0;

	newDirection = _vm->getAngleFromPos(XYFactor, YXFactor);

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
				// FIXME - I am not 100% if this code is right (Fingolfin)
				warning("remapDirection: special dir");

				specdir = specdir & 0x3FFF;
				if (specdir - 90 < dir && dir < specdir + 90)
					dir = specdir;
				else
					dir = specdir + 180;
			}
		}

		flags = _vm->getBoxFlags(walkbox);

		flipX = (walkdata.XYFactor > 0);
		flipY = (walkdata.YXFactor > 0);

		// FIXME - this special cases for the class might be necesary
		// for other games besides Loom!

		// Check for X-Flip
		if ((flags & kBoxXFlip) || isInClass((_vm->_gameId == GID_LOOM256 || _vm->_gameId == GID_LOOM) ? 19 : 30)) {
			dir = 360 - dir;
			flipX = !flipX;
		}
		// Check for Y-Flip
		if ((flags & kBoxYFlip) || isInClass((_vm->_gameId == GID_LOOM256 || _vm->_gameId == GID_LOOM) ? 18 : 29)) {
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

	tmpX = ((actorX + 8000) << 16) + walkdata.xfrac + (walkdata.XYFactor >> 8) * scalex;
	walkdata.xfrac = (uint16)tmpX;
	actorX = (tmpX >> 16) - 8000;

	tmpY = (actorY << 16) + walkdata.yfrac + (walkdata.YXFactor >> 8) * scalex;
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

	if (ignoreBoxes != 0)
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
	if (visible && _vm->_currentRoom != newRoom && _vm->_vars[_vm->VAR_TALK_ACTOR] == number) {
		_vm->clearMsgQueue();
	}

	x = dstX;
	y = dstY;
	room = newRoom;
	needRedraw = true;
	needBgReset = true;

	if (_vm->_vars[_vm->VAR_EGO] == number) {
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

AdjustBoxResult Actor::adjustXYToBeInBox(int dstX, int dstY, int pathfrom) {
	AdjustBoxResult abr, tmp;
	uint threshold;
	uint best;
	int box, iterations = 0;			/* Use iterations for those odd times we get stuck in the loop */
	int firstValidBox, i, j;
	byte flags, b;

	if (_vm->_features & GF_SMALL_HEADER)
		firstValidBox = 0;
	else
		firstValidBox = 1;

	abr.x = dstX;
	abr.y = dstY;
	abr.dist = 0;

	if (ignoreBoxes == 0) {
		threshold = 30;

		while (1) {
			iterations++;
			if (iterations > 1000)
				return abr;							/* Safety net */
			box = _vm->getNumBoxes() - 1;
			if (box < firstValidBox)
				return abr;

			best = (uint) 0xFFFF;
			b = 0;

// FIXME - why was that check here? It apparently causes bug #643001
//			if (!(_vm->_features & GF_OLD256) || box)
			for (j = box; j >= firstValidBox; j--) {
				flags = _vm->getBoxFlags(j);

				if (flags & kBoxInvisible && (!(flags & kBoxPlayerOnly) || isInClass(31)))
					continue;
				
				if (pathfrom >= firstValidBox) {

					if (flags & kBoxLocked && (!(flags & kBoxPlayerOnly)))
						continue;

					i = _vm->getPathToDestBox(pathfrom, j);
					if (i == -1)
						continue;

					if (_vm->_features & GF_OLD256) {
						// FIXME - we check here if the box suggested by getPathToDestBox
						// is locked or not. This prevents us from walking thru
						// closed doors in some cases in Zak256. However a better fix
						// would be to recompute the box matrix whenever flags change.
						flags = _vm->getBoxFlags(i);
						if (flags & kBoxLocked && (!(flags & kBoxPlayerOnly)))
							continue;
						if (flags & kBoxInvisible && (!(flags & kBoxPlayerOnly) || isInClass(31)))
							continue;
					}
				}

				if (!_vm->inBoxQuickReject(j, dstX, dstY, threshold))
					continue;

				if (_vm->checkXYInBoxBounds(j, dstX, dstY)) {
					abr.x = dstX;
					abr.y = dstY;
					abr.dist = j;
					return abr;
				}

				tmp = _vm->getClosestPtOnBox(j, dstX, dstY);

				if (tmp.dist >= best)
					continue;

				abr.x = tmp.x;
				abr.y = tmp.y;

				if (tmp.dist == 0) {
					abr.dist = j;
					return abr;
				}
				best = tmp.dist;
				b = j;
			}

			if (threshold == 0 || threshold * threshold >= best) {
				abr.dist = b;
				return abr;
			}
			threshold = (threshold == 30) ? 80 : 0;
		}
	}

	return abr;
}

void Actor::adjustActorPos() {
	AdjustBoxResult abr;
	byte flags;

	abr = adjustXYToBeInBox(x, y, -1);

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

	flags = _vm->getBoxFlags(walkbox);
	if (flags & 7) {
		turnToDirection(facing);
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
		costumeNeedsInit = false;
	}
	moving = 0;
	visible = true;
	needRedraw = true;
}

void Scumm::showActors() {
	int i;
	Actor *a;

	for (i = 1; i < NUM_ACTORS; i++) {
		a = derefActor(i);
		if (a->isInCurrentRoom())
			a->showActor();
	}
}

void Scumm::stopTalk() {
	int act;

	_sound->stopTalkSound();

	_haveMsg = 0;
	_talkDelay = 0;

	act = _vars[VAR_TALK_ACTOR];
	if (act && act < 0x80) {
		Actor *a = derefActorSafe(act, "stopTalk");
		if ((a->isInCurrentRoom() && _useTalkAnims) || (_features & GF_NEW_COSTUMES)) {
			a->startAnimActor(a->talkFrame2);
			_useTalkAnims = false;
		}
		_vars[VAR_TALK_ACTOR] = 0xFF;
	}
	_keepText = false;
	restoreCharsetBg();
}

void Scumm::clearMsgQueue() {
	_messagePtr = (byte *)" ";
	stopTalk();
}

void Scumm::walkActors() {
	int i;
	Actor *a;

	for (i = 1; i < NUM_ACTORS; i++) {
		a = derefActor(i);
		if (a->isInCurrentRoom())
			if (_features & GF_AFTER_V3)
				a->walkActorOld();
			else
				a->walkActor();
	}
}

/* Used in Scumm v5 only. Play sounds associated with actors */
void Scumm::playActorSounds() {
	int i;
	Actor *a;

	for (i = 1; i < NUM_ACTORS; i++) {
		a = derefActor(i);
		if (a->cost.animCounter2 && a->isInCurrentRoom() && a->sound) {
			_currentScript = 0xFF;
			_sound->addSoundToQueue(a->sound[0]);
			for (i = 1; i < NUM_ACTORS; i++) {
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

	actors = new Actor * [NUM_ACTORS];
	
	// Make a list of all actors in this room
	for (i = 1; i < NUM_ACTORS; i++) {
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
			CHECK_HEAP getMaskFromBox(a->walkbox);
			a->drawActorCostume();
			CHECK_HEAP a->animateCostume();
		}
	}
	
	delete [] actors;
}

// Used in Scumm v8, to allow the verb coin to be drawn over the inventory
// chest. I'm assuming that draw order won't matter here.
void Scumm::processUpperActors() {
	Actor *a;
	int i;

	for (i = 1; i < NUM_ACTORS; i++) {
		a = derefActor(i);
		if (a->isInCurrentRoom() && a->costume && a->layer < 0) {
			CHECK_HEAP getMaskFromBox(a->walkbox);
			a->drawActorCostume();
			CHECK_HEAP a->animateCostume();
		}
	}
}

void Actor::drawActorCostume() {
	if (!needRedraw)
		return;

	needRedraw = false;

	setupActorScale();

	if (!(_vm->_features & GF_NEW_COSTUMES)) {
		CostumeRenderer cr(_vm);

		cr._actorX = x - _vm->virtscr[0].xstart;
		cr._actorY = y - elevation;
		cr._scaleX = scalex;
		cr._scaleY = scaley;

		cr._outheight = _vm->virtscr[0].height;

		// FIXME - Hack to fix two glitches in the scene where Bobbin
		// heals Rusty: Bobbin's feet get masked when Rusty shows him
		// what happens to The Forge, and Rusty gets masked after
		// Bobbin heals him. (Room 34)
		//
		// It also fixes a much less noticable glitch when Bobbin
		// jumps out of Mandible's cage. (Room 43)
		//
		// When an actor is moved around without regards to walkboxes,
		// its walkbox is set to 0. Unfortunately that's a valid
		// walkbox in older games, and its mask may be completely
		// wrong for this purpose.
		//
		// So instead use the mask of the box where the actor happens
		// to be at the moment or, if it's not in any box, don't mask
		// at all.
		//
		// This is similar to the _zbuf == 100 check used for AKOS
		// costumes, except I haven't been able to figure out the
		// proper check here. It's not quite enough to check if
		// ignoreBoxes != 0 and checking if walkbox == 0 yields too
		// many false positives, e.g. Bobbin leaving the sandy beach
		// towards the forest, or Stoke leaving the room where he
		// locks up "Rusty".
		//
		// Until someone can find the proper fix, only apply it to the
		// rooms where it's actually known to be needed.

		if (_vm->_gameId == GID_LOOM256 && (_vm->_currentRoom == 34 || _vm->_currentRoom == 43) && walkbox == 0) {
			int num_boxes, i;

			cr._zbuf = 0;
			num_boxes = _vm->getNumBoxes();

			// Sometimes boxes overlap, so the direction of this
			// loop matters in some rooms.

			for (i = 0; i < num_boxes; i++) {
				if (_vm->checkXYInBoxBounds(i, x, y)) {
					cr._zbuf = _vm->getMaskFromBox(i);
					break;
				}
			}
		} else
			cr._zbuf = _vm->getMaskFromBox(walkbox);

		if (forceClip)
			cr._zbuf = forceClip;
		else if (isInClass(20))
			cr._zbuf = 0;
		else if (cr._zbuf > _vm->gdi._numZBuffer)
			cr._zbuf = (byte)_vm->gdi._numZBuffer;

		cr._shadow_mode = shadow_mode;
		cr._shadow_table = _vm->_shadowPalette;

		cr.setCostume(costume);
		cr.setPalette(palette);
		cr.setFacing(this);

		cr._draw_top = top = 0xFF;
		cr._draw_bottom = bottom = 0;

		cr._dirty_id = number;

		/* if the actor is partially hidden, redraw it next frame */
		if (cr.drawCostume(cost) & 1) {
			needBgReset = true;
			needRedraw = true;
		}
		top = cr._draw_top;
		bottom = cr._draw_bottom;
	} else {
		AkosRenderer ar(_vm);
		ar.charsetmask = true;
		ar._actorX = x - _vm->virtscr[0].xstart;
		ar._actorY = y - elevation;
		ar._scaleX = scalex;
		ar._scaleY = scaley;
		ar._zbuf = forceClip;
		if (ar._zbuf == 100) {
			ar._zbuf = _vm->getMaskFromBox(walkbox);
			if (ar._zbuf > _vm->gdi._numZBuffer)
				ar._zbuf = _vm->gdi._numZBuffer;
		}

		ar.outptr = _vm->virtscr[0].screenPtr + _vm->virtscr[0].xstart;
		ar.outwidth = _vm->virtscr[0].width;
		ar.outheight = _vm->virtscr[0].height;

		ar._shadow_mode = shadow_mode;
		ar._shadow_table = _vm->_shadowPalette;

		ar.setCostume(costume);
		ar.setPalette(palette);
		ar.setFacing(this);

		ar._draw_top = top = 0x7fffffff;
		ar._draw_bottom = bottom = 0;

		ar._dirty_id = number;

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
		top = ar._draw_top;
		bottom = ar._draw_bottom;
	}
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
		for (j = 1; j < NUM_ACTORS; j++) {
			Actor *a = derefActor(j);
			a->needRedraw |= fg;
			a->needBgReset |= bg;
		}
	} else {
		for (i = 0; i < gdi._numStrips; i++) {
			int strip = _screenStartStrip + i;
			if (testGfxAnyUsageBits(strip)) {
				for (j = 1; j < NUM_ACTORS; j++) {
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
	for (i = 1; i < NUM_ACTORS; i++) {
		Actor *a = derefActor(i);
		assert(a->number == i);
		if (testGfxUsageBit(x >> 3, i) && !getClass(i, 32) && y >= a->top && y <= a->bottom) {
			return i;
		}
	}
	return 0;
}

void Scumm::actorTalk() {
	int oldact;
	Actor *a;

	_msgPtrToAdd = _charsetBuffer;
	_messagePtr = addMessageToStack(_messagePtr);
	assert((int)(_msgPtrToAdd - _charsetBuffer) < (int)(sizeof(_charsetBuffer)));

	if (_actorToPrintStrFor == 0xFF) {
		if (!_keepText)
			stopTalk();
		_vars[VAR_TALK_ACTOR] = 0xFF;
		oldact = 0;
	} else {
		a = derefActorSafe(_actorToPrintStrFor, "actorTalk");
		if (!a->isInCurrentRoom() && !(_features & GF_NEW_COSTUMES)) {
			oldact = 0xFF;
		} else {
			if (!_keepText)
				stopTalk();
			_vars[VAR_TALK_ACTOR] = a->number;
			if (!_string[0].no_talk_anim) {
				a->startAnimActor(a->talkFrame1);
				_useTalkAnims = true;
			}
			oldact = _vars[VAR_TALK_ACTOR];
		}
	}
	if (oldact >= 0x80)
		return;

	if (_vars[VAR_TALK_ACTOR] > 0x7F) {
		_charsetColor = (byte)_string[0].color;
	} else {
		a = derefActorSafe(_vars[VAR_TALK_ACTOR], "actorTalk(2)");
		_charsetColor = a->talkColor;
	}
	_charsetBufPos = 0;
	_talkDelay = 0;
	_haveMsg = 0xFF;
	_vars[VAR_HAVE_MSG] = 0xFF;
	CHARSET_1();
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

	abr = adjustXYToBeInBox(destX, destY, walkbox);

	if (!isInCurrentRoom()) {
		x = abr.x;
		y = abr.y;
		if (dir != -1)
			setDirection(dir);
		return;
	}

	if (ignoreBoxes != 0) {
		abr.dist = 0;
		walkbox = 0;
	} else {
		// FIXME: this prevents part of bug #605970 (Loom) from
		// occuring, and also fixes a walk bug with Rusty's ghost.
		// Not sure if there is a better way to achieve this.
		if (walkbox == 0)
			adjustActorPos();

		if (_vm->checkXYInBoxBounds(walkdata.destbox, abr.x, abr.y)) {
			abr.dist = walkdata.destbox;
		} else {
			abr = adjustXYToBeInBox(abr.x, abr.y, walkbox);
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

	if (moving == 0)
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
		if ((!walkbox && (!(_vm->_features & GF_SMALL_HEADER)))) {
			setBox(walkdata.destbox);
			walkdata.curbox = walkdata.destbox;
			break;
		}
		if (walkbox == walkdata.destbox)
			break;
		box = _vm->getPathToDestBox(walkbox, walkdata.destbox);
		if (box == -1 || box > 0xF0) {
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
	ScummPoint gateLoc[5];	// Gate locations
	int new_dir, next_box;

	if (!moving)
		return;

	if (moving & MF_NEW_LEG) {
	restart:
		moving &= ~MF_NEW_LEG;

		if (walkbox == 0xFF) {
			walkbox = walkdata.destbox;
			walkdata.curbox = walkdata.destbox;
			moving |= MF_LAST_LEG;
			calcMovementFactor(walkdata.destx, walkdata.desty);
			return;
		}

		if (walkbox == walkdata.destbox) {
			moving |= MF_LAST_LEG;
			calcMovementFactor(walkdata.destx, walkdata.desty);
			return;
		}

		next_box = _vm->getPathToDestBox(walkbox, walkdata.destbox);

		if (next_box == -1) {
			moving |= MF_LAST_LEG;
			return;
		}

		walkdata.curbox = next_box;

		_vm->findPathTowardsOld(this, walkbox, next_box, walkdata.destbox, gateLoc);
		if (gateLoc[2].x == 32000 && gateLoc[3].x == 32000) {
			moving |= MF_LAST_LEG;
			calcMovementFactor(walkdata.destx, walkdata.desty);
			return;
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
		goto restart;

	}

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
	moving |= MF_NEW_LEG;
	goto restart;
}

byte *Actor::getActorName() {
	byte *ptr = _vm->getResourceAddress(rtActorName, number);
	if (ptr == NULL)
		return (byte *)" ";
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
		warning("Remap actor %d invalid costume", number, costume);
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

	// FIXME!!! - Ender's hack to workaround a crash in Full Throttle.
	//	      After touching the ladder at the Fuel Tower, memory 
	//	      gets trashed when the police fly down to the tower.
	//	      This seems to happen with an akpl 256 supposidly 256
	//	      bytes long, but which in reality is less than 65 bytes?!?
	//		Someone -please- fix this properly.
	if (akpl_size > 255) {
		warning("Actor palette for actor %d/costume %d is %d bytes - too big!", number, costume, akpl_size);
		return;
	}

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
		for (j = 1; j < NUM_ACTORS; j++) {
			a = derefActor(j);
			if (testGfxUsageBit(strip, j) && a->top != 0xFF && a->needBgReset) {
				clearGfxUsageBit(strip, j);
				if ((a->bottom - a->top) >= 0)
					gdi.resetBackground(a->top, a->bottom, i);
			}
		}
	}

	for (i = 1; i < NUM_ACTORS; i++) {
		a = derefActor(i);
		a->needBgReset = false;
	}
}

void Actor::classChanged(int cls, bool value) {
	switch(cls) {
	case 20:	// Never clip
		break;
	case 21:	// Always clip
		forceClip = value;
		break;
	case 22:	// Ignore boxes
		ignoreBoxes = value;
		break;
	case 29:	// Y flip
		break;
	case 30:	// X flip
		break;
	case 31:	// ??
		break;
	}
}

bool Actor::isInClass(int cls) {
	return _vm->getClass(number, cls);
}
