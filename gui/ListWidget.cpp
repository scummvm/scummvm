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


/*
 * TODO:
 * - Fix the random key repeating bug
 * - Allow escape to abort changes
 * - Add key repeat (for backspace, etc)
 */


// Height of one entry
#define	LINE_HEIGHT		10


ListWidget::ListWidget(Dialog *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w - kScrollBarWidth, h)
{
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE;
	_type = kListWidget;
	_numberingMode = kListNumberingOne;
	_entriesPerPage = (_h - 4) / LINE_HEIGHT;
	_currentPos = 3;
	_selectedItem = -1;
	_scrollBar = new ScrollBarWidget(boss, _x + _w, _y, kScrollBarWidth, _h);
	_scrollBar->setTarget(this);
	
	// FIXME: This flag should come from widget definition
	_editable = true;

	_editMode = false;

	// FIXME - fill in dummy data for now
	_list.push_back("A simple game?");
	_list.push_back("This space for rent!");
	_list.push_back("To be or not to be...");
	_list.push_back("It's not easy come up with dummy text :-)");
	_list.push_back("Foo bar baz");
	_list.push_back("Empty slots follow:");
	_list.push_back("");
	_list.push_back("");
	_list.push_back("Now again a filled slot");
	_list.push_back("We need some more text!");
	_list.push_back("Because only this way...");
	_list.push_back("...can you see the scrollbar...");
	_list.push_back("...and verify that it works!");
	_list.push_back("One");
	_list.push_back("Two");
	_list.push_back("Three");
	_list.push_back("Four");
	_list.push_back("The End");


	_scrollBar->_numEntries = _list.size();
	_scrollBar->_entriesPerPage = _entriesPerPage;
	_scrollBar->_currentPos = _currentPos;
	_scrollBar->recalc();
}

ListWidget::~ListWidget()
{
}

void ListWidget::handleMouseDown(int x, int y, int button)
{
	int oldSelectedItem = _selectedItem;

	if (_flags & WIDGET_ENABLED) {
		_selectedItem = (y - 2) / LINE_HEIGHT + _currentPos;

		if (_editMode && oldSelectedItem != _selectedItem) {
			// loose caret
			_list[_selectedItem].deleteLastChar();
			_editMode = false;
		}
		draw();
	}
}

void ListWidget::handleKeyDown(char key, int modifiers)
{
	bool dirty = false;
	int oldSelectedItem = _selectedItem;

	if (_editMode) {

		// get rid of caret
		_list[_selectedItem].deleteLastChar();

		if (key == '\n' || key == '\r') {
			// enter, exit editmode
			_editMode = false;
			dirty = true;
		}
		else if (_editMode && key == 8) {	// backspace
			_list[_selectedItem].deleteLastChar();
			dirty = true;
		} else if (_editMode &&
					// filter keystrokes
					( ( key >= 'a' && key <= 'z' )
					|| ( key >= 'A' && key <= 'Z' )
					|| ( key >= '0' && key <= '9' )
					|| ( key == ' ')
					) )
			{

			_list[_selectedItem] += key;
			dirty = true;
		}

	} else {
		// not editmode

		switch (key) {
		case '\n':	// enter
		case '\r':
			if (_selectedItem >= 0) {
				_editMode = true;
				dirty = true;
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
		if (_currentPos != data) {
			_currentPos = data;
			draw();
		}
		break;
	}
}

void ListWidget::drawWidget(bool hilite)
{
	NewGui	*gui = _boss->getGui();
	int		i, pos;
	String	buffer;

	// Draw the list items
	// FIXME - this is just a temporary demo hack
	for (i = 0, pos = _currentPos; i < _entriesPerPage; i++, pos++) {
		if (_numberingMode == kListNumberingZero || _numberingMode == kListNumberingOne) {
			char temp[10];
			sprintf(temp, "%2d. ", (pos + _numberingMode));
			buffer = temp;
		} else
			buffer = "";
		buffer += _list[pos];
	
		gui->drawString(buffer, _x+5, _y+2 + LINE_HEIGHT * i, _w - 10,
							(_selectedItem == pos && _hasFocus) ? gui->_textcolorhi : gui->_textcolor);
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
		if (_currentPos < 0)
			_currentPos = 0;
	}

	_scrollBar->_currentPos = _currentPos;
	_scrollBar->recalc();
}
