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
 * MIT License:
 *
 * Copyright (c) 2009 Alexei Svitkine, Eugene Sandulenko
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "common/system.h"
#include "common/events.h"

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"

#include "wage/wage.h"
#include "wage/design.h"
#include "wage/gui.h"
#include "wage/dialog.h"

namespace Wage {

enum {
	kDialogHeight = 113
};

Dialog::Dialog(Gui *gui, int width, const char *text, DialogButtonArray *buttons, uint defaultButton) :
		_gui(gui), _text(text), _buttons(buttons), _defaultButton(defaultButton) {
	assert(_gui->_engine);
	assert(_gui->_engine->_world);

	_font = getDialogFont();

	_tempSurface.create(width + 1, kDialogHeight + 1, Graphics::PixelFormat::createFormatCLUT8());

	_bbox.left = (_gui->_screen.w - width) / 2;
	_bbox.top = (_gui->_screen.h - kDialogHeight) / 2;
	_bbox.right = (_gui->_screen.w + width) / 2;
	_bbox.bottom = (_gui->_screen.h + kDialogHeight) / 2;

	_pressedButton = -1;

	_mouseOverPressedButton = false;

	// Adjust button positions
	for (uint i = 0; i < _buttons->size(); i++)
		_buttons->operator[](i)->bounds.translate(_bbox.left, _bbox.top);

	_needsRedraw = true;
}

Dialog::~Dialog() {
	for (uint i = 0; i < _buttons->size(); i++)
		delete _buttons->operator[](i);
}

const Graphics::Font *Dialog::getDialogFont() {
	return _gui->_wm._fontMan->getFont(Graphics::MacFont(Graphics::kMacFontChicago, 12));
}

void Dialog::paint() {
	Design::drawFilledRect(&_gui->_screen, _bbox, kColorWhite, _gui->_wm.getPatterns(), kPatternSolid);
	_font->drawString(&_gui->_screen, _text, _bbox.left + 24, _bbox.top + 16, _bbox.width(), kColorBlack);

	static int boxOutline[] = { 1, 0, 0, 1, 1 };
	drawOutline(_bbox, boxOutline, ARRAYSIZE(boxOutline));

	for (uint i = 0; i < _buttons->size(); i++) {
		DialogButton *button = _buttons->operator[](i);
		static int buttonOutline[] = { 0, 0, 0, 0, 1 };

		if (i == _defaultButton) {
			buttonOutline[0] = buttonOutline[1] = 1;
		} else {
			buttonOutline[0] = buttonOutline[1] = 0;
		}

		int color = kColorBlack;

		if ((int)i == _pressedButton && _mouseOverPressedButton) {
			Common::Rect bb(button->bounds.left + 5, button->bounds.top + 5,
				button->bounds.right - 5, button->bounds.bottom - 5);

			Design::drawFilledRect(&_gui->_screen, bb, kColorBlack, _gui->_wm.getPatterns(), kPatternSolid);

			color = kColorWhite;
		}
		int w = _font->getStringWidth(button->text);
		int x = button->bounds.left + (button->bounds.width() - w) / 2;
		int y = button->bounds.top + 6;

		_font->drawString(&_gui->_screen, button->text, x, y, _bbox.width(), color);

		drawOutline(button->bounds, buttonOutline, ARRAYSIZE(buttonOutline));
	}

	g_system->copyRectToScreen(_gui->_screen.getBasePtr(_bbox.left, _bbox.top), _gui->_screen.pitch,
			_bbox.left, _bbox.top, _bbox.width() + 1, _bbox.height() + 1);

	_needsRedraw = false;
}

void Dialog::drawOutline(Common::Rect &bounds, int *spec, int speclen) {
	for (int i = 0; i < speclen; i++)
		if (spec[i] != 0)
			Design::drawRect(&_gui->_screen, bounds.left + i, bounds.top + i, bounds.right - i, bounds.bottom - i,
						1, kColorBlack, _gui->_wm.getPatterns(), kPatternSolid);
}

int Dialog::run() {
	bool shouldQuit = false;
	Common::Rect r(_bbox);

	_tempSurface.copyRectToSurface(_gui->_screen.getBasePtr(_bbox.left, _bbox.top), _gui->_screen.pitch, 0, 0, _bbox.width() + 1, _bbox.height() + 1);
	_gui->_wm.pushArrowCursor();

	while (!shouldQuit) {
		Common::Event event;

		while (_gui->_engine->_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				_gui->_engine->_shouldQuit = true;
				shouldQuit = true;
				break;
			case Common::EVENT_MOUSEMOVE:
				mouseMove(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_LBUTTONDOWN:
				mouseClick(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_LBUTTONUP:
				shouldQuit = mouseRaise(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					_pressedButton = -1;
					shouldQuit = true;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		if (_needsRedraw)
			paint();

		g_system->updateScreen();
		g_system->delayMillis(50);
	}

	_gui->_screen.copyRectToSurface(_tempSurface.getBasePtr(0, 0), _tempSurface.pitch, _bbox.left, _bbox.top, _bbox.width() + 1, _bbox.height() + 1);
	g_system->copyRectToScreen(_gui->_screen.getBasePtr(r.left, r.top), _gui->_screen.pitch, r.left, r.top, r.width() + 1, r.height() + 1);

	_gui->_wm.popCursor();

	return _pressedButton;
}

int Dialog::matchButton(int x, int y) {
	for (uint i = 0; i < _buttons->size(); i++)
		if (_buttons->operator[](i)->bounds.contains(x, y))
			return i;

	return -1;
}

void Dialog::mouseMove(int x, int y) {
	if (_pressedButton != -1) {
		int match = matchButton(x, y);

		if (_mouseOverPressedButton && match != _pressedButton) {
			_mouseOverPressedButton = false;
			_needsRedraw = true;
		} else if (!_mouseOverPressedButton && match == _pressedButton) {
			_mouseOverPressedButton = true;
			_needsRedraw = true;
		}
	}
}

void Dialog::mouseClick(int x, int y) {
	int match = matchButton(x, y);

	if (match != -1) {
		_pressedButton = match;
		_mouseOverPressedButton = true;

		_needsRedraw = true;
	}
}

int Dialog::mouseRaise(int x, int y) {
	bool res = false;

	if (_pressedButton != -1) {
		if (matchButton(x, y) == _pressedButton)
			res = true;
	}

	return res;
}

} // End of namespace Wage
