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

namespace GUI {

enum {
	kTabHeight = 16,
	kBigTabHeight = 21,

	kTabLeftOffset = 4,
	kTabSpacing = 2,
	kTabPadding = 3
};

TabWidget::TabWidget(GuiObject *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w, h) {
	init();
}

TabWidget::TabWidget(GuiObject *boss, String name)
	: Widget(boss, name) {
	init();
}

void TabWidget::init() {
	_flags = WIDGET_ENABLED;
	_type = kTabWidget;
	_activeTab = -1;

	_tabWidth = 40;

	if (g_gui.getWidgetSize() == kBigWidgetSize) {
		_tabHeight = kBigTabHeight;
	} else {
		_tabHeight = kTabHeight;
	}
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

	// Determine the new tab width
	int newWidth = g_gui.getStringWidth(title) + 2 * kTabPadding;
	if (_tabWidth < newWidth)
		_tabWidth = newWidth;
	int maxWidth = (_w - kTabLeftOffset) / numTabs - kTabLeftOffset;
	if (_tabWidth > maxWidth)
		_tabWidth = maxWidth;

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


void TabWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	assert(y < _tabHeight);

	// Determine which tab was clicked
	int tabID = -1;
	x -= kTabLeftOffset;
	if (x >= 0 && x % (_tabWidth + kTabSpacing) < _tabWidth) {
		tabID = x / (_tabWidth + kTabSpacing);
		if (tabID >= (int)_tabs.size())
			tabID = -1;
	}

	// If a tab was clicked, switch to that pane
	if (tabID >= 0) {
		setActiveTab(tabID);
	}
}

bool TabWidget::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	// TODO: maybe there should be a way to switch between tabs
	// using the keyboard? E.g. Alt-Shift-Left/Right-Arrow or something
	// like that.
	return Widget::handleKeyDown(ascii, keycode, modifiers);
}

void TabWidget::handleScreenChanged() {
	for (uint i = 0; i < _tabs.size(); ++i) {
		Widget *w = _tabs[i].firstWidget;
		while (w) {
			w->handleScreenChanged();
			w = w->next();
		}
	}
	if (g_gui.getWidgetSize() == kBigWidgetSize) {
		_tabHeight = kBigTabHeight;
	} else {
		_tabHeight = kTabHeight;
	}
	Widget::handleScreenChanged();
}

void TabWidget::drawWidget(bool hilite) {
	Common::Array<Common::String> tabs;
	for (int i = 0; i < (int)_tabs.size(); ++i) {
		tabs.push_back(_tabs[i].title);
	}
	g_gui.theme()->drawTab(Common::Rect(_x, _y, _x+_w, _y+_h), _tabHeight, _tabWidth, tabs, _activeTab, _hints);
}

Widget *TabWidget::findWidget(int x, int y) {
	if (y < _tabHeight) {
		// Click was in the tab area
		return this;
	} else {
		// Iterate over all child widgets and find the one which was clicked
		return Widget::findWidgetInChain(_firstWidget, x, y - _tabHeight);
	}
}

} // End of namespace GUI
