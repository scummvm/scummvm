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

#include "scumm/he/intern_he.h"
#include "scumm/he/basketball/basketball.h"
#include "scumm/he/logic_he.h"

namespace Scumm {

#define STICKY_CURSOR_VALUE 2

static void checkCursorBounds(const Common::Point &oldCursorPos, Common::Point *cursorPos) {
	Common::Rect windowRect(0, 0, 640, 480);

	// Make sure the cursor wasn't pushed out of the window...
	if ((cursorPos->x < (windowRect.left + 1)) && (oldCursorPos.x > windowRect.left)) {
		cursorPos->x = (windowRect.left + 1);
	}

	if ((cursorPos->x > (windowRect.right - 1)) && (oldCursorPos.x < windowRect.right)) {
		cursorPos->x = (windowRect.right - 1);
	}

	if ((cursorPos->y < (windowRect.top + 1)) && (oldCursorPos.y > windowRect.top)) {
		cursorPos->y = (windowRect.top + 1);
	}

	if ((cursorPos->y > (windowRect.bottom - 1)) && (oldCursorPos.y < windowRect.bottom)) {
		cursorPos->y = (windowRect.bottom - 1);
	}
}


int LogicHEBasketball::u32_userUpdateCursorPos(int xScrollVal, int yScrollVal) {
	int xChange = 0;
	int yChange = 0;

	Common::Point currentCursorPos = _vm->_mouse;

	// Update that position due to any camera scrolling that happened this frame...
	Common::Point newCursorPos;
	newCursorPos.x = currentCursorPos.x - xScrollVal;
	newCursorPos.y = currentCursorPos.y - yScrollVal;

	// Make sure the cursor wasn't pushed out of the window...
	checkCursorBounds(currentCursorPos, &newCursorPos);

	// Update the cursor position...
	_vm->_mouse = newCursorPos;

	// Calculate how much the cursor actually changed this frame...
	xChange = newCursorPos.x - currentCursorPos.x;
	yChange = newCursorPos.y - currentCursorPos.y;


	writeScummVar(_vm1->VAR_U32_USER_VAR_A, 1);
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, xChange);
	writeScummVar(_vm1->VAR_U32_USER_VAR_C, yChange);

	return 1;
}

int LogicHEBasketball::u32_userMakeCursorSticky(int lastCursorX, int lastCursorY) {
	int success = 0;

	int xChange = 0;
	int yChange = 0;

	Common::Point currentCursorPos = _vm->_mouse;

	Common::Point newCursorPos;

	// Update that position due to any camera scrolling that happened this frame...
	newCursorPos.x = lastCursorX + ((currentCursorPos.x - lastCursorX) / STICKY_CURSOR_VALUE);
	newCursorPos.y = lastCursorY + ((currentCursorPos.y - lastCursorY) / STICKY_CURSOR_VALUE);

	// Make sure the cursor wasn't pushed out of the window...
	checkCursorBounds(currentCursorPos, &newCursorPos);

	// Update the cursor position...
	_vm->_mouse = newCursorPos;

	// Calculate how much the cursor actually changed this frame...
	xChange = newCursorPos.x - currentCursorPos.x;
	yChange = newCursorPos.y - currentCursorPos.y;


	writeScummVar(_vm1->VAR_U32_USER_VAR_A, success);
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, xChange);
	writeScummVar(_vm1->VAR_U32_USER_VAR_C, yChange);

	return 1;
}

int LogicHEBasketball::u32_userCursorTrackMovingObject(int xChange, int yChange) {
	Common::Point currentCursorPos = _vm->_mouse;
	Common::Point newCursorPos;

	// Update that position due to any camera scrolling that happened this frame...
	newCursorPos.x = currentCursorPos.x + xChange;
	newCursorPos.y = currentCursorPos.y + yChange;

	// Make sure the cursor wasn't pushed out of the window...
	checkCursorBounds(currentCursorPos, &newCursorPos);

	// Update the cursor position...
	_vm->_mouse = newCursorPos;

	writeScummVar(_vm1->VAR_U32_USER_VAR_A, 1);
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, xChange);
	writeScummVar(_vm1->VAR_U32_USER_VAR_C, yChange);

	return 1;
}

int LogicHEBasketball::u32_userGetCursorPos() {
	writeScummVar(_vm1->VAR_U32_USER_VAR_A, _vm->_mouse.x);
	writeScummVar(_vm1->VAR_U32_USER_VAR_B, _vm->_mouse.y);

	return 1;
}

} // End of namespace Scumm
