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
#include "charset.h"
#include "object.h"
#include "resource.h"
#include "scumm.h"
#include "verbs.h"

void Scumm::redrawVerbs()
{
	int i;
	for (i = 0; i < _maxVerbs; i++)
		drawVerb(i, 0);
	verbMouseOver(0);
	_verbRedraw = false;
}

void Scumm::checkExecVerbs()
{
	int i, over;
	VerbSlot *vs;

	if (_userPut <= 0 || _mouseButStat == 0)
		return;

	if (_mouseButStat < MBS_MAX_KEY) {
		/* Check keypresses */
		vs = &_verbs[1];
		for (i = 1; i < _maxVerbs; i++, vs++) {
			if (vs->verbid && vs->saveid == 0 && vs->curmode == 1) {
				if (_mouseButStat == vs->key) {
					runInputScript(1, vs->verbid, 1);
					return;
				}
			}
		}
		runInputScript(4, _mouseButStat, 1);
	} else if (_mouseButStat & MBS_MOUSE_MASK) {
		byte code = _mouseButStat & MBS_LEFT_CLICK ? 1 : 2;
		if (mouse.y >= virtscr[0].topline && mouse.y < virtscr[0].topline + virtscr[0].height) {
			over = checkMouseOver(mouse.x, mouse.y);
			if (over != 0) {
				runInputScript(1, _verbs[over].verbid, code);
				return;
			}
			runInputScript(2, 0, code);
		} else {
			over = checkMouseOver(mouse.x, mouse.y);

			// FIXME For the future: Indy3 and under inv scrolling
			/*
			   if (over >= 31 && over <= 36) 
			   over += _inventoryOffset;
			 */
			runInputScript(1, over != 0 ? _verbs[over].verbid : 0, code);
		}
	}
}

void Scumm::verbMouseOver(int verb)
{
	if (_verbMouseOver == verb)
		return;

	if (_verbs[_verbMouseOver].type != kImageVerbType) {
		drawVerb(_verbMouseOver, 0);
		_verbMouseOver = verb;
	}

	if (_verbs[verb].type != kImageVerbType && _verbs[verb].hicolor) {
		drawVerb(verb, 1);
		_verbMouseOver = verb;
	}
}

int Scumm::checkMouseOver(int x, int y)
{
	VerbSlot *vs;
	int i = _maxVerbs - 1;

	vs = &_verbs[i];
	do {
		if (vs->curmode != 1 || !vs->verbid || vs->saveid || y < vs->y || y >= vs->bottom)
			continue;
		if (vs->center) {
			if (x < -(vs->right - vs->x - vs->x) || x >= vs->right)
				continue;
		} else {
			if (x < vs->x || x >= vs->right)
				continue;
		}

		return i;
	} while (--vs, --i);
	return 0;
}

void Scumm::drawVerb(int verb, int mode)
{
	VerbSlot *vs;
	bool tmp;

	if (!verb)
		return;

	vs = &_verbs[verb];

	if (!vs->saveid && vs->curmode && vs->verbid) {
		if (vs->type == kImageVerbType) {
			drawVerbBitmap(verb, vs->x, vs->y);
			return;
		}
		
		// FIXME: Commenting this out fixes some drawing glitches,
		// but TOTALLY screws up scrolling conversation items.
		//if (_gameId != GID_CMI)
			restoreVerbBG(verb);

		_string[4].charset = vs->charset_nr;
		_string[4].xpos = vs->x;
		_string[4].ypos = vs->y;
		_string[4].right = _realWidth - 1;
		_string[4].center = vs->center;

		if (vs->curmode == 2)
			_string[4].color = vs->dimcolor;
		else if (mode && vs->hicolor)
			_string[4].color = vs->hicolor;
		else
			_string[4].color = vs->color;

		// FIXME For the future: Indy3 and under inv scrolling
		/*
		   if (verb >= 31 && verb <= 36) 
		   verb += _inventoryOffset;
		 */

		_messagePtr = getResourceAddress(rtVerb, verb);
		if (!_messagePtr)
			return;
		assert(_messagePtr);

		if ((_features & GF_AFTER_V8) && (_messagePtr[0] == '/')) {
			char pointer[20];
			int i, j;

			translateText(_messagePtr, _transText);

			for (i = 0, j = 0; (_messagePtr[i] != '/' || j == 0) && j < 19; i++) {
				if (_messagePtr[i] != '/')
					pointer[j++] = _messagePtr[i];
			}
			pointer[j] = 0;
			_messagePtr = _transText;
		}

		tmp = _charset->_center;
		_charset->_center = 0;
		drawString(4);
		_charset->_center = tmp;

		vs->right = _charset->_strRight;
		vs->bottom = _charset->_strBottom;
		vs->oldleft = _charset->_strLeft;
		vs->oldright = _charset->_strRight;
		vs->oldtop = _charset->_strTop;
		vs->oldbottom = _charset->_strBottom;
		_charset->_strLeft = _charset->_strRight;
	} else {
		restoreVerbBG(verb);
	}
}

void Scumm::restoreVerbBG(int verb)
{
	VerbSlot *vs;

	vs = &_verbs[verb];

	if (vs->oldleft != -1) {
		restoreBG(vs->oldleft, vs->oldtop, vs->oldright, vs->oldbottom, vs->bkcolor);
		vs->oldleft = -1;
	}
}

void Scumm::drawVerbBitmap(int verb, int x, int y)
{
	VirtScreen *vs;
	VerbSlot *vst;
	byte twobufs, *imptr;
	int ydiff, xstrip;
	int imgw, imgh;
	int i, tmp;
	byte *obim;
	ImageHeader *imhd;
	uint32 size;

	if ((vs = findVirtScreen(y)) == NULL)
		return;

	gdi.disableZBuffer();

	twobufs = vs->alloctwobuffers;
	vs->alloctwobuffers = 0;

	xstrip = x >> 3;
	ydiff = y - vs->topline;

	obim = getResourceAddress(rtVerb, verb);
	if (_features & GF_SMALL_HEADER) {
		int obj;
		obj = READ_LE_UINT16(obim + 6);
		size = READ_LE_UINT32(obim);

		imgw = (*(obim + size + 11));
		imgh = (*(obim + size + 17)) >> 3;
		imptr = (obim + 8);
	} else {
		imhd = (ImageHeader *)findResourceData(MKID('IMHD'), obim);
		if (_features & GF_AFTER_V7) {
			imgw = READ_LE_UINT16(&imhd->v7.width) >> 3;
			imgh = READ_LE_UINT16(&imhd->v7.height) >> 3;
		} else {
			imgw = READ_LE_UINT16(&imhd->old.width) >> 3;
			imgh = READ_LE_UINT16(&imhd->old.height) >> 3;
		}

		if (_features & GF_AFTER_V8) {
			warning("drawVerbBitmap(%d, %d, %d)", verb, x, y);
			imptr = findResource(MKID('IMAG'), obim);
			assert(imptr);
			imptr = findResource(MKID('WRAP'), imptr);
			assert(imptr);
			imptr = findResource(MKID('OFFS'), imptr);
			assert(imptr);
			// Get the address of the second SMAP (corresponds to IM01)
			imptr += READ_LE_UINT32(imptr + 12);
		} else
			imptr = findResource(MKID('IM01'), obim);
		if (!imptr)
			error("No image for verb %d", verb);
	}
	for (i = 0; i < imgw; i++) {
		tmp = xstrip + i;
		if (tmp < gdi._numStrips)
			gdi.drawBitmap(imptr, vs, tmp, ydiff, imgh << 3, i, 1, Gdi::dbAllowMaskOr);
	}

	vst = &_verbs[verb];
	vst->right = vst->x + imgw * 8 - 1;
	vst->bottom = vst->y + imgh * 8 - 1;
	vst->oldleft = vst->x;
	vst->oldright = vst->right;
	vst->oldtop = vst->y;
	vst->oldbottom = vst->bottom;

	gdi.enableZBuffer();

	vs->alloctwobuffers = twobufs;
}

int Scumm::getVerbSlot(int id, int mode)
{
	int i;
	for (i = 1; i < _maxVerbs; i++) {
		if (_verbs[i].verbid == id && _verbs[i].saveid == mode) {
			return i;
		}
	}
	return 0;
}

void Scumm::killVerb(int slot)
{
	VerbSlot *vs;

	if (slot == 0)
		return;

	vs = &_verbs[slot];
	vs->verbid = 0;
	vs->curmode = 0;

	nukeResource(rtVerb, slot);

	if (vs->saveid == 0) {
		drawVerb(slot, 0);
		verbMouseOver(0);
	}
	vs->saveid = 0;
}

void Scumm::setVerbObject(uint room, uint object, uint verb)
{
	byte *obimptr;
	byte *obcdptr;
	uint32 size, size2;
	FindObjectInRoom foir;
	int i;

	if (whereIsObject(object) == WIO_FLOBJECT)
		error("Can't grab verb image from flobject");

	if (_features & GF_SMALL_HEADER) {
		for (i = (_numLocalObjects-1); i > 0; i--) {
			if (_objs[i].obj_nr == object) {
				findObjectInRoom(&foir, foImageHeader, object, room);
				size = READ_LE_UINT32(foir.obim);
				obcdptr = getResourceAddress(rtRoom, room) + getOBCDOffs(object);
				size2 = READ_LE_UINT32(obcdptr);
				createResource(rtVerb, verb, size + size2);
				obimptr = getResourceAddress(rtRoom, room) - foir.roomptr + foir.obim;
				obcdptr = getResourceAddress(rtRoom, room) + getOBCDOffs(object);
				memcpy(getResourceAddress(rtVerb, verb), obimptr, size);
				memcpy(getResourceAddress(rtVerb, verb) + size, obcdptr, size2);
			}
		}
	} else {
		findObjectInRoom(&foir, foImageHeader, object, room);
		size = READ_BE_UINT32_UNALIGNED(foir.obim + 4);
		createResource(rtVerb, verb, size);
		obimptr = getResourceAddress(rtRoom, room) - foir.roomptr + foir.obim;
		memcpy(getResourceAddress(rtVerb, verb), obimptr, size);
	}
}
