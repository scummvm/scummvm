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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include "widget.h"
#include "common/str.h"
#include "common/array.h"

namespace GUI {

class TabWidget : public Widget {
	typedef Common::String String;
	struct Tab {
		String title;
		Widget *firstWidget;
	};
	typedef Common::Array<Tab> TabList;
protected:
	int _activeTab;
	TabList _tabs;
	int _tabWidth;
	int _tabHeight;

	int _tabOffset;
	int _tabSpacing;
	int _tabPadding;

public:
	TabWidget(GuiObject *boss, int x, int y, int w, int h);
	TabWidget(GuiObject *boss, const String &name);
	~TabWidget();

	void init();

	virtual int16	getChildY() const;

	// Problem: how to add items to a tab?
	// First off, widget should allow non-dialog bosses, (i.e. also other widgets)
	// Could add a common base class for Widgets and Dialogs.
	// Then you add tabs using the following method, which returns a unique ID
	int addTab(const String &title);

	// Maybe we need to remove tabs again? Hm
	//void removeTab(int tabID);

	/** Set the active tab by specifying a valid tab ID.
	 * setActiveTab changes the value of _firstWidget. This means new
	 * Widgets are always added to the active tab.
	 */
	void setActiveTab(int tabID);

	virtual void handleMouseDown(int x, int y, int button, int clickCount);
	virtual bool handleKeyDown(uint16 ascii, int keycode, int modifiers);

	virtual void handleScreenChanged();

protected:
	virtual void drawWidget(bool hilite);

	virtual Widget *findWidget(int x, int y);
};

} // End of namespace GUI

#endif
