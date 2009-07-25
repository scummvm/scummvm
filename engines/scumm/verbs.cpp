/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * $URL$
 * $Id$
 *
 */

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm_v0.h"
#include "scumm/scumm_v7.h"
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
	int prep;
	const char *name;
};

static const VerbSettings v0VerbTable_English[] = {
	{ 1,  8, 0,   0, "Open"},
	{ 2,  8, 1,   0, "Close"},
	{ 3,  0, 2,   4, "Give"},
	{ 4, 32, 0,   0, "Turn on"},
	{ 5, 32, 1,   0, "Turn off"},
	{ 6, 32, 2,   2, "Fix"},
	{ 7, 24, 0,   0, "New Kid"},
	{ 8, 24, 1,   2, "Unlock"},
	{ 9,  0, 0,   0, "Push"},
	{10,  0, 1,   0, "Pull"},
	{11, 24, 2, 255, "Use"},
	{12,  8, 2,   0, "Read"},
	{13, 15, 0,   0, "Walk to"},
	{14, 15, 1,   0, "Pick up"},
	{15, 15, 2,   0, "What is"}
};

// FIXME: Replace * with the correct character
static const VerbSettings v0VerbTable_German[] = {
	{ 1,  0, 1,   0, "Ziehe"},
	{ 2,  0, 0,   0, "Dr<cke"},
	{ 3,  7, 2,   4, "Lese"},
	{ 4, 23, 0,   0, "Schlie*e auf"},
	{ 5, 37, 1,   0, "Ein"},
	{ 6, 37, 0,   0, "Aus"},
	{ 7, 34, 2,   0, "Person"},
	{ 8, 13, 1,   2, "Schlie*e"},
	{ 9,  7, 1,   0, "Nimm"},
	{10,  0, 2,   0, "Gebe"},
	{11, 23, 1, 255, "Repariere"},
	{12,  7, 0,   0, "$ffne"},
	{13, 13, 0,   0, "Gehe zu"},
	{14, 13, 2,   0, "Was ist"},
	{15, 23, 2,   0, "Benutz"}
};

void ScummEngine_v0::resetVerbs() {
	VirtScreen *virt = &_virtscr[kVerbVirtScreen];
	VerbSlot *vs;
	int i;

	for (i = 1; i < 16; i++)
		killVerb(i);

	for (i = 1; i < 16; i++) {
		vs = &_verbs[i];
		vs->verbid = v0VerbTable_English[i - 1].id;
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
		vs->prep = v0VerbTable_English[i - 1].prep;

		if (_language == Common::DE_DEU) {
			vs->curRect.left = v0VerbTable_German[i - 1].x_pos * 8;
			vs->curRect.top = v0VerbTable_German[i - 1].y_pos * 8 + virt->topline + 8;
			loadPtrToResource(rtVerb, i, (const byte*)v0VerbTable_German[i - 1].name);
		} else {
			vs->curRect.left = v0VerbTable_English[i - 1].x_pos * 8;
			vs->curRect.top = v0VerbTable_English[i - 1].y_pos * 8 + virt->topline + 8;
			loadPtrToResource(rtVerb, i, (const byte*)v0VerbTable_English[i - 1].name);
		}
	}
}

void ScummEngine_v0::setNewKidVerbs() {
	VirtScreen *virt = &_virtscr[kVerbVirtScreen];
	VerbSlot *vs;
	int i;

	for (i = 1; i < 16; i++)
		killVerb(i);

	for (i = 1; i < 4; i++) {
		vs = &_verbs[i];
		vs->verbid = i;
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
		vs->prep = 0;
		vs->curRect.left = (i * 8) * 8;
		vs->curRect.top = virt->topline + 8;

		Actor *a = derefActor(VAR(96 + i), "setNewKidVerbs");
		loadPtrToResource(rtVerb, i, (const byte*)a->getActorName());
	}
	setUserState(191);
}

void ScummEngine_v0::switchActor(int slot) {
	VAR(VAR_EGO) = VAR(97 + slot);
	actorFollowCamera(VAR(VAR_EGO));
	resetVerbs();
	resetSentence(false);
	setUserState(247);
}

void ScummEngine_v2::initV2MouseOver() {
	int i;
	int arrow_color, color, hi_color;

	if (_game.version == 2) {
		color = 13;
		hi_color = 14;
		arrow_color = 1;
	} else {
		color = 16;
		hi_color = 7;
		arrow_color = 6;
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
	VirtScreen *vs = &_virtscr[kVerbVirtScreen];
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
	int inventoryArea = (_game.platform == Common::kPlatformNES) ? 48: 32;
	int object = 0;

	y -= _virtscr[kVerbVirtScreen].topline;

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
		if (_game.version == 0) {
				_activeInventory = object;

		} else {
			runInputScript(kInventoryClickArea, object, 0);
		}
	}
}

void ScummEngine_v2::redrawV2Inventory() {
	VirtScreen *vs = &_virtscr[kVerbVirtScreen];
	int i;
	int max_inv;
	Common::Rect inventoryBox;
	int inventoryArea = (_game.platform == Common::kPlatformNES) ? 48: 32;
	int maxChars = (_game.platform == Common::kPlatformNES) ? 13: 18;

	_mouseOverBoxV2 = -1;

	if (!(_userState & 64))	// Don't draw inventory unless active
		return;

	// Clear on all invocations
	inventoryBox.top = vs->topline + inventoryArea;
	inventoryBox.bottom = vs->topline + vs->h;
	inventoryBox.left = 0;
	inventoryBox.right = vs->w;
	restoreBackground(inventoryBox);

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
		_string[1].right = _mouseOverBoxesV2[i].rect.right - 1;
		_string[1].color = _mouseOverBoxesV2[i].color;

		_v0ObjectInInventory = true;
		const byte *tmp = getObjOrActorName(obj);
		_v0ObjectInInventory = false;
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
		_string[1].right = _mouseOverBoxesV2[kInventoryUpArrow].rect.right - 1;
		_string[1].color = _mouseOverBoxesV2[kInventoryUpArrow].color;
		if (_game.platform == Common::kPlatformNES)
			drawString(1, (const byte *)"\x7E");
		else
			drawString(1, (const byte *)" \1\2");
	}

	// If necessary, draw "down" arrow
	if (_inventoryOffset + 4 < getInventoryCount(_scummVars[VAR_EGO])) {
		_string[1].xpos = _mouseOverBoxesV2[kInventoryDownArrow].rect.left;
		_string[1].ypos = _mouseOverBoxesV2[kInventoryDownArrow].rect.top + vs->topline;
		_string[1].right = _mouseOverBoxesV2[kInventoryDownArrow].rect.right - 1;
		_string[1].color = _mouseOverBoxesV2[kInventoryDownArrow].color;
		if (_game.platform == Common::kPlatformNES)
			drawString(1, (const byte *)"\x7F");
		else
			drawString(1, (const byte *)" \3\4");
	}
}

void ScummEngine::redrawVerbs() {
	if (_game.version <= 2 && !(_userState & 128)) // Don't draw verbs unless active
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

void ScummEngine_v0::handleMouseOver(bool updateInventory) {
	ScummEngine_v2::handleMouseOver(updateInventory);

	drawSentence();
}

#ifdef ENABLE_HE
void ScummEngine_v72he::checkExecVerbs() {
	VAR(VAR_MOUSE_STATE) = 0;

	if (_userPut <= 0 || _mouseAndKeyboardStat == 0)
		return;

	VAR(VAR_MOUSE_STATE) = _mouseAndKeyboardStat;

	ScummEngine::checkExecVerbs();
}
#endif

void ScummEngine::checkExecVerbs() {
	int i, over;
	VerbSlot *vs;

	if (_userPut <= 0 || _mouseAndKeyboardStat == 0)
		return;

	if (_mouseAndKeyboardStat < MBS_MAX_KEY) {
		/* Check keypresses */
		if (!(_game.id == GID_MONKEY && _game.platform == Common::kPlatformSegaCD)) {
			// This is disabled in the SegaCD version as the "vs->key" values setup
			// by script-17 conflict with the values expected by the generic keyboard
			// input script. See tracker item #1193185.
			vs = &_verbs[1];
			for (i = 1; i < _numVerbs; i++, vs++) {
				if (vs->verbid && vs->saveid == 0 && vs->curmode == 1) {
					if (_mouseAndKeyboardStat == vs->key) {
						// Trigger verb as if the user clicked it
						runInputScript(kVerbClickArea, vs->verbid, 1);
						return;
					}
				}
			}
		}

		if ((_game.id == GID_INDY4 || _game.id == GID_PASS) && _mouseAndKeyboardStat >= '0' && _mouseAndKeyboardStat <= '9') {
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

		if (_game.platform == Common::kPlatformFMTowns && _game.version == 3) {
			// HACK: In the FM-TOWNS games Indy3, Loom and Zak the most significant bit is set for special keys
			// like F5 (=0x8005) or joystick buttons (mask 0xFE00, e.g. SELECT=0xFE40 for the save/load menu).
			// Hence the distinction with (_mouseAndKeyboardStat < MBS_MAX_KEY) between mouse- and key-events is not applicable
			// to this games, so we have to remap the special keys here.
			if (_mouseAndKeyboardStat == 319) {
				_mouseAndKeyboardStat = 0x8005;
			}
		}

		if ((_game.platform == Common::kPlatformFMTowns && _game.id == GID_ZAK) &&
			(_mouseAndKeyboardStat >= 315 && _mouseAndKeyboardStat <= 318)) {
			// Hack: Handle switching to a person via F1-F4 keys.
			// This feature isn't available in the scripts of the FM-TOWNS version.
			int fKey = _mouseAndKeyboardStat - 314;
			int switchSlot = getVerbSlot(36, 0);
			// check if switch-verb is enabled
			if (_verbs[switchSlot].curmode == 1) {
				// Check if person is available (see script 23 from ZAK_FM-TOWNS and script 4 from ZAK_PC).
				// Zak: Var[144 Bit 15], Annie: Var[145 Bit 0], Melissa: Var[145 Bit 1], Leslie: Var[145 Bit 2]
				if (!readVar(0x890E + fKey)) {
					runInputScript(kVerbClickArea, 36 + fKey, 0);
				}
			}
			return;
		}

		// Generic keyboard input
		runInputScript(kKeyClickArea, _mouseAndKeyboardStat, 1);
	} else if (_mouseAndKeyboardStat & MBS_MOUSE_MASK) {
		VirtScreen *zone = findVirtScreen(_mouse.y);
		const byte code = _mouseAndKeyboardStat & MBS_LEFT_CLICK ? 1 : 2;

		// This could be kUnkVirtScreen.
		// Fixes bug #1536932: "MANIACNES: Crash on click in speechtext-area"
		if (!zone)
			return;

		over = findVerbAtPos(_mouse.x, _mouse.y);
		if (over != 0) {
			// Verb was clicked
			runInputScript(kVerbClickArea, _verbs[over].verbid, code);
		} else {
			// Scene was clicked
			runInputScript((zone->number == kMainVirtScreen) ? kSceneClickArea : kVerbClickArea, 0, code);
		}
	}
}

void ScummEngine_v2::checkExecVerbs() {
	int i, over;
	VerbSlot *vs;

	if (_userPut <= 0 || _mouseAndKeyboardStat == 0)
		return;

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

		// Generic keyboard input
		runInputScript(4, _mouseAndKeyboardStat, 1);
	} else if (_mouseAndKeyboardStat & MBS_MOUSE_MASK) {
		VirtScreen *zone = findVirtScreen(_mouse.y);
		const byte code = _mouseAndKeyboardStat & MBS_LEFT_CLICK ? 1 : 2;
		const int inventoryArea = (_game.platform == Common::kPlatformNES) ? 48: 32;

		// This could be kUnkVirtScreen.
		// Fixes bug #1536932: "MANIACNES: Crash on click in speechtext-area"
		if (!zone)
			return;

		if (zone->number == kVerbVirtScreen && _mouse.y <= zone->topline + 8) {
			// Click into V2 sentence line
			runInputScript(5, 0, 0);
		} else if (zone->number == kVerbVirtScreen && _mouse.y > zone->topline + inventoryArea) {
			// Click into V2 inventory
			checkV2Inventory(_mouse.x, _mouse.y);
		} else {
			over = findVerbAtPos(_mouse.x, _mouse.y);
			if (over != 0) {
				// Verb was clicked
				runInputScript(1, _verbs[over].verbid, code);
			} else {
				// Scene was clicked
				runInputScript((zone->number == kMainVirtScreen) ? kSceneClickArea : kVerbClickArea, 0, code);
			}
		}
	}
}

void ScummEngine_v0::runObject(int obj, int entry) {
	int prev = _v0ObjectInInventory;

	if (getVerbEntrypoint(obj, entry) != 0) {
		_v0ObjectInInventory = prev;
		runObjectScript(obj, entry, false, false, NULL);
	} else if (entry != 13 && entry != 15) {
		if (_activeVerb != 3) {
			VAR(9) = entry;
			runScript(3, 0, 0, 0);

		// For some reasons, certain objects don't have a "give" script
		} else if (VAR(5) > 0 && VAR(5) < 8) {
			if (_activeInventory)
				setOwnerOf(_activeInventory, VAR(5));
		}
	}
}

void ScummEngine_v2::runObject(int obj, int entry) {
	if (getVerbEntrypoint(obj, entry) != 0) {
		runObjectScript(obj, entry, false, false, NULL);
	} else if (entry != 13 && entry != 15) {
		VAR(9) = entry;
		runScript(3, 0, 0, 0);
	}

	_activeInventory = 0;
	_activeObject = 0;
	_activeVerb = 13;
}

bool ScummEngine_v0::verbMoveToActor(int actor) {
	Actor *a = derefActor(VAR(VAR_EGO), "checkExecVerbs");
	Actor *a2 =derefActor(actor, "checkExecVerbs");

	if (!a->_moving) {
		int dist =  getDist(a->getRealPos().x, a->getRealPos().y, a2->getRealPos().x, a2->getRealPos().y);
		if (dist > 5)
			a->startWalkActor(a2->getRealPos().x, a2->getRealPos().y, 1);
		else
			return false;

		return true;
	} 

	return true;
}

bool ScummEngine_v0::verbMove(int object, int objectIndex, bool invObject) {
	int x, y, dir;
	Actor *a = derefActor(VAR(VAR_EGO), "checkExecVerbs");

	if (_currentMode != 3 && _currentMode != 2)
		return false;

	if (a->_moving)
		return true;

	_v0ObjectIndex = true;
	getObjectXYPos(objectIndex, x, y, dir);
	_v0ObjectIndex = false;
	// Detect distance from target object
	int dist =  getDist(a->getRealPos().x, a->getRealPos().y, x, y);

	// FIXME: This should be changed once the walkbox problem is fixed
	if (dist > 0x5) {
		a->startWalkActor(x, y, dir);
		VAR(6) = x;
		VAR(7) = y;
		return true;
	} else {
		// Finished walk, are we picking up the item?
		if (_verbPickup) {
			int oldActive = _activeObject, oldIndex = _activeObjectIndex;
			_activeObject = object;
			_activeObjectIndex = objectIndex;

			_v0ObjectIndex = true;
			// Execute pickup 
			runObject(objectIndex, 14);			
			_v0ObjectIndex = false;

			_activeObject = oldActive;
			_activeObjectIndex = oldIndex;

			// Finished picking up
			_verbPickup = false;
		}	
	}

	return false;
}

bool ScummEngine_v0::verbObtain(int obj, int objIndex) {
	bool didPickup = false;

	int prep, where = whereIsObjectInventory(obj);

	if (objIndex == 0)
		return false;

	if (where != WIO_INVENTORY) {
		_v0ObjectIndex = true;
		prep = verbPrep(objIndex);

		if (prep == 1 || prep == 4) {
			if (_activeVerb != 13 && _activeVerb != 14) {
				_verbPickup = true;
				didPickup = true;
			}
		} else {
			_verbPickup = false;
		}

		if (verbMove(obj, objIndex, false))
			return true;

		if (didPickup && (prep == 1 || prep == 4))
			if (_activeVerb != 13 && _activeVerb != 14)
				_activeInventory = obj;
	}

	return false;
}

int ScummEngine_v0::verbPrep(int object) {
	if (!_v0ObjectInInventory) 	
		_v0ObjectIndex = true;
	else
		_v0ObjectIndex = false;
	byte *ptr = getOBCDFromObject(object);
	_v0ObjectIndex = false;
	assert(ptr);
	return (*(ptr + 11) >> 5);
}

bool ScummEngine_v0::verbExecutes(int object, bool inventory) {
	_v0ObjectInInventory = inventory;
	int prep = verbPrep(object);

	if (prep == 2 || prep == 0) {
		return true;
	}

	return false;
}

bool ScummEngine_v0::verbExec() {
	int prep = 0;
	int entry = (_currentMode != 0 && _currentMode != 1) ? _activeVerb : 15;

	if ((!_activeInvExecute && _activeObject && getObjectIndex(_activeObject) == -1)) {
		resetSentence();
		return false;
	}

	// Lets try walk to the object
	if (_activeObject && _activeObjectIndex && !_activeObjectObtained && _currentMode != 0) {
		prep = verbPrep(_activeObjectIndex);
		
		if (verbObtain(_activeObject, _activeObjectIndex))
			return true;

		_activeObjectObtained = true;
	}

	if (_activeObject2 && _activeObject2Index && !_activeObject2Obtained && _currentMode != 0) {
		prep = verbPrep(_activeObject2Index);

		_v0ObjectInInventory = false;
		if (verbObtain(_activeObject2, _activeObject2Index))
			return true;
		
		if (prep != 1 && prep != 4) {
			_activeInventory = _activeObject;
			_activeObject = _activeObject2;
			_activeObjectIndex = _activeObject2Index;
			_activeObject2 = 0;
			_activeObject2Index = 0;
		}

		_activeObject2Obtained = true;
	}

	// Give-To
	if (_activeVerb == 3 && _activeInventory && _activeActor) {
		// FIXME: Actors need to turn and face each other
		// And walk to each other
		if (verbMoveToActor(_activeActor))
			return true;

		_v0ObjectInInventory = true;
		VAR(5) = _activeActor;
		runObject(_activeInventory , 3);
		_v0ObjectInInventory = false;

		resetSentence();
		return false;
	}

	if (_activeActor) {
		_v0ObjectIndex = true;
		runObject(_activeActor, entry);
		_v0ObjectIndex = false;
		_verbExecuting = false;

		resetSentence();
		return false;
	}

	// If we've finished walking (now near target), execute the action
	if (_activeObject && _activeObjectIndex && verbPrep(_activeObjectIndex) == 2) {
		_v0ObjectIndex = true;
		runObject(_activeObjectIndex, entry);
		_v0ObjectIndex = false;
		_verbExecuting = false;

		if ((_currentMode == 3 || _currentMode == 2) && _activeVerb == 13)
			return false;

		resetSentence();
		return false;
	}

	// We acted on an inventory item
	if (_activeInventory && verbExecutes(_activeInventory, true) && _activeVerb != 3) {
		_v0ObjectInInventory = true;
		runObject(_activeInventory, _activeVerb);

		_verbExecuting = false;

		if (_currentMode == 3 && _activeVerb == 13) {
			resetSentence(true);
			return false;
		}
		
		resetSentence();
		return false;
	}

	if (_activeObject) {
		_v0ObjectIndex = true;
		runObject(_activeObjectIndex, entry);
		_v0ObjectIndex = false;
	} else if (_activeInventory) {
		if (verbExecutes(_activeInventory, true) == false) {
			if (_activeObject2 && verbExecutes(_activeObject2, true)) {
				_v0ObjectInInventory = true;
				
				_activeObject = _activeInventory;
				_activeInventory = _activeObject2;
				
				runObject(_activeObject, _activeVerb);
			} else {
				_v0ObjectInInventory = true;
				runObject(_activeInventory, _activeVerb);
			}
		} else {
			runObject(_activeInventory, _activeVerb);
			_v0ObjectInInventory = true;
		}
	}

	_verbExecuting = false;

	if (_activeVerb == 13) {
		resetSentence(true);
		return false;
	}

	resetSentence();

	return false;
}

void ScummEngine_v0::checkExecVerbs() {
	Actor *a;
	VirtScreen *zone = findVirtScreen(_mouse.y);

	// Is a verb currently executing
	if (_verbExecuting) {
		// Check if mouse click
		if (_mouseAndKeyboardStat & MBS_MOUSE_MASK) {
			int over = findVerbAtPos(_mouse.x, _mouse.y);
			int act  = getActorFromPos(_virtualMouse.x, _virtualMouse.y);
			int obj  = findObject(_virtualMouse.x, _virtualMouse.y);
			
			if (over && over != _activeVerb) {
				_activeVerb = over;
				_verbExecuting = false;
				return;
			}

			if (!obj && !act && !over) {
				resetSentence(false);	
			} else {
				a = derefActor(VAR(VAR_EGO), "checkExecVerbs");
				a->stopActorMoving();
			}
		} else {
			if (_verbExecuting && !verbExec())
				return;
		}
	}

	// What-Is selected, any object we hover over is selected, on mouse press we set to WalkTo
	if (_activeVerb == 15) {
		int obj = findObject(_virtualMouse.x, _virtualMouse.y);
		int objIdx = findObjectIndex(_virtualMouse.x, _virtualMouse.y);
		_activeObject = obj;
		_activeObjectIndex = objIdx;

		if ((_mouseAndKeyboardStat & MBS_MOUSE_MASK))
			_activeVerb = 13;	// Walk-To

		return;
	}

	if (_userPut <= 0 || _mouseAndKeyboardStat == 0)
		return;

	if (_mouseAndKeyboardStat < MBS_MAX_KEY) {
		/* Check keypresses */
		// TODO
	} else if (_mouseAndKeyboardStat & MBS_MOUSE_MASK) {
		if (zone->number == kVerbVirtScreen && _mouse.y <= zone->topline + 8) {
			// TODO
		} else if (zone->number == kVerbVirtScreen && _mouse.y > zone->topline + 32) {
			int prevInventory = _activeInventory;

			// Click into V2 inventory
			checkV2Inventory(_mouse.x, _mouse.y);
			if (!_activeInventory)
				return;
			
			// Did we just change the selected inventory item?
			if (prevInventory && prevInventory != _activeInventory && _activeInventory != _activeObject2) {
				_activeObject = 0;
				_activeInvExecute = true;
				_activeObject2Inv = true;
				_activeObject2 = _activeInventory;
				_activeInventory = prevInventory;
				return;
			}

			// is the new selected inventory the same as the last selected?, reset to previous if it is
			if (_activeInventory == _activeObject2)
				_activeInventory = prevInventory;

			// Inventory Selected changed
			if (prevInventory != _activeInventory)
				if (!_activeObject2 || prevInventory != _activeObject2)
					return;

			if (_activeVerb == 11 && !((!(_activeObject || _activeInventory)) || !_activeObject2))
				return;
		} else {
			int over = findVerbAtPos(_mouse.x, _mouse.y);
			int act  = getActorFromPos(_virtualMouse.x, _virtualMouse.y);
			int obj  = findObject(_virtualMouse.x, _virtualMouse.y);
			int objIdx = findObjectIndex(_virtualMouse.x, _virtualMouse.y);

			if ((_activeObject || _activeInventory) && act) {
				obj = 0;
				objIdx = 0;
			}

			// Handle New Kid verb options
			if (_activeVerb == 7 || over == 7) {
				// Disable New-Kid (in the secret lab)
				if (_currentMode == 2 || _currentMode == 0)
					return;

				if (_activeVerb != 7) {
					_activeVerb = over;
					over = 0;
				} 

				if (over) {
					_activeVerb = 13;
					switchActor(_verbs[over].verbid - 1);
					return;
				}

				setNewKidVerbs();

				return;
			}
			
			// Clicked on nothing, walk here?
			if (!over && !act && _activeVerb == 13 && !obj && _currentMode != 0) {
				// Clear all selected
				resetSentence();

				// 0xB31
				VAR(6) = _virtualMouse.x / V12_X_MULTIPLIER;
				VAR(7) = _virtualMouse.y / V12_Y_MULTIPLIER;

				if (zone->number == kMainVirtScreen) {
					a = derefActor(VAR(VAR_EGO), "checkExecVerbs");
					a->stopActorMoving();
					a->startWalkActor(_virtualMouse.x / V12_X_MULTIPLIER, _virtualMouse.y / V12_Y_MULTIPLIER, -1);
				}
				return;
			}

			// No new verb, use previous
			if (over == 0)
				over = _activeVerb;

			// No verb selected, use walk-to
			if (!_activeVerb)
				_activeVerb = over = 13;		// Walk-To

			// New verb selected
			if (_activeVerb != over) {
				_activeVerb = over;
				if (_activeVerb == 13) {
					resetSentence();
				}
				return;
			}

			// Only allowing targetting actors if its the GIVE/USE verb
			if (_activeVerb == 3 || _activeVerb == 11) {
				// Different actor selected?
				if (act) {
					if (_activeActor != act) {
						_activeActor = act;
						return;
					}
				}
			}

			if (obj && obj != _activeObject) {
				if (!_activeObject)
					if (_activeInventory)
						_activeInvExecute = true;

				// USE
				if (_activeVerb == 11 || _activeVerb == 8) {
					if (obj != _activeObject || obj != _activeObject2) {
						if (!_activeObject || _activeInventory) {
							_activeObject = obj;
							_activeObjectIndex = objIdx;
							return;
						} else {
							if (_activeObject2 != obj) {
								_activeObject2 = obj;
								_activeObject2Index = objIdx;
								return;
							}
						}
					}
				} else {
					_activeObject = obj;
					_activeObjectIndex = objIdx;
						
					if (_activeVerb != 13)
						return;
					
					//return;
				}
			}
		}

		_verbExecuting = true;

	}	// mouse k/b action
}

void ScummEngine::verbMouseOver(int verb) {
	// Don't do anything unless verbs are active
	if (_game.version <= 2 && !(_userState & 128))
		return;

	if (_game.id == GID_FT)
		return;

	if (_verbMouseOver != verb) {
		if (_verbs[_verbMouseOver].type != kImageVerbType) {
			drawVerb(_verbMouseOver, 0);
			_verbMouseOver = verb;
		}

		if (_verbs[verb].type != kImageVerbType && _verbs[verb].hicolor) {
			drawVerb(verb, 1);
			_verbMouseOver = verb;
		}
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

#ifdef ENABLE_SCUMM_7_8
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

		// Set the specified charset id
		int oldID = _charset->getCurID();
		_charset->setCurID(vs->charset_nr);

		// Compute the text rect
		vs->curRect.right = 0;
		vs->curRect.bottom = 0;
		const byte *msg2 = msg;
		while (*msg2) {
			const int charWidth = _charset->getCharWidth(*msg2);
			const int charHeight = _charset->getCharHeight(*msg2);
			vs->curRect.right += charWidth;
			if (vs->curRect.bottom < charHeight)
				vs->curRect.bottom = charHeight;
			msg2++;
		}
		vs->curRect.right += vs->curRect.left;
		vs->curRect.bottom += vs->curRect.top;
		vs->oldRect = vs->curRect;

		const int maxWidth = _screenWidth - vs->curRect.left;
		if (_charset->getStringWidth(0, buf) > maxWidth && _game.version == 8) {
			byte tmpBuf[384];
			memcpy(tmpBuf, msg, 384);

			int len = resStrLen(tmpBuf) - 1;
			while (len >= 0) {
				if (tmpBuf[len] == ' ') {
					tmpBuf[len] = 0;
					if (_charset->getStringWidth(0, tmpBuf) <= maxWidth) {
						break;
					}
				}
				--len;
			}
			enqueueText(tmpBuf, vs->curRect.left, vs->curRect.top, color, vs->charset_nr, vs->center);
			if (len >= 0) {
				enqueueText(&msg[len + 1], vs->curRect.left, vs->curRect.top + _verbLineSpacing, color, vs->charset_nr, vs->center);
				vs->curRect.bottom += _verbLineSpacing;
			}
		} else {
			enqueueText(msg, vs->curRect.left, vs->curRect.top, color, vs->charset_nr, vs->center);
		}
		_charset->setCurID(oldID);
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
	} else if (_game.id != GID_FT) {
		restoreVerbBG(verb);
	}
}

void ScummEngine::restoreVerbBG(int verb) {

	VerbSlot *vs;

	vs = &_verbs[verb];

	if (vs->oldRect.left != -1) {
		restoreBackground(vs->oldRect, vs->bkcolor);
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
	uint32 size;

	if ((vs = findVirtScreen(y)) == NULL)
		return;

	_gdi->disableZBuffer();

	twobufs = vs->hasTwoBuffers;
	vs->hasTwoBuffers = false;

	xstrip = x / 8;
	ydiff = y - vs->topline;

	obim = getResourceAddress(rtVerb, verb);
	assert(obim);
	if (_game.features & GF_OLD_BUNDLE) {
		imgw = obim[0];
		imgh = obim[1] / 8;
		imptr = obim + 2;
	} else if (_game.features & GF_SMALL_HEADER) {
		size = READ_LE_UINT32(obim);

		imgw = (*(obim + size + 11));
		imgh = (*(obim + size + 17)) / 8;
		imptr = getObjectImage(obim, 1);
	} else {
		const ImageHeader *imhd = (const ImageHeader *)findResourceData(MKID_BE('IMHD'), obim);
		if (_game.version >= 7) {
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
		_gdi->drawBitmap(imptr, vs, tmp, ydiff, imgw * 8, imgh * 8, i, 1, Gdi::dbAllowMaskOr | Gdi::dbObjectMode);
	}

	vst = &_verbs[verb];
	vst->curRect.right = vst->curRect.left + imgw * 8;
	vst->curRect.bottom = vst->curRect.top + imgh * 8;
	vst->oldRect = vst->curRect;

	_gdi->enableZBuffer();

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

	_res->nukeResource(rtVerb, slot);

	if (_game.version <= 6 && vs->saveid == 0) {
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

	if (_game.heversion >= 70) { // Windows titles. Here we always ignore room
		room = getObjectRoom(object);
	}

	if (whereIsObject(object) == WIO_FLOBJECT)
		error("Can't grab verb image from flobject");

	if (_game.features & GF_OLD_BUNDLE) {
		for (i = (_numLocalObjects-1); i > 0; i--) {
			if (_objs[i].obj_nr == object) {
				findObjectInRoom(&foir, foImageHeader, object, room);
				size = READ_LE_UINT16(foir.obim);
				byte *ptr = _res->createResource(rtVerb, verb, size + 2);
				obcdptr = getResourceAddress(rtRoom, room) + getOBCDOffs(object);
				ptr[0] = *(obcdptr + 9);	// Width
				ptr[1] = *(obcdptr + 15);	// Height
				memcpy(ptr + 2, foir.obim, size);
				return;
			}
		}
	} else if (_game.features & GF_SMALL_HEADER) {
		for (i = (_numLocalObjects-1); i > 0; i--) {
			if (_objs[i].obj_nr == object) {
				// FIXME: the only thing we need from the OBCD is the image size!
				// So we could use almost the same code (except for offsets)
				// as in the GF_OLD_BUNDLE code. But of course that would break save games
				// unless we insert special conversion code... <sigh>
				findObjectInRoom(&foir, foImageHeader, object, room);
				size = READ_LE_UINT32(foir.obim);
				obcdptr = getResourceAddress(rtRoom, room) + getOBCDOffs(object);
				size2 = READ_LE_UINT32(obcdptr);
				_res->createResource(rtVerb, verb, size + size2);
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
		_res->createResource(rtVerb, verb, size);
		obimptr = getResourceAddress(rtRoom, room) - foir.roomptr + foir.obim;
		memcpy(getResourceAddress(rtVerb, verb), obimptr, size);
	}
}

} // End of namespace Scumm
