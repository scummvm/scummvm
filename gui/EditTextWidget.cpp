/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "EditTextWidget.h"
#include "dialog.h"
#include "newgui.h"

EditTextWidget::EditTextWidget(Dialog *boss, int x, int y, int w, int h, const String &text)
	: StaticTextWidget(boss, x, y-1, w, h+2, text, kTextAlignLeft), _backupString(text) {
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE;
	_type = kEditTextWidget;

	_caretVisible = false;
	_caretTime = 0;

	_pos = _label.size();
}

void EditTextWidget::handleTickle() {
	uint32 time = _boss->getGui()->get_time();
	if (_caretTime < time) {
		_caretTime = time + kCaretBlinkTime;
		if (_caretVisible) {
			drawCaret(true);
		} else {
			drawCaret(false);
		}
	}
}

void EditTextWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	// TODO - once we support "real editing" (i.e. caret can be at any spot),
	// a mouse click should place the caret.
}

bool EditTextWidget::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	bool handled = true;
	bool dirty = false;

	// First remove caret
	if (_caretVisible)
		drawCaret(true);

	switch (keycode) {
		case '\n':	// enter/return
		case '\r':
			_boss->releaseFocus();
			dirty = true;
			break;
		case 27:	// escape
			_label = _backupString;
			_boss->releaseFocus();
			dirty = true;
			break;
		case 8:		// backspace
			_label.deleteLastChar();
			if (_pos > 0)
				_pos--;
			dirty = true;
			break;
		case 127:	// delete
			_label.deleteChar(_pos);
			dirty = true;
			break;
		case 256 + 20:	// left arrow
			if (_pos > 0)
				_pos--;
			break;
		case 256 + 19:	// right arrow
			if (_pos < _label.size())
				_pos++;
			break;
			break;
		case 256 + 22:	// home
			_pos = 0;
			break;
		case 256 + 23:	// end
			_pos = _label.size();
			break;
		default:
			if (isprint((char)ascii)) {
				_label.insertChar((char)ascii, _pos++);
				//_label += (char)ascii;
				dirty = true;
			} else {
				handled = false;
			}
	}

	if (dirty)
		draw();

	return handled;
}

void EditTextWidget::drawWidget(bool hilite) {
	NewGui *gui = _boss->getGui();

	// Draw a thin frame around us.
	gui->hline(_x, _y, _x + _w - 1, gui->_color);
	gui->hline(_x, _y + _h - 1, _x +_w - 1, gui->_shadowcolor);
	gui->vline(_x, _y, _y + _h - 1, gui->_color);
	gui->vline(_x + _w - 1, _y, _y + _h - 1, gui->_shadowcolor);

	// Draw the text
	_align = (gui->getStringWidth(_label) > _w - 6) ? kTextAlignRight : kTextAlignLeft;
	gui->drawString(_label, _x + 2, _y + 3, _w - 6, gui->_textcolor, _align);
}

void EditTextWidget::drawCaret(bool erase) {
	// Only draw if item is visible
	if (!isVisible() || !_boss->isVisible())
		return;

	NewGui *gui = _boss->getGui();

	int16 color = erase ? gui->_bgcolor : gui->_textcolorhi;
	int x = _x + _boss->getX() + 2;
	int y = _y + _boss->getY() + 1;

	int width = 0;
	for (int i = 0; i < _pos; i++)
		width += gui->getCharWidth(_label[i]);

	if (width > _w - 6)
		width = _w - 6;
	x += width;

	gui->vline(x, y, y + kLineHeight, color);
	gui->addDirtyRect(x, y, 2, kLineHeight);

	_caretVisible = !erase;
}
