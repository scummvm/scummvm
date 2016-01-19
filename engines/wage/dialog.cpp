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

#include "wage/wage.h"
#include "wage/design.h"
#include "wage/gui.h"
#include "wage/dialog.h"

namespace Wage {

enum {
	kDialogWidth = 292,
	kDialogHeight = 114
};

Dialog::Dialog(Gui *gui, const char *text, DialogButtonArray *buttons) : _gui(gui), _text(text), _buttons(buttons) {
	assert(_gui->_engine);
	assert(_gui->_engine->_world);

	_font = getDialogFont();

	_tempSurface.create(kDialogWidth, kDialogHeight, Graphics::PixelFormat::createFormatCLUT8());

	_bbox.left = (_gui->_screen.w - kDialogWidth) / 2;
	_bbox.top = (_gui->_screen.h - kDialogHeight) / 2;
	_bbox.right = (_gui->_screen.w + kDialogWidth) / 2;
	_bbox.bottom = (_gui->_screen.h + kDialogHeight) / 2;

	_defaultButton = NULL;
	_pressedButton = NULL;

	_mouseOverPressedButton = false;
}

Dialog::~Dialog() {
}

const Graphics::Font *Dialog::getDialogFont() {
	return _gui->getFont("Chicago-12", Graphics::FontManager::kBigGUIFont);
}

void Dialog::paint() {
	Design::drawFilledRect(&_gui->_screen, _bbox, kColorWhite, _gui->_patterns, kPatternSolid);
	_font->drawString(&_gui->_screen, _text, _bbox.left + 24, _bbox.right + 32, _bbox.width(), kColorBlack);

	static int boxOutline[] = { 1, 0, 0, 1, 1 };
	drawOutline(_bbox, boxOutline, ARRAYSIZE(boxOutline));

	for (int i = 0; i < _buttons->size(); i++) {
		DialogButton *button = _buttons->operator[](i);
		static int buttonOutline[] = { 0, 0, 0, 0, 1 };

		if (button == _defaultButton) {
			buttonOutline[0] = buttonOutline[1] = 1;
		} else {
			buttonOutline[0] = buttonOutline[1] = 0;
		}

		int color = kColorBlack;

		if (_pressedButton == button && _mouseOverPressedButton) {
			Common::Rect bb(button->bounds.left + 5, button->bounds.top + 5,
				button->bounds.right - 5, button->bounds.bottom - 5);

			Design::drawFilledRect(&_gui->_screen, bb, kColorBlack, _gui->_patterns, kPatternSolid);

			color = kColorWhite;
		}
		int w = _font->getStringWidth(button->text);
		int x = button->bounds.left + (button->bounds.width() - w) / 2;
		int y = button->bounds.top + 19;

		_font->drawString(&_gui->_screen, button->text, _bbox.left + x, _bbox.right + y, _bbox.width(), color);

		drawOutline(button->bounds, buttonOutline, ARRAYSIZE(buttonOutline));
	}

	g_system->copyRectToScreen(_gui->_screen.getBasePtr(_bbox.left, _bbox.top), _gui->_screen.pitch,
			_bbox.left, _bbox.top, _bbox.width(), _bbox.height());
}

void Dialog::drawOutline(Common::Rect &bounds, int *spec, int speclen) {
	for (int i = 0; i < speclen; i++)
		if (spec[i] != 0)
			Design::drawRect(&_gui->_screen, bounds.left + i, bounds.top + i, bounds.right - 1 - 2*i, bounds.bottom - 1 - 2*i,
						1, kColorBlack, _gui->_patterns, kPatternSolid);
}

} // End of namespace Wage
