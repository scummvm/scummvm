/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
#include "costume.h"
#include "resource.h"
#include "scumm/sound.h"

#include <math.h>

void Actor::initActor(int mode)
{
	if (mode == 1) {
		costume = 0;
		room = 0;
		x = 0;
		y = 0;
		facing = 180;
		newDirection = 180;
	} else if (mode == 2) {
		facing = 180;
		newDirection = 180;
	}

	elevation = 0;
	width = 24;
	talkColor = 15;
	new_2 = 0;
	new_1 = -80;
	scaley = scalex = 0xFF;
	charset = 0;
	sound[0] = 0;
	sound[1] = 0;
	sound[2] = 0;
	sound[3] = 0;
	sound[4] = 0;
	sound[5] = 0;
	sound[6] = 0;
	sound[7] = 0;
	newDirection = 0;

	stopActorMoving();

	shadow_mode = 0;
	layer = 0;

	setActorWalkSpeed(8, 2);

	ignoreBoxes = 0;
	forceClip = 0;
	new_3 = 0;
	initFrame = 1;
	walkFrame = 2;
	standFrame = 3;
	talkFrame1 = 4;
	talkFrame2 = 5;

	walk_script = 0;
	talk_script = 0;

	if (_vm->_features & GF_AFTER_V7) {
		_vm->_classData[number] = _vm->_classData[0];
	} else {
		_vm->_classData[number] = 0;
	}
}

void Actor::stopActorMoving()
{
	_vm->stopScriptNr(walk_script);
	moving = 0;
}

void Actor::setActorWalkSpeed(uint newSpeedX, uint newSpeedY)
{
	if (newSpeedX == speedx && newSpeedY == speedy)
		return;

	speedx = newSpeedX;
	speedy = newSpeedY;

	if (moving) {
		calcMovementFactor(walkdata.newx, walkdata.newy);
	}
}

int Scumm::getAngleFromPos(int x, int y)
{
	if (_gameId == GID_DIG) {
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

int Actor::calcMovementFactor(int newX, int newY)
{
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

int Actor::remapDirection(int dir, bool is_walking)
{
	int specdir;
	byte flags;
	bool flipX;
	bool flipY;

	// FIXME - why was this here? It breaks some animations in LoomCD, and logical,
	// why should we not honor box flags when ignoreBoxes is on? If this change
	// breaks anything, or if you know why the if() was in originally, please
	// tell this to Fingolfin.
	//if (!ignoreBoxes) {
	if (1) {
		specdir = _vm->_extraBoxFlags[walkbox];
		if (specdir) {
			if (specdir & 0x8000) {
				dir = specdir & 0x3FFF;
			} else {
				// FIXME make actors at top of lab walk
				// facing the correct way in the dig
				if (_vm->_gameId != GID_DIG)
					error("remapDirection: special dir not implemented");
			}
		}

		flags = _vm->getBoxFlags(walkbox);

		flipX = (walkdata.XYFactor > 0);
		flipY = (walkdata.YXFactor > 0);

		// FIXME - this special cases for the class might be necesary for other
		// games besides Loom CD!

		// Check for X-Flip
		if ((flags & kBoxXFlip) || isInClass(_vm->_gameId == GID_LOOM256 ? 19 : 30)) {
			dir = 360 - dir;
			flipX = !flipX;
		}
		// Check for Y-Flip
		if ((flags & kBoxYFlip) || isInClass(_vm->_gameId == GID_LOOM256 ? 18 : 29)) {
			dir = 180 - dir;
			flipY = !flipY;
		}

		switch (flags & 7) {
		case 1:
			if (is_walking)	                                // Actor is walking
				return flipX ? 90 : 270;
			else											// Actor is standing/turning
				return (dir == 90) ? 90 : 270;
		case 2:
			if (is_walking)                                 // Actor is walking
				return flipY ? 180 : 0;
			else											// Actor is standing/turning
				return (dir == 0) ? 0 : 180;
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
	/* Or 1024 in to signal direction interpolation should be done */
	return normalizeAngle(dir) | 1024;
}

int Actor::updateActorDirection(bool is_walking)
{
	int from, to;
	int diff;
	int dirType;
	int dir;
	int num;
	bool shouldInterpolate;

	dirType = _vm->akos_hasManyDirections(this);

	from = toSimpleDir(dirType, facing);
	dir = remapDirection(newDirection, is_walking);
	shouldInterpolate = (dir & 1024) ? true : false;
	to = toSimpleDir(dirType, dir & 1023);
	num = dirType ? 8 : 4;

	if (shouldInterpolate) {
		// Turn left or right, depending on which is shorter.
		diff = to - from;
		if (abs(diff) > (num >> 1))
			diff = -diff;

		if (diff > 0) {
			to = from + 1;
		} else if (diff < 0){
			to = from - 1;
		}
	}

	dir = fromSimpleDir(dirType, (to + num) % num);

	return dir;
}

void Actor::setBox(int box)
{
	walkbox = box;

	setupActorScale();
}


int Actor::actorWalkStep()
{
	int tmpX, tmpY;
	int actorX, actorY;
	int distX, distY;
	int direction;

	needRedraw = true;
	needBgReset = true;

	direction = updateActorDirection(true);
	if (!(moving & MF_IN_LEG) || facing != direction) {
		// FIXME - frame is never set and thus always 0! See actor.h comment
		if (walkFrame != frame || facing != direction) {
			startWalkAnim(walkFrame == frame ? 2 : 1, direction);
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


void Actor::setupActorScale()
{
	uint16 scale;
	byte *resptr;

	if (_vm->_features & GF_NO_SCALLING) {
		scalex = 0xFF;
		scaley = 0xFF;
		return;
	}

	if (ignoreBoxes != 0)
		return;

	if (_vm->getBoxFlags(walkbox) & kBoxPlayerOnly)
		return;

	scale = _vm->getBoxScale(walkbox);

	if (scale & 0x8000) {
		scale = (scale & 0x7FFF) + 1;
		resptr = _vm->getResourceAddress(rtScaleTable, scale);
		if (resptr == NULL)
			error("Scale table %d not defined", scale);
		int theY = y;
		if (theY >= _vm->_realHeight)
			theY = _vm->_realHeight - 1;
		else if (theY < 0)
			theY = 0;
		scale = resptr[theY];
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

	if (scale == 1 && _vm->_currentRoom == 76) {
		scale = 0xff;
		if (y < 356)
			scale -= 2 * (356 - y);
	}

	if (scale > 255)
		warning("Actor %d at %d, scale %d out of range", number, y, scale);

	scalex = (byte)scale;
	scaley = (byte)scale;
}

void Actor::startAnimActor(int f)
{
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

		// FIXME: This is a hack to fix decapitation, which somehow occurs only on
		// the standFrame (CHORE mode 3). We hack around this by simply using the
		// initFrame instead. As far as it goes, I see no difference. Apart from
		// keeping my lovely head intact.
		if (f == 3 && _vm->_gameId == GID_INDY3_256 && (_vm->_currentRoom != 79))
			f = initFrame;

		if (isInCurrentRoom() && costume) {
			animProgress = 0;
			cost.animCounter1 = 0;
			needRedraw = true;

			if (initFrame == f)
				cost.reset();

			if (f != 0x3E) {
				_vm->cost_decodeData(this, f, (uint) - 1);
			}
		}

		needBgReset = true;
	}
}

void Actor::animateActor(int anim)
{
	int cmd, dir;

	if (_vm->_features & GF_AFTER_V7) {

		if (anim == 0xFF)
			anim = 2000;

		cmd = anim / 1000;
		dir = anim % 1000;

	} else {

		cmd = anim >> 2;
		dir = oldDirToNewDir(anim & 3);

		// Convert into old cmd code
		cmd = 0x3F - cmd + 2;

	}

	switch (cmd) {
	case 2:
		stopActorMoving();
		startAnimActor(standFrame);
		break;
	case 3:
		moving &= ~MF_TURN;
		setDirection(dir);
		break;
	case 4:
		turnToDirection(dir);
		break;
	default:
		startAnimActor(anim);
	}
}

void Actor::setDirection(int direction)
{
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
		if (_vm->_features & GF_AFTER_V7)
			_vm->akos_decodeData(this, vald, aMask);
		else
			_vm->cost_decodeData(this, vald, aMask);
	}

	needRedraw = true;
	needBgReset = true;
}

void Actor::putActor(int dstX, int dstY, byte newRoom)
{
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

int Actor::getActorXYPos(int &xPos, int &yPos)
{
	if (!isInCurrentRoom())
		return -1;

	xPos = x;
	yPos = y;
	return 0;
}

AdjustBoxResult Actor::adjustXYToBeInBox(int dstX, int dstY, int pathfrom)
{
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

			if (!(_vm->_features & GF_OLD256) || box)
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

void Actor::adjustActorPos()
{
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

	if (_vm->_features & GF_AFTER_V7) {
		stopActorMoving();
	}

	flags = _vm->getBoxFlags(walkbox);
	if (flags & 7) {
		turnToDirection(facing);
	}
}

void Actor::turnToDirection(int newdir)
{
	if (newdir == -1)
		return;

	moving &= ~MF_TURN;

	if (newdir != facing) {
		moving = MF_TURN;
		newDirection = newdir;

		// FIXME - workaround for bug #558236
		if (_vm->_gameId == GID_INDY4 && room == 39 && x == 617 && y == 125 && newdir == 180)
			startAnimActor(standFrame);
	}
}

void Actor::hideActor()
{
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

void Actor::showActor()
{
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

void Scumm::showActors()
{
	int i;
	Actor *a;

	for (i = 1; i < NUM_ACTORS; i++) {
		a = derefActor(i);
		if (a->isInCurrentRoom())
			a->showActor();
	}
}

void Scumm::stopTalk()
{
	int act;

	_sound->stopTalkSound();

	_haveMsg = 0;
	_talkDelay = 0;

	act = _vars[VAR_TALK_ACTOR];
	if (act && act < 0x80) {
		Actor *a = derefActorSafe(act, "stopTalk");
		if ((a->isInCurrentRoom() && _useTalkAnims) || (_features & GF_AFTER_V7)) {
			a->startAnimActor(a->talkFrame2);
			_useTalkAnims = false;
		}
		_vars[VAR_TALK_ACTOR] = 0xFF;
	}
	_keepText = false;
	restoreCharsetBg();
}

void Scumm::clearMsgQueue()
{
	_messagePtr = (byte *)" ";
	stopTalk();
}

void Scumm::walkActors()
{
	int i;
	Actor *a;

	for (i = 1; i < NUM_ACTORS; i++) {
		a = derefActor(i);
		if (a->isInCurrentRoom())
			if (_features & GF_OLD256)
				a->walkActorOld();
			else
				a->walkActor();
	}
}

/* Used in Scumm v5 only. Play sounds associated with actors */
void Scumm::playActorSounds()
{
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

void Scumm::processActors()
{
	int i;
	Actor *actors[MAX_ACTORS], *a, **ac, **ac2, *tmp, **end;
	int numactors = 0;

	// Make a list of all actors in this room
	for (i = 1; i < NUM_ACTORS; i++) {
		a = derefActor(i);
		if (a->isInCurrentRoom())
			actors[numactors++] = a;
	}
	if (!numactors)
		return;

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
}

void Actor::drawActorCostume()
{
	if (!needRedraw)
		return;

	needRedraw = false;

	setupActorScale();

	if (!(_vm->_features & GF_AFTER_V7)) {
		CostumeRenderer cr(_vm);

		cr._actorX = x - _vm->virtscr->xstart;
		cr._actorY = y - elevation;
		cr._scaleX = scalex;
		cr._scaleY = scaley;

		cr._outheight = _vm->virtscr->height;

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
		// This is similar to the clipping == 100 check used for AKOS
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

		cr._shadow_table = _vm->_shadowPalette;

		cr.setCostume(costume);
		cr.setPalette(palette);
		cr.setFacing(facing);

		top = 0xFF;

		bottom = 0;

		/* if the actor is partially hidden, redraw it next frame */
		if (cr.drawCostume(this) & 1) {
			needBgReset = true;
			needRedraw = true;
		}
	} else {
		AkosRenderer ar(_vm);
		ar.charsetmask = true;
		ar._x = x - _vm->virtscr->xstart;
		ar._y = y - elevation;
		ar.scale_x = scalex;
		ar.scale_y = scaley;
		ar.clipping = forceClip;
		if (ar.clipping == 100) {
			ar.clipping = _vm->getMaskFromBox(walkbox);
			if (ar.clipping > (byte)_vm->gdi._numZBuffer)
				ar.clipping = _vm->gdi._numZBuffer;
		}

		ar.outptr = _vm->virtscr->screenPtr + _vm->virtscr->xstart;
		ar.outwidth = _vm->virtscr->width;
		ar.outheight = _vm->virtscr->height;

		ar.shadow_mode = shadow_mode;
		ar.shadow_table = _vm->_shadowPalette;

		ar.setCostume(costume);
		ar.setPalette(palette);
		ar.setFacing(this);

		ar.dirty_id = number;

		ar.cd = &cost;

		ar.draw_top = top = 0x7fffffff;
		ar.draw_bottom = bottom = 0;
		ar.drawCostume();
		top = ar.draw_top;
		bottom = ar.draw_bottom;
	}
}

void Actor::animateCostume()
{
	if (costume == 0)
		return;

	animProgress++;
	if (animProgress >= animSpeed) {
		animProgress = 0;

		if (_vm->_features & GF_AFTER_V7) {
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

void Scumm::setActorRedrawFlags(bool fg, bool bg)
{
	int i, j;
	uint32 bits;

	if (_fullRedraw) {
		for (j = 0; j < NUM_ACTORS; j++) {
			Actor *a = derefActor(j);
			if (fg)
				a->needRedraw = true;
			if (bg)
				a->needBgReset = true;
		}
	} else {
		for (i = 0; i < gdi._numStrips; i++) {
			bits = gfxUsageBits[_screenStartStrip + i];
			if (bits & 0x3FFFFFFF) {
				for (j = 0; j < NUM_ACTORS; j++) {
					if ((bits & (1 << j)) && bits != (uint32)(1 << j)) {
						Actor *a = derefActor(j);
						if (fg)
							a->needRedraw = true;
						if (bg)
							a->needBgReset = true;
					}
				}
			}
		}
	}
}

int Scumm::getActorFromPos(int x, int y)
{
	uint32 drawbits;
	int i;

	drawbits = gfxUsageBits[x >> 3];
	if (!(drawbits & 0x3FFFFFFF))
		return 0;
	for (i = 1; i < NUM_ACTORS; i++) {
		Actor *a = derefActor(i);
		if (drawbits & (1 << i) && !getClass(i, 32) && y >= a->top && y <= a->bottom) {
			return i;
		}
	}
	return 0;
}

void Scumm::actorTalk()
{
	int oldact;
	Actor *a;

	_msgPtrToAdd = charset._buffer;
	_messagePtr = addMessageToStack(_messagePtr);
	assert((int)(_msgPtrToAdd - charset._buffer) < (int)(sizeof(charset._buffer)));

	if (_actorToPrintStrFor == 0xFF) {
		if (!_keepText)
			stopTalk();
		_vars[VAR_TALK_ACTOR] = 0xFF;
		oldact = 0;
	} else {
		a = derefActorSafe(_actorToPrintStrFor, "actorTalk");
		if (!a->isInCurrentRoom() && !(_features & GF_AFTER_V7)) {
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
	charset._bufPos = 0;
	_talkDelay = 0;
	_haveMsg = 0xFF;
	_vars[VAR_HAVE_MSG] = 0xFF;
	CHARSET_1();
}

void Actor::setActorCostume(int c)
{
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

	if (_vm->_features & GF_AFTER_V7) {
		for (i = 0; i < 256; i++)
			palette[i] = 0xFF;
	} else {
		for (i = 0; i < 32; i++)
			palette[i] = 0xFF;
	}

}

void Actor::startWalkActor(int destX, int destY, int dir)
{
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

void Actor::startWalkAnim(int cmd, int angle)
{
	if (angle == -1)
		angle = facing;

/*FIXME: (yazoo): the walk script are buggy in dig causing
 * troubles while walking. It's disabled until I can
 * find a proper fix
 * note: walk scripts aren't required to make the game
 * work as usual */

/*	int16 args[16];

	if (walk_script != 0) {
		args[2] = angle;
		args[0] = number;
		args[1] = cmd;
		_vm->runScript(walk_script, 1, 0, args);
	} else*/  {
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

void Actor::walkActor()
{
	int j;
	int16 foundPathX, foundPathY;

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
			j = updateActorDirection(false);
			if (facing != j)
				setDirection(j);
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
		j = _vm->getPathToDestBox(walkbox, walkdata.destbox);
		if (j == -1 || j > 0xF0) {
			walkdata.destbox = walkbox;
			moving |= MF_LAST_LEG;
			return;
		}
		walkdata.curbox = j;

		if (_vm->findPathTowards(this, walkbox, j, walkdata.destbox, foundPathX, foundPathY))
			break;
		if (calcMovementFactor(foundPathX, foundPathY))
			return;

		setBox(walkdata.curbox);
	} while (1);

	moving |= MF_LAST_LEG;
	calcMovementFactor(walkdata.destx, walkdata.desty);
}

void Actor::walkActorOld()
{
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

	if (moving & MF_IN_LEG) {
		if (actorWalkStep())
			return;
	}

	if (moving & MF_LAST_LEG) {
		moving = 0;
		startWalkAnim(3, walkdata.destdir);
		return;
	}

	if (moving & MF_TURN) {
		new_dir = updateActorDirection(false);
		if (facing != new_dir) {
			setDirection(new_dir);
			return;
		}
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

byte *Actor::getActorName()
{
	byte *ptr = _vm->getResourceAddress(rtActorName, number);
	if (ptr == NULL)
		return (byte *)" ";
	return ptr;
}

void Actor::remapActorPalette(int r_fact, int g_fact, int b_fact, int threshold)
{
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

void Scumm::resetActorBgs()
{
	Actor *a;
	int i;
	uint32 onlyActorFlags, bitpos;

	for (i = 0; i < gdi._numStrips; i++) {
		onlyActorFlags = (gfxUsageBits[_screenStartStrip + i] &= 0x3FFFFFFF);
		a = getFirstActor();
		bitpos = 1;

		while (onlyActorFlags) {
			if (onlyActorFlags & 1 && a->top != 0xFF && a->needBgReset) {
				gfxUsageBits[_screenStartStrip + i] ^= bitpos;

				if ((a->bottom - a->top) >= 0)
					gdi.resetBackground(a->top, a->bottom, i);
			}
			bitpos <<= 1;
			onlyActorFlags >>= 1;
			a++;
		}
	}

	for (i = 1, a = getFirstActor(); ++a, i < NUM_ACTORS; i++) {
		a->needBgReset = false;
	}
}

void Actor::classChanged(int cls, bool value)
{
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

bool Actor::isInClass(int cls)
{
	return _vm->getClass(number, cls);
}
