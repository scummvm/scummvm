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
#include "dialog.h"
#include "newgui.h"


/*
 * Some thoughts:
 * - We should split out the scrollbar into a seperate widget. This will
 *   simplify the drawing & mouse handling considerably, but also requires
 *   us to come up with a way to couple both widgets (shouldn't be to hard)
 * - Write a class to encapsulate the data instead of using std::list<string>.
 *   How exactly this will look and what it does has yet to be determined.
 * - The handleKey method of widgets is currently never called, code for that has
 *   to be added to dialog.cpp
 * - ...
 */


// Height of one entry
#define	LINE_HEIGHT		10


// Up/down arrow for the scrollbar
static uint32 up_arrow[8] = {
	0x00000000,
	0x00000000,
	0x00001000,
	0x00001000,
	0x00011100,
	0x00011100,
	0x00110110,
	0x00100010,
};

static uint32 down_arrow[8] = {
	0x00000000,
	0x00000000,
	0x00100010,
	0x00110110,
	0x00011100,
	0x00011100,
	0x00001000,
	0x00001000,
};

ListWidget::ListWidget(Dialog *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w, h)
{
	_flags = WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG;
	_type = kListWidget;
}

ListWidget::~ListWidget()
{
}

void ListWidget::handleClick(int button)
{
	if (_flags & WIDGET_ENABLED) {
	}
}

void ListWidget::handleMouseMoved(int x, int y, int state)
{
}


void ListWidget::handleKey(char key, int modifiers)
{
}

void ListWidget::drawWidget(bool hilite)
{
	NewGui *gui = _boss->getGui();
	int		rightX = _x + _w - 1;
	int		leftX = rightX - 8;
	int		bottomY = _y + _h;
	
	gui->frameRect(leftX, _y, 9, _h, gui->_shadowcolor);
	
	// Up arrow
	gui->fillRect(leftX, _y, 9, 10, gui->_bgcolor);
	gui->frameRect(leftX, _y, 9, 10, gui->_color);
	gui->drawBitmap(up_arrow, leftX, _y, gui->_textcolor);

	// Down arrow
	gui->fillRect(leftX, bottomY - 9, 9, 10, gui->_bgcolor);
	gui->frameRect(leftX, bottomY - 9, 9, 10, gui->_color);
	gui->drawBitmap(down_arrow, leftX, bottomY - 9, gui->_textcolor);

	// Slider
	// FIXME - determine slider position and size. This depends on:
	// * the number of entries/page
	// * total number of entries
	// * current scroll position (i.e. idx of "first" visible entry
	gui->fillRect(leftX, _y+20, 9, 4, gui->_textcolor);
	gui->frameRect(leftX, _y+20, 9, 4, gui->_color);
	
	// Now draw the list items
	// FIXME - this is just a temporary demo hack
	gui->drawString("1. A simple game", _x+1, _y+1, _w - 10, gui->_textcolor);
	gui->drawString("2. This space for rent", _x+1, _y+1 + LINE_HEIGHT, _w - 10, gui->_textcolorhi);
	gui->drawString("3. To be or not to be", _x+1, _y+1 + LINE_HEIGHT*2, _w - 10, gui->_textcolor);
}
