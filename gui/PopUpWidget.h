/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#ifndef POPUPWIDGET_H
#define POPUPWIDGET_H

#include "widget.h"
#include "common/str.h"
#include "common/list.h"

enum {
	kPopUpItemSelectedCmd	= 'POPs'
};

/* PopUpWidget
 * A popup or dropdown widget which, when clicked, "pop up" a list of items and
 * lets the user pick on of them.
 *
 * Implementation wise, when the user selects an item, then a kPopUpItemSelectedCmd 
 * is broadcast, with data being equal to the tag value of the selected entry.
 */
class PopUpWidget : public Widget, public CommandSender {
	friend class PopUpDialog;
	typedef Common::String String;
	
	struct Entry {
		String	name;
		uint32	tag;
	};
	typedef Common::List<Entry> EntryList;
protected:
	static const String emptyStr;

	EntryList		_entries;
	int				_selectedItem;

public:
	PopUpWidget(Dialog *boss, int x, int y, int w, int h);

	void handleMouseDown(int x, int y, int button, int clickCount);
/*
	void handleMouseUp(int x, int y, int button, int clickCount);
//	void handleMouseWheel(int x, int y, int direction);	// Scroll through entries with scroll wheel
	void handleMouseMoved(int x, int y, int button);	// Redraw selections depending on mouse position
//	bool handleKeyDown(uint16 ascii, int keycode, int modifiers);	// Scroll through entries with arrow keys etc.
//	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
*/

	void appendEntry(const String &entry, uint32 tag = (uint32)-1);
//	void setEntries(const EntryList &entries);
	void clearEntries();

	void setSelected(int item);
	int getSelected() const						{ return _selectedItem; }
	const String& getSelectedString() const		{ return (_selectedItem >= 0) ? _entries[_selectedItem].name : emptyStr; }

protected:
	void drawWidget(bool hilite);
};

#endif
