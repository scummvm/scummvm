/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/verbs.h"

namespace Scumm {

enum {
	kInventoryUpArrow = 4,
	kInventoryDownArrow = 5,
	kSentenceLine = 6
};

struct VerbSettings {
	int id;
	int x_pos;
	int y_pos;
	const char *name;
};

static const VerbSettings C64VerbTable[] =
{
	{ 1,  8, 0, "Open"},
	{ 2,  8, 1, "Close"},
	{ 3,  0, 2, "Give"},
	{ 4, 32, 0, "Turn On"},
	{ 5, 32, 1, "Turn Off"},
	{ 6, 32, 2, "Fix"},
	{ 7, 24, 0, "New Kid"},
	{ 8, 24, 1, "Unlock"},
	{ 9,  0, 0, "Push"},
	{10,  0, 1, "Pull"},
	{11, 24, 2, "Use"},
	{12,  8, 2, "Read"},
	{13, 15, 0, "Walk To"},
	{14, 15, 1, "Pick Up"},
	{15, 15, 2, "What Is"}
};

void ScummEngine_c64::initC64Verbs() {
	VirtScreen *virt = &virtscr[kVerbVirtScreen];
	VerbSlot *vs;
	int i;

	for (i = 1; i < 16; i++) {
		vs = &_verbs[i];
		vs->verbid = C64VerbTable[i - 1].id;
		vs->color = 5;
		vs->hicolor = 7;
		vs->dimcolor = 11;
		vs->type = kTextVerbType;
		vs->charset_nr = _string[0]._default.charset;
		vs->curmode = 1;
		vs->saveid = 0;
		vs->key = 0;
		vs->center = 0;
		vs->imgindex = 0;

		vs->curRect.left = C64VerbTable[i - 1].x_pos * 8;
		vs->curRect.top = C64VerbTable[i - 1].y_pos * 8 + virt->topline + 8;

		loadPtrToResource(rtVerb, i, (const byte*)C64VerbTable[i - 1].name);
	}
}

void ScummEngine_v2::initV2MouseOver() {
	int i;
	int arrow_color, color, hi_color;

	if (_version == 1) {
		color = 16;
		hi_color = 7;
		arrow_color = 6;
	} else {
		color = 13;
		hi_color = 14;
		arrow_color = 1;
	}

	_mouseOverBoxV2 = -1;

	// Inventory items

	for (i = 0; i < 2; i++) {
		_mouseOverBoxesV2[2 * i].rect.left = 0;
		_mouseOverBoxesV2[2 * i].rect.right = 144;
		_mouseOverBoxesV2[2 * i].rect.top = 32 + 8 * i;
		_mouseOverBoxesV2[2 * i].rect.bottom = _mouseOverBoxesV2[2 * i].rect.top + 8;

		_mouseOverBoxesV2[2 * i].color = color;
		_mouseOverBoxesV2[2 * i].hicolor = hi_color;

		_mouseOverBoxesV2[2 * i + 1].rect.left = 176;
		_mouseOverBoxesV2[2 * i + 1].rect.right = 320;
		_mouseOverBoxesV2[2 * i + 1].rect.top = _mouseOverBoxesV2[2 * i].rect.top;
		_mouseOverBoxesV2[2 * i + 1].rect.bottom = _mouseOverBoxesV2[2 * i].rect.bottom;

		_mouseOverBoxesV2[2 * i + 1].color = color;
		_mouseOverBoxesV2[2 * i + 1].hicolor = hi_color;
	}

	// Inventory arrows

	_mouseOverBoxesV2[kInventoryUpArrow].rect.left = 144;
	_mouseOverBoxesV2[kInventoryUpArrow].rect.right = 176;
	_mouseOverBoxesV2[kInventoryUpArrow].rect.top = 32;
	_mouseOverBoxesV2[kInventoryUpArrow].rect.bottom = 40;

	_mouseOverBoxesV2[kInventoryUpArrow].color = arrow_color;
	_mouseOverBoxesV2[kInventoryUpArrow].hicolor = hi_color;

	_mouseOverBoxesV2[kInventoryDownArrow].rect.left = 144;
	_mouseOverBoxesV2[kInventoryDownArrow].rect.right = 176;
	_mouseOverBoxesV2[kInventoryDownArrow].rect.top = 40;
	_mouseOverBoxesV2[kInventoryDownArrow].rect.bottom = 48;

	_mouseOverBoxesV2[kInventoryDownArrow].color = arrow_color;
	_mouseOverBoxesV2[kInventoryDownArrow].hicolor = hi_color;

	// Sentence line

	_mouseOverBoxesV2[kSentenceLine].rect.left = 0;
	_mouseOverBoxesV2[kSentenceLine].rect.right = 320;
	_mouseOverBoxesV2[kSentenceLine].rect.top = 0;
	_mouseOverBoxesV2[kSentenceLine].rect.bottom = 8;

	_mouseOverBoxesV2[kSentenceLine].color = color;
	_mouseOverBoxesV2[kSentenceLine].hicolor = hi_color;
}

void ScummEngine_v2::initNESMouseOver() {
	int i;
	int arrow_color, color, hi_color;

	color = 0;
	hi_color = 0;
	arrow_color = 0;

	_mouseOverBoxV2 = -1;

	// Inventory items

	for (i = 0; i < 2; i++) {
		_mouseOverBoxesV2[2 * i].rect.left = 16;
		_mouseOverBoxesV2[2 * i].rect.right = 120;
		_mouseOverBoxesV2[2 * i].rect.top = 48 + 8 * i;
		_mouseOverBoxesV2[2 * i].rect.bottom = _mouseOverBoxesV2[2 * i].rect.top + 8;

		_mouseOverBoxesV2[2 * i].color = color;
		_mouseOverBoxesV2[2 * i].hicolor = hi_color;

		_mouseOverBoxesV2[2 * i + 1].rect.left = 152;
		_mouseOverBoxesV2[2 * i + 1].rect.right = 256;
		_mouseOverBoxesV2[2 * i + 1].rect.top = _mouseOverBoxesV2[2 * i].rect.top;
		_mouseOverBoxesV2[2 * i + 1].rect.bottom = _mouseOverBoxesV2[2 * i].rect.bottom;

		_mouseOverBoxesV2[2 * i + 1].color = color;
		_mouseOverBoxesV2[2 * i + 1].hicolor = hi_color;
	}

	// Inventory arrows

	_mouseOverBoxesV2[kInventoryUpArrow].rect.left = 128;
	_mouseOverBoxesV2[kInventoryUpArrow].rect.right = 136;
	_mouseOverBoxesV2[kInventoryUpArrow].rect.top = 48;
	_mouseOverBoxesV2[kInventoryUpArrow].rect.bottom = 56;

	_mouseOverBoxesV2[kInventoryUpArrow].color = arrow_color;
	_mouseOverBoxesV2[kInventoryUpArrow].hicolor = hi_color;

	_mouseOverBoxesV2[kInventoryDownArrow].rect.left = 136;
	_mouseOverBoxesV2[kInventoryDownArrow].rect.right = 144;
	_mouseOverBoxesV2[kInventoryDownArrow].rect.top = 48;
	_mouseOverBoxesV2[kInventoryDownArrow].rect.bottom = 56;

	_mouseOverBoxesV2[kInventoryDownArrow].color = arrow_color;
	_mouseOverBoxesV2[kInventoryDownArrow].hicolor = hi_color;

	// Sentence line

	_mouseOverBoxesV2[kSentenceLine].rect.left = 16;
	_mouseOverBoxesV2[kSentenceLine].rect.right = 256;
	_mouseOverBoxesV2[kSentenceLine].rect.top = 0;
	_mouseOverBoxesV2[kSentenceLine].rect.bottom = 8;

	_mouseOverBoxesV2[kSentenceLine].color = color;
	_mouseOverBoxesV2[kSentenceLine].hicolor = hi_color;
}

void ScummEngine_v2::checkV2MouseOver(Common::Point pos) {
	VirtScreen *vs = &virtscr[kVerbVirtScreen];
	Common::Rect rect;
	byte *ptr, *dst;
	int i, x, y, new_box = -1;

	// Don't do anything unless the inventory is active
	if (!(_userState & 64)) {
		_mouseOverBoxV2 = -1;
		return;
	}

	if (_cursor.state > 0) {
		for (i = 0; i < ARRAYSIZE(_mouseOverBoxesV2); i++) {
			if (_mouseOverBoxesV2[i].rect.contains(pos.x, pos.y - vs->topline)) {
				new_box = i;
				break;
			}
		}
	}

	if (new_box != _mouseOverBoxV2) {
		if (_mouseOverBoxV2 != -1) {
			rect = _mouseOverBoxesV2[_mouseOverBoxV2].rect;

			dst = ptr = vs->getPixels(rect.left, rect.top);

			// Remove highlight.
			for (y = rect.height() - 1; y >= 0; y--) {
				for (x = rect.width() - 1; x >= 0; x--) {
					if (dst[x] == _mouseOverBoxesV2[_mouseOverBoxV2].hicolor)
						dst[x] = _mouseOverBoxesV2[_mouseOverBoxV2].color;
				}
				dst += vs->pitch;
			}

			markRectAsDirty(kVerbVirtScreen, rect);
		}

		if (new_box != -1) {
			rect = _mouseOverBoxesV2[new_box].rect;

			dst = ptr = vs->getPixels(rect.left, rect.top);

			// Apply highlight
			for (y = rect.height() - 1; y >= 0; y--) {
				for (x = rect.width() - 1; x >= 0; x--) {
					if (dst[x] == _mouseOverBoxesV2[new_box].color)
						dst[x] = _mouseOverBoxesV2[new_box].hicolor;
				}
				dst += vs->pitch;
			}

			markRectAsDirty(kVerbVirtScreen, rect);
		}

		_mouseOverBoxV2 = new_box;
	}
}

void ScummEngine_v2::checkV2Inventory(int x, int y) {
	int inventoryArea = (_platform == Common::kPlatformNES) ? 48: 32;
	int object = 0;

	y -= virtscr[kVerbVirtScreen].topline;

	if ((y < inventoryArea) || !(_mouseAndKeyboardStat & MBS_LEFT_CLICK))
		return;

	if (_mouseOverBoxesV2[kInventoryUpArrow].rect.contains(x, y)) {
		if (_inventoryOffset >= 2) {
			_inventoryOffset -= 2;
			redrawV2Inventory();
		}
 	} else if (_mouseOverBoxesV2[kInventoryDownArrow].rect.contains(x, y)) {
 		if (_inventoryOffset + 4 < getInventoryCount(_scummVars[VAR_EGO])) {
			_inventoryOffset += 2;
			redrawV2Inventory();
		}
	}

	for (object = 0; object < 4; object++) {
		if (_mouseOverBoxesV2[object].rect.contains(x, y)) {
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

void ScummEngine_v2::redrawV2Inventory() {
	VirtScreen *vs = &virtscr[kVerbVirtScreen];
	int i;
	int max_inv;
	Common::Rect inventoryBox;
	int inventoryArea = (_platform == Common::kPlatformNES) ? 48: 32;
	int maxChars = (_platform == Common::kPlatformNES) ? 13: 18;

	_mouseOverBoxV2 = -1;

	if (!(_userState & 64))	// Don't draw inventory unless active
		return;

	// Clear on all invocations
	inventoryBox.top = vs->topline + inventoryArea;
	inventoryBox.bottom = vs->topline + virtscr[kVerbVirtScreen].h;
	inventoryBox.left = 0;
	inventoryBox.right = vs->w;
	restoreBG(inventoryBox);

	_string[1].charset = 1;

	max_inv = getInventoryCount(_scummVars[VAR_EGO]) - _inventoryOffset;
	if (max_inv > 4)
		max_inv = 4;
	for (i = 0; i < max_inv; i++) {
		int obj = findInventory(_scummVars[VAR_EGO], i + 1 + _inventoryOffset);
		if (obj == 0)
			break;

		_string[1].ypos = _mouseOverBoxesV2[i].rect.top + vs->topline;
		_string[1].xpos = _mouseOverBoxesV2[i].rect.left;

		_string[1].color = _mouseOverBoxesV2[i].color;

		const byte *tmp = getObjOrActorName(obj);
		assert(tmp);

		// Prevent inventory entries from overflowing by truncating the text
		byte msg[20];
		msg[maxChars] = 0;
		strncpy((char *)msg, (const char *)tmp, maxChars);

		// Draw it
		drawString(1, msg);
	}


	// If necessary, draw "up" arrow
	if (_inventoryOffset > 0) {
		_string[1].xpos = _mouseOverBoxesV2[kInventoryUpArrow].rect.left;
		_string[1].ypos = _mouseOverBoxesV2[kInventoryUpArrow].rect.top + vs->topline;
		_string[1].color = _mouseOverBoxesV2[kInventoryUpArrow].color;
		if (_platform == Common::kPlatformNES)
			drawString(1, (const byte *)"\x7E");
		else
			drawString(1, (const byte *)" \1\2");
	}

	// If necessary, draw "down" arrow
	if (_inventoryOffset + 4 < getInventoryCount(_scummVars[VAR_EGO])) {
		_string[1].xpos = _mouseOverBoxesV2[kInventoryDownArrow].rect.left;
		_string[1].ypos = _mouseOverBoxesV2[kInventoryDownArrow].rect.top + vs->topline;
		_string[1].color = _mouseOverBoxesV2[kInventoryDownArrow].color;
		if (_platform == Common::kPlatformNES)
			drawString(1, (const byte *)"\x7F");
		else
			drawString(1, (const byte *)" \3\4");
	}
}

void ScummEngine::redrawVerbs() {
	if (_version <= 2 && !(_userState & 128)) // Don't draw verbs unless active
		return;

	int i, verb = 0;
	if (_cursor.state > 0)
		verb = findVerbAtPos(_mouse.x, _mouse.y);

	// Iterate over all verbs.
	// Note: This is the correct order (at least for MI EGA, MI2, Full Throttle).
	// Do not change it! If you discover, based on disasm, that some game uses
	// another (e.g. the reverse) order here, you have to use an if/else construct
	// to add it as a special case!
	for (i = 0; i < _numVerbs; i++) {
		if (i == verb && _verbs[verb].hicolor)
			drawVerb(i, 1);
		else
			drawVerb(i, 0);
	}
	_verbMouseOver = verb;
}

void ScummEngine::handleMouseOver(bool updateInventory) {
	if (_completeScreenRedraw) {
		verbMouseOver(0);
	} else {
		if (_cursor.state > 0)
			verbMouseOver(findVerbAtPos(_mouse.x, _mouse.y));
	}
}

void ScummEngine_v2::handleMouseOver(bool updateInventory) {
	ScummEngine::handleMouseOver(updateInventory);

	if (updateInventory) {
		// FIXME/TODO: Reset and redraw the sentence line
		_inventoryOffset = 0;
	}
	if (_completeScreenRedraw || updateInventory) {
		redrawV2Inventory();
	}
	checkV2MouseOver(_mouse);
}

void ScummEngine::checkExecVerbs() {
	int i, over;
	VerbSlot *vs;

	if (VAR_MOUSE_STATE != 0xFF)
		VAR(VAR_MOUSE_STATE) = 0;

	if (_userPut <= 0 || _mouseAndKeyboardStat == 0)
		return;

	if (VAR_MOUSE_STATE != 0xFF)
		VAR(VAR_MOUSE_STATE) = _mouseAndKeyboardStat;

	if (_mouseAndKeyboardStat < MBS_MAX_KEY) {
		/* Check keypresses */
		vs = &_verbs[1];
		for (i = 1; i < _numVerbs; i++, vs++) {
			if (vs->verbid && vs->saveid == 0 && vs->curmode == 1) {
				if (_mouseAndKeyboardStat == vs->key) {
					// Trigger verb as if the user clicked it
					runInputScript(1, vs->verbid, 1);
					return;
				}
			}
		}

		if ((_gameId == GID_INDY4 || _gameId == GID_PASS) && _mouseAndKeyboardStat >= '0' && _mouseAndKeyboardStat <= '9') {
			// To support keyboard fighting in FOA, we need to remap the number keys.
			// FOA apparently expects PC scancode values (see script 46 if you want
			// to know where I got these numbers from). Oddly enough, the The Indy 3
			// part of the "Passport to Adventure" demo expects the same keyboard
			// mapping, even though the full game doesn't.
			static const int numpad[10] = {
					'0',
					335, 336, 337,
					331, 332, 333,
					327, 328, 329
				};
			_mouseAndKeyboardStat = numpad[_mouseAndKeyboardStat - '0'];
		}

		// Generic keyboard input
		runInputScript(4, _mouseAndKeyboardStat, 1);
	} else if (_mouseAndKeyboardStat & MBS_MOUSE_MASK) {
		VirtScreen *zone = findVirtScreen(_mouse.y);
		byte code = _mouseAndKeyboardStat & MBS_LEFT_CLICK ? 1 : 2;
		int inventoryArea = (_platform == Common::kPlatformNES) ? 48: 32;

		if (_version <= 2 && zone->number == kVerbVirtScreen && _mouse.y <= zone->topline + 8) {
			// Click into V2 sentence line
			runInputScript(5, 0, 0);
		} else if (_version <= 2 && zone->number == kVerbVirtScreen && _mouse.y > zone->topline + inventoryArea) {
			// Click into V2 inventory
			((ScummEngine_v2 *)this)->checkV2Inventory(_mouse.x, _mouse.y);
		} else {
			over = findVerbAtPos(_mouse.x, _mouse.y);
			if (over != 0) {
				// Verb was clicked
				runInputScript(1, _verbs[over].verbid, code);
			} else {
				// Scene was clicked
				runInputScript((zone->number == kMainVirtScreen) ? 2 : 1, 0, code);
			}
		}
	}
}

void ScummEngine_c64::checkExecVerbs() {
	Actor *a;
	VirtScreen *zone = findVirtScreen(_mouse.y);

	if (_userPut <= 0 || _mouseAndKeyboardStat == 0)
		return;

	if (zone->number == kVerbVirtScreen && _mouse.y <= zone->topline + 8) {
		// TODO
	} else if (_version <= 2 && zone->number == kVerbVirtScreen && _mouse.y > zone->topline + 32) {
		// Click into V2 inventory
		checkV2Inventory(_mouse.x, _mouse.y);
	} else {
		int over = findVerbAtPos(_mouse.x, _mouse.y);
		if (over) {
			_currentAction = _verbs[over].verbid;
			return;
		}

		// HACK: Reset value
		VAR(VAR_EGO) = 3;

		int object = findObject(_mouse.x, _mouse.y);
		if (object) {
			_activeObject = object;
			if (_currentMode == 3) {
				int x, y, dir;
				a = derefActor(VAR(VAR_EGO), "checkExecVerbs");
				getObjectXYPos(object, x, y, dir);
				a->startWalkActor(x, y, dir);
			}

			int tmp = (_currentMode == 3) ? _currentAction : 15;
			runObjectScript(object, tmp, false, false, NULL);
		} else {
			_activeObject = 0;
			if (zone->number == kMainVirtScreen) {
				a = derefActor(VAR(VAR_EGO), "checkExecVerbs");
				a->startWalkActor(_mouse.x, _mouse.y, -1);
			}
		}
	}
}

void ScummEngine::verbMouseOver(int verb) {
	// Don't do anything unless verbs are active
	if (_version <= 2 && !(_userState & 128))
		return;

	if (_gameId == GID_FT)
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

int ScummEngine::findVerbAtPos(int x, int y) const {
	if (!_numVerbs)
		return 0;

	VerbSlot *vs;
	int i = _numVerbs - 1;

	vs = &_verbs[i];
	do {
		if (vs->curmode != 1 || !vs->verbid || vs->saveid || y < vs->curRect.top || y >= vs->curRect.bottom)
			continue;
		if (vs->center) {
			if (x < -(vs->curRect.right - 2 * vs->curRect.left) || x >= vs->curRect.right)
				continue;
		} else {
			if (x < vs->curRect.left || x >= vs->curRect.right)
				continue;
		}

		return i;
	} while (--vs, --i);

	return 0;
}

#ifndef DISABLE_SCUMM_7_8
void ScummEngine_v7::drawVerb(int verb, int mode) {
	VerbSlot *vs;

	if (!verb)
		return;

	vs = &_verbs[verb];

	if (!vs->saveid && vs->curmode && vs->verbid) {
		if (vs->type == kImageVerbType) {
			drawVerbBitmap(verb, vs->curRect.left, vs->curRect.top);
			return;
		}

		uint8 color = vs->color;
		if (vs->curmode == 2)
			color = vs->dimcolor;
		else if (mode && vs->hicolor)
			color = vs->hicolor;

		const byte *msg = getResourceAddress(rtVerb, verb);
		if (!msg)
			return;

		// Convert the message, and skip a few remaining 0xFF codes (they
		// occur in FT; subtype 10, which is used for the speech associated
		// with the string).
		byte buf[384];
		convertMessageToString(msg, buf, sizeof(buf));
		msg = buf;
		while (*msg == 0xFF)
			msg += 4;

		enqueueText(msg, vs->curRect.left, vs->curRect.top, color, vs->charset_nr, vs->center);

		// Set the specified charset id
		_charset->setCurID(vs->charset_nr);

		// Compute the text rect
		vs->curRect.right = 0;
		vs->curRect.bottom = 0;
		while (*msg) {
			const int charWidth = _charset->getCharWidth(*msg);
			const int charHeight = _charset->getCharHeight(*msg);
			vs->curRect.right += charWidth;
			if (vs->curRect.bottom < charHeight)
				vs->curRect.bottom = charHeight;
			msg++;
		}
		vs->curRect.right += vs->curRect.left;
		vs->curRect.bottom += vs->curRect.top;
		vs->oldRect = vs->curRect;
	}
}
#endif

void ScummEngine::drawVerb(int verb, int mode) {
	VerbSlot *vs;
	bool tmp;

	if (!verb)
		return;

	vs = &_verbs[verb];

	if (!vs->saveid && vs->curmode && vs->verbid) {
		if (vs->type == kImageVerbType) {
			drawVerbBitmap(verb, vs->curRect.left, vs->curRect.top);
			return;
		}

		restoreVerbBG(verb);

		_string[4].charset = vs->charset_nr;
		_string[4].xpos = vs->curRect.left;
		_string[4].ypos = vs->curRect.top;
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

		const byte *msg = getResourceAddress(rtVerb, verb);
		if (!msg)
			return;

		tmp = _charset->_center;
		drawString(4, msg);
		_charset->_center = tmp;

		vs->curRect.right = _charset->_str.right;
		vs->curRect.bottom = _charset->_str.bottom;
		vs->oldRect = _charset->_str;
		_charset->_str.left = _charset->_str.right;
	} else if (_gameId != GID_FT) {
		restoreVerbBG(verb);
	}
}

void ScummEngine::restoreVerbBG(int verb) {

	VerbSlot *vs;

	vs = &_verbs[verb];

	if (vs->oldRect.left != -1) {
		restoreBG(vs->oldRect, vs->bkcolor);
		vs->oldRect.left = -1;
	}
}

void ScummEngine::drawVerbBitmap(int verb, int x, int y) {
	VirtScreen *vs;
	VerbSlot *vst;
	bool twobufs;
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

	twobufs = vs->hasTwoBuffers;
	vs->hasTwoBuffers = false;

	xstrip = x / 8;
	ydiff = y - vs->topline;

	obim = getResourceAddress(rtVerb, verb);
	assert(obim);
	if (_features & GF_OLD_BUNDLE) {
		imgw = obim[0];
		imgh = obim[1] / 8;
		imptr = obim + 2;
	} else if (_features & GF_SMALL_HEADER) {
		size = READ_LE_UINT32(obim);

		imgw = (*(obim + size + 11));
		imgh = (*(obim + size + 17)) / 8;
		imptr = getObjectImage(obim, 1);
	} else {
		imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), obim);
		if (_version >= 7) {
			imgw = READ_LE_UINT16(&imhd->v7.width) / 8;
			imgh = READ_LE_UINT16(&imhd->v7.height) / 8;
		} else {
			imgw = READ_LE_UINT16(&imhd->old.width) / 8;
			imgh = READ_LE_UINT16(&imhd->old.height) / 8;
		}
		imptr = getObjectImage(obim, 1);
	}
	assert(imptr);
	for (i = 0; i < imgw; i++) {
		tmp = xstrip + i;
		gdi.drawBitmap(imptr, vs, tmp, ydiff, imgw * 8, imgh * 8, i, 1, Gdi::dbAllowMaskOr | Gdi::dbObjectMode);
	}

	vst = &_verbs[verb];
	vst->curRect.right = vst->curRect.left + imgw * 8;
	vst->curRect.bottom = vst->curRect.top + imgh * 8;
	vst->oldRect = vst->curRect;

	gdi.enableZBuffer();

	vs->hasTwoBuffers = twobufs;
}

int ScummEngine::getVerbSlot(int id, int mode) const {
	int i;
	for (i = 1; i < _numVerbs; i++) {
		if (_verbs[i].verbid == id && _verbs[i].saveid == mode) {
			return i;
		}
	}
	return 0;
}

void ScummEngine::killVerb(int slot) {
	VerbSlot *vs;

	if (slot == 0)
		return;

	vs = &_verbs[slot];
	vs->verbid = 0;
	vs->curmode = 0;

	res.nukeResource(rtVerb, slot);

	if (_version <= 6 && vs->saveid == 0) {
		drawVerb(slot, 0);
		verbMouseOver(0);
	}
	vs->saveid = 0;
}

void ScummEngine::setVerbObject(uint room, uint object, uint verb) {
	const byte *obimptr;
	const byte *obcdptr;
	uint32 size, size2;
	FindObjectInRoom foir;
	int i;

	if (_heversion >= 70) { // Windows titles. Here we always ignore room
		room = getObjectRoom(object);
	}

	if (whereIsObject(object) == WIO_FLOBJECT)
		error("Can't grab verb image from flobject");

	if (_features & GF_OLD_BUNDLE) {
		for (i = (_numLocalObjects-1); i > 0; i--) {
			if (_objs[i].obj_nr == object) {
				findObjectInRoom(&foir, foImageHeader, object, room);
				size = READ_LE_UINT16(foir.obim);
				byte *ptr = res.createResource(rtVerb, verb, size + 2);
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
				res.createResource(rtVerb, verb, size + size2);
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
		res.createResource(rtVerb, verb, size);
		obimptr = getResourceAddress(rtRoom, room) - foir.roomptr + foir.obim;
		memcpy(getResourceAddress(rtVerb, verb), obimptr, size);
	}
}

} // End of namespace Scumm
