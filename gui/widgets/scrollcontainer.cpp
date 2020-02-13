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
 */

#include "common/util.h"
#include "gui/widgets/scrollcontainer.h"
#include "gui/gui-manager.h"

#include "gui/ThemeEval.h"

namespace GUI {

ScrollContainerWidget::ScrollContainerWidget(GuiObject *boss, int x, int y, int w, int h, uint32 reflowCmd)
	: Widget(boss, x, y, w, h), CommandSender(nullptr), _reflowCmd(reflowCmd) {
	init();
}

ScrollContainerWidget::ScrollContainerWidget(GuiObject *boss, const Common::String &name, const Common::String &dialogName, uint32 reflowCmd)
	: Widget(boss, name), CommandSender(nullptr), _reflowCmd(reflowCmd), _dialogName(dialogName) {
	init();
}

void ScrollContainerWidget::init() {
	setFlags(WIDGET_ENABLED);
	_type = kScrollContainerWidget;
	_backgroundType = ThemeEngine::kWidgetBackgroundPlain;
	_verticalScroll = new ScrollBarWidget(this, _w-16, 0, 16, _h);
	_verticalScroll->setTarget(this);
	_scrolledX = 0;
	_scrolledY = 0;
	_limitH = 140;
	recalc();
}

void ScrollContainerWidget::handleMouseWheel(int x, int y, int direction) {
	_verticalScroll->handleMouseWheel(x, y, direction);
}

void ScrollContainerWidget::recalc() {
	int scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);
	_limitH = _h;

	//calculate virtual height
	const int spacing = g_gui.xmlEval()->getVar("Global.Font.Height", 16); //on the bottom
	int h = 0;
	int min = spacing, max = 0;
	Widget *ptr = _firstWidget;
	while (ptr) {
		if (ptr != _verticalScroll && ptr->isVisible()) {
			int y = ptr->getAbsY() - getChildY();
			min = MIN(min, y - spacing);
			max = MAX(max, y + ptr->getHeight() + spacing);
		}
		ptr = ptr->next();
	}
	h = max - min;

	if (h <= _limitH) _scrolledY = 0;
	if (_scrolledY > h - _limitH) _scrolledY = 0;

	_verticalScroll->_numEntries = h;
	_verticalScroll->_currentPos = _scrolledY;
	_verticalScroll->_entriesPerPage = _limitH;
	_verticalScroll->_singleStep = kLineHeight;
	_verticalScroll->setPos(_w - scrollbarWidth, _scrolledY);
	_verticalScroll->setSize(scrollbarWidth, _limitH-1);
}


ScrollContainerWidget::~ScrollContainerWidget() {}

int16 ScrollContainerWidget::getChildX() const {
	return getAbsX() - _scrolledX;
}

int16 ScrollContainerWidget::getChildY() const {
	return getAbsY() - _scrolledY;
}

uint16 ScrollContainerWidget::getWidth() const {
	return _w - (_verticalScroll->isVisible() ? _verticalScroll->getWidth() : 0);
}

uint16 ScrollContainerWidget::getHeight() const {
	return _limitH;
}

void ScrollContainerWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	Widget::handleCommand(sender, cmd, data);
	switch (cmd) {
	case kSetPositionCmd:
		_scrolledY = _verticalScroll->_currentPos;
		reflowLayout();
		g_gui.scheduleTopDialogRedraw();
		break;
	default:
		break;
	}
}

void ScrollContainerWidget::reflowLayout() {
	Widget::reflowLayout();

	if (!_dialogName.empty()) {
		g_gui.xmlEval()->reflowDialogLayout(_dialogName, _firstWidget);
	}

	//reflow layout of inner widgets
	Widget *ptr = _firstWidget;
	while (ptr) {
		ptr->reflowLayout();
		ptr = ptr->next();
	}

	//hide and move widgets, if needed
	sendCommand(_reflowCmd, 0);

	//recalculate height
	recalc();

	_verticalScroll->setVisible(_verticalScroll->_numEntries > _limitH); //show when there is something to scroll
	_verticalScroll->recalc();
}

void ScrollContainerWidget::drawWidget() {
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + getHeight()), _backgroundType);
}

bool ScrollContainerWidget::containsWidget(Widget *w) const {
	if (w == _verticalScroll || _verticalScroll->containsWidget(w))
		return true;
	return containsWidgetInChain(_firstWidget, w);
}

Widget *ScrollContainerWidget::findWidget(int x, int y) {
	if (_verticalScroll->isVisible() && x >= _w - _verticalScroll->getWidth())
		return _verticalScroll;
	Widget *w = Widget::findWidgetInChain(_firstWidget, x + _scrolledX, y + _scrolledY);
	if (w)
		return w;
	return this;
}

Common::Rect ScrollContainerWidget::getClipRect() const {
	// Make sure the clipping rect contains the scrollbar so it is properly redrawn
	return Common::Rect(getAbsX(), getAbsY(), getAbsX() + _w, getAbsY() + getHeight() - 1); // this -1 is because of container border, which might not be present actually
}

void ScrollContainerWidget::setBackgroundType(ThemeEngine::WidgetBackground backgroundType) {
	_backgroundType = backgroundType;
}

} // End of namespace GUI
