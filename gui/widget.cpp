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
#include "widget.h"
#include "dialog.h"
#include "newgui.h"


Widget::Widget (Dialog *boss, int x, int y, int w, int h)
	: _boss(boss), _x(x), _y(y), _w(w), _h(h), _id(0), _flags(0)
{
	// Insert into the widget list of the boss
	_next = _boss->_firstWidget;
	_boss->_firstWidget = this;
}

void Widget::draw()
{
	if (_flags & WIDGET_INVISIBLE)
		return;

	// Account for our relative position in the dialog
	_x += _boss->_x;
	_y += _boss->_y;

	// Clear background
	if (_flags & WIDGET_CLEARBG)
		_boss->_gui->clearArea(_x, _y, _w, _h);

	// Draw border
	if (_flags & WIDGET_BORDER) {
		_boss->_gui->box(_x, _y, _w, _h);
		_x += 4;
		_y += 4;
	}
	
	// Now perform the actual widget draw
	drawWidget(_flags & WIDGET_HILITED);

	if (_flags & WIDGET_BORDER) {
		_x -= 4;
		_y -= 4;
	}
	
	// Restore x/y
	_x -= _boss->_x;
	_y -= _boss->_y;
}


#pragma mark -


StaticTextWidget::StaticTextWidget(Dialog *boss, int x, int y, int w, int h, const char *text)
	: Widget (boss, x, y, w, h)
{
	// FIXME - maybe we should make a real copy of the string?
	_text = text;
}

void StaticTextWidget::drawWidget(bool hilite)
{
	NewGui *gui = _boss->getGui();
	gui->drawString(_text, _x, _y, _w, hilite ? gui->_textcolorhi : gui->_textcolor);
}


#pragma mark -


ButtonWidget::ButtonWidget(Dialog *boss, int x, int y, int w, int h, const char *label, uint32 cmd)
	: StaticTextWidget(boss, x, y, w, h, label), _cmd(cmd), _hotkey(0)
{
	_flags = WIDGET_ENABLED | WIDGET_BORDER /* | WIDGET_CLEARBG */ ;
}

void ButtonWidget::handleClick(int button)
{
	if (_flags & WIDGET_ENABLED && _cmd)
		_boss->handleCommand(_cmd);
}


#pragma mark -


/* 8x8 checkbox bitmap */
static uint32 checked_img[8] = {
	0x00000000,
	0x01000010,
	0x00100100,
	0x00011000,
	0x00011000,
	0x00100100,
	0x01000010,
	0x00000000,
};

CheckboxWidget::CheckboxWidget(Dialog *boss, int x, int y, int w, int h, const char *label, uint32 cmd)
	: ButtonWidget(boss, x, y, w, h, label, cmd), _state(false)
{
	_flags = WIDGET_ENABLED;
}

void CheckboxWidget::handleClick(int button)
{
	if (_flags & WIDGET_ENABLED) {
		_state = !_state;
		draw();
		if (_cmd)
			_boss->handleCommand(_cmd);
	}
}

void CheckboxWidget::drawWidget(bool hilite)
{
	NewGui *gui = _boss->getGui();
	
	// Draw the box
	gui->box(_x, _y, 14, 14);
	
	// If checked, draw cross inside the box
	if (_state)
		gui->drawBitmap(checked_img, _x + 3, _y + 3, gui->_textcolor);
	else
		gui->clearArea(_x + 3, _y + 3, 8, 8);
	
	// Finally draw the label
	gui->drawString(_text, _x + 20, _y + 3, _w, gui->_textcolor);
}
