/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike: Mark and Thomas Thurman.
 */

#include "common/random.h"
#include "common/system.h"

#include "graphics/cursorman.h"

#include "avalanche/avalanche.h"
#include "avalanche/graphics.h"
#include "avalanche/intro.h"

namespace Avalanche {

Intro::Intro(AvalancheEngine *vm) : _vm(vm), _displayCounter(0) {
	resetPlanes();
}

void Intro::resetPlanes() {
	memset(_planes, 0, sizeof(_planes));
}

void Intro::plotStar(uint8 plane, int x, int y) {
	int offset = (y * 40) + (x / 8);
	int bit = 7 - (x % 8); // choose correct bit inside byte (MSB first, left→right pixels)
	_planes[plane][offset] |= (1 << bit);
}

void Intro::plotStars(uint8 plane, int y) {
	int times = 0;
	int r = _vm->_rnd->getRandomNumber(6);
	if (r == 1 || r == 2 || r == 3)
		times = r;
	else
		return;

	for (int i = 0; i < times; i++)
		plotStar(plane, _vm->_rnd->getRandomNumber(319), y);
}

void Intro::moveStars() {
	for (int p = 0; p < 3; p++) {
		bool moveThisFrame = false;
		if ((p == 0 && (_displayCounter % 10 == 0)) || (p == 1 && (_displayCounter % 2 == 0)) || (p == 2))
			moveThisFrame = true;

		if (moveThisFrame) {
			// Shift plane index 0-2 (stars) DOWN by one line
			// Using memmove to shift 199 lines (7960 bytes) forward by 40 bytes
			memmove(_planes[p] + 40, _planes[p], 199*40);
			// Clear the top line
			memset(_planes[p], 0, 40);
			// Add new stars at the top
			plotStars(p, 0);
		}
	}
}

void Intro::combineAndDraw() {
	for (int y = 0; y < 200; y++) {
		uint8 *dstRow = (uint8 *)_vm->_graphics->getSurface().getBasePtr(0, y); // Get the pointer to the starting pixel of each Row
		for (int xByte = 0; xByte < 40; xByte++) {
			// Get 3 plane bytes for particular row + position
			uint8 b0 = _planes[0][y * 40 + xByte];
			uint8 b1 = _planes[1][y * 40 + xByte];
			uint8 b2 = _planes[2][y * 40 + xByte];

			// Process 8 pixels inside the byte (bit by bit)
			for (int bit = 7; bit >= 0; bit--) {
				uint8 color = kColorBlack; // default bg color
				
				// Priority matching the original 'our_palette' logic:
				// If Bit 3 (Text) is set -> White (15)
				// Else if Bit 2 (Fast) is set -> Light Gray (7)
				// Else if Bit 1 (Mid) is set -> Light Blue (9)
				// Else if Bit 0 (Slow) is set -> Blue (1)
				
				// Plane 3 (soon to be implemented)

				if (b2 & (1 << bit))
					color = kColorLightgray;
				else if (b1 & (1 << bit))
					color = kColorLightblue;
				else if (b0 & (1 << bit))
					color = kColorBlue;

				uint screenX = (xByte * 8 + (7 - bit)) * 2;
				dstRow[screenX] = color;
				dstRow[screenX + 1] = color;
			}
		}
	}
}

void Intro::run() {
	CursorMan.showMouse(false);
	_vm->_graphics->menuRestoreScreen();

    resetPlanes();
    _displayCounter = 0;

    // Pre-populate stars across the screen before we can start shifting
    for (int p = 0; p < 3; p++) {
        for (int y = 0; y < 200; y++) 
			plotStars(p, y);
    }

	while (!_vm->shouldQuit()) {
		uint32 beginLoop = g_system->getMillis();

        _displayCounter++;
        if (_displayCounter > 40) 
			_displayCounter = 1;

        moveStars();
		combineAndDraw();
		_vm->_graphics->refreshScreen();

		Common::Event event;
		while (_vm->getEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN) {
				CursorMan.showMouse(true);
				return;
			}
		}

		uint32 delay = g_system->getMillis() - beginLoop;
		if (delay < 55)
			g_system->delayMillis(55 - delay);
	}

	CursorMan.showMouse(true);
}

} // End of namespace Avalanche
