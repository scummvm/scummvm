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
#include "ScrollBarWidget.h"
#include "dialog.h"
#include "newgui.h"


/*
 * TODO:
 * - Dragging the slider with the mouse
 * - Auto-repeast: if one clicks & holds on one of the arrows, then after a
 *   brief delay, it should start to contiously scroll
 * - Allow for a horizontal scrollbar, too?
 */


// Up arrow
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

// Up arrow
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

ScrollBarWidget::ScrollBarWidget(Dialog *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w, h), CommandSender(boss)
{
	_flags = WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG;
	_type = kScrollBarWidget;
	
	_part = kNoPart;
	
}

void ScrollBarWidget::handleClick(int x, int y, int button)
{
	int old_pos = _currentPos;
	if (y <= 10) {
		// Up arrow
		_currentPos--;
	} else if (y >= _h-10) {
		// Down arrow
		_currentPos++;
	} else if (y < _sliderPos) {
		_currentPos -= _entriesPerPage;
	} else if (y >= _sliderPos + _sliderHeight) {
		_currentPos += _entriesPerPage;
	} else {
		printf("Slider\n");
	}
	
	// Bound checking
	if (_currentPos > _numEntries - _entriesPerPage)
		_currentPos = _numEntries - _entriesPerPage;
	else if (_currentPos < 0)
		_currentPos = 0;
	
	// Redraw & notify, but only if the position really changed
	if (old_pos != _currentPos) {
		recalc();
		draw();
		sendCommand(kSetPositionCmd, _currentPos);
	}
}

void ScrollBarWidget::handleMouseMoved(int x, int y, int button)
{
	int old_part = _part;
	if (y <= 10)				// Up arrow
		_part = kUpArrowPart;
	else if (y >= _h-10)		// Down arrow
		_part = kDownArrowPart;
	else if (y < _sliderPos)
		_part = kPageUpPart;
	else if (y >= _sliderPos + _sliderHeight)
		_part = kPageDownPart;
	else
		_part = kSliderPart;
	
	if (old_part != _part)
		draw();
}

void ScrollBarWidget::recalc()
{
	_sliderHeight = (_h-20) * _entriesPerPage / _numEntries;
	if (_sliderHeight < 4)
		_sliderHeight = 4;
	
	_sliderPos = 10 + (_h-20-_sliderHeight+1) *_currentPos / (_numEntries - _entriesPerPage);
	if (_sliderPos < 0)
		_sliderPos = 0;
	
}


void ScrollBarWidget::drawWidget(bool hilite)
{
	NewGui *gui = _boss->getGui();
	int		bottomY = _y + _h;
	
	gui->frameRect(_x, _y, _w, _h, gui->_shadowcolor);
	
	// Up arrow
	gui->frameRect(_x, _y, _w, 10, gui->_color);
	gui->drawBitmap(up_arrow, _x, _y,
				(hilite && _part == kUpArrowPart) ? gui->_textcolorhi : gui->_textcolor);
	
	// Down arrow
	gui->frameRect(_x, bottomY - 9, _w, 10, gui->_color);
	gui->drawBitmap(down_arrow, _x, bottomY - 9,
				(hilite && _part == kDownArrowPart) ? gui->_textcolorhi : gui->_textcolor);
	
	// Slider
	// FIXME - determine slider position and size. This depends on:
	// * the number of entries per page
	// * total number of entries
	// * current scroll position (i.e. index of "first" visible entry)
	gui->checkerRect(_x, _y + _sliderPos, _w, _sliderHeight,
				(hilite && _part == kSliderPart) ? gui->_textcolorhi : gui->_textcolor);
	gui->frameRect(_x, _y + _sliderPos, _w, _sliderHeight, gui->_color);
}
