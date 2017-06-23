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

#include "sherlock/tattoo/widget_quit.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_user_interface.h"

namespace Sherlock {

namespace Tattoo {

WidgetQuit::WidgetQuit(SherlockEngine *vm) : WidgetBase(vm) {
	_select = _oldSelect = -1;
}

void WidgetQuit::show() {
	Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	ImageFile &images = *ui._interfaceImages;
	Common::Point mousePos = events.mousePos();
	const char *YES = FIXED(Yes);
	const char *NO = FIXED(No);

	// Set up the display area
	_bounds = Common::Rect(_surface.stringWidth(FIXED(AreYouSureYou)) + _surface.widestChar() * 2,
		(_surface.fontHeight() + 7) * 4);
	_bounds.moveTo(mousePos.x - _bounds.width() / 2, mousePos.y - _bounds.height() / 2);

	// Create the surface
	_surface.create(_bounds.width(), _bounds.height());
	_surface.clear(TRANSPARENCY);
	makeInfoArea();

	// Draw the message text
	_surface.writeString(FIXED(AreYouSureYou), Common::Point((_surface.width() - _surface.stringWidth(FIXED(AreYouSureYou))) / 2, 5), INFO_TOP);
	_surface.writeString(FIXED(WishToQuit), Common::Point((_surface.width() - _surface.stringWidth(FIXED(WishToQuit))) / 2,
		_surface.fontHeight() + 9), INFO_TOP);

	// Draw the horizontal bars seperating the commands and the message
	int yp = (_surface.fontHeight() + 4) * 2 + 3;
	for (int idx = 0; idx < 2; ++idx) {
		_surface.SHtransBlitFrom(images[4], Common::Point(0, yp - 1));
		_surface.SHtransBlitFrom(images[5], Common::Point(_surface.width() - images[5]._width, yp - 1));
		_surface.hLine(3, yp, _surface.width() - 4, INFO_TOP);
		_surface.hLine(3, yp + 1, _surface.width() - 4, INFO_MIDDLE);
		_surface.hLine(3, yp + 2, _surface.width() - 4, INFO_BOTTOM);

		const char *btn = (idx == 0) ? YES : NO;
		_surface.writeString(btn, Common::Point((_bounds.width() - _surface.stringWidth(btn)) / 2, yp + 5), INFO_TOP);
		yp += _surface.fontHeight() + 7;
	}

	ui._menuMode = QUIT_MODE;
	summonWindow();
}

void WidgetQuit::handleEvents() {
	Events &events = *_vm->_events;
	Talk &talk = *_vm->_talk;
	Common::Point mousePos = events.mousePos();
	Common::Rect yesRect(_bounds.left, _bounds.top + (_surface.fontHeight() + 4) * 2 + 3, _bounds.right,
		_bounds.top + (_surface.fontHeight() + 4) * 2 + 3 + _surface.fontHeight() + 7);
	Common::Rect noRect(_bounds.left, _bounds.top + (_surface.fontHeight() + 4) * 2 + _surface.fontHeight() + 10,
		_bounds.right, _bounds.top + (_surface.fontHeight() + 4) * 2 + 10 + _surface.fontHeight() * 2 + 7);

	if (talk._talkToAbort)
		return;

	// Determine the highlighted item
	_select = -1;
	if (yesRect.contains(mousePos))
		_select = 1;
	else if (noRect.contains(mousePos))
		_select = 0;

	if (events.kbHit()) {
		Common::KeyState keyState = events.getKey();

		switch (keyState.keycode) {
		case Common::KEYCODE_TAB:
			// If the mouse is not over any of the options, move the mouse so that it points to the first option
			if (_select == -1)
				events.warpMouse(Common::Point(_bounds.right - 10, _bounds.top + (_surface.fontHeight() + 4) * 2
					+ 3 + _surface.fontHeight() + 1));
			else if (_select == 1)
				events.warpMouse(Common::Point(mousePos.x, _bounds.top + (_surface.fontHeight() + 4) * 2
					+ 3 + _surface.fontHeight() * 2 + 11));
			else
				events.warpMouse(Common::Point(mousePos.x, _bounds.top + (_surface.fontHeight() + 4) * 2
					+ 3 + _surface.fontHeight() + 1));
			break;

		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_n:
			close();
			return;

		case Common::KEYCODE_y:
			close();
			_vm->quitGame();
			break;

		default:
			break;
		}
	}

	// Check for change of the highlighted item
	if (_select != _oldSelect) {
		byte color = (_select == 1) ? COMMAND_HIGHLIGHTED : INFO_TOP;
		int yp = (_surface.fontHeight() + 4) * 2 + 8;
		_surface.writeString(FIXED(Yes), Common::Point((_surface.width() - _surface.stringWidth(FIXED(Yes))) / 2, yp), color);

		color = (_select == 0) ? COMMAND_HIGHLIGHTED : INFO_TOP;
		yp += (_surface.fontHeight() + 7);
		_surface.writeString(FIXED(No), Common::Point((_surface.width() - _surface.stringWidth(FIXED(No))) / 2, yp), color);
	}
	_oldSelect = _select;

	// Flag is they started pressing outside of the menu
	if (events._firstPress && !_bounds.contains(mousePos))
		_outsideMenu = true;

	if (events._released || events._rightReleased) {
		events.clearEvents();
		close();
		if (_select == 1)
			// Yes selected
			_vm->quitGame();
	}
}

} // End of namespace Tattoo

} // End of namespace Sherlock
