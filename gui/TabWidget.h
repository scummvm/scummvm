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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include "widget.h"
#include "common/str.h"
#include "common/list.h"

namespace GUI {

class TabWidget : public Widget {
	typedef Common::String String;
	struct Tab {
		String title;
		Widget *firstWidget;
	};
	typedef Common::List<Tab> TabList;
protected:
	int _activeTab;
	TabList _tabs;
	int _tabWidth;

public:
	TabWidget(GuiObject *boss, int x, int y, int w, int h);
	~TabWidget();

	virtual int16	getChildY() const;

// use Dialog::releaseFocus() when changing to another tab

// Problem: how to add items to a tab?
// First off, widget should allow non-dialog bosses, (i.e. also other widgets)
// Could add a common base class for Widgets and Dialogs.
// Then you add tabs using the following method, which returns a unique ID
	int addTab(const String &title);
// Maybe we need to remove tabs again? Hm
	//void removeTab(int tabID);
// Setting the active tab:
	void setActiveTab(int tabID);
// setActiveTab changes the value of _firstWidget. This means Widgets added afterwards
// will be added to the active tab.

	virtual void handleMouseDown(int x, int y, int button, int clickCount);
	virtual bool handleKeyDown(uint16 ascii, int keycode, int modifiers);

protected:
	virtual void drawWidget(bool hilite);

	virtual Widget *findWidget(int x, int y);
};

} // End of namespace GUI

#endif
