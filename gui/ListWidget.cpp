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
#include "ListWidget.h"
#include "ScrollBarWidget.h"
#include "dialog.h"
#include "newgui.h"


ListWidget::ListWidget(Dialog *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w - kScrollBarWidth, h), CommandSender(boss)
{
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE;
	_type = kListWidget;
	_numberingMode = kListNumberingOne;
	_entriesPerPage = (_h - 2) / kLineHeight;
	_currentPos = 0;
	_selectedItem = -1;
	_scrollBar = new ScrollBarWidget(boss, _x + _w, _y, kScrollBarWidth, _h);
	_scrollBar->setTarget(this);
	_currentKeyDown = 0;
	
	// FIXME: This flag should come from widget definition
	_editable = true;

	_editMode = false;
}

ListWidget::~ListWidget()
{
}

void ListWidget::scrollBarRecalc()
{
	_scrollBar->_numEntries = _list.size();
	_scrollBar->_entriesPerPage = _entriesPerPage;
	_scrollBar->_currentPos = _currentPos;
	_scrollBar->recalc();
}

void ListWidget::handleMouseDown(int x, int y, int button, int clickCount)
{
	int oldSelectedItem = _selectedItem;

	if (_flags & WIDGET_ENABLED) {
		_selectedItem = (y - 1) / kLineHeight + _currentPos;

		if (_editMode && oldSelectedItem != _selectedItem) {
			// undo any changes made
			_list[oldSelectedItem] = _backupString;
			_editMode = false;
		}
		draw();
	}
}

void ListWidget::handleMouseUp(int x, int y, int button, int clickCount)
{
	// If this was a double click and the mouse is still over the selected item,
	// send the double click command
	if (clickCount > 1 && (_selectedItem == (y - 1) / kLineHeight + _currentPos)) {
		sendCommand(kListItemDoubleClickedCmd, _selectedItem);
	}
}

bool ListWidget::handleKeyDown(char key, int modifiers)
{
	bool handled = true;
	bool dirty = false;
	int oldSelectedItem = _selectedItem;

	if (_editMode) {

		// get rid of caret
		_list[_selectedItem].deleteLastChar();

		if (key == '\n' || key == '\r') {
			// enter, confirm edit and exit editmode
			_editMode = false;
			dirty = true;
			sendCommand(kListItemChangedCmd, _selectedItem);
		} else if (key == 27) {
			// ESC, abort edit and exit editmode
			_editMode = false;
			dirty = true;
			_list[_selectedItem] = _backupString;
		} else if (key == 8) {	// backspace
			_list[_selectedItem].deleteLastChar();
			dirty = true;
		} else if (// filter keystrokes
					( ( key >= 'a' && key <= 'z' )
					|| ( key >= 'A' && key <= 'Z' )
					|| ( key >= '0' && key <= '9' )
					|| ( key == ' ')
					) )
			{

			_list[_selectedItem] += key;
			dirty = true;
		} else
			handled = false;

	} else {
		// not editmode

		switch (key) {
		case '\n':	// enter
		case '\r':
			if (_selectedItem >= 0) {
				if ((_currentKeyDown != '\n' && _currentKeyDown != '\r')) {		// override continuous enter keydown
					_editMode = true;
					dirty = true;
					_backupString = _list[_selectedItem];
				}
			}
			break;
		case 17:	// up arrow
			if (_selectedItem > 0)
				_selectedItem--;
			break;
		case 18:	// down arrow
			if (_selectedItem < _list.size() - 1)
				_selectedItem++;
			break;
		case 24:	// pageup
			_selectedItem -= _entriesPerPage - 1;
			if (_selectedItem < 0)
				_selectedItem = 0;
			break;
		case 25:	// pagedown
			_selectedItem += _entriesPerPage - 1;
			if (_selectedItem >= _list.size() )
				_selectedItem = _list.size() - 1;
			break;
		case 22:	// home
			_selectedItem = 0;
			break;
		case 23:	// end
			_selectedItem = _list.size() - 1;
			break;
		default:
			handled = false;
		}

		scrollToCurrent();
	}

	if (_editMode)
		// re-add caret
		_list[_selectedItem] += '_';

	if (dirty || _selectedItem != oldSelectedItem)
		draw();

	if (_selectedItem != oldSelectedItem) {
		// also draw scrollbar
		_scrollBar->draw();
	}

	_currentKeyDown = key;
	
	return handled;
}

bool ListWidget::handleKeyUp(char key, int modifiers)
{
	if (key == _currentKeyDown)
		_currentKeyDown = 0;
	return true;
}

void ListWidget::lostFocusWidget()
{
	if (_editMode) {
		// loose caret
		_list[_selectedItem].deleteLastChar();
		_editMode = false;
	}

	draw();
}

void ListWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data)
{
	switch (cmd) {
	case kSetPositionCmd:
		if (_currentPos != (int)data) {
			_currentPos = data;
			draw();
		}
		break;
	}
}

void ListWidget::drawWidget(bool hilite)
{
	NewGui			*gui = _boss->getGui();
	int				i, pos, len = _list.size();
	ScummVM::String	buffer;

	// Draw a thin frame around the list.
	gui->hline(_x, _y, _x+_w-1, gui->_color);
	gui->hline(_x, _y+_h-1, _x+_w-1, gui->_shadowcolor);
	gui->vline(_x, _y, _y+_h-1, gui->_color);

	// Draw the list items
	for (i = 0, pos = _currentPos; i < _entriesPerPage && pos < len; i++, pos++) {
		if (_numberingMode == kListNumberingZero || _numberingMode == kListNumberingOne) {
			char temp[10];
			sprintf(temp, "%2d. ", (pos + _numberingMode));
			buffer = temp;
		} else
			buffer = "";

		buffer += _list[pos];
		
		if (_selectedItem == pos) {
			if (_hasFocus)
				gui->fillRect(_x+1, _y+1 + kLineHeight * i, _w - 1, kLineHeight, gui->_textcolorhi);
			else
				gui->frameRect(_x+1, _y+1 + kLineHeight * i, _w - 1, kLineHeight, gui->_textcolorhi);
		}
		gui->drawString(buffer.c_str(), _x+2, _y+3 + kLineHeight * i, _w - 4,
							(_selectedItem == pos && _hasFocus) ? gui->_bgcolor : gui->_textcolor);
	}
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
