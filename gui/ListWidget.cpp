/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
#include "gui/ListWidget.h"
#include "gui/ScrollBarWidget.h"
#include "gui/dialog.h"
#include "gui/newgui.h"


namespace GUI {

ListWidget::ListWidget(GuiObject *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w - kScrollBarWidth, h), CommandSender(boss) {
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE;
	_type = kListWidget;
	_numberingMode = kListNumberingOne;
	_entriesPerPage = (_h - 2) / kLineHeight;
	_currentPos = 0;
	_selectedItem = -1;
	_scrollBar = new ScrollBarWidget(boss, _x + _w, _y, kScrollBarWidth, _h);
	_scrollBar->setTarget(this);
	_currentKeyDown = 0;

	_caretVisible = false;
	_caretTime = 0;

	// FIXME: This flag should come from widget definition
	_editable = true;

	_editMode = false;
}

ListWidget::~ListWidget() {
}

void ListWidget::setList(const StringList &list) {
	if (_editMode && _caretVisible)
		drawCaret(true);
	int size = list.size();
	_list = list;
	if (_currentPos >= size)
		_currentPos = size - 1;
	if (_currentPos < 0)
		_currentPos = 0;
	_selectedItem = -1;
	_editMode = false;
	scrollBarRecalc();
}

void ListWidget::scrollTo(int item) {
	int size = _list.size();
	if (item >= size)
		item = size - 1;
	if (item < 0)
		item = 0;

	if (_currentPos != item) {
		_currentPos = item;
		scrollBarRecalc();
	}
}

void ListWidget::scrollBarRecalc() {
	_scrollBar->_numEntries = _list.size();
	_scrollBar->_entriesPerPage = _entriesPerPage;
	_scrollBar->_currentPos = _currentPos;
	_scrollBar->recalc();
}

void ListWidget::handleTickle() {
	uint32 time = g_system->get_msecs();
	if (_editMode && _caretTime < time) {
		_caretTime = time + kCaretBlinkTime;
		if (_caretVisible) {
			drawCaret(true);
		} else {
			drawCaret(false);
		}
	}
}

void ListWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (isEnabled()) {
		int oldSelectedItem = _selectedItem;
		_selectedItem = (y - 1) / kLineHeight + _currentPos;
		if (_selectedItem > _list.size() - 1)
			_selectedItem = -1;

		if (oldSelectedItem != _selectedItem) {
			if (_editMode) {
				// undo any changes made
				_list[oldSelectedItem] = _backupString;
				_editMode = false;
				drawCaret(true);
			}
			sendCommand(kListSelectionChangedCmd, _selectedItem);
		}
		draw();
	}
}

void ListWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	// If this was a double click and the mouse is still over the selected item,
	// send the double click command
	if (clickCount == 2 && (_selectedItem == (y - 1) / kLineHeight + _currentPos)) {
		sendCommand(kListItemDoubleClickedCmd, _selectedItem);
	}
}

void ListWidget::handleMouseWheel(int x, int y, int direction) {
	_scrollBar->handleMouseWheel(x, y, direction);
}

bool ListWidget::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	bool handled = true;
	bool dirty = false;
	int oldSelectedItem = _selectedItem;

	if (_editMode) {

		if (_caretVisible)
			drawCaret(true);

		switch (keycode) {
			case '\n':	// enter/return
			case '\r':
				// enter, confirm edit and exit editmode
				_editMode = false;
				dirty = true;
				sendCommand(kListItemActivatedCmd, _selectedItem);
				break;
			case 27:	// escape
				// ESC, abort edit and exit editmode
				_editMode = false;
				dirty = true;
				_list[_selectedItem] = _backupString;
				break;
			case 8:		// backspace
				_list[_selectedItem].deleteLastChar();
				dirty = true;
				break;
			default:
				if (isprint((char)ascii)) {
					_list[_selectedItem] += (char)ascii;
					dirty = true;
				} else {
					handled = false;
				}
		}

	} else {
		// not editmode

		switch (keycode) {
		case '\n':	// enter
		case '\r':
			if (_selectedItem >= 0) {
				// override continuous enter keydown
				if (_editable && (_currentKeyDown != '\n' && _currentKeyDown != '\r')) {
					dirty = true;
					_editMode = true;
					_backupString = _list[_selectedItem];
				}
			}
			break;
		case 256+17:	// up arrow
			if (_selectedItem > 0)
				_selectedItem--;
			break;
		case 256+18:	// down arrow
			if (_selectedItem < _list.size() - 1)
				_selectedItem++;
			break;
		case 256+24:	// pageup
			_selectedItem -= _entriesPerPage - 1;
			if (_selectedItem < 0)
				_selectedItem = 0;
			break;
		case 256+25:	// pagedown
			_selectedItem += _entriesPerPage - 1;
			if (_selectedItem >= _list.size() )
				_selectedItem = _list.size() - 1;
			break;
		case 256+22:	// home
			_selectedItem = 0;
			break;
		case 256+23:	// end
			_selectedItem = _list.size() - 1;
			break;
		default:
			handled = false;
		}

		scrollToCurrent();
	}

	if (dirty || _selectedItem != oldSelectedItem)
		draw();

	if (_selectedItem != oldSelectedItem) {
		sendCommand(kListSelectionChangedCmd, _selectedItem);
		// also draw scrollbar
		_scrollBar->draw();
	}

#if !defined(_WIN32_WCE) && !defined(__PALM_OS__)
	// not done on WinCE / PalmOS because keyboard is emulated and
	// keyup is not generated

	_currentKeyDown = keycode;
#endif

	return handled;
}

bool ListWidget::handleKeyUp(uint16 ascii, int keycode, int modifiers) {
	if (keycode == _currentKeyDown)
		_currentKeyDown = 0;
	return true;
}

void ListWidget::lostFocusWidget() {
	_editMode = false;
	drawCaret(true);
	draw();
}

void ListWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSetPositionCmd:
		if (_currentPos != (int)data) {
			_currentPos = data;
			draw();
		}
		break;
	}
}

void ListWidget::drawWidget(bool hilite) {
	NewGui *gui = &g_gui;
	int i, pos, len = _list.size();
	Common::String buffer;

	// Draw a thin frame around the list.
	gui->hLine(_x, _y, _x + _w - 1, gui->_color);
	gui->hLine(_x, _y + _h - 1, _x + _w - 1, gui->_shadowcolor);
	gui->vLine(_x, _y, _y + _h - 1, gui->_color);

	// Draw the list items
	for (i = 0, pos = _currentPos; i < _entriesPerPage && pos < len; i++, pos++) {
		if (_numberingMode == kListNumberingZero || _numberingMode == kListNumberingOne) {
			char temp[10];
			sprintf(temp, "%2d. ", (pos + _numberingMode));
			buffer = temp;
			buffer += _list[pos];
		} else
			buffer = _list[pos];

		if (_selectedItem == pos) {
			if (_hasFocus)
				gui->fillRect(_x + 1, _y + 1 + kLineHeight * i, _w - 1, kLineHeight, gui->_textcolorhi);
			else
				gui->frameRect(_x + 1, _y + 1 + kLineHeight * i, _w - 1, kLineHeight, gui->_textcolorhi);
		}
		gui->drawString(buffer, _x + 2, _y + 3 + kLineHeight * i, _w - 4,
							(_selectedItem == pos && _hasFocus) ? gui->_bgcolor : gui->_textcolor);
	}
}

void ListWidget::drawCaret(bool erase) {
	// Only draw if item is visible
	if (_selectedItem < _currentPos || _selectedItem >= _currentPos + _entriesPerPage)
		return;
	if (!isVisible() || !_boss->isVisible())
		return;

	NewGui *gui = &g_gui;

	// The item is selected, thus _bgcolor is used to draw the caret and _textcolorhi to erase it
	int16 color = erase ? gui->_textcolorhi : gui->_bgcolor;
	int x = getAbsX() + 3;
	int y = getAbsY() + 1;
	Common::String	buffer;

	y += (_selectedItem - _currentPos) * kLineHeight;

	if (_numberingMode == kListNumberingZero || _numberingMode == kListNumberingOne) {
		char temp[10];
		sprintf(temp, "%2d. ", (_selectedItem + _numberingMode));
		buffer = temp;
		buffer += _list[_selectedItem];
	} else
		buffer = _list[_selectedItem];

	x += gui->getStringWidth(buffer);

	gui->vLine(x, y, y+kLineHeight, color);
	gui->addDirtyRect(x, y, 2, kLineHeight);
	
	_caretVisible = !erase;
}

void ListWidget::scrollToCurrent() {
	// Only do something if the current item is not in our view port
	if (_selectedItem < _currentPos) {
		// it's above our view
		_currentPos = _selectedItem;
	} else if (_selectedItem >= _currentPos + _entriesPerPage ) {
		// it's below our view
		_currentPos = _selectedItem - _entriesPerPage + 1;
	}

	if (_currentPos < 0)
		_currentPos = 0;

	_scrollBar->_currentPos = _currentPos;
	_scrollBar->recalc();
}

void ListWidget::startEditMode() {
	if (_editable && !_editMode && _selectedItem >= 0) {
		_editMode = true;
		_backupString = _list[_selectedItem];
		draw();
	}
}

void ListWidget::abortEditMode() {
	if (_editMode) {
		_editMode = false;
		_list[_selectedItem] = _backupString;
		drawCaret(true);
		draw();
	}
}

} // End of namespace GUI
