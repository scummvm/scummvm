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

#ifndef GUI_LISTWIDGET_H
#define GUI_LISTWIDGET_H

#include "gui/editable.h"
#include "common/str.h"

namespace GUI {

class ScrollBarWidget;

enum NumberingMode {
	kListNumberingOff	= -1,
	kListNumberingZero	= 0,
	kListNumberingOne	= 1
};

// Some special commands
enum {
	kListItemDoubleClickedCmd	= 'LIdb',	// double click on item - 'data' will be item index
	kListItemActivatedCmd		= 'LIac',	// item activated by return/enter - 'data' will be item index
	kListSelectionChangedCmd	= 'Lsch'	// selection changed - 'data' will be item index
};

/* ListWidget */
class ListWidget : public EditableWidget, public CommandSender {
public:
	typedef Common::String String;
	typedef Common::StringList StringList;
protected:
	StringList		_list;
	bool			_editable;
	bool			_editMode;
	NumberingMode	_numberingMode;
	int				_currentPos;
	int				_entriesPerPage;
	int				_selectedItem;
	ScrollBarWidget	*_scrollBar;
	int				_currentKeyDown;

	String			_quickSelectStr;
	uint32			_quickSelectTime;

public:
	ListWidget(GuiObject *boss, int x, int y, int w, int h, WidgetSize ws = kDefaultWidgetSize);
	ListWidget(GuiObject *boss, String name, WidgetSize ws = kDefaultWidgetSize);
	virtual ~ListWidget();

	void init(GuiObject *boss, int w, WidgetSize ws);

	void setList(const StringList& list);
	const StringList& getList()	const			{ return _list; }
	int getSelected() const						{ return _selectedItem; }
	void setSelected(int item);
	const String& getSelectedString() const		{ return _list[_selectedItem]; }
	void setNumberingMode(NumberingMode numberingMode)	{ _numberingMode = numberingMode; }
	bool isEditable() const						{ return _editable; }
	void setEditable(bool editable)				{ _editable = editable; }
	void scrollTo(int item);

	virtual void handleTickle();
	virtual void handleMouseDown(int x, int y, int button, int clickCount);
	virtual void handleMouseUp(int x, int y, int button, int clickCount);
	virtual void handleMouseWheel(int x, int y, int direction);
	virtual bool handleKeyDown(uint16 ascii, int keycode, int modifiers);
	virtual bool handleKeyUp(uint16 ascii, int keycode, int modifiers);
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	virtual bool wantsFocus() { return true; }

	// Made startEditMode for SCUMM's SaveLoadChooser
	void startEditMode();
	void endEditMode();

protected:
	void drawWidget(bool hilite);

	int findItem(int x, int y) const;
	void scrollBarRecalc();

	void abortEditMode();

	Common::Rect getEditRect() const;

	void lostFocusWidget();
	void scrollToCurrent();

	int *_textWidth;
};

} // End of namespace GUI

#endif
