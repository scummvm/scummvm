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

#include "sherlock/tattoo/widget_password.h"
#include "sherlock/tattoo/tattoo.h"
#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/tattoo/tattoo_user_interface.h"

namespace Sherlock {

namespace Tattoo {

WidgetPassword::WidgetPassword(SherlockEngine *vm) : WidgetBase(vm) {
	_blinkFlag = false;
	_blinkCounter = 0;
	_index = 0;
	_cursorColor = 192;
	_insert = true;
}

void WidgetPassword::show() {
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	ImageFile &images = *ui._interfaceImages;

	// Set the up window to be centered on the screen
	_bounds = Common::Rect(_surface.widestChar() * 20 + 6, (_surface.fontHeight() + 7) * 2 + 3);
	_bounds.moveTo(SHERLOCK_SCREEN_WIDTH / 2 - _bounds.width() / 2, SHERLOCK_SCREEN_HEIGHT / 2 - _bounds.height() / 2);

	// Create the surface
	_surface.create(_bounds.width(), _bounds.height());
	_surface.clear(TRANSPARENCY);
	makeInfoArea();

	// Draw the header area
	_surface.writeString(FIXED(EnterPassword), Common::Point((_bounds.width() - _surface.stringWidth(FIXED(EnterPassword))) / 2, 5), INFO_TOP);
	_surface.hLine(3, _surface.fontHeight() + 7, _bounds.width() - 4, INFO_TOP);
	_surface.hLine(3, _surface.fontHeight() + 8, _bounds.width() - 4, INFO_MIDDLE);
	_surface.hLine(3, _surface.fontHeight() + 9, _bounds.width() - 4, INFO_BOTTOM);
	_surface.SHtransBlitFrom(images[4], Common::Point(0, _surface.fontHeight() + 7 - 1));
	_surface.SHtransBlitFrom(images[5], Common::Point(_bounds.width() - images[5]._width, _surface.fontHeight() + 7 - 1));

	// Set the password entry data
	_cursorPos = Common::Point(_surface.widestChar(), _surface.fontHeight() + 12);
	_password = "";
	_index = 0;
	_cursorColor = 192;
	_insert = true;

	// Show the dialog
	ui._menuMode = PASSWORD_MODE;
	summonWindow();
}

void WidgetPassword::handleEvents() {
	Events &events = *_vm->_events;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;
	Common::Point mousePos = events.mousePos();
	const Common::KeyCode &keycode = ui._keyState.keycode;
	char currentChar = (_index == (int)_password.size()) ? ' ' : _password[_index];
	int width = _surface.charWidth(currentChar);

	if (!keycode) {
		// Nothing entered, so keep blinking the cursor
		if (--_blinkCounter < 0) {
			_blinkCounter = 3;
			_blinkFlag = !_blinkFlag;

			byte color, textColor;
			if (_blinkFlag) {
				textColor = 236;
				color = _cursorColor;
			} else {
				textColor = COMMAND_HIGHLIGHTED;
				color = TRANSPARENCY;
			}

			// Draw the cursor and the character it's over
			_surface.fillRect(Common::Rect(_cursorPos.x, _cursorPos.y, _cursorPos.x + width, _cursorPos.y + _surface.fontHeight()), color);
			if (currentChar != ' ')
				_surface.writeString(Common::String::format("%c", _password[_index]), _cursorPos, textColor);
		}
	} else if (keycode == Common::KEYCODE_BACKSPACE && _index) {
		_cursorPos.x -= _surface.charWidth(_password[_index - 1]);

		if (_insert)
			_password.deleteChar(_index - 1);
		else
			_password.setChar(' ', _index - 1);

		// Redraw the text
		--_index;
		_surface.fillRect(Common::Rect(_cursorPos.x, _cursorPos.y, _bounds.width() - 9, _cursorPos.y +
			_surface.fontHeight() - 1), TRANSPARENCY);
		_surface.writeString(_password.c_str() + _index, _cursorPos, COMMAND_HIGHLIGHTED);
	} else if ((keycode == Common::KEYCODE_LEFT && _index > 0)
			|| (keycode == Common::KEYCODE_RIGHT && _index < (int)_password.size() && _cursorPos.x < (_bounds.width() - _surface.widestChar() - 3))
			|| (keycode == Common::KEYCODE_HOME && _index > 0)
			|| (keycode == Common::KEYCODE_END)) {
		// Restore character the cursor was previously over
		_surface.fillRect(Common::Rect(_cursorPos.x, _cursorPos.y, _cursorPos.x + width, _cursorPos.y + _surface.fontHeight()), TRANSPARENCY);
		if (currentChar != ' ')
			_surface.writeString(Common::String::format("%c", _password[_index]), _cursorPos, COMMAND_HIGHLIGHTED);

		switch (keycode) {
		case Common::KEYCODE_LEFT:
			_cursorPos.x -= _surface.charWidth(_password[_index - 1]);
			--_index;
			break;
		case Common::KEYCODE_RIGHT:
			_cursorPos.x += _surface.charWidth(_password[_index]);
			++_index;
			break;
		case Common::KEYCODE_HOME:
			_cursorPos.x = _surface.widestChar();
			_index = 0;
			break;
		case Common::KEYCODE_END:
			_cursorPos.x = _surface.stringWidth(_password) + _surface.widestChar();
			_index = _password.size();

			while (_index > 0 && _password[_index - 1] == ' ') {
				_cursorPos.x -= _surface.charWidth(_password[_index - 1]);
				--_index;
			}
			break;
		default:
			break;
		}
	} else if (keycode == Common::KEYCODE_INSERT) {
		_insert = !_insert;
		_cursorColor = _insert ? 192 : 200;
	} else if (keycode == Common::KEYCODE_DELETE) {
		if (_index < (int)_password.size())
			_password.deleteChar(_index);

		// Redraw the text
		_surface.fillRect(Common::Rect(_cursorPos.x, _cursorPos.y, _bounds.width() - 9, _cursorPos.y +
			_surface.fontHeight() - 1), TRANSPARENCY);
		_surface.writeString(_password.c_str() + _index, _cursorPos, COMMAND_HIGHLIGHTED);
	} else if (keycode == Common::KEYCODE_RETURN || keycode == Common::KEYCODE_ESCAPE) {
		close();
		return;
	} else if ((ui._keyState.ascii >= ' ') && (ui._keyState.ascii <= 'z')) {
		if (_cursorPos.x + _surface.charWidth(ui._keyState.ascii) < _bounds.width() - _surface.widestChar() - 3) {
			if (_insert)
				_password.insertChar(ui._keyState.ascii, _index);
			else
				_password.setChar(ui._keyState.ascii, _index);

			// Redraw the text
			_surface.fillRect(Common::Rect(_cursorPos.x, _cursorPos.y, _bounds.width() - 9, _cursorPos.y +
				_surface.fontHeight() - 1), TRANSPARENCY);
			_surface.writeString(_password.c_str() + _index, _cursorPos, COMMAND_HIGHLIGHTED);

			_cursorPos.x += _surface.charWidth(ui._keyState.ascii);
			++_index;
		}
	}

	// Also handle clicking outside the window to abort
	if (events._firstPress && !_bounds.contains(mousePos))
		_outsideMenu = true;

	if ((events._released || events._rightReleased) && _outsideMenu && !_bounds.contains(mousePos)) {
		close();
	}
}

void WidgetPassword::close() {
	Talk &talk = *_vm->_talk;

	banishWindow();
	if (talk._talkToAbort)
		return;

	// See if they entered the correct password
	Common::String correct1 = FIXED(CorrectPassword);
	Common::String correct2 = Common::String::format("%s?", FIXED(CorrectPassword));
	Common::String correct3 = Common::String::format("%s ?", FIXED(CorrectPassword));

	if (!_password.compareToIgnoreCase(correct1) || !_password.compareToIgnoreCase(correct2)
			|| !_password.compareToIgnoreCase(correct3))
		// They got it correct
		_vm->setFlags(149);

	talk.talkTo("LASC52P");
}

} // End of namespace Tattoo

} // End of namespace Sherlock
