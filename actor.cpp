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
 * Change Log:
 * $Log$
 * Revision 1.8  2001/11/05 19:21:49  strigeus
 * bug fixes,
 * speech in dott
 *
 * Revision 1.7  2001/10/26 17:34:50  strigeus
 * bug fixes, code cleanup
 *
 * Revision 1.6  2001/10/23 19:51:50  strigeus
 * recompile not needed when switching games
 * debugger skeleton implemented
 *
 * Revision 1.5  2001/10/16 10:01:44  strigeus
 * preliminary DOTT support
 *
 * Revision 1.4  2001/10/10 17:18:33  strigeus
 * fixed swapped parameters in o_walkActorToActor
 *
 * Revision 1.3  2001/10/10 11:24:21  strigeus
 * fixed return value from adjustXYToBeInBox
 *
 * Revision 1.2  2001/10/09 18:35:02  strigeus
 * fixed object parent bug
 * fixed some signed/unsigned comparisons
 *
 * Revision 1.1.1.1  2001/10/09 14:30:14  strigeus
 *
 * initial revision
 *
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
		a->facing = 2;
	} else if (mode==2) {
		a->facing = 2;
	}

	a->elevation = 0;
	a->width = 0x18;
	a->talkColor = 0xF;
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
	a->moving = 0;

	setActorWalkSpeed(a, 8, 2);

	a->ignoreBoxes = 0;
	a->neverZClip = 0;
	a->new_3 = 0;
	a->initFrame = 1;
	a->walkFrame = 2;
	a->standFrame = 3;
	a->talkFrame1 = 4;
	a->talkFrame2 = 5;

	_classData[a->number] = 0;
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

	if (diffY != 0) {
		XYFactor = YXFactor * diffX / diffY;
	} else {
		XYFactor = YXFactor * diffX;
		YXFactor = 0;
	}

	if ((uint)abs(XYFactor >> 16) > a->speedx) {
		XYFactor = a->speedx<<16;
		if (diffX < 0)
			XYFactor = -XYFactor;

		if (diffX != 0) {
			YXFactor = XYFactor * diffY / diffX;
		} else {
			YXFactor = XYFactor * diffY;
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

	return actorWalkStep(a);
}

int Scumm::actorWalkStep(Actor *a) {
	int32 XYFactor, YXFactor;
	int actorX, actorY, newx, newy;
	int newXDist;
	int32 tmp,tmp2;

	byte direction;

	a->needRedraw = true;
	a->needBgReset = true;

	XYFactor = a->walkdata.XYFactor;
	YXFactor = a->walkdata.YXFactor;

	direction = XYFactor>0 ? 1 : 0;
	if (abs(YXFactor) * 2 > abs(XYFactor))
		direction = YXFactor>0 ? 2 : 3;
	a->newDirection = direction;

	direction = getProgrDirChange(a, 1);

	if (!(a->moving&2) || a->facing!=direction) {
		if (a->walkFrame != a->animIndex || a->facing != direction) {
			startAnimActor(a, a->walkFrame, direction);
		}
		a->moving|=2;
	}
	
	actorX = a->walkdata.x;
	actorY = a->walkdata.y;
	newx = a->walkdata.newx;
	newy = a->walkdata.newy;

	if (a->walkbox != a->walkdata.curbox) {
		if (checkXYInBoxBounds(a->walkdata.curbox, a->x, a->y)) {
			a->walkbox = a->walkdata.curbox;
			a->mask = getMaskFromBox(a->walkdata.curbox);
			setupActorScale(a);
		}
	}

	newXDist = abs(newx - actorX);
	
	if (newXDist <= abs(a->x - actorX) &&
		abs(newy - actorY) <= abs(a->y - actorY) ){
		a->moving&=~2;
		return 0;
	}

	XYFactor = (XYFactor>>8) * a->scalex;
	YXFactor = (YXFactor>>8) * a->scalex;
	
	tmp = ((a->x + 8000)<<16) + a->walkdata.xfrac + XYFactor;
	tmp2 = (a->y<<16) + a->walkdata.yfrac + YXFactor;

	a->x = (tmp>>16)-8000;
	a->y = tmp2>>16;

	if (abs(a->x - actorX) > newXDist) {
		a->x = newx;
	}

	if (abs(a->y - actorY) > abs(newy - actorY)) {
		a->y = newy;
	}

	a->walkdata.xfrac = tmp&0xFFFF;
	a->walkdata.yfrac = tmp2&0xFFFF;

	if (a->x == newx &&
		a->y == newy) {
		a->moving&=~2;
		return 0;
	}

	return 1;
}

void Scumm::setupActorScale(Actor *a) {
	uint16 scale;
	byte *resptr;

	if (a->ignoreBoxes != 0)
		return;

	scale = getBoxScale(a->walkbox);

	if (scale & 0x8000) {
		scale = (scale&0x7FFF)+1;
		resptr = getResourceAddress(rtScaleTable, scale);
		if (resptr==NULL)
			error("Scale table %d not defined",scale);
		if (a->y >= 0)
			resptr += a->y;
		scale = *resptr;
	}

	if (scale>255)
		error("Actor %d at %d, scale %d out of range", a->number, a->y, scale);
	
	a->scalex = (byte)scale;
	a->scaley = (byte)scale;
}

int Scumm::getProgrDirChange(Actor *a, int mode) {
	int flags;
	byte facing, newdir;
	byte XYflag, YXflag;
	byte lookdir;

	const byte direction_transtab[] = {
		0,2,2,3,2,1,2,3,0,1,2,1,0,1,0,3
	};

	flags = 0;
	if (!a->ignoreBoxes)
		flags = getBoxFlags(a->walkbox);
		
	facing = a->facing;
	newdir = a->newDirection;
	
	XYflag = a->walkdata.XYFactor>0 ? 1 : 0;
	YXflag = a->walkdata.YXFactor>0 ? 1 : 0;

	if ((flags&8) || getClass(a->number, 0x1E)) {
		if (!(newdir&2))
			newdir^=1;
		XYflag = 1 - XYflag;
	}

	if ((flags&0x10) || getClass(a->number, 0x1D)) {
		if (newdir&2)
			newdir^=1;
		YXflag = 1 - YXflag;
	}

	lookdir = direction_transtab[facing*4+newdir];

	if (!(flags&=0x7))
		return lookdir;
	
	if (mode==0) {
		lookdir = newdir;
		if (flags==1 && newdir!=1)
			lookdir = 0;
		
		if (flags==2 && newdir!=3)
			lookdir = 2;
	} else {
		if (flags==1)
			lookdir = XYflag;
		if (flags==2)
			lookdir = 3 - YXflag;
	}
	if (flags==3)
		lookdir=0;
	if (flags==4)
		lookdir=1;
	if (flags==6)
		lookdir=2;
	if (flags==5)
		lookdir = 3;
	return lookdir;
}

void Scumm::startAnimActor(Actor *a, int frame, byte direction) {
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
	
		cost.loadCostume(a->costume);
		
		if (a->initFrame==frame)
			initActorCostumeData(a);

		if (frame!=0x3E) {
			decodeCostData(a, frame*4 + direction, -1);
		}

		if (a->facing != direction)
			fixActorDirection(a, direction);
	}

	a->facing = direction;
	a->needBgReset = true;
}

void Scumm::initActorCostumeData(Actor *a) {
	CostumeData *cd = &a->cost;
	int i;

	cd->hdr = 0;
	for (i=0; i<16; i++)
		cd->a[i] = cd->b[i] = cd->c[i] = cd->d[i] = 0xFFFF;
}

void Scumm::fixActorDirection(Actor *a, byte direction) {
	uint mask;
	int i;
	uint16 vald;

	if (a->facing == direction)
		return;

	mask = 0x8000;
	for (i=0; i<16; i++,mask>>=1) {
		vald = a->cost.d[i];
		if (vald==0xFFFF || (vald&3)==direction)
			continue;
		decodeCostData(a, (vald&0xFC)|direction, mask);
	}
	a->facing = direction;
}

void Scumm::decodeCostData(Actor *a, int frame, uint usemask) {
	byte *p,*r;
	uint mask,j;
	int i;
	byte extra,cmd;
	byte *dataptr;

	p = cost._ptr;
	if (frame > p[6])
		return;

	r = p + READ_LE_UINT16(p + frame*2 + cost._numColors + 42);
	if (r==p)
		return;

	dataptr = p + READ_LE_UINT16(p + cost._numColors + 8);

	mask = READ_LE_UINT16(r);
	r+=2;
	i = 0;
	do {
		if (mask&0x8000) {
			j = READ_LE_UINT16(r);
			r+=2;
			if (usemask&0x8000) {
				if (j==0xFFFF) {
					a->cost.a[i] = 0xFFFF;
					a->cost.b[i] = 0;
					a->cost.d[i] = frame;
				} else {
					extra = *r++;
					cmd = dataptr[j];
					if (cmd==0x7A) {
						a->cost.hdr &= ~(1<<i);
					} else if (cmd==0x79) {
						a->cost.hdr |= (1<<i);
					} else {
						a->cost.a[i] = a->cost.b[i] = j;
						a->cost.c[i] = j + (extra&0x7F);
						if (extra&0x80)
							a->cost.a[i] |= 0x8000;
						a->cost.d[i] = frame;
					}
				}
			} else {
				if (j!=0xFFFF)
					r++;
			}
		}
		i++;
		usemask <<= 1;
		mask <<= 1;
	} while ((uint16)mask);
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
				startAnimActor(a, a->standFrame, a->facing);
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

AdjustBoxResult Scumm::adjustXYToBeInBox(Actor *a, int x, int y) {
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
		
		while(1) {
			box = getNumBoxes() - 1;
			best = (uint)0xFFFF;
			b = 0;

			do {
				flags = getBoxFlags(box);
				if (flags&0x80 && (!(flags&0x20) || getClass(a->number, 0x1F)) )
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
	
	abr = adjustXYToBeInBox(a, a->x, a->y);

	a->x = abr.x;
	a->y = abr.y;
	a->walkbox = (byte)abr.dist; /* not a dist */
	a->walkdata.destbox = (byte)abr.dist;
	a->mask = getMaskFromBox(abr.dist);
	a->walkdata.destx = -1;
	setupActorScale(a);

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
		startAnimActor(a, a->standFrame, a->facing);
		a->moving = 0;
	}
	a->visible = false;
	a->cost.animCounter2 = 0;
	a->needRedraw = false;
	a->needBgReset = true;
}

void Scumm::turnToDirection(Actor *a, int newdir) {
	a->moving = 4;
	a->newDirection = newdir;
}

void Scumm::showActor(Actor *a) {
	if (_currentRoom == 0 || a->visible)
		return;

	adjustActorPos(a);

	ensureResourceLoaded(rtCostume, a->costume);

	if (a->costumeNeedsInit) {
		startAnimActor(a, a->initFrame, a->facing);
		a->costumeNeedsInit = false;
	}
	a->moving = 0;
	a->visible = true;
	a->needRedraw = true;
}

void Scumm::showActors() {
	int i;
	Actor *a;
	
	for (i=1; i<13; i++) {
		a = derefActor(i);
		if (a->room == _currentRoom)
			showActor(a);
	}
}

void Scumm::stopTalk() {
	int act;

	_haveMsg = 0;
	_talkDelay = 0;

	act = _vars[VAR_TALK_ACTOR];
	if (act && act<0x80) {
		Actor *a = derefActorSafe(act, "stopTalk");
		if (_currentRoom == a->room && _useTalkAnims) {
			startAnimActor(a, a->talkFrame2, a->facing);
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
	for (i=1; i<13; i++) {
		a = derefActor(i);	
		if (a->room==_currentRoom)
			walkActor(a);
	}
}

/* Used in Scumm v5 only. Play sounds associated with actors */
void Scumm::playActorSounds() {
	int i;
	Actor *a;
	
	for (i=1; i<13; i++) {
		a = derefActor(i);
		if (a->cost.animCounter2 && a->room==_currentRoom && a->sound) {
			_currentScript = 0xFF;
			addSoundToQueue(a->sound[0]);
			for (i=1; i<13; i++) {
				a = derefActor(i);
				a->cost.animCounter2 = 0;
			}
			return;
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

			j = a->walkdata.destbox;
			if (j) {
				a->walkbox = j;
				a->mask = getMaskFromBox(j);
			}
			startAnimActor(a, a->standFrame, a->facing);
			if (a->walkdata.destdir==0xFF ||
				  a->walkdata.destdir==a->newDirection)
						return;
			a->newDirection = a->walkdata.destdir;
			a->moving = 4;
			return;
		}

		if (a->moving&4) {
			j = getProgrDirChange(a, 0);
			if (a->facing != j) 
				startAnimActor(a, 0x3E, j);
			else
				a->moving = 0;
			return;
		}

		a->walkbox = a->walkdata.curbox;
		a->mask = getMaskFromBox(a->walkdata.curbox);

		setupActorScale(a);
		a->moving = (a->moving&2)|1;
	}

	do {
		a->moving&=~1;
		if (!a->walkbox) {
			a->walkbox = a->walkdata.destbox;
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

		a->walkbox = a->walkdata.curbox;
		a->mask = getMaskFromBox(a->walkdata.curbox);
		setupActorScale(a);
	} while (1);
	a->moving |= 8;
	calcMovementFactor(a, a->walkdata.destx, a->walkdata.desty);
}

void Scumm::processActors() {
	int i;
	Actor *actors[13],*a,**ac,**ac2,*tmp;
	int numactors = 0, cnt,cnt2;

	for (i=1; i<13; i++) {
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
			if ( (*ac2)->y > (*ac)->y ) {
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
			setupActorScale(a);
			setupCostumeRenderer(&cost, a);
			setActorCostPalette(a);
			CHECK_HEAP
			drawActorCostume(a);
			CHECK_HEAP
			actorAnimate(a);
		}
	} while (ac++,--cnt);
}

void Scumm::setupCostumeRenderer(CostumeRenderer *c, Actor *a) {
	c->_actorX = a->x - virtscr->xstart;
	c->_actorY = a->y - a->elevation;
	c->_zbuf = a->mask;
	if (c->_zbuf > gdi._numZBuffer)
		c->_zbuf = (byte)gdi._numZBuffer;
	if (a->neverZClip)
		c->_zbuf = a->neverZClip;
	
	c->_scaleX = a->scalex;
	c->_scaleY = a->scaley;
}

void Scumm::setActorCostPalette(Actor *a) {
	int i;
	byte color;

	cost.loadCostume(a->costume);
	
	for (i=0; i<cost._numColors; i++) {
		color = a->palette[i];
		if (color==255)
			color = cost._ptr[8+i];
		cost._palette[i] = color;
	}
}

void Scumm::drawActorCostume(Actor *a) {
	if (a==NULL || !a->needRedraw)
		return;
	
	a->top = 0xFF;
	a->needRedraw = 0;
	a->bottom = 0;
	cost.loadCostume(a->costume);
	cost._mirror = a->facing!=0 || (cost._ptr[7]&0x80);

	if (cost.drawCostume(a)) {
		a->needRedraw = true;
		a->needBgReset = true;;
	}
}

void Scumm::actorAnimate(Actor *a) {
	if (a==NULL)
		return;
	
	a->animProgress++;
	if (a->animProgress >= a->animSpeed) {
		a->animProgress = 0;
		cost.loadCostume(a->costume);
		if (cost.animate(a)) {
			a->needRedraw = true;
			a->needBgReset = true;
		}
	}
}

void Scumm::setActorRedrawFlags() {
	int i,j;
	int bits;

	for (i=0; i<40; i++) {
		bits = actorDrawBits[_screenStartStrip+i];
		if (bits&0x3FFF) {
			for(j=0; j<13; j++) {
				if ((bits&(1<<j)) && bits!=(1<<j)) {
					Actor *a = derefActor(j);
					a->needRedraw = true;
					a->needBgReset = true;
				}
			}
		}
	}
}

int Scumm::getActorFromPos(int x, int y) {
	uint16 drawbits;
	int i;

	drawbits = actorDrawBits[x>>3];
	if (!(drawbits & 0x3FFF))
		return 0;
	for (i=1; i<13; i++) {
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
				startAnimActor(a,a->talkFrame1,a->facing);
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

	abr = adjustXYToBeInBox(a, x, y);

	_xPos = abr.x;
	_yPos = abr.y;

	if (a->room != _currentRoom) {
		a->x = _xPos;
		a->y = _yPos;
		if (dir != 0xFF)
			a->facing = dir;
		return;
	}

	if (a->ignoreBoxes!=0) {
		abr.x = _xPos;
		abr.y = _yPos;
		abr.dist = 0;
		a->walkbox = 0;
	} else {
		if (checkXYInBoxBounds(a->walkdata.destbox, _xPos,_yPos)) {
			abr.x = _xPos;
			abr.y = _yPos;
			abr.dist = a->walkdata.destbox;
		} else {
			abr = adjustXYToBeInBox(a, _xPos, _yPos);
		}
		if (a->moving && a->walkdata.destdir == dir
			&& a->walkdata.destx == abr.x
			&& a->walkdata.desty == abr.y)
				return;
	}

	if (a->x==abr.x && a->y==abr.y) {
		if (dir!=0xFF && dir!=a->facing) {
			a->newDirection = dir;
			a->moving = 4;
		}
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
		for (i=1; i<13; i++) {
			a = derefActor(i);
			if (a->room == _currentRoom && a->costume == cost)
				return true;
		}

	return false;
}