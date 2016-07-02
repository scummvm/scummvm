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

ScrollContainerWidget::ScrollContainerWidget(GuiObject *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w, h) {
	init();
}

ScrollContainerWidget::ScrollContainerWidget(GuiObject *boss, const Common::String &name)
	: Widget(boss, name) {
	init();
}

void ScrollContainerWidget::init() {
	setFlags(WIDGET_ENABLED);
	_type = kScrollContainerWidget;
	_verticalScroll = new ScrollBarWidget(this, _w-16, 0, 16, _h);
	_verticalScroll->setTarget(this);
	_scrolledX = 0;
	_scrolledY = 0;
	_limitH = 140;
	recalc();
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
		if (ptr != _verticalScroll) {
			int y = ptr->getAbsY() - getChildY();
			min = MIN(min, y - spacing);
			max = MAX(max, y + ptr->getHeight() + spacing);
		}
		ptr = ptr->next();
	}
	h = max - min;

	_verticalScroll->_numEntries = h;
	_verticalScroll->_currentPos = _scrolledY;
	_verticalScroll->_entriesPerPage = _limitH;
	_verticalScroll->setPos(_w - scrollbarWidth, _scrolledY+1);
	_verticalScroll->setSize(scrollbarWidth, _limitH -2);
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
		draw();
		g_gui.doFullRedraw();
		break;
	}
}

void ScrollContainerWidget::reflowLayout() {
	Widget::reflowLayout();

	//reflow layout of inner widgets
	Widget *ptr = _firstWidget;
	while (ptr) {
		ptr->reflowLayout();
		ptr = ptr->next();
	}
	
	//recalculate height
	recalc();

	//hide those widgets which are out of visible area
	ptr = _firstWidget;
	while (ptr) {
		int y = ptr->getAbsY() - getChildY();
		int h = ptr->getHeight();
		bool visible = true;
		if (y + h - _scrolledY < 0) visible = false;
		if (y - _scrolledY > _limitH) visible = false;
		ptr->setVisible(visible);
		ptr = ptr->next();
	}

	_verticalScroll->setVisible(_verticalScroll->_numEntries > _limitH); //show when there is something to scroll
}

void ScrollContainerWidget::drawWidget() {
	g_gui.theme()->drawDialogBackgroundClip(Common::Rect(_x, _y, _x + _w, _y + getHeight() - 1), getBossClipRect(), ThemeEngine::kDialogBackgroundDefault);
}

Widget *ScrollContainerWidget::findWidget(int x, int y) {
	if (_verticalScroll->isVisible() && x >= _w - _verticalScroll->getWidth())
		return _verticalScroll;
	return Widget::findWidgetInChain(_firstWidget, x + _scrolledX, y + _scrolledY);
}

} // End of namespace GUI
