/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"
#include "common/util.h"
#include "graphics/fontman.h"
#include "gui/widget.h"
#include "gui/dialog.h"
#include "gui/eval.h"
#include "gui/newgui.h"

namespace GUI {

Widget::Widget(GuiObject *boss, int x, int y, int w, int h)
	: GuiObject(x, y, w, h), _type(0), _boss(boss),
	  _id(0), _flags(0), _hints(THEME_HINT_FIRST_DRAW), _hasFocus(false) {
	init();
}

Widget::Widget(GuiObject *boss, String name)
	: GuiObject(name), _type(0), _boss(boss),
	  _id(0), _flags(0), _hints(THEME_HINT_FIRST_DRAW), _hasFocus(false) {
	init();
}

void Widget::init() {
	// Insert into the widget list of the boss
	_next = _boss->_firstWidget;
	_boss->_firstWidget = this;
	// HACK: we enable background saving for all widgets by default for now
	_hints = THEME_HINT_FIRST_DRAW | THEME_HINT_SAVE_BACKGROUND;
}

void Widget::resize(int x, int y, int w, int h) {
	_x = x;
	_y = y;
	_w = w;
	_h = h;
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
	//if (_flags & WIDGET_CLEARBG)
	//	gui->fillRect(_x, _y, _w, _h, gui->_bgcolor);

	// Draw border
	if (_flags & WIDGET_BORDER) {
		gui->theme()->drawWidgetBackground(Common::Rect(_x, _y, _x+_w, _y+_h), _hints, Theme::kWidgetBackgroundBorder);
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

	_x = oldX;
	_y = oldY;

	// Draw all children
	Widget *w = _firstWidget;
	while (w) {
		w->draw();
		w = w->_next;
	}

	clearHints(THEME_HINT_FIRST_DRAW);
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

Widget *Widget::findWidgetInChain(Widget *w, const char *name) {
	while (w) {
		if (w->_name == name) {
			return w;
		}
		w = w->_next;
	}
	return 0;
}

bool Widget::isVisible() const {
	if (g_gui.evaluator()->getVar(_name + ".visible") == 0)
		return false;

	return !(_flags & WIDGET_INVISIBLE);
}


#pragma mark -

StaticTextWidget::StaticTextWidget(GuiObject *boss, int x, int y, int w, int h, const String &text, TextAlignment align)
	: Widget(boss, x, y, w, h), _align(align) {
	_flags = WIDGET_ENABLED;
	_type = kStaticTextWidget;
	_label = text;
}

StaticTextWidget::StaticTextWidget(GuiObject *boss, String name, const String &text)
	: Widget(boss, name) {
	_flags = WIDGET_ENABLED;
	_type = kStaticTextWidget;
	_label = text;

	_align = (Graphics::TextAlignment)g_gui.evaluator()->getVar(name + ".align");

	if (_align == EVAL_UNDEF_VAR)
		_align = kTextAlignLeft;
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
	g_gui.theme()->drawText(Common::Rect(_x, _y, _x+_w, _y+_h), _label,
							isEnabled() ? Theme::kStateEnabled : Theme::kStateDisabled,
							g_gui.theme()->convertAligment(_align));
}

#pragma mark -

ButtonWidget::ButtonWidget(GuiObject *boss, int x, int y, int w, int h, const String &label, uint32 cmd, uint8 hotkey, WidgetSize ws)
	: StaticTextWidget(boss, x, y, w, h, label, kTextAlignCenter), CommandSender(boss),
	  _cmd(cmd), _hotkey(hotkey) {
	_flags = WIDGET_ENABLED/* | WIDGET_BORDER*/ | WIDGET_CLEARBG;
	_type = kButtonWidget;
}

ButtonWidget::ButtonWidget(GuiObject *boss, String name, const String &label, uint32 cmd, uint8 hotkey)
	: StaticTextWidget(boss, name, label), CommandSender(boss),
	  _cmd(cmd), _hotkey(hotkey) {
	_flags = WIDGET_ENABLED/* | WIDGET_BORDER*/ | WIDGET_CLEARBG;
	_type = kButtonWidget;
}

void ButtonWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && x >= 0 && x < _w && y >= 0 && y < _h)
		sendCommand(_cmd, 0);
}

void ButtonWidget::drawWidget(bool hilite) {
	g_gui.theme()->drawButton(Common::Rect(_x, _y, _x+_w, _y+_h), _label, isEnabled() ? (hilite ? Theme::kStateHighlight : Theme::kStateEnabled) : Theme::kStateDisabled);
}

#pragma mark -

CheckboxWidget::CheckboxWidget(GuiObject *boss, int x, int y, int w, int h, const String &label, uint32 cmd, uint8 hotkey, WidgetSize ws)
	: ButtonWidget(boss, x, y, w, h, label, cmd, hotkey, ws), _state(false) {
	_flags = WIDGET_ENABLED;
	_type = kCheckboxWidget;
}

CheckboxWidget::CheckboxWidget(GuiObject *boss, String name, const String &label, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, name, label, cmd, hotkey), _state(false) {
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
	g_gui.theme()->drawCheckbox(Common::Rect(_x, _y, _x+_w, _y+_h), _label, _state,
								isEnabled() ? (hilite ? Theme::kStateHighlight : Theme::kStateEnabled) : Theme::kStateDisabled);
}

#pragma mark -

SliderWidget::SliderWidget(GuiObject *boss, int x, int y, int w, int h, uint32 cmd)
	: Widget(boss, x, y, w, h), CommandSender(boss),
	  _cmd(cmd), _value(0), _oldValue(0), _valueMin(0), _valueMax(100), _isDragging(false) {
	_flags = WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG;
	_type = kSliderWidget;
}

SliderWidget::SliderWidget(GuiObject *boss, String name, uint32 cmd)
	: Widget(boss, name), CommandSender(boss),
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
	g_gui.theme()->drawSlider(Common::Rect(_x, _y, _x+_w, _y+_h), valueToPos(_value),
							isEnabled() ? (hilite ? Theme::kStateHighlight : Theme::kStateEnabled) : Theme::kStateDisabled);
}

int SliderWidget::valueToPos(int value) {
	return ((_w - 4) * (value - _valueMin) / (_valueMax - _valueMin));
}

int SliderWidget::posToValue(int pos) {
	return (pos) * (_valueMax - _valueMin) / (_w - 4) + _valueMin;
}

#pragma mark -

GraphicsWidget::GraphicsWidget(GuiObject *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w, h), _gfx(), _alpha(256), _transparency(false) {
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG;
	_type = kGraphicsWidget;
	// HACK: Don't save the background. We want to be sure that redrawing
	//       the widget updates the screen, even when there isn't any image
	//       to draw.
	_hints &= ~THEME_HINT_SAVE_BACKGROUND;
}

GraphicsWidget::GraphicsWidget(GuiObject *boss, String name)
	: Widget(boss, name), _gfx(), _alpha(256), _transparency(false) {
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG;
	_type = kGraphicsWidget;
	// HACK: Don't save the background. We want to be sure that redrawing
	//       the widget updates the screen, even when there isn't any image
	//       to draw.
	_hints &= ~THEME_HINT_SAVE_BACKGROUND;
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

void GraphicsWidget::setGfx(int w, int h, int r, int g, int b) {
	if (w == -1)
		w = _w;
	if (h == -1)
		h = _h;

	_gfx.free();
	_gfx.create(w, h, sizeof(OverlayColor));

	OverlayColor *dst = (OverlayColor*)_gfx.pixels;
	// TODO: get rid of g_system usage
	OverlayColor fillCol = g_system->RGBToColor(r, g, b);
	while (h--) {
		for (int i = 0; i < w; ++i) {
			*dst++ = fillCol;
		}
	}
}

void GraphicsWidget::drawWidget(bool hilite) {
	if (sizeof(OverlayColor) == _gfx.bytesPerPixel && _gfx.pixels) {
		g_gui.theme()->drawSurface(Common::Rect(_x, _y, _x+_w, _y+_h), _gfx, Theme::kStateEnabled, _alpha, _transparency);
	}
}

#pragma mark -

ContainerWidget::ContainerWidget(GuiObject *boss, int x, int y, int w, int h) : Widget(boss, x, y, w, h) {
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG;
	_type = kContainerWidget;
}

ContainerWidget::ContainerWidget(GuiObject *boss, String name) : Widget(boss, name) {
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG;
	_type = kContainerWidget;
}

void ContainerWidget::drawWidget(bool hilite) {
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + _h), _hints, Theme::kWidgetBackgroundBorder);
}

} // End of namespace GUI
