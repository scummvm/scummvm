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
#include "gui/widgets/tab.h"
#include "gui/gui-manager.h"

#include "gui/ThemeEval.h"

namespace GUI {

enum {
	kCmdLeft  = 'LEFT',
	kCmdRight = 'RGHT'
};

static const int kTabTitleSpacing = 2 * 5;

TabWidget::TabWidget(GuiObject *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w, h), _bodyBackgroundType(GUI::ThemeEngine::kDialogBackgroundDefault) {
	init();
}

TabWidget::TabWidget(GuiObject *boss, const String &name)
	: Widget(boss, name), _bodyBackgroundType(GUI::ThemeEngine::kDialogBackgroundDefault) {
	init();
}

void TabWidget::init() {
	setFlags(WIDGET_ENABLED | WIDGET_TRACK_MOUSE);
	_type = kTabWidget;
	_activeTab = -1;
	_firstVisibleTab = 0;
	_lastVisibleTab = 0;
	_navButtonsVisible = false;

	_minTabWidth = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Width");
	_tabHeight = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Height");
	_titleVPad = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Padding.Top");

	_bodyTP = g_gui.xmlEval()->getVar("Globals.TabWidget.Body.Padding.Top");
	_bodyBP = g_gui.xmlEval()->getVar("Globals.TabWidget.Body.Padding.Bottom");
	_bodyLP = g_gui.xmlEval()->getVar("Globals.TabWidget.Body.Padding.Left");
	_bodyRP = g_gui.xmlEval()->getVar("Globals.TabWidget.Body.Padding.Right");

	_butRP = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Padding.Right", 0);
	_butTP = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Padding.Top", 0);
	_butW = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Width", 10);
	_butH = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Height", 10);

	int x = _w - _butRP - _butW * 2 - 2;
	int y = _butTP - _tabHeight;

	String leftArrow = g_gui.useRTL() ? ">" : "<";
	String rightArrow = g_gui.useRTL() ? "<" : ">";

	_navLeft = new ButtonWidget(this, x, y, _butW, _butH, Common::U32String(leftArrow), Common::U32String(), kCmdLeft);
	_navRight = new ButtonWidget(this, x + _butW + 2, y, _butW, _butH, Common::U32String(rightArrow), Common::U32String(), kCmdRight);

	_navLeft->setEnabled(false);
	_navRight->setEnabled(true);

	_lastRead = -1;
}

TabWidget::~TabWidget() {
	// If widgets were added or removed in the current tab, without tabs
	// having been switched using setActiveTab() afterward, then the
	// firstWidget in the _tabs list for the active tab may not be up to
	// date. So update it now.
	if (_activeTab != -1)
		_tabs[_activeTab].firstWidget = _firstWidget;
	_firstWidget = nullptr;
	for (uint i = 0; i < _tabs.size(); ++i) {
		delete _tabs[i].firstWidget;
		_tabs[i].firstWidget = nullptr;
	}
	_tabs.clear();
	delete _navRight;
}

int16 TabWidget::getChildY() const {
	// NOTE: if you change that, make sure to do the same
	// changes in the ThemeLayoutTabWidget (gui/ThemeLayout.cpp)
	return getAbsY() + _tabHeight;
}

uint16 TabWidget::getHeight() const {
	// NOTE: if you change that, make sure to do the same
	// changes in the ThemeLayoutTabWidget (gui/ThemeLayout.cpp)
	// NOTE: this height is used for clipping, so it *includes*
	// tabs, because it starts from getAbsY(), not getChildY()
	return _h + _tabHeight;
}

int TabWidget::addTab(const U32String &title, const String &dialogName) {
	// Add a new tab page
	Tab newTab;
	newTab.title = title;
	newTab.dialogName = dialogName;
	newTab.firstWidget = nullptr;

	// Determine the new tab width
	int newWidth = g_gui.getStringWidth(title) + kTabTitleSpacing;
	if (newWidth < _minTabWidth)
		newWidth = _minTabWidth;
	newTab._tabWidth = newWidth;

	_tabs.push_back(newTab);

	int numTabs = _tabs.size();

	// Activate the new tab
	setActiveTab(numTabs - 1);

	return _activeTab;
}

void TabWidget::removeTab(int tabID) {
	assert(0 <= tabID && tabID < (int)_tabs.size());

	// Deactivate the tab if it's currently the active one
	if (tabID == _activeTab) {
		_tabs[tabID].firstWidget = _firstWidget;
		releaseFocus();
		_firstWidget = nullptr;
	}

	// Dispose the widgets in that tab and then the tab itself
	delete _tabs[tabID].firstWidget;
	_tabs.remove_at(tabID);

	// Adjust _firstVisibleTab if necessary
	if (_firstVisibleTab >= (int)_tabs.size()) {
		_firstVisibleTab = MAX(0, (int)_tabs.size() - 1);
	}

	// The active tab was removed, so select a new active one (if any remains)
	if (tabID == _activeTab) {
		_activeTab = -1;
		if (tabID >= (int)_tabs.size())
			tabID = _tabs.size() - 1;
		if (tabID >= 0)
			setActiveTab(tabID);
	}

	// Finally trigger a redraw
	g_gui.scheduleTopDialogRedraw();
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

		// Also ensure the tab is visible in the tab bar
		if (_firstVisibleTab > tabID)
			setFirstVisible(tabID, true);
		while (_lastVisibleTab < tabID)
			setFirstVisible(_firstVisibleTab + 1, false);

		g_gui.scheduleTopDialogRedraw();
	}
}


void TabWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	Widget::handleCommand(sender, cmd, data);

	switch (cmd) {
	case kCmdLeft:
		if (!_navRight->isEnabled()) {
			_navRight->setEnabled(true);
		}

		if (_firstVisibleTab > 0) {
			setFirstVisible(_firstVisibleTab - 1);
		}
		if (_firstVisibleTab == 0) {
			_navLeft->setEnabled(false);
		}
		break;

	case kCmdRight:
		if (!_navLeft->isEnabled()) {
			_navLeft->setEnabled(true);
		}

		if (_lastVisibleTab + 1 < (int)_tabs.size()) {
			setFirstVisible(_firstVisibleTab + 1, false);
		}
		if (_lastVisibleTab + 1 == (int)_tabs.size()) {
			_navRight->setEnabled(false);
		}
		break;

	default:
		break;
	}
}

void TabWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	assert(y < _tabHeight);

	if (x < 0)
		return;

	// Determine which tab was clicked
	int tabID;
	for (tabID = _firstVisibleTab; tabID <= _lastVisibleTab; ++tabID) {
		x -= _tabs[tabID]._tabWidth;
		if (x < 0)
			break;
	}

	// If a tab was clicked, switch to that pane
	if (tabID <= _lastVisibleTab)
		setActiveTab(tabID);
}

void TabWidget::handleMouseMoved(int x, int y, int button) {
	if (y < 0 || y >= _tabHeight)
		return;

	if (x < 0)
		return;

	// Determine which tab the mouse is on
	int tabID;
	for (tabID = _firstVisibleTab; tabID <= _lastVisibleTab; ++tabID) {
		x -= _tabs[tabID]._tabWidth;
		if (x < 0)
			break;
	}

	if (tabID <= _lastVisibleTab) {
		if (tabID != _lastRead) {
			read(_tabs[tabID].title);
			_lastRead = tabID;
		}
	}
	else
		_lastRead = -1;
}

bool TabWidget::handleKeyDown(Common::KeyState state) {
	if (state.hasFlags(Common::KBD_SHIFT) && state.keycode == Common::KEYCODE_TAB)
		adjustTabs(kTabBackwards);
	else if (state.keycode == Common::KEYCODE_TAB)
		adjustTabs(kTabForwards);

	return Widget::handleKeyDown(state);
}

void TabWidget::adjustTabs(int value) {
	// Determine which tab is next
	int tabID = _activeTab + value;
	int lastVis = _lastVisibleTab;

	if (tabID >= (int)_tabs.size())
		tabID = 0;
	else if (tabID < 0)
		tabID = ((int)_tabs.size() - 1);

	setActiveTab(tabID);

	if (_navButtonsVisible) {
		if (lastVis != _lastVisibleTab) {
			_navLeft->setEnabled(true);
			_navRight->setEnabled(true);
		}

		if (_firstVisibleTab == 0)
			_navLeft->setEnabled(false);
		else if (_lastVisibleTab == (int)_tabs.size() - 1)
			_navRight->setEnabled(false);
	}
}

int TabWidget::getFirstVisible() const {
	return _firstVisibleTab;
}

void TabWidget::setFirstVisible(int tabID, bool adjustIfRoom) {
	assert(0 <= tabID && tabID < (int)_tabs.size());
	_firstVisibleTab = tabID;

	computeLastVisibleTab(adjustIfRoom);

	g_gui.scheduleTopDialogRedraw(); // TODO: Necessary?
}

void TabWidget::reflowLayout() {
	Widget::reflowLayout();

	// NOTE: if you change that, make sure to do the same
	// changes in the ThemeLayoutTabWidget (gui/ThemeLayout.cpp)
	_tabHeight = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Height");
	_minTabWidth = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Width");
	_titleVPad = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Padding.Top");

	_butRP = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Padding.Right", 0);
	_butTP = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Padding.Top", 0);
	_butW = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Width", 10);
	_butH = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Height", 10);

	// If widgets were added or removed in the current tab, without tabs
	// having been switched using setActiveTab() afterward, then the
	// firstWidget in the _tabs list for the active tab may not be up to
	// date. So update it now.
	if (_activeTab != -1)
		_tabs[_activeTab].firstWidget = _firstWidget;

	for (uint i = 0; i < _tabs.size(); ++i) {
		if (!_tabs[i].dialogName.empty()) {
			g_gui.xmlEval()->reflowDialogLayout(_tabs[i].dialogName, _tabs[i].firstWidget);
		}

		Widget *w = _tabs[i].firstWidget;
		while (w) {
			w->reflowLayout();
			w = w->next();
		}
	}

	for (uint i = 0; i < _tabs.size(); ++i) {
		// Determine the new tab width
		int newWidth = g_gui.getStringWidth(_tabs[i].title) + kTabTitleSpacing;
		if (newWidth < _minTabWidth)
			newWidth = _minTabWidth;
		_tabs[i]._tabWidth = newWidth;
	}

	// See how many tabs fit on screen.
	// We do this in a loop, because it will change if we need to
	// add left/right scroll buttons, if we scroll left to use free
	// space on the right, or a combination of those.
	_navButtonsVisible = _firstVisibleTab > 0;
	do {
		computeLastVisibleTab(true);

		if (_firstVisibleTab > 0 || _lastVisibleTab + 1 < (int)_tabs.size()) {
			if (!_navButtonsVisible)
				_navButtonsVisible = true;
			else
				break;
		} else {
			if (_navButtonsVisible)
				_navButtonsVisible = false;
			else
				break;
		}
	} while (true);

	int x = _w - _butRP - _butW * 2 - 2;
	int y = _butTP - _tabHeight;
	_navLeft->resize(x, y, _butW, _butH);
	_navRight->resize(x + _butW + 2, y, _butW, _butH);
}

void TabWidget::drawWidget() {
	Common::Array<Common::U32String> tabs;
	Common::Array<int> widths;
	for (int i = _firstVisibleTab; i <= _lastVisibleTab; ++i) {
		tabs.push_back(_tabs[i].title);
		widths.push_back(_tabs[i]._tabWidth);
	}

	g_gui.theme()->drawDialogBackground(
			Common::Rect(_x + _bodyLP, _y + _bodyTP, _x + _w - _bodyRP, _y + _h - _bodyBP + _tabHeight),
			_bodyBackgroundType);

	g_gui.theme()->drawTab(Common::Rect(_x, _y, _x + _w, _y + _h), _tabHeight, widths, tabs,
				_activeTab - _firstVisibleTab, (g_gui.useRTL() && _useRTL));
}

void TabWidget::draw() {
	Widget::draw();

	if (_navButtonsVisible) {
		_navLeft->draw();
		_navRight->draw();
	}
}

void TabWidget::markAsDirty() {
	Widget::markAsDirty();

	if (_navButtonsVisible) {
		_navLeft->markAsDirty();
		_navRight->markAsDirty();
	}
}

bool TabWidget::containsWidget(Widget *w) const {
	if (w == _navLeft || w == _navRight || _navLeft->containsWidget(w) || _navRight->containsWidget(w))
		return true;
	return containsWidgetInChain(_firstWidget, w);
}


Widget *TabWidget::findWidget(int x, int y) {
	if (y < _tabHeight) {
		if (_navButtonsVisible) {
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

void TabWidget::computeLastVisibleTab(bool adjustFirstIfRoom) {
	int availableWidth = _w;
	if (_navButtonsVisible)
		availableWidth -= 2 + _butW * 2 + _butRP;

	_lastVisibleTab = _tabs.size() - 1;
	for (int i = _firstVisibleTab; i < (int)_tabs.size(); ++i) {
		if (_tabs[i]._tabWidth > availableWidth) {
			if (i > _firstVisibleTab)
				_lastVisibleTab = i - 1;
			else
				_lastVisibleTab = _firstVisibleTab; // Always show 1
			break;
		}
		availableWidth -= _tabs[i]._tabWidth;
	}

	if (adjustFirstIfRoom) {
		// If possible, scroll to fit if there's unused space to the right
		while (_firstVisibleTab > 0 && _tabs[_firstVisibleTab-1]._tabWidth <= availableWidth) {
			availableWidth -= _tabs[_firstVisibleTab-1]._tabWidth;
			_firstVisibleTab--;
		}
	}
}

} // End of namespace GUI
