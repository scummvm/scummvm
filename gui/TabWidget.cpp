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
#include "gui/TabWidget.h"
#include "gui/dialog.h"
#include "gui/newgui.h"
#include "gui/eval.h"

namespace GUI {

enum {
	kCmdLeft  = 'LEFT',
	kCmdRight = 'RGHT'
};

TabWidget::TabWidget(GuiObject *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w, h) {
	init();
}

TabWidget::TabWidget(GuiObject *boss, const String &name)
	: Widget(boss, name) {
	init();
}

void TabWidget::init() {
	_tabOffset = 0;	// TODO
	_tabSpacing = g_gui.theme()->getTabSpacing();
	_tabPadding = g_gui.theme()->getTabPadding();

	_flags = WIDGET_ENABLED;
	_type = kTabWidget;
	_activeTab = -1;
	_firstVisibleTab = 0;

	_tabWidth = g_gui.evaluator()->getVar("TabWidget.tabWidth");
	_tabHeight = g_gui.evaluator()->getVar("TabWidget.tabHeight");
	_titleVPad = g_gui.evaluator()->getVar("TabWidget.titleVPad");

	_butRP = g_gui.evaluator()->getVar("TabWidget.navButtonRightPad", 0);
	_butTP = g_gui.evaluator()->getVar("TabWidget.navButtonTopPad", 0);
	_butW = g_gui.evaluator()->getVar("TabWidget.navButtonW", 10);
	_butH = g_gui.evaluator()->getVar("TabWidget.navButtonH", 10);

	int x = _w - _butRP - _butW * 2 - 2;
	int y = _butTP - _tabHeight;
	_navLeft = new ButtonWidget(this, x, y, _butW, _butH, "<", kCmdLeft, 0);
	_navLeft->clearHints(THEME_HINT_SAVE_BACKGROUND);
	_navRight = new ButtonWidget(this, x + _butW + 2, y, _butW, _butH, ">", kCmdRight, 0);
	_navRight->clearHints(THEME_HINT_SAVE_BACKGROUND);
}

TabWidget::~TabWidget() {
	for (uint i = 0; i < _tabs.size(); ++i) {
		delete _tabs[i].firstWidget;
		_tabs[i].firstWidget = 0;
	}
	_tabs.clear();
}

int16 TabWidget::getChildY() const {
	return getAbsY() + _tabHeight;
}

int TabWidget::addTab(const String &title) {
	// Add a new tab page
	Tab newTab;
	newTab.title = title;
	newTab.firstWidget = 0;

	_tabs.push_back(newTab);

	int numTabs = _tabs.size();

	if (g_gui.evaluator()->getVar("TabWidget.tabWidth") == 0) {
		if (_tabWidth == 0)
			_tabWidth = 40;
		// Determine the new tab width
		int newWidth = g_gui.getStringWidth(title) + 2 * 3;
		if (_tabWidth < newWidth)
			_tabWidth = newWidth;
		int maxWidth = _w / numTabs;
		if (_tabWidth > maxWidth)
			_tabWidth = maxWidth;
	}

	// Activate the new tab
	setActiveTab(numTabs - 1);

	return _activeTab;
}

void TabWidget::setActiveTab(int tabID) {
	assert(0 <= tabID && tabID < (int)_tabs.size());
	if (_activeTab != tabID) {
		// Exchange the widget lists, and switch to the new tab
		if (_activeTab != -1) {
			_tabs[_activeTab].firstWidget = _firstWidget;
			releaseFocus();
		}
		_activeTab = tabID;
		_firstWidget = _tabs[tabID].firstWidget;
		_boss->draw();
	}
}


void TabWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
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
	}
}

void TabWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	assert(y < _tabHeight);

	// Determine which tab was clicked
	int tabID = -1;
	if (x >= 0 && x % (_tabWidth + _tabSpacing) < _tabWidth) {
		tabID = x / (_tabWidth + _tabSpacing);
		if (tabID >= (int)_tabs.size())
			tabID = -1;
	}

	// If a tab was clicked, switch to that pane
	if (tabID >= 0 && tabID + _firstVisibleTab < (int)_tabs.size()) {
		setActiveTab(tabID + _firstVisibleTab);
	}
}

bool TabWidget::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	// TODO: maybe there should be a way to switch between tabs
	// using the keyboard? E.g. Alt-Shift-Left/Right-Arrow or something
	// like that.
	return Widget::handleKeyDown(ascii, keycode, modifiers);
}

void TabWidget::handleScreenChanged() {
	Widget::handleScreenChanged();

	for (uint i = 0; i < _tabs.size(); ++i) {
		Widget *w = _tabs[i].firstWidget;
		while (w) {
			w->handleScreenChanged();
			w = w->next();
		}
	}

	_tabHeight = g_gui.evaluator()->getVar("TabWidget.tabHeight");
	_tabWidth = g_gui.evaluator()->getVar("TabWidget.tabWidth");
	_titleVPad = g_gui.evaluator()->getVar("TabWidget.titleVPad");

	if (_tabWidth == 0) {
		_tabWidth = 40;
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

	_butRP = g_gui.evaluator()->getVar("TabWidget.navButtonRightPad", 0);
	_butTP = g_gui.evaluator()->getVar("TabWidget.navButtonTopPad", 0);
	_butW = g_gui.evaluator()->getVar("TabWidget.navButtonW", 10);
	_butH = g_gui.evaluator()->getVar("TabWidget.navButtonH", 10);

	int x = _w - _butRP - _butW * 2 - 2 - _boss->getChildX();
	int y = _butTP - _boss->getChildY() - _tabHeight;
	_navLeft->resize(x, y, _butW, _butH);
	_navRight->resize(x + _butW + 2, y, _butW, _butH);

	_tabOffset = 0;	// TODO
	_tabSpacing = g_gui.theme()->getTabSpacing();
	_tabPadding = g_gui.theme()->getTabPadding();
}

void TabWidget::drawWidget(bool hilite) {
	Common::Array<Common::String> tabs;
	for (int i = _firstVisibleTab; i < (int)_tabs.size(); ++i) {
		tabs.push_back(_tabs[i].title);
	}
	g_gui.theme()->drawTab(Common::Rect(_x, _y, _x+_w, _y+_h), _tabHeight, _tabWidth, tabs, _activeTab - _firstVisibleTab, _hints, _titleVPad);
}

void TabWidget::draw() {
	Widget::draw();
	if (_tabWidth * _tabs.size() > _w) {
		_navLeft->draw();
		_navRight->draw();
	}
}

Widget *TabWidget::findWidget(int x, int y) {
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
}

} // End of namespace GUI
