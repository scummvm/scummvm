/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/file.h"

#include "agos/agos.h"
#include "agos/intern.h"

using Common::File;

namespace AGOS {

void AGOSEngine::loadMenuFile() {
	Common::File in;

	in.open(getFileName(GAME_MENUFILE));
	if (in.isOpen() == false) {
		error("loadMenuFile: Can't load menus file '%s'", getFileName(GAME_MENUFILE));
	}

	uint fileSize = in.size();
	_menuBase = (byte *)malloc(fileSize);
	if (_menuBase == NULL)
		error("loadMenuFile: Out of memory for menu data");
	in.read(_menuBase, fileSize);
	in.close();
}

// Elvira 1 specific
void AGOSEngine::drawMenuStrip(uint windowNum, uint menuNum) {
	WindowBlock *window = _windowArray[windowNum % 8];

	mouseOff();

	byte *srcPtr = _menuBase;
	int menu = (menuNum != 0) ? menuNum * 4 + 1 : 0;

	while (menu--) {
		if (READ_LE_UINT16(srcPtr) != 0xFFFF) {
			srcPtr += 2;
			while (*srcPtr != 0)
				srcPtr++;
			srcPtr++;
		} else {
			srcPtr += 2;
		}
	}

	clearWindow(window);

	int newline = 0;
	while (READ_LE_UINT16(srcPtr) != 0xFFFF) {
		byte *tmp = srcPtr;
		srcPtr += 2;

		if (newline != 0) {
			windowPutChar(window, 10);
		}

		uint len = 0;
		while (*srcPtr != 0 && *srcPtr != 1) {
			len++;
			srcPtr++;
		}
		if (*srcPtr == 1)
			srcPtr++;

		uint maxLen = window->textMaxLength - len;

		if (window->flags & 1)
			window->textColumnOffset += 4;

		maxLen /= 2;
		while (maxLen--)
			windowPutChar(window, 32);

		srcPtr = tmp;
		uint verb = READ_BE_UINT16(srcPtr); srcPtr += 2;

		while (*srcPtr != 0) {
			windowPutChar(window, *srcPtr++);
		}
		srcPtr++;

		if (verb != 0xFFFE) {
			HitArea *ha = findEmptyHitArea();
			ha->x = window->x * 8 + 3;
			ha->y = window->textRow * 8 + window->y;
			ha->data = menuNum;
			ha->width = window->width * 8 - 6;
			ha->height = 7;
			ha->flags = kBFBoxInUse | kBFInvertTouch;
			ha->id = 30000;
			ha->priority = 1;
			ha->verb = verb;
		}

		newline = 0xFFFF;
	}

	mouseOn();
}


} // End of namespace AGOS
