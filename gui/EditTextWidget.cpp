/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
#include "gui/EditTextWidget.h"
#include "gui/dialog.h"
#include "gui/newgui.h"


namespace GUI {

EditTextWidget::EditTextWidget(GuiObject *boss, int x, int y, int w, int h, const String &text)
	: StaticTextWidget(boss, x, y - 1, w, h + 2, text, kTextAlignLeft), _backupString(text) {
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE;
	_type = kEditTextWidget;

	_caretVisible = false;
	_caretTime = 0;

	_pos = _label.size();

	_labelOffset = (g_gui.getStringWidth(_label) - (_w - 6));
	if (_labelOffset < 0)
		_labelOffset = 0;
}

void EditTextWidget::handleTickle() {
	uint32 time = g_system->get_msecs();
	if (_caretTime < time) {
		_caretTime = time + kCaretBlinkTime;
		drawCaret(_caretVisible);
	}
}

void EditTextWidget::handleMouseDown(int x, int y, int button, int clickCount){
	// First remove caret
	if (_caretVisible)
		drawCaret(true);

	NewGui *gui = &g_gui;

	x += _labelOffset;

	int width = 0;
	int i;

	for (i = 0; i < _label.size(); ++i) {
		width += gui->getCharWidth(_label[i]);
		if (width >= x)
			break;
	}
	_pos = i;
	if (adjustOffset())
		draw();
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
		releaseFocus();
		dirty = true;
		break;
	case 27:	// escape
		_label = _backupString;
		_pos = _label.size() - 1;
		_labelOffset = (g_gui.getStringWidth(_label) - (_w-6));
		if (_labelOffset < 0)
			_labelOffset = 0;
		releaseFocus();
		dirty = true;
		break;
	case 8:		// backspace
		if (_pos > 0) {
			_pos--;
			_label.deleteChar(_pos);
		}
		dirty = true;
		break;
	case 127:	// delete
		_label.deleteChar(_pos);
		dirty = true;
		break;
	case 256 + 20:	// left arrow
		if (_pos > 0) {
			_pos--;
			dirty = adjustOffset();
		}
		break;
	case 256 + 19:	// right arrow
		if (_pos < _label.size()) {
			_pos++;
			dirty = adjustOffset();
		}
		break;
	case 256 + 22:	// home
		_pos = 0;
		dirty = adjustOffset();
		break;
	case 256 + 23:	// end
		_pos = _label.size();
		dirty = adjustOffset();
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
	// Draw a thin frame around us.
	g_gui.hLine(_x, _y, _x + _w - 1, g_gui._color);
	g_gui.hLine(_x, _y + _h - 1, _x +_w - 1, g_gui._shadowcolor);
	g_gui.vLine(_x, _y, _y + _h - 1, g_gui._color);
	g_gui.vLine(_x + _w - 1, _y, _y + _h - 1, g_gui._shadowcolor);

	// Draw the text
	adjustOffset();
	g_gui.drawString(_label, _x + 2, _y + 2, _w - 6, g_gui._textcolor, kTextAlignLeft, -_labelOffset, false);
}

int EditTextWidget::getCaretPos() const {
	int caretpos = 0;
	for (int i = 0; i < _pos; i++)
		caretpos += g_gui.getCharWidth(_label[i]);

	caretpos -= _labelOffset;

	return caretpos;
}

void EditTextWidget::drawCaret(bool erase) {
	// Only draw if item is visible
	if (!isVisible() || !_boss->isVisible())
		return;

	int16 color = erase ? g_gui._bgcolor : g_gui._textcolorhi;
	int x = getAbsX() + 2;
	int y = getAbsY() + 1;

	x += getCaretPos();

	g_gui.vLine(x, y, y + kLineHeight, color);
	g_gui.addDirtyRect(x, y, 2, kLineHeight);

	_caretVisible = !erase;
}

bool EditTextWidget::adjustOffset() {
	// check if the caret is still within the textbox; if it isn't,
	// adjust _labelOffset 

	int caretpos = getCaretPos();

	if (caretpos < 0) {
		// scroll left
		_labelOffset += caretpos;
		return true;
	} else if (caretpos >= _w - 6) {
		// scroll right
		_labelOffset -= (_w - 6 - caretpos);
		return true;
	} else if (_labelOffset > 0) {
		int width = g_gui.getStringWidth(_label);
		if (width - _labelOffset < (_w - 6)) {
			// scroll right
			_labelOffset = (width - (_w - 6));
			if (_labelOffset < 0)
				_labelOffset = 0;
		}
	}

	return false;
}

} // End of namespace GUI
