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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/scalpel/scalpel_screen.h"
#include "sherlock/scalpel/scalpel.h"

namespace Sherlock {

namespace Scalpel {

ScalpelScreen::ScalpelScreen(SherlockEngine *vm) : Screen(vm) {
}

void ScalpelScreen::makeButton(const Common::Rect &bounds, int textX,
		const Common::String &str) {

	Surface &bb = *_backBuffer;
	bb.fillRect(Common::Rect(bounds.left, bounds.top, bounds.right, bounds.top + 1), BUTTON_TOP);
	bb.fillRect(Common::Rect(bounds.left, bounds.top, bounds.left + 1, bounds.bottom), BUTTON_TOP);
	bb.fillRect(Common::Rect(bounds.right - 1, bounds.top, bounds.right, bounds.bottom), BUTTON_BOTTOM);
	bb.fillRect(Common::Rect(bounds.left + 1, bounds.bottom - 1, bounds.right, bounds.bottom), BUTTON_BOTTOM);
	bb.fillRect(Common::Rect(bounds.left + 1, bounds.top + 1, bounds.right - 1, bounds.bottom - 1), BUTTON_MIDDLE);

	gPrint(Common::Point(textX, bounds.top), COMMAND_HIGHLIGHTED, "%c", str[0]);
	gPrint(Common::Point(textX + charWidth(str[0]), bounds.top),
		COMMAND_FOREGROUND, "%s", str.c_str() + 1);
}

void ScalpelScreen::buttonPrint(const Common::Point &pt, byte color, bool slamIt,
		const Common::String &str) {
	int xStart = pt.x - stringWidth(str) / 2;

	if (color == COMMAND_FOREGROUND) {
		// First character needs to be highlighted
		if (slamIt) {
			print(Common::Point(xStart, pt.y + 1), COMMAND_HIGHLIGHTED, "%c", str[0]);
			print(Common::Point(xStart + charWidth(str[0]), pt.y + 1),
				COMMAND_FOREGROUND, "%s", str.c_str() + 1);
		} else {
			gPrint(Common::Point(xStart, pt.y), COMMAND_HIGHLIGHTED, "%c", str[0]);
			gPrint(Common::Point(xStart + charWidth(str[0]), pt.y),
				COMMAND_FOREGROUND, "%s", str.c_str() + 1);
		}
	} else if (slamIt) {
		print(Common::Point(xStart, pt.y + 1), color, "%s", str.c_str());
	} else {
		gPrint(Common::Point(xStart, pt.y), color, "%s", str.c_str());
	}
}

void ScalpelScreen::makePanel(const Common::Rect &r) {
	_backBuffer->fillRect(r, BUTTON_MIDDLE);
	_backBuffer->hLine(r.left, r.top, r.right - 2, BUTTON_TOP);
	_backBuffer->hLine(r.left + 1, r.top + 1, r.right - 3, BUTTON_TOP);
	_backBuffer->vLine(r.left, r.top, r.bottom - 1, BUTTON_TOP);
	_backBuffer->vLine(r.left + 1, r.top + 1, r.bottom - 2, BUTTON_TOP);

	_backBuffer->vLine(r.right - 1, r.top, r.bottom - 1, BUTTON_BOTTOM);
	_backBuffer->vLine(r.right - 2, r.top + 1, r.bottom - 2, BUTTON_BOTTOM);
	_backBuffer->hLine(r.left, r.bottom - 1, r.right - 1, BUTTON_BOTTOM);
	_backBuffer->hLine(r.left + 1, r.bottom - 2, r.right - 1, BUTTON_BOTTOM);
}

void ScalpelScreen::makeField(const Common::Rect &r) {
	_backBuffer->fillRect(r, BUTTON_MIDDLE);
	_backBuffer->hLine(r.left, r.top, r.right - 1, BUTTON_BOTTOM);
	_backBuffer->hLine(r.left + 1, r.bottom - 1, r.right - 1, BUTTON_TOP);
	_backBuffer->vLine(r.left, r.top + 1, r.bottom - 1, BUTTON_BOTTOM);
	_backBuffer->vLine(r.right - 1, r.top + 1, r.bottom - 2, BUTTON_TOP);
}

} // End of namespace Scalpel

} // End of namespace Sherlock
