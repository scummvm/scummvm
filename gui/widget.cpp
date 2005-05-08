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
#include "common/util.h"
#include "gui/widget.h"
#include "gui/dialog.h"
#include "gui/newgui.h"

namespace GUI {

Widget::Widget(GuiObject *boss, int x, int y, int w, int h)
	: GuiObject(x, y, w, h), _type(0), _boss(boss),
	  _id(0), _flags(0), _hasFocus(false) {
	// Insert into the widget list of the boss
	_next = _boss->_firstWidget;
	_boss->_firstWidget = this;
}

Widget::~Widget() {
	delete _next;
}

void Widget::draw() {
	NewGui *gui = &g_gui;

	if (!isVisible() || !_boss->isVisible())
		return;

	int oldX = _x, oldY = _y;

	// Account for our relative position in the dialog
	_x = getAbsX();
	_y = getAbsY();

	// Clear background (unless alpha blending is enabled)
	if (_flags & WIDGET_CLEARBG)
		gui->fillRect(_x, _y, _w, _h, gui->_bgcolor);

	// Draw border
	if (_flags & WIDGET_BORDER) {
		OverlayColor colorA = gui->_color;
		OverlayColor colorB = gui->_shadowcolor;
		if ((_flags & WIDGET_INV_BORDER) == WIDGET_INV_BORDER)
			SWAP(colorA, colorB);
		gui->box(_x, _y, _w, _h, colorA, colorB);
		_x += 4;
		_y += 4;
		_w -= 8;
		_h -= 8;
	}

	// Now perform the actual widget draw
	drawWidget((_flags & WIDGET_HILITED) ? true : false);

	// Restore x/y
	if (_flags & WIDGET_BORDER) {
		_x -= 4;
		_y -= 4;
		_w += 8;
		_h += 8;
	}

	// Flag the draw area as dirty
	gui->addDirtyRect(_x, _y, _w, _h);

	_x = oldX;
	_y = oldY;

	// Draw all children
	Widget *w = _firstWidget;
	while (w) {
		w->draw();
		w = w->_next;
	}
}

Widget *Widget::findWidgetInChain(Widget *w, int x, int y) {
	while (w) {
		// Stop as soon as we find a widget that contains the point (x,y)
		if (x >= w->_x && x < w->_x + w->_w && y >= w->_y && y < w->_y + w->_h)
			break;
		w = w->_next;
	}
	if (w)
		w = w->findWidget(x - w->_x, y - w->_y);
	return w;
}

#pragma mark -

StaticTextWidget::StaticTextWidget(GuiObject *boss, int x, int y, int w, int h, const String &text, TextAlignment align)
	: Widget(boss, x, y, w, h), _align(align) {
	_flags = WIDGET_ENABLED;
	_type = kStaticTextWidget;
	_label = text;
}

void StaticTextWidget::setValue(int value) {
	char buf[256];
	sprintf(buf, "%d", value);
	_label = buf;
}

void StaticTextWidget::setLabel(const String &label) {
	_label = label;
	// TODO: We should automatically redraw when the label is changed.
	// The following doesn't quite work when we are using tabs, plus it
	// is rather clumsy to force a full redraw for a single static text.
	// However, as long as we do blending, it might be the only way.
	//_boss->draw();
}

void StaticTextWidget::setAlign(TextAlignment align) {
	_align = align;
	// TODO: We should automatically redraw when the alignment is changed.
	// See setLabel() for more insights.
}


void StaticTextWidget::drawWidget(bool hilite) {
	NewGui *gui = &g_gui;
	gui->drawString(_label, _x, _y, _w, isEnabled() ? gui->_textcolor : gui->_color, _align);
}

#pragma mark -

ButtonWidget::ButtonWidget(GuiObject *boss, int x, int y, int w, int h, const String &label, uint32 cmd, uint8 hotkey)
	: StaticTextWidget(boss, x, y, w, h, label, kTextAlignCenter), CommandSender(boss),
	  _cmd(cmd), _hotkey(hotkey) {
	_flags = WIDGET_ENABLED | WIDGET_BORDER | WIDGET_CLEARBG;
	_type = kButtonWidget;
}

void ButtonWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && x >= 0 && x < _w && y >= 0 && y < _h)
		sendCommand(_cmd, 0);
}

void ButtonWidget::drawWidget(bool hilite) {
	NewGui *gui = &g_gui;
	gui->drawString(_label, _x, _y + (_h - kLineHeight)/2 + 1, _w,
					!isEnabled() ? gui->_color :
					hilite ? gui->_textcolorhi : gui->_textcolor, _align);
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

CheckboxWidget::CheckboxWidget(GuiObject *boss, int x, int y, int w, int h, const String &label, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, x, y, w, h, label, cmd, hotkey), _state(false) {
	_flags = WIDGET_ENABLED;
	_type = kCheckboxWidget;
}

void CheckboxWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && x >= 0 && x < _w && y >= 0 && y < _h) {
		toggleState();
	}
}

void CheckboxWidget::setState(bool state) {
	if (_state != state) {
		_state = state;
		_flags ^= WIDGET_INV_BORDER;
		draw();
	}
	sendCommand(_cmd, _state);
}

void CheckboxWidget::drawWidget(bool hilite) {
	NewGui *gui = &g_gui;

	// Draw the box
	gui->box(_x, _y, 14, 14, gui->_color, gui->_shadowcolor);
	gui->fillRect(_x + 2, _y + 2, 10, 10, gui->_bgcolor);

	// If checked, draw cross inside the box
	if (_state)
		gui->drawBitmap(checked_img, _x + 4, _y + 3, isEnabled() ? gui->_textcolor : gui->_color);

	// Finally draw the label
	gui->drawString(_label, _x + 20, _y + 3, _w, isEnabled() ? gui->_textcolor : gui->_color);
}

#pragma mark -

SliderWidget::SliderWidget(GuiObject *boss, int x, int y, int w, int h, const String &label, uint labelWidth, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, x, y, w, h, label, cmd, hotkey),
	  _value(0), _oldValue(0), _valueMin(0), _valueMax(100), _isDragging(false),
	  _labelWidth(labelWidth) {
	_flags = WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG;
	_type = kSliderWidget;
}

void SliderWidget::handleMouseMoved(int x, int y, int button) {
	if (isEnabled() && _isDragging && x >= (int)_labelWidth) {
		int newValue = posToValue(x - _labelWidth);
		if (newValue < _valueMin)
			newValue = _valueMin;
		else if (newValue > _valueMax)
			newValue = _valueMax;

		if (newValue != _value) {
			_value = newValue; 
			draw();
			sendCommand(_cmd, _value);	// FIXME - hack to allow for "live update" in sound dialog
		}
	}
}

void SliderWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (isEnabled()) {
		_isDragging = true;
		handleMouseMoved(x, y, button);
	}
}

void SliderWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && _isDragging) {
		sendCommand(_cmd, _value);
	}
	_isDragging = false;
}

void SliderWidget::drawWidget(bool hilite) {
	NewGui *gui = &g_gui;

	// Draw the label, if any
	if (_labelWidth > 0)
		gui->drawString(_label, _x, _y + 2, _labelWidth, isEnabled() ? gui->_textcolor : gui->_color, kTextAlignRight);

	// Draw the box
	gui->box(_x + _labelWidth, _y, _w - _labelWidth, _h, gui->_color, gui->_shadowcolor);

	// Draw the 'bar'
	gui->fillRect(_x + _labelWidth + 2, _y + 2, valueToPos(_value), _h - 4,
				!isEnabled() ? gui->_color :
				hilite ? gui->_textcolorhi : gui->_textcolor);
}

int SliderWidget::valueToPos(int value) {
	return ((_w - _labelWidth - 4) * (value - _valueMin) / (_valueMax - _valueMin));
}

int SliderWidget::posToValue(int pos) {
	return (pos) * (_valueMax - _valueMin) / (_w - _labelWidth - 4) + _valueMin;
}

#pragma mark -

GraphicsWidget::GraphicsWidget(GuiObject *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w, h), _gfx() {
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG; 
	_type = kGraphicsWidget;
}

GraphicsWidget::~GraphicsWidget() {
	_gfx.free();
}

void GraphicsWidget::setGfx(const Graphics::Surface *gfx) {
	_gfx.free();
	
	if (!gfx)
		return;
	if (!gfx->pixels)
		return;
	
	// TODO: add conversion to OverlayColor
	_gfx.create(gfx->w, gfx->h, gfx->bytesPerPixel);
	memcpy(_gfx.pixels, gfx->pixels, gfx->h * gfx->pitch);
}

void GraphicsWidget::drawWidget(bool hilite) {
	if (sizeof(OverlayColor) != _gfx.bytesPerPixel || !_gfx.pixels) {
		// FIXME: It doesn't really make sense to render this text here, since
		// this widget might be used for other things than rendering savegame
		// graphics/previews...
		g_gui.drawString("No preview", _x, _y + _h / 2 - g_gui.getFontHeight() / 2, _w, g_gui._textcolor, Graphics::kTextAlignCenter);
		return;
	}
	
	uint drawWidth = _gfx.w, drawHeight = _gfx.h;
	
	if (_w < _gfx.w)
		drawWidth = _w;
	if (_h < _gfx.h)
		drawHeight = _h;
	
	g_gui.drawSurface(_gfx, _x, _y);
}

} // End of namespace GUI
