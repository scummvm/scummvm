/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "common/util.h"
#include "graphics/fontman.h"
#include "gui/widget.h"
#include "gui/dialog.h"
#include "gui/GuiManager.h"

#include "gui/ThemeEval.h"

namespace GUI {

Widget::Widget(GuiObject *boss, int x, int y, int w, int h)
	: GuiObject(x, y, w, h), _type(0), _boss(boss),
	  _id(0), _flags(0), _hasFocus(false), _state(ThemeEngine::kStateEnabled) {
	init();
}

Widget::Widget(GuiObject *boss, const Common::String &name)
	: GuiObject(name), _type(0), _boss(boss),
	  _id(0), _flags(0), _hasFocus(false), _state(ThemeEngine::kStateDisabled) {
	init();
}

void Widget::init() {
	// Insert into the widget list of the boss
	_next = _boss->_firstWidget;
	_boss->_firstWidget = this;
}

Widget::~Widget() {
	delete _next;
	_next = 0;
}

void Widget::resize(int x, int y, int w, int h) {
	_x = x;
	_y = y;
	_w = w;
	_h = h;
}

void Widget::setFlags(int flags) {
	updateState(_flags, _flags | flags);
	_flags |= flags;
}

void Widget::clearFlags(int flags) {
	updateState(_flags, _flags & ~flags);
	_flags &= ~flags;
}

void Widget::updateState(int oldFlags, int newFlags) {
	if (newFlags & WIDGET_ENABLED) {
		_state = ThemeEngine::kStateEnabled;
		if (newFlags & WIDGET_HILITED)
			_state = ThemeEngine::kStateHighlight;
	} else {
		_state = ThemeEngine::kStateDisabled;
	}
}

void Widget::draw() {
	if (!isVisible() || !_boss->isVisible())
		return;

	int oldX = _x, oldY = _y;

	// Account for our relative position in the dialog
	_x = getAbsX();
	_y = getAbsY();

	// Draw border
	if (_flags & WIDGET_BORDER) {
		g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x+_w, _y+_h), 0, ThemeEngine::kWidgetBackgroundBorder);
		_x += 4;
		_y += 4;
		_w -= 8;
		_h -= 8;
	}

	// Now perform the actual widget draw
	drawWidget();

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

void Widget::setEnabled(bool e) {
	if ((_flags & WIDGET_ENABLED) != e) {
		if (e)
			setFlags(WIDGET_ENABLED);
		else
			clearFlags(WIDGET_ENABLED);

		_boss->draw();
	}
}

bool Widget::isEnabled() const {
	if (g_gui.xmlEval()->getVar("Dialog." + _name + ".Enabled", 1) == 0) {
		return false;
	}
	return ((_flags & WIDGET_ENABLED) != 0);
}

void Widget::setVisible(bool e) {
	if (e)
		clearFlags(WIDGET_INVISIBLE);
	else
		setFlags(WIDGET_INVISIBLE);
}

bool Widget::isVisible() const {
	if (g_gui.xmlEval()->getVar("Dialog." + _name + ".Visible", 1) == 0)
		return false;

	return !(_flags & WIDGET_INVISIBLE);
}

#pragma mark -

StaticTextWidget::StaticTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::String &text, Graphics::TextAlign align)
	: Widget(boss, x, y, w, h), _align(align) {
	setFlags(WIDGET_ENABLED);
	_type = kStaticTextWidget;
	_label = text;
}

StaticTextWidget::StaticTextWidget(GuiObject *boss, const Common::String &name, const Common::String &text)
	: Widget(boss, name) {
	setFlags(WIDGET_ENABLED);
	_type = kStaticTextWidget;
	_label = text;

	_align = (Graphics::TextAlign)g_gui.xmlEval()->getVar("Dialog." + name + ".Align", Graphics::kTextAlignLeft);
}

void StaticTextWidget::setValue(int value) {
	char buf[256];
	sprintf(buf, "%d", value);
	_label = buf;
}

void StaticTextWidget::setLabel(const Common::String &label) {
    if (_label != label) {
        _label = label;

        // when changing the label, add the CLEARBG flag
        // so the widget is completely redrawn, otherwise
        // the new text is drawn on top of the old one.
        setFlags(WIDGET_CLEARBG);
        draw();
        clearFlags(WIDGET_CLEARBG);
    }
}

void StaticTextWidget::setAlign(Graphics::TextAlign align) {
	_align = align;
	// TODO: We should automatically redraw when the alignment is changed.
	// See setLabel() for more insights.
}


void StaticTextWidget::drawWidget() {
	g_gui.theme()->drawText(Common::Rect(_x, _y, _x+_w, _y+_h), _label, _state, _align);
}

#pragma mark -

ButtonWidget::ButtonWidget(GuiObject *boss, int x, int y, int w, int h, const Common::String &label, uint32 cmd, uint8 hotkey)
	: StaticTextWidget(boss, x, y, w, h, label, Graphics::kTextAlignCenter), CommandSender(boss),
	  _cmd(cmd), _hotkey(hotkey) {
	setFlags(WIDGET_ENABLED/* | WIDGET_BORDER*/ | WIDGET_CLEARBG);
	_type = kButtonWidget;
}

ButtonWidget::ButtonWidget(GuiObject *boss, const Common::String &name, const Common::String &label, uint32 cmd, uint8 hotkey)
	: StaticTextWidget(boss, name, label), CommandSender(boss),
	  _cmd(cmd), _hotkey(hotkey) {
	setFlags(WIDGET_ENABLED/* | WIDGET_BORDER*/ | WIDGET_CLEARBG);
	_type = kButtonWidget;
}

void ButtonWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && x >= 0 && x < _w && y >= 0 && y < _h)
		sendCommand(_cmd, 0);
}

void ButtonWidget::drawWidget() {
	g_gui.theme()->drawButton(Common::Rect(_x, _y, _x+_w, _y+_h), _label, _state, getFlags());
}

#pragma mark -

CheckboxWidget::CheckboxWidget(GuiObject *boss, int x, int y, int w, int h, const Common::String &label, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, x, y, w, h, label, cmd, hotkey), _state(false) {
	setFlags(WIDGET_ENABLED);
	_type = kCheckboxWidget;
}

CheckboxWidget::CheckboxWidget(GuiObject *boss, const Common::String &name, const Common::String &label, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, name, label, cmd, hotkey), _state(false) {
	setFlags(WIDGET_ENABLED);
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
		//_flags ^= WIDGET_INV_BORDER;
		draw();
	}
	sendCommand(_cmd, _state);
}

void CheckboxWidget::drawWidget() {
	g_gui.theme()->drawCheckbox(Common::Rect(_x, _y, _x+_w, _y+_h), _label, _state, Widget::_state);
}

#pragma mark -

SliderWidget::SliderWidget(GuiObject *boss, int x, int y, int w, int h, uint32 cmd)
	: Widget(boss, x, y, w, h), CommandSender(boss),
	  _cmd(cmd), _value(0), _oldValue(0), _valueMin(0), _valueMax(100), _isDragging(false) {
	setFlags(WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG);
	_type = kSliderWidget;
}

SliderWidget::SliderWidget(GuiObject *boss, const Common::String &name, uint32 cmd)
	: Widget(boss, name), CommandSender(boss),
	  _cmd(cmd), _value(0), _oldValue(0), _valueMin(0), _valueMax(100), _isDragging(false) {
	setFlags(WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG);
	_type = kSliderWidget;
}

void SliderWidget::handleMouseMoved(int x, int y, int button) {
	if (isEnabled() && _isDragging) {
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

void SliderWidget::handleMouseWheel(int x, int y, int direction) {
	if (isEnabled() && !_isDragging) {
		// Increment or decrement one position
		int newValue = posToValue(valueToPos(_value) - 1 * direction);

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

void SliderWidget::drawWidget() {
	g_gui.theme()->drawSlider(Common::Rect(_x, _y, _x + _w, _y + _h), valueToBarWidth(_value), _state);
}

int SliderWidget::valueToBarWidth(int value) {
	return (_w * (value - _valueMin) / (_valueMax - _valueMin));
}

int SliderWidget::valueToPos(int value) {
	return ((_w - 1) * (value - _valueMin + 1) / (_valueMax - _valueMin));
}

int SliderWidget::posToValue(int pos) {
	return (pos) * (_valueMax - _valueMin) / (_w - 1) + _valueMin;
}

#pragma mark -

GraphicsWidget::GraphicsWidget(GuiObject *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w, h), _gfx(), _alpha(256), _transparency(false) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG);
	_type = kGraphicsWidget;
}

GraphicsWidget::GraphicsWidget(GuiObject *boss, const Common::String &name)
	: Widget(boss, name), _gfx(), _alpha(256), _transparency(false) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG);
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
	_gfx.copyFrom(*gfx);
}

void GraphicsWidget::setGfx(int w, int h, int r, int g, int b) {
	if (w == -1)
		w = _w;
	if (h == -1)
		h = _h;

	_gfx.free();
	_gfx.create(w, h, sizeof(OverlayColor));

	OverlayColor *dst = (OverlayColor*)_gfx.pixels;
	Graphics::PixelFormat overlayFormat = g_system->getOverlayFormat();
	OverlayColor fillCol = overlayFormat.RGBToColor(r, g, b);
	while (h--) {
		for (int i = 0; i < w; ++i) {
			*dst++ = fillCol;
		}
	}
}

void GraphicsWidget::drawWidget() {
	if (sizeof(OverlayColor) == _gfx.bytesPerPixel && _gfx.pixels)
		g_gui.theme()->drawSurface(Common::Rect(_x, _y, _x+_w, _y+_h), _gfx, _state, _alpha, _transparency);
}

#pragma mark -

ContainerWidget::ContainerWidget(GuiObject *boss, int x, int y, int w, int h) : Widget(boss, x, y, w, h) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG);
	_type = kContainerWidget;
}

ContainerWidget::ContainerWidget(GuiObject *boss, const Common::String &name) : Widget(boss, name) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG);
	_type = kContainerWidget;
}

void ContainerWidget::drawWidget() {
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + _h), 0, ThemeEngine::kWidgetBackgroundBorder);
}

} // End of namespace GUI
