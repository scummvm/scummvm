/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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

void Scumm::initActor(Actor *a, int mode) {
	if (mode==1) {
		a->costume = 0;
		a->room = 0;
		a->x = 0;
		a->y = 0;
		a->facing = 180;
		a->newDirection = 180;
	} else if (mode==2) {
		a->facing = 180;
		a->newDirection = 180;
	}

	a->elevation = 0;
	a->width = 24;
	a->talkColor = 15;
	a->new_2 = 0;
	a->new_1 = -80;
	a->scaley = a->scalex = 0xFF;
	a->charset = 0;
	a->sound[0] = 0;
	a->sound[1] = 0;
	a->sound[2] = 0;
	a->sound[3] = 0;
	a->sound[4] = 0;
	a->sound[5] = 0;
	a->sound[6] = 0;
	a->sound[7] = 0;
	a->newDirection = 0;
	
	stopActorMoving(a);

	a->shadow_mode = 0;
	a->layer = 0;

	setActorWalkSpeed(a, 8, 2);

	a->ignoreBoxes = 0;
	a->forceClip = 0;
	a->new_3 = 0;
	a->initFrame = 1;
	a->walkFrame = 2;
	a->standFrame = 3;
	a->talkFrame1 = 4;
	a->talkFrame2 = 5;

	a->walk_script = 0;
	a->talk_script = 0;

	if (_features & GF_AFTER_V7) {
		_classData[a->number] = _classData[0];
	} else {
		_classData[a->number] = 0;
	}
}

void Scumm::stopActorMoving(Actor *a) {
	stopScriptNr(a->walk_script);
	a->moving = 0;
}

void Scumm::setActorWalkSpeed(Actor *a, uint speedx, uint speedy) {
	if (speedx == a->speedx && speedy == a->speedy)
		return;

	a->speedx = speedx;
	a->speedy = speedy;

	if (a->moving) {
		calcMovementFactor(a, a->walkdata.newx, a->walkdata.newy);
	}
}

int Scumm::getAngleFromPos(int x, int y) {
	if (abs(y)*2 < abs(x)) {
		if (x>0)
			return 90;
		return 270;
	} else {
		if (y>0)
			return 180;
		return 0;
	}
}

int Scumm::calcMovementFactor(Actor *a, int newX, int newY) {
	int actorX, actorY;
	int diffX, diffY;
	int32 XYFactor, YXFactor;

	actorX = a->x;
	actorY = a->y;

	if (actorX == newX && actorY == newY)
		return 0;

	diffX = newX - actorX;
	diffY = newY - actorY;
	YXFactor = a->speedy<<16;

	if (diffY < 0)
		YXFactor = -YXFactor;

	XYFactor = YXFactor * diffX;
	if (diffY != 0) {
		XYFactor /= diffY;
	} else {
		YXFactor = 0;
	}

	if ((uint)abs(XYFactor >> 16) > a->speedx) {
		XYFactor = a->speedx<<16;
		if (diffX < 0)
			XYFactor = -XYFactor;

		YXFactor = XYFactor * diffY;
		if (diffX != 0) {
			YXFactor /= diffX;
		} else {
			XYFactor = 0;
		}
	}

	a->walkdata.x = actorX;
	a->walkdata.y = actorY;
	a->walkdata.newx = newX;
	a->walkdata.newy = newY;
	a->walkdata.XYFactor = XYFactor;
	a->walkdata.YXFactor = YXFactor;
	a->walkdata.xfrac = 0;
	a->walkdata.yfrac = 0;
	
	a->newDirection = getAngleFromPos(XYFactor, YXFactor);

	return actorWalkStep(a);
}

int Scumm::remapDirection(Actor *a, int dir) {
	int specdir;
	byte flags;
	byte dirflag;

	if (!a->ignoreBoxes) {
		specdir = _extraBoxFlags[a->walkbox];
		if (specdir) {
			if (specdir & 0x8000) {
				dir = specdir & 0x3FFF;
			} else {
				error("getProgrDirChange: special dir not implemented");
			}
		}

		flags = getBoxFlags(a->walkbox);
		
		dirflag = ((a->walkdata.XYFactor>0) ? 1 : 0) |
			        ((a->walkdata.YXFactor>0) ? 2 : 0);


		if ((flags&8) || getClass(a->number, 0x1E)) {
			dir = 360 - dir;
			dirflag ^= 1;
		}

		if ((flags&0x10) || getClass(a->number, 0x1D)) {
			dir = 180 - dir;
			dirflag ^= 2;
		}

		switch(flags & 7) {
		case 1:
			return (dirflag&1) ? 90 : 270;
		case 2: 
			return (dirflag&2) ? 180 : 0;
		case 3: return 270;
		case 4: return 90;
		case 5: return 0;
		case 6:	return 180;
		}
	}
	return normalizeAngle(dir);
}

int Scumm::updateActorDirection(Actor *a) {
	int from,to;
	int diff;
	int dirType;
	int num;

	dirType = akos_hasManyDirections(a);

	from = toSimpleDir(dirType, a->facing);
	to = toSimpleDir(dirType, remapDirection(a, a->newDirection));
	diff = to - from;

	num = numSimpleDirDirections(dirType);

	if (abs(diff) > (num>>1))
		diff = -diff;

	if (diff==0) {
	} else if (diff>0) {
		from++;
	} else {
		from--;
	}

	return fromSimpleDir(dirType, from & (num-1));
}

void Scumm::setActorBox(Actor *a, int box) {
	a->walkbox = box;
	a->mask = getMaskFromBox(box);
	setupActorScale(a);
}


int Scumm::actorWalkStep(Actor *a) {
	int tmpX, tmpY;
	int actorX, actorY;
	int distX, distY;
	int direction;
	
	a->needRedraw = true;
	a->needBgReset = true;

	direction = updateActorDirection(a);
	if (!(a->moving&MF_IN_LEG) || a->facing!=direction) {
		if (a->walkFrame != a->frame || a->facing != direction) {
			startWalkAnim(a, a->walkFrame==a->frame ? 2 : 1, direction);
		}
		a->moving|=MF_IN_LEG;
	}

	actorX = a->x;
	actorY = a->y;

	if (a->walkbox != a->walkdata.curbox &&
			checkXYInBoxBounds(a->walkdata.curbox, actorX, actorY)) {
		setActorBox(a, a->walkdata.curbox);
	}

	distX = abs(a->walkdata.newx - a->walkdata.x);
	distY = abs(a->walkdata.newy - a->walkdata.y);

	if (
		abs(actorX - a->walkdata.x) >= distX && 
		abs(actorY - a->walkdata.y) >= distY
		) {
		a->moving &= ~MF_IN_LEG;
		return 0;
	}

	tmpX = ((actorX + 8000)<<16) + a->walkdata.xfrac + 
			(a->walkdata.XYFactor>>8) * a->scalex;
	a->walkdata.xfrac = (uint16)tmpX;
	actorX = (tmpX>>16) - 8000;

	tmpY = (actorY<<16) + a->walkdata.yfrac + 
			(a->walkdata.YXFactor>>8) * a->scalex;
	a->walkdata.yfrac = (uint16)tmpY;
	actorY = (tmpY>>16);
	
	if (abs(actorX - a->walkdata.x) > distX) {
		actorX = a->walkdata.newx;
	}

	if (abs(actorY - a->walkdata.y) > distY) {
		actorY = a->walkdata.newy;
	}

	a->x = actorX;
	a->y = actorY;
	return 1;
}


void Scumm::setupActorScale(Actor *a) {
	uint16 scale;
	byte *resptr;
	int y;

	if (a->ignoreBoxes != 0)
		return;

	scale = getBoxScale(a->walkbox);

	if (scale & 0x8000) {
		scale = (scale&0x7FFF)+1;
		resptr = getResourceAddress(rtScaleTable, scale);
		if (resptr==NULL)
			error("Scale table %d not defined",scale);
		y = a->y;
		if (y>=200)
			y=199;
		if (y<0)
			y=0;
		scale = resptr[y];
	}

	if (scale>255)
		warning("Actor %d at %d, scale %d out of range", a->number, a->y, scale); 
	
	a->scalex = (byte)scale;
	a->scaley = (byte)scale;
}

void Scumm::startAnimActor(Actor *a, int frame) {
	if (_features & GF_NEW_COSTUMES) {
		switch(frame) {
		case 1001: frame = a->initFrame; break;
		case 1002: frame = a->walkFrame; break;
		case 1003: frame = a->standFrame; break;
		case 1004: frame = a->talkFrame1; break;
		case 1005: frame = a->talkFrame2; break;
		}

		if (a->costume != 0) {
			a->animProgress = 0;
			a->needRedraw = true;
			a->needBgReset = true;
			if (frame == a->initFrame)
				initActorCostumeData(a);
			akos_decodeData(a, frame, (uint)-1);
		}

	} else {
		if (frame==0x38)
			frame = a->initFrame;

		if (frame==0x39)
			frame = a->walkFrame;

		if (frame==0x3A)
			frame = a->standFrame;

		if (frame==0x3B)
			frame = a->talkFrame1;

		if (frame==0x3C)
			frame = a->talkFrame2;

		if (a->room == _currentRoom && a->costume) {
			a->animProgress = 0;
			a->cost.animCounter1 = 0;
			a->needRedraw = true;
		
			if (a->initFrame==frame)
				initActorCostumeData(a);

			if (frame!=0x3E) {
				cost_decodeData(a, frame, -1);
			}
		}

		a->needBgReset = true;
	}
}

void Scumm::fixActorDirection(Actor *a, int direction) {
	uint mask;
	int i;
	uint16 vald;

	if (a->facing == direction)
		return;

	a->facing = direction;

	if (a->costume==0)
		return;

	mask = 0x8000;
	for (i=0; i<16; i++,mask>>=1) {
		vald = a->cost.frame[i];
		if (vald==0xFFFF)
			continue;
#if !defined(FULL_THROTTLE)
		cost_decodeData(a, vald, mask);
#else
		akos_decodeData(a, vald, mask);
#endif
	}

	a->needRedraw = true;
	a->needBgReset = true;
}

void Scumm::putActor(Actor *a, int x, int y, byte room) {
	if (a->visible && _currentRoom!=room && _vars[VAR_TALK_ACTOR]==a->number) {
		clearMsgQueue();
	}

	a->x = x;
	a->y = y;
	a->room = room;
	a->needRedraw = true;
	a->needBgReset = true;

	if (_vars[VAR_EGO]==a->number) {
		_egoPositioned = true;
	}

	if (a->visible) {
		if (_currentRoom == room) {
			if (a->moving) {
				startAnimActor(a, a->standFrame);
				a->moving = 0;
			}
			adjustActorPos(a);
		} else {
			hideActor(a);
		}
	} else {
		if (_currentRoom == room)
			showActor(a);
	}
}

int Scumm::getActorXYPos(Actor *a) {
	if (a->room != _currentRoom)
		return -1;
	_xPos = a->x;
	_yPos = a->y;
	return 0;
}

AdjustBoxResult Scumm::adjustXYToBeInBox(Actor *a, int x, int y, int pathfrom) {
        // Yazoo: need to recheck with Loom for the class data flags (0x400000)
	AdjustBoxResult abr,tmp;
	uint threshold;
	uint best;
	int box;
	byte flags, b;
	
	abr.x = x;
	abr.y = y;
	abr.dist = 0;

	if (a && a->ignoreBoxes==0) {
		threshold = 30;

                if ((_features & GF_SMALL_HEADER) && (_classData[a->number] & 0x200000))
                       return abr;
		
		while(1) {
			box = getNumBoxes() - 1;
			best = (uint)0xFFFF;
			b = 0;

                        if(((_features & GF_SMALL_HEADER) && box) || !(_features & GF_SMALL_HEADER))
                        do {
				flags = getBoxFlags(box);
				if (flags&0x80 && (!(flags&0x20) || getClass(a->number, 0x1F)) )
					continue;

				if (pathfrom && !getPathToDestBox(pathfrom, box))
					continue;
				
				if (!inBoxQuickReject(box, x, y, threshold))
					continue;
				
				if (checkXYInBoxBounds(box, x, y)) {
					abr.x = x;
					abr.y = y;
					abr.dist = box;
					return abr;
				}

				tmp = getClosestPtOnBox(box, x, y);

				if (tmp.dist >= best)
					continue;

				abr.x = tmp.x;
				abr.y = tmp.y;

				if (tmp.dist==0) {
					abr.dist = box;
					return abr;
				}
				best = tmp.dist;
				b = box;
			} while (--box);
			
			if (threshold==0 || threshold * threshold >= best) {
				abr.dist = b;
				return abr;
			}
			threshold = (threshold==30) ? 80 : 0;
		}
	}
	return abr;
}

void Scumm::adjustActorPos(Actor *a) {
	AdjustBoxResult abr;
	byte flags;
	
	abr = adjustXYToBeInBox(a, a->x, a->y, 0);

	a->x = abr.x;
	a->y = abr.y;
	a->walkdata.destbox = (byte)abr.dist;

	setActorBox(a, abr.dist);

	a->walkdata.destx = -1;

	a->moving = 0;
	a->cost.animCounter2 = 0;

	flags = getBoxFlags(a->walkbox);
	if (flags&7) {
		turnToDirection(a, a->facing);
	}
}

void Scumm::hideActor(Actor *a) {
	if (!a->visible)
		return;

	if (a->moving) {
		startAnimActor(a, a->standFrame);
		a->moving = 0;
	}
	a->visible = false;
	a->cost.animCounter2 = 0;
	a->needRedraw = false;
	a->needBgReset = true;
}

void Scumm::turnToDirection(Actor *a, int newdir) {
	if (newdir==-1)
		return;

	a->moving &= ~4;

	if (newdir != a->facing) {
		a->moving = 4;
		a->newDirection = newdir;
	}
}

void Scumm::showActor(Actor *a) {
	if (_currentRoom == 0 || a->visible)
		return;

	adjustActorPos(a);

	ensureResourceLoaded(rtCostume, a->costume);

	if (a->costumeNeedsInit) {
		startAnimActor(a, a->initFrame);
		a->costumeNeedsInit = false;
	}
	a->moving = 0;
	a->visible = true;
	a->needRedraw = true;
}

void Scumm::showActors() {
	int i;
	Actor *a;
	
	for (i=1; i<NUM_ACTORS; i++) {
		a = derefActor(i);
		if (a->room == _currentRoom)
			showActor(a);
	}
}

void Scumm::stopTalk() {
	int act;

	stopTalkSound();

	_haveMsg = 0;
	_talkDelay = 0;

	act = _vars[VAR_TALK_ACTOR];
	if (act && act<0x80) {
		Actor *a = derefActorSafe(act, "stopTalk");
		if (_currentRoom == a->room && _useTalkAnims) {
			startAnimActor(a, a->talkFrame2);
			_useTalkAnims = false;
		}
		_vars[VAR_TALK_ACTOR] = 0xFF;
	}
	_keepText = false;
	restoreCharsetBg();
}

void Scumm::clearMsgQueue() {
	_messagePtr = (byte*)" ";
	stopTalk();
}

void Scumm::walkActors() {
	int i;
	Actor *a;
	for (i=1; i<NUM_ACTORS; i++) {
		a = derefActor(i);	
		if (a->room==_currentRoom)
			walkActor(a);
	}
}

/* Used in Scumm v5 only. Play sounds associated with actors */
void Scumm::playActorSounds() {
	int i;
	Actor *a;
	
	for (i=1; i<NUM_ACTORS; i++) {
		a = derefActor(i);
		if (a->cost.animCounter2 && a->room==_currentRoom && a->sound) {
			_currentScript = 0xFF;
			addSoundToQueue(a->sound[0]);
			for (i=1; i<NUM_ACTORS; i++) {
				a = derefActor(i);
				a->cost.animCounter2 = 0;
			}
			return;
		}
	}
}


void Scumm::startWalkAnim(Actor *a, int cmd, int angle) {
	int16 args[16];

	if (angle == -1)
		angle = a->facing;

	if (a->walk_script != 0) {
		args[2] = angle;
		args[0] = a->number;
		args[1] = cmd;
		runScript(a->walk_script, 1, 0, args);
	} else {
		switch(cmd) {
		case 1: /* start walk */
			//a->facing = angle;
			fixActorDirection(a, angle);
			startAnimActor(a, a->walkFrame);
			break;
		case 2: /* change dir only */
			fixActorDirection(a, angle);
			break;
		case 3: /* stop walk */
			turnToDirection(a, angle);
			startAnimActor(a, a->standFrame);
			break;
		}
	}
}


void Scumm::walkActor(Actor *a) {
	int j;

	if (!a->moving)
		return;
	
	if (!(a->moving&1)) {
		if (a->moving&2 && actorWalkStep(a))
			return;
	
		if (a->moving&8) {
			a->moving = 0;
			setActorBox(a, a->walkdata.destbox);
			startWalkAnim(a, 3, a->walkdata.destdir);
			return;
		}

		if (a->moving&4) {
			j = updateActorDirection(a);
			if (a->facing != j) 
				fixActorDirection(a,j);
			else
				a->moving = 0;
			return;
		}

		setActorBox(a, a->walkdata.curbox);
		a->moving &= 2;
	}

#if OLD
	a->moving &= ~1;
	
	if (!a->walkbox) {
		a->walkbox = a->walkdata.destbox;
		a->walkdata.curbox = a->walkdata.destbox;
		a->moving |= 8;
		calcMovementFactor(a, a->walkdata.destx, a->walkdata.desty);
		return;
	}
	
	if (a->ignoreBoxes || a->walkbox==a->walkdata.destbox) {
		a->walkdata.curbox = a->walkbox;
		a->moving |= 8;
		calcMovementFactor(a, a->walkdata.destx, a->walkdata.desty);
		return;
	}
	j = getPathToDestBox(a->walkbox,a->walkdata.destbox);
	if (j==0) {
		error("walkActor: no path found between %d and %d", a->walkbox, a->walkdata.destbox);
	}

	a->walkdata.curbox = j;
	if (findPathTowards(a, a->walkbox, j, a->walkdata.destbox)) {
		a->moving |= 8;
		calcMovementFactor(a, a->walkdata.destx, a->walkdata.desty);
		return;
	}
	calcMovementFactor(a, _foundPathX, _foundPathY);
#endif
#if 1
	do {
		a->moving&=~1;
		if (!a->walkbox) {
			setActorBox(a, a->walkdata.destbox);
			a->walkdata.curbox = a->walkdata.destbox;
			break;
		}
		if (a->walkbox == a->walkdata.destbox)
			break;
		j = getPathToDestBox(a->walkbox,a->walkdata.destbox);
		if (j==0) {
			a->walkdata.destbox = a->walkbox;
			a->moving |= 8;
			return;
		}
		a->walkdata.curbox = j;
		if (findPathTowards(a, a->walkbox, j, a->walkdata.destbox))
			break;
		if (calcMovementFactor(a, _foundPathX, _foundPathY))
			return;

		setActorBox(a, a->walkdata.curbox);
	} while (1);
	a->moving |= 8;
	calcMovementFactor(a, a->walkdata.destx, a->walkdata.desty);
#endif
}

void Scumm::processActors() {
	int i;
	Actor *actors[NUM_ACTORS],*a,**ac,**ac2,*tmp;
	int numactors = 0, cnt,cnt2;

	for (i=1; i<NUM_ACTORS; i++) {
		a = derefActor(i);
		if (a->room == _currentRoom)
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
			if ( (*ac2)->y - ((*ac2)->layer<<11) > (*ac)->y - ((*ac)->layer<<11) ) {
				tmp = *ac;
				*ac = *ac2;
				*ac2 = tmp;
			}
		} while (ac2++, --cnt2);
	} while (ac++,--cnt);

	ac = actors;
	cnt = numactors;
	do {
		a = *ac;
		if (a->costume) {
			CHECK_HEAP
			getMaskFromBox(a->walkbox);
			drawActorCostume(a);
			CHECK_HEAP
			actorAnimate(a);
		}
	} while (ac++,--cnt);
}

#if !defined(FULL_THROTTLE)
void Scumm::drawActorCostume(Actor *a) {
	CostumeRenderer cr;

	if (a==NULL || !a->needRedraw)
		return;

	a->needRedraw = false;

	setupActorScale(a);

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
	if(cr.drawCostume(a)&1) {
		a->needBgReset = true;
		a->needRedraw = true;
	}
}
#else
void Scumm::drawActorCostume(Actor *a) {
	AkosRenderer ar;

	if (a==NULL || !a->needRedraw)
		return;

	a->needRedraw = false;

	setupActorScale(a);

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
	ar.charsetmask = _vars[VAR_CHARSET_MASK]!=0;

	ar.outptr = getResourceAddress(rtBuffer, 1) + virtscr->xstart;
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
#endif

void Scumm::actorAnimate(Actor *a) {
#if defined(FULL_THROTTLE)
	byte *akos;
#else
	LoadedCostume lc;
#endif

	if (a==NULL || a->costume == 0)
		return;
	
	a->animProgress++;
	if (a->animProgress >= a->animSpeed) {
		a->animProgress = 0;

#if defined(FULL_THROTTLE)
		akos = getResourceAddress(rtCostume, a->costume);
		assert(akos);
		if (akos_increaseAnims(akos, a)) {
#else
		loadCostume(&lc, a->costume);
		if (cost_increaseAnims(&lc, a)) {
#endif
			a->needRedraw = true;
			a->needBgReset = true;
		}
	}
}

void Scumm::setActorRedrawFlags() {
	uint i,j;
	uint32 bits;

	for (i=0; i<40; i++) {
		bits = gfxUsageBits[_screenStartStrip+i];
		if (bits&0x3FFFFFFF) {
			for(j=0; j<NUM_ACTORS; j++) {
				if ((bits&(1<<j)) && bits!=(uint32)(1<<j)) {
					Actor *a = derefActor(j);
					a->needRedraw = true;
					a->needBgReset = true;
				}
			}
		}
	}
}

int Scumm::getActorFromPos(int x, int y) {
	uint32 drawbits;
	int i;

	drawbits = gfxUsageBits[x>>3];
	if (!(drawbits & 0x3FFFFFFF))
		return 0;
	for (i=1; i<NUM_ACTORS; i++) {
		Actor *a = derefActor(i);
		if (drawbits&(1<<i) && !getClass(i, 32) && y >= a->top && y <= a->bottom) {
			return i;
		}
	}
	return 0;
}

void Scumm::actorTalk() {
	int oldact;
	Actor *a;

	_msgPtrToAdd = charset._buffer;
	_messagePtr = addMessageToStack(_messagePtr);
	
	if (_actorToPrintStrFor==0xFF) {
		if (!_keepText)
			stopTalk();
		_vars[VAR_TALK_ACTOR] = 0xFF;
		oldact = 0;
	} else {
		a = derefActorSafe(_actorToPrintStrFor, "actorTalk");
		if (a->room!=_currentRoom) {
			oldact = 0xFF;
		} else {
			if (!_keepText)
				stopTalk();
			_vars[VAR_TALK_ACTOR] = a->number;
			if (!string[0].no_talk_anim) {
				startAnimActor(a,a->talkFrame1);
				_useTalkAnims = true;
			}
			oldact = _vars[VAR_TALK_ACTOR];
		}
	}
	if (oldact>=0x80)
		return;
	
	if (_vars[VAR_TALK_ACTOR]>0x7F) {
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

void Scumm::setActorCostume(Actor *a, int c) {
	int i;

	a->costumeNeedsInit = true;
	
	if (a->visible) {
		hideActor(a);
		initActorCostumeData(a);
		a->costume = c;
		showActor(a);
	} else {
		a->costume = c;
		initActorCostumeData(a);
	}

	for (i=0; i<32; i++)
		a->palette[i] = 0xFF;
}

void Scumm::startWalkActor(Actor *a, int x, int y, int dir) {
	AdjustBoxResult abr;

	abr = adjustXYToBeInBox(a, x, y, a->walkbox);

	if (a->room != _currentRoom) {
		a->x = abr.x;
		a->y = abr.y;
		if (dir != -1)
			a->facing = dir;
		return;
	}

	if (a->ignoreBoxes!=0) {
		abr.dist = 0;
		a->walkbox = 0;
	} else {
		if (checkXYInBoxBounds(a->walkdata.destbox, abr.x,abr.y)) {
			abr.dist = a->walkdata.destbox;
		} else {
			abr = adjustXYToBeInBox(a, abr.x, abr.y, a->walkbox);
		}
		if (a->moving && a->walkdata.destdir == dir
			&& a->walkdata.destx == abr.x
			&& a->walkdata.desty == abr.y)
				return;
	}

	if (a->x==abr.x && a->y==abr.y) {
		turnToDirection(a, dir);
		return;
	}

	a->walkdata.destx = abr.x;
	a->walkdata.desty = abr.y;
	a->walkdata.destbox = (byte)abr.dist; /* a box */
	a->walkdata.destdir = dir;
	a->moving = (a->moving&2)|1;
	a->walkdata.curbox = a->walkbox;
}

byte *Scumm::getActorName(Actor *a) {
	byte *ptr = getResourceAddress(rtActorName, a->number);
	if(ptr==NULL)
		return (byte*)" ";
	return ptr;
}

bool Scumm::isCostumeInUse(int cost) {
	int i;
	Actor *a;

	if (_roomResource!=0)
		for (i=1; i<NUM_ACTORS; i++) {
			a = derefActor(i);
			if (a->room == _currentRoom && a->costume == cost)
				return true;
		}

	return false;
}

void Scumm::remapActor(Actor *a, int r_fact, int g_fact, int b_fact, int threshold) {
	byte *akos, *rgbs,*akpl;
	int akpl_size, i;
	int r,g,b;
	byte akpl_color;

	if (a->room != _currentRoom) {
		warning("Remap actor %d not in current room",a->number);
		return;
	}

	if (a->costume < 1 || a->costume >= _numCostumes-1){
		warning("Remap actor %d invalid costume",a->number,a->costume);
		return;
	}

	akos = getResourceAddress(rtCostume, a->costume);
	akpl = findResource(MKID('AKPL'), akos);
	
	//get num palette entries
	akpl_size=RES_SIZE(akpl) - 8;

	//skip resource header
	akpl = RES_DATA(akpl);
	
	rgbs = findResource(MKID('RGBS'), akos);

	if (!rgbs) {
		warning("Can't remap actor %d costume %d doesn't contain an RGB block",a->number,a->costume);
		return;
	}
	// skip resource header
	rgbs = RES_DATA(rgbs);
	
	for(i=0; i<akpl_size; i++) {
		r=*rgbs++;
		g=*rgbs++;
		b=*rgbs++;

		akpl_color=*akpl++;

		// allow remap of generic palette entry?
		if (!a->shadow_mode || akpl_color>=16) {
			if (r_fact!=256) r = (r*r_fact) >> 8;
			if (g_fact!=256) g = (g*g_fact) >> 8;
			if (b_fact!=256) b = (b*b_fact) >> 8;
			a->palette[i]=remapPaletteColor(r,g,b,threshold);
		}
	}
}

void Scumm::setupShadowPalette(int slot,int rfact,int gfact,int bfact,int from,int to) {
	byte *table;
	int i,num;
	byte *curpal;

	if (slot<0 || slot > 7)
		error("setupShadowPalette: invalid slot %d", slot);

	if (from<0 || from>255 || to<0 || from>255 || to < from)
		error("setupShadowPalette: invalid range from %d to %d", from, to);

	table = _shadowPalette + slot * 256;
	for(i=0; i<256; i++)
		table[i] = i;

	table += from;
	curpal = _currentPalette + from*3;
	num = to - from + 1;
	do {
		*table++ = remapPaletteColor(
			curpal[0] * rfact >> 8,
			curpal[1] * gfact >> 8,
			curpal[2] * bfact >> 8,
			(uint)-1);
		curpal+=3;
	} while (--num);
}
