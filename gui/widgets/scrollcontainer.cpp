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
	_verticalScroll = nullptr;
	_verticalScroll = new ScrollBarWidget(this, _w-16, 0, 16, _h);
	_verticalScroll->setTarget(this);
	//_navRight = new ButtonWidget(this, x + _butW + 2, y, _butW, _butH, ">", 0, kCmdRight);
	_scrolledX = 0;
	_scrolledY = 0;
	_limitH = 140;
	_clippingArea = Common::Rect(0, 0, _w, _h);
	recalc();
}

void ScrollContainerWidget::recalc() {
	_verticalScroll->_numEntries = _h;
	_verticalScroll->_currentPos = _scrolledY;
	_verticalScroll->_entriesPerPage = _limitH;
	_verticalScroll->setPos(_w - 16, _scrolledY);
	_verticalScroll->setSize(16, _limitH);
	debug("%d %d", _boss->getHeight(), _h);
}


ScrollContainerWidget::~ScrollContainerWidget() {
	_firstWidget = 0;
	//delete _navRight;
}

int16 ScrollContainerWidget::getChildX() const {
	return getAbsX() - _scrolledX;// +_tabHeight;
}

int16 ScrollContainerWidget::getChildY() const {
	return getAbsY() - _scrolledY;// +_tabHeight;
}

uint16 ScrollContainerWidget::getWidth() const {
	return (_boss ? _boss->getWidth() : _w);
}

uint16 ScrollContainerWidget::getHeight() const {
	return _limitH;
}

void ScrollContainerWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	Widget::handleCommand(sender, cmd, data);	
	switch (cmd) {
	case kSetPositionCmd:
		_scrolledY = _verticalScroll->_currentPos;
		recalc();
		draw();
		break;
		/*
	case kCmdLeft:
		if (_firstVisibleTab) {
			_firstVisibleTab--;
			draw();
		}
		break;

	case kCmdRight:
		if (_firstVisibleTab + _w / _tabWidth < (int)_tabs.size()) {
			_firstVisibleTab++;
			draw();
		}
		break;
		*/
	}	
}

void ScrollContainerWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	/*
	assert(y < _tabHeight);

	// Determine which tab was clicked
	int tabID = -1;
	if (x >= 0 && (x % _tabWidth) < _tabWidth) {
		tabID = x / _tabWidth;
		if (tabID >= (int)_tabs.size())
			tabID = -1;
	}

	// If a tab was clicked, switch to that pane
	if (tabID >= 0 && tabID + _firstVisibleTab < (int)_tabs.size()) {
		setActiveTab(tabID + _firstVisibleTab);
	}
	*/
}

bool ScrollContainerWidget::handleKeyDown(Common::KeyState state) {
	/*
	if (state.hasFlags(Common::KBD_SHIFT) && state.keycode == Common::KEYCODE_TAB)
		adjustTabs(kTabBackwards);
	else if (state.keycode == Common::KEYCODE_TAB)
		adjustTabs(kTabForwards);
		*/
	return Widget::handleKeyDown(state);
}

void ScrollContainerWidget::reflowLayout() {
	_clippingArea = Common::Rect(0, 0, _w, _h);
	recalc();
	Widget::reflowLayout();

	/*
	for (uint i = 0; i < _tabs.size(); ++i) {
		Widget *w = _tabs[i].firstWidget;
		while (w) {
			w->reflowLayout();
			w = w->next();
		}
	}

	_tabHeight = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Height");
	_tabWidth = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Width");
	_titleVPad = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Padding.Top");

	if (_tabWidth == 0) {
		_tabWidth = 40;
#ifdef __DS__
	}
	if (true) {
#endif
		int maxWidth = _w / _tabs.size();

		for (uint i = 0; i < _tabs.size(); ++i) {
			// Determine the new tab width
			int newWidth = g_gui.getStringWidth(_tabs[i].title) + 2 * 3;
			if (_tabWidth < newWidth)
				_tabWidth = newWidth;
			if (_tabWidth > maxWidth)
				_tabWidth = maxWidth;
		}
	}

	_butRP = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.PaddingRight", 0);
	_butTP = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Padding.Top", 0);
	_butW = g_gui.xmlEval()->getVar("GlobalsTabWidget.NavButton.Width", 10);
	_butH = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Height", 10);

	int x = _w - _butRP - _butW * 2 - 2;
	int y = _butTP - _tabHeight;
	_navLeft->resize(x, y, _butW, _butH);
	_navRight->resize(x + _butW + 2, y, _butW, _butH);
	*/
}

void ScrollContainerWidget::drawWidget() {
	/*
	Common::Array<Common::String> tabs;
	for (int i = _firstVisibleTab; i < (int)_tabs.size(); ++i) {
		tabs.push_back(_tabs[i].title);
	}	
	g_gui.theme()->drawDialogBackground(Common::Rect(_x + _bodyLP, _y + _bodyTP, _x+_w-_bodyRP, _y+_h-_bodyBP), _bodyBackgroundType);
	*/
	g_gui.theme()->drawDialogBackground(Common::Rect(_x, _y, _x + _w, _y + getHeight()), ThemeEngine::kDialogBackgroundDefault);
	//g_gui.theme()->addDirtyRect(Common::Rect(_x, _y, _x + _w, _y + getHeight()));
	/*
	g_gui.theme()->drawTab(Common::Rect(_x, _y, _x+_w, _y+_h), _tabHeight, _tabWidth, tabs, _activeTab - _firstVisibleTab, 0, _titleVPad);
	*/
}

void ScrollContainerWidget::draw() {
	Widget::draw();
	/*
	if (_tabWidth * _tabs.size() > _w) {
		_navLeft->draw();
		_navRight->draw();
	}
	*/
}

Widget *ScrollContainerWidget::findWidget(int x, int y) {
	/*
	if (y < _tabHeight) {
		if (_tabWidth * _tabs.size() > _w) {
			if (y >= _butTP && y < _butTP + _butH) {
				if (x >= _w - _butRP - _butW * 2 - 2 && x < _w - _butRP - _butW - 2)
					return _navLeft;
				if (x >= _w - _butRP - _butW &&  x < _w - _butRP)
					return _navRight;
			}
		}

		// Click was in the tab area
		return this;
	} else {	
		// Iterate over all child widgets and find the one which was clicked
		return Widget::findWidgetInChain(_firstWidget, x, y - _tabHeight);
	}
	*/
	return Widget::findWidgetInChain(_firstWidget, x + _scrolledX, y + _scrolledY);
}

} // End of namespace GUI
