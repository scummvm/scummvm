/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
	: EditableWidget(boss, x, y - 1, w, h + 2) {
	_editString = text;
	_backupString = text;
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE;
	_type = kEditTextWidget;

	_caretPos = _editString.size();

	_editScrollOffset = (g_gui.getStringWidth(_editString) - (getEditRect().width()));
	if (_editScrollOffset < 0)
		_editScrollOffset = 0;
}

void EditTextWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	// First remove caret
	if (_caretVisible)
		drawCaret(true);

	NewGui *gui = &g_gui;

	x += _editScrollOffset;

	int width = 0;
	uint i;

	for (i = 0; i < _editString.size(); ++i) {
		width += gui->getCharWidth(_editString[i]);
		if (width >= x)
			break;
	}
	if (setCaretPos(i))
		draw();
}

bool EditTextWidget::tryInsertChar(char c, int pos) {
	if (isprint(c)) {
		_editString.insertChar(c, pos);
		return true;
	}
	return false;
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
		// confirm edit and exit editmode
		endEditMode();
		dirty = true;
		break;
	case 27:	// escape
		abortEditMode();
		dirty = true;
		break;
	case 8:		// backspace
		if (_caretPos > 0) {
			_caretPos--;
			_editString.deleteChar(_caretPos);
		}
		dirty = true;
		break;
	case 127:	// delete
		_editString.deleteChar(_caretPos);
		dirty = true;
		break;
	case 256 + 20:	// left arrow
		if (_caretPos > 0) {
			dirty = setCaretPos(_caretPos - 1);
		}
		break;
	case 256 + 19:	// right arrow
		if (_caretPos < (int)_editString.size()) {
			dirty = setCaretPos(_caretPos + 1);
		}
		break;
	case 256 + 22:	// home
		dirty = setCaretPos(0);
		break;
	case 256 + 23:	// end
		dirty = setCaretPos(_editString.size());
		break;
	default:
		if (tryInsertChar((char)ascii, _caretPos)) {
			_caretPos++;
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
	g_gui.drawString(_editString, _x + 2, _y + 2, getEditRect().width(), g_gui._textcolor, kTextAlignLeft, -_editScrollOffset, false);
}

Common::Rect EditTextWidget::getEditRect() const {
	Common::Rect r(2, 1, _w - 2, _h);
	
	return r;
}

int EditTextWidget::getCaretOffset() const {
	int caretpos = 0;
	for (int i = 0; i < _caretPos; i++)
		caretpos += g_gui.getCharWidth(_editString[i]);

	caretpos -= _editScrollOffset;

	return caretpos;
}

void EditTextWidget::receivedFocusWidget() {
}

void EditTextWidget::lostFocusWidget() {
	// If we loose focus, 'commit' the user changes
	_backupString = _editString;
	drawCaret(true);
}

void EditTextWidget::startEditMode() {
}

void EditTextWidget::endEditMode() {
	releaseFocus();
}

void EditTextWidget::abortEditMode() {
	_editString = _backupString;
	_caretPos = _editString.size();
	_editScrollOffset = (g_gui.getStringWidth(_editString) - (getEditRect().width()));
	if (_editScrollOffset < 0)
		_editScrollOffset = 0;
	releaseFocus();
}

bool EditTextWidget::setCaretPos(int newPos) {
	assert(newPos >= 0 && newPos <= (int)_editString.size());
	_caretPos = newPos;
	return adjustOffset();
}

bool EditTextWidget::adjustOffset() {
	// check if the caret is still within the textbox; if it isn't,
	// adjust _editScrollOffset 

	int caretpos = getCaretOffset();
	const int editWidth = getEditRect().width();

	if (caretpos < 0) {
		// scroll left
		_editScrollOffset += caretpos;
		return true;
	} else if (caretpos >= editWidth) {
		// scroll right
		_editScrollOffset -= (editWidth - caretpos);
		return true;
	} else if (_editScrollOffset > 0) {
		const int strWidth = g_gui.getStringWidth(_editString);
		if (strWidth - _editScrollOffset < editWidth) {
			// scroll right
			_editScrollOffset = (strWidth - editWidth);
			if (_editScrollOffset < 0)
				_editScrollOffset = 0;
		}
	}

	return false;
}

} // End of namespace GUI
