/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
#include "gui/TabWidget.h"
#include "gui/dialog.h"
#include "gui/newgui.h"

namespace GUI {

enum {
	kTabHeight = 16,

	kTabLeftOffset = 4,
	kTabSpacing = 2,
	kTabPadding = 3
};

TabWidget::TabWidget(GuiObject *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w, h) {

	_flags = WIDGET_ENABLED;
	_type = kTabWidget;
	_activeTab = -1;

	_tabWidth = 40;
}

TabWidget::~TabWidget() {
	for (int i = 0; i < _tabs.size(); ++i) {
		delete _tabs[i].firstWidget;
		_tabs[i].firstWidget = 0;
	}
	_tabs.clear();
}

int16 TabWidget::getChildY() const {
	return getAbsY() + kTabHeight;
}

int TabWidget::addTab(const String &title) {
	// Add a new tab page
	Tab newTab;
	 newTab.title = title;
	 newTab.firstWidget = NULL;

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
	assert(0 <= tabID && tabID < _tabs.size());
	if (_activeTab != tabID) {
		// Exchange the widget lists, and switch to the new tab
		if (_activeTab != -1)
			_tabs[_activeTab].firstWidget = _firstWidget;
		_activeTab = tabID;
		_firstWidget = _tabs[tabID].firstWidget;
		_boss->draw();
	}
}


void TabWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	assert(y < kTabHeight);

	// Determine which tab was clicked
	int tabID = -1;
	x -= kTabLeftOffset;
	if (x >= 0 && x % (_tabWidth + kTabSpacing) < _tabWidth) {
		tabID = x / (_tabWidth + kTabSpacing);
		if (tabID >= _tabs.size())
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

void TabWidget::drawWidget(bool hilite) {
	NewGui *gui = &g_gui;
	
	// Draw horizontal line
	gui->hLine(_x + 1, _y + kTabHeight - 2, _x + _w - 2, gui->_shadowcolor);

	// Iterate over all tabs and draw them
	int i, x = _x + kTabLeftOffset;
	for (i = 0; i < _tabs.size(); ++i) {
		NewGuiColor color = (i == _activeTab) ? gui->_color : gui->_shadowcolor;
		int yOffset = (i == _activeTab) ? 0 : 2; 
		gui->box(x, _y + yOffset, _tabWidth, kTabHeight - yOffset, color, color);
		gui->drawString(_tabs[i].title, x + kTabPadding, _y + yOffset / 2 + (kTabHeight - kLineHeight - 1), _tabWidth - 2 * kTabPadding, gui->_textcolor, kTextAlignCenter);
		x += _tabWidth + kTabSpacing;
	}

	// Draw more horizontal lines
	gui->hLine(_x+1, _y + kTabHeight - 1, _x + _w - 2, gui->_color);
	gui->hLine(_x+1, _y + _h - 2, _x + _w - 2, gui->_shadowcolor);
	gui->hLine(_x+1, _y + _h - 1, _x + _w - 2, gui->_color);
}


Widget *TabWidget::findWidget(int x, int y) {
	if (y < kTabHeight) {
		// Click was in the tab area
		return this;
	} else {
		// Iterate over all child widgets and find the one which was clicked
		return Widget::findWidgetInChain(_firstWidget, x, y - kTabHeight);
	}
}

} // End of namespace GUI
