/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
#include "gui/editable.h"
#include "gui/newgui.h"


namespace GUI {

EditableWidget::EditableWidget(GuiObject *boss, int x, int y, int w, int h, WidgetSize ws)
 : Widget(boss, x, y, w, h) {
	_caretVisible = false;
	_caretTime = 0;
	_caretPos = 0;	// FIXME

	_caretInverse = false;

	_editScrollOffset = 0;

	switch (ws) {
	case kNormalWidgetSize:
		_font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		break;
	case kBigWidgetSize:
		_font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		break;
	default:
		_font = &g_gui.getFont();
		break;
	}
}

EditableWidget::~EditableWidget() {
}

void EditableWidget::setEditString(const String &str) {
	// TODO: We probably should filter the input string here,
	// e.g. using tryInsertChar.
	_editString = str;
	_caretPos = _editString.size();

	_editScrollOffset = (_font->getStringWidth(_editString) - (getEditRect().width()));
	if (_editScrollOffset < 0)
		_editScrollOffset = 0;
}

bool EditableWidget::tryInsertChar(char c, int pos) {
	if (isprint(c)) {
		_editString.insertChar(c, pos);
		return true;
	}
	return false;
}

void EditableWidget::handleTickle() {
	uint32 time = getMillis();
	if (_caretTime < time) {
		_caretTime = time + kCaretBlinkTime;
		drawCaret(_caretVisible);
	}
}

bool EditableWidget::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
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
			dirty = true;
		}
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

int EditableWidget::getCaretOffset() const {
	int caretpos = 0;
	for (int i = 0; i < _caretPos; i++)
		caretpos += _font->getCharWidth(_editString[i]);

	caretpos -= _editScrollOffset;

	return caretpos;
}

void EditableWidget::drawCaret(bool erase) {
	// Only draw if item is visible
	if (!isVisible() || !_boss->isVisible())
		return;

	Common::Rect editRect = getEditRect();

	int16 color = (erase ^ _caretInverse) ? g_gui._bgcolor : g_gui._textcolorhi;
	int x = editRect.left;
	int y = editRect.top + 1;

	x += getCaretOffset();

	if (y < 0 || y + editRect.height() - 2 >= _h)
		return;
	
	x += getAbsX();
	y += getAbsY();

	g_gui.vLine(x, y, y + editRect.height() - 2, color);
	g_gui.addDirtyRect(x, y, 2, editRect.height() - 2);

	_caretVisible = !erase;
}

bool EditableWidget::setCaretPos(int newPos) {
	assert(newPos >= 0 && newPos <= (int)_editString.size());
	_caretPos = newPos;
	return adjustOffset();
}

bool EditableWidget::adjustOffset() {
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
		const int strWidth = _font->getStringWidth(_editString);
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
