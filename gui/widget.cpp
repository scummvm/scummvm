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
	: _type(0), _boss(boss), _x(x), _y(y), _w(w), _h(h), _id(0), _flags(0)
{
	// Insert into the widget list of the boss
	_next = _boss->_firstWidget;
	_boss->_firstWidget = this;
}

void Widget::draw()
{
	NewGui *gui = _boss->getGui();
	
	if (_flags & WIDGET_INVISIBLE)
		return;

	// Account for our relative position in the dialog
	_x += _boss->_x;
	_y += _boss->_y;

	// Clear background (unless alpha blending is enabled)
	if (_flags & WIDGET_CLEARBG)
		gui->fillRect(_x, _y, _w, _h, gui->_bgcolor);

	// Draw border
	if (_flags & WIDGET_BORDER) {
		gui->box(_x, _y, _w, _h);
		_x += 4;
		_y += 4;
		_w -= 8;
	}
	
	// Now perform the actual widget draw
	drawWidget(_flags & WIDGET_HILITED);
	
	// Flag the draw area as dirty
	gui->addDirtyRect(_x, _y, _w, _h);

	// Restore x/y
	if (_flags & WIDGET_BORDER) {
		_x -= 4;
		_y -= 4;
		_w += 8;
	}
	_x -= _boss->_x;
	_y -= _boss->_y;
}


#pragma mark -


StaticTextWidget::StaticTextWidget(Dialog *boss, int x, int y, int w, int h, const char *text)
	: Widget (boss, x, y, w, h), _label(0), _centred(false)
{
	_type = kStaticTextWidget;
	setLabel(text);
}

StaticTextWidget::~StaticTextWidget()
{
	if (_label) {
		free(_label);
		_label = 0;
	}
}

void StaticTextWidget::setLabel(const char *label)
{
	// Free old label if any
	if (_label)
		free(_label);

	// Duplicate new label
	if (label)
		_label = strdup(label);
	else
		_label = 0;
}

void StaticTextWidget::drawWidget(bool hilite)
{
	NewGui *gui = _boss->getGui();
	gui->drawString(_label, _x, _y, _w, hilite ? gui->_textcolorhi : gui->_textcolor, _centred);
}


#pragma mark -


ButtonWidget::ButtonWidget(Dialog *boss, int x, int y, int w, int h, const char *label, uint32 cmd, uint8 hotkey)
	: StaticTextWidget(boss, x, y, w, h, label), CommandSender(boss), _cmd(cmd), _hotkey(hotkey)
{
	assert(label);
	_flags = WIDGET_ENABLED | WIDGET_BORDER | WIDGET_CLEARBG ;
	_type = kButtonWidget;
	
	setCentred(true);
}

ButtonWidget::~ButtonWidget()
{
	if (_label) {
		free(_label);
		_label = 0;
	}
}

void ButtonWidget::handleMouseUp(int x, int y, int button)
{
	if (_flags & WIDGET_ENABLED && x >= 0 && x < _w && y >= 0 && y < _h)
		sendCommand(_cmd, 0);
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

CheckboxWidget::CheckboxWidget(Dialog *boss, int x, int y, int w, int h, const char *label, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, x, y, w, h, label, cmd, hotkey), _state(false)
{
	_flags = WIDGET_ENABLED;
	_type = kCheckboxWidget;
}

void CheckboxWidget::handleMouseDown(int x, int y, int button)
{
	if (_flags & WIDGET_ENABLED) {
		_state = !_state;
		draw();
		sendCommand(_cmd, 0);
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
		gui->fillRect(_x + 2, _y + 2, 10, 10, gui->_bgcolor);
	
	// Finally draw the label
	gui->drawString(_label, _x + 20, _y + 3, _w, gui->_textcolor);
}

#pragma mark -

SliderWidget::SliderWidget(Dialog *boss, int x, int y, int w, int h, const char *label, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, x, y, w, h, label, cmd, hotkey), _value(0), _old_value(1)
{
	_flags = WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG;
	_type = kSliderWidget;
}

void SliderWidget::handleMouseMoved(int x, int y, int button) { 
	if (_isDragging) {
		int newvalue = x * 100 / _w;

		if (newvalue != _value) {
			_value = newvalue; 
			draw();
		}
	}
}

void SliderWidget::drawWidget(bool hilite)
{
	NewGui *gui = _boss->getGui();
	
	// Draw the box
	gui->box(_x, _y, _w, _h);
	
	// Remove old 'bar' if necessary
	if (_value != _old_value) {
		gui->fillRect(_x + 2 + ((_w - 5) * _old_value / 100), _y + 2, 2, _h - 4, gui->_bgcolor);
		_old_value = _value;
	}

	// Draw the 'bar'
	gui->fillRect(_x + 2 + ((_w - 5) * _value / 100), _y + 2, 2, _h - 4, hilite ? gui->_textcolorhi : gui->_textcolor);
}

void SliderWidget::handleMouseDown(int x, int y, int button) {
	int barx;

	barx=2 + ((_w - 5) * _old_value / 100);
	
	// only start dragging if mouse is over bar
	if (x > (barx-3) && x < (barx+3))
		_isDragging=true;
}

void SliderWidget::handleMouseUp(int x, int y, int button) {
	if (_isDragging)
		_isDragging=false;
}
