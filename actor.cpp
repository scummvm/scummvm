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

	if (_scumm->_features & GF_AFTER_V7) {
		_scumm->_classData[number] = _scumm->_classData[0];
	} else {
		_scumm->_classData[number] = 0;
	}
}

void Actor::stopActorMoving()
{
	_scumm->stopScriptNr(walk_script);
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
		double temp = atan2(x, -y);
		return Scumm::normalizeAngle((int)(temp * 180 / 3.1415926535));
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

	if ((uint) abs(XYFactor >> 16) > speedx) {
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

	newDirection = _scumm->getAngleFromPos(XYFactor, YXFactor);

	return actorWalkStep();
}

int Actor::remapDirection(int dir)
{
	int specdir;
	byte flags;
	bool flipX;
	bool flipY;
	
	if (!ignoreBoxes) {
		specdir = _scumm->_extraBoxFlags[walkbox];
		if (specdir) {
			if (specdir & 0x8000) {
				dir = specdir & 0x3FFF;
			} else {
				error("getProgrDirChange: special dir not implemented");
			}
		}

		flags = _scumm->getBoxFlags(walkbox);

		flipX = (walkdata.XYFactor > 0);
		flipY = (walkdata.YXFactor > 0);

		// Check for X-Flip
		if ((flags & 0x08) || _scumm->getClass(number, 0x1E)) {
			dir = 360 - dir;
			flipX = !flipX;
		}

		// Check for Y-Flip
		if ((flags & 0x10) || _scumm->getClass(number, 0x1D)) {
			dir = 180 - dir;
			flipY = !flipY;
		}

		switch (flags & 7) {
		case 1:
			if (moving&~MF_TURN)				// Actor is walking
				return flipX ? 90 : 270;
			else								// Actor is standing/turning
				return (dir == 90) ? 90 : 270;
		case 2:
			if (moving&~MF_TURN)				// Actor is walking
				return flipY ? 180 : 0;
			else								// Actor is standing/turning
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
	return Scumm::normalizeAngle(dir) | 1024;
}

int Actor::updateActorDirection()
{
	int from, to;
	int diff;
	int dirType;
	int dir;
	int num;
	bool shouldInterpolate;
	
	dirType = _scumm->akos_hasManyDirections(this);

	from = Scumm::toSimpleDir(dirType, facing);
	dir = remapDirection(newDirection);
	shouldInterpolate = (dir & 1024);
	to = Scumm::toSimpleDir(dirType, dir & 1023);
	diff = to - from;	
	num = Scumm::numSimpleDirDirections(dirType);

	if (shouldInterpolate) {
		// Turn left or right, depending on which is shorter.
		if (abs(diff) > (num >> 1))
			diff = -diff;
	
		if (diff == 0) {
		} else if (diff > 0) {
			from++;
		} else {
			from--;
		}
	} else
		from = to;
	
	dir = Scumm::fromSimpleDir(dirType, from & (num - 1));

	return dir;
}

void Actor::setActorBox(int box)
{
	walkbox = box;
	mask = _scumm->getMaskFromBox(box);

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

	direction = updateActorDirection();
	if (!(moving & MF_IN_LEG) || facing != direction) {
		if (walkFrame != frame || facing != direction) {
			startWalkAnim(walkFrame == frame ? 2 : 1, direction);
		}
		moving |= MF_IN_LEG;
	}

	actorX = x;
	actorY = y;

	if (walkbox != walkdata.curbox &&
			_scumm->checkXYInBoxBounds(walkdata.curbox, actorX, actorY)) {
		setActorBox(walkdata.curbox);
	}

	distX = abs(walkdata.newx - walkdata.x);
	distY = abs(walkdata.newy - walkdata.y);

	if (abs(actorX - walkdata.x) >= distX &&
			abs(actorY - walkdata.y) >= distY) {
		moving &= ~MF_IN_LEG;
		return 0;
	}

	tmpX = ((actorX + 8000) << 16) + walkdata.xfrac +
		(walkdata.XYFactor >> 8) * scalex;
	walkdata.xfrac = (uint16)tmpX;
	actorX = (tmpX >> 16) - 8000;

	tmpY = (actorY << 16) + walkdata.yfrac +
		(walkdata.YXFactor >> 8) * scalex;
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

	// FIXME: Special 'no scaling' class for MI1 VGA Floppy
	//	      Not totally sure if this is correct.
	if(_scumm->_gameId == GID_MONKEY_VGA && _scumm->getClass(number, 0x96)) 
		return;

	if (_scumm->_features & GF_NO_SCALLING) {
		scalex = 0xFF;
		scaley = 0xFF;
		return;
	}

	if (ignoreBoxes != 0)
		return;

	if(_scumm->getBoxFlags(walkbox) & 0x20)
		return;

	scale = _scumm->getBoxScale(walkbox);

	if (scale & 0x8000) {
		scale = (scale & 0x7FFF) + 1;
		resptr = _scumm->getResourceAddress(rtScaleTable, scale);
		if (resptr == NULL)
			error("Scale table %d not defined", scale);
		int theY = y;
		if (theY >= 200)
			theY = 199;
		else if (theY < 0)
			theY = 0;
		scale = resptr[theY];
	}

	if (scale > 255)
		warning("Actor %d at %d, scale %d out of range", number, y, scale);

	// FIXME - Quick fix to ft's fuel tower bug (by yazoo)
	if(scale == 1 && _scumm->_currentRoom == 76)
		scale = 0xFF;

	scalex = (byte)scale;
	scaley = (byte)scale;
}

void Actor::startAnimActor(int frame)
{
	if (_scumm->_features & GF_NEW_COSTUMES) {		
		switch (frame) {
		case 1001:
			frame = initFrame;
			break;
		case 1002:
			frame = walkFrame;
			break;
		case 1003:
			frame = standFrame;
			break;
		case 1004:
			frame = talkFrame1;
			break;
		case 1005:
			frame = talkFrame2;
			break;
		}

		if (costume != 0) {
			animProgress = 0;
			needRedraw = true;
			needBgReset = true;
			if (frame == initFrame)
				_scumm->initActorCostumeData(this);
			_scumm->akos_decodeData(this, frame, (uint) - 1);
		}

	} else {
		switch (frame) {
		case 0x38:
			frame = initFrame;
			break;
		case 0x39:
			frame = walkFrame;
			break;
		case 0x3A:
			frame = standFrame;
			break;
		case 0x3B:
			frame = talkFrame1;
			break;
		case 0x3C:
			frame = talkFrame2;
			break;
		}

		if (isInCurrentRoom() && costume) {
			animProgress = 0;
			cost.animCounter1 = 0;
			needRedraw = true;

			if (initFrame == frame)
				_scumm->initActorCostumeData(this);

			if (frame != 0x3E) {
				_scumm->cost_decodeData(this, frame, (uint) - 1);
			}
		}

		needBgReset = true;
	}
}

void Actor::setActorDirection(int direction)
{
	uint aMask;
	int i;
	uint16 vald;

	if (facing == direction)
		return;

	facing = Scumm::normalizeAngle(direction);

	if (costume == 0)
		return;

	aMask = 0x8000;
	for (i = 0; i < 16; i++, aMask >>= 1) {
		vald = cost.frame[i];
		if (vald == 0xFFFF)
			continue;
		if (_scumm->_features & GF_AFTER_V7)
			_scumm->akos_decodeData(this, vald, aMask);
		else
			_scumm->cost_decodeData(this, vald, aMask);
	}

	needRedraw = true;
	needBgReset = true;
}

void Scumm::putActor(Actor * a, int dstX, int dstY, byte room)
{
	if (a->visible && _currentRoom != room
			&& _vars[VAR_TALK_ACTOR] == a->number) {
		clearMsgQueue();
	}

	a->x = dstX;
	a->y = dstY;
	a->room = room;
	a->needRedraw = true;
	a->needBgReset = true;

	if (_vars[VAR_EGO] == a->number) {
		_egoPositioned = true;
	}

	if (a->visible) {
		if (_currentRoom == room) {
			if (a->moving) {
				a->startAnimActor(a->standFrame);
				a->moving = 0;
			}
			a->adjustActorPos();
		} else {
			a->hideActor();
		}
	} else {
		if (_currentRoom == room)
			a->showActor();
	}
}

int Scumm::getActorXYPos(Actor * a)
{
	if (!a)
		return -1;

	if (!a->isInCurrentRoom())
		return -1;
	_xPos = a->x;
	_yPos = a->y;
	return 0;
}

AdjustBoxResult Scumm::adjustXYToBeInBox(Actor * a, int dstX, int dstY, int pathfrom)
{
	AdjustBoxResult abr, tmp;
	uint threshold;
	uint best;
	int box, iterations = 0;			/* Use iterations for those odd times we get stuck in the loop */
	int firstValidBox, j;
	byte flags, b;

	if (_features & GF_SMALL_HEADER)
		firstValidBox = 0;
	else
		firstValidBox = 1;

	abr.x = dstX;
	abr.y = dstY;
	abr.dist = 0;

	if ((_features & GF_SMALL_HEADER) && getClass(a->number, 22))
		return abr;

	if (a && a->ignoreBoxes == 0) {
		threshold = 30;

		while (1) {
			iterations++;
			if (iterations > 1000)
				return abr;							/* Safety net */
			box = getNumBoxes() - 1;
			if (box == 0)
				return abr;

			best = (uint) 0xFFFF;
			b = 0;

			if (((_features & GF_SMALL_HEADER) && box)
					|| !(_features & GF_SMALL_HEADER))
				for (j = box; j >= firstValidBox; j--) {
					flags = getBoxFlags(j);
					if (flags & 0x80 && (!(flags & 0x20) || getClass(a->number, 0x1F)))
						continue;

					if (pathfrom && (getPathToDestBox(pathfrom, j) == -1))
						continue;

					if (!inBoxQuickReject(j, dstX, dstY, threshold))
						continue;

					if (checkXYInBoxBounds(j, dstX, dstY)) {
						abr.x = dstX;
						abr.y = dstY;
						abr.dist = j;
						return abr;
					}

					tmp = getClosestPtOnBox(j, dstX, dstY);

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

	abr = _scumm->adjustXYToBeInBox(this, x, y, 0);

	x = abr.x;
	y = abr.y;
	walkdata.destbox = (byte)abr.dist;

	setActorBox(abr.dist);

	walkdata.destx = -1;

	moving = 0;
	cost.animCounter2 = 0;

	flags = _scumm->getBoxFlags(walkbox);
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
	if (_scumm->_currentRoom == 0 || visible)
		return;

	adjustActorPos();

	_scumm->ensureResourceLoaded(rtCostume, costume);

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

	stopTalkSound();

	_haveMsg = 0;
	_talkDelay = 0;

	act = _vars[VAR_TALK_ACTOR];
	if (act && act < 0x80) {
		Actor *a = derefActorSafe(act, "stopTalk");
		if (a->isInCurrentRoom() && _useTalkAnims) {
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
			addSoundToQueue(a->sound[0]);
			for (i = 1; i < NUM_ACTORS; i++) {
				a = derefActor(i);
				a->cost.animCounter2 = 0;
			}
			return;
		}
	}
}


void Actor::startWalkAnim(int cmd, int angle)
{
	int16 args[16];

	if (angle == -1)
		angle = facing;

	if (walk_script != 0) {
		args[2] = angle;
		args[0] = number;
		args[1] = cmd;
		_scumm->runScript(walk_script, 1, 0, args);
	} else {
		switch (cmd) {
		case 1:										/* start walk */
			setActorDirection(angle);
			startAnimActor(walkFrame);
			break;
		case 2:										/* change dir only */
			setActorDirection(angle);
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

	if (!moving)
		return;

	if (!(moving & MF_NEW_LEG)) {
		if (moving & MF_IN_LEG && actorWalkStep())
			return;

		if (moving & MF_LAST_LEG) {
			moving = 0;
			setActorBox(walkdata.destbox);
			startWalkAnim(3, walkdata.destdir);
			return;
		}

		if (moving & MF_TURN) {
			j = updateActorDirection();
			if (facing != j)
				setActorDirection(j);
			else
				moving = 0;
			return;
		}

		setActorBox(walkdata.curbox);
		moving &= MF_IN_LEG;
	}
#if OLD
	moving &= ~MF_NEW_LEG;

	if (!walkbox) {
		walkbox = walkdata.destbox;
		walkdata.curbox = walkdata.destbox;
		moving |= MF_LAST_LEG;
		calcMovementFactor(walkdata.destx, walkdata.desty);
		return;
	}

	if (ignoreBoxes || walkbox == walkdata.destbox) {
		walkdata.curbox = walkbox;
		moving |= MF_LAST_LEG;
		calcMovementFactor(walkdata.destx, walkdata.desty);
		return;
	}
	j = getPathToDestBox(walkbox, walkdata.destbox);
	if (j == -1) {
		error("walkActor: no path found between %d and %d", walkbox,
					walkdata.destbox);
	}

	walkdata.curbox = j;

	if (_scumm->findPathTowards(this, walkbox, j, walkdata.destbox)) {
		moving |= MF_LAST_LEG;
		calcMovementFactor(walkdata.destx, walkdata.desty);
		return;
	}
	calcMovementFactor(_foundPathX, _foundPathY);
#endif
#if 1
	do {
		moving &= ~MF_NEW_LEG;
		if ((!walkbox && (!(_scumm->_features & GF_SMALL_HEADER)))) {
			setActorBox(walkdata.destbox);
			walkdata.curbox = walkdata.destbox;
			break;
		}
		if (walkbox == walkdata.destbox)
			break;
		j = _scumm->getPathToDestBox(walkbox, walkdata.destbox);
		if (j == -1 || j > 0xF0) {
			walkdata.destbox = walkbox;
			moving |= MF_LAST_LEG;
			return;
		}
		walkdata.curbox = j;
		if (_scumm->_features & GF_OLD256) {
			_scumm->findPathTowardsOld(this, walkbox, j, walkdata.destbox);
			if (_scumm->gateLoc[2].x == 32000 && _scumm->gateLoc[3].x == 32000) {
				moving |= MF_LAST_LEG;
				calcMovementFactor(walkdata.destx, walkdata.desty);
				return;
			}

			if (_scumm->gateLoc[2].x != 32000) {
				if (calcMovementFactor(_scumm->gateLoc[2].x, _scumm->gateLoc[2].y)) {
					walkdata.destx = _scumm->gateLoc[3].x;
					walkdata.desty = _scumm->gateLoc[3].y;
					return;
				}
			}

			if (calcMovementFactor(_scumm->gateLoc[3].x, _scumm->gateLoc[3].y))
				return;

		} else {
			if (_scumm->findPathTowards(this, walkbox, j, walkdata.destbox))
				break;
			if (calcMovementFactor(_scumm->_foundPathX, _scumm->_foundPathY))
				return;
		}

		setActorBox(walkdata.curbox);
	} while (1);
	moving |= MF_LAST_LEG;
	calcMovementFactor(walkdata.destx, walkdata.desty);
#endif
}

void Scumm::processActors()
{
	int i;
	Actor *actors[MAX_ACTORS], *a, **ac, **ac2, *tmp;
	int numactors = 0, cnt, cnt2;

	for (i = 1; i < NUM_ACTORS; i++) {
		a = derefActor(i);
		if (a->isInCurrentRoom())
			actors[numactors++] = a;
	}
	if (!numactors)
		return;

	ac = actors;
	cnt = numactors;
	do {
		ac2 = actors;
		cnt2 = numactors;
		do {
			if ((*ac2)->y - ((*ac2)->layer << 11) > (*ac)->y - ((*ac)->layer << 11)) {
				tmp = *ac;
				*ac = *ac2;
				*ac2 = tmp;
			}
		} while (ac2++, --cnt2);
	} while (ac++, --cnt);

	ac = actors;
	cnt = numactors;
	do {
		a = *ac;
		if (a->costume) {
			CHECK_HEAP getMaskFromBox(a->walkbox);
			drawActorCostume(a);
			CHECK_HEAP actorAnimate(a);
		}
	} while (ac++, --cnt);
}

void Scumm::drawActorCostume(Actor * a)
{
	if (!(_features & GF_AFTER_V7)) {
		CostumeRenderer cr;

		if (a == NULL || !a->needRedraw)
			return;
		
		if (getClass(a->number, 20))
			a->mask = 0;
		else if (getClass(a->number, 21))
			a->forceClip = 1;

		if (_gameId==GID_SAMNMAX && getState(995)) // FIXME: ugly fix for samnmax inventory
			return;

		a->needRedraw = false;

		a->setupActorScale();

		/* First, zero initialize all fields */
		memset(&cr, 0, sizeof(cr));

		cr._actorX = a->x - virtscr->xstart;
		cr._actorY = a->y - a->elevation;
		cr._scaleX = a->scalex;
		cr._scaleY = a->scaley;

		cr._outheight = virtscr->height;
		cr._vm = this;

		cr._zbuf = a->mask;
		if (cr._zbuf > gdi._numZBuffer)
			cr._zbuf = (byte)gdi._numZBuffer;
		if (a->forceClip)
			cr._zbuf = a->forceClip;

		cr._shadow_table = _shadowPalette;

		cost_setCostume(&cr, a->costume);
		cost_setPalette(&cr, a->palette);
		cost_setFacing(&cr, a);

		a->top = 0xFF;

		a->bottom = 0;

		/* if the actor is partially hidden, redraw it next frame */
		if (cr.drawCostume(a) & 1) {
			a->needBgReset = true;
			a->needRedraw = true;
		}
	} else {
		AkosRenderer ar;

		if (a == NULL || !a->needRedraw)
			return;

		a->needRedraw = false;

		a->setupActorScale();

		/* First, zero initialize all fields */
		memset(&ar, 0, sizeof(ar));

		ar.x = a->x - virtscr->xstart;
		ar.y = a->y - a->elevation;
		ar.scale_x = a->scalex;
		ar.scale_y = a->scaley;
		ar.clipping = a->forceClip;
		if (ar.clipping == 100) {
			ar.clipping = a->mask;
			if (ar.clipping > (byte)gdi._numZBuffer)
				ar.clipping = gdi._numZBuffer;
		}
		ar.charsetmask = _vars[VAR_CHARSET_MASK] != 0;

		ar.outptr = virtscr->screenPtr + virtscr->xstart;
		ar.outwidth = virtscr->width;
		ar.outheight = virtscr->height;

		ar.shadow_mode = a->shadow_mode;
		ar.shadow_table = _shadowPalette;

		akos_setCostume(&ar, a->costume);
		akos_setPalette(&ar, a->palette);
		akos_setFacing(&ar, a);

		ar.dirty_id = a->number;

		ar.cd = &a->cost;

		ar.draw_top = a->top = 0x7fffffff;
		ar.draw_bottom = a->bottom = 0;
		akos_drawCostume(&ar);
		a->top = ar.draw_top;
		a->bottom = ar.draw_bottom;
	}
}

void Scumm::actorAnimate(Actor * a)
{
	byte *akos;
	LoadedCostume lc;

	if (a == NULL || a->costume == 0)
		return;

	a->animProgress++;
	if (a->animProgress >= a->animSpeed) {
		a->animProgress = 0;

		if (_features & GF_AFTER_V7) {
			akos = getResourceAddress(rtCostume, a->costume);
			assert(akos);
			if (akos_increaseAnims(akos, a)) {
				a->needRedraw = true;
				a->needBgReset = true;
			}
		} else {
			loadCostume(&lc, a->costume);
			if (cost_increaseAnims(&lc, a)) {
				a->needRedraw = true;
				a->needBgReset = true;
			}
		}
	}
}

void Scumm::setActorRedrawFlags()
{
	int i, j;
	uint32 bits;

	for (i = 0; i < 40; i++) {
		bits = gfxUsageBits[_screenStartStrip + i];
		if (bits & 0x3FFFFFFF) {
			for (j = 0; j < NUM_ACTORS; j++) {
				if ((bits & (1 << j)) && bits != (uint32)(1 << j)) {
					Actor *a = derefActor(j);
					a->needRedraw = true;
					a->needBgReset = true;
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
		if (drawbits & (1 << i) && !getClass(i, 32) && y >= a->top
				&& y <= a->bottom) {
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
	assert((int)(_msgPtrToAdd - charset._buffer) <
				 (int)(sizeof(charset._buffer)));

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
			if (!string[0].no_talk_anim) {
				a->startAnimActor(a->talkFrame1);
				_useTalkAnims = true;
			}
			oldact = _vars[VAR_TALK_ACTOR];
		}
	}
	if (oldact >= 0x80)
		return;

	if (_vars[VAR_TALK_ACTOR] > 0x7F) {
		_charsetColor = (byte)string[0].color;
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
		_scumm->initActorCostumeData(this);
		costume = c;
		showActor();
	} else {
		costume = c;
		_scumm->initActorCostumeData(this);
	}

	for (i = 0; i < 32; i++)
		palette[i] = 0xFF;
}

void Actor::startWalkActor(int destX, int destY, int dir)
{
	AdjustBoxResult abr;

	abr = _scumm->adjustXYToBeInBox(this, destX, destY, walkbox);

	if (!isInCurrentRoom()) {
		x = abr.x;
		x = abr.y;
		if (dir != -1)
			setActorDirection(dir);
		return;
	}

	if (ignoreBoxes != 0) {
		abr.dist = 0;
		walkbox = 0;
	} else {
		if (_scumm->checkXYInBoxBounds(walkdata.destbox, abr.x, abr.y)) {
			abr.dist = walkdata.destbox;
		} else {
			abr = _scumm->adjustXYToBeInBox(this, abr.x, abr.y, walkbox);
		}
		if (moving && walkdata.destdir == dir
				&& walkdata.destx == abr.x && walkdata.desty == abr.y)
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

byte *Actor::getActorName()
{
	byte *ptr = _scumm->getResourceAddress(rtActorName, number);
	if (ptr == NULL)
		return (byte *)" ";
	return ptr;
}

void Actor::remapActor(int r_fact, int g_fact, int b_fact,
											 int threshold)
{
	byte *akos, *rgbs, *akpl;
	int akpl_size, i;
	int r, g, b;
	byte akpl_color;

	if (!isInCurrentRoom()) {
		warning("Remap actor %d not in current room", number);
		return;
	}

	if (costume < 1 || costume >= _scumm->_numCostumes - 1) {
		warning("Remap actor %d invalid costume", number, costume);
		return;
	}

	akos = _scumm->getResourceAddress(rtCostume, costume);
	akpl = findResource(MKID('AKPL'), akos);

	//get num palette entries
	akpl_size = RES_SIZE(akpl) - 8;

	//skip resource header
	akpl = RES_DATA(akpl);

	rgbs = findResource(MKID('RGBS'), akos);

	if (!rgbs) {
		warning("Can't remap actor %d costume %d doesn't contain an RGB block",
						number, costume);
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
			palette[i] = _scumm->remapPaletteColor(r, g, b, threshold);
		}
	}
}

void Actor::walkActorOld()
{
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

		next_box = _scumm->getPathToDestBox(walkbox, walkdata.destbox);

		if (next_box == -1) {
			moving |= MF_LAST_LEG;
			return;
		}

		walkdata.curbox = next_box;

		_scumm->findPathTowardsOld(this, walkbox, next_box, walkdata.destbox);
		if (_scumm->gateLoc[2].x == 32000 && _scumm->gateLoc[3].x == 32000) {
			moving |= MF_LAST_LEG;
			calcMovementFactor(walkdata.destx, walkdata.desty);
			return;
		}

		if (_scumm->gateLoc[2].x != 32000) {
			if (calcMovementFactor(_scumm->gateLoc[2].x, _scumm->gateLoc[2].y)) {
				// FIXME - why is the first actor used here?!? Somebody please add a comment
				_scumm->getFirstActor()->walkdata.point3x = _scumm->gateLoc[3].x;
				_scumm->getFirstActor()->walkdata.point3y = _scumm->gateLoc[3].y;
				return;
			}
		}

		if (calcMovementFactor(_scumm->gateLoc[3].x, _scumm->gateLoc[3].y))
			return;

		walkbox = walkdata.destbox;
		mask = _scumm->getMaskFromBox(walkbox);
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
		new_dir = updateActorDirection();
		if (facing != new_dir) {
			setActorDirection(new_dir);
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
	mask = _scumm->getMaskFromBox(walkbox);
	moving &= MF_IN_LEG;
	moving |= MF_NEW_LEG;
	goto restart;
}
