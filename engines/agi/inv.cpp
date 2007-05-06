/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

#include "common/stdafx.h"

#include "agi/agi.h"
#include "agi/sprite.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"

namespace Agi {

/*
 * Messages and coordinates
 */

#define NOTHING_X	16
#define NOTHING_Y	3
#define NOTHING_MSG	"nothing"

#define ANY_KEY_X	4
#define ANY_KEY_Y	24
#define ANY_KEY_MSG	"Press a key to return to the game"

#define YOUHAVE_X	11
#define YOUHAVE_Y	0
#define YOUHAVE_MSG	"You are carrying:"

#define SELECT_X	2
#define SELECT_Y	24
#define SELECT_MSG	"Press ENTER to select, ESC to cancel"

void AgiEngine::printItem(int n, int fg, int bg) {
	printText(objectName(_intobj[n]), 0, n % 2 ? 39 - strlen(objectName(_intobj[n])) : 1,
			(n / 2) + 2, 40, fg, bg);
}

int AgiEngine::findItem() {
	int r, c;

	r = g_mouse.y / CHAR_LINES;
	c = g_mouse.x / CHAR_COLS;

	debugC(6, kDebugLevelInventory, "r = %d, c = %d", r, c);

	if (r < 2)
		return -1;

	return (r - 2) * 2 + (c > 20);
}

int AgiEngine::showItems() {
	unsigned int x, i;

	for (x = i = 0; x < _game.numObjects; x++) {
		if (objectGetLocation(x) == EGO_OWNED) {
			/* add object to our list! */
			_intobj[i] = x;
			printItem(i, STATUS_FG, STATUS_BG);
			i++;
		}
	}

	if (i == 0) {
		printText(NOTHING_MSG, 0, NOTHING_X, NOTHING_Y, 40, STATUS_FG, STATUS_BG);
	}

	return i;
}

void AgiEngine::selectItems(int n) {
	int fsel = 0;
	bool exit_select = false;

	while (!exit_select) {
		if (n > 0)
			printItem(fsel, STATUS_BG, STATUS_FG);

		switch (waitAnyKey()) {
		case KEY_ENTER:
			setvar(vSelItem, _intobj[fsel]);
			exit_select = true;
			break;
		case KEY_ESCAPE:
			setvar(vSelItem, 0xff);
			exit_select = true;
			break;
		case KEY_UP:
			if (fsel >= 2)
				fsel -= 2;
			break;
		case KEY_DOWN:
			if (fsel + 2 < n)
				fsel += 2;
			break;
		case KEY_LEFT:
			if (fsel % 2 == 1)
				fsel--;
			break;
		case KEY_RIGHT:
			if (fsel % 2 == 0 && fsel + 1 < n)
				fsel++;
			break;
		case BUTTON_LEFT:{
				int i = findItem();
				if (i >= 0 && i < n) {
					setvar(vSelItem, _intobj[fsel = i]);
					debugC(6, kDebugLevelInventory, "item found: %d", fsel);
					showItems();
					printItem(fsel, STATUS_BG, STATUS_FG);
					_gfx->doUpdate();
					exit_select = true;
				}
				break;
			}
		default:
			break;
		}
		
		if (!exit_select) {
			showItems();
			_gfx->doUpdate();
		}
	}

	debugC(6, kDebugLevelInventory, "selected: %d", fsel);
}

/*
 * Public functions
 */

/**
 * Display inventory items.
 */
void AgiEngine::inventory() {
	int oldFg, oldBg;
	int n;

	/* screen is white with black text */
	oldFg = _game.colorFg;
	oldBg = _game.colorBg;
	_game.colorFg = 0;
	_game.colorBg = 15;
	_gfx->clearScreen(_game.colorBg);

	printText(YOUHAVE_MSG, 0, YOUHAVE_X, YOUHAVE_Y, 40, STATUS_FG, STATUS_BG);

	/* FIXME: doesn't check if objects overflow off screen... */

	_intobj = (uint8 *)malloc(4 + _game.numObjects);
	memset(_intobj, 0, (4 + _game.numObjects));

	n = showItems();

	if (getflag(fStatusSelectsItems)) {
		printText(SELECT_MSG, 0, SELECT_X, SELECT_Y, 40, STATUS_FG, STATUS_BG);
	} else {
		printText(ANY_KEY_MSG, 0, ANY_KEY_X, ANY_KEY_Y, 40, STATUS_FG, STATUS_BG);
	}

	_gfx->flushScreen();

	/* If flag 13 is set, we want to highlight & select an item.
	 * opon selection, put objnum in var 25. Then on esc put in
	 * var 25 = 0xff.
	 */

	if (getflag(fStatusSelectsItems))
		selectItems(n);

	free(_intobj);

	if (!getflag(fStatusSelectsItems))
		waitAnyKey();

	_gfx->clearScreen(0);
	writeStatus();
	_picture->showPic();
	_game.colorFg = oldFg;
	_game.colorBg = oldBg;
	_game.hasPrompt = 0;
	flushLines(_game.lineUserInput, 24);
}

} // End of namespace Agi
