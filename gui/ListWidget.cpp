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
 * - The handleKey method of widgets is currently never called, code for that has
 *   to be added to dialog.cpp
 * - Once the above item is done, implement scrolling using arrow keys,
 *   pageup/pagedown, home/end keys etc.
 * - Allow user to select an entry w/ the mouse
 * - Implement editing of the selected string in a generic fashion
 */


// Height of one entry
#define	LINE_HEIGHT		10


ListWidget::ListWidget(Dialog *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w - kScrollBarWidth, h)
{
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG;
	_type = kListWidget;
	_numberingMode = kListNumberingOne;
	_entriesPerPage = (_h - 4) / LINE_HEIGHT;
	_currentPos = 3;
	_selectedItem = -1;
	_scrollBar = new ScrollBarWidget(boss, _x + _w, _y, kScrollBarWidth, _h);
	_scrollBar->setTarget(this);
	
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
	if (_flags & WIDGET_ENABLED) {
		_selectedItem = (y - 2) / LINE_HEIGHT + _currentPos;
		draw();
	}
}

void ListWidget::handleKeyDown(char key, int modifiers)
{
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
							(_selectedItem == pos) ? gui->_textcolorhi : gui->_textcolor);
	}
}
