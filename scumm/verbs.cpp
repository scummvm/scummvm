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
#include "charset.h"
#include "object.h"
#include "resource.h"
#include "scumm.h"
#include "verbs.h"

enum {
	kInventoryUpArrow = 4,
	kInventoryDownArrow = 5,
	kSentenceLine = 6
};

void Scumm::initV2MouseOver() {
	int i;
	int arrow_color, color, hi_color;

	if (_version == 1) {
		color = 4;
		hi_color = 7;
		arrow_color = 6;
	} else {
		color = 13;
		hi_color = 14;
		arrow_color = 1;
	}

	v2_mouseover_box = -1;

	// Inventory items

	for (i = 0; i < 2; i++) {
		v2_mouseover_boxes[2 * i].rect.left = 0;
		v2_mouseover_boxes[2 * i].rect.right = 144;
		v2_mouseover_boxes[2 * i].rect.top = 32 + 8 * i;
		v2_mouseover_boxes[2 * i].rect.bottom = v2_mouseover_boxes[2 * i].rect.top + 8;

		v2_mouseover_boxes[2 * i].color = color;
		v2_mouseover_boxes[2 * i].hicolor = hi_color;


		v2_mouseover_boxes[2 * i + 1].rect.left = 176;
		v2_mouseover_boxes[2 * i + 1].rect.right = 320;
		v2_mouseover_boxes[2 * i + 1].rect.top = v2_mouseover_boxes[2 * i].rect.top;
		v2_mouseover_boxes[2 * i + 1].rect.bottom = v2_mouseover_boxes[2 * i].rect.bottom;

		v2_mouseover_boxes[2 * i + 1].color = color;
		v2_mouseover_boxes[2 * i + 1].hicolor = hi_color;
	}

	// Inventory arrows

	v2_mouseover_boxes[kInventoryUpArrow].rect.left = 144;
	v2_mouseover_boxes[kInventoryUpArrow].rect.right = 176;
	v2_mouseover_boxes[kInventoryUpArrow].rect.top = 32;
	v2_mouseover_boxes[kInventoryUpArrow].rect.bottom = 40;

	v2_mouseover_boxes[kInventoryUpArrow].color = arrow_color;
	v2_mouseover_boxes[kInventoryUpArrow].hicolor = hi_color;

	v2_mouseover_boxes[kInventoryDownArrow].rect.left = 144;
	v2_mouseover_boxes[kInventoryDownArrow].rect.right = 176;
	v2_mouseover_boxes[kInventoryDownArrow].rect.top = 40;
	v2_mouseover_boxes[kInventoryDownArrow].rect.bottom = 48;

	v2_mouseover_boxes[kInventoryDownArrow].color = arrow_color;
	v2_mouseover_boxes[kInventoryDownArrow].hicolor = hi_color;

	// Sentence line

	v2_mouseover_boxes[kSentenceLine].rect.left = 0;
	v2_mouseover_boxes[kSentenceLine].rect.right = 320;
	v2_mouseover_boxes[kSentenceLine].rect.top = 0;
	v2_mouseover_boxes[kSentenceLine].rect.bottom = 8;

	v2_mouseover_boxes[kSentenceLine].color = color;
	v2_mouseover_boxes[kSentenceLine].hicolor = hi_color;
}

void Scumm::checkV2MouseOver(ScummVM::Point pos) {
	VirtScreen *vs = &virtscr[2];
	ScummVM::Rect rect;
	byte *ptr, *dst;
	int i, x, y, new_box = -1;

	// Don't do anything unless the inventory is active
	if (!(_userState & 64)) {
		v2_mouseover_box = -1;
		return;
	}

	if (_cursor.state > 0) {
		for (i = 0; i < ARRAYSIZE(v2_mouseover_boxes); i++) {
			if (v2_mouseover_boxes[i].rect.contains(pos.x, pos.y - vs->topline)) {
				new_box = i;
				break;
			}
		}
	}

	if (new_box != v2_mouseover_box) {
		if (v2_mouseover_box != -1) {
			rect = v2_mouseover_boxes[v2_mouseover_box].rect;

			dst = ptr = vs->screenPtr + vs->xstart + rect.top * _screenWidth + rect.left;

			// Remove highlight.
			for (y = rect.height() - 1; y >= 0; y--) {
				for (x = rect.width() - 1; x >= 0; x--) {
					if (dst[x] == v2_mouseover_boxes[v2_mouseover_box].hicolor)
						dst[x] = v2_mouseover_boxes[v2_mouseover_box].color;
				}
				dst += _screenWidth;
			}

			updateDirtyRect(2, rect.left, rect.right, rect.top, rect.bottom, 0);
		}

		if (new_box != -1) {
			rect = v2_mouseover_boxes[new_box].rect;

			dst = ptr = vs->screenPtr + vs->xstart + rect.top * _screenWidth + rect.left;

			// Apply highlight
			for (y = rect.height() - 1; y >= 0; y--) {
				for (x = rect.width() - 1; x >= 0; x--) {
					if (dst[x] == v2_mouseover_boxes[new_box].color)
						dst[x] = v2_mouseover_boxes[new_box].hicolor;
				}
				dst += _screenWidth;
			}

			updateDirtyRect(2, rect.left, rect.right, rect.top, rect.bottom, 0);
		}

		v2_mouseover_box = new_box;
	}
}

void Scumm::checkV2Inventory(int x, int y) {
	int object = 0;

	y -= virtscr[2].topline;

	if ((y < 34) || !(_mouseButStat & MBS_LEFT_CLICK)) 
		return;

	if (v2_mouseover_boxes[kInventoryUpArrow].rect.contains(x, y)) {
		if (_inventoryOffset >= 2) {
			_inventoryOffset -= 2;
			redrawV2Inventory();
		}
 	} else if (v2_mouseover_boxes[kInventoryDownArrow].rect.contains(x, y)) {
 		if (_inventoryOffset + 4 < getInventoryCount(_scummVars[VAR_EGO])) {
			_inventoryOffset += 2;
			redrawV2Inventory();
		}
	}

	for (object = 0; object < 4; object++) {
		if (v2_mouseover_boxes[object].rect.contains(x, y)) {
			break;
		}
	}

	if (object >= 4)
		return;

	object = findInventory(_scummVars[VAR_EGO], object + 1 + _inventoryOffset);
	if (object > 0) {
		runInputScript(3, object, 0);
	}
}

void Scumm::redrawV2Inventory() {
	int i;
	int max_inv;
	ScummVM::Rect inventoryBox;

	v2_mouseover_box = -1;

	if (!(_userState & 64))	// Don't draw inventory unless active
		return;

	// Clear on all invocations
	inventoryBox.top = virtscr[2].topline + 32;
	inventoryBox.bottom = virtscr[2].topline + virtscr[2].height;
	inventoryBox.left = 0;
	inventoryBox.right = virtscr[2].width;
	restoreBG(inventoryBox);

	_string[1].charset = 1;

	max_inv = getInventoryCount(_scummVars[VAR_EGO]) - _inventoryOffset;
	if (max_inv > 4)
		max_inv = 4;
	for (i = 0; i < max_inv; i++) {
		int obj = findInventory(_scummVars[VAR_EGO], i + 1 + _inventoryOffset);
		if (obj == 0)
			break;
		
		_string[1].ypos = v2_mouseover_boxes[i].rect.top + virtscr[2].topline;
		_string[1].xpos = v2_mouseover_boxes[i].rect.left;

		_string[1].color = v2_mouseover_boxes[i].color;
		_messagePtr = getObjOrActorName(obj);
		assert(_messagePtr);

		// Prevent inventory entries from overflowing by truncating the text
		// after 144/8 = 18 chars
		byte msg[18 + 1];
		memcpy(msg, _messagePtr, 18),
		msg[18] = 0;
		_messagePtr = msg;
		
		// Draw it
		drawString(1);
	}


	// If necessary, draw "up" arrow
	if (_inventoryOffset > 0) {
		_string[1].xpos = v2_mouseover_boxes[kInventoryUpArrow].rect.left;
		_string[1].ypos = v2_mouseover_boxes[kInventoryUpArrow].rect.top + virtscr[2].topline;
	        _string[1].color = v2_mouseover_boxes[kInventoryUpArrow].color;
		_messagePtr = (const byte *)" \1\2";
		drawString(1);
	}

	// If necessary, draw "down" arrow
	if (_inventoryOffset + 4 < getInventoryCount(_scummVars[VAR_EGO])) {
		_string[1].xpos = v2_mouseover_boxes[kInventoryDownArrow].rect.left;
		_string[1].ypos = v2_mouseover_boxes[kInventoryDownArrow].rect.top + virtscr[2].topline;
	        _string[1].color = v2_mouseover_boxes[kInventoryDownArrow].color;
		_messagePtr = (const byte *)" \3\4";
		drawString(1);
	}
}

void Scumm::redrawVerbs() {
	if (_version <= 2 && !(_userState & 128)) // Don't draw verbs unless active
		return;

	int i;
	int verb = (_cursor.state > 0 ? checkMouseOver(_mouse.x, _mouse.y) : 0);
	for (i = _maxVerbs-1; i >= 0; i--) {
		if (i == verb && _verbs[verb].hicolor)
			drawVerb(i, 1);
		else
			drawVerb(i, 0);
	}
	_verbMouseOver = verb;
}

void Scumm::checkExecVerbs() {
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
					// Trigger verb as if the user clicked it
					runInputScript(1, vs->verbid, 1);
					return;
				}
			}
		}
		// Generic keyboard input
		runInputScript(4, _mouseButStat, 1);
	} else if (_mouseButStat & MBS_MOUSE_MASK) {
		VirtScreen *zone = findVirtScreen(_mouse.y);
		byte code = _mouseButStat & MBS_LEFT_CLICK ? 1 : 2;
		if (_version <= 2 && zone->number == 2 && _mouse.y <= zone->topline + 8) {
			// Click into V2 sentence line
			runInputScript(5, 0, 0);
		} else if (_version <= 2 && zone->number == 2 && _mouse.y > zone->topline + 32) {
			// Click into V2 inventory
			checkV2Inventory(_mouse.x, _mouse.y);
		} else {
			over = checkMouseOver(_mouse.x, _mouse.y);
			if (over != 0) {
				// Verb was clicked
				runInputScript(1, _verbs[over].verbid, code);
			} else {
				// Scene was clicked
				runInputScript((zone->number == 0) ? 2 : 1, 0, code);
			}
		}
	}
}

void Scumm::verbMouseOver(int verb) {
	// Don't do anything unless verbs are active
	if (_version <= 2 && !(_userState & 128))
		return;

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

int Scumm::checkMouseOver(int x, int y) const {
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

void Scumm::drawVerb(int verb, int mode) {
	if (_version <= 2 && !(_userState & 128)) // Don't draw verbs unless active
		return;

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
		
		restoreVerbBG(verb);

		_string[4].charset = vs->charset_nr;
		_string[4].xpos = vs->x;
		_string[4].ypos = vs->y;
		_string[4].right = _screenWidth - 1;
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

		if ((_version == 8) && (_messagePtr[0] == '/')) {
			translateText(_messagePtr, _transText);
			_messagePtr = _transText;
		}

		tmp = _charset->_center;
		_charset->_center = 0;
		drawString(4);
		_charset->_center = tmp;

		vs->right = _charset->_str.right;
		vs->bottom = _charset->_str.bottom;
		vs->old = _charset->_str;
		_charset->_str.left = _charset->_str.right;
	} else {
		restoreVerbBG(verb);
	}
}

void Scumm::restoreVerbBG(int verb) {
	VerbSlot *vs;

	vs = &_verbs[verb];

	if (vs->old.left != -1) {
		restoreBG(vs->old, vs->bkcolor);
		vs->old.left = -1;
	}
}

void Scumm::drawVerbBitmap(int verb, int x, int y) {
	VirtScreen *vs;
	VerbSlot *vst;
	byte twobufs;
	const byte *imptr = 0;
	int ydiff, xstrip;
	int imgw, imgh;
	int i, tmp;
	byte *obim;
	const ImageHeader *imhd;
	uint32 size;

	if ((vs = findVirtScreen(y)) == NULL)
		return;

	gdi.disableZBuffer();

	twobufs = vs->alloctwobuffers;
	vs->alloctwobuffers = 0;

	xstrip = x >> 3;
	ydiff = y - vs->topline;

	obim = getResourceAddress(rtVerb, verb);
	assert(obim);
	if (_features & GF_OLD_BUNDLE) {
		imgw = obim[0];
		imgh = obim[1] >> 3;
		imptr = obim + 2;
	} else if (_features & GF_SMALL_HEADER) {
		size = READ_LE_UINT32(obim);

		imgw = (*(obim + size + 11));
		imgh = (*(obim + size + 17)) >> 3;
		imptr = getObjectImage(obim, 1);
	} else {
		imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), obim);
		if (_version >= 7) {
			imgw = READ_LE_UINT16(&imhd->v7.width) >> 3;
			imgh = READ_LE_UINT16(&imhd->v7.height) >> 3;
		} else {
			imgw = READ_LE_UINT16(&imhd->old.width) >> 3;
			imgh = READ_LE_UINT16(&imhd->old.height) >> 3;
		}
		imptr = getObjectImage(obim, 1);
	}
	assert(imptr);
	if (_version == 1) {
		gdi._C64ObjectMode = true;
		gdi.decodeC64Gfx(imptr, gdi._C64ObjectMap, imgw * imgh * 3);
	}
	for (i = 0; i < imgw; i++) {
		tmp = xstrip + i;
		if (tmp < gdi._numStrips)
			gdi.drawBitmap(imptr, vs, tmp, ydiff, imgw * 8, imgh * 8, i, 1, Gdi::dbAllowMaskOr);
	}

	vst = &_verbs[verb];
	vst->right = vst->x + imgw * 8;
	vst->bottom = vst->y + imgh * 8;
	vst->old.left = vst->x;
	vst->old.right = vst->right;
	vst->old.top = vst->y;
	vst->old.bottom = vst->bottom;

	gdi.enableZBuffer();

	vs->alloctwobuffers = twobufs;
}

int Scumm::getVerbSlot(int id, int mode) const {
	int i;
	for (i = 1; i < _maxVerbs; i++) {
		if (_verbs[i].verbid == id && _verbs[i].saveid == mode) {
			return i;
		}
	}
	return 0;
}

void Scumm::killVerb(int slot) {
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

void Scumm::setVerbObject(uint room, uint object, uint verb) {
	const byte *obimptr;
	const byte *obcdptr;
	uint32 size, size2;
	FindObjectInRoom foir;
	int i;

	if (whereIsObject(object) == WIO_FLOBJECT)
		error("Can't grab verb image from flobject");

	if (_features & GF_OLD_BUNDLE) {
		for (i = (_numLocalObjects-1); i > 0; i--) {
			if (_objs[i].obj_nr == object) {
				findObjectInRoom(&foir, foImageHeader, object, room);
				size = READ_LE_UINT16(foir.obim);
				byte *ptr = createResource(rtVerb, verb, size + 2);
				obcdptr = getResourceAddress(rtRoom, room) + getOBCDOffs(object);
				ptr[0] = *(obcdptr + 9);	// Width
				ptr[1] = *(obcdptr + 15);	// Height
				memcpy(ptr + 2, foir.obim, size);
				return;
			}
		}
	} else if (_features & GF_SMALL_HEADER) {
		for (i = (_numLocalObjects-1); i > 0; i--) {
			if (_objs[i].obj_nr == object) {
				// FIXME - the only thing we need from the OBCD is the image size!
				// So we could use almost the same code (save for offsets)
				// as in the GF_OLD_BUNDLE code. But of course that would break save games
				// unless we insert special conversion code... <sigh>
				findObjectInRoom(&foir, foImageHeader, object, room);
				size = READ_LE_UINT32(foir.obim);
				obcdptr = getResourceAddress(rtRoom, room) + getOBCDOffs(object);
				size2 = READ_LE_UINT32(obcdptr);
				createResource(rtVerb, verb, size + size2);
				obimptr = getResourceAddress(rtRoom, room) - foir.roomptr + foir.obim;
				obcdptr = getResourceAddress(rtRoom, room) + getOBCDOffs(object);
				memcpy(getResourceAddress(rtVerb, verb), obimptr, size);
				memcpy(getResourceAddress(rtVerb, verb) + size, obcdptr, size2);
				return;
			}
		}
	} else {
		findObjectInRoom(&foir, foImageHeader, object, room);
		size = READ_BE_UINT32(foir.obim + 4);
		createResource(rtVerb, verb, size);
		obimptr = getResourceAddress(rtRoom, room) - foir.roomptr + foir.obim;
		memcpy(getResourceAddress(rtVerb, verb), obimptr, size);
	}
}
