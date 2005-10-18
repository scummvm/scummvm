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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/util.h"
#include "graphics/fontman.h"
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

StaticTextWidget::StaticTextWidget(GuiObject *boss, int x, int y, int w, int h, const String &text, TextAlignment align, WidgetSize ws)
	: Widget(boss, x, y, w, h), _align(align), _ws(ws) {
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

ButtonWidget::ButtonWidget(GuiObject *boss, int x, int y, int w, int h, const String &label, uint32 cmd, uint8 hotkey, WidgetSize ws)
	: StaticTextWidget(boss, x, y, w, h, label, kTextAlignCenter, ws), CommandSender(boss),
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
	const int off = (_h - g_gui.getFontHeight()) / 2;
	gui->drawString(_label, _x, _y + off, _w,
					!isEnabled() ? gui->_color :
					hilite ? gui->_textcolorhi : gui->_textcolor, _align);
}

#pragma mark -

CheckboxWidget::CheckboxWidget(GuiObject *boss, int x, int y, int w, int h, const String &label, uint32 cmd, uint8 hotkey, WidgetSize ws)
	: ButtonWidget(boss, x, y, w, h, label, cmd, hotkey, ws), _state(false) {
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
	int fontHeight = gui->getFontHeight();

	// Draw the box
	gui->box(_x, _y, fontHeight + 4, fontHeight + 4, gui->_color, gui->_shadowcolor);
	gui->fillRect(_x + 2, _y + 2, fontHeight, fontHeight, gui->_bgcolor);

	// If checked, draw cross inside the box
	if (_state) {
		Graphics::Surface &surf = gui->getScreen();
		Common::Point p0, p1, p2, p3;
		OverlayColor color = isEnabled() ? gui->_textcolor : gui->_color;

		p0 = Common::Point(_x + 4, _y + 4);
		p1 = Common::Point(_x + fontHeight - 1, _y + 4);
		p2 = Common::Point(_x + 4, _y + fontHeight - 1);
		p3 = Common::Point(_x + fontHeight - 1, _y + fontHeight - 1);

		if (_ws == kBigWidgetSize) {
			surf.drawLine(p0.x + 1, p0.y, p3.x, p3.y - 1, color);
			surf.drawLine(p0.x, p0.y + 1, p3.x - 1, p3.y, color);
			surf.drawLine(p0.x + 1, p0.y + 1, p3.x - 1, p3.y - 1, color);
			surf.drawLine(p2.x + 1, p2.y - 1, p1.x - 1, p1.y + 1, color);
			surf.drawLine(p2.x + 1, p2.y, p1.x, p1.y + 1, color);
			surf.drawLine(p2.x, p2.y - 1, p1.x - 1, p1.y, color);
		} else {
			surf.drawLine(p0.x, p0.y, p3.x, p3.y, color);
			surf.drawLine(p2.x, p2.y, p1.x, p1.y, color);
		}
	}

	// Finally draw the label
	gui->drawString(_label, _x + fontHeight + 10, _y + 3, _w, isEnabled() ? gui->_textcolor : gui->_color);
}

#pragma mark -

SliderWidget::SliderWidget(GuiObject *boss, int x, int y, int w, int h, uint32 cmd)
	: Widget(boss, x, y, w, h), CommandSender(boss),
	  _cmd(cmd), _value(0), _oldValue(0), _valueMin(0), _valueMax(100), _isDragging(false) {
	_flags = WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG;
	_type = kSliderWidget;
}

void SliderWidget::handleMouseMoved(int x, int y, int button) {
	if (isEnabled() && _isDragging && x >= 0) {
		int newValue = posToValue(x);
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

	// Draw the box
	gui->box(_x, _y, _w, _h, gui->_color, gui->_shadowcolor);

	// Draw the 'bar'
	gui->fillRect(_x + 2, _y + 2, valueToPos(_value), _h - 4,
				!isEnabled() ? gui->_color :
				hilite ? gui->_textcolorhi : gui->_textcolor);
}

int SliderWidget::valueToPos(int value) {
	return ((_w - 4) * (value - _valueMin) / (_valueMax - _valueMin));
}

int SliderWidget::posToValue(int pos) {
	return (pos) * (_valueMax - _valueMin) / (_w - 4) + _valueMin;
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

	if (!gfx || !gfx->pixels)
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
	} else
		g_gui.drawSurface(_gfx, _x, _y);
}

} // End of namespace GUI
