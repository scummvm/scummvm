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
	_backBuffer1.create(320, 200, g_system->getScreenFormat());
	_backBuffer2.create(320, 200, g_system->getScreenFormat());
	activateBackBuffer1();
}

void ScalpelScreen::makeButton(const Common::Rect &bounds, int textX,
		const Common::String &buttonText, bool textContainsHotkey) {

	Surface &bb = _backBuffer;
	bb.fillRect(Common::Rect(bounds.left, bounds.top, bounds.right, bounds.top + 1), BUTTON_TOP);
	bb.fillRect(Common::Rect(bounds.left, bounds.top, bounds.left + 1, bounds.bottom), BUTTON_TOP);
	bb.fillRect(Common::Rect(bounds.right - 1, bounds.top, bounds.right, bounds.bottom), BUTTON_BOTTOM);
	bb.fillRect(Common::Rect(bounds.left + 1, bounds.bottom - 1, bounds.right, bounds.bottom), BUTTON_BOTTOM);
	bb.fillRect(Common::Rect(bounds.left + 1, bounds.top + 1, bounds.right - 1, bounds.bottom - 1), BUTTON_MIDDLE);

	buttonPrint(Common::Point(textX, bounds.top), COMMAND_FOREGROUND, false, buttonText, textContainsHotkey);
}

// ButtonText is supposed to have its hotkey as a prefix. The hotkey will get highlighted.
void ScalpelScreen::buttonPrint(const Common::Point &pt, uint color, bool slamIt,
		const Common::String &buttonText, bool textContainsHotkey) {
	int xStart = pt.x;
	int skipTextOffset = textContainsHotkey ? +1 : 0; // skip first char in case text contains hotkey

	// Center text around given x-coordinate
	if (textContainsHotkey) {
		xStart -= (stringWidth(Common::String(buttonText.c_str() + 1)) / 2);
	} else {
		xStart -= (stringWidth(buttonText) / 2);
	}

	if (color == COMMAND_FOREGROUND) {
		uint16 prefixOffsetX = 0;
		byte hotkey = buttonText[0];

		// Hotkey needs to be highlighted
		if (textContainsHotkey) {
			Common::String prefixText = Common::String(buttonText.c_str() + 1);
			uint16 prefixTextLen = prefixText.size();
			uint16 prefixTextPos = 0;

			// Hotkey was passed additionally, we search for the hotkey inside the button text and
			// remove it from there. We then draw the whole text as highlighted and afterward
			// the processed text again as regular text (without the hotkey)
			while (prefixTextPos < prefixTextLen) {
				if (prefixText[prefixTextPos] == hotkey) {
					// Hotkey found, remove remaining text
					while (prefixTextPos < prefixText.size()) {
						prefixText.deleteLastChar();
					}
					break;
				}
				prefixTextPos++;
			}

			if (prefixTextPos < prefixTextLen) {
				// only adjust in case hotkey character was actually found
				prefixOffsetX = stringWidth(prefixText);
			}
		}

		if (slamIt) {
			print(Common::Point(xStart, pt.y + 1),
				COMMAND_FOREGROUND, "%s", buttonText.c_str() + skipTextOffset);
			print(Common::Point(xStart + prefixOffsetX, pt.y + 1), COMMAND_HIGHLIGHTED, "%c", hotkey);
		} else {
			gPrint(Common::Point(xStart, pt.y),
				COMMAND_FOREGROUND, "%s", buttonText.c_str() + skipTextOffset);
			gPrint(Common::Point(xStart + prefixOffsetX, pt.y), COMMAND_HIGHLIGHTED, "%c", hotkey);
		}
	} else if (slamIt) {
		print(Common::Point(xStart, pt.y + 1), color, "%s", buttonText.c_str() + skipTextOffset);
	} else {
		gPrint(Common::Point(xStart, pt.y), color, "%s", buttonText.c_str() + skipTextOffset);
	}
}

void ScalpelScreen::makePanel(const Common::Rect &r) {
	_backBuffer.fillRect(r, BUTTON_MIDDLE);
	_backBuffer.hLine(r.left, r.top, r.right - 2, BUTTON_TOP);
	_backBuffer.hLine(r.left + 1, r.top + 1, r.right - 3, BUTTON_TOP);
	_backBuffer.vLine(r.left, r.top, r.bottom - 1, BUTTON_TOP);
	_backBuffer.vLine(r.left + 1, r.top + 1, r.bottom - 2, BUTTON_TOP);

	_backBuffer.vLine(r.right - 1, r.top, r.bottom - 1, BUTTON_BOTTOM);
	_backBuffer.vLine(r.right - 2, r.top + 1, r.bottom - 2, BUTTON_BOTTOM);
	_backBuffer.hLine(r.left, r.bottom - 1, r.right - 1, BUTTON_BOTTOM);
	_backBuffer.hLine(r.left + 1, r.bottom - 2, r.right - 1, BUTTON_BOTTOM);
}

void ScalpelScreen::makeField(const Common::Rect &r) {
	_backBuffer.fillRect(r, BUTTON_MIDDLE);
	_backBuffer.hLine(r.left, r.top, r.right - 1, BUTTON_BOTTOM);
	_backBuffer.hLine(r.left + 1, r.bottom - 1, r.right - 1, BUTTON_TOP);
	_backBuffer.vLine(r.left, r.top + 1, r.bottom - 1, BUTTON_BOTTOM);
	_backBuffer.vLine(r.right - 1, r.top + 1, r.bottom - 2, BUTTON_TOP);
}

} // End of namespace Scalpel

} // End of namespace Sherlock
