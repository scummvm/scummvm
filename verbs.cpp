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
 * Revision 1.2  2001/10/09 19:02:28  strigeus
 * command line parameter support
 *
 * Revision 1.1.1.1  2001/10/09 14:30:13  strigeus
 *
 * initial revision
 *
 *
 */

#include "stdafx.h"
#include "scumm.h"

void Scumm::redrawVerbs() {
	int i;
	for (i=0; i<_maxVerbs; i++)
		drawVerb(i, 0);
	verbMouseOver(0);
}

void Scumm::checkExecVerbs() {
	int i,over;
	VerbSlot *vs;

	if (_userPut<=0 || _mouseButStat==0)
		return;

	if (_mouseButStat < 0x200) {
		/* Check keypresses */
		vs = &verbs[1];
		for (i=1; i<_maxVerbs; i++,vs++) {
			if (vs->verbid && vs->saveid && vs->curmode==1) {
				if (_mouseButStat == vs->key) {
					runInputScript(1, vs->verbid, 1);
					return;
				}
			}
		}
		runInputScript(4, _mouseButStat, 1);
	} else if (_mouseButStat&0xC000) {
		byte code = _mouseButStat&0x8000 ? 1 : 2;
		if (mouse.y >= virtscr[0].topline && mouse.y < virtscr[0].topline + virtscr[0].height) {
			over = checkMouseOver(mouse.x, mouse.y);
			if (over != 0) {
				runInputScript(1,verbs[over].verbid,code);
				return;
			}
			runInputScript(2, 0, code);
		} else {
			over=checkMouseOver(mouse.x, mouse.y);
			runInputScript(1, over!=0 ? verbs[over].verbid : 0, code);
		}
	}
}

void Scumm::verbMouseOver(int verb) {
	if (_verbMouseOver==verb)
		return;

	if (verbs[_verbMouseOver].type!=1) {
		drawVerb(_verbMouseOver, 0);
		_verbMouseOver = verb;
	}

	if (verbs[verb].type!=1 && verbs[verb].hicolor) {
		drawVerb(verb, 1);
		_verbMouseOver = verb;
	}
}

int Scumm::checkMouseOver(int x, int y) {
	VerbSlot *vs;
	int i = _maxVerbs;

	vs = &verbs[i];
	do {
		if (vs->curmode!=1 || !vs->verbid || vs->saveid ||
				y < vs->y || y >= vs->bottom)
				continue;
		if (vs->center) {
			if (x < -(vs->right - vs->x - vs->x) || x >= vs->right)
					continue;
		} else {
			if (x < vs->x || x >= vs->right)
				continue;
		}
		return i;
	} while (--vs, i--);
	return 0;
}

void Scumm::drawVerb(int vrb, int mode) {
	VerbSlot *vs;
	byte color;
	byte tmp;

	if (!vrb)
		return;

	vs = &verbs[vrb];

	if (!vs->saveid && vs->curmode && vs->verbid) {
		if (vs->type==1) {
			drawVerbBitmap(vrb, vs->x, vs->y);
			return;
		}
		restoreVerbBG(vrb);

		_stringCharset[4] = vs->charset_nr;
		_stringXpos[4] = vs->x;
		_stringYpos[4] = vs->y;
		_stringRight[4] = 319;
		_stringCenter[4] = vs->center;
		if (mode && vs->hicolor)
			color = vs->hicolor;
		else
			color = vs->color;
		_stringColor[4] = color;
		if (vs->curmode==2)
			_stringColor[4] = vs->dimcolor;
		_messagePtr = getResourceAddress(8, vrb);
		assert(_messagePtr);
		tmp = charset._center;
		charset._center = 0;
		drawString(4);
		charset._center = tmp;
		vs->right = charset._strRight;
		vs->bottom = charset._strBottom;
		vs->oldleft = charset._strLeft;
		vs->oldright = charset._strRight;
		vs->oldtop = charset._strTop;
		vs->oldbottom = charset._strBottom;
		charset._strLeft = charset._strRight;
	} else {
		restoreVerbBG(vrb);
	}
}

void Scumm::restoreVerbBG(int verb) {
	VerbSlot *vs;

	vs = &verbs[verb];

	if (vs->oldleft != -1) {
		dseg_4E3C = vs->bkcolor;
		restoreBG(vs->oldleft, vs->oldtop, vs->oldright, vs->oldbottom);
		vs->oldleft = -1;
	}
}

void Scumm::drawVerbBitmap(int vrb, int x, int y) {
	int nozbufs;
	VirtScreen *vs;
	VerbSlot *vst;
	byte twobufs, *imptr;
	int ydiff, xstrip;
	int imgw, imgh;
	int i;
	byte *IMHD_ptr;

	if (findVirtScreen(y) == -1)
		return;

	_lastXstart = virtscr[0].xstart;
	nozbufs = _numZBuffer;
	_numZBuffer = 0;

	vs = &virtscr[gdi.virtScreen];

	twobufs = vs->alloctwobuffers;
	vs->alloctwobuffers = 0;

	xstrip = x>>3;
	ydiff = y - vs->topline;

	IMHD_ptr = findResource2(MKID('IMHD'), getResourceAddress(8, vrb));

	imgw = READ_LE_UINT16(IMHD_ptr+0x14) >> 3;
	imgh = READ_LE_UINT16(IMHD_ptr+0x16) >> 3;
	
	imptr = findResource2(MKID('IM01'), NULL);
	if (!imptr)
		error("No image for verb %d", vrb);

	for (i=0; i<imgw; i++) {
		_drawBmpX = xstrip + i;
		if (_drawBmpX < 40) {
			_drawBmpY = ydiff;
			gdi.numLinesToProcess = imgh<<3;
			drawBmp(imptr, i, 1, 1, "Verb", READ_LE_UINT16(IMHD_ptr+8));
		}
	}

	vst = &verbs[vrb];
	vst->right = vst->x + imgw*8;
	vst->bottom = vst->y + imgh*8;
	vst->oldleft = vst->x;
	vst->oldright = vst->right;
	vst->oldtop = vst->y;
	vst->oldbottom = vst->bottom;
	_numZBuffer = nozbufs;

	vs->alloctwobuffers = twobufs;
}

int Scumm::getVerbSlot(int id, int mode) {
	int i;
	for (i=1; i<_maxVerbs; i++) {
		if (verbs[i].verbid == id && verbs[i].saveid == mode) {
			return i;
		}
	}
	return 0;
}

void Scumm::killVerb(int slot) {
	VerbSlot *vs;

	if (slot==0)
		return;

	vs = &verbs[slot];
	vs->verbid = 0;
	vs->curmode = 0;

	nukeResource(8, slot);

	if (vs->saveid==0) {
		drawVerb(slot, 0);
		verbMouseOver(0);
	}
	vs->saveid = 0;
}

void Scumm::setVerbObject(int room, int object, int verb) {
	int numobj, i;
	byte  *obimptr;
	uint32 imoffs,size;
	byte *roomptr,*tmp_roomptr;
	ImageHeader *imhd;
	RoomHeader *roomhdr;

	if (whereIsObject(object) == 4)
		error("Can't grab verb image from flobject");

	ensureResourceLoaded(1,room);
	roomptr = getResourceAddress(1, room);
	roomhdr = (RoomHeader*)findResource(MKID('RMHD'), roomptr);

	numobj = READ_LE_UINT16(&roomhdr->numObjects);
	if (numobj==0)
		error("No images found in room %d", room);
	if (numobj > 200)
		error("More (%d) than %d objects in room %d", numobj, 200, room);

	tmp_roomptr = roomptr;
	for (i=1; i<=numobj; i++) {
		obimptr = findResource(MKID('OBIM'), tmp_roomptr);
		if (obimptr==NULL)
			error("Not enough image blocks in room %d", room);
		imhd = (ImageHeader*)findResource2(MKID('IMHD'), obimptr);
		if ( READ_LE_UINT16(&imhd->obj_id) == object) {
			imoffs = obimptr - roomptr;
			size = READ_BE_UINT32_UNALIGNED(obimptr+4);
			createResource(8, verb, size);
			obimptr = getResourceAddress(1, room) + imoffs;
			memcpy(getResourceAddress(8, verb), obimptr, size);
			return;
		}
		tmp_roomptr = NULL;
	}
	error("Image %d not found in room %d", object, room);
}
